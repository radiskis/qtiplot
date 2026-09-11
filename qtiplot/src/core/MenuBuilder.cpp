/***************************************************************************
    File                 : MenuBuilder.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Menu and toolbar construction helper for ApplicationWindow

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

#include "MenuBuilder.h"
#include "ApplicationWindow.h"
#include "PlotController3D.h"
#include "Folder.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Graph3D.h"
#include "Note.h"
#include "ScriptEdit.h"
#include "ScriptWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QKeySequence>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QDockWidget>
#include <QToolButton>
#include <QFontComboBox>
#include <QMdiArea>
#include "ColorButton.h"

void MenuBuilder::initPlot3DToolBar(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &Box = app->Box;
	auto &Frame = app->Frame;
	auto &None = app->None;
	auto &actionAnimate = app->actionAnimate;
	auto &actionFitFrame = app->actionFitFrame;
	auto &actionPerspective = app->actionPerspective;
	auto &actionResetRotation = app->actionResetRotation;
	auto &back = app->back;
	auto &barstyle = app->barstyle;
	auto &ceil = app->ceil;
	auto &conestyle = app->conestyle;
	auto &coord = app->coord;
	auto &crossHairStyle = app->crossHairStyle;
	auto &d_3D_orthogonal = app->d_3D_orthogonal;
	auto &filledmesh = app->filledmesh;
	auto &floor = app->floor;
	auto &floordata = app->floordata;
	auto &flooriso = app->flooriso;
	auto &floornone = app->floornone;
	auto &floorstyle = app->floorstyle;
	auto &front = app->front;
	auto &grids = app->grids;
	auto &hiddenline = app->hiddenline;
	auto &left = app->left;
	auto &plot3DTools = app->plot3DTools;
	auto &plotstyle = app->plotstyle;
	auto &pointstyle = app->pointstyle;
	auto &polygon = app->polygon;
	auto &right = app->right;
	auto &wireframe = app->wireframe;
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

	plot3DTools = new QToolBar( app->tr( "3D Surface" ), app );
	plot3DTools->setObjectName("plot3DTools"); // app is needed for QMainWindow::restoreState()
	plot3DTools->setIconSize( QSize(20,20) );
	app->addToolBarBreak( Qt::TopToolBarArea );
	app->addToolBar( Qt::TopToolBarArea, plot3DTools );

	coord = new QActionGroup( app );
	Box = new QAction( coord );
	Box->setIcon(QIcon(":/box.png"));
	

	Frame = new QAction( coord );
	Frame->setIcon(QIcon(":/free_axes.png"));
	

	None = new QAction( coord );
	None->setIcon(QIcon(":/no_axes.png"));
	

	plot3DTools->addAction(Frame);
	plot3DTools->addAction(Box);
	plot3DTools->addAction(None);
	Box->setChecked( true );

	plot3DTools->addSeparator();

	// grid actions
	grids = new QActionGroup( app );
	grids->setEnabled( true );
	grids->setExclusive( false );
	front = new QAction( grids );
	front->setCheckable( true );
	front->setIcon(QIcon(":/frontGrid.png"));
	back = new QAction( grids );
	back->setCheckable( true );
	back->setIcon(QIcon(":/backGrid.png"));
	right = new QAction( grids );
	right->setCheckable( true );
	right->setIcon(QIcon(":/leftGrid.png"));
	left = new QAction( grids );
	left->setCheckable( true );
	left->setIcon(QIcon(":/rightGrid.png"));
	ceil = new QAction( grids );
	ceil->setCheckable( true );
	ceil->setIcon(QIcon(":/ceilGrid.png"));
	floor = new QAction( grids );
	floor->setCheckable( true );
	floor->setIcon(QIcon(":/floorGrid.png"));

	plot3DTools->addAction(front);
	plot3DTools->addAction(back);
	plot3DTools->addAction(right);
	plot3DTools->addAction(left);
	plot3DTools->addAction(ceil);
	plot3DTools->addAction(floor);

	plot3DTools->addSeparator();

	actionPerspective = new QAction( app );
	actionPerspective->setCheckable( true );
	actionPerspective->setIcon(QIcon(":/perspective.png"));
	plot3DTools->addAction( actionPerspective );
	actionPerspective->setChecked(!d_3D_orthogonal);
	QObject::connect(actionPerspective, &QAction::toggled, app->plotController3D(), &PlotController3D::togglePerspective);

	actionResetRotation = new QAction( app );
	actionResetRotation->setCheckable( false );
	actionResetRotation->setIcon(QIcon(":/reset_rotation.png"));
	plot3DTools->addAction( actionResetRotation );
	QObject::connect(actionResetRotation, &QAction::triggered, app->plotController3D(), &PlotController3D::resetRotation);

	actionFitFrame = new QAction( app );
	actionFitFrame->setCheckable( false );
	actionFitFrame->setIcon(QIcon(":/fit_frame.png"));
	plot3DTools->addAction( actionFitFrame );
	QObject::connect(actionFitFrame, &QAction::triggered, app->plotController3D(), &PlotController3D::fitFrameToLayer);

	plot3DTools->addSeparator();

	//plot style actions
	plotstyle = new QActionGroup( app );
	wireframe = new QAction( plotstyle );
	wireframe->setCheckable( true );
	wireframe->setEnabled( true );
	wireframe->setIcon(QIcon(":/lineMesh.png"));
	hiddenline = new QAction( plotstyle );
	hiddenline->setCheckable( true );
	hiddenline->setEnabled( true );
	hiddenline->setIcon(QIcon(":/grid_only.png"));
	polygon = new QAction( plotstyle );
	polygon->setCheckable( true );
	polygon->setEnabled( true );
	polygon->setIcon(QIcon(":/no_grid.png"));
	filledmesh = new QAction( plotstyle );
	filledmesh->setCheckable( true );
	filledmesh->setIcon(QIcon(":/grid_poly.png"));
	pointstyle = new QAction( plotstyle );
	pointstyle->setCheckable( true );
	pointstyle->setIcon(QIcon(":/pointsMesh.png"));

	conestyle = new QAction( plotstyle );
	conestyle->setCheckable( true );
	conestyle->setIcon(QIcon(":/cones.png"));

	crossHairStyle = new QAction( plotstyle );
	crossHairStyle->setCheckable( true );
	crossHairStyle->setIcon(QIcon(":/crosses.png"));

	barstyle = new QAction( plotstyle );
	barstyle->setCheckable( true );
	barstyle->setIcon(QIcon(":/plot_bars.png"));

	plot3DTools->addAction(barstyle);
	plot3DTools->addAction(pointstyle);

	plot3DTools->addAction(conestyle);
	plot3DTools->addAction(crossHairStyle);
	plot3DTools->addSeparator();

	plot3DTools->addAction(wireframe);
	plot3DTools->addAction(hiddenline);
	plot3DTools->addAction(polygon);
	plot3DTools->addAction(filledmesh);
	filledmesh->setChecked( true );

	plot3DTools->addSeparator();

	//floor actions
	floorstyle = new QActionGroup( app );
	floordata = new QAction( floorstyle );
	floordata->setCheckable( true );
	floordata->setIcon(QIcon(":/floor.png"));
	flooriso = new QAction( floorstyle );
	flooriso->setCheckable( true );
	flooriso->setIcon(QIcon(":/isolines.png"));
	floornone = new QAction( floorstyle );
	floornone->setCheckable( true );
	floornone->setIcon(QIcon(":/no_floor.png"));

	plot3DTools->addAction(floordata);
	plot3DTools->addAction(flooriso);
	plot3DTools->addAction(floornone);
	floornone->setChecked( true );

	plot3DTools->addSeparator();

	actionAnimate = new QAction( app );
	actionAnimate->setCheckable( true );
	actionAnimate->setIcon(QIcon(":/movie.png"));
	plot3DTools->addAction(actionAnimate);

	plot3DTools->hide();

	QObject::connect(actionAnimate, &QAction::toggled, app->plotController3D(), &PlotController3D::toggle3DAnimation);
	QObject::connect( coord, &QActionGroup::triggered, app->plotController3D(), &PlotController3D::pickCoordSystem);
	QObject::connect( floorstyle, &QActionGroup::triggered, app->plotController3D(), &PlotController3D::pickFloorStyle);
	QObject::connect( plotstyle, &QActionGroup::triggered, app->plotController3D(), &PlotController3D::pickPlotStyle);

	QObject::connect( left, &QAction::triggered, app->plotController3D(), &PlotController3D::setLeftGrid3DPlot);
	QObject::connect( right, &QAction::triggered, app->plotController3D(), &PlotController3D::setRightGrid3DPlot);
	QObject::connect( ceil, &QAction::triggered, app->plotController3D(), &PlotController3D::setCeilGrid3DPlot);
	QObject::connect( floor, &QAction::triggered, app->plotController3D(), &PlotController3D::setFloorGrid3DPlot);
	QObject::connect( back, &QAction::triggered, app->plotController3D(), &PlotController3D::setBackGrid3DPlot);
	QObject::connect( front, &QAction::triggered, app->plotController3D(), &PlotController3D::setFrontGrid3DPlot);
}


void MenuBuilder::initToolBars(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAddColToTable = app->actionAddColToTable;
	auto &actionAddEllipse = app->actionAddEllipse;
	auto &actionAddErrorBars = app->actionAddErrorBars;
	auto &actionAddFormula = app->actionAddFormula;
	auto &actionAddFunctionCurve = app->actionAddFunctionCurve;
	auto &actionAddImage = app->actionAddImage;
	auto &actionAddInsetCurveLayer = app->actionAddInsetCurveLayer;
	auto &actionAddInsetLayer = app->actionAddInsetLayer;
	auto &actionAddLayer = app->actionAddLayer;
	auto &actionAddRectangle = app->actionAddRectangle;
	auto &actionAddText = app->actionAddText;
	auto &actionAddZoomPlot = app->actionAddZoomPlot;
	auto &actionAdjustColumnWidth = app->actionAdjustColumnWidth;
	auto &actionAlignBottom = app->actionAlignBottom;
	auto &actionAlignLeft = app->actionAlignLeft;
	auto &actionAlignRight = app->actionAlignRight;
	auto &actionAlignTop = app->actionAlignTop;
	auto &actionAppendProject = app->actionAppendProject;
	auto &actionAutomaticLayout = app->actionAutomaticLayout;
	auto &actionBoxPlot = app->actionBoxPlot;
	auto &actionClearSelection = app->actionClearSelection;
	auto &actionColorMap = app->actionColorMap;
	auto &actionContourMap = app->actionContourMap;
	auto &actionCopySelection = app->actionCopySelection;
	auto &actionCopyWindow = app->actionCopyWindow;
	auto &actionCustomSharedAxisLayers = app->actionCustomSharedAxisLayers;
	auto &actionCutSelection = app->actionCutSelection;
	auto &actionDecreaseIndent = app->actionDecreaseIndent;
	auto &actionDecreasePrecision = app->actionDecreasePrecision;
	auto &actionDisregardCol = app->actionDisregardCol;
	auto &actionDragCurve = app->actionDragCurve;
	auto &actionDrawPoints = app->actionDrawPoints;
	auto &actionExportPDF = app->actionExportPDF;
	auto &actionExtractGraphs = app->actionExtractGraphs;
	auto &actionExtractLayers = app->actionExtractLayers;
	auto &actionFind = app->actionFind;
	auto &actionFindNext = app->actionFindNext;
	auto &actionFindPrev = app->actionFindPrev;
	auto &actionFlipMatrixHorizontally = app->actionFlipMatrixHorizontally;
	auto &actionFlipMatrixVertically = app->actionFlipMatrixVertically;
	auto &actionFontBold = app->actionFontBold;
	auto &actionFontBox = app->actionFontBox;
	auto &actionFontItalic = app->actionFontItalic;
	auto &actionFontSize = app->actionFontSize;
	auto &actionGrayMap = app->actionGrayMap;
	auto &actionGreekMajSymbol = app->actionGreekMajSymbol;
	auto &actionGreekSymbol = app->actionGreekSymbol;
	auto &actionImagePlot = app->actionImagePlot;
	auto &actionImageProfilesPlot = app->actionImageProfilesPlot;
	auto &actionIncreaseIndent = app->actionIncreaseIndent;
	auto &actionIncreasePrecision = app->actionIncreasePrecision;
	auto &actionLoad = app->actionLoad;
	auto &actionLowerEnrichment = app->actionLowerEnrichment;
	auto &actionMagnify = app->actionMagnify;
	auto &actionMagnifyHor = app->actionMagnifyHor;
	auto &actionMagnifyVert = app->actionMagnifyVert;
	auto &actionMathSymbol = app->actionMathSymbol;
	auto &actionMoveColFirst = app->actionMoveColFirst;
	auto &actionMoveColLast = app->actionMoveColLast;
	auto &actionMoveColLeft = app->actionMoveColLeft;
	auto &actionMoveColRight = app->actionMoveColRight;
	auto &actionMoveRowDown = app->actionMoveRowDown;
	auto &actionMoveRowUp = app->actionMoveRowUp;
	auto &actionNewFolder = app->actionNewFolder;
	auto &actionNewFunctionPlot = app->actionNewFunctionPlot;
	auto &actionNewGraph = app->actionNewGraph;
	auto &actionNewLegend = app->actionNewLegend;
	auto &actionNewMatrix = app->actionNewMatrix;
	auto &actionNewNote = app->actionNewNote;
	auto &actionNewProject = app->actionNewProject;
	auto &actionNewSurfacePlot = app->actionNewSurfacePlot;
	auto &actionNewTable = app->actionNewTable;
	auto &actionNoteExecute = app->actionNoteExecute;
	auto &actionNoteExecuteAll = app->actionNoteExecuteAll;
	auto &actionNoteStop = app->actionNoteStop;
	auto &actionOpen = app->actionOpen;
	auto &actionOpenExcel = app->actionOpenExcel;
	auto &actionOpenOds = app->actionOpenOds;
	auto &actionOpenTemplate = app->actionOpenTemplate;
	auto &actionPasteSelection = app->actionPasteSelection;
	auto &actionPlot2HorizontalLayers = app->actionPlot2HorizontalLayers;
	auto &actionPlot2VerticalLayers = app->actionPlot2VerticalLayers;
	auto &actionPlot3DBars = app->actionPlot3DBars;
	auto &actionPlot3DHiddenLine = app->actionPlot3DHiddenLine;
	auto &actionPlot3DPolygons = app->actionPlot3DPolygons;
	auto &actionPlot3DRibbon = app->actionPlot3DRibbon;
	auto &actionPlot3DScatter = app->actionPlot3DScatter;
	auto &actionPlot3DTrajectory = app->actionPlot3DTrajectory;
	auto &actionPlot3DWireFrame = app->actionPlot3DWireFrame;
	auto &actionPlot3DWireSurface = app->actionPlot3DWireSurface;
	auto &actionPlot4Layers = app->actionPlot4Layers;
	auto &actionPlotArea = app->actionPlotArea;
	auto &actionPlotDoubleYAxis = app->actionPlotDoubleYAxis;
	auto &actionPlotHistogram = app->actionPlotHistogram;
	auto &actionPlotHorSteps = app->actionPlotHorSteps;
	auto &actionPlotHorizontalBars = app->actionPlotHorizontalBars;
	auto &actionPlotL = app->actionPlotL;
	auto &actionPlotLP = app->actionPlotLP;
	auto &actionPlotP = app->actionPlotP;
	auto &actionPlotPie = app->actionPlotPie;
	auto &actionPlotSpline = app->actionPlotSpline;
	auto &actionPlotStackedHistograms = app->actionPlotStackedHistograms;
	auto &actionPlotStackedLayers = app->actionPlotStackedLayers;
	auto &actionPlotVectXYAM = app->actionPlotVectXYAM;
	auto &actionPlotVectXYXY = app->actionPlotVectXYXY;
	auto &actionPlotVertSteps = app->actionPlotVertSteps;
	auto &actionPlotVerticalBars = app->actionPlotVerticalBars;
	auto &actionPlotVerticalDropLines = app->actionPlotVerticalDropLines;
	auto &actionPrint = app->actionPrint;
	auto &actionPrintPreview = app->actionPrintPreview;
	auto &actionRaiseEnrichment = app->actionRaiseEnrichment;
	auto &actionRedo = app->actionRedo;
	auto &actionReplace = app->actionReplace;
	auto &actionRotateMatrix = app->actionRotateMatrix;
	auto &actionRotateMatrixMinus = app->actionRotateMatrixMinus;
	auto &actionSaveProject = app->actionSaveProject;
	auto &actionSaveTemplate = app->actionSaveTemplate;
	auto &actionSetAscValues = app->actionSetAscValues;
	auto &actionSetLabelCol = app->actionSetLabelCol;
	auto &actionSetMatrixValues = app->actionSetMatrixValues;
	auto &actionSetRandomNormalValues = app->actionSetRandomNormalValues;
	auto &actionSetRandomValues = app->actionSetRandomValues;
	auto &actionSetXCol = app->actionSetXCol;
	auto &actionSetYCol = app->actionSetYCol;
	auto &actionSetYErrCol = app->actionSetYErrCol;
	auto &actionSetZCol = app->actionSetZCol;
	auto &actionShowColStatistics = app->actionShowColStatistics;
	auto &actionShowColumnValuesDialog = app->actionShowColumnValuesDialog;
	auto &actionShowCurvesDialog = app->actionShowCurvesDialog;
	auto &actionShowExplorer = app->actionShowExplorer;
	auto &actionShowLayerDialog = app->actionShowLayerDialog;
	auto &actionShowLog = app->actionShowLog;
	auto &actionShowRowStatistics = app->actionShowRowStatistics;
	auto &actionShowScriptWindow = app->actionShowScriptWindow;
	auto &actionSortTable = app->actionSortTable;
	auto &actionStackBars = app->actionStackBars;
	auto &actionStackColumns = app->actionStackColumns;
	auto &actionStemPlot = app->actionStemPlot;
	auto &actionSubscript = app->actionSubscript;
	auto &actionSuperscript = app->actionSuperscript;
	auto &actionSwapColumns = app->actionSwapColumns;
	auto &actionTextColor = app->actionTextColor;
	auto &actionTimeStamp = app->actionTimeStamp;
	auto &actionUnderline = app->actionUnderline;
	auto &actionUndo = app->actionUndo;
	auto &actionUnzoom = app->actionUnzoom;
	auto &actionWaterfallPlot = app->actionWaterfallPlot;
	auto &btnArrow = app->btnArrow;
	auto &btnCursor = app->btnCursor;
	auto &btnLine = app->btnLine;
	auto &btnMovePoints = app->btnMovePoints;
	auto &btnPicker = app->btnPicker;
	auto &btnPointer = app->btnPointer;
	auto &btnRemovePoints = app->btnRemovePoints;
	auto &btnSelect = app->btnSelect;
	auto &btnZoomIn = app->btnZoomIn;
	auto &btnZoomOut = app->btnZoomOut;
	auto &columnTools = app->columnTools;
	auto &dataTools = app->dataTools;
	auto &displayBar = app->displayBar;
	auto &editTools = app->editTools;
	auto &fileTools = app->fileTools;
	auto &formatToolBar = app->formatToolBar;
	auto &info = app->info;
	auto &noteTools = app->noteTools;
	auto &plotMatrixBar = app->plotMatrixBar;
	auto &plotTools = app->plotTools;
	auto &tableTools = app->tableTools;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
	auto &actionCommentSelection = app->actionCommentSelection;
	auto &actionUncommentSelection = app->actionUncommentSelection;
#endif
#ifdef Q_OS_WIN
#endif

	initPlot3DToolBar(app);

	app->setWindowIcon(QIcon(":/logo.png"));
	QPixmap openIcon, saveIcon;

	fileTools = new QToolBar(app->tr( "File" ), app);
	fileTools->setObjectName("fileTools"); // app is needed for QMainWindow::restoreState()
	fileTools->setIconSize( QSize(18,20) );
	app->addToolBar( Qt::TopToolBarArea, fileTools );

	fileTools->addAction(actionNewProject);
	fileTools->addAction(actionNewFolder);
	fileTools->addAction(actionNewTable);
	fileTools->addAction(actionNewMatrix);
	fileTools->addAction(actionNewNote);
	fileTools->addAction(actionNewGraph);
	fileTools->addAction(actionNewFunctionPlot);
	fileTools->addAction(actionNewSurfacePlot);
	fileTools->addSeparator ();
	fileTools->addAction(actionOpen);
	fileTools->addAction(actionOpenTemplate);
	fileTools->addAction(actionOpenExcel);
	fileTools->addAction(actionOpenOds);
	fileTools->addAction(actionAppendProject);
	fileTools->addAction(actionSaveProject);
	fileTools->addAction(actionSaveTemplate);
	fileTools->addSeparator ();
	fileTools->addAction(actionLoad);
	fileTools->addSeparator ();
	fileTools->addAction(actionCopyWindow);
	fileTools->addAction(actionPrint);
	fileTools->addAction(actionPrintPreview);
	fileTools->addAction(actionExportPDF);
	fileTools->addSeparator();
	fileTools->addAction(actionShowExplorer);
	fileTools->addAction(actionShowLog);
#ifdef SCRIPTING_PYTHON
	fileTools->addAction(actionShowScriptWindow);
#endif

	editTools = new QToolBar(app->tr("Edit"), app);
	editTools->setObjectName("editTools"); // app is needed for QMainWindow::restoreState()
	editTools->setIconSize( QSize(18,20) );
	app->addToolBar( editTools );

	editTools->addAction(actionUndo);
	editTools->addAction(actionRedo);
	editTools->addAction(actionCutSelection);
	editTools->addAction(actionCopySelection);
	editTools->addAction(actionPasteSelection);
	editTools->addAction(actionClearSelection);

	noteTools = new QToolBar(app->tr("Notes"), app);
	noteTools->setObjectName("noteTools"); // app is needed for QMainWindow::restoreState()
	noteTools->setIconSize( QSize(18,20) );
#ifdef SCRIPTING_PYTHON
	noteTools->addAction(actionNoteExecuteAll);
	noteTools->addAction(actionNoteExecute);
	noteTools->addAction(actionNoteStop);
	noteTools->addAction(actionCommentSelection);
	noteTools->addAction(actionUncommentSelection);
#endif
	noteTools->addAction(actionDecreaseIndent);
	noteTools->addAction(actionIncreaseIndent);
	noteTools->addAction(actionFind);
	noteTools->addAction(actionFindPrev);
	noteTools->addAction(actionFindNext);
	noteTools->addAction(actionReplace);
	app->addToolBar( noteTools );

	plotTools = new QToolBar(app->tr("Plot"), app);
	plotTools->setObjectName("plotTools"); // app is needed for QMainWindow::restoreState()
	plotTools->setIconSize( QSize(16,20) );
	app->addToolBar( plotTools );

	plotTools->addAction(actionAddLayer);
	plotTools->addAction(actionAddInsetLayer);
	plotTools->addAction(actionAddInsetCurveLayer);
	plotTools->addSeparator();
	plotTools->addAction(actionShowLayerDialog);
	plotTools->addAction(actionAutomaticLayout);
	plotTools->addSeparator();
	plotTools->addAction(actionExtractLayers);
	plotTools->addAction(actionExtractGraphs);
	plotTools->addSeparator();
	plotTools->addAction(actionAddErrorBars);
	plotTools->addAction(actionShowCurvesDialog);
	plotTools->addAction(actionAddFunctionCurve);
	plotTools->addAction(actionNewLegend);
	plotTools->addSeparator ();
	plotTools->addAction(actionUnzoom);

	dataTools = new QActionGroup( app );
	dataTools->setExclusive( true );

	btnPointer = new QAction(app->tr("Disable &Tools"), app);
	btnPointer->setActionGroup(dataTools);
	btnPointer->setCheckable( true );
	btnPointer->setIcon(QIcon(":/pointer.png"));
	btnPointer->setChecked(true);
	plotTools->addAction(btnPointer);

	actionMagnify->setActionGroup(dataTools);
	actionMagnify->setCheckable( true );

	actionMagnifyHor->setActionGroup(dataTools);
	actionMagnifyHor->setCheckable( true );

	actionMagnifyVert->setActionGroup(dataTools);
	actionMagnifyVert->setCheckable( true );

	btnZoomIn = new QAction(app->tr("&Zoom In"), app);
	btnZoomIn->setShortcut( app->tr("Ctrl++") );
	btnZoomIn->setActionGroup(dataTools);
	btnZoomIn->setCheckable( true );
	btnZoomIn->setIcon(QIcon(":/zoom.png"));

	btnZoomOut = new QAction(app->tr("&Zoom Out"), app);
	btnZoomOut->setShortcut( app->tr("Ctrl+-") );
	btnZoomOut->setActionGroup(dataTools);
	btnZoomOut->setCheckable( true );
	btnZoomOut->setIcon(QIcon(":/zoomOut.png"));

	QMenu *menu_zoom = new QMenu(app);
	menu_zoom->addAction(actionMagnify);
	menu_zoom->addAction(actionMagnifyHor);
	menu_zoom->addAction(actionMagnifyVert);
	menu_zoom->addAction(btnZoomIn);
	menu_zoom->addAction(btnZoomOut);

	QToolButton *btn_zoom = new QToolButton(app);
	btn_zoom->setMenu(menu_zoom);
	btn_zoom->setPopupMode(QToolButton::MenuButtonPopup);
	btn_zoom->setDefaultAction(actionMagnify);
	QObject::connect(menu_zoom, &QMenu::triggered, btn_zoom, &QToolButton::setDefaultAction);

	plotTools->addWidget(btn_zoom);

	btnCursor = new QAction(app->tr("&Data Reader"), app);
	btnCursor->setShortcut( app->tr("CTRL+D") );
	btnCursor->setActionGroup(dataTools);
	btnCursor->setCheckable( true );
	btnCursor->setIcon(QIcon(":/select.png"));
	plotTools->addAction(btnCursor);

	btnSelect = new QAction(app->tr("&Select Data Range"), app);
	btnSelect->setShortcut(QKeySequence(app->tr("Alt+Shift+S")));
	btnSelect->setActionGroup(dataTools);
	btnSelect->setCheckable( true );
	btnSelect->setIcon(QIcon(":/cursors.png"));
	plotTools->addAction(btnSelect);

	btnPicker = new QAction(app->tr("S&creen Reader"), app);
	btnPicker->setActionGroup(dataTools);
	btnPicker->setCheckable( true );
	btnPicker->setIcon(QIcon(":/cursor_16.png"));
	plotTools->addAction(btnPicker);

	actionDrawPoints = new QAction(app->tr("&Draw Data Points"), app);
	actionDrawPoints->setActionGroup(dataTools);
	actionDrawPoints->setCheckable( true );
	actionDrawPoints->setIcon(QIcon(":/draw_points.png"));
	plotTools->addAction(actionDrawPoints);

	btnMovePoints = new QAction(app->tr("&Move Data Points..."), app);
	btnMovePoints->setShortcut( app->tr("Ctrl+ALT+M") );
	btnMovePoints->setActionGroup(dataTools);
	btnMovePoints->setCheckable( true );
	btnMovePoints->setIcon(QIcon(":/hand.png"));
	plotTools->addAction(btnMovePoints);

	btnRemovePoints = new QAction(app->tr("Remove &Bad Data Points..."), app);
	btnRemovePoints->setShortcut( app->tr("Alt+B") );
	btnRemovePoints->setActionGroup(dataTools);
	btnRemovePoints->setCheckable( true );
	btnRemovePoints->setIcon(QIcon(":/delete.png"));
	plotTools->addAction(btnRemovePoints);

	actionDragCurve = new QAction(app->tr("Dra&g Curve"), app);
	actionDragCurve->setActionGroup(dataTools);
	actionDragCurve->setCheckable( true );
	actionDragCurve->setIcon(QIcon(":/drag_curve.png"));
	plotTools->addAction(actionDragCurve);

	QObject::connect(dataTools, &QActionGroup::triggered, app, &ApplicationWindow::pickDataTool);
	plotTools->addSeparator ();

	actionAddFormula = new QAction(app->tr("Add E&quation"), app);
	actionAddFormula->setShortcut( app->tr("ALT+Q") );
	
	actionAddFormula->setIcon(QIcon(":/formula.png"));
	QObject::connect(actionAddFormula, &QAction::triggered, app, &ApplicationWindow::addTexFormula);
	plotTools->addAction(actionAddFormula);

	actionAddText = new QAction(app->tr("Add &Text"), app);
	actionAddText->setShortcut(QKeySequence(app->tr("Shift+T")));
	actionAddText->setIcon(QIcon(":/text.png"));
	
	QObject::connect(actionAddText, &QAction::triggered, app, &ApplicationWindow::addText);
	plotTools->addAction(actionAddText);

	btnArrow = new QAction(app->tr("Draw &Arrow"), app);
	btnArrow->setShortcut( app->tr("CTRL+ALT+A") );
	btnArrow->setActionGroup(dataTools);
	btnArrow->setCheckable( true );
	btnArrow->setIcon(QIcon(":/arrow.png"));
	plotTools->addAction(btnArrow);

	btnLine = new QAction(app->tr("Draw &Line"), app);
	btnLine->setShortcut( app->tr("CTRL+ALT+L") );
	btnLine->setActionGroup(dataTools);
	btnLine->setCheckable( true );
	btnLine->setIcon(QIcon(":/lPlot.png"));
	plotTools->addAction(btnLine);

	QPixmap pix = QPixmap(16, 16);
	pix.fill(Qt::transparent);
	QPainter p;
	p.begin(&pix);
	p.setBrush(Qt::lightGray);
	p.drawRect(QRect(1, 2, 12, 10));

    actionAddRectangle = new QAction(app->tr("Add &Rectangle"), app);
	actionAddRectangle->setShortcut( app->tr("CTRL+ALT+R") );
	
	actionAddRectangle->setIcon(QIcon(pix));
	QObject::connect(actionAddRectangle, &QAction::triggered, app, &ApplicationWindow::addRectangle);
	plotTools->addAction(actionAddRectangle);

	pix.fill(Qt::transparent);
	p.setRenderHint(QPainter::Antialiasing);
	p.drawEllipse(QRect(1, 2, 14, 12));
	p.end();

	actionAddEllipse = new QAction(app->tr("Add &Ellipse"), app);
	actionAddEllipse->setShortcut( app->tr("CTRL+ALT+E") );
	
	actionAddEllipse->setIcon(QIcon(pix));
	QObject::connect(actionAddEllipse, &QAction::triggered, app, &ApplicationWindow::addEllipse);
	plotTools->addAction(actionAddEllipse);

	plotTools->addAction(actionTimeStamp);
	plotTools->addAction(actionAddImage);

	plotTools->addSeparator();
	plotTools->addAction(actionRaiseEnrichment);
	plotTools->addAction(actionLowerEnrichment);
	plotTools->addAction(actionAlignLeft);
	plotTools->addAction(actionAlignRight);
	plotTools->addAction(actionAlignTop);
	plotTools->addAction(actionAlignBottom);

	plotTools->hide();

	tableTools = new QToolBar(app->tr("Table"), app);
	tableTools->setObjectName("tableTools"); // app is needed for QMainWindow::restoreState()
	tableTools->setIconSize( QSize(16, 20));
	app->addToolBar(Qt::TopToolBarArea, tableTools);

	QMenu *menuPlotLine = new QMenu(app);
	menuPlotLine->addAction(actionPlotL);
	menuPlotLine->addAction(actionPlotHorSteps);
	menuPlotLine->addAction(actionPlotVertSteps);

	QToolButton *btnPlotLine = new QToolButton(app);
	btnPlotLine->setMenu(menuPlotLine);
	btnPlotLine->setPopupMode(QToolButton::MenuButtonPopup);
	btnPlotLine->setDefaultAction(actionPlotL);
	QObject::connect(menuPlotLine, &QMenu::triggered, btnPlotLine, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnPlotLine);

	QMenu *menuPlotScatter = new QMenu(app);
	menuPlotScatter->addAction(actionPlotP);
	menuPlotScatter->addAction(actionPlotVerticalDropLines);

	QToolButton *btnPlotScatter = new QToolButton(app);
	btnPlotScatter->setMenu(menuPlotScatter);
	btnPlotScatter->setPopupMode(QToolButton::MenuButtonPopup);
	btnPlotScatter->setDefaultAction(actionPlotP);
	QObject::connect(menuPlotScatter, &QMenu::triggered, btnPlotScatter, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnPlotScatter);

	QMenu *menuPlotLineSymbol = new QMenu(app);
	menuPlotLineSymbol->addAction(actionPlotLP);
	menuPlotLineSymbol->addAction(actionPlotSpline);

	QToolButton *btnPlotLineSymbol = new QToolButton(app);
	btnPlotLineSymbol->setMenu(menuPlotLineSymbol);
	btnPlotLineSymbol->setPopupMode(QToolButton::MenuButtonPopup);
	btnPlotLineSymbol->setDefaultAction(actionPlotLP);
	QObject::connect(menuPlotLineSymbol, &QMenu::triggered, btnPlotLineSymbol, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnPlotLineSymbol);

	QMenu *menuPlotBars = new QMenu(app);
	menuPlotBars->addAction(actionPlotVerticalBars);
	menuPlotBars->addAction(actionPlotHorizontalBars);
	menuPlotBars->addAction(actionStackColumns);
	menuPlotBars->addAction(actionStackBars);

	QToolButton *btnPlotBars = new QToolButton(app);
	btnPlotBars->setMenu(menuPlotBars);
	btnPlotBars->setPopupMode(QToolButton::MenuButtonPopup);
	btnPlotBars->setDefaultAction(actionPlotVerticalBars);
	QObject::connect(menuPlotBars, &QMenu::triggered, btnPlotBars, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnPlotBars);

	tableTools->addAction(actionPlotArea);
	tableTools->addAction(actionPlotPie);

	QMenu *menuStatisticPlots = new QMenu(app);
	menuStatisticPlots->addAction(actionBoxPlot);
	menuStatisticPlots->addAction(actionPlotHistogram);
	menuStatisticPlots->addAction(actionPlotStackedHistograms);
	menuStatisticPlots->addAction(actionStemPlot);

	QToolButton *btnStatisticPlots = new QToolButton(app);
	btnStatisticPlots->setMenu(menuStatisticPlots);
	btnStatisticPlots->setPopupMode(QToolButton::MenuButtonPopup);
	btnStatisticPlots->setDefaultAction(actionBoxPlot);
	QObject::connect(menuStatisticPlots, &QMenu::triggered, btnStatisticPlots, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnStatisticPlots);

	QMenu *menuVectorPlots = new QMenu(app);
	menuVectorPlots->addAction(actionPlotVectXYXY);
	menuVectorPlots->addAction(actionPlotVectXYAM);

	QToolButton *btnVectorPlots = new QToolButton(app);
	btnVectorPlots->setMenu(menuVectorPlots);
	btnVectorPlots->setPopupMode(QToolButton::MenuButtonPopup);
	btnVectorPlots->setDefaultAction(actionPlotVectXYXY);
	QObject::connect(menuVectorPlots, &QMenu::triggered, btnVectorPlots, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnVectorPlots);

	QMenu *menuPlotSpecial = new QMenu(app);
	menuPlotSpecial->addAction(actionPlotDoubleYAxis);
	menuPlotSpecial->addAction(actionWaterfallPlot);
	menuPlotSpecial->addAction(actionAddZoomPlot);
	menuPlotSpecial->addAction(actionPlot2VerticalLayers);
	menuPlotSpecial->addAction(actionPlot2HorizontalLayers);
	menuPlotSpecial->addAction(actionPlot4Layers);
	menuPlotSpecial->addAction(actionPlotStackedLayers);
	menuPlotSpecial->addAction(actionCustomSharedAxisLayers);

	QToolButton *btnPlotSpecial = new QToolButton(app);
	btnPlotSpecial->setMenu(menuPlotSpecial);
	btnPlotSpecial->setPopupMode(QToolButton::MenuButtonPopup);
	btnPlotSpecial->setDefaultAction(actionPlotDoubleYAxis);
	QObject::connect(menuPlotSpecial, &QMenu::triggered, btnPlotSpecial, &QToolButton::setDefaultAction);
	tableTools->addWidget(btnPlotSpecial);

	tableTools->addSeparator ();

	QMenu *menu3DPlots = new QMenu(app);
	menu3DPlots->addAction(actionPlot3DBars);
	menu3DPlots->addAction(actionPlot3DRibbon);
	menu3DPlots->addAction(actionPlot3DScatter);
	menu3DPlots->addAction(actionPlot3DTrajectory);

	QToolButton *btn3DPlots = new QToolButton(app);
	btn3DPlots->setMenu(menu3DPlots);
	btn3DPlots->setPopupMode(QToolButton::MenuButtonPopup);
	btn3DPlots->setDefaultAction(actionPlot3DBars);
	QObject::connect(menu3DPlots, &QMenu::triggered, btn3DPlots, &QToolButton::setDefaultAction);
	tableTools->addWidget(btn3DPlots);

	tableTools->setEnabled(false);
    tableTools->hide();

	columnTools = new QToolBar(app->tr( "Column"), app);
	columnTools->setObjectName("columnTools"); // app is needed for QMainWindow::restoreState()
	columnTools->setIconSize(QSize(16, 20));
	app->addToolBar(Qt::TopToolBarArea, columnTools);

	columnTools->addAction(actionShowColumnValuesDialog);
	columnTools->addAction(actionSetAscValues);
	columnTools->addAction(actionSetRandomValues);
	columnTools->addAction(actionSetRandomNormalValues);
	columnTools->addSeparator();
	columnTools->addAction(actionSortTable);
	columnTools->addAction(actionShowColStatistics);
	columnTools->addAction(actionShowRowStatistics);
	columnTools->addSeparator();
	columnTools->addAction(actionSetXCol);
	columnTools->addAction(actionSetYCol);
	columnTools->addAction(actionSetZCol);
	columnTools->addAction(actionSetYErrCol);
	columnTools->addAction(actionSetLabelCol);
	columnTools->addAction(actionDisregardCol);
	columnTools->addSeparator();
	columnTools->addAction(actionMoveColFirst);
	columnTools->addAction(actionMoveColLeft);
	columnTools->addAction(actionMoveColRight);
	columnTools->addAction(actionMoveColLast);
	columnTools->addAction(actionSwapColumns);
	columnTools->addSeparator();
	columnTools->addAction(actionAdjustColumnWidth);
	columnTools->addAction(actionMoveRowUp);
	columnTools->addAction(actionMoveRowDown);
	columnTools->addSeparator();
	columnTools->addAction(actionAddColToTable);
    columnTools->setEnabled(false);
	columnTools->hide();

	displayBar = new QToolBar( app->tr( "Data Display" ), app );
    displayBar->setAllowedAreas( Qt::TopToolBarArea | Qt::BottomToolBarArea );
	displayBar->setObjectName("displayBar"); // app is needed for QMainWindow::restoreState()
	info = new QLineEdit( app );
	displayBar->addWidget( info );
	info->setReadOnly(true);
    QPalette palette;
    palette.setColor(QPalette::Text, QColor(Qt::green));
	palette.setColor(QPalette::HighlightedText, QColor(Qt::darkGreen));
	palette.setColor(QPalette::Base, QColor(Qt::black));
	info->setPalette(palette);

	app->addToolBar( Qt::TopToolBarArea, displayBar );
	displayBar->hide();

    app->insertToolBarBreak(displayBar);

	plotMatrixBar = new QToolBar( app->tr( "Matrix Plot" ), app);
	plotMatrixBar->setObjectName("plotMatrixBar");
	app->addToolBar(Qt::BottomToolBarArea, plotMatrixBar);

	QMenu *menu3DMatrix = new QMenu(app);
	menu3DMatrix->addAction(actionPlot3DWireFrame);
	menu3DMatrix->addAction(actionPlot3DHiddenLine);
	menu3DMatrix->addAction(actionPlot3DWireSurface);
	menu3DMatrix->addAction(actionPlot3DPolygons);

	QToolButton *btn3DMatrix = new QToolButton(app);
	btn3DMatrix->setMenu(menu3DMatrix);
	btn3DMatrix->setPopupMode(QToolButton::MenuButtonPopup);
	btn3DMatrix->setDefaultAction(actionPlot3DWireSurface);
	QObject::connect(menu3DMatrix, &QMenu::triggered, btn3DMatrix, &QToolButton::setDefaultAction);
	plotMatrixBar->addWidget(btn3DMatrix);

	plotMatrixBar->addAction(actionPlot3DBars);
	plotMatrixBar->addAction(actionPlot3DScatter);

	plotMatrixBar->addSeparator();

	QMenu *menuContourPlot = new QMenu(app);
	menuContourPlot->addAction(actionColorMap);
	menuContourPlot->addAction(actionContourMap);
	menuContourPlot->addAction(actionGrayMap);

	QToolButton *btnContourPlot = new QToolButton(app);
	btnContourPlot->setMenu(menuContourPlot);
	btnContourPlot->setPopupMode(QToolButton::MenuButtonPopup);
	btnContourPlot->setDefaultAction(actionColorMap);
	QObject::connect(menuContourPlot, &QMenu::triggered, btnContourPlot, &QToolButton::setDefaultAction);
	plotMatrixBar->addWidget(btnContourPlot);

	plotMatrixBar->addAction(actionImagePlot);
	plotMatrixBar->addAction(actionImageProfilesPlot);
	plotMatrixBar->addAction(actionPlotHistogram);
	plotMatrixBar->addSeparator();
	plotMatrixBar->addAction(actionSetMatrixValues);
	plotMatrixBar->addAction(actionFlipMatrixHorizontally);
	plotMatrixBar->addAction(actionFlipMatrixVertically);
	plotMatrixBar->addAction(actionRotateMatrix);
	plotMatrixBar->addAction(actionRotateMatrixMinus);
	plotMatrixBar->addSeparator();
	plotMatrixBar->addAction(actionIncreasePrecision);
	plotMatrixBar->addAction(actionDecreasePrecision);
	plotMatrixBar->hide();

	formatToolBar = new QToolBar(app->tr( "Format" ), app);
	formatToolBar->setObjectName("formatToolBar");
	app->addToolBar(Qt::TopToolBarArea, formatToolBar);

	QFontComboBox *fb = new QFontComboBox();
	QObject::connect(fb, &QFontComboBox::currentFontChanged, app, &ApplicationWindow::setFontFamily);
	actionFontBox = formatToolBar->addWidget(fb);

	QSpinBox *sb = new QSpinBox();
	QObject::connect(sb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), app, &ApplicationWindow::setFontSize);
	actionFontSize = formatToolBar->addWidget(sb);

	formatToolBar->addAction(actionFontBold);
	formatToolBar->addAction(actionFontItalic);

	formatToolBar->addAction(actionUnderline);
	formatToolBar->addAction(actionSuperscript);
	formatToolBar->addAction(actionSubscript);
	formatToolBar->addAction(actionGreekSymbol);
	formatToolBar->addAction(actionGreekMajSymbol);
	formatToolBar->addAction(actionMathSymbol);

	ColorButton *cBtn = new ColorButton();
	QObject::connect(cBtn, &ColorButton::colorChanged, app, &ApplicationWindow::setTextColor);
	actionTextColor = formatToolBar->addWidget(cBtn);

	formatToolBar->setEnabled(false);
	formatToolBar->hide();

	QList<QToolBar *> toolBars = app->toolBarsList();
	for (QToolBar *t : toolBars)
		QObject::connect(t, &QToolBar::actionTriggered, app, &ApplicationWindow::performCustomAction);
}


void MenuBuilder::initMainMenu(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAbout = app->actionAbout;
	auto &actionAddEllipse = app->actionAddEllipse;
	auto &actionAddErrorBars = app->actionAddErrorBars;
	auto &actionAddFormula = app->actionAddFormula;
	auto &actionAddFunctionCurve = app->actionAddFunctionCurve;
	auto &actionAddImage = app->actionAddImage;
	auto &actionAddInsetCurveLayer = app->actionAddInsetCurveLayer;
	auto &actionAddInsetLayer = app->actionAddInsetLayer;
	auto &actionAddLayer = app->actionAddLayer;
	auto &actionAddRectangle = app->actionAddRectangle;
	auto &actionAddText = app->actionAddText;
	auto &actionAutomaticLayout = app->actionAutomaticLayout;
	auto &actionCheckUpdates = app->actionCheckUpdates;
	auto &actionChooseHelpFolder = app->actionChooseHelpFolder;
	auto &actionClearLogInfo = app->actionClearLogInfo;
	auto &actionClearSelection = app->actionClearSelection;
	auto &actionColorMap = app->actionColorMap;
	auto &actionContourMap = app->actionContourMap;
	auto &actionCopySelection = app->actionCopySelection;
	auto &actionDeleteFitTables = app->actionDeleteFitTables;
	auto &actionDeleteLayer = app->actionDeleteLayer;
	auto &actionDonate = app->actionDonate;
	auto &actionDownloadManual = app->actionDownloadManual;
	auto &actionExtractGraphs = app->actionExtractGraphs;
	auto &actionExtractLayers = app->actionExtractLayers;
	auto &actionGrayMap = app->actionGrayMap;
	auto &actionHelpBugReports = app->actionHelpBugReports;
	auto &actionHelpForums = app->actionHelpForums;
	auto &actionHomePage = app->actionHomePage;
	auto &actionImagePlot = app->actionImagePlot;
	auto &actionImageProfilesPlot = app->actionImageProfilesPlot;
	auto &actionNewLegend = app->actionNewLegend;
	auto &actionPasteSelection = app->actionPasteSelection;
	auto &actionPlot3DBars = app->actionPlot3DBars;
	auto &actionPlot3DHiddenLine = app->actionPlot3DHiddenLine;
	auto &actionPlot3DPolygons = app->actionPlot3DPolygons;
	auto &actionPlot3DScatter = app->actionPlot3DScatter;
	auto &actionPlot3DWireFrame = app->actionPlot3DWireFrame;
	auto &actionPlot3DWireSurface = app->actionPlot3DWireSurface;
	auto &actionPlotHistogram = app->actionPlotHistogram;
	auto &actionRedo = app->actionRedo;
	auto &actionShowConfigureDialog = app->actionShowConfigureDialog;
	auto &actionShowCurvesDialog = app->actionShowCurvesDialog;
	auto &actionShowExplorer = app->actionShowExplorer;
	auto &actionShowHelp = app->actionShowHelp;
	auto &actionShowLayerDialog = app->actionShowLayerDialog;
	auto &actionShowLog = app->actionShowLog;
	auto &actionShowUndoStack = app->actionShowUndoStack;
	auto &actionTechnicalSupport = app->actionTechnicalSupport;
	auto &actionTimeStamp = app->actionTimeStamp;
	auto &actionToolBars = app->actionToolBars;
	auto &actionTranslations = app->actionTranslations;
	auto &actionUndo = app->actionUndo;
	auto &analysisMenu = app->analysisMenu;
	auto &btnArrow = app->btnArrow;
	auto &btnLine = app->btnLine;
	auto &decayMenu = app->decayMenu;
	auto &edit = app->edit;
	auto &exportPlotMenu = app->exportPlotMenu;
	auto &fileMenu = app->fileMenu;
	auto &fillMenu = app->fillMenu;
	auto &filterMenu = app->filterMenu;
	auto &foldersMenu = app->foldersMenu;
	auto &format = app->format;
	auto &graphMenu = app->graphMenu;
	auto &help = app->help;
	auto &importMenu = app->importMenu;
	auto &matrixMenu = app->matrixMenu;
	auto &multiPeakMenu = app->multiPeakMenu;
	auto &newMenu = app->newMenu;
	auto &normMenu = app->normMenu;
	auto &plot2DMenu = app->plot2DMenu;
	auto &plot3DMenu = app->plot3DMenu;
	auto &plotDataMenu = app->plotDataMenu;
	auto &recent = app->recent;
	auto &recentMenuAction = app->recentMenuAction;
	auto &scriptingMenu = app->scriptingMenu;
	auto &smoothMenu = app->smoothMenu;
	auto &tableMenu = app->tableMenu;
	auto &view = app->view;
	auto &windowsMenu = app->windowsMenu;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
	auto &actionShowConsole = app->actionShowConsole;
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	app->menuBar()->setObjectName("menuBar");

	fileMenu = new QMenu(app);
	fileMenu->setObjectName("fileMenu");
	QObject::connect(fileMenu, &QMenu::aboutToShow, app, &ApplicationWindow::fileMenuAboutToShow);
	app->menuBar()->addMenu(fileMenu);

	recent = new QMenu(app);
	recentMenuAction = recent->menuAction();
	newMenu = new QMenu(app);
	newMenu->setObjectName("newMenu");
	exportPlotMenu = new QMenu(app);
	exportPlotMenu->setObjectName("exportPlotMenu");
	importMenu = new QMenu(app);
	importMenu->setObjectName("importMenu");

	edit = new QMenu(app);
	edit->setObjectName("editMenu");
	app->menuBar()->addMenu(edit);

	edit->addAction(actionUndo);
	edit->addAction(actionRedo);
	edit->addSeparator();
	edit->addAction(actionCopySelection);
	edit->addAction(actionPasteSelection);
	edit->addAction(actionClearSelection);
	edit->addSeparator();
	edit->addAction(actionDeleteFitTables);
	edit->addAction(actionClearLogInfo);
	edit->addSeparator();
	edit->addAction(actionShowConfigureDialog);

	QObject::connect(edit, &QMenu::aboutToShow, app, &ApplicationWindow::editMenuAboutToShow);

	view = new QMenu(app);
	view->setObjectName("viewMenu");
	app->menuBar()->addMenu(view);

	view->addAction(actionToolBars);
	view->addAction(actionShowExplorer);
	view->addAction(actionShowLog);
	view->addAction(actionShowUndoStack);
#ifdef SCRIPTING_CONSOLE
	view->addAction(actionShowConsole);
#endif

	scriptingMenu = new QMenu(app);
	scriptingMenu->setObjectName("scriptingMenu");
	QObject::connect(scriptingMenu, &QMenu::aboutToShow, app, &ApplicationWindow::scriptingMenuAboutToShow);
	app->menuBar()->addMenu(scriptingMenu);

	graphMenu = new QMenu(app);
	graphMenu->setObjectName("graphMenu");
	
	app->menuBar()->addMenu(graphMenu);

	graphMenu->addAction(actionShowCurvesDialog);
	graphMenu->addAction(actionAddFunctionCurve);
	graphMenu->addAction(actionAddErrorBars);
	graphMenu->addAction(actionNewLegend);
	graphMenu->addSeparator();
	graphMenu->addAction(actionAddFormula);
	graphMenu->addAction(actionAddText);
	graphMenu->addAction(btnArrow);
	graphMenu->addAction(btnLine);
	graphMenu->addAction(actionAddRectangle);
	graphMenu->addAction(actionAddEllipse);
	graphMenu->addAction(actionTimeStamp);
	graphMenu->addAction(actionAddImage);
	graphMenu->addSeparator();//layers section
	graphMenu->addAction(actionAddLayer);
	graphMenu->addAction(actionAddInsetLayer);
	graphMenu->addAction(actionAddInsetCurveLayer);
	graphMenu->addAction(actionShowLayerDialog);
	graphMenu->addAction(actionAutomaticLayout);
	graphMenu->addSeparator();
	graphMenu->addAction(actionExtractLayers);
	graphMenu->addAction(actionExtractGraphs);
	graphMenu->addSeparator();
	graphMenu->addAction(actionDeleteLayer);

	plot3DMenu = new QMenu(app);
	plot3DMenu->setObjectName("plot3DMenu");
	plot3DMenu->addAction(actionPlot3DWireFrame);
	plot3DMenu->addAction(actionPlot3DHiddenLine);
	plot3DMenu->addAction(actionPlot3DPolygons);
	plot3DMenu->addAction(actionPlot3DWireSurface);
	plot3DMenu->addSeparator();
	plot3DMenu->addAction(actionPlot3DBars);
	plot3DMenu->addAction(actionPlot3DScatter);
	plot3DMenu->addSeparator();
	plot3DMenu->addAction(actionColorMap);
	plot3DMenu->addAction(actionContourMap);
	plot3DMenu->addAction(actionGrayMap);
	plot3DMenu->addSeparator();
	plot3DMenu->addAction(actionImagePlot);
	plot3DMenu->addAction(actionImageProfilesPlot);
	plot3DMenu->addSeparator();
	plot3DMenu->addAction(actionPlotHistogram);
	app->menuBar()->addMenu(plot3DMenu);

	matrixMenu = new QMenu(app);
	matrixMenu->setObjectName("matrixMenu");
	QObject::connect(matrixMenu, &QMenu::aboutToShow, app, &ApplicationWindow::matrixMenuAboutToShow);
	app->menuBar()->addMenu(matrixMenu);

    plot2DMenu = new QMenu(app);
	plot2DMenu->setObjectName("plot2DMenu");
    QObject::connect(plot2DMenu, &QMenu::aboutToShow, app, &ApplicationWindow::plotMenuAboutToShow);
    app->menuBar()->addMenu(plot2DMenu);

    plotDataMenu = new QMenu(app);
	plotDataMenu->setObjectName("plotDataMenu");
	
    QObject::connect(plotDataMenu, &QMenu::aboutToShow, app, &ApplicationWindow::plotDataMenuAboutToShow);
    app->menuBar()->addMenu(plotDataMenu);

	normMenu = new QMenu(app);
	normMenu->setObjectName("normMenu");

	fillMenu = new QMenu();
	fillMenu->setObjectName("fillMenu");

	analysisMenu = new QMenu(app);
	analysisMenu->setObjectName("analysisMenu");
    QObject::connect(analysisMenu, &QMenu::aboutToShow, app, &ApplicationWindow::analysisMenuAboutToShow);
    app->menuBar()->addMenu(analysisMenu);

	tableMenu = new QMenu(app);
	tableMenu->setObjectName("tableMenu");
    QObject::connect(tableMenu, &QMenu::aboutToShow, app, &ApplicationWindow::tableMenuAboutToShow);
    app->menuBar()->addMenu(tableMenu);

	smoothMenu = new QMenu(app);
	smoothMenu->setObjectName("smoothMenu");

	filterMenu = new QMenu(app);
	filterMenu->setObjectName("filterMenu");

	decayMenu = new QMenu(app);
	decayMenu->setObjectName("decayMenu");

	multiPeakMenu = new QMenu(app);
	multiPeakMenu->setObjectName("multiPeakMenu");

	format = new QMenu(app);
	format->setObjectName("formatMenu");
	app->menuBar()->addMenu(format);

	windowsMenu = new QMenu(app);
	windowsMenu->setObjectName("windowsMenu");
	
	foldersMenu = new QMenu(app);

	QObject::connect(windowsMenu, &QMenu::aboutToShow, app, &ApplicationWindow::windowsMenuAboutToShow);
	QObject::connect(windowsMenu, &QMenu::triggered, app, &ApplicationWindow::windowsMenuTriggered);
	QObject::connect(foldersMenu, &QMenu::triggered, app, &ApplicationWindow::foldersMenuTriggered);
	app->menuBar()->addMenu(windowsMenu);
	

	help = new QMenu(app);
	help->setObjectName("helpMenu");
	app->menuBar()->addMenu(help);

	help->addAction(actionShowHelp);
	help->addAction(actionChooseHelpFolder);
	help->addSeparator();

	help->addAction(actionHomePage);
	help->addAction(actionCheckUpdates);
	help->addAction(actionDownloadManual);
	help->addAction(actionTranslations);
	help->addSeparator();

	help->addAction(actionTechnicalSupport);
	help->addAction(actionDonate);
	help->addAction(actionHelpForums);
	help->addAction(actionHelpBugReports);
	help->addAction(app->tr("Open Log Folder / Report a Problem..."), app, &ApplicationWindow::openLogFolder);
	help->addSeparator();
	help->addAction(actionAbout);

	QList<QMenu *> menus;
	menus << windowsMenu << view << graphMenu << fileMenu << format << edit;
	menus << help << plot2DMenu << analysisMenu;
	menus << matrixMenu << plot3DMenu << plotDataMenu << scriptingMenu;
	menus << tableMenu << newMenu << exportPlotMenu << importMenu;

	for (QMenu *m : menus)
    	QObject::connect(m, &QMenu::triggered, app, &ApplicationWindow::performCustomAction);

	app->disableActions();
}


void MenuBuilder::tableMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAddColToTable = app->actionAddColToTable;
	auto &actionAdjustColumnWidth = app->actionAdjustColumnWidth;
	auto &actionClearTable = app->actionClearTable;
	auto &actionConvertTableBinning = app->actionConvertTableBinning;
	auto &actionConvertTableDirect = app->actionConvertTableDirect;
	auto &actionConvertTableRegularXYZ = app->actionConvertTableRegularXYZ;
	auto &actionDeleteRows = app->actionDeleteRows;
	auto &actionDisregardCol = app->actionDisregardCol;
	auto &actionExtractTableData = app->actionExtractTableData;
	auto &actionGoToColumn = app->actionGoToColumn;
	auto &actionGoToRow = app->actionGoToRow;
	auto &actionHideSelectedColumns = app->actionHideSelectedColumns;
	auto &actionMoveColFirst = app->actionMoveColFirst;
	auto &actionMoveColLast = app->actionMoveColLast;
	auto &actionMoveColLeft = app->actionMoveColLeft;
	auto &actionMoveColRight = app->actionMoveColRight;
	auto &actionMoveRowDown = app->actionMoveRowDown;
	auto &actionMoveRowUp = app->actionMoveRowUp;
	auto &actionSetAscValues = app->actionSetAscValues;
	auto &actionSetLabelCol = app->actionSetLabelCol;
	auto &actionSetRandomNormalValues = app->actionSetRandomNormalValues;
	auto &actionSetRandomValues = app->actionSetRandomValues;
	auto &actionSetXCol = app->actionSetXCol;
	auto &actionSetXErrCol = app->actionSetXErrCol;
	auto &actionSetYCol = app->actionSetYCol;
	auto &actionSetYErrCol = app->actionSetYErrCol;
	auto &actionSetZCol = app->actionSetZCol;
	auto &actionShowAllColumns = app->actionShowAllColumns;
	auto &actionShowColsDialog = app->actionShowColsDialog;
	auto &actionShowColumnOptionsDialog = app->actionShowColumnOptionsDialog;
	auto &actionShowColumnValuesDialog = app->actionShowColumnValuesDialog;
	auto &actionShowRowsDialog = app->actionShowRowsDialog;
	auto &actionSwapColumns = app->actionSwapColumns;
	auto &actionTableRecalculate = app->actionTableRecalculate;
	auto &fillMenu = app->fillMenu;
	auto &tableMenu = app->tableMenu;
#ifdef HAVE_ALGLIB
	auto &actionConvertTableRandomXYZ = app->actionConvertTableRandomXYZ;
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

    tableMenu->clear();
	fillMenu->clear();

    QMenu *setAsMenu = tableMenu->addMenu(app->tr("Set Columns &As"));
	setAsMenu->addAction(actionSetXCol);
	setAsMenu->addAction(actionSetYCol);
	setAsMenu->addAction(actionSetZCol);
	setAsMenu->addSeparator();
	setAsMenu->addAction(actionSetLabelCol);
	setAsMenu->addAction(actionDisregardCol);
	setAsMenu->addSeparator();
	setAsMenu->addAction(actionSetXErrCol);
	setAsMenu->addAction(actionSetYErrCol);
	setAsMenu->addSeparator();
	setAsMenu->addAction(app->tr("&Read-only"), app, &ApplicationWindow::setReadOnlyColumns);
	setAsMenu->addAction(app->tr("Read/&Write"), app, &ApplicationWindow::setReadWriteColumns);

	tableMenu->addAction(actionShowColumnOptionsDialog);
	tableMenu->addSeparator();

	tableMenu->addAction(actionShowColumnValuesDialog);
	tableMenu->addAction(actionTableRecalculate);

	fillMenu->addAction(actionSetAscValues);
	fillMenu->addAction(actionSetRandomValues);
	fillMenu->addAction(actionSetRandomNormalValues);
	tableMenu->addMenu (fillMenu);

	tableMenu->addAction(actionClearTable);
	tableMenu->addSeparator();
	tableMenu->addAction(actionAddColToTable);
	tableMenu->addAction(actionShowColsDialog);
	tableMenu->addSeparator();
	tableMenu->addAction(actionHideSelectedColumns);
	tableMenu->addAction(actionShowAllColumns);
	tableMenu->addAction(actionAdjustColumnWidth);
	tableMenu->addSeparator();
	tableMenu->addAction(actionMoveColFirst);
	tableMenu->addAction(actionMoveColLeft);
	tableMenu->addAction(actionMoveColRight);
	tableMenu->addAction(actionMoveColLast);
	tableMenu->addAction(actionSwapColumns);
	tableMenu->addSeparator();
	tableMenu->addAction(actionShowRowsDialog);
	tableMenu->addAction(actionDeleteRows);

	QMenu *moveRowMenu = tableMenu->addMenu(app->tr("Move Row"));
	moveRowMenu->addAction(actionMoveRowUp);
	moveRowMenu->addAction(actionMoveRowDown);

	tableMenu->addSeparator();
	tableMenu->addAction(actionGoToRow);
	tableMenu->addAction(actionGoToColumn);
	tableMenu->addAction(actionExtractTableData);
	tableMenu->addSeparator();

	QMenu *convertToMatrixMenu = tableMenu->addMenu(app->tr("Convert to &Matrix"));
	convertToMatrixMenu->addAction(actionConvertTableDirect);
	convertToMatrixMenu->addAction(actionConvertTableBinning);
	convertToMatrixMenu->addAction(actionConvertTableRegularXYZ);
#ifdef HAVE_ALGLIB
	convertToMatrixMenu->addAction(actionConvertTableRandomXYZ);
#endif

    app->reloadCustomActions();

	QTimer::singleShot(0, app, &ApplicationWindow::checkRecoveryOnStartup);
}


void MenuBuilder::plotDataMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionDragCurve = app->actionDragCurve;
	auto &actionDrawPoints = app->actionDrawPoints;
	auto &actionMagnify = app->actionMagnify;
	auto &actionMagnifyHor = app->actionMagnifyHor;
	auto &actionMagnifyVert = app->actionMagnifyVert;
	auto &actionUnzoom = app->actionUnzoom;
	auto &btnCursor = app->btnCursor;
	auto &btnMovePoints = app->btnMovePoints;
	auto &btnPicker = app->btnPicker;
	auto &btnPointer = app->btnPointer;
	auto &btnRemovePoints = app->btnRemovePoints;
	auto &btnSelect = app->btnSelect;
	auto &btnZoomIn = app->btnZoomIn;
	auto &btnZoomOut = app->btnZoomOut;
	auto &plotDataMenu = app->plotDataMenu;
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

    plotDataMenu->clear();
	plotDataMenu->addAction(btnPointer);
	plotDataMenu->addSeparator();
	plotDataMenu->addAction(actionMagnify);
	plotDataMenu->addAction(actionMagnifyHor);
	plotDataMenu->addAction(actionMagnifyVert);
	plotDataMenu->addAction(btnZoomIn);
	plotDataMenu->addAction(btnZoomOut);
	plotDataMenu->addAction(actionUnzoom);
	plotDataMenu->addSeparator();
	plotDataMenu->addAction(btnCursor);
	plotDataMenu->addAction(btnSelect);
	plotDataMenu->addAction(btnPicker);
	plotDataMenu->addSeparator();
	plotDataMenu->addAction(actionDrawPoints);
	plotDataMenu->addAction(btnMovePoints);
	plotDataMenu->addAction(btnRemovePoints);
	plotDataMenu->addAction(actionDragCurve);

    app->reloadCustomActions();
}


void MenuBuilder::plotMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAddZoomPlot = app->actionAddZoomPlot;
	auto &actionBoxPlot = app->actionBoxPlot;
	auto &actionCustomLayout = app->actionCustomLayout;
	auto &actionCustomSharedAxisLayers = app->actionCustomSharedAxisLayers;
	auto &actionHorSharedAxisLayers = app->actionHorSharedAxisLayers;
	auto &actionPlot2HorizontalLayers = app->actionPlot2HorizontalLayers;
	auto &actionPlot2VerticalLayers = app->actionPlot2VerticalLayers;
	auto &actionPlot3DBars = app->actionPlot3DBars;
	auto &actionPlot3DRibbon = app->actionPlot3DRibbon;
	auto &actionPlot3DScatter = app->actionPlot3DScatter;
	auto &actionPlot3DTrajectory = app->actionPlot3DTrajectory;
	auto &actionPlot4Layers = app->actionPlot4Layers;
	auto &actionPlotArea = app->actionPlotArea;
	auto &actionPlotDoubleYAxis = app->actionPlotDoubleYAxis;
	auto &actionPlotHistogram = app->actionPlotHistogram;
	auto &actionPlotHorSteps = app->actionPlotHorSteps;
	auto &actionPlotHorizontalBars = app->actionPlotHorizontalBars;
	auto &actionPlotL = app->actionPlotL;
	auto &actionPlotLP = app->actionPlotLP;
	auto &actionPlotP = app->actionPlotP;
	auto &actionPlotPie = app->actionPlotPie;
	auto &actionPlotPolar = app->actionPlotPolar;
	auto &actionPlotSpline = app->actionPlotSpline;
	auto &actionPlotStackedHistograms = app->actionPlotStackedHistograms;
	auto &actionPlotStackedLayers = app->actionPlotStackedLayers;
	auto &actionPlotVectXYAM = app->actionPlotVectXYAM;
	auto &actionPlotVectXYXY = app->actionPlotVectXYXY;
	auto &actionPlotVertSteps = app->actionPlotVertSteps;
	auto &actionPlotVerticalBars = app->actionPlotVerticalBars;
	auto &actionPlotVerticalDropLines = app->actionPlotVerticalDropLines;
	auto &actionSharedAxesLayers = app->actionSharedAxesLayers;
	auto &actionShowPlotWizard = app->actionShowPlotWizard;
	auto &actionStackBars = app->actionStackBars;
	auto &actionStackColumns = app->actionStackColumns;
	auto &actionStackSharedAxisLayers = app->actionStackSharedAxisLayers;
	auto &actionStemPlot = app->actionStemPlot;
	auto &actionVertSharedAxisLayers = app->actionVertSharedAxisLayers;
	auto &actionWaterfallPlot = app->actionWaterfallPlot;
	auto &plot2DMenu = app->plot2DMenu;
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

	plot2DMenu->clear();

	plot2DMenu->addAction(actionShowPlotWizard);
	plot2DMenu->addSeparator();

	plot2DMenu->addAction(actionPlotL);
	plot2DMenu->addAction(actionPlotP);
	plot2DMenu->addAction(actionPlotLP);
	plot2DMenu->addAction(actionPlotPolar);

    QMenu *specialPlotMenu = plot2DMenu->addMenu (app->tr("Special Line/Symb&ol"));
	specialPlotMenu->addAction(actionPlotVerticalDropLines);
	specialPlotMenu->addAction(actionPlotSpline);
	specialPlotMenu->addAction(actionPlotVertSteps);
	specialPlotMenu->addAction(actionPlotHorSteps);
	specialPlotMenu->addSeparator();
	specialPlotMenu->addAction(actionPlotDoubleYAxis);
	specialPlotMenu->addAction(actionWaterfallPlot);
	specialPlotMenu->addAction(actionAddZoomPlot);

	plot2DMenu->addSeparator();
	plot2DMenu->addAction(actionPlotVerticalBars);
	plot2DMenu->addAction(actionPlotHorizontalBars);
	QMenu *specialBarMenu = plot2DMenu->addMenu (app->tr("Spec&ial Bar/Column"));
	specialBarMenu->addAction(actionStackBars);
	specialBarMenu->addAction(actionStackColumns);

	plot2DMenu->addAction(actionPlotArea);
	plot2DMenu->addAction(actionPlotPie);
	plot2DMenu->addAction(actionPlotVectXYXY);
	plot2DMenu->addAction(actionPlotVectXYAM);
	plot2DMenu->addSeparator();

	QMenu *statMenu = plot2DMenu->addMenu (app->tr("Statistical &Graphs"));
	statMenu->addAction(actionBoxPlot);
	statMenu->addAction(actionPlotHistogram);
	statMenu->addAction(actionPlotStackedHistograms);
	statMenu->addSeparator();
	statMenu->addAction(actionStemPlot);

    QMenu *panelsMenu = plot2DMenu->addMenu (app->tr("Pa&nel"));
	panelsMenu->addAction(actionPlot2VerticalLayers);
	panelsMenu->addAction(actionPlot2HorizontalLayers);
	panelsMenu->addAction(actionPlot4Layers);
	panelsMenu->addAction(actionPlotStackedLayers);
	panelsMenu->addAction(actionCustomLayout);

	QMenu *gridMenu = plot2DMenu->addMenu (app->tr("Shared A&xes Panel"));
	gridMenu->addAction(actionVertSharedAxisLayers);
	gridMenu->addAction(actionHorSharedAxisLayers);
	gridMenu->addAction(actionSharedAxesLayers);
	gridMenu->addAction(actionStackSharedAxisLayers);
	gridMenu->addAction(actionCustomSharedAxisLayers);

	QMenu *plot3D = plot2DMenu->addMenu (app->tr("3&D Plot"));
	plot3D->addAction(actionPlot3DRibbon);
	plot3D->addAction(actionPlot3DBars);
	plot3D->addAction(actionPlot3DScatter);
	plot3D->addAction(actionPlot3DTrajectory);

    app->reloadCustomActions();
}


void MenuBuilder::scriptingMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAddNoteTab = app->actionAddNoteTab;
	auto &actionCloseNoteTab = app->actionCloseNoteTab;
	auto &actionCustomActionDialog = app->actionCustomActionDialog;
	auto &actionDecreaseIndent = app->actionDecreaseIndent;
	auto &actionFind = app->actionFind;
	auto &actionFindNext = app->actionFindNext;
	auto &actionFindPrev = app->actionFindPrev;
	auto &actionIncreaseIndent = app->actionIncreaseIndent;
	auto &actionNoteEvaluate = app->actionNoteEvaluate;
	auto &actionNoteExecute = app->actionNoteExecute;
	auto &actionNoteExecuteAll = app->actionNoteExecuteAll;
	auto &actionNoteStop = app->actionNoteStop;
	auto &actionRenameNoteTab = app->actionRenameNoteTab;
	auto &actionReplace = app->actionReplace;
	auto &actionRestartScripting = app->actionRestartScripting;
	auto &actionScriptingLang = app->actionScriptingLang;
	auto &actionShowNoteLineNumbers = app->actionShowNoteLineNumbers;
	auto &noteTools = app->noteTools;
	auto &scriptingMenu = app->scriptingMenu;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
	auto &actionCommentSelection = app->actionCommentSelection;
	auto &actionOpenQtDesignerUi = app->actionOpenQtDesignerUi;
	auto &actionUncommentSelection = app->actionUncommentSelection;
#endif
#ifdef Q_OS_WIN
#endif

    scriptingMenu->clear();
#ifdef SCRIPTING_PYTHON
	scriptingMenu->addAction(actionScriptingLang);
	scriptingMenu->addAction(actionRestartScripting);
    scriptingMenu->addAction(actionCustomActionDialog);
    scriptingMenu->addAction(actionOpenQtDesignerUi);
#endif

	Note *note = (Note *)app->activeWindow(ApplicationWindow::NoteWindow);
    if (note){
		scriptingMenu->addSeparator();

        bool noteHasText = !note->text().isEmpty();
    	noteTools->setEnabled(noteHasText);
		if (noteHasText){
			if (scriptEnv->objectName() == QString("Python")){
				scriptingMenu->addAction(actionNoteExecute);
				scriptingMenu->addAction(actionNoteExecuteAll);
				scriptingMenu->addAction(actionNoteStop);
			}
			scriptingMenu->addAction(actionNoteEvaluate);

			#ifdef SCRIPTING_PYTHON
			if (scriptEnv->objectName() == QString("Python") && note->currentEditor() && note->currentEditor()->textCursor().hasSelection()){
				scriptingMenu->addSeparator();
				scriptingMenu->addAction(actionCommentSelection);
				scriptingMenu->addAction(actionUncommentSelection);
			}
			#endif

			scriptingMenu->addSeparator();
			scriptingMenu->addAction(actionIncreaseIndent);
			scriptingMenu->addAction(actionDecreaseIndent);
			scriptingMenu->addSeparator();
			scriptingMenu->addAction(actionFind);
			scriptingMenu->addAction(actionFindNext);
			scriptingMenu->addAction(actionFindPrev);
			scriptingMenu->addAction(actionReplace);
			scriptingMenu->addSeparator();
		}
		scriptingMenu->addAction(actionRenameNoteTab);
		scriptingMenu->addAction(actionAddNoteTab);
		if (note->tabs() > 1)
			scriptingMenu->addAction(actionCloseNoteTab);
		scriptingMenu->addSeparator();
		actionShowNoteLineNumbers->setChecked(note->hasLineNumbers());
		scriptingMenu->addAction(actionShowNoteLineNumbers);
    }

	app->reloadCustomActions();
}


void MenuBuilder::analysisMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAutoCorrelate = app->actionAutoCorrelate;
	auto &actionBandBlockFilter = app->actionBandBlockFilter;
	auto &actionBandPassFilter = app->actionBandPassFilter;
	auto &actionBaseline = app->actionBaseline;
	auto &actionChiSquareTest = app->actionChiSquareTest;
	auto &actionConvolute = app->actionConvolute;
	auto &actionCorrelate = app->actionCorrelate;
	auto &actionDeconvolute = app->actionDeconvolute;
	auto &actionDifferentiate = app->actionDifferentiate;
	auto &actionFFT = app->actionFFT;
	auto &actionFitExpGrowth = app->actionFitExpGrowth;
	auto &actionFitGauss = app->actionFitGauss;
	auto &actionFitLinear = app->actionFitLinear;
	auto &actionFitLorentz = app->actionFitLorentz;
	auto &actionFitSigmoidal = app->actionFitSigmoidal;
	auto &actionFitSlope = app->actionFitSlope;
	auto &actionFrequencyCount = app->actionFrequencyCount;
	auto &actionHighPassFilter = app->actionHighPassFilter;
	auto &actionIntegrate = app->actionIntegrate;
	auto &actionInterpolate = app->actionInterpolate;
	auto &actionLowPassFilter = app->actionLowPassFilter;
	auto &actionMatrixFFTDirect = app->actionMatrixFFTDirect;
	auto &actionMatrixFFTInverse = app->actionMatrixFFTInverse;
	auto &actionMultiPeakGauss = app->actionMultiPeakGauss;
	auto &actionMultiPeakLorentz = app->actionMultiPeakLorentz;
	auto &actionNormalizeSelection = app->actionNormalizeSelection;
	auto &actionNormalizeTable = app->actionNormalizeTable;
	auto &actionOneSampletTest = app->actionOneSampletTest;
	auto &actionShapiroWilk = app->actionShapiroWilk;
	auto &actionShowColStatistics = app->actionShowColStatistics;
	auto &actionShowExpDecay3Dialog = app->actionShowExpDecay3Dialog;
	auto &actionShowExpDecayDialog = app->actionShowExpDecayDialog;
	auto &actionShowFitDialog = app->actionShowFitDialog;
	auto &actionShowFitPolynomDialog = app->actionShowFitPolynomDialog;
	auto &actionShowIntDialog = app->actionShowIntDialog;
	auto &actionShowRowStatistics = app->actionShowRowStatistics;
	auto &actionShowTwoExpDecayDialog = app->actionShowTwoExpDecayDialog;
	auto &actionSmoothAverage = app->actionSmoothAverage;
	auto &actionSmoothFFT = app->actionSmoothFFT;
	auto &actionSmoothLowess = app->actionSmoothLowess;
	auto &actionSmoothSavGol = app->actionSmoothSavGol;
	auto &actionSortSelection = app->actionSortSelection;
	auto &actionSortTable = app->actionSortTable;
	auto &actionSubtractLine = app->actionSubtractLine;
	auto &actionSubtractReference = app->actionSubtractReference;
	auto &actionTranslateHor = app->actionTranslateHor;
	auto &actionTranslateVert = app->actionTranslateVert;
	auto &actionTwoSampletTest = app->actionTwoSampletTest;
	auto &analysisMenu = app->analysisMenu;
	auto &decayMenu = app->decayMenu;
	auto &filterMenu = app->filterMenu;
	auto &multiPeakMenu = app->multiPeakMenu;
	auto &normMenu = app->normMenu;
	auto &smoothMenu = app->smoothMenu;
#ifdef HAVE_ALGLIB
#endif
#ifdef HAVE_TAMUANOVA
	auto &actionOneWayANOVA = app->actionOneWayANOVA;
	auto &actionTwoWayANOVA = app->actionTwoWayANOVA;
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

    analysisMenu->clear();
    MdiSubWindow *w = app->activeWindow();
    if (!w) {
        return;
    }

	if (w->inherits("MultiLayer")){
        QMenu *translateMenu = analysisMenu->addMenu (app->tr("&Translate"));
        translateMenu->addAction(actionTranslateVert);
        translateMenu->addAction(actionTranslateHor);

		QMenu *subtractMenu = analysisMenu->addMenu(app->tr("S&ubtract"));
		subtractMenu->addAction(actionBaseline);
		subtractMenu->addAction(actionSubtractReference);
		subtractMenu->addAction(actionSubtractLine);

        analysisMenu->addSeparator();
        analysisMenu->addAction(actionDifferentiate);
		actionIntegrate->setText(app->tr("&Integrate") + "...");
		analysisMenu->addAction(actionIntegrate);
        analysisMenu->addAction(actionShowIntDialog);
        analysisMenu->addSeparator();

		smoothMenu->clear();
        analysisMenu->addMenu(smoothMenu);
        smoothMenu->addAction(actionSmoothSavGol);
        smoothMenu->addAction(actionSmoothAverage);
        smoothMenu->addAction(actionSmoothLowess);
        smoothMenu->addAction(actionSmoothFFT);

		filterMenu->clear();
		analysisMenu->addMenu(filterMenu);
        filterMenu->addAction(actionLowPassFilter);
        filterMenu->addAction(actionHighPassFilter);
        filterMenu->addAction(actionBandPassFilter);
        filterMenu->addAction(actionBandBlockFilter);

        analysisMenu->addSeparator();
        analysisMenu->addAction(actionInterpolate);
        analysisMenu->addAction(actionFFT);
        analysisMenu->addSeparator();
        analysisMenu->addAction(actionFitSlope);
        analysisMenu->addAction(actionFitLinear);
        analysisMenu->addAction(actionShowFitPolynomDialog);
        analysisMenu->addSeparator();

		decayMenu->clear();
		analysisMenu->addMenu(decayMenu);
        decayMenu->addAction(actionShowExpDecayDialog);
        decayMenu->addAction(actionShowTwoExpDecayDialog);
        decayMenu->addAction(actionShowExpDecay3Dialog);

        analysisMenu->addAction(actionFitExpGrowth);
        analysisMenu->addAction(actionFitSigmoidal);
        analysisMenu->addAction(actionFitGauss);
        analysisMenu->addAction(actionFitLorentz);

		multiPeakMenu->clear();
		analysisMenu->addMenu(multiPeakMenu);
        multiPeakMenu->addAction(actionMultiPeakGauss);
        multiPeakMenu->addAction(actionMultiPeakLorentz);
        analysisMenu->addSeparator();
        analysisMenu->addAction(actionShowFitDialog);
	} else if (w->inherits("Matrix")){
		actionIntegrate->setText(app->tr("&Integrate"));
        analysisMenu->addAction(actionIntegrate);
        analysisMenu->addSeparator();
        analysisMenu->addAction(actionFFT);
        analysisMenu->addAction(actionMatrixFFTDirect);
        analysisMenu->addAction(actionMatrixFFTInverse);
	} else if (w->inherits("Table")){
		QMenu *statsMenu = analysisMenu->addMenu (app->tr("Descriptive S&tatistics"));
		statsMenu->addAction(actionShowColStatistics);
		statsMenu->addAction(actionShowRowStatistics);
		statsMenu->addAction(actionFrequencyCount);
		statsMenu->addAction(actionShapiroWilk);

		QMenu *tTestMenu = analysisMenu->addMenu (app->tr("&Hypothesis Testing"));
		tTestMenu->addAction(actionOneSampletTest);
		tTestMenu->addAction(actionTwoSampletTest);
		tTestMenu->addAction(actionChiSquareTest);

#ifdef HAVE_TAMUANOVA
		QMenu *anovaMenu = analysisMenu->addMenu (app->tr("ANO&VA"));
		anovaMenu->addAction(actionOneWayANOVA);
		anovaMenu->addAction(actionTwoWayANOVA);
#endif
        analysisMenu->addSeparator();

		Table *table = qobject_cast<Table *>(w);
		bool columns = table ? table->selectedColumns().count() > 1 : false;
		QString sortMenuText = columns ? "&" + app->tr("Sort Columns") : app->tr("Sort Colu&mn");
		QMenu *sortMenu = analysisMenu->addMenu(sortMenuText);
		if (table){
			sortMenu->addAction(QIcon(":/sort_ascending.png"), app->tr("&Ascending"), table, &Table::sortColAsc);
			sortMenu->addAction(QIcon(":/sort_descending.png"), app->tr("&Descending"), table, &Table::sortColDesc);
		}
		if (columns)
			sortMenu->addAction(actionSortSelection);
		analysisMenu->addMenu(sortMenu);

        analysisMenu->addAction(actionSortTable);

		normMenu->clear();
		analysisMenu->addMenu(normMenu);
        normMenu->addAction(actionNormalizeSelection);
        normMenu->addAction(actionNormalizeTable);

        analysisMenu->addSeparator();
		analysisMenu->addAction(actionDifferentiate);
		actionIntegrate->setText(app->tr("&Integrate") + "...");
		analysisMenu->addAction(actionIntegrate);
        analysisMenu->addSeparator();
		analysisMenu->addAction(actionFFT);
		analysisMenu->addSeparator();
        analysisMenu->addAction(actionCorrelate);
        analysisMenu->addAction(actionAutoCorrelate);
        analysisMenu->addSeparator();
        analysisMenu->addAction(actionConvolute);
        analysisMenu->addAction(actionDeconvolute);
        analysisMenu->addSeparator();
        analysisMenu->addAction(actionFitSlope);
		analysisMenu->addAction(actionFitLinear);
        analysisMenu->addAction(actionShowFitDialog);
	}
    app->reloadCustomActions();
}


void MenuBuilder::matrixMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionConvertMatrixDirect = app->actionConvertMatrixDirect;
	auto &actionConvertMatrixXYZ = app->actionConvertMatrixXYZ;
	auto &actionConvertMatrixYXZ = app->actionConvertMatrixYXZ;
	auto &actionFlipMatrixHorizontally = app->actionFlipMatrixHorizontally;
	auto &actionFlipMatrixVertically = app->actionFlipMatrixVertically;
	auto &actionGoToColumn = app->actionGoToColumn;
	auto &actionGoToRow = app->actionGoToRow;
	auto &actionInvertMatrix = app->actionInvertMatrix;
	auto &actionMatrixColumnRow = app->actionMatrixColumnRow;
	auto &actionMatrixCustomScale = app->actionMatrixCustomScale;
	auto &actionMatrixDefaultScale = app->actionMatrixDefaultScale;
	auto &actionMatrixDeterminant = app->actionMatrixDeterminant;
	auto &actionMatrixGrayScale = app->actionMatrixGrayScale;
	auto &actionMatrixRainbowScale = app->actionMatrixRainbowScale;
	auto &actionMatrixXY = app->actionMatrixXY;
	auto &actionRotateMatrix = app->actionRotateMatrix;
	auto &actionRotateMatrixMinus = app->actionRotateMatrixMinus;
	auto &actionSetMatrixDimensions = app->actionSetMatrixDimensions;
	auto &actionSetMatrixProperties = app->actionSetMatrixProperties;
	auto &actionSetMatrixValues = app->actionSetMatrixValues;
	auto &actionTableRecalculate = app->actionTableRecalculate;
	auto &actionTransposeMatrix = app->actionTransposeMatrix;
	auto &actionViewMatrix = app->actionViewMatrix;
	auto &actionViewMatrixImage = app->actionViewMatrixImage;
	auto &matrixMenu = app->matrixMenu;
#ifdef HAVE_ALGLIB
	auto &actionExpandMatrix = app->actionExpandMatrix;
	auto &actionShrinkMatrix = app->actionShrinkMatrix;
	auto &actionSmoothMatrix = app->actionSmoothMatrix;
#endif
#ifdef HAVE_TAMUANOVA
#endif
#ifdef SCRIPTING_CONSOLE
#endif
#ifdef SCRIPTING_PYTHON
#endif
#ifdef Q_OS_WIN
#endif

	matrixMenu->clear();
	matrixMenu->addAction(actionSetMatrixProperties);
	matrixMenu->addAction(actionSetMatrixDimensions);
	matrixMenu->addSeparator();
	matrixMenu->addAction(actionSetMatrixValues);
	matrixMenu->addAction(actionTableRecalculate);
	matrixMenu->addSeparator();
	matrixMenu->addAction(actionRotateMatrix);
	matrixMenu->addAction(actionRotateMatrixMinus);
	matrixMenu->addAction(actionFlipMatrixVertically);
	matrixMenu->addAction(actionFlipMatrixHorizontally);
	matrixMenu->addSeparator();
#ifdef HAVE_ALGLIB
	matrixMenu->addAction(actionExpandMatrix);
	matrixMenu->addAction(actionShrinkMatrix);
	matrixMenu->addAction(actionSmoothMatrix);
	matrixMenu->addSeparator();
#endif
	matrixMenu->addAction(actionTransposeMatrix);
	matrixMenu->addAction(actionInvertMatrix);
	matrixMenu->addAction(actionMatrixDeterminant);
	matrixMenu->addSeparator();
	matrixMenu->addAction(actionGoToRow);
	matrixMenu->addAction(actionGoToColumn);
	matrixMenu->addSeparator();
	QMenu *matrixViewMenu = matrixMenu->addMenu (app->tr("Vie&w"));
	matrixViewMenu->addAction(actionViewMatrixImage);
	matrixViewMenu->addAction(actionViewMatrix);
    QMenu *matrixPaletteMenu = matrixMenu->addMenu (app->tr("&Palette"));
	matrixPaletteMenu->addAction(actionMatrixDefaultScale);
	matrixPaletteMenu->addAction(actionMatrixGrayScale);
	matrixPaletteMenu->addAction(actionMatrixRainbowScale);
	matrixPaletteMenu->addAction(actionMatrixCustomScale);
	matrixMenu->addSeparator();
	matrixMenu->addAction(actionMatrixColumnRow);
    matrixMenu->addAction(actionMatrixXY);
	matrixMenu->addSeparator();
	QMenu *convertToTableMenu = matrixMenu->addMenu (app->tr("&Convert to Spreadsheet"));
	convertToTableMenu->addAction(actionConvertMatrixDirect);
	convertToTableMenu->addAction(actionConvertMatrixXYZ);
	convertToTableMenu->addAction(actionConvertMatrixYXZ);

	Matrix* m = (Matrix*)app->activeWindow(ApplicationWindow::MatrixWindow);
	if (!m)
		return;

	bool empty = m->isEmpty();
	actionFlipMatrixVertically->setDisabled(empty);
	actionFlipMatrixHorizontally->setDisabled(empty);
#ifdef HAVE_ALGLIB
	actionSmoothMatrix->setDisabled(empty);
#endif
	actionInvertMatrix->setDisabled(empty);
	actionMatrixDeterminant->setDisabled(empty);
	convertToTableMenu->setDisabled(empty);

	actionViewMatrixImage->setChecked(m->viewType() == Matrix::ImageView);
	actionViewMatrix->setChecked(m->viewType() == Matrix::TableView);
	actionMatrixColumnRow->setChecked(m->headerViewType() == Matrix::ColumnRow);
	actionMatrixColumnRow->setEnabled(m->viewType() == Matrix::TableView);
	actionMatrixXY->setChecked(m->headerViewType() == Matrix::XY);
	actionMatrixXY->setEnabled(m->viewType() == Matrix::TableView);

	actionMatrixDefaultScale->setChecked(m->colorMapType() == Matrix::Default);
    actionMatrixGrayScale->setChecked(m->colorMapType() == Matrix::GrayScale);
	actionMatrixRainbowScale->setChecked(m->colorMapType() == Matrix::Rainbow);
	actionMatrixCustomScale->setChecked(m->colorMapType() == Matrix::Custom);

    app->reloadCustomActions();
}


void MenuBuilder::fileMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionAppendProject = app->actionAppendProject;
	auto &actionCloseAllWindows = app->actionCloseAllWindows;
	auto &actionCloseProject = app->actionCloseProject;
	auto &actionExportAllGraphs = app->actionExportAllGraphs;
	auto &actionExportExcel = app->actionExportExcel;
	auto &actionExportGraph = app->actionExportGraph;
	auto &actionExportLayer = app->actionExportLayer;
	auto &actionExportMatrix = app->actionExportMatrix;
	auto &actionExportOds = app->actionExportOds;
	auto &actionExportPDF = app->actionExportPDF;
	auto &actionImportDatabase = app->actionImportDatabase;
	auto &actionImportImage = app->actionImportImage;
	auto &actionImportSound = app->actionImportSound;
	auto &actionLoad = app->actionLoad;
	auto &actionLoadImage = app->actionLoadImage;
	auto &actionNewFolder = app->actionNewFolder;
	auto &actionNewFunctionPlot = app->actionNewFunctionPlot;
	auto &actionNewGraph = app->actionNewGraph;
	auto &actionNewMatrix = app->actionNewMatrix;
	auto &actionNewNote = app->actionNewNote;
	auto &actionNewProject = app->actionNewProject;
	auto &actionNewSurfacePlot = app->actionNewSurfacePlot;
	auto &actionNewTable = app->actionNewTable;
	auto &actionOpen = app->actionOpen;
	auto &actionOpenExcel = app->actionOpenExcel;
	auto &actionOpenOds = app->actionOpenOds;
	auto &actionOpenTemplate = app->actionOpenTemplate;
	auto &actionPresentationODF = app->actionPresentationODF;
	auto &actionPrint = app->actionPrint;
	auto &actionPrintAllPlots = app->actionPrintAllPlots;
	auto &actionPrintPreview = app->actionPrintPreview;
	auto &actionSaveProject = app->actionSaveProject;
	auto &actionSaveProjectAs = app->actionSaveProjectAs;
	auto &actionSaveTemplate = app->actionSaveTemplate;
	auto &actionSaveWindow = app->actionSaveWindow;
	auto &actionShowExportASCIIDialog = app->actionShowExportASCIIDialog;
	auto &exportPlotMenu = app->exportPlotMenu;
	auto &fileMenu = app->fileMenu;
	auto &importMenu = app->importMenu;
	auto &newMenu = app->newMenu;
	auto &recent = app->recent;
	auto &recentMenuAction = app->recentMenuAction;
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

	if (fileMenu)
		fileMenu->clear();
	if (newMenu)
		newMenu->clear();
	if (exportPlotMenu)
		exportPlotMenu->clear();
	if (importMenu)
		importMenu->clear();

	fileMenu->addMenu(newMenu);
	newMenu->addAction(actionNewProject);
    newMenu->addAction(actionNewFolder);
	newMenu->addAction(actionNewTable);
	newMenu->addAction(actionNewMatrix);
	newMenu->addAction(actionNewNote);
	newMenu->addAction(actionNewGraph);
	newMenu->addAction(actionNewFunctionPlot);
	newMenu->addAction(actionNewSurfacePlot);
	fileMenu->addAction(actionOpen);
	fileMenu->addAction(actionOpenExcel);
	fileMenu->addAction(actionOpenOds);
	fileMenu->addAction(actionLoadImage);
	fileMenu->addAction(actionAppendProject);
	recentMenuAction = fileMenu->addMenu(recent);
	recentMenuAction->setText(app->tr("&Recent Projects"));
	fileMenu->addAction(actionCloseProject);
	fileMenu->addSeparator();
	fileMenu->addAction(actionSaveProject);
	fileMenu->addAction(actionSaveProjectAs);
	fileMenu->addSeparator();
	fileMenu->addAction(actionSaveWindow);
	fileMenu->addAction(actionOpenTemplate);
	fileMenu->addAction(actionSaveTemplate);
	fileMenu->addSeparator();
	fileMenu->addAction(actionPrint);
	fileMenu->addAction(actionPrintPreview);
	fileMenu->addAction(actionPrintAllPlots);
	fileMenu->addSeparator();

	MdiSubWindow *w = app->activeWindow();
	if (w){
		if (w->inherits("MultiLayer") || w->inherits("Graph3D") || w->inherits("PolarGraph")){
			fileMenu->addMenu (exportPlotMenu);
			if (qobject_cast<MultiLayer*>(w))
				exportPlotMenu->addAction(actionExportLayer);
			exportPlotMenu->addAction(actionExportGraph);
			exportPlotMenu->addAction(actionExportAllGraphs);
			exportPlotMenu->addAction(actionPresentationODF);
		} else if (w->inherits("Table") || w->inherits("Matrix")){
			QMenu *exportMenu = fileMenu->addMenu(app->tr("Export"));
			exportMenu->addAction(actionShowExportASCIIDialog);
			exportMenu->addAction(actionExportExcel);
			exportMenu->addAction(actionExportOds);
			exportMenu->addAction(actionExportPDF);
			if (w->inherits("Matrix"))
				exportMenu->addAction(actionExportMatrix);
		}
	}

	fileMenu->addMenu(importMenu);
	importMenu->addAction(actionLoad);
	importMenu->addAction(actionImportSound);
	importMenu->addAction(actionImportImage);
	importMenu->addAction(actionImportDatabase);

	fileMenu->addSeparator();
	fileMenu->addAction(actionCloseAllWindows);

	app->reloadCustomActions();
}

void MenuBuilder::editMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
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

	app->updateUndoRedoActions();
}


void MenuBuilder::windowsMenuAboutToShow(ApplicationWindow *app)
{
	if (!app) return;
	auto scriptEnv = app->scriptingEnv();
	auto &actionCloseWindow = app->actionCloseWindow;
	auto &actionCopyWindow = app->actionCopyWindow;
	auto &actionFindWindow = app->actionFindWindow;
	auto &actionHideActiveWindow = app->actionHideActiveWindow;
	auto &actionNextWindow = app->actionNextWindow;
	auto &actionPrevWindow = app->actionPrevWindow;
	auto &actionRename = app->actionRename;
	auto &actionResizeActiveWindow = app->actionResizeActiveWindow;
	auto &actionShowScriptWindow = app->actionShowScriptWindow;
	auto &current_folder = app->current_folder;
	auto &d_workspace = app->d_workspace;
	auto &foldersMenu = app->foldersMenu;
	auto &windowsMenu = app->windowsMenu;
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

	windowsMenu->clear();
	foldersMenu->clear();

	int folder_param = 0;
	Folder *f = app->projectFolder();
	while (f){
		QAction *a;
		if (folder_param < 9)
			a = foldersMenu->addAction("&" + QString::number(folder_param+1) + " " + f->path());
		else
			a = foldersMenu->addAction(f->path());

		a->setData(folder_param);
		folder_param++;
		
		a->setChecked(f == current_folder);

		f = f->folderBelow();
	}

	windowsMenu->addMenu(foldersMenu)->setText(app->tr("&Folders"));
	windowsMenu->addSeparator();

	QList<MdiSubWindow *> windows = current_folder->windowsList();
	int n = int(windows.count());
	if (!n){
		#ifdef SCRIPTING_PYTHON
			windowsMenu->addAction(actionShowScriptWindow);
		#endif
		return;
	}

	windowsMenu->addAction(app->tr("&Cascade"), app, &ApplicationWindow::cascade);
	windowsMenu->addAction(app->tr("&Tile"), d_workspace, &QMdiArea::tileSubWindows);
	windowsMenu->addSeparator();
	windowsMenu->addAction(actionNextWindow);
	windowsMenu->addAction(actionPrevWindow);
	windowsMenu->addSeparator();
	windowsMenu->addAction(actionFindWindow);
	windowsMenu->addAction(actionRename);
	windowsMenu->addAction(actionCopyWindow);
	windowsMenu->addSeparator();
#ifdef SCRIPTING_PYTHON
	windowsMenu->addAction(actionShowScriptWindow);
	windowsMenu->addSeparator();
#endif

	windowsMenu->addAction(actionResizeActiveWindow);
	windowsMenu->addAction(actionHideActiveWindow);
	windowsMenu->addAction(actionCloseWindow);

	if (n > 0)
		windowsMenu->addSeparator();

	bool moreWindows = (n >= 10);
	if (moreWindows)
		n = 9;

	for (int i = 0; i < n; ++i){
		MdiSubWindow *w = windows.at(i);
		if (!w)
			continue;

		QAction *a = windowsMenu->addAction("&" + QString::number(i + 1) + " " + w->windowTitle());
		a->setData(i);
		
		a->setChecked(current_folder->activeWindow() == w);
	}

	if (moreWindows){
		windowsMenu->addSeparator();
		windowsMenu->addAction(app->tr("More windows..."), app, &ApplicationWindow::showMoreWindows);
	}

	app->reloadCustomActions();
}

