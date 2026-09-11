/***************************************************************************
    File                 : TitlePicker.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Title picker

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
#include "TitlePicker.h"
#include "Graph.h"

#include <qwt_text_label.h>

#include <QMouseEvent>
#include <QPen>

TitlePicker::TitlePicker(Graph *plot):
	QObject(plot)
{
    d_selected = false;
	title = plot ? plot->titleLabel() : nullptr;
	if (title){
		title->setFocusPolicy(Qt::StrongFocus);
		title->installEventFilter(this);
	}
}

bool TitlePicker::eventFilter(QObject *object, QEvent *e)
{
	if (object != title)
		return false;

    if (e->type() == QEvent::MouseButtonDblClick){
        emit doubleClicked();
		d_selected = true;
        return true;
    }

	if (e->type() == QEvent::MouseButtonPress){
		 const QMouseEvent *me = static_cast<const QMouseEvent *>(e);

		 emit clicked();

		 if (me->button() == Qt::RightButton)
			emit showTitleMenu();
         else if (me->button() == Qt::LeftButton)
		 	setSelected();

		 return !(me->modifiers() & Qt::ShiftModifier);
    }

	if (e->type() == QEvent::KeyPress){
		const QKeyEvent *ke = static_cast<const QKeyEvent *>(e);
		switch (ke->key())
			{
			case Qt::Key_Delete:
			emit removeTitle();
            return true;
			}
		}

    return QObject::eventFilter(object, e);
}

void TitlePicker::setSelected(bool select)
{
    if (!title || d_selected == select)
        return;

    d_selected = select;

    QwtText text = title->text();
    if (select)
        text.setBorderPen(QPen(Qt::blue));
    else
        text.setBorderPen(QPen(Qt::NoPen));

    if (QwtPlot *p = qobject_cast<QwtPlot *>(parent()))
        p->setTitle(text);
}
