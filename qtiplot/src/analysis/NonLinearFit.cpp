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
#include <MyParser.h>
#include <FunctionCurve.h>

#include <QApplication>
#include <QMessageBox>
#include <QTextStream>

NonLinearFit::NonLinearFit(ApplicationWindow *parent, Graph *g)
: Fit(parent, g)
{
	init();
}

NonLinearFit::NonLinearFit(ApplicationWindow *parent, QwtPlotCurve *c)
: Fit(parent, c)
{
	init();
	setDataFromCurve(c);
}

NonLinearFit::NonLinearFit(ApplicationWindow *parent, QwtPlotCurve *c, double start, double end)
: Fit(parent, c)
{
	init();
	setDataFromCurve(c, start, end);
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

bool NonLinearFit::setParametersList(const QStringList& lst)
{
	if (lst.count() < 1){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot - Fit Error"),
				tr("You must provide a list containing at least one parameter for this type of fit. Operation aborted!"));
		d_init_err = true;
		if (d_p > 0)
			freeWorkspace();
		d_p = 0;
		return false;
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

	return true;
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
		double step = (d_x[d_n - 1] - X0)/(d_points - 1);
		for (int i=0; i<d_points; i++){
		    x = X0 + i*step;
			X[i] = x;
			Y[i] = parser.EvalRemoveSingularity(&x, false);
		}
	} else {
		for (int i=0; i<d_points; i++) {
		    x = d_x[i];
			X[i] = x;
			Y[i] = parser.EvalRemoveSingularity(&x, false);
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
    return parser.EvalRemoveSingularity(&x, false);
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

FunctionCurve * NonLinearFit::insertFitFunctionCurve(const QString& name, int penWidth, bool updateData)
{
	FunctionCurve *c = Fit::insertFitFunctionCurve(name, penWidth, false);
	if (c){
		QMapIterator<QString, double> i(d_constants);
 		while (i.hasNext()) {
     		i.next();
			c->setConstant(i.key(), i.value());
 		}
	}
	if (updateData)
		c->loadData(d_points);
	return c;
}

QStringList NonLinearFit::guessParameters(const QString& s, bool *error, string *errMsg, const QString& var)
{
	QString text = s;
	text.remove(QRegExp("\\s")).remove(".");

	QStringList parList;
	try {
		MyParser parser;
		ParserTokenReader reader(&parser);

		QLocale locale = QLocale();

		const char *formula = text.toAscii().data();
		int length = text.toAscii().length();
		reader.SetFormula (formula);
		reader.IgnoreUndefVar(true);
		int pos = 0;
		while(pos < length){
			ParserToken<value_type, string_type> token = reader.ReadNextToken();
			QString str = QString(token.GetAsString().c_str());

			bool isNumber;
			locale.toDouble(str, &isNumber);

			if (token.GetCode () == cmVAR && str.contains(QRegExp("\\D"))
				&& str != var && !parList.contains(str) && !isNumber){
				if (str.endsWith("e", Qt::CaseInsensitive) &&
					str.count("e", Qt::CaseInsensitive) == 1){

					QString aux = str;
					aux.remove("e", Qt::CaseInsensitive);

					bool auxIsNumber;
					locale.toDouble(aux, &auxIsNumber);
					if (!auxIsNumber)
						parList << str;
				} else
					parList << str;
			}

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

bool NonLinearFit::removeDataSingularities()
{
	MyParser parser;
	for (int i = 0; i < d_p; i++){
		double param = gsl_vector_get(d_param_init, i);
		parser.DefineVar(d_param_names[i].ascii(), &param);
	}

	QMapIterator<QString, double> it(d_constants);
 	while (it.hasNext()) {
     	it.next();
		parser.DefineConst(it.key().ascii(), it.value());
 	}

	double xvar;
	parser.DefineVar("x", &xvar);
	parser.SetExpr(d_formula.ascii());

	bool confirm = true;
	for (int i = 0; i < d_n; i++){
		xvar = d_x[i];
		try {
			parser.EvalRemoveSingularity(&xvar);
		} catch(MyParser::Pole){
			QApplication::restoreOverrideCursor();
			if(confirm){
				switch(QMessageBox::question((ApplicationWindow *)parent(), QObject::tr("QtiPlot"),
				QObject::tr("Found non-removable singularity at x = %1.").arg(xvar) + "\n" + tr("Ignore") + "?",
				QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel,
				QMessageBox::Yes)){
					case QMessageBox::YesToAll:
						confirm = false;
						removePole(i);
					break;
					case QMessageBox::Cancel:
						return false;
					default:
						removePole(i);
				}
			} else
				removePole(i);
		}
	}
	return true;
}

void NonLinearFit::removePole(int pole)
{
	int n = d_n - 1;
	double *aux_x = (double *)malloc(n*sizeof(double));
	if (!aux_x)
		return;

	double *aux_y = (double *)malloc(n*sizeof(double));
	if (!aux_y){
		free (aux_x);
		return;
	}

	double *aux_w = (double *)malloc(n*sizeof(double));
	if (!aux_w){
		free (aux_x);
		free (aux_y);
		return;
	}

	for (int i = 0; i < pole; i++){
		aux_x [i] = d_x[i];
		aux_y [i] = d_y[i];
		aux_w [i] = d_w[i];
	}

	for (int i = pole + 1; i < d_n; i++){
		int j = i - 1;
		aux_x [j] = d_x[i];
		aux_y [j] = d_y[i];
		aux_w [j] = d_w[i];
	}

	free (d_x);
	free (d_y);
	free (d_w);

	d_x = aux_x;
	d_y = aux_y;
	d_w = aux_w;
	d_n = n;
}
