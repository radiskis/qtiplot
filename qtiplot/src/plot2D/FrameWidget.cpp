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
	d_color(Qt::black),
	d_angle(0)
{
	setAttribute(Qt::WA_DeleteOnClose);

	QPoint pos = plot->canvas()->pos();
	pos = QPoint(pos.x() + 10, pos.y() + 10);
	move(pos);

	connect (this, SIGNAL(showMenu()), plot->multiLayer(), SIGNAL(showMarkerPopupMenu()));
	connect (this, SIGNAL(showDialog()), plot->multiLayer(), SIGNAL(showEnrichementDialog()));

	setMouseTracking(true);
	show();
	setFocus();
}

void FrameWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
	drawFrame(&p, rect());
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

	int old_frame_style = d_frame;
	d_frame = style;

	if (d_frame == Shadow)
		resize(width() + 5, height() + 5);
	else if (old_frame_style == Shadow)
		resize(width() - 5, height() - 5);
}

void FrameWidget::updateCoordinates()
{
    d_x = calculateXValue();
    d_y = calculateYValue();
	d_x_right = calculateRightValue();
	d_y_bottom = calculateBottomValue();
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

void FrameWidget::setBoundingRect(double left, double top, double right, double bottom)
{
    if (d_x == left && d_y == top && d_x_right == right && d_y_bottom == bottom)
        return;

    d_x = left;
    d_y = top;
    d_x_right = right;
    d_y_bottom = bottom;

    if (!d_plot)
        return;

	QPoint pos(d_plot->transform(QwtPlot::xBottom, d_x), d_plot->transform(QwtPlot::yLeft, d_y));
	pos = d_plot->canvas()->mapToParent(pos);

	QPoint bottomRight(d_plot->transform(QwtPlot::xBottom, d_x_right),
						d_plot->transform(QwtPlot::yLeft, d_y_bottom));
	bottomRight = d_plot->canvas()->mapToParent(bottomRight);

    resize(QSize(abs(bottomRight.x() - pos.x() + 1), abs(bottomRight.y() - pos.y() + 1)));
	QWidget::move(pos);
}

void FrameWidget::setRect(int x, int y, int w, int h)
{
    if (pos() == QPoint(x, y) && size() == QSize(w, h))
        return;

    resize(QSize(w, h));
    move(QPoint(x, y));
}

void FrameWidget::setSize(int w, int h)
{
    if (size() == QSize(w, h))
        return;

    resize(QSize(w, h));
    d_x_right = calculateRightValue();
	d_y_bottom = calculateBottomValue();
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

double FrameWidget::calculateRightValue()
{
	QPoint d_pos = d_plot->canvas()->mapFromParent(geometry().bottomRight());
	return d_plot->invTransform(QwtPlot::xBottom, d_pos.x());
}

double FrameWidget::calculateBottomValue()
{
	QPoint d_pos = d_plot->canvas()->mapFromParent(geometry().bottomRight());
	return d_plot->invTransform(QwtPlot::yLeft, d_pos.y());
}

QwtDoubleRect FrameWidget::boundingRect() const
{
    return QwtDoubleRect(d_x, d_y, qAbs(d_x_right - d_x), qAbs(d_y_bottom - d_y));
}

void FrameWidget::drawFrame(QPainter *p, const QRect& rect)
{
	p->save();
	if (d_frame == Line){
		p->setPen(QPen(d_color, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		p->setBrush(QBrush(Qt::white));
        QwtPainter::drawRect(p, rect.adjusted(0, 0, -1, -1));
	} else if (d_frame == Shadow){
		p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		p->setBrush(QBrush(Qt::black));
		p->drawRect(rect.adjusted(5, 5, 0, 0));
		p->setPen(QPen(d_color, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		p->setBrush(QBrush(Qt::white));
		QwtPainter::drawRect(p, rect.adjusted(0, 0, -5, -5));
	} else
		p->fillRect(rect, Qt::white);

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
	s += "<Color>" + d_color.name() + "</Color>\n";
	s += "<x>" + QString::number(d_x, 'g', 14) + "</x>\n";
	s += "<y>" + QString::number(d_y, 'g', 14) + "</y>\n";
	return s;
}
