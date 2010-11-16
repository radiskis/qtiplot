/***************************************************************************
	File                 : tTest.h
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
#ifndef TTEST_H
#define TTEST_H

#include <StatisticTest.h>

//! Student's t-Test
class tTest : public StatisticTest
{
	Q_OBJECT

	public:
		tTest(ApplicationWindow *parent, double testValue, double level,
			const QString& sample1 = QString(), const QString& sample2 = QString(), bool paired = false);

		bool setSample1(const QString& colName){return setData(colName);};
		bool setSample2(const QString& colName, bool paired = false);

		virtual QString logInfo();

		double t();
		double statistic(){return t();};
		double pValue();
		int dof();

		double power(double alpha, int size = 0);
		//! Lower Confidence Limit
		double lcl(double confidenceLevel);
		//! Upper Confidence Limit
		double ucl(double confidenceLevel);

	protected:
		double t(int size);
		void freeMemory();

		Statistics *d_sample2;
		bool d_independent_test;
		double d_s12, d_diff;
};

#endif
