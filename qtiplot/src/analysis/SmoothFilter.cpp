/***************************************************************************
    File                 : SmoothFilter.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
                           (C) 2009 by Jonas Bähr <jonas * fs.ei.tum.de>
						   (C) 2010 by Knut Franke <knut.franke * gmx.de>
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical smoothing of data sets

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
#include "SmoothFilter.h"

#include <QApplication>
#include <QMessageBox>

#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_poly.h>

SmoothFilter::SmoothFilter(ApplicationWindow *parent, QwtPlotCurve *c, int m)
: Filter(parent, c)
{
	setDataFromCurve(c);
	init(m);
}

SmoothFilter::SmoothFilter(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end, int m)
: Filter(parent, c)
{
	setDataFromCurve(c, start, end);
	init(m);
}

SmoothFilter::SmoothFilter(ApplicationWindow *parent, Graph *g, const QString& curveTitle, int m)
: Filter(parent, g)
{
	setDataFromCurve(curveTitle);
	init(m);
}

SmoothFilter::SmoothFilter(ApplicationWindow *parent, Graph *g, const QString& curveTitle,
                             double start, double end, int m)
: Filter(parent, g)
{
	setDataFromCurve(curveTitle, start, end);
    init(m);
}

SmoothFilter::SmoothFilter(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int start, int end, int m)
: Filter(parent, t)
{
	setDataFromTable(t, xCol, yCol, start, end);
	init(m);
}

void SmoothFilter::init (int m)
{
    setObjectName(tr("Smoothed"));
    setMethod(m);
    d_points = d_n;
    d_smooth_points = 2;
    d_sav_gol_points = 2;
    d_polynom_order = 2;
}


void SmoothFilter::setMethod(int m)
{
	if (m < 1 || m > 4){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("Unknown smooth filter. Valid values are: 1 - Savitky-Golay, 2 - FFT, 3 - Moving Window Average, 4 - Lowess."));
		d_init_err = true;
		return;
	}
	d_method = (SmoothMethod)m;
}

void SmoothFilter::calculateOutputData(double *x, double *y)
{
	for (int i = 0; i < d_points; i++){
		x[i] = d_x[i];
		y[i] = d_y[i];//filtering frequencies
	}

	switch((int)d_method)
	{
		case 1:
			d_explanation = QString::number(d_smooth_points) + " " + tr("points") + " " + tr("Savitzky-Golay smoothing");
			smoothSavGol(x, y);
			break;
		case 2:
			d_explanation = QString::number(d_smooth_points) + " " + tr("points") + " " + tr("FFT smoothing");
			smoothFFT(x, y);
			break;
		case 3:
			d_explanation = QString::number(d_smooth_points) + " " + tr("points") + " " + tr("average smoothing");
			smoothAverage(x, y);
			break;
		case 4:
			d_explanation = tr("Lowess smoothing with f=%1 and %2 iterations").arg(d_f).arg(d_iterations);
			smoothLowess(x, y);
			break;
	}
}

void SmoothFilter::smoothFFT(double *x, double *y)
{
	gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(d_n);
	gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(d_n);
	gsl_fft_real_transform (y, 1, d_n, real, work);//FFT forward
	gsl_fft_real_wavetable_free (real);

	double df = 1.0/(double)(x[1] - x[0]);
	double lf = df/(double)d_smooth_points;//frequency cutoff
	df = 0.5*df/(double)d_n;

    for (int i = 0; i < d_n; i++){
	   x[i] = d_x[i];
	   y[i] = i*df > lf ? 0 : y[i];//filtering frequencies
	}

	gsl_fft_halfcomplex_wavetable *hc = gsl_fft_halfcomplex_wavetable_alloc (d_n);
	gsl_fft_halfcomplex_inverse (y, 1, d_n, hc, work);//FFT inverse
	gsl_fft_halfcomplex_wavetable_free (hc);
	gsl_fft_real_workspace_free (work);
}

void SmoothFilter::smoothAverage(double *, double *y)
{
	int p2 = d_smooth_points/2;
	double m = double(2*p2+1);
	double aux = 0.0;
    double *s = new double[d_n];

	s[0] = y[0];
	for (int i=1; i<p2; i++){
		aux = 0.0;
		for (int j=-i; j<=i; j++)
			aux += y[i+j];

		s[i] = aux/(double)(2*i+1);
	}
	for (int i=p2; i<d_n-p2; i++){
		aux = 0.0;
		for (int j=-p2; j<=p2; j++)
			aux += y[i+j];

		s[i] = aux/m;
	}
	for (int i=d_n-p2; i<d_n-1; i++){
		aux = 0.0;
		for (int j=d_n-i-1; j>=i-d_n+1; j--)
			aux += y[i+j];

		s[i] = aux/(double)(2*(d_n-i-1)+1);
	}
	s[d_n-1] = y[d_n-1];

    for (int i = 0; i<d_n; i++)
        y[i] = s[i];

    delete[] s;
}

/**
 * \brief Compute Savitzky-Golay coefficients and store them into #h.
 *
 * This function follows GSL conventions in that it writes its result into a matrix allocated by
 * the caller and returns a non-zero result on error.
 *
 * The coefficient matrix is defined as the matrix H mapping a set of input values to the values
 * of the polynomial of order #polynom_order which minimizes squared deviations from the input
 * values. It is computed using the formula \$H=V(V^TV)^(-1)V^T\$, where \$V\$ is the Vandermonde
 * matrix of the point indices.
 *
 * For a short description of the mathematical background, see
 * http://www.statistics4u.info/fundstat_eng/cc_filter_savgol_math.html
 */
int SmoothFilter::savitzkyGolayCoefficients(int points, int polynom_order, gsl_matrix *h)
{
	int error = 0; // catch GSL error codes

	// compute Vandermonde matrix
	gsl_matrix *vandermonde = gsl_matrix_alloc(points, polynom_order + 1);
	for (int i = 0; i < points; ++i){
		gsl_matrix_set(vandermonde, i, 0, 1.0);
		for (int j = 1; j <= polynom_order; ++j)
			gsl_matrix_set(vandermonde, i, j, gsl_matrix_get(vandermonde, i, j - 1) * i);
	}

	// compute V^TV
	gsl_matrix *vtv = gsl_matrix_alloc(polynom_order + 1, polynom_order + 1);
	error = gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, vandermonde, vandermonde, 0.0, vtv);

	if (!error){
		// compute (V^TV)^(-1) using LU decomposition
		gsl_permutation *p = gsl_permutation_alloc(polynom_order + 1);
		int signum;
		error = gsl_linalg_LU_decomp(vtv, p, &signum);

		if (!error){
			gsl_matrix *vtv_inv = gsl_matrix_alloc(polynom_order + 1, polynom_order + 1);
			error = gsl_linalg_LU_invert(vtv, p, vtv_inv);
			if (!error) {
				// compute (V^TV)^(-1)V^T
				gsl_matrix *vtv_inv_vt = gsl_matrix_alloc(polynom_order + 1, points);
				error = gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, vtv_inv, vandermonde, 0.0, vtv_inv_vt);

				if (!error) {
					// finally, compute H = V(V^TV)^(-1)V^T
					error = gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, vandermonde, vtv_inv_vt, 0.0, h);
				}
				gsl_matrix_free(vtv_inv_vt);
			}
			gsl_matrix_free(vtv_inv);
		}
		gsl_permutation_free(p);
	}
	gsl_matrix_free(vtv);
	gsl_matrix_free(vandermonde);

	return error;
}

/**
 * \brief Savitzky-Golay smoothing of (uniformly distributed) data.
 *
 * When the data is not uniformly distributed, Savitzky-Golay looses its interesting conservation
 * properties. On the other hand, a central point of the algorithm is that for uniform data, the
 * operation can be implemented as a convolution.
 *
 * There are at least three possible approaches to handling edges of the data vector (cutting them
 * off, zero padding and using the left-/rightmost smoothing polynomial for computing smoothed
 * values near the edges). Zero-padding is a particularly bad choice for signals with a distinctly
 * non-zero baseline and cutting off edges makes further computations on the original and smoothed
 * signals more difficult; therefore, deviating from the user-specified number of left/right
 * adjacent points (by smoothing over a fixed window at the edges) would be the least annoying
 * alternative;
 */
void SmoothFilter::smoothSavGol(double *, double *y_inout)
{
	// total number of points in smoothing window
	int points = d_sav_gol_points + d_smooth_points + 1;

	if (points < d_polynom_order + 1){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The polynomial order must be lower than the number of left points plus the number of right points!"));
		d_init_err = true;
		return;
	}

	if (d_n < points){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("Tried to smooth over more points (left+right+1=%1) than given as input (%2).").arg(points).arg(d_n));
		d_init_err = true;
		return;
	}

	// Savitzky-Golay coefficient matrix, y' = H y
	gsl_matrix *h = gsl_matrix_alloc(points, points);
	if (int error = savitzkyGolayCoefficients(points, d_polynom_order, h)){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("Internal error in Savitzky-Golay algorithm.\n") + gsl_strerror(error));
		gsl_matrix_free(h);
		d_init_err = true;
		return;
	}

	// allocate memory for the result (temporary; don't overwrite y_inout while we still read from it)
	double *result = (double *)malloc(d_n*sizeof(double));
	if (!result){
		memoryErrorMessage();
		return;
	}

	// handle left edge by zero padding
	for (int i = 0; i < d_sav_gol_points; i++){
		double convolution = 0.0;
		for (int k = d_sav_gol_points - i; k < points; k++)
			convolution += gsl_matrix_get(h, d_sav_gol_points, k) * y_inout[i - d_sav_gol_points + k];
		result[i] = convolution;
	}
	// central part: convolve with fixed row of h (as given by number of left points to use)
	for (int i = d_sav_gol_points; i < d_n - d_smooth_points; i++){
		double convolution = 0.0;
		for (int k = 0; k < points; k++)
			convolution += gsl_matrix_get(h, d_sav_gol_points, k) * y_inout[i - d_sav_gol_points + k];
		result[i] = convolution;
	}

	// handle right edge by zero padding
	for (int i = d_n - d_smooth_points; i < d_n; i++){
		double convolution = 0.0;
		for (int k = 0; i - d_sav_gol_points + k < d_n; k++)
			convolution += gsl_matrix_get(h, d_sav_gol_points, k) * y_inout[i - d_sav_gol_points + k];
		result[i] = convolution;
	}

	// deallocate memory
	gsl_matrix_free(h);

	// write result into *y_inout
	for (int i = 0; i < d_n; i++)
		y_inout[i] = result[i];

	free(result);
}

void SmoothFilter::setSmoothPoints(int points, int left_points)
{
	if (points < 0 || left_points < 0){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The number of points must be positive!"));
		d_init_err = true;
		return;
	} else if (d_polynom_order > points + left_points){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The polynomial order must be lower than the number of left points plus the number of right points!"));
		d_init_err = true;
		return;
	}

	d_smooth_points = points;
	d_sav_gol_points = left_points;
}

void SmoothFilter::setPolynomOrder(int order)
{
	if (d_method != SavitzkyGolay){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("Setting polynomial order is only available for Savitzky-Golay smooth filters! Ignored option!"));
		return;
	}

	if (order > d_smooth_points + d_sav_gol_points){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The polynomial order must be lower than the number of left points plus the number of right points!"));
		d_init_err = true;
		return;
	}
	d_polynom_order = order;
}

void SmoothFilter::setLowessParameter(double f, int iterations)
{
	if (d_method != Lowess){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("Setting Lowess parameter is only available for Lowess smooth filters! Ignored option!"));
		return;
	}

	if (f < 0 || f > 1){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The parameter f must be between 0 and 1!"));
		d_init_err = true;
		return;
	}
	if (iterations < 1){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("The number of iterations must be at least 1!"));
		d_init_err = true;
		return;
	}
	d_f = f;
	d_iterations = iterations;
}

#include "lowess.c" // from the R project; see also lowess.doc from the R sources
void SmoothFilter::smoothLowess(double *x, double *y)
{
    double initial_y[d_n]; // we need to conserve the initial y as y will become the output
    for (int i = 0; i < d_n; i++)
        initial_y[i] = y[i];
    double delta = 0.0; // see lowess.doc
    double robustness_weights[d_n]; // currently unused output
    double residuals[d_n]; // currently unused output

    clowess(x, initial_y, d_n, d_f, d_iterations, delta, // inputs
            y, robustness_weights, residuals); // outputs
}

