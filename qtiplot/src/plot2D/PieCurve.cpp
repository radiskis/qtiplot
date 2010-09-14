/***************************************************************************
	File                 : PieCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004 - 2010 by Ion Vasilie
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Pie plot class

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
#include "PieCurve.h"
#include "MultiLayer.h"
#include <Table.h>

#include <ColorBox.h>
#include <PenStyleBox.h>

#include <QPaintDevice>
#include <QPainter>
#include <QPainterPath>
#include <QVarLengthArray>

#include <qwt_plot_canvas.h>
#include <qwt_painter.h>

PieCurve::PieCurve(Table *t, const QString& name, int startRow, int endRow):
	DataCurve(t, QString(), name, startRow, endRow),
	d_pie_ray(50),
	d_first_color(0),
	d_start_azimuth(270),
	d_view_angle(33),
	d_thickness(33),
	d_horizontal_offset(0),
	d_edge_dist(25),
	d_counter_clockwise(false),
	d_auto_labeling(true),
	d_values(false),
	d_percentages(true),
	d_categories(false),
	d_fixed_labels_pos(true)
{
	setPen(QPen(QColor(Qt::black), 1, Qt::SolidLine));
	setBrush(QBrush(Qt::SolidPattern));
    setStyle(QwtPlotCurve::UserCurve);
	setType(Graph::Pie);
	setPlotStyle(Graph::Pie);
	d_table_rows = QVarLengthArray<int>(0);
}

void PieCurve::clone(PieCurve* c)
{
    if (!c)
        return;

    d_pie_ray = c->radius();
	d_first_color = c->firstColor();
	d_start_azimuth = c->startAzimuth();
	d_view_angle = c->viewAngle();
	d_thickness = c->thickness();
	d_horizontal_offset = c->horizontalOffset();
	d_edge_dist = c->labelsEdgeDistance();
	d_counter_clockwise = c->counterClockwise();
	d_auto_labeling = c->labelsAutoFormat();
	d_values = c->labelsValuesFormat();
	d_percentages = c->labelsPercentagesFormat();
	d_categories = c->labelCategories();
	d_fixed_labels_pos = c->fixedLabelsPosition();

	d_table_rows = c->d_table_rows;

	QList <PieLabel *> lst = c->labelsList();
	foreach(PieLabel *t, lst){
		PieLabel *nl = addLabel(t, true);
		if (nl && t->isHidden())
            nl->hide();
	}
}

void PieCurve::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
    int size = dataSize();
	if ( !painter ||  size <= 0 )
		return;

	if (to < 0)
		to = size - 1;

    if (size > 1)
        drawSlices(painter, xMap, yMap, from, to);
    else
        drawDisk(painter, xMap, yMap);
}

void PieCurve::drawDisk(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap) const
{
	QwtPlotCanvas* canvas = plot()->canvas();
	QPoint center = QPoint(canvas->x() + canvas->width()/2, canvas->y() + canvas->height()/2);

	// calculate resolution factors
	double x_factor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	double y_factor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();

	const double width = canvas->width()*x_factor;
	const double height = canvas->height()*y_factor;
	const double x_center = center.x()*x_factor + d_horizontal_offset*0.01*width;
	const double y_center = center.y()*y_factor;

	const double ray_x = d_pie_ray*0.005*qMin(width, height);
	const double view_angle_rad = d_view_angle*M_PI/180.0;
	const double ray_y = ray_x*sin(view_angle_rad);
	const double thick = 0.01*d_thickness*ray_x*cos(view_angle_rad);

	QRectF pieRect;
	pieRect.setX(x_center - ray_x);
	pieRect.setY(y_center - ray_y);
	pieRect.setWidth(2*ray_x);
	pieRect.setHeight(2*ray_y);

	QRectF pieRect2 = pieRect;
    pieRect2.translate(0, thick);

    painter->save();

    painter->setPen(QwtPainter::scaledPen(pen()));
    painter->setBrush(QBrush(color(0), QwtPlotCurve::brush().style()));

    QPointF start(x_center + ray_x, y_center);
    QPainterPath path(start);
    path.lineTo(start.x(), start.y() + thick);
    path.arcTo(pieRect2, 0, -180.0);
    QPointF aux = path.currentPosition();
    path.lineTo(aux.x(), aux.y() - thick);
    path.arcTo(pieRect, -180.0, 180.0);
    painter->drawPath(path);

    painter->drawEllipse(pieRect);

    if (d_texts_list.size() > 0){
        PieLabel* l = d_texts_list[0];
        if (l){
            QString s;
            if (d_auto_labeling){
				if (d_categories)
					s += QString::number(d_table_rows[0]) + "\n";

                if (d_values && d_percentages)
                    s += ((Graph *)plot())->locale().toString(y(0), 'g', 4) + " (100%)";
                else if (d_values)
                    s += ((Graph *)plot())->locale().toString(y(0), 'g', 4);
                else if (d_percentages)
                    s += "100%";
                l->setText(s);
				if (l->isHidden())
					l->show();
            } else
				l->setText(l->customText());

            if (d_fixed_labels_pos){
                double a_deg = d_start_azimuth + 180.0;
                if (a_deg > 360)
                    a_deg -= 360;
                double a_rad = a_deg*M_PI/180.0;
                double rx = ray_x*(1 + 0.01*d_edge_dist);
                const double x = x_center + rx*cos(a_rad);
                double ry = ray_y*(1 + 0.01*d_edge_dist);
                double y = y_center + ry*sin(a_rad);
                if (a_deg > 0 && a_deg < 180)
                    y += thick;

                double dx = xMap.invTransform(x - l->width()*x_factor/2);
                double dy = yMap.invTransform(y - l->height()*y_factor/2);
                l->setOriginCoord(dx, dy);
            }
        }
    }
    painter->restore();
}

void PieCurve::drawSlices(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	QwtPlotCanvas* canvas = plot()->canvas();
	QPoint center = QPoint(canvas->x() + canvas->width()/2, canvas->y() + canvas->height()/2);

	// calculate resolution factors
	double x_factor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	double y_factor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();

	const double width = canvas->width()*x_factor;
	const double height = canvas->height()*y_factor;
	const double x_center = center.x()*x_factor + d_horizontal_offset*0.01*width;
	const double y_center = center.y()*y_factor;

	const double ray_x = d_pie_ray*0.005*qMin(width, height);
	const double view_angle_rad = d_view_angle*M_PI/180.0;
	const double ray_y = ray_x*sin(view_angle_rad);
	const double thick = 0.01*d_thickness*ray_x*cos(view_angle_rad);

	QRectF pieRect;
	pieRect.setX(x_center - ray_x);
	pieRect.setY(y_center - ray_y);
	pieRect.setWidth(2*ray_x);
	pieRect.setHeight(2*ray_y);

	QRectF pieRect2 = pieRect;
    pieRect2.translate(0, thick);

	double sum = 0.0;
	for (int i = from; i <= to; i++)
		sum += y(i);

	const int sign = d_counter_clockwise ? 1 : -1;

	const int size = dataSize();
    double *start_angle = new double[size];
    double *end_angle = new double[size];
    double aux_angle = d_start_azimuth;
	for (int i = from; i <= to; i++){
	    double a = -sign*y(i)/sum*360.0;
		start_angle[i] = aux_angle;

		double end = aux_angle + a;
		if (end >= 360)
            end -= 360;
        else if (end < 0)
            end += 360;

		end_angle[i] = end;
		aux_angle = end;
	}

	int angle = (int)(5760 * d_start_azimuth/360.0);
	if (d_counter_clockwise)
		angle = (int)(5760 * (1 - d_start_azimuth/360.0));

	painter->save();

	QLocale locale = ((Graph *)plot())->multiLayer()->locale();
	for (int i = from; i <= to; i++){
		const double yi = y(i);
		const double q = yi/sum;
		const int value = (int)(q*5760);

		painter->setPen(QwtPainter::scaledPen(pen()));
		painter->setBrush(QBrush(color(i), QwtPlotCurve::brush().style()));

		double deg = q*360;
		double start_3D_view_angle = start_angle[i];
		double end_3D_view_angle = end_angle[i];
		if (d_counter_clockwise){
		    start_3D_view_angle = end_angle[i];
            end_3D_view_angle = start_angle[i];
		}

        bool draw3D = false;
        if (deg <= 180 && start_3D_view_angle >= 0 && start_3D_view_angle < 180){
            if ((end_3D_view_angle > 180 && end_3D_view_angle > start_3D_view_angle)){
                deg = 180 - start_3D_view_angle;
                end_3D_view_angle = 180.0;
            }
            draw3D = true;
		} else if (start_3D_view_angle >= 180 && end_3D_view_angle < start_3D_view_angle){
		    if (end_3D_view_angle > 180)
                end_3D_view_angle = 180;
            deg = end_3D_view_angle;
            start_3D_view_angle = 0;
            draw3D = true;
		} else if (deg > 180 && start_3D_view_angle >= 180){
            deg = 180;
            end_3D_view_angle = 180;
            start_3D_view_angle = 0;
            draw3D = true;
		}

		if (draw3D){
            double rad = start_3D_view_angle/180.0*M_PI;
			QPointF start(x_center + ray_x*cos(rad), y_center + ray_y*sin(rad));
			QPainterPath path(start);
			path.lineTo(start.x(), start.y() + thick);
			path.arcTo(pieRect2, -start_3D_view_angle, -deg);
        	QPointF aux = path.currentPosition();
        	path.lineTo(aux.x(), aux.y() - thick);
        	path.arcTo(pieRect, -end_3D_view_angle, deg);
        	painter->drawPath(path);
        } else {
            if (start_3D_view_angle >= 0 && start_3D_view_angle < 180){
                if (end_3D_view_angle > 180)
                    end_3D_view_angle = 0;

                double rad = start_3D_view_angle/180.0*M_PI;
                QPointF start(x_center + ray_x*cos(rad), y_center + ray_y*sin(rad));
                QPainterPath path(start);
                path.lineTo(start.x(), start.y() + thick);

                deg = 180 - start_3D_view_angle;
                path.arcTo(pieRect2, -start_3D_view_angle, -deg);
                QPointF aux = path.currentPosition();
                path.lineTo(aux.x(), aux.y() - thick);
                path.arcTo(pieRect, -180, deg);
                painter->drawPath(path);

                path.moveTo(QPointF(x_center + ray_x, y_center));
                aux = path.currentPosition();
                path.lineTo(aux.x(), aux.y() + thick);
                path.arcTo(pieRect2, 0, -end_3D_view_angle);
                aux = path.currentPosition();
                path.lineTo(aux.x(), aux.y() - thick);
                path.arcTo(pieRect, -end_3D_view_angle, end_3D_view_angle);
                painter->drawPath(path);
            }
        }

		painter->drawPie(pieRect, sign*angle, sign*value);
		angle += value;

		if (i >= d_texts_list.size())
			continue;

		PieLabel* l = d_texts_list[i];
		if (l){
			QString s;
			if (d_auto_labeling){
				if (d_categories)
					s += QString::number(d_table_rows[i]) + "\n";
				if (d_values && d_percentages)
					s += locale.toString(yi, 'g', 4) + " (" + locale.toString(q*100, 'g', 4) + "%)";
				else if (d_values)
					s += locale.toString(yi, 'g', 4);
				else if (d_percentages)
					s += locale.toString(q*100, 'g', 4) + "%";
                l->setText(s);
				if (l->isHidden())
					l->show();
			} else
				l->setText(l->customText());

            if (d_fixed_labels_pos){
                double a_deg = start_angle[i] - sign*q*180.0;
                if (a_deg > 360)
                    a_deg -= 360.0;
                double a_rad = a_deg*M_PI/180.0;

                double rx = ray_x*(1 + 0.01*d_edge_dist);
                const double x = x_center + rx*cos(a_rad);

                double ry = ray_y*(1 + 0.01*d_edge_dist);
                double y = y_center + ry*sin(a_rad);
                if (a_deg > 0 && a_deg < 180)
                    y += thick;

                double dx = xMap.invTransform(x - l->width()*x_factor/2);
                double dy = yMap.invTransform(y - l->height()*y_factor/2);
                l->setOriginCoord(dx, dy);
            }
		}
	}
	painter->restore();
	delete [] start_angle;
	delete [] end_angle;
}

QColor PieCurve::color(int i) const
{
	return ColorBox::color((d_first_color + i) % ColorBox::numPredefinedColors());
}

void PieCurve::setBrushStyle(const Qt::BrushStyle& style)
{
	QBrush br = QwtPlotCurve::brush();
	if (br.style() == style)
		return;

	br.setStyle(style);
	setBrush(br);
}

void PieCurve::loadData()
{
	Graph *d_plot = (Graph *)plot();
	QLocale locale = d_plot->multiLayer()->locale();
	QVarLengthArray<double> X(abs(d_end_row - d_start_row) + 1);
	d_table_rows.resize(abs(d_end_row - d_start_row) + 1);

	int size = 0;
	int ycol = d_table->colIndex(title().text());
	for (int i = d_start_row; i <= d_end_row; i++ ){
		QString xval = d_table->text(i, ycol);
		bool valid_data = true;
		if (!xval.isEmpty()){
            X[size] = locale.toDouble(xval, &valid_data);
            if (valid_data){
				d_table_rows[size] = i + 1;
                size++;
			}
		}
	}
	X.resize(size);
	d_table_rows.resize(size);
	setData(X.data(), X.data(), size);

	int labels = d_texts_list.size();
	//If there are no labels (initLabels() wasn't called yet) or if we have enough labels: do nothing!
	if(d_texts_list.isEmpty() || labels == size)
		return;

	//Else add new pie labels.
	for (int i = labels; i < size; i++ ){
		PieLabel* l = new PieLabel(d_plot, this);
		d_texts_list << l;
		l->hide();
	}
}

PieLabel* PieCurve::addLabel(PieLabel *l, bool clone)
{
	if (!l)
		return 0;

	Graph *g = (Graph *)plot();
	if (clone){
		PieLabel *newLabel = new PieLabel(g, this);
		newLabel->clone(l);
		newLabel->setCustomText(l->customText());
		d_texts_list << newLabel;
		if (l->text().isEmpty())
			newLabel->hide();
		return newLabel;
	} else {
		l->setPieCurve(this);
		d_texts_list << l;
		if (l->text().isEmpty())
			l->hide();
	}
	return l;
}

void PieCurve::initLabels()
{
	int dataPoints = dataSize();
	double sum = 0.0;
	for (int i = 0; i < dataPoints; i++)
		sum += y(i);

    Graph *d_plot = (Graph *)plot();
	QLocale locale = d_plot->multiLayer()->locale();
	int ycol = d_table->colIndex(title().text());
	int aux = 0;
	for (int i = d_start_row; i <= d_end_row; i++){
		QString xval = d_table->text(i, ycol);
		if (!xval.isEmpty() && aux < dataPoints){
			PieLabel* l = new PieLabel(d_plot, this);
			d_texts_list << l;
			l->setText(locale.toString(d_table->cell(i, ycol)/sum*100, 'g', 4) + "%");
			aux++;
		}
	}
}

void PieCurve::clearLabels()
{
	d_auto_labeling = false;
	foreach(PieLabel * l, d_texts_list)
		l->setText("");
}

PieLabel::PieLabel(Graph *plot, PieCurve *pie):LegendWidget(plot),
	d_pie_curve(pie),
	d_custom_text(QString::null)
{
	setBackgroundColor(QColor(255, 255, 255, 0));
	setFrameStyle(0);
	plot->add(this, false);
}

QString PieLabel::customText()
{
	if (d_custom_text.isEmpty())
		return text();

	return d_custom_text;
}

void PieLabel::closeEvent(QCloseEvent* e)
{
    setText(QString::null);
    hide();
	e->ignore();
}

QString PieLabel::saveToString()
{
	if (!d_pie_curve)
		return LegendWidget::saveToString();

	if (text().isEmpty())
		return QString::null;

	QString s = "<PieText>\n";
	s += "<index>" + QString::number(d_pie_curve->labelsList().indexOf(this)) + "</index>\n";
	s += FrameWidget::saveToString();
	s += "<Text>\n" + text() + "\n</Text>\n";

	QFont f = font();
	s += "<Font>" + f.family() + "\t";
	s += QString::number(f.pointSize())+"\t";
	s += QString::number(f.weight())+"\t";
	s += QString::number(f.italic())+"\t";
	s += QString::number(f.underline())+"\t";
	s += QString::number(f.strikeOut())+"</Font>\n";

	s += "<TextColor>" + textColor().name()+"</TextColor>\n";
	QColor bc = backgroundColor();
	s += "<Background>" + bc.name() + "</Background>\n";
	s += "<Alpha>" + QString::number(bc.alpha()) + "</Alpha>\n";
	s += "<Angle>" + QString::number(angle()) + "</Angle>\n";
	return s + "</PieText>\n";
}

void PieLabel::restore(Graph *g, const QStringList& lst)
{
	PieLabel *l = NULL;
	QStringList::const_iterator line;
	QColor backgroundColor = Qt::white, textColor = Qt::black;
	QFont f = QFont();
	double x = 0.0, y = 0.0;
	QString text;
	int frameStyle = 0, angle = 0;
	QPen framePen = QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
		if (s.contains("<index>")){
			int index = s.remove("<index>").remove("</index>").toInt();
			PieCurve *pie = (PieCurve *)g->curve(0);
			if(pie){
				QList<PieLabel *> labels = pie->labelsList();
				if (index >= 0 && index < labels.size())
					l = labels.at(index);
			}
		} else if (s.contains("<Frame>"))
			frameStyle = s.remove("<Frame>").remove("</Frame>").toInt();
		else if (s.contains("<Color>"))
			framePen.setColor((QColor(s.remove("<Color>").remove("</Color>"))));
		else if (s.contains("<FrameWidth>"))
			framePen.setWidth(s.remove("<FrameWidth>").remove("</FrameWidth>").toInt());
		else if (s.contains("<LineStyle>"))
			framePen.setStyle(PenStyleBox::penStyle(s.remove("<LineStyle>").remove("</LineStyle>").toInt()));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<Text>")){
			QStringList txt;
			while ( s != "</Text>" ){
				s = *(++line);
				txt << s;
			}
			txt.pop_back();
			text = txt.join("\n");
		} else if (s.contains("<Font>")){
			QStringList lst = s.remove("<Font>").remove("</Font>").split("\t");
			f = QFont(lst[0], lst[1].toInt(), lst[2].toInt(), lst[3].toInt());
			f.setUnderline(lst[4].toInt());
			f.setStrikeOut(lst[5].toInt());
		} else if (s.contains("<TextColor>"))
			textColor = QColor(s.remove("<TextColor>").remove("</TextColor>"));
		else if (s.contains("<Background>"))
			backgroundColor = QColor(s.remove("<Background>").remove("</Background>"));
		else if (s.contains("<Alpha>"))
			backgroundColor.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
		else if (s.contains("<Angle>"))
			angle = s.remove("<Angle>").remove("</Angle>").toInt();
	}

	if (l){
		l->setFrameStyle(frameStyle);
		l->setFramePen(framePen);
		l->setText(text);
		l->setFont(f);
		l->setTextColor(textColor);
		l->setBackgroundColor(backgroundColor);
		l->setAngle(angle);
		l->setOriginCoord(x, y);
	}
}
