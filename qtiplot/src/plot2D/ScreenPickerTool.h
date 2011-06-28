/***************************************************************************
    File                 : ScreenPickerTool.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Plot tool for selecting arbitrary points.

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
#ifndef SCREEN_PICKER_TOOL_H
#define SCREEN_PICKER_TOOL_H

#include <PlotToolInterface.h>
#include <DoubleSpinBox.h>

#include <QObject>
#include <QPointer>
#include <QLabel>
#include <QSpinBox>

#include <qwt_double_rect.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>

class ApplicationWindow;
class Table;
class Matrix;
class DataCurve;

/*!Plot tool for selecting arbitrary points.
 *
 * This is a rather thin wrapper around QwtPlotPicker, providing selection of points
 * on a Graph/Plot and displaying coordinates.
 */
class ScreenPickerTool : public QwtPlotPicker, public PlotToolInterface
{
	Q_OBJECT
	public:
		enum MoveRestriction { NoRestriction, Vertical, Horizontal };
		ScreenPickerTool(Graph *graph, const QObject *status_target=NULL, const char *status_slot="");
		virtual ~ScreenPickerTool();
		virtual void append(const QwtDoublePoint &pos);
		void setMoveRestriction(ScreenPickerTool::MoveRestriction r){d_move_restriction = r;};

		double xValue(){return d_selection_marker.xValue();};
		double yValue(){return d_selection_marker.yValue();};

	signals:
		/*! Emitted whenever a new message should be presented to the user.
		 *
		 * You don't have to connect to this signal if you alreay specified a reciever during initialization.
		 */
		void statusText(const QString&);
	protected:
        virtual bool eventFilter(QObject *obj, QEvent *event);
		virtual void append(const QPoint &point);
		QwtPlotMarker d_selection_marker;
		MoveRestriction d_move_restriction;
};

/*!Plot tool for drawing arbitrary points.
 *
 */
class DrawPointTool : public ScreenPickerTool
{
	Q_OBJECT
	public:
		DrawPointTool(ApplicationWindow *app, Graph *graph, const QObject *status_target=NULL, const char *status_slot="");
		virtual int rtti() const { return Rtti_DrawDataPoints;};

	protected:
        virtual bool eventFilter(QObject *obj, QEvent *event);
		void appendPoint(const QwtDoublePoint &point);
		DataCurve *d_curve;
		Table *d_table;
		ApplicationWindow *d_app;
};

/*!Plot tool for image analysis.
 *
 */
class ImageProfilesTool : public ScreenPickerTool
{
	Q_OBJECT
	public:
		ImageProfilesTool(ApplicationWindow *app, Graph *graph, Matrix *m, Table *horTable, Table *verTable);
		void connectPlotLayers();

		ImageProfilesTool* clone(Graph *g);

		virtual ~ImageProfilesTool();
		virtual void append(const QwtDoublePoint &pos);
		virtual int rtti() const { return Rtti_ImageProfilesTool;};

		int averagePixels(){return averageBox->value();}
		void setAveragePixels(int pixels);

		QPointer<Matrix> matrix(){return d_matrix;};
		QPointer<Table> horizontalTable(){return d_hor_table;};
		QPointer<Table> verticalTable(){return d_ver_table;};

	private slots:
		void modifiedMatrix(Matrix *);
		void updateCursorPosition();
		void updateCursorWidth(int width);

	private:
		void setCursorWidth(int width);

	protected:
		ApplicationWindow *d_app;
		QPointer<Matrix> d_matrix;
		QPointer<Table> d_hor_table, d_ver_table;
		DoubleSpinBox *horSpinBox, *vertSpinBox;
		QSpinBox *averageBox;
		QLabel *zLabel;
		QWidget *d_box;
};

#endif // ifndef SCREEN_PICKER_TOOL_H
