#include "qwt3d_surfaceplot.h"

using namespace std;
using namespace Qwt3D;

/**
Initializes with dataNormals()==false, NOFLOOR, resolution() == 1
*/
#if QT_VERSION < 0x040000
SurfacePlot::SurfacePlot( QWidget* parent, const char* name )
    : Plot3D( parent, name )
#else
SurfacePlot::SurfacePlot( QWidget * parent, const QGLWidget * shareWidget)
    : Plot3D( parent, shareWidget) 
#endif
{
	datanormals_p = false;
	normalLength_p = 0.02;
	normalQuality_p = 3;

	resolution_p = 1;
	actualDataG_ = new GridData();
	actualDataC_ = new CellData();

	actualData_p = actualDataG_;

	floorstyle_ = NOFLOOR;
	point_size_ = 5;
	datapoints_	= false;
	facemode_	= false;
	sidemode_	= false;
	floormode_	= true;
}

SurfacePlot::~SurfacePlot()
{
	delete actualDataG_;
	delete actualDataC_;
}

void SurfacePlot::showNormals(bool b)
{
  datanormals_p = b;
}

/**
Values < 0 or > 1 are ignored
*/
void SurfacePlot::setNormalLength(double val)
{
	if (val<0 || val>1)
		return;
	normalLength_p = val;
}

/**
Values < 3 are ignored 
*/
void SurfacePlot::setNormalQuality(int val) 
{
	if (val<3)
		return;
	normalQuality_p = val;
}

/**
	Calculates the smallest x-y-z parallelepiped enclosing the data.
	It can be accessed by hull();
*/
void SurfacePlot::calculateHull()
{
	if (actualData_p->empty())
		return;
	setHull(actualData_p->hull());
}

/*!
  Sets data resolution (res == 1 original resolution) and updates widget
	If res < 1, the function does nothing
*/
void SurfacePlot::setResolution( int res )
{
  if (!actualData_p || actualData_p->datatype == Qwt3D::POLYGON)
    return;
  
  if ((resolution_p == res) || res < 1)
		return;
	
	resolution_p = res;
	updateNormals();
	updateData();
	if (initializedGL())
    updateGL();

	emit resolutionChanged(res);
}

void SurfacePlot::updateNormals()
{
	SaveGlDeleteLists(displaylists_p[NormalObject], 1); 
	
	if (plotStyle() == NOPLOT && !normals() || !actualData_p)
		return;

	displaylists_p[NormalObject] = glGenLists(1);
	glNewList(displaylists_p[NormalObject], GL_COMPILE);

  if (actualData_p->datatype == Qwt3D::POLYGON)
    createNormalsC();
  else if (actualData_p->datatype == Qwt3D::GRID)
    createNormalsG();
		
	glEndList();
}

void SurfacePlot::createData()
{
	if (!actualData_p)
		return;
  if (actualData_p->datatype == Qwt3D::POLYGON)
    createDataC();
  else if (actualData_p->datatype == Qwt3D::GRID)
    createDataG();
}


void SurfacePlot::createFloorData()
{
	if (!actualData_p)
		return;
	if (actualData_p->datatype == Qwt3D::POLYGON)
		createFloorDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createFloorDataG();  
}

void SurfacePlot::createSideData()
{
	if (!actualData_p)
		return;
	if (actualData_p->datatype == Qwt3D::POLYGON)
		createSideDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createSideDataG();  
}

void SurfacePlot::createFaceData()
{
	if (!actualData_p)
		return;
	if (actualData_p->datatype == Qwt3D::POLYGON)
		createFaceDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createFaceDataG();  
}

/**
	The returned value is not affected by resolution(). The pair gives (columns,rows) for grid data
, (number of cells,1) for free formed data (datatype() == POLYGON) and (0,0) else
*/
pair<int,int> SurfacePlot::facets() const
{
	if (!hasData())
		return pair<int,int>(0,0);

  if (actualData_p->datatype == Qwt3D::POLYGON)
  	return pair<int,int>(int(actualDataC_->cells.size()), 1);
  else if (actualData_p->datatype == Qwt3D::GRID)
  	return pair<int,int>(actualDataG_->columns(), actualDataG_->rows()); 	
  else
    return pair<int,int>(0,0);    
}

void SurfacePlot::createPoints()
{
  Dot pt(point_size_,true);
  createEnrichment(pt);
}

void SurfacePlot::createEnrichment(Enrichment& p)
{
	if (!actualData_p)
    return;
  
  //todo future work
  if (p.type() != Enrichment::VERTEXENRICHMENT)
    return;
  
  p.assign(*this);
	p.drawBegin();

  VertexEnrichment* ve = (VertexEnrichment*)&p; 
  if (actualData_p->datatype == Qwt3D::POLYGON)
  {	
    for (unsigned i = 0; i != actualDataC_->normals.size(); ++i) 
	    ve->draw(actualDataC_->nodes[i]);
  }
  else if (actualData_p->datatype == Qwt3D::GRID)
 	{
    int step = resolution();
    for (int i = 0; i <= actualDataG_->columns() - step; i += step) 
      for (int j = 0; j <= actualDataG_->rows() - step; j += step) 
  			ve->draw(Triple(actualDataG_->vertices[i][j][0],
										              actualDataG_->vertices[i][j][1],
                                  actualDataG_->vertices[i][j][2]));
  }
  p.drawEnd(); 
}

void SurfacePlot::drawVertex(Triple& vertex, double shift, unsigned int comp)
{
	switch (comp) {
	case 0:
		glVertex3d(shift, vertex.y, vertex.z);		break;
	case 1:
		glVertex3d(vertex.x, shift, vertex.z);		break;
	case 2:
		glVertex3d(vertex.x, vertex.y, shift);		break;
	default:
		glVertex3d(vertex.x, vertex.y, vertex.z);		
	}
}

void SurfacePlot::drawIntersections(vector<Triple>& intersection, double shift, unsigned int comp,
									bool projected, vector<RGBA>* col)
{
	if (intersection.empty())
		return;

	if (intersection.size() > 2) {
		glBegin(GL_LINE_STRIP);
			for (unsigned dd = 0; dd!=intersection.size(); ++dd) {
				if (colour)	glColor4d((*colour)[dd].r, (*colour)[dd].g, (*colour)[dd].b, (*colour)[dd].a);
				drawVertex(intersection[dd], shift, comp);
			}
		glEnd();

		if (projected) {
			glBegin(GL_POINTS);
				drawVertex(intersection[0], shift, comp);
			glEnd();
		}
	} else if (intersection.size() == 2) {
		glBegin(GL_LINES);
			drawVertex(intersection[0], shift, comp);
			drawVertex(intersection[1], shift, comp);

			// small pixel gap problem (see OpenGL spec.)
			drawVertex(intersection[1], shift, comp);
			drawVertex(intersection[0], shift, comp);
		glEnd();

		if (projected) {
			glBegin(GL_POINTS);
				drawVertex(intersection[0], shift, comp);
				drawVertex(intersection[1], shift, comp);
			glEnd();
		}
	}

	intersection.clear();
}
