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

#include <QApplication>
#include <QLocale>

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>

tTest::tTest(ApplicationWindow *parent, double testMean, double level, const QString& sample1, const QString& sample2, bool paired)
: StatisticTest(parent, testMean, level, sample1),
d_sample2(0),
d_independent_test(true)
{
	setObjectName(QObject::tr("Student's t-Test"));

	if (!sample2.isEmpty())
		setSample2(sample2, paired);
}

int tTest::dof()
{
	if (d_sample2 && d_independent_test)
		return d_n  + d_sample2->dataSize() - 2;

	return d_n - 1;
}

double tTest::t()
{
	if (!d_n)
		return 0.0;

	if (d_sample2)
		return (d_diff - d_test_val)/d_s12;

	return sqrt(d_n)*(d_mean - d_test_val)/d_sd;
}

double tTest::t(int size)
{
	return sqrt(size)*(d_mean - d_test_val)/d_sd;
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
			if (p < 0.5)
				p = 2*p;
			else
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
	double st = t();
	if (size){
		if (d_sample2 && d_independent_test)
			dof = size - 2;
		else
			dof = size - 1;

		st = t(size);
	}

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
	if (!d_n || confidenceLevel < 0 || confidenceLevel > 100)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	double conf = gsl_cdf_tdist_Pinv(1 - 0.5*alpha, dof());

	if (d_sample2)
		return d_diff - d_s12*conf;

	return d_mean - d_se*conf;
}

//! Upper Confidence Limit
double tTest::ucl(double confidenceLevel)
{
	if (!d_n || confidenceLevel < 0 || confidenceLevel > 100)
		return 0.0;

	double alpha = 1 - confidenceLevel/100.0;
	double conf = gsl_cdf_tdist_Pinv(1 - 0.5*alpha, dof());

	if (d_sample2)
		return d_diff + d_s12*conf;

	return d_mean + d_se*conf;
}

QString tTest::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	if (d_sample2){
		if (d_independent_test)
			s += QObject::tr("Two Sample Independent t-Test") + "\n";
		else
			s += QObject::tr("Two Sample Paired t-Test") + "\n";
	} else
		s += QObject::tr("One Sample t-Test") + "\n";

	s += "\n";

	if (d_descriptive_statistics){
		s += Statistics::logInfo();
		if (d_sample2){
			s += d_sample2->logInfo(false);
			s += QObject::tr("Difference of Means") + ":\t" + l.toString(d_diff, 'g', p) + "\n\n";
		}
	}

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

	QString mText = QObject::tr("Mean");
	if (d_sample2)
		mText = mText + "1 - " + mText + "2";

	s += QObject::tr("Null Hypothesis") + ":\t\t\t" + mText + h0 + l.toString(d_test_val, 'g', p) + "\n";
	s += QObject::tr("Alternative Hypothesis") + ":\t\t" + mText + ha + l.toString(d_test_val, 'g', p) + "\n\n";

	double pval = pValue();
	QString sep = "\t\t";
	s += QObject::tr("t") + sep + QObject::tr("DoF") + sep + QObject::tr("P Value") + "\n";
	s += sep1;
	s += l.toString(t(), 'g', p) + sep + QString::number(dof()) + sep + l.toString(pval, 'g', p) +  + "\n";
	s += sep1;
	s += "\n";

	if (d_sample2)
		s += QObject::tr("At the %1 level, the difference of the population means").arg(l.toString(d_significance_level, 'g', 6)) + " ";
	else
		s += QObject::tr("At the %1 level, the population mean").arg(l.toString(d_significance_level, 'g', 6)) + " ";

	if (pval < d_significance_level)
		s += QObject::tr("is significantly");
	else
		s += QObject::tr("is not significantly");
	s += " " + compare + " ";

	if (d_sample2)
		s += QObject::tr("than the test difference");
	else
		s += QObject::tr("than the test mean");

	s += " (" + l.toString(d_test_val, 'g', p) + ").\n";
	return s;
}

bool tTest::setSample2(const QString& colName, bool paired)
{
	d_sample2 = new Statistics((ApplicationWindow *)this->parent(), colName);

	unsigned int d_n2 = d_sample2->dataSize();
	if (paired && d_n2 != d_n){
		QMessageBox::information((QWidget *)parent(), QObject::tr("Attention!"),
								 QObject::tr("Paired t-Test requires equal sample sizes."));
		delete d_sample2;
		d_sample2 = 0;
		return false;
	}

	if (paired){
		d_independent_test = false;

		double *data2 = d_sample2->data();
		for (unsigned int i = 0; i < d_n; i++)
			d_data[i] = d_data[i] - data2[i];

		d_s12 = gsl_stats_sd(d_data, 1, d_n)/sqrt(d_n);
		if (d_s12 == 0){
			QMessageBox::warning((QWidget *)parent(), QObject::tr("Attention!"),
			QObject::tr("The test statistics t and P can not be computed because the sample variance of the differences between Sample1 and Sample2 is 0."));
			delete d_sample2;
			d_sample2 = 0;
			return false;
		}
	} else {
		double d_sd2 = d_sample2->standardDeviation();
		double s = sqrt(((d_n - 1)*d_sd*d_sd + (d_n2 - 1)*d_sd2*d_sd2)/(double)(d_n  + d_n2 - 2));
		d_s12 = s*sqrt(1/(double)d_n + 1/(double)d_n2);
	}

	d_diff = d_mean - d_sample2->mean();

	return d_n2 > 0;
}

void tTest::freeMemory()
{
	Statistics::freeMemory();
	if (d_sample2){
		delete d_sample2;
		d_sample2 = NULL;
	}
}
