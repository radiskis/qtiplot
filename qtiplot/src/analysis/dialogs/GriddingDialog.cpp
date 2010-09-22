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

	ApplicationWindow *app = (ApplicationWindow *)parent;
	QLocale locale = app->locale();

	gl1->addWidget(new QLabel(tr("Search Radius")), 2, 0);
	boxRadius = new DoubleSpinBox();
	boxRadius->setDecimals(app->d_decimal_digits);
	boxRadius->setLocale(locale);
	boxRadius->setValue(2.0);
	boxRadius->setMinimum(0.1);
	gl1->addWidget(boxRadius, 2, 1);
	gl1->setRowStretch(3, 1);

	QGroupBox *gb2 = new QGroupBox(tr("Matrix Dimensions"));
	QGridLayout *gl2 = new QGridLayout(gb2);

	gl2->addWidget(new QLabel(tr("Columns")), 0, 0);
	boxCols = new QSpinBox();
	boxCols->setRange(2, 1000000);
	boxCols->setValue(20);
	gl2->addWidget(boxCols, 0, 1);

	gl2->addWidget(new QLabel(tr("Rows")), 1, 0);
	boxRows = new QSpinBox();
	boxRows->setRange(2, 1000000);
	boxRows->setValue(20);
	gl2->addWidget(boxRows, 1, 1);
	gl2->setRowStretch(2, 1);

	QGroupBox *gb3 = new QGroupBox(tr("Matrix Coordinates"));
	QGridLayout *gl3 = new QGridLayout(gb3);

	gl3->addWidget(new QLabel(tr("X Minimum")), 0, 0);
	boxXStart = new DoubleSpinBox();
	boxXStart->setDecimals(app->d_decimal_digits);
	boxXStart->setLocale(locale);
	gl3->addWidget(boxXStart, 0, 1);

	gl3->addWidget(new QLabel(tr("X Maximum")), 1, 0);
	boxXEnd = new DoubleSpinBox();
	boxXEnd->setDecimals(app->d_decimal_digits);
	boxXEnd->setLocale(locale);
	gl3->addWidget(boxXEnd, 1, 1);

	gl3->addWidget(new QLabel(tr("Y Minimum")), 2, 0);
	boxYStart = new DoubleSpinBox();
	boxYStart->setDecimals(app->d_decimal_digits);
	boxYStart->setLocale(locale);
	gl3->addWidget(boxYStart, 2, 1);

	gl3->addWidget(new QLabel(tr("Y Maximum")), 3, 0);
	boxYEnd = new DoubleSpinBox();
	boxYEnd->setDecimals(app->d_decimal_digits);
	boxYEnd->setLocale(locale);
	gl3->addWidget(boxYEnd, 3, 1);
	gl3->setRowStretch(4, 1);

	previewBox = new QGroupBox(tr("&Preview"));
	previewBox->setCheckable(true);
	previewBox->setChecked(true);

	QGridLayout *gl4 = new QGridLayout(previewBox);
	gl4->addWidget(new QLabel(tr("PlotStyle")), 0, 0);

	boxPlotStyle = new QComboBox();
	boxPlotStyle->insertItem(tr("Wireframe"));
	boxPlotStyle->insertItem(tr("Hidden Line"));
	gl4->addWidget(boxPlotStyle, 0, 1);

	showPlotBox = new QCheckBox(tr("Crea&te Plot"));
	showPlotBox->setChecked(true);

	buttonFit = new QPushButton(tr( "&Ok" ));
    buttonFit->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));	

	QHBoxLayout *vl = new QHBoxLayout();
	vl->addWidget(showPlotBox);
	vl->addStretch();
 	vl->addWidget(buttonFit);
	vl->addWidget(buttonCancel);

	QVBoxLayout *hb = new QVBoxLayout();
    hb->addWidget(gb1, 1);
	hb->addWidget(gb2, 1);
	hb->addWidget(gb3, 1);
	hb->addWidget(previewBox);
	hb->addStretch();
    hb->addLayout(vl);

	sp = new Plot3D();
	sp->setRotation(30, 0, 15);
	sp->setScale(1, 1, 1);
	sp->setShift(0.15, 0, 0);
	sp->setZoom(0.9);
	sp->setOrtho(app->d_3D_orthogonal);
	sp->setSmoothMesh(app->d_3D_smooth_mesh);
	sp->setLocale(app->locale());
	sp->setCoordinateStyle(FRAME);
	sp->coordinates()->setNumberFont(app->d_3D_numbers_font);
	for (int i = 0; i < 12; i++)
		sp->coordinates()->axes[i].setLabelFont(app->d_3D_axes_font);

	resize(QSize(600, 400));

	QHBoxLayout *hb1 = new QHBoxLayout(this);
	hb1->addLayout(hb);
	hb1->addWidget(sp, 1);

	loadDataFromTable();
	preview();

	connect(previewBox, SIGNAL(toggled(bool)), this, SLOT(preview()));
	connect(boxPlotStyle, SIGNAL(activated(int)), this, SLOT(setPlotStyle(int)));

	connect(boxRows, SIGNAL(valueChanged(int)), this, SLOT(preview()));
	connect(boxCols, SIGNAL(valueChanged(int)), this, SLOT(preview()));
	connect(boxRadius, SIGNAL(valueChanged(double)), this, SLOT(preview()));
	connect(boxXStart, SIGNAL(valueChanged(double)), this, SLOT(preview()));
	connect(boxXEnd, SIGNAL(valueChanged(double)), this, SLOT(preview()));
	connect(boxYStart, SIGNAL(valueChanged(double)), this, SLOT(preview()));
	connect(boxYEnd, SIGNAL(valueChanged(double)), this, SLOT(preview()));

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

	Curve *data_curve = new Curve(sp);
	sp->addCurve(data_curve);

	data_curve->setSmoothMesh(app->d_3D_smooth_mesh);
	data_curve->setDataProjection(false);
	data_curve->setProjection(BASE);
	data_curve->setProjection(FACE, false);
	data_curve->setProjection(SIDE, false);

	Dot dot = Dot(5, true);
	data_curve->setPlotStyle(dot);

	Qwt3D::TripleField data;
	Qwt3D::CellField cells;

	int row = 0;
	for (int i = startRow; i <= endRow; i++){
		QString xs = d_table->text(i, xcol);
		QString ys = d_table->text(i, ycol);
		QString zs = d_table->text(i, zcol);
		if (xs.isEmpty() || ys.isEmpty() || zs.isEmpty())
			continue;

		double x = locale.toDouble(xs);
		double y = locale.toDouble(ys);
		double z = locale.toDouble(zs);

		xy(row, 0) = x;
		xy(row, 1) = y;
		xy(row, 2) = z;

		data.push_back (Triple(x, y, z));
		Qwt3D::Cell cell;
		cell.push_back(row);
		if (row > 0)
			cell.push_back(row - 1);
		cells.push_back (cell);

		row++;
	}

	sp->makeCurrent();
	data_curve->loadFromData (data, cells);
	sp->updateData();
	sp->updateGL();

	data.clear();
	cells.clear();

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

void GriddingDialog::preview()
{
	sp->setVisible(previewBox->isChecked());
	if (!previewBox->isChecked())
		return;

	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	Curve *d_preview_curve = 0;
	if (sp->curveList().size() == 1){
		d_preview_curve = new Curve(sp);
		sp->addCurve(d_preview_curve);

		d_preview_curve->setSmoothMesh(app->d_3D_smooth_mesh);
		d_preview_curve->setDataProjection(false);
		d_preview_curve->setProjection(BASE);
		d_preview_curve->setProjection(FACE, false);
		d_preview_curve->setProjection(SIDE, false);
	} else
		d_preview_curve = sp->curve();

	if (boxPlotStyle->currentIndex() == 0)
		d_preview_curve->setPlotStyle(WIREFRAME);
	else
		d_preview_curve->setPlotStyle(HIDDENLINE);

	double xmin = this->boxXStart->value();
	double xmax = this->boxXEnd->value();
	double ymin = this->boxYStart->value();
	double ymax = this->boxYEnd->value();

	int rows = boxRows->value();
	int cols = boxCols->value();

	double xstep = fabs(xmax - xmin)/(cols - 1);
	double ystep = fabs(ymax - ymin)/(rows - 1);

	idwinterpolant z;
	idwbuildmodifiedshepardr(xy, d_nodes, 2, boxRadius->value(), z);

	ap::real_1d_array p;
	p.setlength(2);

	double **data_matrix = Matrix::allocateMatrixData(cols, rows);
	for (int i = 0; i < rows; i++){
		p(1) = ymin + i*ystep;
		for (int j = 0; j < cols; j++){
			p(0) = xmin + j*xstep;
			data_matrix[j][i] = idwcalc(z, p);
		}
	}

	sp->makeCurrent();
	d_preview_curve->loadFromData(data_matrix, cols, rows, xmin, xmax, ymin, ymax);
	resetAxesLabels();
	Matrix::freeMatrixData(data_matrix, cols);

	QApplication::restoreOverrideCursor();
}

void GriddingDialog::setPlotStyle(int style)
{
	Curve *d_preview_curve = 0;
	if (sp->curveList().size() == 2)
		d_preview_curve = sp->curve();
	else
		return;

	sp->makeCurrent();
	if (style == 0)
		d_preview_curve->setPlotStyle(WIREFRAME);
	else
		d_preview_curve->setPlotStyle(HIDDENLINE);
	sp->updateGL();
}

void GriddingDialog::resetAxesLabels()
{
	QString s = tr("X axis");
	sp->coordinates()->axes[X1].setLabelString(s);
	sp->coordinates()->axes[X2].setLabelString(s);
	sp->coordinates()->axes[X3].setLabelString(s);
	sp->coordinates()->axes[X4].setLabelString(s);

	s = tr("Y axis");
	sp->coordinates()->axes[Y1].setLabelString(s);
	sp->coordinates()->axes[Y2].setLabelString(s);
	sp->coordinates()->axes[Y3].setLabelString(s);
	sp->coordinates()->axes[Y4].setLabelString(s);
}

GriddingDialog::~GriddingDialog()
{
	delete sp;
}
