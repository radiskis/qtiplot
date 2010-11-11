/***************************************************************************
	File                 : AnovaDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : ANOVA dialog

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
#ifndef AnovaDialog_H
#define AnovaDialog_H

#include <QDialog>

class QCheckBox;
class QPushButton;
class QListWidget;
class DoubleSpinBox;
class Table;

//! Filter options dialog
class AnovaDialog : public QDialog
{
    Q_OBJECT

public:
	AnovaDialog(Table *t, bool twoWay = false, QWidget* parent = 0, Qt::WFlags fl = 0);

private slots:
	void accept();
	void addData();
	void removeData();
	void showCurrentFolder(bool);

private:
	bool d_two_way;

	QListWidget* availableSamples;
	QListWidget* selectedSamples;

	QCheckBox *currentFolderBox;

	QPushButton* btnAdd;
	QPushButton* btnRemove;
	QPushButton* buttonOk;

	DoubleSpinBox* boxSignificance;
};

#endif
