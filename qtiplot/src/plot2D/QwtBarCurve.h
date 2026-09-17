/***************************************************************************
    File                 : QwtBarCurve.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Bar curve

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
#ifndef BARCURVE_H
#define BARCURVE_H

#include "PlotCurve.h"
#include <qwt_plot.h>

//! Bar curve
class QwtBarCurve: public DataCurve
{
public:
	enum BarStyle{Vertical = 0, Horizontal = 1};
	QwtBarCurve(BarStyle style, Table *t, const QString& xColName, const QString& name, int startRow, int endRow);

	void copy(QwtBarCurve *b);

	QRectF boundingRect() const override;

	BarStyle orientation() const {return bar_style;};

	void setGap (int gap);
	int gap() const {return bar_gap;};

	void setOffset(int offset);
	int offset() const {return bar_offset;};

	double dataOffset() const;

	bool isStacked() const {return d_is_stacked;};
	void setStacked(bool on = true){d_is_stacked = on;};

	QString saveToString() const override;

	QList <QwtBarCurve *> stackedCurvesList() const;
	double stackOffset(int i, QList <QwtBarCurve *> stack) const;

private:
	void drawSeries(QPainter *painter, const QwtScaleMap &xMap,
		const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to) const override;

protected:
	int bar_offset = 0;
	int bar_gap = 0;
	bool d_is_stacked = false;
	BarStyle bar_style = Vertical;
};

#endif
