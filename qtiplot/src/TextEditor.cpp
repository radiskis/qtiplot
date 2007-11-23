/***************************************************************************
    File                 : TextEditor.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A QwtText editor 

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
#include "TextEditor.h"
#include "LegendWidget.h"

#include <QTextCursor>

#include <qwt_text.h>
#include <qwt_text_label.h>
#include <qwt_scale_widget.h>

TextEditor::TextEditor(Graph *g): QTextEdit(g)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFrameShadow ( QFrame::Plain );
	setFrameShape ( QFrame::Box );
	
	QString text;
	if (g->selectedText()){
		d_target = g->selectedText();
		setGeometry(d_target->geometry());
		text = ((LegendWidget*)d_target)->text();
	} else if (g->titleSelected()){
		d_target = g->plotWidget()->titleLabel();
		QwtText t = g->plotWidget()->title();
		text = t.text();
		setAlignment((Qt::Alignment)t.renderFlags());
		setGeometry(d_target->geometry());
	} else if (g->selectedScale()){
		d_target = g->selectedScale();
		QwtScaleWidget *scale = (QwtScaleWidget*)d_target;
		QwtText t = scale->title();
		text = t.text();
		setAlignment((Qt::Alignment)t.renderFlags());
		
		QRect rect = g->axisTitleRect(scale);
		if (scale->alignment() == QwtScaleDraw::BottomScale ||
			scale->alignment() == QwtScaleDraw::TopScale){
			resize(rect.size());
			move(QPoint(d_target->x() + rect.x(), d_target->y() + rect.y()));
		} else {
			resize(QSize(rect.height(), rect.width()));
			move(QPoint(d_target->x() + rect.x(), d_target->y() + rect.y() + rect.height()/2));
		}
	}
			
	QTextCursor cursor = textCursor();
	cursor.insertText(text);
		
	show();
	setFocus();
}

void TextEditor::closeEvent(QCloseEvent *e)
{	
	if (d_target->isA("LegendWidget")){
		((LegendWidget*)d_target)->setText(text());
		((Graph *)parent())->setSelectedText(NULL);
	} else if (d_target->isA("QwtTextLabel")){
		Graph *g = (Graph *)parent();
		QwtText title = g->plotWidget()->title();
		title.setText(text());
		g->plotWidget()->setTitle(title);
		d_target->repaint();
	} else if (d_target->isA("QwtScaleWidget")){
		QwtScaleWidget *scale = (QwtScaleWidget*)d_target;
		QwtText title = scale->title();
		title.setText(text());
		scale->setTitle(title);		
		d_target->repaint();
	}
	
	e->accept();
}
