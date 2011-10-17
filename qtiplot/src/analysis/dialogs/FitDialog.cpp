/***************************************************************************
    File                 : FitDialog.cpp
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
#include "FitDialog.h"
#include <Fit.h>
#include <MultiPeakFit.h>
#include <ExponentialFit.h>
#include <PolynomialFit.h>
#include <PluginFit.h>
#include <NonLinearFit.h>
#include <SigmoidalFit.h>
#include <LogisticFit.h>
#include <MyParser.h>
#include <ApplicationWindow.h>
#include <ColorButton.h>
#include <DoubleSpinBox.h>
#include <FunctionCurve.h>
#include <ScriptEdit.h>
#include <RangeSelectorTool.h>

#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
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

#include <muParserToken.h>

using namespace std;

FitDialog::FitDialog(Graph *g, QWidget* parent)
: QDialog(parent)
{
	setObjectName("FitDialog");
	setWindowTitle(tr("QtiPlot - Fit Wizard"));
	setSizeGripEnabled(true);
	setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_OS_WIN
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
#endif

	d_param_table = 0;
	d_current_fit = 0;
	d_preview_curve = NULL;

	tw = new QStackedWidget();

	initEditPage();
	initFitPage();
	initAdvancedPage();

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addWidget(tw);
    setLayout(vl);

	categoryBox->setCurrentRow (2);
	funcBox->setCurrentRow (0);

	setGraph(g);
	initBuiltInFunctions();

	ApplicationWindow *app = (ApplicationWindow *)parent;
	QString pluginsPath = app->fitPluginsPath;
	QFileInfo fi(pluginsPath);
	if (pluginsPath.isEmpty() || !fi.isDir() || !fi.isWritable())
		choosePluginsFolder();
	else
    	loadPlugins();

	QString modelsPath = app->fitModelsPath;
	QFileInfo fim(modelsPath);
	if (!modelsPath.isEmpty() && fim.isDir() && fim.isWritable())
    	loadUserFunctions();

	if (d_user_functions.size())
		categoryBox->setCurrentRow(0);
	else if (d_plugins.size())
		categoryBox->setCurrentRow(3);
	else
		categoryBox->setCurrentRow(1);
}

void FitDialog::initFitPage()
{
    ApplicationWindow *app = (ApplicationWindow *)parent();

	QColor blue = QColor(85, 85, 255);
	QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, blue);

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(font.pointSize() + 1);

	QLabel *l = new QLabel(tr("Function") + ":");
	l->setFont(font);
	l->setPalette(palette);

	QHBoxLayout *hbox0 = new QHBoxLayout();
    hbox0->addWidget(l);

    lblFunction = new QLabel();
    lblFunction->setFont(font);
    lblFunction->setFrameStyle(QFrame::Box | QFrame::Plain);
	lblFunction->setLineWidth(2);
	lblFunction->setMargin(10);
	lblFunction->setPalette(palette);

    hbox0->addWidget(lblFunction);

	QGroupBox *gb1 = new QGroupBox(tr("Data Set"));
    QGridLayout *gl1 = new QGridLayout(gb1);
    gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	boxCurve = new QComboBox();
    gl1->addWidget(boxCurve, 0, 1);
    gl1->setColumnStretch(1, 1);

    gl1->addWidget(new QLabel( tr("Color")), 1, 0);
	boxColor = new ColorButton();
	boxColor->setColor(Qt::red);
    gl1->addWidget(boxColor, 1, 1);

    gl1->addWidget(new QLabel(tr("From x=")), 0, 2);

	boxFrom = new DoubleSpinBox();
    boxFrom->setLocale(app->locale());
    boxFrom->setDecimals(app->d_decimal_digits);
    connect(boxFrom, SIGNAL(valueChanged(double)), this, SLOT(updatePreview()));
    gl1->addWidget(boxFrom, 0, 3);

	gl1->addWidget(new QLabel( tr("To x=")), 1, 2);

	boxTo = new DoubleSpinBox();
    boxTo->setLocale(app->locale());
    boxTo->setDecimals(app->d_decimal_digits);
    connect(boxTo, SIGNAL(valueChanged(double)), this, SLOT(updatePreview()));
    gl1->addWidget(boxTo, 1, 3);
    gl1->setColumnStretch(3, 1);

	gl1->addWidget(new QLabel(tr("Weighting")), 2, 0);
	boxWeighting = new QComboBox();
	boxWeighting->addItem(tr("No weighting"));
	boxWeighting->addItem(tr("Instrumental"));
	boxWeighting->addItem(tr("Statistical"));
	boxWeighting->addItem(tr("Arbitrary Dataset"));
	boxWeighting->addItem(tr("Direct Weighting"));
    gl1->addWidget(boxWeighting, 2, 1);
	gl1->addWidget(new QLabel(), 2, 2);

	QHBoxLayout *hbox2 = new QHBoxLayout();

	tableNamesBox = new QComboBox();
	tableNamesBox->setVisible(false);
    hbox2->addWidget(tableNamesBox);

	colNamesBox = new QComboBox();
	colNamesBox->setVisible(false);
    hbox2->addWidget(colNamesBox);

	gl1->addLayout(hbox2, 2, 3);

	boxFunction = new QTextEdit();
	boxFunction->setReadOnly(true);
    palette = boxFunction->palette();
    palette.setColor(QPalette::Base, Qt::lightGray);
    boxFunction->setPalette(palette);
    boxFunction->setFont(font);
	boxFunction->setMaximumHeight(50);

    QVBoxLayout *vb = new QVBoxLayout();
    btnSaveGuesses = new QPushButton(tr( "&Save" ));
	btnSaveGuesses->setPixmap(QPixmap(":/filesave.png"));
    connect(btnSaveGuesses, SIGNAL(clicked()), this, SLOT(saveInitialGuesses()));
    vb->addWidget(btnSaveGuesses);

    btnLoadGuesses = new QPushButton(tr("Re&load" ));
	btnLoadGuesses->setPixmap(QPixmap(":/reload.png"));
    connect(btnLoadGuesses, SIGNAL(clicked()), this, SLOT(loadInitialGuesses()));
    vb->addWidget(btnLoadGuesses);

	btnGuess = new QPushButton(tr( "&Guess" ));
	btnGuess->setPixmap(QPixmap(":/help.png"));
    connect(btnGuess, SIGNAL(clicked()), this, SLOT(guessInitialValues()));
    vb->addWidget(btnGuess);

    btnParamRange = new QPushButton(tr("&Range" ));
	btnParamRange->setIcon(QIcon(":/param_range_btn.png"));
    btnParamRange->setCheckable(true);
    connect(btnParamRange, SIGNAL(toggled(bool)), this, SLOT(showParameterRange(bool)));
    vb->addWidget(btnParamRange);

    vb->addStretch();

	QGroupBox *gb0 = new QGroupBox(tr("Initial guesses"));
	QGridLayout *gl0 = new QGridLayout(gb0);

	boxParams = new QTableWidget();
    boxParams->setColumnCount(6);
    boxParams->horizontalHeader()->setClickable(false);
    boxParams->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
	boxParams->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
    boxParams->horizontalHeader()->setResizeMode (2, QHeaderView::Stretch);
    boxParams->horizontalHeader()->setResizeMode (3, QHeaderView::Stretch);
	boxParams->horizontalHeader()->setResizeMode (4, QHeaderView::ResizeToContents);
	boxParams->horizontalHeader()->setResizeMode (5, QHeaderView::Stretch);

    QStringList header = QStringList() << tr("Parameter") << tr("From") << tr("Value") << tr("To") << tr("Constant") << tr("Error");
    boxParams->setHorizontalHeaderLabels(header);
    boxParams->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    boxParams->verticalHeader()->hide();
    boxParams->hideColumn(1);
    boxParams->hideColumn(3);

    gl0->addWidget(boxParams, 0, 0);
	gl0->addLayout(vb, 0, 1);

	QGroupBox *gb3 = new QGroupBox(tr("Algorithm"));
    QGridLayout *gl3 = new QGridLayout(gb3);

	boxAlgorithm = new QComboBox();
	boxAlgorithm->addItem(tr("Scaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Unscaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Nelder-Mead Simplex"));
    gl3->addWidget(boxAlgorithm, 0, 0);

    gl3->addWidget(new QLabel(tr("Iterations")), 0, 1);
	boxPoints = new QSpinBox();
    boxPoints->setRange(10, 10000);
	boxPoints->setSingleStep(50);
	boxPoints->setValue(1000);
    gl3->addWidget(boxPoints, 0, 2);
    gl3->setColumnStretch(2, 1);

	gl3->addWidget(new QLabel( tr("Tolerance")), 0, 3);
	boxTolerance = new DoubleSpinBox();
	boxTolerance->setRange(0.0, 1.0);
	boxTolerance->setSingleStep(1e-4);
    boxTolerance->setLocale(app->locale());
    boxTolerance->setDecimals(13);
    boxTolerance->setValue(1e-4);
	gl3->addWidget(boxTolerance, 0, 4);
	gl3->setColumnStretch(4, 1);

    QHBoxLayout *hbox3 = new QHBoxLayout();
	previewBox = new QCheckBox(tr("&Preview"));
	previewBox->setChecked(false);
    connect(previewBox, SIGNAL(clicked(bool)), this, SLOT(showPreview(bool)));
    hbox3->addWidget(previewBox);
	hbox3->addStretch();
	btnDeleteFitCurves = new QPushButton(tr( "&Delete Fit Curves" ));
    hbox3->addWidget(btnDeleteFitCurves);
	btnDeleteFitCurves->setPixmap(QPixmap(":/close.png"));

	buttonCancel1 = new QPushButton(tr( "&Close" ));
	buttonCancel1->setPixmap(QPixmap(":/delete.png"));
	hbox3->addWidget(buttonCancel1);

    buttonOk = new QPushButton(tr( "&Fit" ) );
	buttonOk->setPixmap(QPixmap(":/play.png"));
	buttonOk->setDefault( true );
    hbox3->addWidget(buttonOk);

	QFrame *frame1 = new QFrame();
	frame1->setFrameStyle(QFrame::Box | QFrame::Plain);
	frame1->setLineWidth(2);

	palette = frame1->palette();
	palette.setColor(QPalette::WindowText, blue);
	frame1->setPalette(palette);

	QLabel *selectFunctionLbl = new QLabel(tr("Select Function"));
	selectFunctionLbl->setFont(font);

	int space = 10;
	QHBoxLayout *frameLayout = new QHBoxLayout(frame1);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(selectFunctionLbl);
	frameLayout->addSpacing (space);

	buttonEdit = new QPushButton();
	buttonEdit->setIcon(QIcon(":/prev.png"));
	buttonEdit->setIconSize(QSize(16, 17));
	buttonEdit->setToolTip(tr("Select Function"));

	QFrame *frame2 = new QFrame();
	frame2->setFrameShape(QFrame::Box);
	frame2->setLineWidth(2);

	palette = this->palette();
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Window, blue);
	frame2->setAutoFillBackground(true);
	frame2->setPalette(palette);

	QLabel *fittingSessionLbl = new QLabel(tr("Fitting Session"));
	fittingSessionLbl->setFont(font);

	frameLayout = new QHBoxLayout(frame2);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(fittingSessionLbl);
	frameLayout->addSpacing (space);

	buttonAdvanced = new QPushButton();
	buttonAdvanced->setIcon(QIcon(":/next.png"));
	buttonAdvanced->setIconSize(QSize(16, 17));
	buttonAdvanced->setToolTip(tr("Custom Output"));

	QFrame *frame3 = new QFrame();
	frame3->setFrameShape(QFrame::Box);
	frame3->setLineWidth(2);

	palette = this->palette();
	palette.setColor(QPalette::WindowText, blue);
	frame3->setPalette(palette);

	QLabel *advancedLbl = new QLabel(tr("Custom Output"));
	advancedLbl->setFont(font);

	frameLayout = new QHBoxLayout(frame3);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(advancedLbl);
	frameLayout->addSpacing (space);

	QGroupBox *g = new QGroupBox();
	QHBoxLayout *hbox4 = new QHBoxLayout(g);
	hbox4->addStretch();
	hbox4->addWidget(frame1);
	hbox4->addWidget(buttonEdit);
	hbox4->addWidget(frame2);
	hbox4->addWidget(buttonAdvanced);
	hbox4->addWidget(frame3);
	hbox4->addStretch();

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addLayout(hbox0);
    vbox1->addWidget(boxFunction);
	vbox1->addWidget(gb0);
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb3);
    vbox1->addSpacing(10);
    vbox1->addLayout(hbox3);
	vbox1->addSpacing(10);
	vbox1->addWidget(g);

    fitPage = new QWidget();
    fitPage->setLayout(vbox1);
    tw->addWidget(fitPage);

	connect( boxCurve, SIGNAL(activated(const QString&)), this, SLOT(activateCurve(const QString&)));
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT(accept()));
	connect( buttonCancel1, SIGNAL( clicked() ), this, SLOT(close()));
	connect( buttonEdit, SIGNAL( clicked() ), this, SLOT(showEditPage()));
	connect( btnDeleteFitCurves, SIGNAL( clicked() ), this, SLOT(deleteFitCurves()));
	connect( boxWeighting, SIGNAL( activated(int) ), this, SLOT( enableWeightingParameters(int) ) );
	connect( buttonAdvanced, SIGNAL(clicked()), this, SLOT(showAdvancedPage() ) );
    connect( tableNamesBox, SIGNAL( activated(int) ), this, SLOT( selectSrcTable(int) ) );

	connect(boxColor, SIGNAL(colorChanged(const QColor &)), this, SLOT(updatePreviewColor(const QColor &)));
	setFocusProxy(boxFunction);
}

void FitDialog::initEditPage()
{
	QColor blue = QColor(85, 85, 255);
	QPalette palette;
    palette.setColor(QPalette::WindowText, blue);

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(font.pointSize() + 1);

    QGridLayout *gl1 = new QGridLayout();
    QLabel *l = new QLabel(tr("Category"));
    l->setFont(font);
    l->setPalette(palette);
    gl1->addWidget(l, 0, 0);

    l = new QLabel(tr("Function"));
    l->setFont(font);
	l->setPalette(palette);
    gl1->addWidget(l, 0, 1);

    l = new QLabel(tr("Expression"));
    l->setFont(font);
	l->setPalette(palette);
    gl1->addWidget(l, 0, 2);

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

	boxUseBuiltIn = new QCheckBox(tr("Fit with &built-in function"));
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

	buttonPlugins = new QPushButton(tr( "Choose plug&ins folder..." ) );
	buttonPlugins->setIcon(QIcon(":/folder_open.png"));
    hbox1->addWidget(buttonPlugins);
	buttonPlugins->hide();

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("Name")), 0, 0);
	boxName = new QLineEdit(tr("user1"));
    gl2->addWidget(boxName, 0, 1);
	btnAddFunc = new QPushButton(tr( "&Save" ));
	btnAddFunc->setPixmap(QPixmap(":/filesave.png"));
    gl2->addWidget(btnAddFunc, 0, 2);
    gl2->addWidget(new QLabel(tr("Parameters")), 1, 0);
	boxParam = new QLabel();
	boxParam->setFrameStyle(QFrame::Box | QFrame::Sunken);
    gl2->addWidget(boxParam, 1, 1);
	btnDelFunc = new QPushButton( tr( "&Remove" ));
	btnDelFunc->setPixmap(QPixmap(":/close.png"));
    gl2->addWidget(btnDelFunc, 1, 2);

    QGroupBox *gb = new QGroupBox();
    gb->setLayout(gl2);

	editBox = new ScriptEdit(((ApplicationWindow *)parent())->scriptingEnv());
	editBox->enableShortcuts();
	connect(editBox->document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(guessParameters()));
	editBox->setFocus();

	boxErrorMsg = new QLabel();
	boxErrorMsg->setFrameStyle(QFrame::Box | QFrame::Sunken);
	boxErrorMsg->hide();

    QVBoxLayout *vbox1 = new QVBoxLayout();
	btnAddTxt = new QPushButton(tr( "Add &expression" ) );
    vbox1->addWidget(btnAddTxt);
	btnAddName = new QPushButton(tr( "Add &name" ));
    vbox1->addWidget(btnAddName);
	buttonClear = new QPushButton(tr( "Rese&t" ));
    vbox1->addWidget(buttonClear);
	vbox1->addStretch();
	buttonCancel2 = new QPushButton(tr( "&Close" ));
	buttonCancel2->setPixmap(QPixmap(":/delete.png"));
    vbox1->addWidget(buttonCancel2);

	QVBoxLayout *vb = new QVBoxLayout();
	vb->addWidget(editBox);
	vb->addWidget(boxErrorMsg);

    QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addLayout(vb);
    hbox2->addLayout(vbox1);

	palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Window, blue);

	QFrame *frame1 = new QFrame();
	frame1->setFrameStyle(QFrame::Box | QFrame::Plain);
	frame1->setLineWidth(2);
	frame1->setAutoFillBackground(true);
	frame1->setPalette(palette);

	QLabel *selectFunctionLbl = new QLabel(tr("Select Function"));
	selectFunctionLbl->setFont(font);

	int space = 10;
	QHBoxLayout *frameLayout = new QHBoxLayout(frame1);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(selectFunctionLbl);
	frameLayout->addSpacing (space);

	btnContinue = new QPushButton();
	btnContinue->setIcon(QIcon(":/next.png"));
	btnContinue->setIconSize(QSize(16, 17));
	btnContinue->setToolTip(tr("Start Fitting Session"));

	QFrame *frame2 = new QFrame();
	frame2->setFrameShape(QFrame::Box);
	frame2->setLineWidth(2);

	palette = this->palette();
	palette.setColor(QPalette::WindowText, blue);
	frame2->setPalette(palette);

	QLabel *fittingSessionLbl = new QLabel(tr("Fitting Session"));
	fittingSessionLbl->setFont(font);

	frameLayout = new QHBoxLayout(frame2);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(fittingSessionLbl);
	frameLayout->addSpacing (space);

	QGroupBox *g = new QGroupBox();
	QHBoxLayout *hbox3 = new QHBoxLayout(g);
	hbox3->addStretch();
	hbox3->addWidget(frame1);
    hbox3->addWidget(btnContinue);
    hbox3->addWidget(frame2);

    QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->addLayout(gl1);
    vbox2->addLayout(hbox1);
    vbox2->addWidget(gb);
    vbox2->addLayout(hbox2);
	vbox2->addSpacing (10);
	vbox2->addWidget(g);

    editPage = new QWidget();
    editPage->setLayout(vbox2);
    tw->addWidget(editPage);

	connect( buttonPlugins, SIGNAL(clicked()), this, SLOT(chooseFolder()));
    connect( buttonClear, SIGNAL(clicked()), this, SLOT(resetFunction()));
	connect( categoryBox, SIGNAL(currentRowChanged (int)), this, SLOT(showFunctionsList(int)));
	connect( funcBox, SIGNAL(currentRowChanged(int)), this, SLOT(showExpression(int)));
	connect( boxUseBuiltIn, SIGNAL(toggled(bool)), this, SLOT(setFunction(bool)));
	connect( btnAddName, SIGNAL(clicked()), this, SLOT(addFunctionName()));
	connect( btnAddTxt, SIGNAL(clicked()), this, SLOT(addFunction()));
	connect( btnContinue, SIGNAL(clicked()), this, SLOT(showFitPage()));
	connect( btnAddFunc, SIGNAL(clicked()), this, SLOT(saveUserFunction()));
	connect( btnDelFunc, SIGNAL(clicked()), this, SLOT(removeUserFunction()));
	connect( buttonCancel2, SIGNAL(clicked()), this, SLOT(close()));
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
	connect(generatePointsBox, SIGNAL(valueChanged(int)), this, SLOT(enableApplyChanges(int)));
    connect(generatePointsBox, SIGNAL(valueChanged(int)), this, SLOT(updatePreview()));
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

	btnConfidenceLimits = new QPushButton(tr( "Co&nf. Bands" ));
	connect(btnConfidenceLimits, SIGNAL(clicked()), this, SLOT(showConfidenceLimits()));
	gl2->addWidget(btnConfidenceLimits, 3, 0);

	boxConfidenceLevel = new DoubleSpinBox();
	boxConfidenceLevel->setLocale(app->locale());
	boxConfidenceLevel->setDecimals(2);
	boxConfidenceLevel->setRange(0.0, 1.0);
	boxConfidenceLevel->setValue(0.95);
	boxConfidenceLevel->setSingleStep(0.01);
	gl2->addWidget(boxConfidenceLevel, 3, 1);

	btnPredictionLimits = new QPushButton(tr( "Pred. &Bands" ));
	connect(btnPredictionLimits, SIGNAL(clicked()), this, SLOT(showPredictionLimits()));
	gl2->addWidget(btnPredictionLimits, 3, 2);

	btnResiduals = new QPushButton(tr( "&Residuals Plot" ));
	connect(btnResiduals, SIGNAL(clicked()), this, SLOT(showResiduals()));
	gl2->addWidget(btnResiduals, 3, 3);

	scaleErrorsBox = new QCheckBox(tr("&Scale Errors with sqrt(Chi^2/doF)"));
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
	hbox1->addStretch();
	buttonCancel3 = new QPushButton(tr( "&Close" ));
	buttonCancel3->setPixmap(QPixmap(":/delete.png"));
    hbox1->addWidget(buttonCancel3);
	btnApply = new QPushButton(tr( "&Apply" ));
	btnApply->setEnabled(false);
	connect( btnApply, SIGNAL(clicked()), this, SLOT(applyChanges()));
    hbox1->addWidget(btnApply);

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb2);
	vbox1->addWidget(scaleErrorsBox);
    vbox1->addWidget(logBox);
    vbox1->addWidget(plotLabelBox);
    vbox1->addStretch();
    vbox1->addLayout(hbox1);

	QColor blue = QColor(85, 85, 255);
	QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, blue);

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(font.pointSize() + 1);

	QFrame *frame1 = new QFrame();
	frame1->setFrameStyle(QFrame::Box | QFrame::Plain);
	frame1->setLineWidth(2);
	frame1->setPalette(palette);

	QLabel *selectFunctionLbl = new QLabel(tr("Fitting Session"));
	selectFunctionLbl->setFont(font);

	int space = 10;
	QHBoxLayout *frameLayout = new QHBoxLayout(frame1);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(selectFunctionLbl);
	frameLayout->addSpacing (space);

	btnBack = new QPushButton();
	connect( btnBack, SIGNAL(clicked()), this, SLOT(returnToFitPage()));
	btnBack->setIcon(QIcon(":/prev.png"));
	btnBack->setIconSize(QSize(16, 17));
	btnBack->setToolTip(tr("Fitting Session"));

	QFrame *frame2 = new QFrame();
	frame2->setFrameShape(QFrame::Box);
	frame2->setLineWidth(2);

	palette = this->palette();
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Window, blue);
	frame2->setAutoFillBackground(true);
	frame2->setPalette(palette);

	QLabel *fittingSessionLbl = new QLabel(tr("Custom Output"));
	fittingSessionLbl->setFont(font);

	frameLayout = new QHBoxLayout(frame2);
	frameLayout->addSpacing (space);
	frameLayout->addWidget(fittingSessionLbl);
	frameLayout->addSpacing (space);

	QGroupBox *g = new QGroupBox();
	QHBoxLayout *hbox2 = new QHBoxLayout(g);
	hbox2->addWidget(frame1);
	hbox2->addWidget(btnBack);
	hbox2->addWidget(frame2);
	hbox2->addStretch();

	vbox1->addSpacing (10);
	vbox1->addWidget(g);

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
	int prec = boxPrecision->value();
	app->fit_output_precision = prec;
	if (d_current_fit)
		d_current_fit->setOutputPrecision(prec);
	for (int i=0; i<boxParams->rowCount(); i++){
		((DoubleSpinBox*)boxParams->cellWidget(i, 2))->setDecimals(prec);
		((RangeLimitBox*)boxParams->cellWidget(i, 1))->setDecimals(prec);
		((RangeLimitBox*)boxParams->cellWidget(i, 3))->setDecimals(prec);
	}

	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	if (d_current_fit && !d_current_fit->isA("PolynomialFit") &&
		!d_current_fit->isA("LinearFit") && !d_current_fit->isA("LinearSlopeFit"))
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

	if (!d_current_fit || !d_current_fit->dataSize()){
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

	if (!d_current_fit || !d_current_fit->dataSize()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_current_fit->covarianceMatrix(matrixName);
}

void FitDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked()){
		lblPoints->show();
		generatePointsBox->show();
	} else {
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
	if (g->rangeSelectorsEnabled())
		boxCurve->setCurrentIndex(boxCurve->findText(g->curveRange(g->rangeSelectorTool()->selectedCurve())));

	activateCurve(boxCurve->currentText());

	connect (d_graph, SIGNAL(closedGraph()), this, SLOT(close()));
	connect (d_graph, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
};

void FitDialog::activateCurve(const QString& s)
{
	QwtPlotCurve *c = d_graph->curve(s.left(s.indexOf(" [")));
	if (!c)
		return;

	double start, end;
	d_graph->range(c, &start, &end);
    boxFrom->setValue(QMIN(start, end));
    boxTo->setValue(QMAX(start, end));
	//Set the same color as the data curve chosen for fit (Feature Request #4031)
	boxColor->setColor(c->pen().color());
};

void FitDialog::saveUserFunction()
{
	if (editBox->text().isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), tr("Please enter a valid function!"));
		editBox->setFocus();
		return;
	} else if (boxName->text().isEmpty()) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"),
				tr("Please enter a function name!"));
		boxName->setFocus();
		return;
	}

	if (builtInFunctionNames().contains(boxName->text())){
		QMessageBox::critical(this, tr("QtiPlot - Error: function name"),
				"<p><b>" + boxName->text() + "</b>" + tr(" is a built-in function name"
					"<p>You must choose another name for your function!"));
		editBox->setFocus();
		return;
	}

	if (editBox->text().contains(boxName->text())){
		QMessageBox::critical(this, tr("QtiPlot - Input function error"),
				tr("You can't define functions recursively!"));
		editBox->setFocus();
		return;
	}

	QString name = boxName->text();
    QStringList lst = userFunctionNames();
	QString formula = parseFormula(editBox->text().simplified().remove(QRegExp("\\s")));
	if (lst.contains(name)){
		int index = lst.indexOf(name);
		d_current_fit = (NonLinearFit *)d_user_functions[index];
        if (d_current_fit->setFormula(formula))
        	d_current_fit->save(d_current_fit->fileName());

		if (funcBox->currentItem()->text() == name)
			showExpression(index);
	} else {
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		d_current_fit = new NonLinearFit(app, d_graph);
		if (!d_current_fit->setFormula(formula))
			return;

		QString filter = tr("QtiPlot fit model")+" (*.fit);;";
		filter += tr("All files")+" (*.*)";
		QString fn = ApplicationWindow::getFileName(app, tr("QtiPlot") + " - " + tr("Save Fit Model As"),
								app->fitModelsPath + "/" + name, filter, 0, true, app->d_confirm_overwrite);
		if (!fn.isEmpty()){
            QFileInfo fi(fn);
            app->fitModelsPath = fi.dir().absolutePath();
            QString baseName = fi.fileName();
            if (!baseName.contains("."))
                fn.append(".fit");

            d_current_fit->setObjectName(name);

            if (d_current_fit->save(fn)){
                QStringList lst = userFunctionNames();
                lst << name;
                lst.sort();
                int index = lst.indexOf(name);
                d_user_functions.insert (index, d_current_fit);

                if (categoryBox->currentRow() == 0){
                    funcBox->clear();
                    funcBox->addItems(lst);
                    funcBox->setCurrentRow(index);
                }

                if (d_user_functions.count()>0 && !boxUseBuiltIn->isEnabled() && categoryBox->currentRow() == 0)
                    boxUseBuiltIn->setEnabled(true);
            }
		}
	}
}

void FitDialog::removeUserFunction()
{
    QStringList lst = userFunctionNames();
    if (lst.isEmpty())
        return;

    QString s = tr("Are you sure you want to remove fit model file:\n %1 ?").arg(d_current_fit->fileName());
    if (QMessageBox::Yes != QMessageBox::question (this, tr("QtiPlot") + " - " + tr("Remove Fit Model"), s, QMessageBox::Yes, QMessageBox::Cancel))
        return;

	QString name = funcBox->currentItem()->text();
	if (lst.contains(name)){
		explainBox->setText(QString());

		int index = lst.indexOf(name);
		if (0 <= index && index < d_user_functions.size()){
            QFile f((d_user_functions[index])->fileName());
            f.remove();
            d_user_functions.removeAt(index);
		}

        lst.remove(name);
		funcBox->clear();
		funcBox->addItems(lst);
		funcBox->setCurrentRow(0);

		if (!lst.count())
			boxUseBuiltIn->setEnabled(false);
	}
}

void FitDialog::showFitPage()
{
	QString formula = editBox->text().simplified().remove(QRegExp("\\s"));
	if (formula.isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), tr("Please enter a valid function!"));
		editBox->setFocus();
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
    if (!boxUseBuiltIn->isChecked()){
        d_current_fit = new NonLinearFit(app, d_graph);
		formula = parseFormula(formula);
		d_current_fit->setFormula(formula);
    }

	if (d_current_fit->error())
		return;

    if (d_current_fit->type() == Fit::BuiltIn &&
		(d_current_fit->isA("PolynomialFit") || d_current_fit->isA("LinearFit")
		|| d_current_fit->isA("LinearSlopeFit"))){
		btnParamRange->setChecked(false);
		btnParamRange->setEnabled(false);
		showParameterRange(false);
        boxAlgorithm->setEnabled(false);
		boxPoints->setEnabled(false);
		boxTolerance->setEnabled(false);
		btnGuess->setVisible(false);
    } else {
        btnParamRange->setEnabled(true);
        boxAlgorithm->setEnabled(true);
		boxPoints->setEnabled(true);
		boxTolerance->setEnabled(true);
		btnGuess->setVisible(d_current_fit->type() == Fit::BuiltIn);
    }

	QStringList paramList = d_current_fit->parameterNames();
	int parameters = d_current_fit->numParameters();
	boxParams->clearContents();
	boxParams->setRowCount(parameters);
    boxParams->hideColumn(4);

    int aux = parameters;
	if (aux > 7)
		aux = 7;
	boxParams->setMinimumHeight(4 + (aux + 1)*boxParams->horizontalHeader()->height());

	QLocale locale = app->locale();
	int prec = boxPrecision->value();
    for (int i = 0; i<parameters; i++){
        QTableWidgetItem *it = new QTableWidgetItem(paramList[i]);
#ifdef Q_CC_MSVC
        it->setFlags(it->flags() & (~Qt::ItemIsEditable));
#else
        it->setFlags(!Qt::ItemIsEditable);
#endif
        it->setBackground(QBrush(Qt::lightGray));
        it->setForeground(Qt::black);
        QFont font = it->font();
        font.setBold(true);
        it->setFont(font);
        boxParams->setItem(i, 0, it);

		RangeLimitBox *rbl = new RangeLimitBox(RangeLimitBox::LeftLimit);
		rbl->setLocale(locale);
		rbl->setDecimals(prec);
		boxParams->setCellWidget(i, 1, rbl);

		RangeLimitBox *rbr = new RangeLimitBox(RangeLimitBox::RightLimit);
		rbr->setLocale(locale);
		rbr->setDecimals(prec);
		boxParams->setCellWidget(i, 3, rbr);

		DoubleSpinBox *sb = new DoubleSpinBox();
		sb->setLocale(locale);
		sb->setDecimals(prec);
		sb->setValue(d_current_fit->initialGuess(i));
        connect(sb, SIGNAL(valueChanged(double)), this, SLOT(updatePreview()));
        boxParams->setCellWidget(i, 2, sb);

        it = new QTableWidgetItem();
		it->setFlags(!Qt::ItemIsEditable);
		it->setText("--");
		boxParams->setItem(i, 5, it);
	}

    for (int i = 0; i<parameters; i++)
        boxParams->item (i, 0)->setText(paramList[i]);

	if (d_current_fit->type() == Fit::User){
        boxParams->showColumn(4);
		for (int i = 0; i<boxParams->rowCount(); i++ ){
            QTableWidgetItem *it = new QTableWidgetItem();
#ifdef Q_CC_MSVC
            it->setFlags(it->flags() & (~Qt::ItemIsEditable));
#else
            it->setFlags(!Qt::ItemIsEditable);
#endif
            it->setBackground(QBrush(Qt::lightGray));
            boxParams->setItem(i, 4, it);

			QCheckBox *cb = new QCheckBox();
            boxParams->setCellWidget(i, 4, cb);
		}
	}

	boxFunction->setText(formula);
	lblFunction->setText(boxName->text() +" (x, " + boxParam->text().simplified() + ")");

	tw->setCurrentWidget(fitPage);
	if (previewBox->isChecked())
		updatePreview();
}

void FitDialog::showEditPage()
{
	tw->setCurrentWidget(editPage);
}

void FitDialog::showAdvancedPage()
{
	tw->setCurrentWidget (advancedPage);
	if (d_current_fit && (d_current_fit->isA("PolynomialFit") ||
		d_current_fit->isA("LinearFit") || d_current_fit->isA("LinearSlopeFit"))){
		scaleErrorsBox->setChecked(false);
		scaleErrorsBox->setEnabled(false);
	} else {
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		if (app)
			scaleErrorsBox->setChecked(app->fit_scale_errors);
		scaleErrorsBox->setEnabled(true);
	}
}

void FitDialog::setFunction(bool ok)
{
	editBox->setEnabled(!ok);
	boxName->setEnabled(!ok);
	btnAddFunc->setEnabled(!ok);
	btnAddName->setEnabled(!ok);
	btnAddTxt->setEnabled(!ok);
	buttonClear->setEnabled(!ok);

	setCurrentFit(funcBox->currentRow());//reset current fit
	setEditorTextColor(Qt::black);// reset text color to black
	boxErrorMsg->hide();
	boxErrorMsg->clear();

	if (ok){
		boxName->setText(funcBox->currentItem()->text());
		editBox->setText(explainBox->text());
		boxParam->setText(d_current_fit->parameterNames().join(", "));
	}
}

void FitDialog::showFunctionsList(int category)
{
	boxUseBuiltIn->setChecked(false);
	boxUseBuiltIn->setEnabled(false);
	boxUseBuiltIn->hide();
	buttonPlugins->hide();
	btnDelFunc->setEnabled(false);
    funcBox->blockSignals(true);
	funcBox->clear();
    explainBox->clear();
	polynomOrderLabel->hide();
	polynomOrderBox->hide();

	switch (category)
	{
		case 0:
			if (d_user_functions.count() > 0){
				boxUseBuiltIn->setEnabled(true);

                QStringList lst;
                foreach(Fit *fit, d_user_functions)
                    lst << fit->objectName();
                funcBox->addItems(lst);
			}
            buttonPlugins->setText(tr("Choose &models folder..."));
            buttonPlugins->show();
			boxUseBuiltIn->setText(tr("Fit with selected &user function"));
			boxUseBuiltIn->show();
			btnDelFunc->setEnabled(true);
        break;

		case 1:
			boxUseBuiltIn->setText(tr("Fit using &built-in function"));
			boxUseBuiltIn->show();
			boxUseBuiltIn->setEnabled(true);
			funcBox->addItems(builtInFunctionNames());
        break;

		case 2:
			showParseFunctions();
        break;

		case 3:
            buttonPlugins->setText(tr("Choose plug&ins folder..."));
            buttonPlugins->show();
			boxUseBuiltIn->setText(tr("Fit using &plugin function"));
			boxUseBuiltIn->show();
            boxUseBuiltIn->setEnabled(d_plugins.size() > 0);
            QStringList lst;
            foreach(Fit *fit, d_plugins)
                lst << fit->objectName();
            funcBox->addItems(lst);
        break;
	}
    funcBox->blockSignals(false);
	funcBox->setCurrentRow (0);
}

void FitDialog::chooseFolder()
{
	if (categoryBox->currentRow() == 3)//plugins
        choosePluginsFolder();
	else if (!categoryBox->currentRow())//user-defined
	    chooseFitModelsFolder();
}

void FitDialog::choosePluginsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the plugins folder"), app->fitPluginsPath);
        if (!dir.isEmpty()){
            funcBox->clear();
            explainBox->clear();
            app->fitPluginsPath = dir;
            loadPlugins();
        }
}

void FitDialog::chooseFitModelsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the fit models folder"), app->fitModelsPath);
	if (!dir.isEmpty()){
		funcBox->clear();
		explainBox->clear();
		app->fitModelsPath = dir;
		loadUserFunctions();

		QString path = app->fitModelsPath + "/";
		foreach(Fit *fit, d_built_in_functions)
			fit->setFileName(path + fit->objectName() + ".fit");
	}
}

void FitDialog::loadPlugins()
{
    d_plugins.clear();
	typedef char* (*fitFunc)();

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString path = app->fitPluginsPath + "/";
	QString modelsDirPath = app->fitModelsPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks, QDir::Name);
    QStringList names;
	for (int i=0; i<lst.count(); i++){
	    QString file = lst[i];
	    if (QLibrary::isLibrary (file)){
            QLibrary lib(path + file);
            PluginFit *fit = new PluginFit(app, d_graph);
            fit->load(lib.library());
            d_plugins << fit;
            names << fit->objectName();
            fit->setFileName(modelsDirPath + fit->objectName() + ".fit");
	    }
	}

    if (d_plugins.size() > 0){
        funcBox->addItems(names);
        funcBox->setCurrentRow(0);
        boxUseBuiltIn->setEnabled(true);
    } else
        boxUseBuiltIn->setEnabled(false);
}

void FitDialog::showParseFunctions()
{
	funcBox->addItems(MyParser::functionsList());
}

void FitDialog::setCurrentFit(int function)
{
	if (function < 0)
        return;

	switch(categoryBox->currentRow()){
		case 0:
			if (d_user_functions.size() > function)
				d_current_fit = d_user_functions[function];
		break;
		case 1:
			d_current_fit = d_built_in_functions[function];
		break;
		case 2:
		break;
		case 3:
			if (d_plugins.size() > 0)
		    	d_current_fit = d_plugins[function];
		break;
	}
}

void FitDialog::showExpression(int function)
{
    if (function < 0)
        return;

	setCurrentFit(function);
	if (!d_current_fit)
		return;

	switch(categoryBox->currentRow()){
		case 0:
			if (d_user_functions.size() > function)
				explainBox->setText(d_current_fit->formula());
			else
				explainBox->clear();
			setFunction(boxUseBuiltIn->isChecked());
		break;
		case 1:
			polynomOrderLabel->hide();
			polynomOrderBox->hide();
			if (qobject_cast<MultiPeakFit *>(d_current_fit) || qobject_cast<PolynomialFit *>(d_current_fit)){
				if (qobject_cast<MultiPeakFit *>(d_current_fit)){
					polynomOrderLabel->setText(tr("Peaks"));
					((MultiPeakFit *)d_current_fit)->setNumPeaks(polynomOrderBox->value());
				} else {
					polynomOrderLabel->setText(tr("Polynomial Order"));
					((PolynomialFit *)d_current_fit)->setOrder(polynomOrderBox->value());
				}
				polynomOrderLabel->show();
				polynomOrderBox->show();
			}
			explainBox->setText(d_current_fit->formula());
			setFunction(boxUseBuiltIn->isChecked());
		break;
		case 2:
			explainBox->setText(MyParser::explainFunction(function));
		break;
		case 3:
			if (d_plugins.size() > 0){
				explainBox->setText(d_current_fit->formula());
				setFunction(boxUseBuiltIn->isChecked());
			} else
				explainBox->clear();
		break;
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
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	QString curve = boxCurve->currentText();
	QStringList curvesList = d_graph->analysableCurvesList();
	if (!curvesList.contains(curve)){
		QMessageBox::critical(app, tr("QtiPlot - Warning"),
				tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
		boxCurve->clear();
		boxCurve->addItems(curvesList);
		return;
	}

	double start = boxFrom->value();
	double end = boxTo->value();
	double eps = boxTolerance->value();

	if (start >= end){
		QMessageBox::critical(app, tr("QtiPlot - Input error"),
				tr("Please enter x limits that satisfy: from < end!"));
		boxTo->setFocus();
		return;
	}

	int n = 0, rows = boxParams->rowCount();
	if (!boxParams->isColumnHidden(4)){
		for (int i=0; i<rows; i++){//count the non-constant parameters
            QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 4);
			if (!cb->isChecked())
				n++;
		}
	} else
		n = rows;

	QStringList parameters = QStringList();
	MyParser parser;
	bool error = false;
	double *paramsInit = (double *)malloc(n*sizeof(double));
	double *paramRangeLeft = (double *)malloc(n*sizeof(double));
	double *paramRangeRight = (double *)malloc(n*sizeof(double));
	if (!paramsInit || !paramRangeLeft || !paramRangeRight)
		return;

	QString formula = boxFunction->text();
	NonLinearFit *nlf = qobject_cast<NonLinearFit *>(d_current_fit);
	if (nlf)
		nlf->removeConstants();

	try {
		if (!boxParams->isColumnHidden(4)){
			int j = 0;
			for (int i=0; i<rows; i++){
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 4);
				if (!cb->isChecked()){
					paramRangeLeft[j] = ((RangeLimitBox*)boxParams->cellWidget(j, 1))->value();
					paramRangeRight[j] = ((RangeLimitBox*)boxParams->cellWidget(j, 3))->value();
					paramsInit[j] = ((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value();
					parser.DefineVar(boxParams->item(i, 0)->text().ascii(), &paramsInit[j]);
					parameters << boxParams->item(i, 0)->text();
					j++;
				} else {
					double val = ((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value();
					QString constName = boxParams->item(i, 0)->text();
					nlf->setConstant(constName, val);
					parser.DefineConst(constName.ascii(), val);
				}
			}
		} else {
			for (int i=0; i<n; i++) {
				paramRangeLeft[i] = ((RangeLimitBox*)boxParams->cellWidget(i, 1))->value();
				paramRangeRight[i] = ((RangeLimitBox*)boxParams->cellWidget(i, 3))->value();
				paramsInit[i] = ((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value();
				parser.DefineVar(boxParams->item(i, 0)->text().ascii(), &paramsInit[i]);
				parameters << boxParams->item(i, 0)->text();
			}
		}

		parser.SetExpr(formula.ascii());
		double x = start;
		parser.DefineVar("x", &x);
		parser.Eval();
	} catch(mu::ParserError &e) {
		QString errorMsg = boxFunction->text() + " = " + formula + "\n" + QString::fromStdString(e.GetMsg()) + "\n" +
			tr("Please verify that you have initialized all the parameters!");

		QMessageBox::critical(app, tr("QtiPlot - Input function error"), errorMsg);
		boxFunction->setFocus();
		error = true;
	}

	if (!error){
		if (d_current_fit->type() == Fit::BuiltIn)
			modifyGuesses (paramsInit);

		if (nlf){
			if (!nlf->setParametersList(parameters)) return;
			if (!nlf->setFormula(formula, false)) return;
		}

		d_current_fit->setInitialGuesses(paramsInit);

		QString name = boxCurve->currentText();
		name = name.left(name.indexOf(" ["));
		if (!d_current_fit->setDataFromCurve(d_graph->curve(name), start, end) ||
			!d_current_fit->setWeightingData ((Fit::WeightingMethod)boxWeighting->currentIndex(),
						   tableNamesBox->currentText() + "_" + colNamesBox->currentText())) return;

		if (btnParamRange->isEnabled()){
			for (int i = 0; i < n; i++)
				d_current_fit->setParameterRange(i, paramRangeLeft[i], paramRangeRight[i]);
		}

		d_current_fit->setTolerance(eps);
		d_current_fit->setOutputPrecision(app->fit_output_precision);
		d_current_fit->setAlgorithm((Fit::Algorithm)boxAlgorithm->currentIndex());
		d_current_fit->setColor(boxColor->color());
		d_current_fit->generateFunction(generatePointsBtn->isChecked(), generatePointsBox->value());
		d_current_fit->setMaximumIterations(boxPoints->value());
		if (!d_current_fit->isA("PolynomialFit") && !d_current_fit->isA("LinearFit") && !d_current_fit->isA("LinearSlopeFit"))
			d_current_fit->scaleErrors(scaleErrorsBox->isChecked());
		d_current_fit->fit();
		d_result_curves << d_current_fit->resultCurve();
		double *res = d_current_fit->results();
		double *err = d_current_fit->errors();
		QLocale locale = app->locale();
		if (!boxParams->isColumnHidden(4)){
			int j = 0;
			for (int i = 0; i < rows; i++){
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 4);
				if (!cb->isChecked()){
					boxParams->item(i, 5)->setText(QString(QChar(0x00B1)) + " " + locale.toString(err[j], 'e', boxPrecision->value()));
					((DoubleSpinBox*)boxParams->cellWidget(i, 2))->setValue(res[j++]);
				} else
					boxParams->item(i, 5)->setText("--");
			}
		} else {
			for (int i = 0; i < rows; i++){
				((DoubleSpinBox*)boxParams->cellWidget(i, 2))->setValue(res[i]);
				boxParams->item(i, 5)->setText(QString(QChar(0x00B1)) + " " + locale.toString(err[i], 'e', boxPrecision->value()));
			}
		}

		if (globalParamTableBox->isChecked() && d_param_table)
			d_current_fit->writeParametersToTable(d_param_table, true);

		if (previewBox->isChecked())
			updatePreview();

		free (paramsInit);
		free (paramRangeLeft);
		free (paramRangeRight);
	}
}

void FitDialog::modifyGuesses(double* initVal)
{
	if (!d_current_fit)
		return;

	ExponentialFit *ef = qobject_cast<ExponentialFit *>(d_current_fit);
	if (ef){
		if (ef->isExponentialGrowth())
			initVal[1] = -initVal[1];
		return;
	}
}

void FitDialog::changeDataRange()
{
	if (!d_graph->rangeSelectorsEnabled())
		return;

	boxCurve->setCurrentIndex(boxCurve->findText(d_graph->curveRange(d_graph->rangeSelectorTool()->selectedCurve())));

	double start = d_graph->selectedXStartValue();
	double end = d_graph->selectedXEndValue();

	boxFrom->setValue(QMIN(start, end));
	boxTo->setValue(QMAX(start, end));
}

void FitDialog::setSrcTables(QList<MdiSubWindow*> tables)
{
	if (tables.isEmpty()){
		tableNamesBox->addItem(tr("No data tables"));
		colNamesBox->addItem(tr("No data tables"));
		return;
	}

	srcTables = tables;
	tableNamesBox->clear();
	foreach(MdiSubWindow *w, srcTables)
		tableNamesBox->addItem(w->objectName());

	tableNamesBox->setCurrentIndex(tableNamesBox->findText(boxCurve->currentText().split("_", QString::SkipEmptyParts)[0]));
	selectSrcTable(tableNamesBox->currentIndex());
}

void FitDialog::selectSrcTable(int tabnr)
{
	colNamesBox->clear();

	if (tabnr >= 0 && tabnr < srcTables.count()){
		Table *t = (Table*)srcTables.at(tabnr);
		if (t)
			colNamesBox->addItems(t->colNames());
	}
}

void FitDialog::enableWeightingParameters(int index)
{
	bool on = (index == Fit::Dataset || index == Fit::Direct);

	tableNamesBox->setVisible(on);
	colNamesBox->setVisible(on);
}

void FitDialog::closeEvent (QCloseEvent * e)
{
    if (d_graph && d_preview_curve){
        d_preview_curve->detach();
        delete d_preview_curve;
        d_graph->replot();
    }

	if(d_current_fit && d_current_fit->dataSize() && plotLabelBox->isChecked())
		d_current_fit->showLegend();

	if (d_current_fit)
		delete d_current_fit;

	e->accept();
}

void FitDialog::enableApplyChanges(int)
{
	btnApply->setEnabled(true);
}

void FitDialog::deleteFitCurves()
{
	foreach(QwtPlotCurve *c, d_result_curves){
		if (((PlotCurve *)c)->type() != Graph::Function){
			Table *t = ((DataCurve *)c)->table();
			if (t){
				t->askOnCloseEvent(false);
				t->close();
			}
		}
		d_graph->removeCurve(c);
	}

	if (!d_result_curves.isEmpty()){
		d_graph->replot();
		d_result_curves.clear();
	}

	boxCurve->clear();
	boxCurve->addItems(d_graph->analysableCurvesList());
	changeDataRange();
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
	d_built_in_functions << new LinearFit(app, d_graph);
	d_built_in_functions << new LinearSlopeFit(app, d_graph);
	d_built_in_functions << new LogisticFit(app, d_graph);

	fit = new MultiPeakFit(app, d_graph, MultiPeakFit::Lorentz);
	fit->enablePeakCurves(app->generatePeakCurves);
	fit->setPeakCurvesColor(app->peakCurvesColor);
	d_built_in_functions << fit;

	d_built_in_functions << new PolynomialFit(app, d_graph, 1);

    QString path = app->fitModelsPath + "/";
    foreach(Fit *fit, d_built_in_functions)
        fit->setFileName(path + fit->objectName() + ".fit");
}

void FitDialog::setNumPeaks(int peaks)
{
	if (qobject_cast<MultiPeakFit *>(d_current_fit))
		((MultiPeakFit *)d_current_fit)->setNumPeaks(peaks);
	else if (qobject_cast<PolynomialFit *>(d_current_fit))
		((PolynomialFit *)d_current_fit)->setOrder(peaks);

	showExpression(funcBox->currentRow());
}

QStringList FitDialog::builtInFunctionNames()
{
	QStringList lst;
	foreach(Fit *fit, d_built_in_functions)
		lst << fit->objectName();
	return lst;
}

void FitDialog::loadUserFunctions()
{
    d_user_functions.clear();
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString path = app->fitModelsPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks, QDir::Name);
	QStringList names;
	for (int i=0; i<lst.count(); i++){
        NonLinearFit *fit = new NonLinearFit(app, d_graph);
        if (fit->load(path + lst[i])){
            if (fit->type() == Fit::User){
                d_user_functions << fit;
                names << fit->objectName();
            } else if (fit->type() == Fit::BuiltIn){
                QStringList lst = builtInFunctionNames();
                int index = lst.indexOf(fit->objectName());
                if (index >= 0 && index < d_built_in_functions.size()){
                    Fit *f = d_built_in_functions[index];
                    f->setFileName(fit->fileName());
                    for (int i=0; i<f->numParameters(); i++)
                        f->setInitialGuess(i, fit->initialGuess(i));
                }
            } else if (fit->type() == Fit::Plugin){
                QStringList lst = plugInNames();
                int index = lst.indexOf(fit->objectName());
                if (index >= 0 && index < d_plugins.size()){
                    Fit *f = d_plugins[index];
                    f->setFileName(fit->fileName());
                    for (int i=0; i<f->numParameters(); i++)
                        f->setInitialGuess(i, fit->initialGuess(i));
                }
            }
        }
    }

	funcBox->clear();
    if (d_user_functions.size() > 0){
        funcBox->addItems(names);
        funcBox->setCurrentRow(0);
        boxUseBuiltIn->setEnabled(true);
    } else
        boxUseBuiltIn->setEnabled(false);
}

QStringList FitDialog::userFunctionNames()
{
	QStringList lst;
	foreach(Fit *fit, d_user_functions)
		lst << fit->objectName();
	return lst;
}

void FitDialog::saveInitialGuesses()
{
    if (!d_current_fit)
        return;

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QFileInfo fi(app->fitModelsPath);
	if (app->fitModelsPath.isEmpty() || !fi.isDir() || !fi.isWritable())
		chooseFitModelsFolder();

	int rows = boxParams->rowCount();
    for (int i=0; i<rows; i++)
        d_current_fit->setInitialGuess(i, ((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value());

    QString fileName = d_current_fit->fileName();
    if (!fileName.isEmpty())
        d_current_fit->save(fileName);
    else {
		QString filter = tr("QtiPlot fit model") + " (*.fit);;";
		filter += tr("All files") + " (*.*)";
		QString fn = ApplicationWindow::getFileName(app, tr("QtiPlot") + " - " + tr("Save Fit Model As"),
								app->fitModelsPath + "/" + d_current_fit->objectName(), filter, 0, true, app->d_confirm_overwrite);
		if (!fn.isEmpty()){
            QFileInfo fi(fn);
            QString baseName = fi.fileName();
            if (!baseName.contains("."))
                fn.append(".fit");

			QString name = boxName->text();
			bool rename = (qobject_cast<NonLinearFit*>(d_current_fit) && !name.isEmpty() && d_current_fit->objectName() != name);
			if (rename)
				d_current_fit->setObjectName(name);

			if (d_current_fit->save(fn)){
				if (rename)
					loadUserFunctions();
				else
					d_user_functions.append(d_current_fit);
			}
		}
	}
}

void FitDialog::loadInitialGuesses()
{
	if (!d_current_fit)
		return;
	if (!d_current_fit->load(d_current_fit->fileName()))
		return;

	for (int i = 0; i < d_current_fit->numParameters(); i++){
		((DoubleSpinBox*)boxParams->cellWidget(i, 2))->setValue(d_current_fit->initialGuess(i));
		boxParams->item(i, boxParams->columnCount() - 1)->setText("--");
	}

	if (previewBox->isChecked())
		updatePreview();
}

void FitDialog::guessInitialValues()
{
	if (!d_current_fit)
		return;

	if (d_current_fit->type() == Fit::BuiltIn){
		QString name = boxCurve->currentText();
		d_current_fit->setDataFromCurve(name.left(name.indexOf(" [")), boxFrom->value(), boxTo->value());
		d_current_fit->guessInitialValues();
		//modifyGuesses (paramsInit);
	}

	for (int i = 0; i < d_current_fit->numParameters(); i++){
		((DoubleSpinBox*)boxParams->cellWidget(i, 2))->setValue(d_current_fit->initialGuess(i));
		boxParams->item(i, boxParams->columnCount() - 1)->setText("--");
	}

	if (previewBox->isChecked())
		updatePreview();
}

QStringList FitDialog::plugInNames()
{
	QStringList lst;
	foreach(Fit *fit, d_plugins)
		lst << fit->objectName();
	return lst;
}

void FitDialog::returnToFitPage()
{
	applyChanges();
	tw->setCurrentWidget(fitPage);
}

void FitDialog::showPreview(bool on)
{
	if (on)
		updatePreview();
	else if (d_preview_curve){
        d_preview_curve->setVisible(false);
        d_graph->replot();
    }
}

void FitDialog::updatePreview()
{
    if (!d_current_fit || !previewBox->isChecked())
		return;

	if (!d_preview_curve){
		d_preview_curve = new FunctionCurve();
		d_preview_curve->attach(d_graph);
		d_preview_curve->setRenderHint(QwtPlotItem::RenderAntialiased, d_graph->antialiasing());
		d_preview_curve->setPen(QPen(boxColor->color(), 1));
	}

	bool changedVar = false;
	int parameters = boxParams->rowCount();
	QMap<QString, double> variables = d_preview_curve->constants();
	if (variables.size () != parameters)
		changedVar = true;
	else {
		for (int i = 0; i < parameters; i++){
			QString key = boxParams->item(i, 0)->text();
			if (variables.value(key) != ((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value()){
				changedVar = true;
				break;
			}
		}
 	}

	if (!changedVar && d_preview_curve->startRange() == boxFrom->value() &&
		d_preview_curve->endRange() == boxTo->value() &&
		d_preview_curve->formulas()[0] == d_current_fit->formula()){
		if (!d_preview_curve->isVisible()){
			d_preview_curve->setVisible(true);
			d_graph->replot();
		}
		return;
	}

	d_preview_curve->setRange(boxFrom->value(), boxTo->value());
	d_preview_curve->setFormula(d_current_fit->formula());

	if (changedVar){
		d_preview_curve->removeConstants();
		for (int i = 0; i < parameters; i++)
			d_preview_curve->setConstant(boxParams->item(i, 0)->text(),
						((DoubleSpinBox*)boxParams->cellWidget(i, 2))->value());
	}

	d_preview_curve->loadData(generatePointsBox->value());
	if (!d_preview_curve->isVisible())
		d_preview_curve->setVisible(true);
	d_graph->replot();
}

void FitDialog::updatePreviewColor(const QColor &c)
{
    if (!d_preview_curve)
		return;

	d_preview_curve->setPen(QPen(c, 1));
	d_graph->replot();
}

void FitDialog::showParameterRange(bool on)
{
    if (on){
        boxParams->showColumn(1);
        boxParams->showColumn(3);
    } else {
        boxParams->hideColumn(1);
        boxParams->hideColumn(3);
    }
}

QString FitDialog::parseFormula(const QString& s)
{
	QString formula = s;

	QStringList lst = userFunctionNames();
	for (int i=0; i<lst.count(); i++){
		if (formula.contains(lst[i]))
			formula.replace(lst[i], "(" + (d_user_functions[i])->formula() + ")");
	}

	QStringList builtInFunctions = builtInFunctionNames();
	for (int i=0; i<(int)builtInFunctions.count(); i++){
		if (formula.contains(builtInFunctions[i])){
			Fit *fit = d_built_in_functions[i];
			formula.replace(builtInFunctions[i], "(" + fit->formula() + ")");
		}
	}
	return formula;
}

void FitDialog::showResiduals()
{
	if (!d_current_fit || !d_current_fit->dataSize()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_current_fit->showResiduals();
}

void FitDialog::showConfidenceLimits()
{
	if (!d_current_fit || !d_current_fit->dataSize()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_current_fit->showConfidenceLimits(boxConfidenceLevel->value());
}

void FitDialog::showPredictionLimits()
{
	if (!d_current_fit || !d_current_fit->dataSize()){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	d_current_fit->showPredictionLimits(boxConfidenceLevel->value());
}

void FitDialog::guessParameters()
{
	if (boxUseBuiltIn->isChecked())
		return;

	QString text = editBox->text().remove(QRegExp("\\s")).remove(".");
	if (text.isEmpty()){
		boxParam->clear();
		return;
	}

	bool error = false;
	string errMsg;
	boxParam->setText(NonLinearFit::guessParameters(text, &error, &errMsg).join(", "));
	if (error){
		setEditorTextColor(Qt::red);// highlight text in red
		boxErrorMsg->setText(tr("Error: ") + QString(errMsg.c_str()));
		boxErrorMsg->show();
	} else {
		setEditorTextColor(Qt::black);// reset text color to black
		boxErrorMsg->hide();
		boxErrorMsg->clear();
	}
}

void FitDialog::setEditorTextColor(const QColor& c)
{
	QPalette palette = editBox->palette();
	palette.setColor (QPalette::Text, c);
	editBox->setPalette(palette);
}
