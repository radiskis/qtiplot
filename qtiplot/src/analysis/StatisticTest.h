/***************************************************************************
	File                 : StatisticTest.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Base class for statistic tests

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
#ifndef STATISTICTEST_H
#define STATISTICTEST_H

#include <Statistics.h>

class Table;

//! Abstract base class for statistic tests
class StatisticTest : public Statistics
{
	Q_OBJECT

	public:
		enum Tail{Left = 0, Right = 1, Both = 2};
		enum TestType{StudentTest = 0, NormalityTest = 1, ChiSquareTest = 2, AnovaTest = 3};

		StatisticTest(ApplicationWindow *parent, double testValue = 0.0, double level = 0.05, const QString& sample = QString());
		void setTail(const Tail& tail){d_tail = tail;};
		void setTestValue(double val){d_test_val = val;};
		void showDescriptiveStatistics(bool show = true){d_descriptive_statistics = show;};

		void setSignificanceLevel(double s){
			if (s < 0.0 || s > 1.0)
				return;
			d_significance_level = s;
		}

		virtual QString logInfo(){return Statistics::logInfo();};
		virtual double statistic(){return 0.0;};
		virtual double pValue(){return 0.0;};
		//! Lower Confidence Limit
		virtual double lcl(double){return 0.0;};
		//! Upper Confidence Limit
		virtual double ucl(double){return 0.0;};

		//! Returns a pointer to the table created to display the results
		virtual Table *resultTable(const QString& name = QString());
		virtual void outputResultsTo(Table *);

	protected:
		double d_test_val;
		double d_significance_level;
		int d_tail;
		bool d_descriptive_statistics;
};

#endif
