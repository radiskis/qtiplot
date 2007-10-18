/***************************************************************************
    File                 : MatrixSizeDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2007 by Ion Vasilief
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
#ifndef MATRIXSIZEDIALOG_H
#define MATRIXSIZEDIALOG_H

#include "Matrix.h"
#include <QDialog>

class QGroupBox;
class QPushButton;
class QSpinBox;
class QLineEdit;

//! Matrix dimensions dialog
class MatrixSizeDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
    MatrixSizeDialog(Matrix *m, QWidget* parent = 0, Qt::WFlags fl = 0);

public slots:
	//! Accept changes and quit
	void accept();

private:
    QPushButton* buttonOk;
	QPushButton* buttonCancel;
    QGroupBox* groupBox1, *groupBox2;
	QSpinBox *boxCols, *boxRows;
	QLineEdit *boxXStart, *boxYStart, *boxXEnd, *boxYEnd;
	Matrix *d_matrix;
};

#endif // MATRIXSIZEDIALOG_H
