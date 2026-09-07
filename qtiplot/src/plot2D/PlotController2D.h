/***************************************************************************
    File                 : PlotController2D.h
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

#ifndef PLOT_CONTROLLER_2D_H
#define PLOT_CONTROLLER_2D_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPen>
#include "Graph.h"
#include "MultiLayer.h"

class ApplicationWindow;
class Table;
class Matrix;
class Note;

class PlotController2D : public QObject
{
    Q_OBJECT

public:
    explicit PlotController2D(ApplicationWindow *app);
    virtual ~PlotController2D();

    MultiLayer* newGraph(const QString& caption = tr("Graph") + "1");
    MultiLayer* multilayerPlot(int c, int r, int style, const MultiLayer::AlignPolicy& align = MultiLayer::AlignLayers);
    MultiLayer* multilayerPlot(Table* w, const QStringList& colList, int style, int startRow = 0, int endRow = -1);
    MultiLayer* multilayerPlot(const QString& caption, int layers = 1, int rows = 1, int cols = 1);
    MultiLayer* waterfallPlot();
    MultiLayer* waterfallPlot(Table *t, const QStringList& list);
    void connectMultilayerPlot(MultiLayer *g);
    void addLayer();
    void addInsetLayer(bool curves = false);
    void addInsetCurveLayer();
    void deleteLayer();
    void extractGraphs();
    void extractLayers();
    MultiLayer* plotSpectrogram(Matrix *m, Graph::CurveType type);
    MultiLayer* plotGrayScale(Matrix *m = 0);
    MultiLayer* plotContour(Matrix *m = 0);
    MultiLayer* plotColorMap(Matrix *m = 0);
    MultiLayer* plotImage(Matrix *m = 0);
    MultiLayer* plotImageProfiles(Matrix *m = 0);
    void autoArrangeLayers();
    void initMultilayerPlot(MultiLayer* g, const QString& name = QString());
    void plot2VerticalLayers();
    void plot2HorizontalLayers();
    void plot4Layers();
    void plotStackedLayers();
    void plotStackedHistograms();
    void plotStackSharedAxisLayers();
    void plotVerticalSharedAxisLayers();
    void plotHorizontalSharedAxisLayers();
    void plotSharedAxesLayers();
    void plotCustomLayout(bool sharedAxes = false);
    void plotCustomLayoutSharedAxes();
    MultiLayer * newFunctionPlot(QStringList &formulas, double start, double end, int points = 100, const QString& var = "x", int type = 0);
    void setPreferences(Graph* g);
    void setGraphDefaultSettings(bool autoscale,bool scaleFonts,bool resizeLayers,bool antialiasing);
    void setArrowDefaultSettings(double lineWidth,  const QColor& c, Qt::PenStyle style, int headLength, int headAngle, bool fillHead);
    void plotL();
    void plotP();
    void plotLP();
    void plotPie();
    void plotVerticalBars();
    void plotHorizontalBars();
    void plotStackBar();
    void plotStackColumn();
    void plotArea();
    void plotVertSteps();
    void plotHorSteps();
    void plotSpline();
    void plotVerticalDropLines();
    MultiLayer* plotHistogram();
    MultiLayer* plotHistogram(Matrix *m);
    void plotVectXYXY();
    void plotVectXYAM();
    void plotBox();
    void plotDoubleYAxis();
    void zoomRectanglePlot();
    QString stemPlot(Table *t = 0, const QString& colName = QString(), int power = 0, int startRow = 0, int endRow = -1);
    Note *newStemPlot();
    bool validFor2DPlot(Table *table, Graph::CurveType type);
    MultiLayer* generate2DGraph(Graph::CurveType type);
    void copyActiveLayer();
    void custom2DPlotTools(MultiLayer *);

private:
    ApplicationWindow *d_app;
};

#endif // PLOT_CONTROLLER_2D_H
