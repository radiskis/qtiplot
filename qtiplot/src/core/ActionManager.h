/***************************************************************************
    File                 : ActionManager.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Central action coordinator and custom action manager

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

#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class ApplicationWindow;
class QMdiSubWindow;
class QAction;
class QMenu;
class QToolBar;

class ActionManager : public QObject
{
    Q_OBJECT

public:
    explicit ActionManager(ApplicationWindow *app);
    virtual ~ActionManager();

    void createActions();
    void initToolBars();
    void initPlot3DToolBar();
    void initMainMenu();
    void translateActionsStrings();

    void customMenu(QMdiSubWindow *w);
    void disableActions();
    void customColumnActions();
    void customToolBars(QMdiSubWindow *w);
    void disableToolbars();

    void showCustomActionDialog();
    void addCustomAction(QAction *action, const QString &parentName, int index = -1);
    void reloadCustomActions();
    void removeCustomAction(QAction *action);
    void performCustomAction(QAction *action);
    void loadCustomActions();

    QList<QMenu *> customizableMenusList();
    QList<QMenu *> menusList();
    QList<QToolBar *> toolBarsList();

    QMenu* addCustomMenu(const QString &title, const QString &parentName);
    void removeCustomMenu(const QString &title);
    void reloadCustomMenus();

    QList<QAction *> customActionsList() const;
    QList<QMenu *> customMenusList() const;

    QAction* action(const QString &name) const;
    void registerAction(const QString &name, QAction *action);

    void tableMenuAboutToShow();
    void plotDataMenuAboutToShow();
    void plotMenuAboutToShow();
    void scriptingMenuAboutToShow();
    void analysisMenuAboutToShow();
    void matrixMenuAboutToShow();
    void fileMenuAboutToShow();
    void editMenuAboutToShow();
    void windowsMenuAboutToShow();

private:
    ApplicationWindow *d_app;
    QMap<QString, QAction*> d_actions;
};

#endif // ACTION_MANAGER_H
