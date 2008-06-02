/***************************************************************************
    File                 : FrequencyCountDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Frequency count options dialog

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
#include "FrequencyCountDialog.h"
#include "../../ApplicationWindow.h"
#include "../../DoubleSpinBox.h"
#include "../../Table.h"

#include <QGroupBox>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>

FrequencyCountDialog::FrequencyCountDialog(Table *t, const QString& colName, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    d_source_table(t),
    d_result_table(NULL),
    d_col_name(colName),
    d_col_values(NULL)
{
	setObjectName( "FrequencyCountDialog" );
	setWindowTitle(tr("QtiPlot - Frequency count"));
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

    QGroupBox *gb1 = new QGroupBox();
    QGridLayout *gl1 = new QGridLayout(gb1);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	double min = 0.0, max = 0.0, step = 0.0;
	if (t){
        QVarLengthArray<double> values = t->col(t->colIndex(colName));
        int rows = t->numRows();
        gsl_vector *d_col_values = gsl_vector_alloc(rows);
        if (d_col_values){
            for (int i=0; i<rows; i++)
                gsl_vector_set(d_col_values, i, values[i]);

            gsl_vector_minmax (d_col_values, &min, &max);
            min = floor(min);
            max = ceil(max);
            step = (max - min)/10.0;
        }
	}

    gl1->addWidget(new QLabel(tr("From Minimum")), 0, 0);

	boxStart = new DoubleSpinBox();
	boxStart->setValue(min);
	boxStart->setDecimals(app->d_decimal_digits);
	boxStart->setLocale(app->locale());
	gl1->addWidget(boxStart, 0, 1);

    gl1->addWidget(new QLabel(tr("To Maximum")), 1, 0);

    boxEnd = new DoubleSpinBox();
    boxEnd->setValue(max);
    boxEnd->setDecimals(app->d_decimal_digits);
    boxEnd->setLocale(app->locale());
    gl1->addWidget(boxEnd, 1, 1);

    gl1->addWidget(new QLabel(tr("Step Size")), 2, 0);

    boxStep = new DoubleSpinBox();
    boxStep->setValue(step);
    boxStep->setDecimals(app->d_decimal_digits);
    boxStep->setLocale(app->locale());
    gl1->addWidget(boxStep, 2, 1);

    gl1->setRowStretch(3, 1);
	gl1->setColumnStretch(1, 1);

	buttonApply = new QPushButton(tr( "&Apply" ));
    buttonApply->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Cancel" ));
    buttonOk = new QPushButton(tr( "&Ok" ));

    QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(buttonApply);
 	vl->addWidget(buttonOk);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1, 1);
    hb->addLayout(vl);

	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

bool FrequencyCountDialog::apply()
{
	double from = boxStart->value();
	double to = boxEnd->value();
    if (from >= to){
        QMessageBox::critical(this, tr("QtiPlot - Frequency input error"),
            tr("Please enter frequency limits that satisfy: From < To !"));
        boxEnd->setFocus();
        return false;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
        return false;

    if (!d_result_table){
        d_result_table = app->newTable(30, 4, app->generateUniqueName(tr("Count"), true),
                                    tr("Frequency count of %1").arg(d_col_name));
        d_result_table->setColName(0, tr("BinCtr"));
        d_result_table->setColName(1, tr("Count"));
        d_result_table->setColName(2, tr("BinEnd"));
        d_result_table->setColName(3, tr("Sum"));
        d_result_table->showMaximized();
    }

	return true;
}

void FrequencyCountDialog::accept()
{
	if (apply())
        close();
}

FrequencyCountDialog::~FrequencyCountDialog()
{
    if (d_col_values)
        gsl_vector_free(d_col_values);
}
