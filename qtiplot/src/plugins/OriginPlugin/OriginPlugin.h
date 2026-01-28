/*
#
#   Copyright © 2011 Stephan Zevenhuizen,
#   Condensed Matter and Interfaces, Debye Institute, Utrecht University.
#   OriginPlugin, plugin for QtiPlot 0.9.8.8, (06-11-2011).
#
*/
/*
    This file is part of OriginPlugin (plugin for QtiPlot 0.9.8.8).

    OriginPlugin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OriginPlugin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OriginPlugin.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ORIGINPLUGIN_H
#define ORIGINPLUGIN_H

#include <QObject>
#include <ImportExportPlugin.h>

class OriginPlugin : public QObject, ImportExportPlugin
{
	Q_OBJECT
    Q_INTERFACES(ImportExportPlugin)
public:
	QStringList importFormats();
	Table * import(const QString & fname, int sheet);
};

#endif