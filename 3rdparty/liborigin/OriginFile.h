/***************************************************************************
    File                 : OriginFile.h
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2007 Stefan Gerlach
                           (C) 2007-2008 Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin file import class

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

#ifndef ORIGIN_FILE_H
#define ORIGIN_FILE_H

/* version 0.0 2007-09-26 */
#define LIBORIGIN_VERSION 0x00070926
#define LIBORIGIN_VERSION_STRING "2007-09-26"

#include "OriginObj.h"
#include "OriginParser.h"
#include <memory>

using namespace std;

class OriginFile
{
public:
	OriginFile(const char* fileName);

	int parse();
	double Version() const { return version/100.0; }		//!< get version of project file

	const tree<Origin::ProjectNode>* project() const { return &parser->projectTree; }
	//spreadsheet properties
	int numSpreads() const { return parser->speadSheets.size(); }			//!< get number of spreadsheets
	const char *spreadName(int s) const { return parser->speadSheets[s].name.c_str(); }	//!< get name of spreadsheet s
	bool spreadHidden(int s) const { return parser->speadSheets[s].hidden; }	//!< is spreadsheet s hidden
	bool spreadLoose(int s) const { return parser->speadSheets[s].loose; }	//!< is spreadsheet s loose
	Origin::Rect spreadWindowRect(int s) const { return parser->speadSheets[s].clientRect; }		//!< get window rectangle of spreadsheet s
	const char *spreadLabel(int s) const { return parser->speadSheets[s].label.c_str(); }	//!< get label of spreadsheet s
	boost::posix_time::ptime spreadCreationDate(int s) const { return parser->speadSheets[s].creationDate; }	//!< get creation date of spreadsheet s
	boost::posix_time::ptime spreadModificationDate(int s) const { return parser->speadSheets[s].modificationDate; }	//!< get modification date of spreadsheet s
	Origin::Window::State spreadState(int s) const { return parser->speadSheets[s].state; }	//!< get window state of spreadsheet s
	Origin::Window::Title spreadTitle(int s) const { return parser->speadSheets[s].title; }	//!< get window state of spreadsheet s
	int numCols(int s) const { return parser->speadSheets[s].columns.size(); }		//!< get number of columns of spreadsheet s
	int numRows(int s,int c) const { return parser->speadSheets[s].columns[c].data.size(); }	//!< get number of rows of column c of spreadsheet s
	int maxRows(int s) const { return parser->speadSheets[s].maxRows; }		//!< get maximum number of rows of spreadsheet s

	//spreadsheet's column properties
	const char *colName(int s, int c) const { return parser->speadSheets[s].columns[c].name.c_str(); }	//!< get name of column c of spreadsheet s
	Origin::SpreadColumn::ColumnType colType(int s, int c) const { return parser->speadSheets[s].columns[c].type; }	//!< get type of column c of spreadsheet s
	const char *colCommand(int s, int c) const { return parser->speadSheets[s].columns[c].command.c_str(); }	//!< get command of column c of spreadsheet s
	const char *colComment(int s, int c) const { return parser->speadSheets[s].columns[c].comment.c_str(); }	//!< get comment of column c of spreadsheet s
	Origin::ValueType colValueType(int s, int c) const { return parser->speadSheets[s].columns[c].valueType; }	//!< get value type of column c of spreadsheet s
	int colValueTypeSpec(int s, int c) const { return parser->speadSheets[s].columns[c].valueTypeSpecification; }	//!< get value type specification of column c of spreadsheet s
	int colSignificantDigits(int s, int c) const { return parser->speadSheets[s].columns[c].significantDigits; }	//!< get significant digits of column c of spreadsheet s
	int colDecPlaces(int s, int c) const { return parser->speadSheets[s].columns[c].decimalPlaces; }	//!< get decimal places of column c of spreadsheet s
	Origin::NumericDisplayType colNumDisplayType(int s, int c) const { return parser->speadSheets[s].columns[c].numericDisplayType; }	//!< get numeric display type of column c of spreadsheet s
	int colWidth(int s, int c) const { return parser->speadSheets[s].columns[c].width; }	//!< get width of column c of spreadsheet s
	Origin::variant oData(int s, int c, int r) const {
		return parser->speadSheets[s].columns[c].data[r];
	}	//!< get data of column c/row r of spreadsheet s

	//Matrix properties
	int numMatrices() const { return parser->matrixes.size(); }			//!< get number of matrices
	const char *matrixName(int m) const { return parser->matrixes[m].name.c_str(); }	//!< get name of matrix m
	bool matrixHidden(int m) const { return parser->matrixes[m].hidden; }	//!< is matrix m hidden
	Origin::Rect matrixWindowRect(int m) const { return parser->matrixes[m].clientRect; }		//!< get window rectangle of matrix m
	const char *matrixLabel(int m) const { return parser->matrixes[m].label.c_str(); }	//!< get label of matrix m
	boost::posix_time::ptime matrixCreationDate(int m) const { return parser->matrixes[m].creationDate; }	//!< get creation date of matrix m
	boost::posix_time::ptime matrixModificationDate(int m) const { return parser->matrixes[m].modificationDate; }	//!< get modification date of matrix m
	Origin::Window::State matrixState(int m) const { return parser->matrixes[m].state; }	//!< get window state of matrix m
	Origin::Window::Title matrixTitle(int m) const { return parser->matrixes[m].title; }	//!< get window state of matrix m
	unsigned short numMatrixCols(int m) const { return parser->matrixes[m].columnCount; }		//!< get number of columns of matrix m
	unsigned short numMatrixRows(int m) const { return parser->matrixes[m].rowCount; }	//!< get number of rows of matrix m
	const char *matrixFormula(int m) const { return parser->matrixes[m].command.c_str(); }	//!< get formula of matrix m
	int matrixValueTypeSpec(int m) const { return parser->matrixes[m].valueTypeSpecification; }	//!< get value type specification of matrix m
	int matrixSignificantDigits(int m) const { return parser->matrixes[m].significantDigits; }	//!< get significant digits of matrix m
	int matrixDecPlaces(int m) const { return parser->matrixes[m].decimalPlaces; }	//!< get decimal places of matrix m
	Origin::NumericDisplayType matrixNumDisplayType(int m) const { return parser->matrixes[m].numericDisplayType; }	//!< get numeric display type of matrix m
	int matrixWidth(int m) const { return parser->matrixes[m].width; }	//!< get width of matrix m
	Origin::Matrix::ViewType matrixViewType(int m) const { return parser->matrixes[m].view; }	//!< get view type of matrix m
	Origin::Matrix::HeaderViewType matrixHeaderViewType(int m) const { return parser->matrixes[m].header; }	//!< get header view type of matrix m
	double matrixData(int m, int c, int r) const { return parser->matrixes[m].data[r*parser->matrixes[m].columnCount+c]; }	//!< get data of row r of column c of matrix m
	vector<double> matrixData(int m) const { return parser->matrixes[m].data; }	//!< get data of matrix m

	//Function properties
	int numFunctions() const { return parser->functions.size(); }			//!< get number of functions
	int functionIndex(const char* s) const { return parser->findFunctionByName(s); }	//!< get name of function s
	const char *functionName(int s) const { return parser->functions[s].name.c_str(); }	//!< get name of function s
	int functionType(int s) const { return parser->functions[s].type; }		//!< get type of function s
	double functionBegin(int s) const { return parser->functions[s].begin; }	//!< get begin of interval of function s
	double functionEnd(int s) const { return parser->functions[s].end; }	//!< get end of interval of function s
	int functionPoints(int s) const { return parser->functions[s].totalPoints; }	//!< get number of points in interval of function s
	const char *functionFormula(int s) const { return parser->functions[s].formula.c_str(); }	//!< get formula of function s

	//graph properties
	int numGraphs() const { return parser->graphs.size(); }			//!< get number of graphs
	const char *graphName(int s) const { return parser->graphs[s].name.c_str(); }	//!< get name of graph s
	const char *graphLabel(int s) const { return parser->graphs[s].label.c_str(); }	//!< get name of graph s
	boost::posix_time::ptime graphCreationDate(int s) const { return parser->graphs[s].creationDate; }	//!< get creation date of graph s
	boost::posix_time::ptime graphModificationDate(int s) const { return parser->graphs[s].modificationDate; }	//!< get modification date of graph s
	Origin::Window::State graphState(int s) const { return parser->graphs[s].state; }	//!< get window state of graph s
	Origin::Window::Title graphTitle(int s) const { return parser->graphs[s].title; }	//!< get window state of graph s
	bool graphHidden(int s) const { return parser->graphs[s].hidden; }	//!< is graph s hidden
	Origin::Rect graphRect(int s) const { return Origin::Rect(parser->graphs[s].width, parser->graphs[s].height); }		//!< get rectangle of graph s
	Origin::Rect graphWindowRect(int s) const { return parser->graphs[s].clientRect; }		//!< get window rectangle of graph s
	int numLayers(int s) const { return parser->graphs[s].layers.size(); }			//!< get number of layers of graph s
	Origin::Rect layerRect(int s, int l) const { return parser->graphs[s].layers[l].clientRect; }		//!< get rectangle of layer l of graph s
	Origin::TextBox layerXAxisTitle(int s, int l) const { return parser->graphs[s].layers[l].xAxis.label; }		//!< get label of X-axis of layer l of graph s
	Origin::TextBox layerYAxisTitle(int s, int l) const { return parser->graphs[s].layers[l].yAxis.label; }		//!< get label of Y-axis of layer l of graph s
	Origin::TextBox layerLegend(int s, int l) const { return parser->graphs[s].layers[l].legend; }		//!< get legend of layer l of graph s
	vector<Origin::TextBox> layerTexts(int s, int l) const { return parser->graphs[s].layers[l].texts; } //!< get texts of layer l of graph s
	vector<Origin::Line> layerLines(int s, int l) const { return parser->graphs[s].layers[l].lines; } //!< get lines of layer l of graph s
	vector<Origin::Bitmap> layerBitmaps(int s, int l) const { return parser->graphs[s].layers[l].bitmaps; } //!< get bitmaps of layer l of graph s
	Origin::GraphAxisBreak layerXBreak(int s, int l) const { return parser->graphs[s].layers[l].xAxisBreak; } //!< get break of horizontal axis of layer l of graph s
	Origin::GraphAxisBreak layerYBreak(int s, int l) const { return parser->graphs[s].layers[l].yAxisBreak; } //!< get break of vertical axis of layer l of graph s
	Origin::GraphLayerRange layerXRange(int s, int l) const {
		return Origin::GraphLayerRange(parser->graphs[s].layers[l].xAxis.min, parser->graphs[s].layers[l].xAxis.max, parser->graphs[s].layers[l].xAxis.step);
	} //!< get X-range of layer l of graph s
	Origin::GraphLayerRange layerYRange(int s, int l) const {
		return Origin::GraphLayerRange(parser->graphs[s].layers[l].yAxis.min, parser->graphs[s].layers[l].yAxis.max, parser->graphs[s].layers[l].yAxis.step);
	} //!< get Y-range of layer l of graph s
	vector<int> layerXTicks(int s, int l) const {
		vector<int> tick;
		tick.push_back(parser->graphs[s].layers[l].xAxis.majorTicks);
		tick.push_back(parser->graphs[s].layers[l].xAxis.minorTicks);
		return tick;
	} //!< get X-axis ticks of layer l of graph s
	vector<int> layerYTicks(int s, int l) const {
		vector<int> tick;
		tick.push_back(parser->graphs[s].layers[l].yAxis.majorTicks);
		tick.push_back(parser->graphs[s].layers[l].yAxis.minorTicks);
		return tick;
	} //!< get Y-axis ticks of layer l of graph s
	vector<Origin::GraphGrid> layerGrid(int s, int l) const {
		vector<Origin::GraphGrid> grid;
		grid.push_back(parser->graphs[s].layers[l].xAxis.majorGrid);
		grid.push_back(parser->graphs[s].layers[l].xAxis.minorGrid);
		grid.push_back(parser->graphs[s].layers[l].yAxis.majorGrid);
		grid.push_back(parser->graphs[s].layers[l].yAxis.minorGrid);
		return grid;
	} //!< get grid of layer l of graph s
	vector<Origin::GraphAxisFormat> layerAxisFormat(int s, int l) const {
		vector<Origin::GraphAxisFormat> format;
		format.push_back(parser->graphs[s].layers[l].yAxis.formatAxis[0]); //bottom
		format.push_back(parser->graphs[s].layers[l].yAxis.formatAxis[1]); //top
		format.push_back(parser->graphs[s].layers[l].xAxis.formatAxis[0]); //left
		format.push_back(parser->graphs[s].layers[l].xAxis.formatAxis[1]); //right
		return format;
	} //!< get title and format of axes of layer l of graph s
	vector<Origin::GraphAxisTick> layerAxisTickLabels(int s, int l) const {
		vector<Origin::GraphAxisTick> tick;
		tick.push_back(parser->graphs[s].layers[l].yAxis.tickAxis[0]); //bottom
		tick.push_back(parser->graphs[s].layers[l].yAxis.tickAxis[1]); //top
		tick.push_back(parser->graphs[s].layers[l].xAxis.tickAxis[0]); //left
		tick.push_back(parser->graphs[s].layers[l].xAxis.tickAxis[1]); //right
		return tick;
	} //!< get tick labels of axes of layer l of graph s
	vector<double> layerHistogram(int s, int l) const {
		vector<double> range;
		range.push_back(parser->graphs[s].layers[l].histogramBin);
		range.push_back(parser->graphs[s].layers[l].histogramBegin);
		range.push_back(parser->graphs[s].layers[l].histogramEnd);
		return range;
	} //!< get histogram bin of layer l of graph s
	int layerXScale(int s, int l) const { return parser->graphs[s].layers[l].xAxis.scale; }		//!< get scale of X-axis of layer l of graph s
	int layerYScale(int s, int l) const { return parser->graphs[s].layers[l].yAxis.scale; }		//!< get scale of Y-axis of layer l of graph s
	int numCurves(int s, int l) const { return parser->graphs[s].layers[l].curves.size(); }			//!< get number of curves of layer l of graph s
	const char *curveDataName(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].dataName.c_str(); }	//!< get data source name of curve c of layer l of graph s
	const char *curveXColName(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].xColumnName.c_str(); }	//!< get X-column name of curve c of layer l of graph s
	const char *curveYColName(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].yColumnName.c_str(); }	//!< get Y-column name of curve c of layer l of graph s
	int curveType(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].type; }	//!< get type of curve c of layer l of graph s
	int curveLineStyle(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].lineStyle; }	//!< get line style of curve c of layer l of graph s
	unsigned char curveLineColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].lineColor; }	//!< get line color of curve c of layer l of graph s
	int curveLineConnect(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].lineConnect; }	//!< get line connect of curve c of layer l of graph s
	double curveLineWidth(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].lineWidth; }	//!< get line width of curve c of layer l of graph s

	bool curveIsFilledArea(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillArea; }	//!< get is filled area of curve c of layer l of graph s
	unsigned char curveFillAreaColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaColor; }	//!< get area fillcolor of curve c of layer l of graph s
	unsigned char curveFillAreaFirstColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaFirstColor; }	//!< get area first fillcolor of curve c of layer l of graph s
	int curveFillPattern(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPattern; }	//!< get fill pattern of curve c of layer l of graph s
	unsigned char curveFillPatternColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPatternColor; }	//!< get fill pattern color of curve c of layer l of graph s
	double curveFillPatternWidth(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPatternWidth; }	//!< get fill pattern line width of curve c of layer l of graph s
	int curveFillPatternBorderStyle(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPatternBorderStyle; }	//!< get fill pattern border style of curve c of layer l of graph s
	unsigned char curveFillPatternBorderColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPatternBorderColor; }	//!< get fill pattern border color of curve c of layer l of graph s
	double curveFillPatternBorderWidth(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].fillAreaPatternBorderWidth; }	//!< get fill pattern border line width of curve c of layer l of graph s

	int curveSymbolType(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].symbolType; }	//!< get symbol type of curve c of layer l of graph s
	unsigned char curveSymbolColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].symbolColor; }	//!< get symbol color of curve c of layer l of graph s
	unsigned char curveSymbolFillColor(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].symbolFillColor; }	//!< get symbol fill color of curve c of layer l of graph s
	double curveSymbolSize(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].symbolSize; }	//!< get symbol size of curve c of layer l of graph s
	int curveSymbolThickness(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].symbolThickness; }	//!< get symbol thickness of curve c of layer l of graph s

	Origin::PieProperties curvePieProperties(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].pie; }	//!< get pie properties of curve c of layer l of graph s
	Origin::VectorProperties curveVectorProperties(int s, int l, int c) const { return parser->graphs[s].layers[l].curves[c].vector; }	//!< get vector properties of curve c of layer l of graph s

	int numNotes() const { return parser->notes.size(); }			//!< get number of notes
	const char *noteName(int n) const { return parser->notes[n].name.c_str(); }	//!< get name of note n
	const char *noteLabel(int n) const { return parser->notes[n].label.c_str(); }	//!< get label of note n
	const char *noteText(int n) const { return parser->notes[n].text.c_str(); }	//!< get text of note n
	boost::posix_time::ptime noteCreationDate(int n) const { return parser->notes[n].creationDate; }	//!< get creation date of note n
	boost::posix_time::ptime noteModificationDate(int n) const { return parser->notes[n].modificationDate; }	//!< get modification date of note n
	Origin::Window::State noteState(int n) const { return parser->notes[n].state; }	//!< get window state of note n
	Origin::Window::Title noteTitle(int n) const { return parser->notes[n].title; }	//!< get window state of note n

	const char* resultsLogString() const { return parser->resultsLog.c_str();}		//!< get Results Log

private:
	int version;
	auto_ptr<OriginParser> parser;
};

#endif // ORIGIN_FILE_H
