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
#include <ApplicationWindow.h>
#include <Folder.h>
#include <Table.h>
#include <DoubleSpinBox.h>

#include <QGroupBox>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>

#include <gsl/gsl_histogram.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort_vector.h>

FrequencyCountDialog::FrequencyCountDialog(Table *t, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    d_source_table(t),
    d_result_table(NULL),
    d_col_name(""),
    d_col_values(NULL),
	d_bins(10)
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
        int col = -1;
        int sr = 0;
        int er = t->numRows();
        int ts = t->table()->currentSelection();
        if (ts >= 0){
            Q3TableSelection sel = t->table()->selection(ts);
            sr = sel.topRow();
            er = sel.bottomRow() + 1;
            col = sel.leftCol();
            d_col_name = t->colName(col);
        }
        int size = 0;
        for (int i = sr; i < er; i++){
            if (!t->text(i, col).isEmpty())
                size++;
        }

        if (size > 1)
            d_col_values = gsl_vector_alloc(size);

        if (d_col_values){
            int aux = 0;
            for (int i = sr; i < er; i++){
                if (!t->text(i, col).isEmpty()){
                    gsl_vector_set(d_col_values, aux, t->cell(i, col));
                    aux++;
                }
            }

            gsl_sort_vector(d_col_values);

            min = floor(gsl_vector_get(d_col_values, 0));
            max = ceil(gsl_vector_get(d_col_values, size - 1));
            step = (max - min)/(double)d_bins;

            int p = app->d_decimal_digits;
            double *data = d_col_values->data;
            QLocale l = app->locale();
            QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + t->objectName() + "\"]\n";
            s += tr("Statistics on %1").arg(d_col_name) + ":\n";
            s += tr("Mean") + " = " + l.toString(gsl_stats_mean (data, 1, size), 'f', p) + "\n";
            s += tr("Standard Deviation") + " = " + l.toString(gsl_stats_sd(data, 1, size), 'f', p) + "\n";
            s += tr("Median") + " = " + l.toString(gsl_stats_median_from_sorted_data(data, 1, size), 'f', p) + "\n";
            s += tr("Size") + " = " + QString::number(size) + "\n";
            s += "--------------------------------------------------------------------------------------\n";
            app->updateLog(s);
        }
	}

    gl1->addWidget(new QLabel(tr("From Minimum")), 0, 0);

	boxStart = new DoubleSpinBox();
	boxStart->setLocale(app->locale());
	boxStart->setValue(min);
	boxStart->setDecimals(app->d_decimal_digits);
	gl1->addWidget(boxStart, 0, 1);

    gl1->addWidget(new QLabel(tr("To Maximum")), 1, 0);

    boxEnd = new DoubleSpinBox();
	boxEnd->setLocale(app->locale());
    boxEnd->setValue(max);
    boxEnd->setDecimals(app->d_decimal_digits);
    gl1->addWidget(boxEnd, 1, 1);

    gl1->addWidget(new QLabel(tr("Step Size")), 2, 0);

    boxStep = new DoubleSpinBox();
	boxStep->setLocale(app->locale());
    boxStep->setValue(step);
    boxStep->setDecimals(app->d_decimal_digits);
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
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

bool FrequencyCountDialog::apply()
{
    if (!d_col_values){
        QMessageBox::critical(this, tr("QtiPlot - Error"),
            tr("Not enough data points, operation aborted!"));
        return false;
	}

	double from = boxStart->value();
	double to = boxEnd->value();
    if (from >= to){
        QMessageBox::critical(this, tr("QtiPlot - Frequency input error"),
            tr("Please enter frequency limits that satisfy: From < To !"));
        boxEnd->setFocus();
        return false;
	}

	int old_bins = d_bins;
	double bin_size = boxStep->value();
	d_bins = int((to - from)/bin_size + 1);
	if (!d_bins)
		return false;

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

	gsl_histogram *h = gsl_histogram_alloc(d_bins);
	if (!h)
		return false;

	double *range = (double *) malloc((d_bins + 2)*sizeof(double));
	if (!range)
		return false;
	for (int i = 0; i <= d_bins + 1; i++)
		range[i] = from + i*bin_size;
	gsl_histogram_set_ranges (h, range, d_bins + 1);
	free(range);

	int dataSize = d_col_values->size;
	for (int i = 0; i < dataSize; i++ )
		gsl_histogram_increment (h, gsl_vector_get(d_col_values, i));

	if (d_bins > d_result_table->numRows())
		d_result_table->setNumRows(d_bins);

	for(int i = d_bins; i < old_bins; i++){
		d_result_table->setText(i, 0, "");
		d_result_table->setText(i, 1, "");
		d_result_table->setText(i, 2, "");
		d_result_table->setText(i, 3, "");
	}

	double sum = 0.0;
	for (int i = 0; i<d_bins; i++ ){
		double aux = gsl_histogram_get (h, i);
		sum += aux;
		double lower, upper;
		gsl_histogram_get_range (h, i, &lower, &upper);
		d_result_table->setCell(i, 0, 0.5*(lower + upper));
		d_result_table->setCell(i, 1, aux);
		d_result_table->setCell(i, 2, upper);
		d_result_table->setCell(i, 3, sum);
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
