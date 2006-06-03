/***************************************************************************
    File                 : widget.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Extension to QWidget
                           
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
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QString>

//! Extension to QWidget
class MyWidget: public QWidget
{
	Q_OBJECT

public:	
	
	//! Constructor
	/**
	 * \param label window label
	 * \param parent parent widget
	 * \param name window name
	 * \param f window flags
	 * \sa setCaptionPolicy(), captionPolicy()
	 */
	MyWidget(const QString& label = QString(), QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);

	enum CaptionPolicy{Name = 0, Label = 1, Both = 2};
	enum Status{Hidden = -1, Normal = 0, Minimized = 1, Maximized = 2};

	//! Return the window label
	QString windowLabel(){return QString(w_label);};
	//! Set the window label
	void setWindowLabel(const QString& s){w_label = s;};

	//! Return the window name
	QString name(){return objectName();};
	//! Set the window name
	void setName(const QString& s){setObjectName(s);};

	//! Return the caption policy
	CaptionPolicy captionPolicy(){return caption_policy;};
	//! Set the caption policy
	/**
	 * The caption policy can be
	 * Name -> caption determined by the window name
	 * Label -> caption detemined by the window label
	 * Both -> caption = "name - label"
	 */
	void setCaptionPolicy(CaptionPolicy policy);

	//! Return the creation date
	QString birthDate(){return birthdate;};
	//! Set the creation date
	void setBirthDate(const QString& s){birthdate = s;};

	//! Return the window status as a string
	QString aspect();
	//! Return the window status flag (hidden, normal, minimized or maximized)
	Status status(){return w_status;};
	//! Set the window status flag (hidden, normal, minimized or maximized)
	void setStatus(Status s){w_status = s;};

	// TODO:
	//! Not implemented yet
	virtual QString saveAsTemplate(const QString& ){return QString();};
	// TODO:
	//! Not implemented yet
	virtual void restore(const QStringList& ){};
	
	// TODO:
	//! Not implemented, yet
	virtual void print(){};
	// TODO:
	//! Not implemented, yet
	virtual QString saveToString(const QString &){return QString();};
	
	// TODO: make this return something useful
	//! Size of the widget as a string
	virtual QString sizeToString();

	//! Notifies a change in the status of a former maximized window after it was shown as normal as a result of an indirect user action (e.g.: another window was maximized)
	void setNormal();

	//! Notifies that a window was hidden by a direct user action
	void setHidden();

	// event handlers
	//! Close event handler 
	/**
	 * Ask the user "delete, hide, or cancel?" if the 
	 * "ask on close" flag is set.
	 */
	void closeEvent( QCloseEvent *);
	//! Toggle the "ask on close" flag
	void askOnCloseEvent(bool ask){askOnClose = ask;};
	//! General event handler (updates the window status if it changed)
	bool event( QEvent *e );

	//! Show the window maximized
	void showMaximized();
	//! Tells if a resize event was requested by the user or generated programatically
	bool userRequested(){return user_request;};

signals:  
	//! Emitted when the window was closed
	void closedWindow(QWidget *);
	//! Emitted when the window was hidden
	void hiddenWindow(MyWidget *);
	void modifiedWindow(QWidget *);
	void resizedWindow(QWidget *);
	//! Emitted when the window status changed
	void statusChanged(MyWidget *);

private:
	//! The window label
	/**
	 * \sa setWindowLabel(), windowLabel(), setCaptionPolicy()
	 */
	QString w_label;
	//! The creation date
	QString birthdate;
	//! The window status
	Status w_status;
	//! The caption policy
	/**
	 * \sa setCaptionPolicy(), captionPolicy()
	 */
	CaptionPolicy caption_policy;
	//! Toggle on/off: Ask the user "delete, hide, or cancel?" on a close event
	bool askOnClose;
	//! Tells if the showMaximized action was requested by the user or generated programatically
	bool user_request;
};

#endif
