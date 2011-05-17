/***************************************************************************
    File                 : Grid.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D Grid class

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
#include "Graph.h"
#include "Grid.h"
#include <ColorBox.h>

#include <qwt_plot_canvas.h>
#include <qwt_painter.h>
#include <qwt_scale_widget.h>
#include <QPainter>

Grid::Grid() : QwtPlotGrid(),
d_maj_pen_y(QPen(Qt::blue, 0.5, Qt::SolidLine)),
d_min_pen_y(QPen(Qt::gray, 0.4, Qt::DotLine)),
mrkX(NULL),
mrkY(NULL)
{
	setMajPen(QPen(Qt::blue, 0.5, Qt::SolidLine));
	setMinPen(QPen(Qt::gray, 0.4, Qt::DotLine));
	enableX(false);
	enableY(false);
    setRenderHint(QwtPlotItem::RenderAntialiased, false);
}

/*!
  \brief Draw the grid

  The grid is drawn into the bounding rectangle such that
  gridlines begin and end at the rectangle's borders. The X and Y
  maps are used to map the scale divisions into the drawing region
  screen.
  \param painter  Painter
  \param mx X axis map
  \param my Y axis
  \param r Contents rect of the plot canvas
  */
void Grid::draw(QPainter *painter,
		const QwtScaleMap &mx, const QwtScaleMap &my,
		const QRect &r) const
{

	double x_factor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	double y_factor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();

	//  draw minor X gridlines
	QPen pen = minPen();
	pen.setWidthF(pen.widthF() * x_factor);
	painter->setPen(pen);

	if (xMinEnabled()){
		drawLines(painter, r, Qt::Vertical, mx,
				xScaleDiv().ticks(QwtScaleDiv::MinorTick));
		drawLines(painter, r, Qt::Vertical, mx,
				xScaleDiv().ticks(QwtScaleDiv::MediumTick));
	}

	//  draw minor Y gridlines
	pen = d_min_pen_y;
	pen.setWidthF(pen.widthF() * y_factor);
	painter->setPen(pen);

	if (yMinEnabled()){
		drawLines(painter, r, Qt::Horizontal, my,
				yScaleDiv().ticks(QwtScaleDiv::MinorTick));
		drawLines(painter, r, Qt::Horizontal, my,
				yScaleDiv().ticks(QwtScaleDiv::MediumTick));
	}

	//  draw major X gridlines
	pen = majPen();
	pen.setWidthF(pen.widthF() * x_factor);
	painter->setPen(pen);

	if (xEnabled()){
		drawLines(painter, r, Qt::Vertical, mx,
				xScaleDiv().ticks (QwtScaleDiv::MajorTick));
	}

	//  draw major Y gridlines
	pen = d_maj_pen_y;
	pen.setWidthF(pen.widthF() * y_factor);
	painter->setPen(pen);

	if (yEnabled()){
		drawLines(painter, r, Qt::Horizontal, my,
				yScaleDiv().ticks (QwtScaleDiv::MajorTick));
	}
}

void Grid::drawLines(QPainter *painter, const QRect &rect,
		Qt::Orientation orientation, const QwtScaleMap &map,
		const QwtValueList &values) const
{
	if (values.isEmpty())
		return;
		
	const int x1 = rect.left();
	const int x2 = rect.right();
	const int y1 = rect.top();
	const int y2 = rect.bottom();

	Graph *g = (Graph *)this->plot();
	if (g && g->canvasFrameWidth()){
		for (uint i = 0; i < (uint)values.count(); i++){
			const int value = map.transform(values[i]);
			if ( orientation == Qt::Horizontal ){
				if ((value > y1 + 1) && (value < y2 - 1))
					QwtPainter::drawLine(painter, x1, value, x2, value);
			} else {
				if ((value > x1 + 1) && (value < x2 - 1))
					QwtPainter::drawLine(painter, value, y1, value, y2);
			}
		}
	} else {
		for (uint i = 0; i < (uint)values.count(); i++){
			const int value = map.transform(values[i]);
			if ( orientation == Qt::Horizontal ){
				if ((value > y1) && (value < y2))
					QwtPainter::drawLine(painter, x1, value, x2, value);
			} else {
				if ((value > x1) && (value < x2))
					QwtPainter::drawLine(painter, value, y1, value, y2);
			}
		}
	}
}

void Grid::load(const QStringList& grid)
{
	Graph *d_plot = (Graph *)plot();
	if (!d_plot)
		return;

	bool majorOnX = grid[1].toInt();
	bool minorOnX = grid[2].toInt();
	bool majorOnY = grid[3].toInt();
	bool minorOnY = grid[4].toInt();
	bool xZeroOn = false;
	bool yZeroOn = false;
	int xAxis = QwtPlot::xBottom;
	int yAxis = QwtPlot::yLeft;

    QPen majPenX, minPenX, majPenY, minPenY;
	if (grid.count() >= 21){ // since 0.9 final
		majPenX = QPen(QColor(grid[5]), grid[7].toDouble(), Graph::getPenStyle(grid[6].toInt()));
		minPenX = QPen(QColor(grid[8]), grid[10].toDouble(), Graph::getPenStyle(grid[9].toInt()));
		majPenY = QPen(QColor(grid[11]), grid[13].toDouble(), Graph::getPenStyle(grid[12].toInt()));
		minPenY = QPen(QColor(grid[14]), grid[16].toDouble(), Graph::getPenStyle(grid[15].toInt()));

		xZeroOn = grid[17].toInt();
		yZeroOn = grid[18].toInt();
        xAxis = grid[19].toInt();
        yAxis = grid[20].toInt();
        if (grid.count() >= 22)
            setRenderHint(QwtPlotItem::RenderAntialiased, grid[21].toInt());

	} else { // older versions of QtiPlot (<= 0.9rc3)
		majPenX = QPen(ColorBox::defaultColor(grid[5].toInt()), grid[7].toDouble(), Graph::getPenStyle(grid[6].toInt()));
		minPenX = QPen(ColorBox::defaultColor(grid[8].toInt()), grid[10].toDouble(), Graph::getPenStyle(grid[9].toInt()));
		majPenY = majPenX;
		minPenY = minPenX;

		xZeroOn = grid[11].toInt();
		yZeroOn = grid[12].toInt();

		if (grid.count() == 15){
			xAxis = grid[13].toInt();
			yAxis = grid[14].toInt();
		}
	}

	setMajPenX(majPenX);
	setMinPenX(minPenX);
	setMajPenY(majPenY);
	setMinPenY(minPenY);

	enableX(majorOnX);
	enableXMin(minorOnX);
	enableY(majorOnY);
	enableYMin(minorOnY);

	setAxis(xAxis, yAxis);

	enableZeroLineX(xZeroOn);
	enableZeroLineY(yZeroOn);
}

void Grid::enableZeroLineX(bool enable)
{
	Graph *d_plot = (Graph *)plot();
	if (!d_plot)
		return;

	if (!mrkX && enable){
		mrkX = new QwtPlotMarker();
		d_plot->insertMarker(mrkX);
		mrkX->setRenderHint(QwtPlotItem::RenderAntialiased, false);
		mrkX->setAxis(xAxis(), yAxis());
		mrkX->setLineStyle(QwtPlotMarker::VLine);
		mrkX->setValue(0.0, 0.0);

		QColor c = Qt::black;
		if (d_plot->axisEnabled (QwtPlot::yLeft))
			c = d_plot->axisWidget(QwtPlot::yLeft)->palette().color(QPalette::Foreground);
		else if (d_plot->axisEnabled (QwtPlot::yRight))
			c = d_plot->axisWidget(QwtPlot::yRight)->palette().color(QPalette::Foreground);

		mrkX->setLinePen(QPen(c, d_plot->axesLinewidth(), Qt::SolidLine));
	} else if (mrkX && !enable) {
		mrkX->detach();
		d_plot->replot();
		mrkX = NULL;
	}
}

void Grid::enableZeroLineY(bool enable)
{
	Graph *d_plot = (Graph *)plot();
	if (!d_plot)
		return;

	if (!mrkY && enable) {
		mrkY = new QwtPlotMarker();
		d_plot->insertMarker(mrkY);
		mrkY->setRenderHint(QwtPlotItem::RenderAntialiased, false);
		mrkY->setAxis(xAxis(), yAxis());
		mrkY->setLineStyle(QwtPlotMarker::HLine);
		mrkY->setValue(0.0, 0.0);

		QColor c = Qt::black;
		if (d_plot->axisEnabled (QwtPlot::xBottom))
			c = d_plot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Foreground);
		else if (d_plot->axisEnabled (QwtPlot::xTop))
			c = d_plot->axisWidget(QwtPlot::xTop)->palette().color(QPalette::Foreground);

		mrkY->setLinePen(QPen(c, d_plot->axesLinewidth(), Qt::SolidLine));
	} else if (mrkY && !enable){
		mrkY->detach();
		d_plot->replot();
		mrkY = NULL;
	}
}

const QPen& Grid::xZeroLinePen()
{
	if (mrkX)
		return mrkX->linePen();

	return QPen();
}

void Grid::setXZeroLinePen(const QPen &p)
{
	if (mrkX && mrkX->linePen() != p)
		mrkX->setLinePen(p);
}

const QPen& Grid::yZeroLinePen()
{
	if (mrkY)
		return mrkY->linePen();

	return QPen();
}

void Grid::setYZeroLinePen(const QPen &p)
{
	if (mrkY && mrkY->linePen() != p)
		mrkY->setLinePen(p);
}

void Grid::copy(Grid *grid)
{
	if (!grid)
		return;

	setMajPenX(grid->majPenX());
	setMinPenX(grid->minPenX());
	setMajPenY(grid->majPenY());
	setMinPenY(grid->minPenY());

	enableX(grid->xEnabled());
	enableXMin(grid->xMinEnabled());
	enableY(grid->yEnabled());
	enableYMin(grid->yMinEnabled());

	setAxis(grid->xAxis(), grid->yAxis());
	setZ(grid->z());

	enableZeroLineX(grid->xZeroLineEnabled());
	enableZeroLineY(grid->yZeroLineEnabled());
	setRenderHint(QwtPlotItem::RenderAntialiased, grid->testRenderHint(QwtPlotItem::RenderAntialiased));
}

QString Grid::saveToString()
{
	QString s = "grid\t";
	s += QString::number(xEnabled())+"\t";
	s += QString::number(xMinEnabled())+"\t";
	s += QString::number(yEnabled())+"\t";
	s += QString::number(yMinEnabled())+"\t";

	s += majPenX().color().name()+"\t";
	s += QString::number(majPenX().style() - 1)+"\t";
	s += QString::number(majPenX().widthF())+"\t";

	s += minPenX().color().name()+"\t";
	s += QString::number(minPenX().style() - 1)+"\t";
	s += QString::number(minPenX().widthF())+"\t";

    s += majPenY().color().name()+"\t";
	s += QString::number(majPenY().style() - 1)+"\t";
	s += QString::number(majPenY().widthF())+"\t";

	s += minPenY().color().name()+"\t";
	s += QString::number(minPenY().style() - 1)+"\t";
	s += QString::number(minPenY().widthF())+"\t";

	s += QString::number(xZeroLineEnabled())+"\t";
	s += QString::number(yZeroLineEnabled())+"\t";
	s += QString::number(xAxis())+"\t";
	s += QString::number(yAxis())+"\t";
	s += QString::number(testRenderHint(QwtPlotItem::RenderAntialiased))+"\n";
	return s;
}
