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
#include <QFileInfo>
#include <QMessageBox>

#include <qwt_plot_canvas.h>

ImageWidget::ImageWidget(Graph *plot, const QString& fn):FrameWidget(plot),
d_save_xpm(false)
{
	if (load(fn, false)){
		QSize picSize = d_pix.size();
		int w = plot->canvas()->width();
		if (picSize.width() > w)
			picSize.setWidth(w);

		int h = plot->canvas()->height();
		if (picSize.height() > h)
			picSize.setHeight(h);

		setSize(picSize);
	}
	move(plot->canvas()->pos());
}

ImageWidget::ImageWidget(Graph *plot, const QImage& image):FrameWidget(plot),
d_save_xpm(true)
{
	
	d_pix = QPixmap::fromImage(image);
	
	QSize picSize = image.size();
	int w = plot->canvas()->width();
	if (picSize.width() > w)
		picSize.setWidth(w);

	int h = plot->canvas()->height();
	if (picSize.height() > h)
		picSize.setHeight(h);

	setSize(picSize);
	move(plot->canvas()->pos());
}

bool ImageWidget::load(const QString& fn, bool update)
{
    if (fn.isEmpty())
        return false;

    QFileInfo fi(fn);
    if (!fi.exists ()){
        QMessageBox::critical((QWidget *)plot()->multiLayer()->applicationWindow(),
        tr("QtiPlot - File openning error"),
        tr("The file: <b>%1</b> doesn't exist!").arg(fn));
        return false;
    }
    if (!fi.isReadable()){
        QMessageBox::critical((QWidget *)plot()->multiLayer()->applicationWindow(),
        tr("QtiPlot - File openning error"),
        tr("You don't have the permission to open this file: <b>%1</b>").arg(fn));
        return false;
    }

	QList<QByteArray> lst = QImageReader::supportedImageFormats() << "JPG";
	for (int i=0; i<(int)lst.count(); i++){
		if (fn.contains("." + lst[i])){
			d_pix.load(fn, lst[i], QPixmap::Auto);
			d_file_name = fn;
			if (update)
				repaint();
			return true;
		}
	}
	return false;
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
	if (d_pix.isNull())
		return;

	QPainter p(this);
	QRect r = rect();
	drawFrame(&p, r);
	switch(d_frame){
		case None:
			break;
		case Line:
			r.adjust(1, 1, -1, -1);
		break;
		case Shadow:
			r.adjust(1, 1, -d_frame_width, -d_frame_width);
		break;
	}
	p.drawPixmap(r, d_pix);
	e->accept();
}

void ImageWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	QRect r = QRect(x, y, width(), height());
	drawFrame(painter, r);
	switch(d_frame){
		case None:
			break;
		case Line:
			r.adjust(1, 1, -1, -1);
		break;
		case Shadow:
			r.adjust(1, 1, -d_frame_width, -d_frame_width);
		break;
	}
	painter->drawPixmap(r, d_pix);
}

void ImageWidget::setPixmap(const QPixmap& pix)
{
	d_pix = pix;
	int width = pix.width();
	int height = pix.height();
	if (d_frame == Shadow){
		width += d_frame_width;
		height += d_frame_width;
	}
	resize(QSize(width, height));
	repaint();
}

void ImageWidget::clone(ImageWidget* t)
{
	d_frame = t->frameStyle();
	d_color = t->frameColor();
	d_file_name = t->fileName();
	d_save_xpm = t->saveInternally();
	setPixmap(t->pixmap());
	setCoordinates(t->xValue(), t->yValue(), t->right(), t->bottom());
}

QString ImageWidget::saveToString()
{
	QString s = "<Image>\n";
	s += FrameWidget::saveToString();
	s += "<path>" + d_file_name + "</path>\n";
	if (d_save_xpm){
		s += "<xpm>\n";
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QIODevice::WriteOnly);
		d_pix.save(&buffer, "XPM");
		s += QString(bytes);
		s += "</xpm>\n";
	}
	return s + "</Image>\n";
}

void ImageWidget::restore(Graph *g, const QStringList& lst)
{
	int frameStyle = 0;
	QColor frameColor = Qt::black;
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	QStringList::const_iterator line;
	QString fn;
	bool save_xpm = false;
	ImageWidget *i = NULL;
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
			i = g->addImage(s.remove("<path>").remove("</path>"));
		else if (s.contains("<xpm>")){
			save_xpm = true;	
			if (!i){			
				QString xpm;
				while ( s != "</xpm>" ){
					s = *(++line);
					xpm += s + "\n";
				}
				QImage image;
    			if (image.loadFromData(xpm.toAscii()))
					i = g->addImage(image);
			}
		}
	}
	
	if (i){
		i->setFrameStyle(frameStyle);
		i->setFrameColor(frameColor);
		i->setCoordinates(x, y, right, bottom);
		i->setSaveInternally(save_xpm);
	}
}
