/***************************************************************************
    File                 : PluginFit.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Plugin Fit class

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
#include "PluginFit.h"

#include <QLibrary>

PluginFit::PluginFit(ApplicationWindow *parent, Graph *g)
: Fit(parent, g)
{
	init();
}

PluginFit::PluginFit(ApplicationWindow *parent, PlotCurve *c)
: Fit(parent, c)
{
	init();
	setDataFromCurve(c);
}

PluginFit::PluginFit(ApplicationWindow *parent, PlotCurve *c, double start, double end)
: Fit(parent, c)
{
	init();
	setDataFromCurve(c, start, end);
}

PluginFit::PluginFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle)
: Fit(parent, g)
{
	init();
	setDataFromCurve(curveTitle);
}

PluginFit::PluginFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end)
: Fit(parent, g)
{
	init();
	setDataFromCurve(curveTitle, start, end);
}

PluginFit::PluginFit(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int startRow, int endRow)
: Fit(parent, t)
{
	init();
	setDataFromTable(t, xCol, yCol, startRow, endRow);
}

void PluginFit::init()
{
	d_explanation = tr("Plugin Fit");
    d_fit_type = Plugin;
}

bool PluginFit::load(const QString& pluginName)
{
	if (!QFile::exists (pluginName)){
		reportError(tr("QtiPlot - File not found"),
				tr("Plugin file: <p><b> %1 </b> <p>not found. Operation aborted!").arg(pluginName));
		return false;
	}

	d_library.setFileName(pluginName);
	if (!d_library.load())
		return false;

	d_fsimplex = (fit_function_simplex) d_library.resolve( "function_d" );
	if (!d_fsimplex)
		return false;

	d_f = (fit_function) d_library.resolve( "function_f" );
	if (!d_f)
		return false;

	d_df = (fit_function_df) d_library.resolve( "function_df" );
	if (!d_df)
		return false;

	d_fdf = (fit_function_fdf) d_library.resolve( "function_fdf" );
	if (!d_fdf)
		return false;

	f_eval = (fitFunctionEval) d_library.resolve("function_eval");
	if (!f_eval)
		return false;

	typedef char* (*fitFunc)();
	fitFunc fitFunction = (fitFunc) d_library.resolve("parameters");
	if (fitFunction){
		d_param_names = QString(fitFunction()).split(",", Qt::SkipEmptyParts);
		d_p = (int)d_param_names.count();
        initWorkspace(d_p);
	} else
		return false;

	fitFunc fitExplain = (fitFunc) d_library.resolve("explanations");
	if (fitExplain)
		d_param_explain = QString(fitExplain()).split(",", Qt::SkipEmptyParts);
	else
		for (int i=0; i<d_p; i++)
			d_param_explain << "";

	fitFunction = (fitFunc) d_library.resolve( "name" );
	setObjectName(QString(fitFunction()));

	fitFunction = (fitFunc) d_library.resolve( "function" );
	if (fitFunction)
		d_formula = QString(fitFunction());
	else
		return false;

	return true;
}

void PluginFit::calculateFitCurveData(double *X, double *Y)
{
	if (d_gen_function) {
		double X0 = d_x[0];
		double step = (d_x[d_n-1]-X0)/(d_points-1);
		for (int i=0; i<d_points; i++){
		    double x = X0+i*step;
			X[i] = x;
			Y[i]= f_eval(x, d_results);
		}
	} else {
		for (int i=0; i<d_points; i++) {
		    double x = d_x[i];
			X[i] = x;
			Y[i]= f_eval(x, d_results);
		}
	}
}
