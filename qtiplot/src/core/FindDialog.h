/***************************************************************************
    File                 : FindDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Find dialog

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
#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLabel>

class QPushButton;
class QCheckBox;
class QComboBox;
class QLabel;

//! Find dialog
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );
    ~FindDialog() override;

private:
	QPushButton* buttonFind = nullptr;
	QPushButton* buttonCancel = nullptr;
	QPushButton* buttonReset = nullptr;

	QLabel *labelStart = nullptr;
	QComboBox* boxFind = nullptr;

    QCheckBox* boxWindowNames = nullptr;
    QCheckBox* boxWindowLabels = nullptr;
	QCheckBox* boxFolderNames = nullptr;

	QCheckBox* boxCaseSensitive = nullptr;
    QCheckBox* boxPartialMatch = nullptr;
	QCheckBox* boxSubfolders = nullptr;

public slots:

	//! Displays the project current folder path
	void setStartPath();

protected slots:

	void accept() override;
};

#endif // exportDialog_H
