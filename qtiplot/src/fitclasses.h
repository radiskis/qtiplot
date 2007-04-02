/***************************************************************************
    File                 : fitclasses.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Multiple classes derived from Fit
                           
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
#ifndef FITCLASSES_H
#define FITCLASSES_H

#include "Fit.h"

class SigmoidalFit : public Fit
{
	Q_OBJECT

	public:
		SigmoidalFit(ApplicationWindow *parent, Graph *g);
		SigmoidalFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
		SigmoidalFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);
		void guessInitialValues();

	private:
		void init();
		void calculateFitCurveData(double *par, double *X, double *Y);
};

class NonLinearFit : public Fit
{
	Q_OBJECT

	public:
		NonLinearFit(ApplicationWindow *parent, Graph *g);
		NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
		NonLinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);

		void setParametersList(const QStringList& lst);
		void setFormula(const QString& s);

	private:
		void calculateFitCurveData(double *par, double *X, double *Y);
		void init();
};

class PluginFit : public Fit
{
	Q_OBJECT

	public:
		PluginFit(ApplicationWindow *parent, Graph *g);
		PluginFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
		PluginFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);

		bool load(const QString& pluginName);

	private:
		void init();
		typedef double (*fitFunctionEval)(double, double *);
		void calculateFitCurveData(double *par, double *X, double *Y);
		fitFunctionEval f_eval;
};

class PolynomialFit : public Fit
{
	Q_OBJECT

	public:
		PolynomialFit(ApplicationWindow *parent, Graph *g, int order = 2, bool legend = false);
		PolynomialFit(ApplicationWindow *parent, Graph *g, QString& curveTitle, int order = 2, bool legend = false);
		PolynomialFit(ApplicationWindow *parent, Graph *g, QString& curveTitle, double start, double end, int order = 2, bool legend = false);

		virtual QString legendInfo();
		void fit();

		static QString generateFormula(int order);
		static QStringList generateParameterList(int order);

	private:
		void init();
		void calculateFitCurveData(double *par, double *X, double *Y);

		int d_order;
		bool show_legend;
};

class LinearFit : public Fit
{
	Q_OBJECT

	public:
		LinearFit(ApplicationWindow *parent, Graph *g);
		LinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle);
		LinearFit(ApplicationWindow *parent, Graph *g, const QString& curveTitle, double start, double end);

		void fit();

	private:
		void init();
		void calculateFitCurveData(double *par, double *X, double *Y);
};
#endif