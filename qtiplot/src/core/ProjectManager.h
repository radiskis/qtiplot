/***************************************************************************
    File                 : ProjectManager.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Project lifecycle manager, autosave and recovery coordinator

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

#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMessageBox>

class ApplicationWindow;
class Folder;
class MdiSubWindow;
class QAction;
class QTimerEvent;

class ProjectManager : public QObject
{
	Q_OBJECT

public:
	explicit ProjectManager(ApplicationWindow *app);
	virtual ~ProjectManager();

	//! Project lifecycle workflows
	void newProject();
	void closeProject();
	bool saveProject(bool compress = false);
	void saveProjectAs(const QString& fileName = QString(), bool compress = false);
	bool saveFolder(Folder *folder, const QString& fn, bool compress = false);
	bool saveFolderAsProject(Folder *f);
	void open();
	ApplicationWindow* open(const QString& fn, bool factorySettings = false, bool newProject = true);
	ApplicationWindow* openProject(const QString& fn, bool factorySettings = false, bool newProject = true);
	Folder* appendProject(const QString& fn, Folder* parentFolder = nullptr);
	void openRecentProject(QAction *action);

	//! State tracking
	void savedProject();
	void modifiedProject();
	void modifiedProject(MdiSubWindow *w);

	//! Autosave and crash recovery
	void autoSaveRecovery();
	void checkRecoveryOnStartup();
	bool handleTimerEvent(QTimerEvent *e);
	void resetAutosaveTimer();

	//! Recent projects and UI helpers
	void updateRecentProjectsList(const QString& fn = QString());
	QMessageBox::StandardButton showSaveProjectMessage();
	QString getSaveProjectName(const QString& fileName, bool *compress = nullptr, int scope = 0);
	bool isProjectFile(const QString& fn) const;
	bool isFileReadable(const QString& fn) const;

private:
	ApplicationWindow *d_app;
};

#endif // PROJECT_MANAGER_H
