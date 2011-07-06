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
class QAbstractButton;
class QDialogButtonBox;
class QSpinBox;
class QComboBox;
class QLineEdit;

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
	//! Apply changes
	void apply();
	void buttonClicked(QAbstractButton *);
	void updateDimensionsInfo();

private:
	void calculateDimensions(int *newRows, int *newColumns);
	Matrix *d_matrix;

	QDialogButtonBox *buttonBox;
	QSpinBox* boxColumns, *boxRows;
	QComboBox *boxMethod, *boxAction;
	QLineEdit *dimensionsLabel;
};

#endif // MATRIXRESAMPLINGDIALOG_H
