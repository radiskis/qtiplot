/***************************************************************************
	File                 : Anova.h
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
#ifndef Anova_H
#define Anova_H

#include <Statistics.h>
#include <tamu_anova.h>

class Table;

//! Abstract base class for data analysis operations
class Anova : public Statistics
{
	Q_OBJECT

	public:
		Anova(ApplicationWindow *parent, double level = 0.05, bool twoWay = false);
		void setSignificanceLevel(double);
		void showDescriptiveStatistics(bool show = true){d_descriptive_statistics = show;};
		void showAnovaTwoWayInteractions(bool show = true){d_show_interactions = show;};
		void setAnovaTwoWayType(int type){d_anova_type = (gsl_anova_twoway_types)type;};
		bool addSample(const QString& colName, int aLevel = 1, int bLevel = 1);
		bool run();

		virtual QString logInfo();

		double fStat(){return d_at.F;};
		double pValue(){return d_at.p;};
		double ssm(){return d_at.SSTr;};
		double sse(){return d_at.SSE;};
		double sst(){return d_at.SST;};

	protected:
		bool twoWayANOVA();
		bool oneWayANOVA();
		void freeMemory();
		QString levelName(int level, bool b = false);

		double d_significance_level;
		bool d_descriptive_statistics;
		bool d_two_way;
		bool d_show_interactions;
		gsl_anova_twoway_types d_anova_type;
		QList<Statistics *> d_data_samples;
		QList<int> d_factorA_levels;
		QList<int> d_factorB_levels;

		tamu_anova_table d_at;
		tamu_anova_table_twoway d_att;
};

#endif
