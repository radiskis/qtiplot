/***************************************************************************
    File                 : RangeSelectorTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for selecting ranges on curves.

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
#include "RangeSelectorTool.h"
#include "Graph.h"
#include "PlotCurve.h"
#include "MultiLayer.h"
#include <ErrorBarsCurve.h>
#include <ApplicationWindow.h>

#include <qwt_symbol.h>
#include <QPoint>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QEvent>
#include <QLocale>
#include <QTextStream>
#include <QWidget>
#include <QCheckBox>
#include <QDialogButtonBox>

RangeSelectorTool::RangeSelectorTool(Graph *graph, const QObject *status_target, const char *status_slot)
	: QwtPlotPicker(graph->canvas()),
	PlotToolInterface(graph)
{
	d_selection_dialog = NULL;
	d_selected_curve = NULL;
	for (int i = 0; i < d_graph->curveCount(); i++) {
		d_selected_curve = d_graph->curve(i);
		if (d_selected_curve && d_selected_curve->rtti() == QwtPlotItem::Rtti_PlotCurve
				&& d_selected_curve->dataSize() > 0)
			break;
		d_selected_curve = NULL;
	}
	if (!d_selected_curve) {
		QMessageBox::critical(d_graph, tr("QtiPlot - Warning"),
				tr("All the curves on this plot are empty!"));
		return;
	}

    d_enabled = true;
	d_visible = true;
	d_active_point = 0;
	d_inactive_point = d_selected_curve->dataSize() - 1;
	int marker_size = 20;

	d_active_marker.setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(QColor(255,255,255,0)),//QBrush(QColor(255,255,0,128)),
				QPen(Qt::red,2), QSize(marker_size,marker_size)));
	d_active_marker.setLineStyle(QwtPlotMarker::VLine);
	d_active_marker.setLinePen(QPen(Qt::red, 1, Qt::DashLine));
	d_inactive_marker.setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(QColor(255,255,255,0)), //QBrush(QColor(255,255,0,128)),
				QPen(Qt::black,2), QSize(marker_size,marker_size)));
	d_inactive_marker.setLineStyle(QwtPlotMarker::VLine);
	d_inactive_marker.setLinePen(QPen(Qt::black, 1, Qt::DashLine));
	d_active_marker.setValue(d_selected_curve->x(d_active_point),
			d_selected_curve->y(d_active_point));
	d_inactive_marker.setValue(d_selected_curve->x(d_inactive_point),
			d_selected_curve->y(d_inactive_point));
	d_active_marker.attach(d_graph);
	d_inactive_marker.attach(d_graph);

	setTrackerMode(QwtPicker::AlwaysOn);
	setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
	d_graph->canvas()->setCursor(QCursor(QPixmap(":/vizor.png"), -1, -1));
	d_graph->canvas()->setFocus();
	d_graph->replot();

	if (status_target)
		connect(this, SIGNAL(statusText(const QString&)), status_target, status_slot);
	emit statusText(tr("Click or use Ctrl+arrow key to select range (arrows select active cursor)!"));
}

RangeSelectorTool::~RangeSelectorTool()
{
	if (d_selection_dialog)
		delete d_selection_dialog;

	d_active_marker.detach();
	d_inactive_marker.detach();
	d_graph->canvas()->unsetCursor();
	d_graph->replot();
}

void RangeSelectorTool::pointSelected(const QPoint &pos)
{
	int dist, point;
	QwtPlotItem *item = d_graph->closestCurve(pos.x(), pos.y(), dist, point);
	if (!item || item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram || dist >= 5) // 5 pixels tolerance
		return;

	QwtPlotCurve *curve = (QwtPlotCurve *)item;
	if (curve == d_selected_curve)
		setActivePoint(point);
	else {
        d_selected_curve = curve;

        d_active_point = point;
		d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));

        d_active_point > 0 ? d_inactive_point = 0 : d_inactive_point = d_selected_curve->dataSize() - 1;
		d_inactive_marker.setValue(curve->x(d_inactive_point), curve->y(d_inactive_point));
		emitStatusText();
		emit changed();
	}
	d_graph->replot();
}

void RangeSelectorTool::setSelectedCurve(QwtPlotCurve *curve)
{
	if (!curve || d_selected_curve == curve || !d_enabled)
		return;
	d_selected_curve = curve;
	d_active_point = 0;
	d_inactive_point = d_selected_curve->dataSize() - 1;
	d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));
	d_inactive_marker.setValue(d_selected_curve->x(d_inactive_point), d_selected_curve->y(d_inactive_point));
	emitStatusText();
	emit changed();
}

void RangeSelectorTool::setActivePoint(int point)
{
	if (!d_enabled || point == d_active_point)
		return;
	d_active_point = point;
	d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));
	emitStatusText();
	emit changed();
}

void RangeSelectorTool::emitStatusText()
{
    QLocale locale = d_graph->multiLayer()->locale();
	if (((PlotCurve *)d_selected_curve)->type() == Graph::Function ||
		((PlotCurve *)d_selected_curve)->type() == Graph::Histogram){
		 double x = d_selected_curve->x(d_active_point);
		 double y = d_selected_curve->y(d_active_point);
         emit statusText(QString("%1 <=> %2[%3]: x=%4; y=%5; dx=%6; dy=%7")
			.arg(d_active_marker.xValue() > d_inactive_marker.xValue() ? tr("Right") : tr("Left"))
			.arg(d_selected_curve->title().text())
			.arg(d_active_point + 1)
			.arg(locale.toString(x, 'G', 16))
			.arg(locale.toString(y, 'G', 16))
			.arg(locale.toString(fabs(x - d_selected_curve->x(d_inactive_point)), 'G', 16))
			.arg(locale.toString(fabs(y - d_selected_curve->y(d_inactive_point)), 'G', 16)));
    } else if (((PlotCurve *)d_selected_curve)->type() == Graph::ErrorBars){
         emit statusText(QString("%1 <=> %2[%3]: x=%4; y=%5; err=%6")
			.arg(d_active_marker.xValue() > d_inactive_marker.xValue() ? tr("Right") : tr("Left"))
			.arg(d_selected_curve->title().text())
			.arg(d_active_point + 1)
			.arg(locale.toString(d_selected_curve->x(d_active_point), 'G', 16))
			.arg(locale.toString(d_selected_curve->y(d_active_point), 'G', 16))
			.arg(locale.toString(((ErrorBarsCurve*)d_selected_curve)->errorValue(d_active_point), 'G', 16)));
    } else {
		DataCurve *c = (DataCurve*)d_selected_curve;
		Table *t = c->table();
		Table *xt = c->xTable();
		if (!t || !xt)
			return;

		int row = c->tableRow(d_active_point);
		double x = c->x(d_active_point);
		double y = c->y(d_active_point);

		ApplicationWindow *app = d_graph->multiLayer()->applicationWindow();
		int prec = 15;
		if (app)
			prec = app->d_decimal_digits;

		int xcol = xt->colIndex(c->xColumnName());
		QString xs = locale.toString(x - c->xOffset(), 'G', prec);
		if (xt->columnType(xcol) != Table::Numeric)
			xs = xt->text(row, xcol);

		int ycol = t->colIndex(c->title().text());
		QString ys = locale.toString(y - c->yOffset(), 'G', prec);
		if (t->columnType(ycol) != Table::Numeric)
			ys = t->text(row, ycol);

		emit statusText(QString("%1 <=> %2[%3]: x=%4; y=%5; dx=%6; dy=%7")
			.arg(d_active_marker.xValue() > d_inactive_marker.xValue() ? tr("Right") : tr("Left"))
			.arg(c->title().text())
			.arg(row + 1)
			.arg(xs)
			.arg(ys)
			.arg(locale.toString(fabs(x - c->x(d_inactive_point)), 'G', prec))
			.arg(locale.toString(fabs(y - c->y(d_inactive_point)), 'G', prec)));
    }
}

void RangeSelectorTool::switchActiveMarker()
{
	QwtDoublePoint tmp = d_active_marker.value();
	d_active_marker.setValue(d_inactive_marker.value());
	d_inactive_marker.setValue(tmp);
	int tmp2 = d_active_point;
	d_active_point = d_inactive_point;
	d_inactive_point = tmp2;
	d_graph->replot();

	emitStatusText();
}

bool RangeSelectorTool::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type()) {
		case QEvent::KeyPress:
			if (keyEventFilter((QKeyEvent*)event))
				return true;
			break;
		default:
			break;
	}
	return QwtPlotPicker::eventFilter(obj, event);
}

bool RangeSelectorTool::keyEventFilter(QKeyEvent *ke)
{
	switch(ke->key()) {
		case Qt::Key_Up:
			{
				int n_curves = d_graph->curveCount();
				int start = d_graph->curveIndex(d_selected_curve) + 1;
				for (int i = start; i < start + n_curves; ++i)
					if (d_graph->curve(i % n_curves)->dataSize() > 0) {
						setSelectedCurve(d_graph->curve(i % n_curves));
						break;
					}
				d_graph->replot();
				return true;
			}
		case Qt::Key_Down:
			{
				int n_curves = d_graph->curveCount();
				int start = d_graph->curveIndex(d_selected_curve) + n_curves - 1;
				for (int i = start; i > start - n_curves; --i)
					if (d_graph->curve(i % n_curves)->dataSize() > 0) {
						setSelectedCurve(d_graph->curve(i % n_curves));
						break;
					}
				d_graph->replot();
				return true;
			}
		case Qt::Key_Right:
		case Qt::Key_Plus:
			{
				if (ke->modifiers() & Qt::ControlModifier) {
					int n_points = d_selected_curve->dataSize();
					setActivePoint((d_active_point + 1) % n_points);
					d_graph->replot();
				} else
					switchActiveMarker();
				return true;
			}
		case Qt::Key_Left:
		case Qt::Key_Minus:
			{
				if (ke->modifiers() & Qt::ControlModifier) {
					int n_points = d_selected_curve->dataSize();
					setActivePoint((d_active_point - 1 + n_points) % n_points);
					d_graph->replot();
				} else
					switchActiveMarker();
				return true;
			}
		default:
			break;
	}
	return false;
}

void RangeSelectorTool::cutSelection()
{
	if (!d_selected_curve)
        return;

	if (mightNeedMultipleSelection())
		showSelectionDialog(Cut);
	else {
		copySelectedCurve();
		clearSelectedCurve();
	}
}

void RangeSelectorTool::copySelection()
{
    if (!d_selected_curve)
        return;

	if (mightNeedMultipleSelection())
		showSelectionDialog();
	else
		copySelectedCurve();
}

void RangeSelectorTool::clearSelection()
{
    if (!d_selected_curve)
        return;

	if (mightNeedMultipleSelection())
		showSelectionDialog(Delete);
	else
		clearSelectedCurve();
}

void RangeSelectorTool::copySelectedCurve()
{
    if (!d_selected_curve)
        return;

	int start_point = QMIN(d_active_point, d_inactive_point);
	int end_point = QMAX(d_active_point, d_inactive_point);
	QLocale locale = d_graph->multiLayer()->locale();
	ApplicationWindow *app = d_graph->multiLayer()->applicationWindow();
	int prec = 15;
	if (app)
		prec = app->d_decimal_digits;

	QString text;
	for (int i = start_point; i <= end_point; i++){
		text += locale.toString(d_selected_curve->x(i), 'G', prec) + "\t";
		text += locale.toString(d_selected_curve->y(i), 'G', prec) + "\n";
	}
	QApplication::clipboard()->setText(text);
}

bool RangeSelectorTool::mightNeedMultipleSelection()
{
	int count = 0;
	if (((PlotCurve*)d_selected_curve)->type() != Graph::Function){
		QString xCol = ((DataCurve*)d_selected_curve)->xColumnName();
		for (int i = 0; i < d_graph->curveCount(); i++){
			PlotCurve *curve = (PlotCurve*)d_graph->curve(i);
			if (curve->type() != Graph::Function &&
				((DataCurve *)curve)->xColumnName() == xCol)
				count++;
		}
	}
	return count > 1 ? true : false;
}

void RangeSelectorTool::cutMultipleSelection()
{
	copyMultipleSelection();
	clearMultipleSelection();
}

void RangeSelectorTool::copyMultipleSelection()
{
	if (d_selection_dialog)
		d_selection_dialog->hide();

	int start_point = QMIN(d_active_point, d_inactive_point);
	int end_point = QMAX(d_active_point, d_inactive_point);
	QLocale locale = d_graph->multiLayer()->locale();
	QString text;
	QList <PlotCurve*> cvs;
	for (int j = 0; j < d_selection_lst.count(); j++){
		QCheckBox *box = d_selection_lst[j];
		if (box->isChecked())
			cvs << (PlotCurve*)d_graph->curve(box->text());
	}

	int curves = cvs.size();
	for (int i = start_point; i <= end_point; i++){
		text += locale.toString(d_selected_curve->x(i), 'G', 16);
		for (int j = 0; j < curves; j++){
			PlotCurve *curve = cvs[j];
			if (curve->type() == Graph::ErrorBars)
				text += "\t" + locale.toString(((ErrorBarsCurve*)curve)->errorValue(i), 'G', 16);
			else
				text += "\t" + locale.toString(curve->y(i), 'G', 16);
		}
		text += "\n";
	}

	QApplication::clipboard()->setText(text);
}

void RangeSelectorTool::clearMultipleSelection()
{
	if (d_selection_dialog)
		d_selection_dialog->hide();

	int start_point = QMIN(d_active_point, d_inactive_point);
	int start_row = ((DataCurve*)d_selected_curve)->tableRow(start_point);
	int end_point = QMAX(d_active_point, d_inactive_point);
	int end_row = ((DataCurve*)d_selected_curve)->tableRow(end_point);

	for (int j = 0; j < d_selection_lst.count(); j++){
		QCheckBox *box = d_selection_lst[j];
		if (box->isChecked()){
			PlotCurve *curve = (PlotCurve*)d_graph->curve(box->text());
			if (!curve || curve->type() == Graph::Function)
				continue;

			Table *t = ((DataCurve*)curve)->table();
			if (!t)
				continue;
			QString name = curve->title().text();
			int col = t->colIndex(name);
			if (t->isReadOnlyColumn(col)){
				QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
				tr("The column '%1' is read-only! Operation aborted!").arg(name));
				continue;
			}

			for (int i = start_row; i <= end_row; i++)
				t->setText(i, col, "");
			t->notifyChanges();
		}
	}

	bool ok_update = (end_point - start_point + 1) < d_selected_curve->dataSize() ? true : false;
	if (ok_update){
		d_active_point = 0;
		d_inactive_point = d_selected_curve->dataSize() - 1;
		d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));
		d_inactive_marker.setValue(d_selected_curve->x(d_inactive_point), d_selected_curve->y(d_inactive_point));
		emitStatusText();
		emit changed();
		d_graph->replot();
	}
}

void RangeSelectorTool::showSelectionDialog(RangeEditOperation op)
{
	d_selection_lst.clear();

	if (d_selection_dialog)
		delete d_selection_dialog;

	d_selection_dialog = new QDialog(0, Qt::Tool);
	d_selection_dialog->setAttribute(Qt::WA_DeleteOnClose);
	if (op > Copy)
		d_selection_dialog->setWindowTitle(tr("Remove data from curves?"));
	else
		d_selection_dialog->setWindowTitle(tr("Copy data to clipboard?"));

	d_selection_dialog->setModal(true);
	d_selection_dialog->setActiveWindow();

	QVBoxLayout *vb = new QVBoxLayout(d_selection_dialog);

	QString xCol = ((DataCurve*)d_selected_curve)->xColumnName();
	for (int i = 0; i < d_graph->curveCount(); i++){
		PlotCurve *curve = (PlotCurve*)d_graph->curve(i);
		if (curve->type() != Graph::Function &&
			((DataCurve *)curve)->xColumnName() == xCol){
			QCheckBox *box = new QCheckBox(curve->title().text());
			box->setChecked(true);
			vb->addWidget(box);
			d_selection_lst << box;

			QList<ErrorBarsCurve *> errorBars = ((DataCurve *)curve)->errorBarsList();
			foreach(ErrorBarsCurve *err, errorBars){
				box = new QCheckBox(err->title().text());
				box->setChecked(true);
				vb->addWidget(box);
				d_selection_lst << box;
			}
		}
	}
	vb->addStretch();

	QDialogButtonBox *btnBox = new QDialogButtonBox ();

	QPushButton *closeBtn = btnBox->addButton(QDialogButtonBox::Ok);
	switch (op){
		case Copy:
			connect(closeBtn, SIGNAL(clicked()), this, SLOT(copyMultipleSelection()));
		break;

		case Cut:
			connect(closeBtn, SIGNAL(clicked()), this, SLOT(cutMultipleSelection()));
		break;

		case Delete:
			connect(closeBtn, SIGNAL(clicked()), this, SLOT(clearMultipleSelection()));
		break;
	}

	QPushButton *cancelBtn = btnBox->addButton(QDialogButtonBox::Cancel);
	connect(cancelBtn, SIGNAL(clicked()), d_selection_dialog, SLOT(close()));
	vb->addWidget(btnBox);

	d_selection_dialog->show();
}

void RangeSelectorTool::clearSelectedCurve()
{
	if (!d_selected_curve)
		return;

	if (((PlotCurve *)d_selected_curve)->type() != Graph::Function){
        Table *t = ((DataCurve*)d_selected_curve)->table();
        if (!t)
            return;

		if (t->isReadOnlyColumn(t->colIndex(((DataCurve *)d_selected_curve)->xColumnName()))){
    		QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
        	tr("The column '%1' is read-only! Operation aborted!").arg(((DataCurve *)d_selected_curve)->xColumnName()));
			return;
		} else if (t->isReadOnlyColumn(t->colIndex(d_selected_curve->title().text()))){
    		QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
			tr("The column '%1' is read-only! Operation aborted!").arg(d_selected_curve->title().text()));
			return;
   		}

        int start_point = QMIN(d_active_point, d_inactive_point);
        int start_row = ((DataCurve*)d_selected_curve)->tableRow(start_point);
        int end_point = QMAX(d_active_point, d_inactive_point);
        int end_row = ((DataCurve*)d_selected_curve)->tableRow(end_point);
        int col = t->colIndex(d_selected_curve->title().text());
        bool ok_update = (end_point - start_point + 1) < d_selected_curve->dataSize() ? true : false;
        for (int i = start_row; i <= end_row; i++){
            t->setText(i, col, "");
        }
        t->notifyChanges();

        if (ok_update){
            d_active_point = 0;
            d_inactive_point = d_selected_curve->dataSize() - 1;
            d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));
            d_inactive_marker.setValue(d_selected_curve->x(d_inactive_point), d_selected_curve->y(d_inactive_point));
            emitStatusText();
            emit changed();
            d_graph->replot();
        }
    }
}

// Paste text from the clipboard
void RangeSelectorTool::pasteSelection()
{
	QString text = QApplication::clipboard()->text();
	if (text.isEmpty())
		return;

    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function ||
		((PlotCurve *)d_selected_curve)->type() == Graph::Graph::ErrorBars)
        return;

    Table *t = ((DataCurve*)d_selected_curve)->table();
    if (!t)
        return;

	if (t->isReadOnlyColumn(t->colIndex(((DataCurve *)d_selected_curve)->xColumnName()))){
    	QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
        tr("The column '%1' is read-only! Operation aborted!").arg(((DataCurve *)d_selected_curve)->xColumnName()));
		return;
	} else if (t->isReadOnlyColumn(t->colIndex(d_selected_curve->title().text()))){
    	QMessageBox::warning(d_graph, tr("QtiPlot - Warning"),
		tr("The column '%1' is read-only! Operation aborted!").arg(d_selected_curve->title().text()));
		return;
   	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QTextStream ts( &text, QIODevice::ReadOnly );
    int start_point = QMIN(d_active_point, d_inactive_point);
    int start_row = ((DataCurve*)d_selected_curve)->tableRow(start_point);
    int end_point = QMAX(d_active_point, d_inactive_point);
    int end_row = ((DataCurve*)d_selected_curve)->tableRow(end_point);
    int col = t->colIndex(d_selected_curve->title().text());

    int prec; char f;
    t->columnNumericFormat(col, &f, &prec);
    QLocale locale = d_graph->multiLayer()->locale();
    for (int i = start_row; i <= end_row; i++){
        QString s = ts.readLine();
        if (s.isEmpty())
            continue;

        QStringList cellTexts = s.split("\t");
        if (cellTexts.count() >= 2){
            bool numeric;
            double value = locale.toDouble(cellTexts[1], &numeric);
			if (numeric)
                t->setText(i, col, locale.toString(value, f, prec));
			else
                t->setText(i, col, cellTexts[1]);
        }

        if(ts.atEnd())
            break;
    }

    t->notifyChanges();

    d_active_marker.setValue(d_selected_curve->x(d_active_point), d_selected_curve->y(d_active_point));
    d_inactive_marker.setValue(d_selected_curve->x(d_inactive_point), d_selected_curve->y(d_inactive_point));
    emitStatusText();
    emit changed();
    d_graph->replot();

	QApplication::restoreOverrideCursor();
}

void RangeSelectorTool::setCurveRange()
{
    if (!d_selected_curve)
        return;

    if (((PlotCurve *)d_selected_curve)->type() != Graph::Function){
        ((DataCurve*)d_selected_curve)->setRowRange(QMIN(d_active_point, d_inactive_point),
                                    QMAX(d_active_point, d_inactive_point));
        d_graph->updatePlot();
        d_graph->notifyChanges();
    }
}

void RangeSelectorTool::setEnabled(bool on)
{
    d_enabled = on;
    if (on)
		d_graph->canvas()->setCursor(QCursor(QPixmap(":/vizor.png"), -1, -1));
}

void RangeSelectorTool::setVisible(bool on)
{
	if (d_visible == on)
		return;

	d_visible = on;

    if (on){
		setTrackerMode(QwtPicker::AlwaysOn);
		d_graph->canvas()->setCursor(QCursor(QPixmap(":/vizor.png"), -1, -1));
		d_active_marker.attach(d_graph);
		d_inactive_marker.attach(d_graph);
	} else {
		d_enabled = false;
		setTrackerMode(QwtPicker::AlwaysOff);
        d_active_marker.detach();
		d_inactive_marker.detach();
		d_graph->canvas()->unsetCursor();
	}
	d_graph->replot();
}
