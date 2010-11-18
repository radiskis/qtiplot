/***************************************************************************
    File                 : FunctionDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Function dialog

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
#ifndef FUNCTIONDIALOG_H
#define FUNCTIONDIALOG_H

#include <Graph.h>

class QAbstractButton;
class QDialogButtonBox;
class QTableWidget;
class QStackedWidget;
class QWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QSpinBox;
class QLabel;
class QTextEdit;
class DoubleSpinBox;
class ScriptEdit;
class ApplicationWindow;

//! Function dialog
class FunctionDialog : public QDialog
{
    Q_OBJECT

public:
	FunctionDialog(ApplicationWindow* parent, bool standAlone = true, Qt::WFlags fl = 0 );

    void setCurveToModify(Graph *g, int curve);
    void setCurveToModify(FunctionCurve *c);
	void setGraph(Graph *g){graph = g;};

public slots:
	bool apply();

private slots:
	void raiseWidget(int index);
	void insertFunction();
	void updateFunctionExplain(int);
	void accept();
	bool acceptFunction();
	bool acceptParametric();
	bool acceptPolar();
	void showFunctionLog();
	void showXParLog();
	void showYParLog();
	void showPolarRadiusLog();
	void showPolarThetaLog();
	void setActiveEditor(ScriptEdit *edit){d_active_editor = edit;};
	void guessConstants();
	void buttonClicked(QAbstractButton *);
	void clearList();

private:
	void setConstants(FunctionCurve *c, const QMap<QString, double>& constants);

	Graph *graph;
	int curveID;

	ScriptEdit* boxXFunction;
    ScriptEdit* boxYFunction;
	ScriptEdit* boxPolarRadius;
	ScriptEdit* boxPolarTheta;
    QComboBox* boxType;
    QLabel* textFunction;
    DoubleSpinBox* boxFrom;
    DoubleSpinBox* boxTo;
	QLineEdit* boxParameter;
	DoubleSpinBox* boxParFrom;
	DoubleSpinBox* boxParTo;
	QLineEdit* boxPolarParameter;
	DoubleSpinBox* boxPolarFrom;
	DoubleSpinBox* boxPolarTo;
    QPushButton* buttonClear;
	QSpinBox* boxPoints;
	QSpinBox* boxParPoints;
	QSpinBox* boxPolarPoints;
	QStackedWidget* optionStack;
	ScriptEdit* boxFunction;
	QWidget* functionPage;
	QWidget* polarPage;
	QWidget* parametricPage;
	QTableWidget *boxConstants;
	QPushButton *addFunctionBtn;
	QComboBox* boxMathFunctions;
	QTextEdit* boxFunctionExplain;
	QPushButton *buttonFunctionLog, *buttonXParLog, *buttonYParLog, *buttonPolarRadiusLog, *buttonPolarRThetaLog;
	QDialogButtonBox *buttonBox;

	ApplicationWindow *d_app;
	ScriptEdit *d_active_editor;
	bool d_stand_alone;
};

#endif // FUNCTIONDIALOG_H
