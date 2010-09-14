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
#include <LegendWidget.h>
#include <PieCurve.h>
#include <MultiLayer.h>

#include <QTextCursor>
#include <QMessageBox>

#include <qwt_text.h>
#include <qwt_text_label.h>
#include <qwt_scale_widget.h>

TextEditor::TextEditor(Graph *g): QTextEdit(g), d_graph(g)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFrameShadow(QFrame::Plain);
	setFrameShape(QFrame::Box);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Active, QPalette::WindowText, Qt::blue);
	palette.setColor(QPalette::Active, QPalette::Base, Qt::white);
	setPalette(palette);

	bool moveCrs = true;
	QString text;
	if (g->activeText()){
		setParent(g->multiLayer()->canvas());
		d_target = g->activeText();
		setGeometry(d_target->geometry());
		text = ((LegendWidget*)d_target)->text();
		d_target->hide();
		setFont(((LegendWidget*)d_target)->font());
	} else if (g->titleSelected()){
		d_target = g->titleLabel();
		QwtText t = g->title();
		text = t.text();
		setAlignment((Qt::Alignment)t.renderFlags());
		setFont(t.font());
		setGeometry(d_target->geometry());
	} else if (g->selectedScale()){
		d_target = g->selectedScale();
		QwtScaleWidget *scale = (QwtScaleWidget*)d_target;
		QwtText t = scale->title();

		int axis = -1;
		switch(scale->alignment()){
			case QwtScaleDraw::BottomScale:
				axis = QwtPlot::xBottom;
			break;
			case QwtScaleDraw::LeftScale:
				axis = QwtPlot::yLeft;
			break;
			case QwtScaleDraw::TopScale:
				axis = QwtPlot::xTop;
			break;
			case QwtScaleDraw::RightScale:
				axis = QwtPlot::yRight;
			break;
		}
		text = g->axisTitleString(axis);

		setAlignment((Qt::Alignment)t.renderFlags());
		setFont(t.font());

		QRect rect = g->axisTitleRect(scale);
		if (scale->alignment() == QwtScaleDraw::BottomScale ||
			scale->alignment() == QwtScaleDraw::TopScale){
			resize(rect.size());
			move(QPoint(d_target->x() + rect.x(), d_target->y() + rect.y()));
		} else {
			resize(QSize(rect.height(), rect.width()));
			if (scale->alignment() == QwtScaleDraw::LeftScale)
                move(QPoint(d_target->x() + rect.x(), d_target->y() + rect.y() + rect.height()/2));
            else if (scale->alignment() == QwtScaleDraw::RightScale)
                move(QPoint(d_target->x() - rect.height(), d_target->y() + rect.y() + rect.height()/2));

			t.setText(" ");
			t.setBackgroundPen(QPen(Qt::NoPen));
			scale->setTitle(t);
			moveCrs = false;
		}
	}

	QTextCursor cursor = textCursor();
	cursor.insertText(text);
	d_initial_text = text;

	setWordWrapMode (QTextOption::NoWrap);
	setAlignment(Qt::AlignCenter);

	QTextFrame *frame = document()->rootFrame();
	QTextFrameFormat format = frame->frameFormat();
	format.setTopMargin(format.topMargin () + 3);
	frame->setFrameFormat(format);
	show();

	if (moveCrs)
		setTextCursor(cursorForPosition(mapFromGlobal(QCursor::pos())));
	setFocus();
}

void TextEditor::closeEvent(QCloseEvent *e)
{
	QString s = text();
	int lines = s.count(QRegExp("\n"));
	if ( lines >= 100 && QMessageBox::question(d_graph->multiLayer(),
		tr("QtiPlot") + " - " + tr("Confirmation"),
		tr("Are you sure you want to add %1 text lines into this text box?").arg(lines),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
		s = d_initial_text;

	if (d_target->isA("LegendWidget")){
		((LegendWidget*)d_target)->setText(s);
        d_target->show();
		d_graph->setActiveText(NULL);
	} else if (d_target->isA("PieLabel")){
		((PieLabel*)d_target)->setCustomText(s);
        d_target->show();
		d_graph->setActiveText(NULL);
	} else if (d_target->isA("QwtTextLabel")){
		QwtText title = d_graph->title();
		if(s.isEmpty())
			s = " ";
		title.setText(s);
		d_graph->setTitle(title);
	} else if (d_target->isA("QwtScaleWidget")){
		if(s.isEmpty())
			s = " ";

		int axis = -1;
		QwtScaleWidget *scale = (QwtScaleWidget*)d_target;
		switch(scale->alignment()){
			case QwtScaleDraw::BottomScale:
				axis = QwtPlot::xBottom;
			break;
			case QwtScaleDraw::LeftScale:
				axis = QwtPlot::yLeft;
			break;
			case QwtScaleDraw::TopScale:
				axis = QwtPlot::xTop;
			break;
			case QwtScaleDraw::RightScale:
				axis = QwtPlot::yRight;
			break;
		}
		d_graph->setAxisTitle(axis, s);
	}

	if (d_initial_text != s)
		d_graph->notifyChanges();

    d_target->repaint();
	e->accept();
}

void TextEditor::formatText(const QString& prefix, const QString& postfix)
{
	QTextCursor cursor = textCursor();
	QString markedText = cursor.selectedText();
	cursor.insertText(prefix + markedText + postfix);
	if(markedText.isEmpty()){
		cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, postfix.size());
		setTextCursor(cursor);
	}
	setFocus();
}

void TextEditor::addSymbol(const QString& letter)
{
	textCursor().insertText(letter);
}
