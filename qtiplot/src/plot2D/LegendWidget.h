/***************************************************************************
    File                 : LegendWidget.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A 2D Plot Legend Widget

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
#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include "FrameWidget.h"
#include "Graph.h"

#include <qwt_plot.h>
#include <qwt_text.h>
	
#include <QWidget>

class PlotCurve;
	
class LegendWidget: public FrameWidget
{
	Q_OBJECT

public:
    LegendWidget(Graph *);
	~LegendWidget();

	void clone(LegendWidget* t);

	QString text(){return d_text->text();};
	void setText(const QString& s);

	QColor textColor(){return d_text->color();};
	void setTextColor(const QColor& c);

	QColor backgroundColor(){return d_text->backgroundBrush().color();};
	void setBackgroundColor(const QColor& c);

	QFont font(){return d_text->font();};
	void setFont(const QFont& font);

	void showTextEditor();
	void showTextDialog(){emit showDialog();};
	void showContextMenu(){emit showMenu();};

    void print(QPainter *p, const QwtScaleMap map[QwtPlot::axisCnt]);

private:
	PlotCurve* getCurve(const QString& s, int &point);
	void drawFrame(QPainter *p, const QRect& rect);
	void drawVector(PlotCurve *c, QPainter *p, int x, int y, int l);
	void drawSymbol(PlotCurve *c, int point, QPainter *p, int x, int y, int l);
	void drawText(QPainter *, const QRect&, QwtArray<long>, int);

	QwtArray<long> itemsHeight(int y, int symbolLineLength, int &width, int &height);
	int symbolsMaxWidth();
	QString parse(const QString& str);

	virtual void paintEvent(QPaintEvent *e);

	//! Pointer to the QwtText object
	QwtText* d_text;

	//! Distance between symbols and legend text
	int h_space;

	//! Distance between frame and content
	int left_margin, top_margin;

	//! Length of the symbol line
	int line_length;

signals:
	void enableEditor();
};

#endif
