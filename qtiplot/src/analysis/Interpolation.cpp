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

#include <gsl/gsl_sort.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>
#include <GslRAII.h>
#include <vector>

Interpolation::Interpolation(ApplicationWindow *parent, PlotCurve *c, int m)
: Filter(parent, c)
{
	init(m);
	setDataFromCurve(c);
}

Interpolation::Interpolation(ApplicationWindow *parent, PlotCurve *c, double start, double end, int m)
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
        reportError(tr("QtiPlot") + " - " + tr("Error"),
        tr("Unknown interpolation method. Valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
        return;
    }

	d_method = m;
	const gsl_interp_type *method = nullptr;
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
	d_min_points = method ? method->min_size : (d_method + 3);
}


void Interpolation::setMethod(int m)
{
	if (m < 0 || m > 2){
    	reportError(tr("QtiPlot - Error"),
    	tr("Unknown interpolation method, valid values are: 0 - Linear, 1 - Cubic, 2 - Akima."));
    	return;
    }
	const gsl_interp_type *method = nullptr;
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
	int min_points = method ? method->min_size : (m + 3);

	if (d_n < min_points){
    	reportError(tr("QtiPlot") + " - " + tr("Error"),
    	tr("You need at least %1 points in order to perform this operation!").arg(min_points));
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
	GslRAII::UniqueInterpAccel acc(gsl_interp_accel_alloc());
	const gsl_interp_type *method = nullptr;
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

	GslRAII::UniqueSpline interp(gsl_spline_alloc(method, d_n));
	if (!acc || !interp) {
		memoryErrorMessage();
		return;
	}
	gsl_spline_init(interp.get(), d_x, d_y, d_n);

    double step = (d_to - d_from)/(double)(d_points - 1);
    for (int j = 0; j < d_points; j++){
	   x[j] = d_from + j*step;
	   y[j] = gsl_spline_eval(interp.get(), x[j], acc.get());
	}
}

int Interpolation::sortedCurveData(PlotCurve *c, double start, double end, double **x, double **y)
{
	if (!c || c->rtti() != QwtPlotItem::Rtti_PlotCurve)
		return 0;

	int n = c->dataSize();
	if (n <= 0)
		return 0;

	std::vector<double> xtemp(n);
	std::vector<double> ytemp(n);

	if (c->curveType() == PlotCurve::Yfx){
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

	std::vector<size_t> p(n);
	gsl_sort_index(p.data(), xtemp.data(), 1, n);

	std::vector<double> xtemp2(n);
	std::vector<double> ytemp2(n);

	for (int i = 0; i < n; i++){
		xtemp2[i] = xtemp[p[i]];
		ytemp2[i] = ytemp[p[i]];
	}

	int i_start = -1, i_end = -1;
	for (int i = 0; i < n; i++) {
		if (xtemp2[i] >= start){
			i_start = i;
			break;
		}
	}
	for (int i = n - 1; i >= 0; i--) {
		if (xtemp2[i] <= end){
			i_end = i;
			break;
		}
	}

	if (i_start < 0 || i_end < 0 || i_start > i_end)
		return 0;

	n = i_end - i_start + 1;
	if (n > c->dataSize())
		n = c->dataSize();
	if (n <= 0)
		return 0;

	(*x) = static_cast<double *>(malloc(n*sizeof(double)));
	if (!(*x)){
		memoryErrorMessage();
		return 0;
	}

	(*y) = static_cast<double *>(malloc(n*sizeof(double)));
	if (!(*y)){
		free(*x);
		*x = nullptr;
		memoryErrorMessage();
		return 0;
	}

	int j = 0;
	for (int i = i_start; i <= i_end; i++){
		(*x)[j] = xtemp2[i];
		(*y)[j] = ytemp2[i];
		j++;
	}

	double pr_x = (*x)[0];
	for (int i = 1; i < n; i++){
		double xval = (*x)[i];
		if (xval <= pr_x){
			free(*x);
			free(*y);
			*x = nullptr;
			*y = nullptr;
			return -1;//x values must be monotonically increasing in GSL interpolation routines
		}
		pr_x = xval;
	}
	return n;
}
