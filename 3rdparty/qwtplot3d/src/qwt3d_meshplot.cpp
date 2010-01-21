#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning ( disable : 4305 )
#pragma warning ( disable : 4786 )
#endif

#include "qwt3d_curve.h"
#include "qwt3d_enrichment_std.h"

using namespace std;
using namespace Qwt3D;


/////////////////////////////////////////////////////////////////////////////////
//
//     cell specific
//


void Curve::createDataC()
{		
	if (facemode_)	createFaceData();
	if (sidemode_)	createSideData();
	if (floormode_)	createFloorData();
  
  if (plotStyle() == NOPLOT)
    return;

  if (plotStyle() == Qwt3D::POINTS)
  {
    createPoints();
    return;
  }
  else if (plotStyle() == Qwt3D::USER)
  {
    if (userplotstyle_p)
      createEnrichment(*userplotstyle_p);
    return;
  }

    glPushAttrib(GL_POLYGON_BIT|GL_LINE_BIT|GL_COLOR_BUFFER_BIT);
	setDeviceLineWidth(meshLineWidth());
  GLStateBewarer sb(GL_POLYGON_OFFSET_FILL,true);
	setDevicePolygonOffset(polygonOffset(),1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int idx = 0;
	if (plotStyle() != WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_QUADS);

		bool hl = (plotStyle() == HIDDENLINE);
		if (hl)
		{
            RGBA col = plot_p->backgroundRGBAColor();
			glColor4d(col.r, col.g, col.b, col.a);
		}
		
		for (unsigned i=0; i!=actualDataC_->cells.size(); ++i)
		{
			glBegin(GL_POLYGON);
			for (unsigned j=0; j!=actualDataC_->cells[i].size(); ++j)
			{
				idx = actualDataC_->cells[i][j];
				setColorFromVertexC(idx, hl);
				glVertex3d( actualDataC_->nodes[idx].x, actualDataC_->nodes[idx].y, actualDataC_->nodes[idx].z );
				glNormal3d( actualDataC_->normals[idx].x, actualDataC_->normals[idx].y, actualDataC_->normals[idx].z );
			}
			glEnd();
		}
	}

	if (plotStyle() == FILLEDMESH || plotStyle() == WIREFRAME || plotStyle() == HIDDENLINE)
	{
		glColor4d(meshColor().r, meshColor().g, meshColor().b, meshColor().a);
		{
			for (unsigned i=0; i!=actualDataC_->cells.size(); ++i)
			{
				glBegin(GL_LINE_LOOP);
				for (unsigned j=0; j!=actualDataC_->cells[i].size(); ++j)
				{
					idx = actualDataC_->cells[i][j];
					glVertex3d( actualDataC_->nodes[idx].x, actualDataC_->nodes[idx].y, actualDataC_->nodes[idx].z );
				}
				glEnd();
			}
		}
	}
    glPopAttrib();
}

// ci = cell index
// cv = vertex index in cell ci
void Curve::setColorFromVertexC(int node, bool skip)
{
	if (skip)
		return;

	RGBA col = (*datacolor_p)(
		actualDataC_->nodes[node].x, actualDataC_->nodes[node].y, actualDataC_->nodes[node].z);
		
	glColor4d(col.r, col.g, col.b, col.a);
}

void Curve::createFloorDataC()
{
	switch (floorStyle())
	{
	case FLOORDATA:
		Data2FloorC();
		break;
	case FLOORISO:
		Isolines2FloorC(dataProjected());
		break;
	default:
		break;
	}
}

void Curve::createSideDataC()
{
	switch (floorStyle())
	{
	case FLOORDATA:
		Data2SideC();
		break;
	case FLOORISO:
		Isolines2SideC(dataProjected());
		if (dataProjected())	createPoints();
		break;
	default:
		break;
	}
}

void Curve::createFaceDataC()
{
	switch (floorStyle())
	{
	case FLOORDATA:
		Data2FrontC();
		break;
	case FLOORISO:
		Isolines2FrontC(dataProjected());
		if (dataProjected())	createPoints();
		break;
	default:
		break;
	}
}

void Curve::DatamapC(unsigned int comp)
{	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	Triple tmin = actualData_p->hull().minVertex;
	double shift = tmin(comp);

	for (unsigned i = 0; i!=actualDataC_->cells.size(); ++i) {
		glBegin(GL_POLYGON);
			for (unsigned j=0; j!=actualDataC_->cells[i].size(); ++j) {
				int	idx	= actualDataC_->cells[i][j];
				Triple curr = actualDataC_->nodes[idx];
				
				setColorFromVertexC(idx);
				drawVertex(curr, shift, comp);
			}
		glEnd();
	}
}

void Curve::IsolinesC(unsigned comp, bool projected)
{
	if (isolines() <= 0 || actualData_p->empty())
		return;

	Triple tmax = actualData_p->hull().maxVertex;
	Triple tmin = actualData_p->hull().minVertex;
		
	double delta = tmax(comp) - tmin(comp);
	double shift = tmin(comp);
	double step  = delta / isolines();
	
	RGBA col;

	TripleField nodes;
	TripleField intersection;
	
	double lambda = 0;
	
	GLStateBewarer sb2(GL_LINE_SMOOTH, false);

	for (unsigned int k = 0; k != isolines(); ++k) {
		double val = shift + k * step;		
		
		for (unsigned int i = 0; i != actualDataC_->cells.size(); ++i) {
			nodes.clear();
			unsigned int cellnodes = actualDataC_->cells[i].size();
			for (unsigned int j = 0; j != cellnodes; ++j) {
				nodes.push_back(actualDataC_->nodes[actualDataC_->cells[i][j]]);
			}

			double diff = 0;
			for (unsigned int m = 0; m != cellnodes; ++m) {
				unsigned int mm = (m+1) % cellnodes;

				bool outer = (val >= nodes[mm](comp) && val <= nodes[m](comp));
				bool inner = (val >= nodes[m](comp) && val <= nodes[mm](comp));

				if (inner || outer) {
					diff = nodes[mm](comp) - nodes[m](comp);

					if (isPracticallyZero(diff)) {			// degenerated
						intersection.push_back(nodes[m]);
						intersection.push_back(nodes[mm]);
						continue;
					}

					Triple intersect;
					double component[3];

					lambda = (val - nodes[m](comp)) / diff;

					for (unsigned int c = 0; c!=3; ++c) {
						component[c] = (nodes[m](c) + lambda * (nodes[mm](c)-nodes[m](c)));
					}

					switch (comp) {
					case 0:
						intersect = Triple(val, component[1], component[2]);
						break;
					case 1:
						intersect = Triple(component[0], val, component[2]);
						break;
					case 2:
						intersect = Triple(component[0], component[1], val);
						break;
					}

					intersection.push_back(intersect);
				}
			}
			col = (*datacolor_p)(nodes[0].x,nodes[0].y,nodes[0].z);
			glColor4d(col.r, col.g, col.b, col.a);

			drawIntersections(intersection, shift, comp, projected);
		}
	}
}

void Curve::createNormalsC()
{
	if (!normals() || actualData_p->empty())
		return;

	if (actualDataC_->nodes.size() != actualDataC_->normals.size())
		return;
  Arrow arrow;
  arrow.setQuality(normalQuality());

	Triple basev, topv, norm;	
		
	double diag = (actualData_p->hull().maxVertex-actualData_p->hull().minVertex).length() * normalLength();

  arrow.assign(*this);
  arrow.drawBegin();
	for (unsigned i = 0; i != actualDataC_->normals.size(); ++i) 
	{
		basev = actualDataC_->nodes[i];
		topv = basev + actualDataC_->normals[i];
		
			norm = topv-basev;
			norm.normalize();
			norm	*= diag;

      arrow.setTop(basev+norm);
      arrow.setColor((*datacolor_p)(basev.x,basev.y,basev.z));
      arrow.draw(basev);
	}
  arrow.drawEnd();
}

/*! 
	Convert user (non-rectangular) mesh based data to internal structure.
	See also Qwt3D::TripleField and Qwt3D::CellField
*/
bool Curve::loadFromData(TripleField const& data, CellField const& poly, QString titlestr)
{	
	actualDataG_->clear();
	actualData_p = actualDataC_;

	actualDataC_->datatype = Qwt3D::POLYGON;
	actualDataC_->nodes = data;
	actualDataC_->cells = poly;
	actualDataC_->normals = TripleField(actualDataC_->nodes.size());

	if (!titlestr.isEmpty())	setTitle(titlestr);

	unsigned i;

//  normals for the moment
	Triple n, u, v;
	for ( i = 0; i < poly.size(); ++i) 
	{
		if (poly[i].size() < 3)
			n = Triple(0,0,0);
		else
		{
			for (unsigned j = 0; j < poly[i].size(); ++j) 
			{
				unsigned jj = (j+1) % poly[i].size(); 
				unsigned pjj = (j) ? j-1 : poly[i].size()-1;
				u = actualDataC_->nodes[poly[i][jj]]-actualDataC_->nodes[poly[i][j]];		
				v = actualDataC_->nodes[poly[i][pjj]]-actualDataC_->nodes[poly[i][j]];
				n = normalizedcross(u,v);
				actualDataC_->normals[poly[i][j]] += n;
			}
		}
	}
	for ( i = 0; i != actualDataC_->normals.size(); ++i) 
	{
		actualDataC_->normals[i].normalize();
	}  
	
	ParallelEpiped hull(Triple(DBL_MAX,DBL_MAX,DBL_MAX),Triple(-DBL_MAX,-DBL_MAX,-DBL_MAX));

	for (i = 0; i!=data.size(); ++i)
	{
		if (data[i].x < hull.minVertex.x)
			hull.minVertex.x = data[i].x;
		if (data[i].y < hull.minVertex.y)
			hull.minVertex.y = data[i].y;
		if (data[i].z < hull.minVertex.z)
			hull.minVertex.z = data[i].z;
		
		if (data[i].x > hull.maxVertex.x)
			hull.maxVertex.x = data[i].x;
		if (data[i].y > hull.maxVertex.y)
			hull.maxVertex.y = data[i].y;
		if (data[i].z > hull.maxVertex.z)
			hull.maxVertex.z = data[i].z;
	}

	actualDataC_->setHull(hull);
	emit readInFinished(title()->string());

	updateData();
	return true;
}	

TripleField* Curve::getNodeData(int *nodes)
{
	if (!actualDataC_)	return 0;

	*nodes = actualDataC_->nodes.size();

	/* allocate some space for the nodes */
	TripleField* nodeData = new TripleField();

	*nodeData = actualDataC_->nodes;

	return nodeData;
}

CellField* Curve::getCellData(int *cells)
{
	if (!actualDataC_)	return 0;

	*cells = actualDataC_->cells.size();

	/* allocate some space for the cells */
	CellField* cellData = new CellField();

	*cellData = actualDataC_->cells;

	return cellData;
}

void Curve::deleteData(TripleField* data)
{
	data->clear();
	delete data;
}

void Curve::deleteData(CellField* poly)
{
	poly->clear();
	delete poly;
}

void Curve::animateData(TripleField* data)
{
	if (!actualDataC_)	return;

	actualDataC_->nodes = *data;
}

void Curve::animateData(TripleField* data, CellField* poly)
{
	if (!actualDataC_)	return;

	actualDataC_->nodes = *data;
	actualDataC_->cells = *poly;
}
