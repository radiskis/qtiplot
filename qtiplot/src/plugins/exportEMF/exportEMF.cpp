/*
#
#   Copyright c 2011 Stephan Zevenhuizen,
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
#include "exportEMF.h"
#include <Matrix.h>
#include <EmfEngine.h>
//#include <iostream>

QStringList exportEMF::exportFormats() const
{
	//std::cout << "In exportFormats.\n";
	QStringList list = QStringList() << "emf";
	return list;
}

bool exportEMF::exportMatrix(Matrix * m, const QString & fname, bool exportSelection)
{
	//std::cout << "In exportMatrix.\n";
	int width = m -> numRows();
	int height = m -> numCols();
	EmfPaintDevice emf(QSize(width, height), fname);
	QPainter p(& emf);
	p.drawImage(QRect(0, 0, width, height), m -> matrixModel() -> renderImage());
	p.end();
	return true;
}

bool exportEMF::exportGraph(Graph * g, const QString & fname, const QSizeF & customSize, int unit, double fontsFactor)
{
	//std::cout << "In exportGraph.\n";
	int res = g -> logicalDpiX();
	QSize size = g -> boundingRect().size();
	if (customSize.isValid()) {
		// size = Graph::customPrintSize(customSize, unit, res); 
    }
	EmfPaintDevice emf(size, fname);
	g -> draw(& emf, size, fontsFactor);
	return true;
}

bool exportEMF::exportMultiLayerPlot(MultiLayer * m, const QString & fname, const QSizeF & customSize, int unit, double fontsFactor)
{
	//std::cout << "In exportMultiLayerPlot.\n";
	int res = m -> logicalDpiX();
	QSize size = m -> canvas() -> size();
	if (customSize.isValid()) {
		// size = Graph::customPrintSize(customSize, unit, res); 
    }
	EmfPaintDevice emf(size, fname);
	m -> draw(& emf, customSize, unit, res, fontsFactor);		
	return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(FreeSoftwareQtiPlotExportEMF, exportEMF);
#endif
