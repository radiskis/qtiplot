/***************************************************************************
	File                 : Spectrogram.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's Spectrogram Class
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
#include "Spectrogram.h"
#include "Graph.h"
#include "MultiLayer.h"
#include <ColorMapEditor.h>
#include <ApplicationWindow.h>
#include <PlotCurve.h>

#include <math.h>
#include <QPen>
#include <QPainter>
#include <qwt_scale_widget.h>

Spectrogram::Spectrogram(Graph *graph, Matrix *m):
	QwtPlotSpectrogram(QString(m->objectName())),
	d_graph(graph),
	d_matrix(m),
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(QwtLinearColorMap()),
	d_show_labels(true),
	d_labels_color(Qt::black),
	d_labels_font(QFont()),
	d_white_out_labels(false),
	d_labels_angle(0.0),
	d_labels_x_offset(0.0),
	d_labels_y_offset(0.0)
{
setData(MatrixData(m));
double step = fabs(data().range().maxValue() - data().range().minValue())/5.0;

QwtValueList contourLevels;
for ( double level = data().range().minValue() + step;
	level < data().range().maxValue(); level += step )
    contourLevels += level;

setContourLevels(contourLevels);
}

void Spectrogram::updateData(Matrix *m)
{
	if (!m)
		return;

	if (!d_graph)
		return;

	setData(MatrixData(m));
	setLevelsNumber(levels());

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		colorAxis->setColorMap(data().range(), colorMap());

	d_graph->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
	d_graph->replot();
}

void Spectrogram::setLevelsNumber(int levels)
{
double step = fabs(data().range().maxValue() - data().range().minValue())/(double)levels;

QwtValueList contourLevels;
for ( double level = data().range().minValue() + step;
	level < data().range().maxValue(); level += step )
    contourLevels += level;

setContourLevels(contourLevels);
}

void Spectrogram::setContourLevels (const QwtValueList & levels)
{
	QwtPlotSpectrogram::setContourLevels(levels);
	createLabels();
}

bool Spectrogram::hasColorScale()
{
	if (!d_graph)
		return false;

	if (!d_graph->axisEnabled (color_axis))
		return false;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	return colorAxis->isColorBarEnabled();
}

void Spectrogram::showColorScale(int axis, bool on)
{
if (hasColorScale() == on && color_axis == axis)
	return;

if (!d_graph)
	return;

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
colorAxis->setColorBarEnabled(false);

color_axis = axis;

// We must switch main and the color scale axes and their respective scales
	int xAxis = this->xAxis();
	int yAxis = this->yAxis();
	int oldMainAxis = QwtPlot::xBottom;
	if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop){
		oldMainAxis = xAxis;
		xAxis = 5 - color_axis;
	} else if (axis == QwtPlot::yLeft || axis == QwtPlot::yRight){
		oldMainAxis = yAxis;
		yAxis = 1 - color_axis;
	}

// First we switch axes
setAxis(xAxis, yAxis);

// Next we switch axes scales
QwtScaleDiv *scDiv = d_graph->axisScaleDiv(oldMainAxis);
if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	d_graph->setAxisScale(xAxis, scDiv->lowerBound(), scDiv->upperBound());
else if (axis == QwtPlot::yLeft || color_axis == QwtPlot::yRight)
	d_graph->setAxisScale(yAxis, scDiv->lowerBound(), scDiv->upperBound());

colorAxis = d_graph->axisWidget(color_axis);
d_graph->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
colorAxis->setColorBarEnabled(on);
colorAxis->setColorMap(data().range(), colorMap());
if (!d_graph->axisEnabled(color_axis))
	d_graph->enableAxis(color_axis);
colorAxis->show();
d_graph->updateLayout();
}

int Spectrogram::colorBarWidth()
{
	if (!d_graph)
		return 0;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	return colorAxis->colorBarWidth();
}

void Spectrogram::setColorBarWidth(int width)
{
	if (!d_graph)
		return;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	colorAxis->setColorBarWidth(width);
}

Spectrogram* Spectrogram::copy(Graph *g)
{
	Spectrogram *new_s = new Spectrogram(g, matrix());
	new_s->setDisplayMode(QwtPlotSpectrogram::ImageMode, testDisplayMode(QwtPlotSpectrogram::ImageMode));
	new_s->setDisplayMode(QwtPlotSpectrogram::ContourMode, testDisplayMode(QwtPlotSpectrogram::ContourMode));
	new_s->setCustomColorMap(color_map);
	new_s->setAxis(xAxis(), yAxis());
	new_s->setDefaultContourPen(defaultContourPen());
	new_s->color_map_policy = color_map_policy;
	new_s->d_show_labels = d_show_labels;
	new_s->d_labels_angle = d_labels_angle;
	new_s->d_labels_color = d_labels_color;
	new_s->d_labels_font = d_labels_font;
	new_s->d_labels_x_offset = d_labels_x_offset;
	new_s->d_labels_y_offset = d_labels_y_offset;
	new_s->setContourLevels(contourLevels());
	return new_s;
}

void Spectrogram::setGrayScale()
{
color_map = QwtLinearColorMap(Qt::black, Qt::white);
setColorMap(color_map);
color_map_policy = GrayScale;

if (!d_graph)
	return;

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(data().range(), colorMap());
}

void Spectrogram::setDefaultColorMap()
{
if (!d_graph)
	return;

color_map = d_graph->multiLayer()->applicationWindow()->d_3D_color_map;
setColorMap(color_map);
color_map_policy = Default;

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(this->data().range(), this->colorMap());
}

void Spectrogram::setCustomColorMap(const QwtLinearColorMap& map)
{
setColorMap(map);
color_map = map;
color_map_policy = Custom;

if (!d_graph)
	return;

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(this->data().range(), this->colorMap());
}

QString Spectrogram::saveToString()
{
QString s = "<spectrogram>\n";
s += "\t<matrix>" + QString(d_matrix->objectName()) + "</matrix>\n";

if (color_map_policy != Custom)
	s += "\t<ColorPolicy>" + QString::number(color_map_policy) + "</ColorPolicy>\n";
else
	s += ColorMapEditor::saveToXmlString(color_map);
s += "\t<Image>"+QString::number(testDisplayMode(QwtPlotSpectrogram::ImageMode))+"</Image>\n";

bool contourLines = testDisplayMode(QwtPlotSpectrogram::ContourMode);
s += "\t<ContourLines>"+QString::number(contourLines)+"</ContourLines>\n";
if (contourLines){
	QwtValueList levels = contourLevels();
	s += "\t\t<Levels>" + QString::number(levels.size()) + "</Levels>\n";
	for (int i = 0; i < levels.size(); i++)
		s += "\t\t<z>" + QString::number(levels[i]) + "</z>\n";

	bool defaultPen = defaultContourPen().style() != Qt::NoPen;
	s += "\t\t<DefaultPen>"+QString::number(defaultPen)+"</DefaultPen>\n";
	if (defaultPen){
		s += "\t\t\t<PenColor>"+defaultContourPen().color().name()+"</PenColor>\n";
		s += "\t\t\t<PenWidth>"+QString::number(defaultContourPen().widthF())+"</PenWidth>\n";
		s += "\t\t\t<PenStyle>"+QString::number(defaultContourPen().style() - 1)+"</PenStyle>\n";
		}
	}
QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
if (colorAxis && colorAxis->isColorBarEnabled()){
	s += "\t<ColorBar>\n\t\t<axis>" + QString::number(color_axis) + "</axis>\n";
	s += "\t\t<width>" + QString::number(colorAxis->colorBarWidth()) + "</width>\n";
	s += "\t</ColorBar>\n";
	}
s += "\t<Visible>"+ QString::number(isVisible()) + "</Visible>\n";
return s+"</spectrogram>\n";
}

QwtDoubleRect Spectrogram::boundingRect() const
{
	return d_matrix->boundingRect();
}

void Spectrogram::createLabels()
{
	foreach(QwtPlotMarker *m, d_labels_list){
		m->detach();
		delete m;
	}
	d_labels_list.clear();

	QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
    for (int l = 0; l < numLevels; l++){
		PlotMarker *m = new PlotMarker(l, d_labels_angle);

		QwtText t = QwtText(QString::number(levels[l]));
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

		/*QSize size = t.textSize();
        int dx = int(d_labels_x_offset*0.01*size.height());
        int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());
        int x2 = d_plot->transform(x_axis, x(index)) + dx;
        int y2 = d_plot->transform(y_axis, y(index)) + dy;

        switch(d_labels_align){
            case Qt::AlignLeft:
            break;
            case Qt::AlignHCenter:
                x2 -= size.width()/2;
            break;
            case Qt::AlignRight:
                x2 -= size.width();
            break;
        }*/

        if (d_graph)
			m->attach(d_graph);
		d_labels_list << m;
	}
}

void Spectrogram::showContourLineLabels(bool show)
{
	if (show == d_show_labels)
        return;

    d_show_labels = show;

    foreach(PlotMarker *m, d_labels_list){
    	if (d_show_labels)
			m->attach(d_graph);
		else
			m->detach();
	}
}

void Spectrogram::drawContourLines (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtRasterData::ContourLines &lines) const
{
	QwtPlotSpectrogram::drawContourLines(p, xMap, yMap, lines);
	if (d_show_labels)
		updateLabels(p, xMap, yMap, lines);
}

void Spectrogram::updateLabels(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QwtRasterData::ContourLines &contourLines) const
{
	QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
	int x_axis = xAxis();
	int y_axis = yAxis();
    for (int l = 0; l < numLevels; l++){
        const double level = levels[l];
        const QPolygonF &lines = contourLines[level];
        int i = (int)lines.size()/2;

		QwtPlotMarker *mrk = d_labels_list[l];
		if (!mrk)
			return;

		QSize size = mrk->label().textSize();
        int dx = int(d_labels_x_offset*0.01*size.height());
        int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());

		double x = lines[i].x();
		double y = lines[i].y();

		int x2 = d_graph->transform(x_axis, x) + dx;
        int y2 = d_graph->transform(y_axis, y) + dy;

        mrk->setValue(d_graph->invTransform(x_axis, x2),
					d_graph->invTransform(y_axis, y2));

		//mrk->setValue(lines[i].x(), lines[i].y());
    }
}

void Spectrogram::setLabelsFont(const QFont& font)
{
	if (font == d_labels_font)
        return;

    d_labels_font = font;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setFont(font);
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsColor(const QColor& c)
{
    if (c == d_labels_color)
        return;

    d_labels_color = c;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setColor(c);
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsWhiteOut(bool whiteOut)
{
    if (whiteOut == d_white_out_labels)
        return;

    d_white_out_labels = whiteOut;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if (whiteOut)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsOffset(int x, int y)
{
    if (x == d_labels_x_offset && y == d_labels_y_offset)
        return;

    d_labels_x_offset = x;
    d_labels_y_offset = y;
}

void Spectrogram::setLabelsRotation(double angle)
{
    if (angle == d_labels_angle)
        return;

    d_labels_angle = angle;

    foreach(PlotMarker *m, d_labels_list)
		m->setAngle(angle);
}

double MatrixData::value(double x, double y) const
{
	x += 0.5*dx;
	y -= 0.5*dy;

	int i = abs((y - y_start)/dy);
	int j = abs((x - x_start)/dx);
	if (d_m && i >= 0 && i < n_rows && j >=0 && j < n_cols)
		return d_m[i][j];

	return 0.0;
}
