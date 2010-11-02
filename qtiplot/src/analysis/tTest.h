/***************************************************************************
	File                 : tTest.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Abstract base class for statistics data analysis

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

#include <Statistics.h>

class Table;

//! Abstract base class for data analysis operations
class tTest : public Statistics
{
	Q_OBJECT

	public:
		enum Tail{Left = 0, Right = 1, Both = 2};

		tTest(ApplicationWindow *parent, double testMean, double level, const QString& sample = QString());
		void setTail(const Tail&);
		void setTestMean(double);
		void setSignificanceLevel(double);
		virtual QString logInfo();

		double t();
		double pValue();
		int dof();

		double power(double alpha, int size = 0);
		//! Lower Confidence Limit
		double lcl(double confidenceLevel);
		//! Upper Confidence Limit
		double ucl(double confidenceLevel);

	protected:
		double d_test_mean;
		double d_significance_level;
		int d_tail;
};

#endif
