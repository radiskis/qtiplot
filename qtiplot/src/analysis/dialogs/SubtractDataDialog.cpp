/***************************************************************************
	File                 : SubtractDataDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Subtract reference data dialog

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
#include <SubtractDataDialog.h>
#include <ApplicationWindow.h>
#include <Folder.h>
#include <Graph.h>
#include <DoubleSpinBox.h>
#include <PlotCurve.h>
#include <RangeSelectorTool.h>

#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>

SubtractDataDialog::SubtractDataDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setObjectName( "SubtractDataDialog" );
	setWindowTitle(tr("QtiPlot") + " - " + tr("Math on Data Sets"));
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled( true );

    QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);
	QLabel *l1 = new QLabel(tr("Cu&rve"));
	gl1->addWidget(l1, 0, 0);

	boxInputName = new QComboBox();
	l1->setBuddy(boxInputName);
	gl1->addWidget(boxInputName, 0, 1);

	QLabel *l2 = new QLabel(tr("&Operator"));
	gl1->addWidget(l2, 1, 0);

	boxOperation = new QComboBox();
	boxOperation->addItems(QStringList() << "+" << "-" << "/" << "*");
	boxOperation->setCurrentIndex(1);
	l2->setBuddy(boxOperation);
	gl1->addWidget(boxOperation, 1, 1);

	btnDataset = new QRadioButton(tr("&Data set"));
	btnDataset->setChecked(true);
	gl1->addWidget(btnDataset, 2, 0);

	boxReferenceName = new QComboBox();
	gl1->addWidget(boxReferenceName, 2, 1);

	boxColumnName = new QComboBox();
	gl1->addWidget(boxColumnName, 2, 2);

	boxCurrentFolder = new QCheckBox(tr("Current &folder"));
	gl1->addWidget(boxCurrentFolder, 3, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	boxReferenceName->addItems(app->tableNames());
	updateTableColumns(0);

	btnValue = new QRadioButton(tr("&Value"));
	gl1->addWidget(btnValue, 4, 0);

	boxOffset = new DoubleSpinBox();
	boxOffset->setDecimals(app->d_decimal_digits);
	boxOffset->setLocale(app->locale());
	boxOffset->setEnabled(false);
	gl1->addWidget(boxOffset, 4, 1);
	gl1->setRowStretch(5, 1);

	buttonApply = new QPushButton(tr( "&Apply" ));
	buttonApply->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
	vl->addWidget(buttonApply);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1, 1);
    hb->addLayout(vl);

	connect( btnDataset, SIGNAL( toggled(bool) ), boxOffset, SLOT( setDisabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxReferenceName, SLOT( setEnabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxColumnName, SLOT( setEnabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxCurrentFolder, SLOT( setEnabled(bool) ) );

	connect( btnValue, SIGNAL( toggled(bool) ), boxOffset, SLOT( setEnabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxReferenceName, SLOT( setDisabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxColumnName, SLOT( setDisabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxCurrentFolder, SLOT( setDisabled(bool) ) );

	connect( boxCurrentFolder, SIGNAL( toggled(bool) ), this, SLOT( setCurrentFolder(bool) ) );
	connect( boxReferenceName, SIGNAL( activated(int) ), this, SLOT( updateTableColumns(int) ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( interpolate() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( close() ) );
}

void SubtractDataDialog::setCurrentFolder(bool on)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	boxReferenceName->clear();
	if (on){
		Folder *f = app->currentFolder();
		if (f){
			QStringList tables;
			foreach (MdiSubWindow *w, f->windowsList()){
				if (w->inherits("Table"))
					tables << w->objectName();
			}
			boxReferenceName->addItems(tables);
		}
	} else
		boxReferenceName->addItems(app->tableNames());

	updateTableColumns(0);
}

void SubtractDataDialog::updateTableColumns(int tabnr)
{
	boxColumnName->clear();

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	Table *t = app->table(boxReferenceName->itemText(tabnr));
	if (!t)
		return;

	QStringList columns;
	for (int i = 0; i < t->numCols(); i++){
		if (t->colPlotDesignation(i) != Table::X)
			columns << t->colLabel(i);
	}
	boxColumnName->addItems(columns);
}

double SubtractDataDialog::combineValues(double v1, double v2)
{
	double res = v1;
	switch(boxOperation->currentIndex()){
		case 0:
			res += v2;
		break;
		case 1:
			res -= v2;
		break;
		case 2:
			res /= v2;
		break;
		case 3:
			res *= v2;
		break;
	}
	return res;
}

void SubtractDataDialog::interpolate()
{
	if (!graph)
		return;

	QString name = boxInputName->currentText();
	DataCurve *c = graph->dataCurve(graph->curveIndex(name.left(name.indexOf(" ["))));
	if (!c)
		return;

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	Table *inputTable = c->table();
	if (!inputTable)
		return;

	int startRow = c->startRow(), endRow = c->endRow();
	if (startRow < 0)
		startRow = 0;
	if (endRow < 0)
		endRow = c->dataSize() - 1;

	int xCol = inputTable->colIndex(c->xColumnName());
	int yCol = inputTable->colIndex(c->title().text());
	if (btnValue->isChecked()){
		double offset = boxOffset->value();
		for (int i = startRow; i <= endRow; i++){
			if (!inputTable->text(i, yCol).isEmpty() && !inputTable->text(i, xCol).isEmpty())
				inputTable->setCell(i, yCol, combineValues(inputTable->cell(i, yCol), offset));
		}
		inputTable->notifyChanges(c->title().text());
		return;
	}

	Table *refTable = app->table(boxReferenceName->currentText());
	if (!refTable)
		return;

	int refCol = refTable->colIndex(boxColumnName->currentText());
	if (refCol < 0)
		return;

	int refXCol = refTable->colX(refCol);
	if (refTable == inputTable && refXCol == xCol){//same X column
		for (int i = startRow; i <= endRow; i++){
			if (!inputTable->text(i, yCol).isEmpty() && !inputTable->text(i, xCol).isEmpty())
				inputTable->setCell(i, yCol, combineValues(inputTable->cell(i, yCol), refTable->cell(i, refCol)));
		}
		inputTable->notifyChanges(c->title().text());
		return;
	}

	int refPoints = refTable->numRows();
	int aux = 0;
	for (int i = 0; i < refPoints; i++){//count valid reference data set points
		if (!refTable->text(i, refXCol).isEmpty() && !refTable->text(i, refCol).isEmpty())
			aux++;
	}
	refPoints = aux;
	if (refPoints < 2){
		QMessageBox::critical(app, tr("QtiPlot") + " - " + tr("Error"),
		tr("Data set %1 must containt at least two valid data points in order to perform this operation!").arg(refTable->objectName() + "_" + boxColumnName->currentText()));
		return;
	}

	double *x = (double *)malloc(refPoints*sizeof(double));
	if (!x)
		return;

	double *y = (double *)malloc(refPoints*sizeof(double));
	if (!y){
		free (x);
		return;
	}

	aux = 0;
	for (int i = 0; i < refTable->numRows(); i++){
		if (!refTable->text(i, refXCol).isEmpty() && !refTable->text(i, refCol).isEmpty()){
			x[aux] = refTable->cell(i, refXCol);
			y[aux] = refTable->cell(i, refCol);
			aux++;
		}
	}

	//sort data with respect to x value
	size_t *p = (size_t *)malloc(refPoints*sizeof(size_t));
	if (!p){
		free(x); free(y);
		return;
	}

	gsl_sort_index(p, x, 1, refPoints);

	double *xtemp = (double *)malloc(refPoints*sizeof(double));
	if (!xtemp){
		free(x); free(y); free(p);
		return;
	}

	double *ytemp = (double *)malloc(refPoints*sizeof(double));
	if (!ytemp){
		free(x); free(y); free(p); free(xtemp);
		return;
	}

	for (int i = 0; i < refPoints; i++){
		xtemp[i] = x[p[i]];
		ytemp[i] = y[p[i]];
	}
	free(x);
	free(y);
	free(p);

	//make linear interpolation on sorted data
	gsl_interp_accel *acc = gsl_interp_accel_alloc();
	gsl_spline *interp = gsl_spline_alloc(gsl_interp_linear, refPoints);
	gsl_spline_init (interp, xtemp, ytemp, refPoints);

	for (int i = startRow; i <= endRow; i++){
		if (!inputTable->text(i, yCol).isEmpty() && !inputTable->text(i, xCol).isEmpty())
			inputTable->setCell(i, yCol, combineValues(inputTable->cell(i, yCol), gsl_spline_eval(interp, inputTable->cell(i, xCol), acc)));
	}
	inputTable->notifyChanges(c->title().text());

	gsl_spline_free (interp);
	gsl_interp_accel_free (acc);
	free(xtemp);
	free(ytemp);
}

void SubtractDataDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	graph = g;
	boxInputName->addItems(g->analysableCurvesList());
	if (g->rangeSelectorsEnabled())
		boxInputName->setCurrentIndex(boxInputName->findText(g->curveRange(g->rangeSelectorTool()->selectedCurve())));

	connect (graph, SIGNAL(destroyed()), this, SLOT(close()));
}
