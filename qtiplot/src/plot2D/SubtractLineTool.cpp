/***************************************************************************
	File                 : SubtractLineTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 - 2011 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Plot tool for substracting a straight line

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
#include "SubtractLineTool.h"
#include <RangeSelectorTool.h>
#include <ApplicationWindow.h>
#include <PlotCurve.h>

#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include <QApplication>

SubtractLineTool::SubtractLineTool(Graph *graph, ApplicationWindow *app, const QObject *status_target, const char *status_slot)
	: PlotToolInterface(graph)
{
	d_selected_points = 0;
	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	d_picker_tool = new ScreenPickerTool(d_graph, this, SIGNAL(statusText(const QString&)));
	d_graph->canvas()->setCursor(QCursor(QPixmap(":/cursor.png"), -1, -1));

	QString msg = tr("Move cursor and click to select and double-click/press 'Enter' to set the position of the first point!");
	emit statusText(msg);

	connect(d_picker_tool, SIGNAL(selected(const QwtDoublePoint &)), this, SLOT(selectPoint(const QwtDoublePoint &)));
	d_graph->canvas()->grabMouse();
}

SubtractLineTool::~SubtractLineTool()
{
	d_graph->canvas()->releaseMouse();

	if (d_picker_tool)
		delete d_picker_tool;
}

void SubtractLineTool::selectPoint(const QwtDoublePoint &pos)
{
	d_selected_points++;
	if (d_selected_points == 2){
		d_line.setP2(QPointF(pos.x(), pos.y()));
		finalize();
	} else {
		d_line.setP1(QPointF(pos.x(), pos.y()));

		d_first_point_marker = new QwtPlotMarker();
		d_first_point_marker->setLinePen(QPen(Qt::red, 1, Qt::DashLine));
		d_first_point_marker->setLineStyle(QwtPlotMarker::Cross);
		d_first_point_marker->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(Qt::red, 1), QSize(12, 12)));
		d_first_point_marker->setValue(pos.x(), pos.y());
		d_first_point_marker->attach(d_graph);
		d_graph->replot();

		QString msg = tr("First point selected! Click to select and double-click/press 'Enter' to set the position of the 2nd point!");
		emit statusText(msg);
	}
}

void SubtractLineTool::finalize()
{
	delete d_picker_tool; d_picker_tool = NULL;
	d_graph->canvas()->releaseMouse();
	d_first_point_marker->detach();
	delete d_first_point_marker;

	if (d_graph->activeTool() && d_graph->activeTool()->rtti() == PlotToolInterface::Rtti_RangeSelector){
		((RangeSelectorTool *)d_graph->activeTool())->setEnabled();
	} else
		d_graph->canvas()->unsetCursor();

	double den = d_line.dx();
	double slope = 0.0;
	double intercept = 0.0;
	if (den != 0.0){
		slope = d_line.dy()/den;
		intercept = d_line.y1() - slope*d_line.x1();
	}

	for (int i = 0; i < d_graph->curveCount(); i++){
		DataCurve *c = d_graph->dataCurve(i);
		if (!c || c->type() == Graph::ErrorBars)
			continue;

		Table *t = c->table();
		int yCol = t->colIndex(c->title().text());
		int startRow = c->startRow(), endRow = c->endRow();
		if (startRow < 0)
			startRow = 0;
		if (endRow < 0)
			endRow = c->dataSize() - 1;
		for (int j = startRow; j <= endRow; j++){
			if (!t->text(j, yCol).isEmpty()){
				int index = j - startRow;
				t->setCell(j, yCol, c->y(index) - (c->x(index)*slope + intercept));
			}
		}
		t->notifyChanges(c->title().text());
	}
	d_graph->setActiveTool(NULL);
}
