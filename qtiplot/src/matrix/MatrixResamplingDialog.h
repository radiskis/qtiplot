/***************************************************************************
	File                 : MatrixResamplingDialog.h
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
#ifndef MATRIXRESAMPLINGDIALOG_H
#define MATRIXRESAMPLINGDIALOG_H

#include <QDialog>

class Matrix;
class QPushButton;
class QSpinBox;
class QComboBox;

//! Matrix properties dialog
class MatrixResamplingDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
	MatrixResamplingDialog(Matrix *m, bool shrink = false, QWidget* parent = 0, Qt::WFlags fl = 0 );

private slots:
	//! Accept and quit
	void accept();
	//! Apply changes
	void apply();

private:
	Matrix *d_matrix;

    QPushButton* buttonOk;
	QPushButton* buttonApply;
	QPushButton* buttonCancel;
	QSpinBox* boxColumns, *boxRows;
	QComboBox *boxMethod, *boxAction;
};

#endif // MATRIXRESAMPLINGDIALOG_H
