/***************************************************************************
    File                 : importOPJ.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2007 by Ion Vasilief, Alex Kargovsky, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, kargovsky*yumr.phys.msu.su, thzs*gmx.net
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
#include "importOPJ.h"

#include <QRegExp>
#include <QMessageBox>
#include <QDockWidget>
#include <QLocale>
#include <QDate>
#include <QDir>
#include <QTemporaryFile>
#include "Matrix/Matrix.h"
#include "Matrix/MatrixModel.h"
#include "ColorBox.h"
#include "plot2D/MultiLayer.h"
#include "Note.h"
#include "Folder.h"
#include "plot2D/QwtHistogram.h"
#include "plot2D/QwtPieCurve.h"
#include "plot2D/VectorCurve.h"
#include "plot2D/LegendWidget.h"
#include "plot2D/Grid.h"
#include "plot2D/ArrowMarker.h"
#include "plot2D/ImageWidget.h"

#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"

#include <gsl/gsl_math.h>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

#define OBJECTXOFFSET 200

QString strreverse(const QString &str) //QString reversing
{
	QString out="";
	for(int i=str.length()-1; i>=0; --i)
	{
		out+=str[i];
	}
	return out;
}

QString posixTimeToString(ptime pt)
{
	return QDateTime::fromString(to_iso_string(pt).c_str(), "yyyyMMddThhmmss").toString("dd.MM.yyyy hh:mm:ss");
}

ImportOPJ::ImportOPJ(ApplicationWindow *app, const QString& filename) :
		mw(app)
{
	xoffset=0;
	OriginFile opj((const char *)filename.latin1());
	parse_error = opj.parse();
	importTables(opj);
	importGraphs(opj);
	importNotes(opj);
	if(filename.endsWith(".opj", Qt::CaseInsensitive))
		createProjectTree(opj);
	mw->showResults(opj.resultsLogString(), mw->logWindow->isVisible());
}

inline uint qHash(const tree<Origin::ProjectNode>::iterator &key)
{
	return qHash(key->name.c_str());
}

bool ImportOPJ::createProjectTree(const OriginFile& opj)
{
	const tree<Origin::ProjectNode>* projectTree = opj.project();
	tree<Origin::ProjectNode>::iterator root = projectTree->begin(projectTree->begin());
	if(!root.node)
		return false;
	FolderListItem* item = (FolderListItem*)mw->folders->firstChild();
	item->setText(0, root->name.c_str());
	item->folder()->setName(root->name.c_str());
	Folder* projectFolder = mw->projectFolder();
	QHash<tree<Origin::ProjectNode>::iterator, Folder*> parent;
	parent[root] = projectFolder;
	for(tree<Origin::ProjectNode>::iterator sib = projectTree->begin(root); sib != projectTree->end(root); ++sib)
	{
		if(sib->type == 1)
		{
			parent[sib] = mw->addFolder(sib->name.c_str(), parent.value(projectTree->parent(sib)));
			parent[sib]->setBirthDate(posixTimeToString(sib->creationDate));
			parent[sib]->setModificationDate(posixTimeToString(sib->modificationDate));
		}
		else
		{
			MdiSubWindow* w = projectFolder->window(sib->name.c_str());
			if(w)
			{
				parent.value(projectTree->parent(sib))->addWindow(w);
				projectFolder->removeWindow(w);
			}
		}
	}
	mw->changeFolder(projectFolder, true);
	return true;
}

int ImportOPJ::translateOrigin2QtiplotLineStyle(int linestyle) {
	int qtiplotstyle=0;
	switch (linestyle)
	{
		case Origin::GraphCurve::Solid:
			qtiplotstyle=0;
			break;
		case Origin::GraphCurve::Dash:
		case Origin::GraphCurve::ShortDash:
			qtiplotstyle=1;
			break;
		case Origin::GraphCurve::Dot:
		case Origin::GraphCurve::ShortDot:
			qtiplotstyle=2;
			break;
		case Origin::GraphCurve::DashDot:
		case Origin::GraphCurve::ShortDashDot:
			qtiplotstyle=3;
			break;
		case Origin::GraphCurve::DashDotDot:
			qtiplotstyle=4;
			break;
	}
	return qtiplotstyle;
}

bool ImportOPJ::importTables(const OriginFile& opj)
{
	int visible_count=0;
	int QtiPlot_scaling_factor=10; //in Origin width is measured in characters while in QtiPlot - pixels --- need to be accurate
	for (int s=0; s<opj.numSpreads(); s++)
	{
		int columnCount = opj.numCols(s);
		int maxrows = opj.maxRows(s);
		if(!columnCount) //remove tables without cols
			continue;

		Table *table = (opj.spreadHidden(s)||opj.spreadLoose(s))&&opj.Version()==7.5 ? mw->newHiddenTable(opj.spreadName(s), opj.spreadLabel(s), maxrows, columnCount)
										: mw->newTable(opj.spreadName(s), maxrows, columnCount);
		if (!table)
			return false;

		Origin::Rect windowRect;
		if(opj.Version() == 7.5)
		{
			windowRect = opj.spreadWindowRect(s);
			table->resize(windowRect.width() - (table->frameGeometry().width() - table->width()),
				windowRect.height() - (table->frameGeometry().height() - table->height()));
		}

		table->setCaptionPolicy((MdiSubWindow::CaptionPolicy)opj.spreadTitle(s));
		table->setBirthDate(posixTimeToString(opj.spreadCreationDate(s)));

        QLocale locale = mw->locale();
		table->setWindowLabel(opj.spreadLabel(s));
		for (int j=0; j<columnCount; j++)
		{
			QString name(opj.colName(s,j));
			table->setColName(j, name.replace(QRegExp(".*_"),""));
			table->setCommand(j, QString(opj.colCommand(s,j)));
			table->setColComment(j, QString(opj.colComment(s,j)));
			table->setColumnWidth(j, opj.colWidth(s,j)*QtiPlot_scaling_factor);

			switch(opj.colType(s,j))
			{
			case Origin::SpreadColumn::X:
				table->setColPlotDesignation(j, Table::X);
				break;
			case Origin::SpreadColumn::Y:
				table->setColPlotDesignation(j, Table::Y);
				break;
			case Origin::SpreadColumn::Z:
				table->setColPlotDesignation(j, Table::Z);
				break;
			case Origin::SpreadColumn::XErr:
				table->setColPlotDesignation(j, Table::xErr);
				break;
			case Origin::SpreadColumn::YErr:
				table->setColPlotDesignation(j, Table::yErr);
				break;
			case Origin::SpreadColumn::Label:
				table->setColPlotDesignation(j, Table::Label);
				break;
			default:
				table->setColPlotDesignation(j, Table::None);
			}

            table->setHeaderColType();//update header

            double **d_cells = new double* [columnCount];
            for(int i = 0; i < columnCount; ++i)
                d_cells[i] = new double [table->numRows()];

			for(int i=0; i<opj.numRows(s,j); ++i)
			{
				Origin::variant value = opj.oData(s,j,i);
				if(opj.colType(s,j) != Origin::SpreadColumn::Label && opj.colValueType(s,j) != Origin::Text)
				{// number
					if(value.type() != typeid(double))
						continue;

					double val = boost::get<double>(value);
					if(fabs(val)>0 && fabs(val)<2.0e-300)// empty entry
						continue;

                    table->setText(i, j, locale.toString(val, 'g', 16));
                    d_cells[j][i] = val;
				}
				else// label? doesn't seem to work
				{
					table->setText(i, j, QString(value.type() == typeid(string) ? boost::get<string>(value).c_str() : ""));
				}
			}
            table->saveToMemory(d_cells);

			QString format;
			switch(opj.colValueType(s,j))
			{
			case Origin::Numeric:
			case Origin::TextNumeric:
				int f;
				if(opj.colNumDisplayType(s,j)==0)
					f=0;
				else
					switch(opj.colValueTypeSpec(s,j))
					{
					case 0: //Decimal 1000
						f=1;
						break;
					case 1: //Scientific
						f=2;
						break;
					case 2: //Engeneering
					case 3: //Decimal 1,000
						f=0;
						break;
					}
				table->setColNumericFormat(f, opj.colDecPlaces(s,j), j);
				break;
			case Origin::Text:
				table->setTextFormat(j);
				break;
			case Origin::Date:
				switch(opj.colValueTypeSpec(s,j))
				{
                case -128:
                    format="dd/MM/yyyy";
					break;
                case -119:
                    format="dd/MM/yyyy HH:mm";
					break;
                case -118:
                    format="dd/MM/yyyy HH:mm:ss";
					break;
				case 0:
				case 9:
				case 10:
					format="dd.MM.yyyy";
					break;
				case 2:
					format="MMM d";
					break;
				case 3:
					format="M/d";
					break;
				case 4:
					format="d";
					break;
				case 5:
				case 6:
					format="ddd";
					break;
				case 7:
					format="yyyy";
					break;
				case 8:
					format="yy";
					break;
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					format="yyMMdd";
					break;
				case 16:
				case 17:
					format="MMM";
					break;
				case 19:
					format="M-d-yyyy";
					break;
				default:
					format="dd.MM.yyyy";
				}
				table->setDateFormat(format, j);
				break;
			case Origin::Time:
				switch(opj.colValueTypeSpec(s,j)+128)
				{
				case 0:
					format="hh:mm";
					break;
				case 1:
					format="hh";
					break;
				case 2:
					format="hh:mm:ss";
					break;
				case 3:
					format="hh:mm:ss.zzz";
					break;
				case 4:
					format="hh ap";
					break;
				case 5:
					format="hh:mm ap";
					break;
				case 6:
					format="mm:ss";
					break;
				case 7:
					format="mm:ss.zzz";
					break;
				case 8:
					format="hhmm";
					break;
				case 9:
					format="hhmmss";
					break;
				case 10:
					format="hh:mm:ss.zzz";
					break;
				}
				table->setTimeFormat(format, j);
				break;
			case Origin::Month:
				switch(opj.colValueTypeSpec(s,j)){
                    case 0:
                        format = "MMM";
					break;
                    case 1:
                        format = "MMMM";
					break;
                    case 2:
                        format = "M";
					break;
				}
				table->setMonthFormat(format, j);
				break;
			case Origin::Day:
				switch(opj.colValueTypeSpec(s,j)){
                    case 0:
                        format = "ddd";
					break;
                    case 1:
                        format = "dddd";
					break;
                    case 2:
                        format = "d";
					break;
				}
				table->setDayFormat(format, j);
				break;
			}
            table->freeMemory();
		}


		if(!(opj.spreadHidden(s)||opj.spreadLoose(s))||opj.Version()!=7.5)
		{
			switch(opj.spreadState(s))
			{
			case Origin::Window::Minimized:
				mw->minimizeWindow(table);
				break;
			case Origin::Window::Maximized:
				mw->maximizeWindow(table);
				break;
			default:
				table->showNormal();
			}

			//cascade the tables
			if(opj.Version() == 7.5)
			{
				table->move(QPoint(windowRect.left, windowRect.top));
			}
			else
			{
				int dx=table->verticalHeaderWidth();
				int dy=table->frameGeometry().height() - table->height();
				table->move(QPoint(visible_count*dx+xoffset*OBJECTXOFFSET,visible_count*dy));
				visible_count++;
			}
		}
	}

//Import matrices
	for (int s=0; s<opj.numMatrices(); s++)
	{
		int columnCount = opj.numMatrixCols(s);
		int rowCount = opj.numMatrixRows(s);

		Matrix* Matrix = mw->newMatrix(opj.matrixName(s), rowCount, columnCount);
		if (!Matrix)
			return false;

		Origin::Rect windowRect;
		if(opj.Version() == 7.5){
			windowRect = opj.matrixWindowRect(s);
			Matrix->resize(windowRect.width() - (Matrix->frameGeometry().width() - Matrix->width()),
				windowRect.height() - (Matrix->frameGeometry().height() - Matrix->height()));
		}

		Matrix->setCaptionPolicy((MdiSubWindow::CaptionPolicy)opj.matrixTitle(s));
		Matrix->setBirthDate(posixTimeToString(opj.matrixCreationDate(s)));

		Matrix->setWindowLabel(opj.matrixLabel(s));
		Matrix->setFormula(opj.matrixFormula(s));
		Matrix->setColumnsWidth(opj.matrixWidth(s)*QtiPlot_scaling_factor);
		if(opj.matrixViewType(s) == Origin::Matrix::ImageView)
			Matrix->setViewType(Matrix::ImageView);
		if(opj.matrixHeaderViewType(s) == Origin::Matrix::XY)
			Matrix->setHeaderViewType(Matrix::XY);
		vector<double> data = opj.matrixData(s);
		double *matrix_data = Matrix->matrixModel()->dataVector();
		int size = Matrix->numRows()*Matrix->numCols();
		for(int i=0; i < size; ++i)
		{
			matrix_data[i] = fabs(data[i]) < 2.0e-300 && fabs(data[i]) > 0 ? GSL_NAN : data[i];
		}

		QChar format;
		switch(opj.matrixValueTypeSpec(s))
		{
		case 0: //Decimal 1000
			format='f';
			break;
		case 1: //Scientific
			format='e';
			break;
		case 2: //Engeneering
		case 3: //Decimal 1,000
			format='g';
			break;
		}
		Matrix->setNumericFormat(format, opj.matrixSignificantDigits(s));

		if(!opj.matrixHidden(s)||opj.Version()!=7.5)
		{
			switch(opj.matrixState(s))
			{
			case Origin::Window::Minimized:
				mw->minimizeWindow(Matrix);
				break;
			case Origin::Window::Maximized:
				mw->maximizeWindow(Matrix);
				break;
			default:
				Matrix->showNormal();
			}

			//cascade the matrices
			if(opj.Version() == 7.5)
				Matrix->move(QPoint(windowRect.left, windowRect.top));
			else {
				int dx=Matrix->verticalHeaderWidth();
				int dy=Matrix->frameGeometry().height() - Matrix->height();
				Matrix->move(QPoint(visible_count*dx+xoffset*OBJECTXOFFSET,visible_count*dy));
				visible_count++;
			}
		}
	}

	if(visible_count>0)
		xoffset++;
	return true;
}

bool ImportOPJ::importNotes(const OriginFile& opj)
{
	int visible_count=0;
	for (int n=0; n<opj.numNotes(); n++)
	{
		QString name=opj.noteName(n);
		QRegExp rx("^@\\((\\S+)\\)$");
		if(rx.indexIn(name)==0)
			name=rx.cap(1);

		Note *note = mw->newNote(name);
		if(!note)
			return false;
		note->setWindowLabel(opj.noteLabel(n));
		note->setText(opj.noteText(n));
		note->setBirthDate(posixTimeToString(opj.noteCreationDate(n)));

		//cascade the notes
		int dx=20;
		int dy=note->frameGeometry().height() - note->height();
		note->move(QPoint(visible_count*dx+xoffset*OBJECTXOFFSET,visible_count*dy));
		visible_count++;
	}
	if(visible_count>0)
		xoffset++;
	return true;
}

bool ImportOPJ::importGraphs(const OriginFile& opj)
{
	double pi=3.141592653589793;
	int visible_count=0;
	int tickTypeMap[]={0,3,1,2};
	for (int g=0; g<opj.numGraphs(); ++g)
	{
		MultiLayer *ml = mw->multilayerPlot(opj.graphName(g), 0);
		if (!ml)
			return false;

		ml->setCaptionPolicy((MdiSubWindow::CaptionPolicy)opj.graphTitle(g));
		ml->setBirthDate(posixTimeToString(opj.graphCreationDate(g)));
		ml->hide();//!hack used in order to avoid resize and repaint events
		ml->setWindowLabel(opj.graphLabel(g));

		Origin::Rect graphRect = opj.graphRect(g);
		Origin::Rect graphWindowRect = opj.graphWindowRect(g);
		ml->resize(graphWindowRect.width() - (ml->frameGeometry().width() - ml->width()),
                    graphWindowRect.height() - (ml->frameGeometry().height() - ml->height()));

		double fXScale = (double)ml->width()/(double)graphRect.width();
		double fYScale = (double)ml->height()/(double)graphRect.height();
		fXScale = fYScale = QMIN(fXScale, fYScale);

		double fWindowFactor =  QMIN((double)graphWindowRect.width()/500.0, (double)graphWindowRect.height()/350.0);
		double fFontScaleFactor = 0.37*fWindowFactor;
		double fVectorArrowScaleFactor = 0.08*fWindowFactor;

		for(int l=0; l<opj.numLayers(g); ++l)
		{
			Graph *graph = ml->addLayer();
			if(!graph)
				return false;

			Origin::Rect layerRect = opj.layerRect(g,l);

			graph->setXAxisTitle(parseOriginText(QString::fromLocal8Bit(opj.layerXAxisTitle(g,l).text.c_str())));
			graph->setYAxisTitle(parseOriginText(QString::fromLocal8Bit(opj.layerYAxisTitle(g,l).text.c_str())));
			LegendWidget* legend = 0;
			if(!opj.layerLegend(g,l).text.empty())
			{
				legend = graph->newLegend(parseOriginText(QString::fromLocal8Bit(opj.layerLegend(g,l).text.c_str())));
			}
			int auto_color=0;
			int style=0;
			for(int c=0; c<opj.numCurves(g,l); c++)
			{
				try{
				QString data(opj.curveDataName(g,l,c));
				int color=0;
				switch(opj.curveType(g,l,c))
				{
				case Origin::GraphCurve::Line:
					style=Graph::Line;
					break;
				case Origin::GraphCurve::Scatter:
					style=Graph::Scatter;
					break;
				case Origin::GraphCurve::LineSymbol:
					style=Graph::LineSymbols;
					break;
				case Origin::GraphCurve::ErrorBar:
				case Origin::GraphCurve::XErrorBar:
					style=Graph::ErrorBars;
					break;
				case Origin::GraphCurve::Column:
					style=Graph::VerticalBars;
					break;
				case Origin::GraphCurve::Bar:
					style=Graph::HorizontalBars;
					break;
				case Origin::GraphCurve::Histogram:
					style=Graph::Histogram;
					break;
				case Origin::GraphCurve::Pie:
					style=Graph::Pie;
					break;
				case Origin::GraphCurve::Box:
					style=Graph::Box;
					break;
				case Origin::GraphCurve::FlowVector:
					style=Graph::VectXYXY;
					break;
				case Origin::GraphCurve::Vector:
					style=Graph::VectXYAM;
					break;
				case Origin::GraphCurve::Area:
				case Origin::GraphCurve::AreaStack:
					style=Graph::Area;
					break;
				case Origin::GraphCurve::TextPlot:
					style=Origin::GraphCurve::TextPlot;
					break;
				default:
					continue;
				}
				QString tableName;
				QStringList formulas;
				double start, end;
				int s;
				PlotCurve *curve = NULL;
				switch(data[0].toAscii())
				{
				case 'T':
					tableName = data.right(data.length()-2);
					if(style==Graph::ErrorBars)
					{
						int flags=opj.curveSymbolType(g,l,c);
						curve = (PlotCurve *)graph->addErrorBars(tableName + "_" + opj.curveXColName(g,l,c), mw->table(tableName), tableName + "_" + opj.curveYColName(g,l,c),
							((flags&0x10)==0x10?0:1), ceil(opj.curveLineWidth(g,l,c)), ceil(opj.curveSymbolSize(g,l,c)), QColor(Qt::black),
							(flags&0x40)==0x40, (flags&2)==2, (flags&1)==1);
					}
					else if(style==Graph::Histogram)
						curve = (PlotCurve *)graph->insertCurve(mw->table(tableName), tableName + "_" + opj.curveYColName(g,l,c), style);
					else if(style==Graph::Pie || style==Graph::Box)
					{
						QStringList names;
						names << (tableName + "_" + opj.curveYColName(g,l,c));
						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style==Graph::VectXYXY)
					{
						QStringList names;
						Origin::VectorProperties vector = opj.curveVectorProperties(g,l,c);
						names << (tableName + "_" + opj.curveXColName(g,l,c))
							<< (tableName + "_" + opj.curveYColName(g,l,c))
							<< (tableName + "_" + QString(vector.endXColumnName.c_str()))
							<< (tableName + "_" + QString(vector.endYColumnName.c_str()));

						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style==Graph::VectXYAM)
					{
						QStringList names;
						Origin::VectorProperties vector = opj.curveVectorProperties(g,l,c);
						names << (tableName + "_" + opj.curveXColName(g,l,c))
							<< (tableName + "_" + opj.curveYColName(g,l,c))
							<< (tableName + "_" + QString(vector.angleColumnName.c_str()))
							<< (tableName + "_" + QString(vector.magnitudeColumnName.c_str()));

						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style == Origin::GraphCurve::TextPlot)
					{
						Table* table = mw->table(tableName);
						QString labelsCol = opj.curveYColName(g,l,c);
						int xcol = table->colX(table->colIndex(labelsCol));
						int ycol = table->colY(table->colIndex(labelsCol));
						if (xcol < 0 || ycol < 0)
							return false;

						DataCurve* mc = graph->masterCurve(table->colName(xcol), table->colName(ycol));
						if(mc)
						{
							graph->replot();
							mc->setLabelsColumnName(labelsCol);
						}
					}
					else
						curve = (PlotCurve *)graph->insertCurve(mw->table(tableName), tableName + "_" + opj.curveXColName(g,l,c), tableName + "_" + opj.curveYColName(g,l,c), style);
					break;
				case 'F':
					s=opj.functionIndex(data.right(data.length()-2).toStdString().c_str());
					int type;
					if(opj.functionType(s)==Origin::Function::Polar)//Polar
					{
						type=2;
						formulas << opj.functionFormula(s) << "x";
						start = pi/180*opj.functionBegin(s) ;
						end = pi/180*opj.functionEnd(s);
					}
					else
					{
						type=0;
						formulas << opj.functionFormula(s);
						start = opj.functionBegin(s);
						end = opj.functionEnd(s);
					}
					curve = (PlotCurve *)graph->addFunction(formulas, start, end, opj.functionPoints(s), "x", type, opj.functionName(s));

					mw->updateFunctionLists(type, formulas);
					break;
				default:
					continue;
				}

				//strange behavior of insert curve - legend added - need to roolback legend text
				if(legend)
				{
					legend->setText(parseOriginText(QString::fromLocal8Bit(opj.layerLegend(g,l).text.c_str())));
				}

				CurveLayout cl = graph->initCurveLayout(style, opj.numCurves(g,l));
				cl.sSize = ceil(opj.curveSymbolSize(g,l,c));
				cl.penWidth=opj.curveSymbolThickness(g,l,c);
				color=opj.curveSymbolColor(g,l,c);
				if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area)&&color==0xF7)//0xF7 -Automatic color
					color=auto_color++;
				cl.symCol=color;
				switch(opj.curveSymbolType(g,l,c)&0xFF)
				{
				case 0: //NoSymbol
					cl.sType=0;
					break;
				case 1: //Rect
					cl.sType=2;
					break;
				case 2: //Ellipse
				case 20://Sphere
					cl.sType=1;
					break;
				case 3: //UTriangle
					cl.sType=6;
					break;
				case 4: //DTriangle
					cl.sType=5;
					break;
				case 5: //Diamond
					cl.sType=3;
					break;
				case 6: //Cross +
					cl.sType=9;
					break;
				case 7: //Cross x
					cl.sType=10;
					break;
				case 8: //Snow
					cl.sType=13;
					break;
				case 9: //Horizontal -
					cl.sType=11;
					break;
				case 10: //Vertical |
					cl.sType=12;
					break;
				case 15: //LTriangle
					cl.sType=7;
					break;
				case 16: //RTriangle
					cl.sType=8;
					break;
				case 17: //Hexagon
				case 19: //Pentagon
					cl.sType=15;
					break;
				case 18: //Star
					cl.sType=14;
					break;
				default:
					cl.sType=0;
				}

				switch(opj.curveSymbolType(g,l,c)>>8)
				{
				case 0:
					cl.fillCol=color;
					break;
				case 1:
				case 2:
				case 8:
				case 9:
				case 10:
				case 11:
					color=opj.curveSymbolFillColor(g,l,c);
					if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area)&&color==0xF7)//0xF7 -Automatic color
						color=17;// depend on Origin settings - not stored in file
					cl.fillCol=color;
					break;
				default:
					cl.fillCol=-1;
				}

				cl.lWidth = ceil(opj.curveLineWidth(g,l,c));
				color=opj.curveLineColor(g,l,c);
				cl.lCol=(color==0xF7?0:color); //0xF7 -Automatic color
				int linestyle=opj.curveLineStyle(g,l,c);
				cl.filledArea=(opj.curveIsFilledArea(g,l,c)||style==Graph::VerticalBars||style==Graph::HorizontalBars||style==Graph::Histogram||style == Graph::Pie)?1:0;
				if(cl.filledArea)
				{
					switch(opj.curveFillPattern(g,l,c))
					{
					case 0:
						cl.aStyle=0;
						break;
					case 1:
					case 2:
					case 3:
						cl.aStyle=4;
						break;
					case 4:
					case 5:
					case 6:
						cl.aStyle=5;
						break;
					case 7:
					case 8:
					case 9:
						cl.aStyle=6;
						break;
					case 10:
					case 11:
					case 12:
						cl.aStyle=1;
						break;
					case 13:
					case 14:
					case 15:
						cl.aStyle=2;
						break;
					case 16:
					case 17:
					case 18:
						cl.aStyle=3;
						break;
					}
					color=(cl.aStyle==0 ? opj.curveFillAreaColor(g,l,c) : opj.curveFillPatternColor(g,l,c));
					cl.aCol=(color==0xF7?0:color); //0xF7 -Automatic color
					if (style == Graph::VerticalBars || style == Graph::HorizontalBars || style == Graph::Histogram || style == Graph::Pie)
					{
						color=opj.curveFillPatternBorderColor(g,l,c);
						cl.lCol = (color==0xF7?0:color); //0xF7 -Automatic color
						color=(cl.aStyle==0 ? opj.curveFillAreaColor(g,l,c) : opj.curveFillPatternColor(g,l,c));
						cl.aCol=(color==0xF7?cl.lCol:color); //0xF7 -Automatic color
						cl.lWidth = ceil(opj.curveFillPatternBorderWidth(g,l,c));
						linestyle=opj.curveFillPatternBorderStyle(g,l,c);
					}
				}
				switch (linestyle)
				{
					case Origin::GraphCurve::Solid:
						cl.lStyle=0;
						break;
					case Origin::GraphCurve::Dash:
					case Origin::GraphCurve::ShortDash:
						cl.lStyle=1;
						break;
					case Origin::GraphCurve::Dot:
					case Origin::GraphCurve::ShortDot:
						cl.lStyle=2;
						break;
					case Origin::GraphCurve::DashDot:
					case Origin::GraphCurve::ShortDashDot:
						cl.lStyle=3;
						break;
					case Origin::GraphCurve::DashDotDot:
						cl.lStyle=4;
						break;
				}

				graph->updateCurveLayout(curve, &cl);
				if (style == Graph::VerticalBars || style == Graph::HorizontalBars)
				{
					QwtBarCurve *b = (QwtBarCurve*)graph->curve(c);
					if (b)
						b->setGap(qRound(100-opj.curveSymbolSize(g,l,c)*10));
				}
				else if(style == Graph::Histogram)
				{
					QwtHistogram *h = (QwtHistogram*)graph->curve(c);
					if (h)
					{
						vector<double> bin=opj.layerHistogram(g,l);
						if(bin.size()==3)
							h->setBinning(false, bin[0], bin[1], bin[2]);
						h->loadData();
					}
				}
				else if(style == Graph::Pie)
				{
					QwtPieCurve *p = (QwtPieCurve*)graph->curve(c);
					switch (linestyle)
					{
					case Origin::GraphCurve::Solid:
						cl.lStyle=Qt::SolidLine;
						break;
					case Origin::GraphCurve::Dash:
					case Origin::GraphCurve::ShortDash:
						cl.lStyle=Qt::DashLine;
						break;
					case Origin::GraphCurve::Dot:
					case Origin::GraphCurve::ShortDot:
						cl.lStyle=Qt::DotLine;
						break;
					case Origin::GraphCurve::DashDot:
					case Origin::GraphCurve::ShortDashDot:
						cl.lStyle=Qt::DashDotLine;
						break;
					case Origin::GraphCurve::DashDotDot:
						cl.lStyle=Qt::DashDotDotLine;
						break;
					}
					p->setPen(QPen(ColorBox::color(cl.lCol), cl.lWidth, (Qt::PenStyle)cl.lStyle));
					p->setFirstColor(opj.curveFillAreaFirstColor(g,l,c));
					//geometry
                    p->setRadius(opj.curvePieProperties(g,l,c).radius);
                    p->setThickness(opj.curvePieProperties(g,l,c).thickness);
					p->setViewAngle(opj.curvePieProperties(g,l,c).viewAngle);
					p->setStartAzimuth(opj.curvePieProperties(g,l,c).rotation);
					p->setCounterClockwise(opj.curvePieProperties(g,l,c).clockwiseRotation);
                    p->setHorizontalOffset(opj.curvePieProperties(g,l,c).horizontalOffset);
					//labels
					p->setLabelsEdgeDistance(opj.curvePieProperties(g,l,c).distance);
					p->setLabelsAutoFormat(opj.curvePieProperties(g,l,c).formatAutomatic);
					p->setLabelPercentagesFormat(opj.curvePieProperties(g,l,c).formatPercentages);
                    p->setLabelValuesFormat(opj.curvePieProperties(g,l,c).formatValues);
                    p->setLabelCategories(opj.curvePieProperties(g,l,c).formatCategories);
                    p->setFixedLabelsPosition(opj.curvePieProperties(g,l,c).positionAssociate);
				}
				else if(style == Graph::VectXYXY || style == Graph::VectXYAM)
				{
					Origin::VectorProperties vector = opj.curveVectorProperties(g,l,c);
					graph->updateVectorsLayout(c, ColorBox::color(cl.symCol), ceil(vector.width),
						floor(vector.arrowLenght*fVectorArrowScaleFactor + 0.5), vector.arrowAngle, vector.arrowClosed, vector.position);
				}
				switch(opj.curveLineConnect(g,l,c))
				{
				case Origin::GraphCurve::NoLine:
					graph->setCurveStyle(c, QwtPlotCurve::NoCurve);
					break;
				case Origin::GraphCurve::Straight:
					graph->setCurveStyle(c, QwtPlotCurve::Lines);
					break;
				case Origin::GraphCurve::BSpline:
				case Origin::GraphCurve::Bezier:
				case Origin::GraphCurve::Spline:
					graph->setCurveStyle(c, 5);
					break;
				case Origin::GraphCurve::StepHorizontal:
				case Origin::GraphCurve::StepHCenter:
					graph->setCurveStyle(c, QwtPlotCurve::Steps);
					break;
				case Origin::GraphCurve::StepVertical:
				case Origin::GraphCurve::StepVCenter:
					graph->setCurveStyle(c, 6);
					break;
				}

				}
				catch(...)
				{
				}
			}
			Origin::GraphLayerRange rangeX=opj.layerXRange(g,l);
			vector<int>    ticksX=opj.layerXTicks(g,l);
			Origin::GraphLayerRange rangeY=opj.layerYRange(g,l);
			vector<int>	   ticksY=opj.layerYTicks(g,l);
			if(style == Graph::HorizontalBars)
			{
				graph->setScale(0,rangeX.min,rangeX.max,rangeX.step,ticksX[0],ticksX[1],opj.layerXScale(g,l));
				graph->setScale(2,rangeY.min,rangeY.max,rangeY.step,ticksY[0],ticksY[1],opj.layerYScale(g,l));
			}
			else if(style != Graph::Box)
			{

				Origin::GraphAxisBreak breakX = opj.layerXBreak(g,l);
				Origin::GraphAxisBreak breakY = opj.layerYBreak(g,l);
				if(breakX.show)
					graph->setScale(2,rangeX.min,rangeX.max,rangeX.step,ticksX[0],ticksX[1],opj.layerXScale(g,l),
									false,
									breakX.from, breakX.to,
									breakX.position,
									breakX.scaleIncrementBefore, breakX.scaleIncrementAfter,
									breakX.minorTicksBefore, breakX.minorTicksAfter, breakX.log10);
				else
					graph->setScale(2,rangeX.min,rangeX.max,rangeX.step,ticksX[0],ticksX[1],opj.layerXScale(g,l));

				if(breakY.show)
					graph->setScale(0,rangeY.min,rangeY.max,rangeY.step,ticksY[0],ticksY[1],opj.layerXScale(g,l),
					false,
					breakY.from, breakY.to,
					breakY.position,
					breakY.scaleIncrementBefore, breakY.scaleIncrementAfter,
					breakY.minorTicksBefore, breakY.minorTicksAfter, breakY.log10);
				else
					graph->setScale(0,rangeY.min,rangeY.max,rangeY.step,ticksY[0],ticksY[1],opj.layerYScale(g,l));
			}

			//grid
			vector<Origin::GraphGrid> grids = opj.layerGrid(g,l);
			Grid *grid = graph->grid();
			grid->enableX(!grids[0].hidden);
			grid->enableXMin(!grids[1].hidden);
			grid->enableY(!grids[2].hidden);
			grid->enableYMin(!grids[3].hidden);

			grid->setMajPenX(QPen(ColorBox::color(grids[0].color), ceil(grids[0].width),
							Graph::getPenStyle(translateOrigin2QtiplotLineStyle(grids[0].style))));
			grid->setMinPenX(QPen(ColorBox::color(grids[1].color), ceil(grids[1].width),
							Graph::getPenStyle(translateOrigin2QtiplotLineStyle(grids[1].style))));
			grid->setMajPenY(QPen(ColorBox::color(grids[2].color), ceil(grids[2].width),
							Graph::getPenStyle(translateOrigin2QtiplotLineStyle(grids[2].style))));
			grid->setMinPenY(QPen(ColorBox::color(grids[3].color), ceil(grids[3].width),
							Graph::getPenStyle(translateOrigin2QtiplotLineStyle(grids[3].style))));

			grid->setAxis(2, 0);
			grid->enableZeroLineX(0);
			grid->enableZeroLineY(0);

			vector<Origin::GraphAxisFormat> formats=opj.layerAxisFormat(g,l);
			vector<Origin::GraphAxisTick> ticks=opj.layerAxisTickLabels(g,l);
			for(int i=0; i<4; ++i)
			{
				QString data(ticks[i].dataName.c_str());
				QString tableName=data.right(data.length()-2) + "_" + ticks[i].columnName.c_str();

				QString formatInfo;
				int format = 0;
				int type = 0;
				int prec = ticks[i].decimalPlaces;
				switch(ticks[i].valueType)
				{
				case Origin::Numeric:
					type = ScaleDraw::Numeric;
					switch(ticks[i].valueTypeSpecification)
					{
					case 0: //Decimal 1000
						if(prec != -1)
							format = 1;
						else
							format = 0;

						break;
					case 1: //Scientific
						format=2;
						break;
					case 2: //Engeneering
					case 3: //Decimal 1,000
						format=0;
						break;
					}
					if(prec == -1)
						prec = 2;
					break;
				case Origin::Text: //Text
					type=ScaleDraw::Text;
					break;
				case 2: // Date
					type=ScaleDraw::Date;
					break;
				case 3: // Time
					type=ScaleDraw::Time;
					break;
				case Origin::Month: // Month
					type=ScaleDraw::Month;
					format=ticks[i].valueTypeSpecification;
					break;
				case Origin::Day: // Day
					type=ScaleDraw::Day;
					format=ticks[i].valueTypeSpecification;
					break;
				case Origin::ColumnHeading:
					type=ScaleDraw::ColHeader;
					switch(ticks[i].valueTypeSpecification)
					{
					case 0: //Decimal 1000
						format=1;
						break;
					case 1: //Scientific
						format=2;
						break;
					case 2: //Engeneering
					case 3: //Decimal 1,000
						format=0;
						break;
					}
					prec=2;
					break;
				default:
					type=ScaleDraw::Numeric;
					format=0;
					prec=2;
				}

				graph->showAxis(i, type, tableName, mw->table(tableName), !(formats[i].hidden),
					tickTypeMap[formats[i].majorTicksType], tickTypeMap[formats[i].minorTicksType],
					!(ticks[i].hidden),	ColorBox::color(formats[i].color), format, prec,
					ticks[i].rotation, 0, "", (ticks[i].color==0xF7 ? ColorBox::color(formats[i].color) : ColorBox::color(ticks[i].color)));

				QFont fnt = graph->axisTitleFont(i);
				int fontSize = 0;
				switch(i)
				{
				case 0:
				case 1:
					fontSize = opj.layerYAxisTitle(g,l).fontSize;
					break;
				case 2:
				case 3:
					fontSize = opj.layerXAxisTitle(g,l).fontSize;
					break;
				}
				if(fontSize > 0)
				{
					fnt.setPointSize(floor(fontSize*fFontScaleFactor + 0.5));
					fnt.setBold(false);
					graph->setAxisTitleFont(i, fnt);

					fnt = graph->axisFont(i);
					fnt.setPointSize(floor(ticks[i].fontSize*fFontScaleFactor + 0.5));
					graph->setAxisFont(i, fnt);
				}
			}

			graph->setAutoscaleFonts(true);

			int nXDelta = graph->width() - graph->canvas()->width();
			int nYDelta = graph->height() - graph->canvas()->height();
			QPoint posCanvas =  graph->canvas()->pos();

			graph->resize(layerRect.width()*fXScale + nXDelta,
				layerRect.height()*fYScale + nYDelta);

			int newXGraphPos = layerRect.left*fXScale - posCanvas.x() - ml->x();
			int newYGraphPos = layerRect.top*fYScale - posCanvas.y() - ml->y();
			graph->move((newXGraphPos > 0 ? newXGraphPos : 0), (newYGraphPos > 0 ? newYGraphPos : 0));

			graph->resize(layerRect.width()*fXScale + nXDelta,
				layerRect.height()*fYScale + nYDelta);

			//add texts
			vector<Origin::TextBox> texts=opj.layerTexts(g, l);
			if(style != Graph::Pie)
			{
				for(int i=0; i<texts.size(); ++i)
				{
					addText(texts[i], graph, 0, layerRect, fFontScaleFactor, fXScale, fYScale);
				}
			}

			if(legend)
				addText(opj.layerLegend(g,l), graph, legend, layerRect, fFontScaleFactor, fXScale, fYScale);

			vector<Origin::Line> lines = opj.layerLines(g, l);
			for(int i=0; i<lines.size(); ++i)
			{
				ArrowMarker mrk;
				mrk.setStartPoint(lines[i].begin.x, lines[i].begin.y);
				mrk.setEndPoint(lines[i].end.x, lines[i].end.y);
				mrk.drawStartArrow(lines[i].begin.shapeType > 0);
				mrk.drawEndArrow(lines[i].end.shapeType > 0);
				mrk.setHeadLength(lines[i].end.shapeLength);
                mrk.setHeadAngle(arrowAngle(lines[i].end.shapeLength, lines[i].end.shapeWidth));
				mrk.setColor(ColorBox::color(lines[i].color));
				mrk.setWidth((int)lines[i].width);
				Qt::PenStyle s;

				switch(lines[i].style)
				{
				case Origin::GraphCurve::Solid:
					s=Qt::SolidLine;
					break;
				case Origin::GraphCurve::Dash:
				case Origin::GraphCurve::ShortDash:
					s=Qt::DashLine;
					break;
				case Origin::GraphCurve::Dot:
				case Origin::GraphCurve::ShortDot:
					s=Qt::DotLine;
					break;
				case Origin::GraphCurve::DashDot:
				case Origin::GraphCurve::ShortDashDot:
					s=Qt::DashDotLine;
					break;
				case Origin::GraphCurve::DashDotDot:
					s=Qt::DashDotDotLine;
					break;
				default:
					s=Qt::SolidLine;
				}
				mrk.setStyle(s);
				graph->addArrow(&mrk);
			}

			vector<Origin::Bitmap> bitmaps=opj.layerBitmaps(g, l);
			for(int i=0; i<bitmaps.size(); ++i)
			{
				QPixmap bmp;
				bmp.loadFromData(bitmaps[i].data, bitmaps[i].size, "BMP");
				QTemporaryFile file;
				file.setFileTemplate(QDir::tempPath() + "/XXXXXX.bmp");
				if (file.open())
				{
					bmp.save(file.fileName(), "BMP");
					ImageWidget *mrk = graph->addImage(file.fileName());
					double left, top, right, bottom;
					left = top = right = bottom = 0.0;
					switch(bitmaps[i].attach)
					{
					case Origin::Scale:
						left = bitmaps[i].left;
						top = bitmaps[i].top;
						right = left + bitmaps[i].width;
						bottom = top - bitmaps[i].height;
						break;
					case Origin::Frame:
						if(bitmaps[i].width > 0)
						{
							left = (rangeX.max-rangeX.min)*bitmaps[i].left + rangeX.min;
							right = left + bitmaps[i].width;
						}
						else
						{
							right = (rangeX.max-rangeX.min)*bitmaps[i].left + rangeX.min;
							left = right + bitmaps[i].width;
						}

						if(bitmaps[i].height > 0)
						{
							top = rangeY.max - (rangeY.max-rangeY.min)*bitmaps[i].top;
							bottom = top - bitmaps[i].height;
						}
						else
						{
							bottom = rangeY.max - (rangeY.max-rangeY.min)*bitmaps[i].top;
							top = bottom - bitmaps[i].height;
						}
						break;
					case Origin::Page:
						//rect graphRect = opj.graphRect(g);
						left = (rangeX.max-rangeX.min)*(bitmaps[i].left - (double)layerRect.left/(double)graphRect.width())/((double)layerRect.width()/(double)graphRect.width()) + rangeX.min;
						top = rangeY.max - (rangeY.max-rangeY.min)*(bitmaps[i].top - (double)layerRect.top/(double)graphRect.height())/((double)layerRect.height()/(double)graphRect.height());
						right = left + bitmaps[i].width;
						bottom = top - bitmaps[i].height;
						break;
					}

					mrk->setCoordinates(left, top, right, bottom);
				}
			}
		}
		//cascade the graphs
		if(!opj.graphHidden(g))
		{
			ml->move(QPoint(graphWindowRect.left, graphWindowRect.top));

			switch(opj.graphState(g))
			{
			case Origin::Window::Minimized:
				mw->minimizeWindow(ml);
				break;
			case Origin::Window::Maximized:
				ml->show(); // to correct scaling with maximize
				mw->maximizeWindow(ml);
				break;
			default:
				ml->show();
			}
		}
		else
		{
			ml->show();
			//ml->arrangeLayers(true,true);
			mw->hideWindow(ml);
		}
	}
	if(visible_count>0)
		xoffset++;
	return true;
}

void ImportOPJ::addText(const Origin::TextBox& _text, Graph* graph, LegendWidget* txt, const Origin::Rect& layerRect, double fFontScaleFactor, double fXScale, double fYScale)
{
	int bkg;
	switch(_text.borderType)
	{
	case Origin::TextBox::BlackLine:
		bkg=1;
		break;
	case Origin::TextBox::Shadow:
	case Origin::TextBox::DarkMarble:
		bkg=2;
		break;
	default:
		bkg=0;
		break;
	}

	if(!txt)
		txt=graph->newLegend(parseOriginText(QString::fromLocal8Bit(_text.text.c_str())));

	QFont font(mw->plotLegendFont);
	font.setPointSize(floor(_text.fontSize*fFontScaleFactor + 0.5));
	txt->setAngle(_text.rotation);
	txt->setTextColor(ColorBox::color(_text.color));
	txt->setFont(font);
	txt->setFrameStyle(bkg);

	Origin::Rect txtRect=_text.clientRect;
	int x=(txtRect.left>layerRect.left ? txtRect.left-layerRect.left : 0);
	int y=(txtRect.top>layerRect.top ? txtRect.top-layerRect.top : 0);
	txt->move(QPoint(x*fXScale, y*fYScale));

	/*QRect qtiRect=graph->canvas()->geometry();
	txt->setOrigin(QPoint(x*qtiRect.width()/layerRect.width(),
		y*qtiRect.height()/layerRect.height()));*/
}

QString ImportOPJ::parseOriginText(const QString &str)
{
	QStringList lines=str.split("\n");
	QString text="";
	for(int i=0; i<lines.size(); ++i)
	{
		if(i>0)
			text.append("\n");
		text.append(parseOriginTags(lines[i]));
	}
	return text;
}

QString ImportOPJ::parseOriginTags(const QString &str)
{
	QString line=str;
	//Lookbehind conditions are not supported - so need to reverse string
	QRegExp rx("\\)[^\\)\\(]*\\((?!\\s*[buig\\+\\-]\\s*\\\\)");
	QRegExp rxfont("\\)[^\\)\\(]*\\((?![^\\:]*\\:f\\s*\\\\)");
	QString linerev = strreverse(line);
	QString lBracket=strreverse("&lbracket;");
	QString rBracket=strreverse("&rbracket;");
	QString ltagBracket=strreverse("&ltagbracket;");
	QString rtagBracket=strreverse("&rtagbracket;");
	int pos1=rx.indexIn(linerev);
	int pos2=rxfont.indexIn(linerev);

	while (pos1>-1 || pos2>-1) {
		if(pos1==pos2)
		{
			QString value = rx.cap(0);
			int len=value.length();
			value=rBracket+value.mid(1,len-2)+lBracket;
			linerev.replace(pos1, len, value);
		}
		else if ((pos1>pos2&&pos2!=-1)||pos1==-1)
		{
			QString value = rxfont.cap(0);
			int len=value.length();
			value=rtagBracket+value.mid(1,len-2)+ltagBracket;
			linerev.replace(pos2, len, value);
		}
		else if ((pos2>pos1&&pos1!=-1)||pos2==-1)
		{
			QString value = rx.cap(0);
			int len=value.length();
			value=rtagBracket+value.mid(1,len-2)+ltagBracket;
			linerev.replace(pos1, len, value);
		}

		pos1=rx.indexIn(linerev);
		pos2=rxfont.indexIn(linerev);
	}
	linerev.replace(ltagBracket, "(");
	linerev.replace(rtagBracket, ")");

	line = strreverse(linerev);

	//replace \b(...), \i(...), \u(...), \g(...), \+(...), \-(...), \f:font(...) tags
	QString rxstr[]={
		"\\\\\\s*b\\s*\\(",
		"\\\\\\s*i\\s*\\(",
		"\\\\\\s*u\\s*\\(",
		"\\\\\\s*g\\s*\\(",
		"\\\\\\s*\\+\\s*\\(",
		"\\\\\\s*\\-\\s*\\(",
		"\\\\\\s*f\\:[^\\(]*\\("};
	int postag[]={0,0,0,0,0,0,0};
	QString ltag[]={"<b>","<i>","<u>","<font face=Symbol>","<sup>","<sub>","<font face=%1>"};
	QString rtag[]={"</b>","</i>","</u>","</font>","</sup>","</sub>","</font>"};
	QRegExp rxtags[7];
	for(int i=0; i<7; ++i)
		rxtags[i].setPattern(rxstr[i]+"[^\\(\\)]*\\)");

	bool flag=true;
	while(flag) {
		for(int i=0; i<7; ++i)
		{
			postag[i] = rxtags[i].indexIn(line);
			while (postag[i] > -1) {
				QString value = rxtags[i].cap(0);
				int len=value.length();
				int pos2=value.indexOf("(");
				if(i<6)
					value=ltag[i]+value.mid(pos2+1,len-pos2-2)+rtag[i];
				else
				{
					int posfont=value.indexOf("f:");
					value=ltag[i].arg(value.mid(posfont+2,pos2-posfont-2))+value.mid(pos2+1,len-pos2-2)+rtag[i];
				}
				line.replace(postag[i], len, value);
				postag[i] = rxtags[i].indexIn(line);
			}
		}
		flag=false;
		for(int i=0; i<7; ++i)
		{
			if(rxtags[i].indexIn(line)>-1)
			{
				flag=true;
				break;
			}
		}
	}

	//replace unclosed tags
	for(int i=0; i<6; ++i)
		line.replace(QRegExp(rxstr[i]), ltag[i]);
	rxfont.setPattern(rxstr[6]);
	int pos = rxfont.indexIn(line);
	while (pos > -1) {
		QString value = rxfont.cap(0);
		int len=value.length();
		int posfont=value.indexOf("f:");
		value=ltag[6].arg(value.mid(posfont+2,len-posfont-3));
		line.replace(pos, len, value);
		pos = rxfont.indexIn(line);
	}

	line.replace("&lbracket;", "(");
	line.replace("&rbracket;", ")");

	return line;
}

//TODO: bug in grid dialog
//		scale/minor ticks checkbox
//		histogram: autobin export
//		if prec not setted - automac+4digits
