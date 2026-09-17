/***************************************************************************
    File                 : Integration.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical integration of data sets

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
#ifndef INTEGRATION_H
#define INTEGRATION_H

#include "Filter.h"

class Integration : public Filter
{
Q_OBJECT

public:
	enum Integrand{DataSet, AnalyticalFunction};

	Integration(ApplicationWindow *parent, Graph *g = nullptr);
	Integration(ApplicationWindow *parent, PlotCurve *c);
	Integration(ApplicationWindow *parent, PlotCurve *c, double start, double end);
	Integration(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
	Integration(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);
	Integration(ApplicationWindow *parent, Table *t, const QString& xCol, const QString& yCol, int start = 0, int end = -1, bool sort = false);
	Integration(const QString& formula, const QString& var, ApplicationWindow *parent, Graph *g, double start, double end);

	void setWorkspaceSize(int size){d_workspace_size = size;};

	double area() const {return d_area;};
	QString variable() const {return d_variable;};
	QString formula() const {return d_formula;};

private:
    void init();
    QString logInfo() override;

    void output() override;

	double trapez();
	double gslIntegration();

    //! the value of the integral
    double d_area = 0.0;
	//! the value of the estimated error in GSL integration
	double d_error = 0.0;
	//! the value of the workspace size in GSL integration
	int d_workspace_size = 1000;

	//! the type of the integrand
	Integrand d_integrand = DataSet;
	//! Analytical function to be integrated
	QString d_formula;
	//! Variable name for the function to be integrated
	QString d_variable;
};

#endif
