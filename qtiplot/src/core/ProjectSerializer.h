/***************************************************************************
    File                 : ProjectSerializer.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Project file XML serialization and deserialization

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

#ifndef PROJECT_SERIALIZER_H
#define PROJECT_SERIALIZER_H

#include <QString>
#include <QStringList>

class ApplicationWindow;
class Folder;
class MdiSubWindow;
class Table;
class TableStatistics;
class Matrix;
class Note;
class Graph;
class MultiLayer;

class ProjectSerializer
{
public:
	//! Save folder hierarchy and all its windows to a .qti project file (optionally compressed)
	static bool saveFolder(Folder *folder, const QString& fn, bool compress, ApplicationWindow *app);

	//! Read and restore an entire project from file
	static ApplicationWindow* openProject(const QString& fn, ApplicationWindow *app, bool factorySettings, bool newProject, int &fileVersion);

	//! Append a project file as a subfolder to an existing project
	static Folder* appendProject(const QString& fn, Folder *parentFolder, ApplicationWindow *app, int &fileVersion);

	//! Restore a template file (.qpt, .qst, .qtt, .qmt)
	static MdiSubWindow* openTemplate(const QString& fn, ApplicationWindow *app, int &fileVersion);

	//! Restore individual MDI window types from serialized string lists
	static Note* openNote(ApplicationWindow* app, const QStringList &flist, int fileVersion);
	static Matrix* openMatrix(ApplicationWindow* app, const QStringList &flist, int fileVersion);
	static Table* openTable(ApplicationWindow* app, const QStringList &flist, int fileVersion);
	static TableStatistics* openTableStatistics(ApplicationWindow* app, const QStringList &flist, int fileVersion);
	static Graph* openGraph(ApplicationWindow* app, MultiLayer *plot, const QStringList &list, int fileVersion);
};

#endif // PROJECT_SERIALIZER_H
