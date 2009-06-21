/***************************************************************************
    File                 : FunctionCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Function curve class

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
#include "FunctionCurve.h"
#include <Graph.h>
#include <MyParser.h>
#include <ScaleEngine.h>

#include <QMessageBox>

FunctionCurve::FunctionCurve(const QString& name):
	PlotCurve(name),
	d_function_type(Normal),
	d_variable("x")
{
	setType(Graph::Function);
	setPlotStyle(Graph::Line);
	d_formulas = QStringList();
}

FunctionCurve::FunctionCurve(const FunctionType& t, const QString& name):
	PlotCurve(name),
	d_function_type(t),
	d_variable("x")
{
	setType(Graph::Function);
	setPlotStyle(Graph::Line);
}

void FunctionCurve::setRange(double from, double to)
{
	d_from = from;
	d_to = to;
}

void FunctionCurve::copy(FunctionCurve* f)
{
	d_function_type = f->functionType();
	d_variable = f->variable();
	d_constants = f->constants();
	d_formulas = f->formulas();
	d_from = f->startRange();
	d_to = f->endRange();
}

QString FunctionCurve::saveToString()
{
	QString s = "<Function>\n";
	s += "<Type>" + QString::number(d_function_type) + "</Type>\n";
	s += "<Title>" + title().text() + "</Title>\n";
	s += "<Expression>" + d_formulas.join("\t") + "</Expression>\n";
	s += "<Variable>" + d_variable + "</Variable>\n";
	s += "<Range>" + QString::number(d_from,'g',15) + "\t" + QString::number(d_to,'g',15) + "</Range>\n";
	s += "<Points>" + QString::number(dataSize()) + "</Points>\n";

	ScaleEngine *sc_engine = (ScaleEngine *)plot()->axisScaleEngine(xAxis());
	if (d_from > 0 && d_to > 0 && sc_engine &&
		sc_engine->type() == QwtScaleTransformation::Log10)
		s += "<Log10>1</Log10>\n";

	QMapIterator<QString, double> i(d_constants);
 	while (i.hasNext()) {
     	i.next();
		s += "<Constant>" + i.key() + "\t" + QString::number(i.value(), 'g', 15) + "</Constant>\n";
 	}
	s += saveCurveLayout();
	s += "</Function>\n";
	return s;
}

void FunctionCurve::restore(Graph *g, const QStringList& lst)
{
	if (!g)
		return;

	int type = 0;
	int points = 0, style = 0;
	int logScale = 0;
	QwtPlotCurve::CurveStyle lineStyle = QwtPlotCurve::NoCurve;
	QStringList formulas;
	QString var, title = QString::null;
	double start = 0.0, end = 0.0;
	QMap<QString, double> constants;
	QStringList::const_iterator line;
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Type>"))
			type = s.remove("<Type>").remove("</Type>").stripWhiteSpace().toInt();
		else if (s.contains("<Title>"))
			title = s.remove("<Title>").remove("</Title>").stripWhiteSpace();
		else if (s.contains("<Expression>"))
			formulas = s.remove("<Expression>").remove("</Expression>").split("\t");
		else if (s.contains("<Variable>"))
			var = s.remove("<Variable>").remove("</Variable>").stripWhiteSpace();
		else if (s.contains("<Range>")){
			QStringList l = s.remove("<Range>").remove("</Range>").split("\t");
			if (l.size() == 2){
				start = l[0].toDouble();
				end = l[1].toDouble();
			}
		} else if (s.contains("<Points>"))
			points = s.remove("<Points>").remove("</Points>").stripWhiteSpace().toInt();
		else if (s.contains("<Log10>"))
			logScale = s.remove("<Log10>").remove("</Log10>").stripWhiteSpace().toInt();
		else if (s.contains("<Constant>")){
			QStringList l = s.remove("<Constant>").remove("</Constant>").split("\t");
			if (l.size() == 2)
				constants.insert(l[0], l[1].toDouble());
		} else if (s.contains("<Style>")){
			style = s.remove("<Style>").remove("</Style>").stripWhiteSpace().toInt();
		} else if (s.contains("<LineStyle>")){
			lineStyle = (QwtPlotCurve::CurveStyle)(s.remove("<LineStyle>").remove("</LineStyle>").stripWhiteSpace().toInt());
			break;
		}
	}

	FunctionCurve *c = new FunctionCurve((FunctionCurve::FunctionType)type, title);
	c->setRange(start, end);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->setConstants(constants);
	c->loadData(points, logScale);
	c->setPlotStyle(style);
	g->insertCurve(c);
	g->setCurveStyle(g->curveIndex(c), lineStyle);

	QStringList l;
	for (line++; line != lst.end(); line++)
        l << *line;
	c->restoreCurveLayout(l);
	g->updatePlot();
}

QString FunctionCurve::legend()
{
	QString label = title().text() + ": ";
	if (d_function_type == Normal)
		label += d_formulas[0];
	else if (d_function_type == Parametric)
	{
		label += "X(" + d_variable + ")=" + d_formulas[0];
		label += ", Y(" + d_variable + ")=" + d_formulas[1];
	}
	else if (d_function_type == Polar)
	{
		label += "R(" + d_variable + ")=" + d_formulas[0];
		label += ", Theta(" + d_variable + ")=" + d_formulas[1];
	}
	return label;
}

void FunctionCurve::loadData(int points, bool xLog10Scale)
{
    if (!points)
        points = dataSize();

	double *X = (double *)malloc(points*sizeof(double));
	if (!X){
		QMessageBox::critical(0, "QtiPlot - Memory Allocation Error",
		"Not enough memory, operation aborted!");
		return;
	}
	double *Y = (double *)malloc(points*sizeof(double));
	if (!Y){
		QMessageBox::critical(0, "QtiPlot  - Memory Allocation Error",
		"Not enough memory, operation aborted!");
		free(X);
		return;
	}

    double step = (d_to - d_from)/(double)(points - 1);
    bool error = false;

	if (d_function_type == Normal){
		MyParser parser;
		double x;
		try {
			parser.DefineVar(d_variable.ascii(), &x);
			QMapIterator<QString, double> i(d_constants);
 			while (i.hasNext()){
     			i.next();
				parser.DefineConst(i.key().ascii(), i.value());
 			}
			parser.SetExpr(d_formulas[0].ascii());

			X[0] = d_from; x = d_from; Y[0] = parser.Eval();

			ScaleEngine *sc_engine = 0;
			if (plot())
				sc_engine = (ScaleEngine *)plot()->axisScaleEngine(xAxis());

			if (xLog10Scale || (d_from > 0 && d_to > 0 && sc_engine &&
				sc_engine->type() == QwtScaleTransformation::Log10)){
				step = log10(d_to/d_from)/(double)(points - 1);
				for (int i = 1; i < points; i++ ){
					x = d_from*pow(10, i*step);
					X[i] = x;
					Y[i] = parser.Eval();
				}
			} else {
				for (int i = 1; i < points; i++ ){
					x += step;
					X[i] = x;
					Y[i] = parser.Eval();
				}
			}
		} catch(mu::ParserError &e) {
			error = true;
		}
	} else if (d_function_type == Parametric || d_function_type == Polar) {
		QStringList aux = d_formulas;
		MyParser xparser;
		MyParser yparser;
		double par;
		if (d_function_type == Polar) {
			QString swap=aux[0];
			aux[0]="("+swap+")*cos("+aux[1]+")";
			aux[1]="("+swap+")*sin("+aux[1]+")";
		}

		try {
			xparser.DefineVar(d_variable.ascii(), &par);
			yparser.DefineVar(d_variable.ascii(), &par);
			xparser.SetExpr(aux[0].ascii());
			yparser.SetExpr(aux[1].ascii());
			par = d_from;
			for (int i = 0; i<points; i++ ){
				X[i] = xparser.Eval();
				Y[i] = yparser.Eval();
				par += step;
			}
		} catch(mu::ParserError &) {
			error = true;
		}
	}

	if (error)
		return;

	setData(X, Y, points);
	free(X); free(Y);
}
