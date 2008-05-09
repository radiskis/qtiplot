/***************************************************************************
    File                 : LineNumberDisplay.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying line numbers for a QTextEdit

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
#include "LineNumberDisplay.h"
#include <QScrollBar>

LineNumberDisplay::LineNumberDisplay(QTextEdit *te, QWidget *parent)
		 : QTextEdit(parent), d_text_edit(te)
{
	setReadOnly(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(QFrame::Panel | QFrame::Raised);
	setMaximumWidth(0);
	setLineWidth(0);
	setFocusPolicy(Qt::NoFocus);

	if (te){
		connect(te, SIGNAL(textChanged()), this, SLOT(updateLineNumbers()));
		connect((QObject *)te->verticalScrollBar(), SIGNAL(valueChanged(int)), 
			(QObject *)verticalScrollBar(), SLOT(setValue(int)));
	}
}

void LineNumberDisplay::updateLineNumbers()
{
	if (!isVisible() || !d_text_edit)
		return;
	
	int lines = d_text_edit->document()->blockCount();
	if (document()->blockCount() - 1 == lines)
		return;
		
	QString aux;
	for(int i = 0; i < lines; i++)
		aux += QString::number(i + 1) + "\n";
	
	setPlainText(aux);
	
	QFontMetrics fm(font(), this);
	setMaximumWidth(2*fm.boundingRect(QString::number(lines)).width());
	verticalScrollBar()->setValue(d_text_edit->verticalScrollBar()->value());
}

void LineNumberDisplay::showEvent(QShowEvent *e)
{
	e->accept();
	if (isVisible())
		updateLineNumbers();
}
