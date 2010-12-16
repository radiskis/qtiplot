/***************************************************************************
    File                 : OdsFileHandler.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : An XML handler for parsing Open Document Format Spreadsheets (.ods)

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
#ifndef OdsFileHandler_H
#define OdsFileHandler_H

#include <QXmlDefaultHandler>
#include <QDateTime>
#include <vector>

class ApplicationWindow;
class Table;

class OdsFileHandler : public QXmlDefaultHandler
{
public:
	OdsFileHandler(ApplicationWindow *app, const QString& odsFileName, const QString& legendName = QString::null);

    bool startElement(const QString &namespaceURI, const QString &localName,
                       const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                     const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &){return false;};
    QString errorString() const;

	Table *sheet(int index);
	int sheetsCount(){return d_tables.size();};

private:
	ApplicationWindow * d_app;
	QString d_ods_file_name;
	QString d_legend_name;
    QString currentText;
    QString errorStr;
	//Index of the last non-empty cell in a sheet
	unsigned int d_last_column;
	//Total number of rows in a sheet
	unsigned int d_rows;
	//Current column in a row
	int d_col;

	enum CellType {EmptyCell = 0, Float, String, Date, Time, Boolean, Currency, Percent};

	struct cell_data{
		int	row;
		int	col;
		double	d;
		QString	str;
		QDateTime date_time;
		CellType type;
	};

	std::vector <cell_data> cells;

	QStringList d_sheet_names;
	QList<Table *> d_tables;
};

#endif
