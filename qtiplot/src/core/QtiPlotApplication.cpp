/***************************************************************************
	File                 : QtiPlotApplication.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
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

#ifdef Q_WS_MAC
	void qt_mac_set_menubar_merge(bool enable);
#endif

QtiPlotApplication::QtiPlotApplication( int & argc, char ** argv) : QApplication( argc, argv)
{
	QStringList args = arguments();
	args.removeFirst(); // remove application name

	if( (args.count() == 1) && (args[0] == "-m" || args[0] == "--manual") )
		ApplicationWindow::showStandAloneHelp();
	else if ( (args.count() == 1) && (args[0] == "-a" || args[0] == "--about") ) {
	#ifdef Q_OS_WIN
		QMessageBox *msg = ApplicationWindow::about();
		connect(msg, SIGNAL(destroyed()), this, SLOT(quit()));
	#else
		ApplicationWindow::about(false);
	#endif
	} else {
		bool factorySettings = false;
		if (args.contains("-d") || args.contains("--default-settings"))
			factorySettings = true;

		ApplicationWindow *mw = new ApplicationWindow(factorySettings);
		mw->restoreApplicationGeometry();
	#ifdef QTIPLOT_SUPPORT
		mw->showDonationDialog();
	#endif
		if (mw->autoSearchUpdates){
			mw->autoSearchUpdatesRequest = true;
			mw->searchForUpdates();
		}
		mw->parseCommandLineArguments(args);
	}

	#ifdef QTIPLOT_DEMO
		QTimer::singleShot(600000, this, SLOT(close()));
	#endif

	#ifdef Q_WS_MAC
		qt_mac_set_menubar_merge(false);
	#endif
}

#ifdef QTIPLOT_DEMO
void QtiPlotApplication::close()
{
	ApplicationWindow *mw = d_windows.last();
	if (mw)
		mw->showDemoVersionMessage();

	quit();
}
#endif

bool QtiPlotApplication::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::FileOpen:
	{
		QString file = static_cast<QFileOpenEvent *>(event)->file();
		if (!d_windows.isEmpty()){
			foreach(ApplicationWindow *w, d_windows){
				if (w->projectname == file){
					if (!w->isActiveWindow() && d_windows.count() > 1){
						w->hide();
						w->show();
					}
					return true;
				}
			}

			ApplicationWindow *mw = d_windows.last();
			if (mw){
				ApplicationWindow *app = mw->open(file, false, true);
				if (mw->projectname == "untitled"){
					mw->close();
					d_windows.removeAll(mw);
				}
				if (app)
					d_windows << app;
			}
		}
		return true;
	}

	default:
		return QApplication::event(event);
	}
}
