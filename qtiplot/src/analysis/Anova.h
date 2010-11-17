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

#include <StatisticTest.h>
#include <tamu_anova.h>

//! ANOVA
class Anova : public StatisticTest
{
	Q_OBJECT

	public:
		Anova(ApplicationWindow *parent, bool twoWay = false, double level = 0.05);

		void showAnovaTwoWayInteractions(bool show = true){d_show_interactions = show;};
		void setAnovaTwoWayModel(int type){d_anova_type = (gsl_anova_twoway_types)type;};
		bool addSample(const QString& colName, int aLevel = 1, int bLevel = 1);
		bool run();

		virtual QString logInfo();

		//! \name One-Way ANOVA results
		//@{
		double fStat(){return d_at.F;};
		double statistic(){return d_at.F;};
		double pValue(){return d_at.p;};
		double ssm(){return d_at.SSTr;};
		//@}

		//! \name Two-Way ANOVA results
		//@{
		double fStatA(){return d_att.FA;};
		double fStatB(){return d_att.FB;};
		double fStatAB(){return d_att.FAB;};

		double pValueA(){return d_att.pA;};
		double pValueB(){return d_att.pB;};
		double pValueAB(){return d_att.pAB;};

		double ssa(){return d_att.SSA;};
		double ssb(){return d_att.SSB;};
		double ssab(){return d_att.SSAB;};

		double msa(){return d_att.MSA;};
		double msb(){return d_att.MSB;};
		double msab(){return d_att.MSAB;};
		//@}

		double sse(){if (d_two_way) return d_att.SSE; return d_at.SSE;};
		double sst(){if (d_two_way) return d_att.SST; return d_at.SST;};
		double mse(){if (d_two_way) return d_att.MSE; return d_at.MSE;};

		//! Returns a pointer to the table created to display the results
		virtual Table *resultTable(const QString& name = QString());
		virtual void outputResultsTo(Table *);

	protected:
		bool twoWayANOVA();
		bool oneWayANOVA();
		void freeMemory();
		QString levelName(int level, bool b = false);

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
