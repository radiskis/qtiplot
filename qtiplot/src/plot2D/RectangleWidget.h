/***************************************************************************
    File                 : RectangleWidget.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying rectangles in 2D plots

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
#ifndef RECTANGLE_WIDGET_H
#define RECTANGLE_WIDGET_H

#include "FrameWidget.h"

class RectangleWidget: public FrameWidget
{
	Q_OBJECT

public:
	RectangleWidget(Graph *);

	virtual QString saveToString();

	void clone(RectangleWidget* t);
	static void restore(Graph *g, const QStringList& lst);

	void updateCoordinates();
	void setLinkedLayer(int layerIndex);

public slots:
    void closedLinkedLayer(QObject *);

private:
    //! Index of the linked layer using the rectangle coordinates as a zoom region
    int d_linked_layer;
};

#endif
