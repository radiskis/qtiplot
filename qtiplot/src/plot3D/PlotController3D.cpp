/***************************************************************************
    File                 : PlotController3D.cpp
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

#include "PlotController3D.h"
#include "ApplicationWindow.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph3D.h"
#include "PolarGraph.h"
#include "Folder.h"
#include "SurfaceDialog.h"
#include "Plot3DDialog.h"
#include "Spectrogram.h"
#include "PlotCurve.h"
#include "QwtHistogram.h"
#include "qwt3d_coordsys.h"
#include <QApplication>
#include <QMessageBox>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QToolBar>
#include <QInputDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QColor>
#include <QFileInfo>

PlotController3D::PlotController3D(ApplicationWindow *app)
    : QObject(app), d_app(app)
{
}

PlotController3D::~PlotController3D()
{
}

void PlotController3D::plot3DRibbon()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!w)
		return;

	Table *table = static_cast<Table*>(w);
	if(table->selectedColumns().count() == 1){
		if (!validFor3DPlot(table))
			return;
		plotXYZ(table, table->colName(table->selectedColumn()), Graph3D::Ribbon);
	} else
		QMessageBox::warning(d_app, d_app->tr("QtiPlot - Plot error"), d_app->tr("You must select exactly one column for plotting!"));
}

void PlotController3D::plot3DWireframe()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	plot3DMatrix(0, Qwt3D::WIREFRAME);
}

void PlotController3D::plot3DHiddenLine()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	plot3DMatrix(0, Qwt3D::HIDDENLINE);
}

void PlotController3D::plot3DPolygons()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	plot3DMatrix(0, Qwt3D::FILLED);
}

void PlotController3D::plot3DWireSurface()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	plot3DMatrix(0, Qwt3D::FILLEDMESH);
}

void PlotController3D::plot3DBars()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow();
    if (!w)
		return;

	if (w->inherits("Table")){
		Table *table = static_cast<Table *>(w);
		if (!validFor3DPlot(table))
			return;

		if(table->selectedColumns().count() == 1)
			plotXYZ(table, table->colName(table->selectedColumn()), Graph3D::Bars);
		else
			QMessageBox::warning(d_app, d_app->tr("QtiPlot - Plot error"),d_app->tr("You must select exactly one column for plotting!"));
	}
	else if(w->inherits("Matrix"))
		plot3DMatrix(0, Qwt3D::USER);
}

void PlotController3D::plot3DScatter()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if (w->inherits("Table"))
	{
		Table *table = static_cast<Table *>(w);
		if (!validFor3DPlot(table))
			return;

		if(table->selectedColumns().count() == 1)
			plotXYZ(table, table->colName(table->selectedColumn()), Graph3D::Scatter);
		else
			QMessageBox::warning(d_app, d_app->tr("QtiPlot - Plot error"),d_app->tr("You must select exactly one column for plotting!"));
	}
	else if(w->inherits("Matrix"))
		plot3DMatrix(0, Qwt3D::POINTS);
}

void PlotController3D::plot3DTrajectory()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *table = qobject_cast<Table *>(d_app->activeWindow(ApplicationWindow::TableWindow));
    if (!table)
		return;
    if (!validFor3DPlot(table))
        return;

    if(table->selectedColumns().count() == 1)
        plotXYZ(table, table->colName(table->selectedColumn()), Graph3D::Trajectory);
    else
        QMessageBox::warning(d_app, d_app->tr("QtiPlot - Plot error"), d_app->tr("You must select exactly one column for plotting!"));
}

void PlotController3D::remove3DMatrixPlots(Matrix *m)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Graph3D") && ((Graph3D*)w)->matrix() == m)
			((Graph3D*)w)->clearData();
		else if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers){
				bool update = false;
				QList<QwtPlotItem *> curvesList = g->curvesList();
				for (QwtPlotItem *it : curvesList){
					if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && ((Spectrogram *)it)->matrix() == m){
						g->removeCurve(it);
						update = true;
					} else if (((PlotCurve *)it)->rtti() == Graph::Histogram && ((QwtHistogram *)it)->matrix() == m){
						g->removeCurve(it);
						update = true;
					}
				}
				if (update)
					g->updatePlot();
			}
		}
	}
	QApplication::restoreOverrideCursor();
}

void PlotController3D::updateMatrixPlots(Matrix *m)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Graph3D") && ((Graph3D*)w)->matrix() == m)
			((Graph3D*)w)->updateMatrixData(m);
		else if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers){
				bool update = false;
				QList<QwtPlotItem *> curvesList = g->curvesList();
				for (QwtPlotItem *it : curvesList){
					if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
						Spectrogram *sp = (Spectrogram *)it;
						if (sp->matrix() == m){
							sp->updateData();
							update = true;
						}
					} else if (((PlotCurve *)it)->rtti() == Graph::Histogram){
						QwtHistogram *h = (QwtHistogram *)it;
						if (h->matrix() == m){
							h->loadData();
							update = true;
						}
					}
				}
				if (update)
					g->updatePlot();
			}
		}
	}
	QApplication::restoreOverrideCursor();
}

void PlotController3D::updateMatrixPlotLabels(Matrix *m)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		MultiLayer *plot2D = qobject_cast<MultiLayer *>(w);
		Graph3D *plot3D = qobject_cast<Graph3D *>(w);
		if (plot3D && plot3D->matrix() == m){
			plot3D->resetAxesLabels();
			plot3D->surface()->update();
		} else if (plot2D){
			QList<Graph *> layers = plot2D->layersList();
			for (Graph *g : layers){
				bool update = false;
				QList<QwtPlotItem *> curvesList = g->curvesList();
				for (QwtPlotItem *it : curvesList){
					if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
						Spectrogram *sp = (Spectrogram *)it;
						if (sp->matrix() == m){
							g->updateAxesTitles();
							update = true;
						}
					}
				}
				if (update)
					g->updatePlot();
			}
		}
	}

	d_app->modifiedProject();

	QApplication::restoreOverrideCursor();
}

void PlotController3D::add3DData()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	if (!d_app->hasTable()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no tables available in d_app project.</h4>"
					"<p><h4>Please create a table and try again!</h4>"));
		return;
	}

	QStringList zColumns = d_app->columnsList(Table::Z);
	if ((int)zColumns.count() <= 0){
		QMessageBox::critical(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("There are no available columns with plot designation set to Z!"));
		return;
	}

	bool ok;
	QString column = QInputDialog::getItem(d_app, d_app->tr("QtiPlot - Choose data set"),
									d_app->tr("Column") + ": ", zColumns, 0, false, &ok);
	if (ok && !column.isEmpty())
		insertNew3DData(column);
}

void PlotController3D::change3DData()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	bool ok;
	QString column = QInputDialog::getItem(d_app, d_app->tr("QtiPlot - Choose data set"),
									d_app->tr("Column") + ": ", d_app->columnsList(Table::Z), 0, false, &ok);
	if (ok && !column.isEmpty())
		change3DData(column);
}

void PlotController3D::change3DMatrix()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	QStringList matrices = d_app->matrixNames();
	int currentIndex = 0;
	Graph3D* g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (g && g->matrix())
		currentIndex = matrices.indexOf(g->matrix()->objectName());

	bool ok;
	QString matrixName = QInputDialog::getItem(d_app, d_app->tr("QtiPlot - Choose matrix to plot"),
							d_app->tr("Matrix") + ": ", matrices, currentIndex, false, &ok);
	if (ok && !matrixName.isEmpty())
		change3DMatrix(matrixName);
}

void PlotController3D::change3DMatrix(const QString& matrix_name)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_3D_autoscale = d_app->d_3D_autoscale;

	Graph3D *g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
    if (!g)
		return;

	Matrix *m = d_app->matrix(matrix_name);
	if (!m)
        return;

	if (d_3D_autoscale)
        g->addMatrixData(m);
    else
		g->addMatrixData(m, g->xStart(), g->xStop(), g->yStart(), g->yStop(), g->zStart(), g->zStop());

	emit d_app->modified();
}

void PlotController3D::add3DMatrixPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	QStringList matrices = d_app->matrixNames();
	if ((int)matrices.count() <= 0){
		QMessageBox::warning(d_app, d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no matrices available in d_app project.</h4>"
					"<p><h4>Please create a matrix and try again!</h4>"));
		return;
	}

	bool ok;
	QString matrixName = QInputDialog::getItem(d_app, d_app->tr("QtiPlot - Choose matrix to plot"),
							d_app->tr("Matrix") + ": ", matrices, 0, false, &ok);
	if (ok && !matrixName.isEmpty())
		insert3DMatrixPlot(matrixName);
}

void PlotController3D::insert3DMatrixPlot(const QString& matrix_name)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
    if (!g)
		return;

	g->addMatrixData(d_app->matrix(matrix_name));
	emit d_app->modified();
}

void PlotController3D::insertNew3DData(const QString& colName)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
    if (!g)
		return;

	g->insertNewData(d_app->table(colName),colName);
	emit d_app->modified();
}

void PlotController3D::change3DData(const QString& colName)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
    if (!g)
		return;

	g->changeDataColumn(d_app->table(colName), colName, g->tablePlotType());
	emit d_app->modified();
}

void PlotController3D::editSurfacePlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D*)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
    if (!g)
		return;

	SurfaceDialog* sd = new SurfaceDialog(d_app);
	sd->setAttribute(Qt::WA_DeleteOnClose);

	if (g->hasData() && g->userFunction())
		sd->setFunction(g);
	else if (g->hasData() && g->parametricSurface())
		sd->setParametricSurface(g);
	else
		sd->setGraph(g);
	sd->exec();
}

void PlotController3D::newSurfacePlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	SurfaceDialog* sd = new SurfaceDialog(d_app);
	sd->setAttribute(Qt::WA_DeleteOnClose);
	sd->exec();
}

Graph3D* PlotController3D::plotSurface(const QString& formula, double xl, double xr,
		double yl, double yr, double zl, double zr, int columns, int rows)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	Graph3D *plot = newPlot3D();
	if (!plot)
		return 0;
	plot->addFunction(formula, xl, xr, yl, yr, zl, zr, columns, rows);
	emit d_app->modified();
	return plot;
}

Graph3D* PlotController3D::plotParametricSurface(const QString& xFormula, const QString& yFormula,
		const QString& zFormula, double ul, double ur, double vl, double vr,
		int columns, int rows, bool uPeriodic, bool vPeriodic)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	Graph3D *plot = newPlot3D();
	if (!plot)
		return 0;
	plot->addParametricSurface(xFormula, yFormula, zFormula, ul, ur, vl, vr, columns, rows, uPeriodic, vPeriodic);
	emit d_app->modified();
	return plot;
}

Graph3D* PlotController3D::newPlot3D(const QString& title)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString label = title;
	if (label.isEmpty() || d_app->alreadyUsedName(label))
		label = d_app->generateUniqueName(d_app->tr("Graph"));

	Graph3D *plot = new Graph3D("", d_app, 0);
	plot->setWindowTitle(label);
	plot->setObjectName(label);

	initPlot3D(plot);

	emit d_app->modified();
	QApplication::restoreOverrideCursor();
	return plot;
}

Graph3D* PlotController3D::plotXYZ(Table* table, const QString& zColName, int type)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	Graph3D *plot = newPlot3D();
	if (!plot)
		return 0;

	int zCol = table->colIndex(zColName);
	if (type == Graph3D::Ribbon){
		int ycol = table->colY(zCol);
		plot->addRibbon(table, table->colName(table->colX(ycol)), zColName);
	} else
		plot->addData(table, table->colX(zCol), table->colY(zCol), zCol, type);

	emit d_app->modified();
	return plot;
}

MdiSubWindow* PlotController3D::newPolarPlot(const QString& title)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	PolarGraph* w = new PolarGraph(d_app->generateUniqueName(title.isEmpty() ? d_app->tr("Polar") : title), d_app);
	initPolarPlot(w);
	return w;
}

void PlotController3D::plotPolar()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *table = qobject_cast<Table *>(d_app->activeWindow(ApplicationWindow::TableWindow));
    if (!table)
		return;

	if (table->selectedColumns().count() < 2){
		QMessageBox::warning(d_app, d_app->tr("QtiPlot - Plot error"),
				d_app->tr("You must select at least two columns for plotting!"));
		return;
	}

	QStringList s = table->selectedColumns();
    plotPolar(table, s);
}

MdiSubWindow* PlotController3D::plotPolar(Table* table, const QStringList& colList, int startRow, int endRow)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

    if (!table || colList.size() < 2) return 0;
    
    PolarGraph* w = (PolarGraph*)newPolarPlot();

    // Check column designations: if one is X and one is Y, X is Theta and Y is Radius
    QString thetaCol = colList[0];
    QString rCol = colList[1];
    int col0Index = table->colIndex(colList[0]);
    int col1Index = table->colIndex(colList[1]);
    if (col0Index >= 0 && col1Index >= 0) {
        if (table->colPlotDesignation(col0Index) == Table::X && table->colPlotDesignation(col1Index) == Table::Y) {
            thetaCol = colList[0];
            rCol = colList[1];
        } else if (table->colPlotDesignation(col0Index) == Table::Y && table->colPlotDesignation(col1Index) == Table::X) {
            rCol = colList[0];
            thetaCol = colList[1];
        }
    }

    w->addCurve(table, rCol, thetaCol, startRow, endRow);
    return w;
}

void PlotController3D::initPlot3D(Graph3D *plot)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_mdi_windows_area = d_app->d_mdi_windows_area;
	auto &d_workspace = d_app->d_workspace;
	auto &plot3DTools = d_app->plot3DTools;

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(plot);
	else
		plot->setParent(0);

	connectSurfacePlot(plot);

	plot->setWindowIcon(QPixmap(":/trajectory.png"));
	plot->show();

	d_app->addListViewItem(plot);

	if (!plot3DTools->isVisible())
		plot3DTools->show();

	if (!plot3DTools->isEnabled())
		plot3DTools->setEnabled(true);

	d_app->windowActivated(plot);
}

void PlotController3D::initPolarPlot(PolarGraph *w)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_mdi_windows_area = d_app->d_mdi_windows_area;
	auto &d_workspace = d_app->d_workspace;

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(w);
	else
		w->setParent(0);

	w->setWindowIcon(QIcon(":/lpPlot.png"));
	w->show();

	d_app->addListViewItem(w);
	d_app->windowActivated(w);
}

void PlotController3D::setFramed3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &actionShowAxisDialog = d_app->actionShowAxisDialog;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFramed();
	actionShowAxisDialog->setEnabled(true);
}

void PlotController3D::setBoxed3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &actionShowAxisDialog = d_app->actionShowAxisDialog;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setBoxed();
	actionShowAxisDialog->setEnabled(true);
}

void PlotController3D::removeAxes3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &actionShowAxisDialog = d_app->actionShowAxisDialog;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setNoAxes();
	actionShowAxisDialog->setEnabled(false);
}

void PlotController3D::removeGrid3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setPolygonStyle();
}

void PlotController3D::setHiddenLineGrid3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setHiddenLineStyle();
}

void PlotController3D::setPoints3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setDotStyle();
}

void PlotController3D::setCones3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setConeStyle();
}

void PlotController3D::setCrosses3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setCrossStyle();
}

void PlotController3D::setBars3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setBarStyle();
}

void PlotController3D::setLineGrid3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setWireframeStyle();
}

void PlotController3D::setFilledMesh3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFilledMeshStyle();
}

void PlotController3D::setFloorData3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFloorData();
}

void PlotController3D::setFloorIso3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFloorIsolines();
}

void PlotController3D::setEmptyFloor3DPlot()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setEmptyFloor();
}

void PlotController3D::setFrontGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFrontGrid(on);
}

void PlotController3D::setBackGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setBackGrid(on);
}

void PlotController3D::setFloorGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setFloorGrid(on);
}

void PlotController3D::setCeilGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setCeilGrid(on);
}

void PlotController3D::setRightGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setRightGrid(on);
}

void PlotController3D::setLeftGrid3DPlot(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setLeftGrid(on);
}

void PlotController3D::pickPlotStyle( QAction* action )
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &barstyle = d_app->barstyle;
	auto &conestyle = d_app->conestyle;
	auto &crossHairStyle = d_app->crossHairStyle;
	auto &filledmesh = d_app->filledmesh;
	auto &hiddenline = d_app->hiddenline;
	auto &pointstyle = d_app->pointstyle;
	auto &polygon = d_app->polygon;
	auto &wireframe = d_app->wireframe;

	if (!action )
		return;

	if (action == polygon)
		removeGrid3DPlot();
	else if (action == filledmesh)
		setFilledMesh3DPlot();
	else if (action == wireframe)
		setLineGrid3DPlot();
	else if (action == hiddenline)
		setHiddenLineGrid3DPlot();
	else if (action == pointstyle)
		setPoints3DPlot();
	else if (action == conestyle)
		setCones3DPlot();
	else if (action == crossHairStyle)
		setCrosses3DPlot();
	else if (action == barstyle)
		setBars3DPlot();

	emit d_app->modified();
}

void PlotController3D::pickCoordSystem( QAction* action)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &Box = d_app->Box;
	auto &Frame = d_app->Frame;
	auto &None = d_app->None;
	auto &grids = d_app->grids;

	if (!action)
		return;

	if (action == Box || action == Frame)
	{
		if (action == Box)
			setBoxed3DPlot();
		if (action == Frame)
			setFramed3DPlot();
		grids->setEnabled(true);
	}
	else if (action == None)
	{
		removeAxes3DPlot();
		grids->setEnabled(false);
	}

	emit d_app->modified();
}

void PlotController3D::pickFloorStyle( QAction* action )
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &floordata = d_app->floordata;
	auto &flooriso = d_app->flooriso;

	if (!action)
		return;

	if (action == floordata)
		setFloorData3DPlot();
	else if (action == flooriso)
		setFloorIso3DPlot();
	else
		setEmptyFloor3DPlot();

	emit d_app->modified();
}

void PlotController3D::custom3DActions(QMdiSubWindow *w)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &Box = d_app->Box;
	auto &Frame = d_app->Frame;
	auto &None = d_app->None;
	auto &actionAnimate = d_app->actionAnimate;
	auto &actionPerspective = d_app->actionPerspective;
	auto &barstyle = d_app->barstyle;
	auto &conestyle = d_app->conestyle;
	auto &crossHairStyle = d_app->crossHairStyle;
	auto &filledmesh = d_app->filledmesh;
	auto &floordata = d_app->floordata;
	auto &flooriso = d_app->flooriso;
	auto &floornone = d_app->floornone;
	auto &grids = d_app->grids;
	auto &hiddenline = d_app->hiddenline;
	auto &pointstyle = d_app->pointstyle;
	auto &polygon = d_app->polygon;
	auto &wireframe = d_app->wireframe;

	if (w && w->inherits("Graph3D"))
	{
		Graph3D* plot = (Graph3D*)w;
		actionAnimate->setChecked(plot->isAnimated());
		actionPerspective->setChecked(!plot->isOrthogonal());
		switch(plot->plotStyle())
		{
			case FILLEDMESH:
				wireframe->setChecked( false );
				hiddenline->setChecked( false );
				polygon->setChecked( false );
				filledmesh->setChecked( true );
				pointstyle->setChecked( false );
				barstyle->setChecked( false );
				conestyle->setChecked( false );
				crossHairStyle->setChecked( false );
				break;

			case FILLED:
				wireframe->setChecked( false );
				hiddenline->setChecked( false );
				polygon->setChecked( true );
				filledmesh->setChecked( false );
				pointstyle->setChecked( false );
				barstyle->setChecked( false );
				conestyle->setChecked( false );
				crossHairStyle->setChecked( false );
				break;

			case Qwt3D::USER:
				wireframe->setChecked( false );
				hiddenline->setChecked( false );
				polygon->setChecked( false );
				filledmesh->setChecked( false );

				if (plot->pointType() == Graph3D::VerticalBars)
				{
					pointstyle->setChecked( false );
					conestyle->setChecked( false );
					crossHairStyle->setChecked( false );
					barstyle->setChecked( true );
				}
				else if (plot->pointType() == Graph3D::Dots)
				{
					pointstyle->setChecked( true );
					barstyle->setChecked( false );
					conestyle->setChecked( false );
					crossHairStyle->setChecked( false );
				}
				else if (plot->pointType() == Graph3D::HairCross)
				{
					pointstyle->setChecked( false );
					barstyle->setChecked( false );
					conestyle->setChecked( false );
					crossHairStyle->setChecked( true );
				}
				else if (plot->pointType() == Graph3D::Cones)
				{
					pointstyle->setChecked( false );
					barstyle->setChecked( false );
					conestyle->setChecked( true );
					crossHairStyle->setChecked( false );
				}
				break;

			case WIREFRAME:
				wireframe->setChecked( true );
				hiddenline->setChecked( false );
				polygon->setChecked( false );
				filledmesh->setChecked( false );
				pointstyle->setChecked( false );
				barstyle->setChecked( false );
				conestyle->setChecked( false );
				crossHairStyle->setChecked( false );
				break;

			case HIDDENLINE:
				wireframe->setChecked( false );
				hiddenline->setChecked( true );
				polygon->setChecked( false );
				filledmesh->setChecked( false );
				pointstyle->setChecked( false );
				barstyle->setChecked( false );
				conestyle->setChecked( false );
				crossHairStyle->setChecked( false );
				break;

			default:
				break;
		}

		switch(plot->coordStyle())
		{
			case Qwt3D::NOCOORD:
				None->setChecked( true );
				Box->setChecked( false );
				Frame->setChecked( false );
				break;

			case Qwt3D::BOX:
				None->setChecked( false );
				Box->setChecked( true );
				Frame->setChecked( false );
				break;

			case Qwt3D::FRAME:
				None->setChecked(false );
				Box->setChecked( false );
				Frame->setChecked(true );
				break;
		}

		switch(plot->floorStyle())
		{
			case NOFLOOR:
				floornone->setChecked( true );
				flooriso->setChecked( false );
				floordata->setChecked( false );
				break;

			case FLOORISO:
				floornone->setChecked( false );
				flooriso->setChecked( true );
				floordata->setChecked( false );
				break;

			case FLOORDATA:
				floornone->setChecked(false );
				flooriso->setChecked( false );
				floordata->setChecked(true );
				break;
		}
		custom3DGrids(plot->grids());
	}
}

void PlotController3D::custom3DGrids(int grids)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &back = d_app->back;
	auto &ceil = d_app->ceil;
	auto &floor = d_app->floor;
	auto &front = d_app->front;
	auto &left = d_app->left;
	auto &right = d_app->right;

	if (Qwt3D::BACK & grids)
		back->setChecked(true);
	else
		back->setChecked(false);

	if (Qwt3D::FRONT & grids)
		front->setChecked(true);
	else
		front->setChecked(false);

	if (Qwt3D::CEIL & grids)
		ceil->setChecked(true);
	else
		ceil->setChecked(false);

	if (Qwt3D::FLOOR & grids)
		floor->setChecked(true);
	else
		floor->setChecked(false);

	if (Qwt3D::RIGHT & grids)
		right->setChecked(true);
	else
		right->setChecked(false);

	if (Qwt3D::LEFT & grids)
		left->setChecked(true);
	else
		left->setChecked(false);
}

void PlotController3D::connectSurfacePlot(Graph3D *plot)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &confirmClosePlot3D = d_app->confirmClosePlot3D;

	connect (plot, &Graph3D::showContextMenu, d_app, &ApplicationWindow::showWindowContextMenu);
	connect (plot, &Graph3D::showOptionsDialog, d_app, &ApplicationWindow::showPlot3dDialog);
	connect (plot, &Graph3D::closedWindow, d_app, &ApplicationWindow::closeWindow);
	connect (plot, &Graph3D::hiddenWindow, d_app, qOverload<MdiSubWindow*>(&ApplicationWindow::hideWindow));
	connect (plot, &Graph3D::statusChanged, d_app, &ApplicationWindow::updateWindowStatus);
	connect(plot, &Graph3D::modified, d_app, &ApplicationWindow::modified);

	plot->askOnCloseEvent(confirmClosePlot3D);
}

void PlotController3D::setPlot3DOptions()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_3D_autoscale = d_app->d_3D_autoscale;
	auto &d_3D_smooth_mesh = d_app->d_3D_smooth_mesh;

	for (MdiSubWindow *w : d_app->windowsList()){
		Graph3D *g = qobject_cast<Graph3D*>(w);
		if (g){
			g->setAutoscale(d_3D_autoscale);
			g->setAntialiasing(d_3D_smooth_mesh);
		}
	}
}

Graph3D * PlotController3D::plot3DMatrix(Matrix *m, int style)
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

	if (!m){
		m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
		if (!m)
			return 0;
	}

	Graph3D *plot = newPlot3D();
	if (!plot)
		return 0;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	plot->addMatrixData(m);
	plot->customPlotStyle(style);

	custom3DActions(plot);
	emit d_app->modified();
	QApplication::restoreOverrideCursor();
	return plot;
}

void PlotController3D::toggle3DAnimation(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->animate(on);
}

void PlotController3D::togglePerspective(bool on)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setOrthogonal(!on);
}

void PlotController3D::resetRotation()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->setRotation(30, 0, 15);
}

void PlotController3D::fitFrameToLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Graph3D *g = (Graph3D *)d_app->activeWindow(ApplicationWindow::Plot3DWindow);
	if (!g)
		return;

	g->findBestLayout();
}

bool PlotController3D::validFor3DPlot(Table *table)
{

	if (!d_app) return false;
	ApplicationWindow *app = d_app;

	if (table->numCols()<2){
		QMessageBox::critical(0,d_app->tr("QtiPlot - Error"),d_app->tr("You need at least two columns for d_app operation!"));
		return false;
	}
	if (table->selectedColumn() < 0 || table->colPlotDesignation(table->selectedColumn()) != Table::Z){
		QMessageBox::critical(0,d_app->tr("QtiPlot - Error"),d_app->tr("Please select a Z column for d_app operation!"));
		return false;
	}
	if (table->noXColumn()){
		QMessageBox::critical(0,d_app->tr("QtiPlot - Error"),d_app->tr("You need to define a X column first!"));
		return false;
	}
	if (table->noYColumn()){
		QMessageBox::critical(0,d_app->tr("QtiPlot - Error"),d_app->tr("You need to define a Y column first!"));
		return false;
	}
	return true;
}
