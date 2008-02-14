/***************************************************************************
    File                 : MdiSubWindow.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : MDI sub window

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
#include "MdiSubWindow.h"
#include "Folder.h"
#include "ApplicationWindow.h"

#include <QMessageBox>
#include <QEvent>
#include <QCloseEvent>
#include <QString>
#include <QDateTime>
#include <QMenu>

MdiSubWindow::MdiSubWindow(const QString& label, ApplicationWindow *app, const QString& name, Qt::WFlags f):
		QMdiSubWindow (app, f),
		d_app(app),
		d_folder(app->currentFolder()),
		d_label(label),
		d_status(Normal),
		d_caption_policy(Both),
		d_confirm_close(true),
		d_birthdate(QDateTime::currentDateTime ().toString(Qt::LocalDate)),
		d_min_restore_size(QSize())
{
	setObjectName(name);
	setAttribute(Qt::WA_DeleteOnClose);
	if (d_folder)
		d_folder->addWindow(this);
}

void MdiSubWindow::updateCaption()
{
switch (d_caption_policy)
	{
	case Name:
        setWindowTitle(objectName());
	break;

	case Label:
		if (!d_label.isEmpty())
            setWindowTitle(d_label);
		else
            setWindowTitle(objectName());
	break;

	case Both:
		if (!d_label.isEmpty())
            setWindowTitle(objectName() + " - " + d_label);
		else
            setWindowTitle(objectName());
	break;
	}
};

void MdiSubWindow::resizeEvent( QResizeEvent* e )
{
	emit resizedWindow(this);
	QMdiSubWindow::resizeEvent( e );
}

void MdiSubWindow::closeEvent( QCloseEvent *e )
{
	if (d_confirm_close){
    	switch( QMessageBox::information(this, tr("QtiPlot"),
				tr("Do you want to hide or delete") + "<p><b>'" + objectName() + "'</b> ?",
				tr("Delete"), tr("Hide"), tr("Cancel"), 0, 2)){
		case 0:
			emit closedWindow(this);
			e->accept();
		break;

		case 1:
			e->ignore();
			emit hiddenWindow(this);
		break;

		case 2:
			e->ignore();
		break;
		}
    } else {
		emit closedWindow(this);
    	e->accept();
    }
}

QString MdiSubWindow::aspect()
{
QString s = tr("Normal");
switch (d_status)
	{
	case Normal:
	break;

	case Minimized:
		return tr("Minimized");
	break;

	case Maximized:
		return tr("Maximized");
	break;

	case Hidden:
		return tr("Hidden");
	break;
	}
return s;
}

QString MdiSubWindow::sizeToString()
{
return QString::number(8*sizeof(this)/1024.0, 'f', 1) + " " + tr("kB");
}

void MdiSubWindow::changeEvent(QEvent *event)
{
	if (!isHidden() && event->type() == QEvent::WindowStateChange){
		Status oldStatus = d_status;
		Status newStatus = Normal;
		if( windowState() & Qt::WindowMinimized ){
		    if (oldStatus != Minimized)
                d_min_restore_size = frameSize();
	    	newStatus = Minimized;
		} else if ( windowState() & Qt::WindowMaximized )
	     	newStatus = Maximized;

		if (newStatus != oldStatus){
			d_status = newStatus;
    		emit statusChanged (this);
		}
	}
	QMdiSubWindow::changeEvent(event);
}

bool MdiSubWindow::eventFilter(QObject *object, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu && object == widget()){
        emit showContextMenu();
        return true;
	}

	if (e->type() == QEvent::Move && object == widget()){
		QObjectList lst = children();
		foreach(QObject *o, lst){
			if (o->isA("QMenu") && d_app){
			    d_app->customWindowTitleBarMenu(this, (QMenu *)o);
				break;
			}
		}
	}
	return QMdiSubWindow::eventFilter(object, e);
}

void MdiSubWindow::setStatus(Status s)
{
	if (d_status == s)
		return;

	d_status = s;
	emit statusChanged (this);
}

void MdiSubWindow::setHidden()
{
    d_status = Hidden;
    emit statusChanged (this);
    hide();
}

void MdiSubWindow::setNormal()
{
	showNormal();
	d_status = Normal;
	emit statusChanged (this);
}

void MdiSubWindow::setMinimized()
{
	d_status = Minimized;
	emit statusChanged (this);
	showMinimized();
}

void MdiSubWindow::setMaximized()
{
	showMaximized();
	d_status = Maximized;
	emit statusChanged (this);
}
