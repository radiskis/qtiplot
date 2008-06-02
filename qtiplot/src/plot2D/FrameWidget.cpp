/***************************************************************************
    File                 : FrameWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Base class for tool widgets in 2D plots

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
#include "FrameWidget.h"
#include "SelectionMoveResizer.h"
#include "../ApplicationWindow.h"

#include <QPainter>

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>

#include <iostream>

FrameWidget::FrameWidget(Graph *plot):QWidget(plot),
	d_plot(plot),
	d_frame (0),
	d_angle(0)
{
	setAttribute(Qt::WA_DeleteOnClose);

	QPoint pos = plot->canvas()->pos();
	pos = QPoint(pos.x() + 10, pos.y() + 10);
	move(pos);

	connect (this, SIGNAL(showMenu()), plot, SIGNAL(showMarkerPopupMenu()));

	setMouseTracking(true);
	show();
	setFocus();
}

void FrameWidget::paintEvent(QPaintEvent *e)
{
	if (d_frame == Shadow)
		resize(width() + 5, height() + 5);

    QPainter p(this);
	//drawFrame(&p, QRect(0, 0, width - 1, height - 1));
	e->accept();
}

void FrameWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	drawFrame(painter, QRect(x, y, width(), height()));
}

void FrameWidget::setFrameStyle(int style)
{
	if (d_frame == style)
		return;

	d_frame = style;
}

void FrameWidget::updateCoordinates()
{
    d_x = calculateXValue();
    d_y = calculateYValue();
}

void FrameWidget::setOriginCoord(double x, double y)
{
	QPoint pos(d_plot->transform(QwtPlot::xBottom, x), d_plot->transform(QwtPlot::yLeft, y));
	pos = d_plot->canvas()->mapToParent(pos);
	QWidget::move(pos);

	d_x = x;
	d_y = y;
}

void FrameWidget::move(const QPoint& pos)
{
	QWidget::move(pos);
	updateCoordinates();
}

double FrameWidget::calculateXValue()
{
	QPoint d_pos = d_plot->canvas()->mapFromParent(pos());
	return d_plot->invTransform(QwtPlot::xBottom, d_pos.x());
}

double FrameWidget::calculateYValue()
{
	QPoint d_pos = d_plot->canvas()->mapFromParent(pos());
	return d_plot->invTransform(QwtPlot::yLeft, d_pos.y());
}

void FrameWidget::drawFrame(QPainter *p, const QRect& rect)
{
	p->save();
	p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

	if (d_frame == Line)
        QwtPainter::drawRect(p, rect);
	else if (d_frame == Shadow) {
		QRect shadow_right = QRect(rect.right() + 1, rect.y() + 5, 5, rect.height());
		QRect shadow_bottom = QRect(rect.x() + 5, rect.bottom() + 1, rect.width(), 5);
		p->setBrush(QBrush(Qt::black));
		p->drawRect(shadow_right);
		p->drawRect(shadow_bottom);
		//p->setBrush(d_text->backgroundBrush());
		QwtPainter::drawRect(p,rect);
	}
	p->restore();
}

void FrameWidget::mousePressEvent (QMouseEvent *)
{
	d_plot->activateGraph();
	/*bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
	if (!shiftPressed)
        d_plot->deselectMarker();
	d_plot->select(this, shiftPressed);*/

	d_plot->deselectMarker();
	d_plot->select(this);
}

QString FrameWidget::saveToString()
{
	QString s = "<Frame>" + QString::number(d_frame) + "</Frame>\n";
	s += "<x>" + QString::number(d_x, 'g', 14) + "</x>\n";
	s += "<y>" + QString::number(d_y, 'g', 14) + "</y>\n";
	return s;
}
