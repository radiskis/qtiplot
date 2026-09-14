/***************************************************************************
    File                 : SurfaceDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Define surface plot dialog

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
#ifndef SURFACEDIALOG_H
#define SURFACEDIALOG_H

#include <QDialog>

class QPushButton;
class DoubleSpinBox;
class QCheckBox;
class QComboBox;
class QStackedWidget;
class QSpinBox;
class Graph3D;
class QLineEdit;
class ScriptEdit;

//! Define surface plot dialog
class SurfaceDialog : public QDialog
{
    Q_OBJECT

public:
    SurfaceDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

public slots:
	void setFunction(Graph3D *);
	void setParametricSurface(Graph3D *);
	void setGraph(Graph3D *g){d_graph = g;};

private slots:
	void clearFunction();
	void accept() override;
	void showFunctionLog();
	void showXLog();
	void showYLog();
	void showZLog();

private:
	Graph3D *d_graph = nullptr;

	void initFunctionPage();
	void initParametricSurfacePage();
	void acceptParametricSurface();
	void acceptFunction();

	QWidget* functionPage = nullptr;
	QWidget* parametricPage = nullptr;
	QStackedWidget* optionStack = nullptr;
    QPushButton* buttonOk = nullptr;
	QPushButton* buttonCancel = nullptr;
	QPushButton* buttonClear = nullptr;
	QComboBox* boxType = nullptr;
	ScriptEdit* boxFunction = nullptr;
	DoubleSpinBox* boxXFrom = nullptr;
	DoubleSpinBox* boxXTo = nullptr;
	DoubleSpinBox* boxYFrom = nullptr;
	DoubleSpinBox* boxYTo = nullptr;
	DoubleSpinBox* boxZFrom = nullptr;
	DoubleSpinBox* boxZTo = nullptr;

	ScriptEdit* boxX = nullptr;
	ScriptEdit* boxY = nullptr;
	ScriptEdit* boxZ = nullptr;

	DoubleSpinBox* boxUFrom = nullptr;
	DoubleSpinBox* boxUTo = nullptr;
	DoubleSpinBox* boxVFrom = nullptr;
	DoubleSpinBox* boxVTo = nullptr;

	QCheckBox *boxUPeriodic = nullptr;
	QCheckBox *boxVPeriodic = nullptr;
	QSpinBox *boxColumns = nullptr;
	QSpinBox *boxRows = nullptr;
	QSpinBox *boxFuncColumns = nullptr;
	QSpinBox *boxFuncRows = nullptr;
	QPushButton *buttonRecentFunc = nullptr;
	QPushButton *buttonXLog = nullptr;
	QPushButton *buttonYLog = nullptr;
	QPushButton *buttonZLog = nullptr;
};

#endif
