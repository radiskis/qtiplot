/***************************************************************************
    File                 : FFTDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Fast Fourier transform options dialog

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
#include "FFTDialog.h"
#include <FFT.h>
#include <ApplicationWindow.h>
#include <Table.h>
#include <Graph.h>
#include <PlotCurve.h>
#include <MultiLayer.h>
#include <Matrix.h>
#include <DoubleSpinBox.h>
#include <fft2D.h>

#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QApplication>

FFTDialog::FFTDialog(int type, QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
	setWindowTitle(tr("QtiPlot - FFT Options"));
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

	d_table = 0;
	graph = 0;
	d_type = type;

	forwardBtn = new QRadioButton(tr("&Forward"));
	forwardBtn->setChecked( true );
	backwardBtn = new QRadioButton(tr("&Inverse"));

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(forwardBtn);
	hbox1->addWidget(backwardBtn);
	hbox1->addStretch();

	QGroupBox *gb1 = new QGroupBox();
	gb1->setLayout(hbox1);

	QGridLayout *gl1 = new QGridLayout();
	gl1->setColumnStretch(1, 1);
	if (d_type == onGraph)
		gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	else if (d_type == onTable)
		gl1->addWidget(new QLabel(tr("Sampling")), 0, 0);

	if (d_type != onMatrix){
		boxName = new QComboBox();
		connect(boxName, SIGNAL(activated(const QString&)), this, SLOT(activateDataSet(const QString&)));
		gl1->addWidget(boxName, 0, 1);
		setFocusProxy(boxName);
	}

	ApplicationWindow *app = (ApplicationWindow *)parent;

	boxSampling = new DoubleSpinBox();
	boxSampling->setDecimals(app->d_decimal_digits);
	boxSampling->setLocale(app->locale());

	if (d_type == onTable || d_type == onMatrix){
		gl1->addWidget(new QLabel(tr("Real")), 1, 0);
		boxReal = new QComboBox();
		gl1->addWidget(boxReal, 1, 1);

		gl1->addWidget(new QLabel(tr("Imaginary")), 2, 0);
		boxImaginary = new QComboBox();
		gl1->addWidget(boxImaginary, 2, 1);

		if (d_type == onTable){
			gl1->addWidget(new QLabel(tr("Sampling Interval")), 3, 0);
			gl1->addWidget(boxSampling, 3, 1);
		} else
			connect(boxReal, SIGNAL(activated(const QString&)), this, SLOT(activateDataSet(const QString&)));
	} else if (d_type == onGraph){
		gl1->addWidget(new QLabel(tr("Sampling Interval")), 1, 0);
		gl1->addWidget(boxSampling, 1, 1);
	}

	QGroupBox *gb2 = new QGroupBox();
	gb2->setLayout(gl1);

	boxNormalize = new QCheckBox(tr( "&Normalize Amplitude" ));
	boxNormalize->setChecked(app->d_fft_norm_amp);

	boxOrder = new QCheckBox(tr( "&Shift Results" ));
	boxOrder->setChecked(app->d_fft_shift_res);

	if (d_type == onMatrix){
		boxPower2 = new QCheckBox(tr( "&Zero pad to nearest power of 2 size" ));
		boxPower2->setChecked(app->d_fft_power2);
		boxPower2->hide();
	}

	QVBoxLayout *vbox1 = new QVBoxLayout();
	vbox1->addWidget(gb1);
	vbox1->addWidget(gb2);
	vbox1->addWidget(boxNormalize);
	vbox1->addWidget(boxOrder);
	if (d_type == onMatrix)
		vbox1->addWidget(boxPower2);
	vbox1->addStretch();

	buttonOK = new QPushButton(tr("&OK"));
	buttonOK->setDefault( true );
	buttonCancel = new QPushButton(tr("&Close"));

	QVBoxLayout *vbox2 = new QVBoxLayout();
	vbox2->addWidget(buttonOK);
	vbox2->addWidget(buttonCancel);
	vbox2->addStretch();

	QHBoxLayout *hbox2 = new QHBoxLayout(this);
	hbox2->addLayout(vbox1);
	hbox2->addLayout(vbox2);

	// signals and slots connections
	connect(buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
	if (d_type == onMatrix){
		updateShiftLabel();
		connect(forwardBtn, SIGNAL(toggled(bool)), this, SLOT(updateShiftLabel()));
		connect(backwardBtn, SIGNAL(toggled(bool)), this, SLOT(updateShiftLabel()));
	}
}

void FFTDialog::updateShiftLabel()
{
	if (d_type != onMatrix)
		return;

	if (forwardBtn->isChecked())
		boxOrder->setText(tr("DC &Shift Center"));
	else
		boxOrder->setText(tr("Undo &Shift"));
}

void FFTDialog::accept()
{
    if (d_type == onMatrix){
        fftMatrix();
        close();
        return;
    }

	ApplicationWindow *app = (ApplicationWindow *)parent();
    FFT *fft = NULL;
	if (graph){
		QString name = boxName->currentText();
		fft = new FFT(app, graph->curve(name.left(name.indexOf(" ["))));
	} else if (d_table){
		if (boxReal->currentText().isEmpty()){
			QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please choose a column for the real part of the data!"));
			boxReal->setFocus();
			return;
		}
        fft = new FFT(app, d_table, boxReal->currentText(), boxImaginary->currentText());
	}
    fft->setInverseFFT(backwardBtn->isChecked());
    fft->setSampling(boxSampling->value());
    fft->normalizeAmplitudes(boxNormalize->isChecked());
    fft->shiftFrequencies(boxOrder->isChecked());
    fft->run();
    delete fft;
	close();
}

void FFTDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	graph = g;
	boxName->insertStringList(g->analysableCurvesList());
	activateDataSet(boxName->currentText());
}

void FFTDialog::activateDataSet(const QString& s)
{
	if (d_type == onMatrix){
		ApplicationWindow *app = (ApplicationWindow *)parent();
		if (app){
			Matrix *m = app->matrix(s);
			if (m)
				boxPower2->setVisible(!isPower2(m->numRows()) || !isPower2(m->numCols()));
		}
	} else if (d_table){
		int col = d_table->colIndex(s);
		boxSampling->setValue(d_table->cell(1, col) - d_table->cell(0, col));
	} else if (graph){
		PlotCurve *c = graph->curve(s.left(s.indexOf(" [")));
		if (!c)
			return;
		boxSampling->setValue(c->x(1) - c->x(0));
	}
}

void FFTDialog::setTable(Table *t)
{
	if (!t)
		return;

	d_table = t;
	QStringList l = t->columnsList();
	boxName->insertStringList (l);
	boxReal->insertStringList (l);
	boxImaginary->insertStringList (l);

	int xcol = t->firstXCol();
	if (xcol >= 0){
		boxName->setCurrentItem(xcol);
		boxSampling->setValue(d_table->cell(1, xcol) - d_table->cell(0, xcol));
	}

	l = t->selectedColumns();
	int selected = (int)l.size();
	if (!selected){
		boxReal->setCurrentText(QString());
		boxImaginary->setCurrentText(QString());
	} else if (selected == 1) {
		boxReal->setCurrentItem(t->colIndex(l[0]));
		boxImaginary->setCurrentText(QString());
	} else {
		boxReal->setCurrentItem(t->colIndex(l[0]));
		boxImaginary->setCurrentItem(t->colIndex(l[1]));
	}
};

void FFTDialog::setMatrix(Matrix *m)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QStringList lst = app->matrixNames();
	boxReal->addItems(lst);
	if (m){
		boxReal->setCurrentIndex(lst.indexOf(m->objectName()));
		d_type = onMatrix;
		boxPower2->setVisible(!isPower2(m->numRows()) || !isPower2(m->numCols()));
	}
	boxImaginary->addItem (" ");
	boxImaginary->addItems(lst);
}

void FFTDialog::fftMatrix()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	Matrix *mReal = app->matrix(boxReal->currentText());
	if (!mReal)
		return;

	FFT *fft = new FFT(app, mReal, app->matrix(boxImaginary->currentText()), backwardBtn->isChecked(),
					boxOrder->isChecked(), boxNormalize->isChecked(), boxPower2->isChecked());
	fft->run();
	delete fft;
}

void FFTDialog::closeEvent (QCloseEvent * e)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (app){
		app->d_fft_norm_amp = boxNormalize->isChecked();
		app->d_fft_shift_res = boxOrder->isChecked();
		if (d_type == onMatrix)
			app->d_fft_power2 = boxPower2->isChecked();
	}

	e->accept();
}
