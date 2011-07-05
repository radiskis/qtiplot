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
#include <fft2D.h>
#include <ApplicationWindow.h>
#include <Table.h>
#include <Graph.h>
#include <PlotCurve.h>
#include <MultiLayer.h>
#include <Matrix.h>
#include <DoubleSpinBox.h>

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

	d_matrix = 0;
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
	if (d_type == onGraph)
		gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	else if (d_type == onTable)
		gl1->addWidget(new QLabel(tr("Sampling")), 0, 0);

	if (d_type != onMatrix){
		boxName = new QComboBox();
		connect(boxName, SIGNAL(activated(const QString&)), this, SLOT(activateCurve(const QString&)));
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
		}
	} else if (d_type == onGraph){
		gl1->addWidget(new QLabel(tr("Sampling Interval")), 1, 0);
		gl1->addWidget(boxSampling, 1, 1);
	}

	QGroupBox *gb2 = new QGroupBox();
	gb2->setLayout(gl1);

	boxNormalize = new QCheckBox(tr( "&Normalize Amplitude" ));
	boxNormalize->setChecked(app->d_fft_norm_amp);

	if (d_type != onMatrix){
		boxOrder = new QCheckBox(tr( "&Shift Results" ));
		boxOrder->setChecked(app->d_fft_shift_res);
	} else {
		boxPower2 = new QCheckBox(tr( "&Zero pad to nearest power of 2" ));
		boxPower2->setChecked(app->d_fft_power2);
	}

	QVBoxLayout *vbox1 = new QVBoxLayout();
	vbox1->addWidget(gb1);
	vbox1->addWidget(gb2);
	vbox1->addWidget(boxNormalize);
	if (d_type != onMatrix)
		vbox1->addWidget(boxOrder);
	else
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
	connect( buttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( close() ) );
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
	activateCurve(boxName->currentText());
}

void FFTDialog::activateCurve(const QString& s)
{
	if (d_table){
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
        d_matrix = m;
    }
    boxImaginary->addItem (" ");
    boxImaginary->addItems(lst);
}

void FFTDialog::fftMatrix()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	Matrix *mReal = app->matrix(boxReal->currentText());
	if (!mReal)
		return;

	bool inverse = backwardBtn->isChecked();
	int c = mReal->numCols();
	int r = mReal->numRows();
	int cols = c;
	int rows = r;

	if (boxPower2->isChecked()){
		if (!isPower2(c))
			cols = next2Power(c);
		if (!isPower2(r))
			rows = next2Power(r);
	}

	bool errors = false;
	Matrix *mIm = app->matrix(boxImaginary->currentText());
	if (!mIm)
		errors = true;
	else if (mIm && (mIm->numCols() != cols || mIm->numRows() != rows)){
		errors = true;
		QMessageBox::warning(app, tr("QtiPlot"),
		tr("The two matrices have different dimensions, the imaginary part will be neglected!"));
	}

	double **x_int_re = Matrix::allocateMatrixData(rows, cols, true); // real coeff matrix
	if (!x_int_re)
		return;
	double **x_int_im = Matrix::allocateMatrixData(rows, cols, true); // imaginary coeff  matrix
	if (!x_int_im){
		Matrix::freeMatrixData(x_int_re, rows);
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (int i = 0; i < r; i++)
		for (int j = 0; j < c; j++)
			x_int_re[i][j] = mReal->cell(i, j);

	if (!errors){
		for (int i = 0; i < r; i++)
			for (int j = 0; j < c; j++)
				x_int_im[i][j] = mIm->cell(i, j);
	}

	double **x_fin_re = NULL, **x_fin_im = NULL;
	if (inverse){
		x_fin_re = Matrix::allocateMatrixData(rows, cols);
		x_fin_im = Matrix::allocateMatrixData(rows, cols);
		if (!x_fin_re || !x_fin_im){
			Matrix::freeMatrixData(x_int_re, rows);
			Matrix::freeMatrixData(x_int_im, rows);
			QApplication::restoreOverrideCursor();
			return;
		}
		fft2d_inv(x_int_re, x_int_im, x_fin_re, x_fin_im, cols, rows);
	} else
		fft2d(x_int_re, x_int_im, cols, rows);

	Matrix *realCoeffMatrix = app->newMatrix(rows, cols);
	QString realCoeffMatrixName = app->generateUniqueName(tr("RealMatrixFFT"));
	app->setWindowName(realCoeffMatrix, realCoeffMatrixName);
	realCoeffMatrix->setWindowLabel(tr("Real part of the FFT transform of") + " " + mReal->objectName());

	Matrix *imagCoeffMatrix = app->newMatrix(rows, cols);
	QString imagCoeffMatrixName = app->generateUniqueName(tr("ImagMatrixFFT"));
	app->setWindowName(imagCoeffMatrix, imagCoeffMatrixName);
	imagCoeffMatrix->setWindowLabel(tr("Imaginary part of the FFT transform of") + " " + mReal->objectName());

	Matrix *ampMatrix = app->newMatrix(rows, cols);
	QString ampMatrixName = app->generateUniqueName(tr("AmplitudeMatrixFFT"));
	app->setWindowName(ampMatrix, ampMatrixName);
	ampMatrix->setWindowLabel(tr("Amplitudes of the FFT transform of") + " " + mReal->objectName());

	// Automatically set a centered frequency domain range, suggestion of Dr. Armin Bayer (Qioptiq - www.qioptiq.com)
	double fxmax = 0.0, fymax = 0.0, fxmin = 0.0, fymin = 0.0;
	if (inverse){
		fxmax = 1.0/mReal->dx();
		fymax = 1.0/mReal->dy();

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double re = x_fin_re[i][j];
				double im = x_fin_im[i][j];
				realCoeffMatrix->setCell(i, j, re);
				imagCoeffMatrix->setCell(i, j, im);
				ampMatrix->setCell(i, j, sqrt(re*re + im*im));
			}
		}
		Matrix::freeMatrixData(x_fin_re, rows);
		Matrix::freeMatrixData(x_fin_im, rows);
	} else {
		double dfx = 2*mReal->dx()*(cols - 1);
		double dfy = 2*mReal->dy()*(rows - 1);
		fxmax = cols/dfx;
		fymax = rows/dfy;
		fxmin = -(cols - 2)/dfx;
		fymin = -(rows - 2)/dfy;

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double re = x_int_re[i][j];
				double im = x_int_im[i][j];
				realCoeffMatrix->setCell(i, j, re);
				imagCoeffMatrix->setCell(i, j, im);
				ampMatrix->setCell(i, j, sqrt(re*re + im*im));
			}
		}
	}
	Matrix::freeMatrixData(x_int_re, rows);
	Matrix::freeMatrixData(x_int_im, rows);

	if (boxNormalize->isChecked()){
		double amp_min, amp_max;
		ampMatrix->range(&amp_min, &amp_max);
		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				double amp = ampMatrix->cell(i, j);
				ampMatrix->setCell(i, j, amp/amp_max);
			}
		}
	}

	realCoeffMatrix->resize(mReal->size());
	imagCoeffMatrix->resize(mReal->size());
	ampMatrix->resize(mReal->size());

	realCoeffMatrix->setCoordinates(fxmin, fxmax, fymin, fymax);
	imagCoeffMatrix->setCoordinates(fxmin, fxmax, fymin, fymax);
	ampMatrix->setCoordinates(fxmin, fxmax, fymin, fymax);

	Matrix::ViewType view = mReal->viewType();
	realCoeffMatrix->setViewType(view);
	imagCoeffMatrix->setViewType(view);
	ampMatrix->setViewType(view);

	Matrix::HeaderViewType headView = mReal->headerViewType();
	realCoeffMatrix->setHeaderViewType(headView);
	imagCoeffMatrix->setHeaderViewType(headView);
	ampMatrix->setHeaderViewType(headView);

	const LinearColorMap map = mReal->colorMap();
	imagCoeffMatrix->setColorMap(map);
	realCoeffMatrix->setColorMap(map);
	ampMatrix->setColorMap(map);

	QApplication::restoreOverrideCursor();
}

void FFTDialog::closeEvent (QCloseEvent * e)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if(app){
		app->d_fft_norm_amp = boxNormalize->isChecked();
		if (!d_matrix)
			app->d_fft_shift_res = boxOrder->isChecked();
		else
			app->d_fft_power2 = boxPower2->isChecked();
	}

	e->accept();
}
