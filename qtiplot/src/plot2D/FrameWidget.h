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
#include <QPen>
#include <qwt_plot.h>

class Graph;

class FrameWidget: public QWidget
{
	Q_OBJECT

public:
	//! Length units.
	enum Unit{Inch, Millimeter, Centimeter, Point, Pixel, Scale};

    FrameWidget(Graph *);

	Graph *plot() const {return d_plot;};

	//! The kinds of frame a FrameWidget can draw around.
	enum FrameStyle{None = 0, Line = 1, Shadow = 2};
	//! The attach to policy.
	enum AttachPolicy {Page, Scales};

    //! Returns the x axis coordiante of the top left corner
    double xValue() const {return d_x;};
    //! Returns the y axis coordiante of the top left corner
	double yValue() const {return d_y;};
	//! Sets the position of the top left corner in axis coordinates
	void setOriginCoord(double x, double y);
	//! Sets the position of the top left corner in paint coordinates
	void setOrigin(int x, int y){move(QPoint(x, y));};
	void move(const QPoint& pos);

	//! Return bounding rectangle in plot coordinates.
	QRectF boundingRect() const;
	//! Set position (xValue() and yValue()), right and bottom values giving everything in plot coordinates.
	void setCoordinates(double left, double top, double right, double bottom);
	//! Set size in paint coordinates (pixels).
    void setSize(int w, int h){setSize(QSize(w, h));};
    //! Set size in paint coordinates.
    void setSize(const QSize& newSize);
    //! Set geometry, giving everything in paint coordinates.
	void setRect(int x, int y, int w, int h);

	double right() const {return d_x_right;};
	double bottom() const {return d_y_bottom;};

	static double xIn(QWidget *w, Unit unit);
	static double yIn(QWidget *w, Unit unit);
	static double widthIn(QWidget *w, Unit unit);
	static double heightIn(QWidget *w, Unit unit);
    //! Set geometry of a widget in arbitrary units.
    static void setRect(QWidget *, double x, double y, double w, double h, Unit unit = Pixel);

	int frameStyle() const {return d_frame;};
	void setFrameStyle(int style);

	QPen framePen() const {return d_frame_pen;};
	void setFramePen(const QPen& p){d_frame_pen = p;};

	Qt::PenStyle frameLineStyle() const {return d_frame_pen.style();};
	void setFrameLineStyle(const Qt::PenStyle& s){d_frame_pen.setStyle(s);};

	QColor frameColor() const {return d_frame_pen.color();};
	void setFrameColor(const QColor& c){d_frame_pen.setColor(c);};

	double frameWidth() const {return d_frame_pen.widthF();};
	void setFrameWidth(double w){d_frame_pen.setWidthF(w);};

	QColor backgroundColor() const {return palette().color(QPalette::Window);};
	void setBackgroundColor(const QColor& c){QPalette pal = palette(); pal.setColor(QPalette::Window, c); setPalette(pal);};

	QBrush brush() const {return d_brush;};
	void setBrush(const QBrush& b){d_brush = b;};

	int angle() const {return d_angle;};
	void setAngle(int ang){d_angle = ang;};

	void showContextMenu(){emit showMenu();};
	void showPropertiesDialog(){emit showDialog();};

	virtual void print(QPainter *p, const QwtScaleMap map[QwtPlot::axisCnt]);
    void resetOrigin(){setOriginCoord(d_x, d_y);};
	void resetCoordinates(){setCoordinates(d_x, d_y, d_x_right, d_y_bottom);};

    virtual void updateCoordinates();
	virtual QString saveToString() const;

	void setAttachPolicy(AttachPolicy attachTo);
	AttachPolicy attachPolicy() const {return d_attach_policy;};

	bool isOnTop() const {return d_on_top;};
	void setOnTop(bool on = true);

	void mousePressEvent(QMouseEvent *) override;

signals:
	void showDialog();
	void showMenu();
	void changedCoordinates(const QRectF&);

protected:
    //! Returns the x axis coordinate of the top left corner based on the pixel value
    double calculateXValue();
    //! Returns the y axis coordinate of the top left corner based on the pixel value
    double calculateYValue();
	//! Returns the x axis coordinate of the bottom right corner based on the pixel value
    double calculateRightValue();
    //! Returns the y axis coordinate of the bottom right corner based on the pixel value
    double calculateBottomValue();

	virtual void drawFrame(QPainter *p, const QRect& rect);
	void paintEvent(QPaintEvent *e) override;
	void contextMenuEvent(QContextMenuEvent * ) override {emit showMenu();};

	//! Parent plot
	Graph *d_plot = nullptr;

	//! Frame type
	int d_frame = None;
	//! Pen used to draw the frame
	QPen d_frame_pen;
	//! Background brush
	QBrush d_brush;

	//! Rotation angle: not implemented yet
	int d_angle = 0;

    //! X axis coordinate of the top left corner
	double d_x = 0.0;
	//! Y axis coordinate of the top left corner
	double d_y = 0.0;
	//! The right side position in scale coordinates.
	double d_x_right = 0.0;
    //! The bottom side position in scale coordinates.
    double d_y_bottom = 0.0;
	//! Frame width in pixels
	int d_shadow_width = 4;
	//! The attach to policy
	AttachPolicy d_attach_policy = Scales;
	bool d_on_top = true;
};

#endif
