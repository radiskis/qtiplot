/***************************************************************************
    File                 : PlotController2D.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D plot creation, layout, and layer management controller

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                   *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                            *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                            *
 *                                                                         *
 ***************************************************************************/

#include "PlotController2D.h"
#include "ApplicationWindow.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Note.h"
#include "Folder.h"
#include "SelectionMoveResizer.h"
#include "LegendWidget.h"
#include "ArrowMarker.h"
#include "ImageWidget.h"
#include "RectangleWidget.h"
#include "AddWidgetTool.h"
#include "Grid.h"
#include "QwtHistogram.h"
#include "PieCurve.h"
#include "BoxCurve.h"
#include "QwtBarCurve.h"
#include "VectorCurve.h"
#include "FunctionCurve.h"
#include "Spectrogram.h"
#include "CurvesDialog.h"
#include "PlotDialog.h"
#include "AxesDialog.h"
#include "LineDialog.h"
#include "FunctionDialog.h"
#include "AssociationsDialog.h"
#include "LayerDialog.h"
#include "DataPickerTool.h"
#include "ScreenPickerTool.h"
#include "RangeSelectorTool.h"
#include "TranslateCurveTool.h"
#include <qwt_plot_magnifier.h>
#include <QUndoGroup>
#include <QInputDialog>
#include <gsl/gsl_sort.h>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDateTime>
#include <QToolButton>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>

PlotController2D::PlotController2D(ApplicationWindow *app)
    : QObject(app), d_app(app)
{
}

PlotController2D::~PlotController2D()
{
}

void PlotController2D::plotBox()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &Box = d_app->Box;

    generate2DGraph(Graph::Box);
}

void PlotController2D::plotVerticalBars()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::VerticalBars);
}

void PlotController2D::plotHorizontalBars()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::HorizontalBars);
}

void PlotController2D::plotStackBar()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::StackBar);
}

void PlotController2D::plotStackColumn()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::StackColumn);
}

MultiLayer* PlotController2D::plotHistogram()
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

    return generate2DGraph(Graph::Histogram);
}

MultiLayer* PlotController2D::plotHistogram(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!m){
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	MultiLayer* g = newGraph();
	if (g)
		g->activeLayer()->addHistogram(m);
	QApplication::restoreOverrideCursor();
	return g;
}

void PlotController2D::plotArea()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::Area);
}

void PlotController2D::plotPie()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *table = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!table)
		return;

	if(table->selectedColumns().count() != 1){
		QMessageBox::warning(d_app, tr("QtiPlot - Plot error"),
				tr("You must select exactly one column for plotting!"));
		return;
	}

	QStringList s = table->selectedColumns();
	if (s.count()>0){
		QTableWidgetSelectionRange sel = table->getSelection();
		multilayerPlot(table, s, Graph::Pie, sel.topRow(), sel.bottomRow());
	} else
		QMessageBox::warning(d_app, tr("QtiPlot - Error"), tr("Please select a column to plot!"));
}

void PlotController2D::plotL()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::Line);
}

void PlotController2D::plotP()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::Scatter);
}

void PlotController2D::plotLP()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::LineSymbols);
}

void PlotController2D::plotVerticalDropLines()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::VerticalDropLines);
}

void PlotController2D::plotSpline()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::Spline);
}

void PlotController2D::plotVertSteps()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::VerticalSteps);
}

void PlotController2D::plotHorSteps()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	generate2DGraph(Graph::HorizontalSteps);
}

void PlotController2D::plotVectXYXY()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *table = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!table)
		return;
	if (!validFor2DPlot(table, Graph::VectXYXY))
		return;

	QStringList s = table->selectedColumns();
	if (s.count() == 4) {
	QTableWidgetSelectionRange sel = table->getSelection();
		multilayerPlot(table, s, Graph::VectXYXY, sel.topRow(), sel.bottomRow());
	} else
		QMessageBox::warning(d_app, tr("QtiPlot - Error"), tr("Please select four columns for d_app operation!"));
}

void PlotController2D::plotVectXYAM()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    Table *table = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!table)
		return;
	if (!validFor2DPlot(table, Graph::VectXYAM))
		return;

	QStringList s = table->selectedColumns();
	if (s.count() == 4){
	QTableWidgetSelectionRange sel = table->getSelection();
		multilayerPlot(table, s, Graph::VectXYAM, sel.topRow(), sel.bottomRow());
	} else
		QMessageBox::warning(d_app, tr("QtiPlot - Error"), tr("Please select four columns for d_app operation!"));
}

MultiLayer* PlotController2D::multilayerPlot(const QString& caption, int layers, int rows, int cols)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	MultiLayer* ml = new MultiLayer(d_app, layers, rows, cols);
	initMultilayerPlot(ml, caption);
	return ml;
}

MultiLayer* PlotController2D::newGraph(const QString& caption)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	QString name = caption;
	while(d_app->alreadyUsedName(name))
		name = d_app->generateUniqueName(tr("Graph"));

	MultiLayer *ml = multilayerPlot(name);
	if (ml){
		Graph *g = ml->activeLayer();
		if (g){
			setPreferences(g);
			g->newLegend();
		}
		ml->arrangeLayers(false, true);
	}

	return ml;
}

MultiLayer* PlotController2D::multilayerPlot(Table* w, const QStringList& colList, int style, int startRow, int endRow)
{//used when plotting selected columns

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;
	auto &defaultCurveLineWidth = d_app->defaultCurveLineWidth;
	auto &defaultSymbolSize = d_app->defaultSymbolSize;

	if (!w)
		return 0;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	MultiLayer* g = multilayerPlot(d_app->generateUniqueName(tr("Graph")));
	Graph *ag = g->activeLayer();
	if (!ag)
		return 0;

	setPreferences(ag);
	ag->addCurves(w, colList, style, defaultCurveLineWidth, defaultSymbolSize, startRow, endRow);

	g->arrangeLayers(false, true);
	ag->newLegend();

	QApplication::restoreOverrideCursor();
	return g;
}

MultiLayer* PlotController2D::multilayerPlot(int c, int r, int style, const MultiLayer::AlignPolicy& align)
{//used when plotting from the panel menu

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;
	auto &defaultCurveLineWidth = d_app->defaultCurveLineWidth;
	auto &defaultSymbolSize = d_app->defaultSymbolSize;

	Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!t)
		return 0;

	if (!validFor2DPlot(t, (Graph::CurveType)style))
		return 0;

	QStringList list = t->drawableColumnSelection();
	if((int)list.count() < 1) {
		QMessageBox::warning(d_app, tr("QtiPlot - Plot error"), tr("Please select a Y column to plot!"));
		return 0;
	}

	int curves = list.count();
	if (r < 0)
		r = curves;

	int layers = c*r;
	MultiLayer* g = multilayerPlot(d_app->generateUniqueName(tr("Graph")), layers, r, c);
	QList<Graph *> layersList = g->layersList();
	int i = 0;
	for (Graph *ag : layersList){
		setPreferences(ag);
		if (i < curves){
			QStringList lst = QStringList() << list[i];
			for (int j = 0; j < curves; j++){
				int col = t->colIndex(list[j]);
				if (t->colPlotDesignation(col) == Table::xErr ||
					t->colPlotDesignation(col) == Table::yErr ||
					t->colPlotDesignation(col) == Table::Label){
					lst << list[j];
				}
			}
			ag->addCurves(t, lst, style, defaultCurveLineWidth, defaultSymbolSize);
		}
		i++;
	}

	if (align == MultiLayer::AlignCanvases){
		g->setAlignPolicy(align);
		g->setSpacing(0, 0);
		g->setCommonLayerAxes();
		connect(layersList.last(), &Graph::updatedLayout, g, &MultiLayer::updateLayersLayout);
	} else {
		g->arrangeLayers(false, true);
		for (Graph *ag : layersList){
			if (ag->curveCount())
				ag->newLegend();
		}
	}
	return g;
}

MultiLayer* PlotController2D::waterfallPlot()
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!t)
		return 0;

	return waterfallPlot(t, t->selectedYColumns());
}

MultiLayer* PlotController2D::waterfallPlot(Table *t, const QStringList& list)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!t)
		return 0;

	if(list.count() < 1){
		QMessageBox::warning(d_app, tr("QtiPlot - Plot error"),
		tr("Please select a Y column to plot!"));
		return 0;
	}

	MultiLayer* ml = new MultiLayer(d_app);

	Graph *g = ml->activeLayer();
	setPreferences(g);
	g->enableAxis(QwtPlot::xTop, false);
	g->enableAxis(QwtPlot::yRight, false);
	g->setCanvasFrame(0);
	g->setTitle(QString());
	g->setContentsMargins(0, 0, 0, 0);
	g->setFrame(0);
	g->addCurves(t, list, Graph::Line);
	g->setWaterfallOffset(10, 20);

	initMultilayerPlot(ml);
	ml->arrangeLayers(false, true);
	ml->setWaterfallLayout();

	g->newLegend()->move(QPoint(g->x() + g->canvas()->x() + 5, 5));

	return ml;
}

void PlotController2D::initMultilayerPlot(MultiLayer* g, const QString& name)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_mdi_windows_area = d_app->d_mdi_windows_area;
	auto &d_print_cropmarks = d_app->d_print_cropmarks;
	auto &d_scale_plots_on_print = d_app->d_scale_plots_on_print;
	auto &d_workspace = d_app->d_workspace;

	QString label = name;
	while(d_app->alreadyUsedName(label))
		label = d_app->generateUniqueName(tr("Graph"));

	g->setWindowTitle(label);
	g->setObjectName(label);
	g->setWindowIcon(QPixmap(":/graph.png"));
	g->setScaleLayersOnPrint(d_scale_plots_on_print);
	g->printCropmarks(d_print_cropmarks);

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(g);
	else
		g->setParent(0);

	connectMultilayerPlot(g);
	if (!qApp->arguments().contains("-X"))
		g->showNormal();

	d_app->addListViewItem(g);
        d_app->windowActivated(g);
}

void PlotController2D::setPreferences(Graph* g)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &antialiasing2DPlots = d_app->antialiasing2DPlots;
	auto &autoScaleFonts = d_app->autoScaleFonts;
	auto &autoscale2DPlots = d_app->autoscale2DPlots;
	auto &axesLineWidth = d_app->axesLineWidth;
	auto &canvasFrameWidth = d_app->canvasFrameWidth;
	auto &d_Douglas_Peuker_tolerance = d_app->d_Douglas_Peuker_tolerance;
	auto &d_canvas_frame_color = d_app->d_canvas_frame_color;
	auto &d_curve_max_antialising_size = d_app->d_curve_max_antialising_size;
	auto &d_decimation_method = d_app->d_decimation_method;
	auto &d_default_2D_grid = d_app->d_default_2D_grid;
	auto &d_disable_curve_antialiasing = d_app->d_disable_curve_antialiasing;
	auto &d_graph_axes_labels_dist = d_app->d_graph_axes_labels_dist;
	auto &d_graph_axis_labeling = d_app->d_graph_axis_labeling;
	auto &d_graph_background_color = d_app->d_graph_background_color;
	auto &d_graph_background_opacity = d_app->d_graph_background_opacity;
	auto &d_graph_border_color = d_app->d_graph_border_color;
	auto &d_graph_border_width = d_app->d_graph_border_width;
	auto &d_graph_canvas_color = d_app->d_graph_canvas_color;
	auto &d_graph_canvas_opacity = d_app->d_graph_canvas_opacity;
	auto &d_graph_tick_labels_dist = d_app->d_graph_tick_labels_dist;
	auto &d_show_axes = d_app->d_show_axes;
	auto &d_show_axes_labels = d_app->d_show_axes_labels;
	auto &d_show_empty_cell_gap = d_app->d_show_empty_cell_gap;
	auto &d_speed_mode_points = d_app->d_speed_mode_points;
	auto &d_synchronize_graph_scales = d_app->d_synchronize_graph_scales;
	auto &defaultPlotMargin = d_app->defaultPlotMargin;
	auto &drawBackbones = d_app->drawBackbones;
	auto &majTicksLength = d_app->majTicksLength;
	auto &majTicksStyle = d_app->majTicksStyle;
	auto &minTicksLength = d_app->minTicksLength;
	auto &minTicksStyle = d_app->minTicksStyle;
	auto &plotAxesFont = d_app->plotAxesFont;
	auto &plotNumbersFont = d_app->plotNumbersFont;
	auto &plotTitleFont = d_app->plotTitleFont;
	auto &titleOn = d_app->titleOn;

	if (!g)
		return;

	if (!g->isPiePlot()){
		for (int i = 0; i < QwtPlot::axisCnt; i++){
			bool show = d_show_axes[i];
			g->enableAxis(i, show);
			if(show){
				ScaleDraw *sd = (ScaleDraw *)g->axisScaleDraw (i);
				sd->enableComponent(QwtAbstractScaleDraw::Labels, d_show_axes_labels[i]);
				sd->setSpacing(d_graph_tick_labels_dist);
				if (i == QwtPlot::yRight && !d_show_axes_labels[i])
					g->setAxisTitle(i, tr(" "));
			}
		}

		g->grid()->copy(d_default_2D_grid);
		g->showMissingDataGap(d_show_empty_cell_gap);

		g->updateSecondaryAxis(QwtPlot::xTop);
		g->updateSecondaryAxis(QwtPlot::yRight);

		QList<int> ticksList;
		ticksList<<majTicksStyle<<majTicksStyle<<majTicksStyle<<majTicksStyle;
		g->setMajorTicksType(ticksList);
		ticksList.clear();
		ticksList<<minTicksStyle<<minTicksStyle<<minTicksStyle<<minTicksStyle;
		g->setMinorTicksType(ticksList);

		g->setTicksLength (minTicksLength, majTicksLength);
		g->setAxesLinewidth(axesLineWidth);
		g->drawAxesBackbones(drawBackbones);
		g->setCanvasFrame(canvasFrameWidth, d_canvas_frame_color);
		for (int i = 0; i < QwtPlot::axisCnt; i++)
			g->setAxisTitleDistance(i, d_graph_axes_labels_dist);

		g->enableSpeedMode(d_decimation_method, d_speed_mode_points, d_Douglas_Peuker_tolerance, false);
	}

	g->setAxisTitlePolicy(d_graph_axis_labeling);
	g->setSynchronizedScaleDivisions(d_synchronize_graph_scales);
	g->initFonts(plotAxesFont, plotNumbersFont);
	g->initTitle(titleOn, plotTitleFont);

	g->setContentsMargins(defaultPlotMargin, defaultPlotMargin, defaultPlotMargin, defaultPlotMargin);
	g->enableAutoscaling(autoscale2DPlots);
	g->setAutoscaleFonts(autoScaleFonts);
	g->setAntialiasing(antialiasing2DPlots);
	g->disableCurveAntialiasing(d_disable_curve_antialiasing, d_curve_max_antialising_size);
	g->setFrame(d_graph_border_width, d_graph_border_color);

	QColor c = d_graph_background_color;
	c.setAlphaF(0.01*d_graph_background_opacity);
	g->setBackgroundColor(c);

	c = d_graph_canvas_color;
	c.setAlphaF(0.01*d_graph_canvas_opacity);
	g->setCanvasBackground(c);
}

void PlotController2D::setGraphDefaultSettings(bool autoscale, bool scaleFonts,
												bool resizeLayers, bool antialiasing)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &antialiasing2DPlots = d_app->antialiasing2DPlots;
	auto &autoResizeLayers = d_app->autoResizeLayers;
	auto &autoScaleFonts = d_app->autoScaleFonts;
	auto &autoscale2DPlots = d_app->autoscale2DPlots;

	if (autoscale2DPlots == autoscale &&
		autoScaleFonts == scaleFonts &&
		autoResizeLayers != resizeLayers &&
		antialiasing2DPlots == antialiasing)
		return;

	autoscale2DPlots = autoscale;
	autoScaleFonts = scaleFonts;
	autoResizeLayers = !resizeLayers;
	antialiasing2DPlots = antialiasing;

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		MultiLayer *ml = qobject_cast<MultiLayer*>(w);
		if (!ml)
			continue;

		ml->setScaleLayersOnResize(autoResizeLayers);
		QList<Graph *> layers = ml->layersList();
		for (Graph *g : layers){
			g->enableAutoscaling(autoscale2DPlots);
			g->updateScale();
			g->setAutoscaleFonts(autoScaleFonts);
			g->setAntialiasing(antialiasing2DPlots);
		}
	}
}

void PlotController2D::setArrowDefaultSettings(double lineWidth,  const QColor& c, Qt::PenStyle style,
		int headLength, int headAngle, bool fillHead)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultArrowColor = d_app->defaultArrowColor;
	auto &defaultArrowHeadAngle = d_app->defaultArrowHeadAngle;
	auto &defaultArrowHeadFill = d_app->defaultArrowHeadFill;
	auto &defaultArrowHeadLength = d_app->defaultArrowHeadLength;
	auto &defaultArrowLineStyle = d_app->defaultArrowLineStyle;
	auto &defaultArrowLineWidth = d_app->defaultArrowLineWidth;

	if (defaultArrowLineWidth == lineWidth &&
		defaultArrowColor == c &&
		defaultArrowLineStyle == style &&
		defaultArrowHeadLength == headLength &&
		defaultArrowHeadAngle == headAngle &&
		defaultArrowHeadFill == fillHead)
		return;

	defaultArrowLineWidth = lineWidth;
	defaultArrowColor = c;
	defaultArrowLineStyle = style;
	defaultArrowHeadLength = headLength;
	defaultArrowHeadAngle = headAngle;
	defaultArrowHeadFill = fillHead;
	d_app->saveSettings();
}

void PlotController2D::plotVerticalSharedAxisLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(1, 2, defaultCurveStyle, MultiLayer::AlignCanvases);
}

void PlotController2D::plotHorizontalSharedAxisLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(2, 1, defaultCurveStyle, MultiLayer::AlignCanvases);
}

void PlotController2D::plotSharedAxesLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(2, 2, defaultCurveStyle, MultiLayer::AlignCanvases);
}

void PlotController2D::plotStackSharedAxisLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(1, -1, defaultCurveStyle, MultiLayer::AlignCanvases);
}

void PlotController2D::plotCustomLayout(bool sharedAxes)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_layer_canvas_height = d_app->d_layer_canvas_height;
	auto &d_layer_canvas_width = d_app->d_layer_canvas_width;
	auto &d_layer_geometry_unit = d_app->d_layer_geometry_unit;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t || !validFor2DPlot(t, (Graph::CurveType)defaultCurveStyle))
		return;

	QStringList list = t->drawableColumnSelection();
	int curves = list.count();
	if(curves < 1){
		QMessageBox::warning(d_app, tr("QtiPlot - Plot error"), tr("Please select a Y column to plot!"));
		return;
	}

	LayerDialog *id = new LayerDialog(d_app, true);
	id->setLayers(curves);
	id->setRows(curves);
	id->setMargins(5, 5, 5, 5);
	id->setLayerCanvasSize(d_layer_canvas_width, d_layer_canvas_height, d_layer_geometry_unit);
	if (sharedAxes)
		id->setSharedAxes();
	id->exec();
}

void PlotController2D::plot2VerticalLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(1, 2, defaultCurveStyle);
}

void PlotController2D::plot2HorizontalLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(2, 1, defaultCurveStyle);
}

void PlotController2D::plot4Layers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(2, 2, defaultCurveStyle);
}

void PlotController2D::plotStackedLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &defaultCurveStyle = d_app->defaultCurveStyle;

	multilayerPlot(1, -1, defaultCurveStyle);
}

void PlotController2D::plotStackedHistograms()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	multilayerPlot(1, -1, Graph::Histogram);
}

void PlotController2D::zoomRectanglePlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

    QStringList lst = t->selectedYColumns();
    int cols = lst.size();
	if (cols < 1){
		QMessageBox::critical(d_app, tr("QtiPlot - Error"),
		tr("You need to select at least one Y column for d_app operation!"));
		return;
	}

	QTableWidgetSelectionRange sel = t->getSelection();
    MultiLayer *ml = multilayerPlot(t, lst, Graph::LineSymbols, sel.topRow(), sel.bottomRow());
    if (ml){
        Graph *ag = ml->activeLayer();
        ag->setTitle("");
        ag->setAxisTitle(QwtPlot::xBottom, " ");
        ag->setAxisTitle(QwtPlot::yLeft, " ");
        ag->setCanvasFrame();
        ag->drawAxesBackbones(false);
        ag->showGrid();
        ag->removeLegend();

        RectangleWidget *r = new RectangleWidget(ag);
        QColor c = Qt::yellow;
        c.setAlpha(100);
        r->setBackgroundColor(c);
        r->setFrameColor(Qt::blue);
        ag->add(r, false);

		Graph *g = ml->addLayer();
        if (!g)
            return;

		setPreferences(g);
        g->setTitle("");
        g->setAxisTitle(QwtPlot::xBottom, " ");
        g->setAxisTitle(QwtPlot::yLeft, " ");
        g->copyCurves(ag);
        g->drawAxesBackbones(false);
        g->setCanvasFrame();
        g->setCanvasBackground(c);

        ml->setRows(2);
        ml->setCols(1);
        ml->arrangeLayers(false);

        QRect canvasRect = ag->canvas()->geometry();
        r->setRect(canvasRect.x(), canvasRect.bottom() - 50, 100, 50);

        g->setCanvasCoordinates(r->boundingRect());
        r->setLinkedLayer(1);
    }
}

void PlotController2D::plotDoubleYAxis()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

	QStringList lst = t->selectedYColumns();
	int cols = lst.size();
	if (cols < 2){
		QMessageBox::critical(d_app, tr("QtiPlot - Error"),
		tr("You need at least two columns for d_app operation!"));
		return;
	}

	QTableWidgetSelectionRange sel = t->getSelection();
	MultiLayer *ml = multilayerPlot(t, lst, Graph::LineSymbols, sel.topRow(), sel.bottomRow());
	if (ml){
		Graph *g = ml->activeLayer();
		g->enableAxis(QwtPlot::yRight);
		QwtPlotCurve *c = g->curve(cols - 1);
		if (c){
			c->setYAxis(QwtPlot::yRight);
			g->setAutoScale();
		}
		g->updateAxisTitle(QwtPlot::yRight);
		g->setSynchronizedScaleDivisions(false);
	}
}

MultiLayer* PlotController2D::newFunctionPlot(QStringList &formulas, double start, double end, int points, const QString& var, int type)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	MultiLayer *ml = newGraph();
	if (ml){
		Graph *g = ml->activeLayer();
		if (g){
			g->enableAutoscaling();
			g->addFunction(formulas, start, end, points, var, type);
			g->enableAutoscaling(false);
		}
	}

	d_app->updateFunctionLists(type, formulas);
	return ml;
}

void PlotController2D::autoArrangeLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	plot->setMargins(5, 5, 5, 5);
	//plot->setSpacing(5, 5);
	plot->arrangeLayers(true, false);

	if (plot->isWaterfallPlot())
		plot->updateWaterfalls();
}

void PlotController2D::extractGraphs()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

    if (plot->numLayers() < 2){
        QMessageBox::critical(d_app, tr("QtiPlot - Error"),
        tr("You must have more than one layer in the active window!"));
		return;
    }

    QList<Graph *> lst = plot->layersList();
    for (Graph *g : lst){
		MultiLayer *nw = multilayerPlot(d_app->generateUniqueName(tr("Graph")), 0, plot->getRows(), plot->getCols());
        nw->resize(plot->size());
		Graph *ng = nw->addLayer(g->pos().x(), g->pos().y(), g->width(), g->height());
		if (ng)
            ng->copy(g);
    }
}

void PlotController2D::extractLayers()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph *g = d_app->activePlotLayer(false);
	if (!g)
		return;
	MultiLayer *plot = g->multiLayer();
	if (!plot)
		return;

	int curves = g->curveCount();
	if (curves < 2){
		QMessageBox::critical(d_app, tr("QtiPlot - Error"),
		tr("You must have more than one dataset in the active layer!"));
		return;
	}

	for(int i = 0; i < curves; i++){
		Graph *ng = plot->addLayer(g->pos().x(), g->pos().y(), g->width(), g->height());
		if (ng){
			ng->copy(g);
			for(int j = 0; j < curves; j++){
				if (j != i)
					ng->removeCurve(j);
			}
		}
	}
	plot->removeLayer(g);
	plot->arrangeLayers(true, false);
}

void PlotController2D::addInsetLayer(bool curves)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

    Graph *al = plot->activeLayer();
    if (!al)
        return;

    QRect r = al->geometry();
	Graph *g = plot->addLayer(r.x() + r.width()/2, al->canvas()->y(), r.width()/2, r.height()/2, true);
    if (g){
        g->setTitle("");
        g->setAxisTitle(QwtPlot::xBottom, "");
        g->setAxisTitle(QwtPlot::yLeft, "");
        g->enableAxis(QwtPlot::yRight, false);
        g->enableAxis(QwtPlot::xTop, false);

        QColor c = Qt::white;
        c.setAlpha(0);
        g->setBackgroundColor(c);
        g->setCanvasBackground(c);
        if (curves)
            g->copyCurves(al);
    }
}

void PlotController2D::addInsetCurveLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    addInsetLayer(true);
}

void PlotController2D::addLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	if (plot->numLayers() == 0){
		setPreferences(plot->addLayer());
		return;
	}

	switch(QMessageBox::information(d_app,
				tr("QtiPlot - Guess best origin for the new layer?"),
				tr("Do you want QtiPlot to guess the best position for the new layer?\n Warning: d_app will rearrange existing layers!"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel ) ){
		case QMessageBox::Yes:
				setPreferences(plot->addLayer());
				plot->arrangeLayers(true, true);
		break;

		case QMessageBox::No:
			setPreferences(plot->addLayer(0, 0, plot->canvasRect().width(), plot->canvasRect().height()));
		break;

		case QMessageBox::Cancel:
			return;
			break;
	}
}

void PlotController2D::deleteLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	plot->confirmRemoveLayer();
}

void PlotController2D::copyActiveLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &lastCopiedLayer = d_app->lastCopiedLayer;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph *g = plot->activeLayer();
	if (!g)
		return;

	lastCopiedLayer = g;
	connect(g, &QObject::destroyed, d_app, &ApplicationWindow::closedLastCopiedLayer);
	g->copyImage();
}

void PlotController2D::custom2DPlotTools(MultiLayer *plot)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &actionAddEllipse = d_app->actionAddEllipse;
	auto &actionAddFormula = d_app->actionAddFormula;
	auto &actionAddRectangle = d_app->actionAddRectangle;
	auto &actionAddText = d_app->actionAddText;
	auto &actionDragCurve = d_app->actionDragCurve;
	auto &actionDrawPoints = d_app->actionDrawPoints;
	auto &actionMagnify = d_app->actionMagnify;
	auto &actionMagnifyHor = d_app->actionMagnifyHor;
	auto &actionMagnifyVert = d_app->actionMagnifyVert;
	auto &btnArrow = d_app->btnArrow;
	auto &btnCursor = d_app->btnCursor;
	auto &btnLine = d_app->btnLine;
	auto &btnMovePoints = d_app->btnMovePoints;
	auto &btnPicker = d_app->btnPicker;
	auto &btnPointer = d_app->btnPointer;
	auto &btnRemovePoints = d_app->btnRemovePoints;
	auto &btnSelect = d_app->btnSelect;
	auto &btnZoomIn = d_app->btnZoomIn;

	if (!plot)
		return;

	actionAddText->setChecked(false);
	actionAddFormula->setChecked(false);
	actionAddRectangle->setChecked(false);
	actionAddEllipse->setChecked(false);

	if (plot->activeLayer())
		d_app->graphSelectionChanged(plot->activeLayer()->selectionMoveResizer());

	QList<Graph *> layers = plot->layersList();
    for (Graph *g : layers){
    	PlotToolInterface *active_tool = g->activeTool();
    	if (active_tool){
			if (active_tool->rtti() == PlotToolInterface::Rtti_PlotTool){
				btnPicker->setChecked(true);
				return;
			} else if (active_tool->rtti() == PlotToolInterface::Rtti_DataPicker){
				switch(((DataPickerTool *)active_tool)->mode()){
					case DataPickerTool::Display:
						btnCursor->setChecked(true);
					break;
					case DataPickerTool::Move:
						btnMovePoints->setChecked(true);
					break;
					case DataPickerTool::Remove:
						btnRemovePoints->setChecked(true);
					break;
					case DataPickerTool::MoveCurve:
						actionDragCurve->setChecked(true);
					break;
				}
				return;
			} else if (active_tool->rtti() == PlotToolInterface::Rtti_DrawDataPoints){
				actionDrawPoints->setChecked(true);
				return;
			} else if (active_tool->rtti() == PlotToolInterface::Rtti_AddWidgetTool){
				switch(((AddWidgetTool *)active_tool)->widgetType()){
					case AddWidgetTool::Text:
						actionAddText->setChecked(true);
					break;
					case AddWidgetTool::TexEquation:
						actionAddFormula->setChecked(true);
					break;
					case AddWidgetTool::Rectangle:
						actionAddRectangle->setChecked(true);
					break;
					case AddWidgetTool::Ellipse:
						actionAddEllipse->setChecked(true);
					break;
					default:
						break;
				}
				return;
			}
		} else if (g->hasPanningMagnifierEnabled()){
			QwtPlotMagnifier *magnifier = g->magnifyTool();
			if (!magnifier->isAxisEnabled(QwtPlot::xBottom) && !magnifier->isAxisEnabled(QwtPlot::xTop))
				actionMagnifyVert->setChecked(true);
			else if (!magnifier->isAxisEnabled(QwtPlot::yLeft) && !magnifier->isAxisEnabled(QwtPlot::yRight))
				actionMagnifyHor->setChecked(true);
			else
				actionMagnify->setChecked(true);
			return;
		} else if (g->drawArrow()){
			btnArrow->setChecked(true);
			return;
    	} else if (g->drawLineActive()){
			btnLine->setChecked(true);
			return;
    	} else if (g->rangeSelectorsEnabled()){
			btnSelect->setChecked(true);
			return;
		} else if (g->zoomOn()){
    		btnZoomIn->setChecked(true);
    		return;
    	}
    }
	btnPointer->setChecked(true);
}

void PlotController2D::connectMultilayerPlot(MultiLayer *g)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &btnPointer = d_app->btnPointer;
	auto &confirmClosePlot2D = d_app->confirmClosePlot2D;
	auto &d_undo_group = d_app->d_undo_group;
	auto &info = d_app->info;

	connect (g, &MultiLayer::showEnrichementDialog, d_app, &ApplicationWindow::showEnrichementDialog);
	connect (g, &MultiLayer::showCurvesDialog, d_app, &ApplicationWindow::showCurvesDialog);
	connect (g, &MultiLayer::drawLineEnded, btnPointer, &QAction::setChecked);
	connect (g, &MultiLayer::showMarkerPopupMenu, d_app, &ApplicationWindow::showMarkerPopupMenu);
	connect (g, &MultiLayer::closedWindow, d_app, &ApplicationWindow::closeWindow);
	connect (g, &MultiLayer::hiddenWindow, d_app, qOverload<MdiSubWindow*>(&ApplicationWindow::hideWindow));
	connect (g, &MultiLayer::statusChanged, d_app, &ApplicationWindow::updateWindowStatus);
	connect (g, &MultiLayer::cursorInfo, info, &QLineEdit::setText);
	connect (g, &MultiLayer::modifiedWindow, d_app, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect (g, &MultiLayer::modifiedPlot, d_app, qOverload<>(&ApplicationWindow::modifiedProject));
	connect (g, &MultiLayer::showLineDialog, d_app, &ApplicationWindow::showLineDialog);
	connect (g, &MultiLayer::pasteMarker, d_app, &ApplicationWindow::pasteSelection);
	connect (g, &MultiLayer::setPointerCursor, d_app, &ApplicationWindow::pickPointerCursor);
	connect (g, &MultiLayer::currentFontChanged, d_app, &ApplicationWindow::setFormatBarFont);
	connect (g, &MultiLayer::currentColorChanged, d_app, &ApplicationWindow::setFormatBarColor);

	g->askOnCloseEvent(confirmClosePlot2D);

	if (d_undo_group)
		d_undo_group->addStack(g->undoStack());
}

MultiLayer* PlotController2D::plotGrayScale(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!m) {
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

	return plotSpectrogram(m, Graph::GrayScale);
}

MultiLayer* PlotController2D::plotContour(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!m) {
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

	return plotSpectrogram(m, Graph::Contour);
}

MultiLayer* PlotController2D::plotColorMap(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!m) {
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

	return plotSpectrogram(m, Graph::ColorMap);
}

MultiLayer* PlotController2D::plotImage(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

    if (!m) {
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MultiLayer* g = multilayerPlot(d_app->generateUniqueName(tr("Graph")));
	Graph* plot = g->activeLayer();
	setPreferences(plot);
	Spectrogram *s = plot->plotSpectrogram(m, Graph::GrayScale);
	if (!s)
		return 0;

	s->setAxes(QwtPlot::xTop, QwtPlot::yLeft);
	plot->enableAxis(QwtPlot::xTop, true);
	plot->setScale(QwtPlot::xTop, qMin(m->xStart(), m->xEnd()), qMax(m->xStart(), m->xEnd()));
	plot->setScale(QwtPlot::xBottom, qMin(m->xStart(), m->xEnd()), qMax(m->xStart(), m->xEnd()));
	plot->enableAxis(QwtPlot::xBottom, false);
	plot->enableAxis(QwtPlot::yRight, false);
	plot->setScale(QwtPlot::yLeft, qMin(m->yStart(), m->yEnd()), qMax(m->yStart(), m->yEnd()),
					0.0, 5, 5, Graph::Linear, true);
	plot->setAxisTitle(QwtPlot::yLeft, QString());
	plot->setAxisTitle(QwtPlot::xTop, QString());
	plot->setTitle(QString());

	g->arrangeLayers(false, true);

	emit d_app->modified();
	QApplication::restoreOverrideCursor();
	return g;
}

MultiLayer* PlotController2D::plotSpectrogram(Matrix *m, Graph::CurveType type)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (type == Graph::ImagePlot)
		return plotImage(m);
	else if (type == Graph::Histogram)
		return plotHistogram(m);

	if (!m)
		return 0;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	MultiLayer* g = multilayerPlot(d_app->generateUniqueName(tr("Graph")));
	Graph* plot = g->activeLayer();
	setPreferences(plot);

	Spectrogram *sp = plot->plotSpectrogram(m, type);
	if (sp && type == Graph::ColorMap)
		sp->setCustomColorMap(m->colorMap());

	g->arrangeLayers(false, true);
	QApplication::restoreOverrideCursor();
	return g;
}

MultiLayer* PlotController2D::plotImageProfiles(Matrix *m)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

    if (!m) {
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MultiLayer* g = multilayerPlot(d_app->generateUniqueName(tr("Profiles")), 0);
    g->resize(650, 600);
    g->plotProfiles(m);

	Table *horTable = d_app->newHiddenTable(tr("Horizontal"), QString(), m->numCols(), 2);
    Table *verTable = d_app->newHiddenTable(tr("Vertical"), QString(), m->numRows(), 2);

	Graph *sg = g->layer(1);
	if (sg){
		ImageProfilesTool *ipt = new ImageProfilesTool(d_app, sg, m, horTable, verTable);
		ipt->connectPlotLayers();
		sg->setActiveTool(ipt);
	}

	QApplication::restoreOverrideCursor();
	return g;
}

bool PlotController2D::validFor2DPlot(Table *table, Graph::CurveType type)
{

	if (!d_app) return false;
	ApplicationWindow *app = d_app;
	auto &Box = d_app->Box;

	if (!table->selectedYColumns().count()){
  		QMessageBox::warning(d_app, tr("QtiPlot - Error"), tr("Please select a Y column to plot!"));
  	    return false;
  	} else if (type != Graph::Box && type != Graph::Histogram && type != Graph::Pie){
		if (table->numCols() < 2) {
			QMessageBox::critical(d_app, tr("QtiPlot - Error"),tr("You need at least two columns for d_app operation!"));
			return false;
		} else if (table->noXColumn()) {
			QMessageBox::critical(d_app, tr("QtiPlot - Error"), tr("Please set a default X column for d_app table, first!"));
			return false;
		}
	}
	return true;
}

MultiLayer* PlotController2D::generate2DGraph(Graph::CurveType type)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return 0;

    if (w->inherits("Table")){
        Table *table = static_cast<Table *>(w);
		QTableWidgetSelectionRange sel = table->getSelection();
        return multilayerPlot(table, table->drawableColumnSelection(), type, sel.topRow(), sel.bottomRow());
    } else if (w->inherits("Matrix")){
        Matrix *m = static_cast<Matrix *>(w);
        return plotHistogram(m);
    }
	return 0;
}

Note * PlotController2D::newStemPlot()
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	Table *t = (Table *)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return nullptr;

    int ts = t->table()->currentSelection();
    if (ts < 0)
		return nullptr;

	Note *n = d_app->newNote();
	if (!n)
		return nullptr;
	n->hide();

	ScriptEdit* editor = n->currentEditor();
	QStringList lst = t->selectedColumns();
	if (lst.isEmpty()){
		QTableWidgetSelectionRange sel = t->getSelection();
			for (int i = sel.leftColumn(); i <= sel.rightColumn(); i++)
				editor->insertPlainText(stemPlot(t, t->colName(i), 1001, sel.topRow() + 1, sel.bottomRow() + 1) + "\n");
	} else {
		for (int i = 0; i < lst.count(); i++)
			editor->insertPlainText(stemPlot(t, lst[i], 1001) + "\n");
	}

	n->show();
	return n;
}

QString PlotController2D::stemPlot(Table *t, const QString& colName, int power, int startRow, int endRow)
{

	if (!d_app) return QString();
	ApplicationWindow *app = d_app;
	auto &ceil = d_app->ceil;

	if (!t)
		return QString();

	int col = t->colIndex(colName);
	if (col < 0){
		QMessageBox::critical(d_app, tr("QtiPlot - Error"),
		tr("Data set: %1 doesn't exist!").arg(colName));
		return QString();
	}

	startRow--;
	endRow--;
	if (startRow < 0 || startRow >= t->numRows())
		startRow = 0;
	if (endRow < 0 || endRow >= t->numRows())
		endRow = t->numRows() - 1;

	QString result = tr("Stem and leaf plot of dataset") + ": " + colName + " ";
	result += tr("from row") + ": " + QString::number(startRow + 1) + " ";
	result += tr("to row") + ": " + QString::number(endRow + 1) + "\n";

	int rows = 0;
	for (int j = startRow; j <= endRow; j++){
		if (!t->text(j, col).isEmpty())
		   rows++;
	}

	if (rows >= 1){
		double *data = (double *)malloc(rows * sizeof (double));
		if (!data){
			result += tr("Not enough memory for d_app dataset!") + "\n";
			return result;
		}

		result += "\n" + tr("Stem") + " | " + tr("Leaf");
		result += "\n---------------------\n";

		int row = 0;
		for (int j = startRow; j <= endRow; j++){
			if (!t->text(j, col).isEmpty()){
				data[row] = t->cell(j, col);
				row++;
			}
		}
		gsl_sort (data, 1, rows);

		if (power > 1e3){
			power = std::ceil(log10(data[rows - 1] - data[0]) - log10(rows - 1));
			bool ok;
			int input = QInputDialog::getInt(d_app, tr("Please confirm the stem unit!"),
                                      tr("Data set") + ": " + colName + ", " + tr("stem unit") + " = 10<sup>n</sup>, n = ",
                                      power, -1000, 1000, 1, &ok);
			if (ok)
				power = input;
		}

		double stem_unit = pow(10.0, power);
		double leaf_unit = stem_unit/10.0;

		int prev_stem = int(data[0]/stem_unit);
		result += "      " + QString::number(prev_stem) + " | ";

		for (int j = 0; j <rows; j++){
			double val = data[j];
			int stem = int(val/stem_unit);
			int leaf = int(qRound((val - stem*stem_unit)/leaf_unit));
			for (int k = prev_stem + 1; k < stem + 1; k++)
			  result += "\n      " + QString::number(k) + " | ";
			result += QString::number(leaf);
			prev_stem = stem;
		}

		result += "\n---------------------\n";
		result += tr("Stem unit") + ": " + d_app->locale().toString(stem_unit) + "\n";
		result += tr("Leaf unit") + ": " + d_app->locale().toString(leaf_unit) + "\n";

		QString legend = tr("Key") + ": " + QString::number(prev_stem) + " | ";
		int leaf = int(qRound((data[rows - 1] - prev_stem*stem_unit)/leaf_unit));
		legend += QString::number(leaf);
		legend += " " + tr("means") + ": " + d_app->locale().toString(prev_stem*stem_unit + leaf*leaf_unit) + "\n";

		result += legend + "---------------------\n";
		free(data);
	} else
		result += "\t" + tr("Input error: empty data set!") + "\n";
	return result;
}
