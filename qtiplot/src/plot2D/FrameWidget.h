/***************************************************************************
    File                 : FrameWidget.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Base class for tool widgets in 2D plots

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
#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include <QWidget>
#include <qwt_plot.h>

#include "Graph.h"

class FrameWidget: public QWidget
{
	Q_OBJECT

public:
    FrameWidget(Graph *);

	Graph *plot(){return d_plot;};

	//! The kinds of frame a FrameWidget can draw around.
	enum FrameStyle{None = 0, Line = 1, Shadow = 2};

    //! Returns the x axis coordiante of the top left corner
    double xValue(){return d_x;};
    //! Returns the y axis coordiante of the top left corner
	double yValue(){return d_y;};
	//! Sets the position of the top left corner in axis coordinates
	void setOriginCoord(double x, double y);
	//! Sets the position of the top left corner in paint coordinates
	void setOrigin(int x, int y){move(QPoint(x, y));};
	void move(const QPoint& pos);

	//! Return bounding rectangle in plot coordinates.
	QRectF boundingRect() const;
	//! Set position (xValue() and yValue()), right and bottom values giving everything in plot coordinates.
	void setCoordinates(double left, double top, double right, double bottom);
	//! Set size in paint coordinates.
    void setSize(int w, int h){setSize(QSize(w, h));};
    //! Set size in paint coordinates.
    void setSize(const QSize& newSize);
    //! Set geometry, giving everything in paint coordinates.
	void setRect(int x, int y, int w, int h);

	double right(){return d_x_right;};
	double bottom(){return d_y_bottom;};

	int frameStyle(){return d_frame;};
	void setFrameStyle(int style);

	QColor frameColor(){return d_color;};
	void setFrameColor(const QColor& c){d_color = c;};

	int angle(){return d_angle;};
	void setAngle(int ang){d_angle = ang;};

	void showContextMenu(){emit showMenu();};
	void showPropertiesDialog(){emit showDialog();};

    virtual void print(QPainter *p, const QwtScaleMap map[QwtPlot::axisCnt]);
    void resetOrigin(){setOriginCoord(d_x, d_y);};
	void resetCoordinates(){setCoordinates(d_x, d_y, d_x_right, d_y_bottom);};

    virtual void updateCoordinates();
	virtual QString saveToString();

signals:
	void showDialog();
	void showMenu();

protected:
    //! Returns the x axis coordinate of the top left corner based on the pixel value
    double calculateXValue();
    //! Returns the y axis coordinate of the top left corner based on the pixel value
    double calculateYValue();
	//! Returns the x axis coordinate of the bottom right corner based on the pixel value
    double calculateRightValue();
    //! Returns the y axis coordinate of the bottom right corner based on the pixel value
    double calculateBottomValue();

	void drawFrame(QPainter *p, const QRect& rect);
	virtual void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *);
	void contextMenuEvent(QContextMenuEvent * ){emit showMenu();};

	//! Parent plot
	Graph *d_plot;

	//! Frame type
	int d_frame;
	//! Frame color
	QColor d_color;

	//! Rotation angle: not implemented yet
	int d_angle;

    //! X axis coordinate of the top left corner
	double d_x;
	//! Y axis coordinate of the top left corner
	double d_y;
	//! The right side position in scale coordinates.
	double d_x_right;
    //! The bottom side position in scale coordinates.
    double d_y_bottom;
	//! Frame width in pixels
	int d_frame_width;
};

#endif
