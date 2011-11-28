/***************************************************************************
    File                 : MatrixSizeDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Matrix dimensions dialog

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
#include <MatrixSizeDialog.h>
#include <MatrixCommand.h>
#include <ApplicationWindow.h>
#include <DoubleSpinBox.h>

#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QLayout>

MatrixSizeDialog::MatrixSizeDialog( Matrix *m, QWidget* parent, Qt::WFlags fl )
	: QDialog( parent, fl ),
	d_matrix(m)
{
	setWindowTitle(tr("QtiPlot - Matrix Dimensions"));
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled(true);

	QGroupBox *dimBox = new QGroupBox(tr("Dimensions"));
	QHBoxLayout *topLayout = new QHBoxLayout(dimBox);
	topLayout->addWidget(new QLabel(tr("Columns") + " x " + tr("Rows") + " = "));

	boxCols = new QSpinBox();
	boxCols->setRange(1, INT_MAX);
	topLayout->addWidget(boxCols, 1);

	topLayout->addWidget( new QLabel(" x "));

	boxRows = new QSpinBox();
	boxRows->setRange(1, INT_MAX);
	topLayout->addWidget(boxRows, 1);

	tabsWidget = new QTabWidget();

	coordinatesBox = new QGroupBox();
	QGridLayout *centerLayout = new QGridLayout(coordinatesBox);
	centerLayout->addWidget( new QLabel(tr( "X (Columns)" )), 0, 1 );
	centerLayout->addWidget( new QLabel(tr( "Y (Rows)" )), 0, 2 );

	centerLayout->addWidget( new QLabel(tr( "First" )), 1, 0 );

	QLocale locale = m->locale();
	boxXStart = new DoubleSpinBox();
	boxXStart->setLocale(locale);
	centerLayout->addWidget( boxXStart, 1, 1 );

	boxYStart = new DoubleSpinBox();
	boxYStart->setLocale(locale);
	centerLayout->addWidget( boxYStart, 1, 2 );

	centerLayout->addWidget( new QLabel(tr( "Last" )), 2, 0 );
	boxXEnd = new DoubleSpinBox();
	boxXEnd->setLocale(locale);
	centerLayout->addWidget( boxXEnd, 2, 1 );

	boxYEnd = new DoubleSpinBox();
	boxYEnd->setLocale(locale);
	centerLayout->addWidget( boxYEnd, 2, 2 );
	centerLayout->setRowStretch(3, 1);

	tabsWidget->addTab(coordinatesBox, tr("xy &Mapping"));

	xLabelsBox = new QGroupBox();
	QGridLayout *xLabelsLayout = new QGridLayout(xLabelsBox);
	xLabelsLayout->addWidget(new QLabel(tr("Long Name")), 0, 0);
	xLabelLineEdit = new QLineEdit();
	xLabelsLayout->addWidget(xLabelLineEdit, 0, 1);
	xLabelsLayout->addWidget(new QLabel(tr("Units")), 1, 0);
	xUnitLineEdit = new QLineEdit();
	xLabelsLayout->addWidget(xUnitLineEdit, 1, 1);
	xLabelsLayout->addWidget(new QLabel(tr("Comments")), 2, 0);
	xCommentsTextEdit = new QTextEdit();
	xCommentsTextEdit->setMaximumHeight(80);
	xLabelsLayout->addWidget(xCommentsTextEdit, 2, 1);
	xLabelsLayout->setColumnStretch(1, 1);
	xLabelsLayout->setRowStretch(3, 1);
	tabsWidget->addTab(xLabelsBox, tr("&x Labels"));

	yLabelsBox = new QGroupBox();
	QGridLayout *yLabelsLayout = new QGridLayout(yLabelsBox);
	yLabelsLayout->addWidget(new QLabel(tr("Long Name")), 0, 0);
	yLabelLineEdit = new QLineEdit();
	yLabelsLayout->addWidget(yLabelLineEdit, 0, 1);
	yLabelsLayout->addWidget(new QLabel(tr("Units")), 1, 0);
	yUnitLineEdit = new QLineEdit();
	yLabelsLayout->addWidget(yUnitLineEdit, 1, 1);
	yLabelsLayout->addWidget(new QLabel(tr("Comments")), 2, 0);
	yCommentsTextEdit = new QTextEdit();
	yCommentsTextEdit->setMaximumHeight(80);
	yLabelsLayout->addWidget(yCommentsTextEdit, 2, 1);
	yLabelsLayout->setColumnStretch(1, 1);
	yLabelsLayout->setRowStretch(3, 1);
	tabsWidget->addTab(yLabelsBox, tr("&y Labels"));

	zLabelsBox = new QGroupBox();
	QGridLayout *zLabelsLayout = new QGridLayout(zLabelsBox);
	zLabelsLayout->addWidget(new QLabel(tr("Long Name")), 0, 0);
	zLabelLineEdit = new QLineEdit();
	zLabelsLayout->addWidget(zLabelLineEdit, 0, 1);
	zLabelsLayout->addWidget(new QLabel(tr("Units")), 1, 0);
	zUnitLineEdit = new QLineEdit();
	zLabelsLayout->addWidget(zUnitLineEdit, 1, 1);
	zLabelsLayout->addWidget(new QLabel(tr("Comments")), 2, 0);
	zCommentsTextEdit = new QTextEdit();
	zCommentsTextEdit->setMaximumHeight(80);
	zLabelsLayout->addWidget(zCommentsTextEdit, 2, 1);
	zLabelsLayout->setColumnStretch(1, 1);
	zLabelsLayout->setRowStretch(3, 1);
	tabsWidget->addTab(zLabelsBox, tr("&z Labels"));

	xLabelLineEdit->setText(m->xLabel());
	xUnitLineEdit->setText(m->xUnit());
	xCommentsTextEdit->setPlainText(m->xComment());

	yLabelLineEdit->setText(m->yLabel());
	yUnitLineEdit->setText(m->yUnit());
	yCommentsTextEdit->setPlainText(m->yComment());

	zLabelLineEdit->setText(m->zLabel());
	zUnitLineEdit->setText(m->zUnit());
	zCommentsTextEdit->setPlainText(m->zComment());

	QHBoxLayout *bottomLayout = new QHBoxLayout();
	bottomLayout->addStretch();

	buttonProperties = new QPushButton();
	buttonProperties->setIcon(QIcon(":/configure.png"));
	buttonProperties->setToolTip(tr("Open Properties Dialog"));
	buttonProperties->setAutoDefault(false);
	bottomLayout->addWidget( buttonProperties );

	buttonValues = new QPushButton();
	buttonValues->setIcon(QIcon(":/formula.png"));
	buttonValues->setToolTip(tr("Open Set Values Dialog"));
	buttonValues->setAutoDefault(false);
	bottomLayout->addWidget( buttonValues );

	buttonApply = new QPushButton(tr("&Apply"));
	buttonApply->setDefault( true );
	bottomLayout->addWidget(buttonApply);
	buttonOk = new QPushButton(tr("&OK"));
	bottomLayout->addWidget( buttonOk );
	buttonCancel = new QPushButton(tr("&Cancel"));
	bottomLayout->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(dimBox);
	mainLayout->addWidget(tabsWidget);
	mainLayout->addLayout(bottomLayout);

	boxRows->setValue(m->numRows());
	boxCols->setValue(m->numCols());

	boxXStart->setValue(m->xStart());
	boxYStart->setValue(m->yStart());
	boxXEnd->setValue(m->xEnd());
	boxYEnd->setValue(m->yEnd());

	connect( buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
	connect( buttonOk, SIGNAL(clicked()), this, SLOT(accept() ));
	connect( buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect( buttonValues, SIGNAL(clicked()), this, SLOT(openValuesDialog()));
	connect( buttonProperties, SIGNAL(clicked()), this, SLOT(openPropertiesDialog()));
}

void MatrixSizeDialog::openValuesDialog()
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	connect(this, SIGNAL(destroyed()), app, SLOT(showMatrixValuesDialog()));
	this->accept();
}

void MatrixSizeDialog::openPropertiesDialog()
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	connect(this, SIGNAL(destroyed()), app, SLOT(showMatrixDialog()));
	this->accept();
}

void MatrixSizeDialog::apply()
{
	double fromX = boxXStart->value();
	double toX = boxXEnd->value();
	double fromY = boxYStart->value();
	double toY = boxYEnd->value();
	double oxs = d_matrix->xStart();
	double oxe = d_matrix->xEnd();
	double oys = d_matrix->yStart();
	double oye = d_matrix->yEnd();

	QUndoStack *stack = d_matrix->undoStack();
	if(oxs != fromX || oxe != toX || oys != fromY || oye != toY){
		stack->push(new MatrixSetCoordinatesCommand(d_matrix,
						oxs, oxe, oys, oye, fromX, toX, fromY, toY,
						tr("Set Coordinates x[%1 : %2], y[%3 : %4]").arg(fromX).arg(toX).arg(fromY).arg(toY)));
		d_matrix->setCoordinates(fromX, toX, fromY, toY);
	}
	d_matrix->setDimensions(boxRows->value(), boxCols->value());

	QString xLabel = xLabelLineEdit->text();
	if (xLabel != d_matrix->xLabel())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->xLabel(), xLabel, Qt::XAxis, 0,
						tr("Set X Axis Label") + " \"" + xLabel + "\""));

	QString xUnit = xUnitLineEdit->text();
	if (xUnit != d_matrix->xUnit())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->xUnit(), xUnit, Qt::XAxis, 1,
						tr("Set X Axis Unit") + " \"" + xUnit + "\""));

	QString xComment = xCommentsTextEdit->toPlainText();
	if (xComment != d_matrix->xComment())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->xComment(), xComment, Qt::XAxis, 2, tr("Set X Axis Comment")));

	QString yLabel = yLabelLineEdit->text();
	if (yLabel != d_matrix->yLabel())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->yLabel(), yLabel, Qt::YAxis, 0,
						tr("Set Y Axis Label") + " \"" + yLabel + "\""));

	QString yUnit = yUnitLineEdit->text();
	if (yUnit != d_matrix->yUnit())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->yUnit(), yUnit, Qt::YAxis, 1,
						tr("Set Y Axis Unit") + " \"" + yUnit + "\""));

	QString yComment = yCommentsTextEdit->toPlainText();
	if (yComment != d_matrix->yComment())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->yComment(), yComment, Qt::YAxis, 2, tr("Set Y Axis Comment")));

	QString zLabel = zLabelLineEdit->text();
	if (zLabel != d_matrix->zLabel())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->zLabel(), zLabel, Qt::ZAxis, 0,
						tr("Set Z Axis Label") + " \"" + zLabel + "\""));

	QString zUnit = zUnitLineEdit->text();
	if (zUnit != d_matrix->zUnit())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->zUnit(), zUnit, Qt::ZAxis, 1,
						tr("Set Z Axis Unit") + " \"" + zUnit + "\""));

	QString zComment = zCommentsTextEdit->toPlainText();
	if (zComment != d_matrix->zComment())
		stack->push(new MatrixSetStringCommand(d_matrix, d_matrix->zComment(), zComment, Qt::ZAxis, 2, tr("Set Z Axis Comment")));
}

void MatrixSizeDialog::accept()
{
	apply();
	close();
}
