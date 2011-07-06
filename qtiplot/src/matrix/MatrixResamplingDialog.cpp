/***************************************************************************
	File                 : MatrixResamplingDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Matrix resampling dialog

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
#include "MatrixResamplingDialog.h"
#include "Matrix.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QSpinBox>

MatrixResamplingDialog::MatrixResamplingDialog(Matrix *m, bool shrink, QWidget* parent, Qt::WFlags fl)
	: QDialog(parent, fl),
	d_matrix(m)
{
	setWindowTitle(tr("Resample") + " " + m->objectName());
	setAttribute(Qt::WA_DeleteOnClose);

	dimensionsLabel = new QLineEdit;
	dimensionsLabel->setAlignment(Qt::AlignCenter);
	dimensionsLabel->setReadOnly(true);

	QPalette palette = dimensionsLabel->palette();
	palette.setColor(QPalette::Base, Qt::lightGray);
	dimensionsLabel->setPalette(palette);

	boxAction = new QComboBox();
	boxAction->addItem(tr("Expand for every cell"));
	boxAction->addItem(tr("Shrink into 1x1 for every"));
	boxAction->addItem(tr("Resize freely"));

	if (shrink)
		boxAction->setCurrentIndex(1);

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout *topLayout = new QGridLayout(gb1);
	topLayout->addWidget(new QLabel(tr("Columns")), 0, 0);
	boxColumns = new QSpinBox();
	boxColumns->setRange(1, INT_MAX);
	boxColumns->setValue(2);
	topLayout->addWidget(boxColumns, 0, 1);

	topLayout->addWidget(new QLabel(tr("Rows")), 1, 0);
	boxRows = new QSpinBox();
	boxRows->setRange(1, INT_MAX);
	boxRows->setValue(2);
	topLayout->addWidget(boxRows, 1, 1);

	topLayout->addWidget(new QLabel(tr("Method")), 2, 0);
	boxMethod = new QComboBox();
	boxMethod->addItem(tr("Bilinear"));
	boxMethod->addItem(tr("Bicubic"));

	topLayout->addWidget(boxMethod, 2, 1);
	topLayout->setRowStretch(3, 1);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Close);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonClicked(QAbstractButton *)));

	connect(boxAction, SIGNAL(activated(int)), this, SLOT(updateDimensionsInfo()));
	connect(boxColumns, SIGNAL(valueChanged(int)), this, SLOT(updateDimensionsInfo()));
	connect(boxRows, SIGNAL(valueChanged(int)), this, SLOT(updateDimensionsInfo()));

	updateDimensionsInfo();

	QVBoxLayout * mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(dimensionsLabel);
	mainLayout->addWidget(boxAction);
	mainLayout->addWidget(gb1);
	mainLayout->addWidget(buttonBox);
}

void MatrixResamplingDialog::calculateDimensions(int *newRows, int *newColumns)
{
	int r = boxRows->value();
	int c = boxColumns->value();
	int rows = d_matrix->numRows();
	int cols = d_matrix->numCols();

	switch(boxAction->currentIndex()){
		case 0:
			rows *= r;
			cols *= c;
		break;
		case 1:
			rows /= r;
			cols /= c;
			if (d_matrix->numRows() % r)
				rows++;
			if (d_matrix->numCols() % c)
				cols++;
		break;
		case 2:
			rows = r;
			cols = c;
		break;
	}

	*newRows = rows;
	*newColumns = cols;
}

void MatrixResamplingDialog::updateDimensionsInfo()
{
	int rows = d_matrix->numRows();
	int cols = d_matrix->numCols();

	QString s = tr("Columns") + " x " + tr("Rows") + ": ";
	s += QString::number(cols) + "x" + QString::number(rows) + " --> ";

	calculateDimensions(&rows, &cols);

	s += QString::number(cols) + "x" + QString::number(rows);
	dimensionsLabel->setText(s);
}

void MatrixResamplingDialog::apply()
{
	int rows = 0, cols = 0;
	calculateDimensions(&rows, &cols);
	if (rows < 2 || cols < 2)
		return;

	d_matrix->resample(rows, cols, (Matrix::ResamplingMethod)boxMethod->currentIndex());
}

void MatrixResamplingDialog::buttonClicked(QAbstractButton *btn)
{
	switch(buttonBox->standardButton(btn)){
		case QDialogButtonBox::Ok:
			apply();
			close();
		break;
		case QDialogButtonBox::Apply:
			apply();
		break;
		case QDialogButtonBox::Close:
			close();
		break;
		default:
			break;
	}
}
