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
#include <Graph.h>
#include <MultiLayer.h>
#include <PenStyleBox.h>
#include <ApplicationWindow.h>

#include <QPainter>
#include <QPaintEngine>

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>

FrameWidget::FrameWidget(Graph *plot):QWidget(plot->multiLayer()->canvas()),
	d_plot(plot),
	d_frame (0),
	d_frame_pen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin)),
	d_brush(QBrush()),
	d_angle(0),
	d_shadow_width(5),
	d_attach_policy((AttachPolicy)plot->multiLayer()->applicationWindow()->d_graph_attach_policy),
	d_on_top(true)
{
	setAttribute(Qt::WA_DeleteOnClose);

	QPalette palette;
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);

	QPoint pos = plot->canvas()->pos();
	pos = QPoint(pos.x() + 10, pos.y() + 10);
	move(pos);

	connect (this, SIGNAL(showMenu()), plot->multiLayer(), SIGNAL(showMarkerPopupMenu()));
	connect (this, SIGNAL(showDialog()), plot->multiLayer(), SIGNAL(showEnrichementDialog()));

	d_plot->raiseEnrichements();
	raise();

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
	int right = map[QwtPlot::xBottom].transform(calculateRightValue());
	int bottom = map[QwtPlot::yLeft].transform(calculateBottomValue());

	drawFrame(painter, QRect(x, y, abs(right - x), abs(bottom - y)));
}

void FrameWidget::setFrameStyle(int style)
{
	if (d_frame == style)
		return;

	int old_frame_style = d_frame;
	d_frame = style;

	if (d_frame == Shadow)
		resize(width() + d_shadow_width, height() + d_shadow_width);
	else if (old_frame_style == Shadow)
		resize(width() - d_shadow_width, height() - d_shadow_width);

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
	pos = d_plot->canvas()->mapTo(d_plot->multiLayer()->canvas(), pos);
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

	QWidget *layerCanvas = d_plot->canvas();
	QWidget *windowCanvas = (QWidget *)d_plot->parent();

	QPoint pos(d_plot->transform(QwtPlot::xBottom, d_x), d_plot->transform(QwtPlot::yLeft, d_y));
	pos = layerCanvas->mapTo(windowCanvas, pos);

	QPoint bottomRight(d_plot->transform(QwtPlot::xBottom, d_x_right),
						d_plot->transform(QwtPlot::yLeft, d_y_bottom));
	bottomRight = layerCanvas->mapTo(windowCanvas, bottomRight);

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
	QPoint d_pos = d_plot->canvas()->mapFrom(d_plot->multiLayer()->canvas(), pos());
	return d_plot->invTransform(QwtPlot::xBottom, d_pos.x());
}

double FrameWidget::calculateYValue()
{
	QPoint d_pos = d_plot->canvas()->mapFrom(d_plot->multiLayer()->canvas(), pos());
	return d_plot->invTransform(QwtPlot::yLeft, d_pos.y());
}

double FrameWidget::calculateRightValue()
{
	QPoint d_pos = d_plot->canvas()->mapFrom(d_plot->multiLayer()->canvas(), geometry().bottomRight());
	return d_plot->invTransform(QwtPlot::xBottom, d_pos.x());
}

double FrameWidget::calculateBottomValue()
{
	QPoint d_pos = d_plot->canvas()->mapFrom(d_plot->multiLayer()->canvas(), geometry().bottomRight());
	return d_plot->invTransform(QwtPlot::yLeft, d_pos.y());
}

QRectF FrameWidget::boundingRect() const
{
    return QRectF(d_x, d_y, fabs(d_x_right - d_x), fabs(d_y_bottom - d_y));
}

void FrameWidget::drawFrame(QPainter *p, const QRect& rect)
{
	QColor background = palette().color(QPalette::Window);

	p->save();
	if (d_frame == Line){
		QPen pen = QwtPainter::scaledPen(d_frame_pen);
		p->setPen(pen);
		int lw = pen.width()/2;
		QRect r = rect.adjusted(lw, lw, -lw - 1, -lw - 1);
		if (background.alpha() != 0)
			p->fillRect(r, background);
		if (d_brush.style() != Qt::NoBrush)
			p->setBrush(d_brush);

        QwtPainter::drawRect(p, r);
	} else if (d_frame == Shadow){
		int lw = d_frame_pen.width()/2;

		// calculate resolution factor
		double factorX = (double)p->paintEngine()->paintDevice()->logicalDpiX()/(double)plot()->logicalDpiX();
		double factorY = (double)p->paintEngine()->paintDevice()->logicalDpiY()/(double)plot()->logicalDpiY();

		int d = d_shadow_width + lw;
		if (!(lw % 2))
			d += 1;

		QPainterPath contents;
		QRect r = rect.adjusted(lw, lw, -qRound(d*factorX), -qRound(d*factorY));
		contents.addRect(r);

		QPainterPath shadow;
		shadow.addRect(rect.adjusted(qRound(d_shadow_width*factorX), qRound(d_shadow_width*factorY), 0, 0));

		p->fillPath(shadow.subtracted(contents), Qt::black);//draw shadow
		if (background.alpha() != 0)
			p->fillRect(r, background);
		p->setPen(QwtPainter::scaledPen(d_frame_pen));
		if (d_brush.style() != Qt::NoBrush)
			p->setBrush(d_brush);
		QwtPainter::drawRect(p, r);
	} else {
		if (background.alpha() != 0)
			p->fillRect(rect, background);
		if (d_brush.style() != Qt::NoBrush)
			p->fillRect(rect, d_brush);
	}

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
	s += "<Color>" + d_frame_pen.color().name() + "</Color>\n";
	s += "<FrameWidth>" + QString::number(d_frame_pen.widthF()) + "</FrameWidth>\n";
	s += "<LineStyle>" + QString::number(PenStyleBox::styleIndex(d_frame_pen.style())) + "</LineStyle>\n";

	s += "<x>" + QString::number(d_x, 'g', 14) + "</x>\n";
	s += "<y>" + QString::number(d_y, 'g', 14) + "</y>\n";
	s += "<right>" + QString::number(d_x_right, 'g', 15) + "</right>\n";
    s += "<bottom>" + QString::number(d_y_bottom, 'g', 15) + "</bottom>\n";
    s += "<attachTo>" + QString::number(d_attach_policy) + "</attachTo>\n";
	s += "<onTop>" + QString::number(d_on_top) + "</onTop>\n";
	s += "<visible>" + QString::number(!isHidden()) + "</visible>\n";
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
	double val = 0.0;
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
	double dpi = (double)w->logicalDpiX();
	double val = 0.0;
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
	double dpi = (double)w->logicalDpiY();
	double val = 0.0;
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

void FrameWidget::setRect(QWidget *widget, double x, double y, double w, double h, Unit unit)
{
    if (!widget)
        return;

	int dpiX = widget->logicalDpiX();
	int dpiY = widget->logicalDpiY();

	FrameWidget *fw = qobject_cast<FrameWidget *>(widget);
    if (fw){
        switch(unit){
            case Pixel:
                fw->setRect(qRound(x), qRound(y), qRound(w), qRound(h));
            break;
            case Inch:
                fw->setRect(qRound(x*dpiX), qRound(y*dpiY), qRound(w*dpiX), qRound(h*dpiY));
            break;
            case Millimeter:
                fw->setRect(qRound(x*dpiX/25.4), qRound(y*dpiY/25.4), qRound(w*dpiX/25.4), qRound(h*dpiY/25.4));
            break;
            case Centimeter:
                fw->setRect(qRound(x*dpiX/2.54), qRound(y*dpiY/2.54), qRound(w*dpiX/2.54), qRound(h*dpiY/2.54));
            break;
            case Point:
                fw->setRect(qRound(x*dpiX/72.0), qRound(y*dpiY/72.0), qRound(w*dpiX/72.0), qRound(h*dpiY/72.0));
            break;
            case Scale:
                fw->setCoordinates(x, y, x + w, y + h);
            break;
        }
        return;
    }

    switch(unit){
		case Pixel:
			widget->setGeometry(qRound(x), qRound(y), qRound(w), qRound(h));
		break;
		case Inch:
			widget->setGeometry(qRound(x*dpiX), qRound(y*dpiY), qRound(w*dpiX), qRound(h*dpiY));
		break;
		case Millimeter:
			widget->setGeometry(qRound(x*dpiX/25.4), qRound(y*dpiY/25.4), qRound(w*dpiX/25.4), qRound(h*dpiY/25.4));
		break;
		case Centimeter:
			widget->setGeometry(qRound(x*dpiX/2.54), qRound(y*dpiY/2.54), qRound(w*dpiX/2.54), qRound(h*dpiY/2.54));
		break;
		case Point:
			widget->setGeometry(qRound(x*dpiX/72.0), qRound(y*dpiY/72.0), qRound(w*dpiX/72.0), qRound(h*dpiY/72.0));
		break;
		default:
            break;
	}
}

void FrameWidget::setAttachPolicy(AttachPolicy attachTo)
{
	if (attachTo != d_attach_policy)
		d_attach_policy = attachTo;
}

void FrameWidget::setOnTop(bool on)
{
	if (d_on_top == on)
		return;

	d_on_top = on;
	if (on)
		raise();
	else
		lower();

	plot()->notifyChanges();
}
