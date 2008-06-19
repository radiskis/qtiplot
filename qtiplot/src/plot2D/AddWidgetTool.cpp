/***************************************************************************
    File                 : AddWidgetTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Tool for adding enrichements to a plot.

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
#include "AddWidgetTool.h"
#include "Graph.h"
#include "LegendWidget.h"
#include "TexWidget.h"
#include "../ApplicationWindow.h"

#include <QAction>
#include <QCursor>
#include <QMessageBox>

#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_text_label.h>

AddWidgetTool::AddWidgetTool(WidgetType type, Graph *graph, QAction *action, const QObject *status_target, const char *status_slot)
	: QwtPicker(graph), 
	PlotToolInterface(graph),
	d_action(action),
	d_widget_type(type)
{	
	setTrackerMode(QwtPicker::AlwaysOff);
	setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
	graph->setCursor(QCursor(Qt::IBeamCursor));
	
	QwtPlotCanvas *canvas = graph->canvas();
	canvas->setCursor(QCursor(Qt::IBeamCursor));
	canvas->installEventFilter(this);
	
	QwtTextLabel *title = graph->titleLabel();
	if (title)
		title->installEventFilter(this);
	
	for ( uint i = 0; i < QwtPlot::axisCnt; i++ ){
        QwtScaleWidget *scale = graph->axisWidget(i);
        if (scale)
            scale->installEventFilter(this);
    }
		
	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	emit statusText(tr("Click on plot to choose the position of the new object!"));
}

AddWidgetTool::~AddWidgetTool()
{
	d_graph->unsetCursor();
	
	QwtPlotCanvas *canvas = d_graph->canvas();
	canvas->unsetCursor();
	canvas->removeEventFilter(this);
	
	QwtTextLabel *title = d_graph->titleLabel();
	if (title)
		title->removeEventFilter(this);
	
	for ( uint i = 0; i < QwtPlot::axisCnt; i++ ){
        QwtScaleWidget *scale = d_graph->axisWidget(i);
        if (scale)
            scale->removeEventFilter(this);
    }
	
	ApplicationWindow *app = d_graph->multiLayer()->applicationWindow();
	if (app)
		app->pickPointerCursor();
	d_action->setChecked(false);
}

void AddWidgetTool::addEquation(const QPoint& point)
{		
	TexWidget *t = new TexWidget(d_graph);
	if (!t)
		return;
	
	t->setFormula(tr("enter your text here"));
	t->move(point);
	t->setFrameStyle(1);
	d_graph->add(t, false);
	t->showPropertiesDialog();
}

void AddWidgetTool::addText(const QPoint& point)
{
	LegendWidget *l = new LegendWidget(d_graph);
	if (!l)
		return;
	
	l->move(point);

	d_graph->add(l, false);
	l->setText(tr("enter your text here"));
	ApplicationWindow *app = d_graph->multiLayer()->applicationWindow();
	if (app){
		l->setFrameStyle(app->legendFrameStyle);
		l->setFont(app->plotLegendFont);
		l->setTextColor(app->legendTextColor);
		l->setBackgroundColor(app->legendBackground);
	}
    l->showTextDialog();
}

void AddWidgetTool::addWidget(const QPoint& point)
{
	switch (d_widget_type){
		case Text:
			addText(point);
		break;
		case TexEquation:
			addEquation(point);
		break;
		default:
			break;
	}
    d_graph->setActiveTool(NULL);
}

bool AddWidgetTool::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type()) {
		case QEvent::MouseButtonPress:								
			addWidget(d_graph->mapFromGlobal(QCursor::pos()));
			return true;
		default:
			break;
	}
	return QwtPicker::eventFilter(obj, event);
}
