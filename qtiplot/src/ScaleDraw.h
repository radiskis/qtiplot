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
				double left_break = -DBL_MAX, double right_break = DBL_MAX);
	QwtScaleTransformation* transformation() const;
	virtual QwtScaleDiv divideScale(double x1, double x2, int maxMajSteps,
		int maxMinSteps, double stepSize = 0.0) const;
	virtual void autoScale (int maxNumSteps, double &x1, double &x2, double &stepSize) const;

    double axisBreakLeft() const;
    double axisBreakRight() const;
	void setAxisBreak(double from, double to){d_break_left = from; d_break_right = to;};

	int breakWidth() const;
	void setBreakWidth(int width);

	int breakPosition() const;
	void setBreakPosition(int pos);

    double stepBeforeBreak() const;
    void setStepBeforeBreak(double step);

    double stepAfterBreak() const;
    void setStepAfterBreak(double step);

    int minTicksBeforeBreak() const;
    void setMinTicksBeforeBreak(int ticks);

    int minTicksAfterBreak() const;
    void setMinTicksAfterBreak(int ticks);

    bool log10ScaleAfterBreak() const;
    void setLog10ScaleAfterBreak(bool on);

	QwtScaleTransformation::Type type() const;
	void setType(QwtScaleTransformation::Type type){d_type = type;};

	bool hasBreak() const;
	void clone(const ScaleEngine *engine);

private:
	QwtScaleTransformation::Type d_type;
	double d_break_left, d_break_right;
	//! Position of axis break (% of axis length)
	int d_break_pos;
	//! Scale increment before and after break
	double d_step_before, d_step_after;
	//! Minor ticks before and after break
	int d_minor_ticks_before, d_minor_ticks_after;
	//! Log10 scale after break
	bool d_log10_scale_after;
	//! Width of the axis break in pixels
	int d_break_width;
};

class ScaleTransformation: public QwtScaleTransformation
{
public:
	ScaleTransformation(const ScaleEngine *engine):QwtScaleTransformation(Other), d_engine(engine){};
	virtual double xForm(double x, double, double, double p1, double p2) const;
	QwtScaleTransformation* copy() const;

private:
    //! The scale engine that generates the transformation
	const ScaleEngine* d_engine;
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
	virtual void drawBackbone(QPainter *painter) const;
	void drawBreak(QPainter *p, double value, int len) const;

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
