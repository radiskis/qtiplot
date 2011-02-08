/***************************************************************************
	File                 : ChiSquareTest.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Chi Square Test for Variance

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
#include "ChiSquareTest.h"
#include <StatisticTest.h>

#include <QApplication>
#include <QLocale>

#include <gsl/gsl_cdf.h>

ChiSquareTest::ChiSquareTest(ApplicationWindow *parent, double testValue, double level, const QString& sample)
: StatisticTest(parent, testValue, level, sample)
{
	setObjectName(QObject::tr("Chi-square Test"));
}

double ChiSquareTest::chiSquare()
{
	if (!d_n)
		return 0.0;

	return (d_n - 1)*d_variance/d_test_val;
}

double ChiSquareTest::pValue()
{
	if (!d_n)
		return 0.0;

	double p = gsl_cdf_chisq_P(chiSquare(), dof());
	switch(d_tail){
		case Left:
		break;
		case Right:
			p = 1 - p;
		break;
		case Both:
			if (p < 0.5)
				p = 2*p;
			else
				p = 2*(1 - p);
		break;
	}
	return p;
}

//! Lower Confidence Limit
double ChiSquareTest::lcl(double confidenceLevel)
{
	if (!d_n || confidenceLevel < 0 || confidenceLevel > 100)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	int df = dof();
	return df*d_variance/gsl_cdf_chisq_Pinv(1 - 0.5*alpha, df);
}

//! Upper Confidence Limit
double ChiSquareTest::ucl(double confidenceLevel)
{
	if (!d_n || confidenceLevel < 0 || confidenceLevel > 100)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	int df = dof();
	return df*d_variance/gsl_cdf_chisq_Pinv(0.5*alpha, df);
}

QString ChiSquareTest::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	s += QObject::tr("Chi-square Test for Variance") + "\n";
	s += "\n";

	if (d_descriptive_statistics)
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

	QString mText = QObject::tr("Variance");
	s += QObject::tr("Null Hypothesis") + ":\t\t\t" + mText + h0 + l.toString(d_test_val, 'g', p) + "\n";
	s += QObject::tr("Alternative Hypothesis") + ":\t\t" + mText + ha + l.toString(d_test_val, 'g', p) + "\n\n";

	double pval = pValue();
	QString sep = "\t\t";
	s += QObject::tr("Chi-Square") + sep + QObject::tr("DoF") + sep + QObject::tr("P Value") + "\n";
	s += sep1;
	s += l.toString(chiSquare(), 'g', p) + sep + QString::number(dof()) + sep + l.toString(pval, 'g', p) +  + "\n";
	s += sep1;
	s += "\n";

	s += QObject::tr("At the %1 level, the population variance").arg(l.toString(d_significance_level, 'g', 6)) + " ";
	if (pval < d_significance_level)
		s += QObject::tr("is significantly");
	else
		s += QObject::tr("is not significantly");
	s += " " + compare + " ";

	s += QObject::tr("than the test variance");
	s += " (" + l.toString(d_test_val, 'g', p) + ").\n";
	return s;
}
