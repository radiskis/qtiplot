/***************************************************************************
    File                 : ScreenPickerTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Tool for selecting arbitrary points on a plot.

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
#include "ScreenPickerTool.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <Matrix.h>
#include <Graph.h>
#include <PlotCurve.h>
#include <MultiLayer.h>
#include <SymbolBox.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>

#include <QLayout>
#include <QApplication>

ScreenPickerTool::ScreenPickerTool(Graph *graph, const QObject *status_target, const char *status_slot)
	: QwtPlotPicker(graph->canvas()),
	PlotToolInterface(graph, status_target, status_slot),
	d_move_restriction(NoRestriction)
{
	d_selection_marker.setLineStyle(QwtPlotMarker::Cross);
	d_selection_marker.setLinePen(QPen(Qt::red, 1));
	setTrackerMode(QwtPicker::AlwaysOn);
	setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
	d_graph->canvas()->setCursor(QCursor(QPixmap(":/cursor.png"), -1, -1));

	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	emit statusText(tr("Click on plot or move cursor to display coordinates!"));
}

ScreenPickerTool::~ScreenPickerTool()
{
	d_selection_marker.detach();
	d_graph->canvas()->unsetCursor();
	d_graph->replot();
}

void ScreenPickerTool::append(const QPoint &point)
{
	append(invTransform(point));
}

void ScreenPickerTool::append(const QwtDoublePoint &pos)
{
	double x0 = d_selection_marker.xValue();//old position
	double y0 = d_selection_marker.yValue();

	switch(d_move_restriction){
		case NoRestriction:
			d_selection_marker.setValue(pos);
		break;

		case Vertical:
			d_selection_marker.setYValue(pos.y());
		break;

		case Horizontal:
			d_selection_marker.setXValue(pos.x());
		break;
	}

	double x = d_selection_marker.xValue();
	double y = d_selection_marker.yValue();
	double dx = fabs(x - x0);
	double dy = fabs(y - y0);
	if (d_selection_marker.plot() == NULL){
		d_selection_marker.attach(d_graph);
		dx = 0;
		dy = 0;
	}

	QLocale locale = d_graph->multiLayer()->locale();
	if (d_move_restriction)
		emit statusText(QString("x=%1; y=%2")
			.arg(locale.toString(x, 'G', 14))
			.arg(locale.toString(y, 'G', 14)));
	else
		emit statusText(QString("x=%1; y=%2; dx=%3; dy=%4")
				.arg(locale.toString(x, 'G', 14))
				.arg(locale.toString(y, 'G', 14))
				.arg(locale.toString(dx, 'G', 14))
				.arg(locale.toString(dy, 'G', 14)));

	d_graph->replot();
}

bool ScreenPickerTool::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type()) {
		case QEvent::MouseButtonDblClick:
			emit selected(d_selection_marker.value());
			return true;

		case QEvent::KeyPress:
			{
				QKeyEvent *ke = (QKeyEvent*) event;
				switch(ke->key()) {
					case Qt::Key_Enter:
					case Qt::Key_Return:
					{
                        QwtDoublePoint pos = invTransform(canvas()->mapFromGlobal(QCursor::pos()));
						append(pos);
						emit selected(pos);
						return true;
					}
					default:
						break;
				}
			}
		default:
			break;
	}
	return QwtPlotPicker::eventFilter(obj, event);
}

DrawPointTool::DrawPointTool(ApplicationWindow *app, Graph *graph, const QObject *status_target, const char *status_slot)
	: ScreenPickerTool(graph, status_target, status_slot),
	d_app(app)
{
	d_curve = NULL;
	d_table = NULL;
}

void DrawPointTool::appendPoint(const QwtDoublePoint &pos)
{
	if (!d_app)
		return;

	QLocale locale = d_app->locale();
	int prec = d_app->d_decimal_digits;
	emit statusText(QString("x=%1; y=%2")
					.arg(locale.toString(pos.x(), 'G', prec))
					.arg(locale.toString(pos.y(), 'G', prec)));

	if (!d_table){
		d_table = d_app->newHiddenTable(d_app->generateUniqueName(tr("Draw")), "", 30, 2, "");
		d_app->modifiedProject();
	}

	int rows = 0;
	if (d_curve)
		rows = d_curve->dataSize();

	if (d_table->numRows() <= rows)
		d_table->setNumRows(rows + 10);

	d_table->setCell(rows, 0, pos.x());
	d_table->setCell(rows, 1, pos.y());

	if (!d_curve){
		d_curve = new DataCurve(d_table, d_table->colName(0), d_table->colName(1));
		d_curve->setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
		d_curve->setPen(QPen(Qt::black, d_app->defaultCurveLineWidth));
		d_curve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::black),
						  QPen(Qt::black, d_app->defaultCurveLineWidth),
						  QSize(d_app->defaultSymbolSize, d_app->defaultSymbolSize)));
		d_graph->insertPlotItem(d_curve, Graph::LineSymbols);
	}

	d_curve->setFullRange();
	d_graph->updatePlot();
}

bool DrawPointTool::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type()) {
		case QEvent::MouseButtonDblClick:
			appendPoint(d_selection_marker.value());
			return true;
		case QEvent::KeyPress:
			{
				QKeyEvent *ke = (QKeyEvent*) event;
				switch(ke->key()) {
					case Qt::Key_Enter:
					case Qt::Key_Return:
					{
                        QwtDoublePoint pos = invTransform(canvas()->mapFromGlobal(QCursor::pos()));
                        d_selection_marker.setValue(pos);
                        if (d_selection_marker.plot() == NULL)
                            d_selection_marker.attach(d_graph);
                        d_graph->replot();
						emit selected(d_selection_marker.value());

						appendPoint(pos);
						return true;
					}
					default:
						break;
				}
			}
		default:
			break;
	}
	return QwtPlotPicker::eventFilter(obj, event);
}

ImageProfilesTool::ImageProfilesTool(ApplicationWindow *app, Graph *graph, Matrix *m, Table *horTable, Table *verTable)
	: ScreenPickerTool(graph, app->infoLineEdit(), SLOT(setText(const QString&))),
	d_app(app),
	d_matrix(m),
	d_hor_table(horTable),
	d_ver_table(verTable),
	d_box(NULL)
{
	d_selection_marker.setAxis(QwtPlot::xTop, QwtPlot::yLeft);

	if (d_matrix){
		double xVal = 0.5*(m->xStart() + m->xEnd());
		double yVal = 0.5*(m->yStart() + m->yEnd());
		if (d_graph){
			connect(d_matrix, SIGNAL(destroyed()), d_graph, SLOT(disableImageProfilesTool()));
			connect(d_matrix, SIGNAL(modifiedData(Matrix *)), this, SLOT(modifiedMatrix(Matrix *)));

			averageBox = new QSpinBox;
			averageBox->setMinimum(1);
			averageBox->setSuffix(" " + tr("pixels"));

			horSpinBox = new DoubleSpinBox('g');
			horSpinBox->setMinimumWidth(80);
			horSpinBox->setSingleStep(1.0);
			horSpinBox->setLocale(QLocale());
			horSpinBox->setDecimals(6);

			vertSpinBox = new DoubleSpinBox('g');
			vertSpinBox->setMinimumWidth(80);
			vertSpinBox->setSingleStep(1.0);
			vertSpinBox->setLocale(QLocale());
			vertSpinBox->setDecimals(6);

			zLabel = new QLabel();
			zLabel->setMinimumWidth(80);
			zLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

			append(QwtDoublePoint(xVal, yVal));

			connect(averageBox, SIGNAL(valueChanged(int)), this, SLOT(updateCursorWidth(int)));
			connect(horSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateCursorPosition()));
			connect(vertSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateCursorPosition()));

			MultiLayer *plot = d_graph->multiLayer();
			if (plot){
				QPalette pal = plot->palette();
				pal.setColor(QPalette::Window, QColor(Qt::lightGray));
				plot->setPalette(pal);

				d_box = new QWidget();
				QHBoxLayout *hl = new QHBoxLayout(d_box);
				hl->setSpacing(5);
				hl->addWidget(new QLabel(tr("Average")));
				hl->addWidget(averageBox);
				hl->addWidget(new QLabel(tr("Position") + ": " + tr("x")));
				hl->addWidget(horSpinBox);
				hl->addWidget(new QLabel(tr("y")));
				hl->addWidget(vertSpinBox);
				hl->addWidget(new QLabel(tr("Z-Value")));
				hl->addWidget(zLabel);

				plot->toolBox()->insertWidget(0, d_box);
			}
		}

		MatrixModel *mm = d_matrix->matrixModel();
		if (d_hor_table){
			for (int i = 0; i < d_hor_table->numRows(); i++)
				d_hor_table->setCell(i, 0, mm->x(i));
		}
		if (d_ver_table){
			for (int i = 0; i < d_ver_table->numRows(); i++)
				d_ver_table->setCell(i, 0, mm->y(i));
		}
	}
}

void ImageProfilesTool::connectPlotLayers()
{
	if (!d_graph)
		return;

	MultiLayer *plot = d_graph->multiLayer();
	if (!plot)
		return;

	Graph *gHor = plot->layer(2);
	if (gHor)
		gHor->addCurves(d_hor_table, QStringList(d_hor_table->colName(1)));

	Graph *gVert = plot->layer(3);
	if (gVert){
		DataCurve *c = gVert->insertCurve(d_ver_table, d_ver_table->colName(1), d_ver_table->colName(0), Graph::Line);
		if (c){
			c->setAxis(QwtPlot::xTop, QwtPlot::yLeft);
			c->setCurveType(QwtPlotCurve::Xfy);
		}
	}
}

void ImageProfilesTool::updateCursorPosition()
{
	append(QwtDoublePoint(horSpinBox->value(), vertSpinBox->value()));

	if (d_graph)
		d_graph->replot();
}

void ImageProfilesTool::setAveragePixels(int pixels)
{
	averageBox->blockSignals(true);
	averageBox->setValue(pixels);
	averageBox->blockSignals(false);
	setCursorWidth(pixels);
}

void ImageProfilesTool::setCursorWidth(int width)
{
	QBrush br = QBrush(Qt::red);
	if (width > 1){
		QColor c = Qt::red;
		c.setAlphaF(0.25);
		br.setColor(c);
	}
	d_selection_marker.setLinePen(QPen(br, width));
}

void ImageProfilesTool::updateCursorWidth(int width)
{
	setCursorWidth(width);
	if (d_graph)
		d_graph->replot();
	append(QwtDoublePoint(horSpinBox->value(), vertSpinBox->value()));
}

void ImageProfilesTool::modifiedMatrix(Matrix *m)
{
	if (!m)
		return;

	double mmin, mmax;
	m->range(&mmin, &mmax);
	mmin = floor(mmin);
	mmax = ceil(mmax);

	MatrixModel *mm = m->matrixModel();
	if (d_hor_table){
		d_hor_table->setNumRows(m->numCols());
		for (int i = 0; i < d_hor_table->numRows(); i++)
			d_hor_table->setCell(i, 0, mm->x(i));
	}
	if (d_ver_table){
		d_ver_table->setNumRows(m->numRows());
		for (int i = 0; i < d_ver_table->numRows(); i++)
			d_ver_table->setCell(i, 0, mm->y(i));
	}

	if (d_graph){
		d_graph->setScale(QwtPlot::yLeft, QMIN(m->yStart(), m->yEnd()), QMAX(m->yStart(), m->yEnd()),
					0.0, 5, 5, Graph::Linear, true);
		d_graph->setScale(QwtPlot::xTop, QMIN(m->xStart(), m->xEnd()), QMAX(m->xStart(), m->xEnd()));
		d_graph->replot();

		MultiLayer *plot = d_graph->multiLayer();
		Graph *gHor = plot->layer(2);
		if (gHor){
			gHor->setScale(QwtPlot::xBottom, QMIN(m->xStart(), m->xEnd()), QMAX(m->xStart(), m->xEnd()));
			gHor->setScale(QwtPlot::yLeft, mmin, mmax);
			gHor->replot();
		}

		Graph *gVert = plot->layer(3);
		if (gVert){
			gVert->setScale(QwtPlot::xTop, mmin, mmax);
			gVert->setScale(QwtPlot::yLeft, QMIN(m->yStart(), m->yEnd()), QMAX(m->yStart(), m->yEnd()),
					0.0, 5, 5, Graph::Linear, true);
			gVert->replot();
		}
	}

	append(QwtDoublePoint(d_selection_marker.xValue(), d_selection_marker.yValue()));
}

ImageProfilesTool* ImageProfilesTool::clone(Graph *g)
{
	if (!d_matrix || !d_app)
		return 0;

	Table *hTable = d_app->newHiddenTable(QString::null, QString::null, d_matrix->numCols(), 2);
	Table *vTable = d_app->newHiddenTable(QString::null, QString::null, d_matrix->numRows(), 2);

	ImageProfilesTool *tool = new ImageProfilesTool(d_app, g, d_matrix, hTable, vTable);
	tool->setAveragePixels(averageBox->value());
	if (g && g->multiLayer()){
		Graph *gHor = g->multiLayer()->layer(2);
		if (gHor)
			gHor->removeCurve(0);

		Graph *gVert = g->multiLayer()->layer(3);
		if (gVert)
			gVert->removeCurve(0);
	}
	tool->connectPlotLayers();
	tool->append(QwtDoublePoint(xValue(), yValue()));
	return tool;
}

void ImageProfilesTool::append(const QwtDoublePoint &pos)
{
	ScreenPickerTool::append(pos);

	if (!d_app || !d_matrix)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	MultiLayer *plot = d_graph->multiLayer();
	Graph *gHor = plot->layer(2);
	if (gHor)
		gHor->enableAutoscaling(false);

	Graph *gVert = plot->layer(3);
	if (gVert){
		gVert->enableAutoscaling(false);
		QwtPlotCurve *c = gVert->curve(0);
		if (c)
			c->setCurveType(QwtPlotCurve::Xfy);
	}

	double x = pos.x();
	double y = pos.y();

	horSpinBox->setValue(x);
	vertSpinBox->setValue(y);

	x += 0.5*d_matrix->dx();
	y -= 0.5*d_matrix->dy();

	int row = qRound(fabs((y - d_matrix->yStart())/d_matrix->dy()));
	int col = qRound(fabs((x - d_matrix->xStart())/d_matrix->dx()));

	int rows = d_matrix->numRows();
	int cols = d_matrix->numCols();

	if (row < 0)
		row = 0;
	else if (row >= rows)
		row = rows - 1;

	if (col < 0)
		col = 0;
	else if (col >= cols)
		col = cols - 1;

	zLabel->setText(QLocale().toString(d_matrix->cell(row, col)));

	int pixels = averageBox->value();
	if (d_hor_table){
		if (d_hor_table->numRows() != cols)
			d_hor_table->setNumRows(cols);
		if (pixels > 1){
			int endPixel = (pixels%2) ? row + pixels/2 : row + pixels/2 - 1;
			for (int i = 0; i < cols; i++){
				double val = 0.0;
				int n = pixels;
				for (int j = row - pixels/2; j <= endPixel; j++){
					if (j < 0 || j >= rows){
						n--;
						continue;
					}
					val += d_matrix->cell(j, i);
				}
				d_hor_table->setCell(i, 1, val/(double)n);
			}
		} else {
			for (int i = 0; i < cols; i++)
				d_hor_table->setCell(i, 1, d_matrix->cell(row, i));
		}
		d_hor_table->notifyChanges();
	}

	if (d_ver_table){
		if (d_ver_table->numRows() != rows)
			d_ver_table->setNumRows(rows);
		if (pixels > 1){
			int endPixel = (pixels%2) ? col + pixels/2 : col + pixels/2 - 1;
			for (int i = 0; i < rows; i++){
				double val = 0.0;
				int n = pixels;
				for (int j = col - pixels/2; j <= endPixel; j++){
					if (j < 0 || j >= cols){
						n--;
						continue;
					}
					val += d_matrix->cell(i, j);
				}
				d_ver_table->setCell(i, 1, val/(double)n);
			}
		} else {
			for (int i = 0; i < rows; i++)
				d_ver_table->setCell(i, 1, d_matrix->cell(i, col));
		}
		d_ver_table->notifyChanges();
	}

	QLocale locale = d_app->locale();
	int prec = d_app->d_decimal_digits;
	emit statusText(QString("x=%1; y=%2; z=%3")
					.arg(locale.toString(pos.x(), 'G', prec))
					.arg(locale.toString(pos.y(), 'G', prec))
					.arg(locale.toString(d_matrix->cell(row, col), 'G', prec)));

	QApplication::restoreOverrideCursor();
}

ImageProfilesTool::~ImageProfilesTool()
{
	if (d_hor_table){
		d_hor_table->askOnCloseEvent(false);
		d_hor_table->close();
	}

	if (d_ver_table){
		d_ver_table->askOnCloseEvent(false);
		d_ver_table->close();
	}

	if (d_box){
		delete d_box;
		d_box = 0;
	}
}
