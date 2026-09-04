/***************************************************************************
	File                 : QtiPlotApplication.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2010 - 2011 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot application

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

#include <QtiPlotApplication.h>
#include <ApplicationWindow.h>
#include <QFileOpenEvent>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include "Logger.h"
#include <new>

#ifdef Q_WS_MAC
	void qt_mac_set_menubar_merge(bool enable);
	void qt_mac_set_dock_menu(QMenu *menu);
#endif

bool QtiPlotApplication::notify(QObject *receiver, QEvent *event)
{
	try {
		return QApplication::notify(receiver, event);
	} catch (const std::bad_alloc &) {
		qCCritical(lcIo) << "Memory exhaustion (std::bad_alloc) caught in application event dispatcher for object:"
		                 << (receiver ? receiver->objectName() : "unknown");
		QMessageBox::critical(activeWindow(), tr("QtiPlot - Out of Memory"),
			tr("There is not enough memory to complete the requested operation.<br>"
			   "Your current project and data remain intact. Please save your work."));
		return false;
	} catch (const std::exception &e) {
		qCCritical(lcIo) << "Uncaught standard exception in event dispatcher:" << e.what();
		QMessageBox::critical(activeWindow(), tr("QtiPlot - Error"),
			tr("An error occurred while processing this operation:<br><b>%1</b>").arg(e.what()));
		return false;
	}
}

QtiPlotApplication::QtiPlotApplication( int & argc, char ** argv) : QApplication( argc, argv)
{
	QStringList args;
	for (int i = 1; i < argc; ++i)
		args.append(QString::fromLocal8Bit(argv[i]));
	if (args.isEmpty()) {
		args = arguments();
		if (!args.isEmpty()) args.removeFirst();
	}

	if( (args.count() == 1) && (args[0] == "-m" || args[0] == "--manual") )
		ApplicationWindow::showStandAloneHelp();
	else if ( (args.count() == 1) && (args[0] == "-a" || args[0] == "--about") ) {
	#ifdef Q_OS_WIN
		QMessageBox *msg = ApplicationWindow::about();
		connect(msg, &QObject::destroyed, this, &QtiPlotApplication::quit);
	#else
		ApplicationWindow::about(false);
	#endif
	} else {
		bool factorySettings = false;
		if (args.contains("-d") || args.contains("--default-settings"))
			factorySettings = true;

		ApplicationWindow *mw = new ApplicationWindow(factorySettings);
		if (!args.contains("-X") && !args.contains("-c") && !args.contains("--console")) {
			mw->restoreApplicationGeometry();
		}

		if (mw->autoSearchUpdates && !args.contains("-X") && !args.contains("-c") && !args.contains("--console")){
			mw->autoSearchUpdatesRequest = true;
			mw->searchForUpdates();
		}
		mw->parseCommandLineArguments(args);
	}

	#ifdef Q_WS_MAC
		qt_mac_set_menubar_merge(false);
		updateDockMenu();
	#endif
}

void QtiPlotApplication::close()
{
	quit();
}

bool QtiPlotApplication::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::FileOpen:
	{
		QString file = static_cast<QFileOpenEvent *>(event)->file();
		if (!d_windows.isEmpty()){
			for (ApplicationWindow *w : d_windows){
				if (w->projectname == file){
					if (!w->isActiveWindow() && d_windows.count() > 1)
						activateWindow(w);
					return true;
				}
			}

			ApplicationWindow *mw = d_windows.last();
			if (mw){
				mw->open(file, false, true);
				if (mw->projectname == "untitled"){
					mw->close();
					d_windows.removeAll(mw);
				}
			}
		}

	#ifdef Q_WS_MAC
		updateDockMenu();
	#endif
		return true;
	}

	default:
		return QApplication::event(event);
	}
}

void QtiPlotApplication::remove(ApplicationWindow *w)
{
	d_windows.removeAll(w);

#ifdef Q_WS_MAC
	updateDockMenu();
#endif
}

void QtiPlotApplication::activateWindow(ApplicationWindow *w)
{
	if (!w)
		return;

	if (qApp->arguments().contains("-X"))
		return;

	((QWidget *)w)->activateWindow();
	w->raise();


#ifdef Q_WS_MAC
	updateDockMenu();
#endif
}

#ifdef Q_WS_MAC
void QtiPlotApplication::updateDockMenu()
{
	QMenu *dockMenu = new QMenu();

#ifdef QT_MAC_USE_COCOA
	for (ApplicationWindow *w : d_windows){
		QAction *a = dockMenu->addAction(w->windowTitle());
		a->setIconVisibleInMenu(true);
		a->setCheckable(true);
		a->setChecked(w == activeWindow() && !w->isMinimized());
		a->setData(QVariant(d_windows.indexOf(w)));
	}
	dockMenu->addSeparator();
	connect(dockMenu, &QMenu::triggered, this, QOverload<QAction *>::of(&QtiPlotApplication::activateWindow));
#endif

	dockMenu->addAction(QObject::tr("New Window"), this, &QtiPlotApplication::newWindow);

	qt_mac_set_dock_menu(dockMenu);
}

void QtiPlotApplication::activateWindow(QAction *a)
{
	if (!a)
		return;

	QVariant data = a->data();
	if (!data.isValid())
		return;
	activateWindow(d_windows.at(data.toInt()));
}

void QtiPlotApplication::newWindow()
{
	if (d_windows.isEmpty())
		return;

	ApplicationWindow *mw = new ApplicationWindow();
	if (!mw)
		return;

	mw->restoreApplicationGeometry();
	mw->initWindow();

	updateDockMenu();
}
#endif
