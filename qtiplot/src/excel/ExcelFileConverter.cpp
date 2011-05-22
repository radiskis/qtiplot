/***************************************************************************
	File                 : ExcelFileConverter.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : An object converting Excel files to Open Document Format Spreadsheets (.ods)

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
#include "ExcelFileConverter.h"
#include "ApplicationWindow.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>

ExcelFileConverter::ExcelFileConverter(const QString& fileName, ApplicationWindow *app, bool keepInputFile)
: QObject(app),
d_file_name(fileName),
soffice(0),
java(0),
d_keep_input_file(keepInputFile)
{
#ifdef Q_WS_X11
	d_soffice_already_running = false;
	QProcess *ps = new QProcess();
	ps->start("ps -ef");
	if (ps->waitForFinished() && ps->readAllStandardOutput().contains("soffice"))
		d_soffice_already_running = true;
	ps->kill();
#endif
	startOpenOfficeServer();
}

void ExcelFileConverter::startOpenOfficeServer()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	soffice = new QProcess(app);
	connect(soffice, SIGNAL(started()), this, SLOT(startConvertion()));
	connect(soffice, SIGNAL(error(QProcess::ProcessError)),
			this, SLOT(displayOfficeError(QProcess::ProcessError)));

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString program = app->d_soffice_path;
	QStringList arguments;
	arguments << "-headless";
	arguments << "-accept=socket,host=127.0.0.1,port=8100;urp;";
	arguments << "-nofirststartwizard";

	soffice->start(program, arguments);
	soffice->waitForStarted();
}

void ExcelFileConverter::startConvertion()
{
	if (!soffice)
		return;

	if (d_keep_input_file)
		d_output_file = QDir::tempPath() + "/" + QFileInfo(d_file_name).baseName() + ".ods";
	else {
		d_output_file = d_file_name;
		d_output_file.replace(".xls", ".ods");
	}

	if (QFile::exists(d_output_file) || java)
		return;

	ApplicationWindow *app = (ApplicationWindow *)parent();

	java = new QProcess(app);
	connect(java, SIGNAL(finished(int, QProcess::ExitStatus)),
			this, SLOT(finish(int, QProcess::ExitStatus)));
	connect(java, SIGNAL(error(QProcess::ProcessError)),
			this, SLOT(displayJavaError(QProcess::ProcessError)));

	QString compiler = app->d_java_path;

	QStringList arguments;
	arguments << "-jar";
	arguments << app->d_jodconverter_path;
	arguments << d_file_name << d_output_file;

	java->start(compiler, arguments);
	java->waitForFinished();
}

void ExcelFileConverter::finish(int, QProcess::ExitStatus exitStatus)
{
	QApplication::restoreOverrideCursor();

	if (exitStatus != QProcess::NormalExit){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot"), tr("Operation failed"));
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!QFile::exists(d_output_file)){
		if (!QFile::exists(app->d_jodconverter_path)){
			QMessageBox::critical(app, tr("Operation failed"),
			tr("Coudn't start") + " <a href=\"http://www.artofsolving.com/opensource/jodconverter\">" + tr("JODConverter") + "</a>" + "<br>"
			+ tr("Please set the correct path in the preferences dialog") + "!");
			return;
		} else {
			if (java){
				java->kill();
				java = 0;
			}
			startConvertion();
			return;
		}
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (!d_keep_input_file)
		QFile::remove(d_file_name);

	if (soffice)
		soffice->kill();

#ifdef Q_WS_X11
	if (!d_soffice_already_running)
		QProcess::execute("killall soffice.bin");
#endif

	QApplication::restoreOverrideCursor();
}

void ExcelFileConverter::displayJavaError(QProcess::ProcessError error)
{
	if (java && java->pid()){
		displayError("<a href=\"http://www.java.com/\">" + tr("Java") + "</a>", error);

		java->kill();
		java = 0;
	}
}

void ExcelFileConverter::displayOfficeError(QProcess::ProcessError error)
{
	if (soffice && soffice->pid())
		displayError("<a href=\"http://www.openoffice.org/\">" + tr("OpenOffice.org") + "</a>/" +
					 "<a href=\"http://www.documentfoundation.org/\">" + tr("LibreOffice") + "</a>", error);

	soffice->kill();
	soffice = 0;
}

void ExcelFileConverter::displayError(const QString& process, QProcess::ProcessError error)
{
	QString msg;
	switch(error){
		case QProcess::FailedToStart:
				msg = tr("Couldn't start") + " " +  process + "<br>" +
				tr("Please set the correct path in the preferences dialog");
		break;

		case QProcess::Crashed:
			msg = process + " " + tr("crashed");
		break;

		case QProcess::Timedout:
			msg = process + " " + tr("timedout");
		break;

		case QProcess::WriteError:
			msg = process + " " + tr("write error");
		break;

		case QProcess::ReadError:
			msg = process + " " + tr("read error");
		break;

		case QProcess::UnknownError:
			msg =  process + " " + tr("unknown error");
		break;
	}

	QApplication::restoreOverrideCursor();
	QMessageBox::critical((ApplicationWindow *)parent(), tr("Operation failed"), msg + "!");
}
