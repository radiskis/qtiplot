/***************************************************************************
    File                 : Fit.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Abstract base class for data analysis operations

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
#include "Filter.h"

#include <Table.h>
#include <FrameWidget.h>
#include <LegendWidget.h>
#include <FunctionCurve.h>
#include <PlotCurve.h>
#include <MultiLayer.h>
#include <ColorBox.h>
#include <Matrix.h>

#include <QApplication>
#include <QMessageBox>
#include <QLocale>

#include <gsl/gsl_sort.h>

Filter::Filter(ApplicationWindow *parent, QwtPlotCurve *c)
: QObject(parent)
{
	init();
	if (c)
		setObjectName(c->title().text());
}

Filter::Filter( ApplicationWindow *parent, Graph *g, const QString& name)
: QObject(parent)
{
	init();
	setObjectName(name);
	d_graph = g;
	d_output_graph = g;
}

Filter::Filter( ApplicationWindow *parent, Table *t, const QString& name)
: QObject(parent)
{
	init();
	setObjectName(name);
	d_table = t;
}

Filter::Filter( ApplicationWindow *parent, Matrix *m, const QString& name)
: QObject(parent)
{
	init();
	setObjectName(name);
	d_matrix = m;
}

void Filter::init()
{
	d_n = 0;
	d_curveColor = Qt::red;
	d_tolerance = 1e-4;
	d_points = 100;
	d_max_iterations = 1000;
	d_curve = 0;
	d_result_curve = 0;
	d_prec = ((ApplicationWindow *)parent())->fit_output_precision;
	d_init_err = false;
    d_sort_data = true;
    d_min_points = 2;
    d_explanation = objectName();
    d_graph = 0;
    d_table = 0;
    d_result_table = 0;
	d_output_graph = 0;
	d_graphics_display = true;
	d_update_output_graph = true;
	d_y_col_name = QString::null;
	d_matrix = NULL;
}

void Filter::setInterval(double from, double to)
{
	if (!d_curve){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("Please assign a curve first!"));
		return;
	}
	setDataFromCurve (d_curve->title().text(), from, to);
}

void Filter::setDataCurve(QwtPlotCurve *curve, double start, double end)
{
	if (!curve)
		return;

    if (d_n > 0)//delete previousely allocated memory
		freeMemory();

	d_init_err = false;
	d_curve = curve;
    if (d_sort_data)
        d_n = sortedCurveData(d_curve, start, end, &d_x, &d_y);
    else
    	d_n = curveData(d_curve, start, end, &d_x, &d_y);

	if (d_n == -1){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("Several data points have the same x value causing divisions by zero, operation aborted!"));
		d_init_err = true;
        return;
	}else if (d_n < d_min_points){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("You need at least %1 points in order to perform this operation!").arg(d_min_points));
		d_init_err = true;
        return;
	}

    d_from = start;
    d_to = end;
}

int Filter::curveIndex(const QString& curveTitle, Graph *g)
{
	if (curveTitle.isEmpty()){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Filter Error"),
				tr("Please enter a valid curve name!"));
		d_init_err = true;
		return -1;
	}

	if (g){
		d_graph = g;
		d_output_graph = g;
	}

	if (!d_graph){
		d_init_err = true;
		return -1;
	}

	return d_graph->curveIndex(curveTitle);
}

bool Filter::setDataFromCurve(QwtPlotCurve *c)
{
	if (!c || !c->plot()){
		d_init_err = true;
		return false;
	}

	d_graph = qobject_cast<Graph *>(c->plot());
	if (!d_graph){
		d_init_err = true;
		return false;
	}
	d_output_graph = d_graph;

  	d_graph->range(c, &d_from, &d_to);
    setDataCurve(c, d_from, d_to);
	return true;
}

bool Filter::setDataFromCurve(QwtPlotCurve *c, double from, double to)
{
	if (!c || !c->plot()){
		d_init_err = true;
		return false;
	}

	d_graph = qobject_cast<Graph *>(c->plot());
	if (!d_graph){
		d_init_err = true;
		return false;
	}
	d_output_graph = d_graph;
    setDataCurve(c, from, to);
	return true;
}

bool Filter::setDataFromCurve(const QString& curveTitle, Graph *g)
{
	int index = curveIndex(curveTitle, g);
	if (index < 0){
		d_init_err = true;
		return false;
	}

  	d_graph->range(curveTitle, &d_from, &d_to);
    setDataCurve(d_graph->curve(index), d_from, d_to);
	return true;
}

bool Filter::setDataFromCurve(const QString& curveTitle, double from, double to, Graph *g)
{
	int index = curveIndex(curveTitle, g);
	if (index < 0){
		d_init_err = true;
		return false;
	}

	setDataCurve(d_graph->curve(index), from, to);
	return true;
}

void Filter::setColor(int colorId)
{
	d_curveColor = ColorBox::defaultColor(colorId);
}

void Filter::setColor(const QString& colorName)
{
	QColor c = QColor(colorName);
	if (d_curveColor == c)
		return;

	d_curveColor = c;
	if (colorName == "green")
		d_curveColor = Qt::green;
	else if (colorName == "darkYellow")
		d_curveColor = Qt::darkYellow;
	if (!ColorBox::isValidColor(c)){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Color Name Error"),
				tr("The color name '%1' is not valid, a default color (red) will be used instead!").arg(colorName));
		d_curveColor = Qt::red;
		return;
	}
}

void Filter::showLegend()
{
	if (!d_output_graph)
		return;

	QList <FrameWidget *> lst = d_output_graph->enrichmentsList();
	foreach(FrameWidget *fw, lst){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l && l->isAutoUpdateEnabled()){
			l->setText(l->text() + "\n" + legendInfo());
			l->repaint();
			return;
		}
	}

	LegendWidget *l = d_output_graph->newLegend(legendInfo());
	l->repaint();
}

bool Filter::run()
{
	if (d_matrix){
		output();
		return true;
	}

	if (d_init_err)
		return false;

	if (d_n < 0){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("You didn't specify a valid data set for this operation!"));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	output();//data analysis and output
	((ApplicationWindow *)parent())->updateLog(logInfo());

	QApplication::restoreOverrideCursor();
	return true;
}

void Filter::output()
{
	double *x = (double *)malloc(d_points*sizeof(double));
	if (!x){
		memoryErrorMessage();
		return;
	}
	double *y = (double *)malloc(d_points*sizeof(double));
	if (!y){
		free(x);
		memoryErrorMessage();
		return;
	}

	calculateOutputData(x, y); //does the data analysis
	if (!d_init_err)
		addResultCurve(x, y);
	free(x);
	free(y);
}

int Filter::sortedCurveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
	if (!c)
		return 0;

	int i_start = 0, i_end = 0;
	int n = curveRange(c, start, end, &i_start, &i_end);

	(*x) = (double *)malloc(n*sizeof(double));
	if (!x){
		memoryErrorMessage();
		return 0;
	}

	(*y) = (double *)malloc(n*sizeof(double));
	if (!y){
		free(x);
		memoryErrorMessage();
		return 0;
	}

	double *xtemp = new double[n];
	double *ytemp = new double[n];

	int j = 0;
	if (c->curveType() == QwtPlotCurve::Yfx){
		for (int i = i_start; i <= i_end; i++){
			xtemp[j] = c->x(i);
			ytemp[j++] = c->y(i);
		}
	} else {
		for (int i = i_start; i <= i_end; i++){
			xtemp[j] = c->y(i);
			ytemp[j++] = c->x(i);
		}
	}

	size_t *p = new size_t[n];
	gsl_sort_index(p, xtemp, 1, n);

	for (int i = 0; i < n; i++){
		(*x)[i] = xtemp[p[i]];
		(*y)[i] = ytemp[p[i]];
	}

	delete[] xtemp;
	delete[] ytemp;
	delete[] p;
	return n;
}

int Filter::curveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
    if (!c)
        return 0;

   	int i_start = 0, i_end = 0;
	int n = curveRange(c, start, end, &i_start, &i_end);

	(*x) = (double *)malloc(n*sizeof(double));
	if (!x){
		memoryErrorMessage();
		return 0;
	}

	(*y) = (double *)malloc(n*sizeof(double));
	if (!y){
		free(x);
		memoryErrorMessage();
		return 0;
	}

	int j = 0;
	if (c->curveType() == QwtPlotCurve::Yfx){
		for (int i = i_start; i <= i_end; i++){
			(*x)[j] = c->x(i);
			(*y)[j++] = c->y(i);
		}
	} else {
		for (int i = i_start; i <= i_end; i++){
			(*x)[j] = c->y(i);
			(*y)[j++] = c->x(i);
		}
	}
	return n;
}

int Filter::curveRange(QwtPlotCurve *c, double start, double end, int *iStart, int *iEnd)
{
    if (!c)
        return 0;

    int n = c->dataSize();
    int i_start = 0, i_end = n;

	if (c->x(0) < c->x(n-1)){
    	for (int i = 0; i < n; i++){
  	   	 if (c->x(i) >= start){
  	    	  i_start = i;
          	break;
        	}
		}
    	for (int i = n-1; i >= 0; i--){
  	    	if (c->x(i) <= end){
  	      		i_end = i;
          		break;
        	}
		}
	} else {
    	for (int i = 0; i < n; i++){
  	   	 if (c->x(i) <= end){
  	    	  i_start = i;
          	break;
        	}
		}
    	for (int i = n-1; i >= 0; i--){
  	    	if (c->x(i) >= start){
  	      		i_end = i;
          		break;
        	}
		}
	}

	*iStart = QMIN(i_start, i_end);
	*iEnd = QMAX(i_start, i_end);
    n = abs(i_end - i_start) + 1;
    return n;
}

QwtPlotCurve* Filter::addResultCurve(double *x, double *y)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	const QString tableName = app->generateUniqueName(QString(objectName()));
	QString dataSet;
	if (d_curve)
		dataSet = d_curve->title().text();
	else
		dataSet = d_y_col_name;

	d_result_table = app->newHiddenTable(tableName, d_explanation + " " + tr("of") + " " + dataSet, d_points, 2);
	if (!d_result_table)
		return 0;

	for (int i = 0; i < d_points; i++){
		d_result_table->setText(i, 0, locale.toString(x[i], 'g', app->d_decimal_digits));
		d_result_table->setText(i, 1, locale.toString(y[i], 'g', app->d_decimal_digits));
	}

	DataCurve *c = 0;
	if (d_graphics_display){
		c = new DataCurve(d_result_table, tableName + "_1", tableName + "_2");
		if (d_curve){
			c->setCurveType(d_curve->curveType());
			c->setAxis(d_curve->xAxis(), d_curve->yAxis());
		}
		if (c->curveType() == QwtPlotCurve::Yfx)
			c->setData(x, y, d_points);
		else
			c->setData(y, x, d_points);
		c->setPen(QPen(d_curveColor, 1));

		if (!d_output_graph)
			createOutputGraph();

		d_output_graph->insertPlotItem(c, Graph::Line);
		if (d_update_output_graph)
			d_output_graph->updatePlot();

		d_result_curve = c;
	}
	return (QwtPlotCurve*)c;
}

void Filter::enableGraphicsDisplay(bool on, Graph *g)
{
	d_graphics_display = on;
	if (on){
		if (g)
			d_output_graph = g;
		else
			createOutputGraph();
	}
}

MultiLayer * Filter::createOutputGraph()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return NULL;

	MultiLayer *ml = app->newGraph(tr("Plot") + objectName());
	if (ml)
   		d_output_graph = ml->activeLayer();

	return ml;
}

bool Filter::setDataFromTable(Table *t, const QString& xColName, const QString& yColName, int startRow, int endRow, bool sort)
{
	d_init_err = true;

	if (!t)
		return false;

	int xcol = t->colIndex(xColName);
	int ycol = t->colIndex(yColName);
	if (xcol < 0 || ycol < 0)
		return false;

	if (t->columnType(xcol) != Table::Numeric || t->columnType(ycol) != Table::Numeric)
		return false;

    startRow--; endRow--;
	if (startRow < 0 || startRow >= t->numRows())
		startRow = 0;
	if (endRow < 0 || endRow >= t->numRows())
		endRow = t->numRows() - 1;

    int from = QMIN(startRow, endRow);
    int to = QMAX(startRow, endRow);

	int r = abs(to - from) + 1;
    QVector<double> X(r), Y(r);

	int size = 0;
	for (int i = from; i <= to; i++){
		QString xval = t->text(i, xcol);
		QString yval = t->text(i, ycol);
		if (!xval.isEmpty() && !yval.isEmpty()){
		    bool valid_data = true;
            X[size] = t->locale().toDouble(xval, &valid_data);
            Y[size] = t->locale().toDouble(yval, &valid_data);
            if (valid_data)
                size++;
		}
	}

	if (size < d_min_points){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
		tr("You need at least %1 points in order to perform this operation!").arg(d_min_points));
        return false;
	}

    if (d_n > 0)//delete previousely allocated memory
		freeMemory();

	d_graph = 0;
	d_curve = 0;
	d_n = size;
	d_init_err = false;
	d_table = t;
	d_y_col_name = t->colName(ycol);
	X.resize(d_n);
	Y.resize(d_n);
	d_from = X[0];
    d_to = X[d_n-1];
	d_sort_data = sort;

	d_x = (double *)malloc(d_n*sizeof(double));
	if (!d_x){
		memoryErrorMessage();
		return false;
	};

    d_y = (double *)malloc(d_n*sizeof(double));
	if (!d_y){
		memoryErrorMessage();
		free(d_x);
		return false;
	};

    for (int i = 0; i < d_n; i++){
        d_x[i] = X[i];
        d_y[i] = Y[i];
    }

	if (d_sort_data){
    	size_t *p = new size_t[d_n];
   		gsl_sort_index(p, X.data(), 1, d_n);
    	for (int i=0; i<d_n; i++){
        	d_x[i] = X[p[i]];
  	    	d_y[i] = Y[p[i]];
		}
		delete[] p;
    }
	return true;
}

void Filter::memoryErrorMessage()
{
	d_init_err = true;

	QApplication::restoreOverrideCursor();

	QMessageBox::critical((ApplicationWindow *)parent(),
		tr("QtiPlot") + " - " + tr("Memory Allocation Error"),
		tr("Not enough memory, operation aborted!"));
}

void Filter::freeMemory()
{
	if (d_x){
		free(d_x);
		d_x = NULL;
	}
	if (d_y) {
		free(d_y);
		d_y = NULL;
	}
}

Filter::~Filter()
{
	if (d_n > 0)//delete the memory allocated for the data
		freeMemory();
}
