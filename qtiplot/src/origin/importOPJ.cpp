/***************************************************************************
    File                 : importOPJ.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006-2010 by Ion Vasilief, Alex Kargovsky
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
#include "importOPJ.h"

#include <QApplication>
#include <QRegExp>
#include <QMessageBox>
#include <QDockWidget>
#include <QLocale>
#include <QDate>
#include <QDir>
#include <QTemporaryFile>
#include <Matrix.h>
#include <MatrixModel.h>
#include <ColorBox.h>
#include <PatternBox.h>
#include <Folder.h>
#include <Note.h>
#include <MultiLayer.h>
#include <QwtHistogram.h>
#include <PieCurve.h>
#include <VectorCurve.h>
#include <LegendWidget.h>
#include <Grid.h>
#include <ArrowMarker.h>
#include <ImageWidget.h>
#include <RectangleWidget.h>
#include <EllipseWidget.h>
#include <Spectrogram.h>
#include <ScreenPickerTool.h>
#include <Graph3D.h>
#include <BoxCurve.h>
#include <SymbolBox.h>

#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"

#include <gsl/gsl_math.h>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

#define OBJECTXOFFSET 200

QMap<Origin::GraphCurve::LineStyle, Qt::PenStyle> ImportOPJ::lineStyles;
QMap<Origin::GraphCurve::LineStyle, Qwt3D::LINESTYLE> ImportOPJ::line3DStyles;
QMap<Origin::FillPattern, int> ImportOPJ::patternStyles;
QMap<Origin::ProjectNode::NodeType, QString> ImportOPJ::classes;
QMap<Origin::GraphAxis::Scale, ScaleTransformation::Type> ImportOPJ::scaleTypes;

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
	classes[Origin::ProjectNode::Graph3D] = "Graph3D";
	classes[Origin::ProjectNode::Note] = "Note";

	lineStyles[Origin::GraphCurve::Solid] = Qt::SolidLine;
	lineStyles[Origin::GraphCurve::Dash] = Qt::DashLine;
	lineStyles[Origin::GraphCurve::ShortDash] = Qt::DashLine;
	lineStyles[Origin::GraphCurve::Dot] = Qt::DotLine;
	lineStyles[Origin::GraphCurve::ShortDot] = Qt::DotLine;
	lineStyles[Origin::GraphCurve::DashDot] = Qt::DashDotLine;
	lineStyles[Origin::GraphCurve::ShortDashDot] = Qt::DashDotLine;
	lineStyles[Origin::GraphCurve::DashDotDot] = Qt::DashDotDotLine;

	line3DStyles[Origin::GraphCurve::Solid] = Qwt3D::SOLID;
	line3DStyles[Origin::GraphCurve::Dash] = Qwt3D::DASH;
	line3DStyles[Origin::GraphCurve::ShortDash] = Qwt3D::SHORTDASH;
	line3DStyles[Origin::GraphCurve::Dot] = Qwt3D::DOT;
	line3DStyles[Origin::GraphCurve::ShortDot] = Qwt3D::SHORTDOT;
	line3DStyles[Origin::GraphCurve::DashDot] = Qwt3D::DASHDOT;
	line3DStyles[Origin::GraphCurve::ShortDashDot] = Qwt3D::SHORTDASHDOT;
	line3DStyles[Origin::GraphCurve::DashDotDot] = Qwt3D::DASHDOTDOT;

	scaleTypes[Origin::GraphAxis::Linear] = ScaleTransformation::Linear;
	scaleTypes[Origin::GraphAxis::Log10] = ScaleTransformation::Log10;
	scaleTypes[Origin::GraphAxis::Probability] = ScaleTransformation::Probability;
	scaleTypes[Origin::GraphAxis::Probit] = ScaleTransformation::Probability;
	scaleTypes[Origin::GraphAxis::Reciprocal] = ScaleTransformation::Reciprocal;
	scaleTypes[Origin::GraphAxis::OffsetReciprocal] = ScaleTransformation::Reciprocal;
	scaleTypes[Origin::GraphAxis::Logit] = ScaleTransformation::Logit;
	scaleTypes[Origin::GraphAxis::Ln] = ScaleTransformation::Ln;
	scaleTypes[Origin::GraphAxis::Log2] = ScaleTransformation::Log2;

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
	try {
		OriginFile opj((const char *)filename.toLocal8Bit());
		parse_error = opj.parse();
		importTables(opj);
		importGraphs(opj);
		importNotes(opj);
		if(filename.endsWith(".opj", Qt::CaseInsensitive))
			createProjectTree(opj);
		mw->showResults(opj.resultsLogString().c_str(), mw->logWindow->isVisible());
	} catch(const std::logic_error& er){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(mw, "Origin Project Import Error", QString(er.what()));
	}
}

inline uint qHash(const tree<Origin::ProjectNode>::iterator &key)
{
	return qHash(key->name.c_str());
}

QColor originToQtColor(const Origin::Color& color)
{
	if (color.type == Origin::Color::None)
		return QColor();

	return (color.type == Origin::Color::Regular ? ColorBox::defaultColor(color.regular) : QColor(color.custom[0], color.custom[1], color.custom[2]));
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
		if(sib->type == Origin::ProjectNode::Folder){
			parent[sib] = mw->addFolder(sib->name.c_str(), parent.value(projectTree->parent(sib)));
			parent[sib]->setBirthDate(posixTimeToString(sib->creationDate));
			parent[sib]->setModificationDate(posixTimeToString(sib->modificationDate));
		} else {
			QString name = sib->name.c_str();
			if(sib->type == Origin::ProjectNode::Note){
				QRegExp rx("^@\\((\\S+)\\)$");
				if(rx.indexIn(name) == 0)
					name = rx.cap(1);
			}

			MdiSubWindow* w = projectFolder->window(name, classes[sib->type]);
			if(w){
				Folder *f = parent.value(projectTree->parent(sib));
				if (f){
					f->addWindow(w);
					projectFolder->removeWindow(w);
					f->setActiveWindow(w);
				}
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
	for(unsigned int s = 0; s < opj.spreadCount(); ++s){
		Origin::SpreadSheet spread = opj.spread(s);
		int columnCount = spread.columns.size();
		unsigned int maxrows = spread.maxRows;
		if(!columnCount) //remove tables without cols
			continue;

		Table *table = (spread.hidden || spread.loose) && opj.version() >= 6.0 ? mw->newHiddenTable(spread.name.c_str(), spread.label.c_str(), maxrows, columnCount)
										: mw->newTable(spread.name.c_str(), maxrows, columnCount);
		if (!table)
			return false;

		Origin::Rect windowRect;
		if(opj.version() >= 6.0){
			windowRect = spread.frameRect;
			table->resize(windowRect.width() - (table->frameGeometry().width() - table->width()),
				windowRect.height() - (table->frameGeometry().height() - table->height()));
		}

		table->setCaptionPolicy((MdiSubWindow::CaptionPolicy)spread.title);
		table->setBirthDate(posixTimeToString(spread.creationDate));

        QLocale locale = mw->locale();
		table->setWindowLabel(spread.label.c_str());
		for(int j = 0; j < columnCount; ++j){
			Origin::SpreadColumn column = spread.columns[j];
			QString name(column.name.c_str());
			table->setColName(j, name.replace(QRegExp(".*_"), ""), false, false);
			table->setCommand(j, QString(column.command.c_str()));
			table->setColComment(j, QString(column.comment.c_str()));
			table->setColumnWidth(j, column.width * QtiPlot_scaling_factor);

			switch(column.type){
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

			bool set_text_column = true;
			bool has_texts = false;
			for(unsigned int i = 0; i < column.data.size(); ++i){
				if (i >= maxrows)
					break;

				Origin::variant value = column.data[i];
				if(column.valueType != Origin::Text){// number
					if(value.type() == typeid(string)){//Origin::TextNumeric column should be set to Text
						has_texts = true;
						table->setText(i, j, QString(boost::get<string>(value).c_str()));
					}

					if(value.type() != typeid(double))
						continue;

					double val = boost::get<double>(value);
					if(fabs(val)>0 && fabs(val)<2.0e-300)// empty entry
						continue;

					set_text_column = false;
                    table->setText(i, j, locale.toString(val, 'g', 16));
                    d_cells[j][i] = val;
				} else {// label? doesn't seem to work
					table->setText(i, j, QString(value.type() == typeid(string) ? boost::get<string>(value).c_str() : ""));
				}
			}

            table->saveToMemory(d_cells);

			QString format = formatString(column.valueType, column.valueTypeSpecification);
			switch(column.valueType)
			{
			case Origin::Numeric:
			case Origin::TextNumeric:
			{
				if (set_text_column){
					table->setTextFormat(j);
					break;
				}

				int f = 0;
				if (column.numericDisplayType != 0){
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
				}
				table->setColNumericFormat(f, column.decimalPlaces, j, !has_texts);
				break;
			}
			case Origin::Text:
				table->setTextFormat(j);
				break;
			case Origin::Date:
				table->setDateFormat(format, j);
				break;
			case Origin::Time:
				table->setTimeFormat(formatString(column.valueType, column.valueTypeSpecification + 128), j);
				break;
			case Origin::Month:
			case Origin::Day:
				table->setDayFormat(format, j);
				break;
			default:
				break;
			}
            table->freeMemory();
		}

		if(!(spread.hidden || spread.loose) || opj.version() != 7.5){
			switch(spread.state){
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
			if (opj.version() >= 6.0)
				table->move(QPoint(windowRect.left, windowRect.top));
			else {
				int dx = table->verticalHeaderWidth();
				int dy = table->frameGeometry().height() - table->height();
				table->move(QPoint(visible_count*dx + xoffset*OBJECTXOFFSET, visible_count*dy));
				++visible_count;
			}
		}
	}

//Import matrices
	for(unsigned int s = 0; s < opj.matrixCount(); ++s){
		Origin::Matrix matrix = opj.matrix(s);
		int columnCount = matrix.columnCount;
		int rowCount = matrix.rowCount;

		Matrix* Matrix = mw->newMatrix(matrix.name.c_str(), rowCount, columnCount);
		if (!Matrix)
			return false;

		Origin::Rect windowRect;
		if(opj.version() >= 6.0){
			windowRect = matrix.frameRect;
			Matrix->resize(windowRect.width() - (Matrix->frameGeometry().width() - Matrix->width()),
				windowRect.height() - (Matrix->frameGeometry().height() - Matrix->height()));
		}

		Matrix->setCoordinates(matrix.coordinates[3], matrix.coordinates[1], matrix.coordinates[2], matrix.coordinates[0]);
		Matrix->setCaptionPolicy((MdiSubWindow::CaptionPolicy)matrix.title);
		Matrix->setBirthDate(posixTimeToString(matrix.creationDate));

		Matrix->setWindowLabel(matrix.label.c_str());
		Matrix->setFormula(matrix.command.c_str());
		Matrix->setColumnsWidth(matrix.width * QtiPlot_scaling_factor);
		if(matrix.view == Origin::Matrix::ImageView){
			Matrix->setViewType(Matrix::ImageView);
			if(opj.version() > 7.5)
				Matrix->setGrayScale();
			else {
				Origin::ColorMap colorMap = matrix.colorMap;
				colorMap.levels.pop_back();
				Matrix->setColorMap(qwtColorMap(Matrix, colorMap));
			}
		}

		if(matrix.header == Origin::Matrix::XY)
			Matrix->setHeaderViewType(Matrix::XY);

		vector<double>* data = &matrix.data;
		double* matrix_data = Matrix->matrixModel()->dataVector();
		int size = Matrix->numRows()*Matrix->numCols();
		for(int i = 0; i < size; ++i){
			double val = data->at(i);
			double fval = fabs(val);
			if (!(fval < 2.0e-300 && fval > 0))
				matrix_data[i] = val;
		}

		QChar format;
		int prec = 6;
		switch(matrix.valueTypeSpecification){
			case 0: //Decimal 1000
				format='f';
				prec = matrix.decimalPlaces;
				break;
			case 1: //Scientific
				format='e';
				prec = matrix.decimalPlaces;
				break;
			case 2: //Engineering
			case 3: //Decimal 1,000
				format='g';
				prec = matrix.significantDigits;
				break;
		}
		Matrix->setNumericFormat(format, prec);

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
			if(opj.version() >= 6.0)
				Matrix->move(QPoint(windowRect.left, windowRect.top));
			else {
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
	for(unsigned int n = 0; n < opj.noteCount(); ++n){
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
		note->setText(QString(_note.text.c_str()));
		note->setCaptionPolicy((MdiSubWindow::CaptionPolicy)_note.title);
		note->setBirthDate(posixTimeToString(_note.creationDate));

		Origin::Rect windowRect;
		windowRect = _note.frameRect;
		note->resize(windowRect.width() - (note->frameGeometry().width() - note->width()),
			windowRect.height() - (note->frameGeometry().height() - note->height()));

		note->move(QPoint(windowRect.left, windowRect.top));

		switch(_note.state){
			case Origin::Window::Minimized:
				mw->minimizeWindow(note);
				break;
			case Origin::Window::Maximized:
				mw->maximizeWindow(note);
				break;
			default:
				note->showNormal();
		}

		if(_note.hidden)
			mw->hideWindow(note);
	}

	return true;
}

bool ImportOPJ::importGraphs(const OriginFile& opj)
{
	double pi = M_PI;
	int tickTypeMap[]={0,3,1,2};

	Origin::Rect maximazedFrame, standardFrame;
	if (opj.graphCount()){
		MultiLayer* fake = mw->multilayerPlot("fake", 0);
		fake->setParent(0);
		frameWidth = fake->frameGeometry().width() - fake->geometry().width();
		frameHeight = fake->frameGeometry().height() - fake->geometry().height();
		standardFrame = Origin::Rect(fake->geometry().width(), fake->geometry().height());
		fake->setMaximized();
		maximazedFrame = Origin::Rect(fake->geometry().width(), fake->geometry().height());
		fake->askOnCloseEvent(false);
		fake->close();
	}

	for(unsigned int g = 0; g < opj.graphCount(); ++g){
		Origin::Graph _graph = opj.graph(g);
		MultiLayer *ml = mw->multilayerPlot(_graph.name.c_str(), 0);
		if (!ml)
			continue;

		ml->setCaptionPolicy((MdiSubWindow::CaptionPolicy)_graph.title);
		ml->setBirthDate(posixTimeToString(_graph.creationDate));
		ml->setWindowLabel(_graph.label.c_str());

		Origin::Rect graphRect(_graph.width, _graph.height);
		Origin::Rect graphWindowRect = _graph.frameRect.isValid() ? _graph.frameRect : (_graph.state == Origin::Window::Maximized ? maximazedFrame : standardFrame);
		double ratio = (double)(graphWindowRect.width() - frameWidth)/(double)(graphWindowRect.height() - frameHeight);

		int width = _graph.width;
		int height = _graph.height;
		if((double)(_graph.width)/(double)(_graph.height) < ratio)
			width = height * ratio;
		else
			height = width / ratio;


		int yOffset = LayerButton::btnSize();

		ml->resize(graphWindowRect.width(), graphWindowRect.height() + 3*yOffset);

		double fScale = (double)(graphWindowRect.width() - frameWidth)/(double)width;
		double fWindowFactor =  QMIN((double)graphWindowRect.width()/500.0, (double)graphWindowRect.height()/350.0);
		double fFontScaleFactor = 0.4;
		double fVectorArrowScaleFactor = 0.08*fWindowFactor;

		bool imageProfileTool = false;
		bool boxWhiskersPlot = false;
		bool showColorScale = false;
		unsigned int layers = _graph.layers.size();
		bool doubleAxesLayout = false;
		for(unsigned int l = 0; l < layers; ++l){
			Origin::GraphLayer& layer = _graph.layers[l];

			if(layer.is3D() || layer.isXYY3D){
				importGraph3D(opj, g, l);
				continue;
			}

			Graph *graph = ml->addLayer();
			if(!graph)
				continue;

			if (opj.version() >= 8.0)
				graph->setAxisTitlePolicy(Graph::ColName);
			else
				graph->setAxisTitlePolicy(mw->d_graph_axis_labeling);
			graph->setAutoscaleFonts(false);

			Origin::Rect layerRect = layer.clientRect;
			if (l){
				Origin::GraphLayer& prevLayer = _graph.layers[l - 1];
				if (layers == 2 && (prevLayer.clientRect.left == layerRect.left &&
					prevLayer.clientRect.top == layerRect.top &&
					prevLayer.clientRect.right == layerRect.right &&
					prevLayer.clientRect.bottom == layerRect.bottom))
					doubleAxesLayout = true;
			}

			QColor bkg = originToQtColor(layer.backgroundColor);
			if(layer.backgroundColor.type == Origin::Color::None)
				bkg.setAlpha(0);
			graph->setCanvasBackground(bkg);

			int style = 0;
			bool matrixImage = false;
			bool horizontalBars = false;
			Origin::GraphCurve XYZContourCurve;
			Table *XYZContourTable = 0;

			for(unsigned int c = 0; c < layer.curves.size(); ++c){
				Origin::GraphCurve& _curve = layer.curves[c];
				try {
				QString data(_curve.dataName.c_str());
				int color = 0;
				switch(_curve.type){
					case Origin::GraphCurve::Line:
						style = Graph::Line;
						break;
					case Origin::GraphCurve::Scatter:
						style = Graph::Scatter;
						break;
					case Origin::GraphCurve::LineSymbol:
						style = Graph::LineSymbols;
						break;
					case Origin::GraphCurve::ErrorBar:
					case Origin::GraphCurve::XErrorBar:
						style = Graph::ErrorBars;
						break;
					case Origin::GraphCurve::Column:
						if (_graph.templateName == "BAR"){
							style = Graph::HorizontalBars;
							horizontalBars = true;
						} else
							style = Graph::VerticalBars;
						break;
					case Origin::GraphCurve::Bar:
						style = Graph::HorizontalBars;
						horizontalBars = true;
						break;
					case Origin::GraphCurve::Histogram:
						style = Graph::Histogram;
						break;
					case Origin::GraphCurve::Pie:
						style = Graph::Pie;
						break;
					case Origin::GraphCurve::Box:
						style = Graph::Box;
						break;
					case Origin::GraphCurve::FlowVector:
						style = Graph::VectXYXY;
						break;
					case Origin::GraphCurve::Vector:
						style = Graph::VectXYAM;
						break;
					case Origin::GraphCurve::Area:
					case Origin::GraphCurve::AreaStack:
						style = Graph::Area;
						break;
					case Origin::GraphCurve::TextPlot:
						style = Origin::GraphCurve::TextPlot;
						break;
					case Origin::GraphCurve::XYZContour:
					case Origin::GraphCurve::Contour:
						style = Origin::GraphCurve::Contour;
						break;
					case Origin::GraphCurve::MatrixImage:
						style = Origin::GraphCurve::MatrixImage;
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

				switch(data[0].toAscii()){
				case 'T':{
					tableName = data.right(data.length() - 2).replace ("_", "-");
					Table* table = mw->table(tableName);
					if (!table)
						break;

					if(style==Graph::ErrorBars){
						int flags=_curve.symbolType;
						curve = (PlotCurve*)graph->addErrorBars(QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), table, QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()),
							((flags&0x10)==0x10?0:1), ceil(_curve.lineWidth), ceil(_curve.symbolSize), QColor(Qt::black),
							(flags&0x40)==0x40, (flags&2)==2, (flags&1)==1);
					} else if(style==Graph::Histogram)
						curve = (PlotCurve*)graph->insertCurve(table, QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);
					else if(style==Graph::Pie || style==Graph::Box){
						QStringList names;
						names << QString("%1_%2").arg(tableName, _curve.yColumnName.c_str());
						graph->addCurves(mw->table(tableName), names, style);

						if(style == Graph::Box){
							curve = graph->curve(c);
							((BoxCurve *)curve)->setBoxWidth(_curve.boxWidth);
							boxWhiskersPlot = true;
						}
					} else if(style==Graph::VectXYXY){
						QStringList names;
						Origin::VectorProperties vector = _curve.vector;
						names << QString("%1_%2").arg(tableName, _curve.xColumnName.c_str())
							<< QString("%1_%2").arg(tableName, _curve.yColumnName.c_str())
							<< (tableName + "_" + QString(vector.endXColumnName.c_str()))
							<< (tableName + "_" + QString(vector.endYColumnName.c_str()));

						graph->addCurves(table, names, style);
					} else if(style==Graph::VectXYAM){
						QStringList names;
						Origin::VectorProperties vector = _curve.vector;
						names << QString("%1_%2").arg(tableName, _curve.xColumnName.c_str())
							<< QString("%1_%2").arg(tableName, _curve.yColumnName.c_str())
							<< (tableName + "_" + QString(vector.angleColumnName.c_str()))
							<< (tableName + "_" + QString(vector.magnitudeColumnName.c_str()));

						graph->addCurves(table, names, style);
					} else if(style == Origin::GraphCurve::TextPlot){
						QString labelsCol(_curve.yColumnName.c_str());
						int xcol = table->colX(table->colIndex(labelsCol));
						int ycol = table->colY(table->colIndex(labelsCol));
						if (xcol < 0 || ycol < 0)
							return false;

						DataCurve* mc = graph->masterCurve(table->colName(xcol), table->colName(ycol));
						if(mc){
							graph->replot();
							mc->setLabelsColumnName(labelsCol);
							setCurveLabelsProperties(mc, _curve, fFontScaleFactor);
						}
					} else if (style == Origin::GraphCurve::Contour){
						Matrix* m = mw->tableToMatrixRegularXYZ(table, tableName + "_" + _curve.zColumnName.c_str());
						if (!m)
							break;
						m->setHidden();

						curve = (PlotCurve*)graph->plotSpectrogram(m, Graph::ColorMap);
						_curve.colorMap.levels.pop_back();
						importSpectrogram(graph, (Spectrogram*)curve, layer, _curve, fFontScaleFactor);

						XYZContourCurve = _curve;
						XYZContourTable = table;

						if (layers > 1 && (l + 1 < layers)){//if multilayers only last layer has color scale bar enabled
							QwtScaleWidget *scale = graph->axisWidget(QwtPlot::yRight);
							if (scale)
								scale->setColorBarEnabled(false);
						} else
							showColorScale = true;
					} else
						curve = (PlotCurve *)graph->insertCurve(table, QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);

					break;
				}

				case 'M':{
					QString matrixName = data.right(data.length()-2);
					Matrix* m = mw->matrix(matrixName);
					if (!m)
						break;

					if(_curve.type == Origin::GraphCurve::Contour){
						showColorScale = true;
						curve = (PlotCurve*)graph->plotSpectrogram(m, Graph::ColorMap);
						importSpectrogram(graph, (Spectrogram*)curve, layer, _curve, fFontScaleFactor);
					} else if(style == Origin::GraphCurve::MatrixImage){
						Spectrogram* sp = graph->plotSpectrogram(m, Graph::GrayScale);
						if (!sp)
							break;
						sp->setGrayScale();
						matrixImage = true;
					} else if (style == Graph::Histogram)
						curve = (PlotCurve*)graph->addHistogram(m);

					break;
				}

				case 'F':
					s = opj.functionIndex(data.right(data.length()-2).toStdString());
					function = opj.function(s);

					int type;
					if(function.type == Origin::Function::Polar){
						type = 2;
						formulas << function.formula.c_str() << "x";
						start = pi/180 * function.begin;
						end = pi/180 * function.end;
					} else {
						type = 0;
						formulas << function.formula.c_str();
						start = function.begin;
						end = function.end;
					}
					curve = (PlotCurve*)graph->addFunction(formulas, start, end, function.totalPoints, "x", type, function.name.c_str());
					graph->setCanvasFrame(1, Qt::black);
					mw->updateFunctionLists(type, formulas);
					break;
				default:
					continue;
				}

				CurveLayout cl = graph->initCurveLayout(style, layer.curves.size());

				color=_curve.lineColor.regular;
				cl.lCol = ColorBox::defaultColor(_curve.lineColor.type==Origin::Color::Automatic?0:color); //0xF7 -Automatic color

				cl.sSize = ceil(_curve.symbolSize*0.5);
				cl.penWidth=_curve.symbolThickness;
				color=_curve.symbolColor.regular;

				cl.symCol = ColorBox::defaultColor(_curve.symbolColor.regular);
				if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area || style==Graph::Box)&&_curve.symbolColor.type == Origin::Color::Automatic)//0xF7 -Automatic color
					cl.symCol = cl.lCol;

				switch(_curve.symbolType&0xFF){
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

				switch(_curve.symbolType>>8){
					case 0:
						cl.fillCol = ColorBox::defaultColor(_curve.symbolFillColor.regular);
						if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area || style==Graph::Box)&&
						   (_curve.symbolFillColor.type == Origin::Color::Automatic || _curve.symbolFillColor.type == Origin::Color::None))
							cl.fillCol = cl.symCol;
							break;
					case 1:
					case 2:
					case 8:
					case 9:
					case 10:
					case 11:
						color =_curve.symbolFillColor.regular;
						if((style==Graph::Scatter || style==Graph::LineSymbols || style==Graph::Area || style==Graph::Box)&&_curve.symbolFillColor.type==Origin::Color::Automatic)//0xF7 -Automatic color
							color=17;// depend on Origin settings - not stored in file
						cl.fillCol = ColorBox::defaultColor(color);
						if (_curve.symbolFillColor.type==Origin::Color::None)
							cl.fillCol = QColor();
						break;
					default:
						cl.fillCol = QColor();
				}

				cl.lWidth = _curve.lineWidth;
				int linestyle=_curve.lineStyle;
				cl.filledArea=(_curve.fillArea || style==Graph::VerticalBars || style==Graph::HorizontalBars || style==Graph::Histogram || style == Graph::Pie || style == Graph::Box) ? 1 : 0;
				if(cl.filledArea){
					Origin::Color color;
					cl.aStyle = _curve.fillAreaPattern == Origin::NoFill ? 0 : patternStyles[(Origin::FillPattern)_curve.fillAreaPattern];
					color = (cl.aStyle == 0 ? _curve.fillAreaColor : _curve.fillAreaPatternColor);
					cl.aCol = (color.type == Origin::Color::Automatic ? 0 : color.regular); //0xF7 -Automatic color
					if(style == Graph::VerticalBars || style == Graph::HorizontalBars || style == Graph::Histogram || style == Graph::Pie || style == Graph::Box){
						color = _curve.fillAreaPatternBorderColor;
						cl.lCol = ColorBox::defaultColor(color.type == Origin::Color::Automatic ? 0 : color.regular); //0xF7 -Automatic color
						color = (cl.aStyle == 0 ? _curve.fillAreaColor : _curve.fillAreaPatternColor);
						if (!originToQtColor(_curve.fillAreaColor).isValid() && !cl.aStyle)
							cl.filledArea = 0;

						cl.aCol = (color.type == Origin::Color::Automatic ? cl.lCol : ColorBox::defaultColor(color.regular)); //0xF7 -Automatic color
						cl.lWidth = _curve.fillAreaPatternBorderWidth;
						linestyle = _curve.fillAreaPatternBorderStyle;
					}
				}
				cl.lStyle = lineStyles[(Origin::GraphCurve::LineStyle)linestyle] - 1;

				if(style != Origin::GraphCurve::Contour)
					graph->updateCurveLayout(curve, &cl);

				if(style == Graph::VerticalBars || style == Graph::HorizontalBars){
					QwtBarCurve *b = (QwtBarCurve*)graph->curve(c);
					if (b)
						b->setGap(qRound(100 -_curve.symbolSize*10));
				} else if(style == Graph::Histogram){
					QwtHistogram *h = (QwtHistogram*)graph->curve(c);
					if(h){
						h->setBinning(false, layer.histogramBin, layer.histogramBegin, layer.histogramEnd);
						h->loadData();
					}
				} else if(style == Graph::Pie){
					PieCurve *p = (PieCurve *)graph->curve(c);
					cl.lStyle = lineStyles[(Origin::GraphCurve::LineStyle)linestyle];
					p->setPen(QPen(cl.lCol, cl.lWidth, (Qt::PenStyle)cl.lStyle));
					p->setBrushStyle(PatternBox::brushStyle(cl.aStyle));
					if(_curve.fillAreaColor.type == Origin::Color::Increment)
						p->setFirstColor(_curve.fillAreaColor.starting);
					//geometry
					p->setRadius(_curve.pie.radius);
					p->setThickness(_curve.pie.thickness);
					p->setViewAngle(_curve.pie.viewAngle);
					p->setStartAzimuth(_curve.pie.rotation);
					p->setCounterClockwise(_curve.pie.clockwiseRotation);
					p->setHorizontalOffset(_curve.pie.horizontalOffset);
					//labels
					p->setLabelsEdgeDistance(_curve.pie.distance);
					p->setLabelsAutoFormat(false);
					p->setLabelPercentagesFormat(_curve.pie.formatPercentages);
					p->setLabelValuesFormat(_curve.pie.formatValues);
					p->setLabelCategories(_curve.pie.formatCategories);
					p->setFixedLabelsPosition(_curve.pie.positionAssociate);

					graph->setFrame(0);
				} else if(style == Graph::VectXYXY || style == Graph::VectXYAM){
					graph->updateVectorsLayout(c, cl.symCol, _curve.vector.width,
						floor(_curve.vector.arrowLenght*fVectorArrowScaleFactor + 0.5), _curve.vector.arrowAngle, _curve.vector.arrowClosed, _curve.vector.position);
				}

				switch(_curve.lineConnect){
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
				{}
			}

			if (horizontalBars){
				Origin::GraphAxis xAxis = layer.xAxis;
				Origin::GraphAxis yAxis = layer.yAxis;
				layer.xAxis = yAxis;
				layer.yAxis = xAxis;
			}

			//grid
			Grid *grid = graph->grid();
			grid->enableX(!layer.xAxis.majorGrid.hidden);
			grid->enableXMin(!layer.xAxis.minorGrid.hidden);
			grid->enableY(!layer.yAxis.majorGrid.hidden);
			grid->enableYMin(!layer.yAxis.minorGrid.hidden);

			grid->setMajPenX(QPen(ColorBox::defaultColor(layer.xAxis.majorGrid.color), ceil(layer.xAxis.majorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.majorGrid.style]));
			grid->setMinPenX(QPen(ColorBox::defaultColor(layer.xAxis.minorGrid.color), ceil(layer.xAxis.minorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.minorGrid.style]));
			grid->setMajPenY(QPen(ColorBox::defaultColor(layer.yAxis.majorGrid.color), ceil(layer.yAxis.majorGrid.width),
							lineStyles[(Origin::GraphCurve::LineStyle)layer.yAxis.majorGrid.style]));
			grid->setMinPenY(QPen(ColorBox::defaultColor(layer.yAxis.minorGrid.color), ceil(layer.yAxis.minorGrid.width),
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

			if (matrixImage){
				formats.clear();
				formats.push_back(layer.yAxis.formatAxis[0]); //bottom
				formats.push_back(layer.yAxis.formatAxis[1]); //top
				formats.push_back(layer.xAxis.formatAxis[1]); //left
				formats.push_back(layer.xAxis.formatAxis[0]); //right

				ticks.clear();
				ticks.push_back(layer.yAxis.tickAxis[0]); //bottom
				ticks.push_back(layer.yAxis.tickAxis[1]); //top
				ticks.push_back(layer.xAxis.tickAxis[1]); //left
				ticks.push_back(layer.xAxis.tickAxis[0]); //right
			}

			for(int i = 0; i < QwtPlot::axisCnt; ++i){
				QString data(ticks[i].dataName.c_str());
				QString tableName = data.right(data.length()-2) + "_" + ticks[i].columnName.c_str();

				QString formatInfo = tableName;
				int format = ScaleDraw::Automatic;
				int type = 0;
				int prec = ticks[i].decimalPlaces;
				int precisionNeeded = 0;
				if(prec == -1){
					foreach(double value, graph->axisScaleDiv(i)->ticks(QwtScaleDiv::MajorTick)){
						QStringList decimals = QString::number(value).split(".");
						if(decimals.size() > 1){
							int p = decimals[1].length();
							if(p > precisionNeeded)
								precisionNeeded = p;
						}
					}
				}

				switch(ticks[i].valueType){
					case Origin::Numeric:
						type = ScaleDraw::Numeric;
						switch(ticks[i].valueTypeSpecification){
							case 0: //Decimal 1000 (Automatic)
								if (prec == -1)
									prec = 6;
								break;
							case 3: //Decimal 1,000
								format = ScaleDraw::Decimal;
								prec = (prec != -1 ? prec : precisionNeeded);
								break;
							case 1: //Scientific
								if (prec == -1)
									prec = 1;
								format = ScaleDraw::Superscripts;
								break;
							case 2: //Engineering
								if (prec == -1)
									prec = 1;
								format = ScaleDraw::Engineering;
								break;
						}
						break;
					case Origin::Text:
						type=ScaleDraw::Text;
						break;
					case Origin::Date:
						type = ScaleDraw::Date;
						formatInfo = ";" + formatString(ticks[i].valueType, ticks[i].valueTypeSpecification);
						break;
					case Origin::Time:
						type = ScaleDraw::Time;
						formatInfo = ";" + formatString(ticks[i].valueType, ticks[i].valueTypeSpecification);
						break;
					case Origin::Month:
						type=ScaleDraw::Month;
						format=ticks[i].valueTypeSpecification;
						break;
					case Origin::Day:
						type=ScaleDraw::Day;
						format=ticks[i].valueTypeSpecification;
						break;
					case Origin::ColumnHeading:
						type=ScaleDraw::ColHeader;
						switch(ticks[i].valueTypeSpecification){
							case 0: //Decimal 1000
								format=1;
								break;
							case 1: //Scientific
								format=2;
								break;
							case 2: //Engineering
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

				if (i == QwtPlot::yRight && showColorScale){
					QwtScaleWidget *scale = graph->axisWidget(i);
					if (scale){
						scale->setColorBarEnabled(true);
						double barWidth = qRound(layer.colorScale.colorBarThickness*0.01*scale->font().pointSize());
						if (barWidth < 200)
							scale->setColorBarWidth(barWidth);

						ScaleDraw *sd = (ScaleDraw *)scale->scaleDraw();
						if (sd){
							sd->enableComponent(QwtAbstractScaleDraw::Labels);
							sd->enableComponent(QwtAbstractScaleDraw::Backbone, false);
							sd->enableComponent(QwtAbstractScaleDraw::Ticks, false);
							sd->setSpacing(qRound(layer.colorScale.labelGap*0.01*scale->font().pointSize()));
							if (opj.version() >= 7.0)
								sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);
						}
						graph->setAxisLabelsColor(i, originToQtColor(layer.colorScale.labelsColor));
						if (!layer.colorScale.reverseOrder)
							graph->axisScaleEngine(i)->setAttribute(QwtScaleEngine::Inverted, true);
					}
				} else {
					QString formula = "";
					QString factor = QString(formats[i].factor.c_str()).replace(",", ".");
					double f = factor.toDouble();
					if (!factor.isEmpty() && f != 0.0){
						if (i == QwtPlot::xBottom || i == QwtPlot::xTop)
							formula = "x/";
						else
							formula = "y/";
						QLocale locale = mw->locale();
						locale.setNumberOptions(QLocale::OmitGroupSeparator);
						formula += locale.toString(f);
					}

					graph->showAxis(i, type, formatInfo, mw->table(tableName), !(formats[i].hidden),
						tickTypeMap[formats[i].majorTicksType], tickTypeMap[formats[i].minorTicksType],
						!(ticks[i].hidden),	ColorBox::defaultColor(formats[i].color), format, prec,
						-ticks[i].rotation, 0, formula, (ticks[i].color == 0xF7 ? ColorBox::defaultColor(formats[i].color) : ColorBox::defaultColor(ticks[i].color)),
						4, true, ScaleDraw::ShowAll, parseOriginText(QString(formats[i].prefix.c_str())),
						parseOriginText(QString(formats[i].suffix.c_str())));
				}

				QwtScaleWidget *scale = graph->axisWidget(i);
				if (scale)
					scale->setPenWidth((int)formats[i].thickness);

				QFont fnt = graph->axisTitleFont(i);
				int fontSize = formats[i].label.fontSize;
				if (fontSize > 0){
					fnt.setPointSizeF(fontSize*fFontScaleFactor);
					fnt.setBold(false);
					graph->setAxisTitleFont(i, fnt);
				}

				graph->setAxisTitle(i, parseOriginText(QString::fromLocal8Bit(formats[i].label.text.c_str())));
				graph->setAxisTitleColor(i, originToQtColor(formats[i].label.color));

				fnt = graph->axisFont(i);
				fnt.setPointSizeF(ticks[i].fontSize*fFontScaleFactor);
				graph->setAxisFont(i, fnt);

				if (_graph.isLayout)
					graph->enableAxis(i, false);
			}

			//set scale limits
			Origin::GraphAxisBreak breakX = layer.xAxisBreak;
			Origin::GraphAxisBreak breakY = layer.yAxisBreak;
			bool invert = (layer.xAxis.min > layer.xAxis.max);
			if (style != Graph::Box){
				if (breakX.show){
					for (int i = 2; i < QwtPlot::axisCnt; i++)
						graph->setScale(i, layer.xAxis.min,layer.xAxis.max,layer.xAxis.step,layer.xAxis.majorTicks,layer.xAxis.minorTicks,scaleTypes[(Origin::GraphAxis::Scale)layer.xAxis.scale],
								invert, breakX.from, breakX.to, breakX.position, breakX.scaleIncrementBefore, breakX.scaleIncrementAfter,
								breakX.minorTicksBefore, breakX.minorTicksAfter, breakX.log10);
				} else {
					for (int i = 2; i < QwtPlot::axisCnt; i++){
						if (graph->axisWidget(i)->isColorBarEnabled())
							continue;
						ScaleDraw *sd = (ScaleDraw *)graph->axisScaleDraw(i);
						if (sd->scaleType() == ScaleDraw::Date){
							double start = layer.xAxis.min;
							double step = layer.xAxis.step*8.64e4;
							QDateTime startDate = Table::dateTime(start);
							sd->setDateTimeOrigin(startDate);
							graph->setAxisScale(i, 0, startDate.secsTo(Table::dateTime(layer.xAxis.max)), step);
							graph->setAxisStep(i, step);
							graph->reloadCurvesData();
							continue;
						}
						graph->setScale(i, layer.xAxis.min,layer.xAxis.max,layer.xAxis.step,layer.xAxis.majorTicks,layer.xAxis.minorTicks,scaleTypes[(Origin::GraphAxis::Scale)layer.xAxis.scale], invert);
					}
				}
			}

			invert = (layer.yAxis.min > layer.yAxis.max) || matrixImage;
			if (breakY.show){
				for (int i = 0; i <= 1; i++)
					graph->setScale(i, layer.yAxis.min,layer.yAxis.max,layer.yAxis.step,layer.yAxis.majorTicks,layer.yAxis.minorTicks,scaleTypes[(Origin::GraphAxis::Scale)layer.xAxis.scale], //??xAxis??
						invert, breakY.from, breakY.to, breakY.position, breakY.scaleIncrementBefore, breakY.scaleIncrementAfter,
						breakY.minorTicksBefore, breakY.minorTicksAfter, breakY.log10);
			} else {
				for (int i = 0; i <= 1; i++){
					if (!graph->axisWidget(i)->isColorBarEnabled())
						graph->setScale(i, layer.yAxis.min,layer.yAxis.max,layer.yAxis.step,layer.yAxis.majorTicks,layer.yAxis.minorTicks,scaleTypes[(Origin::GraphAxis::Scale)layer.yAxis.scale], invert);
				}
			}

			if (XYZContourTable)
				this->parseXYZContourPlotAxisTitles(graph, XYZContourTable, XYZContourCurve);

			int cfw = graph->canvasFrameWidth();
			if (cfw){
				for(int i = 0; i < QwtPlot::axisCnt; ++i){
					QwtScaleWidget *scale = graph->axisWidget(i);
					if (scale)
						scale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, (scale->penWidth() - cfw > 0));
				}
			}

			graph->setCanvasGeometry(QRect(layerRect.left*fScale, layerRect.top*fScale + yOffset,
										   layerRect.width()*fScale, layerRect.height()*fScale));
			graph->updateLayout();
			graph->updateCurveLabels();

			if(!layer.legend.text.empty())
				addText(layer.legend, graph, fFontScaleFactor, fScale);

			//add texts
			for(unsigned int i = 0; i < layer.texts.size(); ++i)
				addText(layer.texts[i], graph, fFontScaleFactor, fScale);

			if (style == Graph::Pie)
				setPieTexts((PieCurve *)graph->curve(0), graph, layer, fFontScaleFactor, fScale);

			for(unsigned int i = 0; i < layer.lines.size(); ++i){
				ArrowMarker mrk;
				//if (layer.lines[i].attach == Origin::Page || style == Graph::Pie || ){
					mrk.setAttachPolicy(ArrowMarker::Page);

					QPoint pos = graph->canvas()->mapFrom(ml->canvas(), QPoint(layer.lines[i].clientRect.left*fScale + 2*layer.lines[i].width, layer.lines[i].clientRect.top*fScale + yOffset));
					mrk.setEndPoint(graph->invTransform(QwtPlot::xBottom, pos.x()), graph->invTransform(QwtPlot::yLeft, pos.y()));
					pos = graph->canvas()->mapFrom(ml->canvas(), QPoint(layer.lines[i].clientRect.right*fScale, layer.lines[i].clientRect.bottom*fScale + yOffset));
					mrk.setStartPoint(graph->invTransform(QwtPlot::xBottom, pos.x()), graph->invTransform(QwtPlot::yLeft, pos.y()));
				/*} else {
					mrk.setStartPoint(layer.lines[i].begin.x, layer.lines[i].begin.y);
					mrk.setEndPoint(layer.lines[i].end.x, layer.lines[i].end.y);
				}*/

				mrk.drawStartArrow(layer.lines[i].begin.shapeType > 0);
				mrk.drawEndArrow(layer.lines[i].end.shapeType > 0);
				mrk.setHeadLength(layer.lines[i].end.shapeLength);
                mrk.setHeadAngle(arrowAngle(layer.lines[i].end.shapeLength, layer.lines[i].end.shapeWidth));
				mrk.setColor(originToQtColor(layer.lines[i].color));
				mrk.setWidth(layer.lines[i].width);
				mrk.setStyle(lineStyles[(Origin::GraphCurve::LineStyle)layer.lines[i].style]);
				graph->addArrow(&mrk);
			}

			for(unsigned int i = 0; i < layer.figures.size(); ++i){
				FrameWidget* fw = 0;
				switch(layer.figures[i].type){
					case Origin::Figure::Rectangle:
						fw = new RectangleWidget(graph);
						break;
					case Origin::Figure::Circle:
						fw = new EllipseWidget(graph);
						break;
				}

				double lw = layer.figures[i].width;
				fw->setSize(layer.figures[i].clientRect.width()*fScale + 2*lw, layer.figures[i].clientRect.height()*fScale + 2*lw);
				fw->move(QPoint(layer.figures[i].clientRect.left*fScale, layer.figures[i].clientRect.top*fScale + yOffset));
				fw->setFrameColor(originToQtColor(layer.figures[i].color));
				fw->setFrameWidth(lw);
				fw->setFrameLineStyle(lineStyles[(Origin::GraphCurve::LineStyle)layer.figures[i].style]);

				QColor bkg = originToQtColor(layer.figures[i].fillAreaColor);
				if (!bkg.isValid())
					bkg.setAlpha(0);
				fw->setBackgroundColor(bkg);
				fw->setBrush(QBrush(originToQtColor(layer.figures[i].useBorderColor ? layer.figures[i].color : layer.figures[i].fillAreaPatternColor), PatternBox::brushStyle(patternStyles[(Origin::FillPattern)layer.figures[i].fillAreaPattern])));
				graph->add(fw, false);
			}

			for(unsigned int i = 0; i < layer.bitmaps.size(); ++i){
				QPixmap bmp;
				QString windowName = QString::null;
				if (layer.bitmaps[i].size > 0)
					bmp.loadFromData(layer.bitmaps[i].data, layer.bitmaps[i].size, "BMP");
				else {
					windowName = QString(layer.bitmaps[i].windowName.c_str());
					bmp = ImageWidget::windowPixmap(mw, windowName, QSize(layer.bitmaps[i].clientRect.width()*fScale, layer.bitmaps[i].clientRect.height()*fScale));
				}

				QTemporaryFile file;
				file.setFileTemplate(QDir::tempPath() + "/XXXXXX.bmp");
				if(file.open()){
					bmp.save(file.fileName(), "BMP");
					ImageWidget* img = graph->addImage(file.fileName());
					img->setSaveInternally();
					if (!windowName.isEmpty()){
						img->setWindowName(windowName);
						img->setOnTop(false);
					}
					img->setRect(layer.bitmaps[i].clientRect.left*fScale, layer.bitmaps[i].clientRect.top*fScale + yOffset, layer.bitmaps[i].clientRect.width()*fScale, layer.bitmaps[i].clientRect.height()*fScale);

					int bkg = 0;
					switch(layer.bitmaps[i].borderType){
						case Origin::BlackLine:
						case Origin::DarkMarble:
							bkg = 1;
							break;
						case Origin::Shadow:
							bkg = 2;
							break;
						default:
							bkg = 0;
							break;
					}
					img->setFrameStyle(bkg);
				}
			}

			if (layer.imageProfileTool)
				imageProfileTool = true;

			if (boxWhiskersPlot){
				QStringList curveNames;
				for (int i = 0; i < graph->curveCount(); i++){
					BoxCurve *box = (BoxCurve *)graph->curve(i);
					if (!box || box->type() != Graph::Box)
						continue;
					Table *t = box->table();
					if (t)
						curveNames << box->title().text().remove(t->objectName() + "_");
					box->setData(QwtSingleArrayData(double(i + 1), QwtArray<double>(), 0));

					box->setVisibleLabels(layer.percentile.labels);
					switch(layer.percentile.labels){
						case 2:
							box->showBoxLabels();
						break;
						case 4:
							box->showWhiskerLabels();
						break;
						case 6:
							box->showBoxLabels();
							box->showWhiskerLabels();
						break;
						default:
							break;
					}

					int b_style = 0;
					if (layer.percentile.diamondBox)
						b_style = BoxCurve::Diamond;
					else {
						switch(layer.percentile.boxRange){
							case 0:
								b_style = BoxCurve::NoBox;
							break;
							case 4:
								b_style = BoxCurve::WindBox;
							break;
							case 6:
								b_style = BoxCurve::Notch;
							break;
							default:
								b_style = BoxCurve::Rect;
								break;
						}
					}
					box->setBoxStyle(b_style);

					int b_range = 0;
					switch(layer.percentile.boxRange){
						case 1:
							b_range = BoxCurve::SE;
						break;
						case 2:
							b_range = BoxCurve::SD;
						break;
						case 3:
							b_range = BoxCurve::r25_75;
						break;
						case 4:
							b_range = BoxCurve::r10_90;
						break;
						case 6:
							b_range = BoxCurve::r25_75;
						break;
						default:
							b_range = BoxCurve::UserDef;
						break;
					}
					box->setBoxRange(b_range, layer.percentile.boxCoeff);

					int w_range = 0;
					switch(layer.percentile.whiskersRange){
						case 0:
							w_range = BoxCurve::None;
						break;
						case 1:
							w_range = BoxCurve::r10_90;
						break;
						case 2:
							w_range = BoxCurve::r5_95;
						break;
						case 3:
							w_range = BoxCurve::r1_99;
						break;
						case 4:
							w_range = BoxCurve::SD;
						break;
						case 5:
							w_range = BoxCurve::SE;
						break;
						case 7:
							w_range = BoxCurve::MinMax;
						break;
						case 8:
							w_range = BoxCurve::UserDef;
						break;
						default:
							w_range = BoxCurve::UserDef;
							break;
					}
					box->setWhiskersRange(w_range, layer.percentile.whiskersCoeff);

					box->loadData();

					unsigned short size = layer.percentile.symbolSize;
					QColor fillColor = originToQtColor(layer.percentile.symbolFillColor);
					QBrush brush = QBrush();
					if (fillColor.isValid())
						brush = QBrush(fillColor);

					QPen pen = box->pen();
					QColor color = originToQtColor(layer.percentile.symbolColor);
					if (color.isValid() && layer.percentile.symbolColor.type != Origin::Color::Automatic)
						pen = QPen(color, 1);

					box->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, brush, pen, QSize(size, size)));
					box->setP99Style(originToQwtSymbolStyle(layer.percentile.p99SymbolType));
					box->setMeanStyle(originToQwtSymbolStyle(layer.percentile.meanSymbolType));
					box->setMaxStyle(originToQwtSymbolStyle(layer.percentile.maxSymbolType));
					box->setMinStyle(originToQwtSymbolStyle(layer.percentile.minSymbolType));
					box->setP1Style(originToQwtSymbolStyle(layer.percentile.p1SymbolType));
				}
				ScaleDraw *sd = (ScaleDraw *)graph->axisScaleDraw(QwtPlot::xBottom);
				sd->setLabelsList(curveNames);
				sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);
				graph->setAxisScaleDraw(QwtPlot::xBottom, sd);

				sd = (ScaleDraw *)graph->axisScaleDraw(QwtPlot::xTop);
				sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);
				graph->setAxisScaleDraw(QwtPlot::xTop, sd);
				graph->replot();
			}
		}

		if (doubleAxesLayout)
			convertDoubleAxesPlot(ml);

		if (imageProfileTool){
			Graph *graph = ml->layer(1);
			if (graph){
				Spectrogram *sp = (Spectrogram *) graph->plotItem(0);
				if (sp){
					Table *vt = NULL;
					Table *ht = NULL;
					Graph *g = ml->layer(2);
					if (g){
						DataCurve *c = g->dataCurve(0);
						if (c)
							ht = c->table();
					}
					g = ml->layer(3);
					if (g){
						DataCurve *c = g->dataCurve(0);
						if (c)
							vt = c->table();
					}

					ImageProfilesTool *tool = new ImageProfilesTool(mw, graph, sp->matrix(), ht, vt, mw->infoLineEdit(), SLOT(setText(const QString&)));
					graph->setActiveTool(tool);

					Origin::GraphLayer& layer = _graph.layers[0];
					double x = layer.vLine * (layer.xAxis.max - layer.xAxis.min) + layer.xAxis.min;
					double y = layer.hLine * (layer.yAxis.max - layer.yAxis.min) + layer.yAxis.min;
					tool->append(QwtDoublePoint(x, y));
				}
			}
		}

		//cascade the graphs
		if(ml->numLayers() > 0){
			if(!_graph.hidden){
				ml->move(QPoint(graphWindowRect.left, graphWindowRect.top));

				switch(_graph.state){
					case Origin::Window::Minimized:
						ml->setMinimized();
						break;
					case Origin::Window::Maximized:
						ml->show(); // to correct scaling with maximize
						ml->setScaleLayersOnResize(true);
						ml->setMaximized();
						break;
					default:
						ml->setScaleLayersOnResize(false);
						ml->show();
						ml->setScaleLayersOnResize(mw->autoResizeLayers);
				}
			} else {
				ml->show();
				mw->hideWindow(ml);
			}

			foreach (Graph *g, ml->layersList())
				g->setAutoscaleFonts(mw->autoScaleFonts && _graph.state != Origin::Window::Maximized);
		} else {
			ml->askOnCloseEvent(false);
			ml->close();
		}
	}

	return true;
}

void ImportOPJ::importSpectrogram(Graph *graph, Spectrogram *sp, const Origin::GraphLayer& layer, const Origin::GraphCurve& _curve, double fFontScaleFactor)
{
	if (!graph || !sp)
		return;

	graph->enableAxis(QwtPlot::yRight);
	graph->setAxisTicksLength(QwtPlot::yRight, Graph::NoTicks, Graph::NoTicks, 0, 0);

	int levelsCount = _curve.colorMap.levels.size();
	if (sp->matrix() && levelsCount > 2 && levelsCount < 100){
		double vmin = 0.0, vmax = 0.0;
		sp->matrix()->range(&vmin, &vmax);

		QwtValueList ticksList;
		ticksList << floor(vmin);
		for(Origin::ColorMapVector::const_iterator it = _curve.colorMap.levels.begin() + 1; it != _curve.colorMap.levels.end(); ++it)
			ticksList << it->first;
		ticksList << ceil(vmax);

		QwtValueList ticks[QwtScaleDiv::NTickTypes];
		ticks[QwtScaleDiv::MajorTick] = ticksList;
		ticks[QwtScaleDiv::MediumTick] = QwtValueList();
		ticks[QwtScaleDiv::MinorTick] = QwtValueList();

		QwtScaleDiv div(vmin, vmax, ticks);
		if (!layer.colorScale.reverseOrder)
			div.invert();
		graph->setAxisScaleDiv(QwtPlot::yRight, div);
	} else
		graph->setAxisMaxMajor(QwtPlot::yRight, 10);

	sp->setCustomColorMap(qwtColorMap(sp->matrix(), _curve.colorMap));

	QwtValueList levels;
	QList<QPen> penList;
	bool labelsOn = false;
	for(Origin::ColorMapVector::const_iterator it = _curve.colorMap.levels.begin() + 1; it != _curve.colorMap.levels.end(); ++it){
		if(it->second.lineVisible){
			Origin::ColorMapVector::const_iterator next = it;
			++next;
			if (next != _curve.colorMap.levels.end()){
				levels.push_back(it->first);
				penList << QPen(originToQtColor(next->second.lineColor), next->second.lineWidth, lineStyles[(Origin::GraphCurve::LineStyle)next->second.lineStyle]);
				if(next->second.labelVisible)
					labelsOn = true;
			}

			sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
		}
	}

	Origin::ColorMapVector::const_iterator it = _curve.colorMap.levels.begin() + 1;
	penList << QPen(originToQtColor(it->second.lineColor), it->second.lineWidth, lineStyles[(Origin::GraphCurve::LineStyle)it->second.lineStyle]);

	sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, _curve.colorMap.fillEnabled);
	sp->setContourLevels(levels);
	sp->setContourPenList(penList);
	sp->showContourLineLabels(labelsOn);
	if (labelsOn){
		sp->setLabelsWhiteOut(_curve.text.whiteOut);
		sp->setLabelsColor(originToQtColor(_curve.text.color));
		QFont fnt = sp->labelsFont();
		fnt.setBold(_curve.text.fontBold);
		fnt.setItalic(_curve.text.fontItalic);
		fnt.setUnderline(_curve.text.fontUnderline);
		fnt.setPointSizeF(_curve.text.fontSize*fFontScaleFactor);
		sp->setLabelsFont(fnt);
	}
}

bool ImportOPJ::importGraph3D(const OriginFile& opj, unsigned int g, unsigned int l)
{
	static QVector<Qwt3D::AXIS> xAxes, yAxes, zAxes;
	xAxes << Qwt3D::X1 << Qwt3D::X2 << Qwt3D::X3 << Qwt3D::X4;
	yAxes << Qwt3D::Y1 << Qwt3D::Y2 << Qwt3D::Y3 << Qwt3D::Y4;
	zAxes << Qwt3D::Z1 << Qwt3D::Z2 << Qwt3D::Z3 << Qwt3D::Z4;

	int auto_color = -1;
	int type = 0;
	Origin::Graph _graph = opj.graph(g);
	Origin::GraphLayer& layer = _graph.layers[l];
	for(unsigned int c = 0; c < layer.curves.size(); ++c){
		Origin::GraphCurve& _curve = layer.curves[c];
		QString data(_curve.dataName.c_str());
		double fFontScaleFactor = 2.0;
		switch(_curve.type){
			case Origin::GraphCurve::Line3D:
				type = _curve.connectSymbols ? Graph3D::Trajectory : Graph3D::Scatter;
				break;
			case Origin::GraphCurve::LineSymbol:
			case Origin::GraphCurve::Area:
				type = Graph3D::Ribbon;
				break;
			case Origin::GraphCurve::Column:
				type = Graph3D::Bars;
				break;
			case  Origin::GraphCurve::Mesh3D:
				break;
			default:
				continue;
		}

		Graph3D *plot = mw->newPlot3D();
		if (!plot)
			return false;

		plot->setName(_graph.name.c_str());
		plot->setWindowLabel(_graph.label.c_str());

		plot->setCaptionPolicy((MdiSubWindow::CaptionPolicy)_graph.title);
		plot->setBirthDate(posixTimeToString(_graph.creationDate));
		plot->hide();//!hack used in order to avoid resize and repaint events

		Origin::Rect graphRect(_graph.width, _graph.height);
		Origin::Rect graphWindowRect = _graph.frameRect;{
			double ratio = (double)(graphWindowRect.width() - frameWidth)/(double)(graphWindowRect.height() - frameHeight);
			int width = _graph.width;
			int height = _graph.height;
			if((double)(_graph.width)/(double)(_graph.height) < ratio)
				width = height * ratio;
			else
				height = width / ratio;

			//plot->resize(graphWindowRect.width(), graphWindowRect.height());

			double fScale = (double)(graphWindowRect.width() - frameWidth)/(double)width;
			fFontScaleFactor *= 170.0*fScale/72.0*1.3; //Ion: empirically decreased if from 300*...
		}

		Origin::Rect layerRect = layer.clientRect;

		QFont font = plot->xAxisLabelFont();
		font.setPointSize(floor(layer.xAxis.formatAxis[0].label.fontSize*fFontScaleFactor + 0.5));
		QString label = parseOriginText(QString::fromLocal8Bit(layer.xAxis.formatAxis[0].label.text.c_str()), true);
		RGBA xLabelColor = Qt2GL(originToQtColor(layer.xAxis.formatAxis[0].label.color));
		if (label.isEmpty()){
			label = parseOriginText(QString::fromLocal8Bit(layer.xAxis.formatAxis[1].label.text.c_str()), true);
			xLabelColor = Qt2GL(originToQtColor(layer.xAxis.formatAxis[1].label.color));
			font.setPointSize(floor(layer.xAxis.formatAxis[1].label.fontSize*fFontScaleFactor + 0.5));
		}
		plot->setXAxisLabel(label);
		plot->setXAxisLabelFont(font);

		label = parseOriginText(QString::fromLocal8Bit(layer.yAxis.formatAxis[0].label.text.c_str()), true);
		font.setPointSize(floor(layer.yAxis.formatAxis[0].label.fontSize*fFontScaleFactor + 0.5));
		RGBA yLabelColor = Qt2GL(originToQtColor(layer.yAxis.formatAxis[0].label.color));
		if (label.isEmpty()){
			label = parseOriginText(QString::fromLocal8Bit(layer.yAxis.formatAxis[1].label.text.c_str()), true);
			yLabelColor = Qt2GL(originToQtColor(layer.yAxis.formatAxis[1].label.color));
			font.setPointSize(floor(layer.yAxis.formatAxis[1].label.fontSize*fFontScaleFactor + 0.5));
		}
		plot->setYAxisLabel(label);
		plot->setYAxisLabelFont(font);

		label = parseOriginText(QString::fromLocal8Bit(layer.zAxis.formatAxis[0].label.text.c_str()), true);
		font.setPointSize(floor(layer.zAxis.formatAxis[0].label.fontSize*fFontScaleFactor + 0.5));
		RGBA zLabelColor = Qt2GL(originToQtColor(layer.zAxis.formatAxis[0].label.color));
		if (label.isEmpty()){
			label = parseOriginText(QString::fromLocal8Bit(layer.zAxis.formatAxis[1].label.text.c_str()), true);
			zLabelColor = Qt2GL(originToQtColor(layer.zAxis.formatAxis[1].label.color));
			font.setPointSize(floor(layer.zAxis.formatAxis[1].label.fontSize*fFontScaleFactor + 0.5));
		}
		plot->setZAxisLabel(label);
		plot->setZAxisLabelFont(font);

		double majorTickLength =  layer.xAxis.formatAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].majorTickLength;
		plot->setXAxisTickLength(majorTickLength, majorTickLength*0.6);
		majorTickLength =  layer.yAxis.formatAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].majorTickLength;
		plot->setYAxisTickLength(majorTickLength, majorTickLength*0.6);
		majorTickLength =  layer.zAxis.formatAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].majorTickLength;
		plot->setZAxisTickLength(majorTickLength, majorTickLength*0.6);

		if(layer.backgroundColor.type != Origin::Color::None)
			plot->setBackgroundColor(originToQtColor(layer.backgroundColor));

		plot->coordinateSystem()->setGridLines(true, true, Qwt3D::LEFT | Qwt3D::FLOOR | Qwt3D::BACK);

		RGBA axisColor = Qt2GL(ColorBox::defaultColor(layer.xAxis.formatAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].color));
		RGBA numberColor = layer.xAxis.tickAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].color == 0xF7 ? axisColor : Qt2GL(ColorBox::defaultColor(layer.xAxis.tickAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].color));
		Qwt3D::GridLine majorGrid(!layer.xAxis.majorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.xAxis.majorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.majorGrid.style], layer.xAxis.majorGrid.width);
		Qwt3D::GridLine minorGrid(!layer.xAxis.majorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.xAxis.majorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.xAxis.majorGrid.style], layer.xAxis.majorGrid.width);
		double width = layer.xAxis.formatAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].thickness;
		font = plot->numbersFont();
		font.setBold(layer.xAxis.tickAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].fontBold);
		font.setPointSize(floor(layer.xAxis.tickAxis[(layer.xAxis.position == Origin::GraphAxis::Bottom ? 0 : 1)].fontSize*fFontScaleFactor + 0.5));
		foreach(Qwt3D::AXIS axis, xAxes){
			plot->coordinateSystem()->axes[axis].setColor(axisColor);
			plot->coordinateSystem()->axes[axis].setNumberColor(numberColor);
			plot->coordinateSystem()->axes[axis].setLabelColor(xLabelColor);
			plot->coordinateSystem()->setMajorGridLines(axis, majorGrid);
			plot->coordinateSystem()->setMinorGridLines(axis, minorGrid);
			plot->coordinateSystem()->axes[axis].setMinors(layer.xAxis.minorTicks + 1);
			plot->coordinateSystem()->axes[axis].setLineWidth(width);
			plot->setNumbersFont(font);
		}

		axisColor = Qt2GL(ColorBox::defaultColor(layer.yAxis.formatAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].color));
		numberColor = layer.yAxis.tickAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].color == 0xF7 ? axisColor : Qt2GL(ColorBox::defaultColor(layer.yAxis.tickAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].color));
		majorGrid = Qwt3D::GridLine(!layer.yAxis.majorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.yAxis.majorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.yAxis.majorGrid.style], layer.yAxis.majorGrid.width);
		minorGrid = Qwt3D::GridLine(!layer.yAxis.minorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.yAxis.minorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.yAxis.minorGrid.style], layer.yAxis.minorGrid.width);
		width = layer.yAxis.formatAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].thickness;
		font.setBold(layer.yAxis.tickAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].fontBold);
		font.setPointSize(floor(layer.yAxis.tickAxis[(layer.yAxis.position == Origin::GraphAxis::Left ? 0 : 1)].fontSize*fFontScaleFactor + 0.5));
		foreach(Qwt3D::AXIS axis, yAxes){
			plot->coordinateSystem()->axes[axis].setColor(axisColor);
			plot->coordinateSystem()->axes[axis].setNumberColor(numberColor);
			plot->coordinateSystem()->axes[axis].setLabelColor(yLabelColor);
			plot->coordinateSystem()->setMajorGridLines(axis, majorGrid);
			plot->coordinateSystem()->setMinorGridLines(axis, minorGrid);
			plot->coordinateSystem()->axes[axis].setMinors(layer.yAxis.minorTicks + 1);
			plot->coordinateSystem()->axes[axis].setLineWidth(width);
			plot->setNumbersFont(font);
		}

		axisColor = Qt2GL(ColorBox::defaultColor(layer.zAxis.formatAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].color));
		numberColor = layer.zAxis.tickAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].color == 0xF7 ? axisColor : Qt2GL(ColorBox::defaultColor(layer.zAxis.tickAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].color));
		majorGrid = Qwt3D::GridLine(!layer.zAxis.majorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.zAxis.majorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.zAxis.majorGrid.style], layer.zAxis.majorGrid.width);
		minorGrid = Qwt3D::GridLine(!layer.zAxis.minorGrid.hidden, Qt2GL(ColorBox::defaultColor(layer.zAxis.minorGrid.color)), line3DStyles[(Origin::GraphCurve::LineStyle)layer.zAxis.minorGrid.style], layer.zAxis.minorGrid.width);
		width = layer.zAxis.formatAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].thickness;
		font.setBold(layer.zAxis.tickAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].fontBold);
		font.setPointSize(floor(layer.zAxis.tickAxis[(layer.zAxis.position == Origin::GraphAxis::Front ? 0 : 1)].fontSize*fFontScaleFactor + 0.5));
		foreach(Qwt3D::AXIS axis, zAxes){
			plot->coordinateSystem()->axes[axis].setColor(axisColor);
			plot->coordinateSystem()->axes[axis].setNumberColor(numberColor);
			plot->coordinateSystem()->axes[axis].setLabelColor(zLabelColor);
			plot->coordinateSystem()->setMajorGridLines(axis, majorGrid);
			plot->coordinateSystem()->setMinorGridLines(axis, minorGrid);
			plot->coordinateSystem()->axes[axis].setMinors(layer.zAxis.minorTicks + 1);
			plot->coordinateSystem()->axes[axis].setLineWidth(width);
			plot->setNumbersFont(font);
		}

		QStringList formulas;

		plot->showColorLegend(false);
		plot->setFramed();

		QColor clr = (_curve.symbolColor.type == Origin::Color::Automatic ? ColorBox::defaultColor(++auto_color) : originToQtColor(_curve.symbolColor));
		plot->setDataColors(clr, clr);

		bool smooth;
		switch(_curve.symbolType&0xFF){
			case 2: //Ellipse
			case 20://Sphere
				smooth = true;
				break;
			default:
				smooth = false;
		}
		plot->setDotOptions(ceil(_curve.symbolSize), smooth);

		switch(data[0].toAscii()){
			case 'T':{
				Table* t = mw->table(data.right(data.length()-2));
				if (_curve.zColumnName.empty()){
					plot->addRibbon(t, t->objectName() + "_" + _curve.xColumnName.c_str(), t->objectName() + "_" + _curve.yColumnName.c_str());
					QColor color = originToQtColor(_curve.surface.frontColor);
					plot->setDataColorMap(QwtLinearColorMap(color, color));
					plot->setMeshColor(_curve.lineColor.type == Origin::Color::Automatic ? ColorBox::defaultColor(0) : originToQtColor(_curve.fillAreaPatternBorderColor));
				} else {
					plot->addData(t, t->colIndex(_curve.xColumnName.c_str()), t->colIndex(_curve.yColumnName.c_str()), t->colIndex(_curve.zColumnName.c_str()), type);
					plot->setMeshColor(_curve.lineColor.type == Origin::Color::Automatic ? ColorBox::defaultColor(0) : originToQtColor(_curve.lineColor));
				}

				plot->setMeshLineWidth(_curve.lineWidth);
			}
			break;
			case 'M':{
				Matrix* matrix = mw->matrix(data.right(data.length()-2));
				plot->addMatrixData(matrix);
				switch(_curve.surface.type){
					case Origin::SurfaceProperties::ColorMap3D:{
						if(_curve.surface.surface.fill && _curve.surface.grids != Origin::SurfaceProperties::None)
							plot->customPlotStyle(Qwt3D::FILLEDMESH);
						else if(_curve.surface.surface.fill)
							plot->customPlotStyle(Qwt3D::FILLED);
						else if(_curve.surface.grids != Origin::SurfaceProperties::None)
							plot->customPlotStyle(Qwt3D::WIREFRAME);

						ColorVector colors;
						for(Origin::ColorMapVector::const_iterator it = _curve.surface.colorMap.levels.begin() + 1; it != _curve.surface.colorMap.levels.end() - 1; ++it)
							colors.push_back(Qt2GL(originToQtColor(it->second.fillColor)));
						plot->setDataColorMap(colors, qwtColorMap(matrix, _curve.surface.colorMap));

						if(_curve.surface.bottomContour.fill)
							plot->setFloorData();
						else if(_curve.surface.bottomContour.contour)
							plot->setFloorIsolines();
						}
					break;
					case Origin::SurfaceProperties::ColorFill:{
						if(_curve.surface.grids != Origin::SurfaceProperties::None)
							plot->customPlotStyle(Qwt3D::FILLEDMESH);
						 else
							plot->customPlotStyle(Qwt3D::FILLED);

						QColor color = originToQtColor(_curve.surface.frontColor);
						plot->setDataColorMap(QwtLinearColorMap(color, color));
					 }
					break;
					case Origin::SurfaceProperties::WireFrame:{
						if(_curve.surface.grids != Origin::SurfaceProperties::None)
							plot->customPlotStyle(Qwt3D::WIREFRAME);
						else
							plot->customPlotStyle(Qwt3D::HIDDENLINE);
					 }
					break;
					case Origin::SurfaceProperties::Bars:{
						plot->customPlotStyle(Qwt3D::USER);
						ColorVector colors;
						for(Origin::ColorMapVector::const_iterator it = _curve.surface.colorMap.levels.begin() + 1; it != _curve.surface.colorMap.levels.end(); ++it)
								colors.push_back(Qt2GL(originToQtColor(it->second.fillColor)));
						plot->setDataColorMap(colors, qwtColorMap(matrix, _curve.surface.colorMap));
					}
					break;
					default:
						break;
				 }
				plot->setMeshColor(originToQtColor(_curve.surface.gridColor));
				plot->setMeshLineWidth(_curve.surface.gridLineWidth);
			}
		break;
		default:
			continue;
		}

		/*if(_curve.connectSymbols)
			plot->setWireframeStyle();
		else
			plot->setDotStyle();*/
		//plot->setScales(layer.xAxis.min, layer.xAxis.max, layer.yAxis.min, layer.yAxis.max, layer.zAxis.min, layer.zAxis.max);

		if (!layer.isXYY3D){
			int majorTicks = ceil((layer.xAxis.max - layer.xAxis.min)/layer.xAxis.step);
			plot->setScale(0, layer.xAxis.min, layer.xAxis.max, majorTicks, layer.xAxis.minorTicks, (Qwt3D::SCALETYPE)scaleTypes[(Origin::GraphAxis::Scale)layer.xAxis.scale]);

			majorTicks = ceil((layer.yAxis.max - layer.yAxis.min)/layer.yAxis.step);
			plot->setScale(1, layer.yAxis.min, layer.yAxis.max, majorTicks, layer.yAxis.minorTicks, (Qwt3D::SCALETYPE)scaleTypes[(Origin::GraphAxis::Scale)layer.yAxis.scale]);

			majorTicks = ceil((layer.zAxis.max - layer.zAxis.min)/layer.zAxis.step);
			plot->setScale(2, layer.zAxis.min, layer.zAxis.max, majorTicks, layer.zAxis.minorTicks, (Qwt3D::SCALETYPE)scaleTypes[(Origin::GraphAxis::Scale)layer.zAxis.scale]);
		}
		plot->resetAxesLabels();

		if(!_graph.hidden){
			plot->move(QPoint(graphWindowRect.left, graphWindowRect.top));

			switch(_graph.state){
				case Origin::Window::Minimized:
					mw->minimizeWindow(plot);
					break;
				case Origin::Window::Maximized:
					plot->show(); // to correct scaling with maximize
					mw->maximizeWindow(plot);
					break;
				default:
					plot->show();
			}
		} else {
			plot->show();
			mw->hideWindow(plot);
		}
	}

	return true;
}

void ImportOPJ::setPieTexts(PieCurve *p, Graph* graph, const Origin::GraphLayer& layer, double fFontScaleFactor, double fScale)
{
	if (!p || !graph)
		return;

	QList <PieLabel *> pieTexts = p->labelsList();
	QFont font(mw->plotLegendFont);
	int lsize = layer.pieTexts.size();
	for(int i = 0; i < lsize && i < pieTexts.size(); ++i){
		Origin::TextBox text = layer.pieTexts[lsize - i - 1];
		font.setPointSizeF(text.fontSize*fFontScaleFactor);
		QFontMetrics fm(font, graph);
		PieLabel *l = pieTexts[i];
		l->setFont(font);
		l->setTextColor(originToQtColor(text.color));

		int bkg;
		QColor bkgColor = Qt::white;
		switch(text.borderType){
			case Origin::BlackLine:
				bkg = 1;
				break;
			case Origin::Shadow:
				bkg = 2;
				break;
			case Origin::DarkMarble:
				bkg = 1;
				bkgColor = Qt::darkGray;
				break;
			case Origin::BlackOut:
				bkg = 0;
				bkgColor = Qt::black;
				break;
			default:
				bkg = 0;
				bkgColor.setAlpha(0);
				break;
		}
		l->setFrameStyle(bkg);
		l->setBackgroundColor(bkgColor);
		l->setCustomText(QString(text.text.c_str()));
		l->move(QPoint(qRound(text.clientRect.left*fScale - fm.averageCharWidth()*fFontScaleFactor - l->framePen().width()),
				qRound((text.clientRect.top - fm.lineSpacing())*fScale) - LayerButton::btnSize() - l->framePen().width()));
		l->setAttachPolicy(FrameWidget::Page);
	}
}

void ImportOPJ::addText(const Origin::TextBox& text, Graph* graph, double fFontScaleFactor, double fScale)
{
	int bkg;
	QColor bkgColor = Qt::white;
	switch(text.borderType){
		case Origin::BlackLine:
			bkg = 1;
			break;
		case Origin::Shadow:
			bkg = 2;
			break;
		case Origin::DarkMarble:
			bkg = 1;
			bkgColor = Qt::darkGray;
			break;
		case Origin::BlackOut:
			bkg = 0;
			bkgColor = Qt::black;
			break;
		default:
			bkg = 0;
			bkgColor.setAlpha(0);
			break;
	}

	LegendWidget* txt = graph->newLegend(parseOriginText(QString::fromLocal8Bit(text.text.c_str())));

	QFont font(mw->plotLegendFont);
	font.setPointSizeF(text.fontSize*fFontScaleFactor);
	txt->setFont(font);
	txt->setAngle(text.rotation);
	txt->setTextColor(originToQtColor(text.color));
	txt->setFrameStyle(bkg);
	txt->setBackgroundColor(bkgColor);

	QFont font2(mw->plotLegendFont);
	font2.setPointSize(text.fontSize);
	QFontMetrics fm(font2, graph);
	txt->move(QPoint(qRound(text.clientRect.left*fScale - fm.averageCharWidth()*fFontScaleFactor - txt->framePen().width()),
					 qRound((text.clientRect.top - fm.lineSpacing())*fScale) + LayerButton::btnSize() - txt->framePen().width()));

	if (text.rotation == 90)//TODO: better take into account the rotation
		txt->move(QPoint(txt->x(), txt->y() - txt->width()));

	txt->setAttachPolicy(FrameWidget::Page);
}

void ImportOPJ::parseXYZContourPlotAxisTitles(Graph *g, Table *t, const Origin::GraphCurve& curve)
{
	if (!g || !t)
		return;

	Graph::AxisTitlePolicy d_axis_title_policy = g->axisTitlePolicy();

	for (int axis = 0; axis < QwtPlot::axisCnt; axis++){
		QString s = g->axisTitleString(axis);
		if (s.trimmed().isEmpty())
			continue;

		QString comment = QString::null;
		if (s.contains("%(?Y)", Qt::CaseInsensitive)){
			QString name = QString(curve.yColumnName.c_str());
			if (d_axis_title_policy > 1)
				comment = t->comment(t->colIndex(name)).trimmed().replace("\n", " ");

			switch(d_axis_title_policy){
				case Graph::ColName:
					s.replace("%(?Y)", name, Qt::CaseInsensitive);
				break;
				case Graph::ColComment:
					if (!comment.isEmpty())
						s.replace("%(?Y)", comment, Qt::CaseInsensitive);;
				break;
				case Graph::NameAndComment:
					if (!comment.isEmpty())
						name += " (" + comment + ")";
					s.replace("%(?Y)", name, Qt::CaseInsensitive);
				break;
				default:
					break;
			}
		}

		if (s.contains("%(?X)", Qt::CaseInsensitive)){
			QString name = QString(curve.xColumnName.c_str());
			if (d_axis_title_policy > 1)
					comment = t->comment(t->colIndex(name)).trimmed().replace("\n", " ");

			switch(d_axis_title_policy){
				case Graph::ColName:
				  s.replace("%(?X)", name, Qt::CaseInsensitive);
				break;
				case Graph::ColComment:
				  if (!comment.isEmpty())
					  s.replace("%(?X)", comment, Qt::CaseInsensitive);;
				break;
				case Graph::NameAndComment:
				  if (!comment.isEmpty())
					  name += " (" + comment + ")";
				  s.replace("%(?X)", name, Qt::CaseInsensitive);
				break;
				default:
				  break;
			}
		}

		g->setAxisTitleString(axis, s);
	}
}

QString ImportOPJ::parseOriginText(const QString &str, bool removeTags)
{
	QStringList lines = str.trimmed().split("\n");
	QString text = "";
	for(int i = 0; i < lines.size(); ++i){
		if(i > 0)
			text.append("\n");
		text.append(parseOriginTags(lines[i], removeTags));
	}
	return text;
}

QString ImportOPJ::parseOriginTags(const QString &str, bool removeTags)
{
	QString line = str;

	QStringList tagsLst = QStringList()
		<< "\\\\\\s*b\\s*\\(.*\\)"
		<< "\\\\\\s*i\\s*\\(.*\\)"
		<< "\\\\\\s*u\\s*\\(.*\\)"
		<< "\\\\\\s*g\\s*\\(.*\\)"
		<< "\\\\\\s*\\+\\s*\\(.*\\)"
		<< "\\\\\\s*\\-\\s*\\(.*\\)"
		<< "\\\\\\s*f\\:.*\\(.*\\)"
		<< "\\\\\\s*c(\\d)+\\(.*\\)" // \c2(...) like tags
		<< "\\\\\\s*p(\\d)+\\(.*\\)"; // \p163(...) like tags

	if (removeTags){
		foreach (QString s, tagsLst){
			QRegExp fontModifier(s);
			fontModifier.setMinimal(true);
			int index = line.indexOf(fontModifier);
			while (index >= 0){
				int pos = line.indexOf("(", index) + 1;
				int length = fontModifier.matchedLength();
				int l = index + length;
				line = line.left(index) + line.mid(pos, l - pos - 1) + line.right(line.length() - l);

				index = line.indexOf(fontModifier);
			}
		}
		return parseAsciiCodes(line);
	}

	line = parseAsciiCodes(line);

	//replace \b(...), \i(...), \u(...), \g(...), \+(...), \-(...), \f:font(...) tags
	QString ltag[] = {"<b>","<i>","<u>","<font face=Symbol>","<sup>","<sub>","<font face=%1>", "<font color=%1>"};
	QString rtag[] = {"</b>","</i>","</u>","</font>","</sup>","</sub>","</font>","</font>"};

	for(int i = 0; i < tagsLst.size(); i++){
		QRegExp fontModifier(tagsLst[i]);
		fontModifier.setMinimal(true);
		int index = line.indexOf(fontModifier);
		while (index >= 0){
			int pos = line.indexOf("(", index) + 1;
			int length = fontModifier.matchedLength();
			int l = index + length;

			QString left = line.left(index);
			QString mid = line.mid(pos, l - pos - 1);
			QString right = line.right(line.length() - l);

			if (i == 6){
				int posFont = line.indexOf(":", index) + 1;
				QString fontName = line.mid(posFont, pos - posFont - 1);
				line = QString(left + ltag[i] + mid + rtag[i] + right).arg(fontName);
			} else if (i == 7){
				int posC = index + 2;
				int colIndex = line.mid(posC, pos - posC - 1).toInt() - 1;
				line = QString(left + ltag[i] + mid + rtag[i] + right).arg(ColorBox::defaultColor(colIndex).name());
			} else if (i == 8){// remove \p163(...) like tags
				line = left + mid + right;
			} else
				line = left + ltag[i] + mid + rtag[i] + right;

			index = line.indexOf(fontModifier);
		}
	}
	return line;
}

QString ImportOPJ::parseAsciiCodes(const QString& str)
{
	QString line = str;

	QRegExp charCode("\\\\\\s*\\(\\d+\\)");
	charCode.setMinimal(true);
	int index = line.indexOf(charCode);
	while (index >= 0){//replace ASCII codes
		int pos1 = line.indexOf("(", index) + 1;
		int length = charCode.matchedLength();
		QString mid = line.mid(pos1, index + length - pos1 - 1);
		line = line.replace(index, length, QString(QChar(mid.toInt())));

		index = line.indexOf(charCode);
	}
	return line;
}

QwtLinearColorMap ImportOPJ::qwtColorMap(Matrix *m, const Origin::ColorMap& colorMap)
{
	if (!m)
		return QwtLinearColorMap();

	double mmin = 0.0, mmax = 0.0;
	m->range(&mmin, &mmax);

	Origin::ColorMapVector::const_iterator it = colorMap.levels.begin();
	QColor color1 = originToQtColor(it->second.fillColor);

	it = colorMap.levels.end() - 1;
	QColor color2 = originToQtColor(it->second.fillColor);

	QwtLinearColorMap qwt_color_map = QwtLinearColorMap(color1, color2);
	qwt_color_map.setMode(QwtLinearColorMap::FixedColors);

	double dl = fabs(mmax - mmin);
	for(it = colorMap.levels.begin() + 1; it != colorMap.levels.end() - 1; ++it)
		qwt_color_map.addColorStop(fabs(it->first - mmin)/dl, originToQtColor(it->second.fillColor));

	return qwt_color_map;
}

QwtSymbol::Style ImportOPJ::originToQwtSymbolStyle(unsigned char type)
{
	int sType = 0;
	switch(type){
		case 0: //NoSymbol
			sType=0;
			break;
		case 1: //Rect
			sType=2;
			break;
		case 2: //Ellipse
		case 20://Sphere
			sType=1;
			break;
		case 3: //UTriangle
			sType=6;
			break;
		case 4: //DTriangle
			sType=5;
			break;
		case 5: //Diamond
			sType=3;
			break;
		case 6: //Cross +
			sType=9;
			break;
		case 7: //Cross x
			sType=10;
			break;
		case 8: //Snow
			sType=13;
			break;
		case 9: //Horizontal -
			sType=11;
			break;
		case 10: //Vertical |
			sType=12;
			break;
		case 15: //LTriangle
			sType=7;
			break;
		case 16: //RTriangle
			sType=8;
			break;
		case 17: //Hexagon
		case 19: //Pentagon
			sType=15;
			break;
		case 18: //Star
			sType=14;
			break;
		default:
			sType=0;
	}
	return SymbolBox::style(sType);
}

void ImportOPJ::convertDoubleAxesPlot(MultiLayer *ml)
{
	if (!ml || ml->numLayers() != 2)
		return;

	ml->setScaleLayersOnResize(false);

	Graph *l1 = ml->layer(1);
	Graph *l2 = ml->layer(2);
	if (!l1 || !l2)
		return;

	QPoint pos = l1->pos();

	l1->copyScaleWidget(l2, QwtPlot::xTop);
	l1->copyScaleWidget(l2, QwtPlot::yRight);
	l1->copyCurves(l2);
	l1->copyScaleDraw(l2, QwtPlot::xTop);
	l1->copyScaleDraw(l2, QwtPlot::yRight);
	l1->setAxisLabelRotation(QwtPlot::xTop, l2->labelsRotation(QwtPlot::xTop));

	foreach (FrameWidget *e, l2->enrichmentsList()){
		PieLabel *l = qobject_cast<PieLabel *>(e);
		if (l)
			continue;

		l1->add(e);
	}

	foreach (ArrowMarker *a, l2->arrowsList())
		l1->addArrow(a);

	foreach (FrameWidget *e, l1->enrichmentsList()){
		LegendWidget *l = qobject_cast<LegendWidget *>(e);
		if (!l || !l->text().contains("(2."))
			continue;

		l->setText(l->text().replace("(2.", "("));
	}

	int yOffset = l2->axisWidget(QwtPlot::xTop)->height();
	l1->setCanvasSize(l2->canvas()->size());
	ml->removeLayer(l2);
	ml->resize(ml->width(), ml->height() + yOffset);
}

void ImportOPJ::setCurveLabelsProperties(DataCurve *mc, const Origin::GraphCurve& _curve, double fFontScaleFactor)
{
	if (!mc)
		return;

	mc->setLabelsRotation(_curve.text.rotation);
	mc->setLabelsWhiteOut(_curve.text.whiteOut);
	mc->setLabelsOffset(_curve.text.xOffset, _curve.text.yOffset);
	mc->setLabelsColor(originToQtColor(_curve.text.color));
	int align = -1;
	switch(_curve.text.justify){
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
	fnt.setPointSizeF(_curve.text.fontSize*fFontScaleFactor);
	mc->setLabelsFont(fnt);
}

QString ImportOPJ::formatString(const Origin::ValueType& type, int valueTypeSpecification)
{
	QString format = QString();
	switch(type){
		case Origin::Date:
			switch(valueTypeSpecification){
				case -128:
					format="dd/MM/yyyy";
					break;
				case 0:
					format="dd.MM.yyyy";
					break;
				case 1:
					format="d MMMM yyyy";
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
				case 9:
				case -119:
					format="dd.MM.yyyy HH:mm";
					break;
				case 10:
				case -118:
					format="dd.MM.yyyy HH:mm:ss";
					break;
				case 11:
					format="yyMMdd";
					break;
				case 12:
					format="yyMMdd hh:mm";
					break;
				case 13:
					format="yyMMdd hh:mm:ss";
					break;
				case 14:
					format="yyMMdd hhmm";
					break;
				case 15:
					format="yyMMdd hhmmss";
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
			break;
		case Origin::Time:
			switch(valueTypeSpecification + 128)
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
			break;
		case Origin::Month:
			switch(valueTypeSpecification)
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
			break;
		case Origin::Day:
			switch(valueTypeSpecification)
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
			break;
		default:
			break;
	}
	return format;
}

//TODO: bug in grid dialog
//		scale/minor ticks checkbox
//		histogram: autobin export
//		if prec not setted - automac+4digits
