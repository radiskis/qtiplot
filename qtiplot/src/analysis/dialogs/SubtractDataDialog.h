/***************************************************************************
	File                 : SubtractDataDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Subtract reference data dialog

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
#ifndef SUBTRACT_DATA_DIALOG_H
#define SUBTRACT_DATA_DIALOG_H

#include <QDialog>

class QPushButton;
class QRadioButton;
class QComboBox;
class QCheckBox;
class Graph;
class DoubleSpinBox;

//! Subtract reference data dialog
class SubtractDataDialog : public QDialog
{
	Q_OBJECT

public:
	SubtractDataDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
	void setGraph(Graph *g);

private slots:
	void interpolate();
	void updateTableColumns(int tabnr);
	void setCurrentFolder(bool);

private:
	double combineValues(double v1, double v2);

	Graph *graph;
	QPushButton* buttonApply;
	QPushButton* buttonCancel;
	QComboBox* boxInputName;
	QComboBox* boxReferenceName;
	QComboBox* boxColumnName;
	QComboBox* boxOperation;
	QRadioButton *btnDataset;
	QRadioButton *btnValue;
	QCheckBox *boxCurrentFolder;
	DoubleSpinBox* boxOffset;
};

#endif
