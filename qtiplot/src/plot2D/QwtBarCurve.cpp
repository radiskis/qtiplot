/***************************************************************************
    File                 : QwtBarCurve.cpp
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
#include "QwtBarCurve.h"
#include "Graph.h"
#include <QPainter>
#include <qwt_painter.h>
#include <stdio.h>

QwtBarCurve::QwtBarCurve(BarStyle style, Table *t, const QString& xColName, const QString& name, int startRow, int endRow):
    DataCurve(t, xColName, name, startRow, endRow),
    bar_offset(0),
    bar_gap(20),
	d_is_stacked(false)
{
	bar_style = style;

	QPen pen = QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	pen.setCosmetic(true);
	setPen(pen);
	setBrush(QBrush(Qt::red));
	setStyle(QwtPlotCurve::UserCurve);

	if (bar_style == Vertical){
		setType(Graph::VerticalBars);
		setPlotStyle(Graph::VerticalBars);
	} else {
		setType(Graph::HorizontalBars);
		setPlotStyle(Graph::HorizontalBars);
	}
}

void QwtBarCurve::copy(QwtBarCurve *b)
{
	bar_gap = b->bar_gap;
	bar_offset = b->bar_offset;
	bar_style = b->bar_style;
	d_is_stacked = b->isStacked();
}

void QwtBarCurve::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if ( !painter || dataSize() <= 0 )
		return;

	if (to < 0)
		to = dataSize() - 1;

	painter->save();
	painter->setPen(QwtPainter::scaledPen(pen()));
	painter->setBrush(QwtPlotCurve::brush());

	double dx, dy, ref;
	double bar_width = 0;

	if (bar_style == Vertical)
		ref = yMap.transform(1e-100); //smalest positive value for log scales
	else
		ref = xMap.transform(1e-100);

	if (bar_style == Vertical){
		dx = xMap.xTransform(x(from + 1)) - xMap.xTransform(x(from));
		for (int i = from + 2; i < to; i++){
			double min = xMap.xTransform(x(i + 1)) - xMap.xTransform(x(i));
			if (min <= dx)
				dx = min;
		}
		bar_width = dx*(1 - bar_gap*0.01);
	} else {
		dy = abs(yMap.xTransform(y(from + 1)) - yMap.xTransform(y(from)));
		for (int i = from + 2; i<to; i++){
			double min = yMap.xTransform(y(i + 1)) - yMap.xTransform(y(i));
			if (min <= dy)
				dy = min;
		}
		bar_width = dy*(1 - bar_gap*0.01);
	}

	QList <QwtBarCurve *> stack = stackedCurvesList();

	const double half_width = (0.5 - bar_offset*0.01)*bar_width;
	double bw1 = bar_width;
	for (int i = from; i <= to; i++){
		const double px = xMap.xTransform(x(i));
		const double py = yMap.xTransform(y(i));

		QRectF rect = QRectF();
		if (bar_style == Vertical){
			if (y(i) < 0)
				rect = QRectF(px - half_width, ref, bw1, (py - ref));
			else {
				if (stack.isEmpty())
					rect = QRectF(px - half_width, py, bw1, (ref - py));
				else {
					double sOffset = stackOffset(i, stack);
					rect = QRectF(px - half_width, yMap.xTransform(y(i) + sOffset), bw1, 1);
					rect.setBottom(yMap.xTransform(sOffset));
				}
			}
		} else {
			if (x(i) < 0)
				rect = QRectF(px, py - half_width, (ref - px), bw1);
			else {
				if (stack.isEmpty())
					rect = QRectF(ref, py - half_width, (px - ref), bw1);
				else {
					double sOffset = stackOffset(i, stack);
					rect = QRectF(xMap.xTransform(sOffset), py - half_width, 1, bw1);
					rect.setRight(xMap.xTransform(x(i) + sOffset));
				}
			}
		}

		if (d_is_stacked)
			painter->fillRect(rect, Qt::white);
		painter->drawRect(rect);
	}
	painter->restore();
}

QList <QwtBarCurve *> QwtBarCurve::stackedCurvesList() const
{
	QList <QwtBarCurve *> stack;
	Graph *g = (Graph *)plot();
	if (!g)
		return stack;

	for (int i = 0; i < g->curveCount(); i++){
		DataCurve *c = g->dataCurve(i);
		if (!c || c == this)
			continue;

		if (c->type() != Graph::VerticalBars && c->type() != Graph::HorizontalBars)
			continue;

		QwtBarCurve *bc = (QwtBarCurve *)c;
		if (bc->isStacked() && bc->orientation() == bar_style &&
			g->curveIndex((QwtPlotItem *)bc) < g->curveIndex((QwtPlotItem *)this))
			stack << bc;
	}
	return stack;
}

double QwtBarCurve::stackOffset(int i, QList <QwtBarCurve *> stack) const
{
	double n = (double)dataSize();
	if (i < 0 || i >= n)
		return 0.0;

	double stackOffset = 0.0;
	if (bar_style == Vertical){
		foreach(QwtBarCurve *bc, stack)
			stackOffset += bc->y(i);
	} else {
		foreach(QwtBarCurve *bc, stack)
			stackOffset += bc->x(i);
	}
	return stackOffset;
}

QwtDoubleRect QwtBarCurve::boundingRect() const
{
	QwtDoubleRect rect = QwtPlotCurve::boundingRect();

	double n = (double)dataSize();

	if (bar_style == Vertical){
		double dx = (rect.right() - rect.left())/n;
		rect.setLeft(rect.left() - dx);
		rect.setRight(rect.right() + dx);
	} else {
		double dy = (rect.bottom() - rect.top())/n;
		rect.setTop(rect.top() - dy);
		rect.setBottom(rect.bottom() + dy);
	}

	if (!d_is_stacked)
		return rect;

	QList <QwtBarCurve *> stack = stackedCurvesList();
	if (!stack.isEmpty()){
		double maxStackOffset = 0.0;
		for (int i = 0; i < n; i++){
			const double soffset = stackOffset(i, stack);
			if (soffset > maxStackOffset)
				maxStackOffset = soffset;
		}
		if (bar_style == Vertical)
			rect.setBottom(rect.bottom() + maxStackOffset);
		else
			rect.setRight(rect.right() + maxStackOffset);
	}
	return rect;
}

void QwtBarCurve::setGap (int gap)
{
	if (bar_gap == gap)
		return;

	bar_gap =gap;
}

void QwtBarCurve::setOffset(int offset)
{
	if (bar_offset == offset)
		return;

	bar_offset = offset;
}

double QwtBarCurve::dataOffset()
{
	if (bar_style == Vertical){
		const QwtScaleMap &xMap = plot()->canvasMap(xAxis());
		double dx = xMap.xTransform(x(1)) - xMap.xTransform(x(0));
		if (plot()->isVisible()){
			for (int i = 2; i < dataSize(); i++){
				double min = xMap.xTransform(x(i)) - xMap.xTransform(x(i - 1));
				if (min <= dx)
					 dx = min;
			}
			double bar_width = dx*(1 - bar_gap*0.01);
			double x1 = xMap.xTransform(minXValue()) + bar_offset*0.01*bar_width;
			return xMap.invTransform(x1) - minXValue();
		} else
			return 0.5*bar_offset*0.01*dx*(1 - bar_gap*0.01);
	} else {
		const QwtScaleMap &yMap = plot()->canvasMap(yAxis());
		double dy = yMap.xTransform(y(1)) - yMap.xTransform(y(0));
		if (plot()->isVisible()){
			for (int i = 2; i < dataSize(); i++){
				double min = yMap.xTransform(y(i)) - yMap.xTransform(y(i - 1));
				if (min <= dy)
					dy = min;
			}
			double bar_width = dy*(1 - bar_gap*0.01);
			double y1 = yMap.xTransform(minYValue()) + bar_offset*0.01*bar_width;
			return yMap.invTransform(y1) - minYValue();
		} else
			return 0.5*bar_offset*0.01*dy*(1 - bar_gap*0.01);
	}
	return 0;
}

QString QwtBarCurve::saveToString()
{
	QString s = DataCurve::saveToString();
	if (d_is_stacked)
		s += "<StackWhiteOut>1</StackWhiteOut>\n";
	return s;
}
