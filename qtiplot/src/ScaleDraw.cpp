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
#include "plot2D/ScaleEngine.h"

#include <QDateTime>
#include <QPainter>
#include <QMatrix>

#include <qwt_painter.h>
#include <qwt_text.h>

/*****************************************************************************
 *
 * Class ScaleDraw
 *
 *****************************************************************************/

ScaleDraw::ScaleDraw(Plot *plot, const QString& s):
	d_plot(plot),
	d_type(Numeric),
	d_numeric_format(Automatic),
	d_fmt('g'),
    d_prec(4),
	formula_string (s),
	d_majTicks(Out),
	d_minTicks(Out),
	d_selected(false)
{}

QwtText ScaleDraw::label(double value) const
{
	switch (d_type){
		case Numeric:	
			if (d_numeric_format == Superscripts){
				QLocale locale = d_plot->locale();
				QString txt = locale.toString(transformValue(value), 'e', d_prec);
				QStringList list = txt.split("e", QString::SkipEmptyParts);
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
			else if (d_plot)
				return QwtText(d_plot->locale().toString(transformValue(value), d_fmt, d_prec));
			else
				return QwtText(QLocale::system().toString(transformValue(value), d_fmt, d_prec));
		break;
	}
}

void ScaleDraw::drawLabel(QPainter *painter, double value) const
{
    if (!d_plot)
        return;

    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
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

void ScaleDraw::setNumericFormat(NumericFormat format)
{
	if (d_numeric_format == format)
		return;

	d_numeric_format = format;
	
	switch (format){	
		case Automatic:
			d_fmt = 'g';
		break;
		case Scientific:
			d_fmt = 'e';
		break;
		case Decimal:
			d_fmt = 'f';
		case Superscripts:
			d_fmt = 's';
		break;
	}
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

int ScaleDraw::axis() const
{
	int a = QwtPlot::xBottom;
    int align = alignment();
    switch(align){
        case BottomScale:
        break;
        case TopScale:
            a = QwtPlot::xTop;
        break;
        case LeftScale:
            a = QwtPlot::yLeft;
        break;
        case RightScale:
            a = QwtPlot::yRight;
        break;
    }
	return a;
}

void ScaleDraw::drawTick(QPainter *p, double value, int len) const
{
	ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
    if (sc_engine->hasBreak()){
		int align = alignment();
        if (sc_engine->axisBreakLeft() == value && (align == BottomScale || align == LeftScale))
            return;
        else if (sc_engine->axisBreakRight() == value && (align == TopScale || align == RightScale))
            return;
    }
	
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

void ScaleDraw::draw(QPainter *painter, const QPalette& palette) const
{
	drawBreak(painter);
	QwtScaleDraw::draw(painter, palette);
}

void ScaleDraw::drawBreak(QPainter *painter) const
{
	ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
    if (!sc_engine->hasBreak() || !sc_engine->hasBreakDecoration())
        return;
	
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

	int len = majTickLength();
    int pw2 = qwtMin((int)painter->pen().width(), len) / 2;

    QwtScaleMap scaleMap = map();
    const QwtMetricsMap metricsMap = QwtPainter::metricsMap();
    QPoint pos = this->pos();

    if ( !metricsMap.isIdentity() ){
        QwtPainter::resetMetricsMap();
        pos = metricsMap.layoutToDevice(pos);

        if ( orientation() == Qt::Vertical ){
            scaleMap.setPaintInterval(
                metricsMap.layoutToDeviceY((int)scaleMap.p1()),
                metricsMap.layoutToDeviceY((int)scaleMap.p2()));
            len = metricsMap.layoutToDeviceX(len);
        } else {
            scaleMap.setPaintInterval(
                metricsMap.layoutToDeviceX((int)scaleMap.p1()),
                metricsMap.layoutToDeviceX((int)scaleMap.p2()));
            len = metricsMap.layoutToDeviceY(len);
        }
    }

    int lval = scaleMap.transform(sc_engine->axisBreakLeft());
	int rval = scaleMap.transform(sc_engine->axisBreakRight());
    switch(alignment()){
        case LeftScale:
            lval = lval + 1;
			rval = rval + 1;
            QwtPainter::drawLine(painter, pos.x() - pw2, lval, pos.x() - len, lval + len);
			QwtPainter::drawLine(painter, pos.x() - pw2, rval, pos.x() - len, rval + len);
        break;
        case RightScale:
            lval = lval - 1;
			rval = rval - 1;
            QwtPainter::drawLine(painter, pos.x() + pw2, lval, pos.x() + len, lval - len);
			QwtPainter::drawLine(painter, pos.x() + pw2, rval, pos.x() + len, rval - len);
        break;
        case BottomScale:
            QwtPainter::drawLine(painter, lval, pos.y() + pw2, lval - len, pos.y() + len);
			QwtPainter::drawLine(painter, rval, pos.y() + pw2, rval - len, pos.y() + len);
        break;
        case TopScale:
            //lval = lval + 1;
            QwtPainter::drawLine(painter, lval, pos.y() - pw2, lval + len, pos.y() - len);
			QwtPainter::drawLine(painter, rval, pos.y() - pw2, rval + len, pos.y() - len);
        break;
    }
    QwtPainter::setMetricsMap(metricsMap); // restore metrics map
    painter->restore();
}

void ScaleDraw::drawBackbone(QPainter *painter) const
{
    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
    if (!sc_engine->hasBreak()){
        QwtScaleDraw::drawBackbone(painter);
        return;
    }

    QwtScaleMap scaleMap = map();
    const QwtMetricsMap metricsMap = QwtPainter::metricsMap();
    QPoint pos = this->pos();

    if ( !metricsMap.isIdentity() ){
        QwtPainter::resetMetricsMap();
        pos = metricsMap.layoutToDevice(pos);

        if ( orientation() == Qt::Vertical ){
            scaleMap.setPaintInterval(
                metricsMap.layoutToDeviceY((int)scaleMap.p1()),
                metricsMap.layoutToDeviceY((int)scaleMap.p2()));
        } else {
            scaleMap.setPaintInterval(
                metricsMap.layoutToDeviceX((int)scaleMap.p1()),
                metricsMap.layoutToDeviceX((int)scaleMap.p2()));
        }
    }

    const int lb = scaleMap.transform(sc_engine->axisBreakLeft());
    const int rb = scaleMap.transform(sc_engine->axisBreakRight());
	const int bw = painter->pen().width();
    const int bw2 = bw / 2;
    const int len = length() - 1;
    int aux;
    switch(alignment())
    {
        case LeftScale:
            aux = pos.x() - bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb );
            QwtPainter::drawLine(painter, aux, lb, aux, pos.y() + len);
            break;
        case RightScale:
            aux = pos.x() + bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb - bw - 1);
            QwtPainter::drawLine(painter, aux, lb - bw2, aux, pos.y() + len);
            break;
        case TopScale:
            aux = pos.y() - bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, lb - bw2, aux);
            QwtPainter::drawLine(painter, rb, aux, pos.x() + len, aux);
            break;
        case BottomScale:
            aux = pos.y() + bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, lb - bw2, aux);
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
