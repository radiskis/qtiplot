/***************************************************************************
    File                 : Differentiation.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical differentiation of data sets

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
#include "Differentiation.h"
#include <MultiLayer.h>
#include <LegendWidget.h>

#include <QLocale>

Differentiation::Differentiation(ApplicationWindow *parent, Graph *g)
: Filter(parent, g)
{
	init();
}

Differentiation::Differentiation(ApplicationWindow *parent, QwtPlotCurve *c)
: Filter(parent, c)
{
	init();
	setDataFromCurve(c);
}

Differentiation::Differentiation(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end)
: Filter(parent, c)
{
	init();
	setDataFromCurve(c, start, end);
}

Differentiation::Differentiation(ApplicationWindow *parent, Graph *g, const QString& curveTitle)
: Filter(parent, g)
{
	init();
	setDataFromCurve(curveTitle);
}

Differentiation::Differentiation(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end)
: Filter(parent, g)
{
	init();
	setDataFromCurve(curveTitle, start, end);
}

Differentiation::Differentiation(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int start, int end)
: Filter(parent, t)
{
	init();
	setDataFromTable(t, xCol, yCol, start, end);
}

void Differentiation::init()
{
	setObjectName(tr("Derivative"));
    d_min_points = 4;
    d_sort_data = false;
}

void Differentiation::output()
{
    double *result = new double[d_n - 1];
	for (int i = 1; i < d_n - 1; i++){
		double xl = d_x[i - 1];
		double xc = d_x[i];
		double xr = d_x[i + 1];

		if (xr != xc && xl != xc)
			result[i] = 0.5*((d_y[i + 1] - d_y[i])/(xr - xc) + (d_y[i] - d_y[i - 1])/(xc - xl));
		else if (xr != xc)
			result[i] = (d_y[i + 1] - d_y[i])/(xr - xc);
		else if (xl != xc)
			result[i] = (d_y[i] - d_y[i - 1])/(xc - xl);
		else
			result[i] = result[i - 1];
	}

    ApplicationWindow *app = (ApplicationWindow *)parent();
    QLocale locale = app->locale();
    QString tableName = app->generateUniqueName(QString(objectName()));
    QString dataSet;
	if (d_curve)
		dataSet = d_curve->title().text();
	else
		dataSet = d_y_col_name;

    int prec = app->d_decimal_digits;
	int rows = d_n - 2;
	int col = 1;
	if (!d_result_table || d_result_table->numRows() < rows){
		d_result_table = app->newHiddenTable(tableName, tr("Derivative") + " " + tr("of", "Derivative of")  + " " + dataSet, rows, 2);
		for (int i = 1; i < d_n-1; i++) {
			int aux = i - 1;
			d_result_table->setText(aux, 0, locale.toString(d_x[i], 'g', prec));
			d_result_table->setText(aux, 1, locale.toString(result[i], 'g', prec));
		}
	} else {
		col = d_result_table->numCols();
		d_result_table->addCol();
		for (int i = 1; i < d_n-1; i++)
			d_result_table->setText(i - 1, col, locale.toString(result[i], 'g', prec));
	}

    delete[] result;

	if (d_graphics_display){
		if (!d_output_graph){
			createOutputGraph();
			d_output_graph->removeLegend();
		}

		d_output_graph->insertCurve(d_result_table, d_result_table->colLabel(col), 0);
		if (d_update_output_graph)
			d_output_graph->updatePlot();
	}
}
