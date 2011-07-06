/***************************************************************************
    File                 : FFTDialog.h
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
#ifndef FFTDIALOG_H
#define FFTDIALOG_H

#include <QDialog>

class QPushButton;
class QRadioButton;
class DoubleSpinBox;
class QComboBox;
class QCheckBox;
class Graph;
class Table;
class Matrix;

//! Fast Fourier transform options dialog
class FFTDialog : public QDialog
{
    Q_OBJECT

public:
	enum DataType{onGraph = 0, onTable = 1, onMatrix = 2};

    FFTDialog(int type, QWidget* parent = 0, Qt::WFlags fl = 0 );

public slots:
	void setGraph(Graph *g);
	void setTable(Table *t);
	void setMatrix(Matrix *m);

private slots:
	void activateDataSet(const QString& curveName);
	void accept();
	void updateShiftLabel();

private:
    void fftMatrix();
	void closeEvent (QCloseEvent *);

	Graph *graph;
	Table *d_table;
	int d_type;

	QPushButton* buttonOK;
	QPushButton* buttonCancel;
	QRadioButton *forwardBtn, *backwardBtn;
	QComboBox* boxName, *boxReal, *boxImaginary;
	DoubleSpinBox* boxSampling;
	QCheckBox* boxNormalize, *boxOrder, *boxPower2;
};

#endif
