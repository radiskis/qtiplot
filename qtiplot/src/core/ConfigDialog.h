/***************************************************************************
    File                 : ConfigDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Preferences dialog

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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QList>

class ApplicationWindow;
class ApplicationSettings;
class ConfigPage;
class QListWidget;
class QStackedWidget;
class QLabel;
class QPushButton;

//! Preferences dialog
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
    ConfigDialog( QWidget* parent, Qt::WindowFlags fl = {} );
	void setColumnSeparator(const QString& sep);

private slots:
    void languageChange();
	void accept() override;
	void apply();
	void resetDefaultSettings();
	void setCurrentPage(int index);

private:
	ApplicationWindow *d_app = nullptr;
	ApplicationSettings *d_settings = nullptr;
	QList<ConfigPage *> d_pages;

	QListWidget *itemsList = nullptr;
	QStackedWidget *generalDialog = nullptr;
	QLabel *lblPageHeader = nullptr;

	QPushButton *btnDefaultSettings = nullptr;
	QPushButton *buttonApply = nullptr;
	QPushButton *buttonOk = nullptr;
	QPushButton *buttonCancel = nullptr;
};

#endif // CONFIGDIALOG_H
