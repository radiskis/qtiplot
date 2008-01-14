/***************************************************************************
    File                 : ScaleDraw.cpp
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
#include "ScaleDraw.h"
#include "MyParser.h"

#include <QDateTime>
#include <QPainter>
#include <QMatrix>

#include <qwt_painter.h>
#include <qwt_text.h>

QwtScaleTransformation* ScaleEngine::transformation() const
{
	return new ScaleTransformation(this);
}

double ScaleTransformation::xForm(double s, double s1, double s2, double p1, double p2) const
{
	if (!d_engine->hasBreak()){
		QwtScaleTransformation *tr = new QwtScaleTransformation (d_engine->type());
		double res = tr->xForm(s, s1, s2, p1, p2);
		delete tr;
		return res;
	}

    const int d_break_space = d_engine->breakWidth();
    const double d_break_left = d_engine->axisBreakLeft();
    const double d_break_right = d_engine->axisBreakRight();
	const double pm = p1 + (p2 - p1)*(double)d_engine->breakPosition()/100.0;

	double pml, pmr;
	if (p1 < p2){
		pml = pm - d_break_space;
		pmr = pm + d_break_space;
	} else {
		pml = pm + d_break_space;
		pmr = pm - d_break_space;
	}

	if (s > d_break_left && s < d_break_right)
		return pm;

    QwtScaleTransformation::Type  d_type = d_engine->type();
    if (s <= d_break_left){
        if (d_type == QwtScaleTransformation::Linear)
            return p1 + (s - s1)/(d_break_left - s1)*(pml - p1);
        else if (d_type == QwtScaleTransformation::Log10)
            return p1 + log(s/s1)/log(d_break_left/s1)*(pml - p1);
    }

	if (s >= d_break_right){
	    if (d_engine->log10ScaleAfterBreak())
            return pmr + log(s/d_break_right)/log(s2/d_break_right)*(p2 - pmr);
	    else
            return pmr + (s - d_break_right)/(s2 - d_break_right)*(p2 - pmr);
	}

	return DBL_MAX; // something invalid
}

QwtScaleTransformation *ScaleTransformation::copy() const
{
    return new ScaleTransformation(d_engine);
}

/*****************************************************************************
 *
 * Class ScaleEngine
 *
 *****************************************************************************/

ScaleEngine::ScaleEngine(QwtScaleTransformation::Type type,double left_break, double right_break): QwtScaleEngine(),
d_type(type),
d_break_left(left_break),
d_break_right(right_break),
d_break_width(4),
d_break_pos(50),
d_step_before(0.0),
d_step_after(0.0),
d_minor_ticks_before(1),
d_minor_ticks_after(1),
d_log10_scale_after(false)
{}

bool ScaleEngine::hasBreak() const
{
return (d_break_left == d_break_right || (d_break_left == -DBL_MAX && d_break_right == DBL_MAX))?false:true;
}

double ScaleEngine::axisBreakLeft() const
{
    return d_break_left;
}

double ScaleEngine::axisBreakRight() const
{
    return d_break_right;
}

int ScaleEngine::breakWidth() const
{
    return d_break_width;
}

void ScaleEngine::setBreakWidth(int width)
{
    d_break_width = width;
}

int ScaleEngine::breakPosition() const
{
    return d_break_pos;
}

void ScaleEngine::setBreakPosition(int pos)
{
    d_break_pos = pos;
}

double ScaleEngine::stepBeforeBreak() const
{
    return d_step_before;
}

void ScaleEngine::setStepBeforeBreak(double step)
{
    d_step_before = step;
}

double ScaleEngine::stepAfterBreak() const
{
    return d_step_after;
}

void ScaleEngine::setStepAfterBreak(double step)
{
    d_step_after = step;
}

QwtScaleTransformation::Type ScaleEngine::type() const
{
    return d_type;
}

int ScaleEngine::minTicksBeforeBreak() const
{
    return d_minor_ticks_before;
}

void ScaleEngine::setMinTicksBeforeBreak(int ticks)
{
    d_minor_ticks_before = ticks;
}

int ScaleEngine::minTicksAfterBreak() const
{
    return d_minor_ticks_after;
}

void ScaleEngine::setMinTicksAfterBreak(int ticks)
{
    d_minor_ticks_after = ticks;
}

bool ScaleEngine::log10ScaleAfterBreak() const
{
    return d_log10_scale_after;
}

void ScaleEngine::setLog10ScaleAfterBreak(bool on)
{
    d_log10_scale_after = on;
}

void ScaleEngine::clone(const ScaleEngine *engine)
{
    d_type = engine->type();
	d_break_left = engine->axisBreakLeft();
	d_break_right = engine->axisBreakRight();
    d_break_pos = engine->breakPosition();
	d_step_before = engine->stepBeforeBreak();
	d_step_after = engine->stepAfterBreak();
	d_minor_ticks_before = engine->minTicksBeforeBreak();
	d_minor_ticks_after = engine->minTicksAfterBreak();
    d_log10_scale_after = engine->log10ScaleAfterBreak();
    d_break_width = engine->breakWidth();
}

QwtScaleDiv ScaleEngine::divideScale(double x1, double x2, int maxMajSteps,
		int maxMinSteps, double stepSize) const
{
	QwtScaleEngine *engine;
	if (d_type == QwtScaleTransformation::Log10)
		engine = new QwtLog10ScaleEngine();
	else
		engine = new QwtLinearScaleEngine();

	if ((d_break_left == d_break_right) ||
		(d_break_left == -DBL_MAX && d_break_right == DBL_MAX)){
		QwtScaleDiv div = engine->divideScale(x1, x2, maxMajSteps, maxMinSteps, stepSize);
		delete engine;
		return div;
	}

    int max_min_intervals = d_minor_ticks_before;
	if (d_minor_ticks_before == 1)
		max_min_intervals = 3;
	if (d_minor_ticks_before > 1)
		max_min_intervals = d_minor_ticks_before + 1;
	QwtScaleDiv div1 = engine->divideScale(x1, d_break_left, maxMajSteps/2, max_min_intervals, d_step_before);

    max_min_intervals = d_minor_ticks_after;
	if (d_minor_ticks_after == 1)
		max_min_intervals = 3;
	if (d_minor_ticks_after > 1)
		max_min_intervals = d_minor_ticks_after + 1;

    delete engine;
    if (d_log10_scale_after)
		engine = new QwtLog10ScaleEngine();
	else
		engine = new QwtLinearScaleEngine();

    QwtScaleDiv div2 = engine->divideScale(d_break_right, x2, maxMajSteps/2, max_min_intervals, d_step_after);

    QwtValueList ticks[QwtScaleDiv::NTickTypes];
    ticks[QwtScaleDiv::MinorTick] = div1.ticks(QwtScaleDiv::MinorTick) + div2.ticks(QwtScaleDiv::MinorTick);
    ticks[QwtScaleDiv::MediumTick] = div1.ticks(QwtScaleDiv::MediumTick) + div2.ticks(QwtScaleDiv::MediumTick);
    ticks[QwtScaleDiv::MajorTick] = div1.ticks(QwtScaleDiv::MajorTick) + div2.ticks(QwtScaleDiv::MajorTick);

	delete engine;
	return QwtScaleDiv(x1, x2, ticks);
}

void ScaleEngine::autoScale (int maxNumSteps, double &x1, double &x2, double &stepSize) const
{
	QwtScaleEngine *engine;
	if (d_type == QwtScaleTransformation::Log10)
		engine = new QwtLog10ScaleEngine();
	else
		engine = new QwtLinearScaleEngine();

	engine->autoScale(maxNumSteps, x1, x2, stepSize);
	delete engine;
}

ScaleDraw::ScaleDraw(Plot *plot, const QString& s):
	d_plot(plot),
	d_fmt('g'),
    d_prec(4),
	formula_string (s),
	d_majTicks(Out),
	d_minTicks(Out),
	d_selected(false)
{}

QwtText ScaleDraw::label(double value) const
{
	if (d_plot)
		return QwtText(d_plot->locale().toString(transformValue(value), d_fmt, d_prec));
	else
		return QwtText(QLocale::system().toString(transformValue(value), d_fmt, d_prec));
}

void ScaleDraw::drawLabel(QPainter *painter, double value) const
{
    if (!d_plot)
        return;

    int axis = QwtPlot::xBottom;
    switch(alignment()){
        case BottomScale:
        break;
        case TopScale:
            axis = QwtPlot::xTop;
        break;
        case LeftScale:
            axis = QwtPlot::yLeft;
        break;
        case RightScale:
            axis = QwtPlot::yRight;
        break;
    }

    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis);
    if (sc_engine->hasBreak() && sc_engine->axisBreakLeft() == value)
        return;

    QwtText lbl = tickLabel(painter->font(), value);
    if (lbl.isEmpty())
        return;

    const QPoint pos = labelPosition(value);

    QSize labelSize = lbl.textSize(painter->font());
    if ( labelSize.height() % 2 )
        labelSize.setHeight(labelSize.height() + 1);

    const QMatrix m = labelMatrix(pos, labelSize);

    painter->save();
    painter->setMatrix(m, true);
    if (d_selected)
        lbl.setBackgroundPen(QPen(Qt::blue));
    else
        lbl.setBackgroundPen(QPen(Qt::NoPen));

    lbl.draw(painter, QRect(QPoint(0, 0), labelSize));
    painter->restore();
}

double ScaleDraw::transformValue(double value) const
{
	if (!formula_string.isEmpty()){
		double lbl=0.0;
		try{
			MyParser parser;
			if (formula_string.contains("x"))
				parser.DefineVar("x", &value);
			else if (formula_string.contains("y"))
				parser.DefineVar("y", &value);

			parser.SetExpr(formula_string.ascii());
			lbl=parser.Eval();
        }
        catch(mu::ParserError &){
			return 0;
        }
		return lbl;
    } else
        return value;
}

/*!
  \brief Set the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f', 'g'
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa labelFormat()
*/
void ScaleDraw::setLabelFormat(char f, int prec)
{
d_fmt = f;
d_prec = prec;
}

/*!
  \brief Return the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f' or 'g'
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa setLabelFormat()
*/
void ScaleDraw::labelFormat(char &f, int &prec) const
{
    f = d_fmt;
    prec = d_prec;
}

void ScaleDraw::drawTick(QPainter *p, double value, int len) const
{
    int axis = QwtPlot::xBottom;
    switch(alignment()){
        case BottomScale:
        break;
        case TopScale:
            axis = QwtPlot::xTop;
        break;
        case LeftScale:
            axis = QwtPlot::yLeft;
        break;
        case RightScale:
            axis = QwtPlot::yRight;
        break;
    }

    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis);
    if (sc_engine->hasBreak() && sc_engine->axisBreakLeft() == value)
        return;

    QwtScaleDiv scDiv = scaleDiv();
    QwtValueList majTicks = scDiv.ticks(QwtScaleDiv::MajorTick);
    if (majTicks.contains(value) && (d_majTicks == In || d_majTicks == None))
        return;

    QwtValueList medTicks = scDiv.ticks(QwtScaleDiv::MediumTick);
    if (medTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
        return;

    QwtValueList minTicks = scDiv.ticks(QwtScaleDiv::MinorTick);
    if (minTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
        return;

    QwtScaleDraw::drawTick(p, value, len);
}

void ScaleDraw::drawBackbone(QPainter *painter) const
{
    int axis = QwtPlot::xBottom;
    switch(alignment()){
        case BottomScale:
        break;
        case TopScale:
            axis = QwtPlot::xTop;
        break;
        case LeftScale:
            axis = QwtPlot::yLeft;
        break;
        case RightScale:
            axis = QwtPlot::yRight;
        break;
    }

    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis);
    if (!sc_engine->hasBreak()){
        QwtScaleDraw::drawBackbone(painter);
        return;
    }

    const int bw2 = painter->pen().width() / 2;
    const QPoint &pos = this->pos();
    const int len = length() - 1;

    const QwtScaleMap m = map();
    const int lb = m.transform(sc_engine->axisBreakLeft());
    const int rb = m.transform(sc_engine->axisBreakRight());
    int aux;
    switch(alignment())
    {
        case LeftScale:
            aux = pos.x() - bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb );
            QwtPainter::drawLine(painter, aux, lb, aux, pos.y() + len );
            break;
        case RightScale:
            aux = pos.x() + bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb);
            QwtPainter::drawLine(painter, aux, lb, aux, pos.y() + len);
            break;
        case TopScale:
            aux = pos.y() - bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, pos.x() + lb, aux);
            QwtPainter::drawLine(painter, rb, aux, pos.x() + len, aux);
            break;
        case BottomScale:
            aux = pos.y() + bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, pos.x() + lb, aux);
            QwtPainter::drawLine(painter, rb, aux, pos.x() + len, aux);
            break;
    }
}

/*****************************************************************************
 *
 * Class QwtTextScaleDraw
 *
 *****************************************************************************/

QwtTextScaleDraw::QwtTextScaleDraw(const QStringList& list):
					  labels(list)
{}

QwtText QwtTextScaleDraw::label(double value) const
{
	const QwtScaleDiv scDiv = scaleDiv();
	if (!scDiv.contains (value))
		return QwtText();

	QwtValueList lst = scDiv.ticks (QwtScaleDiv::MajorTick);

	int index = 0;
	if (map().transformation()->type() == QwtScaleTransformation::Linear){
        int step = abs(lst[1] - lst[0]);
        index = lst[0] + step*lst.indexOf(value) - 1;
	}
	else if (map().transformation()->type() == QwtScaleTransformation::Log10){
	    if (lst.count() >= 2){
            double step = lst[1]/lst[0];
            index = lst[0]*pow(step, lst.indexOf(value)) - 1;
	    }
	}

	if (index >= 0 && index < (int)labels.count())
        return QwtText(labels[index]);
	else
		return QwtText();
}

/*****************************************************************************
 *
 * Class TimeScaleDraw
 *
 *****************************************************************************/

TimeScaleDraw::TimeScaleDraw(const QTime& t, const QString& format):
		t_origin (t),
		t_format (format)
		{}

QString TimeScaleDraw::origin()
{
return t_origin.toString ( "hh:mm:ss.zzz" );
}


QwtText TimeScaleDraw::label(double value) const
{
QTime t = t_origin.addMSecs ( (int)value );
return QwtText(t.toString ( t_format ));
}

/*****************************************************************************
 *
 * Class DateScaleDraw
 *
 *****************************************************************************/

DateScaleDraw::DateScaleDraw(const QDate& t, const QString& format):
			  t_origin (t),
			  t_format (format)
{}

QString DateScaleDraw::origin()
{
return t_origin.toString ();
}

QwtText DateScaleDraw::label(double value) const
{
QDate t = t_origin.addDays( (int) value );
return QwtText(t.toString ( t_format ));
}

/*****************************************************************************
 *
 * Class WeekDayScaleDraw
 *
 *****************************************************************************/

WeekDayScaleDraw:: WeekDayScaleDraw(NameFormat format):
				d_format(format)
{}

QwtText WeekDayScaleDraw::label(double value) const
{
int val = int(transformValue(value))%7;

if (val < 0)
	val = 7 - abs(val);
else if (val == 0)
	val = 7;

QString day;
switch(d_format)
	{
	case  ShortName:
		day = QDate::shortDayName (val);
	break;

	case  LongName:
		day = QDate::longDayName (val);
	break;

	case  Initial:
		day = (QDate::shortDayName (val)).left(1);
	break;
	}
return QwtText(day);
}

/*****************************************************************************
 *
 * Class MonthScaleDraw
 *
 *****************************************************************************/

MonthScaleDraw::MonthScaleDraw(NameFormat format):
		d_format(format)
{};

QwtText MonthScaleDraw::label(double value) const
{
int val = int(transformValue(value))%12;

if (val < 0)
	val = 12 - abs(val);
else if (val == 0)
	val = 12;

QString day;
switch(d_format)
	{
	case  ShortName:
		day = QDate::shortMonthName (val);
	break;

	case  LongName:
		day = QDate::longMonthName (val);
	break;

	case  Initial:
		day = (QDate::shortMonthName (val)).left(1);
	break;
	}
return QwtText(day);
}

/*****************************************************************************
 *
 * Class QwtSupersciptsScaleDraw
 *
 *****************************************************************************/

QwtSupersciptsScaleDraw::QwtSupersciptsScaleDraw(Plot *plot, const QString& s):
	ScaleDraw(plot, s)
{}

QwtText QwtSupersciptsScaleDraw::label(double value) const
{
	char f;
	int prec;
	labelFormat(f, prec);

	QLocale locale = d_plot->locale();
	QString txt = locale.toString(transformValue(value), 'e', prec);
	QStringList list = txt.split( "e", QString::SkipEmptyParts);
	if (list[0].toDouble() == 0.0)
		return QString("0");

	QString s= list[1];
	int l = s.length();
	QChar sign = s[0];

	s.remove (sign);

	while (l>1 && s.startsWith ("0", false)){
		s.remove ( 0, 1 );
		l = s.length();
	}

	if (sign == '-')
		s.prepend(sign);

	if (list[0] == "1")
		return QwtText("10<sup>" + s + "</sup>");
	else
		return QwtText(list[0] + "x10<sup>" + s + "</sup>");
}
