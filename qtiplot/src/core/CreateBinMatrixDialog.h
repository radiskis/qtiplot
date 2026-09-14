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
#include "Ref.h"

class QPushButton;
class QSpinBox;
class DoubleSpinBox;
class Table;

//! 2D Binning matrix dialog
class CreateBinMatrixDialog : public QDialog
{
    Q_OBJECT

public:
    CreateBinMatrixDialog(Table *t, int startRow, int endRow, QWidget* parent = nullptr, Qt::WindowFlags fl = {});

public slots:
	void accept() override;

signals:
	void options(const QString&);

private:
    QPushButton* buttonOk = nullptr;
	QPushButton* buttonCancel = nullptr;
	DoubleSpinBox *xMinBox = nullptr, *xMaxBox = nullptr, *yMinBox = nullptr, *yMaxBox = nullptr;
	QSpinBox *rowsBox = nullptr, *colsBox = nullptr;
	Ref<Table> d_table;
	int d_start_row = 0, d_end_row = 0, d_x_col = -1, d_y_col = -1;
};

#endif
