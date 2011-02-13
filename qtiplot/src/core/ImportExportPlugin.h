/***************************************************************************
	File                 : ImportExportPlugin.h
	Project              : QtiPlot
	--------------------------------------------------------------------
	Copyright            : (C) 2011 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Base class for import/export plugins

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
#ifndef INTERFACES_H
#define INTERFACES_H

#include <QtPlugin>
#include <ApplicationWindow.h>

class QStringList;

class ImportExportPlugin
{
public:
	virtual ~ImportExportPlugin(){}

	virtual QStringList importFormats(){return QStringList();};
	virtual Table* import(const QString & /*fileName*/, int /*sheet*/ = -1){return 0;}

	virtual QStringList exportFormats() const {return QStringList();};
	virtual bool exportTable(Table *, const QString& /*fname*/, bool /*withLabels*/, bool /*exportComments*/, bool /*exportSelection*/){return false;}
	virtual bool exportMatrix(Matrix *, const QString& /*fname*/, bool /*exportSelection*/){return false;}
	virtual bool exportGraph(Graph *, const QString& /*fname*/, const QSizeF& /*customSize*/, int /*unit*/, double /*fontsFactor*/ = 1){return false;}
	virtual bool exportMultiLayerPlot(MultiLayer *, const QString& /*fname*/, const QSizeF& /*customSize*/, int /*unit*/, double /*fontsFactor*/ = 1){return false;}

	void setApplicationWindow(ApplicationWindow *app){d_app = app;};
	ApplicationWindow *applicationWindow(){return d_app;};

private:
	ApplicationWindow *d_app;
};

Q_DECLARE_INTERFACE(ImportExportPlugin, "com.ProIndependent.QtiPlot.ImportExportPlugin/1.0")
#endif
