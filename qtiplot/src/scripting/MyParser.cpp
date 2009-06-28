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

MyParser::MyParser()
:Parser()
{
DefineConst("pi", M_PI);
DefineConst("Pi", M_PI);
DefineConst("PI", M_PI);

DefineFun("bessel_j0", bessel_J0);
DefineFun("bessel_j1", bessel_J1);
DefineFun("bessel_jn", bessel_Jn);
DefineFun("bessel_y0", bessel_Y0);
DefineFun("bessel_y1", bessel_Y1);
DefineFun("bessel_yn", bessel_Yn);
DefineFun("beta", beta);
DefineFun("erf", erf);
DefineFun("erfc", erfc);
DefineFun("erfz", erfz);
DefineFun("erfq", erfq);
DefineFun("gamma", gamma);
DefineFun("gammaln", gammaln);
DefineFun("hazard", hazard);
}

QStringList MyParser::functionsList()
{
QStringList l;
l << "abs()" << "acos()" << "acosh()" << "asin()" << "asinh()" << "atan()";
l << "atanh()" << "avg(,)" << "bessel_j0()" << "bessel_j1()" << "bessel_jn(,)";
l << "bessel_y0()" << "bessel_y1()" << "bessel_yn(,)" << "beta(,)";
l << "cos()" << "cosh()" << "erf()" << "erfc()" << "erfz()" << "erfq()";
l << "exp()" << "gamma()" << "gammaln()" << "hazard()";
l << "if( , , )" << "ln()" << "log()" << "log2()" << "min()" << "max()";
l << "rint()" << "sign()" << "sin()" << "sinh()" << "sqrt()" << "tan()" << "tanh()";
return l;
}

QString MyParser::explainFunction(int index)
{
QString blabla;
switch (index)
	{
case 0:
	blabla = QObject::tr("abs(x):\n Absolute value of x.");
break;

case 1:
	blabla = QObject::tr("acos(x):\n Inverse cos function.");
break;

case 2:
	blabla = QObject::tr("acosh(x):\n Hyperbolic inverse cos function.");
break;

case 3:
	blabla = QObject::tr("asin(x):\n Inverse sin function.");
break;

case 4:
	blabla = QObject::tr("asinh(x):\n Hyperbolic inverse sin function.");
break;
case 5:
	blabla = QObject::tr("atan(x):\n Inverse tan function.");
break;
case 6:
	blabla = QObject::tr("atanh(x):\n  Hyperbolic inverse tan function.");
break;
case 7:
	blabla = QObject::tr("avg(x,y,...):\n  Mean value of all arguments.");
break;
case 8:
	blabla = QObject::tr("bessel_j0(x):\n  Regular cylindrical Bessel function of zeroth order, J_0(x).");
break;
case 9:
	blabla = QObject::tr("bessel_j1(x):\n  Regular cylindrical Bessel function of first order, J_1(x).");
break;
case 10:
	blabla = QObject::tr("bessel_j1(double x, int n):\n Regular cylindrical Bessel function of order n, J_n(x).");
break;
case 11:
	blabla = QObject::tr("bessel_y0(x):\n Irregular cylindrical Bessel function of zeroth order, Y_0(x), for x>0.");
break;
case 12:
	blabla = QObject::tr("bessel_y1(x):\n Irregular cylindrical Bessel function of first order, Y_1(x), for x>0.");
break;
case 13:
	blabla = QObject::tr("bessel_yn(double x, int n):\n Irregular cylindrical Bessel function of order n, Y_n(x), for x>0.");
break;
case 14:
	blabla = QObject::tr("beta (a,b):\n Computes the Beta Function, B(a,b) = Gamma(a)*Gamma(b)/Gamma(a+b) for a > 0, b > 0.");
break;
case 15:
	blabla = QObject::tr("cos (x):\n Calculate cosine.");
break;
case 16:
	blabla = QObject::tr("cosh(x):\n Hyperbolic cos function.");
break;
case 17:
	blabla = QObject::tr("erf(x):\n  The error function.");
break;
case 18:
	blabla = QObject::tr("erfc(x):\n Complementary error function erfc(x) = 1 - erf(x).");
break;
case 19:
	blabla = QObject::tr("erfz(x):\n The Gaussian probability density function Z(x).");
break;
case 20:
	blabla = QObject::tr("erfq(x):\n The upper tail of the Gaussian probability function Q(x).");
break;
case 21:
	blabla = QObject::tr("exp(x):\n Exponential function: e raised to the power of x.");
break;
case 22:
	blabla = QObject::tr("gamma(x):\n Computes the Gamma function, subject to x not being a negative integer.");
break;
case 23:
	blabla = QObject::tr("gammaln(x):\n Computes the logarithm of the Gamma function, subject to x not a being negative integer. For x<0, log(|Gamma(x)|) is returned.");
break;
case 24:
	blabla = QObject::tr("hazard(x):\n Computes the hazard function for the normal distribution h(x) = erfz(x)/erfq(x).");
break;
case 25:
	blabla = QObject::tr("if(e1, e2, e3):	if e1 then e2 else e3.");
break;
case 26:
	blabla = QObject::tr("ln(x):\n Calculate natural logarithm.");
break;
case 27:
	blabla = QObject::tr("log(x):\n Calculate decimal logarithm.");
break;
case 28:
	blabla = QObject::tr("log2(x):\n Calculate 	logarithm to the base 2.");
break;
case 29:
	blabla = QObject::tr("min(x,y,...):\n Calculate minimum of all arguments.");
break;
case 30:
	blabla = QObject::tr("max(x,y,...):\n Calculate maximum of all arguments.");
break;
case 31:
	blabla = QObject::tr("rint(x):\n Round to nearest integer.");
break;
case 32:
	blabla = QObject::tr("sign(x):\n Sign function: -1 if x<0; 1 if x>0.");
break;
case 33:
	blabla = QObject::tr("sin(x):\n Calculate sine.");
break;
case 34:
	blabla = QObject::tr("sinh(x):\n Hyperbolic sin function.");
break;
case 35:
	blabla = QObject::tr("sqrt(x):\n Square root function.");
break;
case 36:
	blabla = QObject::tr("tan(x):\n Calculate tangent function.");
break;
case 37:
	blabla = QObject::tr("tanh(x):\n Hyperbolic tan function.");
break;
	}
return blabla;
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
				QObject::tr("Found non-removable singularity at x = %1. Operation aborted!").arg(*xvar));
			}
			throw Pole();
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
