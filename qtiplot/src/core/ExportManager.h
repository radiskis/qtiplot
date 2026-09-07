/***************************************************************************
    File                 : ExportManager.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Plot, table, and document export and print manager

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                   *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                            *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                            *
 *                                                                         *
 ***************************************************************************/

#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

class ApplicationWindow;
class Graph;
class MultiLayer;
class Table;
class Matrix;
class ExportDialog;
class QPrinter;

class ExportManager : public QObject
{
    Q_OBJECT

public:
    explicit ExportManager(ApplicationWindow *app);
    virtual ~ExportManager();

    void exportMatrix(const QString& exportFilter);
    void exportExcel();
    void exportOds();
    void exportGraph(const QString& exportFilter);
    void exportLayer();
    void exportPresentationODF();
    void exportAllGraphs();
    ExportDialog* showExportASCIIDialog();
    void exportAllTables(const QString& dir, const QString& filter, const QString& sep, bool colNames, bool colComments, bool expSelection);
    void exportPDF();
    void print();
    void printPreview();
    void setPrintPreviewOptions(QPrinter *printer);
    void printAllPlots();

private:
    ApplicationWindow *d_app;
};

#endif // EXPORT_MANAGER_H
