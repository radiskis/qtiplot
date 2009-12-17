/***************************************************************************
    File                 : SurfaceDialog.cpp
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
#include "SurfaceDialog.h"
#include "Graph3D.h"
#include <MyParser.h>
#include <ApplicationWindow.h>
#include <DoubleSpinBox.h>
#include <ScriptEdit.h>

#include <QMessageBox>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <QCompleter>
#include <QApplication>
#include <QInputDialog>

SurfaceDialog::SurfaceDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setName( "SurfaceDialog" );
	setWindowTitle(tr("QtiPlot - Define surface plot"));
    setSizeGripEnabled( true );

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(new QLabel(tr( "Surface type" )));
	boxType = new QComboBox();
	boxType->addItem( tr( "Function" ) );
	boxType->addItem( tr( "Parametric" ) );
	hbox1->addWidget(boxType);
	hbox1->addStretch();

	optionStack = new QStackedWidget();

	initFunctionPage();
	initParametricSurfacePage();

	buttonClear = new QPushButton(tr("Clea&r"));
	buttonOk = new QPushButton(tr("&OK"));
    buttonOk->setDefault(true);
    buttonCancel = new QPushButton(tr("&Close"));

    QBoxLayout *bl2 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl2->addStretch();
	bl2->addWidget(buttonClear);
	bl2->addWidget(buttonOk);
	bl2->addWidget(buttonCancel);

	QVBoxLayout* vl = new QVBoxLayout(this);
    vl->addLayout(hbox1);
	vl->addWidget(optionStack);
	vl->addLayout(bl2);

	d_graph = 0;
    setFocusProxy(boxFunction);

	connect( boxType, SIGNAL(activated(int)), optionStack, SLOT(setCurrentIndex(int)));
	connect( buttonClear, SIGNAL(clicked()), this, SLOT(clearFunction()));
    connect( buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect( buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void SurfaceDialog::initFunctionPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();

	boxFunction = new ScriptEdit(app->scriptingEnv());
	boxFunction->enableShortcuts();
	boxFunction->setFocus();

	QBoxLayout *bl1 = new QBoxLayout (QBoxLayout::LeftToRight);

	QVBoxLayout *vl1 = new QVBoxLayout();
	vl1->addWidget(new QLabel( tr("f(x,y)=")));

	buttonRecentFunc = new QPushButton(tr("Rece&nt"));
	buttonRecentFunc->setToolTip(tr("Click here to select a recently typed expression"));
	connect(buttonRecentFunc, SIGNAL(clicked()), this, SLOT(showFunctionLog()));
	vl1->addWidget(buttonRecentFunc);
	vl1->addStretch();

	bl1->addLayout(vl1, 1);
	bl1->addWidget(boxFunction, 10);

    QGroupBox *gb1 = new QGroupBox(tr("X - axis"));

	boxXFrom = new DoubleSpinBox();
	boxXFrom->setLocale(locale);

	boxXTo = new DoubleSpinBox();
	boxXTo->setLocale(locale);
	boxXTo->setValue(1.0);

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel( tr("From")), 0, 0);
    gl1->addWidget(boxXFrom, 0, 1);
    gl1->addWidget(new QLabel(tr("To")), 1, 0);
    gl1->addWidget(boxXTo, 1, 1);
    gl1->setRowStretch(2, 1);
	gl1->setColumnStretch(1, 10);
    gb1->setLayout(gl1);

    QGroupBox *gb2 = new QGroupBox(tr("Y - axis"));
	boxYFrom = new DoubleSpinBox();
	boxYFrom->setLocale(locale);
	boxYTo = new DoubleSpinBox();
	boxYTo->setLocale(locale);
	boxYTo->setValue(1.0);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("From")), 0, 0);
    gl2->addWidget(boxYFrom, 0, 1);
    gl2->addWidget(new QLabel(tr("To")), 1, 0);
    gl2->addWidget(boxYTo, 1, 1);
    gl2->setRowStretch(2, 1);
	gl2->setColumnStretch(1, 10);
    gb2->setLayout(gl2);

    QGroupBox *gb3 = new QGroupBox(tr("Z - axis"));
	boxZFrom = new DoubleSpinBox();
	boxZFrom->setLocale(locale);
	boxZTo = new DoubleSpinBox();
	boxZTo->setLocale(locale);
	boxZTo->setValue(1.0);

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel( tr("From")), 0, 0);
    gl3->addWidget(boxZFrom, 0, 1);
    gl3->addWidget(new QLabel(tr("To")), 1, 0);
    gl3->addWidget(boxZTo, 1, 1);
    gl3->setRowStretch(2, 1);
	gl3->setColumnStretch(1, 10);
    gb3->setLayout(gl3);

	QBoxLayout *bl2 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);
	bl2->addWidget(gb3);

	QGroupBox *gb4 = new QGroupBox(tr("Mesh"));
	boxFuncColumns = new QSpinBox();
	boxFuncColumns->setRange(1, 1000);
	boxFuncColumns->setValue(40);

	boxFuncRows = new QSpinBox();
	boxFuncRows->setRange(1, 1000);
	boxFuncRows->setValue(40);

	QGridLayout *hb4 = new QGridLayout(gb4);
    hb4->addWidget(new QLabel( tr("Columns")), 0, 0);
    hb4->addWidget(boxFuncColumns, 0, 1);
    hb4->addWidget(new QLabel(tr("Rows")), 1, 0);
    hb4->addWidget(boxFuncRows, 1, 1);

	functionPage = new QWidget();

	QVBoxLayout* vl = new QVBoxLayout(functionPage);
	vl->addLayout(bl1);
	vl->addLayout(bl2);
	vl->addWidget(gb4);
	vl->addStretch();

	optionStack->addWidget(functionPage);
}

void SurfaceDialog::initParametricSurfacePage()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	int prec = 6;
	if (app)
		prec = app->d_decimal_digits;

	int maxH = 80;
	boxX = new ScriptEdit(app->scriptingEnv());
	boxX->setMaximumHeight(maxH);
	boxX->enableShortcuts();

	boxY = new ScriptEdit(app->scriptingEnv());
	boxY->setMaximumHeight(maxH);
	boxY->enableShortcuts();

	boxZ = new ScriptEdit(app->scriptingEnv());
	boxZ->setMaximumHeight(maxH);
	boxZ->enableShortcuts();

	QString recentTip = tr("Click here to select a recently typed expression");
	QString recentBtnText = tr("Rece&nt");

	buttonXLog = new QPushButton(recentBtnText);
	buttonXLog->setToolTip(recentTip);
	connect(buttonXLog, SIGNAL(clicked()), this, SLOT(showXLog()));

	buttonYLog = new QPushButton(recentBtnText);
	buttonYLog->setToolTip(recentTip);
	connect(buttonYLog, SIGNAL(clicked()), this, SLOT(showYLog()));

	buttonZLog = new QPushButton(recentBtnText);
	buttonZLog->setToolTip(recentTip);
	connect(buttonZLog, SIGNAL(clicked()), this, SLOT(showZLog()));

	QGroupBox *gb = new QGroupBox(tr("Equations"));
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("X(u,v)=")), 0, 0);
    gl->addWidget(boxX, 0, 1);
	gl->addWidget(buttonXLog, 0, 2);

    gl->addWidget(new QLabel(tr("Y(u,v)=")), 1, 0);
    gl->addWidget(boxY, 1, 1);
	gl->addWidget(buttonYLog, 1, 2);

	gl->addWidget(new QLabel(tr("Z(u,v)=")), 2, 0);
    gl->addWidget(boxZ, 2, 1);
	gl->addWidget(buttonZLog, 2, 2);

    gl->setRowStretch(3, 1);

    QGroupBox *gb1 = new QGroupBox(tr("u"));
	boxUFrom = new DoubleSpinBox();
	boxUFrom->setDecimals(prec);
	boxUFrom->setLocale(locale);

	boxUTo = new DoubleSpinBox();
	boxUTo->setLocale(locale);
	boxUTo->setDecimals(prec);
	boxUTo->setValue(M_PI);

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel( tr("From")), 0, 0);
    gl1->addWidget(boxUFrom, 0, 1);
    gl1->addWidget(new QLabel(tr("To")), 1, 0);
    gl1->addWidget(boxUTo, 1, 1);
	boxUPeriodic = new QCheckBox(tr("Periodic"));
	gl1->addWidget(boxUPeriodic, 2, 1);
    gl1->setRowStretch(3, 1);
	gl1->setColumnStretch(1, 10);
    gb1->setLayout(gl1);

    QGroupBox *gb2 = new QGroupBox(tr("v"));
	boxVFrom = new DoubleSpinBox();
	boxVFrom->setDecimals(prec);
	boxVFrom->setLocale(locale);

	boxVTo = new DoubleSpinBox();
	boxVTo->setLocale(locale);
	boxVTo->setDecimals(prec);
	boxVTo->setValue(M_PI);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("From")), 0, 0);
    gl2->addWidget(boxVFrom, 0, 1);
    gl2->addWidget(new QLabel(tr("To")), 1, 0);
    gl2->addWidget(boxVTo, 1, 1);
	boxVPeriodic = new QCheckBox(tr("Periodic"));
	gl2->addWidget(boxVPeriodic, 2, 1);
    gl2->setRowStretch(3, 1);
	gl2->setColumnStretch(1, 10);
    gb2->setLayout(gl2);

	QGroupBox *gb3 = new QGroupBox(tr("Mesh"));
	boxColumns = new QSpinBox();
	boxColumns->setRange(1, 1000);
	boxColumns->setValue(40);

	boxRows = new QSpinBox();
	boxRows->setRange(1, 1000);
	boxRows->setValue(40);

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel( tr("Columns")), 0, 0);
    gl3->addWidget(boxColumns, 0, 1);
    gl3->addWidget(new QLabel(tr("Rows")), 1, 0);
    gl3->addWidget(boxRows, 1, 1);
    gl3->setRowStretch(2, 1);
    gb3->setLayout(gl3);

	QBoxLayout *bl2 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);
	bl2->addWidget(gb3);

	parametricPage = new QWidget();

	QVBoxLayout* vl = new QVBoxLayout(parametricPage);
    vl->addWidget(gb);
	vl->addLayout(bl2);

	optionStack->addWidget(parametricPage);
}

void SurfaceDialog::clearFunction()
{
	if (boxType->currentIndex()){
		boxX->clear();
		boxY->clear();
		boxZ->clear();
	} else
		boxFunction->clear();
}

void SurfaceDialog::setFunction(Graph3D *g)
{
	if (!g)
		return;

	d_graph = g;
	UserFunction *f = d_graph->userFunction();
	if (!f)
		return;

	boxFunction->setText(f->function());
	boxFuncColumns->setValue(f->columns());
	boxFuncRows->setValue(f->rows());
	boxXFrom->setValue(g->xStart());
	boxXTo->setValue(g->xStop());
	boxYFrom->setValue(g->yStart());
	boxYTo->setValue(g->yStop());
	boxZFrom->setValue(g->zStart());
	boxZTo->setValue(g->zStop());
}

void SurfaceDialog::accept()
{
	if (boxType->currentIndex())
		acceptParametricSurface();
	else
		acceptFunction();
}

void SurfaceDialog::acceptParametricSurface()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	MyParser parser;
	double u = 1.0, v = 1.0;
	parser.DefineVar("u", &u);
	parser.DefineVar("v", &v);

    int list_size = 15;
	QString x_formula = boxX->text().simplified();
	try {
		parser.SetExpr(x_formula.ascii());
		parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(app, tr("QtiPlot - X Formula Error"), QString::fromStdString(e.GetMsg()));
		boxX->setFocus();
		return;
	}

    app->d_param_surface_func.remove(x_formula);
	app->d_param_surface_func.push_front(x_formula);
	while ((int)app->d_param_surface_func.size() > list_size)
		app->d_param_surface_func.pop_back();

	QString y_formula = boxY->text().simplified();
	try {
		parser.SetExpr(y_formula.ascii());
		parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(app, tr("QtiPlot - Y Formula Error"), QString::fromStdString(e.GetMsg()));
		boxY->setFocus();
		return;
	}

    app->d_param_surface_func.remove(y_formula);
	app->d_param_surface_func.push_front(y_formula);
	while ((int)app->d_param_surface_func.size() > list_size)
		app->d_param_surface_func.pop_back();

	QString z_formula = boxZ->text().simplified();
	try {
		parser.SetExpr(z_formula.ascii());
		parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(app, tr("QtiPlot - Z Formula Error"), QString::fromStdString(e.GetMsg()));
		boxZ->setFocus();
		return;
	}

    app->d_param_surface_func.remove(z_formula);
	app->d_param_surface_func.push_front(z_formula);
	while ((int)app->d_param_surface_func.size() > list_size)
		app->d_param_surface_func.pop_back();

	double ul = boxUFrom->value();
	double ur = boxUTo->value();
	double vl = boxVFrom->value();
	double vr = boxVTo->value();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (!d_graph)
		app->plotParametricSurface(x_formula, y_formula, z_formula,
							   ul, ur, vl, vr, boxColumns->value(), boxRows->value(),
							   boxUPeriodic->isChecked(), boxVPeriodic->isChecked());
	else
		d_graph->addParametricSurface(x_formula, y_formula, z_formula,
							   ul, ur, vl, vr, boxColumns->value(), boxRows->value(),
							   boxUPeriodic->isChecked(), boxVPeriodic->isChecked());
    QApplication::restoreOverrideCursor();
	close();
}

void SurfaceDialog::acceptFunction()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	double fromX = boxXFrom->value();
	double toX = boxXTo->value();
	double fromY = boxYFrom->value();
	double toY = boxYTo->value();
	double fromZ = boxZFrom->value();
	double toZ = boxZTo->value();

	if (fromX >= toX || fromY >= toY || fromZ >= toZ){
		QMessageBox::critical(app, tr("QtiPlot - Input error"),
					tr("Please enter limits that satisfy: from < end!"));
		boxXTo->setFocus();
		return;
	}

	QString formula = boxFunction->text().simplified();
	bool error = false;
	try{
		MyParser parser;
		double x,y;
		parser.DefineVar("x", &x);
		parser.DefineVar("y", &y);
		parser.SetExpr(formula.ascii());

		x = fromX; y = fromY;
		parser.Eval();
		x = toX; y = toY;
		parser.Eval();
	} catch(mu::ParserError &e){
		QMessageBox::critical(app, tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		boxFunction->setFocus();
		error = true;
	}

	if (!error){
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		if (!d_graph)
			app->plotSurface(formula, fromX, toX, fromY, toY, fromZ, toZ,
						 boxFuncColumns->value(), boxFuncRows->value());
		else
			d_graph->addFunction(formula,fromX, toX, fromY, toY, fromZ, toZ,
						 boxFuncColumns->value(), boxFuncRows->value());

		app->updateSurfaceFuncList(formula);
		QApplication::restoreOverrideCursor();
		close();
	}
}

void SurfaceDialog::setParametricSurface(Graph3D *g)
{
	if (!g)
		return;

	d_graph = g;
	UserParametricSurface *s = d_graph->parametricSurface();

	boxType->setCurrentIndex(1);
	optionStack->setCurrentIndex(1);

	boxX->setText(s->xFormula());
	boxY->setText(s->yFormula());
	boxZ->setText(s->zFormula());

	boxUFrom->setValue(s->uStart());
	boxUTo->setValue(s->uEnd());
	boxVFrom->setValue(s->vStart());
	boxVTo->setValue(s->vEnd());

	boxColumns->setValue(s->columns());
	boxRows->setValue(s->rows());

	boxUPeriodic->setChecked(s->uPeriodic());
	boxVPeriodic->setChecked(s->vPeriodic());
}

void SurfaceDialog::showFunctionLog()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	if (app->surfaceFunc.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), app->surfaceFunc, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxFunction->setText(s);
}

void SurfaceDialog::showXLog()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	if (app->d_param_surface_func.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), app->d_param_surface_func, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxX->setText(s);
}

void SurfaceDialog::showYLog()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	if (app->d_param_surface_func.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), app->d_param_surface_func, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxY->setText(s);
}

void SurfaceDialog::showZLog()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;
	
	if (app->d_param_surface_func.isEmpty()){
		QMessageBox::information(this, tr("QtiPlot"), tr("Sorry, there are no recent expressions available!"));
		return;
	}

	bool ok;
	QString s = QInputDialog::getItem(this, tr("QtiPlot") + " - " + tr("Recent Functions"), tr("Please, choose a function:"), app->d_param_surface_func, 0, false, &ok);
	if (ok && !s.isEmpty())
		boxZ->setText(s);
}
