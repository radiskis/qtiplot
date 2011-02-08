/***************************************************************************
	File                 : ShapiroWilkTest.cpp
	Project              : QtiPlot
	--------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Normality test. The code was taken from R (see file swilk.c)
						   and adapted to C++.
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

#include <ShapiroWilkTest.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sort.h>

#ifndef min
# define min(a, b)		((a) > (b) ? (b) : (a))
#endif

#ifndef sign
# define sign(a)		((a) >= 0 ? 1.0 : -1.0)
#endif

ShapiroWilkTest::ShapiroWilkTest(ApplicationWindow *parent, const QString& sample)
: StatisticTest(parent, 0.0, 0.05, sample),
d_w(0.0),
d_pValue(0.0)
{
	if (d_n < 3 || d_n > 5000){
		QMessageBox::critical(parent, QObject::tr("Attention!"), QObject::tr("Sample size must be between 3 and 5000."));
		this->freeMemory();
	} else {
		setObjectName(QObject::tr("Shapiro-Wilk Normality Test"));
		int init = false;
		int n = d_n;
		int n1 = d_n;
		int n2 = d_n/2;
		int error = 0;
		double a[n2];
		gsl_sort(d_data, 1, d_n);// the data must be sorted first
		swilk(&init, d_data, &n, &n1, &n2, a, &d_w, &d_pValue, &error);
	}
}

QString ShapiroWilkTest::logInfo()
{
	QString s = "\n[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	s += QObject::tr("Normality Test (Shapiro - Wilk)") + "\n\n";
	return s + infoString();
}

QString ShapiroWilkTest::shortLogInfo()
{
	return infoString(false);
}

QString ShapiroWilkTest::infoString(bool header)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;

	QStringList lst;
	lst << QObject::tr("Dataset");
	lst << QObject::tr("N");
	lst << QObject::tr("W");
	lst << QObject::tr("P Value");
	lst << d_col_name;
	lst << QString::number(d_n);
	lst << l.toString(d_w, 'g', p);
	lst << l.toString(d_pValue, 'g', p);

	QFontMetrics fm(app->font());
	int width = 0;
	foreach(QString aux, lst){
		int aw = fm.width(aux);
		if (aw > width)
			width = aw;
	}
	width += 6;

	QString head;
	for (int i = 0; i < 4; i++){
		QString aux = lst[i];
		int spaces = ceil((double)(width - fm.width(aux))/(double)fm.width(QLatin1Char(' '))) + 1;
		head += aux + QString(spaces, QLatin1Char(' '));
	}

	head += QObject::tr("Decision");

	QString s;
	if (header)
		s = head;

	QString val;
	for (int i = 4; i < lst.size(); i++){
		QString aux = lst[i];
		int spaces = ceil((double)(width - fm.width(aux))/(double)fm.width(QLatin1Char(' '))) + 1;
		val += aux + QString(spaces, QLatin1Char(' '));
	}

	if (d_pValue >= d_significance_level)
		val += QObject::tr("Normal at %1 level").arg(l.toString(d_significance_level));
	else
		val += QObject::tr("Not normal at %1 level").arg(l.toString(d_significance_level));

	if (header){
		int scores = ceil((double)fm.width(val)/(double)fm.width(QLatin1Char('-')));
		s +="\n" + QString(scores, QLatin1Char('-')) + "\n";
	}

	return s + val + "\n";
}

void ShapiroWilkTest::swilk(int *init,/* logical: is a[] already initialized ? */
	  double *x, int *n, int *n1, int *n2,
	  double *a,/* coefficients a[] */
      double *w, double *pw, int *ifault)
{

/*	ALGORITHM AS R94 APPL. STATIST. (1995) vol.44, no.4, 547-551.

	Calculates the Shapiro-Wilk W test and its significance level
*/

    /* Initialized data */
	const static double zero = 0.f;
	const static double one = 1.f;
	const static double two = 2.f;

	const static double small = 1e-19f;

    /* polynomial coefficients */
	const static double g[2] = { -2.273f,.459f };
	const static double
      c1[6] = { 0.f,.221157f,-.147981f,-2.07119f, 4.434685f, -2.706056f },
      c2[6] = { 0.f,.042981f,-.293762f,-1.752461f,5.682633f, -3.582633f };
	const static double c3[4] = { .544f,-.39978f,.025054f,-6.714e-4f };
	const static double c4[4] = { 1.3822f,-.77857f,.062767f,-.0020322f };
	const static double c5[4] = { -1.5861f,-.31082f,-.083751f,.0038915f };
	const static double c6[3] = { -.4803f,-.082676f,.0030302f };
	const static double c7[2] = { .164f,.533f };
	const static double c8[2] = { .1736f,.315f };
	const static double c9[2] = { .256f,-.00635f };

    /* System generated locals */
	double r__1;

/*
	Auxiliary routines : poly()  {below}
*/
    /* Local variables */
    int i, j, ncens, i1, nn2;

	double zbar, ssassx, summ2, ssumm2, gamma, delta, range;
	double a1, a2, an, bf, ld, m, s, sa, xi, sx, xx, y, w1;
	double fac, asa, an25, ssa, z90f, sax, zfm, z95f, zsd, z99f, rsn, ssx, xsx;

    *pw = 1.;
    if (*w >= 0.) {
	*w = 1.;
    }
    if (*n < 3) {	*ifault = 1; return;
    }

	an = (double) (*n);
    nn2 = *n / 2;
    if (*n2 < nn2) {	*ifault = 3; return;
    }
    if (*n1 < 3) {	*ifault = 1; return;
    }
    ncens = *n - *n1;
    if (ncens < 0 || (ncens > 0 && *n < 20)) {	*ifault = 4; return;
    }
    if (ncens > 0) {
	delta = (double) ncens / an;
	if (delta > .8f) {	*ifault = 5; return;
	}
    } /* just for -Wall:*/ else { delta = 0.f; }

    --a; /* so we can keep using 1-based indices */

/*	If INIT is false (always when called from R),
 *	calculate coefficients a[] for the test statistic W */
    if (! (*init)) {
	if (*n == 3) {
		const static double sqrth = .70710678f;/* = sqrt(1/2), was .70711f */
	    a[1] = sqrth;
	} else {
	    an25 = an + .25;
	    summ2 = zero;
	    for (i = 1; i <= *n2; ++i) {
			a[i] = gsl_cdf_ugaussian_Pinv ((i - .375f) / an25);
			r__1 = a[i];
			summ2 += r__1 * r__1;
	    }
	    summ2 *= two;
	    ssumm2 = sqrt(summ2);
	    rsn = one / sqrt(an);
	    a1 = poly(c1, 6, rsn) - a[1] / ssumm2;

	    /* Normalize a[] */
	    if (*n > 5) {
		i1 = 3;
		a2 = -a[2] / ssumm2 + poly(c2, 6, rsn);
		fac = sqrt((summ2 - two * (a[1] * a[1]) - two * (a[2] * a[2]))
			 / (one - two * (a1 * a1) - two * (a2 * a2)));
		a[2] = a2;
	    } else {
		i1 = 2;
		fac = sqrt((summ2 - two * (a[1] * a[1])) /
			   ( one  - two * (a1 * a1)));
	    }
	    a[1] = a1;
	    for (i = i1; i <= nn2; ++i)
		a[i] /= - fac;
	}
	*init = (1);
    }

/*	If W is input as negative, calculate significance level of -W */

    if (*w < zero) {
	w1 = 1. + *w;
	*ifault = 0;
	goto L70;
    }

/*	Check for zero range */

    range = x[*n1 - 1] - x[0];
    if (range < small) {
	*ifault = 6;	return;
    }

/*	Check for correct sort order on range - scaled X */

    /* *ifault = 7; <-- a no-op, since it is changed below, in ANY CASE! */
    *ifault = 0;
    xx = x[0] / range;
    sx = xx;
    sa = -a[1];
    j = *n - 1;
    for (i = 1; i < *n1; --j) {
	xi = x[i] / range;
	if (xx - xi > small) {
	    /* Fortran had:	 print *, "ANYTHING"
	     * but do NOT; it *does* happen with sorted x (on Intel GNU/linux 32bit):
	     *  shapiro.test(c(-1.7, -1,-1,-.73,-.61,-.5,-.24, .45,.62,.81,1))
	     */
	    *ifault = 7;
	}
	sx += xi;
	++i;
	if (i != j)
		sa += sign(i - j) * a[min(i,j)];
	xx = xi;
    }
    if (*n > 5000) {
	*ifault = 2;
    }

/*	Calculate W statistic as squared correlation
	between data and coefficients */

    sa /= *n1;
    sx /= *n1;
    ssa = ssx = sax = zero;
    j = *n - 1;
    for (i = 0; i < *n1; ++i, --j) {
	if (i != j)
		asa = sign(i - j) * a[1+min(i,j)] - sa;
	else
	    asa = -sa;
	xsx = x[i] / range - sx;
	ssa += asa * asa;
	ssx += xsx * xsx;
	sax += asa * xsx;
    }

/*	W1 equals (1-W) calculated to avoid excessive rounding error
	for W very near 1 (a potential problem in very large samples) */

    ssassx = sqrt(ssa * ssx);
    w1 = (ssassx - sax) * (ssassx + sax) / (ssa * ssx);
L70:
    *w = 1. - w1;

/*	Calculate significance level for W */

    if (*n == 3) {/* exact P value : */
	const static double pi6 = 1.90985931710274;/* = 6/pi, was  1.909859f */
	const static double stqr= 1.04719755119660;/* = asin(sqrt(3/4)), was 1.047198f */
	*pw = pi6 * (asin(sqrt(*w)) - stqr);
	if(*pw < 0.) *pw = 0.;
	return;
    }
    y = log(w1);
    xx = log(an);
    if (*n <= 11) {
	gamma = poly(g, 2, an);
	if (y >= gamma) {
	    *pw = 1e-99;/* an "obvious" value, was 'small' which was 1e-19f */
	    return;
	}
	y = -log(gamma - y);
	m = poly(c3, 4, an);
	s = exp(poly(c4, 4, an));
    } else {/* n >= 12 */
	m = poly(c5, 4, xx);
	s = exp(poly(c6, 3, xx));
    }
    /*DBG printf("c(w1=%g, w=%g, y=%g, m=%g, s=%g)\n",w1,*w,y,m,s); */

    if (ncens > 0) {/* <==>  n > n1 --- not happening currently when called from R */

/*	Censoring by proportion NCENS/N.
	Calculate mean and sd of normal equivalent deviate of W. */

	const static double three = 3.f;

	const static double z90 = 1.2816f;
	const static double z95 = 1.6449f;
	const static double z99 = 2.3263f;
	const static double zm = 1.7509f;
	const static double zss = .56268f;
	const static double bf1 = .8378f;

	const static double xx90 = .556;
	const static double xx95 = .622;

	ld = -log(delta);
	bf = one + xx * bf1;
	r__1 = pow(xx90, (double) xx);
	z90f = z90 + bf * pow(poly(c7, 2, r__1), (double) ld);
	r__1 = pow(xx95, (double) xx);
	z95f = z95 + bf * pow(poly(c8, 2, r__1), (double) ld);
	z99f = z99 + bf * pow(poly(c9, 2, xx), (double)ld);

/*	Regress Z90F,...,Z99F on normal deviates Z90,...,Z99 to get
	pseudo-mean and pseudo-sd of z as the slope and intercept */

	zfm = (z90f + z95f + z99f) / three;
	zsd = (z90 * (z90f - zfm) +
	       z95 * (z95f - zfm) + z99 * (z99f - zfm)) / zss;
	zbar = zfm - zsd * zm;
	m += zbar * s;
	s *= zsd;
    }
	//*pw = pnorm((double) y, (double)m, (double)s, 0/* upper tail */, 0);
	*pw = gsl_cdf_gaussian_Q(y - m, s);
    /*  = alnorm_(dble((Y - M)/S), 1); */

    return;
} /* swilk */

double ShapiroWilkTest::poly(const double *cc, int nord, double x)
{
/* Algorithm AS 181.2	Appl. Statist.	(1982) Vol. 31, No. 2

	Calculates the algebraic polynomial of order nord-1 with
	array of coefficients cc.  Zero order coefficient is cc(1) = cc[0]
*/
    /* Local variables */
    int j;
    double p, ret_val;/* preserve precision! */

    ret_val = cc[0];
    if (nord > 1) {
	p = x * cc[nord-1];
	for (j = nord - 2; j > 0; j--)
	    p = (p + cc[j]) * x;

	ret_val += p;
    }
    return ret_val;
} /* poly */
