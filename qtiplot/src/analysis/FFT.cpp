/***************************************************************************
    File                 : FFT.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2007 - 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical FFT of data sets

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
#include "FFT.h"
#include <MultiLayer.h>
#include <PlotCurve.h>
#include <ColorBox.h>
#include <Matrix.h>
#include <fft2D.h>

#include <QLocale>
#include <QApplication>

#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_fft_halfcomplex.h>

FFT::FFT(ApplicationWindow *parent, Table *t, const QString& realColName, const QString& imagColName, int from, int to)
: Filter(parent, t)
{
	init();
    setDataFromTable(t, realColName, imagColName, from, to);
}

FFT::FFT(ApplicationWindow *parent, QwtPlotCurve *c)
: Filter(parent, c)
{
	init();
    setDataFromCurve(c);
}

FFT::FFT(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end)
: Filter(parent, c)
{
	init();
    setDataFromCurve(c, start, end);
}

FFT::FFT(ApplicationWindow *parent, Graph *g, const QString& curveTitle)
: Filter(parent, g)
{
	init();
    setDataFromCurve(curveTitle);
}

FFT::FFT(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end)
: Filter(parent, g)
{
	init();
    setDataFromCurve(curveTitle, start, end);
}

FFT::FFT(ApplicationWindow *parent, Matrix *re, Matrix *im, bool inverse, bool shift, bool norm, bool outputPower2Sizes)
: Filter(parent, re),
d_inverse(inverse),
d_normalize(norm),
d_shift_order(shift),
d_im_matrix(im),
d_power2(outputPower2Sizes)
{
	setObjectName(tr("FFT") + " " + tr("2D"));
	d_real_col = -1;
	d_imag_col = -1;
	d_sampling = 1.0;
	d_output_graph = 0;
}

void FFT::init()
{
    setObjectName(tr("FFT"));
    d_inverse = false;
    d_normalize = true;
    d_shift_order = true;
    d_real_col = -1;
    d_imag_col = -1;
	d_sampling = 1.0;
	d_output_graph = 0;
	d_im_matrix = NULL;
	d_re_out_matrix = NULL;
	d_im_out_matrix = NULL;
	d_amp_matrix = NULL;
}

void FFT::fftCurve()
{
	int n2 = d_n/2;
	double *amp = (double *)malloc(d_n*sizeof(double));
	double *result = (double *)malloc(2*d_n*sizeof(double));
	if(!amp || !result){
		memoryErrorMessage();
		return;
	}

	double df = 1.0/(double)(d_n*d_sampling);//frequency sampling
	double aMax = 0.0;//max amplitude
	if(!d_inverse){
		gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(d_n);
		gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(d_n);

		if(!work || !real){
			memoryErrorMessage();
			return;
		}

		gsl_fft_real_transform(d_y, 1, d_n, real, work);
		gsl_fft_halfcomplex_unpack (d_y, result, 1, d_n);

		gsl_fft_real_wavetable_free(real);
		gsl_fft_real_workspace_free(work);
	} else {
		gsl_fft_real_unpack (d_y, result, 1, d_n);
		gsl_fft_complex_wavetable *wavetable = gsl_fft_complex_wavetable_alloc (d_n);
		gsl_fft_complex_workspace *workspace = gsl_fft_complex_workspace_alloc (d_n);

		if(!workspace || !wavetable){
			memoryErrorMessage();
			return;
		}

		gsl_fft_complex_inverse (result, 1, d_n, wavetable, workspace);
		gsl_fft_complex_wavetable_free (wavetable);
		gsl_fft_complex_workspace_free (workspace);
	}

	if (d_shift_order){
		for(int i = 0; i < d_n; i++){
			d_x[i] = (i - n2)*df;
			int j = i + d_n;
			double aux = result[i];
			result[i] = result[j];
			result[j] = aux;
		}
	} else {
		for(int i = 0; i < d_n; i++)
			d_x[i] = i*df;
	}

	for(int i = 0; i < d_n; i++) {
		int i2 = 2*i;
		double real_part = result[i2];
		double im_part = result[i2+1];
		double a = sqrt(real_part*real_part + im_part*im_part);
		amp[i]= a;
		if (a > aMax)
			aMax = a;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	int prec = app->d_decimal_digits;
	for (int i = 0; i < d_n; i++){
		int i2 = 2*i;
		d_result_table->setText(i, 0, locale.toString(d_x[i], 'g', prec));
		d_result_table->setText(i, 1, locale.toString(result[i2], 'g', prec));
		d_result_table->setText(i, 2, locale.toString(result[i2 + 1], 'g', prec));
		if (d_normalize)
			d_result_table->setText(i, 3, locale.toString(amp[i]/aMax, 'g', prec));
		else
			d_result_table->setText(i, 3, locale.toString(amp[i], 'g', prec));
		d_result_table->setText(i, 4, locale.toString(atan(result[i2 + 1]/result[i2]), 'g', prec));
	}

	free(amp);
	free(result);
}

void FFT::fftTable()
{
	double *amp = (double *)malloc(d_n*sizeof(double));
	gsl_fft_complex_wavetable *wavetable = gsl_fft_complex_wavetable_alloc (d_n);
	gsl_fft_complex_workspace *workspace = gsl_fft_complex_workspace_alloc (d_n);

	if(!amp || !wavetable || !workspace){
		memoryErrorMessage();
		return;
	}

	double df = 1.0/(double)(d_n*d_sampling);//frequency sampling
	double aMax = 0.0;//max amplitude
	if(d_inverse)
		gsl_fft_complex_inverse (d_y, 1, d_n, wavetable, workspace);
	else
		gsl_fft_complex_forward (d_y, 1, d_n, wavetable, workspace);

	gsl_fft_complex_wavetable_free (wavetable);
	gsl_fft_complex_workspace_free (workspace);

	if (d_shift_order) {
		int n2 = d_n/2;
		for(int i = 0; i < d_n; i++) {
			d_x[i] = (i - n2)*df;
			int j = i + d_n;
			double aux = d_y[i];
			d_y[i] = d_y[j];
			d_y[j] = aux;
		}
	} else {
		for(int i = 0; i < d_n; i++)
			d_x[i] = i*df;
	}

	for(int i = 0; i < d_n; i++) {
		int i2 = 2*i;
		double a = sqrt(d_y[i2]*d_y[i2] + d_y[i2+1]*d_y[i2+1]);
		amp[i]= a;
		if (a > aMax)
			aMax = a;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	int prec = app->d_decimal_digits;
	for (int i = 0; i < d_n; i++) {
		int i2 = 2*i;
		d_result_table->setText(i, 0, locale.toString(d_x[i], 'g', prec));
		d_result_table->setText(i, 1, locale.toString(d_y[i2], 'g', prec));
		d_result_table->setText(i, 2, locale.toString(d_y[i2 + 1], 'g', prec));
		if (d_normalize)
			d_result_table->setText(i, 3, locale.toString(amp[i]/aMax, 'g', prec));
		else
			d_result_table->setText(i, 3, locale.toString(amp[i], 'g', prec));
		d_result_table->setText(i, 4, locale.toString(atan(d_y[i2 + 1]/d_y[i2]), 'g', prec));
	}
	free(amp);
}

void FFT::output()
{
	if(d_inverse)
		d_explanation = tr("Inverse") + " " + tr("FFT") + " " + tr("of") + " ";
	else
		d_explanation = tr("Forward") + " " + tr("FFT") + " " + tr("of") + " ";

	if (d_matrix){
		d_explanation += d_matrix->objectName();
		return fftMatrix();
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QString tableName = app->generateUniqueName(QString(objectName()));
	QStringList header = QStringList();
	if (d_inverse)
		header << tr("Time");
	else
		header << tr("Frequency");

	if (d_curve)
		 d_explanation += d_curve->title().text();
	else if (d_table)
		d_explanation += d_table->colName(d_real_col);

	d_result_table = app->newHiddenTable(tableName, d_explanation, d_n, 5);
	d_result_table->setHeader(header << tr("Real") << tr("Imaginary") << tr("Amplitude") << tr("Angle"));

    if (d_graph && d_curve)
		fftCurve();
    else if (d_table)
		fftTable();

	if (d_graphics_display)
		outputGraphs();
}

void FFT::outputGraphs()
{
	if (d_matrix)
		return;

	createOutputGraph();

	MultiLayer *ml = d_output_graph->multiLayer();

	d_output_graph->setTitle(QString::null);
	d_output_graph->setYAxisTitle(tr("Angle (deg)"));
	d_output_graph->enableAxis(QwtPlot::xTop, true);
	d_output_graph->enableAxis(QwtPlot::yRight, true);
	if (!d_inverse)
		d_output_graph->setAxisTitle(QwtPlot::xTop, tr("Frequency") + " (" + tr("Hz") + ")");
	else
		d_output_graph->setAxisTitle(QwtPlot::xTop, tr("Time") + + " (" + tr("s") + ")");

	ScaleDraw *sd = (ScaleDraw *)d_output_graph->axisScaleDraw(QwtPlot::yLeft);
	if (sd)
		sd->setShowTicksPolicy(ScaleDraw::HideBegin);
	sd = (ScaleDraw *)d_output_graph->axisScaleDraw(QwtPlot::yRight);
	if (sd)
		sd->setShowTicksPolicy(ScaleDraw::HideBegin);
	sd = (ScaleDraw *)d_output_graph->axisScaleDraw(QwtPlot::xBottom);
	if (sd){
		sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);
		sd->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	}

	QString tableName = d_result_table->objectName();
	PlotCurve *pc = d_output_graph->insertCurve(d_result_table, 0, tableName + "_" + tr("Angle"), 0);
	pc->setPen(QPen(d_curveColor, 1));
	d_output_graph->removeLegend();
	d_output_graph->updatePlot();

	Graph *g = ml->addLayer(0, 0, 0, 0, true);
	g->setTitle(QString::null);
	if (!d_inverse)
		g->setXAxisTitle(tr("Frequency") + " (" + tr("Hz") + ")");
	else
		g->setXAxisTitle(tr("Time") + + " (" + tr("s") + ")");
	g->setYAxisTitle(tr("Amplitude"));
	g->removeLegend();

	sd = (ScaleDraw *)g->axisScaleDraw(QwtPlot::xTop);
	if (sd)
		sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);

	PlotCurve *c = g->insertCurve(d_result_table, 0, tableName + "_" + tr("Amplitude"), 0);
	c->setPen(QPen(d_curveColor, 1));
	g->updatePlot();

	double rb = g->axisScaleDiv(QwtPlot::xBottom)->upperBound();
	d_output_graph->setAxisScale(QwtPlot::xBottom, 0, rb);
	d_output_graph->setAxisScale(QwtPlot::xTop, 0, rb);
	g->setAxisScale(QwtPlot::xBottom, 0, rb);
	g->setAxisScale(QwtPlot::xTop, 0, rb);

	ml->setAlignPolicy(MultiLayer::AlignCanvases);
	ml->setRows(2);
	ml->setCols(1);
	ml->setSpacing(0, 0);
	ml->linkXLayerAxes();
	ml->setCommonLayerAxes(false, true);
	ml->arrangeLayers(false, false);
}

bool FFT::setDataFromTable(Table *t, const QString& realColName, const QString& imagColName, int from, int to)
{
	d_init_err = true;

	if (!t)
		return false;

	d_real_col = d_table->colIndex(realColName);
	if (d_real_col < 0 || t->columnType(d_real_col) != Table::Numeric)
		return false;

    if (!imagColName.isEmpty()){
        d_imag_col = d_table->colIndex(imagColName);
		if (d_imag_col < 0 || t->columnType(d_imag_col) != Table::Numeric)
			return false;
	}

	from--; to--;
	if (from < 0 || from >= t->numRows())
		from = 0;
	if (to < 0 || to >= t->numRows())
		to = t->numRows() - 1;

    if (t && d_table != t)
        d_table = t;

    if (d_n > 0)//delete previousely allocated memory
		freeMemory();

	d_graph = 0;
	d_curve = 0;
	d_init_err = false;

    d_n = abs(to - from) + 1;
    int n2 = 2*d_n;

    d_y = (double *)malloc(n2*sizeof(double));
	if (!d_y){
		memoryErrorMessage();
		return false;
	};

    d_x = (double *)malloc(d_n*sizeof(double));
	if (!d_x){
		memoryErrorMessage();
		free(d_y);
		return false;
	};

    if(d_y && d_x) {// zero-pad data array
		memset( d_y, 0, n2* sizeof( double ) );
		for(int i=0; i<d_n; i++) {
			int i2 = 2*i;
			d_y[i2] = d_table->cell(i, d_real_col);
			if (d_imag_col >= 0)
				d_y[i2+1] = d_table->cell(i, d_imag_col);
		}
	} else {
		memoryErrorMessage();
		return false;
	}
	return true;
}

void FFT::fftMatrix()
{
	if (!d_matrix)
		return;

	ApplicationWindow *app = (ApplicationWindow *)parent();

	int c = d_matrix->numCols();
	int r = d_matrix->numRows();
	int cols = c;
	int rows = r;

	if (d_power2){
		if (!isPower2(c))
			cols = next2Power(c);
		if (!isPower2(r))
			rows = next2Power(r);
	}

	bool errors = !d_im_matrix;
	if (d_im_matrix && (d_im_matrix->numCols() != cols || d_im_matrix->numRows() != rows)){
		errors = true;
		QMessageBox::warning(app, tr("QtiPlot"),
		tr("The two matrices have different dimensions, the imaginary part will be neglected!"));
	}

	double **x_int_re = Matrix::allocateMatrixData(rows, cols, true); // real coeff matrix
	if (!x_int_re)
		return;
	double **x_int_im = Matrix::allocateMatrixData(rows, cols, true); // imaginary coeff  matrix
	if (!x_int_im){
		Matrix::freeMatrixData(x_int_re, rows);
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (int i = 0; i < r; i++)
		for (int j = 0; j < c; j++)
			x_int_re[i][j] = d_matrix->cell(i, j);

	if (!errors){
		for (int i = 0; i < r; i++)
			for (int j = 0; j < c; j++)
				x_int_im[i][j] = d_im_matrix->cell(i, j);
	}

	double **x_fin_re = NULL, **x_fin_im = NULL;
	if (d_inverse){
		x_fin_re = Matrix::allocateMatrixData(rows, cols);
		x_fin_im = Matrix::allocateMatrixData(rows, cols);
		if (!x_fin_re || !x_fin_im){
			Matrix::freeMatrixData(x_int_re, rows);
			Matrix::freeMatrixData(x_int_im, rows);
			QApplication::restoreOverrideCursor();
			return;
		}
		fft2d_inv(x_int_re, x_int_im, x_fin_re, x_fin_im, cols, rows, d_shift_order);
	} else
		fft2d(x_int_re, x_int_im, cols, rows, d_shift_order);

	d_re_out_matrix = app->newMatrix(rows, cols);
	QString realCoeffMatrixName = app->generateUniqueName(tr("RealMatrixFFT"));
	app->setWindowName(d_re_out_matrix, realCoeffMatrixName);
	d_re_out_matrix->setWindowLabel(tr("Real part of the FFT transform of") + " " + d_matrix->objectName());

	d_im_out_matrix = app->newMatrix(rows, cols);
	QString imagCoeffMatrixName = app->generateUniqueName(tr("ImagMatrixFFT"));
	app->setWindowName(d_im_out_matrix, imagCoeffMatrixName);
	d_im_out_matrix->setWindowLabel(tr("Imaginary part of the FFT transform of") + " " + d_matrix->objectName());

	d_amp_matrix = app->newMatrix(rows, cols);
	QString ampMatrixName = app->generateUniqueName(tr("AmplitudeMatrixFFT"));
	app->setWindowName(d_amp_matrix, ampMatrixName);
	d_amp_matrix->setWindowLabel(tr("Amplitudes of the FFT transform of") + " " + d_matrix->objectName());

	// Automatically set a centered frequency domain range, suggestion of Dr. Armin Bayer (Qioptiq - www.qioptiq.com)
	double fxmax = 0.0, fymax = 0.0, fxmin = 0.0, fymin = 0.0;
	if (d_inverse){
		fxmax = 1.0/d_matrix->dx();
		fymax = 1.0/d_matrix->dy();

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double re = x_fin_re[i][j];
				double im = x_fin_im[i][j];
				d_re_out_matrix->setCell(i, j, re);
				d_im_out_matrix->setCell(i, j, im);
				d_amp_matrix->setCell(i, j, sqrt(re*re + im*im));
			}
		}
		Matrix::freeMatrixData(x_fin_re, rows);
		Matrix::freeMatrixData(x_fin_im, rows);
	} else {
		double dfx = 2*d_matrix->dx()*(cols - 1);
		double dfy = 2*d_matrix->dy()*(rows - 1);
		fxmax = cols/dfx;
		fymax = rows/dfy;
		fxmin = -(cols - 2)/dfx;
		fymin = -(rows - 2)/dfy;

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double re = x_int_re[i][j];
				double im = x_int_im[i][j];
				d_re_out_matrix->setCell(i, j, re);
				d_im_out_matrix->setCell(i, j, im);
				d_amp_matrix->setCell(i, j, sqrt(re*re + im*im));
			}
		}
	}
	Matrix::freeMatrixData(x_int_re, rows);
	Matrix::freeMatrixData(x_int_im, rows);

	if (d_normalize){
		double amp_min, amp_max;
		d_amp_matrix->range(&amp_min, &amp_max);
		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double amp = d_amp_matrix->cell(i, j);
				d_amp_matrix->setCell(i, j, amp/amp_max);
			}
		}
	}

	d_re_out_matrix->resize(d_matrix->size());
	d_im_out_matrix->resize(d_matrix->size());
	d_amp_matrix->resize(d_matrix->size());

	d_re_out_matrix->setCoordinates(fxmin, fxmax, fymin, fymax);
	d_im_out_matrix->setCoordinates(fxmin, fxmax, fymin, fymax);
	d_amp_matrix->setCoordinates(fxmin, fxmax, fymin, fymax);

	Matrix::ViewType view = d_matrix->viewType();
	d_re_out_matrix->setViewType(view);
	d_im_out_matrix->setViewType(view);
	d_amp_matrix->setViewType(view);

	Matrix::HeaderViewType headView = d_matrix->headerViewType();
	d_re_out_matrix->setHeaderViewType(headView);
	d_im_out_matrix->setHeaderViewType(headView);
	d_amp_matrix->setHeaderViewType(headView);

	const LinearColorMap map = d_matrix->colorMap();
	d_im_out_matrix->setColorMap(map);
	d_re_out_matrix->setColorMap(map);
	d_amp_matrix->setColorMap(map);

	QApplication::restoreOverrideCursor();
}
