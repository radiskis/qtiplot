/***************************************************************************
    File                 : GriddingDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Gridding options dialog

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
#include "GriddingDialog.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <Matrix.h>
#include <DoubleSpinBox.h>

#include <QApplication>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>

#include <idwint.h>

GriddingDialog::GriddingDialog(Table* t, const QString& colName, int nodes, QWidget* parent, Qt::WFlags fl )
	: QDialog( parent, fl ),
	d_table(t),
	d_col_name(colName),
	d_nodes(nodes)
{
    setObjectName( "GriddingDialog" );
	setWindowTitle(tr("QtiPlot - Random XYZ Gridding"));
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled( true );

    QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Selected Z Dataset")), 0, 0);

	boxName = new QLabel(colName);
	gl1->addWidget(boxName, 0, 1);

	gl1->addWidget(new QLabel(tr("Gridding Method")), 1, 0);
	boxMethod = new QComboBox();
	boxMethod->insertItem(tr("Modified Shepard"));
	gl1->addWidget(boxMethod, 1, 1);

	gl1->addWidget(new QLabel(tr("Columns")), 2, 0);
	boxCols = new QSpinBox();
	boxCols->setRange(2, 1000000);
	boxCols->setValue(20);
	gl1->addWidget(boxCols, 2, 1);

	gl1->addWidget(new QLabel(tr("Rows")), 3, 0);
	boxRows = new QSpinBox();
	boxRows->setRange(2, 1000000);
	boxRows->setValue(20);
	gl1->addWidget(boxRows, 3, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	QLocale locale = app->locale();

	gl1->addWidget(new QLabel(tr("Search Radius")), 4, 0);
	boxRadius = new DoubleSpinBox();
	boxRadius->setDecimals(app->d_decimal_digits);
	boxRadius->setLocale(locale);
	boxRadius->setValue(2.0);
	gl1->addWidget(boxRadius, 4, 1);

	gl1->addWidget(new QLabel(tr("From Xmin")), 5, 0);
	boxXStart = new DoubleSpinBox();
	boxXStart->setDecimals(app->d_decimal_digits);
	boxXStart->setLocale(locale);
	gl1->addWidget(boxXStart, 5, 1);

	gl1->addWidget(new QLabel(tr("To Xmax")), 6, 0);
	boxXEnd = new DoubleSpinBox();
	boxXEnd->setDecimals(app->d_decimal_digits);
	boxXEnd->setLocale(locale);
	gl1->addWidget(boxXEnd, 6, 1);

	gl1->addWidget(new QLabel(tr("From Ymin")), 7, 0);
	boxYStart = new DoubleSpinBox();
	boxYStart->setDecimals(app->d_decimal_digits);
	boxYStart->setLocale(locale);
	gl1->addWidget(boxYStart, 7, 1);

	gl1->addWidget(new QLabel(tr("To Ymax")), 8, 0);
	boxYEnd = new DoubleSpinBox();
	boxYEnd->setDecimals(app->d_decimal_digits);
	boxYEnd->setLocale(locale);
	gl1->addWidget(boxYEnd, 8, 1);

	showPlotBox = new QCheckBox(tr("Show &Plot"));
	showPlotBox->setChecked(true);
	gl1->addWidget(showPlotBox, 9, 1);

	gl1->setRowStretch(10, 1);

	buttonFit = new QPushButton(tr( "&Ok" ));
    buttonFit->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(buttonFit);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1, 1);
    hb->addLayout(vl);

	loadDataFromTable();

	connect( buttonFit, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( close() ) );
}

void GriddingDialog::loadDataFromTable()
{
	if (!d_table)
		return;

	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QLocale locale = app->locale();
	Q3TableSelection sel = d_table->getSelection();

	int startRow = sel.topRow();
	int endRow = sel.bottomRow();
	int zcol = d_table->colIndex(d_col_name);
	if (zcol < 0 || zcol >= d_table->numCols())
		return;

	int ycol = d_table->colY(zcol);
	int xcol = d_table->colX(ycol);

	xy.setlength(d_nodes, 3);

	int row = 0;
	for (int i = startRow; i <= endRow; i++){
		QString xs = d_table->text(i, xcol);
		QString ys = d_table->text(i, ycol);
		QString zs = d_table->text(i, zcol);
		if (xs.isEmpty() || ys.isEmpty() || zs.isEmpty())
			continue;

		xy(row, 0) = locale.toDouble(xs);
		xy(row, 1) = locale.toDouble(ys);
		xy(row, 2) = locale.toDouble(zs);
		row++;
	}

	double xmin = xy(0, 0);
	double xmax = xy(0, 0);
	double ymin = xy(0, 1);
	double ymax = xy(0, 1);
	for (int i = 1; i < d_nodes; i++){
		double x = xy(i, 0);
		double y = xy(i, 1);

		if (x < xmin)
			xmin = x;
		if (y < ymin)
			ymin = y;

		if (x > xmax)
			xmax = x;
		if (y > ymax)
			ymax = y;
	}

	boxXStart->setValue(floor(xmin));
	boxXEnd->setValue(ceil(xmax));
	boxYStart->setValue(floor(ymin));
	boxYEnd->setValue(ceil(ymax));

	QApplication::restoreOverrideCursor();
}

void GriddingDialog::accept()
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	double xmin = this->boxXStart->value();
	double xmax = this->boxXEnd->value();
	double ymin = this->boxYStart->value();
	double ymax = this->boxYEnd->value();

	int rows = boxRows->value();
	int cols = boxCols->value();

	double xstep = fabs(xmax - xmin)/(cols - 1);
	double ystep = fabs(ymax - ymin)/(rows - 1);

	Matrix* m = app->newMatrix(rows, cols);
	m->setCoordinates(xmin, xmax, ymin, ymax);

	idwinterpolant z;
	idwbuildmodifiedshepardr(xy, d_nodes, 2, boxRadius->value(), z);

	ap::real_1d_array p;
	p.setlength(2);
	for (int i = 0; i < rows; i++){
		p(1) = ymin + i*ystep;
		for (int j = 0; j < cols; j++){
			p(0) = xmin + j*xstep;
			m->setCell(i, j, idwcalc(z, p));
		}
	}

	if (showPlotBox->isChecked())
		app->plot3DMatrix(m, 0);

	close();
}
