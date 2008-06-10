/***************************************************************************
    File                 : ImageWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying images in 2D plots

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
#include "ImageWidget.h"
#include <QPainter>
#include <QBuffer>
#include <QImageReader>

ImageWidget::ImageWidget(Graph *plot, const QString& fn):FrameWidget(plot),
d_file_name(fn)
{
    QList<QByteArray> lst = QImageReader::supportedImageFormats();
	for (int i=0; i<(int)lst.count(); i++){
		if (fn.contains("." + lst[i])){
			d_pix.load(fn, lst[i], QPixmap::Auto);
			resize(d_pix.size());
			d_file_name = fn;
			break;
		}
	}
}

void ImageWidget::setFileName(const QString& fn)
{
    d_file_name = fn;
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
	if (d_pix.isNull())
		return;

	QPainter p(this);
	drawFrame(&p, rect());
	p.drawPixmap(rect(), d_pix);

	e->accept();
}

void ImageWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	QRect rect = QRect(x, y, width(), height());
	drawFrame(painter, rect);
	painter->drawPixmap(rect, d_pix);
}

void ImageWidget::setPixmap(const QPixmap& pix)
{
	d_pix = pix;
	int width = pix.width();
	int height = pix.height();
	if (d_frame == Shadow){
		width += 5;
		height += 5;
	}
	resize(QSize(width, height));
	repaint();
}

void ImageWidget::clone(ImageWidget* t)
{
	d_frame = t->frameStyle();
	d_color = t->frameColor();
	d_file_name = t->fileName();
	setPixmap(t->pixmap());
	resize(t->size());
	setOriginCoord(t->xValue(), t->yValue());
	updateCoordinates();
}

QString ImageWidget::saveToString()
{
	QString s = "<Image>\n";
	s += FrameWidget::saveToString();

    s += "<right>" + QString::number(d_x_right, 'g', 15) + "</right>\n";
    s += "<bottom>" + QString::number(d_y_bottom, 'g', 15) + "</bottom>\n";
	s += "<path>" + d_file_name + "</path>\n";
	return s + "</Image>\n";
}

void ImageWidget::restore(Graph *g, const QStringList& lst)
{
	int frameStyle = 0;
	QColor frameColor = Qt::black;
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	QStringList::const_iterator line;
	QString fn;
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Frame>"))
			frameStyle = s.remove("<Frame>").remove("</Frame>").toInt();
		else if (s.contains("<Color>"))
			frameColor = QColor(s.remove("<Color>").remove("</Color>"));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<right>"))
			right = s.remove("<right>").remove("</right>").toDouble();
		else if (s.contains("<bottom>"))
			bottom = s.remove("<bottom>").remove("</bottom>").toDouble();
		else if (s.contains("<path>"))
			fn = s.remove("<path>").remove("</path>");
	}
	ImageWidget *t = g->addImage(fn);
	t->setFrameStyle(frameStyle);
	t->setFrameColor(frameColor);
	t->setBoundingRect(x, y, right, bottom);
}
