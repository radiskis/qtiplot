/***************************************************************************
    File                 : LegendWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A 2D Plot Legend Widget

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
#include "LegendWidget.h"
#include "PlotCurve.h"
#include "QwtPieCurve.h"
#include "VectorCurve.h"
#include "SelectionMoveResizer.h"
#include "MultiLayer.h"
#include <ApplicationWindow.h>
#include <PenStyleBox.h>

#include <QPainter>
#include <QPaintEngine>
#include <QPolygon>
#include <QMessageBox>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_painter.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_layout_metrics.h>
#include <qwt_symbol.h>

#include <iostream>

LegendWidget::LegendWidget(Graph *plot):FrameWidget(plot),
d_angle(0),
h_space(5),
left_margin(10),
top_margin(5),
line_length(20),
d_auto_update(false)
{
	setAttribute(Qt::WA_DeleteOnClose);

	d_text = new QwtText(QString::null, QwtText::RichText);
	d_text->setFont(QFont("Arial", 12, QFont::Normal, false));
	d_text->setRenderFlags(Qt::AlignTop|Qt::AlignLeft);
	d_text->setBackgroundBrush(QBrush(Qt::NoBrush));
	d_text->setColor(Qt::black);
	d_text->setBackgroundPen (QPen(Qt::NoPen));
	d_text->setPaintAttribute(QwtText::PaintBackground);

	move(plot->mapToParent(plot->canvas()->pos() + QPoint(10, 10)));
	connect (this, SIGNAL(enableEditor()), plot, SLOT(enableTextEditor()));

	setMouseTracking(true);
	show();
	setFocus();
}

void LegendWidget::paintEvent(QPaintEvent *e)
{
	QPainter p(this);

	const int symbolLineLength = line_length + symbolsMaxWidth();
	int width, height, textWidth, textHeight;
	QwtArray<long> heights = itemsHeight(&p, symbolLineLength, width, height, textWidth, textHeight);
    resize(width, height);

	drawFrame(&p, rect());
	drawText(&p, QRect(0, 0, textWidth, textHeight), heights, symbolLineLength);
	e->accept();
}

void LegendWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	// calculate resolution factor
	double factor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	// save screen geometry parameters
	int space = h_space;
	int left = left_margin;
	int top = top_margin;

	h_space = int(h_space*factor);
	left_margin = int(left_margin*factor);
	top_margin = int(top_margin*factor);

	const int symbolLineLength = int((line_length + symbolsMaxWidth())*factor);
	int width, height, textWidth, textHeight;
	QwtArray<long> heights = itemsHeight(painter, symbolLineLength, width, height, textWidth, textHeight);

	drawFrame(painter, QRect(x, y, width, height));
	drawText(painter, QRect(x, y, textWidth, textHeight), heights, symbolLineLength);

	// restore screen geometry parameters
	h_space = space;
	left_margin = left;
	top_margin = top;
}

void LegendWidget::setText(const QString& s)
{
	d_text->setText(s);
}

void LegendWidget::setTextColor(const QColor& c)
{
	if ( c == d_text->color() )
		return;

	d_text->setColor(c);
}

void LegendWidget::setFont(const QFont& font)
{
	if ( font == d_text->font() )
		return;

	d_text->setFont(font);
}

void LegendWidget::drawVector(PlotCurve *c, QPainter *p, int x, int y, int l)
{
	if (!c)
		return;

	VectorCurve *v = (VectorCurve*)c;
	p->save();

	if (d_plot->antialiasing())
		p->setRenderHints(QPainter::Antialiasing);

	QPen pen(v->color(), v->width(), Qt::SolidLine);
	p->setPen(pen);
	p->drawLine(x, y, x + l, y);

	p->translate(x + l, y);

	double pi = 4*atan(-1.0);
	int headLength = v->headLength();
	int d = qRound(headLength*tan(pi*(double)v->headAngle()/180.0));

	QPolygon endArray(3);
	endArray[0] = QPoint(0, 0);
	endArray[1] = QPoint(-headLength, d);
	endArray[2] = QPoint(-headLength, -d);

	if (v->filledArrowHead())
		p->setBrush(QBrush(pen.color(), Qt::SolidPattern));

	p->drawPolygon(endArray);
	p->restore();
}

void LegendWidget::drawSymbol(PlotCurve *c, int point, QPainter *p, int x, int y, int l)
{
    if (!c || c->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
        return;

    if (c->type() == Graph::VectXYXY || c->type() == Graph::VectXYAM){
        drawVector(c, p, x, y, l);
        return;
    }

	if (c->type() == Graph::Pie){
		QwtPieCurve *pie = (QwtPieCurve *)c;
		const QBrush br = QBrush(pie->color(point), pie->pattern());
		QPen pen = pie->pen();
		p->save();
		p->setPen (QPen(pen.color(), pen.widthF(), Qt::SolidLine));
		QRect lr = QRect(x, y - l/4, l, l/2);
		p->setBrush(br);
		QwtPainter::drawRect(p, lr);
		p->restore();
		return;
	}

    QwtSymbol symb = c->symbol();
    const QBrush br = c->brush();
    QPen pen = c->pen();
    p->save();
    if (c->style()!=0){
        p->setPen (pen);
		if (c->type() == Graph::VerticalBars || c->type() == Graph::HorizontalBars ||
			c->type() == Graph::Histogram || c->type() == Graph::Box){
			QRect lr = QRect(x, y - l/4, l, l/2);
            p->setBrush(br);
            QwtPainter::drawRect(p, lr);
        } else
            QwtPainter::drawLine(p, x, y, x + l, y);
    }
    int symb_size = symb.size().width();
    if (symb_size > 15)
        symb_size = 15;
    else if (symb_size < 3)
        symb_size = 3;
    symb.setSize(symb_size);
    symb.draw(p, x + l/2, y);
    p->restore();
}

void LegendWidget::drawText(QPainter *p, const QRect& rect,
		QwtArray<long> height, int symbolLineLength)
{
	p->save();
	if (d_plot->antialiasing())
		p->setRenderHints(QPainter::Antialiasing);

	p->setRenderHint(QPainter::TextAntialiasing);

	p->translate(rect.x(), rect.y());

	int aux_a = d_angle;
	if (aux_a >= 270){
        aux_a -= 270;
		p->translate(rect.height()*cos(aux_a*M_PI/180.0), 0.0);
	} else if (aux_a >= 180){
        aux_a -= 180;
		p->translate(this->width(), rect.height()*cos(aux_a*M_PI/180.0));
	} else if (aux_a > 90){
        aux_a -= 90;
		p->translate(rect.width()*sin(aux_a*M_PI/180.0), this->height());
	}  else
		p->translate(0.0, rect.width()*sin(d_angle*M_PI/180.0));

    p->rotate(-d_angle);

	int l = symbolLineLength;
	QString text = d_text->text();
	QStringList titles = text.split("\n", QString::KeepEmptyParts);

	for (int i=0; i<(int)titles.count(); i++){
		int w = left_margin + d_frame_pen.width();
		bool  curveSymbol = false;
		QString s = titles[i];
		while (s.contains("\\l(") || s.contains("\\p{")){
			curveSymbol = true;
			int pos = s.indexOf("\\l(", 0);
			if (pos >= 0){
				QwtText aux(parse(s.left(pos)));
				aux.setFont(d_text->font());
				aux.setColor(d_text->color());
				aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter);

				QSize size = textSize(p, aux);
				QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
				aux.draw(p, tr);
				w += size.width();

				int pos1 = s.indexOf("(", pos);
				int pos2 = s.indexOf(")", pos1);
				int point = -1;
				PlotCurve *curve = getCurve(s.mid(pos1+1, pos2-pos1-1), point);
				if (!curve){
					s = s.right(s.length() - pos2 - 1);
					continue;
                }

            	drawSymbol(curve, point, p, w, height[i], l);
            	w += l + h_space;
            	s = s.right(s.length() - pos2 - 1);
			} else {
			    pos = s.indexOf("\\p{", 0);
                if (pos >= 0){
                    QwtText aux(parse(s.left(pos)));
                    aux.setFont(d_text->font());
                    aux.setColor(d_text->color());
					aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter);

					QSize size = textSize(p, aux);
                    QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
                    aux.draw(p, tr);
                    w += size.width();

                    int pos1 = s.indexOf("{", pos);
                    int pos2 = s.indexOf("}", pos1);
                    int point = s.mid(pos1 + 1, pos2 - pos1 - 1).toInt() - 1;
					drawSymbol((PlotCurve*)d_plot->curve(0), point, p, w, height[i], l);
                	w += l;
                	s = s.right(s.length() - pos2 - 1);
                }
			}
		}

		if (!s.isEmpty()){
			if (curveSymbol)
				w += h_space;
			QwtText aux(parse(s));
			aux.setFont(d_text->font());
			aux.setColor(d_text->color());
			aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter);

			QSize size = textSize(p, aux);
			QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
			aux.draw(p, tr);
		}
	}
	p->restore();
}

QwtArray<long> LegendWidget::itemsHeight(QPainter *p, int symbolLineLength, int &width, int &height,
							 int &textWidth, int &textHeight)
{
	QString text = d_text->text();
	QStringList titles = text.split("\n", QString::KeepEmptyParts);
	int n = (int)titles.count();
	QwtArray<long> heights(n);

	width = 0;
	height = 0;
	int maxL = 0;
	int h = top_margin + d_frame_pen.width();
	for (int i=0; i<n; i++){
		QString s = titles[i];
		int textL = 0;
		bool curveSymbol = false;
		while (s.contains("\\l(") || s.contains("\\p{")){
			int pos = s.indexOf("\\l(", 0);
			curveSymbol = true;
			if (pos >= 0){
				QwtText aux(parse(s.left(pos)));
				aux.setFont(d_text->font());
				QSize size = textSize(p, aux);
				textL += size.width();

				int pos1 = s.indexOf("(", pos);
				int pos2 = s.indexOf(")", pos1);
				int point = -1;
				PlotCurve *curve = getCurve(s.mid(pos1+1, pos2-pos1-1), point);
				if (!curve){
                	s = s.right(s.length() - pos2 - 1);
                    continue;
                }

                textL += symbolLineLength + h_space;
                s = s.right(s.length() - s.indexOf(")", pos) - 1);
            } else {
                pos = s.indexOf("\\p{", 0);
                if (pos >= 0){
                    QwtText aux(parse(s.left(pos)));
                    aux.setFont(d_text->font());
                    QSize size = textSize(p, aux);
                    textL += size.width();
                    textL += symbolLineLength;
                    s = s.right(s.length() - s.indexOf("}", pos) - 1);
                }
            }
		}

		QwtText aux(parse(s));
		aux.setFont(d_text->font());
		QSize size = textSize(p, aux);
		textL += size.width();
		if (curveSymbol)
			textL += h_space;

		if (textL > maxL)
			maxL = textL;

		int textH = size.height();
		height += textH;

		heights[i] = h + textH/2;
		h += textH;
	}

	height += 2*top_margin;
	width = 2*left_margin + maxL;

	int fw = 2*d_frame_pen.width();
    height += fw;
    width += fw;

    textHeight = height;
    textWidth = width;

    int aux_a = d_angle;
	if (aux_a > 270)
		aux_a -= 270;
    if (aux_a >= 180)
        aux_a -= 180;
    if (aux_a > 90)
        aux_a -= 90;

	double angle = aux_a*M_PI/180.0;
	if ((d_angle >= 0 && d_angle <= 90) || (d_angle >= 180 && d_angle <= 270)){
    	height = int(textWidth*sin(angle) + textHeight*cos(angle));
    	width = int(textWidth*cos(angle) + textHeight*sin(angle));
	} else {
    	height = int(textWidth*cos(angle) + textHeight*sin(angle));
    	width = int(textWidth*sin(angle) + textHeight*cos(angle));
	}

	if (d_frame == Shadow){
		width += qRound(d_shadow_width*p->device()->logicalDpiX()/(double)plot()->logicalDpiX());
		height += qRound(d_shadow_width*p->device()->logicalDpiY()/(double)plot()->logicalDpiY());
	}

	return heights;
}

int LegendWidget::symbolsMaxWidth()
{
	int curves = d_plot->curveCount();
	if (!curves)
		return 0;

	int maxL = 0;
	QString text = d_text->text();
	QStringList titles = text.split("\n", QString::KeepEmptyParts);
	for (int i=0; i<(int)titles.count(); i++){
		QString s = titles[i];
		while (s.contains("\\l(")){
			int pos = s.indexOf("\\l(", 0);
		    int pos1 = s.indexOf("(", pos);
            int pos2 = s.indexOf(")", pos1);
			int cv = s.mid(pos1+1, pos2-pos1-1).toInt()-1;
			if (cv < 0 || cv >= curves){
				s = s.right(s.length() - pos2 - 1);
				continue;
			}

			const QwtPlotCurve *c = (QwtPlotCurve *)d_plot->curve(cv);
			if (c && c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
				int l = c->symbol().size().width();
				if (l < 3)
  	            	l = 3;
  	            else if (l > 15)
  	            	l = 15;
  	            if (l>maxL && c->symbol().style() != QwtSymbol::NoSymbol)
					maxL = l;
			}
			s = s.right(s.length() - pos2 - 1);
		}

		if (titles[i].contains("\\p{"))
			maxL = 10;
	}
	return maxL;
}

QString LegendWidget::parse(const QString& str)
{
    QString s = str;
    s.remove(QRegExp("\\l(*)", Qt::CaseSensitive, QRegExp::Wildcard));
    s.remove(QRegExp("\\p{*}", Qt::CaseSensitive, QRegExp::Wildcard));

	QString aux = str;
    while (aux.contains(QRegExp("%(*)", Qt::CaseInsensitive, QRegExp::Wildcard))){//curve name specification
		int pos = str.indexOf("%(", 0, Qt::CaseInsensitive);
        int pos2 = str.indexOf(")", pos, Qt::CaseInsensitive);
		QString spec = str.mid(pos + 2, pos2 - pos - 2);
		QStringList lst = spec.split(",");
		if (!lst.isEmpty()){
        	int cv = lst[0].toInt() - 1;
        	if (d_plot && cv >= 0 && cv < d_plot->curveCount()){
				PlotCurve *c = (PlotCurve *)d_plot->curve(cv);
            	if (c){
					if (lst.count() == 1)
						s = s.replace(pos, pos2-pos+1, c->title().text());
					else if (lst.count() == 3 && c->type() == Graph::Pie){
						Table *t = ((DataCurve *)c)->table();
						int col = t->colIndex(c->title().text());
						int row = lst[2].toInt() - 1;
						s = s.replace(pos, pos2-pos+1, t->text(row, col));
					}
				}
        	}
			aux = aux.right(aux.length() - pos2 - 1);
		}
    }
    return s;
}

PlotCurve* LegendWidget::getCurve(const QString& s, int &point)
{
	point = -1;
	PlotCurve *curve = 0;

	QStringList l = s.split(",");
    if (l.count() == 2)
		point = l[1].toInt() - 1;

	if (!l.isEmpty()){
		l = l[0].split(".");
    	if (l.count() == 2){
    		int cv = l[1].toInt() - 1;
			Graph *layer = d_plot->multiLayer()->layer(l[0].toInt());
			if (layer && cv >= 0 && cv < layer->curveCount())
				return (PlotCurve*)layer->curve(cv);
		} else if (l.count() == 1){
			int cv = l[0].toInt() - 1;
			if (cv >= 0 || cv < d_plot->curveCount())
				return (PlotCurve*)d_plot->curve(cv);
		}
	}
	return curve;
}

void LegendWidget::showTextEditor()
{
    ApplicationWindow *app = d_plot->multiLayer()->applicationWindow();
    if (!app)
        return;

	d_plot->deselect(this);

	if (app->d_in_place_editing)
        enableEditor();
    else
        showDialog();
}

void LegendWidget::clone(LegendWidget* t)
{
	d_frame = t->frameStyle();
	d_angle = t->angle();
	setFramePen(t->framePen());
	d_auto_update = t->isAutoUpdateEnabled();

	setTextColor(t->textColor());
	setBackgroundColor(t->backgroundColor());
	setFont(t->font());
	setText(t->text());
	setOriginCoord(t->xValue(), t->yValue());
}

QString LegendWidget::saveToString()
{
	QString s = "<Legend>\n";
	s += FrameWidget::saveToString();
	s += "<Text>\n" + d_text->text() + "\n</Text>\n";

	QFont f = d_text->font();
	s += "<Font>" + f.family() + "\t";
	s += QString::number(f.pointSize())+"\t";
	s += QString::number(f.weight())+"\t";
	s += QString::number(f.italic())+"\t";
	s += QString::number(f.underline())+"\t";
	s += QString::number(f.strikeOut())+"</Font>\n";

	s += "<TextColor>" + d_text->color().name()+"</TextColor>\n";
	QColor bc = backgroundColor();
	s += "<Background>" + bc.name() + "</Background>\n";
	s += "<Alpha>" + QString::number(bc.alpha()) + "</Alpha>\n";
	s += "<Angle>" + QString::number(d_angle) + "</Angle>\n";
	s += "<AutoUpdate>" + QString::number(d_auto_update) + "</AutoUpdate>\n";
	return s + "</Legend>\n";
}

void LegendWidget::restore(Graph *g, const QStringList& lst)
{
	QColor backgroundColor = Qt::white;
	double x = 0.0, y = 0.0;
	QStringList::const_iterator line;
	LegendWidget *l = new LegendWidget(g);
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Frame>"))
			l->setFrameStyle((s.remove("<Frame>").remove("</Frame>").toInt()));
		else if (s.contains("<Color>"))
			l->setFrameColor(QColor(s.remove("<Color>").remove("</Color>")));
		else if (s.contains("<FrameWidth>"))
			l->setFrameWidth(s.remove("<FrameWidth>").remove("</FrameWidth>").toInt());
		else if (s.contains("<LineStyle>"))
			l->setFrameLineStyle(PenStyleBox::penStyle(s.remove("<LineStyle>").remove("</LineStyle>").toInt()));
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
			l->setText(txt.join("\n"));
		} else if (s.contains("<Font>")){
			QStringList lst = s.remove("<Font>").remove("</Font>").split("\t");
			QFont f = QFont(lst[0], lst[1].toInt(), lst[2].toInt(), lst[3].toInt());
			f.setUnderline(lst[4].toInt());
			f.setStrikeOut(lst[5].toInt());
			l->setFont(f);
		} else if (s.contains("<TextColor>"))
			l->setTextColor(QColor(s.remove("<TextColor>").remove("</TextColor>")));
		else if (s.contains("<Background>"))
			backgroundColor = QColor(s.remove("<Background>").remove("</Background>"));
		else if (s.contains("<Alpha>"))
			backgroundColor.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
		else if (s.contains("<Angle>"))
			l->setAngle(s.remove("<Angle>").remove("</Angle>").toInt());
		else if (s.contains("<AutoUpdate>"))
			l->setAutoUpdate(s.remove("<AutoUpdate>").remove("</AutoUpdate>").toInt());
	}
	if (l){
		l->setBackgroundColor(backgroundColor);
		l->setOriginCoord(x, y);
		g->add(l, false);
	}
}

void LegendWidget::setAngle(int angle)
{
    if (d_angle == angle)
        return;

    d_angle = angle;
	if (d_angle < 0)
		d_angle += 360;
	else if (d_angle >= 360)
		d_angle -= 360;
}

/*
  // bug in Qwt; workaround in QwtText::textSize() doesn't work.
*/
QSize LegendWidget::textSize(QPainter *p, const QwtText& text)
{
	QString s = text.text();
	s.remove("<sub>").remove("</sub>").remove("<sup>").remove("</sup>");
	QFontMetrics fm(text.font(), p->device());
	return fm.boundingRect(s).size();
}

LegendWidget::~LegendWidget()
{
	delete d_text;
}
