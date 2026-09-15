/***************************************************************************
File                 : AssociationsDialog.cpp
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2006 - 2012 by Ion Vasilief
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Plot associations dialog
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
#include "AssociationsDialog.h"
#include "Table.h"
#include "FunctionCurve.h"
#include "PlotCurve.h"
#include "BoxCurve.h"
#include "ErrorBarsCurve.h"
#include "PieCurve.h"
#include "QwtHistogram.h"
#include "VectorCurve.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QEvent>
#include <QLayout>
#include <QApplication>

AssociationsDialog::AssociationsDialog( QWidget* parent, Qt::WindowFlags fl )
	: QDialog( parent, fl ), graph(nullptr)
{
	setObjectName("AssociationsDialog");
	setWindowTitle(tr("QtiPlot - Plot Associations"));
	setSizeGripEnabled(true);
	setFocus();

	QVBoxLayout *vl = new QVBoxLayout();

	QHBoxLayout *hbox1 = new QHBoxLayout ();
	hbox1->addWidget(new QLabel(tr("Spreadsheet: ")));

	tableCaptionLabel = new QLabel();
    hbox1->addWidget(tableCaptionLabel);
    vl->addLayout(hbox1);

	table = new QTableWidget(3, 5);
	table->horizontalHeader()->setSectionsClickable(false);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->verticalHeader()->hide();
	table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	table->setMaximumHeight(8*table->rowHeight(0));
	table->setHorizontalHeaderLabels(QStringList() << tr("Column") << tr("X") << tr("Y") << tr("xErr") << tr("yErr"));
    vl->addWidget(table);

	associations = new QListWidget();
	associations->setSelectionMode (QListWidget::SingleSelection);
    vl->addWidget(associations);

	btnApply = new QPushButton(tr("&Update curves"));
	btnOK = new QPushButton(tr("&OK"));
	btnOK->setDefault(true);
	btnCancel = new QPushButton(tr("&Cancel"));

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addStretch();
    hbox2->addWidget(btnApply);
    hbox2->addWidget(btnOK);
    hbox2->addWidget(btnCancel);
    vl->addStretch();
    vl->addLayout(hbox2);
	setLayout(vl);

	active_table = nullptr;

	connect(associations, &QListWidget::currentRowChanged, this, &AssociationsDialog::updateTable);
	connect(btnOK, &QPushButton::clicked, this, &AssociationsDialog::accept);
	connect(btnCancel, &QPushButton::clicked, this, &AssociationsDialog::close);
	connect(btnApply, &QPushButton::clicked, this, &AssociationsDialog::updateCurves);
}

void AssociationsDialog::accept()
{
	updateCurves();
	close();
}

void AssociationsDialog::updateCurves()
{
	if (!graph)
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	for (int i = 0; i < associations->count(); i++)
		changePlotAssociation(i, plotAssociation(associations->item(i)->text()));
	graph->updatePlot();

	QApplication::restoreOverrideCursor();
}

void AssociationsDialog::changePlotAssociation(int curve, const QStringList& ass)
{
	DataCurve *c = graph->dataCurve(curvesIndicesList[curve]);
	if (!c)
		return;

	if (c->plotAssociation() == ass)
		return;

	QStringList lst = ass;
	if (lst.count() == 1){
		c->setTitle(lst[0]);
		if (c->type() == Graph::Box){
			if (BoxCurve *bc = dynamic_cast<BoxCurve *>(c))
				bc->loadData();
		} else if (c->type() == Graph::Pie){
			if (PieCurve *pc = dynamic_cast<PieCurve *>(c))
				pc->loadData();
		} else if (c->type() == Graph::Histogram){
			if (QwtHistogram *h = dynamic_cast<QwtHistogram *>(c))
				h->loadData();
		}
	} else if (lst.count() == 2){
		c->setXColumnName(lst[0].remove("(X)"));
		c->setTitle(lst[1].remove("(Y)"));
		c->loadData();
	} else if (lst.count() == 3){//curve with error bars
		ErrorBarsCurve *er = dynamic_cast<ErrorBarsCurve *>(c);
		if (!er)
			return;
		QString xColName = lst[0].remove("(X)");
		QString yColName = lst[1].remove("(Y)");
		QString erColName = lst[2].remove("(xErr)").remove("(yErr)");
		DataCurve *master_curve = graph->masterCurve(xColName, yColName);
		if (!master_curve)
			return;

		int type = ErrorBarsCurve::Vertical;
		if (ass.join(",").contains("(xErr)"))
			type = ErrorBarsCurve::Horizontal;
		er->setDirection(type);
		er->setTitle(erColName);
		if (master_curve != er->masterCurve())
			er->setMasterCurve(master_curve);
		else
			er->loadData();
	} else if (lst.count() == 4){
		VectorCurve *v = dynamic_cast<VectorCurve *>(c);
		if (!v)
			return;
		v->setXColumnName(lst[0].remove("(X)"));
		v->setTitle(lst[1].remove("(Y)"));

		QString xEndCol = lst[2].remove("(X)").remove("(A)");
		QString yEndCol = lst[3].remove("(Y)").remove("(M)");
		if (v->vectorEndXAColName() != xEndCol || v->vectorEndYMColName() != yEndCol)
			v->setVectorEnd(xEndCol, yEndCol);
		else
			v->loadData();
	}
	graph->notifyChanges();
}

QStringList AssociationsDialog::plotAssociation(const QString& text)
{
	QString s = text;
	QStringList lst = s.split(": ", Qt::SkipEmptyParts);
	QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

	QString tableName = lst[0];
	QStringList ass = QStringList() << tableName + "_" + cols[0].replace(".", ",");
	for (int i = 1; i < (int)cols.count(); i++ )
		ass << tableName + "_" + cols[i].replace(".", ",");

	return ass;
}

void AssociationsDialog::initTablesList(QList<MdiSubWindow *> lst, int curve)
{
	tables = lst;
	active_table = nullptr;

	if (curve < 0 || curve >= (int)associations->count())
		curve = 0;

	associations->setCurrentRow (curve);
}

Table * AssociationsDialog::findTable(int index)
{
	QString text = associations->item(index)->text();
	QStringList lst= text.split(":", Qt::SkipEmptyParts);
	for (MdiSubWindow *w : tables){
		if (w->objectName() == lst[0])
			return qobject_cast<Table *>(w);
	}
	return nullptr;
}

void AssociationsDialog::updateTable(int index)
{
	Table *t = findTable(index);
	if (!t)
		return;

	if (active_table != t){
		active_table = t;
		tableCaptionLabel->setText(t->objectName());
		table->clearContents();
		table->setRowCount(t->numCols());

		QStringList colNames = t->colNames();
		for (int i=0; i<table->rowCount(); i++ ){
			QTableWidgetItem *cell = new QTableWidgetItem(colNames[i].replace(",", "."));
			cell->setBackground (QBrush(Qt::lightGray));
			cell->setFlags (Qt::ItemIsEnabled);
			table->setItem(i, 0, cell);
			}

		for (int j=1; j < table->columnCount(); j++){
			for (int i=0; i < table->rowCount(); i++ )
				{
				QTableWidgetItem *cell = new QTableWidgetItem();
				cell->setBackground (QBrush(Qt::lightGray));
				table->setItem(i, j, cell);

				QCheckBox* cb = new QCheckBox(table);
				cb->installEventFilter(this);
				table->setCellWidget(i, j, cb);
				}
			}
		}
	updateColumnTypes();
}

void AssociationsDialog::updateColumnTypes()
{
	QString text = associations->currentItem()->text();
	QStringList lst = text.split(": ", Qt::SkipEmptyParts);
	QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

	QString xColName, yColName;

	int n = (int)cols.count();
	if (n >= 2){
		xColName = cols[0].remove("(X)");
		yColName = cols[1].remove("(Y)");

		table->showColumn(1);
		table->hideColumn(3);
		table->hideColumn(4);
		}
	else if (n == 1){//box plots
		yColName = cols[0];
		table->hideColumn(1);
		table->hideColumn(3);
		table->hideColumn(4);
		}

	for (int i=0; i < table->rowCount(); i++ ){
		if (QCheckBox *cb1 = qobject_cast<QCheckBox *>(table->cellWidget(i, 1)))
			cb1->setChecked(table->item(i, 0)->text() == xColName);

		if (QCheckBox *cb2 = qobject_cast<QCheckBox *>(table->cellWidget(i, 2)))
			cb2->setChecked(table->item(i, 0)->text() == yColName);
	}

	bool xerr = false, yerr = false, vectors = false;
	QString errColName, xEndColName, yEndColName;
	if (n > 2){
		table->showColumn(3);
		table->showColumn(4);

		if (cols[2].contains("(xErr)") || cols[2].contains("(yErr)")){//if error bars
			table->horizontalHeaderItem(3)->setText(tr("xErr"));
			table->horizontalHeaderItem(4)->setText(tr("yErr"));
		}

		if (cols[2].contains("(xErr)")){
			xerr = true;
			errColName = cols[2].remove("(xErr)");
		} else if (cols[2].contains("(yErr)")){
			yerr = true;
			errColName = cols[2].remove("(yErr)");
		} else if (cols.count() > 3 && cols[2].contains("(X)") && cols[3].contains("(Y)")){
			vectors = true;
			xEndColName = cols[2].remove("(X)");
			yEndColName = cols[3].remove("(Y)");
			table->horizontalHeaderItem(3)->setText(tr("xEnd"));
			table->horizontalHeaderItem(4)->setText(tr("yEnd"));
		} else if (cols.count() > 3 && cols[2].contains("(A)") && cols[3].contains("(M)")){
			vectors = true;
			xEndColName = cols[2].remove("(A)");
			yEndColName = cols[3].remove("(M)");
			table->horizontalHeaderItem(3)->setText(tr("Angle"));
			table->horizontalHeaderItem(4)->setText(tr("Magn.","Magnitude, vector length"));
		}
	}

	for (int i=0; i < table->rowCount(); i++){
		if (QCheckBox *cb3 = qobject_cast<QCheckBox *>(table->cellWidget(i, 3))){
			if (xerr || vectors)
				cb3->setChecked(table->item(i,0)->text() == errColName || table->item(i,0)->text() == xEndColName);
			else
				cb3->setChecked(false);
		}

		if (QCheckBox *cb4 = qobject_cast<QCheckBox *>(table->cellWidget(i, 4))){
			if (yerr || vectors)
				cb4->setChecked(table->item(i,0)->text() == errColName || table->item(i,0)->text() == yEndColName);
			else
				cb4->setChecked(false);
		}

		if (QCheckBox *cb1 = qobject_cast<QCheckBox *>(table->cellWidget(i, 1))){
			cb1->setEnabled(!xerr && !yerr);
		}
		if (QWidget *w2 = table->cellWidget(i, 2)){
			w2->setEnabled(!xerr && !yerr);
		}
	}
}

void AssociationsDialog::uncheckCol(int col)
{
	for (int i = 0; i < table->rowCount(); i++){
		if (QCheckBox *it = qobject_cast<QCheckBox *>(table->cellWidget(i, col)))
			it->setChecked(false);
	}
}

void AssociationsDialog::setGraph(Graph *g)
{
	graph = g;

	int index = 0;
	for (int i = 0; i < graph->curveCount(); i++){
		QwtPlotItem *it = graph->plotItem(i);
		if (!it)
			continue;

		DataCurve *dc = dynamic_cast<DataCurve *>(it);
		if (!dc || !dc->table()){
			index++;
			continue;
		}

		QStringList lst = dc->plotAssociation();
		QString tableName = dc->table()->objectName();
		lst.replaceInStrings(tableName + "_", "").replaceInStrings(",", ".");
		plotAssociationsList << tableName + ": " + lst.join(",");
		curvesIndicesList << index;
		index++;
	}
	associations->addItems(plotAssociationsList);
	associations->setMaximumHeight((plotAssociationsList.count() + 1)*associations->visualItemRect(associations->item(0)).height());
}

void AssociationsDialog::updatePlotAssociation(int row, int col)
{
	int index = associations->currentRow();
	QString text = associations->currentItem()->text();
	QStringList lst = text.split(": ", Qt::SkipEmptyParts);
	QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

	if (col == 1){
		cols[0] = table->item(row, 0)->text() + "(X)";
		text = lst[0] + ": " + cols.join(",");
		}
	else if (col == 2){
		if (cols.count() >= 2){
			cols[1] = table->item(row, 0)->text() + "(Y)";
			text = lst[0] + ": " + cols.join(",");
		} else // box or pie plots
			text = lst[0] + ": " + table->item(row, 0)->text();
		}
	else if (col == 3){
		if (text.contains("(A)")){//vect XYAM curve
			cols[2] = table->item(row, 0)->text() + "(A)";
			text = lst[0] + ": " + cols.join(",");
			}
		else if (!text.contains("(A)") && text.count("(X)") == 1){
			cols[2] = table->item(row, 0)->text() + "(xErr)";
			text = lst[0] + ": " + cols.join(",");
			uncheckCol(4);
			}
		else if (text.count("(X)") == 2){//vect XYXY curve
			cols[2] = table->item(row, 0)->text() + "(X)";
			text = lst[0] + ": " + cols.join(",");
			}
		}
	else if (col == 4){
		if (text.contains("(M)")){//vect XYAM curve
			cols[3] = table->item(row, 0)->text() + "(M)";
			text = lst[0] + ": " + cols.join(",");
			}
		else if (!text.contains("(M)") && text.count("(X)") == 1){
			cols[2] = table->item(row, 0)->text() + "(yErr)";
			text = lst[0] + ": " + cols.join(",");
			uncheckCol(3);
			}
		else if (text.count("(Y)") == 2){//vect XYXY curve
			cols[3] = table->item(row, 0)->text() + "(Y)";
			text = lst[0] + ": " + cols.join(",");
			}
		}

	//change associations for error bars depending on the curve "index"
	QString old_as = plotAssociationsList[index];
	for (int i = 0; i<(int)plotAssociationsList.count(); i++){
		QString as = plotAssociationsList[i];
		if (as.contains(old_as) && (as.contains("(xErr)") || as.contains("(yErr)"))){
			QStringList ls = as.split(",", Qt::SkipEmptyParts);
			as = text + "," + ls[2];
			plotAssociationsList[i] = as;
			associations->item(i)->setText(as);
			}
		}

	plotAssociationsList [index] = text;
	associations->item(index)->setText(text);
}

bool AssociationsDialog::eventFilter(QObject *object, QEvent *e)
{
	QCheckBox *targetCb = qobject_cast<QCheckBox *>(object);
	if (!targetCb)
		return false;

	if (e->type() == QEvent::MouseButtonPress){
		if (targetCb->isChecked() || !targetCb->isEnabled())
			return true;

		int col = 0, row = 0;
		for (int j = 1; j < table->columnCount(); j++){
			for (int i = 0; i < table->rowCount(); i++){
				if (table->cellWidget(i, j) == targetCb){
					row = i;
					col = j;
					break;
				}
			}
		}

		uncheckCol(col);
		targetCb->setChecked(true);

		updatePlotAssociation(row, col);
		return true;
	}
	else if (e->type() == QEvent::MouseButtonDblClick)
		return true;
	else
		return false;
}
