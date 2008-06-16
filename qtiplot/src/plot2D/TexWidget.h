/***************************************************************************
    File                 : TexWidget.h
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
#ifndef TEXWIDGET_H
#define TEXWIDGET_H

#include <QWidget>
#include "FrameWidget.h"

class TexWidget: public FrameWidget
{
	Q_OBJECT

public:
    TexWidget(Graph *, const QString& s = QString::null, const QPixmap& pix = QPixmap());

	//! Return the pixmap to be drawn.
	QPixmap pixmap() const {return d_pix;};
	void setPixmap(const QPixmap&);
	
	QString formula(){return d_formula;};
	void setFormula(const QString& s){d_formula = s;};
	
	void print(QPainter *p, const QwtScaleMap map[QwtPlot::axisCnt]);
	virtual QString saveToString();
	
	void clone(TexWidget* t);
	static void restore(Graph *g, const QStringList& lst);
	
	void setBestSize();

private:
	void paintEvent(QPaintEvent *e);
	QPixmap d_pix;
	QString d_formula;
	int d_margin;
};

#endif
