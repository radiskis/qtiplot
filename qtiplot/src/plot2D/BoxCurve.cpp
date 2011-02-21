/***************************************************************************
    File                 : BoxCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Box curve

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
#include "BoxCurve.h"
#include "Graph.h"
#include <QPainter>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics.h>

#include <qwt_painter.h>

BoxCurve::BoxCurve(Table *t, const QString& name, int startRow, int endRow):
	DataCurve(t, QString(), name, startRow, endRow),
	min_style(QwtSymbol::XCross),
	max_style(QwtSymbol::XCross),
	mean_style(QwtSymbol::Rect),
	p99_style(QwtSymbol::NoSymbol),
	p1_style(QwtSymbol::NoSymbol),
	b_coeff(75.0),
	w_coeff(95.0),
	b_range(r25_75),
	w_range(r5_95),
	b_style(Rect),
	b_width(80),
	d_box_labels(false),
	d_whiskers_labels(false),
	d_labels_display(Percentage)
{
	d_labels_x_offset = 100;
	d_labels_y_offset = 0;

	setType(Graph::Box);
	setPlotStyle(Graph::Box);
    setStyle(QwtPlotCurve::UserCurve);
}

void BoxCurve::copy(BoxCurve *b)
{
	DataCurve::clone(b);

	mean_style = b->mean_style;
	max_style = b->max_style;
	min_style = b->min_style;
	p99_style = b->p99_style;
	p1_style = b->p1_style;

	b_style = b->b_style;
	b_coeff = b->b_coeff;
	b_range = b->b_range;
	w_range = b->w_range;
	w_coeff = b->w_coeff;
	b_width = b->b_width;

	d_box_labels = b->hasBoxLabels();
	d_whiskers_labels = b->hasWhiskerLabels();
	d_labels_display = b->labelsDisplayPolicy();
	updateLabels();
}

void BoxCurve::draw(QPainter *painter,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if ( !painter || dataSize() <= 0 )
		return;

	int size = dataSize();
	if (to < 0)
		to = size - 1;

	painter->save();
	QPen pen = QwtPainter::scaledPen(this->pen());
	pen.setCapStyle(Qt::FlatCap);
	painter->setPen(pen);

	double *dat = (double *)malloc(size*sizeof(double));
	if (!dat)
		return;

	for (int i = from; i<= to; i++)
		dat[i] = y(i);

	drawBox(painter, xMap, yMap, dat, size);
	drawSymbols(painter, xMap, yMap, dat, size);

	painter->restore();
	free(dat);
}

void BoxCurve::drawBox(QPainter *painter, const QwtScaleMap &xMap,
		const QwtScaleMap &yMap, double *dat, int size) const
{
	const int px = xMap.transform(x(0));
	const int px_min = xMap.transform(x(0) - 0.4);
	const int px_max = xMap.transform(x(0) + 0.4);
	const int box_width = 1 + (px_max - px_min)*b_width/100;
	const int hbw = box_width/2;
	const int median = yMap.transform(gsl_stats_median_from_sorted_data (dat, 1, size));
	int b_lowerq, b_upperq;
	double sd = 0.0, se = 0.0, mean = 0.0;
	if(w_range == SD || w_range == SE || b_range == SD || b_range == SE)
	{
		sd = gsl_stats_sd(dat, 1, size);
		se = sd/sqrt((double)size);
		mean = gsl_stats_mean(dat, 1, size);
	}

	if(b_range == SD)
	{
		b_lowerq = yMap.transform(mean - sd*b_coeff);
		b_upperq = yMap.transform(mean + sd*b_coeff);
	}
	else if(b_range == SE)
	{
		b_lowerq = yMap.transform(mean - se*b_coeff);
		b_upperq = yMap.transform(mean + se*b_coeff);
	}
	else
	{
		b_lowerq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 1-0.01*b_coeff));
		b_upperq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.01*b_coeff));
	}

	//draw box
	if (b_style == Rect)
	{
		const QRect r = QRect(px - hbw, b_upperq, box_width, b_lowerq - b_upperq + 1);
		painter->fillRect(r, QwtPlotCurve::brush());
		painter->drawRect(r);
	}
	else if (b_style == Diamond)
	{
		QPolygon pa(4);
		pa[0] = QPoint(px, b_upperq);
		pa[1] = QPoint(px + hbw, median);
		pa[2] = QPoint(px, b_lowerq);
		pa[3] = QPoint(px - hbw, median);

		painter->setBrush(QwtPlotCurve::brush());
		painter->drawPolygon(pa);
	}
	else if (b_style == WindBox)
	{
		const int lowerq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.25));
		const int upperq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.75));
		QPolygon pa(8);
		pa[0] = QPoint(px + hbw, b_upperq);
		pa[1] = QPoint(int(px + 0.4*box_width), upperq);
		pa[2] = QPoint(int(px + 0.4*box_width), lowerq);
		pa[3] = QPoint(px + hbw, b_lowerq);
		pa[4] = QPoint(px - hbw, b_lowerq);
		pa[5] = QPoint(int(px - 0.4*box_width), lowerq);
		pa[6] = QPoint(int(px - 0.4*box_width), upperq);
		pa[7] = QPoint(px - hbw, b_upperq);

		painter->setBrush(QwtPlotCurve::brush());
		painter->drawPolygon(pa);
	}
	else if (b_style == Notch)
	{
		int j = (int)ceil(0.5*(size - 1.96*sqrt((double)size)));
		int k = (int)ceil(0.5*(size + 1.96*sqrt((double)size)));
		const int lowerCI = yMap.transform(dat[j]);
		const int upperCI = yMap.transform(dat[k]);

		QPolygon pa(10);
		pa[0] = QPoint(px + hbw, b_upperq);
		pa[1] = QPoint(px + hbw, upperCI);
		pa[2] = QPoint(int(px + 0.25*hbw), median);
		pa[3] = QPoint(px + hbw, lowerCI);
		pa[4] = QPoint(px + hbw, b_lowerq);
		pa[5] = QPoint(px - hbw, b_lowerq);
		pa[6] = QPoint(px - hbw, lowerCI);
		pa[7] = QPoint(int(px - 0.25*hbw), median);
		pa[8] = QPoint(px - hbw, upperCI);
		pa[9] = QPoint(px - hbw, b_upperq);

		painter->setBrush(QwtPlotCurve::brush());
		painter->drawPolygon(pa);
	}

	if (w_range)
	{//draw whiskers
		const int l = int(0.1*box_width);
		int w_upperq, w_lowerq;
		if(w_range == SD)
		{
			w_lowerq = yMap.transform(mean - sd*w_coeff);
			w_upperq = yMap.transform(mean + sd*w_coeff);
		}
		else if(w_range == SE)
		{
			w_lowerq = yMap.transform(mean - se*w_coeff);
			w_upperq = yMap.transform(mean + se*w_coeff);
		}
		else
		{
			w_lowerq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 1-0.01*w_coeff));
			w_upperq = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.01*w_coeff));
		}

		painter->drawLine(px - l, w_lowerq, px + l, w_lowerq);
		painter->drawLine(px - l, w_upperq, px + l, w_upperq);

		if (b_style){
			if (w_upperq != b_upperq)
				painter->drawLine(px, w_upperq, px, b_upperq);
			if (w_lowerq != b_lowerq)
				painter->drawLine(px, w_lowerq, px, b_lowerq + 1);
		} else
			painter->drawLine(px, w_upperq, px, w_lowerq);
	}

	//draw median line
	if (b_style == Notch || b_style == NoBox)
		return;
	if (b_style == WindBox)
		painter->drawLine(int(px - 0.4*box_width), median, int(px + 0.4*box_width), median);
	else
		painter->drawLine(px - hbw, median, px + hbw, median);
}

void BoxCurve::drawSymbols(QPainter *painter, const QwtScaleMap &xMap,
		const QwtScaleMap &yMap, double *dat, int size) const
{
	const int px = xMap.transform(x(0));

	QwtSymbol s = this->symbol();
	s.setPen(QwtPainter::scaledPen(s.pen()));

	if (min_style != QwtSymbol::NoSymbol)
	{
		const int py_min = yMap.transform(y(0));
		s.setStyle(min_style);
		s.draw(painter, px, py_min);
	}
	if (max_style != QwtSymbol::NoSymbol)
	{
		const int py_max = yMap.transform(y(size - 1));
		s.setStyle(max_style);
		s.draw(painter, px, py_max);
	}
	if (p1_style != QwtSymbol::NoSymbol)
	{
		const int p1 = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.01));
		s.setStyle(p1_style);
		s.draw(painter, px, p1);
	}
	if (p99_style != QwtSymbol::NoSymbol)
	{
		const int p99 = yMap.transform(gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.99));
		s.setStyle(p99_style);
		s.draw(painter, px, p99);
	}
	if (mean_style != QwtSymbol::NoSymbol)
	{
		const int mean = yMap.transform(gsl_stats_mean(dat, 1, size));
		s.setStyle(mean_style);
		s.draw(painter, px, mean);
	}
}

void BoxCurve::setBoxStyle(int style)
{
	if (b_style == style)
		return;

	b_style = style;
}

void BoxCurve::setBoxRange(int type, double coeff)
{
	if (b_style == WindBox)
	{
		b_range = r10_90;
		b_coeff = 90.0;
		return;
	}

	b_range = type;

	if (b_range == r25_75)
		b_coeff = 75.0;
	else if (b_range == r10_90)
		b_coeff = 90.0;
	else if (b_range == r5_95)
		b_coeff = 95.0;
	else if (b_range == r1_99)
		b_coeff = 99.0;
	else if (b_range == MinMax)
		b_coeff = 100.0;
	else
		b_coeff = coeff;

	updateLabels();
}

void BoxCurve::setWhiskersRange(int type, double coeff)
{
	w_range = type;

	if (w_range == r25_75)
		w_coeff = 75.0;
	else if (w_range == r10_90)
		w_coeff = 90.0;
	else if (w_range == r5_95)
		w_coeff = 95.0;
	else if (w_range == r1_99)
		w_coeff = 99.0;
	else if (w_range == MinMax)
		w_coeff = 100.0;
	else
		w_coeff = coeff;

	updateLabels();
}

QwtDoubleRect BoxCurve::boundingRect() const
{
	QwtDoubleRect rect = QwtPlotCurve::boundingRect();

	double dy=0.2*(rect.bottom()-rect.top());
	rect.setTop(rect.top()-dy);
	rect.setBottom(rect.bottom()+dy);

	rect.setLeft(rect.left()-0.5);
	rect.setRight(rect.right()+0.5);
	return rect;
}

void BoxCurve::loadData()
{
	QVector<double> Y(abs(d_end_row - d_start_row) + 1);
    int ycol = d_table->colIndex(title().text());
	int size = 0;
	for (int i = d_start_row; i <= d_end_row; i++){
		QString s = d_table->text(i, ycol);
        if (!s.isEmpty()){
            bool valid_data = true;
            Y[size] = ((Graph *)plot())->locale().toDouble(s, &valid_data);
            if (valid_data)
                size++;
        }
	}

	if (size>0){
		Y.resize(size);
		gsl_sort (Y.data(), 1, size);
        setData(QwtSingleArrayData(this->x(0), Y, size));
		if (d_show_labels)
			loadLabels();
	} else
		remove();
}

QString BoxCurve::statistics()
{
	if (!plot())
		return QString();

	int size = dataSize();
	double *dat = (double *)malloc(size*sizeof(double));
	if (!dat)
		return QString();

	for (int i = 0; i < size; i++)
		dat[i] = y(i);

	double median = gsl_stats_median_from_sorted_data (dat, 1, size);
	double d1 = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.1);
	double d9 = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.9);
	double q1 = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.25);
	double q3 = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.75);

	QLocale locale = plot()->locale();

	QString s = QObject::tr("Min") + " = " + locale.toString(dat[0], 'f', 2) + "\n";
	s += QObject::tr("D1 (1st decile)") + " = " + locale.toString(d1, 'f', 2) + "\n";
	s += QObject::tr("Q1 (1st quartile)") + " = " + locale.toString(q1, 'f', 2) + "\n";
	s += QObject::tr("Median") + " = " + locale.toString(median, 'f', 2) + "\n";
	s += QObject::tr("Q3 (3rd quartile)") + " = " + locale.toString(q3, 'f', 2) + "\n";
	s += QObject::tr("D9 (9th decile)") + " = " + locale.toString(d9, 'f', 2) + "\n";
	s += QObject::tr("Max") + " = " + QString::number(dat[size - 1]) + "\n";
	s += QObject::tr("Size") + " = " + QString::number(size) + "\n";

	free (dat);
	return s;
}

double BoxCurve::quantile(double f)
{
	if (f < 0 || f > 1)
		return 0.0;

	int size = dataSize();
	double *dat = (double *)malloc(size*sizeof(double));
	if (!dat)
		return 0.0;

	for (int i = 0; i< size; i++)
		dat[i] = y(i);

	double q = gsl_stats_quantile_from_sorted_data (dat, 1, size, f);
	free (dat);

	return q;
}

void BoxCurve::setLabelsDisplayPolicy(const LabelsDisplayPolicy& policy)
{
	if (d_labels_display == policy)
		return;

	d_labels_display = policy;
	updateLabels();
}

void BoxCurve::showBoxLabels(bool on)
{
	if (d_box_labels == on)
		return;

	d_box_labels = on;
	if (!d_show_labels)
		loadLabels();
	updateLabels();
}

void BoxCurve::showWhiskerLabels(bool on)
{
	if (d_whiskers_labels == on)
		return;

	d_whiskers_labels = on;
	if (!d_show_labels)
		loadLabels();
	updateLabels();
}

QString BoxCurve::labelText(int index, double val)
{
	if ((!w_range || !d_whiskers_labels) && (index == 0 || index == 4))
		return QString();

	if (!d_box_labels && index > 0 && index < 4)
		return QString();

	QString s;
	switch(d_labels_display){
		case Percentage:
			s = labelPercentage(index);
		break;

		case Value:
			s = plot()->locale().toString(val, 'f', 2);
		break;

		case PercentageValue:
			s = labelPercentage(index) + " (" + plot()->locale().toString(val, 'f', 2) + ")";
		break;

		case ValuePercentage:
			s = plot()->locale().toString(val, 'f', 2) + " (" + labelPercentage(index) + ")";
		break;
	}
	return s;
}

QString BoxCurve::labelPercentage(int index)
{
	QString s;
	switch(index){
		case 2:
			s = "50%";
		break;

		case 0:
			if (w_range == SD)
				s = "-SD";
			else if (w_range == SE)
				s = "-SE";
			else
				s = QString::number(100 - w_coeff) + "%";
		break;

		case 1:
			if (b_range == SD)
				s = "-SD";
			else if (b_range == SE)
				s = "-SE";
			else
				s = QString::number(100 - b_coeff) + "%";
		break;

		case 4:
			if (w_range == SD)
				s = "SD";
			else if (w_range == SE)
				s = "SE";
			else
				s = QString::number(w_coeff) + "%";
		break;

		case 3:
			if (b_range == SD)
				s = "SD";
			else if (b_range == SE)
				s = "SE";
			else
				s = QString::number(b_coeff) + "%";
		break;
	}
	return s;
}

double * BoxCurve::statisticValues()
{
	int size = this->dataSize();
	if (!size)
		return 0;

	double *dat = (double *)malloc(size*sizeof(double));
	if (!dat)
		return 0;

	for (int i = 0; i < size; i++)
		dat[i] = y(i);

	double b_lowerq, b_upperq;
	double sd = 0.0, se = 0.0, mean = 0.0;
	if(w_range == SD || w_range == SE || b_range == SD || b_range == SE)
	{
		sd = gsl_stats_sd(dat, 1, size);
		se = sd/sqrt((double)size);
		mean = gsl_stats_mean(dat, 1, size);
	}

	if(b_range == SD)
	{
		b_lowerq = mean - sd*b_coeff;
		b_upperq = mean + sd*b_coeff;
	}
	else if(b_range == SE)
	{
		b_lowerq = mean - se*b_coeff;
		b_upperq = mean + se*b_coeff;
	}
	else
	{
		b_lowerq = gsl_stats_quantile_from_sorted_data (dat, 1, size, 1 - 0.01*b_coeff);
		b_upperq = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.01*b_coeff);
	}

	double w_upperq, w_lowerq;
	if(w_range == SD){
		w_lowerq = mean - sd*w_coeff;
		w_upperq = mean + sd*w_coeff;
	} else if(w_range == SE){
		w_lowerq = mean - se*w_coeff;
		w_upperq = mean + se*w_coeff;
	} else {
		w_lowerq = gsl_stats_quantile_from_sorted_data (dat, 1, size, 1 - 0.01*w_coeff);
		w_upperq = gsl_stats_quantile_from_sorted_data (dat, 1, size, 0.01*w_coeff);
	}

	double *v = new double[5];
	v[0] = w_lowerq;
	v[1] = b_lowerq;
	v[2] = gsl_stats_median_from_sorted_data (dat, 1, size);
	v[3] = b_upperq;
	v[4] = w_upperq;

	free (dat);
	return v;
}

void BoxCurve::loadLabels()
{
	clearLabels();

	double *v = statisticValues();
	if (!v)
		return;

	for (int i = 0; i < 5; i++)
		createLabel(v[i]);

	delete[] v;
	d_show_labels = true;
}

void BoxCurve::createLabel(double val)
{
	QwtPlot *d_plot = plot();
	if (!d_plot)
		return;

	int index = d_labels_list.size();
	PlotMarker *m = new PlotMarker(index, d_labels_angle);

	QwtText t = labelText(index, val);
	t.setColor(d_labels_color);
	t.setFont(d_labels_font);
	if (d_white_out_labels)
		t.setBackgroundBrush(QBrush(Qt::white));
	else
		t.setBackgroundBrush(QBrush(Qt::transparent));
	m->setLabel(t);

	int x_axis = xAxis();
	int y_axis = yAxis();
	m->setAxis(x_axis, y_axis);

	const double px_min = d_plot->transform(x_axis, x(0) - 0.4);
	const double px_max = d_plot->transform(x_axis, x(0) + 0.4);
	const double box_width = 1 + (px_max - px_min)*b_width/100.0;
	const double hbw = 0.5*box_width;
	const double l = 0.1*box_width;

	QSize size = t.textSize();
	double dx = d_labels_x_offset*0.01*size.height();
	double dy = -((d_labels_y_offset*0.01 + 0.5)*size.height());
	double x2 = d_plot->transform(x_axis, x(0)) + dx;
	if (index > 0 && index < 4 && b_style != NoBox){
		if (((index == 1 || index == 3) && b_style != Diamond))
			x2 += hbw;
		else if (index == 2){
			switch(b_style){
				case Notch:
					x2 += 0.3*hbw;
				break;
				case WindBox:
					x2 += 0.8*hbw;
				break;
				default:
					x2 += hbw;
				break;
			}
		}
	} else
		x2 += l;

	int y2 = d_plot->transform(y_axis, val) + dy;
	switch(d_labels_align){
		case Qt::AlignLeft:
		break;
		case Qt::AlignHCenter:
			x2 -= size.width()/2;
		break;
		case Qt::AlignRight:
			x2 -= size.width();
		break;
	}
	m->setXValue(d_plot->invTransform(x_axis, x2));
	m->setYValue(d_plot->invTransform(y_axis, y2));
	m->attach(d_plot);
	d_labels_list << m;
}

void BoxCurve::updateLabels(bool updateText)
{
	if (!d_show_labels || d_labels_list.isEmpty())
		return;

	QwtPlot *d_plot = plot();
	if (!d_plot)
		return;

	double *v = statisticValues();
	if (!v)
		return;

	int x_axis = xAxis();
	int y_axis = yAxis();
	const double px_min = d_plot->transform(x_axis, x(0) - 0.4);
	const double px_max = d_plot->transform(x_axis, x(0) + 0.4);
	const double box_width = 1 + (px_max - px_min)*b_width/100.0;
	const double hbw = 0.5*box_width;
	const double l = 0.1*box_width;

	QLocale locale = d_plot->locale();

	foreach(PlotMarker *m, d_labels_list){
		int index = m->index();
		double val = v[index];
		if (updateText){
			QwtText t = m->label();
			t.setText(labelText(index, val));
			m->setLabel(t);
		}
		QSize size = m->label().textSize();
		double dx = d_labels_x_offset*0.01*size.height();
		double dy = -((d_labels_y_offset*0.01 + 0.5)*size.height());
		double x2 = d_plot->transform(x_axis, x(0)) + dx;
		if (index > 0 && index < 4 && b_style != NoBox){
			if (((index == 1 || index == 3) && b_style != Diamond))
				x2 += hbw;
			else if (index == 2){
				switch(b_style){
					case Notch:
						x2 += 0.3*hbw;
					break;
					case WindBox:
						x2 += 0.8*hbw;
					break;
					default:
						x2 += hbw;
					break;
				}
			}
		} else
			x2 += l;

		int y2 = d_plot->transform(y_axis, val) + dy;
		switch(d_labels_align){
			case Qt::AlignLeft:
			break;
			case Qt::AlignHCenter:
				x2 -= size.width()/2;
			break;
			case Qt::AlignRight:
				x2 -= size.width();
			break;
		}
		m->setXValue(d_plot->invTransform(x_axis, x2));
		m->setYValue(d_plot->invTransform(y_axis, y2));
	}

	delete[] v;
}
