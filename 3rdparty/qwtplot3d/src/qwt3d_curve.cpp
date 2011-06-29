#include "qwt3d_curve.h"

using namespace std;
using namespace Qwt3D;

/**
Initializes with dataNormals()==false, NOFLOOR, resolution() == 1
*/
Curve::Curve(QWidget* parent)
:	plot_p(qobject_cast<Plot3D*> (parent))
{
	if (plot_p) {
		title_p = new Label();
		plot_p->setCurve(this);
		plot_p->addTitle(title_p);
		connects();
	}

    plotstyle_		= FILLEDMESH;
    userplotstyle_p = 0;
    shading_		= GOURAUD;
    floorstyle_		= NOFLOOR;
    floormode_		= true;
    isolines_		= 10;
    point_size_		= 5;

    datanormals_p	= false;
    normalLength_p	= 0.02;
    normalQuality_p = 3;

    resolution_p = 1;
    actualDataG_ = new GridData();
    actualDataC_ = new CellData();
    actualData_p = actualDataG_;

    displaylists_p.resize(DisplayListSize);
    meshLineWidth_ = 1;
    datacolor_p = new StandardColor(this, 100);
    setPolygonOffset(0.5);
    setMeshColor(RGBA(0.0,0.0,0.0));
    setMeshLineWidth(1);

	title_p->setFont("Courier", 16, QFont::Bold);
	title_p->setString("");

	titlespace_ = 0.02;
	titlepos_	= plot_p->titleList().size() - 1;
	setTitlePosition(0.98 - (titlepos_ * titlespace_));

    update_displaylists_ = false;
	displaylegend_		 = false;

	legend_.setPlot(plot_p);
	legend_.setLimits(0, 100);
	legend_.setMajors(10);
	legend_.setMinors(2);
	legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
}

Curve::~Curve()
{
	delete actualDataG_;
	delete actualDataC_;

	glDeleteLists(displaylists_p[DataObject], 1);

	for (ELIT it = elist_p.begin(); it!=elist_p.end(); ++it)
		delete (*it);

	elist_p.clear();

	if (plot_p) {
		plot_p->removeCurve(this);
		plot_p->removeTitle(title_p);
	}
	disconnects();
	
	delete title_p;
}

void Curve::connects()
{
	// Parent Plot3D Relayed Signals
	connect(plot_p, SIGNAL(setCurveResolution(int)),					SLOT(setResolution(int)));
	connect(plot_p, SIGNAL(setCurvePolygonOffset(double)),				SLOT(setPolygonOffset(double)));
	connect(plot_p, SIGNAL(setCurveIsolines(unsigned int)),				SLOT(setIsolines(unsigned int)));

	connect(plot_p, SIGNAL(setCurvePlotStyle(Qwt3D::PLOTSTYLE)),		SLOT(setPlotStyle(Qwt3D::PLOTSTYLE)));
	connect(plot_p, SIGNAL(setCurvePlotStyle(Qwt3D::Enrichment&)),		SLOT(setPlotStyle(Qwt3D::Enrichment&)));
	connect(plot_p, SIGNAL(setCurveFloorStyle(Qwt3D::FLOORSTYLE)),		SLOT(setFloorStyle(Qwt3D::FLOORSTYLE)));
	connect(plot_p, SIGNAL(setCurveShading(Qwt3D::SHADINGSTYLE)),		SLOT(setShading(Qwt3D::SHADINGSTYLE)));

	connect(plot_p, SIGNAL(showCurveColorLegend(bool)),					SLOT(showColorLegend(bool)));
	connect(plot_p, SIGNAL(showCurveNormals(bool)),						SLOT(showNormals(bool)));
	connect(plot_p, SIGNAL(setCurveNormalLength(double)),				SLOT(setNormalLength(double)));
	connect(plot_p, SIGNAL(setCurveNormalQuality(int)),					SLOT(setNormalQuality(int)));

	connect(plot_p, SIGNAL(setCurveSmoothMesh(bool)),					SLOT(setSmoothMesh(bool)));
	connect(plot_p, SIGNAL(setCurveMeshColor(Qwt3D::RGBA)),				SLOT(setMeshColor(Qwt3D::RGBA)));
	connect(plot_p, SIGNAL(setCurveMeshLineWidth(double)),				SLOT(setMeshLineWidth(double)));

	connect(plot_p, SIGNAL(setCurveDataColor(Color*)),					SLOT(setDataColor(Color*)));
	connect(plot_p, SIGNAL(setCurveDataProjection(bool)),				SLOT(setDataProjection(bool)));
	connect(plot_p, SIGNAL(setCurveProjection(Qwt3D::PROJECTMODE, bool)),	SLOT(setProjection(Qwt3D::PROJECTMODE, bool)));

	connect(plot_p, SIGNAL(setCurveTitleColor(Qwt3D::RGBA)),			SLOT(setTitleColor(Qwt3D::RGBA)));
	connect(plot_p, SIGNAL(setCurveTitle(const QString&)),				SLOT(setTitle(const QString&)));
	connect(plot_p, SIGNAL(setCurveTitlePosition(double,double,Qwt3D::ANCHOR)),	SLOT(setTitlePosition(double,double,Qwt3D::ANCHOR)));
	connect(plot_p, SIGNAL(setCurveTitleFont(const QString&,int,int,bool)),	SLOT(setTitleFont(const QString&,int,int,bool)));

	connect(plot_p, SIGNAL(createData()),								SLOT(createData()));
	connect(plot_p, SIGNAL(createEnrichments()),						SLOT(createEnrichments()));

	// Reverse relayed signals
	connect(this, SIGNAL(updatePlotData(bool)),							plot_p,	SLOT(updateData(bool)));
	connect(this, SIGNAL(updatePlot()),									plot_p,	SLOT(update()));
}

void Curve::disconnects()
{
	disconnect(plot_p, 0, this, 0);
	disconnect(this, 0, plot_p, 0);
}

void Curve::draw()
{
    // if the display list needs to be updated
	if ( update_displaylists_ ) {
        // create display list if necessary
        if ( 0 == displaylists_p[DataObject] ) {
            displaylists_p[DataObject] = glGenLists(1);
        }

        // glGenLists returns 0 on error. So, if displaylists_p[DataObject] is still 0,
        // then just call drawImplementation()
        if ( 0 == displaylists_p[DataObject] ) {
            drawImplementation();
            return;
        }

        glNewList(displaylists_p[DataObject], GL_COMPILE);
        drawImplementation();
        glEndList();

        updateNormals();
	}

    // glGenLists returns 0 on error. So, if displaylists_p[DataObject] is still 0,
    // then just call drawImplementation()
    if ( 0 == displaylists_p[DataObject] ) {
        drawImplementation();
        return;
    }

    for ( unsigned int i=0; i < DisplayListSize; ++i ) {
        glCallList(displaylists_p[i]);
    }
}

void Curve::drawImplementation()
{
    createEnrichments();
    createData();
}

/*!
	Calculates the smallest x-y-z parallelepiped enclosing the data.
	It can be accessed by hull();
*/
void Curve::calculateHull()
{
	if (actualData_p->empty())	return;

	setHull(actualData_p->hull());
}

void Curve::updateData(bool coord)
{
	update_displaylists_ = true;

	if ( plot_p ) {
		calculateHull();
		emit updatePlotData(coord);
	}
}

void Curve::queueUpdate()
{
	update_displaylists_ = true;
	if ( plot_p ) {
		emit updatePlot();
	}
}

void Curve::updateNormals()
{
	SaveGlDeleteLists(displaylists_p[NormalObject], 1); 

	if (plotStyle() == NOPLOT && !normals() || !actualData_p)	return;

	displaylists_p[NormalObject] = glGenLists(1);
	glNewList(displaylists_p[NormalObject], GL_COMPILE);

	if (actualData_p->datatype == Qwt3D::POLYGON)
		createNormalsC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createNormalsG();

	glEndList();
}

void Curve::createData()
{
	if (!actualData_p)	return;

	switch (shading_) {
		case FLAT:
			glShadeModel(GL_FLAT);
			break;
		case GOURAUD:
			glShadeModel(GL_SMOOTH);
			break;
		default:
			break;
	}
	
	if (actualData_p->datatype == Qwt3D::POLYGON)
		createDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createDataG();
}

void Curve::createFloorData()
{
	if (!actualData_p)	return;

	if (actualData_p->datatype == Qwt3D::POLYGON)
		createFloorDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createFloorDataG();  
}

void Curve::createSideData()
{
	if (!actualData_p)	return;

	if (actualData_p->datatype == Qwt3D::POLYGON)
		createSideDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createSideDataG();  
}

void Curve::createFaceData()
{
	if (!actualData_p)	return;

	if (actualData_p->datatype == Qwt3D::POLYGON)
		createFaceDataC();
	else if (actualData_p->datatype == Qwt3D::GRID)
		createFaceDataG();  
}

/*!
	The returned value is not affected by resolution(). The pair gives (columns,rows) for grid data,
	(number of cells,1) for free formed data (datatype() == POLYGON) and (0,0) else
*/
pair<int,int> Curve::facets() const
{
	if (!hasData()) {
		qDebug() << "Curve: has no data, no facets found!" << this;
		if (actualData_p)	qDebug() << "Curve: Grid type -" << (actualData_p->datatype == Qwt3D::GRID);
		if (actualData_p->datatype == Qwt3D::POLYGON)
			qDebug() << "Curve: Polygon data -" << actualDataC_->cells.size();
		else if (actualData_p->datatype == Qwt3D::GRID)
			qDebug() << "Curve: Grid data -" << actualDataG_->columns() << actualDataG_->rows();
		return pair<int,int>(0,0);
	}

	if (actualData_p->datatype == Qwt3D::POLYGON)
		return pair<int,int>(int(actualDataC_->cells.size()), 1);
	else if (actualData_p->datatype == Qwt3D::GRID)
		return pair<int,int>(actualDataG_->columns(), actualDataG_->rows());     
	else
		return pair<int,int>(0,0);    
}

void Curve::createPoints()
{
//	Dot pt;
//	Cone d(15,32);
//	CrossHair d(0.003,0,true,false);

	Dot pt(5,true);
	createEnrichment(pt);
}

Enrichment* Curve::addEnrichment(Enrichment const& e)
{
	ELIT it = std::find( elist_p.begin(), elist_p.end(), &e );
	if ( elist_p.end() == it ) {
		elist_p.push_back(e.clone());
		update_displaylists_ = true;
	} else	return *it;
	
	return elist_p.back();
}

bool Curve::degrade(Enrichment* e)
{
	ELIT it = std::find(elist_p.begin(), elist_p.end(), e);

	if ( it != elist_p.end() ) {
		delete (*it);
		elist_p.erase(it);
		update_displaylists_ = true;
		return true;
	}
	return false;
}

void Curve::clearEnrichments()
{
    for (ELIT it = elist_p.begin(); it != elist_p.end(); ++it)
        this->degrade(*it);
}

void Curve::createEnrichments()
{
	for (ELIT it = elist_p.begin(); it!=elist_p.end(); ++it)
		this->createEnrichment(**it);
}

void Curve::createEnrichment(Enrichment& p)
{
	if (!actualData_p)	return;
	  
	//todo future work
	if (p.type() != Enrichment::VERTEXENRICHMENT)	return;

	p.assign(*this);
	p.drawBegin();

	VertexEnrichment* ve = (VertexEnrichment*)&p; 
	if (actualData_p->datatype == Qwt3D::POLYGON) {
		for (unsigned i = 0; i != actualDataC_->normals.size(); ++i) 
			ve->draw(actualDataC_->nodes[i]);
	} else if (actualData_p->datatype == Qwt3D::GRID) {
		int step = resolution();
		for (int i = 0; i <= actualDataG_->columns() - step; i += step) 
			for (int j = 0; j <= actualDataG_->rows() - step; j += step) 
				ve->draw(Triple(actualDataG_->vertices[i][j][0],
								actualDataG_->vertices[i][j][1],
								actualDataG_->vertices[i][j][2]));
	}

	p.drawEnd(); 
}

void Curve::drawVertex(Triple& vertex, double shift, unsigned int comp)
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

void Curve::drawIntersections(vector<Triple>& intersection, double shift, unsigned int comp,
							  bool projected, vector<RGBA>* colour)
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

/*!
Set relative caption position (0.5,0.5) means, the anchor point lies in the center of the screen
*/
void Curve::setTitlePosition(double rely, double relx, Qwt3D::ANCHOR anchor)
{
	titlerel_.y = (rely < 0 || rely > 1) ? 0.5 : rely;
	titlerel_.x = (relx < 0 || relx > 1) ? 0.5 : relx;

	titleanchor_ = anchor;
}

/*!
Set caption font
*/
void Curve::setTitleFont(const QString& family, int pointSize, int weight, bool italic)
{ 
	title_p->setFont(family, pointSize, weight, italic);
		
	titlespace_ = ((pointSize/8.0 + weight/50.0)*0.01);	// factor in pointsize & weight to title spacing		
	setTitlePosition(0.98 - (titlepos_ * titlespace_));
}

/*!
	Sets data resolution (res == 1 original resolution) and updates widget
	If res < 1, the function does nothing
*/
void Curve::setResolution(int res)
{
	if (!actualData_p || actualData_p->datatype == Qwt3D::POLYGON)
	    return;

	if ((resolution_p == res) || res < 1)
	    return;
    
	resolution_p = res;
	calculateHull();
	update_displaylists_ = true;

	emit resolutionChanged(res);
}

void Curve::setProjection(Qwt3D::PROJECTMODE val, bool toggle)
{
	switch(val) {
		case Qwt3D::FACE:
			facemode_  = toggle; break;
		case Qwt3D::SIDE:
			sidemode_  = toggle; break;
		case Qwt3D::BASE:
			floormode_ = toggle; break;
	}
}

void Curve::setMeshColor(RGBA rgba)
{
	meshcolor_ = rgba;
	update_displaylists_ = true;
}

/*!
    Assign a new coloring object for the data.
*/
void Curve::setDataColor(Color* col)
{
	Q_ASSERT(datacolor_p);
	
	if (col == datacolor_p)		return;
	
	datacolor_p->destroy();
	datacolor_p = col;
	update_displaylists_ = true;
}

void Curve::showNormals(bool b)
{
	datanormals_p = b;
	update_displaylists_ = true;
}

/*!
	Values < 0 or > 1 are ignored
*/
void Curve::setNormalLength(double val)
{
	if (val < 0 || val > 1)		return;

	normalLength_p = val;
	update_displaylists_ = true;
}

/*!
	Values < 3 are ignored 
*/
void Curve::setNormalQuality(int val) 
{
	if (val < 3)				return;
	
	normalQuality_p = val;
	update_displaylists_ = true;
}

/*!
	Set plotstyle for the standard plotting types. An argument of value Qwt3D::USER is ignored.
*/
void Curve::setPlotStyle( PLOTSTYLE val )
{
	if (val == Qwt3D::USER)		return;
  
	if ( val == plotstyle_ )		return;
   
	delete userplotstyle_p;
	userplotstyle_p = 0;
	plotstyle_ = val;
	update_displaylists_ = true;
}

/*!
	Set plotstyle to Qwt3D::USER and an associated enrichment object.
*/
void Curve::setPlotStyle( Qwt3D::Enrichment& obj )
{
	if (&obj == userplotstyle_p)	return;

	userplotstyle_p = obj.clone();
	plotstyle_ = Qwt3D::USER;
	update_displaylists_ = true;
}

/*!
	Set shading style
*/
void Curve::setShading( SHADINGSTYLE val )
{
	if (val == shading_)	return;

	shading_ = val;
	update_displaylists_ = true;
}

/*!
	Set number of isolines. The lines are equidistant between minimal and maximal Z value
*/
void Curve::setIsolines(unsigned int steps)
{
	if (steps == isolines_)			return;

	isolines_ = steps;
	update_displaylists_ = true;
}

/*!
	Set Polygon offset. The function affects the OpenGL rendering process. 
	Try different values for surfaces with polygons only and with mesh and polygons
*/
void Curve::setPolygonOffset( double val )
{
	if ( val == polygonOffset_ )	return;
	
	polygonOffset_ = val;
	update_displaylists_ = true;
}

void Curve::setMeshLineWidth( double val )
{
	Q_ASSERT(val >= 0);

	if (val < 0 || val == meshLineWidth_ )	return;

	meshLineWidth_ = val;
	update_displaylists_ = true;
}

void Curve::showColorLegend(bool show)
{
	displaylegend_ = show;

	if (show) {
		Color* color = const_cast<Color*>(dataColor());
		if (color)		color->createVector(legend_.colors);
	}
	updateData(false);
}

void Curve::updateColorLegend(int majors, int minors)
{
	if (legend_.axis()->majors() != majors)	legend_.setMajors(majors);
	if (legend_.axis()->minors() != minors)	legend_.setMinors(minors);

	ParallelEpiped hull = data()->hull();
	pair<double, double> limits(hull.minVertex.z, hull.maxVertex.z);

	legend_.setLimits(limits.first, limits.second);
}

void Curve::setColorLegend(int index, bool doublemode, QSize size, QPoint pos)
{
	double w = size.width()/100.0,	h = size.height()/100.0;	// legend color vector as screen size percentage
	double x = pos.x()/100.0,		y = pos.y()/100.0;			// legend anchor as screen position percentage

	switch(index) {
	case 0:
		legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
		legend_.setRelPosition(Tuple(1-x-w, 0.5+y),		Tuple(1-x, 0.5+y+h));
		break;
	case 1:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(1-x, 0.5+y),		Tuple(1-x+w, 0.5+y+h));
		} else {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
			legend_.setRelPosition(Tuple(1-x-w, 0.5-y-h),	Tuple(1-x, 0.5-y));
		}
		break;
	case 2:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
			legend_.setRelPosition(Tuple(1-x-w, 0.5-y-h),	Tuple(1-x, 0.5-y));
		} else {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5+y, x),			Tuple(0.5+y+h, x+w));
		}
		break;
	case 3:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(1-x, 0.5-y-h),		Tuple(1-x+w, 0.5-y));
		} else {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5-y-h, x),		Tuple(0.5-y, x+w));
		}
		break;
	case 4:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5+y, x),			Tuple(0.5+y+h, x+w));
		} else {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(x, 0.5-y-h),		Tuple(x+w, 0.5-y));
		}
		break;
	case 5:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5+y, x-w),		Tuple(0.5+y+h, x));
		} else {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(x, 0.5+y),			Tuple(x+w, 0.5+y+h));
		}
		break;
	case 6:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5-y-h, x),		Tuple(0.5-y, x+w));
		} else {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5-y-h, 1-x-w),	Tuple(0.5-y, 1-x));
		}
		break;
	case 7:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5-y-h, x-w),		Tuple(0.5-y, x));
		} else {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5+y, 1-x-w),		Tuple(0.5+y+h, 1-x));
		}
		break;
	case 8:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(x, 0.5-y-h),		Tuple(x+w, 0.5-y));
		}
		break;
	case 9:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
			legend_.setRelPosition(Tuple(x-w, 0.5-y-h),		Tuple(x, 0.5-y));
		}
		break;
	case 10:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Right);
			legend_.setRelPosition(Tuple(x, 0.5+y),			Tuple(x+w, 0.5+y+h));
		}
		break;
	case 11:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);
			legend_.setRelPosition(Tuple(x-w, 0.5+y),		Tuple(x, 0.5+y+h));
		}
		break;
	case 12:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5-y-h, 1-x-w),	Tuple(0.5-y, 1-x));
		}
		break;
	case 13:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5-y-h, 1-x),		Tuple(0.5-y, 1-x+w));
		}
		break;
	case 14:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Bottom);
			legend_.setRelPosition(Tuple(0.5+y, 1-x-w),		Tuple(0.5+y+h, 1-x));
		}
		break;
	case 15:
		if (doublemode) {
			legend_.setOrientation(ColorLegend::LeftRight, ColorLegend::Top);
			legend_.setRelPosition(Tuple(0.5+y, 1-x),		Tuple(0.5+y+h, 1-x+w));
		}
		break;
	default:
		break;
	}
}
