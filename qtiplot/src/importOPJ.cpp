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
#include "matrix/Matrix.h"
#include "matrix/MatrixModel.h"
#include "ColorBox.h"
#include "PatternBox.h"
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
#include "plot2D/RectangleWidget.h"
#include "plot2D/EllipseWidget.h"

#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"

#include <gsl/gsl_math.h>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

#define OBJECTXOFFSET 200

QMap<Origin::GraphCurve::LineStyle, Qt::PenStyle> ImportOPJ::lineStyles;
QMap<Origin::FillPattern, int> ImportOPJ::patternStyles;
QMap<Origin::ProjectNode::NodeType, QString> ImportOPJ::classes;

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
	stringstream ss;
	ss.imbue(locale(locale::classic(), new time_facet("%d.%m.%Y %H:%M:%S")));
	ss << pt;
	return QString::fromStdString(ss.str());
}

ImportOPJ::ImportOPJ(ApplicationWindow *app, const QString& filename) :
		mw(app)
{
	//////////////////////Origin params to QtiPlot mapping////////////////////
	classes[Origin::ProjectNode::SpreadSheet] = "Table";
	classes[Origin::ProjectNode::Matrix] = "Matrix";
	classes[Origin::ProjectNode::Graph] = "MultiLayer";
	classes[Origin::ProjectNode::Note] = "Note";
	lineStyles[Origin::GraphCurve::Solid] = Qt::SolidLine;
	lineStyles[Origin::GraphCurve::Dash] = Qt::DashLine;
	lineStyles[Origin::GraphCurve::ShortDash] = Qt::DashLine;
	lineStyles[Origin::GraphCurve::Dot] = Qt::DotLine;
	lineStyles[Origin::GraphCurve::ShortDot] = Qt::DotLine;
	lineStyles[Origin::GraphCurve::DashDot] = Qt::DashDotLine;
	lineStyles[Origin::GraphCurve::ShortDashDot] = Qt::DashDotLine;
	lineStyles[Origin::GraphCurve::DashDotDot] = Qt::DashDotDotLine;

	patternStyles[Origin::NoFill] = 255;
	patternStyles[Origin::BDiagDense] = 4;
	patternStyles[Origin::BDiagMedium] = 4;
	patternStyles[Origin::BDiagSparse] = 4;
	patternStyles[Origin::FDiagDense] = 5;
	patternStyles[Origin::FDiagMedium] = 5;
	patternStyles[Origin::FDiagSparse] = 5;
	patternStyles[Origin::DiagCrossDense] = 6;
	patternStyles[Origin::DiagCrossMedium] = 6;
	patternStyles[Origin::DiagCrossSparse] = 6;
	patternStyles[Origin::HorizontalDense] = 1;
	patternStyles[Origin::HorizontalMedium] = 1;
	patternStyles[Origin::HorizontalSparse] = 1;
	patternStyles[Origin::VerticalDense] = 2;
	patternStyles[Origin::VerticalMedium] = 2;
	patternStyles[Origin::VerticalSparse] = 2;
	patternStyles[Origin::CrossDense] = 3;
	patternStyles[Origin::CrossMedium] = 3;
	patternStyles[Origin::CrossSparse] = 3;

	//////////////////////////////////////////////////////////////////////////
	xoffset=0;
	OriginFile opj((const char *)filename.latin1());
	parse_error = opj.parse();
	importTables(opj);
	importGraphs(opj);
	importNotes(opj);
	if(filename.endsWith(".opj", Qt::CaseInsensitive))
		createProjectTree(opj);
	mw->showResults(opj.resultsLogString().c_str(), mw->logWindow->isVisible());
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
		if(sib->type == Origin::ProjectNode::Folder)
		{
			parent[sib] = mw->addFolder(sib->name.c_str(), parent.value(projectTree->parent(sib)));
			parent[sib]->setBirthDate(posixTimeToString(sib->creationDate));
			parent[sib]->setModificationDate(posixTimeToString(sib->modificationDate));
		}
		else
		{
			QString name = sib->name.c_str();
			if(sib->type == Origin::ProjectNode::Note)
			{
				QRegExp rx("^@\\((\\S+)\\)$");
				if(rx.indexIn(name) == 0)
					name = rx.cap(1);
			}

			MdiSubWindow* w = projectFolder->window(name, classes[sib->type]);
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

bool ImportOPJ::importTables(const OriginFile& opj)
{
	int visible_count = 0;
	int QtiPlot_scaling_factor = 10; //in Origin width is measured in characters while in QtiPlot - pixels --- need to be accurate
	for(unsigned int s = 0; s < opj.spreadCount(); ++s)
	{
		Origin::SpreadSheet spread = opj.spread(s);
		int columnCount = spread.columns.size();
		int maxrows = spread.maxRows;
		if(!columnCount) //remove tables without cols
			continue;

		Table *table = (spread.hidden || spread.loose) && opj.version() == 7.5 ? mw->newHiddenTable(spread.name.c_str(), spread.label.c_str(), maxrows, columnCount)
										: mw->newTable(spread.name.c_str(), maxrows, columnCount);
		if (!table)
			return false;

		Origin::Rect windowRect;
		if(opj.version() == 7.5)
		{
			windowRect = spread.frameRect;
			table->resize(windowRect.width() - (table->frameGeometry().width() - table->width()),
				windowRect.height() - (table->frameGeometry().height() - table->height()));
		}

		table->setCaptionPolicy((MdiSubWindow::CaptionPolicy)spread.title);
		table->setBirthDate(posixTimeToString(spread.creationDate));

        QLocale locale = mw->locale();
		table->setWindowLabel(spread.label.c_str());
		for(int j = 0; j < columnCount; ++j)
		{
			Origin::SpreadColumn column = spread.columns[j];
			QString name(column.name.c_str());
			table->setColName(j, name.replace(QRegExp(".*_"),""));
			table->setCommand(j, QString(column.command.c_str()));
			table->setColComment(j, QString(column.comment.c_str()));
			table->setColumnWidth(j, column.width * QtiPlot_scaling_factor);

			switch(column.type)
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

			for(unsigned int i = 0; i < column.data.size(); ++i)
			{
				Origin::variant value = column.data[i];
				if(column.type != Origin::SpreadColumn::Label && column.valueType != Origin::Text)
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
			switch(column.valueType)
			{
			case Origin::Numeric:
			case Origin::TextNumeric:
				int f;
				if(column.numericDisplayType == 0)
					f = 0;
				else
					switch(column.valueTypeSpecification)
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
				table->setColNumericFormat(f, column.decimalPlaces, j);
				break;
			case Origin::Text:
				table->setTextFormat(j);
				break;
			case Origin::Date:
				switch(column.valueTypeSpecification)
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
				switch(column.valueTypeSpecification + 128)
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
				switch(column.valueTypeSpecification)
				{
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
				switch(column.valueTypeSpecification)
				{
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


		if(!(spread.hidden || spread.loose) || opj.version() != 7.5)
		{
			switch(spread.state)
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
			if(opj.version() == 7.5)
			{
				table->move(QPoint(windowRect.left, windowRect.top));
			}
			else
			{
				int dx = table->verticalHeaderWidth();
				int dy = table->frameGeometry().height() - table->height();
				table->move(QPoint(visible_count*dx + xoffset*OBJECTXOFFSET, visible_count*dy));
				++visible_count;
			}
		}
	}

//Import matrices
	for(unsigned int s = 0; s < opj.matrixCount(); ++s)
	{
		Origin::Matrix matrix = opj.matrix(s);
		int columnCount = matrix.columnCount;
		int rowCount = matrix.rowCount;

		Matrix* Matrix = mw->newMatrix(matrix.name.c_str(), rowCount, columnCount);
		if (!Matrix)
			return false;

		Origin::Rect windowRect;
		if(opj.version() == 7.5)
		{
			windowRect = matrix.frameRect;
			Matrix->resize(windowRect.width() - (Matrix->frameGeometry().width() - Matrix->width()),
				windowRect.height() - (Matrix->frameGeometry().height() - Matrix->height()));
		}

		Matrix->setCaptionPolicy((MdiSubWindow::CaptionPolicy)matrix.title);
		Matrix->setBirthDate(posixTimeToString(matrix.creationDate));

		Matrix->setWindowLabel(matrix.label.c_str());
		Matrix->setFormula(matrix.command.c_str());
		Matrix->setColumnsWidth(matrix.width * QtiPlot_scaling_factor);
		if(matrix.view == Origin::Matrix::ImageView)
			Matrix->setViewType(Matrix::ImageView);

		if(matrix.header == Origin::Matrix::XY)
			Matrix->setHeaderViewType(Matrix::XY);

		vector<double>* data = &matrix.data;
		double* matrix_data = Matrix->matrixModel()->dataVector();
		int size = Matrix->numRows()*Matrix->numCols();
		for(int i=0; i < size; ++i)
		{
			matrix_data[i] = fabs(data->at(i)) < 2.0e-300 && fabs(data->at(i)) > 0 ? GSL_NAN : data->at(i);
		}

		QChar format;
		switch(matrix.valueTypeSpecification)
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
		Matrix->setNumericFormat(format, matrix.significantDigits);

		if(!matrix.hidden || opj.version() != 7.5)
		{
			switch(matrix.state)
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
			if(opj.version() == 7.5)
			{
				Matrix->move(QPoint(windowRect.left, windowRect.top));
			}
			else
			{
				int dx = Matrix->verticalHeaderWidth();
				int dy = Matrix->frameGeometry().height() - Matrix->height();
				Matrix->move(QPoint(visible_count*dx + xoffset*OBJECTXOFFSET, visible_count*dy));
				++visible_count;
			}
		}
	}

	if(visible_count > 0)
		++xoffset;

	return true;
}

bool ImportOPJ::importNotes(const OriginFile& opj)
{
	for(unsigned int n = 0; n < opj.noteCount(); ++n)
	{
		Origin::Note _note = opj.note(n);
		QString name = _note.name.c_str();
		QRegExp rx("^@\\((\\S+)\\)$");
		if(rx.indexIn(name) == 0)
			name = rx.cap(1);

		Note* note = mw->newNote(name);
		if(!note)
			return false;

		note->setName(name);

		note->setWindowLabel(_note.label.c_str());
		note->setText(_note.text.c_str());
		note->setCaptionPolicy((MdiSubWindow::CaptionPolicy)_note.title);
		note->setBirthDate(posixTimeToString(_note.creationDate));

		Origin::Rect windowRect;
		windowRect = _note.frameRect;
		note->resize(windowRect.width() - (note->frameGeometry().width() - note->width()),
			windowRect.height() - (note->frameGeometry().height() - note->height()));

		note->move(QPoint(windowRect.left, windowRect.top));

		/*switch(_note.state)
		{
		case Origin::Window::Minimized:
			mw->minimizeWindow(note);
			break;
		case Origin::Window::Maximized:
			mw->maximizeWindow(note);
			break;
		default:
			note->showNormal();
		}*/

		if(_note.hidden)
			mw->hideWindow(note);
	}

	return true;
}

bool ImportOPJ::importGraphs(const OriginFile& opj)
{
	double pi=3.141592653589793;
	int tickTypeMap[]={0,3,1,2};

	MultiLayer* fake = mw->multilayerPlot("fake", 0);
	fake->setParent(0);
	int frameWidth = fake->frameGeometry().width() - fake->geometry().width();
	int frameHeight = fake->frameGeometry().height() - fake->geometry().height();
	fake->askOnCloseEvent(false);
	fake->close();

	for(unsigned int g = 0; g < opj.graphCount(); ++g)
	{
		Origin::Graph _graph = opj.graph(g);
		MultiLayer *ml = mw->multilayerPlot(_graph.name.c_str(), 0);
		if (!ml)
			return false;

		ml->setCaptionPolicy((MdiSubWindow::CaptionPolicy)_graph.title);
		ml->setBirthDate(posixTimeToString(_graph.creationDate));
		ml->hide();//!hack used in order to avoid resize and repaint events
		ml->setWindowLabel(_graph.label.c_str());

		Origin::Rect graphRect(_graph.width, _graph.height);
		Origin::Rect graphWindowRect = _graph.frameRect;
		double ratio = (double)(graphWindowRect.width() - frameWidth)/(double)(graphWindowRect.height() - frameHeight);

		int width = _graph.width;
		int height = _graph.height;
		if((double)(_graph.width)/(double)(_graph.height) < ratio)
		{
			width = height * ratio;
		}
		else
		{
			height = width / ratio;
		}

		//ml->resize(width, height);

		int yOffset = LayerButton::btnSize();

		ml->resize(graphWindowRect.width()/* - frameWidth*/, graphWindowRect.height()/* - frameHeight*/);

		double fScale = (double)(graphWindowRect.width() - frameWidth)/(double)width;

		double fWindowFactor =  QMIN((double)graphWindowRect.width()/500.0, (double)graphWindowRect.height()/350.0);
		double fFontScaleFactor = 300*fScale/72;//0.37*fWindowFactor;
		double fVectorArrowScaleFactor = 0.08*fWindowFactor;

		for(unsigned int l = 0; l < _graph.layers.size(); ++l)
		{
			Origin::GraphLayer& layer = _graph.layers[l];
			Graph *graph = ml->addLayer();
			if(!graph)
				return false;

			Origin::Rect layerRect = layer.clientRect;

			graph->setXAxisTitle(parseOriginText(QString::fromLocal8Bit(layer.xAxis.label.text.c_str())));
			graph->setYAxisTitle(parseOriginText(QString::fromLocal8Bit(layer.yAxis.label.text.c_str())));

			int auto_color = -1;
			int style = 0;
			for(unsigned int c = 0; c < layer.curves.size(); ++c)
			{
				Origin::GraphCurve& _curve = layer.curves[c];
				try
				{
				QString data(_curve.dataName.c_str());
				int color = 0;
				switch(_curve.type)
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
				PlotCurve* curve = NULL;
				Origin::Function function;
				switch(data[0].toAscii())
				{
				case 'T':
					tableName = data.right(data.length()-2);
					if(style==Graph::ErrorBars)
					{
						int flags=_curve.symbolType;
						curve = (PlotCurve*)graph->addErrorBars(QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), mw->table(tableName), QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()),
							((flags&0x10)==0x10?0:1), ceil(_curve.lineWidth), ceil(_curve.symbolSize), QColor(Qt::black),
							(flags&0x40)==0x40, (flags&2)==2, (flags&1)==1);
					}
					else if(style==Graph::Histogram)
						curve = (PlotCurve*)graph->insertCurve(mw->table(tableName), QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);
					else if(style==Graph::Pie || style==Graph::Box)
					{
						QStringList names;
						names << QString("%1_%2").arg(tableName, _curve.yColumnName.c_str());
						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style==Graph::VectXYXY)
					{
						QStringList names;
						Origin::VectorProperties vector = _curve.vector;
						names << QString("%1_%2").arg(tableName, _curve.xColumnName.c_str())
							<< QString("%1_%2").arg(tableName, _curve.yColumnName.c_str())
							<< (tableName + "_" + QString(vector.endXColumnName.c_str()))
							<< (tableName + "_" + QString(vector.endYColumnName.c_str()));

						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style==Graph::VectXYAM)
					{
						QStringList names;
						Origin::VectorProperties vector = _curve.vector;
						names << QString("%1_%2").arg(tableName, _curve.xColumnName.c_str())
							<< QString("%1_%2").arg(tableName, _curve.yColumnName.c_str())
							<< (tableName + "_" + QString(vector.angleColumnName.c_str()))
							<< (tableName + "_" + QString(vector.magnitudeColumnName.c_str()));

						graph->addCurves(mw->table(tableName), names, style);
					}
					else if(style == Origin::GraphCurve::TextPlot)
					{
						Table* table = mw->table(tableName);
						QString labelsCol(_curve.yColumnName.c_str());
						int xcol = table->colX(table->colIndex(labelsCol));
						int ycol = table->colY(table->colIndex(labelsCol));
						if (xcol < 0 || ycol < 0)
							return false;

						DataCurve* mc = graph->masterCurve(table->colName(xcol), table->colName(ycol));
						if(mc)
						{
							graph->replot();
							mc->setLabelsColumnName(labelsCol);
							mc->setLabelsRotation(_curve.text.rotation);
							mc->setLabelsWhiteOut(_curve.text.whiteOut);
							mc->setLabelsOffset(_curve.text.xOffset, _curve.text.yOffset);
							mc->setLabelsColor(ColorBox::color(_curve.text.color));
							int align = -1;
							switch(_curve.text.justify)
							{
							case Origin::TextProperties::Center:
								align = Qt::AlignHCenter;
								break;

							case Origin::TextProperties::Left:
								align = Qt::AlignLeft;
								break;

							case Origin::TextProperties::Right:
								align = Qt::AlignRight;
								break;
							}
							mc->setLabelsAlignment(align);
							QFont fnt = mc->labelsFont();
							fnt.setBold(_curve.text.fontBold);
							fnt.setItalic(_curve.text.fontItalic);
							fnt.setUnderline(_curve.text.fontUnderline);
							fnt.setPointSize(floor(_curve.text.fontSize*fFontScaleFactor + 0.5));
							mc->setLabelsFont(fnt);
						}
					}
					else
						curve = (PlotCurve *)graph->insertCurve(mw->table(tableName), QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);
					break;
				case 'F':
					s = opj.functionIndex(data.right(data.length()-2).toStdString());
					function = opj.function(s);

					int type;
					if(function.type == Origin::Function::Polar)//Polar
					{
						type = 2;
						formulas << function.formula.c_str() << "x";
						start = pi/180 * function.begin;
						end = pi/180 * function.end;
					}
					else
					{
						type = 0;
						formulas << function.formula.c_str();
						start = function.begin;
						end = function.end;
					}
					curve = (PlotCurve*)graph->addFunction(formulas, start, end, function.totalPoints, "x", type, function.name.c_str());

					mw->updateFunctionLists(type, formulas);
					break;
				default:
					continue;
				}

				CurveLayout cl = graph->initCurveLayout(style, layer.curves.size());
				cl.sSize = ceil(_curve.symbolSize);
				cl.penWidth=_curve.symbolThickness;
				color=_curve.symbolColor;
				if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area)&&color==0xF7)//0xF7 -Automatic color
					color=++auto_color;
				cl.symCol=color;
				switch(_curve.symbolType&0xFF)
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

				switch(_curve.symbolType>>8)
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
					color=_curve.symbolFillColor;
					if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area)&&color==0xF7)//0xF7 -Automatic color
						color=17;// depend on Origin settings - not stored in file
					cl.fillCol=color;
					break;
				default:
					cl.fillCol=-1;
				}

				cl.lWidth = ceil(_curve.lineWidth);
				color=_curve.lineColor;
				cl.lCol=(color==0xF7?0:color); //0xF7 -Automatic color
				int linestyle=_curve.lineStyle;
				cl.filledArea=(_curve.fillArea || style==Graph::VerticalBars || style==Graph::HorizontalBars || style==Graph::Histogram || style == Graph::Pie) ? 1 : 0;
				if(cl.filledArea)
				{
					cl.aStyle = _curve.fillAreaPattern == Origin::NoFill ? 0 : patternStyles[(Origin::FillPattern)_curve.fillAreaPattern];
					color = (cl.aStyle==0 ?_curve.fillAreaColor : _curve.fillAreaPatternColor);
					cl.aCol = (color==0xF7?0:color); //0xF7 -Automatic color
					if(style == Graph::VerticalBars || style == Graph::HorizontalBars || style == Graph::Histogram || style == Graph::Pie)
					{
						color = _curve.fillAreaPatternBorderColor;
						cl.lCol = (color==0xF7?0:color); //0xF7 -Automatic color
						color = (cl.aStyle==0 ? _curve.fillAreaColor : _curve.fillAreaPatternColor);
						cl.aCol = (color==0xF7?cl.lCol:color); //0xF7 -Automatic color
						cl.lWidth = ceil(_curve.fillAreaPatternBorderWidth);
						linestyle = _curve.fillAreaPatternBorderStyle;
					}
				}
				cl.lStyle = lineStyles[(Origin::GraphCurve::LineStyle)linestyle] - 1;

				graph->updateCurveLayout(curve, &cl);
				if (style == Graph::VerticalBars || style == Graph::HorizontalBars)
				{
					QwtBarCurve *b = (QwtBarCurve*)graph->curve(c);
					if (b)
						b->setGap(qRound(100-_curve.symbolSize*10));
				}
				else if(style == Graph::Histogram)
				{
					QwtHistogram *h = (QwtHistogram*)graph->curve(c);
					if (h)
					{
						//vector<double> bin=opj.layerHistogram(g,l);
						//if(bin.size()==3)
							h->setBinning(false, layer.histogramBin, layer.histogramBegin, layer.histogramEnd);
						h->loadData();
					}
				}
				else if(style == Graph::Pie)
				{
					QwtPieCurve *p = (QwtPieCurve*)graph->curve(c);
					cl.lStyle = lineStyles[(Origin::GraphCurve::LineStyle)linestyle];
					p->setPen(QPen(ColorBox::color(cl.lCol), cl.lWidth, (Qt::PenStyle)cl.lStyle));
					p->setFirstColor(_curve.fillAreaFirstColor);
					//geometry
                    p->setRadius(_curve.pie.radius);
                    p->setThickness(_curve.pie.thickness);
					p->setViewAngle(_curve.pie.viewAngle);
					p->setStartAzimuth(_curve.pie.rotation);
					p->setCounterClockwise(_curve.pie.clockwiseRotation);
                    p->setHorizontalOffset(_curve.pie.horizontalOffset);
					//labels
					p->setLabelsEdgeDistance(_curve.pie.distance);
					p->setLabelsAutoFormat(_curve.pie.formatAutomatic);
					p->setLabelPercentagesFormat(_curve.pie.formatPercentages);
                    p->setLabelValuesFormat(_curve.pie.formatValues);
                    p->setLabelCategories(_curve.pie.formatCategories);
                    p->setFixedLabelsPosition(_curve.pie.positionAssociate);
				}
				else if(style == Graph::VectXYXY || style == Graph::VectXYAM)
				{
					graph->updateVectorsLayout(c, ColorBox::color(cl.symCol), ceil(_curve.vector.width),
						floor(_curve.vector.arrowLenght*fVectorArrowScaleFactor + 0.5), _curve.vector.arrowAngle, _curve.vector.arrowClosed, _curve.vector.position);
				}
				switch(_curve.lineConnect)
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
			if(style == Graph::HorizontalBars)
			{
				graph->setScale(0,layer.xAxis.min,layer.xAxis.max,layer.xAxis.step,layer.xAxis.majorTicks,layer.xAxis.minorTicks,layer.xAxis.scale);
				graph->setScale(2,layer.yAxis.min,layer.yAxis.max,layer.yAxis.step,layer.yAxis.majorTicks,layer.yAxis.minorTicks,layer.yAxis.scale);
			}
			else if(style != Graph::Box)
			{

				Origin::GraphAxisBreak breakX = layer.xAxisBreak;
				Origin::GraphAxisBreak breakY = layer.yAxisBreak;
				if(breakX.show)
					graph->setScale(2,layer.xAxis.min,layer.xAxis.max,layer.xAxis.step,layer.xAxis.majorTicks,layer.xAxis.minorTicks,layer.xAxis.scale,
									false,
									breakX.from, breakX.to,
									breakX.position,
									breakX.scaleIncrementBefore, breakX.scaleIncrementAfter,
									breakX.minorTicksBefore, breakX.minorTicksAfter, breakX.log10);
				else
					graph->setScale(2,layer.xAxis.min,layer.xAxis.max,layer.xAxis.step,layer.xAxis.majorTicks,layer.xAxis.minorTicks,layer.xAxis.scale);

				if(breakY.show)
					graph->setScale(0,layer.yAxis.min,layer.yAxis.max,layer.yAxis.step,layer.yAxis.majorTicks,layer.yAxis.minorTicks,layer.xAxis.scale, //??xAxis??
					false,
					breakY.from, breakY.to,
					breakY.position,
					breakY.scaleIncrementBefore, breakY.scaleIncrementAfter,
					breakY.minorTicksBefore, breakY.minorTicksAfter, breakY.log10);
				else
					graph->setScale(0,layer.yAxis.min,layer.yAxis.max,layer.yAxis.step,layer.yAxis.majorTicks,layer.yAxis.minorTicks,layer.yAxis.scale);
			}

			//grid
			Grid *grid = graph->grid();
			grid->enableX(!layer.xAxis.majorGrid.hidden);
			grid->enableXMin(!layer.xAxis.minorGrid.hidden);
			grid->enableY(!layer.yAxis.majorGrid.hidden);
			grid->enableYMin(!layer.yAxis.minorGrid.hidden);

			grid->setMajPenX(QPen(ColorBox::color(layer.xAxis.majorGrid.color), ceil(layer.xAxis.majorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.majorGrid.style]));
			grid->setMinPenX(QPen(ColorBox::color(layer.xAxis.minorGrid.color), ceil(layer.xAxis.minorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.minorGrid.style]));
			grid->setMajPenY(QPen(ColorBox::color(layer.yAxis.majorGrid.color), ceil(layer.yAxis.majorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.yAxis.majorGrid.style]));
			grid->setMinPenY(QPen(ColorBox::color(layer.yAxis.minorGrid.color), ceil(layer.yAxis.minorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.yAxis.minorGrid.style]));

			grid->setAxis(2, 0);
			grid->enableZeroLineX(0);
			grid->enableZeroLineY(0);

			vector<Origin::GraphAxisFormat> formats;
			formats.push_back(layer.yAxis.formatAxis[0]); //bottom
			formats.push_back(layer.yAxis.formatAxis[1]); //top
			formats.push_back(layer.xAxis.formatAxis[0]); //left
			formats.push_back(layer.xAxis.formatAxis[1]); //right

			vector<Origin::GraphAxisTick> ticks;
			ticks.push_back(layer.yAxis.tickAxis[0]); //bottom
			ticks.push_back(layer.yAxis.tickAxis[1]); //top
			ticks.push_back(layer.xAxis.tickAxis[0]); //left
			ticks.push_back(layer.xAxis.tickAxis[1]); //right

			for(int i = 0; i < 4; ++i)
			{
				QString data(ticks[i].dataName.c_str());
				QString tableName=data.right(data.length()-2) + "_" + ticks[i].columnName.c_str();

				QString formatInfo;
				int format = 0;
				int type = 0;
				int prec = ticks[i].decimalPlaces;
				int precisionNeeded = 0;
				if(prec == -1)
				{
					foreach(double value, graph->axisScaleDiv(i)->ticks(QwtScaleDiv::MajorTick))
					{
						QStringList decimals = QString::number(value).split(".");
						if(decimals.size() > 1)
						{
							int p = decimals[1].length();
							if(p > precisionNeeded)
								precisionNeeded = p;
						}
					}
				}
				switch(ticks[i].valueType)
				{
				case Origin::Numeric:
					type = ScaleDraw::Numeric;
					switch(ticks[i].valueTypeSpecification)
					{
					case 0: //Decimal 1000
					case 3: //Decimal 1,000
						format = 1;
						prec = (prec != -1 ? prec : precisionNeeded);
						break;
					case 1: //Scientific
						format=2;
						break;
					case 2: //Engeneering
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
					fontSize = layer.yAxis.label.fontSize;
					break;
				case 2:
				case 3:
					fontSize = layer.xAxis.label.fontSize;
					break;
				}
				if(fontSize > 0)
				{
					fnt.setPointSize(floor(fontSize*fFontScaleFactor + 0.5));
					fnt.setBold(false);
					graph->setAxisTitleFont(i, fnt);
				}

				fnt = graph->axisFont(i);
				fnt.setPointSize(floor(ticks[i].fontSize*fFontScaleFactor + 0.5));
				graph->setAxisFont(i, fnt);
			}

			graph->setAutoscaleFonts(true);

			int nXDelta = graph->width() - graph->canvas()->width();
			int nYDelta = graph->height() - graph->canvas()->height();
			QPoint posCanvas =  graph->canvas()->pos();

			graph->resize(layerRect.width()*fScale + nXDelta, layerRect.height()*fScale + nYDelta);
			graph->updateLayout();
			graph->updateCurveLabels();

			//int newXGraphPos = layerRect.left*fScale - posCanvas.x() - ml->x();
			//int newYGraphPos = layerRect.top*fScale - posCanvas.y() - yOffset - ml->y();
			//graph->move((newXGraphPos > 0 ? newXGraphPos : 0), (newYGraphPos > 0 ? newYGraphPos : 0));
			graph->move(layerRect.left*fScale - posCanvas.x(), layerRect.top*fScale - posCanvas.y() - yOffset);

			if(!layer.legend.text.empty())
			{
				addText(layer.legend, graph, fFontScaleFactor, fScale);
			}
			//add texts
			if(style != Graph::Pie)
			{
				for(unsigned int i = 0; i < layer.texts.size(); ++i)
				{
					addText(layer.texts[i], graph, fFontScaleFactor, fScale);
				}
			}
				
			for(unsigned int i = 0; i < layer.lines.size(); ++i)
			{
				ArrowMarker mrk;
				mrk.setStartPoint(layer.lines[i].begin.x, layer.lines[i].begin.y);
				mrk.setEndPoint(layer.lines[i].end.x, layer.lines[i].end.y);
				mrk.drawStartArrow(layer.lines[i].begin.shapeType > 0);
				mrk.drawEndArrow(layer.lines[i].end.shapeType > 0);
				mrk.setHeadLength(layer.lines[i].end.shapeLength);
                mrk.setHeadAngle(arrowAngle(layer.lines[i].end.shapeLength, layer.lines[i].end.shapeWidth));
				mrk.setColor(ColorBox::color(layer.lines[i].color));
				mrk.setWidth((int)layer.lines[i].width);
				mrk.setStyle(lineStyles[(Origin::GraphCurve::LineStyle)layer.lines[i].style]);
				graph->addArrow(&mrk);
			}

			for(unsigned int i = 0; i < layer.figures.size(); ++i)
			{
				FrameWidget* fw;
				switch(layer.figures[i].type)
				{
				case Origin::Figure::Rectangle:
					fw = new RectangleWidget(graph);
					break;
				case Origin::Figure::Circle:
					fw = new EllipseWidget(graph);
				    break;
				}

				fw->setSize(layer.figures[i].clientRect.width()*fScale, layer.figures[i].clientRect.height()*fScale);
				fw->move(QPoint(layer.figures[i].clientRect.left*fScale, layer.figures[i].clientRect.top*fScale - yOffset));
				fw->setFrameColor(ColorBox::color(layer.figures[i].color));
				fw->setFrameWidth(layer.figures[i].width);
				fw->setFrameLineStyle(lineStyles[(Origin::GraphCurve::LineStyle)layer.figures[i].style]);
				fw->setBackgroundColor(ColorBox::color(layer.figures[i].fillAreaColor));
				fw->setBrush(QBrush(ColorBox::color(layer.figures[i].useBorderColor ? layer.figures[i].color : layer.figures[i].fillAreaPatternColor), PatternBox::brushStyle(patternStyles[(Origin::FillPattern)layer.figures[i].fillAreaPattern])));
				graph->add(fw, false);
			}

			for(unsigned int i = 0; i < layer.bitmaps.size(); ++i)
			{
				QPixmap bmp;
				bmp.loadFromData(layer.bitmaps[i].data, layer.bitmaps[i].size, "BMP");
				QTemporaryFile file;
				file.setFileTemplate(QDir::tempPath() + "/XXXXXX.bmp");
				if(file.open())
				{
					bmp.save(file.fileName(), "BMP");
					ImageWidget* mrk = graph->addImage(file.fileName());
					mrk->setRect(layer.bitmaps[i].clientRect.left*fScale, layer.bitmaps[i].clientRect.top*fScale - yOffset, layer.bitmaps[i].clientRect.width()*fScale, layer.bitmaps[i].clientRect.height()*fScale);
				}
			}
		}
		
		//ml->resize(graphWindowRect.width() - frameWidth, graphWindowRect.height() - frameWidth);
		//cascade the graphs
		if(!_graph.hidden)
		{
			ml->move(QPoint(graphWindowRect.left, graphWindowRect.top));

			switch(_graph.state)
			{
			case Origin::Window::Minimized:
				mw->minimizeWindow(ml);
				break;
			case Origin::Window::Maximized:
				ml->show(); // to correct scaling with maximize
				mw->maximizeWindow(ml);
				break;
			default:
				ml->setScaleLayersOnResize(false);
				ml->show();
				ml->setScaleLayersOnResize(true);
			}
		}
		else
		{
			ml->show();
			//ml->arrangeLayers(true,true);
			mw->hideWindow(ml);
		}
	}

	return true;
}

void ImportOPJ::addText(const Origin::TextBox& text, Graph* graph, double fFontScaleFactor, double fScale)
{
	int bkg;
	switch(text.borderType)
	{
	case Origin::TextBox::BlackLine:
		bkg = 1;
		break;
	case Origin::TextBox::Shadow:
	case Origin::TextBox::DarkMarble:
		bkg = 2;
		break;
	default:
		bkg = 0;
		break;
	}

	LegendWidget* txt=graph->newLegend(parseOriginText(QString::fromLocal8Bit(text.text.c_str())));

	QFont font(mw->plotLegendFont);
	font.setPointSize(floor(text.fontSize*fFontScaleFactor + 0.5));
	txt->setAngle(text.rotation);
	txt->setTextColor(ColorBox::color(text.color));
	txt->setFont(font);
	txt->setFrameStyle(bkg);

	//Origin::Rect txtRect=_text.clientRect;
	//int x=(txtRect.left>layerRect.left ? txtRect.left-layerRect.left : 0);
	//int y=(txtRect.top>layerRect.top ? txtRect.top-layerRect.top : 0);
	//txt->move(QPoint((_text.clientRect.left+_text.clientRect.width()/2)*fScale - txt->width()/2, (_text.clientRect.top+_text.clientRect.height()/2)*fScale - LayerButton::btnSize() - txt->height()/2));
	//txt->setRect(_text.clientRect.left*fScale, _text.clientRect.top*fScale - LayerButton::btnSize(), _text.clientRect.width()*fScale, _text.clientRect.height()*fScale);
	txt->move(QPoint(text.clientRect.left*fScale, text.clientRect.top*fScale - LayerButton::btnSize()));
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
