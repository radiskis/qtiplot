#include <math.h>
#include "qwt3d_color.h"
#include "qwt3d_curve.h"
#include "qwt3d_enrichment_std.h"

using namespace Qwt3D;


/////////////////////////////////////////////////////////////////
//
//   CrossHair
//
/////////////////////////////////////////////////////////////////

CrossHair::CrossHair()
{
	configure(0, 1, false, false);
}

CrossHair::CrossHair(double rad, double linewidth, bool smooth, bool boxed)
{
	configure(rad, linewidth, smooth, boxed);
}

void CrossHair::configure(double rad, double linewidth, bool smooth, bool boxed)
{
	curve_ = 0;
	radius_ = rad;
	linewidth_ = linewidth;
	smooth_ = smooth;
	boxed_ = boxed;
}

void CrossHair::drawBegin()
{
	setDeviceLineWidth( linewidth_ );
	oldstate_ = glIsEnabled(GL_LINE_SMOOTH);
	if (smooth_)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);
	glBegin( GL_LINES );
}

void CrossHair::drawEnd()
{
	glEnd();

	if (oldstate_)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);
}

void CrossHair::draw(Qwt3D::Triple const& t)
{
	Qwt3D::Triple pos = curve_->plot()->transform(t);

	RGBA rgba = (*curve_->dataColor())(t);
	glColor4d(rgba.r, rgba.g, rgba.b, rgba.a);

	double diag = (curve_->hull().maxVertex - curve_->hull().minVertex).length() * radius_;

	glVertex3d( pos.x - diag, pos.y, pos.z);
	glVertex3d( pos.x + diag, pos.y, pos.z);

	glVertex3d( pos.x, pos.y - diag, pos.z);
	glVertex3d( pos.x, pos.y + diag, pos.z);

	glVertex3d( pos.x, pos.y, pos.z - diag);
	glVertex3d( pos.x, pos.y, pos.z + diag);

	// hull
	if (!boxed_)
		return;

	glVertex3d( pos.x - diag, pos.y - diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y - diag, pos.z + diag);
	glVertex3d( pos.x - diag, pos.y - diag, pos.z - diag);
	glVertex3d( pos.x + diag, pos.y - diag, pos.z - diag);

	glVertex3d( pos.x - diag, pos.y + diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z + diag);
	glVertex3d( pos.x - diag, pos.y + diag, pos.z - diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z - diag);

	glVertex3d( pos.x - diag, pos.y - diag, pos.z + diag);
	glVertex3d( pos.x - diag, pos.y + diag, pos.z + diag);
	glVertex3d( pos.x - diag, pos.y - diag, pos.z - diag);
	glVertex3d( pos.x - diag, pos.y + diag, pos.z - diag);

	glVertex3d( pos.x + diag, pos.y - diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y - diag, pos.z - diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z - diag);

	glVertex3d( pos.x - diag, pos.y - diag, pos.z - diag);
	glVertex3d( pos.x - diag, pos.y - diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y - diag, pos.z - diag);
	glVertex3d( pos.x + diag, pos.y - diag, pos.z + diag);

	glVertex3d( pos.x - diag, pos.y + diag, pos.z - diag);
	glVertex3d( pos.x - diag, pos.y + diag, pos.z + diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z - diag);
	glVertex3d( pos.x + diag, pos.y + diag, pos.z + diag);
}

/////////////////////////////////////////////////////////////////
//
//   Dot
//
/////////////////////////////////////////////////////////////////

Dot::Dot()
{
	configure(1, false);
}

Dot::Dot(double pointsize, bool smooth)
{
	configure(pointsize, smooth);
}

void Dot::configure(double pointsize, bool smooth)
{
	curve_ = 0;
	pointsize_ = pointsize;
	smooth_ = smooth;
}

void Dot::drawBegin()
{
	setDevicePointSize( pointsize_ );
	oldstate_ = glIsEnabled(GL_POINT_SMOOTH);
	if (smooth_)
		glEnable(GL_POINT_SMOOTH);
	else
		glDisable(GL_POINT_SMOOTH);

	//glPointSize(10);
	glBegin( GL_POINTS );
}

void Dot::drawEnd()
{
	glEnd();

	if (oldstate_)
		glEnable(GL_POINT_SMOOTH);
	else
		glDisable(GL_POINT_SMOOTH);
}

void Dot::draw(Qwt3D::Triple const& pos)
{
	RGBA rgba = (*curve_->dataColor())(pos);
	glColor4d(rgba.r, rgba.g, rgba.b, rgba.a);

	Qwt3D::Triple t = curve_->plot()->transform(pos);
	glVertex3d(t.x, t.y, t.z);
}


/////////////////////////////////////////////////////////////////
//
//   Cone
//
/////////////////////////////////////////////////////////////////

Cone::Cone()
{
	hat      = gluNewQuadric();
	disk     = gluNewQuadric();

	configure(0, 3);
}

Cone::Cone(double rad, unsigned quality)
{
	hat      = gluNewQuadric();
	disk     = gluNewQuadric();

	configure(rad, quality);
}

void Cone::configure(double rad, unsigned quality)
{
	curve_ = 0;
	radius_ = rad;
	quality_ = quality;
	oldstate_ = GL_FALSE;

	gluQuadricDrawStyle(hat,GLU_FILL);
	gluQuadricNormals(hat,GLU_SMOOTH);
	gluQuadricOrientation(hat,GLU_OUTSIDE);
	gluQuadricDrawStyle(disk,GLU_FILL);
	gluQuadricNormals(disk,GLU_SMOOTH);
	gluQuadricOrientation(disk,GLU_OUTSIDE);
}

void Cone::draw(Qwt3D::Triple const& pos)
{  
	RGBA rgba = (*curve_->dataColor())(pos);
	glColor4d(rgba.r,rgba.g,rgba.b,rgba.a);

	GLint mode;
	glGetIntegerv(GL_MATRIX_MODE, &mode);
	glMatrixMode(GL_MODELVIEW );
	glPushMatrix();

	Qwt3D::Triple t = curve_->plot()->transform(pos);
	glTranslatef(t.x, t.y, t.z);

	gluCylinder(hat, 0.0, radius_, radius_*2, quality_, 1);
	glTranslatef(0, 0, radius_*2);
	gluDisk(disk, 0.0, radius_, quality_, 1);

	glPopMatrix();
	glMatrixMode(mode);
}


/////////////////////////////////////////////////////////////////
//
//   Arrow
//
/////////////////////////////////////////////////////////////////

Arrow::Arrow()
{	
	hat     = gluNewQuadric();
	disk    = gluNewQuadric();
	base    = gluNewQuadric();
	bottom  = gluNewQuadric();

	gluQuadricDrawStyle(hat,GLU_FILL);
	gluQuadricNormals(hat,GLU_SMOOTH);
	gluQuadricOrientation(hat,GLU_OUTSIDE);
	gluQuadricDrawStyle(disk,GLU_FILL);
	gluQuadricNormals(disk,GLU_SMOOTH);
	gluQuadricOrientation(disk,GLU_OUTSIDE);
	gluQuadricDrawStyle(base,GLU_FILL);
	gluQuadricNormals(base,GLU_SMOOTH);
	gluQuadricOrientation(base,GLU_OUTSIDE);
	gluQuadricDrawStyle(bottom,GLU_FILL);
	gluQuadricNormals(bottom,GLU_SMOOTH);
	gluQuadricOrientation(bottom,GLU_OUTSIDE);

	configure(3, 0.4, 0.06, 0.02);
}

Arrow::~Arrow()
{
	gluDeleteQuadric(hat);
	gluDeleteQuadric(disk);
	gluDeleteQuadric(base);
	gluDeleteQuadric(bottom);
}

/**
\param segs number of faces for the fields arrows (see the gallery for examples)
\param relconelength see picture
\param relconerad see picture
\param relstemrad see picture
\image html arrowanatomy.png 
*/
void Arrow::configure(int segs, double relconelength, double relconerad, double relstemrad)
{
	curve_ = 0;
	segments_ = segs;
	oldstate_ = GL_FALSE;
	rel_cone_length = relconelength;
	rel_cone_radius = relconerad;
	rel_stem_radius = relstemrad;
}

void Arrow::draw(Qwt3D::Triple const& pos)
{	
	Triple end = top_;
	Triple beg = pos;
	Triple vdiff = end-beg;
	double length = vdiff.length();
	glColor4d(rgba_.r,rgba_.g,rgba_.b,rgba_.a);

	double radius[2];
	radius[0] = rel_cone_radius * length;
	radius[1] = rel_stem_radius * length;

	GLint mode;
	glGetIntegerv(GL_MATRIX_MODE, &mode);

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();


	Triple axis;
	double phi = calcRotation(axis, FreeVector(beg,end));

	glTranslatef(beg.x, beg.y, beg.z);
	glRotatef(phi, axis.x, axis.y, axis.z);

	double baseheight = (1-rel_cone_length) * length;

	glTranslatef(0, 0, baseheight);

	gluCylinder(hat, radius[0], 0.0, rel_cone_length * length, segments_,1);
	gluDisk(disk,radius[1],radius[0], segments_,1);

	glTranslatef(0, 0, -baseheight);

	gluCylinder(base, radius[1],radius[1], baseheight,segments_,1);
	gluDisk(disk,0,radius[1],segments_,1);

	glPopMatrix();
	glMatrixMode(mode);
}


//! transform a vector on the z axis with length |beg-end|, to get them in coincidence with the vector(beg,end)
/**
	\return	      Angle in degree to rotate
	\param axis   The axis to rotate around
	\param vec    result vector of points
*/ 
double Arrow::calcRotation(Triple& axis, FreeVector const& vec)
{
	Triple end = vec.top;			// param end    result vector top point
	Triple beg = vec.base;			// param beg    result vector base point

	Triple firstbeg(0.0,0.0,0.0);
	Triple firstend(0.0,0.0,(end-beg).length());
	
	Triple first = firstend - firstbeg;
	first.normalize();

	Triple second = end-beg;
	second.normalize();

	axis = normalizedcross(first,second);
	double cosphi = dotProduct(first,second);
	
	return 180 * acos(cosphi) / Qwt3D::PI;
}

/////////////////////////////////////////////////////////////////
//
//   Bar
//
/////////////////////////////////////////////////////////////////

Bar::Bar()
{
	configure(0);
}

Bar::Bar(double rad, bool lines, bool filled, bool smooth)
{
	configure(rad, lines, filled, smooth);
}

void Bar::configure(double rad, bool lines, bool filled, bool smooth)
{
	curve_ = 0;
	radius_ = rad;
	d_smooth = smooth;
	d_draw_lines = lines;
	d_filled_bars = filled;
}

void Bar::drawBegin()
{
	diag_ = (curve_->hull().maxVertex - curve_->hull().minVertex).length() * radius_;
	glLineWidth(curve_->meshLineWidth());
	glEnable(GL_POLYGON_OFFSET_FILL);
	if (d_smooth)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);

	glPolygonOffset(1, 1);
}

void Bar::draw(Qwt3D::Triple const& t)
{
	Qwt3D::Triple pos = curve_->plot()->transform(t);
	GLdouble minz = curve_->hull().minVertex.z;

	double xl = pos.x - diag_;
	double xr = pos.x + diag_;
	double yl = pos.y - diag_;
	double yr = pos.y + diag_;

	if (d_filled_bars){
		RGBA rgbat = (*curve_->dataColor())(t);
		glColor4d(rgbat.r, rgbat.g, rgbat.b, rgbat.a);

		glBegin(GL_QUADS);
		glVertex3d(xl,yl,minz);
		glVertex3d(xr,yl,minz);
		glVertex3d(xr,yr,minz);
		glVertex3d(xl,yr,minz);

		glVertex3d(xl,yl,pos.z);
		glVertex3d(xr,yl,pos.z);
		glVertex3d(xr,yr,pos.z);
		glVertex3d(xl,yr,pos.z);

		glVertex3d(xl,yl,minz);
		glVertex3d(xr,yl,minz);
		glVertex3d(xr,yl,pos.z);
		glVertex3d(xl,yl,pos.z);

		glVertex3d(xl,yr,minz);
		glVertex3d(xr,yr,minz);
		glVertex3d(xr,yr,pos.z);
		glVertex3d(xl,yr,pos.z);

		glVertex3d(xl,yl,minz);
		glVertex3d(xl,yr,minz);
		glVertex3d(xl,yr,pos.z);
		glVertex3d(xl,yl,pos.z);

		glVertex3d(xr,yl,minz);
		glVertex3d(xr,yr,minz);
		glVertex3d(xr,yr,pos.z);
		glVertex3d(xr,yl,pos.z);
		glEnd();
	}

	if (!d_draw_lines)
		return;

	Qwt3D::RGBA meshCol = curve_->meshColor();//using mesh color to draw the lines
	glColor3d(meshCol.r, meshCol.g, meshCol.b);

	glBegin(GL_LINES);
		glVertex3d(xl,yl,minz); glVertex3d(xr,yl,minz);
		glVertex3d(xl,yl,pos.z); glVertex3d(xr,yl,pos.z);
		glVertex3d(xl,yr,pos.z); glVertex3d(xr,yr,pos.z);
		glVertex3d(xl,yr,minz); glVertex3d(xr,yr,minz);

		glVertex3d(xl,yl,minz); glVertex3d(xl,yr,minz);
		glVertex3d(xr,yl,minz); glVertex3d(xr,yr,minz);
		glVertex3d(xr,yl,pos.z); glVertex3d(xr,yr,pos.z);
		glVertex3d(xl,yl,pos.z); glVertex3d(xl,yr,pos.z);

		glVertex3d(xl,yl,minz); glVertex3d(xl,yl,pos.z);
		glVertex3d(xr,yl,minz); glVertex3d(xr,yl,pos.z);
		glVertex3d(xr,yr,minz); glVertex3d(xr,yr,pos.z);
		glVertex3d(xl,yr,minz); glVertex3d(xl,yr,pos.z);
	glEnd();
}
