/***************************************************************************
    File                 : NonLinearFit.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : NonLinearFit class

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
#include "NonLinearFit.h"
#include "fit_gsl.h"
#include "../MyParser.h"
#include "../plot2D/FunctionCurve.h"

#include <QApplication>
#include <QMessageBox>
#include <QTextStream>

NonLinearFit::NonLinearFit(ApplicationWindow *parent, Graph *g)
: Fit(parent, g)
{
	init();
}

NonLinearFit::NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle)
: Fit(parent, g)
{
	init();
	setDataFromCurve(curveTitle);
}

NonLinearFit::NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end)
: Fit(parent, g)
{
	init();
	setDataFromCurve(curveTitle, start, end);
}

NonLinearFit::NonLinearFit(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int startRow, int endRow)
: Fit(parent, t)
{
	init();
	setDataFromTable(t, xCol, yCol, startRow, endRow);
}

void NonLinearFit::init()
{
    if (objectName().isEmpty())
        setObjectName(tr("NonLinear"));
	d_formula = QString::null;
	d_f = user_f;
	d_df = user_df;
	d_fdf = user_fdf;
	d_fsimplex = user_d;
	d_explanation = tr("Non-linear Fit");
    d_fit_type = User;
}

bool NonLinearFit::setFormula(const QString& s, bool guess)
{
	if (s.isEmpty()){
		QMessageBox::critical((ApplicationWindow *)parent(),  tr("QtiPlot - Input function error"),
				tr("Please enter a valid non-empty expression! Operation aborted!"));
		d_init_err = true;
		return false;
	}

	if (d_formula == s)
		return true;

	if (guess)
		setParametersList(guessParameters(s));
	if (!d_p){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("There are no parameters specified for this fit operation. Please define a list of parameters first!"));
		d_init_err = true;
		return false;
	}
	
	try {
		double *param = new double[d_p];
		MyParser parser;
		double xvar;
		parser.DefineVar("x", &xvar);
		for (int k = 0; k < (int)d_p; k++){
			param[k] = gsl_vector_get(d_param_init, k);
			parser.DefineVar(d_param_names[k].ascii(), &param[k]);
		}
		
		QMapIterator<QString, double> i(d_constants);
 		while (i.hasNext()) {
     		i.next();
			parser.DefineConst(i.key().ascii(), i.value());
 		}
	
		parser.SetExpr(s.ascii());
		parser.Eval() ;
		delete[] param;
	} catch(mu::ParserError &e){
		QMessageBox::critical((ApplicationWindow *)parent(),  tr("QtiPlot - Input function error"), QString::fromStdString(e.GetMsg()));
		d_init_err = true;
		return false;
	}

	d_init_err = false;
	d_formula = s;
	return true;
}

void NonLinearFit::setParametersList(const QStringList& lst)
{
	if (lst.count() < 1){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("You must provide a list containing at least one parameter for this type of fit. Operation aborted!"));
		d_init_err = true;
		return;
	}

	d_init_err = false;
	d_param_names = lst;

	if (d_p > 0)
		freeWorkspace();
	d_p = (int)lst.count();
	initWorkspace(d_p);

	d_param_explain.clear();
	for (int i=0; i<d_p; i++)
		d_param_explain << "";
}

void NonLinearFit::calculateFitCurveData(double *X, double *Y)
{
	MyParser parser;
	for (int i=0; i<d_p; i++)
		parser.DefineVar(d_param_names[i].ascii(), &d_results[i]);

	QMapIterator<QString, double> i(d_constants);
 	while (i.hasNext()) {
     	i.next();
		parser.DefineConst(i.key().ascii(), i.value());
 	}
	
	double x;
	parser.DefineVar("x", &x);
	parser.SetExpr(d_formula.ascii());

	if (d_gen_function){
		double X0 = d_x[0];
		double step = (d_x[d_n-1]-X0)/(d_points-1);
		for (int i=0; i<d_points; i++){
		    x = X0 + i*step;
			X[i] = x;
			Y[i] = parser.Eval();
		}
	} else {
		for (int i=0; i<d_points; i++) {
		    x = d_x[i];
			X[i] = x;
			Y[i] = parser.Eval();
		}
	}
}

double NonLinearFit::eval(double *par, double x)
{
	MyParser parser;
	for (int i=0; i<d_p; i++)
		parser.DefineVar(d_param_names[i].ascii(), &par[i]);
	
	QMapIterator<QString, double> i(d_constants);
 	while (i.hasNext()) {
     	i.next();
		parser.DefineConst(i.key().ascii(), i.value());
 	}
		
	parser.DefineVar("x", &x);
	parser.SetExpr(d_formula.ascii());
    return parser.Eval();
}
		
void NonLinearFit::setConstant(const QString& parName, double val)
{	
	d_constants.insert(parName, val);			
}

QString NonLinearFit::logFitInfo(int iterations, int status)
{
	QString info = Fit::logFitInfo(iterations, status);
	if (d_constants.isEmpty())
		return info;
	
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	
	QMapIterator<QString, double> i(d_constants);
 	while (i.hasNext()) {
     	i.next();
		info += i.key() + " = " + locale.toString(i.value(), 'g', d_prec) + " ("+ tr("constant") + ")\n";
 	}

	info += "---------------------------------------------------------------------------------------\n";
	return info;
}

QString NonLinearFit::legendInfo()
{
	QString info = Fit::legendInfo();
	if (d_constants.isEmpty())
		return info;
	
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = app->locale();
	
	QMapIterator<QString, double> i(d_constants);
 	while (i.hasNext()) {
     	i.next();
		info += "\n" + i.key() + " = " + locale.toString(i.value(), 'g', d_prec) + " ("+ tr("constant") + ")";
 	}
	return info;
}

FunctionCurve * NonLinearFit::insertFitFunctionCurve(const QString& name, double *x, double *y, int penWidth)
{
	FunctionCurve *c = Fit::insertFitFunctionCurve(name, x, y, penWidth);
	if (c){
		QMapIterator<QString, double> i(d_constants);
 		while (i.hasNext()) {
     		i.next();
			c->setConstant(i.key(), i.value());
 		}
	}
	return c;
}

QStringList NonLinearFit::guessParameters(const QString& s, bool *error, string *errMsg)
{
	QString text = s;
	text.remove(QRegExp("\\s")).remove(".");
		
	QStringList parList;
	try {
		MyParser parser;
		ParserTokenReader reader(&parser);
		
		const char *formula = text.toAscii().data();
		int length = text.toAscii().length();
		reader.SetFormula (formula);
		reader.IgnoreUndefVar(true);
		int pos = 0;
		while(pos < length){
			ParserToken<value_type, string_type> token = reader.ReadNextToken();
			QString str = QString(token.GetAsString().c_str());
			if (token.GetCode () == cmVAR && str.contains(QRegExp("\\D")) 
				&& str != "x" && !parList.contains(str))
				parList << str;
			pos = reader.GetPos();
		}
		parList.sort();
	} catch(mu::ParserError &e) { 
		if (error){
			*error = true;
			*errMsg = e.GetMsg();
		}
		return parList;	
	}
	if (error)
		*error = false;
	return parList;
}
