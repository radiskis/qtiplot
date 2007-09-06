/***************************************************************************
    File                 : Grid.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D Grid class

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
#ifndef MYGRID_H
#define MYGRID_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>

class Grid : public QwtPlotGrid
{
public:
    Grid(){d_x_zero_line = false; d_y_zero_line = false; mrkX = -1; mrkY = -1;};

	void draw (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect &rect) const;
	void drawLines(QPainter *painter, const QRect &rect, Qt::Orientation orientation, const QwtScaleMap &map,
    	const QwtValueList &values) const;

    bool xZeroLineEnabled(){return d_x_zero_line;};
    void enableZeroLineX(bool enable = true);
    bool yZeroLineEnabled(){return d_y_zero_line;};
    void enableZeroLineY(bool enable = true);

	void setMajPenX(const QPen &p){	setMajPen(p);};
	const QPen& majPenX() const {return majPen();};

	void setMinPenX(const QPen &p){	setMinPen(p);};
	const QPen& minPenX() const {return minPen();};
	
	void setMajPenY(const QPen &p){	if (d_maj_pen_y != p) d_maj_pen_y = p;};
	const QPen& majPenY() const {return d_maj_pen_y;};

	void setMinPenY(const QPen &p){	if (d_min_pen_y != p) d_min_pen_y = p;};
	const QPen& minPenY() const {return d_min_pen_y;};
	
	void load(const QStringList& );
	void copy(Grid *);
	QString saveToString();
	
private:
    bool d_x_zero_line, d_y_zero_line;
	QPen d_maj_pen_y;
	QPen d_min_pen_y;

	long mrkX, mrkY;//x=0 et y=0 line markers keys
};

#endif
