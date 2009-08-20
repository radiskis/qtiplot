/***************************************************************************
    File                 : MyParser.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Parser class based on muParser

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
#include "MyParser.h"
#include <QMessageBox>
#include <QApplication>
#include "NonLinearFit.h"
#include "muParserScripting.h"

MyParser::MyParser()
:Parser()
{
	DefineConst("pi", M_PI);
	DefineConst("Pi", M_PI);
	DefineConst("PI", M_PI);
	DefineConst("e", M_E);
	DefineConst("E", M_E);

	for (const muParserScripting::mathFunction *i=muParserScripting::math_functions; i->name; i++){
		if (i->numargs == 1 && i->fun1 != NULL)
			DefineFun(i->name, i->fun1);
		else if (i->numargs == 2 && i->fun2 != NULL)
			DefineFun(i->name, i->fun2);
		else if (i->numargs == 3 && i->fun3 != NULL)
			DefineFun(i->name, i->fun3);
	}
	gsl_set_error_handler_off();
}

const QStringList MyParser::functionsList()
{
  QStringList l;
  for (const muParserScripting::mathFunction *i = muParserScripting::math_functions; i->name; i++){
    if (i->numargs == 2)
      l << QString(i->name) + "(,)";
	else
      l << QString(i->name) + "()";
  }
  return l;
}

QString MyParser::explainFunction(int index)
{
	const muParserScripting::mathFunction i = muParserScripting::math_functions[index];
	return i.description;
}

double MyParser::EvalRemoveSingularity(double *xvar, bool noisy) const
{
	try {
		double result = Eval();
		if ( gsl_isinf(result) || gsl_isnan(result))
			throw Singularity();
		return result;
	} catch (Singularity) {
	    try {
			if (isinf(Eval()))
				throw Pole();
			double backup = *xvar;
			int n;
			frexp (*xvar, &n);
			double xp = *xvar + ldexp(DBL_EPSILON, n);
			double xm = *xvar - ldexp(DBL_EPSILON, n);
			*xvar = xp;
			double yp = Eval();
			if (gsl_isinf(yp) || gsl_isnan(yp))
				throw Pole();
			*xvar = xm;
			double ym = Eval();
			if (gsl_isinf(ym) || gsl_isnan(ym))
				throw Pole();
			*xvar = backup;
			return 0.5*(yp + ym);
	    } catch (Pole) {
	        if (noisy){
	        	QApplication::restoreOverrideCursor();
				QMessageBox::critical(0, QObject::tr("QtiPlot - Math Error"),
				QObject::tr("Found non-removable singularity at x = %1.").arg(*xvar));
				throw Pole();
			}
			return GSL_NAN;
	    }
	}
}

//almost verbatim copy from Parser::Diff, adapted to use EvalRemoveSingularity()

double MyParser::DiffRemoveSingularity(double *xvar, double *a_Var, double a_fPos) const
{
    double fRes(0),
		   fBuf(*a_Var),
           f[4] = {0,0,0,0},
	       a_fEpsilon( (a_fPos == 0) ? (double)1e-10 : 1e-7 * a_fPos );

    *a_Var = a_fPos+2 * a_fEpsilon;  f[0] = EvalRemoveSingularity(xvar);
    *a_Var = a_fPos+1 * a_fEpsilon;  f[1] = EvalRemoveSingularity(xvar);
    *a_Var = a_fPos-1 * a_fEpsilon;  f[2] = EvalRemoveSingularity(xvar);
    *a_Var = a_fPos-2 * a_fEpsilon;  f[3] = EvalRemoveSingularity(xvar);
    *a_Var = fBuf; // restore variable

    fRes = (-f[0] + 8*f[1] - 8*f[2] + f[3]) / (12*a_fEpsilon);
    return fRes;
}
