/***************************************************************************
    File                 : TexWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying Tex content as image in 2D plots

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
#include "TexWidget.h"
#include <Graph.h>
#include <PenStyleBox.h>

#include <QPainter>
#include <QBuffer>
#include <QMessageBox>
#include <QPaintEvent>

TexWidget::TexWidget(Graph *plot, const QString& s, const QPixmap& pix):FrameWidget(plot),
d_pix(pix),
d_formula(s),
d_margin(5)
{
	setObjectName(tr("Equation"));
	if (!d_pix.isNull())
		setSize(QSize(pix.width() + 2*d_margin, pix.height() + 2*d_margin));
}

void TexWidget::paintEvent(QPaintEvent *e)
{
	if (d_pix.isNull())
		return;

	QPainter p(this);
	drawFrame(&p, rect());

	int lw = d_frame_pen.width();
	int w = width() - 2*d_margin - 2*lw;
	int h = height() - 2*d_margin - 2*lw;
	if (d_frame == Shadow){
		w -= d_margin;
		h -= d_margin;
	}
	QRect pixRect = QRect (lw + d_margin, lw + d_margin, w, h);
	p.drawPixmap(pixRect, d_pix);

	e->accept();
}

void TexWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());
	int xr = map[QwtPlot::xBottom].transform(calculateRightValue());
	int yr = map[QwtPlot::yLeft].transform(calculateBottomValue());
	int width = abs(xr - x);
	int height = abs(yr - y);

	drawFrame(painter, QRect(x, y, width, height));

	int lw = d_frame_pen.width();

	// calculate resolution factor
	double xfactor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	double yfactor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();

	int margin_x = qRound(d_margin*xfactor);
	int margin_y = qRound(d_margin*yfactor);

	int w = width - 2*margin_x  - 2*lw;
	int h = height - 2*margin_y - 2*lw;

	if (d_frame == Shadow){
		w -= d_margin;
		h -= d_margin;
	}
	painter->drawPixmap(QRect (x + lw + margin_x, y + lw + margin_y, w, h), d_pix);
}

void TexWidget::setPixmap(const QPixmap& pix)
{
	d_pix = pix;
	int lw = 2*d_frame_pen.width();
	int width = pix.width() + 2*d_margin + lw;
	int height = pix.height() + 2*d_margin + lw;
	if (d_frame == Shadow){
		width += d_margin;
		height += d_margin;
	}
	setSize(QSize(width, height));
	repaint();
}

void TexWidget::setBestSize()
{
	int lw = 2*d_frame_pen.width();
	int w = d_pix.width() + 2*d_margin + lw;
	int h = d_pix.height() + 2*d_margin + lw;
	if (d_frame == Shadow){
		w += d_margin;
		h += d_margin;
	}
	if (size() == QSize(w,h))
		return;

	setSize(QSize(w,h));
}

void TexWidget::clone(TexWidget* t)
{
	d_frame = t->frameStyle();
	setFramePen(t->framePen());
	d_formula = t->formula();
	d_pix = t->pixmap();
	setSize(t->size());
	setOriginCoord(t->xValue(), t->yValue());
}

QString TexWidget::saveToString()
{
	QString s = "<TexFormula>\n";
	s += FrameWidget::saveToString();
	s += "<tex>" + d_formula + "</tex>\n";
	s += "<xpm>\n";
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	d_pix.save(&buffer, "XPM");
	s += QString(bytes);
	s += "</xpm>\n";
	return s + "</TexFormula>\n";
}

void TexWidget::restore(Graph *g, const QStringList& lst)
{
	if (!g)
		return;

	int frameStyle = 0;
	QPen pen = QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	AttachPolicy attachTo = FrameWidget::Page;
	QPixmap pix;
	QStringList::const_iterator line;
	QString formula;
	bool onTop = true, show = true;
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Frame>"))
			frameStyle = s.remove("<Frame>").remove("</Frame>").toInt();
		else if (s.contains("<Color>"))
			pen.setColor(QColor(s.remove("<Color>").remove("</Color>")));
		else if (s.contains("<FrameWidth>"))
			pen.setWidth(s.remove("<FrameWidth>").remove("</FrameWidth>").toInt());
		else if (s.contains("<LineStyle>"))
			pen.setStyle(PenStyleBox::penStyle(s.remove("<LineStyle>").remove("</LineStyle>").toInt()));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<right>"))
			right = s.remove("<right>").remove("</right>").toDouble();
		else if (s.contains("<bottom>"))
			bottom = s.remove("<bottom>").remove("</bottom>").toDouble();
		else if (s.contains("<attachTo>"))
			attachTo = (FrameWidget::AttachPolicy)s.remove("<attachTo>").remove("</attachTo>").toInt();
		else if (s.contains("<onTop>"))
			onTop = s.remove("<onTop>").remove("</onTop>").toInt();
		else if (s.contains("<visible>"))
			show = s.remove("<visible>").remove("</visible>").toInt();
		else if (s.contains("<tex>"))
			formula = s.remove("<tex>").remove("</tex>");
		else if (s.contains("<xpm>")){
			QString xpm;
			while ( s != "</xpm>" ){
				s = *(++line);
				xpm += s + "\n";
			}
			QImage image;
    		if (image.loadFromData(xpm.toAscii()))
				pix = QPixmap::fromImage(image);
		}
	}

	g->updateLayout();

	TexWidget *t = g->addTexFormula(formula, pix);
	t->setFrameStyle(frameStyle);
	t->setFramePen(pen);
	t->setCoordinates(x, y, right, bottom);
	t->setAttachPolicy(attachTo);
	t->setOnTop(onTop);
	t->setVisible(show);
}
