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

class Table;

//! Abstract base class for data analysis operations
class Anova : public Statistics
{
	Q_OBJECT

	public:
		Anova(ApplicationWindow *parent, double level);
		void setSignificanceLevel(double);
		bool addSample(const QString& colName);
		bool run();

		virtual QString logInfo();

		double pValue(){return pVal;};
		double FValue(){return F;};

	protected:
		void freeMemory();

		double d_significance_level;
		bool d_two_way;
		QList<Statistics *> d_data_samples;

		long dfTr, dfE, dfT;
		double SSTr, SSE, SST, MSTr, MSE, F, pVal;
};

#endif
