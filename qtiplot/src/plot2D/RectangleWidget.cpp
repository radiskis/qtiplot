/***************************************************************************
    File                 : RectangleWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying rectangles in 2D plots

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
#include "RectangleWidget.h"
#include <QPainter>
#include <QPaintEngine>
#include <QPalette>
 
RectangleWidget::RectangleWidget(Graph *plot):FrameWidget(plot)
{
	setFrameStyle(Line);	
	setSize(0, 0);
}

/*void RectangleWidget::paintEvent(QPaintEvent *e)
{
	if (d_pix.isNull())
		return;

	QPainter p(this);
	draw(&p, rect());
	e->accept();
}*/

void RectangleWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	//draw(painter, QRect(x, y, width(), height()));
}

void RectangleWidget::clone(RectangleWidget* r)
{
	d_frame = r->frameStyle();
	d_color = r->frameColor();
	setCoordinates(r->xValue(), r->yValue(), r->right(), r->bottom());
}

QString RectangleWidget::saveToString()
{
	QString s = "<Rectangle>\n";
	s += FrameWidget::saveToString();
	return s + "</Rectangle>\n";
}

void RectangleWidget::restore(Graph *g, const QStringList& lst)
{
	int frameStyle = 0;
	QColor frameColor = Qt::black;
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	QStringList::const_iterator line;
	RectangleWidget *r = new RectangleWidget(g);
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Frame>"))
			frameStyle = s.remove("<Frame>").remove("</Frame>").toInt();
		else if (s.contains("<Color>"))
			frameColor = QColor(s.remove("<Color>").remove("</Color>"));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<right>"))
			right = s.remove("<right>").remove("</right>").toDouble();
		else if (s.contains("<bottom>"))
			bottom = s.remove("<bottom>").remove("</bottom>").toDouble();
	}

	if (r){
		g->add(r, false);
		r->setCoordinates(x, y, right, bottom);
		r->setFrameColor(frameColor);		
		r->setFrameStyle(frameStyle);
	}
}
