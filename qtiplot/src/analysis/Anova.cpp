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

Anova::Anova(ApplicationWindow *parent, double level, bool twoWay)
: Statistics(parent),
d_significance_level(level),
d_descriptive_statistics(true),
d_two_way(twoWay),
d_show_interactions(true),
d_anova_type(anova_fixed)
{
}

void Anova::setSignificanceLevel(double s)
{
	if (s < 0.0 || s > 1.0)
		return;

	d_significance_level = s;
}

bool Anova::addSample(const QString& colName, int aLevel, int bLevel)
{
	if (!d_n){
		d_factorA_levels << aLevel;
		d_factorB_levels << bLevel;
		return setData(colName);
	}

	Statistics *sample = new Statistics((ApplicationWindow *)this->parent(), colName);
	if (!sample->dataSize()){
		delete sample;
		return false;
	}

	d_data_samples << sample;
	d_factorA_levels << aLevel;
	d_factorB_levels << bLevel;
	return true;
}

bool Anova::run()
{
	if (!d_n)
		return false;
	if (d_two_way && d_data_samples.size() < 2){
		QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
					QObject::tr("Two-Way ANOVA requires three or more data samples."));
		return false;
	} else if (!d_data_samples.size() && !d_two_way){
		QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
					QObject::tr("One-Way ANOVA requires two or more data samples."));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (d_two_way && !twoWayANOVA())
		return false;
	else if(!oneWayANOVA())
		return false;

	((ApplicationWindow *)parent())->updateLog(logInfo());
	QApplication::restoreOverrideCursor();
	return true;
}

bool Anova::twoWayANOVA()
{
	QList<int> aLevels;
	QList<int> bLevels;
	int samples = d_data_samples.size() + 1;
	for (int i = 0; i < samples; i++){
		int level = d_factorA_levels[i];
		if (!aLevels.contains(level))
			aLevels << level;

		level = d_factorB_levels[i];
		if (!bLevels.contains(level))
			bLevels << level;
	}

	if (aLevels.size() < 2){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
					QObject::tr("Factor A must have two or more levels."));
		return false;
	}
	if (bLevels.size() < 2){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
					QObject::tr("Factor B must have two or more levels."));
		return false;
	}

	for (int i = 1; i <= aLevels.size(); i++){
		for (int j = 1; j <= bLevels.size(); j++){
			bool levelCombinationExists = false;
			for (int k = 0; k < samples; k++){
				if (d_factorA_levels[k] == i && d_factorB_levels[k] == j){
					levelCombinationExists = true;
					break;
				}
			}

			if (!levelCombinationExists){
				QApplication::restoreOverrideCursor();
				QMessageBox::critical((ApplicationWindow *)parent(), QObject::tr("Attention!"),
				QObject::tr("There are no data points in Factor A '%1' and Factor B '%2' level combination.").arg(levelName(i)).arg(levelName(j, true)));
				return false;
			}
		}
	}

	unsigned int n = d_n;
	foreach(Statistics *sample, d_data_samples)
		n += sample->dataSize();

	double *data = (double *)malloc(n*sizeof(double));
	if (!data){
		QApplication::restoreOverrideCursor();
		memoryErrorMessage();
		return false;
	}

	long J[2] = {aLevels.size(), bLevels.size()};

	long f[n][2];
	for (unsigned int i = 0; i < d_n; i++){
		data[i] = d_data[i];
		f[i][0] = d_factorA_levels[0];
		f[i][1] = d_factorB_levels[0];
	}

	int aux = d_n;
	int s = 1;
	foreach(Statistics *sample, d_data_samples){
		int size = sample->dataSize();
		double *sampleData = sample->data();
		for (int i = 0; i < size; i++){
			data[aux] = sampleData[i];
			f[aux][0] = d_factorA_levels[s];
			f[aux][1] = d_factorB_levels[s];
			aux++;
		}
		s++;
	}

	d_att = tamu_anova_twoway(data, f, n, J, d_anova_type);

	free(data);

	QApplication::restoreOverrideCursor();
	return true;
}

bool Anova::oneWayANOVA()
{
	unsigned int n = d_n;
	foreach(Statistics *sample, d_data_samples)
		n += sample->dataSize();

	long *factor = (long *)malloc(n*sizeof(long));
	double *data = (double *)malloc(n*sizeof(double));
	if (!data || !factor){
		QApplication::restoreOverrideCursor();
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

	d_at = tamu_anova(data, factor, n, samples);

	free(data);
	free(factor);
	return true;
}

QString Anova::levelName(int level, bool b)
{
	QString l = QObject::tr("A");
	if (b)
		l = QObject::tr("B");
	return l + QString::number(level);
}

QString Anova::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep = "\t";
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = "[" + QDateTime::currentDateTime().toString(Qt::LocalDate)+ " \"" + d_table->objectName() + "\"]\t";
	if (d_two_way){
		s += QObject::tr("Two-Way ANOVA") + " ";
		switch(d_anova_type){
			case 0:
				s += QObject::tr("Fixed Model");
				break;
			case 1:
				s += QObject::tr("Random Model");
				break;
			case 2:
				s += QObject::tr("Mixed Model");
				break;
		}
		s += "\n";
	} else
		s += QObject::tr("One-Way ANOVA") + "\n";

	s += "\n";

	if (d_descriptive_statistics){
		s += Statistics::logInfo();
		foreach(Statistics *sample, d_data_samples){
			s += sample->sampleName() + sep + QString::number(sample->dataSize()) + sep + l.toString(sample->mean(), 'g', p) + sep;
			s += l.toString(sample->standardDeviation(), 'g', p) + "\t\t" + l.toString(sample->standardError(), 'g', p) + "\n";
		}
		s += sep1 + "\n";
	}

	if (d_two_way){
		s += QObject::tr("Selected Data") + "\n\n";
		s += QObject::tr("Sample") + sep + QObject::tr("Factor A Level") + sep + QObject::tr("Factor B Level") + "\n";
		s += sep1;
		s += sampleName() + sep + levelName(d_factorA_levels[0]) + sep + levelName(d_factorB_levels[0], true) + "\n";
		int i = 1;
		foreach(Statistics *sample, d_data_samples){
			s += sample->sampleName() + sep + levelName(d_factorA_levels[i]) + sep + levelName(d_factorB_levels[i], true) + "\n";
			i++;
		}
		s += sep1 + "\n";
	} else {
		s += QObject::tr("Null Hypothesis") + ":\t\t\t" + QObject::tr("The means of all selected datasets are equal") + "\n";
		s += QObject::tr("Alternative Hypothesis") + ":\t\t" + QObject::tr("The means of one or more selected datasets are different") + "\n\n";
	}

	s += tr("ANOVA") + "\n\n";
	s += QObject::tr("Source") + sep + QObject::tr("DoF") + sep + QObject::tr("Sum of Squares") + sep;
	s += QObject::tr("Mean Square") + sep + QObject::tr("F Value") + sep + QObject::tr("P Value") + "\n";
	s += sep1;
	if (d_two_way){
		s += QObject::tr("A") + sep + QString::number(d_att.dfA) + sep + l.toString(d_att.SSA, 'g', p) + sep + l.toString(d_att.MSA, 'g', p) + sep;
		s += l.toString(d_att.FA, 'g', p) + sep + l.toString(d_att.pA, 'g', p) + "\n";
		s += QObject::tr("B") + sep + QString::number(d_att.dfB) + sep + l.toString(d_att.SSB, 'g', p) + sep + l.toString(d_att.MSB, 'g', p) + sep;
		s += l.toString(d_att.FB, 'g', p) + sep + l.toString(d_att.pB, 'g', p) + "\n";
		if (d_show_interactions){
			s += QObject::tr("A") + "*" + QObject::tr("B") + sep + QString::number(d_att.dfAB) + sep + l.toString(d_att.SSAB, 'g', p) + sep + l.toString(d_att.MSAB, 'g', p) + sep;
			s += l.toString(d_att.FAB, 'g', p) + sep + l.toString(d_att.pAB, 'g', p) + "\n";
		}
		s += QObject::tr("Error") + sep + QString::number(d_att.dfE) + sep + l.toString(d_att.SSE, 'g', p) + sep + l.toString(d_att.MSE, 'g', p) + "\n";
		s += QObject::tr("Total") + sep + QString::number(d_att.dfT) + sep + l.toString(d_att.SST, 'g', p) + "\n";
	} else {
		s += QObject::tr("Model") + sep + QString::number(d_at.dfTr) + sep + l.toString(d_at.SSTr, 'g', p) + sep + l.toString(d_at.MSTr, 'g', p) + sep;
		s += l.toString(d_at.F, 'g', p) + sep + l.toString(d_at.p, 'g', p) + "\n";
		s += QObject::tr("Error") + sep + QString::number(d_at.dfE) + sep + l.toString(d_at.SSE, 'g', p) + sep + l.toString(d_at.MSE, 'g', p) + "\n";
		s += QObject::tr("Total") + sep + QString::number(d_at.dfT) + sep + l.toString(d_at.SST, 'g', p) + "\n";
	}

	s += sep1;
	s += "\n";

	if (!d_two_way){
		s += QObject::tr("At the %1 level, the population means").arg(l.toString(d_significance_level, 'g', 6)) + " ";
		if (d_at.p < d_significance_level)
			s += QObject::tr("are significantly different");
		else
			s += QObject::tr("are not significantly different");

		s += ".\n\n";
	}

	return s;
}

void Anova::freeMemory()
{
	Statistics::freeMemory();
	foreach(Statistics *sample, d_data_samples)
		delete sample;
}
