#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning ( disable : 4305 )
#pragma warning ( disable : 4786 )
#endif

#include "qwt3d_curve.h"
#include "qwt3d_enrichment_std.h"

using namespace std;
using namespace Qwt3D;

void Curve::createDataG()
{
	if (plotStyle() == NOPLOT)
		return;

	if (facemode_)	createFaceData();
	if (sidemode_)	createSideData();
	if (floormode_)	createFloorData();

	if (plotStyle() == Qwt3D::POINTS){
		createPoints();
		return;
	} else if (plotStyle() == Qwt3D::USER && userplotstyle_p){
		createEnrichment(*userplotstyle_p);
		return;
	}

	Qwt3D::GridData *backup = plot_p->transform(actualDataG_);//axis scale transformation

	int i, j, step = resolution();

	glPushAttrib(GL_POLYGON_BIT|GL_LINE_BIT);

	setDeviceLineWidth(meshLineWidth());

	GLStateBewarer sb(GL_POLYGON_OFFSET_FILL, true);
	setDevicePolygonOffset(polygonOffset(), 1.0);

	GLStateBewarer sb2(GL_LINE_SMOOTH, smoothDataMesh());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int lastcol = actualDataG_->columns();
	int lastrow = actualDataG_->rows();
 
	if (plotStyle() != WIREFRAME){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		bool hl = (plotStyle() == HIDDENLINE);
		if (hl){
			RGBA col = plot_p->backgroundRGBAColor();
			glColor4d(col.r, col.g, col.b, col.a);
		}

		for (i = 0; i < lastcol - step; i += step){
			glBegin(GL_TRIANGLE_STRIP);

			setColorFromVertexG(backup->vertices[i][0], hl);
			glNormal3dv(actualDataG_->normals[i][0]);
			glVertex3dv(actualDataG_->vertices[i][0]);

			int ni = i + step;

			setColorFromVertexG(backup->vertices[ni][0], hl);
			glNormal3dv(actualDataG_->normals[ni][0]);
			glVertex3dv(actualDataG_->vertices[ni][0]);

			for (j = 0; j < lastrow - step; j += step){
				int nj = j + step;

				setColorFromVertexG(backup->vertices[i][nj], hl);
				glNormal3dv(actualDataG_->normals[i][nj]);
				glVertex3dv(actualDataG_->vertices[i][nj]);

				setColorFromVertexG(backup->vertices[ni][nj], hl);
				glNormal3dv(actualDataG_->normals[ni][nj]);
				glVertex3dv(actualDataG_->vertices[ni][nj]);
			}
			glEnd();
		}
	}

	if (plotStyle() == FILLEDMESH || plotStyle() == WIREFRAME || plotStyle() == HIDDENLINE){
		glColor4d(meshColor().r, meshColor().g, meshColor().b, meshColor().a);

		if (step < lastcol && step < lastrow){
			glBegin(GL_LINE_LOOP);
				for (i = 0; i < lastcol - step; i += step)
					glVertex3dv(actualDataG_->vertices[i][0]);
				for (j = 0; j < actualDataG_->rows() - step; j += step) 
					glVertex3dv(actualDataG_->vertices[i][j]);
				for (; i >= 0; i -= step) 
					glVertex3dv(actualDataG_->vertices[i][j]);
				for (; j >= 0; j -= step) 
					glVertex3dv(actualDataG_->vertices[0][j]);
			glEnd();
		}

		// weaving
		for (i = step; i < lastcol - step; i += step){
			glBegin(GL_LINE_STRIP);
				for (j = 0; j < lastrow; j += step)
					glVertex3dv(actualDataG_->vertices[i][j]);
			glEnd();
		}
		for (j = step; j < lastrow - step; j += step){
			glBegin(GL_LINE_STRIP);
				for (i = 0; i < lastcol; i += step)
					glVertex3dv(actualDataG_->vertices[i][j]);
			glEnd();
		}
	}

	glPopAttrib();

	if (backup != actualDataG_){
		delete  actualDataG_;
		actualDataG_ = backup;
		actualData_p = actualDataG_;
	}
}

void Curve::setColorFromVertexG(double *v, bool skip)
{
	if (skip)
		return;

	RGBA col = (*datacolor_p)(v[0], v[1], v[2]);
	glColor4d(col.r, col.g, col.b, col.a);
}

void Curve::createNormalsG()
{
	if (!normals() || actualDataG_->empty())
		return;

	Arrow arrow;
	arrow.setQuality(normalQuality());

	Triple basev, topv, norm;	
	
	int step = resolution();

	double diag = (actualDataG_->hull().maxVertex-actualDataG_->hull().minVertex).length() * normalLength();

	arrow.assign(*this);
	arrow.drawBegin();
	for (int i = 0; i <= actualDataG_->columns() - step; i += step){
		for (int j = 0; j <= actualDataG_->rows() - step; j += step){
			basev = Triple(actualDataG_->vertices[i][j][0],actualDataG_->vertices[i][j][1],actualDataG_->vertices[i][j][2]);
			topv = Triple(actualDataG_->vertices[i][j][0]+actualDataG_->normals[i][j][0],
							 actualDataG_->vertices[i][j][1]+actualDataG_->normals[i][j][1],
							 actualDataG_->vertices[i][j][2]+actualDataG_->normals[i][j][2]);	
			
			norm = topv-basev;
			norm.normalize();
			norm	*= diag;

			arrow.setTop(basev+norm);
			arrow.setColor((*datacolor_p)(basev.x,basev.y,basev.z));
			arrow.draw(basev);
		}
	}
	arrow.drawEnd();
}

void Curve::readIn(GridData& gdata, Triple** data, unsigned int columns, unsigned int rows)
{
	gdata.setSize(columns,rows);
	
	ParallelEpiped range(Triple(DBL_MAX,DBL_MAX,DBL_MAX),Triple(-DBL_MAX,-DBL_MAX,-DBL_MAX));

	/* fill out the vertex array for the mesh. */
	for (unsigned i = 0; i != columns; ++i){
		for (unsigned j = 0; j != rows; ++j){
			gdata.vertices[i][j][0] = data[i][j].x; 
			gdata.vertices[i][j][1] = data[i][j].y;
			gdata.vertices[i][j][2] = data[i][j].z;
//			qDebug() << "Curve: Data Values - " << data[i][j].x << data[i][j].y << data[i][j].z;

			if (data[i][j].x > range.maxVertex.x)
				range.maxVertex.x = data[i][j].x;
			if (data[i][j].y > range.maxVertex.y)
				range.maxVertex.y = data[i][j].y;
			if (data[i][j].z > range.maxVertex.z)
				range.maxVertex.z = data[i][j].z;
			if (data[i][j].x < range.minVertex.x)
				range.minVertex.x = data[i][j].x;
			if (data[i][j].y < range.minVertex.y)
				range.minVertex.y = data[i][j].y;
			if (data[i][j].z < range.minVertex.z)
				range.minVertex.z = data[i][j].z;
 		}
	}
	gdata.setHull(range);
	emit readInFinished(title()->string());
}

void Curve::readIn(GridData& gdata, double** data, unsigned int columns, unsigned int rows,
				   double minx, double maxx, double miny, double maxy)
{
	gdata.setPeriodic(false,false);
	gdata.setSize(columns, rows);
	
	double dx = (maxx - minx) / (gdata.columns() - 1);
	double dy = (maxy - miny) / (gdata.rows() - 1);

	double tmin = DBL_MAX;
	double tmax = -DBL_MAX;

	/* fill out the vertex array for the mesh. */
	for (unsigned i = 0; i != columns; ++i){
		for (unsigned j = 0; j != rows; ++j){
			gdata.vertices[i][j][0] = minx + i*dx;
			gdata.vertices[i][j][1] = miny + j*dy;
			gdata.vertices[i][j][2] = data[i][j];

			double val = data[i][j];
			if (val > tmax)
				tmax = val;
			if (val < tmin)
				tmin = val;
 		}
	}

	ParallelEpiped hull = ParallelEpiped(Triple(gdata.vertices[0][0][0], gdata.vertices[0][0][1], tmin), 
										Triple(gdata.vertices[gdata.columns() - 1][gdata.rows() - 1][0],
											   gdata.vertices[gdata.columns() - 1][gdata.rows() - 1][1], tmax));

	gdata.setHull(hull);
	emit readInFinished(title()->string());
}

void Curve::calcNormals(GridData& gdata)
{
	unsigned int rows = gdata.rows();
	unsigned int columns = gdata.columns();
  
	// normals
	Triple u, v, n;  // for cross product

	for (unsigned i = 0; i != columns; ++i) {
		for (unsigned j = 0; j != rows; ++j) {
			n = Triple(0,0,0);

			if (i<columns-1 && j<rows-1) {
				/*	get two vectors to cross */
				u = Triple(gdata.vertices[i+1][j][0] - gdata.vertices[i][j][0],
					gdata.vertices[i+1][j][1] - gdata.vertices[i][j][1],
					gdata.vertices[i+1][j][2] - gdata.vertices[i][j][2]);

				v = Triple(
				gdata.vertices[i][j+1][0] - gdata.vertices[i][j][0],
				gdata.vertices[i][j+1][1] - gdata.vertices[i][j][1],
				gdata.vertices[i][j+1][2] - gdata.vertices[i][j][2]);

				/* get the normalized cross product */ 
				n += normalizedcross(u,v); // right hand system here !
			}

			if (i>0 && j<rows-1) {
				u = Triple(gdata.vertices[i][j+1][0] - gdata.vertices[i][j][0],
					gdata.vertices[i][j+1][1] - gdata.vertices[i][j][1],
					gdata.vertices[i][j+1][2] - gdata.vertices[i][j][2]);
			  
				v = Triple(gdata.vertices[i-1][j][0] - gdata.vertices[i][j][0],
					gdata.vertices[i-1][j][1] - gdata.vertices[i][j][1],
					gdata.vertices[i-1][j][2] - gdata.vertices[i][j][2]);

				n += normalizedcross(u,v); 
			}

			if (i>0 && j>0) {
				u = Triple(gdata.vertices[i-1][j][0] - gdata.vertices[i][j][0],
					gdata.vertices[i-1][j][1] - gdata.vertices[i][j][1],
					gdata.vertices[i-1][j][2] - gdata.vertices[i][j][2]);

				v = Triple(gdata.vertices[i][j-1][0] - gdata.vertices[i][j][0],
					gdata.vertices[i][j-1][1] - gdata.vertices[i][j][1],
					gdata.vertices[i][j-1][2] - gdata.vertices[i][j][2]);

				n += normalizedcross(u,v);
			}

			if (i<columns-1 && j>0) {
				u = Triple(gdata.vertices[i][j-1][0] - gdata.vertices[i][j][0],
					gdata.vertices[i][j-1][1] - gdata.vertices[i][j][1],
					gdata.vertices[i][j-1][2] - gdata.vertices[i][j][2]);

				v = Triple(gdata.vertices[i+1][j][0] - gdata.vertices[i][j][0],
					gdata.vertices[i+1][j][1] - gdata.vertices[i][j][1],
					gdata.vertices[i+1][j][2] - gdata.vertices[i][j][2]);

				n += normalizedcross(u,v);
			}

			n.normalize();
			gdata.normals[i][j][0] = n.x;
			gdata.normals[i][j][1] = n.y;
			gdata.normals[i][j][2] = n.z;
		} 
	} 
}

void Curve::sewPeriodic(GridData& gdata)
{
	// sewing
	Triple n;

	unsigned int columns = gdata.columns();
	unsigned int rows = gdata.rows();

	if (gdata.uperiodic()) {
		for (unsigned i = 0; i != columns; ++i) {
			n = Triple(gdata.normals[i][0][0] + gdata.normals[i][rows-1][0],
				gdata.normals[i][0][1] + gdata.normals[i][rows-1][1],
				gdata.normals[i][0][2] + gdata.normals[i][rows-1][2]);

			n.normalize();        
			gdata.normals[i][0][0] = gdata.normals[i][rows-1][0] = n.x;
			gdata.normals[i][0][1] = gdata.normals[i][rows-1][1] = n.y;
			gdata.normals[i][0][2] = gdata.normals[i][rows-1][2] = n.z;
		}
	}

	if (gdata.vperiodic()) {
		for (unsigned j = 0; j != rows; ++j) {
			n = Triple(gdata.normals[0][j][0] + gdata.normals[columns-1][j][0],
				gdata.normals[0][j][1] + gdata.normals[columns-1][j][1],
				gdata.normals[0][j][2] + gdata.normals[columns-1][j][2]);

			n.normalize();        
			gdata.normals[0][j][0] = gdata.normals[columns-1][j][0] = n.x;
			gdata.normals[0][j][1] = gdata.normals[columns-1][j][1] = n.y;
			gdata.normals[0][j][2] = gdata.normals[columns-1][j][2] = n.z;
		}
	}
}

/*!
	Convert user grid data to internal vertex structure.
	See also NativeReader::read() and Function::create()
*/
bool Curve::loadFromData(Triple** data, unsigned int columns, unsigned int rows, QString titlestr, bool uperiodic, bool vperiodic)
{
	actualDataC_->clear();
	actualData_p = actualDataG_;

	if (!titlestr.isEmpty())	setTitle(titlestr);
	readIn(*actualDataG_, data, columns, rows);
	calcNormals(*actualDataG_);
	actualDataG_->datatype = Qwt3D::GRID;
	actualDataG_->setPeriodic(uperiodic,vperiodic);
	sewPeriodic(*actualDataG_);

	updateData();
	return true;
}	

/*! 
	Convert user grid data to internal vertex structure.
	See also NativeReader::read() and Function::create()
*/
bool Curve::loadFromData(double** data, unsigned int columns, unsigned int rows, double minx, double maxx, double miny, double maxy, QString titlestr)
{
	actualDataC_->clear();
	actualData_p = actualDataG_;

	actualDataG_->setPeriodic(false, false);
	actualDataG_->setSize(columns, rows);

	if (!titlestr.isEmpty())	setTitle(titlestr);
	readIn(*actualDataG_, data, columns, rows, minx, maxx, miny, maxy);
	calcNormals(*actualDataG_);  
	
	updateData();
	return true;
}	


void Curve::createFloorDataG()
{
	switch (floorStyle()){
		case FLOORDATA:
			Data2FloorG();
			break;
		case FLOORISO:
			Isolines2FloorG(dataProjected());
			if (dataProjected())
				createPoints();
			break;
		default:
			break;
	}
}

void Curve::createSideDataG()
{
	switch (floorStyle()){
		case FLOORDATA:
			Data2SideG();
			break;
		case FLOORISO:
			Isolines2SideG(dataProjected());
			if (dataProjected()){
				DataPoints2SideG(dataProjected());
				createPoints();
			}
			break;
		default:
			break;
	}
}

void Curve::createFaceDataG()
{
	switch (floorStyle()){
		case FLOORDATA:
			Data2FrontG();
			break;
		case FLOORISO:
			Isolines2FrontG(dataProjected());
			if (dataProjected()){
				DataPoints2BackG(dataProjected());
				createPoints();
			}
			break;
		default:
			break;
	}
}

void Curve::DatamapG(unsigned int comp)
{
	if (actualDataG_->empty())
		return;

	int step = resolution();
	int cols = actualDataG_->columns();
	int rows = actualDataG_->rows();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_QUADS);
	
	Triple tmin = actualDataG_->hull().minVertex;
	double shift = tmin(comp);

	for (int i = 0; i < cols - step; i += step){
		glBegin(GL_TRIANGLE_STRIP);
			double *vi = actualDataG_->vertices[i][0];
			setColorFromVertexG(vi);
			drawVertex(plot_p->transform(vi, comp), shift, comp);

			int ni = i + step;
			double *vni = actualDataG_->vertices[ni][0];
			setColorFromVertexG(vni);
			drawVertex(plot_p->transform(vni, comp), shift, comp);

			for (int j = 0; j < rows - step; j += step){
				int nj = j + step;

				double *vnj = actualDataG_->vertices[i][nj];
				setColorFromVertexG(vnj);
				drawVertex(plot_p->transform(vnj, comp), shift, comp);

				double *vnij = actualDataG_->vertices[ni][nj];
				setColorFromVertexG(vnij);
				drawVertex(plot_p->transform(vnij, comp), shift, comp);
			}
		glEnd();
	}
}

void Curve::IsolinesG(unsigned int comp, bool projected)
{
	if (isolines() <= 0 || actualDataG_->empty())
		return;

	Triple tmax = actualDataG_->hull().maxVertex;
	Triple tmin = actualDataG_->hull().minVertex;
	
	double delta = tmax(comp) - tmin(comp);
	double shift = tmin(comp);
	double count = delta / isolines();

	RGBA col;

	int step = resolution();
	int cols = actualDataG_->columns();
	int rows = actualDataG_->rows();
	
	Triple t[4];
	vector<Triple> intersection;
	
	double lambda = 0;
	
	GLStateBewarer sb2(GL_LINE_SMOOTH, false);

	for (unsigned int k = 0; k != isolines(); ++k){
		double val = shift + k * count;

		for (int i = 0; i < cols - step; i += step){
			for (int j = 0; j < rows - step; j += step){
				double *v = actualDataG_->vertices[i][j];
				t[0] = plot_p->transform(v, comp);

				col = (*datacolor_p)(v[0], v[1], v[2]);
				glColor4d(col.r, col.g, col.b, col.a);

				int step_i = i + step, step_j = j + step;

				double *vi = actualDataG_->vertices[step_i][j];
				t[1] =  plot_p->transform(vi, comp);

				double *vij = actualDataG_->vertices[step_i][step_j];
				t[2] =  plot_p->transform(vij, comp);

				double *vj = actualDataG_->vertices[i][step_j];
				t[3] =  plot_p->transform(vj, comp);

				double diff = 0;

				for (int m = 0; m != 4; ++m){
					int mm = (m+1)%4;

					bool outer = (val >= t[mm](comp) && val <= t[m](comp));
					bool inner = (val >= t[m](comp) && val <= t[mm](comp));

					if (inner || outer){
						diff = t[mm](comp) - t[m](comp);

						if (isPracticallyZero(diff)){// degenerated
							intersection.push_back(t[m]);
							intersection.push_back(t[mm]);
							continue;
						}

						Triple intersect;
						double component[3];

						lambda = (val - t[m](comp)) / diff;
						for (unsigned int c = 0; c!=3; ++c)
							component[c] = (t[m](c) + lambda * (t[mm](c)-t[m](c)));

						switch (comp){
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
				drawIntersections(intersection, shift, comp, projected);
			}
		}
	}
}

void Curve::DataPointsG(unsigned int comp, bool projected)
{
	if (actualDataG_->empty() || actualDataG_->columns() <= 0)
		return;

	int cols = 0, rows = 0, step = resolution();

	switch (comp){
		case 0:		// iterate through each column
			cols = actualDataG_->columns();
			rows = actualDataG_->rows();
			break;
		case 1:		// iterate through each row
			cols = actualDataG_->rows();
			rows = actualDataG_->columns();
			break;
	}

	Triple tmax = actualDataG_->hull().maxVertex;
	double shift = tmax(comp);

	vector<RGBA>	col;
	vector<Triple>	t;

	col.resize(rows/step);
	t.resize(rows/step);

	vector<Triple>	projection;

	GLStateBewarer sb(GL_LINE_SMOOTH, false);
				
	for (int outer = 0; outer < cols-step; outer += step){
		for (int inner = 0; inner < rows-step; inner += step){
			int i = comp ? inner : outer;
			int j = comp ? outer : inner;

			double *v = actualDataG_->vertices[i][j];
			t[inner] =	Triple(v[0], v[1], v[2]);
			col[inner] = (*datacolor_p)(t[inner].x, t[inner].y, t[inner].z);

			projection.push_back(t[inner]);
		}

		drawIntersections(projection, shift, comp, projected, &col);
	}
}

double** Curve::getData(int *columns, int *rows)
{
	if (!actualDataG_)
		return 0;
	
	*columns = actualDataG_->columns();
	*rows	 = actualDataG_->rows();

	/* allocate some space for the mesh */
	double** data = new double* [*columns];

	for (int i = 0; i < *columns; ++i) {
		data[i] = new double [*rows];
		for (int j = 0; j != *rows; ++j) {
			data[i][j] = actualDataG_->vertices[i][j][2];
		}
	}
	return data;
}

void Curve::deleteData(double** data, int columns)
{
	for (int i = 0; i < columns; i++)
		delete [] data[i];

	delete [] data;
}

void Curve::animateData(double** data)
{
	if (!actualDataG_)
		return;

	int cols = actualDataG_->columns();
	int rows = actualDataG_->rows();
	for (int i = 0; i < cols; ++i){
		for (int j = 0; j < rows; ++j){
			actualDataG_->vertices[i][j][2] = data[i][j];
		}
	}
}
