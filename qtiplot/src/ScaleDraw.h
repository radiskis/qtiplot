/***************************************************************************
    File                 : ScaleDraw.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Extension to QwtScaleDraw

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
#ifndef SCALES_H
#define SCALES_H

#include <QDateTime>
#include <QStringList>
#include <QLocale>

#include "Plot.h"
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <float.h>

class ScaleEngine: public QwtScaleEngine
{
public:
	ScaleEngine(QwtScaleTransformation::Type type = QwtScaleTransformation::Linear, 
				double left_break = -DBL_MAX, double right_break = DBL_MAX): 
				QwtScaleEngine(), d_type(type), d_break_left(left_break), d_break_right(right_break){};
	QwtScaleTransformation* transformation() const;
	virtual QwtScaleDiv divideScale(double x1, double x2, int maxMajSteps,
		int maxMinSteps, double stepSize = 0.0) const;
	virtual void autoScale (int maxNumSteps, double &x1, double &x2, double &stepSize) const;
		
	double axisBreakLeft(){return d_break_left;};
	double axisBreakRight(){return d_break_right;};
	void setAxisBreak(double from, double to){d_break_left = from; d_break_right = to;};
	
	QwtScaleTransformation::Type type(){return d_type;};
	void setType(QwtScaleTransformation::Type type){d_type = type;};
	
	bool hasBreak(){return (d_break_left == d_break_right || (d_break_left == -DBL_MAX && d_break_right == DBL_MAX))?false:true;};

private:
	QwtScaleTransformation::Type d_type;
	double d_break_left, d_break_right;
};

class ScaleTransformation: public QwtScaleTransformation
{
public:
	ScaleTransformation(Type type = QwtScaleTransformation::Linear, double left_break = -DBL_MAX, double right_break = DBL_MAX): 
		QwtScaleTransformation(Other), d_break_left(left_break), d_break_right(right_break), d_type(type){};
	virtual double xForm(double x, double, double, double p1, double p2) const;
	QwtScaleTransformation* copy() const;
			
private:
	double d_break_left, d_break_right;
	Type d_type;
};

//! Extension to QwtScaleDraw
class ScaleDraw: public QwtScaleDraw
{
public:
	enum TicksStyle{None = 0, Out = 1, Both = 2, In = 3};

	ScaleDraw(Plot *plot = 0, const QString& s = QString::null);
	virtual ~ScaleDraw(){};

	QString formulaString() {return formula_string;};
	void setFormulaString(const QString& formula) {formula_string = formula;};

	double transformValue(double value) const;
	virtual QwtText label(double value) const;

	void labelFormat(char &f, int &prec) const;
	void setLabelFormat(char f, int prec);

	int labelNumericPrecision(){return d_prec;};

	int majorTicksStyle(){return d_majTicks;};
	void setMajorTicksStyle(TicksStyle type){d_majTicks = type;};

	int minorTicksStyle(){return d_minTicks;};
	void setMinorTicksStyle(TicksStyle type){d_minTicks = type;};

	void setSelected(bool select = true){d_selected = select;};
	
protected:
    virtual void drawLabel(QPainter *painter, double value) const;
	virtual void drawTick(QPainter *p, double value, int len) const;

	Plot *d_plot;

private:
	QString formula_string;
	char d_fmt;
    int d_prec;
	int d_minTicks, d_majTicks;
	bool d_selected;
};

class QwtTextScaleDraw: public ScaleDraw
{
public:
	QwtTextScaleDraw(const QStringList& list);
	~QwtTextScaleDraw(){};

	QwtText label(double value) const;

	QStringList labelsList(){return labels;};
private:
	QStringList labels;
};

class TimeScaleDraw: public ScaleDraw
{
public:
	TimeScaleDraw(const QTime& t, const QString& format);
	~TimeScaleDraw(){};

	QString origin();
	QString timeFormat() {return t_format;};

	QwtText label(double value) const;

private:
	QTime t_origin;
	QString t_format;
};

class DateScaleDraw: public ScaleDraw
{
public:
	DateScaleDraw(const QDate& t, const QString& format);
	~DateScaleDraw(){};

	QString origin();

	QString format() {return t_format;};
	QwtText label(double value) const;

private:
	QDate t_origin;
	QString t_format;
};

class WeekDayScaleDraw: public ScaleDraw
{
public:
	enum NameFormat{ShortName, LongName, Initial};

	WeekDayScaleDraw(NameFormat format = ShortName);
	~WeekDayScaleDraw(){};

	NameFormat format() {return d_format;};
	QwtText label(double value) const;

private:
	NameFormat d_format;
};

class MonthScaleDraw: public ScaleDraw
{
public:
	enum NameFormat{ShortName, LongName, Initial};

	MonthScaleDraw(NameFormat format = ShortName);
	~MonthScaleDraw(){};

	NameFormat format() {return d_format;};
	QwtText label(double value) const;

private:
	NameFormat d_format;
};

class QwtSupersciptsScaleDraw: public ScaleDraw
{
public:
	QwtSupersciptsScaleDraw(Plot *plot, const QString& s = QString::null);
	~QwtSupersciptsScaleDraw(){};

	QwtText label(double value) const;
};

#endif
