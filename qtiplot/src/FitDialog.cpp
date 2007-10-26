/***************************************************************************
    File                 : FitDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
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
#include "FitDialog.h"
#include "MyParser.h"
#include "ApplicationWindow.h"
#include "ColorBox.h"
#include "Fit.h"
#include "MultiPeakFit.h"
#include "ExponentialFit.h"
#include "PolynomialFit.h"
#include "PluginFit.h"
#include "NonLinearFit.h"
#include "SigmoidalFit.h"
#include "LogisticFit.h"
#include "Matrix.h"

#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QWidget>
#include <QMessageBox>
#include <QComboBox>
#include <QWidgetList>
#include <QRadioButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QLibrary>
#include <QLocale>
#include <stdio.h>

FitDialog::FitDialog(Graph *g, QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
    setName( "FitDialog" );
	setWindowTitle(tr("QtiPlot - Fit Wizard"));
	setSizeGripEnabled( true );

    userFunctionNames = QStringList();
    userFunctions = QStringList();
    userFunctionParams = QStringList();

	d_param_table = 0;
	d_current_fit = 0;

	tw = new QStackedWidget();

	initEditPage();
	initFitPage();
	initAdvancedPage();

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addWidget(tw);
    setLayout(vl);

	categoryBox->setCurrentRow (2);
	funcBox->setCurrentRow (0);

	loadPlugins();
	setGraph(g);
	initBuiltInFunctions();
}

void FitDialog::initFitPage()
{
    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	boxCurve = new QComboBox();
    gl1->addWidget(boxCurve, 0, 1);
    gl1->addWidget(new QLabel(tr("Function")), 1, 0);
	lblFunction = new QLabel();
    gl1->addWidget(lblFunction, 1, 1);
	boxFunction = new QTextEdit();
	boxFunction->setReadOnly(true);
    QPalette palette = boxFunction->palette();
    palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
    boxFunction->setPalette(palette);
	boxFunction->setMaximumHeight(50);
    gl1->addWidget(boxFunction, 2, 1);
	gl1->addWidget(new QLabel( tr("Initial guesses")), 3, 0 );

	boxParams = new QTableWidget();
    boxParams->setColumnCount(3);
    boxParams->horizontalHeader()->setClickable(false);
    boxParams->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
    boxParams->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
    boxParams->horizontalHeader()->setResizeMode (2, QHeaderView::ResizeToContents);
    QStringList header = QStringList() << tr("Parameter") << tr("Value") << tr("Constant");
    boxParams->setHorizontalHeaderLabels(header);
    boxParams->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    boxParams->verticalHeader()->hide();
    gl1->addWidget(boxParams, 3, 1);

	gl1->addWidget(new QLabel( tr("Algorithm")), 4, 0 );
	boxAlgorithm = new QComboBox();
	boxAlgorithm->addItem(tr("Scaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Unscaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Nelder-Mead Simplex"));
    gl1->addWidget(boxAlgorithm, 4, 1);

	gl1->addWidget(new QLabel( tr("Color")), 5, 0);
	boxColor = new ColorBox( false );
	boxColor->setColor(QColor(Qt::red));
    gl1->addWidget(boxColor, 5, 1);

    QGroupBox *gb1 = new QGroupBox();
    gb1->setLayout(gl1);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("From x=")), 0, 0);
	boxFrom = new QLineEdit();
    gl2->addWidget(boxFrom, 0, 1);
	gl2->addWidget(new QLabel( tr("To x=")), 1, 0);
	boxTo = new QLineEdit();
    gl2->addWidget(boxTo, 1, 1);
    QGroupBox *gb2 = new QGroupBox();
    gb2->setLayout(gl2);

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel(tr("Iterations")), 0, 0);
	boxPoints = new QSpinBox();
    boxPoints->setRange(10, 10000);
	boxPoints->setSingleStep(50);
	boxPoints->setValue(1000);
    gl3->addWidget(boxPoints, 0, 1);
	gl3->addWidget(new QLabel( tr("Tolerance")), 1, 0);
	boxTolerance = new QLineEdit("1e-4");
	gl3->addWidget(boxTolerance, 1, 1);
    QGroupBox *gb3 = new QGroupBox();
    gb3->setLayout(gl3);

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->addWidget(gb2);
    hbox1->addWidget(gb3);

    QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(new QLabel(tr( "Weighting Method" )));
	boxWeighting = new QComboBox();
	boxWeighting->addItem(tr("No weighting"));
	boxWeighting->addItem(tr("Instrumental"));
	boxWeighting->addItem(tr("Statistical"));
	boxWeighting->addItem(tr("Arbitrary Dataset"));
    hbox2->addWidget(boxWeighting);
    QGroupBox *gb4 = new QGroupBox();
    gb4->setLayout(hbox2);

	tableNamesBox = new QComboBox();
	tableNamesBox->setEnabled(false);
    hbox2->addWidget(tableNamesBox);
	colNamesBox = new QComboBox();
	colNamesBox->setEnabled(false);
    hbox2->addWidget(colNamesBox);

    QHBoxLayout *hbox3 = new QHBoxLayout();
	buttonEdit = new QPushButton(tr( "<< &Edit function" ) );
    hbox3->addWidget(buttonEdit);
	btnDeleteFitCurves = new QPushButton(tr( "&Delete Fit Curves" ));
    hbox3->addWidget(btnDeleteFitCurves);
	buttonOk = new QPushButton(tr( "&Fit" ) );
	buttonOk->setDefault( true );
    hbox3->addWidget(buttonOk);
	buttonCancel1 = new QPushButton(tr( "&Close" ));
    hbox3->addWidget(buttonCancel1);
	buttonAdvanced = new QPushButton(tr( "Custom &Output >>" ));
    hbox3->addWidget(buttonAdvanced);
    hbox3->addStretch();

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addLayout(hbox1);
    vbox1->addWidget(gb4);
    vbox1->addLayout(hbox3);

    fitPage = new QWidget();
    fitPage->setLayout(vbox1);
    tw->addWidget(fitPage);

	connect( boxCurve, SIGNAL( activated(const QString&) ), this, SLOT( activateCurve(const QString&) ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT(accept()));
	connect( buttonCancel1, SIGNAL( clicked() ), this, SLOT(close()));
	connect( buttonEdit, SIGNAL( clicked() ), this, SLOT(showEditPage()));
	connect( btnDeleteFitCurves, SIGNAL( clicked() ), this, SLOT(deleteFitCurves()));
	connect( boxWeighting, SIGNAL( activated(int) ), this, SLOT( enableWeightingParameters(int) ) );
	connect( buttonAdvanced, SIGNAL(clicked()), this, SLOT(showAdvancedPage() ) );
    connect( tableNamesBox, SIGNAL( activated(int) ), this, SLOT( selectSrcTable(int) ) );

	setFocusProxy(boxFunction);
}

void FitDialog::initEditPage()
{
    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Category")), 0, 0);
    gl1->addWidget(new QLabel(tr("Function")), 0, 1);
    gl1->addWidget(new QLabel(tr("Expression")), 0, 2);

	categoryBox = new QListWidget();
	categoryBox->addItem(tr("User defined"));
	categoryBox->addItem(tr("Built-in"));
	categoryBox->addItem(tr("Basic"));
	categoryBox->addItem(tr("Plugins"));

    gl1->addWidget(categoryBox, 1, 0);
	funcBox = new QListWidget();
    gl1->addWidget(funcBox, 1, 1);
	explainBox = new QTextEdit();
	explainBox->setReadOnly(true);
    gl1->addWidget(explainBox, 1, 2);

	boxUseBuiltIn = new QCheckBox();
	boxUseBuiltIn->setText(tr("Fit with &built-in function"));
	boxUseBuiltIn->hide();

    QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(boxUseBuiltIn);
    hbox1->addStretch();

	polynomOrderLabel = new QLabel( tr("Polynomial Order"));
	polynomOrderLabel->hide();
    hbox1->addWidget(polynomOrderLabel);

	polynomOrderBox = new QSpinBox();
    polynomOrderBox->setMinValue(1);
	polynomOrderBox->setValue(1);
	polynomOrderBox->hide();
	connect(polynomOrderBox, SIGNAL(valueChanged(int)), this, SLOT(setNumPeaks(int)));
    hbox1->addWidget(polynomOrderBox);

	buttonPlugins = new QPushButton(tr( "&Choose plugins folder..." ) );
    hbox1->addWidget(buttonPlugins);
	buttonPlugins->hide();

    buttonClearUsrList = new QPushButton(tr( "Clear user &list" ) );
    hbox1->addWidget(buttonClearUsrList);
	buttonClearUsrList->hide();

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("Name")), 0, 0);
	boxName = new QLineEdit(tr("user1"));
    gl2->addWidget(boxName, 0, 1);
	btnAddFunc = new QPushButton(tr( "&Save" ));
    gl2->addWidget(btnAddFunc, 0, 2);
    gl2->addWidget(new QLabel(tr("Parameters")), 1, 0);
	boxParam = new QLineEdit("a, b");
    gl2->addWidget(boxParam, 1, 1);
	btnDelFunc = new QPushButton( tr( "&Remove" ));
    gl2->addWidget(btnDelFunc, 1, 2);

    QGroupBox *gb = new QGroupBox();
    gb->setLayout(gl2);

	editBox = new QTextEdit();
	editBox->setTextFormat(Qt::PlainText);
	editBox->setFocus();

    QVBoxLayout *vbox1 = new QVBoxLayout();
	btnAddTxt = new QPushButton(tr( "Add &expression" ) );
    vbox1->addWidget(btnAddTxt);
	btnAddName = new QPushButton(tr( "Add &name" ));
    vbox1->addWidget(btnAddName);
	buttonClear = new QPushButton(tr( "Rese&t" ));
    vbox1->addWidget(buttonClear);
	buttonCancel2 = new QPushButton(tr( "&Close" ));
    vbox1->addWidget(buttonCancel2);
    btnContinue = new QPushButton(tr( "&Fit >>" ));
    vbox1->addWidget(btnContinue);
    vbox1->addStretch();

    QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(editBox);
    hbox2->addLayout(vbox1);

    QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->addLayout(gl1);
    vbox2->addLayout(hbox1);
    vbox2->addWidget(gb);
    vbox2->addLayout(hbox2);

    editPage = new QWidget();
    editPage->setLayout(vbox2);
    tw->addWidget(editPage);

	connect( buttonPlugins, SIGNAL( clicked() ), this, SLOT(choosePluginsFolder()));
    connect( buttonClear, SIGNAL( clicked() ), this, SLOT(resetFunction()));
	connect( buttonClearUsrList, SIGNAL( clicked() ), this, SLOT(clearUserList()));
	connect( categoryBox, SIGNAL(currentRowChanged (int)), this, SLOT(showFunctionsList(int) ) );
	connect( funcBox, SIGNAL(currentRowChanged(int)), this, SLOT(showExpression(int)));
	connect( boxUseBuiltIn, SIGNAL(toggled(bool)), this, SLOT(setFunction(bool) ) );
	connect( btnAddName, SIGNAL(clicked()), this, SLOT(addFunctionName() ) );
	connect( btnAddTxt, SIGNAL(clicked()), this, SLOT(addFunction() ) );
	connect( btnContinue, SIGNAL(clicked()), this, SLOT(showFitPage() ) );
	connect( btnAddFunc, SIGNAL(clicked()), this, SLOT(saveUserFunction()));
	connect( btnDelFunc, SIGNAL(clicked()), this, SLOT(removeUserFunction()));
	connect( buttonCancel2, SIGNAL(clicked()), this, SLOT(close()) );
}


void FitDialog::initAdvancedPage()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	generatePointsBtn = new QRadioButton (tr("&Uniform X Function"));
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	connect( generatePointsBtn, SIGNAL(clicked()), this, SLOT(enableApplyChanges()));

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(generatePointsBtn, 0, 0);

	lblPoints = new QLabel( tr("Points"));

	generatePointsBox = new QSpinBox ();
    generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(10);
	generatePointsBox->setValue(app->fitPoints);
	connect( generatePointsBox, SIGNAL(valueChanged(int)), this, SLOT(enableApplyChanges(int)));
    showPointsBox(!app->generateUniformFitPoints);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->addStretch();
    hb->addWidget(lblPoints);
    hb->addWidget(generatePointsBox);
	gl1->addLayout(hb, 0, 1);

	samePointsBtn = new QRadioButton(tr( "Same X as Fitting &Data" ));
    gl1->addWidget(samePointsBtn, 1, 0);
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	connect( samePointsBtn, SIGNAL(clicked()), this, SLOT(enableApplyChanges()));

    QGroupBox *gb1 = new QGroupBox(tr("Generated Fit Curve"));
    gb1->setLayout(gl1);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("Significant Digits")), 0, 1);
	boxPrecision = new QSpinBox ();
    boxPrecision->setRange(0, 13);
	boxPrecision->setValue (app->fit_output_precision);
	connect( boxPrecision, SIGNAL(valueChanged (int)), this, SLOT(enableApplyChanges(int)));
    gl2->addWidget(boxPrecision, 0, 2);
	btnParamTable = new QPushButton(tr( "Parameters &Table" ));
    gl2->addWidget(btnParamTable, 1, 0);
	gl2->addWidget(new QLabel( tr("Name: ")), 1, 1);
	paramTableName = new QLineEdit(tr( "Parameters" ));
    gl2->addWidget(paramTableName, 1, 2);
	globalParamTableBox = new QCheckBox (tr("&One table for all fits"));
	gl2->addWidget(globalParamTableBox, 1, 3);
	btnCovMatrix = new QPushButton(tr( "Covariance &Matrix" ));
    gl2->addWidget(btnCovMatrix, 2, 0);
    gl2->addWidget(new QLabel( tr("Name: ")), 2, 1);
	covMatrixName = new QLineEdit( tr( "CovMatrix" ) );
    gl2->addWidget(covMatrixName, 2, 2);

	scaleErrorsBox = new QCheckBox(tr("Scale Errors with sqrt(Chi^2/doF)"));
	scaleErrorsBox->setChecked(app->fit_scale_errors);
	connect( scaleErrorsBox, SIGNAL(stateChanged (int)), this, SLOT(enableApplyChanges(int)));

    QGroupBox *gb2 = new QGroupBox(tr("Parameters Output"));
    gb2->setLayout(gl2);

	logBox = new QCheckBox (tr("&Write Parameters to Result Log"));
	logBox->setChecked(app->writeFitResultsToLog);
	connect( logBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyChanges(int)));

	plotLabelBox = new QCheckBox (tr("&Paste Parameters to Plot"));
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	connect( plotLabelBox, SIGNAL(stateChanged (int)), this, SLOT(enableApplyChanges(int)));

    QHBoxLayout *hbox1 = new QHBoxLayout();

	btnBack = new QPushButton(tr( "<< &Fit" ));
	connect( btnBack, SIGNAL(clicked()), this, SLOT(showFitPage()));
	connect( btnBack, SIGNAL(clicked()), this, SLOT(applyChanges()));
    hbox1->addWidget(btnBack);

	btnApply = new QPushButton(tr( "&Apply" ));
	btnApply->setEnabled(false);
	connect( btnApply, SIGNAL(clicked()), this, SLOT(applyChanges()));
    hbox1->addWidget(btnApply);

	buttonCancel3 = new QPushButton(tr( "&Close" ));
    hbox1->addWidget(buttonCancel3);
    hbox1->addStretch();

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb2);
	vbox1->addWidget(scaleErrorsBox);
    vbox1->addWidget(logBox);
    vbox1->addWidget(plotLabelBox);
    vbox1->addStretch();
    vbox1->addLayout(hbox1);

    advancedPage = new QWidget();
	advancedPage->setLayout(vbox1);
    tw->addWidget(advancedPage);

	connect(btnParamTable, SIGNAL(clicked()), this, SLOT(showParametersTable()));
	connect(btnCovMatrix, SIGNAL(clicked()), this, SLOT(showCovarianceMatrix()));
	connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(buttonCancel3, SIGNAL(clicked()), this, SLOT(close()));
}

void FitDialog::applyChanges()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->saveSettings();
	btnApply->setEnabled(false);
}

void FitDialog::showParametersTable()
{
	QString tableName = paramTableName->text();
	if (tableName.isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please enter a valid name for the parameters table."));
		return;
	}

	if (!d_current_fit){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_param_table = d_current_fit->parametersTable(tableName);
}

void FitDialog::showCovarianceMatrix()
{
	QString matrixName = covMatrixName->text();
	if (matrixName.isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please enter a valid name for the covariance matrix."));
		return;
	}

	if (!d_current_fit){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_current_fit->covarianceMatrix(matrixName);
}

void FitDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked())
	{
		lblPoints->show();
		generatePointsBox->show();
	}
	else
	{
		lblPoints->hide();
		generatePointsBox->hide();
	}
}

void FitDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	d_graph = g;
	boxCurve->clear();
	boxCurve->addItems(d_graph->analysableCurvesList());

    QString selectedCurve = g->selectedCurveTitle();
	if (!selectedCurve.isEmpty())
	{
	    int index = boxCurve->findText (selectedCurve);
		boxCurve->setCurrentItem(index);
	}
    activateCurve(boxCurve->currentText());

	connect (d_graph, SIGNAL(closedGraph()), this, SLOT(close()));
	connect (d_graph, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
};

void FitDialog::activateCurve(const QString& curveName)
{
	QwtPlotCurve *c = d_graph->curve(curveName);
	if (!c)
		return;

	double start, end;
    d_graph->range(d_graph->curveIndex(curveName), &start, &end);
    boxFrom->setText(QString::number(QMIN(start, end), 'g', 15));
    boxTo->setText(QString::number(QMAX(start, end), 'g', 15));
};

void FitDialog::saveUserFunction()
{
	if (editBox->text().isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), tr("Please enter a valid function!"));
		editBox->setFocus();
		return;
	}
	else if (boxName->text().isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"),
				tr("Please enter a function name!"));
		boxName->setFocus();
		return;
	}
	else if (boxParam->text().remove(QRegExp("[,;\\s]")).isEmpty())
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"),
				tr("Please enter at least one parameter name!"));
		boxParam->setFocus();
		return;
	}

	if (builtInFunctionNames().contains(boxName->text()))
	{
		QMessageBox::critical(this, tr("QtiPlot - Error: function name"),
				"<p><b>" + boxName->text() + "</b>" + tr(" is a built-in function name"
					"<p>You must choose another name for your function!"));
		editBox->setFocus();
		return;
	}
	if (editBox->text().contains(boxName->text()))
	{
		QMessageBox::critical(this, tr("QtiPlot - Input function error"),
				tr("You can't define functions recursevely!"));
		editBox->setFocus();
		return;
	}

	QString name = boxName->text();
	QString f = name + "(x, " + boxParam->text() + ")=" + editBox->text().remove("\n");

	if (userFunctionNames.contains(name))
	{
		int index = userFunctionNames.findIndex(name);
		userFunctions[index] = f;
		userFunctionParams[index] = boxParam->text();

		if (funcBox->currentItem()->text() == name)
			showExpression(index);
	}
	else
	{
		userFunctionNames << name;
		userFunctions << f;
		userFunctionParams << boxParam->text();

		if (categoryBox->currentRow() == 0)
		{
			funcBox->addItem(name);
			funcBox->setCurrentRow (funcBox->count()-1);
		}

		if ((int)userFunctionNames.count()>0 && !boxUseBuiltIn->isEnabled() && categoryBox->currentRow() == 0)
			boxUseBuiltIn->setEnabled(true);
	}
    buttonClearUsrList->setEnabled(true);
	emit saveFunctionsList(userFunctions);
}

void FitDialog::removeUserFunction()
{
	QString name = funcBox->currentItem()->text();
	if (userFunctionNames.contains(name))
	{
		explainBox->setText(QString());

		int index = userFunctionNames.findIndex(name);
		userFunctionNames.remove(name);

		QString f = userFunctions[index];
		userFunctions.remove(f);

		f = userFunctionParams[index];
		userFunctionParams.remove(f);

		funcBox->clear();
		funcBox->addItems (userFunctionNames);
		funcBox->setCurrentRow (0);

		if (!userFunctionNames.count())
            {
			boxUseBuiltIn->setEnabled(false);
            buttonClearUsrList->setEnabled(false);
            }

		emit saveFunctionsList(userFunctions);
	}
}

void FitDialog::showFitPage()
{
    d_param_table_rows = boxParams->rowCount();

	QString par = boxParam->text().simplified();
	QStringList paramList = par.split(QRegExp("[,;]+[\\s]*"), QString::SkipEmptyParts);
	int parameters = (int)paramList.count();
	boxParams->setRowCount(parameters);
    boxParams->hideColumn(2);

	if (parameters > 7)
		parameters = 7;
	boxParams->setMinimumHeight(4+(parameters+1)*boxParams->horizontalHeader()->height());

    for (int i = d_param_table_rows; i<(int)paramList.count(); i++){
        QTableWidgetItem *it = new QTableWidgetItem(paramList[i]);
        it->setFlags(!Qt::ItemIsEditable);
        it->setBackground(QBrush(Qt::lightGray));
        it->setForeground(QBrush(Qt::darkRed));
        QFont font = it->font();
        font.setBold(true);
        it->setFont(font);
        boxParams->setItem(i, 0, it);

        it = new QTableWidgetItem(QString::number(1.0, 'f', boxPrecision->value()));
        it->setTextAlignment(Qt::AlignRight);
        boxParams->setItem(i, 1, it);
	}
    for (int i = 0; i<(int)paramList.count(); i++)
        boxParams->item (i, 0)->setText(paramList[i]);

	// FIXME: this check is pretty ugly, should be changed to a more elegant way some time
	if (!boxUseBuiltIn->isChecked() ||
		(boxUseBuiltIn->isChecked()&& categoryBox->currentRow()!=3 && categoryBox->currentRow()!=1))
	{
        boxParams->showColumn(2);

		for (int i = 0; i<boxParams->rowCount(); i++ ){
            QTableWidgetItem *it = new QTableWidgetItem();
            it->setFlags(!Qt::ItemIsEditable);
            it->setBackground(QBrush(Qt::lightGray));
            boxParams->setItem(i, 2, it);

			QCheckBox *cb = new QCheckBox();
            boxParams->setCellWidget(i, 2, cb);
		}
	}

	boxFunction->setText(editBox->text().simplified());
	lblFunction->setText(boxName->text() +" (x, " + par + ")");

	tw->setCurrentWidget (fitPage);
}

void FitDialog::showEditPage()
{
	tw->setCurrentWidget (editPage);
}

void FitDialog::showAdvancedPage()
{
	tw->setCurrentWidget (advancedPage);
}

void FitDialog::setFunction(bool ok)
{
	editBox->setEnabled(!ok);
	boxParam->setEnabled(!ok);
	boxName->setEnabled(!ok);
	btnAddFunc->setEnabled(!ok);
	btnAddName->setEnabled(!ok);
	btnAddTxt->setEnabled(!ok);
	buttonClear->setEnabled(!ok);

	if (ok){
		boxName->setText(funcBox->currentItem()->text());
		editBox->setText(explainBox->text());

		if (categoryBox->currentRow() == 0 && (int)userFunctionParams.size() > 0)
			boxParam->setText(userFunctionParams[funcBox->currentRow ()]);
		else if (categoryBox->currentRow() == 1)
			boxParam->setText(d_current_fit->parameterNames().join(", "));
		else if (categoryBox->currentRow() == 3 && (int)pluginParameters.size() > 0 )
			boxParam->setText(pluginParameters[funcBox->currentRow()]);
	}
}

void FitDialog::clearUserList()
{
	userFunctions.clear();
	userFunctionNames.clear();
	if (categoryBox->currentRow() == 0)
	{
		funcBox->clear();
		explainBox->clear();
        boxUseBuiltIn->setEnabled(false);
        buttonClearUsrList->setEnabled(false);
	}
	emit clearFunctionsList();
}

void FitDialog::insertFunctionsList(const QStringList& list)
{
	if (!list.count())
	{
		boxUseBuiltIn->setEnabled(false);
		return;
	}

	for (int i = 0; i<(int)list.count(); i++)
	{
		QString s = list[i].simplified();
		if (!s.isEmpty())
		{
            userFunctions << s;

            int pos1 = s.find("(", 0);
            userFunctionNames << s.left(pos1);

            int pos2 = s.find(")", pos1);
            userFunctionParams << s.mid(pos1+4, pos2-pos1-4);
		}
	}
}

void FitDialog::showFunctionsList(int category)
{
	boxUseBuiltIn->setChecked(false);
	boxUseBuiltIn->setEnabled(false);
	boxUseBuiltIn->hide();
	buttonPlugins->hide();
    buttonClearUsrList->hide();
    buttonClearUsrList->setEnabled(false);
	btnDelFunc->setEnabled(false);
    funcBox->blockSignals(true);
	funcBox->clear();
    explainBox->clear();
	polynomOrderLabel->hide();
	polynomOrderBox->hide();

	switch (category)
	{
		case 0:
			if ((int)userFunctionNames.count() > 0)
			{
				showUserFunctions();
                buttonClearUsrList->show();
				boxUseBuiltIn->setEnabled(true);
                buttonClearUsrList->setEnabled(true);
			}

			boxUseBuiltIn->setText(tr("Fit with selected &user function"));
			boxUseBuiltIn->show();
            buttonClearUsrList->show();
			btnDelFunc->setEnabled(true);
			break;

		case 1:
		{
			boxUseBuiltIn->setText(tr("Fit using &built-in function"));
			boxUseBuiltIn->show();
			boxUseBuiltIn->setEnabled(true);
			funcBox->addItems(builtInFunctionNames());
			break;
		}
		
		case 2:
			showParseFunctions();
			break;

		case 3:
			buttonPlugins->show();
			boxUseBuiltIn->setText(tr("Fit using &plugin function"));
			boxUseBuiltIn->show();
			if ((int)pluginFunctionNames.size() > 0)
			{
				funcBox->addItems(pluginFunctionNames);
				boxUseBuiltIn->setEnabled(true);
			}
			break;
	}
    funcBox->blockSignals(false);
	funcBox->setCurrentRow (0);
}

void FitDialog::choosePluginsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString dir = QFileDialog::getExistingDirectory(QDir::currentDirPath(), this, "get directory",
			tr("Choose the plugins folder"), true, true);
	if (!dir.isEmpty())
	{
		pluginFilesList.clear();
		pluginFunctionNames.clear();
		pluginFunctions.clear();
		pluginParameters.clear();
		funcBox->clear();
		explainBox->clear();

		app->fitPluginsPath = dir;
		loadPlugins();
		if ((int)pluginFunctionNames.size() > 0)
		{
			funcBox->addItems(pluginFunctionNames);
			if (!boxUseBuiltIn->isEnabled())
				boxUseBuiltIn->setEnabled(true);

			funcBox->setCurrentRow(0);
		}
		else
			boxUseBuiltIn->setEnabled(false);
	}
}

void FitDialog::loadPlugins()
{
	typedef char* (*fitFunc)();

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString path = app->fitPluginsPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks);

	for (int i=0; i<(int)lst.count(); i++){
		QLibrary lib(path + lst[i]);

		fitFunc name = (fitFunc) lib.resolve( "name" );
		fitFunc function = (fitFunc) lib.resolve("function");
		fitFunc params = (fitFunc) lib.resolve("parameters");

		if ( name && function && params ){
			pluginFunctionNames << QString(name());
			pluginFunctions << QString(function());
			pluginParameters << QString(params());
			pluginFilesList << lib.library();
		}
	}
}

void FitDialog::showUserFunctions()
{
	funcBox->addItems(userFunctionNames);
}

void FitDialog::showParseFunctions()
{
	funcBox->addItems(MyParser::functionsList());
}

void FitDialog::showExpression(int function)
{
    if (function < 0)
        return;

	if (categoryBox->currentRow() == 2)
		explainBox->setText(MyParser::explainFunction(function));
	else if (categoryBox->currentRow() == 1){
		polynomOrderLabel->hide();
		polynomOrderBox->hide();
		if (funcBox->currentItem()->text() == tr("Gauss") ||
			funcBox->currentItem()->text() == tr("Lorentz")){
			polynomOrderLabel->setText(tr("Peaks"));		
			polynomOrderLabel->show();
			polynomOrderBox->show();
		} else if (funcBox->currentItem()->text() == tr("Polynomial")){
			polynomOrderLabel->setText(tr("Polynomial Order"));		
			polynomOrderLabel->show();
			polynomOrderBox->show();
		}
			
		d_current_fit = d_built_in_functions[function];
		explainBox->setText(d_current_fit->formula());
		setFunction(boxUseBuiltIn->isChecked());
	}
	else if (categoryBox->currentRow() == 0){
		if (userFunctions.size() > function) {
			QStringList l = userFunctions[function].split("=");
			explainBox->setText(l[1]);
		} else
			explainBox->clear();
		setFunction(boxUseBuiltIn->isChecked());
	}
	else if (categoryBox->currentRow() == 3){
		if ((int)pluginFunctions.size() > 0){
			explainBox->setText(pluginFunctions[function]);
			setFunction(boxUseBuiltIn->isChecked());
		}else
			explainBox->clear();
	}
}

void FitDialog::addFunction()
{
	QString f = explainBox->text();
	if (categoryBox->currentRow() == 2){//basic parser function
		f = f.left(f.find("(", 0)+1);
		if (editBox->hasSelectedText()){
			QString markedText=editBox->selectedText();
			editBox->insert(f+markedText+")");
		} else
			editBox->insert(f+")");
	}else
		editBox->insert(f);

	editBox->setFocus();
}

void FitDialog::addFunctionName()
{
	if (funcBox->count() > 0){
		editBox->insert(funcBox->currentItem()->text());
		editBox->setFocus();
	}
}

void FitDialog::accept()
{
	QString curve = boxCurve->currentText();
	QStringList curvesList = d_graph->curvesList();
	if (curvesList.contains(curve) <= 0){
		QMessageBox::critical(this,tr("QtiPlot - Warning"),
				tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
		boxCurve->clear();
		boxCurve->addItems(curvesList);
		return;
	}

	if (!validInitialValues())
		return;

	QString from=boxFrom->text().lower();
	QString to=boxTo->text().lower();
	QString tolerance=boxTolerance->text().lower();
	double start, end, eps;
	try{
		MyParser parser;
		parser.SetExpr(from.ascii());
		start=parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
		boxFrom->setFocus();
		return;
	}

	try{
		MyParser parser;
		parser.SetExpr(to.ascii());
		end=parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(this, tr("QtiPlot - End limit error"),QString::fromStdString(e.GetMsg()));
		boxTo->setFocus();
		return;
	}

	if (start>=end){
		QMessageBox::critical(0, tr("QtiPlot - Input error"),
				tr("Please enter x limits that satisfy: from < end!"));
		boxTo->setFocus();
		return;
	}

	try{
		MyParser parser;
		parser.SetExpr(tolerance.ascii());
		eps=parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(0, tr("QtiPlot - Tolerance input error"),QString::fromStdString(e.GetMsg()));
		boxTolerance->setFocus();
		return;
	}

	if (eps<0 || eps>=1){
		QMessageBox::critical(0, tr("QtiPlot - Tolerance input error"),
				tr("The tolerance value must be positive and less than 1!"));
		boxTolerance->setFocus();
		return;
	}

	int i, n=0, rows=boxParams->rowCount();
	if (!boxParams->isColumnHidden(2)){
		for (i=0;i<rows;i++){//count the non-constant parameters
            QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
			if (!cb->isChecked())
				n++;
		}
	} else
		n=rows;

	QStringList parameters;
	MyParser parser;
	bool error=FALSE;

	double paramsInit[n];
	QString formula = boxFunction->text();
	try{
		bool withNames = containsUserFunctionName(formula);
		while(withNames){
			for (i=0; i<(int)userFunctionNames.count(); i++){
				if (formula.contains(userFunctionNames[i])){
					QStringList l = userFunctions[i].split("=");
					formula.replace(userFunctionNames[i], "(" + l[1] + ")");
				}
			}
			withNames = containsUserFunctionName(formula);
		}

		QStringList builtInFunctions = builtInFunctionNames();
		for (i=0; i<(int)builtInFunctions.count(); i++){
			if (formula.contains(builtInFunctions[i])){
				Fit *fit = d_built_in_functions[i];
				formula.replace(builtInFunctions[i], "(" + fit->formula() + ")");
			}
		}

		if (!boxParams->isColumnHidden(2)){
			int j = 0;
			for (i=0;i<rows;i++){
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
				if (!cb->isChecked()){
					paramsInit[j] = boxParams->item(i,1)->text().toDouble();
					parser.DefineVar(boxParams->item(i,0)->text().ascii(), &paramsInit[j]);
					parameters << boxParams->item(i,0)->text();
					j++;
				} else
					formula.replace(boxParams->item(i,0)->text(), boxParams->item(i,1)->text());
			}
		} else {
			for (i=0;i<n;i++) {
				paramsInit[i] = boxParams->item(i,1)->text().toDouble();
				parser.DefineVar(boxParams->item(i,0)->text().ascii(), &paramsInit[i]);
				parameters << boxParams->item(i,0)->text();
			}
		}

		parser.SetExpr(formula.ascii());
		double x=start;
		parser.DefineVar("x", &x);
		parser.Eval();
	} catch(mu::ParserError &e) {
		QString errorMsg = boxFunction->text() + " = " + formula + "\n" + QString::fromStdString(e.GetMsg()) + "\n" +
			tr("Please verify that you have initialized all the parameters!");

		QMessageBox::critical(0, tr("QtiPlot - Input function error"), errorMsg);
		boxFunction->setFocus();
		error = true;
	}

	if (!error){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();

		if (boxUseBuiltIn->isChecked() && categoryBox->currentRow() == 1)
			modifyGuesses (paramsInit);
		else if (boxUseBuiltIn->isChecked() && categoryBox->currentRow() == 3){
			d_current_fit = new PluginFit(app, d_graph);
			if (!((PluginFit*)d_current_fit)->load(pluginFilesList[funcBox->currentRow()])){
				d_current_fit  = 0;
				return;
			}
		} else {
			d_current_fit = new NonLinearFit(app, d_graph);
			((NonLinearFit*)d_current_fit)->setParametersList(parameters);
			((NonLinearFit*)d_current_fit)->setFormula(formula);
		}
		
		if (d_current_fit->objectName() != tr("Polynomial"))
			d_current_fit->setInitialGuesses(paramsInit);
		
		if (!d_current_fit->setDataFromCurve(curve, start, end) ||
			!d_current_fit->setWeightingData ((Fit::WeightingMethod)boxWeighting->currentIndex(),
					       tableNamesBox->currentText()+"_"+colNamesBox->currentText()))
		{
			delete d_current_fit;
			d_current_fit  = 0;
			return;
		}

		d_current_fit->setTolerance(eps);
		d_current_fit->setAlgorithm((Fit::Algorithm)boxAlgorithm->currentIndex());
		d_current_fit->setColor(boxColor->currentIndex());
		d_current_fit->generateFunction(generatePointsBtn->isChecked(), generatePointsBox->value());
		d_current_fit->setMaximumIterations(boxPoints->value());
		d_current_fit->scaleErrors(scaleErrorsBox->isChecked());
		d_current_fit->fit();
		double *res = d_current_fit->results();
		if (!boxParams->isColumnHidden(2)){
			int j = 0;
			for (i=0;i<rows;i++){
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
				if (!cb->isChecked())
					boxParams->item(i, 1)->setText(QString::number(res[j++], 'g', app->fit_output_precision));
			}
		} else {
			for (i=0;i<rows;i++)
				boxParams->item(i, 1)->setText(QString::number(res[i], 'g', app->fit_output_precision));
		}

		if (globalParamTableBox->isChecked() && d_param_table)
			d_current_fit->writeParametersToTable(d_param_table, true);
	}
}

void FitDialog::modifyGuesses(double* initVal)
{
	if (!d_current_fit)
		return;
	
	QString fitName = d_current_fit->objectName();
	if (fitName == tr("ExpDecay1"))
		initVal[1] = 1/initVal[1];
	else if (fitName == tr("ExpGrowth"))
		initVal[1] = -1/initVal[1];
	else if (fitName == tr("ExpDecay2")){
		initVal[1] = 1/initVal[1];
		initVal[3] = 1/initVal[3];
	} else if (fitName == tr("ExpDecay3")){
		initVal[1] = 1/initVal[1];
		initVal[3] = 1/initVal[3];
		initVal[5] = 1/initVal[5];
	}
}

bool FitDialog::containsUserFunctionName(const QString& s)
{
	bool contains = false;
	for (int i=0; i<(int)userFunctionNames.count(); i++)
	{
        QString fn = userFunctionNames[i];
		if (!fn.isEmpty() && s.contains(fn))
		{
			contains = true;
			break;
		}
	}
	return contains;
}

bool FitDialog::validInitialValues()
{
	for (int i=0; i<boxParams->rowCount(); i++){
		if(boxParams->item(i,1)->text().isEmpty()){
			QMessageBox::critical(this, tr("QtiPlot - Input error"),
					tr("Please enter initial guesses for your parameters!"));
			boxParams->setCurrentCell (i, 1);
			return false;
		}

		try{
			MyParser parser;
			parser.SetExpr(boxParams->item(i, 1)->text().ascii());
			parser.Eval();
		} catch (mu::ParserError &e){
			QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
			boxParams->setCurrentCell (i, 1);
			return false;
		}
	}
	return true;
}

void FitDialog::changeDataRange()
{
	double start = d_graph->selectedXStartValue();
	double end = d_graph->selectedXEndValue();
	boxFrom->setText(QString::number(QMIN(start, end), 'g', 15));
	boxTo->setText(QString::number(QMAX(start, end), 'g', 15));
}

void FitDialog::setSrcTables(QWidgetList* tables)
{
	if (tables->isEmpty())
	{
		tableNamesBox->addItem(tr("No data tables"));
		colNamesBox->addItem(tr("No data tables"));
		return;
	}

	srcTables = tables;
	tableNamesBox->clear();
	foreach(QWidget *i, *srcTables)
		tableNamesBox->addItem(i->objectName());

	tableNamesBox->setCurrentIndex(tableNamesBox->findText(boxCurve->currentText().split("_", QString::SkipEmptyParts)[0]));
	selectSrcTable(tableNamesBox->currentIndex());
}

void FitDialog::selectSrcTable(int tabnr)
{
	colNamesBox->clear();

	if (tabnr >= 0 && tabnr < srcTables->count())
	{
		Table *t = (Table*)srcTables->at(tabnr);
		if (t)
			colNamesBox->addItems(t->colNames());
	}
}

void FitDialog::enableWeightingParameters(int index)
{
	if (index == Fit::Dataset){
		tableNamesBox->setEnabled(true);
		colNamesBox->setEnabled(true);
	} else {
		tableNamesBox->setEnabled(false);
		colNamesBox->setEnabled(false);
	}
}

void FitDialog::closeEvent (QCloseEvent * e )
{
	if(d_current_fit && plotLabelBox->isChecked())
		d_current_fit->showLegend();

	e->accept();
}

void FitDialog::enableApplyChanges(int)
{
	btnApply->setEnabled(true);
}

void FitDialog::deleteFitCurves()
{
	d_graph->deleteFitCurves();
	boxCurve->clear();
	boxCurve->addItems(d_graph->curvesList());
}

void FitDialog::resetFunction()
{
	boxName->clear();
	boxParam->clear();
	editBox->clear();
}

void FitDialog::initBuiltInFunctions()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	d_built_in_functions << new SigmoidalFit(app, d_graph);
	d_built_in_functions << new ExponentialFit(app, d_graph);
	d_built_in_functions << new TwoExpFit(app, d_graph);
	d_built_in_functions << new ThreeExpFit(app, d_graph);
	d_built_in_functions << new ExponentialFit(app, d_graph, true);
	
	MultiPeakFit *fit = new MultiPeakFit(app, d_graph, MultiPeakFit::Gauss);
	fit->enablePeakCurves(app->generatePeakCurves);
	fit->setPeakCurvesColor(app->peakCurvesColor);
	d_built_in_functions << fit;
	
	d_built_in_functions << new GaussAmpFit(app, d_graph);
	d_built_in_functions << new LogisticFit(app, d_graph);

	fit = new MultiPeakFit(app, d_graph, MultiPeakFit::Lorentz);
	fit->enablePeakCurves(app->generatePeakCurves);
	fit->setPeakCurvesColor(app->peakCurvesColor);
	d_built_in_functions << fit;
	
	d_built_in_functions << new PolynomialFit(app, d_graph, 1);
}

void FitDialog::setNumPeaks(int peaks)
{
	if (d_current_fit->objectName() == tr("Gauss") || 
		d_current_fit->objectName() == tr("Lorentz"))
		((MultiPeakFit *)d_current_fit)->setNumPeaks(peaks);
	else if (d_current_fit->objectName() == tr("Polynomial"))
		((PolynomialFit *)d_current_fit)->setOrder(peaks);
	
	int index = funcBox->currentRow();	
	d_built_in_functions[index] = d_current_fit;
	showExpression(index);
}

QStringList FitDialog::builtInFunctionNames()
{
	QStringList lst;
	foreach(Fit *fit, d_built_in_functions)
		lst << fit->objectName();
	return lst;
}
