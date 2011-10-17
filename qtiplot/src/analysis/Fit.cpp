/***************************************************************************
    File                 : Fit.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Fit base class

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
#include "Fit.h"
#include "FitModelHandler.h"
#include "fit_gsl.h"
#include <Table.h>
#include <Matrix.h>
#include <ErrorBarsCurve.h>
#include <FunctionCurve.h>
#include <MultiLayer.h>
#include <ColorBox.h>

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_cdf.h>

#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QLocale>
#include <QTextStream>

Fit::Fit( ApplicationWindow *parent, QwtPlotCurve *c)
: Filter( parent, c)
{
	init();
}

Fit::Fit( ApplicationWindow *parent, Graph *g, const QString& name)
: Filter( parent, g, name)
{
	init();
}

Fit::Fit( ApplicationWindow *parent, Table *t, const QString& name)
: Filter( parent, t, name)
{
	init();
}

void Fit::init()
{
	d_p = 0;
	d_n = 0;
	d_x = 0;
	d_y = 0;
	d_w = 0;
	d_curveColor = Qt::red;
	d_solver = ScaledLevenbergMarquardt;
	d_tolerance = 1e-4;
	d_gen_function = true;
	d_points = 100;
	d_max_iterations = 1000;
	d_curve = 0;
	d_formula = QString::null;
	d_result_formula = QString::null;
	d_explanation = QString::null;
	d_weighting = NoWeighting;
	weighting_dataset = QString::null;
	is_non_linear = true;
	d_results = 0;
	d_errors = 0;
	d_residuals = NULL;
	d_init_err = false;
	chi_2 = -1;
	d_rss = 0.0;
	d_adjusted_r_square = GSL_NAN;
	d_scale_errors = false;
	d_sort_data = false;
	d_prec = (((ApplicationWindow *)parent())->fit_output_precision);
	d_param_table = 0;
	d_cov_matrix = 0;
	covar = 0;
	d_param_init = 0;
	d_fit_type = BuiltIn;
	d_param_range_left = 0;
	d_param_range_right = 0;
}

gsl_multifit_fdfsolver * Fit::fitGSL(gsl_multifit_function_fdf f, int &iterations, int &status)
{
	const gsl_multifit_fdfsolver_type *T;
	if (d_solver)
		T = gsl_multifit_fdfsolver_lmder;
	else
		T = gsl_multifit_fdfsolver_lmsder;

	gsl_set_error_handler_off();

	gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc (T, d_n, d_p);
	status = gsl_multifit_fdfsolver_set (s, &f, d_param_init);

	size_t iter = 0;
	bool inRange = true;
	for (int i=0; i<d_p; i++){
		double p = gsl_vector_get(d_param_init, i);
		d_results[i] = p;
		if (p < d_param_range_left[i] || p > d_param_range_right[i]){
			inRange = false;
			break;
		}
	}

	if (status){
	    gsl_multifit_covar (s->J, 0.0, covar);
	    iterations = 0;
	    return s;
	}

	do{
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		if (status)
			break;

		for (int i=0; i<d_p; i++){
			double p = gsl_vector_get(s->x, i);
			if (p < d_param_range_left[i] || p > d_param_range_right[i]){
				inRange = false;
				break;
			}
		}
		if (!inRange)
			break;

		for (int i = 0; i < d_p; i++)
			d_results[i] = gsl_vector_get(s->x, i);

		status = gsl_multifit_test_delta (s->dx, s->x, d_tolerance, d_tolerance);
	} while (inRange && status == GSL_CONTINUE && (int)iter < d_max_iterations);

	gsl_multifit_covar (s->J, 0.0, covar);

	iterations = iter;
	return s;
}

gsl_multimin_fminimizer * Fit::fitSimplex(gsl_multimin_function f, int &iterations, int &status)
{
	const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;

	//size of the simplex
	gsl_vector *ss;
	//initial vertex size vector
	ss = gsl_vector_alloc (f.n);
	//set all step sizes to 1 can be increased to converge faster
	gsl_vector_set_all (ss, 10.0);

	gsl_set_error_handler_off();

	gsl_multimin_fminimizer *s_min = gsl_multimin_fminimizer_alloc (T, f.n);
	status = gsl_multimin_fminimizer_set (s_min, &f, d_param_init, ss);

	double size;
	size_t iter = 0;
	bool inRange = true;
	for (int i=0; i<d_p; i++){
		double p = gsl_vector_get(d_param_init, i);
		d_results[i] = p;
		if (p < d_param_range_left[i] || p > d_param_range_right[i]){
			inRange = false;
			break;
		}
	}

	if (status) {
	    iterations = 0;
	    gsl_vector_free(ss);
	    return s_min;
	}

	do{
		iter++;
		status = gsl_multimin_fminimizer_iterate (s_min);

		if (status)
			break;

        for (int i=0; i<d_p; i++){
			double p = gsl_vector_get(s_min->x, i);
			if (p < d_param_range_left[i] || p > d_param_range_right[i]){
				inRange = false;
				break;
			}
		}
		if (!inRange)
			break;

		for (int i=0; i<d_p; i++)
			d_results[i] = gsl_vector_get(s_min->x, i);

		size = gsl_multimin_fminimizer_size (s_min);
		status = gsl_multimin_test_size (size, d_tolerance);
	}
	while (inRange && status == GSL_CONTINUE && (int)iter < d_max_iterations);

	iterations = iter;
	gsl_vector_free(ss);
	return s_min;
}

bool Fit::setDataFromTable(Table *t, const QString& xColName, const QString& yColName, int from, int to, bool sort)
{
	if (Filter::setDataFromTable(t, xColName, yColName, from, to, sort)){
    	if (d_w)
			free(d_w);

    	d_w = (double *)malloc(d_n*sizeof(double));
        if (!d_w){
            memoryErrorMessage();
            return false;
        }

    	for (int i = 0; i < d_n; i++)//initialize the weighting data to 1.0
       		d_w[i] = 1.0;
		return true;
	} else
		return false;
}

void Fit::setDataCurve(QwtPlotCurve *curve, double start, double end)
{
    Filter::setDataCurve(curve, start, end);

    if (!d_w){
        d_w = (double *)malloc(d_n*sizeof(double));
        if (!d_w){
            memoryErrorMessage();
            return;
        }
	}

    if (d_graph && d_curve && ((PlotCurve *)d_curve)->type() != Graph::Function)
    {
		QList<ErrorBarsCurve *> lst = ((DataCurve *)d_curve)->errorBarsList();
		foreach (ErrorBarsCurve *er, lst){
            if (!er->xErrors()){
                d_weighting = Instrumental;
                for (int i=0; i<d_n; i++){
					double e = er->errorValue(i);
					d_w[i] = 1.0/(e*e);
				}
                weighting_dataset = er->title().text();
                return;
            }
        }
    }
	// if no error bars initialize the weighting data to 1.0
    for (int i=0; i<d_n; i++)
        d_w[i] = 1.0;
}

void Fit::setInitialGuesses(double *x_init)
{
	for (int i = 0; i < d_p; i++)
		gsl_vector_set(d_param_init, i, x_init[i]);
}

void Fit::generateFunction(bool yes, int points)
{
	d_gen_function = yes;
	if (d_gen_function)
		d_points = points;
}

QString Fit::logFitInfo(int iterations, int status)
{
	QString dataSet;
	if (d_curve)
		dataSet = d_curve->title().text();
	else
		dataSet = d_y_col_name;

	QDateTime dt = QDateTime::currentDateTime ();
	QString info = "[" + dt.toString(Qt::LocalDate)+ "\t" + tr("Plot")+ ": ";
	if (!d_graphics_display)
		info += tr("graphics display disabled") + "]\n";
	else if (d_output_graph)
		info += "''" + d_output_graph->multiLayer()->objectName() + "'']\n";

	info += d_explanation + " " + tr("of dataset") + ": " + dataSet;
	if (!d_formula.isEmpty())
		info +=", " + tr("using function") + ": " + d_formula + "\n";
	else
		info +="\n";

	info += tr("Weighting Method") + ": ";
	switch(d_weighting){
		case NoWeighting:
			info += tr("No weighting");
			break;
		case Instrumental:
			info += tr("Instrumental") + ", " + tr("using error bars dataset") + ": " + weighting_dataset;
			break;
		case Statistical:
			info += tr("Statistical");
			break;
		case Dataset:
			info += tr("Arbitrary Dataset") + ": " + weighting_dataset;
			break;
		case Direct:
			info += tr("Direct Weighting using Dataset") + ": " + weighting_dataset;
			break;
	}
	info +="\n";

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	if (is_non_linear){
		if (d_solver == NelderMeadSimplex)
			info+=tr("Nelder-Mead Simplex");
		else if (d_solver == UnscaledLevenbergMarquardt)
			info+=tr("Unscaled Levenberg-Marquardt");
		else
			info+=tr("Scaled Levenberg-Marquardt");

		info+=tr(" algorithm with tolerance = ") + locale.toString(d_tolerance)+"\n";
	}

	info += tr("From x")+" = "+locale.toString(d_x[0], 'e', d_prec)+" "+tr("to x")+" = "+locale.toString(d_x[d_n-1], 'e', d_prec)+"\n";
	int dof = d_n - d_p;
	double chi_2_dof = chi_2/dof;
	for (int i=0; i<d_p; i++){
		info += d_param_names[i];
		if (!d_param_explain[i].isEmpty())
            info += " (" + d_param_explain[i] + ")";
		info += " = " + locale.toString(d_results[i], 'e', d_prec) + " +/- ";
		if (d_scale_errors)
			info += locale.toString(sqrt(chi_2_dof*gsl_matrix_get(covar, i, i)), 'e', d_prec) + "\n";
		else
			info += locale.toString(sqrt(gsl_matrix_get(covar, i, i)), 'e', d_prec) + "\n";
	}
	info += "--------------------------------------------------------------------------------------\n";
	info += "Chi^2/doF = " + locale.toString(chi_2_dof, 'e', d_prec) + "\n";
	info += tr("R^2") + " = " + locale.toString(rSquare(), 'g', d_prec) + "\n";
	if (dof > 1)
		info += tr("Adjusted R^2") + " = " + locale.toString(d_adjusted_r_square, 'g', d_prec) + "\n";
	if (dof >= 1)
		info += tr("RMSE (Root Mean Squared Error)") + " = " + locale.toString(sqrt(d_rss/dof), 'g', d_prec) + "\n";
	info += tr("RSS (Residual Sum of Squares)") + " = " + locale.toString(d_rss, 'g', d_prec) + "\n";
	info += "---------------------------------------------------------------------------------------\n";
	if (is_non_linear){
		info += tr("Iterations")+ " = " + QString::number(iterations) + "\n";
		info += tr("Status") + " = " + gsl_strerror (status) + "\n";
		info +="---------------------------------------------------------------------------------------\n";
	}
	return info;
}

/*
* For details, see: http://www.mathworks.com/access/helpdesk_r13/help/toolbox/curvefit/ch_fitt9.html
*/
double Fit::rSquare()
{
	if (!d_residuals)
		d_residuals = new double[d_n];

	//double sst = gsl_stats_wtss_m (d_w, 1, d_y, 1, d_n, gsl_stats_mean (d_y, 1, d_n));
	double mean = gsl_stats_mean (d_y, 1, d_n);
	double sst = 0.0;
	d_rss = 0.0;
	for (int i = 0; i < d_n; i++){
		double w = d_w[i];
		double y = d_y[i];
		double dy = y - eval(d_results, d_x[i]);
		d_residuals[i] = dy;
		d_rss += w*dy*dy;

		dy = y - mean;
		sst += w*dy*dy;
	}
	d_adjusted_r_square = 1 - d_rss*(d_n - 1)/(sst*(d_n - d_p - 1));
	return 1 - d_rss/sst;
}

QString Fit::legendInfo()
{
	QString dataSet;
	if (d_curve)
		dataSet = d_curve->title().text();
	else
		dataSet = d_y_col_name;

	QString info = "<b>" + tr("Dataset") + "</b>: " + dataSet + "\n";
	info += "<b>" + tr("Function") + "</b>: " + d_formula + "\n";

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();

	double chi_2_dof = chi_2/(d_n - d_p);
	info += "<b>" + tr("Chi^2/doF") + "</b> = " + locale.toString(chi_2_dof, 'e', d_prec) + "\n";
	info += "<b>" + tr("R^2") + "</b> = " + locale.toString(rSquare(), 'g', d_prec) + "\n";

	for (int i = 0; i < d_p; i++){
		info += "<b>" + d_param_names[i] + "</b> = " + locale.toString(d_results[i], 'e', d_prec) + " +/- ";
		if (d_scale_errors)
			info += locale.toString(sqrt(chi_2_dof*gsl_matrix_get(covar, i, i)), 'e', d_prec);
		else
			info += locale.toString(sqrt(gsl_matrix_get(covar, i, i)), 'e', d_prec);

		if (i < d_p - 1)
			info += "\n";
	}
	return info;
}

bool Fit::setWeightingData(WeightingMethod w, const QString& colName)
{
	if (d_w)
		free(d_w);

	d_w = (double *)malloc(d_n*sizeof(double));
	if (!d_w){
		memoryErrorMessage();
		return false;
	}

	switch (w)
	{
		case NoWeighting:
			{
				weighting_dataset = QString::null;
				for (int i=0; i<d_n; i++)
					d_w[i] = 1.0;
			}
			break;
		case Instrumental:
			{
				if (!d_graph && d_table){
					QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Error"),
  	    				tr("You cannot use the instrumental weighting method."));
  	    			return false;
				}

				bool error = true;
				ErrorBarsCurve *er = 0;
				if (((PlotCurve *)d_curve)->type() != Graph::Function){
					QList<ErrorBarsCurve *> lst = ((DataCurve *)d_curve)->errorBarsList();
					foreach (ErrorBarsCurve *er, lst){
                    	if (!er->xErrors()){
                        	weighting_dataset = er->title().text();
                        	error = false;
                        	break;
                    	}
					}
                }
				if (error){
					QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Error"),
					tr("The curve %1 has no associated Y error bars. You cannot use instrumental weighting method.").arg(d_curve->title().text()));
					return false;
				}
				if (er){
					for (int j=0; j<d_n; j++){
						double e = er->errorValue(j);
						d_w[j] = 1.0/(e*e);
					}
				}
			}
			break;
		case Statistical:
			{
				if (d_graph && d_curve)
					weighting_dataset = d_curve->title().text();
				else if (d_table)
					weighting_dataset = d_y_col_name;

				for (int i=0; i<d_n; i++)
					d_w[i] = 1.0/d_y[i];
			}
			break;
		case Dataset:
			{//d_w are equal to the values of the arbitrary dataset
				if (colName.isEmpty())
					return false;

				Table* t = ((ApplicationWindow *)parent())->table(colName);
				if (!t)
					return false;

				if (t->numRows() < d_n){
  	            	QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Error"),
  	                tr("The column %1 has less points than the fitted data set. Please choose another column!").arg(colName));
  	                return false;
  	            }

				weighting_dataset = colName;

				int col = t->colIndex(colName);
				for (int i = 0; i < d_n; i++){
					double e = t->cell(i, col);
					d_w[i] = 1.0/(e*e);
				}
			}
			break;

		case Direct:
			{//d_w are equal to the values of the arbitrary dataset
				if (colName.isEmpty())
					return false;

				Table* t = ((ApplicationWindow *)parent())->table(colName);
				if (!t)
					return false;

				if (t->numRows() < d_n){
  	            	QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Error"),
  	                tr("The column %1 has less points than the fitted data set. Please choose another column!").arg(colName));
  	                return false;
  	            }

				weighting_dataset = colName;

				int col = t->colIndex(colName);
				for (int i = 0; i < d_n; i++)
					d_w[i] = t->cell(i, col);
			}
			break;
	}

	d_weighting = w;
	return true;
}

Table* Fit::parametersTable(const QString& tableName)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	d_param_table = app->table(tableName);
	if (!d_param_table || d_param_table->objectName() != tableName){
		d_param_table = app->newTable(app->generateUniqueName(tableName, false), d_p, 3);
		d_param_table->setHeader(QStringList() << tr("Parameter") << tr("Value") << tr ("Error"));
		d_param_table->setColumnType(0, Table::Text);
		d_param_table->setColPlotDesignation(0, Table::None);
		d_param_table->setColPlotDesignation(2, Table::yErr);
	}

	writeParametersToTable(d_param_table);

	d_param_table->showNormal();
	return d_param_table;
}

void Fit::writeParametersToTable(Table *t, bool append)
{
	if (!t)
		return;

	if (t->numCols() < 3)
		t->setNumCols(3);

	int rows = 0;
	if (append){
		rows = t->numRows();
		t->setNumRows(rows + d_p);
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();

	double chi_2_dof = chi_2/(d_n - d_p);

	for (int i=0; i<d_p; i++){
		int j = rows + i;
		t->setText(j, 0, d_param_names[i]);
		t->setText(j, 1, locale.toString(d_results[i], 'g', d_prec));
		if (d_scale_errors)
		    t->setText(j, 2, locale.toString(sqrt(chi_2_dof*gsl_matrix_get(covar,i,i)), 'g', d_prec));
		else
		    t->setText(j, 2, locale.toString(sqrt(gsl_matrix_get(covar, i, i)), 'g', d_prec));

	}

	for (int i=0; i<3; i++)
		t->table()->adjustColumn(i);
}

Matrix* Fit::covarianceMatrix(const QString& matrixName)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	d_cov_matrix = app->matrix(matrixName);
	if (!d_cov_matrix || d_cov_matrix->objectName() != matrixName)
		d_cov_matrix = app->newMatrix(app->generateUniqueName(matrixName, false), d_p, d_p);

	d_cov_matrix->setNumericPrecision(d_prec);
	for (int i = 0; i < d_p; i++){
		for (int j = 0; j < d_p; j++)
			d_cov_matrix->setCell(i, j, gsl_matrix_get(covar, i, j));
	}
	d_cov_matrix->resetView();
	d_cov_matrix->showNormal();
	return d_cov_matrix;
}

double *Fit::errors()
{
	if (!d_errors)
		d_errors = new double[d_p];

	double chi_2_dof = chi_2/(d_n - d_p);
	for (int i = 0; i < d_p; i++){
		if (d_scale_errors)
			d_errors[i] = sqrt(chi_2_dof*gsl_matrix_get(covar,i,i));
		else
			d_errors[i] = sqrt(gsl_matrix_get(covar,i,i));
	}
	return d_errors;
}

double* Fit::residuals()
{
	if (!d_residuals){
		if (!d_n || error())
			return NULL;

		d_residuals = new double[d_n];
		for (int i=0; i<d_n; i++)
			d_residuals[i] = d_y[i] - eval(d_results, d_x[i]);
	}
	return d_residuals;
}

QwtPlotCurve* Fit::showResiduals()
{
	if (!d_residuals){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("Please perform a fit first!"));
		return NULL;
	}

	if (!d_graphics_display)
		return NULL;

	ApplicationWindow *app = (ApplicationWindow *)parent();
	Table *outputTable = app->newTable(d_n, 2, app->generateUniqueName(tr("FitResiduals"), true), tr("Residuals of %1").arg(d_explanation));
	if (!outputTable)
		return NULL;

	outputTable->setColName(1, tr("residue"));
	for (int i = 0; i < d_n; i++){
		outputTable->setCell(i, 0, d_x[i]);
		outputTable->setCell(i, 1, d_residuals[i]);
	}
	for (int i = 0; i < outputTable->numCols(); i++)
		outputTable->table()->adjustColumn(i);
	app->hideWindow(outputTable);

	if (!d_output_graph)
		createOutputGraph();

	QString tableName = outputTable->objectName();
	DataCurve *c = new DataCurve(outputTable, tableName + "_1", tableName + "_residue");
	c->setData(d_x, d_residuals, d_n);
	c->setPen(QPen(ColorBox::color(ColorBox::colorIndex(d_curveColor) + 1), 1));

	d_output_graph->insertPlotItem(c, Graph::Line);
    d_output_graph->updatePlot();
	return (QwtPlotCurve*)c;
}

void Fit::showConfidenceLimits(double confidenceLevel)
{
	if (!d_graphics_display)
		return;

	if (!d_n){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("Please perform a fit first!"));
		return;
	}

	int points = d_n;
	double *X = NULL;
	if (d_gen_function){
		X = (double *)malloc(d_points*sizeof(double));
		if (!X){
			QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
			tr("Not enough memory!"));
			return;
		}
		points = d_points;
		double X0 = d_from;
		double step = fabs(d_from - d_to)/(points - 1);
		for (int i = 0; i < points; i++)
			X[i] = X0 + i*step;
	} else
		X = d_x;

	double *lcl = (double *)malloc(d_points*sizeof(double));
	if (!lcl){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
		tr("Not enough memory!"));
		return;
	}
	double *ucl = (double *)malloc(d_points*sizeof(double));
	if (!ucl){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
		tr("Not enough memory!"));
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	Table *outputTable = app->newTable(points, 3, app->generateUniqueName(tr("FitStats"), true), tr("Confidence Limits of %1").arg(d_explanation));
	if (!outputTable)
		return;

	outputTable->setColComment(0, tr("Independent Variable"));
	outputTable->setColName(1, tr("LCL"));
	outputTable->setColComment(1, tr("Lower %1 Confidence Limit").arg(confidenceLevel));
	outputTable->setColName(2, tr("UCL"));
	outputTable->setColComment(2, tr("Upper %1 Confidence Limit").arg(confidenceLevel));

	double t = gsl_cdf_tdist_Pinv(1 - 0.5*(1 - confidenceLevel), d_n - d_p);
	double x_mean = gsl_stats_mean(d_x, 1, d_n);
	double sxx = 0.0;//gsl_stats_tss (d_x, 1, d_n);
	for (int i = 0; i < d_n; i++){
		double dx = d_x[i] - x_mean;
		sxx += dx*dx;
	}
	double mse = d_rss/double(d_n - d_p);

	for (int i = 0; i < points; i++){
		double x = X[i];
		double dx = x - x_mean;
		double aux = t*sqrt(mse*(1.0/(double)d_n + dx*dx/sxx));

		outputTable->setCell(i, 0, x);
		double y = eval(d_results, x);
		double lowLimit = y - aux;
		outputTable->setCell(i, 1, lowLimit);
		lcl[i] = lowLimit;
		double upLimit = y + aux;
		outputTable->setCell(i, 2, upLimit);
		ucl[i] = upLimit;
	}

	for (int i = 0; i < outputTable->numCols(); i++)
		outputTable->table()->adjustColumn(i);
	app->hideWindow(outputTable);

	if (!d_output_graph)
		createOutputGraph();

	QString tableName = outputTable->objectName();
	DataCurve *c = new DataCurve(outputTable, tableName + "_1", tableName + "_LCL");
	c->setData(X, lcl, points);
	c->setPen(QPen(ColorBox::color(ColorBox::colorIndex(d_curveColor) + 2), 1));
	d_output_graph->insertPlotItem(c, Graph::Line);

	c = new DataCurve(outputTable, tableName + "_1", tableName + "_UCL");
	c->setData(X, ucl, points);
	c->setPen(QPen(ColorBox::color(ColorBox::colorIndex(d_curveColor) + 2), 1));
	d_output_graph->insertPlotItem(c, Graph::Line);

    d_output_graph->updatePlot();
	free (lcl);
	free (ucl);
	if (d_gen_function)
		free (X);
}

double Fit::lcl(int parIndex, double confidenceLevel)
{
	if (parIndex < 0 || parIndex >= d_p)
		return GSL_NAN;

	double t = gsl_cdf_tdist_Pinv(1 - 0.5*(1 - confidenceLevel), d_n - d_p);
	return d_results[parIndex] - t*sqrt(gsl_matrix_get(covar, parIndex, parIndex));
}

double Fit::ucl(int parIndex, double confidenceLevel)
{
	if (parIndex < 0 || parIndex >= d_p)
		return GSL_NAN;

	double t = gsl_cdf_tdist_Pinv(1 - 0.5*(1 - confidenceLevel), d_n - d_p);
	return d_results[parIndex] + t*sqrt(gsl_matrix_get(covar, parIndex, parIndex));
}

void Fit::showPredictionLimits(double confidenceLevel)
{
	if (!d_graphics_display)
		return;

	if (!d_n){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("Please perform a fit first!"));
		return;
	}

	int points = d_n;
	double *X = NULL;
	if (d_gen_function){
		X = (double *)malloc(d_points*sizeof(double));
		if (!X){
			QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
			tr("Not enough memory!"));
			return;
		}
		points = d_points;
		double X0 = d_from;
		double step = fabs(d_to - d_from)/(points - 1);
		for (int i = 0; i < points; i++)
			X[i] = X0 + i*step;
	} else
		X = d_x;

	double *lcl = (double *)malloc(d_points*sizeof(double));
	if (!lcl){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
		tr("Not enough memory!"));
		return;
	}
	double *ucl = (double *)malloc(d_points*sizeof(double));
	if (!ucl){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
		tr("Not enough memory!"));
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	Table *outputTable = app->newTable(points, 3, app->generateUniqueName(tr("FitStats"), true), tr("Prediction Limits of %1").arg(d_explanation));
	if (!outputTable)
		return;

	outputTable->setColComment(0, tr("Independent Variable"));
	outputTable->setColName(1, tr("LPL"));
	outputTable->setColComment(1, tr("Lower %1 Prediction Limit").arg(confidenceLevel));
	outputTable->setColName(2, tr("UPL"));
	outputTable->setColComment(2, tr("Upper %1 Prediction Limit").arg(confidenceLevel));

	double t = gsl_cdf_tdist_Pinv(1 - 0.5*(1 - confidenceLevel), d_n - d_p);
	double x_mean = gsl_stats_mean(d_x, 1, d_n);
	double sxx = 0.0;
	for (int i = 0; i < d_n; i++){
		double dx = d_x[i] - x_mean;
		sxx += dx*dx;
	}
	double mse = d_rss/double(d_n - d_p);
	for (int i = 0; i < points; i++){
		double x = X[i];
		double dx = x - x_mean;
		double aux = t*sqrt(mse*(1 + 1.0/(double)d_n + dx*dx/sxx));

		outputTable->setCell(i, 0, x);
		double y = eval(d_results, x);
		double lowLimit = y - aux;
		outputTable->setCell(i, 1, lowLimit);
		lcl[i] = lowLimit;
		double upLimit = y + aux;
		outputTable->setCell(i, 2, upLimit);
		ucl[i] = upLimit;
	}
	for (int i = 0; i < outputTable->numCols(); i++)
		outputTable->table()->adjustColumn(i);
	app->hideWindow(outputTable);

	if (!d_output_graph)
		createOutputGraph();

	QString tableName = outputTable->objectName();
	DataCurve *c = new DataCurve(outputTable, tableName + "_1", tableName + "_LPL");
	c->setData(X, lcl, points);
	c->setPen(QPen(ColorBox::color(ColorBox::colorIndex(d_curveColor) + 3), 1));
	d_output_graph->insertPlotItem(c, Graph::Line);

	c = new DataCurve(outputTable, tableName + "_1", tableName + "_UPL");
	c->setData(X, ucl, points);
	c->setPen(QPen(ColorBox::color(ColorBox::colorIndex(d_curveColor) + 3), 1));
	d_output_graph->insertPlotItem(c, Graph::Line);

    d_output_graph->updatePlot();
	free (lcl);
	free (ucl);
	if (d_gen_function)
		free (X);
}

void Fit::fit()
{
	if (!(d_graph || d_table) || d_init_err)
		return;

	if (!d_n){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("You didn't specify a valid data set for this fit operation. Operation aborted!"));
		return;
	}
	if (!d_p){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("There are no parameters specified for this fit operation. Operation aborted!"));
		return;
	}
	if (d_p > d_n){
  		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
  	    tr("You need at least %1 data points for this fit operation. Operation aborted!").arg(d_p));
  	    return;
  	}
	if (d_formula.isEmpty()){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("You must specify a valid fit function first. Operation aborted!"));
		return;
	}

	if (!removeDataSingularities())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	struct FitData d_data = {d_n, d_p, d_x, d_y, d_w, this};

	int status, iterations = d_max_iterations;
	if(d_solver == NelderMeadSimplex){
		gsl_multimin_function f;
		f.f = d_fsimplex;
		f.n = d_p;
		f.params = &d_data;
		gsl_multimin_fminimizer *s_min = fitSimplex(f, iterations, status);

		if (!status) {
		     // allocate memory and calculate covariance matrix based on residuals
		     gsl_matrix *J = gsl_matrix_alloc(d_n, d_p);
		     d_df(s_min->x,(void*)f.params, J);
		     gsl_multifit_covar (J, 0.0, covar);
		     chi_2 = s_min->fval;

		     // free previousely allocated memory
		     gsl_matrix_free (J);
		}
		gsl_multimin_fminimizer_free (s_min);
	} else {
		gsl_multifit_function_fdf f;
		f.f = d_f;
		f.df = d_df;
		f.fdf = d_fdf;
		f.n = d_n;
		f.p = d_p;
		f.params = &d_data;

		gsl_multifit_fdfsolver *s = fitGSL(f, iterations, status);

		chi_2 = pow(gsl_blas_dnrm2(s->f), 2.0);
		gsl_multifit_fdfsolver_free(s);
	}

	generateFitCurve();

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (app->writeFitResultsToLog)
		app->updateLog(logFitInfo(iterations, status));

	QApplication::restoreOverrideCursor();
}

void Fit::generateFitCurve()
{
	if (!d_gen_function)
		d_points = d_n;

	double *X = NULL, *Y = NULL;
	if (d_graphics_display && !d_gen_function){
		X = (double *)malloc(d_points*sizeof(double));
		if (!X){
			QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Memory Allocation Error"),
			tr("Could not allocate enough memory for the fit curves!"));
			return;
		}
		Y = (double *)malloc(d_points*sizeof(double));
		if (!Y){
			QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot  - Memory Allocation Error"),
			tr("Could not allocate enough memory for the fit curves!"));
			free(X);
			return;
		}
		calculateFitCurveData(X, Y);
	}

    customizeFitResults();

	if (d_graphics_display){
		if (!d_output_graph)
			createOutputGraph();

		if (d_gen_function){
			insertFitFunctionCurve(QString(objectName()) + tr("Fit"));
			if (d_update_output_graph)
				d_output_graph->replot();
		} else {
        	d_output_graph->addFitCurve(addResultCurve(X, Y));
			free(X);
			free(Y);
		}
	}
}

FunctionCurve * Fit::insertFitFunctionCurve(const QString& name, int penWidth, bool updateData)
{
	QString title = d_output_graph->generateFunctionName(name);
	FunctionCurve *c = new FunctionCurve(FunctionCurve::Normal, title);
	c->setPen(QPen(d_curveColor, penWidth));
	c->setRange(QMIN(d_from, d_to), QMAX(d_from, d_to));
	c->setFormula(d_formula);
	if (d_curve){
		c->setCurveType(d_curve->curveType());
		c->setAxis(d_curve->xAxis(), d_curve->yAxis());
	}

	for (int j = 0; j < d_p; j++)
		c->setConstant(d_param_names[j], d_results[j]);

	if (updateData)
		c->loadData(d_points);
	d_output_graph->insertPlotItem(c, Graph::Line);
	d_output_graph->addFitCurve(c);
	d_result_curve = c;
	return c;
}

bool Fit::save(const QString& fileName)
{
    QFile f(fileName);
	if ( !f.open( QIODevice::WriteOnly ) ){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(0, tr("QtiPlot") + " - " + tr("File Save Error"),
				tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fileName));
		return false;
	}

    QTextStream out( &f );
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE fit>\n"
         << "<fit version=\"1.0\">\n";

     out << "<model>" + objectName() + "</model>\n";
     out << "<type>" + QString::number(d_fit_type) + "</type>\n";

	 QString function = d_formula;
     out << "<function>" + function.replace("<", "&lt;").replace(">", "&gt;") + "</function>\n";

     QString indent = QString(4, ' ');
     for (int i=0; i< d_p; i++){
         out << "<parameter>\n";
         out << indent << "<name>" + d_param_names[i] + "</name>\n";
         out << indent << "<explanation>" + d_param_explain[i] + "</explanation>\n";
         out << indent << "<value>" + QString::number(gsl_vector_get(d_param_init, i), 'e', 13) + "</value>\n";
         out << "</parameter>\n";
     }
     out << "</fit>\n";
     d_file_name = fileName;
     return true;
}

bool Fit::load(const QString& fileName)
{
    FitModelHandler handler(this);
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(((ApplicationWindow *)parent()), tr("QtiPlot Fit Model"),
                              tr("Cannot read file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
        return false;
    }

    QXmlInputSource xmlInputSource(&file);
    if (reader.parse(xmlInputSource)){
        d_file_name = fileName;
        return true;
    }
    return false;
}

void Fit::setParameterRange(int parIndex, double left, double right)
{
	if (!d_param_range_left || !d_param_range_right || parIndex < 0 || parIndex >= d_p)
		return;

	d_param_range_left[parIndex] = left;
	d_param_range_right[parIndex] = right;
}

void Fit::initWorkspace(int par)
{
	d_min_points = par;
	d_param_init = gsl_vector_alloc(par);
	gsl_vector_set_all (d_param_init, 1.0);

	//gsl_set_error_handler_off();
	covar = gsl_matrix_alloc (par, par);
	/*if (!covar){
		memoryErrorMessage();
		return;
	}*/

	d_results = new double[par];
	d_param_range_left = new double[par];
	d_param_range_right = new double[par];
	for (int i = 0; i<par; i++){
		d_param_range_left[i] = -DBL_MAX;
		d_param_range_right[i] = DBL_MAX;
	}
}

void Fit::freeWorkspace()
{
	if (d_param_init){
		gsl_vector_free(d_param_init);
		d_param_init = NULL;
	}

	if (covar){
		gsl_matrix_free (covar);
		covar = NULL;
	}

	if (d_results){
		delete[] d_results;
		d_results = NULL;
	}

	if (d_errors){
		delete[] d_errors;
		d_errors = NULL;
	}

	if (d_param_range_left){
		delete[] d_param_range_left;
		d_param_range_left = NULL;
	}

	if (d_param_range_right){
		delete[] d_param_range_right;
		d_param_range_right = NULL;
	}
}

void Fit::freeMemory()
{
	Filter::freeMemory();
	if (d_w){
		free(d_w);
		d_w = NULL;
	}

    if (d_residuals) {
		delete[] d_residuals;
		d_residuals = NULL;
    }
}

Fit::~Fit()
{
	freeMemory();

	if (!d_p)
		return;

	freeWorkspace();
}
