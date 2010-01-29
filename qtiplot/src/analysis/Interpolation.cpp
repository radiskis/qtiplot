/***************************************************************************
    File                 : Interpolation.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical interpolation of data sets

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
#include "Interpolation.h"

#include <QMessageBox>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>

Interpolation::Interpolation(ApplicationWindow *parent, QwtPlotCurve *c, int m)
: Filter(parent, c)
{
	init(m);
	setDataFromCurve(c);
}

Interpolation::Interpolation(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end, int m)
: Filter(parent, c)
{
	init(m);
	setDataFromCurve(c, start, end);
}

Interpolation::Interpolation(ApplicationWindow *parent, Graph *g, const QString& curveTitle, int m)
: Filter(parent, g)
{
	init(m);
	setDataFromCurve(curveTitle);
}

Interpolation::Interpolation(ApplicationWindow *parent, Graph *g, const QString& curveTitle,
                             double start, double end, int m)
: Filter(parent, g)
{
	init(m);
	setDataFromCurve(curveTitle, start, end);
}

Interpolation::Interpolation(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int start, int end, int m)
: Filter(parent, t)
{
	init(m);
	setDataFromTable(t, xCol, yCol, start, end);
}

void Interpolation::init(int m)
{
    if (m < 0 || m > 2){
        QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
        tr("Unknown interpolation method. Valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
        d_init_err = true;
        return;
    }

	d_method = m;
	const gsl_interp_type *method = NULL;
	switch(d_method)
	{
		case 0:
			method = gsl_interp_linear;
			setObjectName(tr("Linear") + tr("Int"));
			d_explanation = tr("Linear") + " " + tr("Interpolation");
			break;
		case 1:
			method = gsl_interp_cspline;
			setObjectName(tr("Cubic") + tr("Int"));
			d_explanation = tr("Cubic") + " " + tr("Interpolation");
			break;
		case 2:
			method = gsl_interp_akima;
			setObjectName(tr("Akima") + tr("Int"));
			d_explanation = tr("Akima") + " " + tr("Interpolation");
			break;
	}
    d_sort_data = true;
	d_min_points = d_method + 3; //Guess a min number of points
	//Get the exact number of min points from GSL
	//TODO: add a feature request to GSL developers so that we could use:
	//d_min_points = gsl_interp_min_size(gsl_interp_type *method);
	//It makes more sense than creating a dummy gsl_interp object and get the info from it!
	gsl_interp *interp = gsl_interp_alloc(method, d_min_points);
	d_min_points = gsl_interp_min_size(interp);
	gsl_interp_free (interp);
}


void Interpolation::setMethod(int m)
{
	if (m < 0 || m > 2){
    	QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Error"),
    	tr("Unknown interpolation method, valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
    	d_init_err = true;
    	return;
    }
	int min_points = m + 3;
	const gsl_interp_type *method = NULL;
	switch(m){
		case 0:
			method = gsl_interp_linear;
			break;
		case 1:
			method = gsl_interp_cspline;
			break;
		case 2:
			method = gsl_interp_akima;
			break;
	}
	gsl_interp *interp = gsl_interp_alloc(method, min_points);
	min_points = gsl_interp_min_size(interp);
	gsl_interp_free (interp);

	if (d_n < min_points){
    	QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
    	tr("You need at least %1 points in order to perform this operation!").arg(min_points));
    	d_init_err = true;
    	return;
	}

    d_method = m;
    d_min_points = min_points;

	switch(d_method)
	{
		case 0:
			setObjectName(tr("Linear") + tr("Int"));
			d_explanation = tr("Linear") + " " + tr("Interpolation");
			break;
		case 1:
			setObjectName(tr("Cubic") + tr("Int"));
			d_explanation = tr("Cubic") + " " + tr("Interpolation");
			break;
		case 2:
			setObjectName(tr("Akima") + tr("Int"));
			d_explanation = tr("Akima") + " " + tr("Interpolation");
			break;
	}
}

void Interpolation::calculateOutputData(double *x, double *y)
{
	gsl_interp_accel *acc = gsl_interp_accel_alloc ();
	const gsl_interp_type *method = NULL;
	switch(d_method)
	{
		case 0:
			method = gsl_interp_linear;
			break;
		case 1:
			method = gsl_interp_cspline;
			break;
		case 2:
			method = gsl_interp_akima;
			break;
	}

	gsl_spline *interp = gsl_spline_alloc (method, d_n);
	gsl_spline_init (interp, d_x, d_y, d_n);

    double step = (d_to - d_from)/(double)(d_points - 1);
    for (int j = 0; j < d_points; j++){
	   x[j] = d_from + j*step;
	   y[j] = gsl_spline_eval (interp, x[j], acc);
	}

	gsl_spline_free (interp);
	gsl_interp_accel_free (acc);
}

int Interpolation::sortedCurveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
	if (!c || c->rtti() != QwtPlotItem::Rtti_PlotCurve)
		return 0;

	int n = c->dataSize();
	double *xtemp = (double *)malloc(n*sizeof(double));
	if (!xtemp)
		memoryErrorMessage();

	double *ytemp = (double *)malloc(n*sizeof(double));
	if (!ytemp){
		free(xtemp);
		memoryErrorMessage();
	}

	if (c->curveType() == QwtPlotCurve::Yfx){
		for (int i = 0; i < n; i++){
			xtemp[i] = c->x(i);
			ytemp[i] = c->y(i);
		}
	} else {
		for (int i = 0; i < n; i++){
			xtemp[i] = c->y(i);
			ytemp[i] = c->x(i);
		}
	}

	size_t *p = (size_t *)malloc(n*sizeof(size_t));
	if (!p){
		free(xtemp); free(ytemp);
		memoryErrorMessage();
	}
	gsl_sort_index(p, xtemp, 1, n);

	double *xtemp2 = (double *)malloc(n*sizeof(double));
	if (!xtemp2){
		free(xtemp); free(ytemp); free(p);
		memoryErrorMessage();
	}

	double *ytemp2 = (double *)malloc(n*sizeof(double));
	if (!ytemp2){
		free(xtemp); free(ytemp); free(p); free(xtemp2);
		memoryErrorMessage();
	}

	for (int i = 0; i < n; i++){
		xtemp2[i] = xtemp[p[i]];
		ytemp2[i] = ytemp[p[i]];
	}
	free(xtemp);
	free(ytemp);
	free(p);

	int i_start = 0, i_end = n;
	for (int i = 0; i < i_end; i++)
		if (xtemp2[i] >= start){
			i_start = i;
			break;
		}
	for (int i = i_end - 1; i >= 0; i--)
		if (xtemp2[i] <= end){
			i_end = i;
			break;
		}

	n = i_end - i_start + 1;
	if (n > c->dataSize())
		n = c->dataSize();

	(*x) = (double *)malloc(n*sizeof(double));
	if (!x){
		free(xtemp2); free(ytemp2);
		memoryErrorMessage();
	}

	(*y) = (double *)malloc(n*sizeof(double));
	if (!y){
		free(xtemp2); free(ytemp2); free(x);
		memoryErrorMessage();
	}

	int j = 0;
	for (int i = i_start; i <= i_end; i++){
		(*x)[j] = xtemp2[i];
		(*y)[j] = ytemp2[i];
		j++;
	}
	free(xtemp2);
	free(ytemp2);

	double pr_x = (*x)[0];
	for (int i = 1; i < n; i++){
		double xval = (*x)[i];
		if (xval <= pr_x)
			return -1;//x values must be monotonically increasing in GSL interpolation routines
		pr_x = xval;
	}
	return n;
}
