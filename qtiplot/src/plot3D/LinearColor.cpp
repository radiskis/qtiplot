/***************************************************************************
File                 : LinearColor.cpp
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
#include <LinearColor.h>

LinearColor::LinearColor(Qwt3D::Curve* curve, const LinearColorMap& colorMap):
StandardColor(curve, 0),
d_color_map(colorMap),
d_alpha(1.0)
{}

LinearColor::LinearColor(Qwt3D::Curve* curve, const Qwt3D::ColorVector& colors):
StandardColor(curve, 0),
d_color_map(LinearColorMap())
{
	setColorVector(colors);
}

Qwt3D::RGBA LinearColor::operator()(double, double, double z) const
{
	double zmin, zmax;
	data_->plot()->coordinates()->axes[Z1].limits(zmin, zmax);

	int size = (int)colors_.size() - 1;
	if (size >= 0){
		int index = (int)(size*(z - zmin)/(zmax - zmin));
		if (index < 0)
			index = 0;
		if (index > size)
			index = size;
		return colors_[index];
	}

	const QwtDoubleInterval range = d_color_map.intensityRange().isValid() ? d_color_map.intensityRange() : QwtDoubleInterval(zmin, zmax);
	QRgb color = d_color_map.rgb(range, z);
	return RGBA(qRed(color)/255., qGreen(color)/255., qBlue(color)/255., d_alpha);
}

void LinearColor::setAlpha(double a)
{
	if (a < 0 || a > 1 || d_alpha == a)
		return;

	d_alpha = a;

	unsigned int size = colors_.size();
	if (size){
		for (unsigned int i = 0; i < size; ++i)
			colors_[i].a = a;
	}
}

void LinearColor::resetAlpha()
{
	unsigned int size = colors_.size();
	if (size){
		for (unsigned int i = 0; i < size; ++i)
			colors_[i].a = d_alpha;
	}
}

std::vector<double> LinearColor::colorStops() const
{
	std::vector<double> stops;
	if (colors_.size() > 0)
		return stops;

	QwtDoubleInterval range = d_color_map.intensityRange();
	if (!range.isValid()){
		double zmin, zmax;
		data_->plot()->coordinates()->axes[Z1].limits(zmin, zmax);
		range = QwtDoubleInterval(zmin, zmax);
	}
	double l = range.width();
	double zmin = range.minValue();
	stops.push_back(zmin);
	QwtArray<double> values = d_color_map.colorStops();
	for (int i = 0; i < values.size(); i++){
		double val = zmin + values[i]*l;
		if (stops.back() != val)
			stops.push_back(val);
	}

	return stops;
}

Qwt3D::ColorVector& LinearColor::createVector(Qwt3D::ColorVector& vec) const
{
	if (colors_.size()){
		vec.clear();
		vec = colors_;
		return vec;
	}

	const QwtDoubleInterval range = d_color_map.intensityRange();
	double zmin = range.minValue();

	vec.clear();
	int size = 255;
	double dsize = size;
	double dz = range.width()/dsize;
	for (int i = 0; i < size; i++){
		QRgb color = d_color_map.rgb(range, zmin + i*dz);
		vec.push_back(RGBA(qRed(color)/dsize, qGreen(color)/dsize, qBlue(color)/dsize, d_alpha));
	}

	return vec;
}

void LinearColor::setColorMap(const LinearColorMap& colorMap)
{
	colors_.clear();
	d_color_map = colorMap;
}
