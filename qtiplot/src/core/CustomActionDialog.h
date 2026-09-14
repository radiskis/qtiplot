/***************************************************************************
    File                 : CustomActionDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Custom Action dialog

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
#ifndef CustomActionDialog_H
#define CustomActionDialog_H

#include <QDialog>

class ApplicationWindow;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QComboBox;
class QListWidget;
class QLineEdit;
class QMenu;
class QToolBar;

class CustomActionDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget (must be the application window!=
	 * \param fl window flags
	 */
    CustomActionDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

private slots:
	void chooseIcon();
	void chooseFile();
	void chooseFolder();
	QAction* addAction();
	void removeAction();
	void setCurrentAction(int);
	void saveCurrentAction();
	void addMenu();
	void removeMenu();
	void enableDeleteMenuBtn(const QString &);

private:
	ApplicationWindow *app() const;
	void init();
	void updateDisplayList();
	QAction* actionAt(int row);
	void saveAction(QAction *action);
	void customizeAction(QAction *action);
	bool validUserInput();
	void saveMenu(QMenu *menu);

	QStringList d_app_shortcut_keys;

	QList<QMenu *> d_menus;
	QList<QToolBar *> d_app_toolbars;

    QListWidget *itemsList = nullptr;
    QPushButton *buttonCancel = nullptr, *buttonAdd = nullptr, *buttonRemove = nullptr, *buttonSave = nullptr;
    QPushButton *folderBtn = nullptr, *fileBtn = nullptr, *iconBtn = nullptr;
    QLineEdit *folderBox = nullptr, *fileBox = nullptr, *iconBox = nullptr, *textBox = nullptr, *toolTipBox = nullptr, *shortcutBox = nullptr;
    QRadioButton *menuBtn = nullptr, *toolBarBtn = nullptr;
    QComboBox *menuBox = nullptr, *toolBarBox = nullptr;
    QPushButton *newMenuBtn = nullptr, *removeMenuBtn = nullptr;
};


#endif
