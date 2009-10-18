/***************************************************************************
    File                 : CreateBinMatrixDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D Binning matrix dialog

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
#ifndef BinMatrixDialog_H
#define BinMatrixDialog_H

#include <QDialog>

class QPushButton;
class QSpinBox;
class DoubleSpinBox;
class Table;

//! 2D Binning matrix dialog
class CreateBinMatrixDialog : public QDialog
{
    Q_OBJECT

public:
    CreateBinMatrixDialog(Table *t, int startRow, int endRow, QWidget* parent = 0, Qt::WFlags fl = 0);

public slots:
	void accept();

signals:
	void options(const QString&);

private:
    QPushButton* buttonOk;
	QPushButton* buttonCancel;
	DoubleSpinBox *xMinBox, *xMaxBox, *yMinBox, *yMaxBox;
	QSpinBox *rowsBox, *colsBox;
	Table *d_table;
	int d_start_row, d_end_row, d_x_col, d_y_col;
};

#endif
