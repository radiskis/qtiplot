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
class NonLinearFit;
class Fit;

//! Function dialog
class FunctionDialog : public QDialog
{
    Q_OBJECT

public:
	FunctionDialog(ApplicationWindow* parent, bool standAlone = true, Qt::WindowFlags fl = {});
	~FunctionDialog() override;

    void setCurveToModify(Graph *g, int curve);
    void setCurveToModify(FunctionCurve *c);
	void setGraph(Graph *g){graph = g;};

public slots:
	bool apply();

private slots:
	void raiseWidget(int index);
	void insertFunction();
	void updateFunctionsList(int);
	void updateFunctionExplain(int);
	void accept() override;
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
	void setUserFunctionParameters();

signals:
	void constantsGuessingEnded();

private:
	void setConstants(FunctionCurve *c, const QMap<QString, double>& constants);
	void loadUserFunctions();
	void initBuiltInFitModels();

	Graph *graph = nullptr;
	int curveID = -1;

	ScriptEdit* boxXFunction = nullptr;
    ScriptEdit* boxYFunction = nullptr;
	ScriptEdit* boxPolarRadius = nullptr;
	ScriptEdit* boxPolarTheta = nullptr;
    QComboBox* boxType = nullptr;
    QLabel* textFunction = nullptr;
    DoubleSpinBox* boxFrom = nullptr;
    DoubleSpinBox* boxTo = nullptr;
	QLineEdit* boxParameter = nullptr;
	DoubleSpinBox* boxParFrom = nullptr;
	DoubleSpinBox* boxParTo = nullptr;
	QLineEdit* boxPolarParameter = nullptr;
	DoubleSpinBox* boxPolarFrom = nullptr;
	DoubleSpinBox* boxPolarTo = nullptr;
    QPushButton* buttonClear = nullptr;
	QSpinBox* boxPoints = nullptr;
	QSpinBox* boxParPoints = nullptr;
	QSpinBox* boxPolarPoints = nullptr;
	QStackedWidget* optionStack = nullptr;
	ScriptEdit* boxFunction = nullptr;
	QWidget* functionPage = nullptr;
	QWidget* polarPage = nullptr;
	QWidget* parametricPage = nullptr;
	QTableWidget *boxConstants = nullptr;
	QPushButton *addFunctionBtn = nullptr;
	QComboBox* boxMathFunctions = nullptr;
	QComboBox *boxFunctionCategory = nullptr;
	QTextEdit* boxFunctionExplain = nullptr;
	QPushButton *buttonFunctionLog = nullptr, *buttonXParLog = nullptr, *buttonYParLog = nullptr, *buttonPolarRadiusLog = nullptr, *buttonPolarRThetaLog = nullptr;
	QDialogButtonBox *buttonBox = nullptr;

	ApplicationWindow *d_app = nullptr;
	ScriptEdit *d_active_editor = nullptr;
	bool d_stand_alone = true;
	QList <NonLinearFit*> d_user_functions;
	QList <Fit*> d_fit_models;
};

#endif // FUNCTIONDIALOG_H
