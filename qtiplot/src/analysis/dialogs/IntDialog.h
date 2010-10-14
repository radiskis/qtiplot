/***************************************************************************
    File                 : IntDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004-2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Integration options dialog

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
#ifndef INTDIALOG_H
#define INTDIALOG_H

#include <QDialog>

class QComboBox;
class QPushButton;
class QCheckBox;
class QLineEdit;
class ScriptEdit;
class QSpinBox;
class DoubleSpinBox;
class Graph;

//! Integration options dialog
class IntDialog : public QDialog
{
    Q_OBJECT

public:
    IntDialog(QWidget* parent = 0, Graph *g = 0, Qt::WFlags fl = 0 );

private slots:
	void accept();
	void showFunctionLog();
	void insertFunction();
	void clearFunction();

private:
	bool validInput(const QString& function);

	Graph *d_graph;

    QPushButton* buttonOk;
	QPushButton* buttonCancel;
    QCheckBox* boxPlot;
	ScriptEdit* boxFunction;
	QLineEdit* boxVariable;
	QSpinBox* boxSteps;
	DoubleSpinBox* boxStart;
	DoubleSpinBox* boxEnd;
	DoubleSpinBox* boxTol;

	QPushButton *buttonFunctionLog, *addFunctionBtn, *buttonClear;
	QComboBox* boxMathFunctions;
};

#endif
