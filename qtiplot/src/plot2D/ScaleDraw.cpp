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
#include "Graph.h"
#include "MultiLayer.h"
#include <MyParser.h>
#include "ScaleEngine.h"

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

ScaleDraw::ScaleDraw(Graph *plot, const QString& formula):
	d_plot(plot),
	d_type(Numeric),
	d_numeric_format(Automatic),
	d_fmt('g'),
	d_prec(4),
	d_formula(formula),
	d_majTicks(Out),
	d_minTicks(Out),
	d_selected(false),
	d_name_format(ShortName),
	d_date_time_origin(QDateTime::currentDateTime()),
	d_format_info("YYYY-MM-DDTHH:MM:SS"),
	d_text_labels(QStringList())
{}

ScaleDraw::ScaleDraw(Graph *plot, const QStringList& labels, const QString& format, ScaleType type):
	d_plot(plot),
	d_type(type),
	d_numeric_format(Automatic),
	d_fmt('g'),
	d_prec(4),
	d_formula(""),
	d_majTicks(Out),
	d_minTicks(Out),
	d_selected(false),
	d_name_format(ShortName),
	d_date_time_origin(QDateTime::currentDateTime()),
	d_format_info(format),
	d_text_labels(labels)
{}

ScaleDraw::ScaleDraw(Graph *plot, ScaleDraw* sd):
	d_plot(plot)
{
	d_type = sd->d_type;
	d_numeric_format = sd->d_numeric_format;
	d_fmt = sd->d_fmt;
	d_prec = sd->d_prec;
	d_formula = sd->d_formula;
	d_majTicks = sd->d_majTicks;
	d_minTicks = sd->d_minTicks;
	d_selected = sd->d_selected;
	d_name_format = sd->d_name_format;
	d_date_time_origin = sd->d_date_time_origin;
	d_format_info = sd->d_format_info;
	d_text_labels = sd->d_text_labels;

	setLabelAlignment(sd->labelAlignment());
	setLabelRotation(sd->labelRotation());
}

QwtText ScaleDraw::label(double value) const
{
	switch (d_type){
		case Numeric:
		{
            QLocale locale = d_plot->locale();
			if (d_plot->parent())
				locale = d_plot->multiLayer()->locale();
			if (d_numeric_format == Superscripts){
				QString txt = locale.toString(transformValue(value), 'e', d_prec);
				QStringList list = txt.split("e", QString::SkipEmptyParts);
				if (list[0].toDouble() == 0.0)
					return QString("0");

				QString s = list[1];
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

			} else if (d_numeric_format == Engineering){
				QString eng_suff;
				double new_value = value;

				if(fabs(new_value) >= 1e18){
					eng_suff = 'E';
					new_value /= 1e18;
				} else if(fabs(new_value) >= 1e15){
					eng_suff = 'P';
					new_value /= 1e15;
				} else if(fabs(new_value) >= 1e12){
					eng_suff = 'T';
					new_value /= 1e12;
				} else if(fabs(new_value) >= 1e9){
					eng_suff = 'G';
					new_value /= 1e9;
				} else if(fabs(new_value) >= 1e6){
					eng_suff = 'M';
					new_value /= 1e6;
				} else if(fabs(new_value) >= 1e3){
					eng_suff = 'k';
					new_value /= 1e3;
				} else if(fabs(new_value) >= 1){
					eng_suff = "";
					new_value /= 1.0;
				} else if(fabs(new_value) >= 1e-3){
					eng_suff = 'm';
					new_value /= 1e-3;
				} else if(fabs(new_value) >= 1e-6){
					eng_suff = 'µ';
					new_value /= 1e-6;
				} else if(fabs(new_value) >= 1e-9){
					eng_suff = 'n';
					new_value /= 1e-9;
				} else if(fabs(new_value) >= 1e-12){
					eng_suff = 'p';
					new_value /= 1e-12;
				} else if(fabs(new_value) >= 1e-15){
					eng_suff = 'f';
					new_value /= 1e-15;
				} else {
					eng_suff = 'a';
					new_value /= 1e-18;
				}

				QString txt = locale.toString((new_value), 'f', d_prec);

				if(txt.contains(QRegExp("^0[\\.,]?0*$")))
					return QwtText("0");

				return QwtText(txt + eng_suff);
			} else
				return QwtText(locale.toString(transformValue(value), d_fmt, d_prec));
		break;
		}

		case Day:
		{
			int val = int(transformValue(value))%7;
			if (val < 0)
				val = 7 - abs(val);
			else if (val == 0)
				val = 7;

			QString day;
			switch(d_name_format){
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
		break;
		}

		case Month:
		{
			int val = int(transformValue(value))%12;
			if (val < 0)
				val = 12 - abs(val);
			else if (val == 0)
				val = 12;

			QString day;
			switch(d_name_format){
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
		break;
		}

		case Time:
		{
			QTime time = d_date_time_origin.time().addMSecs((int)value);
			if (d_format_info == "M")
				return QwtText(QString::number(60*time.hour() + time.minute()));
			else if (d_format_info == "S")
				return QwtText(QString::number(3600*time.hour() + 60*time.minute() + time.second()));

			return QwtText(time.toString(d_format_info));
		break;
		}

		case Date:
            return QwtText(d_date_time_origin.addSecs((int)value).toString(d_format_info));
		break;

		case ColHeader:
		case Text:
		{
			const QwtScaleDiv scDiv = scaleDiv();
			if (!scDiv.contains (value))
				return QwtText();

			QwtValueList ticks = scDiv.ticks (QwtScaleDiv::MajorTick);

			double break_offset = 0;
			ScaleEngine *se = (ScaleEngine *)d_plot->axisScaleEngine(axis());
			bool inverted = se->testAttribute(QwtScaleEngine::Inverted);
			if(se->hasBreak()){
			    double lb = se->axisBreakLeft();
			    double rb = se->axisBreakRight();
                if(inverted){
                    if (value <= lb){
						int n_ticks = (int)ticks.count() - 1;
                        double val0 = ticks[0];
						double val1 = ticks[n_ticks];
                        for (int i = 1; i < n_ticks; i++){
                            double aux = ticks[i];
                            if(aux >= rb && val0 > aux){
                                val0 = aux;
								continue;
                            }
							if(aux <= lb && val1 < aux)
                                val1 = aux;
                        }
						break_offset = fabs(val1 - val0);
                    }
				} else {
                    if (value >= rb){
                        double val0 = ticks[0];
                        for (int i = 1; i < (int)ticks.count(); i++){
                            double val = ticks[i];
                            if(val0 <= lb && val >= rb){
                                break_offset = fabs(val - val0);
                                break;
                            }
                            val0 = val;
                        }
                    }
			    }
			}

        	double step = ticks[1] - ticks[0];
        	int index = int(ticks[0] + step*ticks.indexOf(value) - 1);
            int offset = abs((int)floor(break_offset/step));
            if (offset)
                offset--;
            if (step > 0)
                index += offset;
            else
                index -= offset;
			if (index >= 0 && index < (int)d_text_labels.count())
        		return QwtText(d_text_labels[index]);
			else
				return QwtText();
		break;
		}
	}
	return QwtText();
}

void ScaleDraw::drawLabel(QPainter *painter, double value) const
{
    if (!d_plot)
        return;

    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
	if (sc_engine->hasBreak()){
		bool invertedScale = sc_engine->testAttribute(QwtScaleEngine::Inverted);
		if (invertedScale && sc_engine->axisBreakRight() == value)
        	return;
		if (!invertedScale && sc_engine->axisBreakLeft() == value)
        	return;
	}

	QwtText lbl = tickLabel(painter->font(), value);
    if ( lbl.isEmpty() )
        return;

    QPoint pos = labelPosition(value);

    QSize labelSize = lbl.textSize(painter->font());
    if ( labelSize.height() % 2 )
        labelSize.setHeight(labelSize.height() + 1);

    const QwtMetricsMap metricsMap = QwtPainter::metricsMap();
    QwtPainter::resetMetricsMap();

    labelSize = metricsMap.layoutToDevice(labelSize);
    pos = metricsMap.layoutToDevice(pos);

    painter->save();
    painter->setMatrix(labelMatrix( pos, labelSize), true);

	if (d_selected)
        lbl.setBackgroundPen(QPen(Qt::blue));
    else
        lbl.setBackgroundPen(QPen(Qt::NoPen));

	lbl.setRenderFlags(labelAlignment());

    lbl.draw (painter, QRect(QPoint(0, 0), labelSize) );

    QwtPainter::setMetricsMap(metricsMap); // restore metrics map

    painter->restore();
}

double ScaleDraw::transformValue(double value) const
{
	if (!d_formula.isEmpty()){
		double lbl=0.0;
		try{
			MyParser parser;
			if (d_formula.contains("x"))
				parser.DefineVar("x", &value);
			else if (d_formula.contains("y"))
				parser.DefineVar("y", &value);

			parser.SetExpr(d_formula.ascii());
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
    d_type = Numeric;

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
		case Engineering:
			d_fmt = 'f';
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

  \sa setNumericFormat()
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
		bool invertedScale = sc_engine->testAttribute(QwtScaleEngine::Inverted);
		if (align == BottomScale || align == LeftScale){
			if (invertedScale && sc_engine->axisBreakRight() == value)
            	return;
			if (!invertedScale && sc_engine->axisBreakLeft() == value)
				return;
		} else {
			if (!invertedScale && sc_engine->axisBreakRight() == value)
				return;
			if (invertedScale && sc_engine->axisBreakLeft() == value)
            	return;
		}
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
            QwtPainter::drawLine(painter, pos.x(), lval, pos.x() - len, lval + len);
			QwtPainter::drawLine(painter, pos.x(), rval, pos.x() - len, rval + len);
        break;
        case RightScale:
            QwtPainter::drawLine(painter, pos.x(), lval, pos.x() + len, lval - len);
			QwtPainter::drawLine(painter, pos.x(), rval, pos.x() + len, rval - len);
        break;
        case BottomScale:
            QwtPainter::drawLine(painter, lval, pos.y(), lval - len, pos.y() + len);
			QwtPainter::drawLine(painter, rval, pos.y(), rval - len, pos.y() + len);
        break;
        case TopScale:
            QwtPainter::drawLine(painter, lval, pos.y(), lval + len, pos.y() - len);
			QwtPainter::drawLine(painter, rval, pos.y(), rval + len, pos.y() - len);
        break;
    }

    QwtPainter::setMetricsMap(metricsMap); // restore metrics map
    painter->restore();
}

void ScaleDraw::drawBackbone(QPainter *painter) const
{
    ScaleEngine *sc_engine = (ScaleEngine *)d_plot->axisScaleEngine(axis());
    if (!sc_engine->hasBreak()){
    	if (d_plot->isPrinting()){
			const int len = length();
			QPoint pos = this->pos();
			switch(alignment()){
				case LeftScale:
					QwtPainter::drawLine(painter, pos.x(), pos.y(), pos.x(), pos.y() + len );
				break;
				case RightScale:
					QwtPainter::drawLine(painter, pos.x(), pos.y(), pos.x(), pos.y() + len);
				break;
				case TopScale:
					QwtPainter::drawLine(painter, pos.x(), pos.y(), pos.x() + len, pos.y());
				break;
				case BottomScale:
					QwtPainter::drawLine(painter, pos.x(), pos.y(), pos.x() + len, pos.y());
				break;
			}
    	} else
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

	const int start = scaleMap.transform(sc_engine->axisBreakLeft());
	const int end = scaleMap.transform(sc_engine->axisBreakRight());
    int lb = start, rb = end;
	if (sc_engine->testAttribute(QwtScaleEngine::Inverted)){
		lb = end;
		rb = start;
	}

	const int bw = painter->pen().width();
    const int bw2 = bw / 2;
    const int len = length() - 1;
    int aux;
	switch(alignment())
    {
        case LeftScale:
            aux = pos.x() - bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb);
            QwtPainter::drawLine(painter, aux, lb + bw, aux, pos.y() + len);
            break;
        case RightScale:
            aux = pos.x() + bw2;
            QwtPainter::drawLine(painter, aux, pos.y(), aux, rb - bw - 1);
            QwtPainter::drawLine(painter, aux, lb - bw2, aux, pos.y() + len);
            break;
        case TopScale:
            aux = pos.y() - bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, lb - bw2, aux);
            QwtPainter::drawLine(painter, rb + bw, aux, pos.x() + len, aux);
            break;
        case BottomScale:
            aux = pos.y() + bw2;
            QwtPainter::drawLine(painter, pos.x(), aux, lb - bw, aux);
            QwtPainter::drawLine(painter, rb, aux, pos.x() + len, aux);
            break;
    }
}

void ScaleDraw::setDayFormat(NameFormat format)
{
	d_type = Day;
	d_name_format = format;
}

void ScaleDraw::setMonthFormat(NameFormat format)
{
	d_type = Month;
	d_name_format = format;
}

void ScaleDraw::setTimeFormat(const QTime& t, const QString& format)
{
	d_type = Time;
	d_format_info = format;
	d_date_time_origin.setTime(t);
}

void ScaleDraw::setDateFormat(const QDateTime& d, const QString& format)
{
	d_type = Date;
	d_format_info = format;
	d_date_time_origin = d;
}

QString ScaleDraw::formatString()
{
	if (d_type == Time)
		return d_date_time_origin.time().toString() + ";" + d_format_info;
	else if (d_type == Date)
		return d_date_time_origin.toString(Qt::ISODate) + ";" + d_format_info;

	return d_format_info;
}
