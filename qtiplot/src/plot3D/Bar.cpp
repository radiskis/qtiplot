/***************************************************************************
File                 : Bar.cpp
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2006 - 2012 by Ion Vasilief
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : 3D bars (modifed enrichment from QwtPlot3D)
***************************************************************************/
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include <qbitmap.h>

#include <qwt3d_curve.h>
#include <qwt3d_color.h>
#include <qwt3d_plot.h>
#include "Bar.h"

using namespace Qwt3D;

/////////////////////////////////////////////////////////////////
//
//  3D Bars  (modified enrichement example from QwtPlot3D archive)
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
	glLineWidth(curve_->meshLineWidth() );
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
