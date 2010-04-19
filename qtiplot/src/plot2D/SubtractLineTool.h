/***************************************************************************
	File                 : SubtractLineTool.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Plot tool for substracting a straight line

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
#ifndef SUBSTRACT_LINE_TOOL
#define SUBSTRACT_LINE_TOOL

#include "PlotToolInterface.h"
#include <QObject>
#include <QLineF>

#include "ScreenPickerTool.h"

class ApplicationWindow;
class QwtPlotCurve;
class QwtPlotMarker;

/*! Plot tool for substracting a straight line.
 */
class SubtractLineTool : public QObject, public PlotToolInterface
{
	Q_OBJECT
	public:
		SubtractLineTool(Graph *graph, ApplicationWindow *app, const QObject *status_target, const char *status_slot);
		virtual ~SubtractLineTool();

		virtual int rtti() const {return PlotToolInterface::Rtti_SubtractLineTool;};
	signals:
		/*! Emitted whenever a new message should be presented to the user.
		 *
		 * You don't have to connect to this signal if you alreay specified a reciever during initialization.
		 */
		void statusText(const QString&);
	protected slots:
		void selectPoint(const QwtDoublePoint &pos);

	private:
		void finalize();
		int d_selected_points;
		ScreenPickerTool *d_picker_tool;
		QwtPlotMarker *d_first_point_marker;
		QLineF d_line;
};

#endif
