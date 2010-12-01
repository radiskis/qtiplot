/***************************************************************************
    File                 : FunctionDialog.cpp
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
#include "FunctionDialog.h"
#include <MyParser.h>
#include <ApplicationWindow.h>
#include <MultiLayer.h>
#include <FunctionCurve.h>
#include <DoubleSpinBox.h>
#include <ScriptEdit.h>
#include <NonLinearFit.h>

#include <QTextEdit>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QCompleter>
#include <QStringListModel>
#include <QInputDialog>
#include <QApplication>
#include <QDialogButtonBox>

FunctionDialog::FunctionDialog(ApplicationWindow* parent, bool standAlone, Qt::WFlags fl )
: QDialog( parent, fl ), d_app(parent), d_active_editor(0), d_stand_alone(standAlone)
{
	QLocale locale = QLocale();
	int prec = 6;
	if (d_app){
		locale = d_app->locale();
		prec = d_app->d_decimal_digits;
	}

    setObjectName( "FunctionDialog" );
	QString recentTip = tr("Click here to select a recently typed expression");
	QString recentBtnText = tr("Rece&nt");

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(new QLabel(tr( "Curve type " )));
	boxType = new QComboBox();
	boxType->addItem( tr( "Function" ) );
	boxType->addItem( tr( "Parametric plot" ) );
	boxType->addItem( tr( "Polar plot" ) );
	hbox1->addWidget(boxType);

	optionStack = new QStackedWidget();
	optionStack->setFrameShape( QFrame::StyledPanel );
	optionStack->setFrameShadow( QStackedWidget::Plain );

	QVBoxLayout *vl = new QVBoxLayout();
	vl->addWidget(new QLabel(tr( "f(x)= " )));

	buttonFunctionLog = new QPushButton(recentBtnText);
	buttonFunctionLog->setToolTip(recentTip);
	connect(buttonFunctionLog, SIGNAL(clicked()), this, SLOT(showFunctionLog()));
	vl->addWidget(buttonFunctionLog);
	vl->addStretch();

	QGridLayout *gl1 = new QGridLayout();
	gl1->addLayout(vl, 0, 0);

	boxFunction = new ScriptEdit(d_app->scriptingEnv());
	boxFunction->enableShortcuts();
	connect(boxFunction, SIGNAL(textChanged()), this, SLOT(guessConstants()));
	gl1->addWidget(boxFunction, 0, 1);

	gl1->addWidget(new QLabel(tr( "From x= " )), 1, 0);
	boxFrom = new DoubleSpinBox();
	boxFrom->setLocale(locale);
	boxFrom->setDecimals(prec);
	gl1->addWidget(boxFrom, 1, 1);
	gl1->addWidget(new QLabel(tr( "To x= " )), 2, 0);
	boxTo = new DoubleSpinBox();
	boxTo->setValue(1);
	boxTo->setLocale(locale);
	boxTo->setDecimals(prec);
	gl1->addWidget(boxTo, 2, 1);
	gl1->addWidget(new QLabel(tr( "Points" )), 3, 0);
	boxPoints = new QSpinBox();
	boxPoints->setRange(2, INT_MAX);
	boxPoints->setSingleStep(100);
	boxPoints->setValue(100);
	gl1->addWidget(boxPoints, 3, 1);

	boxConstants = new QTableWidget();
    boxConstants->setColumnCount(2);
    boxConstants->horizontalHeader()->setClickable(false);
    boxConstants->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
	boxConstants->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
    QStringList header = QStringList() << tr("Constant") << tr("Value");
    boxConstants->setHorizontalHeaderLabels(header);
    boxConstants->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    boxConstants->verticalHeader()->hide();
	boxConstants->setMinimumWidth(200);
	boxConstants->hide();

	functionPage = new QWidget();

	QHBoxLayout *hb = new QHBoxLayout(functionPage);
	hb->addLayout(gl1);
	optionStack->addWidget( functionPage );

	QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr( "Parameter" )), 0, 0);
	boxParameter = new QLineEdit();
	boxParameter->setText("m");
	gl2->addWidget(boxParameter, 0, 1);

	int maxH = 80;

	boxXFunction = new ScriptEdit(d_app->scriptingEnv());
	boxXFunction->setMaximumHeight(maxH);
	boxXFunction->enableShortcuts();
	connect(boxXFunction, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	connect(boxXFunction, SIGNAL(textChanged()), this, SLOT(guessConstants()));
	gl2->addWidget(boxXFunction, 1, 1);

	buttonXParLog = new QPushButton(recentBtnText);
	buttonXParLog->setToolTip(recentTip);
	connect(buttonXParLog, SIGNAL(clicked()), this, SLOT(showXParLog()));

	QVBoxLayout *vlxpar = new QVBoxLayout();
	vlxpar->addWidget(new QLabel(tr( "x = " )));
	vlxpar->addWidget(buttonXParLog);
	vlxpar->addStretch();
	gl2->addLayout(vlxpar, 1, 0);

	boxYFunction = new ScriptEdit(d_app->scriptingEnv());
	boxYFunction->setMaximumHeight(maxH);
	boxYFunction->enableShortcuts();
	connect(boxYFunction, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	connect(boxYFunction, SIGNAL(textChanged()), this, SLOT(guessConstants()));
	gl2->addWidget(boxYFunction, 2, 1);

	buttonYParLog = new QPushButton(recentBtnText);
	buttonYParLog->setToolTip(recentTip);
	connect(buttonYParLog, SIGNAL(clicked()), this, SLOT(showYParLog()));

	QVBoxLayout *vlypar = new QVBoxLayout();
	vlypar->addWidget(new QLabel(tr( "y = " )));
	vlypar->addWidget(buttonYParLog);
	vlypar->addStretch();
	gl2->addLayout(vlypar, 2, 0);

	gl2->addWidget(new QLabel(tr( "From" )), 3, 0);
	boxParFrom = new DoubleSpinBox();
	boxParFrom->setLocale(locale);
	boxParFrom->setDecimals(prec);
	gl2->addWidget(boxParFrom, 3, 1);

	gl2->addWidget(new QLabel(tr( "To" )), 4, 0);
	boxParTo = new DoubleSpinBox();
	boxParTo->setValue(1);
	boxParTo->setLocale(locale);
	boxParTo->setDecimals(prec);
	gl2->addWidget(boxParTo, 4, 1);

	gl2->addWidget(new QLabel(tr( "Points" )), 5, 0);
	boxParPoints = new QSpinBox();
	boxParPoints->setRange(2, INT_MAX);
	boxParPoints->setSingleStep(100);
	boxParPoints->setValue(100);
	gl2->addWidget(boxParPoints, 5, 1);
	gl2->setRowStretch(6, 1);

	parametricPage = new QWidget();
	parametricPage->setLayout(gl2);
	optionStack->addWidget( parametricPage );

	QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel(tr( "Parameter" )), 0, 0);
	boxPolarParameter = new QLineEdit();
	boxPolarParameter->setText ("t");
	gl3->addWidget(boxPolarParameter, 0, 1);

	boxPolarRadius = new ScriptEdit(d_app->scriptingEnv());
	boxPolarRadius->setMaximumHeight(maxH);
	boxPolarRadius->enableShortcuts();
	connect(boxPolarRadius, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	connect(boxPolarRadius, SIGNAL(textChanged()), this, SLOT(guessConstants()));
	gl3->addWidget(boxPolarRadius, 1, 1);

	buttonPolarRadiusLog = new QPushButton(recentBtnText);
	buttonPolarRadiusLog->setToolTip(recentTip);
	connect(buttonPolarRadiusLog, SIGNAL(clicked()), this, SLOT(showPolarRadiusLog()));

	QVBoxLayout *vlrpar = new QVBoxLayout();
	vlrpar->addWidget(new QLabel(tr( "R =" )));
	vlrpar->addWidget(buttonPolarRadiusLog);
	vlrpar->addStretch();
	gl3->addLayout(vlrpar, 1, 0);

	boxPolarTheta = new ScriptEdit(d_app->scriptingEnv());
	boxPolarTheta->setMaximumHeight(maxH);
	boxPolarTheta->enableShortcuts();
	connect(boxPolarTheta, SIGNAL(activated(ScriptEdit *)), this, SLOT(setActiveEditor(ScriptEdit *)));
	connect(boxPolarTheta, SIGNAL(textChanged()), this, SLOT(guessConstants()));
	gl3->addWidget(boxPolarTheta, 2, 1);

	buttonPolarRThetaLog = new QPushButton(recentBtnText);
	buttonPolarRThetaLog->setToolTip(recentTip);
	connect(buttonPolarRThetaLog, SIGNAL(clicked()), this, SLOT(showPolarThetaLog()));

	QVBoxLayout *vltpar = new QVBoxLayout();
	vltpar->addWidget(new QLabel(tr( "Theta =" )));
	vltpar->addWidget(buttonPolarRThetaLog);
	vltpar->addStretch();
	gl3->addLayout(vltpar, 2, 0);

	gl3->addWidget(new QLabel(tr( "From" )), 3, 0);
	boxPolarFrom = new DoubleSpinBox();
	boxPolarFrom->setLocale(locale);
	boxPolarFrom->setDecimals(prec);
	gl3->addWidget(boxPolarFrom, 3, 1);

	gl3->addWidget(new QLabel(tr( "To" )), 4, 0);
	boxPolarTo = new DoubleSpinBox();
	boxPolarTo->setValue(M_PI);
	boxPolarTo->setLocale(locale);
	boxPolarTo->setDecimals(prec);
	gl3->addWidget(boxPolarTo, 4, 1);

	gl3->addWidget(new QLabel(tr( "Points" )), 5, 0);
	boxPolarPoints = new QSpinBox();
	boxPolarPoints->setRange(2, INT_MAX);
	boxPolarPoints->setSingleStep(100);
	boxPolarPoints->setValue(100);
	gl3->addWidget(boxPolarPoints, 5, 1);
	gl3->setRowStretch(6, 1);

	polarPage = new QWidget();
	polarPage->setLayout(gl3);
	optionStack->addWidget( polarPage );

	boxFunctionExplain = new QTextEdit;
	boxFunctionExplain->setReadOnly(true);
	boxFunctionExplain->setMaximumHeight(80);
	QPalette palette = boxFunctionExplain->palette();
	palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
	boxFunctionExplain->setPalette(palette);

	addFunctionBtn = new QPushButton(tr( "&Add Function" ));
	addFunctionBtn->setAutoDefault(false);
	connect(addFunctionBtn, SIGNAL(clicked()), this, SLOT(insertFunction()));

	boxMathFunctions = new QComboBox();
	boxMathFunctions->addItems(MyParser::functionsList());
	connect(boxMathFunctions, SIGNAL(activated(int)), this, SLOT(updateFunctionExplain(int)));
	updateFunctionExplain(0);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->addWidget(boxMathFunctions);
	vbox->addWidget(addFunctionBtn);

	buttonClear = new QPushButton(tr( "Clea&r Function" ));
	connect( buttonClear, SIGNAL( clicked() ), this, SLOT(clearList()));
	vbox->addWidget(buttonClear);
	vbox->addStretch();

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->addWidget(boxFunctionExplain);
	hbox3->addLayout(vbox);

	QVBoxLayout *vbox2 = new QVBoxLayout();
	vbox2->addLayout(hbox3);
	vbox2->addStretch();

	hb = new QHBoxLayout();
	hb->addWidget(optionStack);
	hb->addWidget(boxConstants);

	QVBoxLayout *vbox1 = new QVBoxLayout(this);
	vbox1->addLayout(hbox1);
	vbox1->addLayout(hb);
	vbox1->addLayout(vbox2);
	vbox1->addStretch();

	if (standAlone){
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
		buttonBox->setCenterButtons(true);
		connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonClicked(QAbstractButton *)));
		vbox1->addWidget(buttonBox);

		setSizeGripEnabled(true);
		setWindowTitle( tr( "QtiPlot - Add function curve" ) );
		setAttribute(Qt::WA_DeleteOnClose);
	}

	setFocusProxy (boxFunction);
	connect( boxType, SIGNAL( activated(int) ), this, SLOT( raiseWidget(int) ) );

	curveID = -1;
	graph = 0;
}

void FunctionDialog::buttonClicked(QAbstractButton *btn)
{
	switch(buttonBox->buttonRole(btn)){
		case QDialogButtonBox::AcceptRole:
			accept();
		break;
		case QDialogButtonBox::RejectRole:
			reject();
		break;
		default:
			apply();
		break;
	}
}

void FunctionDialog::raiseWidget(int index)
{
	optionStack->setCurrentIndex(index);
	guessConstants();
}

void FunctionDialog::setCurveToModify(FunctionCurve *c)
{
	Graph *g = (Graph *)c->plot();
	if (!g)
		return;

	setCurveToModify(g, g->curveIndex(c));
}

void FunctionDialog::setCurveToModify(Graph *g, int curve)
{
	if (!g)
		return;

	graph = g;

	FunctionCurve *c = (FunctionCurve *)graph->curve(curve);
	if (!c)
		return;

	curveID = curve;
	QStringList formulas = c->formulas();

	QMap<QString, double> constants = c->constants();
	if (!constants.isEmpty()){
		boxConstants->clearContents();
		boxConstants->setRowCount(constants.size());
		boxConstants->show();
		QMapIterator<QString, double> i(constants);
		int row = 0;
 		while (i.hasNext()) {
     		i.next();
			boxConstants->setItem(row, 0, new QTableWidgetItem(i.key()));

			DoubleSpinBox *sb = new DoubleSpinBox();
			sb->setLocale(QLocale());
			sb->setValue(i.value());
        	boxConstants->setCellWidget(row, 1, sb);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(apply()));
			row++;
 		}
	} else
		boxConstants->hide();

	if (c->functionType() == FunctionCurve::Normal){
		boxFunction->blockSignals(true);
		boxFunction->setText(formulas[0]);
		boxFunction->blockSignals(false);

		boxFrom->setValue(QMIN(c->startRange(), c->endRange()));
		boxTo->setValue(QMAX(c->startRange(), c->endRange()));
		boxPoints->setValue(c->dataSize());
	} else if (c->functionType() == FunctionCurve::Polar) {
		boxPolarRadius->blockSignals(true);
		boxPolarRadius->setText(formulas[0]);
		boxPolarRadius->blockSignals(false);

		boxPolarTheta->blockSignals(true);
		boxPolarTheta->setText(formulas[1]);
		boxPolarTheta->blockSignals(false);

		boxPolarParameter->setText(c->variable());
		boxPolarFrom->setValue(c->startRange());
		boxPolarTo->setValue(c->endRange());
		boxPolarPoints->setValue(c->dataSize());
	} else if (c->functionType() == FunctionCurve::Parametric) {
		boxXFunction->blockSignals(true);
		boxXFunction->setText(formulas[0]);
		boxXFunction->blockSignals(false);

		boxYFunction->blockSignals(true);
		boxYFunction->setText(formulas[1]);
		boxYFunction->blockSignals(false);

		boxParameter->setText(c->variable());
		boxParFrom->setValue(c->startRange());
		boxParTo->setValue(c->endRange());
		boxParPoints->setValue(c->dataSize());
	}
	boxType->setCurrentIndex(c->functionType());
	optionStack->setCurrentIndex(c->functionType());
}

void FunctionDialog::clearList()
{
	int type = boxType->currentItem();
	switch (type)
	{
		case 0:
			boxFunction->clear();
			break;

		case 1:
			boxXFunction->clear();
			boxYFunction->clear();
			break;

		case 2:
			boxPolarTheta->clear();
			boxPolarRadius->clear();
			break;
	}
}

bool FunctionDialog::acceptFunction()
{
	double start = boxFrom->value();
	double end = boxTo->value();
	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter x limits that satisfy: from < end!"));
		boxTo->setFocus();
		return false;
	}

	QMap<QString, double> constants;
	QString formula = boxFunction->text().simplified();
	try {
		double x = start;
		MyParser parser;
		parser.DefineVar("x", &x);
		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty()){
				constants.insert(constName, val);
				parser.DefineConst(constName.ascii(), val);
			}
		}
		parser.SetExpr(formula.ascii());

		parser.Eval();
		x = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxFunction->setFocus();
		return false;
	}

	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += formula;
	if (d_app){
		d_app->updateFunctionLists(type, formulas);
		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxPoints->value(), "x", type);
			if (plot){
				graph = plot->activeLayer();
				setConstants((FunctionCurve *)graph->curve(graph->curveCount() - 1), constants);
			}
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, "x", start, end, boxPoints->value(), constants);
			else
				setConstants(graph->addFunction(formulas, start, end, boxPoints->value(), "x", type), constants);
		}
	}
	return true;
}

bool FunctionDialog::acceptParametric()
{
	double start = boxParFrom->value();
	double end = boxParTo->value();

	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxParTo->setFocus();
		return false;
	}

	double parameter;
	QString xformula = boxXFunction->text().simplified();
	QString yformula = boxYFunction->text().simplified();

	QMap<QString, double> constants;
	try {
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);
		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty()){
				constants.insert(constName, val);
				parser.DefineConst(constName.ascii(), val);
			}
		}
		parser.SetExpr(xformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxXFunction->setFocus();
		return false;
	}

	try {
		MyParser parser;
		parser.DefineVar((boxParameter->text()).ascii(), &parameter);

		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty())
				parser.DefineConst(constName.ascii(), val);
		}
		parser.SetExpr(yformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxYFunction->setFocus();
		return false;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += xformula;
	formulas += yformula;
	if (d_app){
		d_app->updateFunctionLists(type, formulas);
		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxParPoints->value(), boxParameter->text(), type);
			if (plot){
				graph = plot->activeLayer();
				setConstants((FunctionCurve *)graph->curve(graph->curveCount() - 1), constants);
			}
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, boxParameter->text(), start, end, boxParPoints->value(), constants);
			else
				setConstants(graph->addFunction(formulas, start, end, boxParPoints->value(), boxParameter->text(), type), constants);
		}
	}
	return true;
}

bool FunctionDialog::acceptPolar()
{
	double start = boxPolarFrom->value();
	double end = boxPolarTo->value();
	if (start >= end){
		QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter parameter limits that satisfy: from < end!"));
		boxPolarTo->setFocus();
		return false;
	}

	double parameter;
	QString rformula = boxPolarRadius->text().simplified();
	QString tformula = boxPolarTheta->text().simplified();

	QMap<QString, double> constants;
	try {
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty()){
				constants.insert(constName, val);
				parser.DefineConst(constName.ascii(), val);
			}
		}
		parser.SetExpr(rformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarRadius->setFocus();
		return false;
	}

	try {
		MyParser parser;
		parser.DefineVar((boxPolarParameter->text()).ascii(), &parameter);
		for (int i = 0; i < boxConstants->rowCount(); i++){
			double val = ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
			QString constName = boxConstants->item(i, 0)->text();
			if (!constName.isEmpty())
				parser.DefineConst(constName.ascii(), val);
		}
		parser.SetExpr(tformula.ascii());

		parameter = start;
		parser.Eval();
		parameter = end;
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxPolarTheta->setFocus();
		return false;
	}
	// Collecting all the information
	int type = boxType->currentItem();
	QStringList formulas;
	formulas += rformula;
	formulas += tformula;
	if (d_app){
		d_app->updateFunctionLists(type, formulas);

		if (!graph){
			MultiLayer *plot = d_app->newFunctionPlot(formulas, start, end, boxPolarPoints->value(), boxPolarParameter->text(), type);
			if (plot){
				graph = plot->activeLayer();
				setConstants((FunctionCurve *)graph->curve(graph->curveCount() - 1), constants);
			}
		} else {
			if (curveID >= 0)
				graph->modifyFunctionCurve(curveID, type, formulas, boxPolarParameter->text(), start, end, boxPolarPoints->value(), constants);
			else
				setConstants(graph->addFunction(formulas, start, end, boxPolarPoints->value(), boxPolarParameter->text(), type), constants);
		}
	}
	return true;
}

void FunctionDialog::accept()
{
	if (apply())
		close();
}

bool FunctionDialog::apply()
{
	switch (boxType->currentIndex()){
		case 0:
			return acceptFunction();
			break;

		case 1:
			return acceptParametric();
			break;

		case 2:
			return acceptPolar();
			break;
	}
	return false;
}

void FunctionDialog::showFunctionLog()
{
	if (!d_app)
		return;

	if (d_app->d_recent_functions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->d_recent_functions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxFunction->setText(s);
}

void FunctionDialog::showXParLog()
{
	if (!d_app)
		return;

	if (d_app->xFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->xFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxXFunction->setText(s);
}


void FunctionDialog::showYParLog()
{
	if (!d_app)
		return;

	if (d_app->yFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->yFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxYFunction->setText(s);
}

void FunctionDialog::showPolarRadiusLog()
{
	if (!d_app)
		return;

	if (d_app->rFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->rFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxPolarRadius->setText(s);
}


void FunctionDialog::showPolarThetaLog()
{
	if (!d_app)
		return;

	if (d_app->thetaFunctions.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), d_app->thetaFunctions, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxPolarTheta->setText(s);
}

void FunctionDialog::insertFunction()
{
	QString fname = boxMathFunctions->currentText().remove("(").remove(")").remove(",").remove(";");
	if (optionStack->currentWidget () == functionPage){
		boxFunction->insertFunction(fname);
		boxFunction->setFocus();
	} else if (optionStack->currentWidget () == parametricPage){
		if (d_active_editor == boxYFunction){
			boxYFunction->insertFunction(fname);
			boxYFunction->setFocus();
		} else if (d_active_editor == boxXFunction){
			boxXFunction->insertFunction(fname);
			boxXFunction->setFocus();
		}
	} else if (optionStack->currentWidget () == polarPage){
		if (d_active_editor == boxPolarRadius){
			boxPolarRadius->insertFunction(fname);
			boxPolarRadius->setFocus();
		} else if (d_active_editor == boxPolarTheta){
			boxPolarTheta->insertFunction(fname);
			boxPolarTheta->setFocus();
		}
	}
}

void FunctionDialog::updateFunctionExplain(int index)
{
	boxFunctionExplain->setText(MyParser::explainFunction(index));
}

void FunctionDialog::guessConstants()
{
	QString text;
	QString var = "x";
	switch (boxType->currentIndex()){
		case 0:
			text = boxFunction->text().remove(QRegExp("\\s")).remove(".");
			break;

		case 1:
			text = boxXFunction->text().remove(QRegExp("\\s")).remove(".");
			text += "+" + boxYFunction->text().remove(QRegExp("\\s")).remove(".");
			var = boxParameter->text();
			break;

		case 2:
			text = boxPolarRadius->text().remove(QRegExp("\\s")).remove(".");
			text += "+" + boxPolarTheta->text().remove(QRegExp("\\s")).remove(".");
			var = boxPolarParameter->text();
			break;
	}

	bool error = false;
	string errMsg;
	QStringList lst = NonLinearFit::guessParameters(text, &error, &errMsg, var);
	if (error)
		return;

	QStringList constants;
	QList<double> values;
	for (int i = 0; i < boxConstants->rowCount(); i++){
		constants << boxConstants->item(i, 0)->text();
		values << ((DoubleSpinBox*)boxConstants->cellWidget(i, 1))->value();
	}

	if (lst == constants)
		return;

	boxConstants->setRowCount(lst.size());

	int row = 0;
	foreach(QString s, lst){
		boxConstants->setItem(row, 0, new QTableWidgetItem(s));

		DoubleSpinBox *sb = new DoubleSpinBox();
		sb->setLocale(QLocale());
		boxConstants->setCellWidget(row, 1, sb);
		if (constants.contains(s)){
			int index = constants.indexOf(s);
			if (index >= 0 && index < values.size())
				sb->setValue(values[index]);
		}
		if (!d_stand_alone)
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(apply()));
		row++;
	}

	if (!lst.size())
		boxConstants->hide();
	else if (!boxConstants->isVisible())
		boxConstants->show();
}

void FunctionDialog::setConstants(FunctionCurve *c, const QMap<QString, double>& constants)
{
	if (!c || constants.isEmpty())
		return;

	c->setConstants(constants);
	c->loadData();
	if (graph){
		graph->replot();
		graph->updateMarkersBoundingRect();
		graph->updateSecondaryAxis(QwtPlot::xTop);
		graph->updateSecondaryAxis(QwtPlot::yRight);
		graph->replot();
	}
}
