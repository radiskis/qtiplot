/***************************************************************************
    File                 : ProjectSerializer.cpp
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

#include "ProjectSerializer.h"
#include "ApplicationWindow.h"
#include "Folder.h"
#include "Table.h"
#include "TableStatistics.h"
#include "Matrix.h"
#include "Note.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Graph3D.h"
#include "PolarGraph.h"
#include "PlotCurve.h"
#include "ErrorBarsCurve.h"
#include "FunctionCurve.h"
#include "VectorCurve.h"
#include "BoxCurve.h"
#include "PieCurve.h"
#include "QwtHistogram.h"
#include "Grid.h"
#include "LegendWidget.h"
#include "ArrowMarker.h"
#include "ImageWidget.h"
#include "Spectrogram.h"
#include "SelectionMoveResizer.h"
#include "RectangleWidget.h"
#include "EllipseWidget.h"
#include <QMdiArea>
#include "ColorBox.h"
#include "TexWidget.h"
#include <qwt_scale_widget.h>
#include "ScreenPickerTool.h"
#include "globals.h"
#include "Logger.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QCursor>
#include <QDir>

extern "C"
{
void file_compress(char  *, char  *);
void file_uncompress(char  *);
}

Note* ProjectSerializer::openNote(ApplicationWindow* app, const QStringList &flist, int fileVersion)
{
	Q_UNUSED(fileVersion);
	if (!app || flist.isEmpty())
		return nullptr;

	QStringList lst = flist[0].split("\t", Qt::SkipEmptyParts);
	if (lst.isEmpty())
		return nullptr;

	QString caption = lst[0];
	Note* w = app->newNote(caption);
	if (!w)
		return nullptr;

	if (lst.count() >= 2){
		app->setListViewDate(caption, lst[1]);
		w->setBirthDate(lst[1]);
	}

	if (flist.size() >= 2)
		app->restoreWindowGeometry(w, flist[1]);

	if (flist.size() >= 3){
		lst = flist[2].split("\t");
		if (lst.size() >= 3){
			w->setWindowLabel(lst[1]);
			w->setCaptionPolicy((MdiSubWindow::CaptionPolicy)lst[2].toInt());
		}
	}
	return w;
}


Matrix* ProjectSerializer::openMatrix(ApplicationWindow* app, const QStringList &flist, int fileVersion)
{
	if (!app || flist.isEmpty())
		return nullptr;

	QStringList list = flist.first().split("\t");
	if (list.size() < 4)
		return nullptr;

	int rows = list[1].toInt();
	int cols = list[2].toInt();
	if (rows < 0 || cols < 0 || rows > 10000000 || cols > 100000 || (int64_t)rows * (int64_t)cols > 50000000LL)
		return nullptr;

	QString caption = list[0];
	Matrix* w = app->newMatrix(caption, rows, cols);
	if (!w)
		return nullptr;

	app->setListViewDate(caption, list[3]);
	w->setBirthDate(list[3]);
	w->restore(flist, fileVersion);
	return w;
}


Table* ProjectSerializer::openTable(ApplicationWindow* app, const QStringList &flist, int fileVersion)
{
	if (!app || flist.isEmpty())
		return nullptr;

	QStringList::const_iterator line = flist.begin();
	QStringList list = (*line).split("\t");
	if (list.size() < 4)
		return nullptr;

	QString caption = list[0];
	int rows = list[1].toInt();
	int cols = list[2].toInt();
	if (rows < 0 || cols < 0 || rows > 10000000 || cols > 100000 || (int64_t)rows * (int64_t)cols > 50000000LL)
		return nullptr;

	Table* w = app->newTable(caption, rows, cols);
	if (!w)
		return nullptr;

	app->setListViewDate(caption, list[3]);
	w->setBirthDate(list[3]);

	for (line++; line!=flist.end(); line++){
		QStringList fields = (*line).split("\t");
		if (fields[0] == "geometry" || fields[0] == "tgeometry") {
			app->restoreWindowGeometry(w, *line);
		} else if (fields[0] == "header") {
			fields.pop_front();
			if (fileVersion >= 78)
				w->loadHeader(fields);
			else if (list.size() >= 7){
				w->setColPlotDesignation(list[4].toInt(), Table::X);
				w->setColPlotDesignation(list[6].toInt(), Table::Y);
				w->setHeader(fields);
			}
		} else if (fields[0] == "ColWidth") {
			fields.pop_front();
			w->setColWidths(fields);
		} else if (fields[0] == "com") { // legacy code
			w->setCommands(*line);
		} else if (fields[0] == "<com>") {
			for (line++; line!=flist.end() && *line != "</com>"; line++){
				int col = (*line).mid(9,(*line).length()-11).toInt();
				QString formula;
				for (line++; line!=flist.end() && *line != "</col>"; line++)
					formula += *line + "\n";
				formula.truncate(formula.length()-1);
				w->setCommand(col,formula);
			}
		} else if (fields[0] == "ColType") { // fileVersion > 65
			fields.pop_front();
			w->setColumnTypes(fields);
		} else if (fields[0] == "Comments") { // fileVersion > 71
			fields.pop_front();
			w->setColComments(fields);
			w->setHeaderColType();
		} else if (fields[0] == "WindowLabel" && fields.size() >= 3) { // fileVersion > 71
			w->setWindowLabel(fields[1]);
			w->setCaptionPolicy((MdiSubWindow::CaptionPolicy)fields[2].toInt());
		} else if (fields[0] == "ReadOnlyColumn") { // fileVersion > 91
			fields.pop_front();
			for (int i=0; i < w->numCols(); i++)
				w->setReadOnlyColumn(i, fields[i] == "1");
		} else if (fields[0] == "HiddenColumn") { // fileVersion >= 93
			fields.pop_front();
			for (int i=0; i < w->numCols(); i++)
				w->hideColumn(i, fields[i] == "1");
		} else // <data> or values
			break;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	w->table()->blockSignals(true);
	for (line++; line!=flist.end() && *line != "</data>"; line++){//read and set table values
		QStringList fields = (*line).split("\t");
		int row = fields[0].toInt();
		for (int col=0; col<cols; col++){
		    if (fields.count() >= col+2){
		        QString cell = fields[col+1];
		        if (cell.isEmpty())
                    continue;

				if (w->columnType(col) == Table::Numeric){
		        	if (fileVersion < 90)
                    	w->setCell(row, col, QLocale::c().toDouble(cell.replace(",", ".")));
					else if (fileVersion == 90)
						w->setText(row, col, cell);
					else if (fileVersion >= 91)
						w->setCell(row, col, cell.toDouble());
		        } else
                    w->setText(row, col, cell);
		    }
		}
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
    QApplication::restoreOverrideCursor();

	w->table()->blockSignals(false);
	return w;
}



TableStatistics* ProjectSerializer::openTableStatistics(ApplicationWindow* app, const QStringList &flist, int fileVersion)
{
	if (flist.size() < 2)
		return nullptr;

	QStringList::const_iterator line = flist.begin();

	QStringList list=(*line++).split("\t");
	if (list.size() < 4)
		return nullptr;
	QString caption=list[0];

	QList<int> targets;
	for (int i = 1; i <= (*line).count('\t'); i++)
		targets << (*line).section('\t',i,i).toInt();

	TableStatistics* w = app->newTableStatistics(0, list[2] == "row" ? TableStatistics::row : TableStatistics::column, targets, 0, -1, caption);
	if (!w)
		return nullptr;
	w->setBaseName(list[1]);

	app->setListViewDate(caption, list[3]);
	w->setBirthDate(list[3]);

	for (line++; line!=flist.end(); line++){
		QStringList fields = (*line).split("\t");
		if (fields[0] == "ColStatType"){
			QList <int> colStatTypes;
			for (int i = 1; i < fields.size(); i++)
				colStatTypes << fields[i].toInt();
			w->setColumnStatsTypes(colStatTypes);
		} else if (fields[0] == "Range"){
			w->setRange(fields[1].toInt(), fields[2].toInt());
		} else if (fields[0] == "geometry")
			app->restoreWindowGeometry(w, *line);
		else if (fields[0] == "header"){
			fields.pop_front();

			if (w->numCols() != fields.size())
				w->setNumCols(fields.size());

			if (fileVersion >= 78)
				w->loadHeader(fields);
			else {
				w->setColPlotDesignation(list[4].toInt(), Table::X);
				w->setColPlotDesignation(list[6].toInt(), Table::Y);
				w->setHeader(fields);
			}
		} else if (fields[0] == "ColWidth") {
			fields.pop_front();
			w->setColWidths(fields);
		} else if (fields[0] == "com") { // legacy code
			w->setCommands(*line);
		} else if (fields[0] == "<com>") {
			for (line++; line!=flist.end() && *line != "</com>"; line++)
			{
				int col = (*line).mid(9,(*line).length()-11).toInt();
				QString formula;
				for (line++; line!=flist.end() && *line != "</col>"; line++)
					formula += *line + "\n";
				formula.truncate(formula.length()-1);
				w->setCommand(col,formula);
			}
		} else if (fields[0] == "ColType") { // fileVersion > 65
			fields.pop_front();
			w->setColumnTypes(fields);
		} else if (fields[0] == "Comments") { // fileVersion > 71
			fields.pop_front();
			w->setColComments(fields);
		} else if (fields[0] == "WindowLabel" && fields.size() >= 3) { // fileVersion > 71
			w->setWindowLabel(fields[1]);
			w->setCaptionPolicy((MdiSubWindow::CaptionPolicy)fields[2].toInt());
		}
	}
	return w;
}


Graph* ProjectSerializer::openGraph(ApplicationWindow* app, MultiLayer *plot, const QStringList &list, int fileVersion)
{
	Graph* ag = 0;
	int curveID = 0;
	QList<int> mcIndexes;
	QList<ErrorBarsCurve *> errBars;
	for (int j = 0; j < list.count() - 1; j++){
		QString s = list[j];
		if (s.contains ("ggeometry")){
			QStringList fList = s.split("\t");
			ag = (Graph*)plot->addLayer(fList[1].toInt(), fList[2].toInt(), fList[3].toInt(), fList[4].toInt());
			ag->blockSignals(true);
			if (app)
				ag->setAxisTitlePolicy(app->d_graph_axis_labeling);
		}
		else if (s.startsWith ("<PageGeometry>") && s.endsWith ("</PageGeometry>"))
		{
			QStringList lst = s.remove("<PageGeometry>").remove("</PageGeometry>").split("\t");
			ag->setPageGeometry(QRectF(lst[0].toDouble(), lst[1].toDouble(), lst[2].toDouble(), lst[3].toDouble()));
		}
		else if (s.left(10) == "Background"){
			QStringList fList = s.split("\t");
			QColor c = QColor(fList[1]);
			if (fList.count() == 3)
				c.setAlpha(fList[2].toInt());
			ag->setBackgroundColor(c);
		}
		else if (s.contains ("Margin")){
			QStringList fList=s.split("\t");
			int m = fList[1].toInt();
			ag->setContentsMargins(m, m, m, m);
		}
		else if (s.contains ("Border")){
			QStringList fList=s.split("\t");
			ag->setFrame(fList[1].toInt(), QColor(fList[2]));
		}
		else if (s.contains ("EnabledAxes")){
			QStringList fList=s.split("\t");
			fList.pop_front();
			for (int i=0; i<(int)fList.count(); i++)
				ag->enableAxis(i, fList[i].toInt());
		}
		else if (s.contains ("AxesBaseline")){
			QStringList fList = s.split("\t", Qt::SkipEmptyParts);
			fList.pop_front();
			for (int i=0; i<(int)fList.count(); i++)
				ag->setAxisMargin(i, fList[i].toInt());
		}
		else if (s.contains ("EnabledTicks"))
		{//version < 0.8.6
			QStringList fList=s.split("\t");
			fList.pop_front();
			fList.replaceInStrings("-1", "3");
			ag->setMajorTicksType(fList);
			ag->setMinorTicksType(fList);
		}
		else if (s.contains ("MajorTicks"))
		{//version >= 0.8.6
			QStringList fList=s.split("\t");
			fList.pop_front();
			ag->setMajorTicksType(fList);
		}
		else if (s.contains ("MinorTicks"))
		{//version >= 0.8.6
			QStringList fList=s.split("\t");
			fList.pop_front();
			ag->setMinorTicksType(fList);
		}
		else if (s.contains ("TicksLength")){
			QStringList fList=s.split("\t");
			ag->setTicksLength(fList[1].toInt(), fList[2].toInt());
		}
		else if (s.contains ("EnabledTickLabels")){
			QStringList fList=s.split("\t");
			fList.pop_front();
			for (int i=0; i<int(fList.count()); i++)
				ag->enableAxisLabels(i, fList[i].toInt());
		}
		else if (s.contains ("AxesColors")){
			QStringList fList = s.split("\t");
			fList.pop_front();
			for (int i=0; i<int(fList.count()); i++)
				ag->setAxisColor(i, QColor(fList[i]));
		}
		else if (s.contains ("AxesNumberColors")){
			QStringList fList = s.split("\t");
			fList.pop_front();
			for (int i=0; i<int(fList.count()); i++)
				ag->setAxisLabelsColor(i, QColor(fList[i]));
		}
		else if (s.left(5)=="grid\t"){
			ag->grid()->load(s.split("\t"));
		}
		else if (s.startsWith ("<Antialiasing>") && s.endsWith ("</Antialiasing>")){
			bool antialiasing = s.remove("<Antialiasing>").remove("</Antialiasing>").toInt();
			ag->setAntialiasing(antialiasing);
		}
		else if (s.startsWith ("<Autoscaling>") && s.endsWith ("</Autoscaling>"))
			ag->enableAutoscaling(s.remove("<Autoscaling>").remove("</Autoscaling>").toInt());
		else if (s.startsWith ("<SyncScales>") && s.endsWith ("</SyncScales>"))
			ag->setSynchronizedScaleDivisions(s.remove("<SyncScales>").remove("</SyncScales>").toInt());
		else if (s.startsWith ("<ScaleFonts>") && s.endsWith ("</ScaleFonts>"))
			ag->setAutoscaleFonts(s.remove("<ScaleFonts>").remove("</ScaleFonts>").toInt());
		else if (s.startsWith ("<GridOnTop>") && s.endsWith ("</GridOnTop>"))
			ag->setGridOnTop(s.remove("<GridOnTop>").remove("</GridOnTop>").toInt(), false);
		else if (s.startsWith ("<MissingDataGap>") && s.endsWith ("</MissingDataGap>"))
			ag->showMissingDataGap(s.remove("<MissingDataGap>").remove("</MissingDataGap>").toInt(), false);
		else if (s.startsWith ("<ClipData>") && s.endsWith ("</ClipData>"))
			ag->setClipDataToFrame(s.remove("<ClipData>").remove("</ClipData>").toInt());
		else if (s.contains ("PieCurve")){
			QStringList curve=s.split("\t");
			if (!app->renamedTables.isEmpty()){
				QString caption = (curve[1]).left((curve[1]).lastIndexOf("_"));
				if (app->renamedTables.contains(caption))
				{//modify the name of the curve according to the new table name
					int index = app->renamedTables.indexOf(caption);
					QString newCaption = app->renamedTables[++index];
					curve.replaceInStrings(caption+"_", newCaption+"_");
				}
			}
			QPen pen = QPen(QColor(curve[3]), curve[2].toDouble(),Graph::getPenStyle(curve[4]));

			Table *table = app->table(curve[1]);
			if (table){
				int startRow = 0;
				int endRow = table->numRows() - 1;
				int first_color = curve[7].toInt();
				bool visible = true;
				if (fileVersion >= 90){
					startRow = curve[8].toInt();
					endRow = curve[9].toInt();
					visible = curve[10].toInt();
				}

				if (fileVersion <= 89)
					first_color = app->convertOldToNewColorIndex(first_color);

				if (curve.size() >= 22){//version 0.9.3-rc3
					ag->plotPie(table, curve[1], pen, curve[5].toInt(),
						curve[6].toInt(), first_color, startRow, endRow, visible,
						curve[11].toDouble(), curve[12].toDouble(), curve[13].toDouble(),
						curve[14].toDouble(), curve[15].toDouble(), curve[16].toInt(),
						curve[17].toInt(), curve[18].toInt(), curve[19].toInt(),
						curve[20].toInt(), curve[21].toInt());
				} else
					ag->plotPie(table, curve[1], pen, curve[5].toInt(),
						curve[6].toInt(), first_color, startRow, endRow, visible);
			}
		} else if (s.left(6) == "curve\t"){
			QStringList curve = s.split("\t", Qt::SkipEmptyParts);
			if (!app->renamedTables.isEmpty()){
				QString caption = (curve[2]).left((curve[2]).lastIndexOf("_"));
				if (app->renamedTables.contains(caption))
				{//modify the name of the curve according to the new table name
					int index = app->renamedTables.indexOf(caption);
					QString newCaption = app->renamedTables[++index];
					curve.replaceInStrings(caption+"_", newCaption+"_");
				}
			}

			if (curve.size() < 5)
				continue;

			CurveLayout cl;
			cl.connectType=curve[4].toInt();
			cl.lCol = app->readColorFromProject(curve[5]);
			cl.lStyle = curve[6].toInt();
			cl.lWidth = curve[7].toDouble();
			cl.sSize = curve[8].toInt();
			if (fileVersion <= 78)
				cl.sType=Graph::obsoleteSymbolStyle(curve[9].toInt());
			else
				cl.sType=curve[9].toInt();

			cl.symCol = app->readColorFromProject(curve[10]);
			cl.fillCol = app->readColorFromProject(curve[11]);
			cl.filledArea = curve[12].toDouble();
			cl.aCol = app->readColorFromProject(curve[13]);
			cl.aStyle = curve[14].toInt();
			if(curve.count() < 16)
				cl.penWidth = cl.lWidth;
			else if ((fileVersion >= 79) && (curve[3].toInt() == Graph::Box))
				cl.penWidth = curve[15].toDouble();
			else if ((fileVersion >= 78) && (curve[3].toInt() <= Graph::LineSymbols))
				cl.penWidth = curve[15].toDouble();
			else
				cl.penWidth = cl.lWidth;

			int plotType = curve[3].toInt();
			int size = curve.count();
			Table *w = app->table(curve[2]);
			Table *xt = app->table(curve[1]);
			PlotCurve *c = nullptr;
			if (xt && w && xt != w){
				c = (PlotCurve *)ag->insertCurve(xt, curve[1], w, curve[2], plotType, curve[size - 3].toInt(), curve[size - 2].toInt());
				ag->updateCurveLayout(c, &cl);
				if (c && c->rtti() == QwtPlotItem::Rtti_PlotCurve){
					c->setAxes(curve[size - 5].toInt(), curve[size - 4].toInt());
					c->setVisible(curve.last().toInt());
				}
			} else if (w){
				if (plotType == Graph::VectXYXY || plotType == Graph::VectXYAM){
					QStringList colsList;
					colsList<<curve[2]; colsList<<curve[20]; colsList<<curve[21];
					if (fileVersion < 72)
						colsList.prepend(w->colName(curve[1].toInt()));
					else
						colsList.prepend(curve[1]);

					int startRow = 0;
					int endRow = -1;
					if (fileVersion >= 90){
						startRow = curve[size - 3].toInt();
						endRow = curve[size - 2].toInt();
					}

					c = (PlotCurve *)ag->plotVectors(w, colsList, plotType, startRow, endRow);

					if (fileVersion <= 77){
						int temp_index = app->convertOldToNewColorIndex(curve[15].toInt());
						ag->updateVectorsLayout(curveID, ColorBox::defaultColor(temp_index), curve[16].toDouble(), curve[17].toInt(),
								curve[18].toInt(), curve[19].toInt(), 0, curve[20], curve[21]);
					} else {
						if(plotType == Graph::VectXYXY)
							ag->updateVectorsLayout(curveID, curve[15], curve[16].toDouble(),
								curve[17].toInt(), curve[18].toInt(), curve[19].toInt(), 0);
						else
							ag->updateVectorsLayout(curveID, curve[15], curve[16].toDouble(), curve[17].toInt(),
									curve[18].toInt(), curve[19].toInt(), curve[22].toInt());
					}
				} else if (plotType == Graph::Box)
					c = (PlotCurve *)ag->openBoxDiagram(w, curve, fileVersion);
				else {
					if (fileVersion < 72)
						c = (PlotCurve *)ag->insertCurve(w, curve[1].toInt(), curve[2], plotType);
					else if (fileVersion < 90)
						c = (PlotCurve *)ag->insertCurve(w, curve[1], curve[2], plotType);
					else
						c = (PlotCurve *)ag->insertCurve(w, curve[1], curve[2], plotType, curve[size - 3].toInt(), curve[size - 2].toInt());
				}

				if (plotType == Graph::Histogram){
					QwtHistogram *h = (QwtHistogram *)ag->curve(curveID);
					if (fileVersion <= 76)
						h->setBinning(curve[16].toInt(),curve[17].toDouble(),curve[18].toDouble(),curve[19].toDouble());
					else
						h->setBinning(curve[17].toInt(),curve[18].toDouble(),curve[19].toDouble(),curve[20].toDouble());
					h->loadData();
				}

				if (plotType == Graph::VerticalBars || plotType == Graph::HorizontalBars || plotType == Graph::Histogram){
					if (fileVersion <= 76)
						ag->setBarsGap(curveID, curve[15].toInt(), 0);
					else
						ag->setBarsGap(curveID, curve[15].toInt(), curve[16].toInt());
				}
				ag->updateCurveLayout(c, &cl);
				if (fileVersion >= 88){
					if (c && c->rtti() == QwtPlotItem::Rtti_PlotCurve){
						if (fileVersion < 90)
							c->setAxes(curve[size - 2].toInt(), curve[size - 1].toInt());
						else {
							c->setAxes(curve[size - 5].toInt(), curve[size - 4].toInt());
							c->setVisible(curve.last().toInt());
						}
					}
				}
			} else if(plotType == Graph::Histogram){//histograms from matrices
                Matrix *m = app->matrix(curve[2]);
                QwtHistogram *h = ag->restoreHistogram(m, curve);
                ag->updateCurveLayout(h, &cl);
			}
			curveID++;
		} else if (s == "<CurveLabels>"){
			QStringList lst;
			while ( s!="</CurveLabels>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			ag->restoreCurveLabels(curveID - 1, lst);
		} else if (s.contains("<SkipPoints>")){
			PlotCurve *c = (PlotCurve *)ag->curve(curveID - 1);
			if (c)
				c->setSkipSymbolsCount(s.remove("<SkipPoints>").remove("</SkipPoints>").toInt());
		} else if (s.contains("<StackWhiteOut>")){
			QwtBarCurve *b = (QwtBarCurve *)ag->curve(curveID - 1);
			if (b)
				b->setStacked();
		} else if (s == "<Function>"){//version 0.9.5
			curveID++;
			QStringList lst;
			while ( s != "</Function>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			FunctionCurve::restore(ag, lst);
		} else if (s.contains ("FunctionCurve")){
			QStringList curve = s.split("\t");
			CurveLayout cl;
			cl.connectType = curve[6].toInt();
			cl.lCol = app->readColorFromProject(curve[7]);
			cl.lStyle = curve[8].toInt();
			cl.lWidth=curve[9].toDouble();
			cl.sSize=curve[10].toInt();
			cl.sType=curve[11].toInt();
			cl.symCol = app->readColorFromProject(curve[12]);
			cl.fillCol = app->readColorFromProject(curve[13]);
			cl.filledArea = curve[14].toDouble();
			cl.aCol = app->readColorFromProject(curve[15]);
			cl.aStyle=curve[16].toInt();
			int current_index = 17;
			if(curve.count() < 16)
				cl.penWidth = cl.lWidth;
			else if ((fileVersion >= 79) && (curve[5].toInt() == Graph::Box))
				{
					cl.penWidth = curve[17].toDouble();
					current_index++;
				}
			else if ((fileVersion >= 78) && (curve[5].toInt() <= Graph::LineSymbols))
				{
					cl.penWidth = curve[17].toDouble();
					current_index++;
				}
			else
				cl.penWidth = cl.lWidth;

			PlotCurve *c = (PlotCurve *)ag->insertFunctionCurve(curve[1], curve[2].toInt(), fileVersion);
			c->setPlotStyle(curve[5].toInt());
			ag->updateCurveLayout(c, &cl);
			if (fileVersion >= 88){
				QwtPlotCurve *c = ag->curve(curveID);
				if (c){
                    if(current_index + 1 < curve.size())
                        c->setAxes(curve[current_index].toInt(), curve[current_index+1].toInt());
					if (fileVersion >= 90 && current_index+2 < curve.size())
						c->setVisible(curve.last().toInt());
                    else
                        c->setVisible(true);
				}

			}
			curveID++;
		} else if (s.contains ("ErrorBars")){
			QStringList curve = s.split("\t");
			if (!app->renamedTables.isEmpty()){
				QString caption = (curve[4]).left((curve[4]).lastIndexOf("_"));
				if (app->renamedTables.contains(caption))
				{//modify the name of the curve according to the new table name
					int index = app->renamedTables.indexOf(caption);
					QString newCaption = app->renamedTables[++index];
					curve.replaceInStrings(caption+"_", newCaption+"_");
				}
			}
			Table *w = app->table(curve[3]);
			Table *errTable = app->table(curve[4]);
			if (w && errTable){
				bool useMasterIndex = (curve.size() >= 12);
				DataCurve *mc = useMasterIndex ? ag->dataCurve(curve[11].toInt()) : ag->masterCurve(curve[2], curve[3]);
				ErrorBarsCurve *err = ag->addErrorBars(mc, errTable, curve[4], curve[1].toInt(),
					curve[5].toDouble(), curve[6].toDouble(), QColor(curve[7]),
					curve[8].toInt(), curve[10].toInt(), curve[9].toInt(), true);

				if (!mc && useMasterIndex){
					mcIndexes << curve[11].toInt();
					errBars << err;
				}
			}
			curveID++;
		}
		else if (s == "<spectrogram>"){
			curveID++;
			QStringList lst;
			while ( s!="</spectrogram>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			ag->restoreSpectrogram(app, lst);
		}
		else if (s.left(6) == "scale\t"){
			QStringList scl = s.split("\t");
			scl.pop_front();
			int size = scl.count();
			if (fileVersion < 88){
				double step = scl[2].toDouble();
				if (scl[5] == "0")
					step = 0.0;
				ag->setScale(QwtPlot::xBottom, scl[0].toDouble(), scl[1].toDouble(), step,
						scl[3].toInt(), scl[4].toInt(), scl[6].toInt(), bool(scl[7].toInt()));
				ag->setScale(QwtPlot::xTop, scl[0].toDouble(), scl[1].toDouble(), step,
						scl[3].toInt(), scl[4].toInt(), scl[6].toInt(), bool(scl[7].toInt()));

				step = scl[10].toDouble();
				if (scl[13] == "0")
					step = 0.0;
				ag->setScale(QwtPlot::yLeft, scl[8].toDouble(), scl[9].toDouble(), step, scl[11].toInt(),
						scl[12].toInt(), scl[14].toInt(), bool(scl[15].toInt()));
				ag->setScale(QwtPlot::yRight, scl[8].toDouble(), scl[9].toDouble(), step, scl[11].toInt(),
						scl[12].toInt(), scl[14].toInt(), bool(scl[15].toInt()));
			} else if (size == 8){
				ag->setScale(scl[0].toInt(), scl[1].toDouble(), scl[2].toDouble(), scl[3].toDouble(),
					scl[4].toInt(), scl[5].toInt(), scl[6].toInt(), bool(scl[7].toInt()));
			} else if (size == 18){
				ag->setScale(scl[0].toInt(), scl[1].toDouble(), scl[2].toDouble(), scl[3].toDouble(),
					scl[4].toInt(), scl[5].toInt(), scl[6].toInt(), bool(scl[7].toInt()), scl[8].toDouble(),
					scl[9].toDouble(), scl[10].toInt(), scl[11].toDouble(), scl[12].toDouble(), scl[13].toInt(),
					scl[14].toInt(), bool(scl[15].toInt()), scl[16].toInt(), bool(scl[17].toInt()));
			}
		}
		else if (s.contains ("PlotTitle")){
			QStringList fList=s.split("\t");
			ag->setTitle(fList[1]);
			ag->setTitleColor(QColor(fList[2]));
			ag->setTitleAlignment(fList[3].toInt());
		}
		else if (s.contains ("TitleFont")){
			QStringList fList=s.split("\t");
			QFont fnt=QFont (fList[1],fList[2].toInt(),fList[3].toInt(),fList[4].toInt());
			fnt.setUnderline(fList[5].toInt());
			fnt.setStrikeOut(fList[6].toInt());
			ag->setTitleFont(fnt);
		}
		else if (s.contains ("AxesTitles")){
			QStringList lst=s.split("\t");
			lst.pop_front();
			for (int i=0; i<4; i++){
			    if (lst.count() > i)
                    ag->setScaleTitle(i, lst[i]);
			}
		}
		else if (s.contains ("AxesTitleColors")){
			QStringList colors = s.split("\t", Qt::SkipEmptyParts);
			colors.pop_front();
			for (int i=0; i<int(colors.count()); i++)
				ag->setAxisTitleColor(i, colors[i]);
		}else if (s.contains ("AxesTitleAlignment")){
			QStringList align=s.split("\t", Qt::SkipEmptyParts);
			align.pop_front();
			for (int i=0; i<(int)align.count(); i++)
				ag->setAxisTitleAlignment(i, align[i].toInt());
		} else if (s.contains ("AxesTitleDistance")){
			QStringList align = s.split("\t", Qt::SkipEmptyParts);
			align.pop_front();
			for (int i = 0; i < align.count(); i++)
				ag->setAxisTitleDistance(i, align[i].toInt());
		} else if (s.contains ("<InvertedRightTitle>")){
			QwtScaleWidget *scale = ag->axisWidget(QwtPlot::yRight);
			if (scale)
				scale->setLayoutFlag(QwtScaleWidget::TitleInverted, true);
		} else if (s.contains ("InvertedTitle")){
			QStringList invertTitles = s.split("\t", Qt::SkipEmptyParts);
			invertTitles.pop_front();
			for (int i = 0; i < invertTitles.count(); i++){
				QwtScaleWidget *scale = ag->axisWidget(i);
				if (scale)
					scale->setLayoutFlag(QwtScaleWidget::TitleInverted, invertTitles[i].toInt());
			}
		} else if (s.contains ("ScaleFont")){
			QStringList fList = s.split("\t");
			QFont fnt = QFont (fList[1], fList[2].toInt(), fList[3].toInt(), fList[4].toInt());
			fnt.setUnderline(fList[5].toInt());
			fnt.setStrikeOut(fList[6].toInt());

			int axis=(fList[0].right(1)).toInt();
			ag->setAxisTitleFont(axis,fnt);
		}else if (s.contains ("AxisFont")){
			QStringList fList=s.split("\t");
			QFont fnt=QFont (fList[1],fList[2].toInt(),fList[3].toInt(),fList[4].toInt());
			fnt.setUnderline(fList[5].toInt());
			fnt.setStrikeOut(fList[6].toInt());

			int axis=(fList[0].right(1)).toInt();
			ag->setAxisFont(axis,fnt);
		}
		else if (s.contains ("AxesFormulas"))
		{
			QStringList fList=s.split("\t");
			fList.removeFirst();
			for (int i=0; i<(int)fList.count(); i++)
				ag->setAxisFormula(i, fList[i]);
		}
		else if (s.startsWith("<AxisFormula "))
		{
			int axis = s.mid(18,s.length()-20).toInt();
			QString formula;
			for (j++; j<(int)list.count() && list[j] != "</AxisFormula>"; j++)
				formula += list[j] + "\n";
			formula.truncate(formula.length()-1);
			ag->setAxisFormula(axis, formula);
		}
		else if (s.contains ("LabelsFormat"))
		{
			QStringList fList=s.split("\t");
			fList.pop_front();
			ag->setLabelsNumericFormat(fList);
		}
		else if (s.contains ("LabelsRotation"))
		{
			QStringList fList = s.split("\t");
			ag->setAxisLabelRotation(QwtPlot::xBottom, fList[1].toInt());
			ag->setAxisLabelRotation(QwtPlot::xTop, fList[2].toInt());
			if (fList.size() == 5){
				ag->setAxisLabelRotation(QwtPlot::yLeft, fList[3].toInt());
				ag->setAxisLabelRotation(QwtPlot::yRight, fList[4].toInt());
			}
		}
		else if (s.contains ("DrawAxesBackbone"))
		{
			ag->loadAxesOptions(s.split("\t"));
		}
		else if (s.contains ("AxesLineWidth"))
		{
			QStringList fList=s.split("\t");
			ag->loadAxesLinewidth(fList[1].toInt());
		}
		else if (s.contains ("TickLabelsSpace"))
		{
			QStringList lst = s.split("\t");
			lst.removeFirst();
			for (int i = 0; i < QwtPlot::axisCnt; i++){
				QwtScaleDraw *sd = ag->axisScaleDraw (i);
				if (sd && lst.count() > i)
					sd->setSpacing(lst[i].toInt());
			}
		}
		else if (s.contains ("ShowTicksPolicy"))
		{
			QStringList lst = s.split("\t");
			lst.removeFirst();
			for (int i = 0; i < QwtPlot::axisCnt; i++){
				ScaleDraw *sd = (ScaleDraw *)ag->axisScaleDraw (i);
				if (sd && lst.count() > i)
					sd->setShowTicksPolicy((ScaleDraw::ShowTicksPolicy)lst[i].toInt());
			}
		}
		else if (s.contains ("LabelsPrefix")){
			QStringList lst = s.split("\t");
			lst.removeFirst();
			for (int i = 0; i < QwtPlot::axisCnt && i < lst.size(); i++){
				QString prefix = lst[i];
				if (prefix.isEmpty())
					continue;
				ScaleDraw *sd = (ScaleDraw *)ag->axisScaleDraw(i);
				if (sd){
					sd->setPrefix(prefix);
					ag->axisWidget(i)->setScaleDraw(new ScaleDraw(ag, sd));
				}
			}
		} else if (s.contains ("LabelsSuffix")){
			QStringList lst = s.split("\t");
			lst.removeFirst();
			for (int i = 0; i < QwtPlot::axisCnt && i < lst.size(); i++){
				QString s = lst[i];
				if (s.isEmpty())
					continue;
				ScaleDraw *sd = (ScaleDraw *)ag->axisScaleDraw(i);
				if (sd){
					sd->setSuffix(s);
					ag->axisWidget(i)->setScaleDraw(new ScaleDraw(ag, sd));
				}
			}
		}
		else if (s.contains ("CanvasFrame")){
			QStringList lst = s.split("\t");
			ag->setCanvasFrame(lst[1].toInt(), QColor(lst[2]));
		}
		else if (s.contains ("CanvasBackground"))
		{
			QStringList list = s.split("\t");
			QColor c = QColor(list[1]);
			if (list.count() == 3)
				c.setAlpha(list[2].toInt());
			ag->setCanvasBackground(c);
		}
		else if (s.startsWith ("Legend"))
		{// version <= 0.8.9
			QStringList fList = s.split("\t");
			ag->insertLegend(fList, fileVersion);
		}
		else if (s.startsWith ("<legend>") && s.endsWith ("</legend>"))
		{
			QStringList fList = s.remove("</legend>").split("\t");
			ag->insertLegend(fList, fileVersion);
		}
		else if (s.contains ("textMarker"))
		{// version <= 0.8.9
			QStringList fList = s.split("\t");
			ag->insertText(fList, fileVersion);
		}
		else if (s.startsWith ("<text>") && s.endsWith ("</text>"))
		{
			QStringList fList = s.remove("</text>").split("\t");
			ag->insertText(fList, fileVersion);
		}
		else if (s.startsWith ("<PieLabel>") && s.endsWith ("</PieLabel>"))
		{
			QStringList fList = s.remove("</PieLabel>").split("\t");
			ag->insertText(fList, fileVersion);
		} else if (s == "<PieText>"){//version 0.9.7
			QStringList lst;
			while ( s != "</PieText>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			PieLabel::restore(ag, lst);
		}
		else if (s.contains ("lineMarker"))
		{// version <= 0.8.9
			QStringList fList=s.split("\t");
			ag->addArrow(fList, fileVersion);
		}
		else if (s.startsWith ("<line>") && s.endsWith ("</line>"))
		{
			QStringList fList=s.remove("</line>").split("\t");
			ag->addArrow(fList, fileVersion);
		}
		else if (s.contains ("ImageMarker") || (s.startsWith ("<image>") && s.endsWith ("</image>")))
		{
			QStringList fList=s.remove("</image>").split("\t");
			ag->insertImageMarker(fList, fileVersion);
		} else if (s == "<TexFormula>"){//version 0.9.7
			QStringList lst;
			while ( s != "</TexFormula>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			TexWidget::restore(ag, lst);
		} else if (s == "<Image>"){//version 0.9.7
			QStringList lst;
			while ( s != "</Image>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			ImageWidget::restore(ag, lst);
		} else if (s == "<Legend>"){//version 0.9.7
			QStringList lst;
			while ( s != "</Legend>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			LegendWidget::restore(ag, lst);
		} else if (s == "<Rectangle>"){//version 0.9.7
			QStringList lst;
			while ( s != "</Rectangle>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			RectangleWidget::restore(ag, lst);
		} else if (s == "<Ellipse>"){//version 0.9.7.2
			QStringList lst;
			while ( s != "</Ellipse>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			EllipseWidget::restore(ag, lst);
		} else if (s == "<BackgroundImage>"){//version 0.9.8.4
			QStringList lst;
			while ( s != "</BackgroundImage>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			ag->restoreBackgroundImage(lst);
		} else if (s == "<SymbolImage>"){//version 0.9.8.4
			QStringList lst;
			while ( s != "</SymbolImage>" ){
				s = list[++j];
				lst << s;
			}
			lst.pop_back();
			ag->restoreSymbolImage(curveID - 1, lst);
		} else if (s.contains("AxisType")) {
			QStringList fList = s.split("\t");
			for (int i = 0; i < 4; i++){
				QStringList lst = fList[i+1].split(";", Qt::SkipEmptyParts);
				int format = lst[0].toInt();
				if (format == ScaleDraw::Numeric)
					continue;
				if (format == ScaleDraw::Day)
					ag->setLabelsDayFormat(i, lst[1].toInt());
				else if (format == ScaleDraw::Month)
					ag->setLabelsMonthFormat(i, lst[1].toInt());
				else if (format == ScaleDraw::Time || format == ScaleDraw::Date){
					bool obsoleteDateTime = lst.size() > 2;
					ag->setLabelsDateTimeFormat(i, format, obsoleteDateTime ? lst[2] : lst[1]);
					if (obsoleteDateTime)
						ag->recoverObsoleteDateTimeScale(i, format, lst[1], lst[2]);
				} else if (lst.size() > 1)
					ag->setLabelsTextFormat(i, format, lst[1], app->table(lst[1]));
			}
		}
		else if (fileVersion < 69 && s.contains ("AxesTickLabelsCol"))
		{
			QStringList fList = s.split("\t");
			for (int i = 0; i < 4; i++){
				QString colName = fList[i+1];
				Table *nw = app->table(colName);
				ag->setLabelsTextFormat(i, ag->axisType(i), colName, nw);
			}
		} else if (s.startsWith ("<SpeedMode>") && s.endsWith ("</SpeedMode>")){
			QStringList lst = s.remove("<SpeedMode>").remove("</SpeedMode>").split("\t");
			// Two fields upstream; a third (an obsolete decimation method index)
			// was written by intermediate builds and is ignored.
			if (lst.size() >= 2)
				ag->enableDouglasPeukerSpeedMode(lst[0].toDouble(), lst[1].toInt());
		} else if (s.startsWith ("<ImageProfileTool>") && s.endsWith ("</ImageProfileTool>")){
			QStringList lst = s.remove("<ImageProfileTool>").remove("</ImageProfileTool>").split("\t");
			if (!lst.isEmpty()){
				Table *hort = 0, *vert = 0;
				if (lst.size() >= 2)
					hort = app->table(lst[1]);
				if (lst.size() >= 3)
					vert = app->table(lst[2]);
				ag->setActiveTool(new ImageProfilesTool(app, ag, app->matrix(lst[0]), hort, vert));
			}
		} else if (s.startsWith ("<ImageProfileValues>") && s.endsWith ("</ImageProfileValues>")){
			QStringList lst = s.remove("<ImageProfileValues>").remove("</ImageProfileValues>").split("\t");
			ImageProfilesTool *ipt = ag->imageProfilesTool();
			if (ipt){
				if (lst.size() == 3)
					ipt->setAveragePixels(lst[2].toInt());
				if (lst.size() >= 2)
					ipt->append(QPointF(lst[0].toDouble(), lst[1].toDouble()));
			}
		} else if (s.contains("<waterfall>")){
			QStringList lst = s.trimmed().remove("<waterfall>").remove("</waterfall>").split(",");
			if (lst.size() >= 2)
				ag->setWaterfallOffset(lst[0].toDouble(), lst[1].toDouble());
			if (lst.size() >= 3)
				ag->setWaterfallSideLines(lst[2].toInt());
			ag->updateDataCurves();
		}
	}
	if (ag){
		ag->loadErrorBars(errBars, mcIndexes);
		ag->disableCurveAntialiasing(app->d_disable_curve_antialiasing, app->d_curve_max_antialising_size);
		ag->updateAxesTitles();
		ag->updateLayout();
		ag->blockSignals(false);
	}
    return ag;
}


MdiSubWindow* ProjectSerializer::openTemplate(const QString& fn, ApplicationWindow *app, int &fileVersion)
{
	if (fn.isEmpty() || !QFile::exists(fn)){
		QMessageBox::critical(app, app->tr("QtiPlot - File opening error"),
					app->tr("The file: <b>%1</b> doesn't exist!").arg(fn));
		return 0;
	}

	QFile f(fn);
	QTextStream t(&f);
	t.setEncoding(QStringConverter::Utf8);
	if (!f.open(QIODevice::ReadOnly))
		return nullptr;
	QStringList l=t.readLine().split(QRegularExpression("\\s"), Qt::SkipEmptyParts);
	QString fileType=l[0];
	if (fileType != "QtiPlot"){
		QMessageBox::critical(app, app->tr("QtiPlot - File opening error"),
						app->tr("The file: <b> %1 </b> was not created using QtiPlot!").arg(fn));
		return 0;
	}

	QStringList vl = l[1].split(".", Qt::SkipEmptyParts);
	fileVersion = 100*(vl[0]).toInt()+10*(vl[1]).toInt()+(vl[2]).toInt();
	if (app)
		app->d_file_version = fileVersion;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	MdiSubWindow *w = 0;
	QString templateType;
	t>>templateType;

	if (templateType == "<SurfacePlot>") {
		t.skipWhiteSpace();
		QStringList lst;
		while (!t.atEnd())
			lst << t.readLine();
		w = Graph3D::restore(app, lst, fileVersion);
		if (w)
			((Graph3D *)w)->clearData();
	} else {
		int rows, cols;
		t>>rows; t>>cols;
		t.skipWhiteSpace();
		QString geometry = t.readLine();

		if (templateType == "<multiLayer>"){
			w = app->multilayerPlot(app->generateUniqueName(app->tr("Graph")), 0, rows, cols);
			if (w){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				app->restoreWindowGeometry(w, geometry);
				if (fileVersion > 83){
					QStringList lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					ml->setMargins(lst[1].toInt(),lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					ml->setSpacing(lst[1].toInt(),lst[2].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					ml->setLayerCanvasSize(lst[1].toInt(),lst[2].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					ml->setAlignement(lst[1].toInt(),lst[2].toInt());
				}
				while (!t.atEnd()){//open layers
					QString s = t.readLine();
					if (s.contains("<waterfall>")){
						QStringList lst = s.trimmed().remove("<waterfall>").remove("</waterfall>").split(",");
						Graph *ag = ml->activeLayer();
						if (ag && lst.size() >= 2){
							ag->setWaterfallOffset(lst[0].toDouble(), lst[1].toDouble());
							if (lst.size() >= 3)
								ag->setWaterfallSideLines(lst[2].toInt());
						}
						ml->setWaterfallLayout();
					}
					if (s.left(7) == "<graph>"){
						QStringList lst;
						while ( s != "</graph>" ){
							s = t.readLine();
							lst << s;
						}
					openGraph(app, ml, lst, fileVersion);
					}
					if (s.contains("<LinkXAxes>"))
						ml->linkXLayerAxes(s.trimmed().remove("<LinkXAxes>").remove("</LinkXAxes>").toInt());
					else if (s.contains("<AlignPolicy>"))
						ml->setAlignPolicy((MultiLayer::AlignPolicy)s.trimmed().remove("<AlignPolicy>").remove("</AlignPolicy>").toInt());
					else if (s.contains("<CommonAxes>"))
						ml->setCommonAxesLayout(s.trimmed().remove("<CommonAxes>").remove("</CommonAxes>").toInt());
					else if (s.contains("<ScaleLayers>"))
						ml->setScaleLayersOnResize(s.trimmed().remove("<ScaleLayers>").remove("</ScaleLayers>").toInt());
				}
			}
		} else {
			if (templateType == "<table>")
				w = app->newTable(app->tr("Table1"), rows, cols);
			else if (templateType == "<matrix>")
				w = app->newMatrix(rows, cols);
			if (w){
				QStringList lst;
				while (!t.atEnd())
					lst << t.readLine();
				w->restore(lst, fileVersion, true);
				app->restoreWindowGeometry(w, geometry);
			}
		}
	}

	f.close();
	if (w){
		w->show();
		app->customMenu(w);
		app->customToolBars(w);
	}

	QApplication::restoreOverrideCursor();
	return w;
}


ApplicationWindow* ProjectSerializer::openProject(const QString& fn, ApplicationWindow *app, bool factorySettings, bool newProject, int &fileVersion)
{
	if (newProject)
		app = new ApplicationWindow(factorySettings);

	app->projectname = fn;
	app->d_file_version = fileVersion;
	app->setWindowTitle(QObject::tr("QtiPlot") + " - " + fn);
	app->d_opening_file = true;
	app->d_workspace->blockSignals(true);
	bool scale3DFonts = app->scale3DPlotFonts();
	app->setScale3DPlotFonts(false);

	QFile f(fn);
	QTextStream t( &f );
	t.setEncoding(QStringConverter::Utf8);
	if (!f.open(QIODevice::ReadOnly))
		return nullptr;

	QFileInfo fi(fn);
	QString baseName = fi.fileName();

	t.readLine();
	if (fileVersion < 73)
		t.readLine();

	QString s = t.readLine();
	QStringList list=s.split("\t", Qt::SkipEmptyParts);
	if (list[0] == "<scripting-lang>"){
		if (!app->setScriptingLanguage(list[1]))
			QMessageBox::warning(app, QObject::tr("QtiPlot - File opening error"),
					QObject::tr("The file \"%1\" was created using \"%2\" as scripting language.\n\n"\
						"Initializing support for this language FAILED; I'm using \"%3\" instead.\n"\
						"Various parts of this file may not be displayed as expected.")\
					.arg(fn).arg(list[1]).arg(app->scriptEnv->objectName()));

		s = t.readLine();
		list=s.split("\t", Qt::SkipEmptyParts);
	}
	int aux=0,widgets=list[1].toInt();

	QString titleBase = QObject::tr("Window") + ": ";
	QString title = titleBase + "1/" + QString::number(widgets) + "  ";

	QProgressDialog progress(app);
	progress.setWindowModality(Qt::WindowModal);
	progress.setRange(0, widgets);
	progress.setMinimumWidth(app->width()/2);
	progress.setWindowTitle(QObject::tr("QtiPlot - Opening file") + ": " + baseName);
	progress.setLabelText(title);

	Folder *cf = app->projectFolder();
	app->folders->blockSignals (true);
	app->blockSignals (true);

	//rename project folder item
	FolderListItem *item = (FolderListItem *)app->folders->topLevelItem(0);
	item->setText(0, fi.baseName());
	item->folder()->setObjectName(fi.baseName());

	//process tables and matrix information
	while (!t.atEnd() && !progress.wasCanceled()){
		s = t.readLine();
		list.clear();
		if  (s.left(8) == "<folder>"){
			list = s.split("\t");
			Folder *f = new Folder(app->current_folder, list[1]);
			f->setBirthDate(list[2]);
			f->setModificationDate(list[3]);
			if(list.count() > 4)
				if (list[4] == "current")
					cf = f;

			FolderListItem *fli = new FolderListItem(app->current_folder->folderListItem(), f);
			f->setFolderListItem(fli);

			app->current_folder = f;
		} else if  (s.contains("<open>")) {
			app->current_folder->folderListItem()->setOpen(s.remove("<open>").remove("</open>").toInt());
		} else if  (s == "<table>") {
			title = titleBase + QString::number(++aux)+"/"+QString::number(widgets);
			progress.setLabelText(title);
			QStringList lst;
			while (!t.atEnd() && s!="</table>"){
				s = t.readLine();
				lst<<s;
			}
			lst.pop_back();
			openTable(app, lst, fileVersion);
			progress.setValue(aux);
		} else if (s.left(17)=="<TableStatistics>") {
			QStringList lst;
			while ( s!="</TableStatistics>" ){
				s=t.readLine();
				lst<<s;
			}
			lst.pop_back();
			openTableStatistics(app, lst, fileVersion);
		} else if  (s == "<matrix>") {
			title= titleBase + QString::number(++aux)+"/"+QString::number(widgets);
			progress.setLabelText(title);
			QStringList lst;
			while ( s != "</matrix>" ) {
				s=t.readLine();
				lst<<s;
			}
			lst.pop_back();
			openMatrix(app, lst, fileVersion);
			progress.setValue(aux);
		} else if  (s == "<note>") {
			title= titleBase + QString::number(++aux)+"/"+QString::number(widgets);
			progress.setLabelText(title);
			for (int i=0; i<3; i++){
				s = t.readLine();
				list << s;
			}
			Note* m = openNote(app, list, fileVersion);
			QStringList cont;
			while ( s != "</note>" ){
				s = t.readLine();
				cont << s;
			}
			cont.pop_back();
			m->restore(cont, fileVersion);
			progress.setValue(aux);
		} else if  (s == "</folder>")
			app->goToParentFolder();
	}
	f.close();

	if (progress.wasCanceled()){
		app->saved = true;
		app->close();
		return 0;
	}

	//process the rest
	if (!f.open(QIODevice::ReadOnly))
		return nullptr;

	MultiLayer *plot=0;
	while (!t.atEnd() && !progress.wasCanceled()){
		s = t.readLine();
		if  (s.left(8) == "<folder>"){
			list = s.split("\t");
			if (app->current_folder && list.size() >= 2)
				app->current_folder = app->current_folder->findSubfolder(list[1]);
		} else if  (s == "<multiLayer>"){//process multilayers information
			title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
			progress.setLabelText(title);

			s = t.readLine();
			QStringList graph = s.split("\t");
			QString caption = graph[0];

			plot = app->multilayerPlot(caption, 0,  graph[2].toInt(), graph[1].toInt());
			app->setListViewDate(caption, graph[3]);
			plot->setBirthDate(graph[3]);

			app->restoreWindowGeometry(plot, t.readLine());
			plot->blockSignals(true);

			if (fileVersion > 71){
				QStringList lst = t.readLine().split("\t");
				if (lst.size() > 1)
					plot->setWindowLabel(lst[1]);
				if (lst.size() > 2)
					plot->setCaptionPolicy((MdiSubWindow::CaptionPolicy)lst[2].toInt());
			}
			if (fileVersion > 83){
				QStringList lst=t.readLine().split("\t", Qt::SkipEmptyParts);
				if (lst.size() >= 5)
					plot->setMargins(lst[1].toInt(),lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
				lst=t.readLine().split("\t", Qt::SkipEmptyParts);
				if (lst.size() >= 3)
					plot->setSpacing(lst[1].toInt(),lst[2].toInt());
				lst=t.readLine().split("\t", Qt::SkipEmptyParts);
				if (lst.size() >= 3)
					plot->setLayerCanvasSize(lst[1].toInt(),lst[2].toInt());
				lst=t.readLine().split("\t", Qt::SkipEmptyParts);
				if (lst.size() >= 3)
					plot->setAlignement(lst[1].toInt(),lst[2].toInt());
			}

			while ( s != "</multiLayer>" ){//open layers
				s = t.readLine();
				if (s.contains("<waterfall>")){
					QStringList lst = s.trimmed().remove("<waterfall>").remove("</waterfall>").split(",");
					Graph *ag = plot->activeLayer();
					if (ag && lst.size() >= 2){
						ag->setWaterfallOffset(lst[0].toDouble(), lst[1].toDouble());
						if (lst.size() >= 3)
							ag->setWaterfallSideLines(lst[2].toInt());
					}
					plot->setWaterfallLayout();
				}

				if (s.left(7) == "<graph>"){
					list.clear();
					while ( s != "</graph>" ){
						s = t.readLine();
						list<<s;
					}
					openGraph(app, plot, list, fileVersion);
				}

				if (s.contains("<LinkXAxes>"))
					plot->linkXLayerAxes(s.trimmed().remove("<LinkXAxes>").remove("</LinkXAxes>").toInt());
				else if (s.contains("<AlignPolicy>"))
					plot->setAlignPolicy((MultiLayer::AlignPolicy)s.trimmed().remove("<AlignPolicy>").remove("</AlignPolicy>").toInt());
				else if (s.contains("<CommonAxes>"))
					plot->setCommonAxesLayout(s.trimmed().remove("<CommonAxes>").remove("</CommonAxes>").toInt());
				else if (s.contains("<ScaleLayers>"))
					plot->setScaleLayersOnResize(s.trimmed().remove("<ScaleLayers>").remove("</ScaleLayers>").toInt());
			}
			if (plot->status() == MdiSubWindow::Minimized)
				plot->showMinimized();
			plot->blockSignals(false);
			progress.setValue(aux);
		} else if  (s == "<SurfacePlot>") {//process 3D plots information
			list.clear();
			title = titleBase + QString::number(++aux)+"/"+QString::number(widgets);
			progress.setLabelText(title);
			while ( s != "</SurfacePlot>" ){
				s = t.readLine();
				list << s;
			}
			Graph3D::restore(app, list, fileVersion);
			progress.setValue(aux);
		} else if (s == "</folder>")
			app->goToParentFolder();
		else if (s == "<log>"){//process analysis information
			s = t.readLine();
			QString log = s + "\n";
			while(!s.contains("</log>")){
				s = t.readLine();
				log += s + "\n";
			}
			if (app->current_folder)
				app->current_folder->appendLogInfo(log.remove("</log>"));
		}
	}
	f.close();

	if (progress.wasCanceled()){
		app->saved = true;
		app->close();
		return 0;
	}

	QList<MdiSubWindow*> tables = app->tableList();
	for (MdiSubWindow* w : tables){
		TableStatistics *ts = qobject_cast<TableStatistics *>(w);
		if (ts)
			ts->setBase(app->table(ts->baseName()));
	}

	QFileInfo fi2(f);
	QString fileName = fi2.absoluteFilePath();

	app->updateRecentProjectsList(fileName);
	app->folders->setCurrentItem(cf->folderListItem());
	app->folders->blockSignals (false);
	app->changeFolder(cf, true);//change folder to user defined current folder
	app->blockSignals (false);
	app->renamedTables.clear();
	app->executeNotes();
	app->d_workspace->blockSignals(false);
	app->addWindowsListToCompleter();
	app->restoreApplicationGeometry();
	app->d_opening_file = false;
	app->savedProject();
	app->setScale3DPlotFonts(scale3DFonts);
	return app;
}


Folder* ProjectSerializer::appendProject(const QString& fn, Folder* parentFolder, ApplicationWindow *app, int &fileVersion)
{
	if (fn.isEmpty())
		return 0;

	QFileInfo fi(fn);
	app->workingDir = fi.absolutePath();

	if (fn.endsWith(".qti") || fn.endsWith(".opj", Qt::CaseInsensitive) || fn.endsWith(".ogm", Qt::CaseInsensitive) ||
		fn.endsWith(".ogw", Qt::CaseInsensitive) || fn.endsWith(".ogg", Qt::CaseInsensitive) ||
		fn.endsWith(".xls", Qt::CaseInsensitive) || fn.endsWith(".xlsx", Qt::CaseInsensitive) || fn.endsWith(".ods", Qt::CaseInsensitive)){
		QFileInfo f(fn);
		if (!f.exists ()){
			QMessageBox::critical(app, QObject::tr("QtiPlot - File opening error"), QObject::tr("The file: <b>%1</b> doesn't exist!").arg(fn));
			return 0;
		}
	} else {
		QMessageBox::critical(app,QObject::tr("QtiPlot - File opening error"), QObject::tr("The file: <b>%1</b> is not a QtiPlot or Origin project file!").arg(fn));
		return 0;
	}

	app->d_is_appending_file = true;

	if (fn != app->projectname)
		app->updateRecentProjectsList(fn);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString fname = fn;
	if (fn.contains(".qti.gz")){//decompress using zlib
		file_uncompress(fname.toLocal8Bit().data());
		fname.remove(".gz");
	}

	Folder *cf = app->current_folder;
	if (parentFolder)
		app->changeFolder(parentFolder, true);

	FolderListItem *item = (FolderListItem *)app->current_folder->folderListItem();
	app->folders->blockSignals (true);
	app->blockSignals (true);

	QString baseName = fi.baseName();
	QStringList lst = app->current_folder->subfolders();
	int n = lst.count(baseName);
	if (n){//avoid identical subfolder names
		while (lst.count(baseName + QString::number(n)))
			n++;
		baseName += QString::number(n);
	}

	Folder *new_folder;
	if (parentFolder)
		new_folder = new Folder(parentFolder, baseName);
	else
		new_folder = new Folder(app->current_folder, baseName);

	app->current_folder = new_folder;
	FolderListItem *fli = new FolderListItem(item, app->current_folder);
	app->current_folder->setFolderListItem(fli);

	if (fn.contains(".opj", Qt::CaseInsensitive) || fn.contains(".ogm", Qt::CaseInsensitive) ||
		fn.contains(".ogw", Qt::CaseInsensitive) || fn.contains(".ogg", Qt::CaseInsensitive))
		app->importOPJ(fn, false, false);
	else if (fn.endsWith(".xls", Qt::CaseInsensitive))
		app->importExcel(fn);
	else if (fn.endsWith(".ods", Qt::CaseInsensitive))
		app->importOdfSpreadsheet(fn);
	else {
		QFile f(fname);
		QTextStream t( &f );
		t.setEncoding(QStringConverter::Utf8);
		if (!f.open(QIODevice::ReadOnly))
			return nullptr;

		QString s = t.readLine();
		lst = s.split(QRegularExpression("\\s"), Qt::SkipEmptyParts);
		QString version = lst[1];
		lst = version.split(".", Qt::SkipEmptyParts);
		fileVersion = 100*(lst[0]).toInt()+10*(lst[1]).toInt()+(lst[2]).toInt();
		app->d_file_version = fileVersion;

		t.readLine();
		if (fileVersion < 73)
			t.readLine();

		//process tables and matrix information
		while ( !t.atEnd()){
			s = t.readLine();
			lst.clear();
			if  (s.left(8) == "<folder>"){
				lst = s.split("\t");
				Folder *f = new Folder(app->current_folder, lst[1]);
				f->setBirthDate(lst[2]);
				f->setModificationDate(lst[3]);
				if(lst.count() > 4)
					if (lst[4] == "current")
						cf = f;

				FolderListItem *fli = new FolderListItem(app->current_folder->folderListItem(), f);
				fli->setText(0, lst[1]);
				f->setFolderListItem(fli);

				app->current_folder = f;
			}else if  (s == "<table>"){
				while ( s!="</table>" ){
					s=t.readLine();
					lst<<s;
				}
				lst.pop_back();
				openTable(app, lst, fileVersion);
			}else if  (s == "<matrix>"){
				while ( s != "</matrix>" ){
					s=t.readLine();
					lst<<s;
				}
				lst.pop_back();
				openMatrix(app, lst, fileVersion);
			}else if  (s == "<note>"){
				for (int i=0; i<3; i++){
					s = t.readLine();
					lst << s;
				}
				Note* m = openNote(app, lst, fileVersion);
				QStringList cont;
				while ( s != "</note>" ){
					s = t.readLine();
					cont << s;
				}
				cont.pop_back();
				m->restore(cont, fileVersion);
			} else if  (s == "</folder>")
				app->goToParentFolder();
		}
		f.close();

		//process the rest
		if (!f.open(QIODevice::ReadOnly))
			return nullptr;

		MultiLayer *plot=0;
		while ( !t.atEnd()){
			s=t.readLine();
			if  (s.left(8) == "<folder>"){
				lst = s.split("\t");
                                if (app->current_folder && lst.size() >= 2)
                                    app->current_folder = app->current_folder->findSubfolder(lst[1]);
			}else if  (s == "<multiLayer>"){//process multilayers information
				s=t.readLine();
				QStringList graph=s.split("\t");
				QString caption=graph[0];
				plot = app->multilayerPlot(caption, 0, graph[2].toInt(), graph[1].toInt());
				app->setListViewDate(caption, graph[3]);
				plot->setBirthDate(graph[3]);
				plot->blockSignals(true);

				app->restoreWindowGeometry(plot, t.readLine());

				if (fileVersion > 71){
					QStringList lst = t.readLine().split("\t");
					if (lst.size() >= 3){
						plot->setWindowLabel(lst[1]);
						plot->setCaptionPolicy((MdiSubWindow::CaptionPolicy)lst[2].toInt());
					}
				}

				if (fileVersion > 83){
					QStringList lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					plot->setMargins(lst[1].toInt(),lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					plot->setSpacing(lst[1].toInt(),lst[2].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					plot->setLayerCanvasSize(lst[1].toInt(),lst[2].toInt());
					lst=t.readLine().split("\t", Qt::SkipEmptyParts);
					plot->setAlignement(lst[1].toInt(),lst[2].toInt());
				}

				while ( s != "</multiLayer>" ){//open layers
					s = t.readLine();
					if (s.contains("<waterfall>")){
						QStringList lst = s.trimmed().remove("<waterfall>").remove("</waterfall>").split(",");
						Graph *ag = plot->activeLayer();
						if (ag && lst.size() >= 2){
							ag->setWaterfallOffset(lst[0].toDouble(), lst[1].toDouble());
							if (lst.size() >= 3)
								ag->setWaterfallSideLines(lst[2].toInt());
						}
						plot->setWaterfallLayout();
					}
					if (s.left(7) == "<graph>"){
						lst.clear();
						while ( s != "</graph>" ){
							s = t.readLine();
							lst << s;
						}
						openGraph(app, plot, lst, fileVersion);
					}
					if (s.contains("<LinkXAxes>"))
						plot->linkXLayerAxes(s.trimmed().remove("<LinkXAxes>").remove("</LinkXAxes>").toInt());
					else if (s.contains("<AlignPolicy>"))
						plot->setAlignPolicy((MultiLayer::AlignPolicy)s.trimmed().remove("<AlignPolicy>").remove("</AlignPolicy>").toInt());
					else if (s.contains("<CommonAxes>"))
						plot->setCommonAxesLayout(s.trimmed().remove("<CommonAxes>").remove("</CommonAxes>").toInt());
					else if (s.contains("<ScaleLayers>"))
						plot->setScaleLayersOnResize(s.trimmed().remove("<ScaleLayers>").remove("</ScaleLayers>").toInt());
				}
				if (plot->status() == MdiSubWindow::Minimized)
					plot->showMinimized();
				plot->blockSignals(false);
			} else if (s == "<SurfacePlot>"){//process 3D plots information
				lst.clear();
				while ( s!="</SurfacePlot>" ){
					s = t.readLine();
					lst<<s;
				}
				Graph3D::restore(app, lst, fileVersion);
			} else if (s == "<PolarGraph>"){
				lst.clear();
				while ( s!="</PolarGraph>" ){
					s = t.readLine();
					lst<<s;
				}
				PolarGraph::restore(app, lst);
			} else if  (s == "</folder>")
				app->goToParentFolder();
		}
		f.close();
	}

	app->folders->blockSignals (false);
	//change folder to user defined current folder
	app->changeFolder(cf);
	app->blockSignals (false);
	app->renamedTables.clear();
	QApplication::restoreOverrideCursor();
	app->d_is_appending_file = false;
	return new_folder;
}


bool ProjectSerializer::saveFolder(Folder *folder, const QString& fn, bool compress, ApplicationWindow *app)
{
	if (!folder || !app)
		return false;

	QString tempFn = fn + ".tmp";
	QFile::remove(tempFn);

	QFile f(tempFn);
	if (!f.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"),
			QObject::tr("Cannot write to temporary file: <br><b>%1</b>").arg(tempFn));
		return false;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<MdiSubWindow *> lst = folder->windowsList();
	int windows = lst.count();
	int initial_depth = folder->depth();
	Folder *dir = folder->folderBelow();
	while (dir && dir->depth() > initial_depth){
		windows += dir->windowsList().count();
		dir = dir->folderBelow();
	}

	QTextStream t(&f);
	t.setEncoding(QStringConverter::Utf8);
	t << "QtiPlot " + QString::number(maj_version) + "." + QString::number(min_version) + "."+
			QString::number(patch_version) + " project file\n";
	t << "<scripting-lang>\t" + QString(app->scriptEnv->objectName()) + "\n";
	t << "<windows>\t" + QString::number(windows) + "\n";
	t.flush();
	if (t.status() != QTextStream::Ok || f.error() != QFile::NoError) {
		f.close();
		QFile::remove(tempFn);
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error writing project header to <b>%1</b>.").arg(fn));
		return false;
	}
	f.close();

	for (MdiSubWindow *w : lst)
		w->save(tempFn, app->windowGeometryInfo(w));

	initial_depth = folder->depth();
	dir = folder->folderBelow();
	while (dir && dir->depth() > initial_depth){
		if (!f.open(QIODevice::Append)) {
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error writing folder structure to <b>%1</b>.").arg(fn));
			return false;
		}

		t << "<folder>\t" + QString(dir->objectName()) + "\t" + dir->birthDate() + "\t" + dir->modificationDate();
		if (dir == app->current_folder)
			t << "\tcurrent\n";
		else
			t << "\n";
		t << "<open>" + QString::number(dir->folderListItem()->isOpen()) + "</open>\n";
		t.flush();
		if (t.status() != QTextStream::Ok || f.error() != QFile::NoError) {
			f.close();
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error writing folder data to <b>%1</b>.").arg(fn));
			return false;
		}
		f.close();

		lst = dir->windowsList();
		for (MdiSubWindow *w : lst)
			w->save(tempFn, app->windowGeometryInfo(w));

		if (!f.open(QIODevice::Append)) {
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error writing folder structure to <b>%1</b>.").arg(fn));
			return false;
		}

		if (!dir->logInfo().isEmpty() )
			t << "<log>\n" + dir->logInfo() + "</log>\n" ;

		if ( (dir->children()).isEmpty() )
			t << "</folder>\n";

		int depth = dir->depth();
		dir = dir->folderBelow();
		if (dir){
		    int next_dir_depth = dir->depth();
		    if (next_dir_depth < depth){
		        int diff = depth - next_dir_depth;
		        for (int i = 0; i < diff; i++)
                    t << "</folder>\n";
		    }
		} else {
		    int diff = depth - initial_depth - 1;
            for (int i = 0; i < diff; i++)
                t << "</folder>\n";
		}
	}

	if (!f.isOpen()) {
		if (!f.open(QIODevice::Append)) {
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error finalizing project file <b>%1</b>.").arg(fn));
			return false;
		}
	}

	t << "<open>" + QString::number(folder->folderListItem()->isOpen()) + "</open>\n";
	if (!folder->logInfo().isEmpty())
		t << "<log>\n" + folder->logInfo() + "</log>" ;

	t.flush();
	bool writeOk = (t.status() == QTextStream::Ok) && (f.error() == QFile::NoError);
	f.close();

	if (!writeOk) {
		QFile::remove(tempFn);
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Error writing data to <b>%1</b>. Operation aborted.").arg(fn));
		return false;
	}

	if (compress)
		file_compress(tempFn.toUtf8().data(), (char*)"wb9");

	// Backup existing target file if requested
	if (app->d_backup_files && QFile::exists(fn)) {
		QString bfn = fn + "~";
		QFile::remove(bfn);
		if (!QFile::copy(fn, bfn)) {
			QMessageBox::StandardButton choice = QMessageBox::warning(app, QObject::tr("QtiPlot - File backup error"),
					QObject::tr("Cannot make a backup copy of <b>%1</b> (to %2).<br>If you ignore this, you run the risk of <b>data loss</b>.").arg(fn).arg(bfn),
					QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
			if (choice == QMessageBox::Abort) {
				QFile::remove(tempFn);
				QApplication::restoreOverrideCursor();
				return false;
			}
		}
	}

	// Atomically replace target file
	if (QFile::exists(fn)) {
		if (!QFile::remove(fn)) {
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"), QObject::tr("Cannot overwrite existing file <b>%1</b>.").arg(fn));
			return false;
		}
	}

	if (!QFile::rename(tempFn, fn)) {
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(app, QObject::tr("QtiPlot - File save error"),
			QObject::tr("Cannot rename temporary file <b>%1</b> to <b>%2</b>.<br>Your data is preserved in the temporary file.").arg(tempFn).arg(fn));
		return false;
	}

	QApplication::restoreOverrideCursor();
	return true;
}

