/***************************************************************************
    File                 : ActionTranslator.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Action text, status tip, tooltip, and shortcut translator

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

#include "ActionTranslator.h"
#include "ApplicationWindow.h"
#include <QAction>

void ActionTranslator::translateActionsStrings(ApplicationWindow *app)
{
    if (!app)
        return;
    app->actionFontBold->setToolTip(app->tr("Bold"));
    app->actionFontItalic->setToolTip(app->tr("Italic"));
    app->actionUnderline->setStatusTip(app->tr("Underline (Ctrl+U)"));
	app->actionUnderline->setShortcut(app->tr("Ctrl+U"));
	app->actionGreekSymbol->setToolTip(app->tr("Greek"));
	app->actionGreekMajSymbol->setToolTip(app->tr("Greek"));
	app->actionMathSymbol->setToolTip(app->tr("Mathematical Symbols"));

	app->actionShowCurvePlotDialog->setText(app->tr("&Plot details..."));
	app->actionShowCurveWorksheet->setText(app->tr("&Worksheet"));
	app->actionRemoveCurve->setText(app->tr("&Delete"));

	app->actionCurveFullRange->setText(app->tr("&Reset to Full Range"));
	app->actionEditCurveRange->setText(app->tr("Edit &Range..."));
	app->actionHideCurve->setText(app->tr("&Hide"));
	app->actionHideOtherCurves->setText(app->tr("Hide &Other Curves"));
	app->actionShowAllCurves->setText(app->tr("&Show All Curves"));

	app->actionNewProject->setText(app->tr("New &Project"));
	app->actionNewProject->setToolTip(app->tr("Open a new project"));
	app->actionNewProject->setShortcut(app->tr("Ctrl+N"));

	app->actionAppendProject->setText(app->tr("App&end Project..."));
	app->actionAppendProject->setToolTip(app->tr("Append a project to the current folder"));
	app->actionAppendProject->setShortcut(app->tr("Ctrl+Alt+A"));

	app->actionNewFolder->setText(app->tr("New F&older"));
	app->actionNewFolder->setToolTip(app->tr("Create a new folder"));
	app->actionNewFolder->setShortcut(Qt::Key_F7);

	app->actionNewGraph->setText(app->tr("New &Graph"));
	app->actionNewGraph->setToolTip(app->tr("Create an empty 2D plot"));
	app->actionNewGraph->setShortcut(app->tr("Ctrl+G"));

	app->actionNewNote->setText(app->tr("New &Note"));
	app->actionNewNote->setToolTip(app->tr("Create an empty note window"));

	app->actionNewTable->setText(app->tr("New &Table"));
	app->actionNewTable->setShortcut(app->tr("Ctrl+T"));
	app->actionNewTable->setToolTip(app->tr("New table"));

	app->actionNewMatrix->setText(app->tr("New &Matrix"));
	app->actionNewMatrix->setShortcut(app->tr("Ctrl+M"));
	app->actionNewMatrix->setToolTip(app->tr("New matrix"));

	app->actionNewFunctionPlot->setText(app->tr("New &Function Plot") + "...");
	app->actionNewFunctionPlot->setToolTip(app->tr("Create a new 2D function plot"));
	app->actionNewFunctionPlot->setShortcut(app->tr("Ctrl+F"));

	app->actionNewSurfacePlot->setText(app->tr("New 3D &Surface Plot") + "...");
	app->actionNewSurfacePlot->setToolTip(app->tr("Create a new 3D surface plot"));
	app->actionNewSurfacePlot->setShortcut(app->tr("Ctrl+ALT+Z"));

	app->actionOpen->setText(app->tr("&Open..."));
	app->actionOpen->setShortcut(app->tr("Ctrl+O"));
	app->actionOpen->setToolTip(app->tr("Open project"));

	app->actionExportExcel->setText(app->tr("Export Exce&l ..."));
	app->actionExportExcel->setToolTip(app->tr("Export Excel"));

	app->actionExportOds->setText(app->tr("Export &Open Document Spreadsheet ..."));
	app->actionExportOds->setToolTip(app->tr("Export Open Document Spreadsheet"));

	app->actionOpenOds->setText(app->tr("Open ODF Spreads&heet..."));
	app->actionOpenOds->setShortcut( app->tr("Ctrl+Alt+S") );
	app->actionOpenOds->setToolTip(app->tr("Open ODF Spreadsheet"));

	app->actionOpenExcel->setText(app->tr("Open Exce&l ..."));
	app->actionOpenExcel->setShortcut( app->tr("Ctrl+Shift+E") );
	app->actionOpenExcel->setToolTip(app->tr("Open Excel"));

	app->actionLoadImage->setText(app->tr("Open Image &File..."));
	app->actionLoadImage->setShortcut(app->tr("Ctrl+I"));

	app->actionImportDatabase->setText(app->tr("&Database..."));
	app->actionImportSound->setText(app->tr("&Sound (WAV)..."));
	app->actionImportImage->setText(app->tr("Import I&mage..."));

	app->actionSaveProject->setText(app->tr("&Save Project"));
	app->actionSaveProject->setToolTip(app->tr("Save project"));
	app->actionSaveProject->setShortcut(app->tr("Ctrl+S"));

	app->actionSaveProjectAs->setText(app->tr("Save Project &As..."));
	app->actionSaveProjectAs->setShortcut( app->tr("Ctrl+Shift+S") );

	app->actionOpenTemplate->setText(app->tr("Open Te&mplate..."));
	app->actionOpenTemplate->setToolTip(app->tr("Open template"));

	app->actionSaveTemplate->setText(app->tr("Save As &Template..."));
	app->actionSaveTemplate->setToolTip(app->tr("Save window as template"));

	app->actionSaveWindow->setText(app->tr("Save &Window As..."));

	app->actionLoad->setText(app->tr("&Import ASCII..."));
	app->actionLoad->setToolTip(app->tr("Import data file(s)"));
	app->actionLoad->setShortcut(app->tr("Ctrl+K"));

	app->actionUndo->setText(app->tr("&Undo"));
	app->actionUndo->setToolTip(app->tr("Undo changes"));
	app->actionUndo->setShortcut(app->tr("Ctrl+Z"));

	app->actionRedo->setText(app->tr("&Redo"));
	app->actionRedo->setToolTip(app->tr("Redo changes"));

	app->actionCopyWindow->setText(app->tr("&Duplicate"));
	app->actionCopyWindow->setToolTip(app->tr("Duplicate window"));
	app->actionCopyWindow->setShortcut(app->tr("Ctrl+Alt+D"));

	app->actionCutSelection->setText(app->tr("Cu&t Selection"));
	app->actionCutSelection->setToolTip(app->tr("Cut selection"));
	app->actionCutSelection->setShortcut(app->tr("Ctrl+X"));

	app->actionCopySelection->setText(app->tr("&Copy Selection"));
	app->actionCopySelection->setToolTip(app->tr("Copy selection"));
	app->actionCopySelection->setShortcut(app->tr("Ctrl+C"));

	app->actionPasteSelection->setText(app->tr("&Paste Selection"));
	app->actionPasteSelection->setToolTip(app->tr("Paste selection"));
	app->actionPasteSelection->setShortcut(app->tr("Ctrl+V"));

	app->actionClearSelection->setText(app->tr("&Delete Selection"));
	app->actionClearSelection->setToolTip(app->tr("Delete selection"));
	app->actionClearSelection->setShortcut(app->tr("Del","delete key"));

	app->actionRaiseEnrichment->setText(app->tr("&Front"));
	app->actionRaiseEnrichment->setToolTip(app->tr("Raise object on top"));

	app->actionLowerEnrichment->setText(app->tr("&Back"));
	app->actionLowerEnrichment->setToolTip(app->tr("Lower object to the bottom"));

	app->actionAlignTop->setToolTip(app->tr("Align Top"));
	app->actionAlignBottom->setToolTip(app->tr("Align Bottom"));
	app->actionAlignLeft->setToolTip(app->tr("Align Left"));
	app->actionAlignRight->setToolTip(app->tr("Align Right"));

	app->actionShowExplorer->setText(app->tr("Project &Explorer"));
	app->actionShowExplorer->setShortcut(app->tr("Ctrl+E"));
	app->actionShowExplorer->setToolTip(app->tr("Show project explorer"));

	app->actionFindWindow->setText(app->tr("&Find..."));

	app->actionNextWindow->setText(app->tr("&Next","next window"));
	app->actionNextWindow->setShortcut(app->tr("F5","next window shortcut"));

	app->actionPrevWindow->setText(app->tr("&Previous","previous window"));
	app->actionPrevWindow->setShortcut(app->tr("F6","previous window shortcut"));

	app->actionShowLog->setText(app->tr("Results &Log"));
	app->actionShowLog->setToolTip(app->tr("Show analysis results"));

    app->actionShowUndoStack->setText(app->tr("&Undo/Redo Stack"));
	app->actionShowUndoStack->setToolTip(app->tr("Show available undo/redo commands"));

#ifdef SCRIPTING_CONSOLE
	app->actionShowConsole->setText(app->tr("&Console"));
	app->actionShowConsole->setToolTip(app->tr("Show Scripting console"));
#endif

#ifdef SCRIPTING_PYTHON
	app->actionShowScriptWindow->setText(app->tr("&Script Window"));
	app->actionShowScriptWindow->setToolTip(app->tr("Script Window"));
	app->actionOpenQtDesignerUi->setText(app->tr("Load Custom User &Interface..."));
#endif

	app->actionCustomActionDialog->setText(app->tr("Add &Custom Script Action..."));

	app->actionAddLayer->setText(app->tr("Add La&yer"));
	app->actionAddLayer->setToolTip(app->tr("Add Layer"));
	app->actionAddLayer->setShortcut(app->tr("ALT+L"));

	app->actionShowLayerDialog->setText(app->tr("Arran&ge Layers") + "...");
	app->actionShowLayerDialog->setToolTip(app->tr("Arrange Layers"));
	app->actionShowLayerDialog->setShortcut(app->tr("Shift+A"));

	app->actionAutomaticLayout->setText(app->tr("Automatic Layout"));
	app->actionAutomaticLayout->setToolTip(app->tr("Automatic Layout"));

	app->actionExportLayer->setText(app->tr("&Layer") + "...");
	app->actionExportLayer->setShortcut(app->tr("Ctrl+Shift+L"));

	app->actionExportGraph->setText(app->tr("&Window") + "...");
	app->actionExportGraph->setShortcut(app->tr("Ctrl+Alt+G"));
	app->actionExportGraph->setToolTip(app->tr("Export current graph"));

	app->actionExportAllGraphs->setText(app->tr("&All") + "...");
	app->actionExportAllGraphs->setShortcut(app->tr("Alt+X"));
	app->actionExportAllGraphs->setToolTip(app->tr("Export all graphs"));
	app->actionPresentationODF->setText(app->tr("Create Open &Document Presentation..."));
	app->actionExportPDF->setText(app->tr("&Export PDF") + "...");
	app->actionExportPDF->setShortcut(app->tr("Ctrl+Alt+P"));
	app->actionExportPDF->setToolTip(app->tr("Export to PDF"));

	app->actionPrint->setText(app->tr("&Print..."));
	app->actionPrint->setShortcut(app->tr("Ctrl+P"));
	app->actionPrint->setToolTip(app->tr("Print window"));

	app->actionPrintPreview->setText(app->tr("Print Pre&view..."));
	app->actionPrintPreview->setToolTip(app->tr("Print preview"));

	app->actionPrintAllPlots->setText(app->tr("Print All Plo&ts") + "...");
	app->actionPrintAllPlots->setShortcut(app->tr("Ctrl+Shift+P"));
	app->actionShowExportASCIIDialog->setText(app->tr("E&xport ASCII..."));

	app->actionCloseAllWindows->setText(app->tr("&Quit"));
	app->actionCloseAllWindows->setShortcut(app->tr("Ctrl+Q"));
	app->actionCloseProject->setText(app->tr("&Close"));

	app->actionClearLogInfo->setText(app->tr("Clear &Log Information"));
	app->actionDeleteFitTables->setText(app->tr("Delete &Fit Tables"));

    app->actionToolBars->setText(app->tr("&Toolbars..."));
	app->actionToolBars->setShortcut(app->tr("Ctrl+Shift+T"));

	app->actionShowPlotWizard->setText(app->tr("Plot &Wizard") + "...");
	app->actionShowPlotWizard->setShortcut(app->tr("Ctrl+Alt+W"));

	app->actionShowConfigureDialog->setText(app->tr("&Preferences..."));

	app->actionShowCurvesDialog->setText(app->tr("Add/Remove &Curve..."));
	app->actionShowCurvesDialog->setShortcut(app->tr("ALT+C"));
	app->actionShowCurvesDialog->setToolTip(app->tr("Add curve to graph"));

	app->actionAddErrorBars->setText(app->tr("Add &Error Bars..."));
	app->actionAddErrorBars->setToolTip(app->tr("Add Error Bars..."));
	app->actionAddErrorBars->setShortcut(app->tr("Ctrl+B"));

	app->actionAddFunctionCurve->setText(app->tr("Add &Function..."));
	app->actionAddFunctionCurve->setToolTip(app->tr("Add Function..."));
	app->actionAddFunctionCurve->setShortcut(app->tr("Ctrl+Alt+F"));

	app->actionUnzoom->setText(app->tr("&Rescale to Show All"));
	app->actionUnzoom->setShortcut(app->tr("Ctrl+Shift+R"));
	app->actionUnzoom->setToolTip(app->tr("Best fit"));

	app->actionNewLegend->setText( app->tr("New &Legend"));
	app->actionNewLegend->setShortcut(app->tr("Ctrl+L"));
	app->actionNewLegend->setToolTip(app->tr("Add new legend"));

	app->actionTimeStamp->setText(app->tr("Add Time Stamp"));
	app->actionTimeStamp->setShortcut(app->tr("Ctrl+ALT+T"));
	app->actionTimeStamp->setToolTip(app->tr("Date & time "));

	app->actionAddImage->setText(app->tr("Add &Image"));
	app->actionAddImage->setToolTip(app->tr("Add Image"));
	app->actionAddImage->setShortcut(app->tr("ALT+I"));

	app->actionPlotL->setText(app->tr("&Line"));
	app->actionPlotL->setToolTip(app->tr("Plot as line"));

	app->actionPlotP->setText(app->tr("&Scatter"));
	app->actionPlotP->setToolTip(app->tr("Plot as symbols"));

	app->actionPlotLP->setText(app->tr("Line + S&ymbol"));
	app->actionPlotLP->setToolTip(app->tr("Plot as line + symbols"));

	app->actionPlotVerticalDropLines->setText(app->tr("Vertical &Drop Lines"));

	app->actionPlotSpline->setText(app->tr("&Spline"));
	app->actionPlotVertSteps->setText(app->tr("&Vertical Steps"));
	app->actionPlotHorSteps->setText(app->tr("&Horizontal Steps"));

	app->actionPlotVerticalBars->setText(app->tr("&Columns"));
	app->actionPlotVerticalBars->setToolTip(app->tr("Plot with vertical bars"));

	app->actionPlotHorizontalBars->setText(app->tr("&Rows"));
	app->actionPlotHorizontalBars->setToolTip(app->tr("Plot with horizontal bars"));

	app->actionStackBars->setText(app->tr("Stack &Bar"));
	app->actionStackBars->setToolTip(app->tr("Plot stack bar"));

	app->actionStackColumns->setText(app->tr("Stack &Column"));
	app->actionStackColumns->setToolTip(app->tr("Plot stack column"));

	app->actionPlotArea->setText(app->tr("&Area"));
	app->actionPlotArea->setToolTip(app->tr("Plot area"));

	app->actionPlotPie->setText(app->tr("&Pie"));
	app->actionPlotPie->setToolTip(app->tr("Plot pie"));

	app->actionPlotVectXYXY->setText(app->tr("&Vectors XYXY"));
	app->actionPlotVectXYXY->setToolTip(app->tr("Vectors XYXY"));

	app->actionPlotVectXYAM->setText(app->tr("Vectors XY&AM"));
	app->actionPlotVectXYAM->setToolTip(app->tr("Vectors XYAM"));

	app->actionPlotHistogram->setText( app->tr("&Histogram"));
	app->actionPlotStackedHistograms->setText(app->tr("&Stacked Histogram"));

	app->actionPlot2VerticalLayers->setText(app->tr("&Vertical 2 Layers"));
	app->actionPlot2HorizontalLayers->setText(app->tr("&Horizontal 2 Layers"));
	app->actionPlot4Layers->setText(app->tr("&4 Layers"));
	app->actionPlotStackedLayers->setText(app->tr("&Stacked Layers"));

	app->actionVertSharedAxisLayers->setText(app->tr("&Vertical 2 Layers"));
	app->actionHorSharedAxisLayers->setText(app->tr("&Horizontal 2 Layers"));
	app->actionSharedAxesLayers->setText(app->tr("&4 Layers"));
	app->actionStackSharedAxisLayers->setText(app->tr("&Stacked Layers"));
	app->actionCustomSharedAxisLayers->setText(app->tr("&Custom Layout..."));
	app->actionCustomLayout->setText(app->tr("&Custom Layout..."));

	app->actionStemPlot->setText(app->tr("Stem-and-&Leaf Plot"));
	app->actionStemPlot->setToolTip(app->tr("Stem-and-Leaf Plot"));

    app->actionPlotDoubleYAxis->setText(app->tr("D&ouble-Y"));
    app->actionPlotDoubleYAxis->setToolTip(app->tr("Double Y Axis"));

    app->actionAddZoomPlot->setText(app->tr("&Zoom"));
    app->actionAddZoomPlot->setToolTip(app->tr("Zoom"));

	app->actionWaterfallPlot->setText(app->tr("&Waterfall Plot"));
	app->actionWaterfallPlot->setToolTip(app->tr("Waterfall Plot"));

    app->actionExtractGraphs->setText(app->tr("E&xtract to Graphs"));
    app->actionExtractGraphs->setToolTip(app->tr("Extract to Graphs"));

    app->actionExtractLayers->setText(app->tr("Extract to Layer&s"));
    app->actionExtractLayers->setToolTip(app->tr("Extract to Layers"));

	app->actionPlot3DRibbon->setText(app->tr("&Ribbon"));
	app->actionPlot3DRibbon->setToolTip(app->tr("Plot 3D ribbon"));

	app->actionPlot3DBars->setText(app->tr("&Bars"));
	app->actionPlot3DBars->setToolTip(app->tr("Plot 3D bars"));

	app->actionPlot3DScatter->setText(app->tr("&Scatter"));
	app->actionPlot3DScatter->setToolTip(app->tr("Plot 3D scatter"));

	app->actionPlot3DTrajectory->setText(app->tr("&Trajectory"));
	app->actionPlot3DTrajectory->setToolTip(app->tr("Plot 3D trajectory"));

	app->actionColorMap->setText(app->tr("Contour + &Color Fill"));
	app->actionColorMap->setToolTip(app->tr("Contour Lines + Color Fill"));

	app->actionContourMap->setText(app->tr("Contour &Lines"));
	app->actionContourMap->setToolTip(app->tr("Contour Lines"));

	app->actionGrayMap->setText(app->tr("&Gray Scale Map"));
	app->actionGrayMap->setToolTip(app->tr("Gray Scale Map"));

	app->actionShowColStatistics->setText(app->tr("Statistics on &Columns"));
	app->actionShowColStatistics->setToolTip(app->tr("Selected columns statistics"));

	app->actionShowRowStatistics->setText(app->tr("Statistics on &Rows"));
	app->actionShowRowStatistics->setToolTip(app->tr("Selected rows statistics"));
	app->actionShowIntDialog->setText(app->tr("Integr&ate Function..."));
	app->actionIntegrate->setText(app->tr("&Integrate") + "...");
	app->actionInterpolate->setText(app->tr("Inte&rpolate ..."));
	app->actionLowPassFilter->setText(app->tr("&Low Pass..."));
	app->actionHighPassFilter->setText(app->tr("&High Pass..."));
	app->actionBandPassFilter->setText(app->tr("&Band Pass..."));
	app->actionBandBlockFilter->setText(app->tr("&Band Block..."));
	app->actionFFT->setText(app->tr("&FFT..."));
	app->actionSmoothSavGol->setText(app->tr("&Savitzky-Golay..."));
	app->actionSmoothFFT->setText(app->tr("&FFT Filter..."));
	app->actionSmoothAverage->setText(app->tr("Moving Window &Average..."));
	app->actionSmoothLowess->setText(app->tr("&Lowess..."));
	app->actionDifferentiate->setText(app->tr("&Differentiate"));
	app->actionFitLinear->setText(app->tr("Fit &Linear"));
	app->actionFitSlope->setText(app->tr("Fit Slop&e"));
	app->actionShowFitPolynomDialog->setText(app->tr("Fit &Polynomial ..."));
	app->actionShowExpDecayDialog->setText(app->tr("&First Order ..."));
	app->actionShowTwoExpDecayDialog->setText(app->tr("&Second Order ..."));
	app->actionShowExpDecay3Dialog->setText(app->tr("&Third Order ..."));
	app->actionFitExpGrowth->setText(app->tr("Fit Exponential Gro&wth ..."));
	app->actionFitSigmoidal->setText(app->tr("Fit &Boltzmann (Sigmoidal)"));
	app->actionFitGauss->setText(app->tr("Fit &Gaussian"));
	app->actionFitLorentz->setText(app->tr("Fit Lorent&zian"));

	app->actionShowFitDialog->setText(app->tr("Fit &Wizard..."));
	app->actionShowFitDialog->setShortcut(app->tr("Ctrl+Y"));

	app->actionShowPlotDialog->setText(app->tr("&Plot ..."));
	app->actionShowScaleDialog->setText(app->tr("&Scales..."));
	app->actionShowAxisDialog->setText(app->tr("&Axes..."));
	app->actionShowGridDialog->setText(app->tr("&Grid ..."));
	app->actionShowTitleDialog->setText(app->tr("&Title ..."));
	app->actionShowColumnOptionsDialog->setText(app->tr("Column &Options ..."));
	app->actionShowColumnOptionsDialog->setShortcut(app->tr("Ctrl+Alt+O"));
	app->actionShowColumnValuesDialog->setText(app->tr("Set Column &Values ..."));
	app->actionShowColumnValuesDialog->setShortcut(app->tr("Alt+Q"));
	app->actionTableRecalculate->setText(app->tr("Recalculate"));
	app->actionTableRecalculate->setShortcut(app->tr("Ctrl+Return"));
	app->actionHideSelectedColumns->setText(app->tr("&Hide Selected"));
	app->actionHideSelectedColumns->setToolTip(app->tr("Hide selected columns"));
	app->actionShowAllColumns->setText(app->tr("Sho&w All Columns"));
	app->actionHideSelectedColumns->setToolTip(app->tr("Show all table columns"));
	app->actionSwapColumns->setText(app->tr("&Swap columns"));
	app->actionSwapColumns->setToolTip(app->tr("Swap selected columns"));
	app->actionMoveColRight->setText(app->tr("Move &Right"));
    app->actionMoveColRight->setToolTip(app->tr("Move Right"));
	app->actionMoveColLeft->setText(app->tr("Move &Left"));
    app->actionMoveColLeft->setToolTip(app->tr("Move Left"));
	app->actionMoveColFirst->setText(app->tr("Move to F&irst"));
	app->actionMoveColFirst->setToolTip(app->tr("Move to First"));
	app->actionMoveColLast->setText(app->tr("Move to Las&t"));
    app->actionMoveColLast->setToolTip(app->tr("Move to Last"));
	app->actionShowColsDialog->setText(app->tr("&Columns..."));
	app->actionShowRowsDialog->setText(app->tr("&Rows..."));
	app->actionDeleteRows->setText(app->tr("&Delete Rows Interval..."));
	app->actionMoveRowUp->setText(app->tr("&Upward"));
	app->actionMoveRowUp->setToolTip(app->tr("Move current row upward"));
	app->actionMoveRowDown->setText(app->tr("&Downward"));
	app->actionMoveRowDown->setToolTip(app->tr("Move current row downward"));
	app->actionAdjustColumnWidth->setText(app->tr("Ad&just Column Width"));
	app->actionAdjustColumnWidth->setToolTip(app->tr("Set optimal column width"));

	app->actionExtractTableData->setText(app->tr("&Extract Data..."));

	app->actionAbout->setText(app->tr("&About QtiPlot"));
	app->actionAbout->setShortcut(app->tr("F1"));

	app->actionShowHelp->setText(app->tr("&Help"));
	app->actionShowHelp->setShortcut(app->tr("Ctrl+H"));

	app->actionChooseHelpFolder->setText(app->tr("&Choose Help Folder..."));
	app->actionRename->setText(app->tr("&Rename Window") + "...");

	app->actionCloseWindow->setText(app->tr("Close &Window"));

	app->actionAddColToTable->setText(app->tr("Add Column"));
	app->actionAddColToTable->setToolTip(app->tr("Add Column"));
	app->actionAddColToTable->setShortcut(app->tr("Alt+C"));

	app->actionClearTable->setText(app->tr("Clear"));
	app->actionGoToRow->setText(app->tr("&Go to Row..."));
	app->actionGoToRow->setShortcut(app->tr("Ctrl+Alt+G"));

    app->actionGoToColumn->setText(app->tr("Go to Colum&n..."));
	app->actionGoToColumn->setShortcut(app->tr("Ctrl+Alt+C"));

	app->actionDeleteLayer->setText(app->tr("&Remove Layer"));
	app->actionDeleteLayer->setShortcut(app->tr("Alt+R"));

	app->actionResizeActiveWindow->setText(app->tr("Window &Geometry..."));
	app->actionHideActiveWindow->setText(app->tr("&Hide Window"));
	app->actionHideActiveWindow->setShortcut(app->tr("Ctrl+Alt+H"));
	app->actionShowMoreWindows->setText(app->tr("More Windows..."));
	app->actionPixelLineProfile->setText(app->tr("&View Pixel Line Profile"));
	app->actionIntensityTable->setText(app->tr("&Intensity Table"));
	app->actionShowLineDialog->setText(app->tr("&Properties"));
	app->actionShowTextDialog->setText(app->tr("&Properties"));
	app->actionActivateWindow->setText(app->tr("&Activate Window"));
	app->actionMinimizeWindow->setText(app->tr("Mi&nimize Window"));
	app->actionMaximizeWindow->setText(app->tr("Ma&ximize Window"));
	app->actionHideWindow->setText(app->tr("&Hide Window"));
	app->actionHideWindow->setShortcut(app->tr("Ctrl+Alt+H"));
	app->actionResizeWindow->setText(app->tr("Re&size Window..."));
	app->actionEditSurfacePlot->setText(app->tr("&Surface..."));
	app->actionAdd3DData->setText(app->tr("&Data Set..."));
	app->actionSetMatrixProperties->setText(app->tr("Set &Properties..."));
	app->actionSetMatrixDimensions->setText(app->tr("Set &Dimensions..."));
	app->actionSetMatrixDimensions->setShortcut(app->tr("Ctrl+D"));
	app->actionSetMatrixValues->setText(app->tr("Set &Values..."));
	app->actionSetMatrixValues->setToolTip(app->tr("Set Matrix Values"));
    app->actionSetMatrixValues->setShortcut(app->tr("Alt+Q"));
    app->actionImagePlot->setText(app->tr("&Image Plot"));
    app->actionImagePlot->setToolTip(app->tr("Image Plot"));

	app->actionImageProfilesPlot->setText(app->tr("&Image Profiles"));
	app->actionImageProfilesPlot->setToolTip(app->tr("Image Profiles"));

	app->actionTransposeMatrix->setText(app->tr("&Transpose"));
	app->actionRotateMatrix->setText(app->tr("R&otate 90"));
    app->actionRotateMatrix->setToolTip(app->tr("Rotate 90 Clockwise"));
    app->actionRotateMatrixMinus->setText(app->tr("Rotate &-90"));
    app->actionRotateMatrixMinus->setToolTip(app->tr("Rotate 90 Counterclockwise"));
	app->actionFlipMatrixVertically->setText(app->tr("Flip &V"));
	app->actionFlipMatrixVertically->setToolTip(app->tr("Flip Vertically"));
	app->actionFlipMatrixHorizontally->setText(app->tr("Flip &H"));
	app->actionFlipMatrixHorizontally->setToolTip(app->tr("Flip Horizontally"));

    app->actionMatrixXY->setText(app->tr("Show &X/Y"));
    app->actionMatrixColumnRow->setText(app->tr("Show &Column/Row"));
	app->actionViewMatrix->setText(app->tr("&Data mode"));
	app->actionViewMatrixImage->setText(app->tr("&Image mode"));
	app->actionMatrixDefaultScale->setText(app->tr("&Default"));
    app->actionMatrixGrayScale->setText(app->tr("&Gray Scale"));
	app->actionMatrixRainbowScale->setText(app->tr("&Rainbow"));
	app->actionMatrixCustomScale->setText(app->tr("&Custom"));
	app->actionInvertMatrix->setText(app->tr("&Invert"));
	app->actionMatrixDeterminant->setText(app->tr("&Determinant"));
	app->actionConvertMatrixDirect->setText(app->tr("&Direct"));
	app->actionConvertMatrixXYZ->setText(app->tr("&XYZ Columns"));
	app->actionConvertMatrixYXZ->setText(app->tr("&YXZ Columns"));
	app->actionExportMatrix->setText(app->tr("&Export Image ..."));

	app->actionConvertTableDirect->setText(app->tr("&Direct"));
	app->actionConvertTableBinning->setText(app->tr("2D &Binning"));
	app->actionConvertTableRegularXYZ->setText(app->tr("&Regular XYZ"));
#ifdef HAVE_ALGLIB
	app->actionConvertTableRandomXYZ->setText(app->tr("Random &XYZ..."));
	app->actionExpandMatrix->setText(app->tr("&Expand..."));
	app->actionShrinkMatrix->setText(app->tr("&Shrink..."));
	app->actionSmoothMatrix->setText(app->tr("S&mooth"));
#endif
	app->actionPlot3DWireFrame->setText(app->tr("3D &Wire Frame"));
	app->actionPlot3DHiddenLine->setText(app->tr("3D &Hidden Line"));
	app->actionPlot3DPolygons->setText(app->tr("3D &Polygons"));
	app->actionPlot3DWireSurface->setText(app->tr("3D Wire &Surface"));
	app->actionSortTable->setText(app->tr("Sort Ta&ble") + "...");
	app->actionSortSelection->setText(app->tr("&Custom") + "...");
	app->actionNormalizeTable->setText(app->tr("&Table"));
	app->actionNormalizeSelection->setText(app->tr("&Columns"));
	app->actionCorrelate->setText(app->tr("Co&rrelate"));
	app->actionAutoCorrelate->setText(app->tr("&Autocorrelate"));
	app->actionConvolute->setText(app->tr("&Convolute"));
	app->actionDeconvolute->setText(app->tr("&Deconvolute"));
	app->actionTranslateHor->setText(app->tr("&Horizontal"));
	app->actionTranslateVert->setText(app->tr("&Vertical"));
	app->actionSetAscValues->setText(app->tr("Ro&w Numbers"));
	app->actionSetAscValues->setToolTip(app->tr("Fill selected columns with row numbers"));
	app->actionSetRandomValues->setText(app->tr("&Random Values"));
	app->actionSetRandomValues->setToolTip(app->tr("Fill selected columns with random numbers"));
	app->actionSetRandomNormalValues->setText(app->tr("&Normal Random Numbers"));
	app->actionSetRandomNormalValues->setToolTip(app->tr("Fill selected columns with normal random numbers"));
	app->actionChiSquareTest->setText(app->tr("Chi-square Test for &Variance..."));
	app->actionFrequencyCount->setText(app->tr("&Frequency Count ..."));
	app->actionOneSampletTest->setText(app->tr("&One Sample t-Test..."));
	app->actionTwoSampletTest->setText(app->tr("&Two Sample t-Test..."));
	app->actionShapiroWilk->setText(app->tr("&Normality Test (Shapiro - Wilk)") + "...");
#ifdef HAVE_TAMUANOVA
	app->actionOneWayANOVA->setText(app->tr("&One-Way ANOVA..."));
	app->actionTwoWayANOVA->setText(app->tr("&Two-Way ANOVA..."));
#endif
	app->actionSetXCol->setText(app->tr("&X"));
	app->actionSetXCol->setToolTip(app->tr("Set column as X"));
	app->actionSetYCol->setText(app->tr("&Y"));
	app->actionSetYCol->setToolTip(app->tr("Set column as Y"));
	app->actionSetZCol->setText(app->tr("&Z"));
	app->actionSetZCol->setToolTip(app->tr("Set column as Z"));
	app->actionSetXErrCol->setText(app->tr("X E&rror"));
	app->actionSetYErrCol->setText(app->tr("Y &Error"));
	app->actionSetYErrCol->setToolTip(app->tr("Set as Y Error Bars"));
	app->actionSetLabelCol->setText(app->tr("&Label"));
	app->actionSetLabelCol->setToolTip(app->tr("Set as Labels"));
	app->actionDisregardCol->setText(app->tr("&Disregard"));
	app->actionDisregardCol->setToolTip(app->tr("Disregard Columns"));
	app->actionReadOnlyCol->setText(app->tr("&Read Only"));

	app->actionBoxPlot->setText(app->tr("&Box Plot"));
	app->actionBoxPlot->setToolTip(app->tr("Box and whiskers plot"));

	app->actionSubtractReference->setText(app->tr("&Reference Data..."));
	app->actionSubtractLine->setText(app->tr("&Straight Line..."));
	app->actionMultiPeakGauss->setText(app->tr("&Gaussian..."));
	app->actionMultiPeakLorentz->setText(app->tr("&Lorentzian..."));
	app->actionHomePage->setText(app->tr("&QtiPlot Homepage"));
	app->actionCheckUpdates->setText(app->tr("Search for &Updates"));
	app->actionHelpForums->setText(app->tr("Visit QtiPlot &Forums"));
	app->actionHelpBugReports->setText(app->tr("Report a &Bug"));
	app->actionDownloadManual->setText(app->tr("Download &Manual"));
	app->actionTranslations->setText(app->tr("&Translations"));
	app->actionDonate->setText(app->tr("Make a &Donation"));
	app->actionTechnicalSupport->setText(app->tr("Technical &Support"));

#ifdef SCRIPTING_PYTHON
	app->actionScriptingLang->setText(app->tr("Scripting &language") + "...");
	app->actionCommentSelection->setText(app->tr("Commen&t Selection"));
	app->actionCommentSelection->setToolTip(app->tr("Comment Selection"));
	app->actionCommentSelection->setShortcut(app->tr("Ctrl+Shift+O"));

	app->actionUncommentSelection->setText(app->tr("&Uncomment Selection"));
	app->actionUncommentSelection->setToolTip(app->tr("Uncomment Selection"));
	app->actionUncommentSelection->setShortcut(app->tr("Ctrl+Shift+U"));
#endif
	app->actionRestartScripting->setText(app->tr("&Restart scripting"));

	app->actionNoteExecute->setText(app->tr("E&xecute"));
	app->actionNoteExecute->setToolTip(app->tr("Execute Selected Lines"));
	app->actionNoteExecute->setShortcut(app->tr("Ctrl+J"));

	app->actionNoteExecuteAll->setText(app->tr("Execute &All"));
	app->actionNoteExecuteAll->setShortcut(app->tr("Ctrl+Shift+J"));

	app->actionNoteEvaluate->setText(app->tr("&Evaluate Expression"));
	app->actionNoteEvaluate->setShortcut(app->tr("Ctrl+Return"));

	app->actionNoteStop->setText(app->tr("&Stop Execution"));
	app->actionNoteStop->setShortcut(app->tr("Ctrl+Break"));

	app->actionShowNoteLineNumbers->setText(app->tr("Show Line &Numbers"));
	app->actionRenameNoteTab->setText(app->tr("Rena&me Tab..."));
	app->actionAddNoteTab->setText(app->tr("A&dd Tab"));
	app->actionCloseNoteTab->setText(app->tr("C&lose Tab"));

	app->actionFind->setText(app->tr("&Find..."));
	app->actionFind->setToolTip(app->tr("Show find dialog"));
	app->actionFind->setShortcut(app->tr("Ctrl+Alt+F"));

	app->actionFindNext->setText(app->tr("Find &Next"));
	app->actionFindNext->setToolTip(app->tr("Find Next"));
	app->actionFindNext->setShortcut(app->tr("F3"));

	app->actionFindPrev->setText(app->tr("Find &Previous"));
	app->actionFindPrev->setToolTip(app->tr("Find Previous"));
	app->actionFindPrev->setShortcut(app->tr("F4"));

	app->actionReplace->setText(app->tr("&Replace..."));
	app->actionReplace->setToolTip(app->tr("Show replace dialog"));
	app->actionReplace->setShortcut(app->tr("Ctrl+R"));

	app->actionIncreaseIndent->setToolTip(app->tr("Increase Indent"));
	app->actionDecreaseIndent->setToolTip(app->tr("Decrease Indent"));

	app->btnPointer->setText(app->tr("Disable &tools"));
	app->btnPointer->setToolTip( app->tr( "Pointer" ) );

	app->actionMagnify->setText(app->tr("Zoom &In/Out and Drag Canvas"));
	app->actionMagnify->setToolTip(app->tr("Zoom In (Shift++) or Out (-) and Drag Canvas"));

	app->actionMagnifyHor->setText(app->tr("Zoom/Drag Canvas &Horizontally"));
	app->actionMagnifyVert->setText(app->tr("Zoom/Drag Canvas &Vertically"));

	app->actionMagnifyHor->setToolTip(app->tr("Zoom In/Out and Drag Canvas Horizontally"));
	app->actionMagnifyVert->setToolTip(app->tr("Zoom In/Out and Drag Canvas Vertically"));

	app->btnZoomIn->setText(app->tr("&Zoom In"));
	app->btnZoomIn->setShortcut(app->tr("Ctrl++"));
	app->btnZoomIn->setToolTip(app->tr("Zoom In"));

	app->btnZoomOut->setText(app->tr("Zoom &Out"));
	app->btnZoomOut->setShortcut(app->tr("Ctrl+-"));
	app->btnZoomOut->setToolTip(app->tr("Zoom Out"));

	app->btnCursor->setText(app->tr("&Data Reader"));
	app->btnCursor->setShortcut(app->tr("CTRL+D"));
	app->btnCursor->setToolTip(app->tr("Data reader"));

	app->btnSelect->setText(app->tr("&Select Data Range"));
	app->btnSelect->setShortcut(QKeySequence(app->tr("Alt+Shift+S")));
	app->btnSelect->setToolTip(app->tr("Select data range"));

	app->btnPicker->setText(app->tr("S&creen Reader"));
	app->btnPicker->setToolTip(app->tr("Screen reader"));

    app->actionDrawPoints->setText(app->tr("&Draw Data Points"));
    app->actionDrawPoints->setToolTip(app->tr("Draw Data Points"));

	app->btnMovePoints->setText(app->tr("&Move Data Points..."));
	app->btnMovePoints->setShortcut(app->tr("Ctrl+ALT+M"));
	app->btnMovePoints->setToolTip(app->tr("Move data points"));

	app->actionDragCurve->setText(app->tr("Dra&g Curve"));
	app->actionDragCurve->setToolTip(app->tr("Drag Curve"));

	app->btnRemovePoints->setText(app->tr("Remove &Bad Data Points..."));
	app->btnRemovePoints->setShortcut(app->tr("Alt+B"));
	app->btnRemovePoints->setToolTip(app->tr("Remove data points"));

	app->actionAddText->setText(app->tr("Add &Text"));
	app->actionAddText->setToolTip(app->tr("Add Text"));
	app->actionAddText->setShortcut(QKeySequence(app->tr("Shift+T")));

	app->actionAddFormula->setText(app->tr("Add E&quation"));
	app->actionAddFormula->setToolTip(app->tr("Add Equation"));
	app->actionAddFormula->setShortcut( app->tr("ALT+Q") );

    app->actionAddRectangle->setText(app->tr("Add &Rectangle"));
    app->actionAddRectangle->setToolTip(app->tr("Add Rectangle"));
	app->actionAddRectangle->setShortcut( app->tr("CTRL+ALT+R") );

	app->actionAddEllipse->setText(app->tr("Add &Ellipse"));
    app->actionAddEllipse->setToolTip(app->tr("Add Ellipse/Circle"));
	app->actionAddEllipse->setShortcut( app->tr("CTRL+ALT+E") );

	app->btnArrow->setText(app->tr("Draw &Arrow"));
	app->btnArrow->setShortcut(app->tr("CTRL+ALT+A"));
	app->btnArrow->setToolTip(app->tr("Draw arrow"));

	app->btnLine->setText(app->tr("Draw &Line"));
	app->btnLine->setShortcut(app->tr("CTRL+ALT+L"));
	app->btnLine->setToolTip(app->tr("Draw line"));

	// FIXME: is setText necessary for action groups?
	//	app->coord->setText( app->tr( "Coordinates" ) );
	//	app->coord->setText( app->tr( "&Coord" ) );
	//  app->coord->setStatusTip( app->tr( "Coordinates" ) );
	app->Box->setText( app->tr( "Box" ) );
	app->Box->setText( app->tr( "Box" ) );
	app->Box->setToolTip( app->tr( "Box" ) );
	app->Box->setStatusTip( app->tr( "Box" ) );
	app->Frame->setText( app->tr( "Frame" ) );
	app->Frame->setText( app->tr( "&Frame" ) );
	app->Frame->setToolTip( app->tr( "Frame" ) );
	app->Frame->setStatusTip( app->tr( "Frame" ) );
	app->None->setText( app->tr( "No Axes" ) );
	app->None->setText( app->tr( "No Axes" ) );
	app->None->setToolTip( app->tr( "No axes" ) );
	app->None->setStatusTip( app->tr( "No axes" ) );

	app->front->setToolTip( app->tr( "Front grid" ) );
	app->back->setToolTip( app->tr( "Back grid" ) );
	app->right->setToolTip( app->tr( "Right grid" ) );
	app->left->setToolTip( app->tr( "Left grid" ) );
	app->ceil->setToolTip( app->tr( "Ceiling grid" ) );
	app->floor->setToolTip( app->tr( "Floor grid" ) );

	app->wireframe->setText( app->tr( "Wireframe" ) );
	app->wireframe->setText( app->tr( "Wireframe" ) );
	app->wireframe->setToolTip( app->tr( "Wireframe" ) );
	app->wireframe->setStatusTip( app->tr( "Wireframe" ) );
	app->hiddenline->setText( app->tr( "Hidden Line" ) );
	app->hiddenline->setText( app->tr( "Hidden Line" ) );
	app->hiddenline->setToolTip( app->tr( "Hidden line" ) );
	app->hiddenline->setStatusTip( app->tr( "Hidden line" ) );
	app->polygon->setText( app->tr( "Polygon Only" ) );
	app->polygon->setText( app->tr( "Polygon Only" ) );
	app->polygon->setToolTip( app->tr( "Polygon only" ) );
	app->polygon->setStatusTip( app->tr( "Polygon only" ) );
	app->filledmesh->setText( app->tr( "Mesh & Filled Polygons" ) );
	app->filledmesh->setText( app->tr( "Mesh & Filled Polygons" ) );
	app->filledmesh->setToolTip( app->tr( "Mesh & filled Polygons" ) );
	app->filledmesh->setStatusTip( app->tr( "Mesh & filled Polygons" ) );
	app->pointstyle->setText( app->tr( "Dots" ) );
	app->pointstyle->setText( app->tr( "Dots" ) );
	app->pointstyle->setToolTip( app->tr( "Dots" ) );
	app->pointstyle->setStatusTip( app->tr( "Dots" ) );
	app->barstyle->setText( app->tr( "Bars" ) );
	app->barstyle->setToolTip( app->tr( "Bars" ) );
	app->barstyle->setStatusTip( app->tr( "Bars" ) );
	app->conestyle->setText( app->tr( "Cones" ) );
	app->conestyle->setToolTip( app->tr( "Cones" ) );
	app->conestyle->setStatusTip( app->tr( "Cones" ) );
	app->crossHairStyle->setText( app->tr( "Crosshairs" ) );
	app->crossHairStyle->setToolTip( app->tr( "Crosshairs" ) );
	app->crossHairStyle->setStatusTip( app->tr( "Crosshairs" ) );

	//app->floorstyle->setText( app->tr( "Floor Style" ) );
	//app->floorstyle->setText( app->tr( "Floor Style" ) );
	//app->floorstyle->setStatusTip( app->tr( "Floor Style" ) );
	app->floordata->setText( app->tr( "Floor Data Projection" ) );
	app->floordata->setToolTip( app->tr( "Floor data projection" ) );
	app->floordata->setStatusTip( app->tr( "Floor data projection" ) );
	app->flooriso->setText( app->tr( "Floor Isolines" ) );
	app->flooriso->setToolTip( app->tr( "Floor isolines" ) );
	app->flooriso->setStatusTip( app->tr( "Floor isolines" ) );
	app->floornone->setText( app->tr( "Empty Floor" ) );
	app->floornone->setToolTip( app->tr( "Empty floor" ) );
	app->floornone->setStatusTip( app->tr( "Empty floor" ) );

	app->actionAnimate->setText( app->tr( "Animation" ) );
	app->actionAnimate->setToolTip( app->tr( "Animation" ) );
	app->actionAnimate->setStatusTip( app->tr( "Animation" ) );

	app->actionPerspective->setText( app->tr( "Enable perspective" ) );
	app->actionPerspective->setToolTip( app->tr( "Enable perspective" ) );
	app->actionPerspective->setStatusTip( app->tr( "Enable perspective" ) );

	app->actionResetRotation->setText( app->tr( "Reset rotation" ) );
	app->actionResetRotation->setToolTip( app->tr( "Reset rotation" ) );
	app->actionResetRotation->setStatusTip( app->tr( "Reset rotation" ) );

	app->actionFitFrame->setText( app->tr( "Fit frame to window" ) );
	app->actionFitFrame->setToolTip( app->tr( "Fit frame to window" ) );
	app->actionFitFrame->setStatusTip( app->tr( "Fit frame to window" ) );

}
