/***************************************************************************
	File                 : LinearColorMap.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : A wrapper around QwtLinearColorMap from Qwt

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

#ifndef LINEAR_COLOR_MAP_H
#define LINEAR_COLOR_MAP_H

#include <qwt_color_map.h>

//! A customized QwtLinearColorMap.
/**
 *
 */
class LinearColorMap : public QwtLinearColorMap
{
public:
	//! Constructor.
	LinearColorMap();
	//! Constructor.
	LinearColorMap(const QColor &from, const QColor &to);
	//! Set the intensity range
	void setIntensityRange(const QwtDoubleInterval& range){d_range = range;}
	//! Set the intensity range
	void setIntensityRange(double vmin, double vmax){d_range = QwtDoubleInterval(vmin, vmax);}
	//! Get the intensity range
	QwtDoubleInterval intensityRange() const;
	//! Get the lower range limit
	double lowerBound(){return d_range.minValue();}
	//! Get the upper range limit
	double upperBound(){return d_range.maxValue();}


	//! Exports the map to a pseudo-XML string
	QString toXmlString();
	//! Used when restoring from project files
	static LinearColorMap fromXmlStringList(const QStringList& lst);

private:
	QwtDoubleInterval d_range;
};

#endif
