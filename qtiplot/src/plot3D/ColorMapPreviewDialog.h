/***************************************************************************
    File                 : ColorMapPreviewDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Open file dialog providing a color map preview.

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
#ifndef ColorMapPreviewDialog_H
#define ColorMapPreviewDialog_H

#include <ExtensibleFileDialog.h>

#include <QLabel>

class ColorMapPreviewDialog : public ExtensibleFileDialog
{
	Q_OBJECT
	public:
		ColorMapPreviewDialog(QWidget *parent = 0, Qt::WFlags flags=0);

    public slots:
		void updatePreview(const QString&);
	
	private:
		QLabel *d_preview_label;
};

#endif
