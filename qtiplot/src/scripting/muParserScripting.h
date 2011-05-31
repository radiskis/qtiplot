/***************************************************************************
    File                 : muParserScripting.h
    Project              : QtiPlot
    --------------------------------------------------------------------

    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Evaluate mathematical expressions using muParser

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
#ifndef MUPARSER_SCRIPTING_H
#define MUPARSER_SCRIPTING_H

#include "ScriptingEnv.h"
#include "Script.h"
#include "muParserScript.h"

#include <muParser.h>
#include "math.h"
#include <gsl/gsl_sf.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

//! TODO
class muParserScripting: public ScriptingEnv
{
  Q_OBJECT

  public:
    static const char *langName;
    muParserScripting(ApplicationWindow *parent) : ScriptingEnv(parent, langName){
    	d_initialized = true;
    	gsl_set_error_handler_off();
	}
    static ScriptingEnv *constructor(ApplicationWindow *parent) { return new muParserScripting(parent); }

    bool isRunning() const { return true; }
    Script *newScript(const QString &code, QObject *context, const QString &name="<input>")
    {
      return new muParserScript(this, code, context, name);
    }

    // we do not support global variables
    bool setQObject(QObject*, const char*) { return false; }
    bool setInt(int, const char*) { return false; }
    bool setDouble(double, const char*) { return false; }

	const QStringList mathFunctions() const;
	const QString mathFunctionDoc (const QString &name) const;

	const static QStringList functionsList(bool tableContext = false);
	const static QString explainFunction(const QString &name);

    struct mathFunction
    {
      char *name;
      int numargs;
	  double (*fun1)(double);
      double (*fun2)(double,double);
      double (*fun3)(double,double,double);
      QString description;
    };
    static const mathFunction math_functions[];

  private:
	static double rnd(double x){
		gsl_rng_default_seed = (unsigned int)x*time(NULL);
		const gsl_rng_type * T = gsl_rng_default;
		gsl_rng * r = gsl_rng_alloc (T);
		double u = gsl_rng_uniform (r);
		gsl_rng_free (r);
		return u;
	}

	static double normal(double x){
		const gsl_rng_type * T = gsl_rng_default;
		gsl_rng * r = gsl_rng_alloc (T);
		if (!r)
			return 0.0;
		gsl_rng_set(r, (unsigned int)x*time(NULL));
		double u = gsl_ran_ugaussian(r);
		gsl_rng_free (r);
		return u;
	}

	static double mod(double x, double y){ return fmod(x,y);};
	static double bessel_I0(double x){ return gsl_sf_bessel_I0 (x);};
	static double bessel_I1(double x){ return gsl_sf_bessel_I1 (x);};
	static double bessel_In(double x, double n){ return gsl_sf_bessel_In ((int)n, x);};
	static double bessel_J0(double x){ return gsl_sf_bessel_J0 (x);};
	static double bessel_J1(double x){ return gsl_sf_bessel_J1 (x);};
	static double bessel_Jn(double x, double n){ return gsl_sf_bessel_Jn ((int)n, x);};
	static double bessel_Yn(double x, double n){ return gsl_sf_bessel_Yn ((int)n, x);};
	static double bessel_Jn_zero(double n, double s){ return gsl_sf_bessel_zero_Jnu(n, (unsigned int) s);};
	static double bessel_Y0(double x){ return gsl_sf_bessel_Y0 (x);};
	static double bessel_Y1(double x){ return gsl_sf_bessel_Y1 (x);};
	static double beta(double a, double b){ return gsl_sf_beta (a,b);};
	static double erf(double x){ return gsl_sf_erf (x);};
	static double erfc(double x){ return gsl_sf_erfc (x);};
	static double erf_Z(double x){ return gsl_sf_erf_Z (x);};
	static double erf_Q(double x){ return gsl_sf_erf_Q (x);};
	static double gamma(double x){ return gsl_sf_gamma (x);};
	static double lngamma(double x){ return gsl_sf_lngamma (x);};
	static double hazard(double x){ return gsl_sf_hazard (x);};
	static double lambert_W0(double x){ return gsl_sf_lambert_W0(x);};
	static double lambert_Wm1(double x){ return gsl_sf_lambert_Wm1(x);};
	static double invt(double x, double n){ return gsl_cdf_tdist_P(x, n);};
	static double ttable(double x, double n){ return gsl_cdf_tdist_Pinv(x, n);};
	static double ftable(double x, double m, double n){return gsl_cdf_fdist_Pinv(x, m, n);};
	static double invf(double x, double m, double n){return gsl_cdf_fdist_P(x, m, n);};
	static double gauss_pdf(double x, double sigma){return gsl_ran_gaussian_pdf (x, sigma);};
	static double gauss_cdf(double x, double sigma){return gsl_cdf_gaussian_P (x, sigma);};
	static double inv_gauss_cdf(double x, double sigma){return gsl_cdf_gaussian_Pinv(x, sigma);};
	static double normcdf(double x){return gsl_cdf_ugaussian_P(x);};
	static double norminv(double x){return gsl_cdf_ugaussian_Pinv(x);};
	static double chi2cdf(double x, double n){return gsl_cdf_chisq_P (x, n);};
	static double chi2inv(double x, double n){return gsl_cdf_chisq_Pinv(x, n);};
	static double dilog(double x){return gsl_sf_dilog(x);};
};

class EmptySourceError : public mu::ParserError
{
	public:
		EmptySourceError() {}
};

#endif
