/***************************************************************************
    File                 : PlotController3D.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 3D plot creation, surfaces, and visual state controller

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

#ifndef PLOT_CONTROLLER_3D_H
#define PLOT_CONTROLLER_3D_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>

class ApplicationWindow;
class Table;
class Matrix;
class Graph3D;
class PolarGraph;
class MdiSubWindow;
class QMdiSubWindow;
class QAction;

class PlotController3D : public QObject
{
    Q_OBJECT

public:
    explicit PlotController3D(ApplicationWindow *app);
    virtual ~PlotController3D();

    Graph3D* newPlot3D(const QString& title = QString());
    Graph3D* plotXYZ(Table* table,const QString& zColName, int type);
    MdiSubWindow* newPolarPlot(const QString& title = QString());
    MdiSubWindow* plotPolar(Table* table, const QStringList& colList, int startRow = 0, int endRow = -1);
    void plotPolar();
    Graph3D* plotSurface(const QString& formula, double xl, double xr, double yl, double yr, double zl, double zr, int columns = 40, int rows = 30);
    Graph3D* plotParametricSurface(const QString& xFormula, const QString& yFormula, const QString& zFormula, double ul, double ur, double vl, double vr, int columns, int rows, bool uPeriodic, bool vPeriodic);
    void connectSurfacePlot(Graph3D *plot);
    void newSurfacePlot();
    void editSurfacePlot();
    void remove3DMatrixPlots(Matrix *m);
    void updateMatrixPlots(Matrix *);
    void updateMatrixPlotLabels(Matrix *);
    void add3DData();
    void change3DData();
    void change3DData(const QString& colName);
    void change3DMatrix();
    void change3DMatrix(const QString& matrix_name);
    void insertNew3DData(const QString& colName);
    void add3DMatrixPlot();
    void insert3DMatrixPlot(const QString& matrix_name);
    void plot3DWireframe();
    void plot3DHiddenLine();
    void plot3DPolygons();
    void plot3DWireSurface();
    Graph3D* plot3DMatrix(Matrix *m = 0, int style = 5);
    void plot3DRibbon();
    void plot3DScatter();
    void plot3DTrajectory();
    void plot3DBars();
    void initPolarPlot(PolarGraph *w);
    bool validFor3DPlot(Table *table);
    void toggle3DAnimation(bool on = true);
    void togglePerspective(bool on = true);
    void resetRotation();
    void fitFrameToLayer();
    void setFramed3DPlot();
    void setBoxed3DPlot();
    void removeAxes3DPlot();
    void removeGrid3DPlot();
    void setHiddenLineGrid3DPlot();
    void setLineGrid3DPlot();
    void setPoints3DPlot();
    void setCrosses3DPlot();
    void setCones3DPlot();
    void setBars3DPlot();
    void setFilledMesh3DPlot();
    void setEmptyFloor3DPlot();
    void setFloorData3DPlot();
    void setFloorIso3DPlot();
    void setFloorGrid3DPlot(bool on);
    void setCeilGrid3DPlot(bool on);
    void setRightGrid3DPlot(bool on);
    void setLeftGrid3DPlot(bool on);
    void setFrontGrid3DPlot(bool on);
    void setBackGrid3DPlot(bool on);
    void pickPlotStyle( QAction* action );
    void pickCoordSystem( QAction* action);
    void pickFloorStyle( QAction* action);
    void custom3DActions(QMdiSubWindow *w);
    void custom3DGrids(int grids);
    void initPlot3D(Graph3D *plot);
    void setPlot3DOptions();

private:
    ApplicationWindow *d_app;
};

#endif // PLOT_CONTROLLER_3D_H
