/***************************************************************************
    File                 : ActionManager.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Action management and action state synchronization for ApplicationWindow

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

#include "ActionManager.h"
#include "MenuBuilder.h"
#include "ActionTranslator.h"
#include "ApplicationWindow.h"
#include "CustomActionDialog.h"
#include "Folder.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Graph3D.h"
#include "Note.h"
#include "PolarGraph.h"
#include "ScriptEdit.h"
#include "ScriptWindow.h"
#include "TranslateCurveTool.h"
#include "TableStatistics.h"

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QPainter>
#include <QApplication>
#include <QUndoGroup>
#include <QUndoView>
#include <QUndoStack>
#include <QDockWidget>

ActionManager::ActionManager(ApplicationWindow *app)
    : QObject(app), d_app(app)
{
}

ActionManager::~ActionManager()
{
}

QAction* ActionManager::action(const QString &name) const
{
    return d_actions.value(name, nullptr);
}

void ActionManager::registerAction(const QString &name, QAction *act)
{
    if (act) {
        d_actions[name] = act;
        act->setObjectName(name);
    }
}

QList<QAction *> ActionManager::customActionsList() const
{
    return d_app ? d_app->d_user_actions : QList<QAction *>();
}

QList<QMenu *> ActionManager::customMenusList() const
{
    return d_app ? d_app->d_user_menus : QList<QMenu *>();
}

void ActionManager::initToolBars() { MenuBuilder::initToolBars(d_app); }
void ActionManager::initPlot3DToolBar() { MenuBuilder::initPlot3DToolBar(d_app); }
void ActionManager::initMainMenu() { MenuBuilder::initMainMenu(d_app); }
void ActionManager::translateActionsStrings() { ActionTranslator::translateActionsStrings(d_app); }
void ActionManager::tableMenuAboutToShow() { MenuBuilder::tableMenuAboutToShow(d_app); }
void ActionManager::plotDataMenuAboutToShow() { MenuBuilder::plotDataMenuAboutToShow(d_app); }
void ActionManager::plotMenuAboutToShow() { MenuBuilder::plotMenuAboutToShow(d_app); }
void ActionManager::scriptingMenuAboutToShow() { MenuBuilder::scriptingMenuAboutToShow(d_app); }
void ActionManager::analysisMenuAboutToShow() { MenuBuilder::analysisMenuAboutToShow(d_app); }
void ActionManager::matrixMenuAboutToShow() { MenuBuilder::matrixMenuAboutToShow(d_app); }
void ActionManager::fileMenuAboutToShow() { MenuBuilder::fileMenuAboutToShow(d_app); }
void ActionManager::editMenuAboutToShow() { MenuBuilder::editMenuAboutToShow(d_app); }
void ActionManager::windowsMenuAboutToShow() { MenuBuilder::windowsMenuAboutToShow(d_app); }

void ActionManager::createActions()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &Box = d_app->Box;
	auto &Frame = d_app->Frame;
	auto &actionAbout = d_app->actionAbout;
	auto &actionActivateWindow = d_app->actionActivateWindow;
	auto &actionAdd3DData = d_app->actionAdd3DData;
	auto &actionAddColToTable = d_app->actionAddColToTable;
	auto &actionAddErrorBars = d_app->actionAddErrorBars;
	auto &actionAddFunctionCurve = d_app->actionAddFunctionCurve;
	auto &actionAddImage = d_app->actionAddImage;
	auto &actionAddInsetCurveLayer = d_app->actionAddInsetCurveLayer;
	auto &actionAddInsetLayer = d_app->actionAddInsetLayer;
	auto &actionAddLayer = d_app->actionAddLayer;
	auto &actionAddNoteTab = d_app->actionAddNoteTab;
	auto &actionAddZoomPlot = d_app->actionAddZoomPlot;
	auto &actionAdjustColumnWidth = d_app->actionAdjustColumnWidth;
	auto &actionAlignBottom = d_app->actionAlignBottom;
	auto &actionAlignLeft = d_app->actionAlignLeft;
	auto &actionAlignRight = d_app->actionAlignRight;
	auto &actionAlignTop = d_app->actionAlignTop;
	auto &actionAppendProject = d_app->actionAppendProject;
	auto &actionAutoCorrelate = d_app->actionAutoCorrelate;
	auto &actionAutomaticLayout = d_app->actionAutomaticLayout;
	auto &actionBandBlockFilter = d_app->actionBandBlockFilter;
	auto &actionBandPassFilter = d_app->actionBandPassFilter;
	auto &actionBaseline = d_app->actionBaseline;
	auto &actionBoxPlot = d_app->actionBoxPlot;
	auto &actionCheckUpdates = d_app->actionCheckUpdates;
	auto &actionChiSquareTest = d_app->actionChiSquareTest;
	auto &actionChooseHelpFolder = d_app->actionChooseHelpFolder;
	auto &actionClearLogInfo = d_app->actionClearLogInfo;
	auto &actionClearSelection = d_app->actionClearSelection;
	auto &actionClearTable = d_app->actionClearTable;
	auto &actionCloseAllWindows = d_app->actionCloseAllWindows;
	auto &actionCloseNoteTab = d_app->actionCloseNoteTab;
	auto &actionCloseProject = d_app->actionCloseProject;
	auto &actionCloseWindow = d_app->actionCloseWindow;
	auto &actionColorMap = d_app->actionColorMap;
	auto &actionContourMap = d_app->actionContourMap;
	auto &actionConvertMatrixDirect = d_app->actionConvertMatrixDirect;
	auto &actionConvertMatrixXYZ = d_app->actionConvertMatrixXYZ;
	auto &actionConvertMatrixYXZ = d_app->actionConvertMatrixYXZ;
	auto &actionConvertTableBinning = d_app->actionConvertTableBinning;
	auto &actionConvertTableDirect = d_app->actionConvertTableDirect;
	auto &actionConvertTableRegularXYZ = d_app->actionConvertTableRegularXYZ;
	auto &actionConvolute = d_app->actionConvolute;
	auto &actionCopySelection = d_app->actionCopySelection;
	auto &actionCopyWindow = d_app->actionCopyWindow;
	auto &actionCorrelate = d_app->actionCorrelate;
	auto &actionCurveFullRange = d_app->actionCurveFullRange;
	auto &actionCustomActionDialog = d_app->actionCustomActionDialog;
	auto &actionCustomLayout = d_app->actionCustomLayout;
	auto &actionCustomSharedAxisLayers = d_app->actionCustomSharedAxisLayers;
	auto &actionCutSelection = d_app->actionCutSelection;
	auto &actionDeconvolute = d_app->actionDeconvolute;
	auto &actionDecreaseIndent = d_app->actionDecreaseIndent;
	auto &actionDecreasePrecision = d_app->actionDecreasePrecision;
	auto &actionDeleteFitTables = d_app->actionDeleteFitTables;
	auto &actionDeleteLayer = d_app->actionDeleteLayer;
	auto &actionDeleteRows = d_app->actionDeleteRows;
	auto &actionDifferentiate = d_app->actionDifferentiate;
	auto &actionDisregardCol = d_app->actionDisregardCol;
	auto &actionDonate = d_app->actionDonate;
	auto &actionDownloadManual = d_app->actionDownloadManual;
	auto &actionEditCurveRange = d_app->actionEditCurveRange;
	auto &actionEditSurfacePlot = d_app->actionEditSurfacePlot;
	auto &actionExportAllGraphs = d_app->actionExportAllGraphs;
	auto &actionExportExcel = d_app->actionExportExcel;
	auto &actionExportGraph = d_app->actionExportGraph;
	auto &actionExportLayer = d_app->actionExportLayer;
	auto &actionExportMatrix = d_app->actionExportMatrix;
	auto &actionExportOds = d_app->actionExportOds;
	auto &actionExportPDF = d_app->actionExportPDF;
	auto &actionExtractGraphs = d_app->actionExtractGraphs;
	auto &actionExtractLayers = d_app->actionExtractLayers;
	auto &actionExtractTableData = d_app->actionExtractTableData;
	auto &actionFFT = d_app->actionFFT;
	auto &actionFind = d_app->actionFind;
	auto &actionFindNext = d_app->actionFindNext;
	auto &actionFindPrev = d_app->actionFindPrev;
	auto &actionFindWindow = d_app->actionFindWindow;
	auto &actionFitExpGrowth = d_app->actionFitExpGrowth;
	auto &actionFitGauss = d_app->actionFitGauss;
	auto &actionFitLinear = d_app->actionFitLinear;
	auto &actionFitLorentz = d_app->actionFitLorentz;
	auto &actionFitSigmoidal = d_app->actionFitSigmoidal;
	auto &actionFitSlope = d_app->actionFitSlope;
	auto &actionFlipMatrixHorizontally = d_app->actionFlipMatrixHorizontally;
	auto &actionFlipMatrixVertically = d_app->actionFlipMatrixVertically;
	auto &actionFontBold = d_app->actionFontBold;
	auto &actionFontItalic = d_app->actionFontItalic;
	auto &actionFrequencyCount = d_app->actionFrequencyCount;
	auto &actionGoToColumn = d_app->actionGoToColumn;
	auto &actionGoToRow = d_app->actionGoToRow;
	auto &actionGrayMap = d_app->actionGrayMap;
	auto &actionGreekMajSymbol = d_app->actionGreekMajSymbol;
	auto &actionGreekSymbol = d_app->actionGreekSymbol;
	auto &actionHelpBugReports = d_app->actionHelpBugReports;
	auto &actionHelpForums = d_app->actionHelpForums;
	auto &actionHideActiveWindow = d_app->actionHideActiveWindow;
	auto &actionHideCurve = d_app->actionHideCurve;
	auto &actionHideOtherCurves = d_app->actionHideOtherCurves;
	auto &actionHideSelectedColumns = d_app->actionHideSelectedColumns;
	auto &actionHideWindow = d_app->actionHideWindow;
	auto &actionHighPassFilter = d_app->actionHighPassFilter;
	auto &actionHomePage = d_app->actionHomePage;
	auto &actionHorSharedAxisLayers = d_app->actionHorSharedAxisLayers;
	auto &actionImagePlot = d_app->actionImagePlot;
	auto &actionImageProfilesPlot = d_app->actionImageProfilesPlot;
	auto &actionImportDatabase = d_app->actionImportDatabase;
	auto &actionImportImage = d_app->actionImportImage;
	auto &actionImportSound = d_app->actionImportSound;
	auto &actionIncreaseIndent = d_app->actionIncreaseIndent;
	auto &actionIncreasePrecision = d_app->actionIncreasePrecision;
	auto &actionIntegrate = d_app->actionIntegrate;
	auto &actionIntensityTable = d_app->actionIntensityTable;
	auto &actionInterpolate = d_app->actionInterpolate;
	auto &actionInvertMatrix = d_app->actionInvertMatrix;
	auto &actionLoad = d_app->actionLoad;
	auto &actionLoadImage = d_app->actionLoadImage;
	auto &actionLowPassFilter = d_app->actionLowPassFilter;
	auto &actionLowerEnrichment = d_app->actionLowerEnrichment;
	auto &actionMagnify = d_app->actionMagnify;
	auto &actionMagnifyHor = d_app->actionMagnifyHor;
	auto &actionMagnifyVert = d_app->actionMagnifyVert;
	auto &actionMathSymbol = d_app->actionMathSymbol;
	auto &actionMatrixColumnRow = d_app->actionMatrixColumnRow;
	auto &actionMatrixCustomScale = d_app->actionMatrixCustomScale;
	auto &actionMatrixDefaultScale = d_app->actionMatrixDefaultScale;
	auto &actionMatrixDeterminant = d_app->actionMatrixDeterminant;
	auto &actionMatrixFFTDirect = d_app->actionMatrixFFTDirect;
	auto &actionMatrixFFTInverse = d_app->actionMatrixFFTInverse;
	auto &actionMatrixGrayScale = d_app->actionMatrixGrayScale;
	auto &actionMatrixRainbowScale = d_app->actionMatrixRainbowScale;
	auto &actionMatrixXY = d_app->actionMatrixXY;
	auto &actionMaximizeWindow = d_app->actionMaximizeWindow;
	auto &actionMinimizeWindow = d_app->actionMinimizeWindow;
	auto &actionMoveColFirst = d_app->actionMoveColFirst;
	auto &actionMoveColLast = d_app->actionMoveColLast;
	auto &actionMoveColLeft = d_app->actionMoveColLeft;
	auto &actionMoveColRight = d_app->actionMoveColRight;
	auto &actionMoveRowDown = d_app->actionMoveRowDown;
	auto &actionMoveRowUp = d_app->actionMoveRowUp;
	auto &actionMultiPeakGauss = d_app->actionMultiPeakGauss;
	auto &actionMultiPeakLorentz = d_app->actionMultiPeakLorentz;
	auto &actionNewFolder = d_app->actionNewFolder;
	auto &actionNewFunctionPlot = d_app->actionNewFunctionPlot;
	auto &actionNewGraph = d_app->actionNewGraph;
	auto &actionNewLegend = d_app->actionNewLegend;
	auto &actionNewMatrix = d_app->actionNewMatrix;
	auto &actionNewNote = d_app->actionNewNote;
	auto &actionNewProject = d_app->actionNewProject;
	auto &actionNewSurfacePlot = d_app->actionNewSurfacePlot;
	auto &actionNewTable = d_app->actionNewTable;
	auto &actionNextWindow = d_app->actionNextWindow;
	auto &actionNormalizeSelection = d_app->actionNormalizeSelection;
	auto &actionNormalizeTable = d_app->actionNormalizeTable;
	auto &actionNoteEvaluate = d_app->actionNoteEvaluate;
	auto &actionNoteExecute = d_app->actionNoteExecute;
	auto &actionNoteExecuteAll = d_app->actionNoteExecuteAll;
	auto &actionNoteStop = d_app->actionNoteStop;
	auto &actionOneSampletTest = d_app->actionOneSampletTest;
	auto &actionOpen = d_app->actionOpen;
	auto &actionOpenExcel = d_app->actionOpenExcel;
	auto &actionOpenOds = d_app->actionOpenOds;
	auto &actionOpenTemplate = d_app->actionOpenTemplate;
	auto &actionPasteSelection = d_app->actionPasteSelection;
	auto &actionPixelLineProfile = d_app->actionPixelLineProfile;
	auto &actionPlot2HorizontalLayers = d_app->actionPlot2HorizontalLayers;
	auto &actionPlot2VerticalLayers = d_app->actionPlot2VerticalLayers;
	auto &actionPlot3DBars = d_app->actionPlot3DBars;
	auto &actionPlot3DHiddenLine = d_app->actionPlot3DHiddenLine;
	auto &actionPlot3DPolygons = d_app->actionPlot3DPolygons;
	auto &actionPlot3DRibbon = d_app->actionPlot3DRibbon;
	auto &actionPlot3DScatter = d_app->actionPlot3DScatter;
	auto &actionPlot3DTrajectory = d_app->actionPlot3DTrajectory;
	auto &actionPlot3DWireFrame = d_app->actionPlot3DWireFrame;
	auto &actionPlot3DWireSurface = d_app->actionPlot3DWireSurface;
	auto &actionPlot4Layers = d_app->actionPlot4Layers;
	auto &actionPlotArea = d_app->actionPlotArea;
	auto &actionPlotDoubleYAxis = d_app->actionPlotDoubleYAxis;
	auto &actionPlotHistogram = d_app->actionPlotHistogram;
	auto &actionPlotHorSteps = d_app->actionPlotHorSteps;
	auto &actionPlotHorizontalBars = d_app->actionPlotHorizontalBars;
	auto &actionPlotL = d_app->actionPlotL;
	auto &actionPlotLP = d_app->actionPlotLP;
	auto &actionPlotP = d_app->actionPlotP;
	auto &actionPlotPie = d_app->actionPlotPie;
	auto &actionPlotPolar = d_app->actionPlotPolar;
	auto &actionPlotSpline = d_app->actionPlotSpline;
	auto &actionPlotStackedHistograms = d_app->actionPlotStackedHistograms;
	auto &actionPlotStackedLayers = d_app->actionPlotStackedLayers;
	auto &actionPlotVectXYAM = d_app->actionPlotVectXYAM;
	auto &actionPlotVectXYXY = d_app->actionPlotVectXYXY;
	auto &actionPlotVertSteps = d_app->actionPlotVertSteps;
	auto &actionPlotVerticalBars = d_app->actionPlotVerticalBars;
	auto &actionPlotVerticalDropLines = d_app->actionPlotVerticalDropLines;
	auto &actionPresentationODF = d_app->actionPresentationODF;
	auto &actionPrevWindow = d_app->actionPrevWindow;
	auto &actionPrint = d_app->actionPrint;
	auto &actionPrintAllPlots = d_app->actionPrintAllPlots;
	auto &actionPrintPreview = d_app->actionPrintPreview;
	auto &actionRaiseEnrichment = d_app->actionRaiseEnrichment;
	auto &actionReadOnlyCol = d_app->actionReadOnlyCol;
	auto &actionRedo = d_app->actionRedo;
	auto &actionRemoveCurve = d_app->actionRemoveCurve;
	auto &actionRename = d_app->actionRename;
	auto &actionRenameNoteTab = d_app->actionRenameNoteTab;
	auto &actionReplace = d_app->actionReplace;
	auto &actionResizeActiveWindow = d_app->actionResizeActiveWindow;
	auto &actionResizeWindow = d_app->actionResizeWindow;
	auto &actionRestartScripting = d_app->actionRestartScripting;
	auto &actionRotateMatrix = d_app->actionRotateMatrix;
	auto &actionRotateMatrixMinus = d_app->actionRotateMatrixMinus;
	auto &actionSaveNote = d_app->actionSaveNote;
	auto &actionSaveProject = d_app->actionSaveProject;
	auto &actionSaveProjectAs = d_app->actionSaveProjectAs;
	auto &actionSaveTemplate = d_app->actionSaveTemplate;
	auto &actionSaveWindow = d_app->actionSaveWindow;
	auto &actionScriptingLang = d_app->actionScriptingLang;
	auto &actionSetAscValues = d_app->actionSetAscValues;
	auto &actionSetLabelCol = d_app->actionSetLabelCol;
	auto &actionSetMatrixDimensions = d_app->actionSetMatrixDimensions;
	auto &actionSetMatrixProperties = d_app->actionSetMatrixProperties;
	auto &actionSetMatrixValues = d_app->actionSetMatrixValues;
	auto &actionSetRandomNormalValues = d_app->actionSetRandomNormalValues;
	auto &actionSetRandomValues = d_app->actionSetRandomValues;
	auto &actionSetXCol = d_app->actionSetXCol;
	auto &actionSetXErrCol = d_app->actionSetXErrCol;
	auto &actionSetYCol = d_app->actionSetYCol;
	auto &actionSetYErrCol = d_app->actionSetYErrCol;
	auto &actionSetZCol = d_app->actionSetZCol;
	auto &actionShapiroWilk = d_app->actionShapiroWilk;
	auto &actionSharedAxesLayers = d_app->actionSharedAxesLayers;
	auto &actionShowAllColumns = d_app->actionShowAllColumns;
	auto &actionShowAllCurves = d_app->actionShowAllCurves;
	auto &actionShowAxisDialog = d_app->actionShowAxisDialog;
	auto &actionShowColStatistics = d_app->actionShowColStatistics;
	auto &actionShowColsDialog = d_app->actionShowColsDialog;
	auto &actionShowColumnOptionsDialog = d_app->actionShowColumnOptionsDialog;
	auto &actionShowColumnValuesDialog = d_app->actionShowColumnValuesDialog;
	auto &actionShowConfigureDialog = d_app->actionShowConfigureDialog;
	auto &actionShowCurvePlotDialog = d_app->actionShowCurvePlotDialog;
	auto &actionShowCurveWorksheet = d_app->actionShowCurveWorksheet;
	auto &actionShowCurvesDialog = d_app->actionShowCurvesDialog;
	auto &actionShowExpDecay3Dialog = d_app->actionShowExpDecay3Dialog;
	auto &actionShowExpDecayDialog = d_app->actionShowExpDecayDialog;
	auto &actionShowExplorer = d_app->actionShowExplorer;
	auto &actionShowExportASCIIDialog = d_app->actionShowExportASCIIDialog;
	auto &actionShowFitDialog = d_app->actionShowFitDialog;
	auto &actionShowFitPolynomDialog = d_app->actionShowFitPolynomDialog;
	auto &actionShowGridDialog = d_app->actionShowGridDialog;
	auto &actionShowHelp = d_app->actionShowHelp;
	auto &actionShowIntDialog = d_app->actionShowIntDialog;
	auto &actionShowLayerDialog = d_app->actionShowLayerDialog;
	auto &actionShowLineDialog = d_app->actionShowLineDialog;
	auto &actionShowLog = d_app->actionShowLog;
	auto &actionShowMoreWindows = d_app->actionShowMoreWindows;
	auto &actionShowNoteLineNumbers = d_app->actionShowNoteLineNumbers;
	auto &actionShowPlotDialog = d_app->actionShowPlotDialog;
	auto &actionShowPlotWizard = d_app->actionShowPlotWizard;
	auto &actionShowRowStatistics = d_app->actionShowRowStatistics;
	auto &actionShowRowsDialog = d_app->actionShowRowsDialog;
	auto &actionShowScaleDialog = d_app->actionShowScaleDialog;
	auto &actionShowScriptWindow = d_app->actionShowScriptWindow;
	auto &actionShowTextDialog = d_app->actionShowTextDialog;
	auto &actionShowTitleDialog = d_app->actionShowTitleDialog;
	auto &actionShowTwoExpDecayDialog = d_app->actionShowTwoExpDecayDialog;
	auto &actionShowUndoStack = d_app->actionShowUndoStack;
	auto &actionSmoothAverage = d_app->actionSmoothAverage;
	auto &actionSmoothFFT = d_app->actionSmoothFFT;
	auto &actionSmoothLowess = d_app->actionSmoothLowess;
	auto &actionSmoothSavGol = d_app->actionSmoothSavGol;
	auto &actionSortSelection = d_app->actionSortSelection;
	auto &actionSortTable = d_app->actionSortTable;
	auto &actionStackBars = d_app->actionStackBars;
	auto &actionStackColumns = d_app->actionStackColumns;
	auto &actionStackSharedAxisLayers = d_app->actionStackSharedAxisLayers;
	auto &actionStemPlot = d_app->actionStemPlot;
	auto &actionSubscript = d_app->actionSubscript;
	auto &actionSubtractLine = d_app->actionSubtractLine;
	auto &actionSubtractReference = d_app->actionSubtractReference;
	auto &actionSuperscript = d_app->actionSuperscript;
	auto &actionSwapColumns = d_app->actionSwapColumns;
	auto &actionTableRecalculate = d_app->actionTableRecalculate;
	auto &actionTechnicalSupport = d_app->actionTechnicalSupport;
	auto &actionTimeStamp = d_app->actionTimeStamp;
	auto &actionToolBars = d_app->actionToolBars;
	auto &actionTranslateHor = d_app->actionTranslateHor;
	auto &actionTranslateVert = d_app->actionTranslateVert;
	auto &actionTranslations = d_app->actionTranslations;
	auto &actionTransposeMatrix = d_app->actionTransposeMatrix;
	auto &actionTwoSampletTest = d_app->actionTwoSampletTest;
	auto &actionUnderline = d_app->actionUnderline;
	auto &actionUndo = d_app->actionUndo;
	auto &actionUnzoom = d_app->actionUnzoom;
	auto &actionVertSharedAxisLayers = d_app->actionVertSharedAxisLayers;
	auto &actionViewMatrix = d_app->actionViewMatrix;
	auto &actionViewMatrixImage = d_app->actionViewMatrixImage;
	auto &actionWaterfallPlot = d_app->actionWaterfallPlot;
	auto &appFont = d_app->appFont;
	auto &d_undo_group = d_app->d_undo_group;
	auto &explorerWindow = d_app->explorerWindow;
	auto &logWindow = d_app->logWindow;
	auto &undoStackWindow = d_app->undoStackWindow;
	auto &view = d_app->view;
#ifdef HAVE_ALGLIB
	auto &actionConvertTableRandomXYZ = d_app->actionConvertTableRandomXYZ;
	auto &actionExpandMatrix = d_app->actionExpandMatrix;
	auto &actionShrinkMatrix = d_app->actionShrinkMatrix;
	auto &actionSmoothMatrix = d_app->actionSmoothMatrix;
#endif
#ifdef HAVE_TAMUANOVA
	auto &actionOneWayANOVA = d_app->actionOneWayANOVA;
	auto &actionTwoWayANOVA = d_app->actionTwoWayANOVA;
#endif
#ifdef SCRIPTING_CONSOLE
	auto &actionShowConsole = d_app->actionShowConsole;
	auto &consoleWindow = d_app->consoleWindow;
#endif
#ifdef SCRIPTING_PYTHON
	auto &actionCommentSelection = d_app->actionCommentSelection;
	auto &actionOpenQtDesignerUi = d_app->actionOpenQtDesignerUi;
	auto &actionUncommentSelection = d_app->actionUncommentSelection;
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    actionCustomActionDialog = new QAction(d_app->tr("Add &Custom Script Action..."), d_app);
	connect(actionCustomActionDialog, &QAction::triggered, d_app, &ApplicationWindow::showCustomActionDialog);

	actionNewProject = new QAction(QIcon(":/new.png"), d_app->tr("New &Project"), d_app);
	actionNewProject->setShortcut( d_app->tr("Ctrl+N") );
	connect(actionNewProject, &QAction::triggered, d_app, &ApplicationWindow::newProject);

	actionAppendProject = new QAction(QIcon(":/append_file.png"), d_app->tr("App&end Project..."), d_app);
	connect(actionAppendProject, &QAction::triggered, d_app, [app]{app->appendProject();});

	actionNewFolder = new QAction(QIcon(":/newfolder.png"), d_app->tr("New F&older"), d_app);
	actionNewProject->setShortcut(Qt::Key_F7);
	connect(actionNewFolder, &QAction::triggered, d_app, [app]{app->addFolder();});

	actionNewGraph = new QAction(QIcon(":/new_graph.png"), d_app->tr("New &Graph"), d_app);
	actionNewGraph->setShortcut( d_app->tr("Ctrl+G") );
	connect(actionNewGraph, &QAction::triggered, d_app, [app]{app->newGraph();});

	actionNewNote = new QAction(QIcon(":/new_note.png"), d_app->tr("New &Note"), d_app);
	connect(actionNewNote, &QAction::triggered, d_app, [app]{app->newNote();});

	actionNewTable = new QAction(QIcon(":/table.png"), d_app->tr("New &Table"), d_app);
	actionNewTable->setShortcut( d_app->tr("Ctrl+T") );
	connect(actionNewTable, &QAction::triggered, d_app, [app]{app->newTable();});

	actionNewMatrix = new QAction(QIcon(":/new_matrix.png"), d_app->tr("New &Matrix"), d_app);
	actionNewMatrix->setShortcut( d_app->tr("Ctrl+M") );
	connect(actionNewMatrix, &QAction::triggered, d_app, [app]{app->newMatrix();});

	actionNewFunctionPlot = new QAction(QIcon(":/newF.png"), d_app->tr("New &Function Plot") + "...", d_app);
	actionNewFunctionPlot->setShortcut( d_app->tr("Ctrl+F") );
	connect(actionNewFunctionPlot, &QAction::triggered, d_app, &ApplicationWindow::functionDialog);

	actionNewSurfacePlot = new QAction(QIcon(":/newFxy.png"), d_app->tr("New 3D &Surface Plot") + "...", d_app);
	actionNewSurfacePlot->setShortcut( d_app->tr("Ctrl+ALT+Z") );
	connect(actionNewSurfacePlot, &QAction::triggered, d_app, &ApplicationWindow::newSurfacePlot);

	actionOpen = new QAction(QIcon(":/fileopen.png"), d_app->tr("&Open..."), d_app);
	actionOpen->setShortcut( d_app->tr("Ctrl+O") );
	connect(actionOpen, &QAction::triggered, d_app, [app]{app->open();});

	actionExportExcel = new QAction(QIcon(":/new_excel.png"), d_app->tr("Export Exce&l ..."), d_app);
	connect(actionExportExcel, &QAction::triggered, d_app, &ApplicationWindow::exportExcel);

	actionExportOds = new QAction(QIcon(":/new_ods.png"), d_app->tr("Export &Open Document Spreadsheet ..."), d_app);
	connect(actionExportOds, &QAction::triggered, d_app, &ApplicationWindow::exportOds);

	actionOpenExcel = new QAction(QIcon(":/open_excel.png"), d_app->tr("Open Exce&l ..."), d_app);
	actionOpenExcel->setShortcut( d_app->tr("Ctrl+Shift+E") );
	connect(actionOpenExcel, &QAction::triggered, d_app, [app]{app->importExcel();});

	actionOpenOds = new QAction(QIcon(":/ods_spreadsheet.png"), d_app->tr("Open ODF Spreads&heet..."), d_app);
	actionOpenOds->setShortcut( d_app->tr("Ctrl+Alt+S") );
	connect(actionOpenOds, &QAction::triggered, d_app, [app]{app->importOdfSpreadsheet();});

	actionLoadImage = new QAction(d_app->tr("Open Image &File..."), d_app);
	actionLoadImage->setShortcut( d_app->tr("Ctrl+I") );
	connect(actionLoadImage, &QAction::triggered, d_app, [app]{app->loadImage();});

	actionImportImage = new QAction(QPixmap(":/monalisa.png"), d_app->tr("Import I&mage..."), d_app);
	connect(actionImportImage, &QAction::triggered, d_app, [app]{app->importImage();});

	actionSaveProject = new QAction(QIcon(":/filesave.png"), d_app->tr("&Save Project"), d_app);

	actionSaveProject->setShortcut( d_app->tr("Ctrl+S") );
	connect(actionSaveProject, &QAction::triggered, d_app, &ApplicationWindow::saveProject);

	actionSaveProjectAs = new QAction(QIcon(":/filesaveas.png"), d_app->tr("Save Project &As..."), d_app);
	actionSaveProjectAs->setShortcut( d_app->tr("Ctrl+Shift+S") );
	connect(actionSaveProjectAs, &QAction::triggered, d_app, [app]{app->saveProjectAs();});

	actionOpenTemplate = new QAction(QIcon(":/open_template.png"),d_app->tr("Open Temp&late..."), d_app);
	connect(actionOpenTemplate, &QAction::triggered, d_app, [app]{app->openTemplate();});

	actionSaveTemplate = new QAction(QIcon(":/save_template.png"), d_app->tr("Save As &Template..."), d_app);
	connect(actionSaveTemplate, &QAction::triggered, d_app, [app]{app->saveAsTemplate();});

	actionSaveWindow = new QAction(d_app->tr("Save &Window As..."), d_app);
	connect(actionSaveWindow, &QAction::triggered, d_app, [app]{app->saveWindowAs();});

	actionSaveNote = new QAction(QIcon(":/filesaveas.png"), d_app->tr("Save Note As..."), d_app);
	connect(actionSaveNote, &QAction::triggered, d_app, &ApplicationWindow::saveNoteAs);

	actionLoad = new QAction(QIcon(":/import.png"), d_app->tr("&Import ASCII..."), d_app);
	connect(actionLoad, &QAction::triggered, d_app, [app]{app->importASCII();});

	actionImportSound = new QAction(d_app->tr("&Sound (WAV)..."), d_app);
	connect(actionImportSound, &QAction::triggered, d_app, &ApplicationWindow::importWaveFile);

	actionImportDatabase = new QAction(d_app->tr("&Database..."), d_app);
	connect(actionImportDatabase, &QAction::triggered, d_app, [app]{app->importDatabase();});

	actionUndo = new QAction(QIcon(":/undo.png"), d_app->tr("&Undo"), d_app);
	actionUndo->setShortcut(d_app->tr("Ctrl+Z"));
	connect(actionUndo, &QAction::triggered, d_app, &ApplicationWindow::undo);

	actionRedo = new QAction(QIcon(":/redo.png"), d_app->tr("&Redo"), d_app);
	actionRedo->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
	connect(actionRedo, &QAction::triggered, d_app, &ApplicationWindow::redo);

	connect(d_undo_group, &QUndoGroup::canUndoChanged, d_app, [app](bool canUndo) {
		MdiSubWindow *w = app->activeWindow();
		if (w && !qobject_cast<Note*>(w))
			app->actionUndo->setEnabled(canUndo);
	});
	connect(d_undo_group, &QUndoGroup::canRedoChanged, d_app, [app](bool canRedo) {
		MdiSubWindow *w = app->activeWindow();
		if (w && !qobject_cast<Note*>(w))
			app->actionRedo->setEnabled(canRedo);
	});

	actionCopyWindow = new QAction(QIcon(":/duplicate.png"), d_app->tr("&Duplicate"), d_app);
	actionCopyWindow->setShortcut(d_app->tr("Ctrl+Alt+D"));
	connect(actionCopyWindow, &QAction::triggered, d_app, [app]{app->clone();});

	actionCutSelection = new QAction(QIcon(":/cut.png"), d_app->tr("Cu&t Selection"), d_app);
	actionCutSelection->setShortcut( d_app->tr("Ctrl+X") );
	connect(actionCutSelection, &QAction::triggered, d_app, &ApplicationWindow::cutSelection);

	actionCopySelection = new QAction(QIcon(":/copy.png"), d_app->tr("&Copy Selection"), d_app);
	actionCopySelection->setShortcut( d_app->tr("Ctrl+C") );
	connect(actionCopySelection, &QAction::triggered, d_app, &ApplicationWindow::copySelection);

	actionPasteSelection = new QAction(QIcon(":/paste.png"), d_app->tr("&Paste Selection"), d_app);
	actionPasteSelection->setShortcut( d_app->tr("Ctrl+V") );
	connect(actionPasteSelection, &QAction::triggered, d_app, &ApplicationWindow::pasteSelection);

	actionClearSelection = new QAction(QIcon(":/erase.png"), d_app->tr("&Delete Selection"), d_app);
	actionClearSelection->setShortcut( d_app->tr("Del","delete key") );
	connect(actionClearSelection, &QAction::triggered, d_app, &ApplicationWindow::clearSelection);

	actionRaiseEnrichment = new QAction(QIcon(":/raise.png"), d_app->tr("&Front"), d_app);
	connect(actionRaiseEnrichment, &QAction::triggered, d_app, &ApplicationWindow::raiseActiveEnrichment);
	actionRaiseEnrichment->setEnabled(false);

	actionLowerEnrichment = new QAction(QIcon(":/lower.png"), d_app->tr("&Back"), d_app);
	connect(actionLowerEnrichment, &QAction::triggered, d_app, &ApplicationWindow::lowerActiveEnrichment);
	actionLowerEnrichment->setEnabled(false);

	actionAlignTop = new QAction(QIcon(":/align_top.png"), d_app->tr("Align &Top"), d_app);
	connect(actionAlignTop, &QAction::triggered, d_app, &ApplicationWindow::alignTop);
	actionAlignTop->setEnabled(false);

	actionAlignBottom = new QAction(QIcon(":/align_bottom.png"), d_app->tr("Align &Bottom"), d_app);
	connect(actionAlignBottom, &QAction::triggered, d_app, &ApplicationWindow::alignBottom);
	actionAlignBottom->setEnabled(false);

	actionAlignLeft = new QAction(QIcon(":/align_left.png"), d_app->tr("Align &Left"), d_app);
	connect(actionAlignLeft, &QAction::triggered, d_app, &ApplicationWindow::alignLeft);
	actionAlignLeft->setEnabled(false);

	actionAlignRight = new QAction(QIcon(":/align_right.png"), d_app->tr("Align &Right"), d_app);
	connect(actionAlignRight, &QAction::triggered, d_app, &ApplicationWindow::alignRight);
	actionAlignRight->setEnabled(false);

	actionShowExplorer = explorerWindow->toggleViewAction();
	actionShowExplorer->setIcon(QIcon(":/folder.png"));
	actionShowExplorer->setShortcut( d_app->tr("Ctrl+E") );

	actionFindWindow = new QAction(QIcon(":/find.png"), d_app->tr("&Find..."), d_app);
	connect(actionFindWindow, &QAction::triggered, d_app, &ApplicationWindow::showFindDialogue);

	actionShowLog = logWindow->toggleViewAction();
	actionShowLog->setIcon(QIcon(":/log.png"));

    actionShowUndoStack = undoStackWindow->toggleViewAction();

#ifdef SCRIPTING_CONSOLE
	actionShowConsole = consoleWindow->toggleViewAction();
#endif

	actionAddLayer = new QAction(QIcon(":/newLayer.png"), d_app->tr("Add La&yer"), d_app);
	actionAddLayer->setShortcut( d_app->tr("ALT+L") );
	connect(actionAddLayer, &QAction::triggered, d_app, &ApplicationWindow::addLayer);

	actionShowLayerDialog = new QAction(QIcon(":/arrangeLayers.png"), d_app->tr("Arran&ge Layers"), d_app);
	actionShowLayerDialog->setShortcut( d_app->tr("Shift+A") );
	connect(actionShowLayerDialog, &QAction::triggered, d_app, &ApplicationWindow::showLayerDialog);

	actionAutomaticLayout = new QAction(QIcon(":/auto_layout.png"), d_app->tr("Automatic Layout"), d_app);
	connect(actionAutomaticLayout, &QAction::triggered, d_app, &ApplicationWindow::autoArrangeLayers);

	actionExportLayer = new QAction(d_app->tr("&Layer") + "...", d_app);
	actionExportLayer->setShortcut(d_app->tr("Ctrl+Shift+L"));
	connect(actionExportLayer, &QAction::triggered, d_app, &ApplicationWindow::exportLayer);

	actionExportGraph = new QAction(d_app->tr("&Window") + "...", d_app);
	actionExportGraph->setShortcut( d_app->tr("Ctrl+Alt+G") );
	connect(actionExportGraph, &QAction::triggered, d_app, [app]{app->exportGraph();});

	actionExportAllGraphs = new QAction(d_app->tr("&All") + "...", d_app);
	actionExportAllGraphs->setShortcut( d_app->tr("Alt+X") );
	connect(actionExportAllGraphs, &QAction::triggered, d_app, [app]{app->exportAllGraphs();});
	actionPresentationODF = new QAction(d_app->tr("Create Open &Document Presentation..."), d_app);
	connect(actionPresentationODF, &QAction::triggered, d_app, &ApplicationWindow::exportPresentationODF);
	actionExportPDF = new QAction(QIcon(":/pdf.png"), d_app->tr("&Export PDF") + "...", d_app);
	actionExportPDF->setShortcut( d_app->tr("Ctrl+Alt+P") );
	connect(actionExportPDF, &QAction::triggered, d_app, &ApplicationWindow::exportPDF);

	actionPrint = new QAction(QIcon(":/fileprint.png"), d_app->tr("&Print..."), d_app);
	actionPrint->setShortcut( d_app->tr("Ctrl+P") );
	connect(actionPrint, &QAction::triggered, d_app, &ApplicationWindow::print);

	actionPrintPreview = new QAction(QIcon(":/preview.png"), d_app->tr("Print Pre&view..."), d_app);
	connect(actionPrintPreview, &QAction::triggered, d_app, &ApplicationWindow::printPreview);

	actionPrintAllPlots = new QAction(d_app->tr("Print All Plo&ts"), d_app);
	actionPrintAllPlots->setShortcut(d_app->tr("Ctrl+Shift+P"));
	connect(actionPrintAllPlots, &QAction::triggered, d_app, &ApplicationWindow::printAllPlots);

	actionShowExportASCIIDialog = new QAction(d_app->tr("E&xport ASCII..."), d_app);
	connect(actionShowExportASCIIDialog, &QAction::triggered, d_app, &ApplicationWindow::showExportASCIIDialog);

	actionCloseAllWindows = new QAction(QIcon(":/quit.png"), d_app->tr("&Quit"), d_app);
	actionCloseAllWindows->setShortcut( d_app->tr("Ctrl+Q") );
	connect(actionCloseAllWindows, &QAction::triggered, qApp, &QApplication::closeAllWindows);

	actionCloseProject = new QAction(QIcon(":/delete.png"), d_app->tr("&Close"), d_app);
#ifdef BROWSER_PLUGIN
	connect(actionCloseProject, &QAction::triggered, d_app, &ApplicationWindow::closeProject);
#else
	connect(actionCloseProject, &QAction::triggered, d_app, &ApplicationWindow::newProject);
#endif

	actionClearLogInfo = new QAction(d_app->tr("Clear &Log Information"), d_app);
	connect(actionClearLogInfo, &QAction::triggered, d_app, &ApplicationWindow::clearLogInfo);

	actionDeleteFitTables = new QAction(QIcon(":/close.png"), d_app->tr("Delete &Fit Tables"), d_app);
	connect(actionDeleteFitTables, &QAction::triggered, d_app, &ApplicationWindow::deleteFitTables);

	actionShowPlotWizard = new QAction(QIcon(":/wizard.png"), d_app->tr("Plot &Wizard") + "...", d_app);
	actionShowPlotWizard->setShortcut( d_app->tr("Ctrl+Alt+W") );
	connect(actionShowPlotWizard, &QAction::triggered, d_app, &ApplicationWindow::showPlotWizard);

	actionShowConfigureDialog = new QAction(QIcon(":/configure.png"), d_app->tr("&Preferences..."), d_app);
	connect(actionShowConfigureDialog, &QAction::triggered, d_app, &ApplicationWindow::showPreferencesDialog);

	actionShowCurvesDialog = new QAction(QIcon(":/curves.png"), d_app->tr("Add/Remove &Curve..."), d_app);
	actionShowCurvesDialog->setShortcut( d_app->tr("ALT+C") );
	connect(actionShowCurvesDialog, &QAction::triggered, d_app, &ApplicationWindow::showCurvesDialog);

	actionAddErrorBars = new QAction(QIcon(":/errors.png"), d_app->tr("Add &Error Bars..."), d_app);
	actionAddErrorBars->setShortcut( d_app->tr("Ctrl+B") );
	connect(actionAddErrorBars, &QAction::triggered, d_app, &ApplicationWindow::addErrorBars);

	actionAddFunctionCurve = new QAction(QIcon(":/fx.png"), d_app->tr("Add &Function..."), d_app);
	actionAddFunctionCurve->setShortcut( d_app->tr("Ctrl+Alt+F") );
	connect(actionAddFunctionCurve, &QAction::triggered, d_app, &ApplicationWindow::addFunctionCurve);

	actionUnzoom = new QAction(QIcon(":/unzoom.png"), d_app->tr("&Rescale to Show All"), d_app);
	actionUnzoom->setShortcut( d_app->tr("Ctrl+Shift+R") );
	connect(actionUnzoom, &QAction::triggered, d_app, &ApplicationWindow::setAutoScale);

	actionMagnify = new QAction(QIcon(":/magnifier.png"), d_app->tr("Zoom &In/Out and Drag Canvas"), d_app);

	actionMagnifyHor = new QAction(QIcon(":/magnifier_hor.png"), d_app->tr("Zoom/Drag Canvas &Horizontally"), d_app);
	actionMagnifyVert = new QAction(QIcon(":/magnifier_vert.png"), d_app->tr("Zoom/Drag Canvas &Vertically"), d_app);

	actionNewLegend = new QAction(QIcon(":/legend.png"), d_app->tr("New &Legend"), d_app);
	actionNewLegend->setShortcut( d_app->tr("Ctrl+L") );
	connect(actionNewLegend, &QAction::triggered, d_app, &ApplicationWindow::newLegend);

	actionTimeStamp = new QAction(QIcon(":/clock.png"), d_app->tr("Add Time Stamp"), d_app);
	actionTimeStamp->setShortcut( d_app->tr("Ctrl+ALT+T") );
	connect(actionTimeStamp, &QAction::triggered, d_app, &ApplicationWindow::addTimeStamp);

	actionAddImage = new QAction(QIcon(":/monalisa.png"), d_app->tr("Add &Image"), d_app);
	actionAddImage->setShortcut( d_app->tr("ALT+I") );
	connect(actionAddImage, &QAction::triggered, d_app, &ApplicationWindow::addImage);

	actionPlotL = new QAction(QIcon(":/lPlot.png"), d_app->tr("&Line"), d_app);
	connect(actionPlotL, &QAction::triggered, d_app, &ApplicationWindow::plotL);

	actionPlotP = new QAction(QIcon(":/pPlot.png"), d_app->tr("&Scatter"), d_app);
	connect(actionPlotP, &QAction::triggered, d_app, &ApplicationWindow::plotP);

	actionPlotLP = new QAction(QIcon(":/lpPlot.png"), d_app->tr("Line + S&ymbol"), d_app);
	connect(actionPlotLP, &QAction::triggered, d_app, &ApplicationWindow::plotLP);

	actionPlotPolar = new QAction(QIcon(":/lpPlot.png"), d_app->tr("&Polar"), d_app);
	connect(actionPlotPolar, &QAction::triggered, d_app, [app]{app->plotPolar();});

	actionPlotVerticalDropLines = new QAction(QIcon(":/dropLines.png"), d_app->tr("Vertical &Drop Lines"), d_app);
	connect(actionPlotVerticalDropLines, &QAction::triggered, d_app, &ApplicationWindow::plotVerticalDropLines);

	actionPlotSpline = new QAction(QIcon(":/spline.png"), d_app->tr("&Spline"), d_app);
	connect(actionPlotSpline, &QAction::triggered, d_app, &ApplicationWindow::plotSpline);

	actionPlotHorSteps = new QAction(QPixmap(":/hor_steps.png"), d_app->tr("&Horizontal Steps"), d_app);
	connect(actionPlotHorSteps, &QAction::triggered, d_app, &ApplicationWindow::plotHorSteps);

	actionPlotVertSteps = new QAction(QIcon(":/vert_steps.png"), d_app->tr("&Vertical Steps"), d_app);
	connect(actionPlotVertSteps, &QAction::triggered, d_app, &ApplicationWindow::plotVertSteps);

	actionPlotVerticalBars = new QAction(QIcon(":/vertBars.png"), d_app->tr("&Columns"), d_app);
	connect(actionPlotVerticalBars, &QAction::triggered, d_app, &ApplicationWindow::plotVerticalBars);

	actionPlotHorizontalBars = new QAction(QIcon(":/hBars.png"), d_app->tr("&Rows"), d_app);
	connect(actionPlotHorizontalBars, &QAction::triggered, d_app, &ApplicationWindow::plotHorizontalBars);

	actionStackBars = new QAction(QIcon(":/stack_bar.png"), d_app->tr("Stack &Bar"), d_app);
	connect(actionStackBars, &QAction::triggered, d_app, &ApplicationWindow::plotStackBar);

	actionStackColumns = new QAction(QIcon(":/stack_column.png"), d_app->tr("Stack &Column"), d_app);
	connect(actionStackColumns, &QAction::triggered, d_app, &ApplicationWindow::plotStackColumn);

	actionPlotArea = new QAction(QIcon(":/area.png"), d_app->tr("&Area"), d_app);
	connect(actionPlotArea, &QAction::triggered, d_app, &ApplicationWindow::plotArea);

	actionPlotPie = new QAction(QIcon(":/pie.png"), d_app->tr("&Pie"), d_app);
	connect(actionPlotPie, &QAction::triggered, d_app, &ApplicationWindow::plotPie);

	actionPlotVectXYAM = new QAction(QIcon(":/vectXYAM.png"), d_app->tr("Vectors XY&AM"), d_app);
	connect(actionPlotVectXYAM, &QAction::triggered, d_app, &ApplicationWindow::plotVectXYAM);

	actionPlotVectXYXY = new QAction(QIcon(":/vectXYXY.png"), d_app->tr("&Vectors &XYXY"), d_app);
	connect(actionPlotVectXYXY, &QAction::triggered, d_app, &ApplicationWindow::plotVectXYXY);

	actionPlotHistogram = new QAction(QIcon(":/histogram.png"), d_app->tr("&Histogram"), d_app);
	connect(actionPlotHistogram, &QAction::triggered, d_app, [app]{app->plotHistogram();});

	actionPlotStackedHistograms = new QAction(QIcon(":/stacked_hist.png"), d_app->tr("&Stacked Histogram"), d_app);
	connect(actionPlotStackedHistograms, &QAction::triggered, d_app, &ApplicationWindow::plotStackedHistograms);

	actionStemPlot = new QAction(QIcon(":/leaf.png"), d_app->tr("Stem-and-&Leaf Plot"), d_app);
	connect(actionStemPlot, &QAction::triggered, d_app, &ApplicationWindow::newStemPlot);

	actionPlot2VerticalLayers = new QAction(QIcon(":/panel_v2.png"), d_app->tr("&Vertical 2 Layers"), d_app);
	connect(actionPlot2VerticalLayers, &QAction::triggered, d_app, &ApplicationWindow::plot2VerticalLayers);

	actionPlot2HorizontalLayers = new QAction(QIcon(":/panel_h2.png"), d_app->tr("&Horizontal 2 Layers"), d_app);
	connect(actionPlot2HorizontalLayers, &QAction::triggered, d_app, &ApplicationWindow::plot2HorizontalLayers);

	actionPlot4Layers = new QAction(QIcon(":/panel_4.png"), d_app->tr("&4 Layers"), d_app);
	connect(actionPlot4Layers, &QAction::triggered, d_app, &ApplicationWindow::plot4Layers);

	actionPlotStackedLayers = new QAction(QIcon(":/stacked.png"), d_app->tr("&Stacked Layers"), d_app);
	connect(actionPlotStackedLayers, &QAction::triggered, d_app, &ApplicationWindow::plotStackedLayers);

	actionVertSharedAxisLayers = new QAction(QIcon(":/panel_v2.png"), d_app->tr("&Vertical 2 Layers"), d_app);
	connect(actionVertSharedAxisLayers, &QAction::triggered, d_app, &ApplicationWindow::plotVerticalSharedAxisLayers);

	actionHorSharedAxisLayers = new QAction(QIcon(":/panel_h2.png"), d_app->tr("&Horizontal 2 Layers"), d_app);
	connect(actionHorSharedAxisLayers, &QAction::triggered, d_app, &ApplicationWindow::plotHorizontalSharedAxisLayers);

	actionSharedAxesLayers = new QAction(QIcon(":/panel_4.png"), d_app->tr("&4 Layers"), d_app);
	connect(actionSharedAxesLayers, &QAction::triggered, d_app, &ApplicationWindow::plotSharedAxesLayers);

	actionStackSharedAxisLayers = new QAction(QIcon(":/stacked.png"), d_app->tr("&Stacked Layers"), d_app);
	connect(actionStackSharedAxisLayers, &QAction::triggered, d_app, &ApplicationWindow::plotStackSharedAxisLayers);

	actionCustomSharedAxisLayers = new QAction(QIcon(":/arrangeLayers.png"), d_app->tr("&Custom Layout..."), d_app);
	connect(actionCustomSharedAxisLayers, &QAction::triggered, d_app, &ApplicationWindow::plotCustomLayoutSharedAxes);

	actionCustomLayout = new QAction(QIcon(":/arrangeLayers.png"), d_app->tr("&Custom Layout..."), d_app);
	connect(actionCustomLayout, &QAction::triggered, d_app, &ApplicationWindow::plotCustomLayout);

	actionPlotDoubleYAxis = new QAction(QIcon(":/plot_double_y.png"), d_app->tr("D&ouble-Y"), d_app);
	connect(actionPlotDoubleYAxis, &QAction::triggered, d_app, &ApplicationWindow::plotDoubleYAxis);

	actionAddZoomPlot = new QAction(QIcon(":/add_zoom_plot.png"), d_app->tr("&Zoom"), d_app);
	connect(actionAddZoomPlot, &QAction::triggered, d_app, &ApplicationWindow::zoomRectanglePlot);

	actionWaterfallPlot = new QAction(QIcon(":/waterfall_plot.png"), d_app->tr("&Waterfall Plot"), d_app);
	connect(actionWaterfallPlot, &QAction::triggered, d_app, [app]{app->waterfallPlot();});

	actionExtractGraphs = new QAction(QIcon(":/extract_graphs.png"), d_app->tr("E&xtract to Graphs"), d_app);
	connect(actionExtractGraphs, &QAction::triggered, d_app, &ApplicationWindow::extractGraphs);

	actionExtractLayers = new QAction(QIcon(":/extract_layers.png"), d_app->tr("Extract to &Layers"), d_app);
	connect(actionExtractLayers, &QAction::triggered, d_app, &ApplicationWindow::extractLayers);

	actionAddInsetLayer = new QAction(QIcon(":/add_inset_layer.png"), d_app->tr("Add Inset Layer"), d_app);
	connect(actionAddInsetLayer, &QAction::triggered, d_app, &ApplicationWindow::addInsetLayer);

	actionAddInsetCurveLayer = new QAction(QIcon(":/add_inset_curve_layer.png"), d_app->tr("Add Inset Layer"), d_app);
	connect(actionAddInsetCurveLayer, &QAction::triggered, d_app, &ApplicationWindow::addInsetCurveLayer);

	actionPlot3DRibbon = new QAction(QIcon(":/ribbon.png"), d_app->tr("&Ribbon"), d_app);
	connect(actionPlot3DRibbon, &QAction::triggered, d_app, &ApplicationWindow::plot3DRibbon);

	actionPlot3DBars = new QAction(QIcon(":/bars.png"), d_app->tr("&Bars"), d_app);
	connect(actionPlot3DBars, &QAction::triggered, d_app, &ApplicationWindow::plot3DBars);

	actionPlot3DScatter = new QAction(QIcon(":/scatter.png"), d_app->tr("&Scatter"), d_app);
	connect(actionPlot3DScatter, &QAction::triggered, d_app, &ApplicationWindow::plot3DScatter);

	actionPlot3DTrajectory = new QAction(QIcon(":/trajectory.png"), d_app->tr("&Trajectory"), d_app);
	connect(actionPlot3DTrajectory, &QAction::triggered, d_app, &ApplicationWindow::plot3DTrajectory);

	actionShowColStatistics = new QAction(QIcon(":/col_stat.png"), d_app->tr("Statistics on &Columns"), d_app);
	connect(actionShowColStatistics, &QAction::triggered, d_app, &ApplicationWindow::showColStatistics);

	actionShowRowStatistics = new QAction(QIcon(":/stat_rows.png"), d_app->tr("Statistics on &Rows"), d_app);
	connect(actionShowRowStatistics, &QAction::triggered, d_app, &ApplicationWindow::showRowStatistics);

	actionIntegrate = new QAction(d_app->tr("&Integrate") + "...", d_app);
	connect(actionIntegrate, &QAction::triggered, d_app, &ApplicationWindow::integrate);

	actionShowIntDialog = new QAction(d_app->tr("Integr&ate Function..."), d_app);
	connect(actionShowIntDialog, &QAction::triggered, d_app, &ApplicationWindow::showFunctionIntegrationDialog);

	actionInterpolate = new QAction(d_app->tr("Inte&rpolate ..."), d_app);
	connect(actionInterpolate, &QAction::triggered, d_app, &ApplicationWindow::showInterpolationDialog);

	actionLowPassFilter = new QAction(d_app->tr("&Low Pass..."), d_app);
	connect(actionLowPassFilter, &QAction::triggered, d_app, &ApplicationWindow::lowPassFilterDialog);

	actionHighPassFilter = new QAction(d_app->tr("&High Pass..."), d_app);
	connect(actionHighPassFilter, &QAction::triggered, d_app, &ApplicationWindow::highPassFilterDialog);

	actionBandPassFilter = new QAction(d_app->tr("&Band Pass..."), d_app);
	connect(actionBandPassFilter, &QAction::triggered, d_app, &ApplicationWindow::bandPassFilterDialog);

	actionBandBlockFilter = new QAction(d_app->tr("&Band Block..."), d_app);
	connect(actionBandBlockFilter, &QAction::triggered, d_app, &ApplicationWindow::bandBlockFilterDialog);

	actionFFT = new QAction(d_app->tr("&FFT..."), d_app);
	connect(actionFFT, &QAction::triggered, d_app, &ApplicationWindow::showFFTDialog);

	actionSmoothSavGol = new QAction(d_app->tr("&Savitzky-Golay..."), d_app);
	connect(actionSmoothSavGol, &QAction::triggered, d_app, &ApplicationWindow::showSmoothSavGolDialog);

	actionSmoothFFT = new QAction(d_app->tr("&FFT Filter..."), d_app);
	connect(actionSmoothFFT, &QAction::triggered, d_app, &ApplicationWindow::showSmoothFFTDialog);

	actionSmoothAverage = new QAction(d_app->tr("Moving Window &Average..."), d_app);
	connect(actionSmoothAverage, &QAction::triggered, d_app, &ApplicationWindow::showSmoothAverageDialog);

	actionSmoothLowess = new QAction(d_app->tr("&Lowess..."), d_app);
	connect(actionSmoothLowess, &QAction::triggered, d_app, &ApplicationWindow::showSmoothLowessDialog);

	actionDifferentiate = new QAction(d_app->tr("&Differentiate"), d_app);
	connect(actionDifferentiate, &QAction::triggered, d_app, &ApplicationWindow::differentiate);

	actionFitSlope = new QAction(d_app->tr("Fit Slop&e"), d_app);
	connect(actionFitSlope, &QAction::triggered, d_app, &ApplicationWindow::fitSlope);

	actionFitLinear = new QAction(d_app->tr("Fit &Linear"), d_app);
	connect(actionFitLinear, &QAction::triggered, d_app, &ApplicationWindow::fitLinear);

	actionShowFitPolynomDialog = new QAction(d_app->tr("Fit &Polynomial ..."), d_app);
	connect(actionShowFitPolynomDialog, &QAction::triggered, d_app, &ApplicationWindow::showFitPolynomDialog);

	actionShowExpDecayDialog = new QAction(d_app->tr("&First Order ..."), d_app);
	connect(actionShowExpDecayDialog, &QAction::triggered, d_app, [app]{app->showExpDecayDialog();});

	actionShowTwoExpDecayDialog = new QAction(d_app->tr("&Second Order ..."), d_app);
	connect(actionShowTwoExpDecayDialog, &QAction::triggered, d_app, &ApplicationWindow::showTwoExpDecayDialog);

	actionShowExpDecay3Dialog = new QAction(d_app->tr("&Third Order ..."), d_app);
	connect(actionShowExpDecay3Dialog, &QAction::triggered, d_app, &ApplicationWindow::showExpDecay3Dialog);

	actionFitExpGrowth = new QAction(d_app->tr("Fit Exponential Gro&wth ..."), d_app);
	connect(actionFitExpGrowth, &QAction::triggered, d_app, &ApplicationWindow::showExpGrowthDialog);

	actionFitSigmoidal = new QAction(d_app->tr("Fit &Boltzmann (Sigmoidal)"), d_app);
	connect(actionFitSigmoidal, &QAction::triggered, d_app, &ApplicationWindow::fitSigmoidal);

	actionFitGauss = new QAction(d_app->tr("Fit &Gaussian"), d_app);
	connect(actionFitGauss, &QAction::triggered, d_app, &ApplicationWindow::fitGauss);

	actionFitLorentz = new QAction(d_app->tr("Fit Lorent&zian"), d_app);
	connect(actionFitLorentz, &QAction::triggered, d_app, &ApplicationWindow::fitLorentz);

	actionShowFitDialog = new QAction(d_app->tr("Fit &Wizard..."), d_app);
	actionShowFitDialog->setShortcut( d_app->tr("Ctrl+Y") );
	connect(actionShowFitDialog, &QAction::triggered, d_app, &ApplicationWindow::showFitDialog);

	actionShowPlotDialog = new QAction(d_app->tr("&Plot ..."), d_app);
	connect(actionShowPlotDialog, &QAction::triggered, d_app, &ApplicationWindow::showGeneralPlotDialog);

	actionShowScaleDialog = new QAction(d_app->tr("&Scales..."), d_app);
	connect(actionShowScaleDialog, &QAction::triggered, d_app, &ApplicationWindow::showScaleDialog);

	actionShowAxisDialog = new QAction(d_app->tr("&Axes..."), d_app);
	connect(actionShowAxisDialog, &QAction::triggered, d_app, &ApplicationWindow::showAxisDialog);

	actionShowGridDialog = new QAction(d_app->tr("&Grid ..."), d_app);
	connect(actionShowGridDialog, &QAction::triggered, d_app, &ApplicationWindow::showGridDialog);

	actionShowTitleDialog = new QAction(d_app->tr("&Title ..."), d_app);
	connect(actionShowTitleDialog, &QAction::triggered, d_app, &ApplicationWindow::showTitleDialog);

	actionShowColumnOptionsDialog = new QAction(QIcon(":/configure.png"), d_app->tr("Column &Options ..."), d_app);
	actionShowColumnOptionsDialog->setShortcut(d_app->tr("Ctrl+Alt+O"));
	connect(actionShowColumnOptionsDialog, &QAction::triggered, d_app, &ApplicationWindow::showColumnOptionsDialog);

	actionShowColumnValuesDialog = new QAction(QIcon(":/formula.png"), d_app->tr("Set Column &Values ..."), d_app);
	connect(actionShowColumnValuesDialog, &QAction::triggered, d_app, &ApplicationWindow::showColumnValuesDialog);
	actionShowColumnValuesDialog->setShortcut(d_app->tr("Alt+Q"));

	actionExtractTableData = new QAction(d_app->tr("&Extract Data..."), d_app);
	connect(actionExtractTableData, &QAction::triggered, d_app, &ApplicationWindow::showExtractDataDialog);

	actionTableRecalculate = new QAction(d_app->tr("Recalculate"), d_app);
	actionTableRecalculate->setShortcut(d_app->tr("Ctrl+Return"));
	connect(actionTableRecalculate, &QAction::triggered, d_app, &ApplicationWindow::recalculateTable);

	actionHideSelectedColumns = new QAction(d_app->tr("&Hide Selected"), d_app);
	connect(actionHideSelectedColumns, &QAction::triggered, d_app, &ApplicationWindow::hideSelectedColumns);

	actionShowAllColumns = new QAction(d_app->tr("Sho&w All Columns"), d_app);
	connect(actionShowAllColumns, &QAction::triggered, d_app, &ApplicationWindow::showAllColumns);

	actionSwapColumns = new QAction(QIcon(":/swap_columns.png"), d_app->tr("&Swap columns"), d_app);
	connect(actionSwapColumns, &QAction::triggered, d_app, [app]{app->swapColumns();});

	actionMoveColRight = new QAction(QIcon(":/move_col_right.png"), d_app->tr("Move &Right"), d_app);
	connect(actionMoveColRight, &QAction::triggered, d_app, &ApplicationWindow::moveColumnRight);

	actionMoveColLeft = new QAction(QIcon(":/move_col_left.png"), d_app->tr("Move &Left"), d_app);
	connect(actionMoveColLeft, &QAction::triggered, d_app, &ApplicationWindow::moveColumnLeft);

	actionMoveColFirst = new QAction(QIcon(":/move_col_first.png"), d_app->tr("Move to F&irst"), d_app);
	connect(actionMoveColFirst, &QAction::triggered, d_app, &ApplicationWindow::moveColumnFirst);

	actionMoveColLast = new QAction(QIcon(":/move_col_last.png"), d_app->tr("Move to Las&t"), d_app);
	connect(actionMoveColLast, &QAction::triggered, d_app, &ApplicationWindow::moveColumnLast);

	actionAdjustColumnWidth = new QAction(QIcon(":/adjust_col_width.png"), d_app->tr("Ad&just Column Width"), d_app);
	connect(actionAdjustColumnWidth, &QAction::triggered, d_app, &ApplicationWindow::adjustColumnWidth);

	actionShowColsDialog = new QAction(d_app->tr("&Columns..."), d_app);
	connect(actionShowColsDialog, &QAction::triggered, d_app, &ApplicationWindow::showColsDialog);

	actionShowRowsDialog = new QAction(d_app->tr("&Rows..."), d_app);
	connect(actionShowRowsDialog, &QAction::triggered, d_app, &ApplicationWindow::showRowsDialog);

    actionDeleteRows = new QAction(d_app->tr("&Delete Rows Interval..."), d_app);
	connect(actionDeleteRows, &QAction::triggered, d_app, &ApplicationWindow::showDeleteRowsDialog);

	actionMoveRowUp = new QAction(QIcon(":/move_row_up.png"), d_app->tr("&Upward"), d_app);
	connect(actionMoveRowUp, &QAction::triggered, d_app, &ApplicationWindow::moveTableRowUp);

	actionMoveRowDown = new QAction(QIcon(":/move_row_down.png"), d_app->tr("&Downward"), d_app);
	connect(actionMoveRowDown, &QAction::triggered, d_app, &ApplicationWindow::moveTableRowDown);

	actionAbout = new QAction(d_app->tr("&About QtiPlot"), d_app);
	actionAbout->setShortcut( d_app->tr("F1") );
	connect(actionAbout, &QAction::triggered, d_app, [app](){ app->about(); });

	actionShowHelp = new QAction(d_app->tr("&Help"), d_app);
	actionShowHelp->setShortcut( d_app->tr("Ctrl+H") );
	connect(actionShowHelp, &QAction::triggered, d_app, &ApplicationWindow::showHelp);

	actionChooseHelpFolder = new QAction(d_app->tr("&Choose Help Folder..."), d_app);
	connect(actionChooseHelpFolder, &QAction::triggered, d_app, &ApplicationWindow::chooseHelpFolder);

	actionRename = new QAction(d_app->tr("&Rename Window") + "...", d_app);
	connect(actionRename, &QAction::triggered, d_app, &ApplicationWindow::rename);

	actionNextWindow = new QAction(QIcon(":/next.png"), d_app->tr("&Next","next window"), d_app);
	actionNextWindow->setShortcut( d_app->tr("F5","next window shortcut") );

	actionPrevWindow = new QAction(QIcon(":/prev.png"), d_app->tr("&Previous","previous window"), d_app);
	actionPrevWindow->setShortcut( d_app->tr("F6","previous window shortcut") );

	actionCloseWindow = new QAction(QIcon(":/close.png"), d_app->tr("Close &Window"), d_app);
	connect(actionCloseWindow, &QAction::triggered, d_app, &ApplicationWindow::closeActiveWindow);

	actionAddColToTable = new QAction(QIcon(":/addCol.png"), d_app->tr("Add Column"), d_app);
	connect(actionAddColToTable, &QAction::triggered, d_app, &ApplicationWindow::addColToTable);

	actionGoToRow = new QAction(d_app->tr("&Go to Row..."), d_app);
	actionGoToRow->setShortcut(d_app->tr("Ctrl+Alt+G"));
	connect(actionGoToRow, &QAction::triggered, d_app, &ApplicationWindow::goToRow);

    actionGoToColumn = new QAction(d_app->tr("Go to Colum&n..."), d_app);
	actionGoToColumn->setShortcut(d_app->tr("Ctrl+Alt+C"));
	connect(actionGoToColumn, &QAction::triggered, d_app, &ApplicationWindow::goToColumn);

	actionClearTable = new QAction(QPixmap(":/erase.png"), d_app->tr("Clear"), d_app);
	connect(actionClearTable, &QAction::triggered, d_app, &ApplicationWindow::clearTable);

	actionDeleteLayer = new QAction(QIcon(":/delete.png"), d_app->tr("&Remove Layer"), d_app);
	actionDeleteLayer->setShortcut( d_app->tr("Alt+R") );
	connect(actionDeleteLayer, &QAction::triggered, d_app, &ApplicationWindow::deleteLayer);

	actionResizeActiveWindow = new QAction(QIcon(":/resize.png"), d_app->tr("Window &Geometry..."), d_app);
	connect(actionResizeActiveWindow, &QAction::triggered, d_app, &ApplicationWindow::resizeActiveWindow);

	actionHideActiveWindow = new QAction(d_app->tr("&Hide Window"), d_app);
	actionHideActiveWindow->setShortcut(d_app->tr("Ctrl+Alt+H"));
	connect(actionHideActiveWindow, &QAction::triggered, d_app, &ApplicationWindow::hideActiveWindow);

	actionShowMoreWindows = new QAction(d_app->tr("More windows..."), d_app);
	connect(actionShowMoreWindows, &QAction::triggered, d_app, &ApplicationWindow::showMoreWindows);

	actionPixelLineProfile = new QAction(QIcon(":/pixelProfile.png"), d_app->tr("&View Pixel Line Profile"), d_app);
	connect(actionPixelLineProfile, &QAction::triggered, d_app, &ApplicationWindow::pixelLineProfile);

	actionIntensityTable = new QAction(d_app->tr("&Intensity Table"), d_app);
	connect(actionIntensityTable, &QAction::triggered, d_app, &ApplicationWindow::intensityTable);

	actionShowLineDialog = new QAction(d_app->tr("&Properties"), d_app);
	connect(actionShowLineDialog, &QAction::triggered, d_app, &ApplicationWindow::showLineDialog);

	actionShowTextDialog = new QAction(d_app->tr("&Properties"), d_app);
	connect(actionShowTextDialog, &QAction::triggered, d_app, &ApplicationWindow::showEnrichementDialog);

	actionActivateWindow = new QAction(d_app->tr("&Activate Window"), d_app);
	connect(actionActivateWindow, &QAction::triggered, d_app, [app]{app->activateWindow();});

	actionMinimizeWindow = new QAction(d_app->tr("Mi&nimize Window"), d_app);
	connect(actionMinimizeWindow, &QAction::triggered, d_app, [app]{app->minimizeWindow();});

	actionMaximizeWindow = new QAction(d_app->tr("Ma&ximize Window"), d_app);
	connect(actionMaximizeWindow, &QAction::triggered, d_app, [app]{app->maximizeWindow();});

	actionHideWindow = new QAction(d_app->tr("&Hide Window"), d_app);
	actionHideWindow->setShortcut(d_app->tr("Ctrl+Alt+H"));
	connect(actionHideWindow, &QAction::triggered, d_app, [app]{app->hideWindow();});

	actionResizeWindow = new QAction(QIcon(":/resize.png"), d_app->tr("Re&size Window..."), d_app);
	connect(actionResizeWindow, &QAction::triggered, d_app, &ApplicationWindow::resizeWindow);

	actionEditSurfacePlot = new QAction(d_app->tr("&Surface..."), d_app);
	connect(actionEditSurfacePlot, &QAction::triggered, d_app, &ApplicationWindow::editSurfacePlot);

	actionAdd3DData = new QAction(d_app->tr("&Data Set..."), d_app);
	connect(actionAdd3DData, &QAction::triggered, d_app, &ApplicationWindow::add3DData);

	actionSetMatrixProperties = new QAction(QIcon(":/configure.png"), d_app->tr("Set &Properties..."), d_app);
	connect(actionSetMatrixProperties, &QAction::triggered, d_app, &ApplicationWindow::showMatrixDialog);

	actionSetMatrixDimensions = new QAction(d_app->tr("Set &Dimensions..."), d_app);
	connect(actionSetMatrixDimensions, &QAction::triggered, d_app, &ApplicationWindow::showMatrixSizeDialog);
	actionSetMatrixDimensions->setShortcut(d_app->tr("Ctrl+D"));

	actionSetMatrixValues = new QAction(QIcon(":/formula.png"), d_app->tr("Set &Values..."), d_app);
	connect(actionSetMatrixValues, &QAction::triggered, d_app, &ApplicationWindow::showMatrixValuesDialog);
	actionSetMatrixValues->setShortcut(d_app->tr("Alt+Q"));

	actionImagePlot = new QAction(QIcon(":/image_plot.png"), d_app->tr("&Image Plot"), d_app);
	connect(actionImagePlot, &QAction::triggered, d_app, [app]{app->plotImage();});

	actionImageProfilesPlot = new QAction(QIcon(":/image_profiles.png"), d_app->tr("&Image Profiles"), d_app);
	connect(actionImageProfilesPlot, &QAction::triggered, d_app, [app]{app->plotImageProfiles();});

	actionTransposeMatrix = new QAction(d_app->tr("&Transpose"), d_app);
	connect(actionTransposeMatrix, &QAction::triggered, d_app, &ApplicationWindow::transposeMatrix);

	actionFlipMatrixVertically = new QAction(QIcon(":/flip_vertical.png"), d_app->tr("Flip &V"), d_app);
	actionFlipMatrixVertically->setShortcut(d_app->tr("Ctrl+Shift+V"));
	connect(actionFlipMatrixVertically, &QAction::triggered, d_app, &ApplicationWindow::flipMatrixVertically);

	actionFlipMatrixHorizontally = new QAction(QIcon(":/flip_horizontal.png"), d_app->tr("Flip &H"), d_app);
	actionFlipMatrixHorizontally->setShortcut(d_app->tr("Ctrl+Shift+H"));
	connect(actionFlipMatrixHorizontally, &QAction::triggered, d_app, &ApplicationWindow::flipMatrixHorizontally);

	actionRotateMatrix = new QAction(QIcon(":/rotate_clockwise.png"), d_app->tr("R&otate 90"), d_app);
	actionRotateMatrix->setShortcut(d_app->tr("Ctrl+Shift+R"));
	connect(actionRotateMatrix, &QAction::triggered, d_app, &ApplicationWindow::rotateMatrix90);

	actionRotateMatrixMinus = new QAction(QIcon(":/rotate_counterclockwise.png"), d_app->tr("Rotate &-90"), d_app);
	actionRotateMatrixMinus->setShortcut(d_app->tr("Ctrl+Alt+R"));
	connect(actionRotateMatrixMinus, &QAction::triggered, d_app, &ApplicationWindow::rotateMatrixMinus90);

	actionInvertMatrix = new QAction(d_app->tr("&Invert"), d_app);
	connect(actionInvertMatrix, &QAction::triggered, d_app, &ApplicationWindow::invertMatrix);

	actionMatrixDeterminant = new QAction(d_app->tr("&Determinant"), d_app);
	connect(actionMatrixDeterminant, &QAction::triggered, d_app, &ApplicationWindow::matrixDeterminant);

	actionViewMatrixImage = new QAction(d_app->tr("&Image mode"), d_app);
	actionViewMatrixImage->setShortcut(d_app->tr("Ctrl+Shift+I"));
	connect(actionViewMatrixImage, &QAction::triggered, d_app, &ApplicationWindow::viewMatrixImage);
	

	actionViewMatrix = new QAction(d_app->tr("&Data mode"), d_app);
	actionViewMatrix->setShortcut(d_app->tr("Ctrl+Shift+D"));
	connect(actionViewMatrix, &QAction::triggered, d_app, &ApplicationWindow::viewMatrixTable);
	

    actionMatrixXY = new QAction(d_app->tr("Show &X/Y"), d_app);
	actionMatrixXY->setShortcut(d_app->tr("Ctrl+Shift+X"));
	connect(actionMatrixXY, &QAction::triggered, d_app, &ApplicationWindow::viewMatrixXY);
	

    actionMatrixColumnRow = new QAction(d_app->tr("Show &Column/Row"), d_app);
	actionMatrixColumnRow->setShortcut(d_app->tr("Ctrl+Shift+C"));
	connect(actionMatrixColumnRow, &QAction::triggered, d_app, &ApplicationWindow::viewMatrixColumnRow);
	

    actionMatrixGrayScale = new QAction(d_app->tr("&Gray Scale"), d_app);
	connect(actionMatrixGrayScale, &QAction::triggered, d_app, &ApplicationWindow::setMatrixGrayScale);
	

	actionMatrixDefaultScale = new QAction(d_app->tr("&Default"), d_app);
	connect(actionMatrixDefaultScale, &QAction::triggered, d_app, &ApplicationWindow::setMatrixDefaultScale);
	

	actionMatrixRainbowScale = new QAction(d_app->tr("&Rainbow"), d_app);
	connect(actionMatrixRainbowScale, &QAction::triggered, d_app, &ApplicationWindow::setMatrixRainbowScale);
	

	actionMatrixCustomScale = new QAction(d_app->tr("&Custom"), d_app);
	connect(actionMatrixCustomScale, &QAction::triggered, d_app, &ApplicationWindow::showColorMapDialog);
	

	actionExportMatrix = new QAction(QPixmap(":/monalisa.png"), d_app->tr("&Export Image ..."), d_app);
	connect(actionExportMatrix, &QAction::triggered, d_app, [app]{app->exportMatrix();});

	actionConvertMatrixDirect = new QAction(d_app->tr("&Direct"), d_app);
	connect(actionConvertMatrixDirect, &QAction::triggered, d_app, &ApplicationWindow::convertMatrixToTableDirect);

	actionConvertMatrixXYZ = new QAction(d_app->tr("&XYZ Columns"), d_app);
	connect(actionConvertMatrixXYZ, &QAction::triggered, d_app, &ApplicationWindow::convertMatrixToTableXYZ);

	actionConvertMatrixYXZ = new QAction(d_app->tr("&YXZ Columns"), d_app);
	connect(actionConvertMatrixYXZ, &QAction::triggered, d_app, &ApplicationWindow::convertMatrixToTableYXZ);

    actionMatrixFFTDirect = new QAction(d_app->tr("&Forward FFT"), d_app);
	connect(actionMatrixFFTDirect, &QAction::triggered, d_app, &ApplicationWindow::matrixDirectFFT);

	actionMatrixFFTInverse = new QAction(d_app->tr("&Inverse FFT"), d_app);
	connect(actionMatrixFFTInverse, &QAction::triggered, d_app, &ApplicationWindow::matrixInverseFFT);

	actionConvertTableDirect= new QAction(d_app->tr("&Direct"), d_app);
	connect(actionConvertTableDirect, &QAction::triggered, d_app, &ApplicationWindow::convertTableToMatrix);

	actionConvertTableBinning = new QAction(d_app->tr("2D &Binning"), d_app);
	connect(actionConvertTableBinning, &QAction::triggered, d_app, &ApplicationWindow::showBinMatrixDialog);

	actionConvertTableRegularXYZ = new QAction(d_app->tr("&Regular XYZ"), d_app);
	connect(actionConvertTableRegularXYZ, &QAction::triggered, d_app, [app]{app->tableToMatrixRegularXYZ();});

#ifdef HAVE_ALGLIB
	actionConvertTableRandomXYZ = new QAction(d_app->tr("Random &XYZ..."), d_app);
	connect(actionConvertTableRandomXYZ, &QAction::triggered, d_app, &ApplicationWindow::convertTableToMatrixRandomXYZ);

	actionExpandMatrix = new QAction(d_app->tr("&Expand..."), d_app);
	connect(actionExpandMatrix, &QAction::triggered, d_app, &ApplicationWindow::expandMatrix);

	actionShrinkMatrix = new QAction(d_app->tr("&Shrink..."), d_app);
	connect(actionShrinkMatrix, &QAction::triggered, d_app, &ApplicationWindow::shrinkMatrix);

	actionSmoothMatrix = new QAction(d_app->tr("S&mooth"), d_app);
	connect(actionSmoothMatrix, &QAction::triggered, d_app, &ApplicationWindow::smoothMatrix);
#endif

	actionPlot3DWireFrame = new QAction(QIcon(":/lineMesh.png"), d_app->tr("3D &Wire Frame"), d_app);
	connect(actionPlot3DWireFrame, &QAction::triggered, d_app, &ApplicationWindow::plot3DWireframe);

	actionPlot3DHiddenLine = new QAction(QIcon(":/grid_only.png"), d_app->tr("3D &Hidden Line"), d_app);
	connect(actionPlot3DHiddenLine, &QAction::triggered, d_app, &ApplicationWindow::plot3DHiddenLine);

	actionPlot3DPolygons = new QAction(QIcon(":/no_grid.png"), d_app->tr("3D &Polygons"), d_app);
	connect(actionPlot3DPolygons, &QAction::triggered, d_app, &ApplicationWindow::plot3DPolygons);

	actionPlot3DWireSurface = new QAction(QIcon(":/grid_poly.png"), d_app->tr("3D Wire &Surface"), d_app);
	connect(actionPlot3DWireSurface, &QAction::triggered, d_app, &ApplicationWindow::plot3DWireSurface);

	actionColorMap = new QAction(QIcon(":/color_map.png"), d_app->tr("Contour - &Color Fill"), d_app);
	connect(actionColorMap, &QAction::triggered, d_app, [app]{app->plotColorMap();});

	actionContourMap = new QAction(QIcon(":/contour_map.png"), d_app->tr("Contour &Lines"), d_app);
	connect(actionContourMap, &QAction::triggered, d_app, [app]{app->plotContour();});

	actionGrayMap = new QAction(QIcon(":/gray_map.png"), d_app->tr("&Gray Scale Map"), d_app);
	connect(actionGrayMap, &QAction::triggered, d_app, [app]{app->plotGrayScale();});

	actionSortTable = new QAction(QIcon(":/sort.png"), d_app->tr("Sort Ta&ble") + "...", d_app);
	connect(actionSortTable, &QAction::triggered, d_app, &ApplicationWindow::sortActiveTable);

	actionSortSelection = new QAction(QIcon(":/sort.png"), d_app->tr("&Custom") + "...", d_app);
	connect(actionSortSelection, &QAction::triggered, d_app, &ApplicationWindow::sortSelection);

	actionNormalizeTable = new QAction(d_app->tr("&Table"), d_app);
	connect(actionNormalizeTable, &QAction::triggered, d_app, &ApplicationWindow::normalizeActiveTable);

	actionNormalizeSelection = new QAction(d_app->tr("&Columns"), d_app);
	connect(actionNormalizeSelection, &QAction::triggered, d_app, &ApplicationWindow::normalizeSelection);

	actionCorrelate = new QAction(d_app->tr("Co&rrelate"), d_app);
	connect(actionCorrelate, &QAction::triggered, d_app, &ApplicationWindow::correlate);

	actionAutoCorrelate = new QAction(d_app->tr("&Autocorrelate"), d_app);
	connect(actionAutoCorrelate, &QAction::triggered, d_app, &ApplicationWindow::autoCorrelate);

	actionConvolute = new QAction(d_app->tr("&Convolute"), d_app);
	connect(actionConvolute, &QAction::triggered, d_app, &ApplicationWindow::convolute);

	actionDeconvolute = new QAction(d_app->tr("&Deconvolute"), d_app);
	connect(actionDeconvolute, &QAction::triggered, d_app, &ApplicationWindow::deconvolute);

	actionTranslateHor = new QAction(d_app->tr("&Horizontal"), d_app);
	connect(actionTranslateHor, &QAction::triggered, d_app, &ApplicationWindow::translateCurveHor);

	actionTranslateVert = new QAction(d_app->tr("&Vertical"), d_app);
	connect(actionTranslateVert, &QAction::triggered, d_app, [app]{app->translateCurve(TranslateCurveTool::Vertical);});

	actionSetAscValues = new QAction(QIcon(":/rowNumbers.png"),d_app->tr("Ro&w Numbers"), d_app);
	connect(actionSetAscValues, &QAction::triggered, d_app, &ApplicationWindow::setAscValues);

	actionSetRandomValues = new QAction(QIcon(":/randomNumbers.png"),d_app->tr("&Random Values"), d_app);
	connect(actionSetRandomValues, &QAction::triggered, d_app, &ApplicationWindow::setRandomValues);

	actionSetRandomNormalValues = new QAction(QIcon(":/normalRandomNumbers.png"), d_app->tr("&Normal Random Numbers"), d_app);
	connect(actionSetRandomNormalValues, &QAction::triggered, d_app, &ApplicationWindow::setNormalRandomValues);

	actionFrequencyCount = new QAction(d_app->tr("&Frequency Count ..."), d_app);
	connect(actionFrequencyCount, &QAction::triggered, d_app, &ApplicationWindow::showFrequencyCountDialog);

	actionShapiroWilk = new QAction(d_app->tr("&Normality Test (Shapiro - Wilk)") + "...", d_app);
	connect(actionShapiroWilk, &QAction::triggered, d_app, &ApplicationWindow::testNormality);

	actionChiSquareTest = new QAction(d_app->tr("Chi-square Test for &Variance..."), d_app);
	connect(actionChiSquareTest, &QAction::triggered, d_app, &ApplicationWindow::showChiSquareTestDialog);

	actionOneSampletTest = new QAction(d_app->tr("&One Sample t-Test..."), d_app);
	connect(actionOneSampletTest, &QAction::triggered, d_app, [app]{app->showStudentTestDialog();});

	actionTwoSampletTest = new QAction(d_app->tr("&Two Sample t-Test..."), d_app);
	connect(actionTwoSampletTest, &QAction::triggered, d_app, &ApplicationWindow::showTwoSampleStudentTestDialog);

#ifdef HAVE_TAMUANOVA
	actionOneWayANOVA = new QAction(d_app->tr("&One-Way ANOVA..."), d_app);
	connect(actionOneWayANOVA, &QAction::triggered, d_app, [app]{app->showANOVADialog();});

	actionTwoWayANOVA = new QAction(d_app->tr("&Two-Way ANOVA..."), d_app);
	connect(actionTwoWayANOVA, &QAction::triggered, d_app, &ApplicationWindow::showTwoWayANOVADialog);
#endif

    actionReadOnlyCol = new QAction(d_app->tr("&Read Only"), d_app);
    connect(actionReadOnlyCol, &QAction::triggered, d_app, &ApplicationWindow::setReadOnlyCol);

	actionSetXCol = new QAction(QIcon(":/x_col.png"), d_app->tr("&X"), d_app);
	connect(actionSetXCol, &QAction::triggered, d_app, &ApplicationWindow::setXCol);

	actionSetYCol = new QAction(QIcon(":/y_col.png"), d_app->tr("&Y"), d_app);
	connect(actionSetYCol, &QAction::triggered, d_app, &ApplicationWindow::setYCol);

	actionSetZCol = new QAction(QIcon(":/z_col.png"), d_app->tr("&Z"), d_app);
	connect(actionSetZCol, &QAction::triggered, d_app, &ApplicationWindow::setZCol);

	actionSetXErrCol = new QAction(d_app->tr("X E&rror"), d_app);
	connect(actionSetXErrCol, &QAction::triggered, d_app, &ApplicationWindow::setXErrCol);

	actionSetYErrCol = new QAction(QIcon(":/errors.png"), d_app->tr("Y &Error"), d_app);
	connect(actionSetYErrCol, &QAction::triggered, d_app, &ApplicationWindow::setYErrCol);

	actionDisregardCol = new QAction(QIcon(":/disregard_col.png"), d_app->tr("&Disregard"), d_app);
	connect(actionDisregardCol, &QAction::triggered, d_app, &ApplicationWindow::disregardCol);

	actionSetLabelCol = new QAction(QIcon(":/set_label_col.png"), d_app->tr("&Label"), d_app);
	connect(actionSetLabelCol, &QAction::triggered, d_app, &ApplicationWindow::setLabelCol);

	actionBoxPlot = new QAction(QIcon(":/boxPlot.png"),d_app->tr("&Box Plot"), d_app);
	connect(actionBoxPlot, &QAction::triggered, d_app, &ApplicationWindow::plotBox);

	actionMultiPeakGauss = new QAction(d_app->tr("&Gaussian..."), d_app);
	connect(actionMultiPeakGauss, &QAction::triggered, d_app, &ApplicationWindow::fitMultiPeakGauss);

	actionMultiPeakLorentz = new QAction(d_app->tr("&Lorentzian..."), d_app);
	connect(actionMultiPeakLorentz, &QAction::triggered, d_app, &ApplicationWindow::fitMultiPeakLorentz);

	actionSubtractLine = new QAction(d_app->tr("&Straight Line..."), d_app);
	connect(actionSubtractLine, &QAction::triggered, d_app, &ApplicationWindow::subtractStraightLine);

	actionSubtractReference = new QAction(d_app->tr("&Reference Data..."), d_app);
	connect(actionSubtractReference, &QAction::triggered, d_app, &ApplicationWindow::subtractReferenceData);

	actionBaseline = new QAction(d_app->tr("&Baseline..."), d_app);
	connect(actionBaseline, &QAction::triggered, d_app, &ApplicationWindow::baselineDialog);

	actionCheckUpdates = new QAction(d_app->tr("Search for &Updates"), d_app);
	connect(actionCheckUpdates, &QAction::triggered, d_app, &ApplicationWindow::searchForUpdates);

	actionHomePage = new QAction(d_app->tr("&QtiPlot Homepage"), d_app);
	connect(actionHomePage, &QAction::triggered, d_app, &ApplicationWindow::showHomePage);

	actionHelpForums = new QAction(d_app->tr("QtiPlot &Forums"), d_app);
	connect(actionHelpForums, &QAction::triggered, d_app, &ApplicationWindow::showForums);

	actionHelpBugReports = new QAction(d_app->tr("Report a &Bug"), d_app);
	connect(actionHelpBugReports, &QAction::triggered, d_app, &ApplicationWindow::showBugTracker);

	actionDownloadManual = new QAction(d_app->tr("Download &Manual"), d_app);
	connect(actionDownloadManual, &QAction::triggered, d_app, &ApplicationWindow::downloadManual);

	actionTranslations = new QAction(d_app->tr("&Translations"), d_app);
	connect(actionTranslations, &QAction::triggered, d_app, &ApplicationWindow::downloadTranslation);

	actionDonate = new QAction(d_app->tr("Make a &Donation"), d_app);
	connect(actionDonate, &QAction::triggered, d_app, &ApplicationWindow::showDonationsPage);

	actionTechnicalSupport = new QAction(d_app->tr("Technical &Support"), d_app);
	connect(actionTechnicalSupport, &QAction::triggered, d_app, &ApplicationWindow::showSupportPage);

#ifdef SCRIPTING_PYTHON
	actionScriptingLang = new QAction(d_app->tr("Scripting &language") + "...", d_app);
	connect(actionScriptingLang, &QAction::triggered, d_app, &ApplicationWindow::showScriptingLangDialog);

	actionCommentSelection = new QAction(QIcon(":/comment.png"), d_app->tr("Commen&t Selection"), d_app);
	actionCommentSelection->setEnabled(false);
	connect(actionCommentSelection, &QAction::triggered, d_app, &ApplicationWindow::commentSelection);

	actionUncommentSelection = new QAction(QIcon(":/uncomment.png"), d_app->tr("&Uncomment Selection"), d_app);
	actionUncommentSelection->setEnabled(false);
	connect(actionUncommentSelection, &QAction::triggered, d_app, &ApplicationWindow::uncommentSelection);
#endif

	actionRestartScripting = new QAction(d_app->tr("&Restart scripting"), d_app);
	connect(actionRestartScripting, &QAction::triggered, d_app, &ApplicationWindow::restartScriptingEnv);

	actionNoteExecute = new QAction(QIcon(":/execute_selection.png"), d_app->tr("E&xecute"), d_app);
	actionNoteExecute->setShortcut(d_app->tr("Ctrl+J"));
	connect(actionNoteExecute, &QAction::triggered, d_app, &ApplicationWindow::execute);

	actionNoteExecuteAll = new QAction(QIcon(":/play.png"), d_app->tr("Execute &All"), d_app);
	actionNoteExecuteAll->setShortcut(d_app->tr("Ctrl+Shift+J"));
	connect(actionNoteExecuteAll, &QAction::triggered, d_app, &ApplicationWindow::executeAll);

	actionNoteEvaluate = new QAction(d_app->tr("&Evaluate Expression"), d_app);
	actionNoteEvaluate->setShortcut(d_app->tr("Ctrl+Return"));
	connect(actionNoteEvaluate, &QAction::triggered, d_app, &ApplicationWindow::evaluate);

	actionNoteStop = new QAction(QIcon(":/close.png"), d_app->tr("&Stop Execution"), d_app);
	actionNoteStop->setShortcut(d_app->tr("Ctrl+Break"));
	connect(actionNoteStop, &QAction::triggered, d_app, &ApplicationWindow::stopExecution);

	actionShowNoteLineNumbers = new QAction(d_app->tr("Show Line &Numbers"), d_app);
	
	connect(actionShowNoteLineNumbers, &QAction::toggled, d_app, &ApplicationWindow::showNoteLineNumbers);

	actionFind = new QAction(QIcon(":/find.png"), d_app->tr("&Find..."), d_app);
	actionFind->setShortcut(d_app->tr("Ctrl+Alt+F"));
	connect(actionFind, &QAction::triggered, d_app, &ApplicationWindow::noteFindDialogue);

	actionFindNext = new QAction(QIcon(":/find_next.png"), d_app->tr("Find &Next"), d_app);
	actionFindNext->setShortcut(d_app->tr("F3"));
	connect(actionFindNext, &QAction::triggered, d_app, &ApplicationWindow::noteFindNext);

	actionFindPrev = new QAction(QIcon(":/find_previous.png"), d_app->tr("Find &Previous"), d_app);
	actionFindPrev->setShortcut(d_app->tr("F4"));
	connect(actionFindPrev, &QAction::triggered, d_app, &ApplicationWindow::noteFindPrev);

	actionReplace = new QAction(QIcon(":/replace.png"), d_app->tr("&Replace..."), d_app);
	connect(actionReplace, &QAction::triggered, d_app, &ApplicationWindow::noteReplaceDialogue);

	actionIncreaseIndent = new QAction(QIcon(":/increase_indent.png"), d_app->tr("Increase Indent"), d_app);
	connect(actionIncreaseIndent, &QAction::triggered, d_app, &ApplicationWindow::increaseNoteIndent);

	actionDecreaseIndent = new QAction(QIcon(":/decrease_indent.png"),d_app->tr("Decrease Indent"), d_app);
	connect(actionDecreaseIndent, &QAction::triggered, d_app, &ApplicationWindow::decreaseNoteIndent);

	actionRenameNoteTab = new QAction(d_app->tr("Rena&me Tab..."), d_app);
	connect(actionRenameNoteTab, &QAction::triggered, d_app, &ApplicationWindow::renameCurrentNoteTab);

	actionAddNoteTab = new QAction(QIcon(":/plus.png"), d_app->tr("A&dd Tab"), d_app);
	connect(actionAddNoteTab, &QAction::triggered, d_app, &ApplicationWindow::addNoteTab);

	actionCloseNoteTab = new QAction(QIcon(":/delete.png"), d_app->tr("C&lose Tab"), d_app);
	connect(actionCloseNoteTab, &QAction::triggered, d_app, &ApplicationWindow::closeNoteTab);

#ifdef SCRIPTING_PYTHON
	actionShowScriptWindow = new QAction(QPixmap(":/python.png"), d_app->tr("&Script Window"), d_app);
	actionShowScriptWindow->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F3));
	actionShowScriptWindow->setCheckable( true );
	connect(actionShowScriptWindow, &QAction::triggered, d_app, &ApplicationWindow::showScriptWindow);

	actionOpenQtDesignerUi = new QAction(d_app->tr("Load Custom User &Interface..."), d_app);
	connect(actionOpenQtDesignerUi, &QAction::triggered, d_app, &ApplicationWindow::openQtDesignerUi);
#endif

	actionShowCurvePlotDialog = new QAction(d_app->tr("&Plot details..."), d_app);
	connect(actionShowCurvePlotDialog, &QAction::triggered, d_app, [app]{app->showPlotDialog();});

	actionShowCurveWorksheet = new QAction(d_app->tr("&Worksheet"), d_app);
	connect(actionShowCurveWorksheet, &QAction::triggered, d_app, [app]{app->showCurveWorksheet();});

	actionCurveFullRange = new QAction(d_app->tr("&Reset to Full Range"), d_app);
	connect(actionCurveFullRange, &QAction::triggered, d_app, &ApplicationWindow::setCurveFullRange);

	actionEditCurveRange = new QAction(d_app->tr("Edit &Range..."), d_app);
	connect(actionEditCurveRange, &QAction::triggered, d_app, [app]{app->showCurveRangeDialog();});

	actionRemoveCurve = new QAction(QPixmap(":/close.png"), d_app->tr("&Delete"), d_app);
	connect(actionRemoveCurve, &QAction::triggered, d_app, &ApplicationWindow::removeCurve);

	actionHideCurve = new QAction(d_app->tr("&Hide"), d_app);
	connect(actionHideCurve, &QAction::triggered, d_app, &ApplicationWindow::hideCurve);

	actionHideOtherCurves = new QAction(d_app->tr("Hide &Other Curves"), d_app);
	connect(actionHideOtherCurves, &QAction::triggered, d_app, &ApplicationWindow::hideOtherCurves);

	actionShowAllCurves = new QAction(d_app->tr("&Show All Curves"), d_app);
	connect(actionShowAllCurves, &QAction::triggered, d_app, &ApplicationWindow::showAllCurves);

	actionToolBars = new QAction(d_app->tr("&Toolbars..."), d_app);
	actionToolBars->setShortcut(d_app->tr("Ctrl+Shift+T"));
	connect(actionToolBars, &QAction::triggered, d_app, &ApplicationWindow::showToolBarsMenu);

	actionFontBold = new QAction("B", d_app);
	actionFontBold->setToolTip(d_app->tr("Bold"));
	QFont font = appFont;
	font.setBold(true);
	actionFontBold->setFont(font);
	
	connect(actionFontBold, &QAction::toggled, d_app, &ApplicationWindow::setBoldFont);

	actionFontItalic = new QAction("It", d_app);
	actionFontItalic->setToolTip(d_app->tr("Italic"));
	font = appFont;
	font.setItalic(true);
	actionFontItalic->setFont(font);
	
	connect(actionFontItalic, &QAction::toggled, d_app, &ApplicationWindow::setItalicFont);

	actionSuperscript = new QAction(QPixmap(":/exp.png"), d_app->tr("Superscript"), d_app);
	connect(actionSuperscript, &QAction::triggered, d_app, &ApplicationWindow::insertSuperscript);
    actionSuperscript->setEnabled(false);

	actionSubscript = new QAction(QPixmap(":/index.png"), d_app->tr("Subscript"), d_app);
	connect(actionSubscript, &QAction::triggered, d_app, &ApplicationWindow::insertSubscript);
	actionSubscript->setEnabled(false);

	actionUnderline = new QAction("U", d_app);
	actionUnderline->setToolTip(d_app->tr("Underline (Ctrl+U)"));
	actionUnderline->setShortcut(d_app->tr("Ctrl+U"));
    font = appFont;
	font.setUnderline(true);
	actionUnderline->setFont(font);
	connect(actionUnderline, &QAction::triggered, d_app, &ApplicationWindow::underline);
	actionUnderline->setEnabled(false);

	actionGreekSymbol = new QAction(QString(QChar(0x3B1)) + QString(QChar(0x3B2)), d_app);
	actionGreekSymbol->setToolTip(d_app->tr("Greek"));
	connect(actionGreekSymbol, &QAction::triggered, d_app, &ApplicationWindow::insertGreekSymbol);

    actionGreekMajSymbol = new QAction(QString(QChar(0x393)), d_app);
	actionGreekMajSymbol->setToolTip(d_app->tr("Greek"));
	connect(actionGreekMajSymbol, &QAction::triggered, d_app, &ApplicationWindow::insertGreekMajSymbol);

	actionMathSymbol = new QAction(QString(QChar(0x222B)), d_app);
	actionMathSymbol->setToolTip(d_app->tr("Mathematical Symbols"));
	connect(actionMathSymbol, &QAction::triggered, d_app, &ApplicationWindow::insertMathSymbol);

	actionIncreasePrecision = new QAction(QPixmap(":/increase_decimals.png"), d_app->tr("Increase Precision"), d_app);
	connect(actionIncreasePrecision, &QAction::triggered, d_app, &ApplicationWindow::increasePrecision);

	actionDecreasePrecision = new QAction(QPixmap(":/decrease_decimals.png"), d_app->tr("Decrease Precision"), d_app);
	connect(actionDecreasePrecision, &QAction::triggered, d_app, &ApplicationWindow::decreasePrecision);
}


void ActionManager::customMenu(QMdiSubWindow* w)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &actionAddColToTable = d_app->actionAddColToTable;
	auto &actionAddFormula = d_app->actionAddFormula;
	auto &actionAddFunctionCurve = d_app->actionAddFunctionCurve;
	auto &actionClearSelection = d_app->actionClearSelection;
	auto &actionCopySelection = d_app->actionCopySelection;
	auto &actionCopyWindow = d_app->actionCopyWindow;
	auto &actionCutSelection = d_app->actionCutSelection;
	auto &actionExportGraph = d_app->actionExportGraph;
	auto &actionExportPDF = d_app->actionExportPDF;
	auto &actionFind = d_app->actionFind;
	auto &actionGoToRow = d_app->actionGoToRow;
	auto &actionNoteEvaluate = d_app->actionNoteEvaluate;
	auto &actionPasteSelection = d_app->actionPasteSelection;
	auto &actionPrint = d_app->actionPrint;
	auto &actionPrintAllPlots = d_app->actionPrintAllPlots;
	auto &actionPrintPreview = d_app->actionPrintPreview;
	auto &actionSaveTemplate = d_app->actionSaveTemplate;
	auto &actionSaveWindow = d_app->actionSaveWindow;
	auto &actionSetMatrixValues = d_app->actionSetMatrixValues;
	auto &actionShowAxisDialog = d_app->actionShowAxisDialog;
	auto &actionShowColumnValuesDialog = d_app->actionShowColumnValuesDialog;
	auto &actionShowCurvesDialog = d_app->actionShowCurvesDialog;
	auto &actionShowExportASCIIDialog = d_app->actionShowExportASCIIDialog;
	auto &actionShowGridDialog = d_app->actionShowGridDialog;
	auto &actionShowPlotDialog = d_app->actionShowPlotDialog;
	auto &actionShowScaleDialog = d_app->actionShowScaleDialog;
	auto &actionShowTitleDialog = d_app->actionShowTitleDialog;
	auto &actionTableRecalculate = d_app->actionTableRecalculate;
	auto &analysisMenu = d_app->analysisMenu;
	auto &d_undo_group = d_app->d_undo_group;
	auto &d_undo_view = d_app->d_undo_view;
	auto &format = d_app->format;
	auto &graphMenu = d_app->graphMenu;
	auto &matrixMenu = d_app->matrixMenu;
	auto &plot2DMenu = d_app->plot2DMenu;
	auto &plot3DMenu = d_app->plot3DMenu;
	auto &plotDataMenu = d_app->plotDataMenu;
	auto &scriptingMenu = d_app->scriptingMenu;
	auto &tableMenu = d_app->tableMenu;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    analysisMenu->menuAction()->setVisible(false);
    tableMenu->menuAction()->setVisible(false);
    plotDataMenu->menuAction()->setVisible(false);
    graphMenu->menuAction()->setVisible(false);
    matrixMenu->menuAction()->setVisible(false);
	format->menuAction()->setVisible(false);
    plot2DMenu->menuAction()->setVisible(false);
    plot3DMenu->menuAction()->setVisible(false);
#ifndef SCRIPTING_PYTHON
	scriptingMenu->menuAction()->setVisible(false);
#endif

	d_app->fileMenuAboutToShow();
	d_app->windowsMenuAboutToShow();

	// these use the same keyboard shortcut (Ctrl+Return) and should not be enabled at the same time
	actionNoteEvaluate->setEnabled(false);
	actionTableRecalculate->setEnabled(false);
	// these use the same keyboard shortcut (Alt+C) and should not be enabled at the same time
	actionShowCurvesDialog->setEnabled(false);
	actionAddColToTable->setEnabled(false);
	// these use the same keyboard shortcut (Alt+Q) and should not be enabled at the same time
	actionAddFormula->setEnabled(false);
	actionShowColumnValuesDialog->setEnabled(false);
	actionSetMatrixValues->setEnabled(false);
	// these use the same keyboard shortcut (Ctrl+Alt+F) and should not be enabled at the same time
	actionAddFunctionCurve->setEnabled(false);
	actionFind->setEnabled(false);
	// these use the same keyboard shortcut (Ctrl+Alt+G) and should not be enabled at the same time
	actionExportGraph->setEnabled(false);
	actionGoToRow->setEnabled(false);

	MdiSubWindow *mw = qobject_cast<MdiSubWindow*>(w);
	if (!mw) {
		if (d_undo_group)
			d_undo_group->setActiveStack(nullptr);
		if (d_undo_view)
			d_undo_view->setStack(nullptr);
	} else if (!qobject_cast<Note*>(mw)) {
		QUndoStack *stack = mw->undoStack();
		if (d_undo_group)
			d_undo_group->setActiveStack(stack);
		if (d_undo_view) {
			d_undo_view->setEmptyLabel(mw->objectName() + ": " + d_app->tr("Empty Stack"));
			d_undo_view->setStack(stack);
		}
	} else {
		if (d_undo_group)
			d_undo_group->setActiveStack(nullptr);
		if (d_undo_view)
			d_undo_view->setStack(nullptr);
	}
	d_app->updateUndoRedoActions();

	actionCopyWindow->setEnabled(w);
	actionPrint->setEnabled(w);
	actionPrintPreview->setEnabled(w);
	actionExportPDF->setEnabled(w);

	if(w){
		analysisMenuAboutToShow();

		actionPrintAllPlots->setEnabled(d_app->projectHas2DPlots());
		actionPrint->setEnabled(true);
		actionCutSelection->setEnabled(true);
		actionCopySelection->setEnabled(true);
		actionPasteSelection->setEnabled(true);
		actionClearSelection->setEnabled(true);
		actionSaveTemplate->setEnabled(true);
		actionSaveWindow->setEnabled(true);
		QStringList tables = d_app->tableNames() + d_app->matrixNames();
		if (!tables.isEmpty())
			actionShowExportASCIIDialog->setEnabled(true);
		else
			actionShowExportASCIIDialog->setEnabled(false);

		if (w->inherits("MultiLayer")) {
			actionAddFunctionCurve->setEnabled(true);
			actionShowCurvesDialog->setEnabled(true);
			actionAddFormula->setEnabled(true);
			actionExportGraph->setEnabled(true);

			graphMenu->menuAction()->setVisible(true);
			plotDataMenu->menuAction()->setVisible(true);
			analysisMenu->menuAction()->setVisible(true);
			format->menuAction()->setVisible(true);
			format->clear();
			format->addAction(actionShowPlotDialog);
			format->addSeparator();
            format->addAction(actionShowScaleDialog);
            format->addAction(actionShowAxisDialog);
            actionShowAxisDialog->setEnabled(true);
            format->addSeparator();
            format->addAction(actionShowGridDialog);
			format->addAction(actionShowTitleDialog);
		} else if (w->inherits("Graph3D")) {
			disableActions();

			actionPrint->setEnabled(true);
			actionSaveTemplate->setEnabled(true);
			actionSaveWindow->setEnabled(true);
			actionExportGraph->setEnabled(true);

			format->menuAction()->setVisible(true);
			format->clear();
			format->addAction(actionShowPlotDialog);
			format->addAction(actionShowScaleDialog);
			format->addAction(actionShowAxisDialog);
			format->addAction(actionShowTitleDialog);
			if (((Graph3D*)w)->coordStyle() == Qwt3D::NOCOORD)
				actionShowAxisDialog->setEnabled(false);
		} else if (w->inherits("Table")) {
			tableMenuAboutToShow();

			plot2DMenu->menuAction()->setVisible(true);
			analysisMenu->menuAction()->setVisible(true);
			tableMenu->menuAction()->setVisible(true);

			actionSaveTemplate->setEnabled(!qobject_cast<TableStatistics*>(w));
			actionTableRecalculate->setEnabled(true);
			actionAddColToTable->setEnabled(true);
			actionShowColumnValuesDialog->setEnabled(true);
			actionGoToRow->setEnabled(true);
		} else if (qobject_cast<Matrix*>(w)){
			matrixMenuAboutToShow();

			actionTableRecalculate->setEnabled(true);
			actionSetMatrixValues->setEnabled(true);
			actionGoToRow->setEnabled(true);

			plot3DMenu->menuAction()->setVisible(true);
			analysisMenu->menuAction()->setVisible(true);
			matrixMenu->menuAction()->setVisible(true);
		} else if (qobject_cast<Note*>(w)){
			#ifndef SCRIPTING_PYTHON
			scriptingMenu->menuAction()->setVisible(true);
			#endif
			actionSaveTemplate->setEnabled(false);
			actionNoteEvaluate->setEnabled(true);
			actionFind->setEnabled(true);
			if (ScriptEdit *editor = ((Note *)w)->currentEditor())
				d_app->connectScriptEditor(editor);
		} else if (w->inherits("PolarGraph")) {
			actionExportGraph->setEnabled(true);
			plotDataMenu->menuAction()->setVisible(true);
			analysisMenu->menuAction()->setVisible(true);
			format->menuAction()->setVisible(true);
			format->clear();
			format->addAction(actionShowPlotDialog);
			format->addSeparator();
			format->addAction(actionShowScaleDialog);
			format->addAction(actionShowAxisDialog);
			format->addAction(actionShowGridDialog);
			format->addAction(actionShowTitleDialog);
		} else
			disableActions();
	} else
		disableActions();

	reloadCustomActions();
}


void ActionManager::disableActions()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &actionClearSelection = d_app->actionClearSelection;
	auto &actionCopySelection = d_app->actionCopySelection;
	auto &actionCutSelection = d_app->actionCutSelection;
	auto &actionPasteSelection = d_app->actionPasteSelection;
	auto &actionPrint = d_app->actionPrint;
	auto &actionPrintAllPlots = d_app->actionPrintAllPlots;
	auto &actionSaveTemplate = d_app->actionSaveTemplate;
	auto &actionSaveWindow = d_app->actionSaveWindow;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	actionSaveTemplate->setEnabled(false);
	actionSaveWindow->setEnabled(false);
	actionPrintAllPlots->setEnabled(false);
	actionPrint->setEnabled(false);

	actionCutSelection->setEnabled(false);
	actionCopySelection->setEnabled(false);
	actionPasteSelection->setEnabled(false);
	actionClearSelection->setEnabled(false);
}


void ActionManager::customColumnActions()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &actionAdjustColumnWidth = d_app->actionAdjustColumnWidth;
	auto &actionDisregardCol = d_app->actionDisregardCol;
	auto &actionMoveColFirst = d_app->actionMoveColFirst;
	auto &actionMoveColLast = d_app->actionMoveColLast;
	auto &actionMoveColLeft = d_app->actionMoveColLeft;
	auto &actionMoveColRight = d_app->actionMoveColRight;
	auto &actionSetLabelCol = d_app->actionSetLabelCol;
	auto &actionSetXCol = d_app->actionSetXCol;
	auto &actionSetYCol = d_app->actionSetYCol;
	auto &actionSetYErrCol = d_app->actionSetYErrCol;
	auto &actionSetZCol = d_app->actionSetZCol;
	auto &actionSwapColumns = d_app->actionSwapColumns;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	actionAdjustColumnWidth->setEnabled(false);
    actionMoveColFirst->setEnabled(false);
    actionMoveColLeft->setEnabled(false);
    actionMoveColRight->setEnabled(false);
    actionMoveColLast->setEnabled(false);
    actionSetXCol->setEnabled(false);
    actionSetYCol->setEnabled(false);
    actionSetZCol->setEnabled(false);
    actionSetLabelCol->setEnabled(false);
    actionSetYErrCol->setEnabled(false);
    actionDisregardCol->setEnabled(false);
    actionSwapColumns->setEnabled(false);

	Table *t = (Table*)d_app->activeWindow(ApplicationWindow::TableWindow);
    if (!t)
		return;

    int selectedCols = t->selectedColsNumber();
    if (selectedCols == 1){
        int col = t->selectedColumn();
        if (col > 0){
            actionMoveColFirst->setEnabled(true);
            actionMoveColLeft->setEnabled(true);
        }

        if (col < t->numCols() - 1){
            actionMoveColRight->setEnabled(true);
            actionMoveColLast->setEnabled(true);
        }
	}

	if (selectedCols >= 1){
        actionSetXCol->setEnabled(true);
        actionSetYCol->setEnabled(true);
        actionSetZCol->setEnabled(true);
        actionSetYErrCol->setEnabled(true);
        actionSetLabelCol->setEnabled(true);
        actionDisregardCol->setEnabled(true);
        actionAdjustColumnWidth->setEnabled(true);
	}

	if (selectedCols == 2)
	    actionSwapColumns->setEnabled(true);
}


void ActionManager::customToolBars(QMdiSubWindow* w)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &actionTextColor = d_app->actionTextColor;
	auto &columnTools = d_app->columnTools;
	auto &d_column_tool_bar = d_app->d_column_tool_bar;
	auto &d_format_tool_bar = d_app->d_format_tool_bar;
	auto &d_matrix_tool_bar = d_app->d_matrix_tool_bar;
	auto &d_notes_tool_bar = d_app->d_notes_tool_bar;
	auto &d_plot3D_tool_bar = d_app->d_plot3D_tool_bar;
	auto &d_plot_tool_bar = d_app->d_plot_tool_bar;
	auto &d_table_tool_bar = d_app->d_table_tool_bar;
	auto &formatToolBar = d_app->formatToolBar;
	auto &noteTools = d_app->noteTools;
	auto &plot3DTools = d_app->plot3DTools;
	auto &plotMatrixBar = d_app->plotMatrixBar;
	auto &plotTools = d_app->plotTools;
	auto &tableTools = d_app->tableTools;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    disableToolbars();
	if (!w)
        return;

	actionTextColor->setVisible(false);

	if (qobject_cast<MultiLayer*>(w)){
		actionTextColor->setVisible(true);
		if (d_plot_tool_bar){
			if(!plotTools->isVisible() && !qApp->arguments().contains("-X"))
				plotTools->show();
			plotTools->setEnabled (true);
			d_app->custom2DPlotTools((MultiLayer *)w);
		}
		if(d_format_tool_bar && !formatToolBar->isVisible()){
			formatToolBar->setEnabled (true);
            if (!qApp->arguments().contains("-X"))
                formatToolBar->show();
		}
    } else if (w->inherits("Table")){
        if(d_table_tool_bar){
            if(!tableTools->isVisible() && !qApp->arguments().contains("-X"))
                tableTools->show();
            tableTools->setEnabled (true);
        }
        if (d_column_tool_bar){
            if(!columnTools->isVisible() && !qApp->arguments().contains("-X"))
                columnTools->show();
            columnTools->setEnabled (true);
            customColumnActions();
        }
	} else if (qobject_cast<Matrix*>(w)){
		 if(d_matrix_tool_bar && !plotMatrixBar->isVisible() && !qApp->arguments().contains("-X"))
            plotMatrixBar->show();
        plotMatrixBar->setEnabled (true);
	} else if (qobject_cast<Graph3D*>(w)){
		if(d_plot3D_tool_bar && !plot3DTools->isVisible() && !qApp->arguments().contains("-X"))
			plot3DTools->show();

		plot3DTools->setEnabled(((Graph3D*)w)->plotStyle() != Qwt3D::NOPLOT);
		d_app->custom3DActions(w);
	} else if (qobject_cast<Note*>(w)){
		if(d_format_tool_bar && !formatToolBar->isVisible() && !qApp->arguments().contains("-X"))
            formatToolBar->show();
		if(d_notes_tool_bar && !noteTools->isVisible() && !qApp->arguments().contains("-X"))
            noteTools->show();

        formatToolBar->setEnabled (true);
        noteTools->setEnabled (true);
        d_app->setFormatBarFont(((Note*)w)->currentEditor()->currentFont());
    } else if (qobject_cast<PolarGraph*>(w)){
		actionTextColor->setVisible(true);
		if (d_plot_tool_bar){
			if(!plotTools->isVisible() && !qApp->arguments().contains("-X"))
				plotTools->show();
			plotTools->setEnabled (true);
		}
		if(d_format_tool_bar && !formatToolBar->isVisible()){
			formatToolBar->setEnabled (true);
			if (!qApp->arguments().contains("-X"))
				formatToolBar->show();
		}
	}
}


void ActionManager::disableToolbars()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &columnTools = d_app->columnTools;
	auto &noteTools = d_app->noteTools;
	auto &plot3DTools = d_app->plot3DTools;
	auto &plotMatrixBar = d_app->plotMatrixBar;
	auto &plotTools = d_app->plotTools;
	auto &tableTools = d_app->tableTools;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	plotTools->setEnabled(false);
	tableTools->setEnabled(false);
	columnTools->setEnabled(false);
	plot3DTools->setEnabled(false);
	plotMatrixBar->setEnabled(false);
	noteTools->setEnabled(false);
}


void ActionManager::showCustomActionDialog()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	if (qApp->arguments().contains("-X"))
		return;
    CustomActionDialog *ad = new CustomActionDialog(d_app);
	ad->setAttribute(Qt::WA_DeleteOnClose);
	ad->show();
	ad->setFocus();
}


void ActionManager::addCustomAction(QAction *action, const QString& parentName, int index)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_actions = d_app->d_user_actions;
	auto &d_user_menus = d_app->d_user_menus;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    if (!action) {
        return;
    }

	QList<QToolBar *> toolBars = toolBarsList();
    for (QToolBar *t : toolBars){
        if (t->objectName() == parentName){
			if (action->icon().isNull()){
				QPixmap icon = QPixmap(16, 16);
				QRect r = QRect(0, 0, 15, 15);
				icon.fill (Qt::white);
				QPainter p;
				p.begin(&icon);
				p.setPen(QPen(Qt::red, 2));
				p.drawLine(0, 0, 15, 15);
				p.drawLine(0, 15, 15, 0);
				p.setPen(QPen(Qt::black));
				p.drawRect(r);
				action->setIcon(QIcon(icon));
				action->setIconText(d_app->tr("No Icon"));
			}
            t->addAction(action);
			if (index < 0)
            	d_user_actions << action;
			else if (index >= 0 && index < d_user_actions.size())
				d_user_actions.replace(index, action);
            return;
        }
    }

    QList<QMenu *> menus = customizableMenusList();
    menus += d_user_menus;
    for (QMenu *m : menus){
        if (m->objectName() == parentName){
			if (action->icon().isNull())
				action->setIconText(d_app->tr("No Icon"));
            m->addAction(action);
			if (index < 0)
            	d_user_actions << action;
			else if (index >= 0 && index < d_user_actions.size())
				d_user_actions.replace(index, action);
            return;
        }
    }
}


void ActionManager::reloadCustomActions()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_actions = d_app->d_user_actions;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	reloadCustomMenus();

    QList<QMenu *> menus = customizableMenusList();
	for (QAction *a : d_user_actions){
		if (!a->statusTip().isEmpty()){
    		for (QMenu *m : menus){
        		if (m->objectName() == a->statusTip()){
        		    QList<QAction *> lst = m->actions();
        		    if (!lst.contains(a))
                        m->addAction(a);

					break;
        		}
        	}
		}
	}
}


void ActionManager::removeCustomAction(QAction *action)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_actions = d_app->d_user_actions;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    int index = d_user_actions.indexOf(action);
    if (index >= 0 && index < d_user_actions.count()){
        d_user_actions.removeAt(index);
        delete action;
    }
}


void ActionManager::performCustomAction(QAction *action)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_actions = d_app->d_user_actions;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	if (!action || !d_user_actions.contains(action))
		return;

#ifdef SCRIPTING_PYTHON
	QString lang = scriptEnv->objectName();
	d_app->setScriptingLanguage("Python");

    ScriptEdit *script = new ScriptEdit(scriptEnv, 0);
    script->importASCII(action->data().toString());
    script->executeAll();
    delete script;

    d_app->setScriptingLanguage(lang);// reset old scripting language
#else
    QMessageBox::critical(d_app, d_app->tr("QtiPlot") + " - " + d_app->tr("Error"),
    d_app->tr("QtiPlot was not built with Python scripting support included!"));
#endif
}


void ActionManager::loadCustomActions()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &customActionsDirPath = d_app->customActionsDirPath;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
    QString path = customActionsDirPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks, QDir::Name);
	for (int i = 0; i < lst.count(); i++){// parse menu files first
	    QString fileName = path + lst[i];
		QFileInfo fi(fileName);
		if (fi.completeSuffix () != "qcm")
			continue;

        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

		QXmlStreamReader reader(&file);
		QString title, location;
		if (reader.readNextStartElement()) {
			if (reader.name() == QLatin1String("menu")) {
				while (reader.readNextStartElement()) {
					if (reader.name() == QLatin1String("title"))
						title = reader.readElementText();
					else if (reader.name() == QLatin1String("location"))
						location = reader.readElementText();
					else
						reader.skipCurrentElement();
				}
			}
		}
		if (!title.isEmpty())
			addCustomMenu(title, location);
	}

	for (int i = 0; i < lst.count(); i++){// parse action files
	    QString fileName = path + lst[i];
		QFileInfo fi(fileName);
		if (fi.completeSuffix () != "qca")
			continue;

        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

		QXmlStreamReader reader(&file);
		QAction *action = new QAction(d_app);
		QString parentName, filePath;
		if (reader.readNextStartElement()) {
			if (reader.name() == QLatin1String("action")) {
				while (reader.readNextStartElement()) {
					QString name = reader.name().toString();
					if (name == "text")
						action->setText(reader.readElementText());
					else if (name == "file")
						filePath = reader.readElementText();
					else if (name == "icon") {
						QString iconPath = reader.readElementText();
						if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
							action->setIcon(QIcon(iconPath));
							action->setIconText(iconPath);
						}
					} else if (name == "tooltip")
						action->setToolTip(reader.readElementText());
					else if (name == "shortcut")
						action->setShortcut(reader.readElementText());
					else if (name == "location") {
						parentName = reader.readElementText();
						action->setStatusTip(parentName);
					} else
						reader.skipCurrentElement();
				}
			}
		}
		action->setData(filePath);
		addCustomAction(action, parentName);
	}
}


QList<QMenu *> ActionManager::customizableMenusList()
{
	if (!d_app) return QList<QMenu *>();
	auto scriptEnv = d_app->scriptingEnv();
	auto &analysisMenu = d_app->analysisMenu;
	auto &decayMenu = d_app->decayMenu;
	auto &edit = d_app->edit;
	auto &exportPlotMenu = d_app->exportPlotMenu;
	auto &fileMenu = d_app->fileMenu;
	auto &fillMenu = d_app->fillMenu;
	auto &filterMenu = d_app->filterMenu;
	auto &format = d_app->format;
	auto &graphMenu = d_app->graphMenu;
	auto &help = d_app->help;
	auto &importMenu = d_app->importMenu;
	auto &matrixMenu = d_app->matrixMenu;
	auto &multiPeakMenu = d_app->multiPeakMenu;
	auto &newMenu = d_app->newMenu;
	auto &normMenu = d_app->normMenu;
	auto &plot2DMenu = d_app->plot2DMenu;
	auto &plot3DMenu = d_app->plot3DMenu;
	auto &plotDataMenu = d_app->plotDataMenu;
	auto &scriptingMenu = d_app->scriptingMenu;
	auto &smoothMenu = d_app->smoothMenu;
	auto &tableMenu = d_app->tableMenu;
	auto &view = d_app->view;
	auto &windowsMenu = d_app->windowsMenu;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	QList<QMenu *> lst;
	lst << windowsMenu << view << graphMenu << fileMenu << format << edit;
	lst << help << plot2DMenu;
	lst << analysisMenu << multiPeakMenu  << smoothMenu << filterMenu << decayMenu  << normMenu;
	lst << matrixMenu << plot3DMenu << plotDataMenu << scriptingMenu;
	lst << tableMenu << fillMenu << newMenu << exportPlotMenu << importMenu;
	return lst;
}


QList<QMenu *> ActionManager::menusList()
{
	if (!d_app) return QList<QMenu *>();
	auto scriptEnv = d_app->scriptingEnv();
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	QList<QMenu *> lst;
	QObjectList children = d_app->children();
	for (QObject *w : children){
        if (w->inherits("QMenu"))
            lst << (QMenu *)w;
    }
	return lst;
}


QList<QToolBar *> ActionManager::toolBarsList()
{
	if (!d_app) return QList<QToolBar *>();
	auto scriptEnv = d_app->scriptingEnv();
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	QList<QToolBar *> lst;
	QObjectList children = d_app->children();
	for (QObject *w : children){
        if (w->inherits("QToolBar"))
            lst << (QToolBar *)w;
    }
	return lst;
}


QMenu* ActionManager::addCustomMenu(const QString& title, const QString& parentName)
{
	if (!d_app) return nullptr;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_menus = d_app->d_user_menus;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	if (parentName == d_app->menuBar()->objectName()){
		QMenu *menu = d_app->menuBar()->addMenu(title);
		if (menu){
			menu->setObjectName(title);
			d_user_menus.append(menu);
			connect(menu, &QMenu::triggered, d_app, &ApplicationWindow::performCustomAction);
			return menu;
		}
	}

	QList<QMenu *> menus = customizableMenusList();
	for (QMenu *m : menus){
		if (m->objectName() == parentName){
			QMenu *menu = m->addMenu(title);
			if (menu){
				menu->setObjectName(title);
				d_user_menus.append(menu);
				return menu;
			}
		}
	}
	return nullptr;
}


void ActionManager::removeCustomMenu(const QString& title)
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_menus = d_app->d_user_menus;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	for (QMenu *m : d_user_menus){
		if (m->objectName() == title){
			int index = d_user_menus.indexOf(m);
			if (index >= 0 && index < d_user_menus.count()){
				d_user_menus.removeAt(index);
				delete m;
				return;
			}
		}
	}
}


void ActionManager::reloadCustomMenus()
{
	if (!d_app) return;
	auto scriptEnv = d_app->scriptingEnv();
	auto &d_user_menus = d_app->d_user_menus;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	ApplicationWindow *app = d_app;
	for (QMenu *m : d_user_menus){
		QWidget *parent = m->parentWidget();
		if (parent && !parent->children().contains(m))
			((QMenu *)parent)->addMenu(m);
	}
}

