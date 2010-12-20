/***************************************************************************
	File                 : ExcelFileConverter.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : An object converting Excel files to Open Document Format Spreadsheets (.ods) or to CSV

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
#ifndef ExcelFileConverter_H
#define ExcelFileConverter_H

#include <QObject>
#include <QProcess>

class ApplicationWindow;
class Table;

class ExcelFileConverter : public QObject
{
	Q_OBJECT

public:
	enum OperatingMode{Import = 0, ConvertToCsv = 1, ConvertToOds = 2};

	ExcelFileConverter(const QString& fileName, int sheet, ApplicationWindow *app, const OperatingMode& mode = Import);
	Table *outputTable(){return d_table;};

private slots:
	void startConvertion();
	void displayJavaError(QProcess::ProcessError error);
	void displayOfficeError(QProcess::ProcessError error);
	void finish(int exitCode, QProcess::ExitStatus exitStatus);

private:
	void startOpenOfficeServer();
	void displayError(const QString& process, QProcess::ProcessError error);

	QString d_file_name;
	int d_sheet;
	OperatingMode d_operating_mode;
	QString d_output_file;
	QProcess *soffice, *java;
	Table *d_table;
#ifdef Q_WS_X11
	bool d_soffice_already_running;
#endif
};

#endif
