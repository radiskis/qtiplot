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
#include "Graph.h"
#include "PlotCurve.h"
#include <MultiLayer.h>
#include <cursors.h>
#include <SymbolBox.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>

#include <QLayout>

ScreenPickerTool::ScreenPickerTool(Graph *graph, const QObject *status_target, const char *status_slot)
	: QwtPlotPicker(graph->canvas()),
	PlotToolInterface(graph, status_target, status_slot),
	d_move_restriction(NoRestriction)
{
	d_selection_marker.setLineStyle(QwtPlotMarker::Cross);
	d_selection_marker.setLinePen(QPen(Qt::red,1));
	setTrackerMode(QwtPicker::AlwaysOn);
	setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
	d_graph->canvas()->setCursor(QCursor(QPixmap(cursor_xpm), -1, -1));

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

ImageProfilesTool::ImageProfilesTool(ApplicationWindow *app, Graph *graph, Matrix *m, Table *horTable, Table *verTable,
						const QObject *status_target, const char *status_slot)
	: ScreenPickerTool(graph, status_target, status_slot),
	d_app(app),
	d_matrix(m),
	d_hor_table(horTable),
	d_ver_table(verTable)
{
	d_selection_marker.setAxis(QwtPlot::xTop, QwtPlot::yLeft);

	if (d_matrix){
		double xVal = 0.5*(m->xStart() + m->xEnd());
		double yVal = 0.5*(m->yStart() + m->yEnd());
		if (d_graph){
			connect(d_matrix, SIGNAL(destroyed()), d_graph, SLOT(disableTools()));
			connect(d_matrix, SIGNAL(modifiedData(Matrix *)), this, SLOT(modifiedMatrix(Matrix *)));

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

			connect(horSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateCursorPosition()));
			connect(vertSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateCursorPosition()));

			MultiLayer *plot = d_graph->multiLayer();
			if (plot){
				QPalette pal = plot->palette();
				pal.setColor(QPalette::Window, QColor(Qt::lightGray));
				plot->setPalette(pal);

				QHBoxLayout *box = new QHBoxLayout();
				box->setSpacing(5);
				box->addWidget(new QLabel(tr("Position") + ": " + tr("x")));
				box->addWidget(horSpinBox);
				box->addWidget(new QLabel(tr("y")));
				box->addWidget(vertSpinBox);
				box->addWidget(new QLabel(tr("Z-Value")));
				box->addWidget(zLabel);

				plot->toolBox()->insertLayout(0, box);
			}
		}
	}

	if (d_hor_table){
		for (int i = 0; i < d_hor_table->numRows(); i++)
			d_hor_table->setText(i, 0, QString::number(i));
	}
	if (d_ver_table){
		for (int i = 0; i < d_ver_table->numRows(); i++)
			d_ver_table->setText(i, 0, QString::number(i));
	}
}

void ImageProfilesTool::updateCursorPosition()
{
	append(QwtDoublePoint(horSpinBox->value(), vertSpinBox->value()));

	if (d_graph)
		d_graph->replot();
}

void ImageProfilesTool::modifiedMatrix(Matrix *m)
{
	if (!m)
		return;

	double mmin, mmax;
	m->range(&mmin, &mmax);

	if (d_hor_table){
		d_hor_table->setNumRows(m->numCols());
		for (int i = 0; i < d_hor_table->numRows(); i++)
			d_hor_table->setText(i, 0, QString::number(i));
	}
	if (d_ver_table){
		d_ver_table->setNumRows(m->numRows());
		for (int i = 0; i < d_ver_table->numRows(); i++)
			d_ver_table->setText(i, 0, QString::number(i));
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
	ImageProfilesTool *tool = new ImageProfilesTool(d_app, g, d_matrix, d_hor_table, d_ver_table, d_status_target, d_status_slot);
	tool->append(QwtDoublePoint(xValue(), yValue()));
	return tool;
}

void ImageProfilesTool::append(const QwtDoublePoint &pos)
{
	ScreenPickerTool::append(pos);

	if (!d_app || !d_matrix)
		return;

	double x = pos.x();
	double y = pos.y();

	horSpinBox->setValue(x);
	vertSpinBox->setValue(y);

	x += 0.5*d_matrix->dx();
	y -= 0.5*d_matrix->dy();

	int row = qRound(fabs((y - d_matrix->yStart())/d_matrix->dy()));
	int col = qRound(fabs((x - d_matrix->xStart())/d_matrix->dx()));

	zLabel->setText(QLocale().toString(d_matrix->cell(row, col)));

	if (d_hor_table){
		for (int i = 0; i < d_matrix->numCols(); i++)
			d_hor_table->setCell(i, 1, d_matrix->cell(row, i));
		d_hor_table->notifyChanges();
	}

	if (d_ver_table){
		for (int i = 0; i < d_matrix->numRows(); i++)
			d_ver_table->setCell(i, 1, d_matrix->cell(i, col));
		d_ver_table->notifyChanges();
	}

	QLocale locale = d_app->locale();
	int prec = d_app->d_decimal_digits;
	emit statusText(QString("x=%1; y=%2; z=%3")
					.arg(locale.toString(pos.x(), 'G', prec))
					.arg(locale.toString(pos.y(), 'G', prec))
					.arg(locale.toString(d_matrix->cell(row, col), 'G', prec)));
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
}
