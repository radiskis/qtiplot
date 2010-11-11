/***************************************************************************
	File                 : Anova.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : ANOVA

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
#include "Anova.h"
#include <Table.h>

#include <QApplication>
#include <QLocale>

#include <tamu_anova.h>

Anova::Anova(ApplicationWindow *parent, double level)
: Statistics(parent),
d_significance_level(level),
d_two_way(false)
{
}

void Anova::setSignificanceLevel(double s)
{
	if (s < 0.0 || s > 1.0)
		return;

	d_significance_level = s;
}

bool Anova::addSample(const QString& colName)
{
	if (!d_n)
		return setData(colName);

	Statistics *sample = new Statistics((ApplicationWindow *)this->parent(), colName);
	if (!sample->dataSize()){
		delete sample;
		return false;
	}

	d_data_samples << sample;
	return true;
}

bool Anova::run()
{
	if (!d_n)
		return false;
	if (!d_data_samples.size() && !d_two_way){
		QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
					QObject::tr("One-Way ANOVA requires two or more data samples."));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	unsigned int n = d_n;
	foreach(Statistics *sample, d_data_samples)
		n += sample->dataSize();

	long *factor = (long *)malloc(n*sizeof(long));
	double *data = (double *)malloc(n*sizeof(double));
	if (!data || !factor){
		memoryErrorMessage();
		return false;
	}

	for (unsigned int i = 0; i < d_n; i++){
		factor[i] = 1.0;
		data[i] = d_data[i];
	}

	int aux = d_n;
	int samples = 1;
	foreach(Statistics *sample, d_data_samples){
		int size = sample->dataSize();
		samples++;
		double *sampleData = sample->data();
		for (int i = 0; i < size; i++){
			data[aux] = sampleData[i];
			factor[aux] = samples;
			aux++;
		}
	}

	struct tamu_anova_table tbl = tamu_anova(data, factor, n, samples);

	dfTr = tbl.dfTr;
	dfE = tbl.dfE;
	dfT = tbl.dfT;
	SSTr = tbl.SSTr;
	SSE = tbl.SSE;
	SST = tbl.SST;
	MSTr = tbl.MSTr;
	MSE = tbl.MSE;
	F = tbl.F;
	pVal = tbl.p;

	free(data);
	free(factor);

	((ApplicationWindow *)parent())->updateLog(logInfo());
	QApplication::restoreOverrideCursor();
	return true;
}

QString Anova::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	if (d_two_way)
		s += QObject::tr("Two Way ANOVA") + "\n";
	else
		s += QObject::tr("One Way ANOVA") + "\n";

	s += "\n";
	s += Statistics::logInfo();
	QString sep = "\t";
	foreach(Statistics *sample, d_data_samples){
		s += sample->sampleName() + sep + QString::number(sample->dataSize()) + sep + l.toString(sample->mean(), 'g', p) + sep;
		s += l.toString(sample->standardDeviation(), 'g', p) + "\t\t" + l.toString(sample->standardError(), 'g', p) + "\n";
	}
	s += sep1 + "\n";
	s += QObject::tr("Null Hypothesis") + ":\t\t\t" + QObject::tr("The means of all selected datasets are equal") + "\n";
	s += QObject::tr("Alternative Hypothesis") + ":\t\t" + QObject::tr("The means of one or more selected datasets are different") + "\n\n";
	s += tr("ANOVA") + "\n\n";
	s += tr("Source") + sep + tr("DoF") + sep + tr("Sum of Squares") + sep + tr("Mean Square") + sep + tr("F Value") + sep + tr("P Value") + "\n";
	s += sep1;
	s += tr("Model") + sep + QString::number(dfTr) + sep + l.toString(SSTr, 'g', p) + sep + l.toString(MSTr, 'g', p) + sep;
	s += l.toString(F, 'g', p) + sep + l.toString(pVal, 'g', p) + "\n";
	s += tr("Error") + sep + QString::number(dfE) + sep + l.toString(SSE, 'g', p) + sep + l.toString(MSE, 'g', p) + "\n";
	s += tr("Total") + sep + QString::number(dfT) + sep + l.toString(SST, 'g', p) + "\n";
	s += sep1;
	s += "\n";
	s += QObject::tr("At the %1 level, the population means").arg(l.toString(d_significance_level, 'g', 6)) + " ";
	if (pVal < d_significance_level)
		s += QObject::tr("are significantly different");
	else
		s += QObject::tr("are not significantly different");

	s += ".\n\n";
	return s;
}

void Anova::freeMemory()
{
	Statistics::freeMemory();
	foreach(Statistics *sample, d_data_samples)
		delete sample;
}
