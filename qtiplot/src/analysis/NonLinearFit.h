/***************************************************************************
    File                 : NonLinearFit.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : NonLinearFit class

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
#ifndef NONLINEARFIT_H
#define NONLINEARFIT_H

#include "Fit.h"
	
class NonLinearFit : public Fit
{
	Q_OBJECT

	public:
		NonLinearFit(ApplicationWindow *parent, Graph *g);
		NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
		NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);
		NonLinearFit(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int startRow = 1, int endRow = -1);
	
        double eval(double *par, double x);

		virtual void setParametersList(const QStringList& lst);
		virtual void setFormula(const QString& s);
	
		QMap<QString, double> constants(){return d_constants;};
		void setConstant(const QString& parName, double val);
		void removeConstant(const QString& parName){d_constants.remove(parName);};
		void removeConstants(){d_constants.clear();};
		int constantsCount(){return d_constants.size();};
		QList<QString> constantsList(){return d_constants.keys();};
		double constValue(const QString& name){return d_constants.value(name);};
		
		virtual QString legendInfo();
				
	protected:
		QString logFitInfo(int iterations, int status);
		FunctionCurve* insertFitFunctionCurve(const QString& name, double *x, double *y, int penWidth);

	private:
		void calculateFitCurveData(double *X, double *Y);
		void init();
	
		QMap<QString, double> d_constants;
};
#endif
