/***************************************************************************
    File                 : FitDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Fit Wizard

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
#ifndef FITDIALOG_H
#define FITDIALOG_H

#include <Graph.h>
#include <QDoubleSpinBox>
#include <QCheckBox>

class QPushButton;
class QLineEdit;
class QComboBox;
class QStackedWidget;
class QWidget;
class QTextEdit;
class QListWidget;
class QTableWidget;
class QSpinBox;
class QLabel;
class QRadioButton;
class QLineEdit;
class ColorButton;
class Fit;
class Table;
class DoubleSpinBox;
class FunctionCurve;
class MdiSubWindow;
class ScriptEdit;

//! Fit Wizard
class FitDialog : public QDialog
{
    Q_OBJECT

public:
	FitDialog(Graph *g, QWidget* parent = nullptr);

    void setSrcTables(QList<MdiSubWindow*> tables);

protected:
	void closeEvent (QCloseEvent * e ) override;
    void initFitPage();
	void initEditPage();
	void initAdvancedPage();
	void chooseFitModelsFolder();
	void choosePluginsFolder();

private slots:
	void accept() override;
    //! Clears the function editor, the parameter names and the function name
    void resetFunction();
	void showFitPage();
	void showEditPage();
	void showAdvancedPage();
	void showFunctionsList(int category);
	void showParseFunctions();
	void showExpression(int function);
	void addFunction();
	void addFunctionName();
	void setFunction(bool ok);
	void saveUserFunction();
	void removeUserFunction();
	void setGraph(Graph *g);
	void activateCurve(const QString&);
	void chooseFolder();
	void changeDataRange();
	void selectSrcTable(int tabnr);
	void enableWeightingParameters(int index);
	void showPointsBox(bool);
	void showParametersTable();
	void showCovarianceMatrix();
	void showResiduals();
	void showConfidenceLimits();
	void showPredictionLimits();

	//! Applies the user changes to the numerical format of the output results
	void applyChanges();

	//! Deletes the result fit curves from the plot
	void deleteFitCurves();

    //! Enable the "Apply" button
	void enableApplyChanges(int = 0);
	void setNumPeaks(int peaks);
	void saveInitialGuesses();
	void loadInitialGuesses();
	void guessInitialValues();
	void returnToFitPage();
	void updatePreview();
	void updatePreviewColor(const QColor &);
	void showPreview(bool on);
	void showParameterRange(bool);
	void guessParameters();

private:
	void loadPlugins();
    void loadUserFunctions();
	void initBuiltInFunctions();
	void modifyGuesses(double* initVal);
	QStringList builtInFunctionNames() const;
	QStringList userFunctionNames() const;
	QStringList plugInNames() const;
	QString parseFormula(const QString& s);
	void setEditorTextColor(const QColor& c);
	void setCurrentFit(int);

    Fit *d_current_fit = nullptr;
	Graph *d_graph = nullptr;
	QPointer <Table> d_param_table = nullptr;
	QList <Fit*> d_user_functions, d_built_in_functions, d_plugins;
	QList <PlotCurve*> d_result_curves;
	QList <MdiSubWindow*> srcTables;
	FunctionCurve *d_preview_curve = nullptr;

    QCheckBox* boxUseBuiltIn = nullptr;
	QStackedWidget* tw = nullptr;
    QPushButton* buttonOk = nullptr;
	QPushButton* buttonCancel1 = nullptr;
	QPushButton* buttonCancel2 = nullptr;
	QPushButton* buttonCancel3 = nullptr;
	QPushButton* buttonAdvanced = nullptr;
	QPushButton* buttonClear = nullptr;
	QPushButton* buttonPlugins = nullptr;
	QPushButton* btnBack = nullptr;
	QPushButton* btnSaveGuesses = nullptr, *btnLoadGuesses = nullptr, *btnGuess = nullptr;
	QComboBox* boxCurve = nullptr;
	QComboBox* boxAlgorithm = nullptr;
	QTableWidget* boxParams = nullptr;
	DoubleSpinBox* boxFrom = nullptr;
	DoubleSpinBox* boxTo = nullptr;
	DoubleSpinBox* boxTolerance = nullptr;
	QSpinBox* boxPoints = nullptr, *generatePointsBox = nullptr, *boxPrecision = nullptr, *polynomOrderBox = nullptr;
	QWidget *fitPage = nullptr, *editPage = nullptr, *advancedPage = nullptr;
	ScriptEdit *editBox = nullptr;
	QTextEdit *explainBox = nullptr, *boxFunction = nullptr;
	QListWidget *categoryBox = nullptr, *funcBox = nullptr;
	QLineEdit *boxName = nullptr;
	QLabel *boxErrorMsg = nullptr, *boxParam = nullptr;
	QLabel *lblFunction = nullptr, *lblPoints = nullptr, *polynomOrderLabel = nullptr;
	QPushButton *btnAddFunc = nullptr, *btnDelFunc = nullptr, *btnContinue = nullptr, *btnApply = nullptr;
	QPushButton *buttonEdit = nullptr, *btnAddTxt = nullptr, *btnAddName = nullptr, *btnDeleteFitCurves = nullptr;
	ColorButton* boxColor = nullptr;
	QComboBox *boxWeighting = nullptr, *tableNamesBox = nullptr, *colNamesBox = nullptr;
	QRadioButton *generatePointsBtn = nullptr, *samePointsBtn = nullptr;
	QPushButton *btnParamTable = nullptr, *btnCovMatrix = nullptr, *btnParamRange = nullptr;
	QPushButton *btnResiduals = nullptr, *btnConfidenceLimits = nullptr, *btnPredictionLimits = nullptr;
	DoubleSpinBox *boxConfidenceLevel = nullptr;
	QLineEdit *covMatrixName = nullptr, *paramTableName = nullptr;
	QCheckBox *plotLabelBox = nullptr, *logBox = nullptr, *scaleErrorsBox = nullptr, *globalParamTableBox = nullptr;
	QCheckBox *previewBox = nullptr;
};
#endif // FITDIALOG_H
