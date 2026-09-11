/***************************************************************************
    File                 : AnalysisController.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Analysis, calculus, fitting, and filtering controller

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

#include "AnalysisController.h"
#include "ApplicationWindow.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Note.h"
#include "PlotCurve.h"
#include <Fit.h>
#include <MultiPeakFit.h>
#include <PolynomialFit.h>
#include <SigmoidalFit.h>
#include <LogisticFit.h>
#include <NonLinearFit.h>
#include <PluginFit.h>
#include <ExponentialFit.h>
#include "FitDialog.h"
#include "ExpDecayDialog.h"
#include "SmoothCurveDialog.h"
#include "IntegrationDialog.h"
#include "ScaleEngine.h"
#include "FindDialog.h"
#include "PolynomFitDialog.h"
#include "FilterDialog.h"
#include "FFTFilter.h"
#include "FFTDialog.h"
#include "SmoothFilter.h"
#include "Interpolation.h"
#include "InterpolationDialog.h"
#include "IntDialog.h"
#include "Differentiation.h"
#include "Integration.h"
#include "StudentTestDialog.h"
#include "AnovaDialog.h"
#include "StatisticTest.h"
#include "BaselineDialog.h"
#include "FrequencyCountDialog.h"
#include "SubtractLineTool.h"
#include "SubtractDataDialog.h"
#include "TranslateCurveTool.h"
#include "MultiPeakFitTool.h"
#include "ScreenPickerTool.h"
#include "DataPickerTool.h"
#include "RangeSelectorTool.h"
#include "Folder.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCursor>
#include <QLineEdit>
#include <QToolBar>

AnalysisController::AnalysisController(ApplicationWindow *app)
    : QObject(app), d_app(app)
{
}

AnalysisController::~AnalysisController()
{
}

void AnalysisController::showChiSquareTestDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *t = (Table*)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

	StudentTestDialog *std = new StudentTestDialog(StatisticTest::ChiSquareTest, t, false, d_app);
	std->show();
}

void AnalysisController::showStudentTestDialog(bool twoSamples)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *t = (Table*)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

	StudentTestDialog *std = new StudentTestDialog(StatisticTest::StudentTest, t, twoSamples, d_app);
	std->show();
}

void AnalysisController::testNormality()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *t = (Table*)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

	AnovaDialog *ad = new AnovaDialog(d_app, t, StatisticTest::NormalityTest);
	ad->show();
}

#ifdef HAVE_TAMUANOVA
void AnalysisController::showANOVADialog(bool twoWay)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	Table *t = (Table*)d_app->activeWindow(ApplicationWindow::TableWindow);
	if (!t)
		return;

	AnovaDialog *ad = new AnovaDialog(d_app, t, StatisticTest::AnovaTest, twoWay);
	ad->show();
}

#endif

void AnalysisController::showExpGrowthDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showExpDecayDialog(-1);
}

void AnalysisController::showExpDecayDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showExpDecayDialog(1);
}

void AnalysisController::showExpDecayDialog(int type)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	ExpDecayDialog *edd = new ExpDecayDialog(type, d_app);
	edd->setGraph(g);
	edd->show();
}

void AnalysisController::showTwoExpDecayDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showExpDecayDialog(2);
}

void AnalysisController::showExpDecay3Dialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showExpDecayDialog(3);
}

void AnalysisController::showFitDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	MultiLayer* plot = nullptr;
	if (MultiLayer *ml = qobject_cast<MultiLayer*>(w))
		plot = ml;
	else if (Table *t = qobject_cast<Table*>(w)){
		QStringList columnsLst = t->drawableColumnSelection();
		if (columnsLst.isEmpty()){
			QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"),
			d_app->tr("Please select a 'Y' column first!"));
			return;
		}
		plot = d_app->multilayerPlot(t, columnsLst, Graph::LineSymbols);
	}

	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	FitDialog *fd = new FitDialog(g, d_app);
	connect (plot, &Graph::destroyed, fd, &FindDialog::close);

	fd->setSrcTables(d_app->tableList());
	fd->show();
	fd->resize(fd->minimumSize());
}

void AnalysisController::showFilterDialog(int filter)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if ( g && g->validCurvesDataSize()){
		FilterDialog *fd = new FilterDialog(filter, d_app);
		fd->setGraph(g);
		fd->exec();
	}
}

void AnalysisController::lowPassFilterDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showFilterDialog(FFTFilter::LowPass);
}

void AnalysisController::highPassFilterDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	 showFilterDialog(FFTFilter::HighPass);
}

void AnalysisController::bandPassFilterDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showFilterDialog(FFTFilter::BandPass);
}

void AnalysisController::bandBlockFilterDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showFilterDialog(FFTFilter::BandBlock);
}

void AnalysisController::showFFTDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	FFTDialog *sd = 0;
	if (qobject_cast<MultiLayer *>(w)){
		Graph* g = ((MultiLayer*)w)->activeLayer();
		if ( g && g->validCurvesDataSize() ){
			sd = new FFTDialog(FFTDialog::onGraph, d_app);
			sd->setGraph(g);
		}
	} else if (w->inherits("Table")){
		sd = new FFTDialog(FFTDialog::onTable, d_app);
		sd->setTable((Table*)w);
	} else if (qobject_cast<Matrix *>(w)){
		if (!((Matrix *)w)->isEmpty()){
			sd = new FFTDialog(FFTDialog::onMatrix, d_app);
			sd->setMatrix((Matrix *)w);
		} else
			d_app->showNoDataMessage();
	}

	if (sd)
        sd->exec();
}

void AnalysisController::showSmoothDialog(int m)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	SmoothCurveDialog *sd = new SmoothCurveDialog(m, d_app);
	sd->setGraph(g);
	sd->exec();
}

void AnalysisController::showSmoothSavGolDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    showSmoothDialog(SmoothFilter::SavitzkyGolay);
}

void AnalysisController::showSmoothFFTDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showSmoothDialog(SmoothFilter::FFT);
}

void AnalysisController::showSmoothAverageDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showSmoothDialog(SmoothFilter::Average);
}

void AnalysisController::showSmoothLowessDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	showSmoothDialog(SmoothFilter::Lowess);
}

void AnalysisController::showInterpolationDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	InterpolationDialog *id = new InterpolationDialog(d_app);
	id->setGraph(g);
	id->show();
}

void AnalysisController::showFitPolynomDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	PolynomFitDialog *pfd = new PolynomFitDialog(d_app);
	pfd->setGraph(g);
	pfd->show();
}

void AnalysisController::showFunctionIntegrationDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	IntDialog *id = new IntDialog(d_app, g);
	id->exec();
}

void AnalysisController::showDataSetDialog(int operation)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph *g = plot->activeLayer();
	if (!g)
		return;

	bool ok;
	QStringList curves = g->analysableCurvesList();
	QString txt = QInputDialog::getItem(d_app, d_app->tr("QtiPlot - Choose data set"),
					d_app->tr("Curve") + ": ", curves, 0, false, &ok);
	if (ok && !txt.isEmpty())
		analyzeCurve(g, g->curve(txt), operation);
}

void AnalysisController::analyzeCurve(Graph *g,  QwtPlotCurve *c, int operation)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_2_linear_fit_points = d_app->d_2_linear_fit_points;
	auto &fitPoints = d_app->fitPoints;
	auto &fit_output_precision = d_app->fit_output_precision;
	auto &fit_scale_errors = d_app->fit_scale_errors;
	auto &generateUniformFitPoints = d_app->generateUniformFitPoints;
	auto &pasteFitResultsToPlot = d_app->pasteFitResultsToPlot;

	if (!g || !c)
		return;

	Fit *fitter = 0;
	switch(operation){
	    case ApplicationWindow::NoAnalysis:
	    break;
		case ApplicationWindow::Integrate:
		{
			Integration *i = new Integration(d_app, (PlotCurve*)c);
			i->run();
			delete i;
		}
		break;
		case ApplicationWindow::Diff:
		{
			Differentiation *diff = new Differentiation(d_app, (PlotCurve*)c);
			diff->enableGraphicsDisplay(true);
			diff->run();
			delete diff;
		}
		break;
		case ApplicationWindow::FitLinear:
			fitter = new LinearFit (d_app, g);
		break;
		case ApplicationWindow::FitLorentz:
			fitter = new LorentzFit(d_app, g);
		break;
		case ApplicationWindow::FitGauss:
			fitter = new GaussFit(d_app, g);
		break;
		case ApplicationWindow::FitSigmoidal:
		{
			ScaleEngine *se = (ScaleEngine *)g->axisScaleEngine(c->xAxis());
			if(se->type() == ScaleTransformation::Log10)
				fitter = new LogisticFit (d_app, g);
			else
				fitter = new SigmoidalFit (d_app, g);
		}
		break;
		case ApplicationWindow::FitSlope:
			fitter = new LinearSlopeFit (d_app, g);
		break;
	}

	if (!fitter)
		return;

	if (fitter->setDataFromCurve((PlotCurve*)c)){
		if (operation != ApplicationWindow::FitLinear && operation != ApplicationWindow::FitSlope){
			fitter->guessInitialValues();
			fitter->scaleErrors(fit_scale_errors);
			fitter->generateFunction(generateUniformFitPoints, fitPoints);
		} else if (d_2_linear_fit_points)
			fitter->generateFunction(generateUniformFitPoints, 2);
		fitter->setOutputPrecision(fit_output_precision);
		fitter->fit();
		if (pasteFitResultsToPlot)
			fitter->showLegend();
		delete fitter;
	}
}

void AnalysisController::analysis(int operation)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	if (g->rangeSelectorsEnabled()){
		analyzeCurve(g, g->rangeSelectorTool()->selectedCurve(), operation);
		return;
	}

	QStringList lst = g->analysableCurvesList();
	if (lst.count() == 1)
		analyzeCurve(g, g->curve(0), operation);
	else
		showDataSetDialog(operation);
}

void AnalysisController::integrate()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &current_folder = d_app->current_folder;
	auto &info = d_app->info;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if (w->inherits("MultiLayer")){
		Graph* g = ((MultiLayer *)w)->activeLayer();
		if (!g)
			return;
		IntegrationDialog *id = new IntegrationDialog(g, d_app);
		id->show();
	} else 	if (Matrix *m = qobject_cast<Matrix *>(w)){
		if (!m->isEmpty()){
			QDateTime dt = QDateTime::currentDateTime ();
			QString info = dt.toString(Qt::TextDate);
			info += "\n" + d_app->tr("Integration of %1 from zero is").arg(QString(w->objectName())) + ":\t";
			info += QString::number(m->integrate()) + "\n";
			info += "-------------------------------------------------------------\n";
			current_folder->appendLogInfo(info);
			d_app->showResults(true);
		} else
			d_app->showNoDataMessage();
	} else if (Table *t = qobject_cast<Table *>(w)){
		QStringList lst = t->selectedYColumns();
		int cols = lst.size();
		QTableWidgetSelectionRange sel = t->getSelection();
		if (!cols || sel.topRow() == sel.bottomRow()){
			QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"),
			d_app->tr("Please select a 'Y' column first!"));
			return;
		}

		IntegrationDialog *id = new IntegrationDialog(t, d_app);
		id->show();
	}
}

void AnalysisController::differentiate()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_indexed_colors = d_app->d_indexed_colors;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if (qobject_cast<MultiLayer *>(w))
		analysis(ApplicationWindow::Diff);
	else if (w->inherits("Table")){
		Table *t = qobject_cast<Table *>(w);
		QStringList lst = t->selectedYColumns();
		int cols = lst.size();
		if (!cols){
			QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"), d_app->tr("Please select a 'Y' column first!"));
			return;
		}

		Differentiation *diff = new Differentiation(d_app, nullptr, "", "");
		diff->setUpdateOutputGraph(false);
		int aux = 0;
		for (QString yCol : lst){
			int xCol = t->colX(t->colIndex(yCol));
			diff->setDataFromTable(t, t->colName(xCol), yCol);
			diff->run();
			Graph *g = diff->outputGraph();
			if (!g)
				continue;

			QwtPlotCurve *c = g->curve(aux);
			if (c){
				if (aux < d_indexed_colors.size()){
					QPen pen = c->pen();
					pen.setColor(d_indexed_colors[aux]);
					c->setPen(pen);
				}
				aux++;
			}
		}

		Graph *g = diff->outputGraph();
		if (g){
			g->newLegend();
			g->updatePlot();
		}

		delete diff;
	}
}

void AnalysisController::fitLinear()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_2_linear_fit_points = d_app->d_2_linear_fit_points;
	auto &fit_output_precision = d_app->fit_output_precision;
	auto &generateUniformFitPoints = d_app->generateUniformFitPoints;
	auto &results = d_app->results;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if (qobject_cast<MultiLayer *>(w))
		analysis(ApplicationWindow::FitLinear);
	else if (Table *t = qobject_cast<Table *>(w)){
		QStringList lst = t->selectedYColumns();
		int cols = lst.size();
		if (!cols){
        	QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"), d_app->tr("Please select a 'Y' column first!"));
			return;
		}

		MultiLayer* g = d_app->multilayerPlot(t, t->drawableColumnSelection(), Graph::LineSymbols);
		if (!g)
			return;

		QString legend = d_app->tr("Linear Regression of %1").arg(t->objectName());
		g->setWindowLabel(legend);

		QApplication::setOverrideCursor(Qt::WaitCursor);

		Table *result = d_app->newTable(cols, 5, "", legend);
		result->setColName(0, d_app->tr("Column"));
		result->setColName(1, d_app->tr("Slope"));
		result->setColName(2, d_app->tr("Intercept"));
		result->setColName(3, d_app->tr("Chi^2"));
		result->setColName(4, d_app->tr("R^2"));

		LinearFit *lf = new LinearFit (d_app, g->activeLayer());
		lf->setUpdateOutputGraph(false);
		if (d_2_linear_fit_points)
			lf->generateFunction(generateUniformFitPoints, 2);
		lf->setOutputPrecision(fit_output_precision);

		int aux = 0;
		for (QString yCol : lst){
			if (!lf->setDataFromCurve(yCol))
				continue;

			lf->setColor(aux);
			lf->fit();
			double *res = lf->results();
			result->setText(aux, 0, yCol);
			result->setCell(aux, 1, res[1]);
			result->setCell(aux, 2, res[0]);
			result->setCell(aux, 3, lf->chiSquare());
			result->setCell(aux, 4, lf->rSquare());
			aux++;
		}
		for (int i = 0; i < result->numCols(); i++)
			result->table()->adjustColumn(i);
		result->show();

		Graph *og = lf->outputGraph();
		if (og)
			og->updatePlot();
		delete lf;

		QApplication::restoreOverrideCursor();
	}
}

void AnalysisController::fitSlope()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_2_linear_fit_points = d_app->d_2_linear_fit_points;
	auto &fit_output_precision = d_app->fit_output_precision;
	auto &generateUniformFitPoints = d_app->generateUniformFitPoints;
	auto &results = d_app->results;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if (qobject_cast<MultiLayer *>(w))
		analysis(ApplicationWindow::FitSlope);
	else if (Table *t = qobject_cast<Table *>(w)){
		QStringList lst = t->selectedYColumns();
		int cols = lst.size();
		if (!cols){
        	QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"), d_app->tr("Please select a 'Y' column first!"));
			return;
		}

		MultiLayer* g = d_app->multilayerPlot(t, t->drawableColumnSelection(), Graph::LineSymbols);
		if (!g)
			return;

		QApplication::setOverrideCursor(Qt::WaitCursor);

		QString legend = d_app->tr("Linear Regression of %1").arg(t->objectName());
		g->setWindowLabel(legend);

		Table *result = d_app->newTable(cols, 4, "", legend);
		result->setColName(0, d_app->tr("Column"));
		result->setColName(1, d_app->tr("Slope"));
		result->setColName(2, d_app->tr("Chi^2"));
		result->setColName(3, d_app->tr("R^2"));

		LinearSlopeFit *lf = new LinearSlopeFit (d_app, g->activeLayer());
		lf->setUpdateOutputGraph(false);
		if (d_2_linear_fit_points)
			lf->generateFunction(generateUniformFitPoints, 2);
		lf->setOutputPrecision(fit_output_precision);

		int aux = 0;
		for (QString yCol : lst){
			if (!lf->setDataFromCurve(yCol))
				continue;

			lf->setColor(aux);
			lf->fit();
			double *res = lf->results();
			result->setText(aux, 0, yCol);
			result->setCell(aux, 1, res[0]);
			result->setCell(aux, 2, lf->chiSquare());
			result->setCell(aux, 3, lf->rSquare());
			aux++;
		}
		for (int i = 0; i < result->numCols(); i++)
			result->table()->adjustColumn(i);
		result->show();

		Graph *og = lf->outputGraph();
		if (og)
			og->updatePlot();
		delete lf;

		QApplication::restoreOverrideCursor();
	}
}

void AnalysisController::fitSigmoidal()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	analysis(ApplicationWindow::FitSigmoidal);
}

void AnalysisController::fitGauss()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	analysis(ApplicationWindow::FitGauss);
}

void AnalysisController::fitLorentz()

{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	analysis(ApplicationWindow::FitLorentz);
}

void AnalysisController::deleteFitTables()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		MultiLayer *ml = qobject_cast<MultiLayer*>(w);
		if (!ml)
			continue;
		QList<Graph *> layers = ml->layersList();
		for (Graph *g : layers){
			QList<QwtPlotCurve *> curves = g->fitCurvesList();
			for (QwtPlotCurve *c : curves){
				if (((PlotCurve *)c)->rtti() != Graph::Function){
					Table *t = ((DataCurve *)c)->table();
					if (!t)
						continue;
					t->askOnCloseEvent(false);
					t->close();
				}
			}
		}
	}
}

void AnalysisController::fitMultiPeakGauss()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	fitMultiPeak((int)MultiPeakFit::Gauss);
}

void AnalysisController::fitMultiPeakLorentz()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	fitMultiPeak((int)MultiPeakFit::Lorentz);
}

void AnalysisController::fitMultiPeak(int profile)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &btnPointer = d_app->btnPointer;
	auto &displayBar = d_app->displayBar;
	auto &info = d_app->info;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;
	if (plot->isEmpty()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no plot layers available in d_app window.</h4>"
					"<p><h4>Please add a layer and try again!</h4>"));
		btnPointer->setChecked(true);
		return;
	}

	Graph* g = (Graph*)plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	if (g->isPiePlot()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("This functionality is not available for pie plots!"));
		return;
	} else {
		bool ok;
		int peaks = QInputDialog::getInt(d_app, d_app->tr("QtiPlot - Enter the number of peaks"),
				d_app->tr("Peaks"), 2, 2, 1000000, 1, &ok, d_app->windowFlags());
		if (ok && peaks){
			MultiPeakFitTool *tool = new MultiPeakFitTool(g, d_app, (MultiPeakFit::PeakProfile)profile, peaks);
			connect(tool, &MultiPeakFitTool::statusText, info, &QLineEdit::setText);
			g->setActiveTool(tool);
			displayBar->show();
		}
	}
}

void AnalysisController::subtractStraightLine()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &btnPointer = d_app->btnPointer;
	auto &displayBar = d_app->displayBar;
	auto &info = d_app->info;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;
	if (plot->isEmpty()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no plot layers available in d_app window.</h4>"
					"<p><h4>Please add a layer and try again!</h4>"));
		btnPointer->setChecked(true);
		return;
	}

	Graph* g = (Graph*)plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	if (g->isPiePlot()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("This functionality is not available for pie plots!"));
		return;
	} else {
		SubtractLineTool *tool = new SubtractLineTool(g, d_app);
		connect(tool, &SubtractLineTool::statusText, info, &QLineEdit::setText);
		g->setActiveTool(tool);
		displayBar->show();
	}
}

void AnalysisController::subtractReferenceData()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	SubtractDataDialog *sdd = new SubtractDataDialog(d_app);
	sdd->setGraph(g);
	sdd->exec();
}

void AnalysisController::baselineDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	if (qApp->arguments().contains("-X"))
		return;
	MultiLayer *plot = (MultiLayer *)d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g || !g->validCurvesDataSize())
		return;

	BaselineDialog *bd = new BaselineDialog(d_app);
	bd->setGraph(g);
	bd->show();
}

void AnalysisController::saveFitFunctions(const QStringList& lst)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &fitModelsPath = d_app->fitModelsPath;

	if (!lst.count())
		return;

    QString explain = d_app->tr("Starting with version 0.9.1 QtiPlot stores the user defined fit models to a different location.");
    explain += " " + d_app->tr("If you want to save your already defined models, please choose a destination folder.");
    if (QMessageBox::Ok != QMessageBox::information(d_app, d_app->tr("QtiPlot") + " - " + d_app->tr("Import fit models"), explain,
                            QMessageBox::Ok, QMessageBox::Cancel)) return;

	QString dir = QFileDialog::getExistingDirectory(d_app, d_app->tr("Choose a directory to export the fit models to"), fitModelsPath, QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
	    fitModelsPath = dir;

        for (int i = 0; i<lst.count(); i++){
            QString s = lst[i].simplified();
            if (!s.isEmpty()){
                NonLinearFit *fit = new NonLinearFit(d_app, (Graph*)0);

                QStringList l = s.split("=");
                if (l.count() == 2)
                    fit->setFormula(l[1]);

                fit->save(fitModelsPath + "/" + fit->objectName() + ".fit");
            }
        }
	}
}

void AnalysisController::showFrequencyCountDialog()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

    Table *t = qobject_cast<Table *>(d_app->activeWindow(ApplicationWindow::TableWindow));
	if (!t)
		return;

    int validRows = 0;
    QTableWidgetSelectionRange sel = t->getSelection();
    if (!t->table()->selectedRanges().isEmpty()){
        if (sel.rowCount() > 1 && sel.columnCount() == 1){
            int col = sel.leftColumn();
            for (int i = sel.topRow(); i <= sel.bottomRow(); i++){
                if (!t->text(i, col).isEmpty())
                   validRows++;
                if (validRows > 1){
                    FrequencyCountDialog *fcd = new FrequencyCountDialog(t, d_app);
                    fcd->exec();
                    break;
                }
            }
        }
    }
    if (validRows < 2)
        QMessageBox::warning(d_app, d_app->tr("QtiPlot - Column selection error"),
        d_app->tr("Please select exactly one column and more than one non empty cell!"));
}
