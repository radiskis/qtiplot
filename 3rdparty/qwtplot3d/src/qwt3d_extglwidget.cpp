#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning ( disable : 4305 )
#pragma warning ( disable : 4786 )
#endif

#include <float.h>
#include "qwt3d_extglwidget.h"

using namespace std;
using namespace Qwt3D;

#ifndef WHEEL_DELTA
	#define WHEEL_DELTA 120
#endif


/*!
  This should be the first call in your derived classes constructors.  
*/
ExtGLWidget::ExtGLWidget( QWidget * parent, const QGLWidget * shareWidget)
:	QGLWidget(parent, shareWidget) 
{
	initializedGL_ = false;	
	xRot_ = yRot_ = zRot_ = 0.0;		// default object rotation

	xShift_ = yShift_ = zShift_ = xVPShift_ = yVPShift_ = 0.0;
	xScale_ = yScale_ = zScale_ = 1.0;
	zoom_ = 1;
	ortho_ = true;
	lastMouseMovePosition_ = QPoint(0,0);
	mpressed_ = false;
	mouse_input_enabled_ = true;

	kbd_input_enabled_ = true;

	setFocusPolicy(Qt::StrongFocus);
	assignMouse(Qt::LeftButton, 
				MouseState(Qt::LeftButton, Qt::ShiftModifier), Qt::LeftButton, 
				MouseState(Qt::LeftButton, Qt::AltModifier), 
				MouseState(Qt::LeftButton, Qt::AltModifier), 
				MouseState(Qt::LeftButton, Qt::AltModifier | Qt::ShiftModifier),
				MouseState(Qt::LeftButton, Qt::AltModifier | Qt::ControlModifier),
				MouseState(Qt::LeftButton, Qt::ControlModifier), 
				MouseState(Qt::LeftButton, Qt::ControlModifier));


	assignKeyboard(Qt::Key_Down, Qt::Key_Up,
				KeyboardState(Qt::Key_Right, Qt::ShiftModifier),
				KeyboardState(Qt::Key_Left, Qt::ShiftModifier), Qt::Key_Right, Qt::Key_Left,
				KeyboardState(Qt::Key_Right, Qt::AltModifier),
				KeyboardState(Qt::Key_Left, Qt::AltModifier),
				KeyboardState(Qt::Key_Down, Qt::AltModifier),
				KeyboardState(Qt::Key_Up, Qt::AltModifier),
				KeyboardState(Qt::Key_Down, Qt::AltModifier|Qt::ShiftModifier),
				KeyboardState(Qt::Key_Up, Qt::AltModifier|Qt::ShiftModifier),
				KeyboardState(Qt::Key_Down, Qt::AltModifier|Qt::ControlModifier),
				KeyboardState(Qt::Key_Up, Qt::AltModifier|Qt::ControlModifier),
				KeyboardState(Qt::Key_Right, Qt::ControlModifier),
				KeyboardState(Qt::Key_Left, Qt::ControlModifier),
				KeyboardState(Qt::Key_Down, Qt::ControlModifier),
				KeyboardState(Qt::Key_Up, Qt::ControlModifier));

	setKeySpeed(3,5,5);

	lighting_enabled_ = false;
	disableLighting();
	lights_ = std::vector<Light>(8);
}

/*!
  Set up ortogonal or perspective mode and updates widget
*/
void ExtGLWidget::setOrtho( bool val )
{
	if (val == ortho_)		return;
	ortho_ = val;
	updateGL();
	
	emit projectionChanged(val);
}

/*!
  Set up the OpenGL rendering state
*/
void ExtGLWidget::initializeGL()
{
	glEnable( GL_BLEND );
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	// Set up the lights
	disableLighting();

	GLfloat whiteAmb[4] = {1.0, 1.0, 1.0, 1.0};

	setLightShift(0, 0, 3000);
	glEnable(GL_COLOR_MATERIAL);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, whiteAmb);

	setMaterialComponent(GL_DIFFUSE, 1.0);
	setMaterialComponent(GL_SPECULAR, 0.3);
	setMaterialComponent(GL_SHININESS, 5.0);
	setLightComponent(GL_DIFFUSE, 1.0);
	setLightComponent(GL_SPECULAR, 1.0);

	initializedGL_ = true;
}

void ExtGLWidget::applyModelViewAndProjection(Triple beg, Triple end)
{
	Triple center = beg + (end-beg) / 2;
	double radius = (center-beg).length();
	
	glLoadIdentity();

	glRotatef( xRot_-90, 1.0, 0.0, 0.0 ); 
	glRotatef( yRot_, 0.0, 1.0, 0.0 ); 
	glRotatef( zRot_, 0.0, 0.0, 1.0 );
	glScalef( zoom_ * xScale_, zoom_ * yScale_, zoom_ * zScale_ );
	glTranslatef(xShift_-center.x, yShift_-center.y, zShift_-center.z);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if (beg != end) {		
		if (ortho_)		glOrtho( -radius, +radius, -radius, +radius, 0, 40 * radius);
		else			glFrustum( -radius, +radius, -radius, +radius, 5 * radius, 400 * radius );
	} else {
		if (ortho_)		glOrtho( -1.0, 1.0, -1.0, 1.0, 10.0, 100.0 );
		else			glFrustum( -1.0, 1.0, -1.0, 1.0, 10.0, 100.0 );
	}

	glTranslatef( xVPShift_ * 2 * radius , yVPShift_ * 2 * radius , -7 * radius );
}
