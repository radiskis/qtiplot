/***************************************************************************
    File                 : CanvasPicker.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
                           knut.franke*gmx.de
    Description          : Canvas picker

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
#include "CanvasPicker.h"
#include "LegendWidget.h"
#include "ArrowMarker.h"
#include "PlotCurve.h"
#include "../ApplicationWindow.h"

#include <QVector>
#include <QMessageBox>

#include <qwt_text_label.h>
#include <qwt_plot_canvas.h>

CanvasPicker::CanvasPicker(Graph *graph):
	QObject(graph)
{
	pointSelected = false;
	d_editing_marker = 0;

	QwtPlotCanvas *canvas = graph->canvas();
	canvas->installEventFilter(this);
}

bool CanvasPicker::eventFilter(QObject *object, QEvent *e)
{
	if (object != (QObject *)plot()->canvas())
		return false;

	Graph *g = plot();
	QList<QwtPlotMarker *> lines = g->linesList();
	switch(e->type())
	{
		case QEvent::MouseButtonPress:
			{
                const QMouseEvent *me = (const QMouseEvent *)e;
				if (!(me->modifiers() & Qt::ShiftModifier))
                    g->deselect();

				emit selectPlot();

				bool allAxisDisabled = true;
				for (int i=0; i < QwtPlot::axisCnt; i++){
					if (g->axisEnabled(i)){
						allAxisDisabled = false;
						break;
					}
				}

                int dist, point;
                g->closestCurve(me->pos().x(), me->pos().y(), dist, point);

				if (me->button() == Qt::LeftButton && (g->drawLineActive())){
					startLinePoint = me->pos();
					return true;
				}

				if (me->button() == Qt::LeftButton && g->drawTextActive()){
					drawTextMarker(me->pos());
					return true;
				}

				if (!g->zoomOn() && selectMarker(me)){
					if (me->button() == Qt::RightButton)
						emit showMarkerPopupMenu();
					return true;
				}

				return !(me->modifiers() & Qt::ShiftModifier);
			}
			break;

		case QEvent::MouseButtonDblClick:
			{
				if (d_editing_marker) {
					return d_editing_marker->eventFilter(g->canvas(), e);
				} else if (g->selectedMarker()) {
					if (lines.contains(g->selectedMarker())){
						emit viewLineDialog();
						return true;
					}
				} else if (g->isPiePlot()){
                	emit showPlotDialog(0);
                    return true;
				} else {
					const QMouseEvent *me = (const QMouseEvent *)e;
                    int dist, point;
                    QwtPlotCurve *c = g->closestCurve(me->pos().x(), me->pos().y(), dist, point);
                    if (c && dist < 10)
                        emit showPlotDialog(g->curveIndex(c));
                    else
                        emit showPlotDialog(-1);
					return true;
				}
			}
			break;

		case QEvent::MouseMove:
			{
				const QMouseEvent *me = (const QMouseEvent *)e;
				if (me->state() != Qt::LeftButton)
  	            	return true;

				QPoint pos = me->pos();

				DataCurve *c = g->selectedCurveLabels();
				if (c){
					c->moveLabels(pos);
					return true;
				}

				if (plot()->drawLineActive()) {
					drawLineMarker(pos, g->drawArrow());
					return true;
				}

				return false;
			}
			break;

		case QEvent::MouseButtonRelease:
			{
				const QMouseEvent *me = (const QMouseEvent *)e;
				if (g->drawLineActive()) {
					ApplicationWindow *app = g->multiLayer()->applicationWindow();
					if (!app)
						return true;

					ArrowMarker mrk;
					mrk.attach(g);
					mrk.setStartPoint(startLinePoint);
					mrk.setEndPoint(QPoint(me->x(), me->y()));
					mrk.setColor(app->defaultArrowColor);
					mrk.setWidth(app->defaultArrowLineWidth);
					mrk.setStyle(app->defaultArrowLineStyle);
					mrk.setHeadLength(app->defaultArrowHeadLength);
					mrk.setHeadAngle(app->defaultArrowHeadAngle);
					mrk.fillArrowHead(app->defaultArrowHeadFill);
					mrk.drawEndArrow(g->drawArrow());
					mrk.drawStartArrow(false);

					g->addArrow(&mrk);
					g->drawLine(false);
					mrk.detach();
					g->replot();

					return true;
				}
				return false;
			}
			break;

		case QEvent::KeyPress:
			{
				int key = ((const QKeyEvent *)e)->key();

				QwtPlotMarker *selectedMarker = g->selectedMarker();
				if (lines.contains(selectedMarker) &&
					(key == Qt::Key_Enter || key == Qt::Key_Return)){
					emit viewLineDialog();
					return true;
				}
			}
			break;

		default:
			break;
	}
	return QObject::eventFilter(object, e);
}

void CanvasPicker::disableEditing()
{
	if (d_editing_marker) {
		d_editing_marker->setEditable(false);
		d_editing_marker = NULL;
	}
}

void CanvasPicker::drawTextMarker(const QPoint& point)
{
	LegendWidget *aux = new LegendWidget(plot());
	aux->move(point);

	LegendWidget *l = plot()->addText(aux);
	l->setText(tr("enter your text here"));
	ApplicationWindow *app = plot()->multiLayer()->applicationWindow();
	if (app){
		l->setFrameStyle(app->legendFrameStyle);
		l->setFont(app->plotLegendFont);
		l->setTextColor(app->legendTextColor);
		l->setBackgroundColor(app->legendBackground);
	}
    l->showTextDialog();

    delete aux;
	plot()->drawText(false);
	emit drawTextOff();
}

void CanvasPicker::drawLineMarker(const QPoint& point, bool endArrow)
{
	Graph *g = plot();
	g->canvas()->repaint();
	ArrowMarker mrk;
	mrk.attach(g);

	int clw = g->canvas()->lineWidth();
	mrk.setStartPoint(QPoint(startLinePoint.x() + clw, startLinePoint.y() + clw));
	mrk.setEndPoint(QPoint(point.x() + clw,point.y() + clw));
	mrk.setWidth(1);
	mrk.setStyle(Qt::SolidLine);
	mrk.drawEndArrow(endArrow);
	mrk.drawStartArrow(false);

	if (plot()->drawLineActive())
		mrk.setColor(Qt::black);
	else
		mrk.setColor(Qt::red);

	g->replot();
	mrk.detach();
}

bool CanvasPicker::selectMarker(const QMouseEvent *e)
{
	Graph *g = plot();
	const QPoint point = e->pos();
	QList<QwtPlotMarker *> lines = g->linesList();
	foreach(QwtPlotMarker *i, lines){
		ArrowMarker* mrkL = (ArrowMarker*)i;
		int d = qRound(mrkL->width() + floor(mrkL->headLength()*tan(M_PI*mrkL->headAngle()/180.0)+0.5));
		double dist = mrkL->dist(point.x(), point.y());
		if (dist <= d){
			disableEditing();
			if (e->modifiers() & Qt::ShiftModifier) {
				plot()->setSelectedMarker(i, true);
				return true;
			} else if (e->button() == Qt::RightButton) {
				mrkL->setEditable(false);
				g->setSelectedMarker(i, true);
				return true;
			}
			g->deselectMarker();
			mrkL->setEditable(true);
			g->setSelectedMarker(i, false);
			d_editing_marker = mrkL;
			return true;
		}
	}
	return false;
}
