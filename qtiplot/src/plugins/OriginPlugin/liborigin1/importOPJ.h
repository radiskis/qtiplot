/***************************************************************************
    File                 : importOPJ.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright © 2011 Stephan Zevenhuizen
    Copyright            : (C) 2006-2007 by Ion Vasilief, Alex Kargovsky
    Email (use @ for *)  : ion_vasilief*yahoo.fr, kargovsky*yumr.phys.msu.su
    Description          : Origin project import class

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
#ifndef IMPORTOPJ_H
#define IMPORTOPJ_H

#include "ApplicationWindow.h"
#include <OriginFile.h>
#include <ScaleEngine.h>
#include "qwt3d_types.h"
#include "qwt3d_coordsys.h"
#include <qwt_symbol.h>

//! Origin project import class
class ImportOPJ
{
public:
	ImportOPJ(ApplicationWindow *app, const QString& filename);

	bool createProjectTree(const OriginFile& opj);
	bool importTables(const OriginFile& opj);
	bool importGraphs(const OriginFile& opj);
	bool importNotes(const OriginFile& opj);
	bool importGraph3D(const OriginFile& opj, unsigned int graph, unsigned int layer);
	int error(){return parse_error;};

private:
	void importSpectrogram(Graph *graph, Spectrogram *sp, const Origin::GraphLayer& layer, const Origin::GraphCurve& _curve, double fFontScaleFactor);
	void parseXYZContourPlotAxisTitles(Graph *g, Table *t, const Origin::GraphCurve& curve);
	void convertDoubleAxesPlot(MultiLayer *ml);

	QwtSymbol::Style originToQwtSymbolStyle(unsigned char type);
	LinearColorMap qwtColorMap(const Origin::ColorMap& colorMap);
    int arrowAngle(double length, double width){return ceil(45*atan(0.5*width/length)/atan(1.0));};
	QString parseOriginText(const QString &str);
	QString parseOriginTags(const QString &str);
	void addText(const Origin::TextBox& text, Graph* graph, double fFontScaleFactor, double fScale);
	void setPieTexts(PieCurve *p, Graph* graph, const Origin::GraphLayer& layer, double fFontScaleFactor, double fScale);
	int parse_error;
	int xoffset;
	int frameWidth;
	int frameHeight;
	ApplicationWindow *mw;

	static QMap<Origin::GraphCurve::LineStyle, Qt::PenStyle> lineStyles;
	static QMap<Origin::GraphCurve::LineStyle, Qwt3D::LINESTYLE> line3DStyles;
	static QMap<Origin::GraphAxis::Scale, ScaleTransformation::Type>scaleTypes;
	static QMap<Origin::FillPattern, int> patternStyles;
	static QMap<Origin::ProjectNode::NodeType, QString> classes;
};

#endif //IMPORTOPJ_H
