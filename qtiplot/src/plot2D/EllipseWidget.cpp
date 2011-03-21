/***************************************************************************
    File                 : EllipseWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying ellipses/circles in 2D plots

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
#include "EllipseWidget.h"
#include <Graph.h>
#include <PatternBox.h>
#include <PenStyleBox.h>

#include <QPainter>
#include <QPaintEngine>
#include <QPalette>

#include <qwt_painter.h>

EllipseWidget::EllipseWidget(Graph *plot):FrameWidget(plot)
{
	setFrameStyle(Line);
	setObjectName(tr("Ellipse"));
}

void EllipseWidget::clone(EllipseWidget* r)
{
	d_frame = r->frameStyle();
	setFramePen(r->framePen());
	setBackgroundColor(r->backgroundColor());
	setBrush(r->brush());
	setCoordinates(r->xValue(), r->yValue(), r->right(), r->bottom());
}

QString EllipseWidget::saveToString()
{
	QString s = "<Ellipse>\n";
	s += FrameWidget::saveToString();
	QColor bc = backgroundColor();
	s += "<Background>" + bc.name() + "</Background>\n";
	s += "<Alpha>" + QString::number(bc.alpha()) + "</Alpha>\n";
	s += "<BrushColor>" + d_brush.color().name() + "</BrushColor>\n";
	s += "<BrushStyle>" + QString::number(PatternBox::patternIndex(d_brush.style())) + "</BrushStyle>\n";
	return s + "</Ellipse>\n";
}

void EllipseWidget::restore(Graph *g, const QStringList& lst)
{
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	QStringList::const_iterator line;
	QColor backgroundColor = Qt::white;
	QBrush brush = QBrush();
	EllipseWidget *r = new EllipseWidget(g);
	if (!r)
		return;

	for (line = lst.begin(); line != lst.end(); line++){
		QString s = *line;
		if (s.contains("<Frame>"))
			r->setFrameStyle(s.remove("<Frame>").remove("</Frame>").toInt());
		else if (s.contains("<Color>"))
			r->setFrameColor(QColor(s.remove("<Color>").remove("</Color>")));
		else if (s.contains("<FrameWidth>"))
			r->setFrameWidth(s.remove("<FrameWidth>").remove("</FrameWidth>").toDouble());
		else if (s.contains("<LineStyle>"))
			r->setFrameLineStyle(PenStyleBox::penStyle(s.remove("<LineStyle>").remove("</LineStyle>").toInt()));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<right>"))
			right = s.remove("<right>").remove("</right>").toDouble();
		else if (s.contains("<bottom>"))
			bottom = s.remove("<bottom>").remove("</bottom>").toDouble();
		else if (s.contains("<attachTo>"))
			r->setAttachPolicy((FrameWidget::AttachPolicy)s.remove("<attachTo>").remove("</attachTo>").toInt());
		else if (s.contains("<onTop>"))
			r->setOnTop(s.remove("<onTop>").remove("</onTop>").toInt());
		else if (s.contains("<visible>"))
			r->setVisible(s.remove("<visible>").remove("</visible>").toInt());
		else if (s.contains("<Background>"))
			backgroundColor = QColor(s.remove("<Background>").remove("</Background>"));
		else if (s.contains("<Alpha>"))
			backgroundColor.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
		else if (s.contains("<BrushColor>"))
			brush.setColor(QColor(s.remove("<BrushColor>").remove("</BrushColor>")));
		else if (s.contains("<BrushStyle>"))
			brush.setStyle(PatternBox::brushStyle((s.remove("<BrushStyle>").remove("</BrushStyle>")).toInt()));
	}

	r->setBackgroundColor(backgroundColor);
	r->setBrush(brush);
	r->setCoordinates(x, y, right, bottom);
	g->add(r, false);
}

void EllipseWidget::drawFrame(QPainter *p, const QRect& rect)
{
	p->save();
	if (d_plot->antialiasing())
		p->setRenderHints(QPainter::Antialiasing);

	QPainterPath ellipse;
	if (d_frame == Line){
		QPen pen = QwtPainter::scaledPen(d_frame_pen);
		p->setPen(pen);

		int lw = pen.width()/2;
		QRect r = rect.adjusted(lw + 1, lw + 1, -lw - 1, -lw - 1);

		ellipse.addEllipse(r);
		p->fillPath(ellipse, palette().color(QPalette::Window));
		if (d_brush.style() != Qt::NoBrush)
			p->setBrush(d_brush);

        p->drawEllipse(r);
	} else {
		ellipse.addEllipse(rect);
		p->fillPath(ellipse, palette().color(QPalette::Window));
		if (d_brush.style() != Qt::NoBrush)
			p->fillPath(ellipse, d_brush);
	}

	p->restore();
}
