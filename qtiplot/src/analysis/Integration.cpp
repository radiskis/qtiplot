/***************************************************************************
    File                 : Integration.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2007 - 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical integration of data sets

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
#include "Integration.h"
#include <MyParser.h>
#include <MultiLayer.h>
#include <FunctionCurve.h>
#include <LegendWidget.h>
#include <PatternBox.h>

#include <QMessageBox>
#include <QApplication>
#include <QDateTime>
#include <QLocale>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_integration.h>

Integration::Integration(const QString& formula, const QString& var, ApplicationWindow *parent, Graph *g, double start, double end)
: Filter(parent, g),
d_formula(formula),
d_variable(var)
{
	d_init_err = false;
	d_n = 0;
	d_from = start;
	d_to = end;
	if (d_to == d_from)
		d_init_err = true;

	setObjectName(tr("Integration"));
	d_integrand = AnalyticalFunction;
	d_workspace_size = 1000;
    d_sort_data = false;
}

Integration::Integration(ApplicationWindow *parent, Graph *g)
: Filter(parent, g)
{
	init();
}

Integration::Integration(ApplicationWindow *parent, QwtPlotCurve *c)
: Filter(parent, c)
{
	init();
	setDataFromCurve(c);
}

Integration::Integration(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end)
: Filter(parent, c)
{
	init();
	setDataFromCurve(c, start, end);
}

Integration::Integration(ApplicationWindow *parent, Graph *g, const QString& curveTitle)
: Filter(parent, g)
{
	init();
	setDataFromCurve(curveTitle);
}

Integration::Integration(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end)
: Filter(parent, g)
{
	init();
	setDataFromCurve(curveTitle, start, end);
}

Integration::Integration(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int start, int end, bool sort)
: Filter(parent, t)
{
	init();
	d_sort_data = sort;
	setDataFromTable(t, xCol, yCol, start, end);
}

void Integration::init()
{
	setObjectName(tr("Integration"));
	d_integrand = DataSet;
	d_sort_data = true;
}

double Integration::trapez()
{
	double sum = 0.0;
	double *result = (double *)malloc(d_n*sizeof(double));
	int size = d_n - 1;
	for(int i=0; i < size; i++){
		int j = i + 1;
		if (result)
            result[i] = sum;
		sum += 0.5*(d_y[j] + d_y[i])*(d_x[j] - d_x[i]);
	}

    if (result){
        result[size] = sum;
        d_points = d_n;
        addResultCurve(d_x, result);
        free(result);
    }
    return sum;
}

double evalFunction(double x, void *params)
{
	if (((Integration *)params)->error())
		return 0.0;

	double result = 0.0;
	QString var = ((Integration *)params)->variable();
	QString formula = ((Integration *)params)->formula();

	MyParser parser;
	parser.DefineVar(var.ascii(), &x);
	parser.SetExpr(formula.ascii());

	try {
		result = parser.Eval();
	} catch (mu::ParserError &e){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(0, "QtiPlot - Input error", QString::fromStdString(e.GetMsg()));
		((Integration *)params)->setError();
	}

	return result;
}

double Integration::gslIntegration()
{
	if (d_init_err)
		return 0.0;

	gsl_integration_workspace * w = gsl_integration_workspace_alloc (d_workspace_size);

	gsl_function F;
	F.function = &evalFunction;
	F.params = this;

	gsl_integration_qags (&F, d_from, d_to, 0, d_tolerance, d_workspace_size, w, &d_area, &d_error);

	gsl_integration_workspace_free (w);
	return d_area;
}

QString Integration::logInfo()
{
	if (d_init_err)
		return QString();

	ApplicationWindow *app = (ApplicationWindow *)parent();
    QLocale locale = app->locale();
    int prec = app->d_decimal_digits;

	QString logInfo = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate);
	if (d_integrand == AnalyticalFunction){
		logInfo += "\n" + tr("Numerical integration of") + " f(" + d_variable + ") = " + d_formula + "\n";
		logInfo += tr("From") + " x = " + locale.toString(d_from, 'g', prec) + " ";
		logInfo += tr("to") + " x = " + locale.toString(d_to, 'g', prec) + "\n";
		logInfo += tr("Tolerance") + " = " + locale.toString(d_tolerance, 'g', prec) + "\n";
		logInfo += tr("Area") + " = " + locale.toString(gslIntegration(), 'g', prec) + "\n";
		logInfo += tr("Error") + " = " + locale.toString(d_error, 'g', prec);
	} else if (d_integrand == DataSet){
		if (d_graph)
			logInfo += tr("\tPlot")+ ": ''" + d_graph->multiLayer()->objectName() + "'']\n";
		else
			logInfo += "\n";
		QString dataSet;
		if (d_curve)
			dataSet = d_curve->title().text();
		else
			dataSet = d_y_col_name;
		logInfo += "\n" + tr("Numerical integration of") + ": " + dataSet + " ";
		logInfo += tr("using the Trapezoidal Rule") + "\n";
		logInfo += tr("Points") + ": " + QString::number(d_n) + " " + tr("from") + " x = " + locale.toString(d_from, 'g', prec) + " ";
    	logInfo += tr("to") + " x = " + locale.toString(d_to, 'g', prec) + "\n";

		// use GSL to find maximum value of data set
		gsl_vector *aux = gsl_vector_alloc(d_n);
		for(int i=0; i < d_n; i++)
			gsl_vector_set (aux, i, fabs(d_y[i]));
		int maxID = gsl_vector_max_index (aux);
		gsl_vector_free(aux);

    	logInfo += tr("Peak at") + " x = " + locale.toString(d_x[maxID], 'g', prec)+"\t";
		logInfo += "y = " + locale.toString(d_y[maxID], 'g', prec)+"\n";
		d_area = trapez();
		logInfo += tr("Area") + " = " + locale.toString(d_area, 'g', prec);
	}

	logInfo += "\n-------------------------------------------------------------\n";
    return logInfo;
}

void Integration::output()
{
	if (d_integrand != AnalyticalFunction || d_init_err || !d_graphics_display)
		return;

	FunctionCurve *c = d_output_graph->addFunction(QStringList(d_formula), d_from, d_to, d_points, d_variable, FunctionCurve::Normal);
	if (c){
		QColor color = c->pen().color();
		Qt::BrushStyle brushStyle = Qt::BDiagPattern;
		ApplicationWindow *app = (ApplicationWindow *)parent();
		if (app){
			color.setAlphaF(0.01*app->defaultCurveAlpha);
			brushStyle = PatternBox::brushStyle(app->defaultCurveBrush);
		}
		c->setBrush(QBrush(color, brushStyle));
	}
	d_output_graph->replot();
}
