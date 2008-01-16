/***************************************************************************
    File                 : ScaleDraw.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
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

//! Extension to QwtScaleDraw
class ScaleDraw: public QwtScaleDraw
{
public:
	enum TicksStyle{None = 0, Out = 1, Both = 2, In = 3};
	enum ScaleType{Numeric = 0, Txt = 1, Day = 2, Month = 3, Time = 4, Date = 5, ColHeader = 6};
	enum NumericFormat{Automatic, Decimal, Scientific, Superscripts};

	ScaleDraw(Plot *plot = 0, const QString& s = QString::null);

	QString formulaString() {return formula_string;};
	void setFormulaString(const QString& formula) {formula_string = formula;};

	double transformValue(double value) const;
	virtual QwtText label(double value) const;

	int labelNumericPrecision(){return d_prec;};
	void setNumericPrecision(int prec){d_prec = prec;};

	int majorTicksStyle(){return d_majTicks;};
	void setMajorTicksStyle(TicksStyle type){d_majTicks = type;};

	int minorTicksStyle(){return d_minTicks;};
	void setMinorTicksStyle(TicksStyle type){d_minTicks = type;};

	void setSelected(bool select = true){d_selected = select;};
	
	int axis() const;

	ScaleType scaleType(){return d_type;};
	void setScaleType(ScaleType type){d_type = type;};
	
	void labelFormat(char &f, int &prec) const;
	
	NumericFormat labelNumericFormat(){return d_numeric_format;};
	void setNumericFormat(NumericFormat format);
	
protected:
	virtual void draw (QPainter *, const QPalette &) const;
    virtual void drawLabel(QPainter *painter, double value) const;
	virtual void drawTick(QPainter *p, double value, int len) const;
	virtual void drawBackbone(QPainter *painter) const;
	void drawBreak(QPainter *) const;

	Plot *d_plot;

private:
	QString formula_string;
	char d_fmt;
    int d_prec;
	int d_minTicks, d_majTicks;
	bool d_selected;

	//! Stores the scale type (numeric, text, etc...)
	ScaleType d_type;

	//! Stores the scale numeric format: Automatic, Decimal, Scientific, Superscripts
	NumericFormat d_numeric_format;
};

class QwtTextScaleDraw: public ScaleDraw
{
public:
	QwtTextScaleDraw(const QStringList& list);

	QwtText label(double value) const;

	QStringList labelsList(){return labels;};
private:
	QStringList labels;
};

class TimeScaleDraw: public ScaleDraw
{
public:
	TimeScaleDraw(const QTime& t, const QString& format);

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

	NameFormat format() {return d_format;};
	QwtText label(double value) const;

private:
	NameFormat d_format;
};

#endif
