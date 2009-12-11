/***************************************************************************
    File                 : LineProfileTool.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for calculating intensity profiles of
                           image markers.

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
#include "LineProfileTool.h"
#include "ImageWidget.h"
#include "Graph.h"
#include <ApplicationWindow.h>
#include "MultiLayer.h"

#include <QPoint>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <qwt_plot_canvas.h>

LineProfileTool::LineProfileTool(Graph *graph, ApplicationWindow *app, int average_pixels)
	: QWidget(graph),
	PlotToolInterface(graph),
	d_app(app)
{
	d_op_start = d_op_dp = QPoint(0,0);
	// make sure we average over an odd number of pixels
	d_average_pixels = (average_pixels % 2) ? average_pixels : average_pixels + 1;
	d_target = qobject_cast<ImageWidget*>(d_graph->activeEnrichment());
	if (!d_target)
		QMessageBox::critical(d_graph->window(), tr("QtiPlot - Pixel selection warning"),
				tr("Please select an image marker first."));
	else
		setParent(d_target);

	d_graph->deselectMarker();

	setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
	setCursor(Qt::PointingHandCursor);
	show();
	setFocus();
}

void LineProfileTool::calculateLineProfile(const QPoint& start, const QPoint& end)
{
	QRect rect = d_target->geometry();
	if (!rect.contains(start) || !rect.contains(end)){
		QMessageBox::warning(d_graph, tr("QtiPlot - Pixel selection warning"),
				tr("Please select the end line point inside the image rectangle!"));
		return;
	}

	QPoint o = d_target->pos();
	QPixmap pic = d_target->pixmap();
	QImage image = pic.convertToImage();

	int x1 = start.x()-o.x();
	int x2 = end.x()-o.x();
	int y1 = start.y()-o.y();
	int y2 = end.y()-o.y();

	QSize realSize = pic.size();
	QSize actualSize = d_target->size();

	if (realSize != actualSize){
		double ratioX = (double)realSize.width()/(double)actualSize.width();
		double ratioY = (double)realSize.height()/(double)actualSize.height();
		x1 = int(x1*ratioX);
		x2 = int(x2*ratioX);
		y1 = int(y1*ratioY);
		y2 = int(y2*ratioY);
	}

	QString text =  + "\n";

	//uses the fast Bresenham's line-drawing algorithm
#define sgn(x) ((x<0)?-1:((x>0)?1:0))
	int dx = x2 - x1;      //the horizontal distance of the line
	int dy = y2 - y1;      //the vertical distance of the line
	int dxabs = abs(dx);
	int dyabs = abs(dy);
	int sdx = sgn(dx);
	int sdy = sgn(dy);
	int x = dyabs >> 1;
	int y = dxabs >> 1;
	int px = x1;
	int py = y1;

	int n = (dxabs >= dyabs) ? dxabs : dyabs;
	Table *t = d_app->newTable(n, 4, QString::null, QString::null);
	t->setHeader(QStringList() << tr("pixel") << tr("x") << tr("y") << tr("intensity"));

	if (dxabs >= dyabs){ //the line is more horizontal than vertical
		for(int i = 0; i< dxabs; i++){
			y += dyabs;
			if (y >= dxabs){
				y -= dxabs;
				py += sdy;
			}
			px += sdx;

			t->setText(i, 0, QString::number(i));
			t->setText(i, 1, QString::number(px));
			t->setText(i, 2, QString::number(py));
			t->setCell(i, 3, averageImagePixel(image, px, py, true));
		}
	} else {// the line is more vertical than horizontal
		for(int i = 0; i < dyabs; i++){
			x += dxabs;
			if (x >= dyabs){
				x -= dyabs;
				px += sdx;
			}
			py += sdy;

			t->setText(i, 0, QString::number(i));
			t->setText(i, 1, QString::number(px));
			t->setText(i, 2, QString::number(py));
			t->setCell(i, 3, averageImagePixel(image, px, py, true));
		}
	}
	t->showNormal();

	MultiLayer* plot = d_app->multilayerPlot(t, QStringList(QString(t->objectName())+"_intensity"), 0);
	Graph *g = (Graph*)plot->activeLayer();
	if (g){
		g->setTitle("");
		g->setXAxisTitle(tr("pixels"));
		g->setYAxisTitle(tr("pixel intensity (a.u.)"));
	}

}

double LineProfileTool::averageImagePixel(const QImage& image, int px, int py, bool moreHorizontal)
{
	QRgb pixel;
	int sum=0,start,i;
	int middle=int(0.5*(d_average_pixels-1));
	if (moreHorizontal){
		start=py-middle;
		for(i=0; i<d_average_pixels; i++){
			pixel= image.pixel(px,start+i);
			sum+=qGray(pixel);
		}
	} else {
		start=px-middle;
		for(i=0; i<d_average_pixels; i++) {
			pixel= image.pixel(start+i,py);
			sum+=qGray(pixel);
		}
	}
	return (double)sum/(double)d_average_pixels;
}

void LineProfileTool::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
	p.drawLine(d_op_start, d_op_start+d_op_dp);
}

void LineProfileTool::mousePressEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton)
		return;
	d_op_start = e->pos();
	e->accept();
}

void LineProfileTool::mouseMoveEvent(QMouseEvent *e)
{
	d_op_dp = e->pos() - d_op_start;
	repaint();

	QRect rect = d_target->geometry();
	if (!rect.contains(e->pos()))
		setCursor(Qt::ArrowCursor);
	else
		setCursor(Qt::PointingHandCursor);

	e->accept();
}

void LineProfileTool::mouseReleaseEvent(QMouseEvent *e)
{
	calculateLineProfile(d_op_start, e->pos());
	d_graph->setActiveTool(NULL);// attention: I'm now deleted
}
