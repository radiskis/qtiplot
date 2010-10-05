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

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QSpinBox>

MatrixResamplingDialog::MatrixResamplingDialog(Matrix *m, bool shrink, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
	d_matrix(m)
{
	setWindowTitle( tr( "Resample" ) );
    setAttribute(Qt::WA_DeleteOnClose);

	boxAction = new QComboBox();
	boxAction->addItem(tr("Expand for every cell"));
	boxAction->addItem(tr("Shrink into 1x1 for every"));

	if (shrink)
		boxAction->setCurrentIndex(1);

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout *topLayout = new QGridLayout(gb1);
	topLayout->addWidget(new QLabel(tr("Columns")), 0, 0);
	boxColumns = new QSpinBox();
	boxColumns->setMinimum(1);
	boxColumns->setValue(2);
	topLayout->addWidget(boxColumns, 0, 1);

	topLayout->addWidget(new QLabel(tr("Rows")), 1, 0);
	boxRows = new QSpinBox();
	boxRows->setMinimum(1);
	boxRows->setValue(2);
	topLayout->addWidget(boxRows, 1, 1);

	topLayout->addWidget(new QLabel(tr("Method")), 2, 0);
	boxMethod = new QComboBox();
	boxMethod->addItem(tr("Bilinear"));
	boxMethod->addItem(tr("Bicubic"));

	topLayout->addWidget(boxMethod, 2, 1);
	topLayout->setRowStretch(3, 1);

	buttonOk = new QPushButton(tr( "&OK" ));
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );

	QHBoxLayout * bottomLayout = new QHBoxLayout();
	bottomLayout->addStretch();
	bottomLayout->addWidget( buttonOk );

	buttonApply = new QPushButton(tr( "&Apply" ));
	buttonApply->setAutoDefault( true );
	bottomLayout->addWidget( buttonApply );

	buttonCancel = new QPushButton(tr( "&Close" ));
	buttonCancel->setAutoDefault( true );
	bottomLayout->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(boxAction);
	mainLayout->addWidget(gb1);
	mainLayout->addLayout(bottomLayout);

	// signals and slots connections
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void MatrixResamplingDialog::apply()
{
	int r = boxRows->value();
	int c = boxColumns->value();
	if (r == 1 && c == 1)
		return;

	int rows = d_matrix->numRows();
	int cols = d_matrix->numCols();
	if (boxAction->currentIndex()){
		rows /= r;
		cols /= c;
		if (d_matrix->numRows() % r)
			rows++;
		if (d_matrix->numCols() % c)
			cols++;
	} else {
		rows *= r;
		cols *= c;
	}

	d_matrix->resample(rows, cols, (Matrix::ResamplingMethod)boxMethod->currentIndex());
}

void MatrixResamplingDialog::accept()
{
	apply();
	close();
}
