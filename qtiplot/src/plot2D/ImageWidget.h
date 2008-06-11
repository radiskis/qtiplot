/***************************************************************************
    File                 : ImageWidget.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying images in 2D plots

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
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include "FrameWidget.h"

class ImageWidget: public FrameWidget
{
	Q_OBJECT

public:
    //! Construct an image marker from a file name.
	ImageWidget(Graph *, const QString& fn = QString::null);

	//! Return the pixmap to be drawn.
	QPixmap pixmap() const {return d_pix;};
	void setPixmap(const QPixmap&);

    //! Import image from #d_file_name. Returns true if successful.
	bool load(const QString& fn);
	//! Return #d_file_name.
	QString fileName(){return d_file_name;};

	void print(QPainter *p, const QwtScaleMap map[QwtPlot::axisCnt]);
	virtual QString saveToString();

	void clone(ImageWidget* t);
	static void restore(Graph *g, const QStringList& lst);

private:
	void paintEvent(QPaintEvent *e);
	QPixmap d_pix;
	//! The file from which the image was loaded.
	QString d_file_name;
	//! Flag telling if the pixmap must be saved in the .qti project as XPM
	bool d_save_xpm;
};

#endif
