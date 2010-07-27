/***************************************************************************
	File                 : BaselineDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Subtract baseline dialog

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
#ifndef BASELINE_DIALOG_H
#define BASELINE_DIALOG_H

#include <QDialog>

class QPushButton;
class QRadioButton;
class QComboBox;
class QLineEdit;
class QSpinBox;
class Graph;
class Table;
class PlotCurve;
class BaselineTool;
class QwtPlotCurve;

//! Subtract baseline dialog
class BaselineDialog : public QDialog
{
	Q_OBJECT

public:
	BaselineDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
	void setGraph(Graph *g);

private slots:
	void updateGraphCurves();
	void updateTableColumns(int tabnr);
	void createBaseline();
	void subtractBaseline(bool add = false);
	void undo();
	void modifyBaseline();
	void enableBaselineOptions();

private:
	void disableBaselineTool();
	double combineValues(double v1, double v2, bool add = false);
	void closeEvent(QCloseEvent* e);

	Graph *graph;
	QPushButton* buttonSubtract;
	QPushButton* buttonUndo;
	QPushButton* buttonModify;
	QPushButton* buttonCreate;
	QPushButton* buttonCancel;

	QComboBox* boxTableName;
	QComboBox* boxColumnName;
	QComboBox* boxInputName;
	QComboBox* boxInterpolationMethod;

	QLineEdit* boxEquation;

	QRadioButton *btnAutomatic;
	QRadioButton *btnEquation;
	QRadioButton *btnDataset;

	QSpinBox* boxPoints;

	PlotCurve *d_baseline;
	Table *d_table;
	BaselineTool *d_picker_tool;
};

#endif
