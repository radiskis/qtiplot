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

#include <math.h>
#include <QPen>
#include <qwt_scale_widget.h>

Spectrogram::Spectrogram():
	QwtPlotSpectrogram(),
	d_matrix(0),
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(QwtLinearColorMap())
{
}

Spectrogram::Spectrogram(Matrix *m):
	QwtPlotSpectrogram(QString(m->objectName())),
	d_matrix(m),
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(QwtLinearColorMap())
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

QwtPlot *plot = this->plot();
if (!plot)
	return;

setData(MatrixData(m));
setLevelsNumber(levels());

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(data().range(), colorMap());

plot->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
plot->replot();
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

bool Spectrogram::hasColorScale()
{
	QwtPlot *plot = this->plot();
	if (!plot)
		return false;

	if (!plot->axisEnabled (color_axis))
		return false;

	QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
	return colorAxis->isColorBarEnabled();
}

void Spectrogram::showColorScale(int axis, bool on)
{
if (hasColorScale() == on && color_axis == axis)
	return;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
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
QwtScaleDiv *scDiv = plot->axisScaleDiv(oldMainAxis);
if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	plot->setAxisScale(xAxis, scDiv->lowerBound(), scDiv->upperBound());
else if (axis == QwtPlot::yLeft || color_axis == QwtPlot::yRight)
	plot->setAxisScale(yAxis, scDiv->lowerBound(), scDiv->upperBound());

colorAxis = plot->axisWidget(color_axis);
plot->setAxisScale(color_axis, data().range().minValue(), data().range().maxValue());
colorAxis->setColorBarEnabled(on);
colorAxis->setColorMap(data().range(), colorMap());
if (!plot->axisEnabled(color_axis))
	plot->enableAxis(color_axis);
colorAxis->show();
plot->updateLayout();
}

int Spectrogram::colorBarWidth()
{
QwtPlot *plot = this->plot();
if (!plot)
	return 0;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
return colorAxis->colorBarWidth();
}

void Spectrogram::setColorBarWidth(int width)
{
QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
colorAxis->setColorBarWidth(width);
}

Spectrogram* Spectrogram::copy()
{
Spectrogram *new_s = new Spectrogram(matrix());
new_s->setDisplayMode(QwtPlotSpectrogram::ImageMode, testDisplayMode(QwtPlotSpectrogram::ImageMode));
new_s->setDisplayMode(QwtPlotSpectrogram::ContourMode, testDisplayMode(QwtPlotSpectrogram::ContourMode));
new_s->setColorMap (colorMap());
new_s->setAxis(xAxis(), yAxis());
new_s->setDefaultContourPen(defaultContourPen());
new_s->setLevelsNumber(levels());
new_s->color_map_policy = color_map_policy;
return new_s;
}

void Spectrogram::setGrayScale()
{
color_map = QwtLinearColorMap(Qt::black, Qt::white);
setColorMap(color_map);
color_map_policy = GrayScale;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(data().range(), colorMap());
}

void Spectrogram::setDefaultColorMap()
{
Graph *plot = qobject_cast<Graph *>(this->plot());
if (!plot)
	return;

color_map = plot->multiLayer()->applicationWindow()->d_3D_color_map;
setColorMap(color_map);
color_map_policy = Default;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
if (colorAxis)
	colorAxis->setColorMap(this->data().range(), this->colorMap());
}

void Spectrogram::setCustomColorMap(const QwtLinearColorMap& map)
{
setColorMap(map);
color_map = map;
color_map_policy = Custom;

QwtPlot *plot = this->plot();
if (!plot)
	return;

QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
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
if (contourLines)
	{
	s += "\t\t<Levels>"+QString::number(levels())+"</Levels>\n";
	bool defaultPen = defaultContourPen().style() != Qt::NoPen;
	s += "\t\t<DefaultPen>"+QString::number(defaultPen)+"</DefaultPen>\n";
	if (defaultPen)
		{
		s += "\t\t\t<PenColor>"+defaultContourPen().color().name()+"</PenColor>\n";
		s += "\t\t\t<PenWidth>"+QString::number(defaultContourPen().widthF())+"</PenWidth>\n";
		s += "\t\t\t<PenStyle>"+QString::number(defaultContourPen().style() - 1)+"</PenStyle>\n";
		}
	}
QwtScaleWidget *colorAxis = plot()->axisWidget(color_axis);
if (colorAxis && colorAxis->isColorBarEnabled())
	{
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
