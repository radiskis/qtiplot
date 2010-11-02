/***************************************************************************
	File                 : tTest.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Student's t-Test

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
#include "tTest.h"
#include <Table.h>

#include <QApplication>
#include <QLocale>

#include <gsl/gsl_cdf.h>

tTest::tTest(ApplicationWindow *parent, double testMean, double level, const QString& sample)
: Statistics(parent, sample),
d_test_mean(testMean),
d_significance_level(level),
d_tail(Both)
{
}

void tTest::setTail(const Tail& tail)
{
	d_tail = tail;
}

void tTest::setTestMean(double mean)
{
	d_test_mean = mean;
}

void tTest::setSignificanceLevel(double s)
{
	if (s < 0.0 || s > 1.0)
		return;

	d_significance_level = s;
}

int tTest::dof()
{
	return d_n - 1;
}

double tTest::t()
{
	if (!d_n)
		return .0;

	return sqrt(d_n)*(d_mean - d_test_mean)/d_sd;
}

double tTest::pValue()
{
	if (!d_n)
		return 0.0;

	double p = gsl_cdf_tdist_P(t(), dof());
	switch(d_tail){
		case Left:
		break;
		case Right:
			p = 1 - p;
		break;
		case Both:
			p = 2*(1 - p);
		break;
	}
	return p;
}

double tTest::power(double alpha, int size)
{
	if (!d_n)
		return 0.0;

	int dof = this->dof();
	if (!size)
		size = d_n;
	else
		dof = size - 1;

	double st = t();
	double p = 0.0;
	switch(d_tail){
		case Left:
			p = gsl_cdf_tdist_P(gsl_cdf_tdist_Pinv(alpha, dof) - st, dof);
		break;
		case Right:
			p = 1 - gsl_cdf_tdist_P(gsl_cdf_tdist_Pinv(1 - alpha, dof) - st, dof);
		break;
		case Both:
			p = 1 - gsl_cdf_tdist_P(gsl_cdf_tdist_Pinv(1 - 0.5*alpha, dof) - st, dof);
			p += gsl_cdf_tdist_P(gsl_cdf_tdist_Pinv(0.5*alpha, dof) - st, dof);
		break;
	}
	return p;
}

//! Lower Confidence Limit
double tTest::lcl(double confidenceLevel)
{
	if (!d_n)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	double conf = d_se*gsl_cdf_tdist_Pinv(1 - 0.5*alpha, dof());
	return d_mean - conf;
}

//! Upper Confidence Limit
double tTest::ucl(double confidenceLevel)
{
	if (!d_n)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	double conf = d_se*gsl_cdf_tdist_Pinv(1 - 0.5*alpha, dof());
	return d_mean + conf;
}

QString tTest::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	/*if (d_two_samples)
		s += tr("Two sample t-Test") + "\n";
	else*/
		s += tr("One sample t-Test") + "\n";

	s += "\n";
	s += Statistics::logInfo();

	QString h0, ha, compare;
	switch((int)d_tail){
		case Both:
			h0 = " = ";
			ha = " <> ";
			compare = QObject::tr("different");
		break;
		case Right:
			h0 = " <= ";
			ha = " > ";
			compare = QObject::tr("greater");
		break;
		case Left:
			h0 = " >= ";
			ha = " < ";
			compare = QObject::tr("less");
		break;
	}

	s += QObject::tr("Null Hypothesis") + ":\t\t\t" + QObject::tr("Mean") + h0 + l.toString(d_test_mean, 'g', p) + "\n";
	s += QObject::tr("Alternative Hypothesis") + ":\t\t" + QObject::tr("Mean") + ha + l.toString(d_test_mean, 'g', p) + "\n\n";

	double pval = this->pValue();
	QString sep = "\t\t";
	s += QObject::tr("t") + sep + QObject::tr("DoF") + sep + QObject::tr("P Value") + "\n";
	s += sep1;
	s += l.toString(t(), 'g', 6) + sep + QString::number(dof()) + sep + l.toString(pval, 'g', p) +  + "\n";
	s += sep1;
	s += "\n";
	s += QObject::tr("At the %1 level, the population mean").arg(l.toString(d_significance_level, 'g', 6)) + " ";

	if (pval < d_significance_level)
		s += QObject::tr("is significantly");
	else
		s += QObject::tr("is not significantly");
	s += " " + compare + " " + QObject::tr("than the test mean");
	s += " (" + l.toString(d_test_mean, 'g', p) + ").\n";

	return s;
}
