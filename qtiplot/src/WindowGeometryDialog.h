/***************************************************************************
    File                 : WindowGeometryDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : ImageDialog geometry dialog

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
#ifndef WINDOW_GEOMETRY_DIALOG_H
#define WINDOW_GEOMETRY_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

//! MDI Window geometry dialog
class WindowGeometryDialog : public QDialog
{
    Q_OBJECT

public:
    WindowGeometryDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );

	void setOrigin(const QPoint& o);
	void setSize(const QSize& size);

protected slots:
	void accept();
	void update();
	void adjustHeight(int width);
	void adjustWidth(int height);

signals:
	void setGeometry(int, int, int, int);

protected:
	double aspect_ratio;

private:
    QPushButton* buttonOk;
	QPushButton* buttonCancel;
	QPushButton* buttonApply;
    QSpinBox* boxX, *boxY, *boxWidth, *boxHeight;
	QCheckBox *keepRatioBox;
};


#endif
