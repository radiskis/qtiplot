/*
#
#   Copyright © 2011 Stephan Zevenhuizen,
#   Condensed Matter and Interfaces, Debye Institute, Utrecht University.
#   exportEMF plugin for QtiPlot 0.9.8.8, (26-10-2011).
#
*/
/*
    This file is part of exportEMF (plugin for QtiPlot 0.9.8.8).

    exportEMF is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    exportEMF is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with exportEMF.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EXPORTEMF_H
#define EXPORTEMF_H

#include <QObject>
#include <ImportExportPlugin.h>

class exportEMF : public QObject, ImportExportPlugin
{
	Q_OBJECT
    Q_INTERFACES(ImportExportPlugin)
public:
	QStringList exportFormats() const;
	bool exportMatrix(Matrix *, const QString & fname, bool exportSelection);
	bool exportGraph(Graph *, const QString & fname, const QSizeF & customSize, int unit, double fontsFactor);
	bool exportMultiLayerPlot(MultiLayer *, const QString & fname, const QSizeF & customSize, int unit, double fontsFactor);
};

#endif
