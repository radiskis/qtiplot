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
#include "PieCurve.h"
#include "VectorCurve.h"
#include "SelectionMoveResizer.h"
#include "MultiLayer.h"
#include <ApplicationWindow.h>
#include <PenStyleBox.h>

#ifdef TEX_OUTPUT
	#include <QTeXEngine.h>
#endif

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
line_length(32),
d_auto_update(false),
d_tex_output(false)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setObjectName(tr("Legend"));

	d_text = new QwtText(QString::null);
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
	QwtArray<long> heights = itemsHeight(&p, symbolLineLength, d_frame_pen.width(), width, height, textWidth, textHeight);
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
	double xfactor = (double)painter->device()->logicalDpiX()/(double)plot()->logicalDpiX();
	double yfactor = (double)painter->device()->logicalDpiY()/(double)plot()->logicalDpiY();

	// save screen geometry parameters
	int space = h_space;
	int left = left_margin;
	int top = top_margin;

	h_space = int(h_space*xfactor);
	left_margin = int(left_margin*xfactor);
	top_margin = int(top_margin*yfactor);

	const int dfy = qRound(d_frame_pen.width()*yfactor);
	const int symbolLineLength = int((line_length + symbolsMaxWidth())*xfactor);
	int width, height, textWidth, textHeight;
	QwtArray<long> heights = itemsHeight(painter, symbolLineLength, dfy, width, height, textWidth, textHeight);

#ifdef TEX_OUTPUT
	if (plot()->isExportingTeX()){
		drawFrame(painter, QRect(x, y, qRound(this->width()*xfactor), height));
		((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignLeft);
	} else
#endif
	drawFrame(painter, QRect(x, y, width, height));

	drawText(painter, QRect(x, y, textWidth, textHeight), heights, symbolLineLength);

#ifdef TEX_OUTPUT
	if (plot()->isExportingTeX())
		((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignHCenter);
#endif

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

	QPen pen = v->vectorPen();
	pen.setCosmetic(false);
	p->setPen(QwtPainter::scaledPen(pen));
	p->drawLine(x, y, x + l, y);

	p->translate(x + l, y);

	double pi = 4*atan(-1.0);
	int headLength = qRound(v->headLength()*(double)p->device()->logicalDpiX()/(double)d_plot->logicalDpiX());
	int d = qRound(headLength*tan(pi*(double)v->headAngle()/180.0));

	QPolygon endArray(3);
	endArray[0] = QPoint(0, 0);
	endArray[1] = QPoint(-headLength, d);
	endArray[2] = QPoint(-headLength, -d);

	if (v->filledArrowHead())
		p->setBrush(QBrush(v->color(), Qt::SolidPattern));

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
		PieCurve *pie = (PieCurve *)c;
		const QBrush br = QBrush(pie->color(point), pie->pattern());
		QPen pen = pie->pen();
		pen.setCosmetic(false);
		pen = QwtPainter::scaledPen(pen);

		p->save();
		p->setPen (QPen(pen.color(), pen.widthF(), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
		QRect lr = QRect(x, y - l/4, l, l/2);
		p->setBrush(br);
		QwtPainter::drawRect(p, lr);
		p->restore();
		return;
	}

	p->save();
	if (c->style() != 0){
		QPen pen = c->pen();
		pen.setCosmetic(false);
		p->setPen (QwtPainter::scaledPen(pen));
		if (c->type() == Graph::VerticalBars || c->type() == Graph::HorizontalBars ||
			c->type() == Graph::Histogram || c->type() == Graph::Box){
			QRect lr = QRect(x, y - l/4, l, l/2);
			p->setBrush(c->brush());
			QwtPainter::drawRect(p, lr);

			p->restore();
			return;
		} else
			QwtPainter::drawLine(p, x, y, x + l, y);
	}

	if (c->symbol().pen().style() != Qt::NoPen){
		QwtSymbol symb = c->symbol();
		int symb_size = symb.size().width();
		if (symb_size > 15)
			symb_size = 15;
		else if (symb_size < 3)
			symb_size = 3;
		symb.setSize(symb_size);

		QPen pen = symb.pen();
		pen.setCosmetic(false);
		symb.setPen(pen);

		symb.draw(p, x + l/2, y);
	} else //ImageSymbol ?
		c->symbol().draw(p, x + l/2, y);
	p->restore();
}

void LegendWidget::drawText(QPainter *p, const QRect& rect, QwtArray<long> height, int symbolLineLength)
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

	for (int i = 0; i < titles.count(); i++){
		int w = left_margin + d_frame_pen.width();
		bool  curveSymbol = false;
		QString s = titles[i];
		while (s.contains("\\l(",Qt::CaseInsensitive) || s.contains("\\p{",Qt::CaseInsensitive)){
			curveSymbol = true;
			int pos = s.indexOf("\\l(", 0,Qt::CaseInsensitive);
			int pos2 = s.indexOf(",",pos); // two arguments in case if pie chart
			int pos3 = s.indexOf(")",pos);
			if (pos >= 0 && (pos2 == -1 || pos2 > pos3)){
				QwtText aux(parse(s.left(pos))); //not a pie chart
				aux.setFont(d_text->font());
				aux.setColor(d_text->color());
				aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);

				QSize size = textSize(p, aux);
				QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
				aux.draw(p, tr);
				w += size.width();

				int pos1 = s.indexOf("(", pos);
				int pos2 = s.indexOf(")", pos1);
                if (pos2 == -1){
                     s = s.right(s.length() - pos1 - 1);
				     continue;
                }
				int point = -1;
				PlotCurve *curve = getCurve(s.mid(pos1+1, pos2-pos1-1), point);
				if (!curve){
					s = s.right(s.length() - pos2 - 1);
					continue;
                }

            	drawSymbol(curve, point, p, w, height[i], l);
            	w += l + h_space;
            	s = s.right(s.length() - pos2 - 1);
			} else { // pie chart?
				pos = s.indexOf("\\p{", 0);
                if (pos >= 0){  // old syntax
                    QwtText aux(parse(s.left(pos)));
                    aux.setFont(d_text->font());
                    aux.setColor(d_text->color());
					aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);

					QSize size = textSize(p, aux);
                    QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
                    aux.draw(p, tr);
                    w += size.width();

                    int pos1 = s.indexOf("{", pos);
                    int pos2 = s.indexOf("}", pos1);
                    if (pos2 == -1){
				         s = s.right(s.length() - pos1 - 1);
				         continue;
                    }
                    int point = s.mid(pos1 + 1, pos2 - pos1 - 1).toInt() - 1;
					drawSymbol((PlotCurve*)d_plot->curve(0), point, p, w, height[i], l);
                	w += l;
                	s = s.right(s.length() - pos2 - 1);
                } else {
					pos = s.indexOf("\\l(", 0,Qt::CaseInsensitive);
					if (pos >= 0 && pos2 != -1){ //new syntax
						QwtText aux(parse(s.left(pos)));
						aux.setFont(d_text->font());
						aux.setColor(d_text->color());
						aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);

						QSize size = textSize(p, aux);
						QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
						aux.draw(p, tr);
						w += size.width();

						int pos1 = s.indexOf(",", pos);
						int pos3 = s.indexOf(")", pos1);
						if (pos3 == -1){
							 s = s.right(s.length() - pos1 - 1);
							 continue;
						}
						int point = s.mid(pos1 + 1, pos3 - pos1 - 1).toInt() - 1;
						drawSymbol((PlotCurve*)d_plot->curve(0), point, p, w, height[i], l);
						w += l;
						s = s.right(s.length() - pos3 - 1);
					}
				}
			}
		}

		if (!s.isEmpty()){
			if (curveSymbol)
				w += h_space;
			QwtText aux(parse(s));
			aux.setFont(d_text->font());
			aux.setColor(d_text->color());
			aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);

			QSize size = textSize(p, aux);
			QRect tr = QRect(QPoint(w, height[i] - size.height()/2), size);
			aux.draw(p, tr);
		}
	}
	p->restore();
}

QwtArray<long> LegendWidget::itemsHeight(QPainter *p, int symbolLineLength, int frameWidth, int &width, int &height,
							 int &textWidth, int &textHeight)
{
	QString text = d_text->text();
	QStringList titles = text.split("\n", QString::KeepEmptyParts);
	int n = (int)titles.count();
	QwtArray<long> heights(n);

	width = 0;
	height = 0;
	int maxL = 0;
	int h = top_margin + frameWidth;
	for (int i=0; i<n; i++){
		QString s = titles[i];
		int textL = 0;
		bool curveSymbol = false;
		while (s.contains("\\l(",Qt::CaseInsensitive) || s.contains("\\p{",Qt::CaseInsensitive)){
			int pos = s.indexOf("\\l(", 0,Qt::CaseInsensitive);
			int pos2 = s.indexOf(",",pos); // two arguments in case if pie chart
			int pos3 = s.indexOf(")",pos);
			curveSymbol = true;
			if (pos >= 0 && (pos2==-1 || pos2>pos3)){
				QwtText aux(parse(s.left(pos))); //not a pie chart
				aux.setFont(d_text->font());
				aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);
				QSize size = textSize(p, aux);
				textL += size.width();

				int pos1 = s.indexOf("(", pos);
				int pos2 = s.indexOf(")", pos1);
                if (pos2 == -1){
				    s = s.right(s.length() - pos1 - 1);
				    continue;
                }
				int point = -1;
				PlotCurve *curve = getCurve(s.mid(pos1+1, pos2-pos1-1), point);
				if (!curve){
                	s = s.right(s.length() - pos2 - 1);
                    continue;
                }

                textL += symbolLineLength + h_space;
                s = s.right(s.length() - s.indexOf(")", pos) - 1);
            } else { //Pie chart?
                pos = s.indexOf("\\p{", 0,Qt::CaseInsensitive); //look for old syntax
                if (pos >= 0){
                    QwtText aux(parse(s.left(pos)));
                    aux.setFont(d_text->font());
					aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);
                    QSize size = textSize(p, aux);
                    textL += size.width();
                    textL += symbolLineLength;
					int pos2=s.indexOf("}", pos);
					if (pos2==-1) pos2=pos+3;
					s = s.right(s.length() - pos2 - 1);
                } else {
					pos = s.indexOf("\\l(", 0,Qt::CaseInsensitive); // new syntax
					if (pos >= 0){
						QwtText aux(parse(s.left(pos)));
						aux.setFont(d_text->font());
						aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);
						QSize size = textSize(p, aux);
						textL += size.width();
						textL += symbolLineLength;
						int pos2=s.indexOf(")", pos);
						if (pos2==-1) pos2=pos+3;
						s = s.right(s.length() - pos2 - 1);
					}
				}
            }
		}

		QwtText aux(parse(s));
		aux.setFont(d_text->font());
		aux.setRenderFlags (Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs);
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

	int fw = 2*frameWidth;
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
		while (s.contains("\\l(",Qt::CaseInsensitive)){
			int pos = s.indexOf("\\l(", 0,Qt::CaseInsensitive);
		    int pos1 = s.indexOf("(", pos);
            int pos2 = s.indexOf(")", pos1);
			int pos3 = s.indexOf(",",pos1);
			if (pos3 != -1 && pos3 < pos2 ) pos2=pos3; // for pi charts get dataset number
            if (pos2 == -1){
				s = s.right(s.length() - pos1 - 1);
				continue;
            }

			int point = 0;
			PlotCurve* c = getCurve(s.mid(pos1 + 1, pos2 - pos1 - 1), point);
			if (c && c->type() == Graph::Pie){
				maxL = 2*d_text->font().pointSize();//10;
				line_length = 0;
				s = s.right(s.length() - pos2 - 1);
				continue;
			}

			if (c && c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
				if (c->type() == Graph::Pie ||
					c->type() == Graph::VerticalBars ||
					c->type() == Graph::HorizontalBars ||
					c->type() == Graph::Histogram ||
					c->type() == Graph::Box){
					maxL = 2*d_text->font().pointSize();//10;
					line_length = 0;
				} else {
					int l = c->symbol().size().width();
					if (l < 3)
						l = 3;
					else if (l > 15)
						l = 15;
					if (l>maxL && c->symbol().style() != QwtSymbol::NoSymbol)
						maxL = l;
				}
			}
			s = s.right(s.length() - pos2 - 1);
		}

		if (titles[i].contains("\\p{")){ // old syntax for pie charts
			maxL = 2*d_text->font().pointSize();//10;
			line_length = 0;
		}
	}
	return maxL;
}

QString LegendWidget::parse(const QString& str)
{
    QString s = str;
    s.remove(QRegExp("\\l(*)", Qt::CaseInsensitive, QRegExp::Wildcard));
    s.remove(QRegExp("\\p{*}", Qt::CaseInsensitive, QRegExp::Wildcard));

	QString ltag[] = {"<b>","<i>","<u>","<sup>","<sub>"};
	QString rtag[] = {"</b>","</i>","</u>","</sup>","</sub>"};
	for (int i = 0; i < 5; i++){//remove special tags if they are not paired
		if (s.count(ltag[i]) != s.count(rtag[i]))
			s.remove(QRegExp(ltag[i])).remove(QRegExp(rtag[i]));
	}

	QString aux = str;
    while (aux.contains(QRegExp("%(*)", Qt::CaseInsensitive, QRegExp::Wildcard))){//curve name specification
		int pos = str.indexOf("%(", 0, Qt::CaseInsensitive);
        int pos2 = str.indexOf(")", pos, Qt::CaseInsensitive);
		QString spec = str.mid(pos + 2, pos2 - pos - 2);
		QStringList lst = spec.split(",");
		if (!lst.isEmpty()){
			int lcmd = -1;
			if (lst.count() >= 2){
				if (lst[1].contains("@d", Qt::CaseInsensitive)) //look for d cmd (use curve name), default
					lcmd = 0;
				else if (lst[1].contains("@c", Qt::CaseInsensitive)) //look for c cmd (use column name)
					lcmd = 1;
				else if (lst[1].contains("@wl", Qt::CaseInsensitive)) //look for w cmd (use table name)
					lcmd = 3;
				else if (lst[1].contains("@w", Qt::CaseInsensitive)) //look for wl cmd (use table label)
					lcmd = 2;
				else if (lst[1].contains("@l", Qt::CaseInsensitive)) //look for l cmd (use column comment)
					lcmd = 4;
				else if (lst[1].contains("@u", Qt::CaseInsensitive)) //look for c cmd (use column comment)
					lcmd = 5;
			}

			int point = 0;
			PlotCurve *c = getCurve(lst[0], point);
			if (c){
				Table *t = 0;
				QString colLabel;
				int ycol = -1;
				if (c->type() == Graph::Function || c->type() == Graph::Histogram)
					s = s.replace(pos, pos2 - pos + 1, c->title().text());
				else {
					t = ((DataCurve *)c)->table();
					ycol = t->colIndex(c->title().text());
					colLabel = t->colLabel(ycol);
				}

				if (t){
					switch(lcmd){
						case -1: //use column name
						{
							if (ycol >= 0 && !t->comment(ycol).isEmpty())
								s = s.replace(pos, pos2-pos+1, t->comment(ycol).replace("\n", " "));
							else
								s = s.replace(pos, pos2-pos+1, colLabel);
							break;
						}
						case 0: //use curve title
						{
							s = s.replace(pos, pos2-pos+1, c->title().text());
							break;
						}
						case 1: //use col label
						{
							if (ycol >= 0)
								s = s.replace(pos, pos2-pos+1, colLabel);
							break;
						}
						case 2: //use table name
						{
							s = s.replace(pos, pos2-pos+1, t->name());
							break;
						}
						case 3: // table label
							if (!t->windowLabel().isEmpty())
								s = s.replace(pos, pos2-pos+1, t->windowLabel());
							else
								s = s.replace(pos, pos2-pos+1, colLabel);
							break;
						case 5: //not implemented yet, use case 4 (L) instead
						case 4: //use col comment for 2 arguments, use cell contents for 3 arguments
						{
							switch (lst.count()){
								case 2: //2 arguments
								{
									s = s.replace(pos, pos2-pos+1, t->comment(ycol).replace("\n", " "));
									break;
								}
								case 3:  //3 arguments, display cell contents
								{
									int row = lst[2].toInt() - 1;
									s = s.replace(pos, pos2-pos+1, t->text(((DataCurve *)c)->tableRow(row), ycol));
									break;
								}
								case 4:  //4 arguments, display cell contents
								{
									int col = lst[3].toInt() - 1;
									int row = lst[2].toInt() - 1;
									s = s.replace(pos, pos2-pos+1, t->text(row, col));
									break;
								}
							}
							break;
						}
					}
				}
			} else {
				Table *t = d_plot->multiLayer()->applicationWindow()->table(lst[0]);
				if (t && lst.size() >= 3){
					int col = lst[1].toInt() - 1;
					int row = lst[2].toInt() - 1;
					s = s.replace(pos, pos2-pos+1, t->text(row, col));
				}
			}
			aux = aux.right(aux.length() - pos2 - 1);
		}
    }

#ifdef TEX_OUTPUT
	if (plot()->isExportingTeX()){
		if (!d_tex_output && plot()->escapeTeXStrings())
			s = Graph::escapeTeXSpecialCharacters(s);

		s = Graph::texSuperscripts(s);
	}
#endif

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
	d_tex_output = t->hasTeXOutput();

	setTextColor(t->textColor());
	setBackgroundColor(t->backgroundColor());
	setFont(t->font());
	setText(t->text());
	setOriginCoord(t->xValue(), t->yValue());
	setAttachPolicy(t->attachPolicy());
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
	s += "<TeXOutput>" + QString::number(d_tex_output) + "</TeXOutput>\n";
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
		else if (s.contains("<attachTo>"))
			l->setAttachPolicy((FrameWidget::AttachPolicy)s.remove("<attachTo>").remove("</attachTo>").toInt());
		else if (s.contains("<onTop>"))
			l->setOnTop(s.remove("<onTop>").remove("</onTop>").toInt());
		else if (s.contains("<visible>"))
			l->setVisible(s.remove("<visible>").remove("</visible>").toInt());
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
		else if (s.contains("<TeXOutput>"))
			l->setTeXOutput(s.remove("<TeXOutput>").remove("</TeXOutput>").toInt());
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
  // bug in Qwt; workaround in QwtText::textSize() doesn't work, the returned value is still too small.
*/
QSize LegendWidget::textSize(QPainter *p, const QwtText& text)
{
	QSize size = text.textSize(text.font());
	QwtMetricsMap map;
	map.setMetrics(this, p->device());
	if (!map.isIdentity()){
		QString s = text.text();
		if (s.contains("<sub>") || s.contains("<sup>")){
			int width = size.width() + QFontMetrics(text.font(), p->device()).boundingRect(" ").width();
			size =  QSize(width, size.height());
		} else
			size = QFontMetrics(text.font(), p->device()).boundingRect(s + "  ").size();
	}
	return size;
}

LegendWidget::~LegendWidget()
{
	delete d_text;
}
