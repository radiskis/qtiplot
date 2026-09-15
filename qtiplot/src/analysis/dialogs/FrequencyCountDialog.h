/***************************************************************************
    File                 : FrequencyCountDialog.h
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
#ifndef FREQUENCYCOUNTDIALOG_H
#define FREQUENCYCOUNTDIALOG_H

#include <QDialog>
#include "GslRAII.h"
#include "Ref.h"

class QPushButton;
class DoubleSpinBox;
class Table;

//! Filter options dialog
class FrequencyCountDialog : public QDialog
{
    Q_OBJECT

public:
    FrequencyCountDialog(Table *t, QWidget* parent = nullptr, Qt::WindowFlags fl = {} );
    ~FrequencyCountDialog() override;

private slots:
    bool apply();
    void accept() override;

private:
    Ref<Table> d_source_table;
    Ref<Table> d_result_table;
    QString d_col_name;
    GslRAII::UniqueVector d_col_values;
	int d_bins = 0;

    QPushButton* buttonApply = nullptr;
	QPushButton* buttonCancel = nullptr;
	QPushButton* buttonOk = nullptr;
	DoubleSpinBox* boxStart = nullptr;
	DoubleSpinBox* boxEnd = nullptr;
	DoubleSpinBox* boxStep = nullptr;
};

#endif
