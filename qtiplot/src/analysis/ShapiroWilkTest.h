/***************************************************************************
	File                 : ShapiroWilkTest.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Normality test

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
#ifndef NORMTEST_H
#define NORMTEST_H

#include <StatisticTest.h>

//! Normality test
class ShapiroWilkTest : public StatisticTest
{
	Q_OBJECT

	public:
		ShapiroWilkTest(ApplicationWindow *parent, const QString& sample = QString());

		double w(){return d_w;};
		double statistic(){return d_w;};
		double pValue(){return d_pValue;};
		virtual QString logInfo();
		QString shortLogInfo();

	protected:
		QString infoString(bool header = true);
		void swilk(int *init, double *x, int *n, int *n1, int *n2, double *a,  double *w, double *pw, int *ifault);
		static double poly(const double *cc, int nord, double x);

		double d_w;
		double d_pValue;
};

#endif
