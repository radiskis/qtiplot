/***************************************************************************
    File                 : QwtHistogram.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Histogram class

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
#include "QwtHistogram.h"
#include "Graph.h"
#include <Matrix.h>
#include <MatrixModel.h>
#include <QPainter>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_histogram.h>
#include <qwt_painter.h>

QwtHistogram::QwtHistogram(Table *t, const QString& name, int startRow, int endRow):
	QwtBarCurve(QwtBarCurve::Vertical, t, "", name, startRow, endRow)
{
	init();
}

QwtHistogram::QwtHistogram(Matrix *m):
    QwtBarCurve(QwtBarCurve::Vertical, NULL, "matrix", m->objectName(), 0, 0)
{
	if (m){
		init();
		d_matrix = m;
	}
}

void QwtHistogram::init()
{
	d_autoBin = true;
	d_matrix = 0;
	bar_gap = 0;
	setType(Graph::Histogram);
	setPlotStyle(Graph::Histogram);
	setStyle(QwtPlotCurve::UserCurve);
}

void QwtHistogram::copy(QwtHistogram *h)
{
	QwtBarCurve::copy((QwtBarCurve *)h);

	d_autoBin = h->d_autoBin;
	d_bin_size = h->d_bin_size;
	d_begin = h->d_begin;
	d_end = h->d_end;
}

QwtDoubleRect QwtHistogram::boundingRect() const
{
	QwtDoubleRect rect = QwtPlotCurve::boundingRect();
	rect.setLeft(rect.left() - x(1));
	rect.setRight(rect.right() + x(dataSize() - 1));
	rect.setTop(0);
	rect.setBottom(1.2*rect.bottom());
	return rect;
}

void QwtHistogram::setBinning(bool autoBin, double size, double begin, double end)
{
	d_autoBin = autoBin;
	d_bin_size = size;
	d_begin = begin;
	d_end = end;
}

void QwtHistogram::setBinning(double binSize, double begin, double end)
{
	d_autoBin = false;
	d_bin_size = binSize;
	d_begin = begin;
	d_end = end;
}

void QwtHistogram::setAutoBinning(bool autoBin)
{
	if (d_autoBin == autoBin)
		return;

	d_autoBin = autoBin;
}

void QwtHistogram::loadData()
{
    if (d_matrix){
        loadDataFromMatrix();
        return;
    }

    int r = abs(d_end_row - d_start_row) + 1;
	QVarLengthArray<double> Y(r);

    int ycol = d_table->colIndex(title().text());
	int size = 0;
	for (int i = 0; i<r; i++ ){
		QString yval = d_table->text(i, ycol);
		if (!yval.isEmpty()){
		    bool valid_data = true;
            Y[size] = ((Graph *)plot())->locale().toDouble(yval, &valid_data);
            if (valid_data)
                size++;
		}
	}
	if(size < 2 || (size==2 && Y[0] == Y[1])){//non valid histogram
		double X[2];
		Y.resize(2);
		for (int i = 0; i<2; i++ ){
			Y[i] = 0;
			X[i] = 0;
		}
		setData(X, Y.data(), 2);
		return;
	}

	int n;
	gsl_histogram *h;
	if (d_autoBin){
		n = 10;
		h = gsl_histogram_alloc (n);
		if (!h)
			return;

		gsl_vector *v = gsl_vector_alloc (size);
		for (int i = 0; i<size; i++ )
			gsl_vector_set (v, i, Y[i]);

		double min, max;
		gsl_vector_minmax (v, &min, &max);
		gsl_vector_free (v);

		d_begin = floor(min);
		d_end = ceil(max);
		if (d_end == max)
			d_end += 1.0;

		d_bin_size = (d_end - d_begin)/(double)n;

		gsl_histogram_set_ranges_uniform (h, d_begin, d_end);
	} else {
		n = int((d_end - d_begin)/d_bin_size + 1);
		h = gsl_histogram_alloc (n);
		if (!h)
			return;

		double *range = new double[n+2];
		for (int i = 0; i<= n+1; i++ )
			range[i] = d_begin + i*d_bin_size;

		gsl_histogram_set_ranges (h, range, n+1);
		delete[] range;
	}

	for (int i = 0; i<size; i++ )
		gsl_histogram_increment (h, Y[i]);

    double X[n]; //stores ranges (x) and bins (y)
	Y.resize(n);
	for (int i = 0; i<n; i++ ){
		Y[i] = gsl_histogram_get (h, i);
		double lower, upper;
		gsl_histogram_get_range (h, i, &lower, &upper);
		X[i] = lower;
	}
	setData(X, Y.data(), n);

	d_mean = gsl_histogram_mean(h);
	d_standard_deviation = gsl_histogram_sigma(h);
	d_min = gsl_histogram_min_val(h);
	d_max = gsl_histogram_max_val(h);

	gsl_histogram_free (h);
	if (d_show_labels)
		loadLabels();
}

void QwtHistogram::loadDataFromMatrix()
{
	if (!d_matrix)
		return;

	int size = d_matrix->numRows()*d_matrix->numCols();
	const double *data = d_matrix->matrixModel()->dataVector();

	int n;
	gsl_histogram *h;
	if (d_autoBin){
		double min, max;
		d_matrix->range(&min, &max);
		d_begin = floor(min);
		d_end = ceil(max);
		d_bin_size = 1.0;

		n = qRound((d_end - d_begin)/d_bin_size);
		if (!n)
			return;

		h = gsl_histogram_alloc(n);
		if (!h)
			return;
		gsl_histogram_set_ranges_uniform (h, d_begin, d_end);
	} else {
		n = int((d_end - d_begin)/d_bin_size + 1);
		if (!n)
			return;

		h = gsl_histogram_alloc (n);
		if (!h)
			return;

		double *range = new double[n+2];
		for (int i = 0; i<= n+1; i++ )
			range[i] = d_begin + i*d_bin_size;

		gsl_histogram_set_ranges (h, range, n+1);
		delete[] range;
	}

	for (int i = 0; i<size; i++ )
		gsl_histogram_increment (h, data[i]);

	double X[n], Y[n]; //stores ranges (x) and bins (y)
	for (int i = 0; i<n; i++ ){
		Y[i] = gsl_histogram_get (h, i);
		double lower, upper;
		gsl_histogram_get_range (h, i, &lower, &upper);
		X[i] = lower;
	}

	setData(X, Y, n);

	d_mean = gsl_histogram_mean(h);
	d_standard_deviation = gsl_histogram_sigma(h);
	d_min = gsl_histogram_min_val(h);
	d_max = gsl_histogram_max_val(h);

	gsl_histogram_free (h);
	if (d_show_labels)
		loadLabels();
}


void QwtHistogram::loadLabels()
{
	clearLabels();

	int size = this->dataSize();
	if (!size)
		return;

	QwtPlot *d_plot = plot();
	if (!d_plot)
		return;

	int index = 0;
	for (int i = 0; i < size; i++){
		PlotMarker *m = new PlotMarker(index, d_labels_angle);

		QwtText t = QwtText(QString::number(y(i)));
		t.setColor(d_labels_color);
		t.setFont(d_labels_font);
		if (d_white_out_labels)
			t.setBackgroundBrush(QBrush(Qt::white));
		else
			t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);

		int x_axis = xAxis();
		int y_axis = yAxis();
		m->setAxis(x_axis, y_axis);

		QSize size = t.textSize();
		int dx = int(d_labels_x_offset*0.01*size.height());
		int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());
		int x2 = d_plot->transform(x_axis, x(index)) + dx;
		int y2 = d_plot->transform(y_axis, y(index)) + dy;
		switch(d_labels_align){
			case Qt::AlignLeft:
			break;
			case Qt::AlignHCenter:
				x2 -= size.width()/2;
			break;
			case Qt::AlignRight:
				x2 -= size.width();
			break;
		}
		m->setXValue(d_plot->invTransform(x_axis, x2));
		m->setYValue(d_plot->invTransform(y_axis, y2));
		m->attach(d_plot);
		d_labels_list << m;
		index++;
	}

	d_show_labels = true;
}
