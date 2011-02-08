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

#include <QApplication>
#include <QLocale>

Anova::Anova(ApplicationWindow *parent, bool twoWay, double level)
: StatisticTest(parent, 0.0, level),
d_two_way(twoWay),
d_show_interactions(true),
d_anova_type(anova_fixed)
{
	setObjectName(QObject::tr("ANOVA"));
}

bool Anova::addSample(const QString& colName, int aLevel, int bLevel)
{
	if (!d_n){
		if (d_two_way){
			d_factorA_levels << aLevel;
			d_factorB_levels << bLevel;
		}
		return setData(colName);
	}

	Statistics *sample = new Statistics((ApplicationWindow *)this->parent(), colName);
	if (!sample->dataSize()){
		delete sample;
		return false;
	}

	d_data_samples << sample;
	if (d_two_way){
		d_factorA_levels << aLevel;
		d_factorB_levels << bLevel;
	}
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
		foreach(Statistics *sample, d_data_samples)
			s += sample->logInfo(false);
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

void Anova::outputResultsTo(Table *t)
{
	if (!t)
		return;

	int rows = t->numRows();
	if (d_two_way)
		t->setNumRows(rows + 5);
	else
		t->setNumRows(rows + 3);

	if (t->numCols() < 6)
		t->setNumCols(6);

	if (d_two_way){
		t->setText(rows, 0, QObject::tr("A"));
		t->setText(rows + 1, 0, QObject::tr("B"));
		t->setText(rows + 2, 0, QObject::tr("A*B"));
		t->setText(rows + 3, 0, QObject::tr("Error"));
		t->setText(rows + 4, 0, QObject::tr("Total"));
		t->setColumnType(0, Table::Text);

		t->setCell(rows, 1, d_att.dfA);
		t->setCell(rows, 2, d_att.SSA);
		t->setCell(rows, 3, d_att.MSA);
		t->setCell(rows, 4, d_att.FA);
		t->setCell(rows, 5, d_att.pA);

		rows++;
		t->setCell(rows, 1, d_att.dfB);
		t->setCell(rows, 2, d_att.SSB);
		t->setCell(rows, 3, d_att.MSB);
		t->setCell(rows, 4, d_att.FB);
		t->setCell(rows, 5, d_att.pB);

		rows++;
		t->setCell(rows, 1, d_att.dfAB);
		t->setCell(rows, 2, d_att.SSAB);
		t->setCell(rows, 3, d_att.MSAB);
		t->setCell(rows, 4, d_att.FAB);
		t->setCell(rows, 5, d_att.pAB);

		rows++;
		t->setCell(rows, 1, d_att.dfE);
		t->setCell(rows, 2, d_att.SSE);
		t->setCell(rows, 3, d_att.MSE);

		rows++;
		t->setCell(rows, 1, d_att.dfT);
		t->setCell(rows, 2, d_att.SST);
	} else {
		t->setText(rows, 0, QObject::tr("Model"));
		t->setText(rows + 1, 0, QObject::tr("Error"));
		t->setText(rows + 2, 0, QObject::tr("Total"));
		t->setColumnType(0, Table::Text);

		t->setCell(rows, 1, d_at.dfTr);
		t->setCell(rows, 2, d_at.SSTr);
		t->setCell(rows, 3, d_at.MSTr);
		t->setCell(rows, 4, d_at.F);
		t->setCell(rows, 5, d_at.p);

		rows++;
		t->setCell(rows, 1, d_at.dfE);
		t->setCell(rows, 2, d_at.SSE);
		t->setCell(rows, 3, d_at.MSE);

		rows++;
		t->setCell(rows, 1, d_at.dfT);
		t->setCell(rows, 2, d_at.SST);
	}

	for (int i = 0; i < t->numCols(); i++)
		t->table()->adjustColumn(i);
}

Table * Anova::resultTable(const QString& name)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	Table *t = 0;
	if (d_two_way){
		t = app->newTable(5, 6);

		t->setText(0, 0, QObject::tr("A"));
		t->setText(1, 0, QObject::tr("B"));
		t->setText(2, 0, QObject::tr("A*B"));
		t->setText(3, 0, QObject::tr("Error"));
		t->setText(4, 0, QObject::tr("Total"));
		t->setColumnType(0, Table::Text);

		t->setCell(0, 1, d_att.dfA);
		t->setCell(0, 2, d_att.SSA);
		t->setCell(0, 3, d_att.MSA);
		t->setCell(0, 4, d_att.FA);
		t->setCell(0, 5, d_att.pA);

		t->setCell(1, 1, d_att.dfB);
		t->setCell(1, 2, d_att.SSB);
		t->setCell(1, 3, d_att.MSB);
		t->setCell(1, 4, d_att.FB);
		t->setCell(1, 5, d_att.pB);

		t->setCell(2, 1, d_att.dfAB);
		t->setCell(2, 2, d_att.SSAB);
		t->setCell(2, 3, d_att.MSAB);
		t->setCell(2, 4, d_att.FAB);
		t->setCell(2, 5, d_att.pAB);

		t->setCell(3, 1, d_att.dfE);
		t->setCell(3, 2, d_att.SSE);
		t->setCell(3, 3, d_att.MSE);

		t->setCell(4, 1, d_att.dfT);
		t->setCell(4, 2, d_att.SST);
	} else {
		t = app->newTable(3, 6);

		t->setText(0, 0, QObject::tr("Model"));
		t->setText(1, 0, QObject::tr("Error"));
		t->setText(2, 0, QObject::tr("Total"));
		t->setColumnType(0, Table::Text);

		t->setCell(0, 1, d_at.dfTr);
		t->setCell(0, 2, d_at.SSTr);
		t->setCell(0, 3, d_at.MSTr);
		t->setCell(0, 4, d_at.F);
		t->setCell(0, 5, d_at.p);

		t->setCell(1, 1, d_at.dfE);
		t->setCell(1, 2, d_at.SSE);
		t->setCell(1, 3, d_at.MSE);

		t->setCell(2, 1, d_at.dfT);
		t->setCell(2, 2, d_at.SST);
	}

	QStringList header = QStringList() << QObject::tr("Source") << QObject::tr("DoF") << QObject::tr("Sum of Squares")
						 << QObject::tr("Mean Square") << QObject::tr("F Value") << QObject::tr("P Value");
	t->setHeader(header);

	for (int i = 0; i < t->numCols(); i++)
		t->table()->adjustColumn(i);

	t->setWindowLabel(objectName() + " " + QObject::tr("Result Table"));
	t->show();
	return t;
}

void Anova::freeMemory()
{
	Statistics::freeMemory();
	foreach(Statistics *sample, d_data_samples)
		delete sample;
}
