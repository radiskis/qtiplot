/***************************************************************************
    File                 : ProjectManager.cpp
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

#include "ProjectManager.h"
#include "ProjectSerializer.h"
#include "ApplicationWindow.h"
#include "Folder.h"
#include "Matrix.h"
#include "Table.h"
#include "OpenProjectDialog.h"
#include "CrashHandler.h"
#include "Logger.h"
#include "QtiPlotApplication.h"
#ifdef QTIPLOT_LEDGER
#include "Tracked.h"
#endif

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimerEvent>
#include <QAction>
#include <QMenu>
#include <QCursor>

extern "C"
{
void file_uncompress(char  *);
}

ProjectManager::ProjectManager(ApplicationWindow *app)
	: QObject(app), d_app(app)
{
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::autoSaveRecovery()
{
	if (!d_app || !d_app->projectFolder())
		return;

	// Determine recovery directory and filename
	QString recDir = CrashHandler::recoveryDirPath();
	QDir().mkpath(recDir);

	QString recFn;
	if (d_app->projectname == "untitled" || d_app->projectname.isEmpty() ||
		d_app->projectname.endsWith(".opj", Qt::CaseInsensitive) || d_app->projectname.endsWith(".ogm", Qt::CaseInsensitive) ||
		d_app->projectname.endsWith(".ogw", Qt::CaseInsensitive) || d_app->projectname.endsWith(".ogg", Qt::CaseInsensitive)) {
		qint64 pid = QCoreApplication::applicationPid();
		recFn = QString("%1/recovery_unnamed_%2.qti").arg(recDir).arg(pid);
	} else {
		// Named project: save recovery to sibling file in recovery dir to avoid touching user file
		QFileInfo fi(d_app->projectname);
		qint64 pid = QCoreApplication::applicationPid();
		recFn = QString("%1/recovery_%2_%3.qti").arg(recDir).arg(fi.baseName()).arg(pid);
	}

	qCDebug(lcIo) << "Performing background autosave to recovery file:" << recFn;
	if (d_app->saveFolder(d_app->projectFolder(), recFn, false)) {
		CrashHandler::setSessionRecoveryFile(recFn);
		qCInfo(lcIo) << "Autosave recovery file updated successfully:" << recFn;
	} else {
		qCWarning(lcIo) << "Autosave recovery file failed to write:" << recFn;
	}
}

void ProjectManager::checkRecoveryOnStartup()
{
	QStringList recoveryFiles = CrashHandler::findRecoveryFiles();
	if (recoveryFiles.isEmpty())
		return;

	QString recDir = CrashHandler::recoveryDirPath();
	QString latest = recDir + "/" + recoveryFiles.first();
	QFileInfo fi(latest);

	QString timeStr = fi.lastModified().toString("yyyy-MM-dd hh:mm:ss");
	int choice = QMessageBox::question(d_app, tr("QtiPlot - Session Recovery"),
		tr("QtiPlot detected an unsaved or recovered session from <b>%1</b>.<br>"
		   "Would you like to recover it?").arg(timeStr),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

	if (choice == QMessageBox::Yes) {
		qCInfo(lcIo) << "User chose to recover previous session from:" << latest;
		d_app->open(latest, false, false);
	} else {
		qCInfo(lcIo) << "User declined to recover session; removing stale recovery file:" << latest;
		QFile::remove(latest);
	}
}

bool ProjectManager::handleTimerEvent(QTimerEvent *e)
{
	if (d_app && e->timerId() == d_app->savingTimerId) {
		autoSaveRecovery();
		return true;
	}
	return false;
}

void ProjectManager::resetAutosaveTimer()
{
	if (!d_app)
		return;
	if (d_app->autoSave) {
		if (d_app->savingTimerId)
			d_app->killTimer(d_app->savingTimerId);
		d_app->savingTimerId = d_app->startTimer(d_app->autoSaveTime * 60000);
	} else {
		d_app->savingTimerId = 0;
	}
}

bool ProjectManager::saveProject(bool compress)
{
	if (d_app->projectname == "untitled" || d_app->projectname.endsWith(".opj", Qt::CaseInsensitive) ||
		d_app->projectname.endsWith(".ogm", Qt::CaseInsensitive) || d_app->projectname.endsWith(".ogw", Qt::CaseInsensitive)
		|| d_app->projectname.endsWith(".ogg", Qt::CaseInsensitive)){
		saveProjectAs();
		return false;
	}

	if (!saveFolder(d_app->projectFolder(), d_app->projectname, compress)) {
		QApplication::restoreOverrideCursor();
		return false;
	}
	savedProject();

	resetAutosaveTimer();

	QApplication::restoreOverrideCursor();
	return true;
}

bool ProjectManager::saveFolder(Folder *folder, const QString& fn, bool compress)
{
	return ProjectSerializer::saveFolder(folder, fn, compress, d_app);
}

bool ProjectManager::saveFolderAsProject(Folder *f)
{
	bool compress = false;
	QString fn = getSaveProjectName("", &compress, 1);
	if (!fn.isEmpty())
		return saveFolder(f, fn, compress);
	return false;
}

void ProjectManager::saveProjectAs(const QString& fileName, bool compress)
{
	QString fn = getSaveProjectName(fileName, &compress);
	if (!fn.isEmpty()){
		d_app->projectname = fn;
		if (saveProject(compress)){
			updateRecentProjectsList(d_app->projectname);

			QString baseName = QFileInfo(fn).baseName();
			FolderListItem *item = (FolderListItem *)d_app->folders->topLevelItem(0);
			item->setText(0, baseName);
			item->folder()->setObjectName(baseName);
		}
	}
}

void ProjectManager::savedProject()
{
	d_app->setWindowTitle(tr("QtiPlot") + " - " + d_app->projectname);
	if (d_app->actionSaveProject)
		d_app->actionSaveProject->setEnabled(false);
	d_app->saved = true;

	Folder *f = d_app->projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if (w->inherits("Matrix"))
				((Matrix *)w)->undoStack()->setClean();
		}
		f = f->folderBelow();
	}
}

void ProjectManager::modifiedProject()
{
	if (!d_app->windowTitle().contains("*"))
		d_app->setWindowTitle(tr("QtiPlot") + " - " + d_app->projectname + " *");

	if (d_app->saved == false)
		return;

	if (d_app->actionSaveProject)
		d_app->actionSaveProject->setEnabled(true);

	d_app->saved = false;
}

void ProjectManager::modifiedProject(MdiSubWindow *w)
{
	if (!w)
		return;
	modifiedProject();
}

QMessageBox::StandardButton ProjectManager::showSaveProjectMessage()
{
	if (!d_app->saved){
		QString s = tr("Save changes to project: <p><b> %1 </b> ?").arg(d_app->projectname);
		switch(QMessageBox::information(d_app, tr("QtiPlot"), s, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes)){
			case QMessageBox::Yes:
				saveProject();
				return QMessageBox::Yes;
			break;
			case QMessageBox::No:
			default:
				savedProject();
				return QMessageBox::No;
			break;
			case QMessageBox::Cancel:
				return QMessageBox::Cancel;
			break;
		}
	}
	return QMessageBox::No;
}

void ProjectManager::closeProject()
{
	if (showSaveProjectMessage() == QMessageBox::Cancel)
		return;

	d_app->blockSignals(true);

	Folder *f = d_app->projectFolder();
	f->folderListItem()->setText(0, tr("UNTITLED"));
	d_app->current_folder = f;
	d_app->projectname = "untitled";

	for (MdiSubWindow *w : f->windowsList()){
		w->askOnCloseEvent(false);
		d_app->closeWindow(w);
	}

	if (!(f->children()).isEmpty()){
		Folder *subFolder = f->folderBelow();
		int initial_depth = f->depth();
		while (subFolder && subFolder->depth() > initial_depth){
			for (MdiSubWindow *w : subFolder->windowsList()){
				d_app->removeWindowFromLists(w);
				subFolder->removeWindow(w);
				delete w;
			}
			delete subFolder->folderListItem();
			delete subFolder;

			subFolder = f->folderBelow();
		}
	}

	d_app->blockSignals(false);
	savedProject();
	d_app->setWindowTitle(tr("QtiPlot - untitled"));
	if (d_app->scriptWindow) {
		delete d_app->scriptWindow;
		d_app->scriptWindow = nullptr;
	}
	QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
	QCoreApplication::processEvents();

#ifdef QTIPLOT_LEDGER
	const AllocLedger &l = AllocLedger::instance();
	if (!l.clean()) {
		fprintf(stderr, "LEDGER: objects outliving project:\n%s\n",
				l.report().toUtf8().constData());
		if (qApp->arguments().contains("--assert-ledger"))
			::exit(70);
	}
#endif
}

void ProjectManager::newProject()
{
	if (showSaveProjectMessage() == QMessageBox::Cancel)
		return;

	d_app->saveSettings();//the recent projects must be saved

#ifdef BROWSER_PLUGIN
	closeProject();
	d_app->initWindow();
#else
	ApplicationWindow *ed = new ApplicationWindow();
	ed->restoreApplicationGeometry();
	ed->initWindow();
	d_app->close();
#endif
}

void ProjectManager::open()
{
	if (showSaveProjectMessage() == QMessageBox::Cancel)
		return;

	OpenProjectDialog *open_dialog = new OpenProjectDialog(d_app, d_app->d_extended_open_dialog);
	open_dialog->setDirectory(d_app->workingDir);
	if (open_dialog->exec() != QDialog::Accepted || open_dialog->selectedFiles().isEmpty())
		return;

	d_app->workingDir = open_dialog->directory().path();
	d_app->d_open_project_filter = open_dialog->selectedNameFilter();

	switch(open_dialog->openMode()) {
		case OpenProjectDialog::NewProject:
			{
				QString fn = open_dialog->selectedFiles()[0];
				QFileInfo fi(fn);

				if (d_app->projectname != "untitled"){
					QFileInfo fi(d_app->projectname);
					QString pn = fi.absoluteFilePath();
					if (fn == pn){
						QMessageBox::warning(d_app, tr("QtiPlot - File openning error"),
								tr("The file: <b>%1</b> is the current file!").arg(fn));
						return;
					}
				}

				if (!fi.exists ()){
					QMessageBox::critical(d_app, tr("QtiPlot - File openning error"),
					tr("The file: <b>%1</b> doesn't exist!").arg(fn));
					return;
				}

				#ifdef BROWSER_PLUGIN
				if (isProjectFile(fn))
					closeProject();
				open(fn, false, false);
				#else
				ApplicationWindow *a = open (fn);
				if (a){
					a->workingDir = d_app->workingDir;
					a->d_open_project_filter = d_app->d_open_project_filter;
					if (isProjectFile(fn)){
						d_app->recentProjects = a->recentProjects;//the recent projects must be saved
						d_app->close();
					} else
						d_app->saveSettings();//the recent projects must be saved
				}
				#endif

				break;
			}
		case OpenProjectDialog::NewFolder:
			appendProject(open_dialog->selectedFiles()[0]);
			break;
	}
}

bool ProjectManager::isProjectFile(const QString& fn) const
{
	if (fn.endsWith(".qti", Qt::CaseInsensitive) || fn.endsWith(".qti.gz", Qt::CaseInsensitive) ||
		fn.endsWith(".qti~", Qt::CaseInsensitive) ||
		fn.endsWith(".opj",Qt::CaseInsensitive) || fn.endsWith(".ogg",Qt::CaseInsensitive))
		return true;
	return false;
}

bool ProjectManager::isFileReadable(const QString& file_name) const
{
	QFileInfo fi(file_name);
	if (fi.isDir()){
		QMessageBox::critical(d_app, tr("QtiPlot - File openning error"),
				tr("<b>%1</b> is a directory, please specify a file name!").arg(file_name));
		return false;
	} else if (!fi.exists()){
		QMessageBox::critical(d_app, tr("QtiPlot - File openning error"),
				tr("The file: <b>%1</b> doesn't exist!").arg(file_name));
		return false;
	} else if (fi.exists() && !fi.isReadable()){
		QMessageBox::critical(d_app, tr("QtiPlot - File openning error"),
				tr("You don't have the permission to open this file: <b>%1</b>").arg(file_name));
		return false;
	}
	return true;
}

ApplicationWindow* ProjectManager::open(const QString& fn, bool factorySettings, bool newProject)
{
	if (!isFileReadable(fn)){
		if (d_app->recentProjects.contains(fn)){
			d_app->recentProjects.removeAll(fn);
			updateRecentProjectsList();
		}
		return nullptr;
	}

	if (fn.endsWith(".opj", Qt::CaseInsensitive) || fn.endsWith(".ogm", Qt::CaseInsensitive) ||
		fn.endsWith(".ogw", Qt::CaseInsensitive) || fn.endsWith(".ogg", Qt::CaseInsensitive))
		return d_app->importOPJ(fn, factorySettings, newProject);
	else
#ifdef Q_OS_WIN
	if (d_app->importUsingExcel()){
		if (fn.endsWith(".xl", Qt::CaseInsensitive) || fn.endsWith(".xlsx", Qt::CaseInsensitive) ||
			fn.endsWith(".xlsm", Qt::CaseInsensitive) || fn.endsWith(".xlsb", Qt::CaseInsensitive) ||
			fn.endsWith(".xlam", Qt::CaseInsensitive) || fn.endsWith(".xltx", Qt::CaseInsensitive) ||
			fn.endsWith(".xltm", Qt::CaseInsensitive) || fn.endsWith(".xls", Qt::CaseInsensitive) ||
			fn.endsWith(".xla", Qt::CaseInsensitive) || fn.endsWith(".xlt", Qt::CaseInsensitive) ||
			fn.endsWith(".xlm", Qt::CaseInsensitive) || fn.endsWith(".xlw", Qt::CaseInsensitive)){
			d_app->importExcel(fn);
			return d_app;
		}
	}
#endif

	if (fn.endsWith(".db", Qt::CaseInsensitive) || fn.endsWith(".dbf", Qt::CaseInsensitive) ||
		fn.endsWith(".mdb", Qt::CaseInsensitive) || fn.endsWith(".accdb", Qt::CaseInsensitive)){
		d_app->importDatabase(fn);
		return d_app;
	} else if (fn.endsWith(".xls", Qt::CaseInsensitive) || fn.endsWith(".xlsx", Qt::CaseInsensitive)){
		d_app->importExcel(fn);
		return d_app;
	} else if (fn.endsWith(".py", Qt::CaseInsensitive))
		return d_app->loadScript(fn);
	else if (fn.endsWith(".ods", Qt::CaseInsensitive)){
		d_app->importOdfSpreadsheet(fn);
		return d_app;
	}

	QString fname = fn;
	if (fn.endsWith(".qti.gz", Qt::CaseInsensitive)){//decompress using zlib
		file_uncompress(fname.toLocal8Bit().data());
		fname = fname.left(fname.size() - 3);
	}

	QFile f(fname);
	QTextStream t( &f );
	if (!f.open(QIODevice::ReadOnly)) return nullptr;
	QString s = t.readLine();
	f.close();

	QStringList lst = s.split(QRegularExpression("\\s"), Qt::SkipEmptyParts);
	bool qtiProject = (lst.count() < 2 || lst[0] != "QtiPlot") ? false : true;
	if (!qtiProject){
		if (QFile::exists(fname + "~")){
			int choice = QMessageBox::question(d_app, tr("QtiPlot - File opening error"),
					tr("The file <b>%1</b> is corrupted, but there exists a backup copy.<br>Do you want to open the backup instead?").arg(fn),
					QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
			if (choice == QMessageBox::Yes)
				return open(fname + "~");
			else
				QMessageBox::critical(d_app, tr("QtiPlot - File opening error"),  tr("The file: <b> %1 </b> was not created using QtiPlot!").arg(fn));
			return 0;
		}

		return d_app->plotFile(fn);
	}

	QStringList vl = lst[1].split(".", Qt::SkipEmptyParts);
	d_app->d_file_version = 100*(vl[0]).toInt()+10*(vl[1]).toInt()+(vl[2]).toInt();

	ApplicationWindow* app = openProject(fname, factorySettings, newProject);

	f.close();
	return app;
}

void ProjectManager::openRecentProject(QAction *action)
{
	if (!action)
		return;
	QString fn = action->text();
	int pos = fn.indexOf(" ", 0);
	fn = fn.right(fn.length() - pos - 1);

	if (d_app->projectname != "untitled"){
		QFileInfo fi(d_app->projectname);
		QString pn = fi.absoluteFilePath();

		if (QDir::toNativeSeparators(fn) == QDir::toNativeSeparators(pn)){
			QMessageBox::warning(d_app, tr("QtiPlot - File openning error"),
					tr("The file: <p><b> %1 </b><p> is the current file!").arg(QDir::toNativeSeparators(fn)));
			return;
		}
	}

	if (isProjectFile(fn)){
		if (showSaveProjectMessage() == QMessageBox::Cancel)
			return;
	}

	QFile f(fn);
	if (!f.exists()){
		QMessageBox::critical(d_app, tr("QtiPlot - File Open Error"),
				tr("The file: <b> %1 </b> <p>does not exist anymore!"
					"<p>It will be removed from the list.").arg(fn));

		for (int i = 0; i < d_app->recentProjects.size(); i++){
			if (QDir::toNativeSeparators(d_app->recentProjects[i]) == fn){
				d_app->recentProjects.removeAt(i);
				break;
			}
		}
		updateRecentProjectsList();
		return;
	}

	if (!fn.isEmpty()){
		d_app->saveSettings();//the recent projects must be saved
	#ifdef BROWSER_PLUGIN
		if (isProjectFile(fn))
			closeProject();
		open (fn, false, false);
	#else
		bool isSaved = d_app->saved;
		ApplicationWindow * a = open (fn);
		if (a){
			if (isSaved)
				d_app->savedProject();//force saved state
			if (isProjectFile(fn))
				d_app->close();
			else
				d_app->modifiedProject();
		}
	#endif
	}
}

ApplicationWindow* ProjectManager::openProject(const QString& fn, bool factorySettings, bool newProject)
{
	return ProjectSerializer::openProject(fn, d_app, factorySettings, newProject, d_app->d_file_version);
}

Folder* ProjectManager::appendProject(const QString& fn, Folder* parentFolder)
{
	return ProjectSerializer::appendProject(fn, parentFolder, d_app, d_app->d_file_version);
}

void ProjectManager::updateRecentProjectsList(const QString& fn)
{
	QString nativeFileName = QDir::toNativeSeparators(fn);
	if (!nativeFileName.isEmpty()){
		d_app->recentProjects.removeAll(nativeFileName);
		d_app->recentProjects.push_front(nativeFileName);
	}

	if (d_app->recentProjects.isEmpty())
		return;

	while ((int)d_app->recentProjects.size() > ApplicationWindow::MaxRecentProjects)
		d_app->recentProjects.pop_back();

	if (d_app->recent) {
		d_app->recent->clear();
		for (int i = 0; i<(int)d_app->recentProjects.size(); i++ )
			d_app->recent->addAction("&" + QString::number(i+1) + " " + QDir::toNativeSeparators(d_app->recentProjects[i]));
	}
}

QString ProjectManager::getSaveProjectName(const QString& fileName, bool *compress, int scope)
{
	QString fn = fileName;
	if (fileName.isEmpty()){
		QString filter = tr("QtiPlot project") + " (*.qti);";
		filter += tr("Compressed QtiPlot project") + " (*.qti.gz)";

		QString windowTitle = tr("Save Project As");
		if (scope == 1)
			windowTitle = tr("Save Folder As");
		else if (scope == 2)
			windowTitle = tr("Save Window As");

		QString selectedFilter;
		fn = d_app->getFileName(d_app, windowTitle, d_app->workingDir, filter, &selectedFilter, true, d_app->d_confirm_overwrite);
		if (compress && selectedFilter.contains(".gz"))
			*compress = true;
	}

	if (!fn.isEmpty()){
		QFileInfo fi(fn);
		d_app->workingDir = fi.absolutePath();
		if (fn.endsWith(".qti.gz", Qt::CaseInsensitive))
			fn.remove(".gz");
		if (!fn.endsWith(".qti", Qt::CaseInsensitive))
			fn.append(".qti");
	}
	return fn;
}
