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
	d_angle(0),
	d_frame_width(5)
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
		resize(width() + d_frame_width, height() + d_frame_width);
	else if (old_frame_style == Shadow)
		resize(width() - d_frame_width, height() - d_frame_width);
	
	d_x_right = calculateRightValue();
	d_y_bottom = calculateBottomValue();
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

void FrameWidget::setCoordinates(double left, double top, double right, double bottom)
{
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

void FrameWidget::setSize(const QSize& newSize)
{
    if (size() == newSize)
        return;

    resize(newSize);
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

QRectF FrameWidget::boundingRect() const
{
    return QRectF(d_x, d_y, fabs(d_x_right - d_x), fabs(d_y_bottom - d_y));
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
	d_plot->deselectMarker();
	d_plot->select(this);
}

QString FrameWidget::saveToString()
{
	QString s = "<Frame>" + QString::number(d_frame) + "</Frame>\n";
	s += "<Color>" + d_color.name() + "</Color>\n";
	s += "<x>" + QString::number(d_x, 'g', 14) + "</x>\n";
	s += "<y>" + QString::number(d_y, 'g', 14) + "</y>\n";
	s += "<right>" + QString::number(d_x_right, 'g', 15) + "</right>\n";
    s += "<bottom>" + QString::number(d_y_bottom, 'g', 15) + "</bottom>\n";
	return s;
}

double FrameWidget::xIn(QWidget *w, Unit unit)
{
	double dpi = (double)w->logicalDpiX();
	double val = 0.0;
	switch(unit){
		case Pixel:
			val = (double)w->x();
		break;
		case Inch:
			val = w->x()/dpi;
		break;
		case Millimeter:
			val = 25.4*w->x()/dpi;
		break;
		case Centimeter:
			val = 2.54*w->x()/dpi;
		break;
		case Point:
			val = 72.0*w->x()/dpi;
		break;
		case Scale:
			FrameWidget *fw = qobject_cast<FrameWidget *>(w);
			if (fw)
				val = fw->xValue();
		break;
	}
	return val;
}

double FrameWidget::yIn(QWidget *w, Unit unit)
{
	double dpi = (double)w->logicalDpiY();
	double val;
	switch(unit){
		case Pixel:
			val = (double)w->y();
		break;
		case Inch:
			val = w->y()/dpi;
		break;
		case Millimeter:
			val = 25.4*w->y()/dpi;
		break;
		case Centimeter:
			val = 2.54*w->y()/dpi;
		break;
		case Point:
			val = 72.0*w->y()/dpi;
		break;
		case Scale:
			FrameWidget *fw = qobject_cast<FrameWidget *>(w);
			if (fw)
				val = fw->yValue();
		break;
	}
	return val;
}

double FrameWidget::widthIn(QWidget *w, Unit unit)
{
	double dpi = (double)w->physicalDpiX();
	double val;
	switch(unit){
		case Pixel:
			val = (double)w->width();
		break;
		case Inch:
			val = (double)w->width()/dpi;
		break;
		case Millimeter:
			val = 25.4*w->width()/dpi;
		break;
		case Centimeter:
			val = 2.54*w->width()/dpi;
		break;
		case Point:
			val = 72.0*w->width()/dpi;
		break;
		case Scale:
			FrameWidget *fw = qobject_cast<FrameWidget *>(w);
			if (fw)
				val = fabs(fw->right() - fw->xValue());
		break;
	}
	return val;
}

double FrameWidget::heightIn(QWidget *w, Unit unit)
{
	double dpi = (double)w->physicalDpiY();
	double val;
	switch(unit){
		case Pixel:
			val = (double)w->height();
		break;
		case Inch:
			val = (double)w->height()/dpi;
		break;
		case Millimeter:
			val = 25.4*w->height()/dpi;
		break;
		case Centimeter:
			val = 2.54*w->height()/dpi;
		break;
		case Point:
			val = 72.0*w->height()/dpi;
		break;
		case Scale:
			FrameWidget *fw = qobject_cast<FrameWidget *>(w);
			if (fw)
				val = fabs(fw->bottom() - fw->yValue());
		break;
	}
	return val;
}

void FrameWidget::setRect(double x, double y, double w, double h, Unit unit)
{
	int dpiX = logicalDpiX();
	int dpiY = logicalDpiY();
	switch(unit){
		case Pixel:
			setRect((int)x, (int)y, (int)w, (int)h);
		break;
		case Inch:
			setRect(int(x*dpiX), int(y*dpiY), int(w*dpiX), int(h*dpiY));
		break;
		case Millimeter:
			setRect(int(x*dpiX/25.4), int(y*dpiY/25.4), int(w*dpiX/25.4), int(h*dpiY/25.4));
		break;
		case Centimeter:
			setRect(int(x*dpiX/2.54), int(y*dpiY/2.54), int(w*dpiX/2.54), int(h*dpiY/2.54));
		break;
		case Point:
			setRect(int(x*dpiX/72.0), int(y*dpiY/72.0), int(w*dpiX/72.0), int(h*dpiY/72.0));
		break;
		case Scale:
			setCoordinates(x, y, x + w, y + h);
		break;
	}
}
