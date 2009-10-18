/***************************************************************************
    File                 : AddWidgetTool.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Tool for adding enrichements to a plot.

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
#ifndef ADD_WIDGET_TOOL_H
#define ADD_WIDGET_TOOL_H

#include "PlotToolInterface.h"
#include <QObject>

class QAction;
class QPoint;
class FrameWidget;

/*!Plot tool for adding enrichements.
 *
 * Provides selection of points on a Graph.
 */
class AddWidgetTool : public QObject, public PlotToolInterface
{
	Q_OBJECT
	public:
		enum WidgetType{
        	Text = 0,
        	TexEquation,
        	Rectangle,
			Ellipse,
        	UserWidget = 1000
    	};

		AddWidgetTool(WidgetType type, Graph *graph, QAction *d_action, const QObject *status_target = NULL, const char *status_slot = "");
		virtual ~AddWidgetTool();

		virtual int rtti() const {return PlotToolInterface::Rtti_AddWidgetTool;};
		//! Returns the type of widget to be added
		WidgetType widgetType(){return d_widget_type;};

	signals:
		/*! Emitted whenever a new message should be presented to the user.
		 *
		 * You don't have to connect to this signal if you alreay specified a reciever during initialization.
		 */
		void statusText(const QString&);

	protected:
		void addEllipse(const QPoint& point);
        void addRectangle(const QPoint& point);
		void addEquation(const QPoint& point);
		void addText(const QPoint& point);
		void addWidget(const QPoint& point);

        virtual bool eventFilter(QObject *obj, QEvent *event);
		QAction *d_action;
		WidgetType d_widget_type;
		FrameWidget *d_fw;
};

#endif // ifndef ADD_WIDGET_TOOL_H
