/***************************************************************************
    File                 : IntDialog.cpp
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
#include "IntDialog.h"
#include <Integration.h>
#include <ApplicationWindow.h>
#include <Graph.h>
#include <DoubleSpinBox.h>
#include <ScriptEdit.h>
#include <MyParser.h>

#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QLayout>
#include <QInputDialog>

#include <gsl/gsl_math.h>

IntDialog::IntDialog(QWidget* parent, Graph *g, Qt::WFlags fl )
    : QDialog( parent, fl),
	d_graph(g)
{
	setName( "IntegrationDialog" );
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("QtiPlot - Integration Options"));
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

	QGroupBox *gb1 = new QGroupBox(tr("Function"));
	QGridLayout *gl1 = new QGridLayout(gb1);

	boxMathFunctions = new QComboBox();
	boxMathFunctions->addItems(MyParser::functionsList());

	QVBoxLayout *vl1 = new QVBoxLayout();
	vl1->addWidget(boxMathFunctions);

	addFunctionBtn = new QPushButton(tr("&Add" ));
	addFunctionBtn->setAutoDefault(false);
	connect(addFunctionBtn, SIGNAL(clicked()), this, SLOT(insertFunction()));
	vl1->addWidget(addFunctionBtn);

	buttonClear = new QPushButton(tr("Clea&r" ));
	connect(buttonClear, SIGNAL( clicked() ), this, SLOT(clearFunction()));
	vl1->addWidget(buttonClear);

	buttonFunctionLog = new QPushButton(tr("Rece&nt") );
	buttonFunctionLog->setToolTip(tr("Click here to select a recently typed expression"));
	connect(buttonFunctionLog, SIGNAL(clicked()), this, SLOT(showFunctionLog()));
	vl1->addWidget(buttonFunctionLog);

	vl1->addStretch();
	gl1->addLayout(vl1, 0, 0);

	boxFunction = new ScriptEdit(((ApplicationWindow *)parent)->scriptingEnv());
	boxFunction->enableShortcuts();
	gl1->addWidget(boxFunction, 0, 1);

	gl1->addWidget(new QLabel(tr("Variable")), 1, 0);
	boxVariable = new QLineEdit();
	boxVariable->setText("x");
	gl1->addWidget(boxVariable, 1, 1);

	gl1->addWidget(new QLabel(tr("Subintervals")), 2, 0);
	boxSteps = new QSpinBox();
	boxSteps->setRange(1, INT_MAX);
	boxSteps->setValue(1000);
	boxSteps->setSingleStep(100);
	gl1->addWidget(boxSteps, 2, 1);

	QLocale locale = ((ApplicationWindow *)parent)->locale();
	gl1->addWidget(new QLabel(tr("Tolerance")),3, 0);
	boxTol = new DoubleSpinBox();
	boxTol->setLocale(locale);
	boxTol->setValue(0.01);
	boxTol->setMinimum(0.0);
	boxTol->setSingleStep(0.001);
	gl1->addWidget(boxTol, 3, 1);

	gl1->addWidget(new QLabel(tr("Lower limit")), 4, 0);
	boxStart = new DoubleSpinBox();
	boxStart->setLocale(locale);
	gl1->addWidget(boxStart, 4, 1);

	gl1->addWidget(new QLabel(tr("Upper limit")), 5, 0);
	boxEnd = new DoubleSpinBox();
	boxEnd->setLocale(locale);
	boxEnd->setValue(1.0);
	gl1->addWidget(boxEnd, 5, 1);

	boxPlot = new QCheckBox(tr("&Plot area"));
	boxPlot->setChecked(true);
	gl1->addWidget(boxPlot, 6, 1);
	gl1->setRowStretch(0, 1);

	buttonOk = new QPushButton(tr( "&Integrate" ));
	buttonOk->setDefault( true );
	buttonCancel = new QPushButton(tr("&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
	vl->addWidget(buttonOk);
	vl->addWidget(buttonCancel);
	vl->addStretch();

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->addWidget(gb1);
	hb->addLayout(vl);

	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void IntDialog::accept()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	QString function = boxFunction->text().simplified();
	if (!validInput(function))
		return;

	app->updateFunctionLists(0, QStringList() << function);

	Integration *i = new Integration(function, boxVariable->text(), app, d_graph, boxStart->value(), boxEnd->value());
	if (!i->error()){
		i->setTolerance(boxTol->text().toDouble());
		i->setWorkspaceSize(boxSteps->value());
		i->enableGraphicsDisplay(d_graph && boxPlot->isChecked(), d_graph);
		i->run();
	}
	delete i;
}

bool IntDialog::validInput(const QString& function)
{
	int points = 100;
	double start = boxStart->value();
	double end = boxEnd->value();
	double step = (end - start)/(double)(points - 1.0);
	double x = end;

	MyParser parser;
	parser.DefineVar(boxVariable->text().ascii(), &x);
	parser.SetExpr(function.ascii());

	try {
		parser.Eval();
	} catch(mu::ParserError &e) {
		QMessageBox::critical(this, tr("QtiPlot - Input error"), QString::fromStdString(e.GetMsg()));
		return false;
	}

	x = start;
	int lastButOne = points - 1;
	try {
		double xl = x, xr;
		double y = parser.Eval();
		bool wellDefinedFunction = true;
		if (!gsl_finite(y)){// try to find a first well defined point (might help for some not really bad functions)
			wellDefinedFunction = false;
			for (int i = 0; i < lastButOne; i++){
				xl = x;
				x += step;
				xr = x;
				y = parser.Eval();
				if (gsl_finite(y)){
					wellDefinedFunction = true;
					int iter = 0;
					double x0 = x, y0 = y;
					while(fabs(xr - xl)/step > 1e-15 && iter < points){
						x = 0.5*(xl + xr);
						y = parser.Eval();
						if (gsl_finite(y)){
							xr = x;
							x0 = x;
							y0 = y;
						} else
							xl = x;
						iter++;
					}
					start = x0;
					step = (start - end)/(double)(lastButOne);
					break;
				}
			}
			if (!wellDefinedFunction){
				QMessageBox::critical(0, QObject::tr("QtiPlot"),
				QObject::tr("The function %1 is not defined in the specified interval!").arg(function));
				return false;
			}
		}
	} catch (MyParser::Pole) {return false;}

	return true;
}

void IntDialog::insertFunction()
{
	QString fname = boxMathFunctions->currentText().remove("(").remove(")").remove(",").remove(";");
	boxFunction->insertFunction(fname);
	boxFunction->setFocus();
}

void IntDialog::clearFunction()
{
	boxFunction->clear();
}

void IntDialog::showFunctionLog()
{
	ApplicationWindow *d_app = (ApplicationWindow *)this->parent();
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
