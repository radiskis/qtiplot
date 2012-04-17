/***************************************************************************
File                 : LinearColor.h
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2012 by Ion Vasilief
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Linear Color Map for 3D graph widget
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
#ifndef LINEARCOLOR_H
#define LINEARCOLOR_H

#include <LinearColorMap.h>
#include <qwt3d_curve.h>

using namespace Qwt3D;

class LinearColor : public StandardColor
{
public:
	LinearColor(Qwt3D::Curve*, const LinearColorMap&);
	LinearColor(Qwt3D::Curve*, const Qwt3D::ColorVector&);
	Qwt3D::RGBA operator()(double x, double y, double z) const;

	double alpha(){return d_alpha;}
	void setAlpha(double a);
	void resetAlpha();

	LinearColorMap* colorMapPointer(){return &d_color_map;}
	LinearColorMap colorMap(){return d_color_map;}
	void setColorMap(const LinearColorMap&);

	std::vector<double> colorStops() const;
	Qwt3D::ColorVector& createVector(Qwt3D::ColorVector& vec) const;

protected:
	LinearColorMap d_color_map;
	double d_alpha;
};

#endif // LINEARCOLOR_H
