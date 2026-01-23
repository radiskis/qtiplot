/***************************************************************************
    File                 : TranslateCurveTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for translating curves.

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
#include "TranslateCurveTool.h"
#include "Graph.h"
#include "PlotCurve.h"
#include "FunctionCurve.h"
#include <ApplicationWindow.h>
#include "DataPickerTool.h"
#include "ScreenPickerTool.h"
#include <QMessageBox>
#include <QLocale>
#include <QLineEdit>
#include <qwt_plot_curve.h>

TranslateCurveTool::TranslateCurveTool(Graph *graph, ApplicationWindow *app, Direction dir)
	: PlotToolInterface(graph),
	d_dir(dir),
	d_app(app)
{

	// Phase 1: select curve point
	d_sub_tool = new DataPickerTool(d_graph, app, DataPickerTool::Display);
	connect((DataPickerTool*)d_sub_tool, &DataPickerTool::statusText,
			this, &TranslateCurveTool::statusText);
	connect((DataPickerTool*)d_sub_tool, &DataPickerTool::selected,
			this, &TranslateCurveTool::selectCurvePoint);

	emit statusText(tr("Double-click on plot to select a data point!"));
}

void TranslateCurveTool::selectCurvePoint(QwtPlotCurve *curve, int point_index)
{
	if (!curve)
		return;

	if(((PlotCurve *)curve)->type() != Graph::Function){
		DataCurve *c = (DataCurve *)curve;
		Table *t = c->table();
		if (!t)
			return;

	    if (d_dir == Horizontal && t->isReadOnlyColumn(t->colIndex(c->xColumnName()))){
            QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
            tr("The column '%1' is read-only! Operation aborted!").arg(c->xColumnName()));
			delete d_sub_tool;
			d_graph->setActiveTool(NULL);
			return;
        } else if (d_dir == Vertical && t->isReadOnlyColumn(t->colIndex(c->title().text()))){
            QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
            tr("The column '%1' is read-only! Operation aborted!").arg(c->title().text()));
			delete d_sub_tool;
			d_graph->setActiveTool(NULL);
			return;
        }
	}

	d_selected_curve = curve;
	d_curve_point = QPointF(curve->sample(point_index).x(), curve->sample(point_index).y());
	delete d_sub_tool;

	// Phase 2: select destination
	d_sub_tool = new ScreenPickerTool(d_graph);
	connect((ScreenPickerTool*)d_sub_tool, &ScreenPickerTool::statusText,
			this, &TranslateCurveTool::statusText);
	((ScreenPickerTool*)d_sub_tool)->append(d_curve_point);
	ScreenPickerTool::MoveRestriction moveRestriction = ScreenPickerTool::Vertical;
	if (d_dir == Horizontal)
		moveRestriction = ScreenPickerTool::Horizontal;
	((ScreenPickerTool*)d_sub_tool)->setMoveRestriction(moveRestriction);
	connect((ScreenPickerTool*)d_sub_tool, QOverload<const QPointF&>::of(&ScreenPickerTool::selected), this, &TranslateCurveTool::selectDestination);
	emit statusText(tr("Curve selected! Move cursor and click to choose a point and double-click/press 'Enter' to finish!"));
}

void TranslateCurveTool::selectDestination(const QPointF &point)
{
	delete d_sub_tool;
	if (!d_selected_curve)
		return;

	// Phase 3: execute the translation

	if(((PlotCurve *)d_selected_curve)->type() == Graph::Function){
	    if (d_dir == Horizontal){
            QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
            tr("This operation cannot be performed on function curves."));
        } else {
            FunctionCurve *func = (FunctionCurve *)d_selected_curve;
            if (func->functionType() == FunctionCurve::Normal){
                QString formula = func->formulas().first();
                double d = point.y() - d_curve_point.y();
                if (d > 0)
                    func->setFormula(formula + "+" + QString::number(d, 'g', 15));
                else
                    func->setFormula(formula + QString::number(d, 'g', 15));
                func->loadData();
            }
        }
	    d_graph->setActiveTool(NULL);
	    return;
    } else {
    	DataCurve *c = (DataCurve *)d_selected_curve;
		double d = 0.0;
		QString col_name;
		switch(d_dir) {
			case Vertical:
			{
				col_name = c->title().text();
				d = point.y() - d_curve_point.y();
				break;
			}
			case Horizontal:
			{
				col_name = c->xColumnName();
				d = point.x() - d_curve_point.x();
				break;
			}
		}
		Table *tab = d_app->table(col_name);
		if (!tab) return;
		int col = tab->colIndex(col_name);
		if (tab->columnType(col) != Table::Numeric) {
			QMessageBox::warning(d_app, tr("QtiPlot - Warning"),
				tr("This operation cannot be performed on curves plotted from columns having a non-numerical format."));
			return;
		}

		int prec; char f;
		tab->columnNumericFormat(col, &f, &prec);
		int row_start = c->tableRow(0);
    	int row_end = row_start + c->dataSize();

    	QLocale locale = d_app->locale();
		int j = 0;//point index
		for (int i = row_start; i<row_end; i++){
			if (!tab->text(i, col).isEmpty()){
				tab->setText(i, col, locale.toString(
					(d_dir == Horizontal ? d_selected_curve->sample(j).x() : d_selected_curve->sample(j).y()) + d, f, prec));
			j++;
			}
		}
		d_app->updateCurves(tab, col_name);
		d_app->modifiedProject();
		d_graph->setActiveTool(NULL);// attention: I'm now deleted. Maybe there is a cleaner solution...*/
    }
}
