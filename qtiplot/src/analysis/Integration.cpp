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

#include <QApplication>
#include <QDateTime>
#include <QLocale>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_integration.h>
#include "GslRAII.h"
#include <vector>
#include <algorithm>

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

Integration::Integration(ApplicationWindow *parent, PlotCurve *c)
: Filter(parent, c)
{
	init();
	setDataFromCurve(c);
}

Integration::Integration(ApplicationWindow *parent, PlotCurve *c, double start, double end)
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
	if (d_n <= 0)
		return 0.0;
	double sum = 0.0;
	std::vector<double> result(d_n);
	int size = d_n - 1;
	for(int i=0; i < size; i++){
		int j = i + 1;
		result[i] = sum;
		sum += 0.5*(d_y[j] + d_y[i])*(d_x[j] - d_x[i]);
	}

	result[size] = sum;
	d_points = d_n;
	addResultCurve(d_x, result.data());
	return sum;
}

namespace {
struct EvalContext {
	MyParser parser;
	double x{0.0};
	Integration *integration{nullptr};
};

double evalFunction(double x, void *params)
{
	EvalContext *ctx = static_cast<EvalContext *>(params);
	if (!ctx || !ctx->integration || ctx->integration->error())
		return 0.0;

	ctx->x = x;
	double result = 0.0;
	try {
		result = ctx->parser.Eval();
	} catch (mu::ParserError &e){
		QApplication::restoreOverrideCursor();
		ctx->integration->reportError("QtiPlot - Input error", QString::fromStdWString(e.GetMsg()));
	}

	return result;
}
} // namespace

double Integration::gslIntegration()
{
	if (d_init_err)
		return 0.0;

	GslRAII::UniqueIntegrationWorkspace w(gsl_integration_workspace_alloc(d_workspace_size));
	if (!w) {
		memoryErrorMessage();
		return 0.0;
	}

	EvalContext ctx;
	ctx.integration = this;
	try {
		ctx.parser.DefineVar(d_variable.toStdWString(), &ctx.x);
		ctx.parser.SetExpr(d_formula.toStdWString());
	} catch (mu::ParserError &e) {
		reportError("QtiPlot - Input error", QString::fromStdWString(e.GetMsg()));
		return 0.0;
	}

	gsl_function F;
	F.function = &evalFunction;
	F.params = &ctx;

	gsl_integration_qags (&F, d_from, d_to, 0, d_tolerance, d_workspace_size, w.get(), &d_area, &d_error);

	return d_area;
}

QString Integration::logInfo()
{
	if (d_init_err)
		return QString();

	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(parent());
    QLocale locale = app ? app->locale() : QLocale();
    int prec = app ? app->d_decimal_digits : 6;

	QString logInfo = "[" + QDateTime::currentDateTime().toString(Qt::TextDate);
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

		// Find maximum value of data set
		int maxID = 0;
		double maxVal = (d_n > 0) ? fabs(d_y[0]) : 0.0;
		for(int i=1; i < d_n; i++) {
			double v = fabs(d_y[i]);
			if (v > maxVal) {
				maxVal = v;
				maxID = i;
			}
		}

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
		ApplicationWindow *app = qobject_cast<ApplicationWindow *>(parent());
		if (app){
			color.setAlphaF(0.01*app->defaultCurveAlpha);
			brushStyle = PatternBox::brushStyle(app->defaultCurveBrush);
		}
		c->setBrush(QBrush(color, brushStyle));
	}
	d_output_graph->replot();
}
