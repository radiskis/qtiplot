/***************************************************************************
File                 : ApplicationWindow.cpp
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2004 - 2012 by Ion Vasilief,
					   (C) 2006 - June 2007 Tilman Hoener zu Siederdissen, Knut Franke
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : QtiPlot's main window
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
#include "globals.h"
#include "ApplicationWindow.h"
#include "AnalysisController.h"
#include "ExportManager.h"
#include <QtiPlotApplication.h>
#include "Tracked.h"
#include "Logger.h"
#include "CrashHandler.h"

#include <qwt_global.h>
#include <qwt3d_global.h>
#include <gsl/gsl_version.h>
#include <muParser.h>
#include <zlib.h>

#ifdef SCRIPTING_PYTHON
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
#endif

#include "PlotWizard.h"
#include "ConfigDialog.h"
#include "RenameWindowDialog.h"
#include "ImportASCIIDialog.h"
#include "Folder.h"
#include "FindDialog.h"
#include "OpenProjectDialog.h"
#include "CustomActionDialog.h"
#include "MdiSubWindow.h"
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QStringConverter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPageLayout>
#include <QPageSize>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QDropEvent>
#include "qwt3d_coordsys.h"
#undef SOLID
#undef DASH

#include <SelectionMoveResizer.h>
#include <SymbolBox.h>
#include <ColorBox.h>
#include <ColorButton.h>
#include <PenStyleBox.h>
#include <PatternBox.h>
#include <SymbolDialog.h>
#include <ExportDialog.h>
#include <SetColValuesDialog.h>
#include <ExtractDataDialog.h>
#include <TableDialog.h>
#include <TableStatistics.h>
#include <Fit.h>
#include "PolarGraph.h"
#include <MultiPeakFit.h>
#include <PolynomialFit.h>
#include <SigmoidalFit.h>
#include <LogisticFit.h>
#include <NonLinearFit.h>
#include <Integration.h>
#include <IntegrationDialog.h>
#include <Differentiation.h>
#include <SmoothFilter.h>
#include <FFTFilter.h>
#include <Convolution.h>
#include <Correlation.h>
#include <ExpDecayDialog.h>
#include <PolynomFitDialog.h>
#include <FitDialog.h>
#include <IntDialog.h>
#include <InterpolationDialog.h>
#include <SmoothCurveDialog.h>
#include <FilterDialog.h>
#include <FFTDialog.h>
#include <FrequencyCountDialog.h>
#include <SubtractDataDialog.h>
#include <BaselineDialog.h>
#include <ErrorBarsCurve.h>
#include <LegendWidget.h>
#include <TexWidget.h>
#include <ArrowMarker.h>
#include <ImageWidget.h>
#include <Grid.h>
#include <ScaleDraw.h>
#include <ScaleEngine.h>
#include <QwtHistogram.h>
#include <TranslateCurveTool.h>
#include <LinearColorMap.h>
#include <PolarGraph.h>
#include <FunctionCurve.h>
#include <PieCurve.h>
#include <EllipseWidget.h>
#include <RectangleWidget.h>
#include <Spectrogram.h>
#include <EnrichmentDialog.h>
#include <TextEditor.h>
#include <CurvesDialog.h>
#include <PlotDialog.h>
#include <AxesDialog.h>
#include <LineDialog.h>
#include <ErrDialog.h>
#include <FunctionDialog.h>
#include <AssociationsDialog.h>
#include <ImageExportDialog.h>
#include <CurveRangeDialog.h>
#include <LayerDialog.h>
#include <TextDialog.h>
// TODO: move tool-specific code to an extension manager
#include <ScreenPickerTool.h>
#include <DataPickerTool.h>
#include <MultiPeakFitTool.h>
#include <LineProfileTool.h>
#include <RangeSelectorTool.h>
#include <PlotToolInterface.h>
#include <SubtractLineTool.h>
#include <AddWidgetTool.h>
#include <SurfaceDialog.h>
#include <Graph3D.h>
#include <Plot3DDialog.h>
#include <ColorMapDialog.h>
#include <MatrixDialog.h>
#include <MatrixSizeDialog.h>
#include <MatrixValuesDialog.h>
#include <MatrixModel.h>
#include <MatrixCommand.h>
#include <Note.h>
#include <ScriptingLangDialog.h>
#include <ScriptWindow.h>
#include <ScriptUndoScope.h>
#include <PythonSyntaxHighlighter.h>
#include <CreateBinMatrixDialog.h>
#include <StudentTestDialog.h>
#include <ImportExportPlugin.h>
#include <ExcelFileConverter.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include <string>


#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_magnifier.h>
#include <qwt_symbol.h>

#include <QHideEvent>
#include <QPluginLoader>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPixmapCache>
#include <QMenuBar>
#include <QClipboard>
#include <QTranslator>
#include <QSplitter>
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QtPrintSupport/QPrinter>
#include <QActionGroup>
#include <QAction>
#include <QToolBar>
#include <QKeySequence>
#include <QImageReader>
#include <QImageWriter>
#include <QDateTime>
#include <QShortcut>
#include <QDockWidget>
#include <QTextStream>
#include <QVarLengthArray>
#include <QList>
#include <QUrl>
// #include <QAssistantClient>
#include <QDesktopServices>
#include <QFontComboBox>
#include <QSpinBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QUndoStack>
#include <QUndoGroup>
#include <QUndoView>
#include <QCompleter>
#include <QStringListModel>
#include <QNetworkProxy>
#include <QHostInfo>
#include <QTextDocumentWriter>
#include <QToolButton>
#if defined(Q_OS_WIN) && defined(QT_AXCONTAINER_LIB)
	#include <QAxObject>
#endif

#include <zlib.h>
#include <iostream>

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#ifdef HAVE_ALGLIB
	#include "GriddingDialog.h"
	#include "MatrixResamplingDialog.h"
#endif

#include "AnovaDialog.h"

using namespace Qwt3D;

extern "C"
{
void file_compress(char  *, char  *) {}
void file_uncompress(char  *) {}
}

using namespace std;

ApplicationWindow::ApplicationWindow(bool factorySettings)
: QMainWindow(), scripted(ScriptingLangManager::newEnv(this)), d_app_settings(new ApplicationSettings(this)), d_project_manager(new ProjectManager(this)), d_action_manager(new ActionManager(this)), d_plot_controller_2d(new PlotController2D(this)), d_plot_controller_3d(new PlotController3D(this)), d_analysis_controller(new AnalysisController(this)), d_export_manager(new ExportManager(this))
{
	setAttribute(Qt::WA_DeleteOnClose);
	init(factorySettings);
	((QtiPlotApplication *)QCoreApplication::instance ())->append(this);
}

void ApplicationWindow::init(bool factorySettings)
{
	projectname = "untitled";
	setWindowTitle(tr("QtiPlot - untitled"));
	setObjectName(tr("QtiPlot"));
	setDefaultOptions();
	QPixmapCache::setCacheLimit(20*QPixmapCache::cacheLimit ());

	tablesDepend = new QMenu(this);

	explorerWindow = new QDockWidget( this );
	explorerWindow->setWindowTitle(tr("Project Explorer"));
	explorerWindow->setObjectName("explorerWindow"); // this is needed for QMainWindow::restoreState()
	explorerWindow->setMinimumHeight(150);
	addDockWidget( Qt::BottomDockWidgetArea, explorerWindow );

	actionSaveProject = nullptr;
	folders = new FolderListView(this);
	folders->setHeaderLabels( QStringList() << tr("Folder") );
	folders->setRootIsDecorated( true );
    folders->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	folders->header()->hide();
	folders->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(folders, &FolderListView::contextMenuRequested,
			this, [this](QTreeWidgetItem *it, const QPoint &p){showFolderPopupMenu(it, p, true);});
	connect(folders, &FolderListView::dragItems,
			this, &ApplicationWindow::dragFolderItems);
	connect(folders, &FolderListView::dropItems,
			this, &ApplicationWindow::dropFolderItems);
	connect(folders, &FolderListView::renameItem,
			this, [this](QTreeWidgetItem *it){startRenameFolder(it);});
	connect(folders, &FolderListView::addFolderItem, this, [this]{addFolder();});
	connect(folders, &FolderListView::deleteSelection, this, &ApplicationWindow::deleteSelectedItems);

	current_folder = new Folder( 0, tr("UNTITLED"));
	FolderListItem *fli = new FolderListItem(folders, current_folder);
	current_folder->setFolderListItem(fli);
	fli->setOpen( true );

	connect(folders, &QTreeWidget::currentItemChanged,
			this, &ApplicationWindow::folderItemChanged);
	connect(folders, &FolderListView::itemRenamed,
			this, &ApplicationWindow::renameFolder);

	lv = new FolderListView();
    QStringList lvHeaders;
	lvHeaders << tr("Name") << tr("Type") << tr("View") << tr("Created") << tr("Label");
    lv->setHeaderLabels(lvHeaders);

	lv->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	lv->setMinimumHeight(80);
	lv->setSelectionMode(QAbstractItemView::ExtendedSelection);

	explorerSplitter = new QSplitter(Qt::Horizontal, explorerWindow);
	explorerSplitter->addWidget(folders);
	explorerSplitter->addWidget(lv);
	explorerWindow->setWidget(explorerSplitter);

	QList<int> splitterSizes;
	explorerSplitter->setSizes( splitterSizes << 45 << 45);
	explorerWindow->hide();

	logWindow = new QDockWidget(this);
	logWindow->setObjectName("logWindow"); // this is needed for QMainWindow::restoreState()
	logWindow->setWindowTitle(tr("Results Log"));
	addDockWidget( Qt::TopDockWidgetArea, logWindow );

	results=new QTextEdit(logWindow);
	results->setReadOnly (true);

	logWindow->setWidget(results);
	logWindow->hide();

#ifdef SCRIPTING_CONSOLE
	consoleWindow = new QDockWidget(this);
	consoleWindow->setObjectName("consoleWindow"); // this is needed for QMainWindow::restoreState()
	consoleWindow->setWindowTitle(tr("Scripting Console"));
	addDockWidget( Qt::TopDockWidgetArea, consoleWindow );
	console = new QTextEdit(consoleWindow);
	console->setReadOnly(true);
	consoleWindow->setWidget(console);
	consoleWindow->hide();
#endif

	undoStackWindow = new QDockWidget(this);
	undoStackWindow->setObjectName("undoStackWindow"); // this is needed for QMainWindow::restoreState()
	undoStackWindow->setWindowTitle(tr("Undo Stack"));
	addDockWidget(Qt::RightDockWidgetArea, undoStackWindow);

	d_undo_group = new QUndoGroup(this);

	d_undo_view = new QUndoView(undoStackWindow);
	d_undo_view->setCleanIcon(QIcon(":/filesave.png"));
	undoStackWindow->setWidget(d_undo_view);
	undoStackWindow->hide();

	// Needs to be done after initialization of dock windows,
	// because we now use QDockWidget::toggleViewAction()
	createActions();
	initToolBars();
	initMainMenu();

	d_workspace = new QMdiArea();
	d_workspace->setOption(QMdiArea::DontMaximizeSubWindowOnActivation);
	d_workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	d_workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	d_workspace->setActivationOrder(QMdiArea::ActivationHistoryOrder);
	setCentralWidget(d_workspace);

	setAcceptDrops(true);

	hiddenWindows = new QList<QWidget*>();

	scriptWindow = 0;
    d_text_editor = nullptr;

	d_default_2D_grid = new Grid();

	renamedTables = QStringList();
	if (!factorySettings)
		readSettings();
	createLanguagesList();
	insertTranslatedStrings();
	disableToolbars();

	assistant = 0;
	// new QAssistantClient( QString(), this );

	connect(tablesDepend, &QMenu::triggered, this, [this](QAction *act){showTable(act);});

	connect(actionNextWindow, &QAction::triggered, d_workspace, &QMdiArea::activateNextSubWindow);
	connect(actionPrevWindow, &QAction::triggered, d_workspace, &QMdiArea::activatePreviousSubWindow);

	connect(this, &ApplicationWindow::modified, this, [this]{modifiedProject();});
        connect(d_workspace, &QMdiArea::subWindowActivated, this, &ApplicationWindow::windowActivated);
        connect(lv, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item){maximizeWindow(item);});
        connect(lv, &QTreeWidget::itemDoubleClicked, this, &ApplicationWindow::folderItemDoubleClicked);
	connect(lv, &FolderListView::contextMenuRequested,
			this, &ApplicationWindow::showWindowPopupMenu);
	connect(lv, &FolderListView::dragItems,
			this, &ApplicationWindow::dragFolderItems);
	connect(lv, &FolderListView::dropItems,
			this, &ApplicationWindow::dropFolderItems);
	connect(lv, &FolderListView::renameItem, this, [this](QTreeWidgetItem *it){startRenameFolder(it);});
	connect(lv, &FolderListView::addFolderItem, this, [this]{addFolder();});
	connect(lv, &FolderListView::deleteSelection, this, &ApplicationWindow::deleteSelectedItems);
	connect(lv, &FolderListView::itemRenamed, this, [this](QTreeWidgetItem *item, int col, const QString &s){renameWindow(item, col, s);});

	connect(scriptEnv, &ScriptingEnv::error,
			this, &ApplicationWindow::scriptError);
	connect(scriptEnv, &ScriptingEnv::print, this, &ApplicationWindow::scriptPrint);

	connect(recent, &QMenu::triggered, this, &ApplicationWindow::openRecentProject);
	connect(explorerWindow, &QDockWidget::dockLocationChanged, this, &ApplicationWindow::updateExplorerWindowLayout);

	// this has to be done after connecting scriptEnv
	scriptEnv->initialize();

	//apply user settings
	updateAppFonts();
	setScriptingLanguage(defaultScriptingLang);
	setAppColors(workspaceColor, panelsColor, panelsTextColor, true);

	loadCustomActions();
	initCompleter();
	loadPlugins();

#ifdef SCRIPTING_PYTHON
	if (defaultScriptingLang == QString("Python"))
		executeStartupScripts();
#endif
}

void ApplicationWindow::updateExplorerWindowLayout(Qt::DockWidgetArea area)
{
	switch(area){
		case Qt::LeftDockWidgetArea:
		case Qt::RightDockWidgetArea:
			explorerSplitter->setOrientation(Qt::Vertical);
		break;

		case Qt::TopDockWidgetArea:
		case Qt::BottomDockWidgetArea:
		case Qt::NoDockWidgetArea:
			explorerSplitter->setOrientation(Qt::Horizontal);
		break;

		default:
			break;
	}
}

void ApplicationWindow::initWindow()
{
	switch(d_init_window_type){
		case TableWindow:
			newTable();
		break;
		case MatrixWindow:
			newMatrix();
		break;
		case MultiLayerWindow:
			newGraph();
		break;
		case NoteWindow:
			newNote();
		break;
		default:
			break;
	}
	savedProject();
}

void ApplicationWindow::setDefaultOptions()
{
	d_fft_norm_amp = false;
	d_fft_shift_res = true;
	d_fft_power2 = true;

	d_int_sort_data = false;
	d_int_show_plot = true;
	d_int_results_table = true;

	d_show_empty_cell_gap = true;
	d_stats_significance_level = 0.05;
	d_stats_result_table = false;
	d_stats_result_log = true;
	d_stats_result_notes = false;
	d_descriptive_stats = true;
	d_stats_confidence = true;
	d_stats_power = true;
	d_stats_output = true;

	d_confirm_modif_2D_points = true;
	d_ask_web_connection = true;
	d_open_last_project = false;
	d_force_muParser = true;
	d_indexed_colors = ColorBox::defaultColors();
	d_indexed_color_names = ColorBox::defaultColorNames();

	d_symbols_list = SymbolBox::defaultSymbols();

	d_latex_compiler = MathTran;
	d_mdi_windows_area = true;
	d_open_project_filter = QString();//tr("QtiPlot project") + " (*.qti)";

	d_comment_highlight_color = Qt::red;
	d_class_highlight_color = Qt::darkMagenta;
	d_numeric_highlight_color = Qt::darkGreen;
	d_keyword_highlight_color = Qt::darkBlue;
	d_function_highlight_color = Qt::blue;
	d_quotation_highlight_color = Qt::darkYellow;

    d_notes_tab_length = 20;
    d_completer = nullptr;
    d_completion = true;
    d_note_line_numbers = true;

	d_auto_update_table_values = true;
	d_show_table_paste_dialog = true;
	d_active_window = nullptr;
    d_matrix_undo_stack_size = 10;
    d_table_undo_stack_size = 1000;
    d_undo_memory_budget_mb = 64;

	d_opening_file = false;
	d_is_appending_file = false;
    d_in_place_editing = true;

	d_matrix_tool_bar = true;
	d_file_tool_bar = true;
	d_table_tool_bar = true;
	d_column_tool_bar = true;
	d_edit_tool_bar = true;
	d_notes_tool_bar = true;
	d_plot_tool_bar = true;
	d_plot3D_tool_bar = true;
	d_display_tool_bar = false;
	d_format_tool_bar = true;

	appStyle = qApp->style()->objectName();
	d_app_rect = QRect();

	lastCopiedLayer = 0;
	d_enrichement_copy = nullptr;
	d_arrow_copy = nullptr;

	savingTimerId = 0;

	autoSearchUpdatesRequest = false;

	show_windows_policy = ActiveFolder;
	d_script_win_on_top = false;
	d_script_win_rect = QRect(0, 0, 500, 300);
	d_init_window_type = TableWindow;

	QString aux = qApp->applicationDirPath();
	workingDir = aux;

	d_excel_import_method = ExcelFormatLibrary;
#ifdef Q_OS_WIN
	d_has_excel = false;
	detectExcel();
	if (d_has_excel)
		d_excel_import_method = LocalExcelInstallation;
	d_java_path = QDir::toNativeSeparators("C:/Program Files/Java/jre6/bin/java.exe");
	d_soffice_path = QDir::toNativeSeparators("C:/Program Files/OpenOffice.org 3/program/soffice.exe");
#endif
#ifdef Q_OS_MAC
	d_java_path = "/usr/bin/java";
	d_soffice_path = "/Applications/OpenOffice.org.app/Contents/MacOS/soffice";
#endif
#ifdef Q_WS_X11
	d_java_path = "/usr/bin/java";
	d_soffice_path = "/usr/bin/soffice";
	d_latex_compiler_path = "/usr/bin/latex";
	d_latex_compiler = Local;
#endif
	d_jodconverter_path = QDir::toNativeSeparators(aux + "/jodconverter/lib/jodconverter-cli-2.2.2.jar");
	if (d_excel_import_method == ExcelFormatLibrary && QFile(d_soffice_path).exists())
		d_excel_import_method = LocalOpenOffice;

#ifdef TRANSLATIONS_PATH
	d_translations_folder = TRANSLATIONS_PATH;
	fitPluginsPath = "/usr/local/qtiplot/fitPlugins";
#else
	d_translations_folder = aux + "/translations";
	fitPluginsPath = aux + "/fitPlugins";
#endif

#ifdef MANUAL_PATH
	helpFilePath = MANUAL_PATH;
	helpFilePath += "/index.html";
#else
	helpFilePath = aux + "/manual/index.html";
#endif

#ifdef PYTHON_CONFIG_PATH
	d_python_config_folder = PYTHON_CONFIG_PATH;
#else
	d_python_config_folder = aux;
#endif

	d_startup_scripts_folder = aux + "/scripts";
	fitModelsPath = QString();
	templatesDir = aux;
	asciiDirPath = aux;
	imagesDirPath = aux;
	scriptsDirPath = aux;
	customActionsDirPath = QString();

	appFont = QFont();
    d_notes_font = appFont;

	QString family = appFont.family();
	int pointSize = appFont.pointSize();
	tableTextFont = appFont;
	tableHeaderFont = appFont;
	plotAxesFont = QFont(family, pointSize, QFont::Bold, false);
	plotNumbersFont = QFont(family, pointSize );
	plotLegendFont = appFont;
	plotTitleFont = QFont(family, pointSize + 2, QFont::Bold,false);

	autoSearchUpdates = false;
	appLanguage = QLocale::system().name().section('_',0,0);
	show_windows_policy = ApplicationWindow::ActiveFolder;

	workspaceColor = QColor("darkGray");
	panelsColor = QColor("#ffffff");
	panelsTextColor = QColor("#000000");
	tableBkgdColor = QColor("#ffffff");
	tableTextColor = QColor("#000000");
	tableHeaderColor = QColor("#000000");

	d_decimation_method = Graph::LTTB;
	d_Douglas_Peuker_tolerance = 0.0;
	d_speed_mode_points = 3000;
	d_speed_mode_export = false;

	d_graph_background_color = Qt::white;
	d_graph_canvas_color = Qt::white;
	d_graph_border_color = Qt::black;
	d_graph_background_opacity = 0;
	d_graph_canvas_opacity = 0;
	d_graph_border_width = 0;
	d_graph_tick_labels_dist = 4;
	d_graph_axes_labels_dist = 2;

	autoSave = true;
	autoSaveTime = 15;
	d_backup_files = true;

	defaultScriptingLang = "muParser";
/*#ifdef SCRIPTING_PYTHON
	defaultScriptingLang = "Python";
#endif*/

	d_decimal_digits = 13;
	d_muparser_c_locale = true;

	d_extended_open_dialog = true;
	d_extended_export_dialog = true;
	d_extended_import_ASCII_dialog = true;
	d_extended_plot_dialog = true;

	d_add_curves_dialog_size = QSize(700, 400);
	d_show_current_folder = false;

	d_confirm_overwrite = true;
	confirmCloseFolder = true;
	confirmCloseTable = true;
	confirmCloseMatrix = true;
	confirmClosePlot2D = true;
	confirmClosePlot3D = true;
	confirmCloseNotes = true;
	d_inform_rename_table = true;

	d_show_table_comments = false;

	titleOn = true;
	d_show_axes = QVector<bool> (QwtPlot::axisCnt, true);
	d_show_axes_labels = QVector<bool> (QwtPlot::axisCnt, true);
	canvasFrameWidth = 0;
	d_canvas_frame_color = Qt::black;
	defaultPlotMargin = 0;
	drawBackbones = true;
	axesLineWidth = 1;
	autoscale2DPlots = true;
	autoScaleFonts = true;
	autoResizeLayers = true;
	antialiasing2DPlots = false;
	d_scale_plots_on_print = false;
	d_print_cropmarks = false;
	d_graph_legend_display = Graph::Auto;
	d_graph_attach_policy = FrameWidget::Scales;
	d_graph_axis_labeling = Graph::Default;
	d_synchronize_graph_scales = true;
	d_print_paper_size = QPageSize::A4;
	d_printer_orientation = QPageLayout::Landscape;
	defaultCurveStyle = int(Graph::LineSymbols);
	defaultCurveLineWidth = 1;
	d_curve_line_style = 0;//Qt::SolidLine;
	defaultCurveBrush = 0;
	defaultCurveAlpha = 100;
	defaultSymbolSize = 7;
	defaultSymbolEdge = 1.0;
	d_fill_symbols = true;
	d_symbol_style = 0;
	d_indexed_symbols = true;
	d_keep_aspect_ration = true;

	d_disable_curve_antialiasing = true;
	d_curve_max_antialising_size = 1000;

	majTicksStyle = int(ScaleDraw::Out);
	minTicksStyle = int(ScaleDraw::Out);
	minTicksLength = 5;
	majTicksLength = 9;

	legendFrameStyle = int(LegendWidget::Line);
	d_frame_widget_pen = QPen(Qt::black, 1, Qt::SolidLine);
	legendTextColor = Qt::black;
	legendBackground = Qt::white;
	legendBackground.setAlpha(0); // transparent by default;
	d_legend_default_angle = 0;
	d_frame_geometry_unit = (int)FrameWidget::Scale;
	d_layer_geometry_unit = (int)FrameWidget::Pixel;
	d_layer_canvas_width = 400;
	d_layer_canvas_height = 300;

	d_rect_default_background = Qt::white;
	d_rect_default_brush = QBrush(Qt::white);

	defaultArrowLineWidth = 1;
	defaultArrowColor = Qt::black;
	defaultArrowHeadLength = 4;
	defaultArrowHeadAngle = 45;
	defaultArrowHeadFill = true;
	defaultArrowLineStyle = Graph::getPenStyle("SolidLine");

	d_3D_legend = true;
	d_3D_projection = NOFLOOR;
	d_3D_shading = Qwt3D::FLAT;
	d_3D_smooth_mesh = true;
	d_3D_resolution = 1;
	d_3D_orthogonal = false;
	d_3D_autoscale = true;
	d_3D_scale_fonts = true;
	d_3D_axes_font = QFont(family, pointSize, QFont::Normal, false);
	d_3D_numbers_font = QFont(family, pointSize);
	d_3D_title_font = QFont(family, pointSize + 2, QFont::Normal, false);
	d_3D_color_map = LinearColorMap::rainbow();
	d_3D_color_map.setMode(QwtLinearColorMap::FixedColors);
	d_3D_mesh_color = Qt::black;
	d_3D_axes_color = Qt::black;
	d_3D_numbers_color = Qt::black;
	d_3D_labels_color = Qt::black;
	d_3D_background_color = Qt::white;
	d_3D_grid_color = Qt::blue;
	d_3D_minor_grid_color = Qt::lightGray;
	d_3D_minor_grids = true;
	d_3D_major_grids = true;
#pragma message("CHECKING SOLID USAGE")
	// d_3D_major_style = Qwt3D::SOLID;
	// d_3D_minor_style = Qwt3D::DASH;
	d_3D_major_width = 1.0;
	d_3D_minor_width = 0.8;

	fit_output_precision = 13;
	pasteFitResultsToPlot = false;
	d_write_fit_results_to_log = true;
	generateUniformFitPoints = true;
	fitPoints = 100;
	generatePeakCurves = true;
	peakCurvesColor = Qt::green;
	fit_scale_errors = true;
	d_2_linear_fit_points = true;
	d_multi_peak_messages = true;

	columnSeparator = "\t";
	ignoredLines = 0;
	renameColumns = true;
	strip_spaces = false;
	simplify_spaces = false;
	d_ASCII_file_filter = "*";
	d_ASCII_import_locale = QLocale::system();
	d_ASCII_import_mode = int(ImportASCIIDialog::NewTables);
	d_ASCII_import_first_row_role = 0;//column names
	d_ASCII_comment_string = "#";
	d_ASCII_import_comments = false;
	d_ASCII_import_read_only = false;
	d_ASCII_import_preview = true;
	d_preview_lines = 100;
	d_import_ASCII_dialog_size = QSize();
    d_ASCII_end_line = LF;
	d_eol = LF;
#ifdef Q_OS_MAC
    d_ASCII_end_line = CR;
	d_eol = CR;
#endif

	d_export_col_separator = "\t";
	d_export_col_names = false;
    d_export_col_comment = false;
	d_export_table_selection = false;
	d_export_ASCII_file_filter = ".dat";

	d_scale_fonts_factor = 0.0;
	d_image_export_filter = ".png";
	d_export_transparency = false;
	d_export_quality = 100;
	d_export_compression = 1;
	d_export_raster_size = QSizeF();
	d_export_size_unit = FrameWidget::Pixel;
	d_export_vector_resolution = 600;
	d_export_bitmap_resolution = QWidget().logicalDpiX();
	d_export_color = true;
	d_export_escape_tex_strings = true;
	d_export_tex_font_sizes = true;
	d_3D_export_text_mode = 1; //VectorWriter::NATIVE
	d_3D_export_sort = 1; //VectorWriter::SIMPLESORT
}

void ApplicationWindow::initToolBars()
{
	if (d_action_manager)
		d_action_manager->initToolBars();
}

void ApplicationWindow::insertTranslatedStrings()
{
	if (projectname == "untitled")
		setWindowTitle(tr("QtiPlot - untitled"));

	lv->setColumnText (0, tr("Name"));
	lv->setColumnText (1, tr("Type"));
	lv->setColumnText (2, tr("View"));
	//lv->setColumnText (3, tr("Size"));
	lv->setColumnText (3, tr("Created"));
	lv->setColumnText (4, tr("Label"));

	if (scriptWindow)
		scriptWindow->setWindowTitle(tr("QtiPlot - Script Window"));
	explorerWindow->setWindowTitle(tr("Project Explorer"));
	logWindow->setWindowTitle(tr("Results Log"));
	undoStackWindow->setWindowTitle(tr("Undo Stack"));
#ifdef SCRIPTING_CONSOLE
	consoleWindow->setWindowTitle(tr("Scripting Console"));
#endif
	displayBar->setWindowTitle(tr("Data Display"));
	tableTools->setWindowTitle(tr("Table"));
	columnTools->setWindowTitle(tr("Column"));
	plotTools->setWindowTitle(tr("Plot"));
	fileTools->setWindowTitle(tr("File"));
	editTools->setWindowTitle(tr("Edit"));
	noteTools->setWindowTitle(tr("Notes"));
	plotMatrixBar->setWindowTitle(tr("Matrix Plot"));
	plot3DTools->setWindowTitle(tr("3D Surface"));
	formatToolBar->setWindowTitle(tr("Format"));

	recentMenuAction->setText(tr("&Recent Projects"));
	fileMenu->setTitle(tr("&File"));
	newMenu->setTitle(tr("&New"));
	exportPlotMenu->setTitle(tr("&Export Graph"));
	importMenu->setTitle(tr("&Import"));
	edit->setTitle(tr("&Edit"));
	view->setTitle(tr("&View"));
	graphMenu->setTitle(tr("&Graph"));
	plot3DMenu->setTitle(tr("3D &Plot"));
	matrixMenu->setTitle(tr("&Matrix"));
    plot2DMenu->setTitle(tr("&Plot"));
    plotDataMenu->setTitle(tr("&Data"));
	normMenu->setTitle(tr("&Normalize"));
	fillMenu->setTitle(tr("&Fill Columns With"));
	tableMenu->setTitle(tr("&Table"));
	smoothMenu->setTitle(tr("&Smooth"));
	filterMenu->setTitle(tr("&FFT filter"));
	decayMenu->setTitle(tr("Fit E&xponential Decay"));
	multiPeakMenu->setTitle(tr("Fit &Multi-peak"));
	analysisMenu->setTitle(tr("&Analysis"));
	format->setTitle(tr("F&ormat"));
	scriptingMenu->setTitle(tr("&Scripting"));
	windowsMenu->setTitle(tr("&Windows"));
	help->setTitle(tr("&Help"));

	translateActionsStrings();
	customMenu(activeWindow());
}

void ApplicationWindow::initMainMenu()
{
	if (d_action_manager)
		d_action_manager->initMainMenu();
}

void ApplicationWindow::tableMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->tableMenuAboutToShow();
}

void ApplicationWindow::openLogFolder()
{
	QString path = Logger::logDirPath();
	QDir().mkpath(path);
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ApplicationWindow::autoSaveRecovery()
{
	if (d_project_manager)
		d_project_manager->autoSaveRecovery();
}

void ApplicationWindow::checkRecoveryOnStartup()
{
	if (d_project_manager)
		d_project_manager->checkRecoveryOnStartup();
}

void ApplicationWindow::plotDataMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->plotDataMenuAboutToShow();
}

void ApplicationWindow::plotMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->plotMenuAboutToShow();
}

void ApplicationWindow::customMenu(QMdiSubWindow* w)
{
	if (d_action_manager)
		d_action_manager->customMenu(w);
}

void ApplicationWindow::disableActions()
{
	if (d_action_manager)
		d_action_manager->disableActions();
}

void ApplicationWindow::customColumnActions()
{
	if (d_action_manager)
		d_action_manager->customColumnActions();
}

void ApplicationWindow::customToolBars(QMdiSubWindow* w)
{
	if (d_action_manager)
		d_action_manager->customToolBars(w);
}

void ApplicationWindow::disableToolbars()
{
	if (d_action_manager)
		d_action_manager->disableToolbars();
}



























void ApplicationWindow::renameListViewItem(const QString& oldName,const QString& newName)
{
	QList<QTreeWidgetItem*> items = lv->findItems(oldName, Qt::MatchExactly | Qt::MatchCaseSensitive, 0);
	if (!items.isEmpty())
		items[0]->setText(0, newName);
}

void ApplicationWindow::setListViewLabel(const QString& caption,const QString& label)
{
	QList<QTreeWidgetItem*> items = lv->findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive, 0);
	if (!items.isEmpty()){
		QString s = label;
		items[0]->setText(4, s.replace("\n", " "));
	}
}

void ApplicationWindow::setListViewDate(const QString& caption,const QString& date)
{
	QList<QTreeWidgetItem*> items = lv->findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive, 0);
	if (!items.isEmpty())
		items[0]->setText(3, date);
}

void ApplicationWindow::setListView(const QString& caption,const QString& view)
{
	QList<QTreeWidgetItem*> items = lv->findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive, 0);
	if (!items.isEmpty())
		items[0]->setText(2, view);
}

QString ApplicationWindow::listViewDate(const QString& caption)
{
	QList<QTreeWidgetItem*> items = lv->findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive, 0);
	if (!items.isEmpty())
		return items[0]->text(3);
	else
		return "";
}

void ApplicationWindow::updateTableNames(const QString& oldName, const QString& newName)
{
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows) {
		if (w->inherits("MultiLayer")) {
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers)
				g->updateCurveNames(oldName, newName);
		} else if (w->inherits("Graph3D")) {
			QString name = ((Graph3D*)w)->formula();
			if (name.contains(oldName, Qt::CaseSensitive)) {
				name.replace(oldName,newName);
				((Graph3D*)w)->setPlotAssociation(name);
			}
		}
	}
}

void ApplicationWindow::updateColNames(const QString& oldName, const QString& newName)
{
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers)
				g->updateCurveNames(oldName, newName, false);
		} else if (w->inherits("Graph3D")){
			QString name = ((Graph3D*)w)->formula();
			if (name.contains(oldName)){
				name.replace(oldName,newName);
				((Graph3D*)w)->setPlotAssociation(name);
			}
		}
	}
}

void ApplicationWindow::changeMatrixName(const QString& oldName, const QString& newName)
{
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Graph3D"))
		{
			QString s = ((Graph3D*)w)->formula();
			if (s.contains(oldName))
			{
				s.replace(oldName, newName);
				((Graph3D*)w)->setPlotAssociation(s);
			}
		}
		else if (w->inherits("MultiLayer"))
		{
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers){
				for (int i=0; i<g->curveCount(); i++){
					QwtPlotItem *sp = (QwtPlotItem *)g->plotItem(i);
					if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && sp->title().text() == oldName)
						sp->setTitle(newName);
				}
			}
		}
	}
}














Graph3D* ApplicationWindow::plotSurface(const QString& formula, double xl, double xr,
		double yl, double yr, double zl, double zr, int columns, int rows)
{
	return d_plot_controller_3d ? d_plot_controller_3d->plotSurface(formula, xl, xr, yl, yr, zl, zr, columns, rows) : nullptr;
}

Graph3D* ApplicationWindow::plotParametricSurface(const QString& xFormula, const QString& yFormula,
		const QString& zFormula, double ul, double ur, double vl, double vr,
		int columns, int rows, bool uPeriodic, bool vPeriodic)
{
	return d_plot_controller_3d ? d_plot_controller_3d->plotParametricSurface(xFormula, yFormula, zFormula, ul, ur, vl, vr, columns, rows, uPeriodic, vPeriodic) : nullptr;
}

void ApplicationWindow::updateSurfaceFuncList(const QString& s)
{
	surfaceFunc.removeAll(s);
	surfaceFunc.push_front(s);
	while ((int)surfaceFunc.size() > 10)
		surfaceFunc.pop_back();
}

Graph3D* ApplicationWindow::newPlot3D(const QString& title)
{
	return d_plot_controller_3d ? d_plot_controller_3d->newPlot3D(title) : nullptr;
}

Graph3D* ApplicationWindow::plotXYZ(Table* table, const QString& zColName, int type)
{
	return d_plot_controller_3d ? d_plot_controller_3d->plotXYZ(table, zColName, type) : nullptr;
}







Matrix* ApplicationWindow::importImage(const QString& fileName, bool newWindow)
{
	QString fn = fileName;
	if (fn.isEmpty()){
		fn = getFileName(this, tr("QtiPlot - Import image from file"), imagesDirPath, imageFilter(), 0, false);
		if ( !fn.isEmpty() ){
			QFileInfo fi(fn);
			imagesDirPath = fi.absolutePath();
		}
	}

	QImageReader reader(fn);
	QImage image = reader.read();
	if (image.isNull()){
		QMessageBox::critical(this, tr("QtiPlot - Error"), reader.errorString());
		return 0;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	MdiSubWindow *w = activeWindow(MatrixWindow);
	Matrix* m = nullptr;
	if (w && !newWindow){
		m = (Matrix *)w;
		m->importImage(image);
	} else {
		m = new Matrix(scriptEnv, image, "", this);
		initMatrix(m, generateUniqueName(tr("Matrix")));
		m->show();
		m->setWindowLabel(fn);
		m->setCaptionPolicy(MdiSubWindow::Both);
	}

	QApplication::restoreOverrideCursor();
	return m;
}

QString ApplicationWindow::imageFilter()
{
	QList<QByteArray> list = QImageReader::supportedImageFormats();
	QString filter = tr("Images") + " (";
	for (int i = 0; i < (int)list.count(); i++){
		if (i > 0)
			filter += " ";
		filter += "*." + QString::fromLatin1(list[i].toLower());
	}
	filter += ");;" + tr("All Files") + " (*)";
	return filter;
}

void ApplicationWindow::loadImage()
{
	QString fn = getFileName(this, tr("QtiPlot - Load image from file"), imagesDirPath, imageFilter(), 0, false);
	if ( !fn.isEmpty() ){
		loadImage(fn);
		QFileInfo fi(fn);
		imagesDirPath = fi.absolutePath();
	}
}

void ApplicationWindow::loadImage(const QString& fn)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	MultiLayer *plot = multilayerPlot(generateUniqueName(tr("Graph")));
	plot->setWindowLabel(fn);
	plot->setCaptionPolicy(MdiSubWindow::Both);

	Graph *g = plot->activeLayer();
	g->setTitle("");
	for (int i=0; i<4; i++)
		g->enableAxis(i, false);
	g->addImage(fn);
	QApplication::restoreOverrideCursor();
}

MultiLayer* ApplicationWindow::multilayerPlot(const QString& caption, int layers, int rows, int cols)
{
	return d_plot_controller_2d ? d_plot_controller_2d->multilayerPlot(caption, layers, rows, cols) : nullptr;
}

MultiLayer* ApplicationWindow::newGraph(const QString& caption)
{
	return d_plot_controller_2d ? d_plot_controller_2d->newGraph(caption) : nullptr;
}

MultiLayer* ApplicationWindow::multilayerPlot(Table* w, const QStringList& colList, int style, int startRow, int endRow)
{//used when plotting selected columns
	return d_plot_controller_2d ? d_plot_controller_2d->multilayerPlot(w, colList, style, startRow, endRow) : nullptr;
}

MultiLayer* ApplicationWindow::multilayerPlot(int c, int r, int style, const MultiLayer::AlignPolicy& align)
{//used when plotting from the panel menu
	return d_plot_controller_2d ? d_plot_controller_2d->multilayerPlot(c, r, style, align) : nullptr;
}

MultiLayer* ApplicationWindow::waterfallPlot()
{
	return d_plot_controller_2d ? d_plot_controller_2d->waterfallPlot() : nullptr;
}

MultiLayer* ApplicationWindow::waterfallPlot(Table *t, const QStringList& list)
{
	return d_plot_controller_2d ? d_plot_controller_2d->waterfallPlot(t, list) : nullptr;
}


void ApplicationWindow::setAutoUpdateTableValues(bool on)
{
	if (d_auto_update_table_values == on)
		return;

	d_auto_update_table_values = on;

	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
            if (w->inherits("Table"))
            	((Table *)w)->setAutoUpdateValues(d_auto_update_table_values);
		}
		f = f->folderBelow();
	}
}

void ApplicationWindow::customTable(Table* w)
{
	QPalette palette;
	palette.setColor(QPalette::Base, tableBkgdColor);
	palette.setColor(QPalette::Text, tableTextColor);
	w->setPalette(palette);

	w->setHeaderColor(tableHeaderColor);
	w->setTextFont(tableTextFont);
	w->setHeaderFont(tableHeaderFont);
	w->showComments(d_show_table_comments);
	w->setNumericPrecision(d_decimal_digits);
}

void ApplicationWindow::setPreferences(Graph* g)
{
	if (d_plot_controller_2d)
		d_plot_controller_2d->setPreferences(g);
}

/*
 * return the current Plot (used for the Python interface)
 */
MultiLayer* ApplicationWindow::currentPlot()
{
	MultiLayer* p = (MultiLayer*)activeWindow(MultiLayerWindow);
	return p;
}

/*
 * return the current Table (used for the Python interface)
 */
Table* ApplicationWindow::currentTable()
{
	Table* w = (Table*)activeWindow(TableWindow);
	return w;
}

/*
 *creates a new empty table
 */
Table* ApplicationWindow::newTable()
{
	Table* w = new Table(scriptEnv, 30, 2, "", this, 0);
	initTable(w, generateUniqueName(tr("Table")));
	if (!qApp->arguments().contains("-X"))
		w->showNormal();
	return w;
}

/*
 *used when opening a project file
 */
Table* ApplicationWindow::newTable(const QString& caption, int r, int c)
{
	Table* w = new Table(scriptEnv, r, c, "", this, 0);
	initTable(w, caption);
	if (d_is_appending_file && w->objectName() != caption){//the table was renamed
		renamedTables << caption << w->objectName();
		if (d_inform_rename_table){
			QApplication::restoreOverrideCursor();
			QMessageBox:: warning(this, tr("QtiPlot - Renamed Window"),
			tr("The table '%1' already exists. It has been renamed '%2'.").arg(caption).arg(w->objectName()));
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		}
	}
	if (!qApp->arguments().contains("-X"))
		w->showNormal();
	return w;
}

Table* ApplicationWindow::newTable(int r, int c, const QString& name, const QString& legend)
{
	Table* w = new Table(scriptEnv, r, c, legend, this, 0);
	initTable(w, name);
	return w;
}

Table* ApplicationWindow::newHiddenTable(const QString& name, const QString& label, int r, int c, const QString& text)
{
	Table* w = new Table(scriptEnv, r, c, label, this, 0);

	if (!text.isEmpty()) {
		QStringList rows = text.split("\n", Qt::SkipEmptyParts);
		QStringList list = rows[0].split("\t");
		w->setHeader(list);

		QString rlist;
		for (int i=0; i<r; i++){
			rlist=rows[i+1];
			list = rlist.split("\t");
			for (int j=0; j<c; j++)
				w->setText(i, j, list[j]);
		}
	}

	initTable(w, name);
	hideWindow(w);
	return w;
}

void ApplicationWindow::initTable(Table* w, const QString& caption)
{
	QString name = caption;
	while(name.isEmpty() || alreadyUsedName(name))
		name = generateUniqueName(tr("Table"));

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(w);
	else
		w->setParent(0);

	connectTable(w);
	customTable(w);

	w->setObjectName(name);
	w->setWindowIcon(QPixmap(":/worksheet.png") );
	addListViewItem(w);

	if (d_undo_group)
		d_undo_group->addStack(w->undoStack());
}

/*
 * !creates a new table with type statistics on target columns/rows of table base
 */
TableStatistics *ApplicationWindow::newTableStatistics(Table *base, int type, QList<int> target, int start, int end, const QString &caption)
{
	TableStatistics* s = new TableStatistics(scriptEnv, this, base, (TableStatistics::Type) type, target, start, end);
	if (caption.isEmpty())
		initTable(s, s->objectName());
	else
		initTable(s, caption);
	s->showNormal();
	return s;
}

/*
 * return the current note (used for the Python interface)
 */
Note* ApplicationWindow::currentNote()
{
	Note* m = (Note*)activeWindow(NoteWindow);
	return m;
}

/*
 *creates a new empty note window
 */
Note* ApplicationWindow::newNote(const QString& caption)
{
	Note* m = new Note(scriptEnv, "", this);

	QString name = caption;
	while(name.isEmpty() || alreadyUsedName(name))
		name = generateUniqueName(tr("Notes"));

	m->setObjectName(name);
	m->setWindowIcon(QPixmap(":/note.png"));
	m->askOnCloseEvent(confirmCloseNotes);

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(m);
	else
		m->setParent(0);

	addListViewItem(m);

	connect(m, &MdiSubWindow::modifiedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect(m, &MdiSubWindow::resizedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect(m, &MdiSubWindow::closedWindow, this, &ApplicationWindow::closeWindow);
	connect(m, &MdiSubWindow::hiddenWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::hideWindow));
	connect(m, &MdiSubWindow::statusChanged, this, &ApplicationWindow::updateWindowStatus);
	connect(m, &Note::dirPathChanged, this, &ApplicationWindow::scriptsDirPathChanged);
	connect(m, &Note::currentEditorChanged, this, &ApplicationWindow::scriptingMenuAboutToShow);

	if (!qApp->arguments().contains("-X"))
		m->showNormal();
	return m;
}

void ApplicationWindow::connectScriptEditor(ScriptEdit *editor)
{
	if (!editor)
		return;

	QTextDocument *doc = editor->document();
	MdiSubWindow *w = activeWindow();
	if (qobject_cast<Note*>(w)) {
		actionUndo->setEnabled(doc && doc->isUndoAvailable());
		actionRedo->setEnabled(doc && doc->isRedoAvailable());
	}

	connect(editor, &ScriptEdit::undoAvailable, this, [this](bool available){
		if (qobject_cast<Note*>(activeWindow()))
			actionUndo->setEnabled(available);
	}, Qt::UniqueConnection);
	connect(editor, &ScriptEdit::redoAvailable, this, [this](bool available){
		if (qobject_cast<Note*>(activeWindow()))
			actionRedo->setEnabled(available);
	}, Qt::UniqueConnection);
}

/*
 * return the current Matrix (used for the Python interface)
 */
Matrix* ApplicationWindow::currentMatrix()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	return m;
}

Matrix* ApplicationWindow::newMatrix(int rows, int columns)
{
	Matrix* m = new Matrix(scriptEnv, rows, columns, "", this, 0);
	initMatrix(m, generateUniqueName(tr("Matrix")));
	if (!qApp->arguments().contains("-X"))
		m->showNormal();
	return m;
}

Matrix* ApplicationWindow::newMatrix(const QString& caption, int r, int c)
{
	Matrix* w = new Matrix(scriptEnv, r, c, "", this, 0);
	initMatrix(w, caption);
	if (d_is_appending_file && w->objectName() != caption){//the matrix was renamed
		renamedTables << caption << w->objectName();
		if (d_inform_rename_table){
			QApplication::restoreOverrideCursor();
			QMessageBox:: warning(this, tr("QtiPlot - Renamed Window"),
			tr("The table '%1' already exists. It has been renamed '%2'.").arg(caption).arg(w->objectName()));
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		}
	}

	if (!qApp->arguments().contains("-X"))
		w->showNormal();
	return w;
}

void ApplicationWindow::viewMatrixImage()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m->undoStack()->push(new MatrixSetViewCommand(m, m->viewType(), Matrix::ImageView, tr("Set Image Mode")));
	m->setViewType(Matrix::ImageView);
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::viewMatrixTable()
{
	Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m->undoStack()->push(new MatrixSetViewCommand(m, m->viewType(), Matrix::TableView, tr("Set Data Mode")));
	m->setViewType(Matrix::TableView);
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::viewMatrixXY()
{
    Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m->undoStack()->push(new MatrixSetHeaderViewCommand(m, m->headerViewType(), Matrix::XY, tr("Show X/Y")));
	m->setHeaderViewType(Matrix::XY);
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::viewMatrixColumnRow()
{
    Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m->undoStack()->push(new MatrixSetHeaderViewCommand(m, m->headerViewType(), Matrix::ColumnRow, tr("Show Column/Row")));
	m->setHeaderViewType(Matrix::ColumnRow);
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::setMatrixDefaultScale()
{
	Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m->undoStack()->push(new MatrixSetColorMapCommand(m, m->colorMapType(), m->colorMap(),
						Matrix::Default, LinearColorMap(), tr("Set Default Palette")));
	m->setDefaultColorMap();
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::setMatrixGrayScale()
{
	Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m->undoStack()->push(new MatrixSetColorMapCommand(m, m->colorMapType(), m->colorMap(),
						Matrix::GrayScale, LinearColorMap(), tr("Set Gray Scale Palette")));
	m->setGrayScale();
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::setMatrixRainbowScale()
{
	Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m->undoStack()->push(new MatrixSetColorMapCommand(m, m->colorMapType(), m->colorMap(),
						Matrix::Rainbow, LinearColorMap(), tr("Set Rainbow Palette")));
	m->setRainbowColorMap();
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::showColorMapDialog()
{
	Matrix* m = static_cast<Matrix*>(activeWindow(MatrixWindow));
	if (!m)
		return;

	ColorMapDialog *cmd = new ColorMapDialog(this);
	cmd->setMatrix(m);
	cmd->exec();
}

void ApplicationWindow::transposeMatrix()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->transpose();
}

void ApplicationWindow::flipMatrixVertically()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->flipVertically();
}

void ApplicationWindow::flipMatrixHorizontally()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->flipHorizontally();
}

void ApplicationWindow::rotateMatrix90()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->rotate90();
}

void ApplicationWindow::rotateMatrixMinus90()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->rotate90(false);
}

void ApplicationWindow::matrixDeterminant()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	QDateTime dt = QDateTime::currentDateTime ();
	QString info=dt.toString(Qt::TextDate);
	info+= "\n" + tr("Determinant of ") + QString(m->objectName()) + ":\t";
	info+= "det = " + QString::number(m->determinant()) + "\n";
	info+="-------------------------------------------------------------\n";

	current_folder->appendLogInfo(info);

	showResults(true);
}

void ApplicationWindow::invertMatrix()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->invert();
}

Table* ApplicationWindow::convertMatrixToTableDirect()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return 0;

	return matrixToTable(m, Direct);
}

Table* ApplicationWindow::convertMatrixToTableXYZ()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return 0;

	return matrixToTable(m, XYZ);
}

Table* ApplicationWindow::convertMatrixToTableYXZ()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return 0;

	return matrixToTable(m, YXZ);
}

Table* ApplicationWindow::matrixToTable(Matrix* m, MatrixToTableConversion conversionType)
{
	if (!m)
		return 0;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int rows = m->numRows();
	int cols = m->numCols();
	MatrixModel *mModel = m->matrixModel();

	Table* w = nullptr;
	if (conversionType == Direct){
		w = new Table(scriptEnv, rows, cols, "", this, 0);
		for (int i = 0; i<rows; i++){
			for (int j = 0; j<cols; j++)
				w->setCell(i, j, m->cell(i,j));
		}
	} else if (conversionType == XYZ){
		int tableRows = rows*cols;
		w = new Table(scriptEnv, tableRows, 3, "", this, 0);
		for (int i = 0; i<rows; i++){
			for (int j = 0; j<cols; j++){
				int cell = i*cols + j;
				w->setCell(cell, 0, mModel->x(j));
				w->setCell(cell, 1, mModel->y(i));
				w->setCell(cell, 2, mModel->cell(i, j));
			}
		}
	} else if (conversionType == YXZ){
		int tableRows = rows*cols;
		w = new Table(scriptEnv, tableRows, 3, "", this, 0);
		for (int i = 0; i<cols; i++){
			for (int j = 0; j<rows; j++){
				int cell = i*rows + j;
				w->setCell(cell, 0, mModel->x(i));
				w->setCell(cell, 1, mModel->y(j));
				w->setCell(cell, 2, mModel->cell(i, j));
			}
		}
	}


	initTable(w, generateUniqueName(tr("Table")));
	w->setWindowLabel(m->windowLabel());
	w->setCaptionPolicy(m->captionPolicy());
	w->resize(m->size());
	if (!qApp->arguments().contains("-X"))
		w->showNormal();

	QApplication::restoreOverrideCursor();
	return w;
}

void ApplicationWindow::initMatrix(Matrix* m, const QString& caption)
{
	QString name = caption;
	while(alreadyUsedName(name)){name = generateUniqueName(tr("Matrix"));}

	m->setWindowTitle(name);
	m->setObjectName(name);
	m->setWindowIcon(QPixmap(":/matrix.png"));
	m->askOnCloseEvent(confirmCloseMatrix);
	m->setNumericPrecision(d_decimal_digits);

	if (d_mdi_windows_area)
		d_workspace->addSubWindow(m);
	else
		m->setParent(0);

	addListViewItem(m);
	if (d_undo_group)
		d_undo_group->addStack(m->undoStack());

	connect(m, &Matrix::modifiedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect(m, &Matrix::modifiedLabel, d_plot_controller_3d, &PlotController3D::updateMatrixPlotLabels);
	connect(m, &Matrix::modifiedData, d_plot_controller_3d, &PlotController3D::updateMatrixPlots);
	connect(m, &Matrix::resizedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect(m, &Matrix::closedWindow, this, &ApplicationWindow::closeWindow);
	connect(m, &Matrix::hiddenWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::hideWindow));
	connect(m, &Matrix::statusChanged, this, &ApplicationWindow::updateWindowStatus);
	connect(m, &Matrix::showContextMenu, this, &ApplicationWindow::showWindowContextMenu);

	emit modified();
}

void ApplicationWindow::showBinMatrixDialog()
{
	Table* t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QTableWidgetSelectionRange sel = t->getSelection();
	if (t->selectedYColumns().size() != 1 || fabs(sel.topRow() - sel.bottomRow()) < 2){
        QMessageBox::warning(this, tr("QtiPlot - Column selection error"),
			tr("You must select a single Y column that has an associated X column!"));
		return;
	}

	int ycol = t->colIndex(t->selectedYColumns()[0]);
	if (t->isEmptyColumn(ycol)){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("The selected Y column is empty, operation aborted!"));
		return;
	} else if (t->isEmptyColumn(t->colX(ycol))){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("The associated X column is empty, operation aborted!"));
		return;
	}

	CreateBinMatrixDialog *cbmd = new CreateBinMatrixDialog(t, sel.topRow(), sel.bottomRow(), this);
	cbmd->exec();
}

void ApplicationWindow::showNoDataMessage()
{
	QMessageBox::critical(this, tr("QtiPlot"), tr("Input error: empty data set!"));
}

#ifdef HAVE_ALGLIB
void ApplicationWindow::smoothMatrix()
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	m->smooth();
}

void ApplicationWindow::expandMatrix()
{
	showMatrixResamplingDialog();
}

void ApplicationWindow::shrinkMatrix()
{
	showMatrixResamplingDialog(true);
}

void ApplicationWindow::showMatrixResamplingDialog(bool shrink)
{
	Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	if (!m->isEmpty()){
		MatrixResamplingDialog *mrd = new MatrixResamplingDialog(m, shrink, this);
		mrd->exec();
	} else
		showNoDataMessage();
}

void ApplicationWindow::convertTableToMatrixRandomXYZ()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QStringList selection = t->selectedColumns();
		QTableWidgetSelectionRange sel = t->getSelection();
	if (selection.size() != 1 || t->colPlotDesignation(t->colIndex(selection[0])) != Table::Z ||
		fabs(sel.topRow() - sel.bottomRow()) < 2){
		QMessageBox::warning(this, tr("QtiPlot - Column selection error"), tr("You must select exactly one Z column!"));
		return;
	}

	int startRow = sel.topRow();
	int endRow = sel.bottomRow();
	int zcol = t->colIndex(selection[0]);
	if (zcol < 0 || zcol >= t->numCols())
		return;

	int ycol = t->colY(zcol);
	int xcol = t->colX(ycol);

	int cells = 0;
	for (int i = startRow; i <= endRow; i++){
		QString xs = t->text(i, xcol);
		QString ys = t->text(i, ycol);
		QString zs = t->text(i, zcol);
		if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty())
			cells++;
	}
	if (!cells)
		return;

	GriddingDialog *gd = new GriddingDialog(t, selection[0], cells, this);
	gd->exec();
}
#endif




#ifdef HAVE_TAMUANOVA
#ifdef HAVE_TAMUANOVA
#endif
#endif

Matrix* ApplicationWindow::tableToMatrixRegularXYZ(Table* t, const QString& colName)
{
	if (!t)
		t = (Table*)activeWindow(TableWindow);
	if (!t)
		return 0;

	int startRow = 0;
	int endRow = t->numRows() - 1;

	int zcol = -1;
	if (colName.isEmpty()){
		QTableWidgetSelectionRange sel = t->getSelection();
		if (t->selectedColumns().size() != 1 ||
			t->colPlotDesignation(t->colIndex(t->selectedColumns()[0])) != Table::Z ||
			fabs(sel.topRow() - sel.bottomRow()) < 2){
			QMessageBox::warning(this, tr("QtiPlot - Column selection error"), tr("You must select exactly one Z column!"));
			return 0;
		}
		zcol = t->colIndex(t->selectedColumns()[0]);
		startRow = sel.topRow();
		endRow = sel.bottomRow();
	} else
		zcol = t->colIndex(colName);

	if (zcol < 0 || zcol >= t->numCols())
		return 0;

	int ycol = t->colY(zcol);
	int xcol = t->colX(ycol);

	int cells = 0;
	for (int i = startRow; i <= endRow; i++){
		QString xs = t->text(i, xcol);
		QString ys = t->text(i, ycol);
		QString zs = t->text(i, zcol);
		if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty())
			cells++;
	}

	if (!cells)
		return 0;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QLocale locale = this->locale();
	bool xVariesFirst = false;
	int firstValidRow = startRow;
	double x0 = 0.0, y0 = 0.0, xstart = 0.0, ystart = 0.0;
	double tolerance = 0.15;
	for (int i = startRow; i <= endRow; i++){
		QString xs = t->text(i, xcol);
		QString ys = t->text(i, ycol);
		QString zs = t->text(i, zcol);
		if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty()){
			x0 = locale.toDouble(xs);
			y0 = locale.toDouble(ys);
			xstart = x0;
			ystart = y0;
			firstValidRow = i;
			for (int j = i + 1; j <= endRow; j++){
				xs = t->text(j, xcol);
				ys = t->text(j, ycol);
				zs = t->text(j, zcol);
				if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty()){
					double x = locale.toDouble(xs);
					double y = locale.toDouble(ys);
					if (fabs(x - x0) > fabs(tolerance*x0) && fabs(y - y0) <= fabs(tolerance*y0))
						xVariesFirst = true;
					break;
				}
			}
			break;
		}
	}

	int rows = 0;
	int cols = 0;
	for (int i = firstValidRow; i <= endRow; i++){
		QString xs = t->text(i, xcol);
		QString ys = t->text(i, ycol);
		QString zs = t->text(i, zcol);
		if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty()){
			double x = locale.toDouble(xs);
			double y = locale.toDouble(ys);
			if (xVariesFirst){
				if (fabs(y - y0) <= tolerance){
					cols++;
					y0 = y;
				} else
					break;
			} else {
				if (fabs(x - x0) <= tolerance){
					rows++;
					x0 = x;
				} else
					break;
			}
		}
	}

	if (rows)
		cols = cells/rows;
	else
		rows = cells/cols;

	double xend = xstart, yend = ystart;
	Matrix* m = newMatrix(rows, cols);
	for (int i = startRow; i <= endRow; i++){
		QString xs = t->text(i, xcol);
		QString ys = t->text(i, ycol);
		QString zs = t->text(i, zcol);
		if (!xs.isEmpty() && !ys.isEmpty() && !zs.isEmpty()){
			int row = 0, col = 0;
			if (xVariesFirst){
				col = i%cols;
				row = i/cols;
			} else {
				row = i%rows;
				col = i/rows;
			}

			double x = locale.toDouble(xs);
			double y = locale.toDouble(ys);
			if (x > xend)
				xend = x;
			if (y > yend)
				yend = y;

			m->setCell(row, col, locale.toDouble(zs));
		}
	}

	m->setCoordinates(qMin(xstart, xend), qMax(xstart, xend), qMin(ystart, yend), qMax(ystart, yend));

	QApplication::restoreOverrideCursor();
	return m;
}

Matrix* ApplicationWindow::convertTableToMatrix()
{
	Table* t = (Table*)activeWindow(TableWindow);
	if (!t)
		return 0;

	return tableToMatrix (t);
}

Matrix* ApplicationWindow::tableToMatrix(Table* t)
{
	if (!t)
		return 0;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int rows = t->numRows();
	int cols = t->numCols();
        for (int i = cols - 1; i >= 0; i--){
                if (t->isEmptyColumn(i))
                    cols--;
        }

        if (!cols){
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("QtiPlot - Error"), tr("The selected table is empty, operation aborted!"));
            return 0;
        }

	Matrix* m = newMatrix(rows, cols);
	for (int i = 0; i<rows; i++){
		for (int j = 0; j<cols; j++)
			m->setCell(i, j, t->cell(i, j));
	}

	m->showNormal();

	QApplication::restoreOverrideCursor();
	return m;
}

MdiSubWindow* ApplicationWindow::window(const QString& name, bool label)
{
	QList<MdiSubWindow *> windows = windowsList();
	if (label){
		for (MdiSubWindow *w : windows){
			if (w->windowLabel() == name)
				return w;
		}
	} else {
		for (MdiSubWindow *w : windows){
			if (w->objectName() == name)
				return w;
		}
	}
	return  nullptr;
}

Table* ApplicationWindow::table(const QString& name)
{
	QString caption = name.left(name.lastIndexOf("_"));
	Folder *f = projectFolder();
	while (f){
		for (MdiSubWindow *w : f->windowsList()){
			if (w->inherits("Table") && w->objectName() == caption)
				return qobject_cast<Table*>(w);
		}
		f = f->folderBelow();
	}
	return  0;
}

Matrix* ApplicationWindow::matrix(const QString& name)
{
	QString caption = name;
	if (d_is_appending_file && !renamedTables.isEmpty() && renamedTables.contains(caption)){
		int index = renamedTables.indexOf(caption);
		caption = renamedTables[index + 1];
	}

	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if (w->inherits("Matrix") && w->objectName() == caption)
				return (Matrix*)w;
		}
		f = f->folderBelow();
	}
	return  0;
}

MdiSubWindow *ApplicationWindow::activeWindow(WindowType type)
{
	if (!d_active_window){
		QList<MdiSubWindow *> windows = current_folder->windowsList();
		if (!current_folder->activeWindow() && windows.size() > 0){
			d_active_window = windows[0];
		} else
			return nullptr;
	}

	switch(type){
		case TableWindow:
			if (qobject_cast<Table *>(d_active_window))
				return d_active_window;
		break;
		case MatrixWindow:
			if (qobject_cast<Matrix *>(d_active_window))
				return d_active_window;
		break;
		case MultiLayerWindow:
			if (qobject_cast<MultiLayer *>(d_active_window))
				return d_active_window;
		break;
		case NoteWindow:
			if (qobject_cast<Note *>(d_active_window))
				return d_active_window;
		break;
		case Plot3DWindow:
			if (qobject_cast<Graph3D *>(d_active_window))
				return d_active_window;
		break;
		case PolarGraphWindow:
			if (qobject_cast<PolarGraph *>(d_active_window))
				return d_active_window;
		break;
		default:
			return d_active_window;
	}
	return nullptr;
}

void ApplicationWindow::windowActivated(QMdiSubWindow *w)
{
	MdiSubWindow *window = qobject_cast<MdiSubWindow *>(w);
	if (!window)
		return;

	if (d_active_window && d_active_window == window)
		return;

	d_active_window = window;

	customToolBars(window);
	customMenu(window);

	if (d_opening_file)
		return;

	for (MdiSubWindow *ow : current_folder->windowsList()){
		if (ow != window && ow->status() == MdiSubWindow::Maximized){
			ow->setNormal();
			break;
		}
	}

	Folder *f = window->folder();
	if (f)
		f->setActiveWindow(window);

	d_workspace->setActiveSubWindow(0);
	d_workspace->setActiveSubWindow(window);
	window->raise();

	QUndoStack *stack = window->undoStack();
	if (d_undo_group)
		d_undo_group->setActiveStack(stack);
	if (d_undo_view) {
		d_undo_view->setEmptyLabel(window->objectName() + ": " + tr("Empty Stack"));
		d_undo_view->setStack(stack);
	}
	updateUndoRedoActions();

	emit modified();
}

void ApplicationWindow::addErrorBars()
{
	Graph *g = activePlotLayer(false);
	if (!g)
		return;

	if (!g->curveCount()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("There are no curves available on this plot!"));
		return;
	}
	if (g->isPiePlot()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		return;
	}

	ErrDialog* ed = new ErrDialog(this);
	ed->setCurveNames(g->analysableCurvesList());
	ed->setSrcTables(tableList());
	ed->exec();
}

void ApplicationWindow::removeCurves(const QString& name)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers)
                g->removeCurves(name);
		} else if (w->inherits("Graph3D")){
			Graph3D *g3d = (Graph3D*)w;
			if (g3d->hasAssociation(name))
				g3d->clearData();
		}
	}


	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::updateCurves(Table *t, const QString& name)
{
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers)
                g->updateCurvesData(t, name);
		} else if (w->inherits("Graph3D")){
			Graph3D* g = (Graph3D*)w;
			if (g->table() == t && g->hasAssociation(name))
				g->updateData(t);
		}
	}
}

void ApplicationWindow::showPreferencesDialog()
{
	ConfigDialog* cd = new ConfigDialog(this);
	cd->setColumnSeparator(columnSeparator);
	cd->exec();
}

void ApplicationWindow::setSaveSettings(bool autoSaving, int min)
{
	if (autoSave==autoSaving && autoSaveTime==min)
		return;

	autoSave=autoSaving;
	autoSaveTime=min;

	killTimer(savingTimerId);

	if (autoSave)
		savingTimerId=startTimer(autoSaveTime*60000);
	else
		savingTimerId=0;
}

void ApplicationWindow::changeAppStyle(const QString& s)
{
	// style keys are case insensitive
	if (appStyle.toLower() == s.toLower())
		return;

	qApp->setStyle(s);
	appStyle = qApp->style()->objectName();

	QPalette pal = qApp->palette();
	pal.setColor (QPalette::Active, QPalette::Base, QColor(panelsColor));
	qApp->setPalette(pal);

}

void ApplicationWindow::changeAppFont(const QFont& f)
{
	if (appFont == f)
		return;

	appFont = f;
	updateAppFonts();
}

void ApplicationWindow::updateAppFonts()
{
	qApp->setFont(appFont);
	this->setFont(appFont);
	info->setFont(QFont(appFont.family(), 2 + appFont.pointSize(), QFont::Bold,false));
}

void ApplicationWindow::updateConfirmOptions(bool askTables, bool askMatrices, bool askPlots2D,
		bool askPlots3D, bool askNotes)
{
	QList<MdiSubWindow *> windows = windowsList();


	if (confirmCloseTable != askTables){
		confirmCloseTable=askTables;
		for (MdiSubWindow *w : windows){
			if (w->inherits("Table"))
				w->askOnCloseEvent(confirmCloseTable);
		}
	}

	if (confirmCloseMatrix != askMatrices){
		confirmCloseMatrix = askMatrices;
		for (MdiSubWindow *w : windows){
			if (w->inherits("Matrix"))
				w->askOnCloseEvent(confirmCloseMatrix);
		}
	}

	if (confirmClosePlot2D != askPlots2D){
		confirmClosePlot2D=askPlots2D;
		for (MdiSubWindow *w : windows){
			if (w->inherits("MultiLayer"))
				w->askOnCloseEvent(confirmClosePlot2D);
		}
	}

	if (confirmClosePlot3D != askPlots3D){
		confirmClosePlot3D=askPlots3D;
		for (MdiSubWindow *w : windows){
			if (w->inherits("Graph3D"))
				w->askOnCloseEvent(confirmClosePlot3D);
		}
	}

	if (confirmCloseNotes != askNotes){
		confirmCloseNotes = askNotes;
		for (MdiSubWindow *w : windows){
			if (w->inherits("Note"))
				w->askOnCloseEvent(confirmCloseNotes);
		}
	}
}



ApplicationWindow * ApplicationWindow::plotFile(const QString& fn)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	ApplicationWindow *app = new ApplicationWindow();
	app->restoreApplicationGeometry();

    QList<QByteArray> lst = QImageReader::supportedImageFormats() << "JPG";
    QStringList tempList;
    for (QByteArray temp : lst)// convert QList<QByteArray> to QStringList to be able to 'filter'
        tempList.append(QString(temp));

    QFileInfo fi(fn);
    QStringList l = tempList.filter(fi.suffix(), Qt::CaseInsensitive);
    if (l.count() > 0)
        app->importImage(fn);
    else {
        Table* t = app->newTable();
        if (!t) {
            QApplication::restoreOverrideCursor();
            return nullptr;
        }
        t->importASCII(fn, app->columnSeparator, 0, app->renameColumns, app->strip_spaces, app->simplify_spaces,
                app->d_ASCII_import_comments, app->d_ASCII_comment_string,
				app->d_ASCII_import_read_only, Table::Overwrite, app->d_ASCII_import_locale, app->d_ASCII_end_line);
        t->setCaptionPolicy(MdiSubWindow::Both);
        app->multilayerPlot(t, t->YColumns(), defaultCurveStyle);
    }

	QApplication::restoreOverrideCursor();
    savedProject();//force saved state
    close();
	return app;
}

Table * ApplicationWindow::importOdfSpreadsheet(const QString& fileName, int sheet)
{
	QString fn = fileName;
	if (fn.isEmpty()){
		fn = getFileName(this, tr("Open ODF Spreadsheet File"), QString(), "*.ods", 0, false);
		if (fn.isEmpty())
			return nullptr;
	}

	ImportExportPlugin *plugin = importPlugin(fn);
	if (plugin)
		return plugin->import(fn, sheet);

	return 0;
}



#ifdef Q_OS_WIN
void ApplicationWindow::detectExcel()
{
	QString excelApp = "excel.exe";
	QString path = "\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths";
	QStringList keys = QSettings("HKEY_CURRENT_USER" + path, QSettings::NativeFormat).childGroups();
	if (!keys.contains(excelApp))
		keys = QSettings("HKEY_LOCAL_MACHINE" + path, QSettings::NativeFormat).childGroups();

	d_has_excel = keys.contains(excelApp);
}

bool ApplicationWindow::importUsingExcel()
{
	return (d_excel_import_method == LocalExcelInstallation) && d_has_excel;
}
#endif

Table * ApplicationWindow::importExcel(const QString& fileName, int sheet)
{
	QString fn = fileName;
	if (fn.isEmpty()){
		QString filter = tr("Excel files") + " (*.xls)";
#ifdef Q_OS_WIN
		if (importUsingExcel())
			filter = tr("Excel files") + " (*.xl *.xlsx *.xlsm *.xlsb *.xlam *.xltx *.xltm *.xls *.xla *.xlt *.xlm *.xlw)";
		else
#endif
		if (d_excel_import_method == LocalOpenOffice)
			filter = tr("Excel files") + " (*.xls *.xlsx)";

		fn = getFileName(this, tr("Open Excel File"), QString(), filter, 0, false);
		if (fn.isEmpty())
			return nullptr;
	}

	if (d_excel_import_method == LocalOpenOffice){
		ExcelFileConverter fc(fn, this, true);

		Table *t = importOdfSpreadsheet(fc.outputFile(), sheet);
		if (t)
			t->setWindowLabel(fn);
		QFile::remove(fc.outputFile());

		recentProjects.pop_front();
		updateRecentProjectsList(fn);

		return t;
	}

	ImportExportPlugin *plugin = importPlugin(fn);
	if (plugin)
		return plugin->import(fn, sheet);

	return 0;
}

Table * ApplicationWindow::importDatabase(const QString& fileName, int table)
{
	QString fn = fileName;
	if (fn.isEmpty()){
		QStringList filters;
		filters << tr("dBase") + " (*.dbf)";
	#ifdef Q_OS_WIN
		filters << tr("Microsoft Access") + " (*.mdb *accdb)";
	#else
		filters << tr("Microsoft Access") + " (*.mdb)";
	#endif
		filters << tr("SQLite 3") + " (*.db)";

		fn = getFileName(this, tr("Open Database"), QString(), filters.join(";"), 0, false);
		if (fn.isEmpty())
			return 0;
	}

	ImportExportPlugin *plugin = importPlugin(fn);
	if (plugin)
		return plugin->import(fn, table);

	return 0;
}

Table * ApplicationWindow::importWaveFile()
{
	QString fn = getFileName(this, tr("Open File"), QString(), "*.wav", 0, false);
	if (fn.isEmpty())
		return nullptr;

	QString log = QDateTime::currentDateTime ().toString(Qt::TextDate) + " - ";
	log += tr("Imported sound file") + ": " + fn + "\n";
	ifstream file(fn.toLocal8Bit().constData(), ios::in | ios::binary);

	file.seekg(4, ios::beg);
	int chunkSize;
	file.read( (char*) &chunkSize, 4 ); // read the ChunkSize

	file.seekg(16, ios::beg);
	int	subChunk1Size;
	file.read( (char*) &subChunk1Size, 4 ); // read the SubChunk1Size

	short format;
	file.read( (char*) &format, sizeof(short) ); // read the file format.  This should be 1 for PCM
	log += tr("Format") + ": ";
	if (format != 1){
		QMessageBox::information(this, tr("QtiPlot"),
		tr("This is not a PCM type WAV file, operation aborted!"));
		log +=  QString::number(format) + "\n";
		showResults(log, true);
		return nullptr;
	} else
		log += tr("PCM") + "\n";

	short channels;
	file.read( (char*) &channels, sizeof(short) ); // read the # of channels (1 or 2)
	QStringList header;
	if (channels == 2)
		header << tr("Time") << tr("Left") << tr("Right");
	else if (channels == 1)
		header << tr("Time") << tr("Data");
	log += tr("Channels") + ": " + QString::number(channels) + "\n";

	int sampleRate;
	file.read( (char*) &sampleRate, sizeof(int) );
	log += tr("Sample Rate") + ": " + QString::number(sampleRate) + "\n";

	int byteRate;
	file.read( (char*) &byteRate, sizeof(int) );
	log += tr("Byte Rate") + ": " + QString::number(byteRate) + "\n";

	short blockAlign;
	file.read( (char*) &blockAlign, sizeof(short) );
	log += tr("Block Align") + ": " + QString::number(blockAlign) + "\n";

	short bitsPerSample;
	file.read( (char*) &bitsPerSample, sizeof(short) );
	log += tr("Bits Per Sample") + ": " + QString::number(bitsPerSample) + "\n";
	log += "__________________________________\n";
	showResults(log, true);

	int rows = (chunkSize - 36)/blockAlign;
	Table *t = newTable(rows, int(channels + 1), QFileInfo(fn).baseName(), fn);
	if (!t)
		return nullptr;

	t->setHeader(header);

	file.seekg(44, ios::beg);

	double dt = 1.0/(double)sampleRate;
	int aux = 1;
	if (bitsPerSample == 8){
		if (channels == 2)
			file.seekg(46, ios::beg);//???Why???
		int size = 1;
		uchar ch;
		while(!file.eof()){
			int i = aux - 1;
			t->setCell(i, 0, dt*aux);
			for (int j = 1; j <= channels; j++){
				file.read((char*) &ch, size);
				t->setText(i, j, QString::number(ch));
			}
			aux++;
			if (aux>rows) break;
		}
	} else if (bitsPerSample == 16){
		int size = sizeof(short);
		short ch;
		while(!file.eof()){
			int i = aux - 1;
			t->setCell(i, 0, dt*aux);
			for (int j = 1; j <= channels; j++){
				file.read((char*) &ch, size);
				t->setText(i, j, QString::number(ch));
			}
			aux++;
			if (aux>rows) break;
		}
	}

	file.close();
	t->show();
	return t;
}

void ApplicationWindow::importASCII(const QString& fileName)
{
	ImportASCIIDialog *import_dialog = new ImportASCIIDialog(!activeWindow(TableWindow) && !activeWindow(MatrixWindow), this, d_extended_import_ASCII_dialog);
	import_dialog->setDirectory(asciiDirPath);
	import_dialog->selectFilter(d_ASCII_file_filter);
	if (!fileName.isEmpty()){
		import_dialog->selectFile(fileName);
		import_dialog->setCurrentPath(fileName);
	}
	if (d_import_ASCII_dialog_size.isValid())
		import_dialog->resize(d_import_ASCII_dialog_size);
	if (import_dialog->exec() != QDialog::Accepted)
		return;

	asciiDirPath = import_dialog->directory().path();
    d_ASCII_import_mode = import_dialog->importMode();
    columnSeparator = import_dialog->columnSeparator();
    ignoredLines = import_dialog->ignoredLines();
    renameColumns = import_dialog->useFirstRow();
    strip_spaces = import_dialog->stripSpaces();
    simplify_spaces = import_dialog->simplifySpaces();
    d_ASCII_import_locale = import_dialog->decimalSeparators();
    d_ASCII_comment_string = import_dialog->commentString();
    d_ASCII_import_comments = import_dialog->useSecondRow();
    d_ASCII_import_read_only = import_dialog->readOnly();
	d_ASCII_end_line = (EndLineChar)import_dialog->endLineChar();
    saveSettings();

	importASCII(import_dialog->selectedFiles(),
			import_dialog->importMode(),
			import_dialog->columnSeparator(),
			import_dialog->ignoredLines(),
			import_dialog->renameColumns(),
			import_dialog->stripSpaces(),
			import_dialog->simplifySpaces(),
			import_dialog->importComments(),
			import_dialog->decimalSeparators(),
			import_dialog->commentString(),
			import_dialog->readOnly(),
			import_dialog->endLineChar(),
			import_dialog->columnTypes(),
			import_dialog->columnFormats());
}

void ApplicationWindow::importASCII(const QStringList& files, int import_mode, const QString& local_column_separator,
        int local_ignored_lines, bool local_rename_columns, bool local_strip_spaces, bool local_simplify_spaces,
        bool local_import_comments, QLocale local_separators, const QString& local_comment_string,
		bool import_read_only, int endLineChar, const QList<int>& colTypes, const QStringList& colFormats)
{
	if (files.isEmpty())
		return;

	switch(import_mode) {
		case ImportASCIIDialog::NewTables:
			{
				int dx = 0, dy = 0;
				QStringList sorted_files = files;
				sorted_files.sort();
				int filesCount = sorted_files.size();
				for (int i=0; i<filesCount; i++){
					Table *w = newTable();
					if (!w)
						continue;

					w->importASCII(sorted_files[i], local_column_separator, local_ignored_lines,
                                   local_rename_columns, local_strip_spaces, local_simplify_spaces,
                                   local_import_comments, local_comment_string, import_read_only,
								   Table::Overwrite, local_separators, endLineChar, -1, colTypes, colFormats);
					if (!w) continue;
					w->setWindowLabel(sorted_files[i]);
					w->setCaptionPolicy(MdiSubWindow::Both);

					QString name = QFileInfo(sorted_files[i]).baseName();
					if (!alreadyUsedName(name) && !name.contains(QRegularExpression("\\W")))
						setWindowName(w, name);

					if (i == 0){
						dx = w->verticalHeaderWidth();
						dy = w->frameGeometry().height() - w->widget()->height();
					}
					if (filesCount > 1)
						w->move(QPoint(i*dx, i*dy));
				}
				modifiedProject();
				break;
			}
        case ImportASCIIDialog::NewMatrices:
			{
				int dx = 0, dy = 0;
				QStringList sorted_files = files;
				sorted_files.sort();
				int filesCount = sorted_files.size();
				for (int i=0; i<filesCount; i++){
					Matrix *w = newMatrix();
					if (!w)
                        continue;
					w->importASCII(sorted_files[i], local_column_separator, local_ignored_lines,
                                local_strip_spaces, local_simplify_spaces, local_comment_string,
								Matrix::Overwrite, local_separators, endLineChar);
					w->setWindowLabel(sorted_files[i]);
					w->setCaptionPolicy(MdiSubWindow::Both);

					QString name = QFileInfo(sorted_files[i]).baseName();
					if (!alreadyUsedName(name) && !name.contains(QRegularExpression("\\W")))
						setWindowName(w, name);

					if (i == 0){
						dx = w->verticalHeaderWidth();
						dy = w->frameGeometry().height() - w->widget()->height();
					}
					if (filesCount > 1)
						w->move(QPoint(i*dx,i*dy));
				}
				modifiedProject();
				break;
			}

		case ImportASCIIDialog::NewColumns:
		case ImportASCIIDialog::NewRows:
			{
				MdiSubWindow *w = activeWindow();
				if (!w)
                    return;

				if (w->inherits("Table")){
					Table *t = (Table*)w;
					for (int i = 0; i < files.size(); i++){
                        t->importASCII(files[i], local_column_separator, local_ignored_lines, local_rename_columns,
							local_strip_spaces, local_simplify_spaces, local_import_comments,
							local_comment_string, import_read_only, (Table::ImportMode)(import_mode - 2),
							local_separators, endLineChar, -1, colTypes, colFormats);
					}
					t->notifyChanges();
					emit modifiedProject(t);
				} else if (w->inherits("Matrix")){
					Matrix *m = (Matrix *)w;
					for (int i=0; i<files.size(); i++){
						m->importASCII(files[i], local_column_separator, local_ignored_lines,
                         local_strip_spaces, local_simplify_spaces, local_comment_string,
						 (Matrix::ImportMode)(import_mode - 2), local_separators, endLineChar);
					}
				}
				w->setWindowLabel(files.join("; "));
				w->setCaptionPolicy(MdiSubWindow::Name);
				break;
			}
		case ImportASCIIDialog::Overwrite:
			{
				MdiSubWindow *w = activeWindow();
				if (!w)
                    return;

				if (w->inherits("Table")){
				    Table *t = (Table *)w;
					t->importASCII(files[0], local_column_separator, local_ignored_lines, local_rename_columns,
                                    local_strip_spaces, local_simplify_spaces, local_import_comments,
									local_comment_string, import_read_only, Table::Overwrite, local_separators, endLineChar, -1,
									colTypes, colFormats);
					t->notifyChanges();
				} else if (w->inherits("Matrix")){
				    Matrix *m = (Matrix *)w;
					m->importASCII(files[0], local_column_separator, local_ignored_lines,
                          local_strip_spaces, local_simplify_spaces, local_comment_string,
						  Matrix::Overwrite, local_separators, endLineChar);
				}

                w->setWindowLabel(files[0]);
				w->setCaptionPolicy(MdiSubWindow::Both);

				QString name = QFileInfo(files[0]).baseName();
				if (!alreadyUsedName(name) && !name.contains(QRegularExpression("\\W")))
					setWindowName(w, name);

                modifiedProject();
				break;
			}
	}
}

void ApplicationWindow::open()
{
	if (d_project_manager)
		d_project_manager->open();
}

bool ApplicationWindow::isProjectFile(const QString& fn)
{
	if (d_project_manager)
		return d_project_manager->isProjectFile(fn);
	return false;
}

bool ApplicationWindow::isFileReadable(const QString& file_name)
{
	if (d_project_manager)
		return d_project_manager->isFileReadable(file_name);
	return false;
}

ApplicationWindow* ApplicationWindow::open(const QString& fn, bool factorySettings, bool newProject)
{
	if (d_project_manager)
		return d_project_manager->open(fn, factorySettings, newProject);
	return nullptr;
}

void ApplicationWindow::openRecentProject(QAction *action)
{
	if (d_project_manager)
		d_project_manager->openRecentProject(action);
}

ApplicationWindow* ApplicationWindow::openProject(const QString& fn, bool factorySettings, bool newProject)
{
	if (d_project_manager)
		return d_project_manager->openProject(fn, factorySettings, newProject);
	return nullptr;
}

void ApplicationWindow::executeNotes()
{
	QList<Note *> autoNotes;
	QList<MdiSubWindow *> lst = projectFolder()->windowsList();
	for (MdiSubWindow *widget : lst) {
		Note *n = qobject_cast<Note *>(widget);
		if (n && n->autoexec())
			autoNotes << n;
	}
	if (autoNotes.isEmpty())
		return;

	// In non-interactive or batch test mode (-X), do not execute unprompted
	if (qApp->arguments().contains("-X"))
		return;

	if (QMessageBox::question(this, tr("Execute Notes"),
		tr("This project contains %1 note(s) configured to execute scripts automatically.\nDo you want to execute them now?").arg(autoNotes.size()),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		for (Note *n : autoNotes)
			n->executeAll();
	}
}

void ApplicationWindow::scriptError(const QString &message, const QString &scriptName, int lineNumber)
{
	if (qApp->arguments().contains("-X")) {
		fprintf(stderr, "Script Error [%s:%d]: %s\n", scriptName.toUtf8().constData(), lineNumber, message.toUtf8().constData());
		return;
	}

	QString msg = message;
	if (!scriptName.isEmpty() && lineNumber > 0)
		msg = tr("In %1, line %2:\n%3").arg(scriptName).arg(lineNumber).arg(message);
	else if (lineNumber > 0)
		msg = tr("Line %1:\n%2").arg(lineNumber).arg(message);

	QMessageBox::critical(this, tr("QtiPlot") + " - "+ tr("Script Error"), msg);
}

void ApplicationWindow::scriptPrint(const QString &text)
{
	if (qApp->arguments().contains("-X")) {
		printf("%s\n", text.toLocal8Bit().constData());
		fflush(stdout);
		return;
	}
#ifdef SCRIPTING_CONSOLE
	if(!text.trimmed().isEmpty()) console->append(text);
#else
	printf("%s\n", text.toLocal8Bit().constData());
#endif
}

bool ApplicationWindow::setScriptingLanguage(const QString &lang, bool force)
{
	if (!force && scriptEnv && lang == scriptEnv->objectName()) return true;
	if (lang.isEmpty()) return false;

	ScriptingEnv *newEnv = ScriptingLangManager::newEnv(lang.toLatin1().constData(), this);
	if (!newEnv)
		return false;

	connect(newEnv, &ScriptingEnv::error, this, &ApplicationWindow::scriptError);
	connect(newEnv, &ScriptingEnv::print, this, &ApplicationWindow::scriptPrint);
	if (!newEnv->initialize()){
		delete newEnv;
		return false;
	}

	// notify everyone who might be interested
	ScriptingChangeEvent *sce = new ScriptingChangeEvent(newEnv);
	QApplication::sendEvent(this, sce);
	delete sce;

	initCompleter();

	for (QObject *i : findChildren<QObject*>())
		QApplication::postEvent(i, new ScriptingChangeEvent(newEnv));
	if (scriptWindow)
		for (QObject *i : scriptWindow->findChildren<QObject*>())
			QApplication::postEvent(i, new ScriptingChangeEvent(newEnv));

#ifdef SCRIPTING_PYTHON
	bool python = (lang == QString("Python"));
	if (actionCommentSelection) actionCommentSelection->setEnabled(python);
	if (actionUncommentSelection) actionUncommentSelection->setEnabled(python);
#endif

	return true;
}

void ApplicationWindow::showScriptingLangDialog()
{
	ScriptingLangDialog* d = new ScriptingLangDialog(scriptEnv, this);
	d->exec();
}

void ApplicationWindow::restartScriptingEnv()
{
	if (setScriptingLanguage(scriptEnv->objectName(), true))
		executeNotes();
	else
		QMessageBox::critical(this, tr("QtiPlot - Scripting Error"),
				tr("Scripting language \"%1\" failed to initialize.").arg(scriptEnv->objectName()));
}

void ApplicationWindow::openTemplate()
{
	QString filter = tr("QtiPlot 2D Graph Template") + " (*.qpt);";
	filter += tr("QtiPlot 3D Surface Template") + " (*.qst);";
	filter += tr("QtiPlot Table Template") + " (*.qtt);";
	filter += tr("QtiPlot Matrix Template") + " (*.qmt)";

	QString fn = getFileName(this, tr("QtiPlot - Open Template File"), templatesDir, filter, 0, false);
	if (!fn.isEmpty()){
		QFileInfo fi(fn);
		templatesDir = fi.absolutePath();
		if (fn.contains(".qmt") || fn.contains(".qpt") || fn.contains(".qtt") || fn.contains(".qst"))
			openTemplate(fn);
		else {
			QMessageBox::critical(this,tr("QtiPlot - File opening error"),
					tr("The file: <b>%1</b> is not a QtiPlot template file!").arg(fn));
			return;
		}
	}
}

MdiSubWindow* ApplicationWindow::openTemplate(const QString& fn)
{
	return ProjectSerializer::openTemplate(fn, this, d_file_version);
}

void ApplicationWindow::readSettings()
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#else
	QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#endif

	d_app_settings->load(settings);

	// Synchronize local ApplicationWindow members from d_app_settings
	d_app_rect = d_app_settings->d_app_rect;
	autoSearchUpdates = d_app_settings->autoSearchUpdates;
	appLanguage = d_app_settings->appLanguage;
	show_windows_policy = (ShowWindowsPolicy)d_app_settings->show_windows_policy;
	recentProjects = d_app_settings->recentProjects;
	d_excel_import_method = (ExcelImportMethod)d_app_settings->d_excel_import_method;
	appStyle = d_app_settings->appStyle;
	autoSave = d_app_settings->autoSave;
	autoSaveTime = d_app_settings->autoSaveTime;
	d_backup_files = d_app_settings->d_backup_files;
	d_init_window_type = (WindowType)d_app_settings->d_init_window_type;
	d_completion = d_app_settings->d_completion;
	d_open_last_project = d_app_settings->d_open_last_project;
	defaultScriptingLang = d_app_settings->defaultScriptingLang;
	setLocale(d_app_settings->d_locale);
	QLocale::setDefault(d_app_settings->d_locale);
	d_decimal_digits = d_app_settings->d_decimal_digits;
	d_clipboard_locale = d_app_settings->d_clipboard_locale;
	d_muparser_c_locale = d_app_settings->d_muparser_c_locale;
	d_force_muParser = d_app_settings->d_force_muParser;
	d_matrix_undo_stack_size = d_app_settings->d_matrix_undo_stack_size;
	d_table_undo_stack_size = d_app_settings->d_table_undo_stack_size;
	d_undo_memory_budget_mb = d_app_settings->d_undo_memory_budget_mb;
	d_eol = (EndLineChar)d_app_settings->d_eol;
	appFont = d_app_settings->appFont;
	d_indexed_colors = d_app_settings->d_indexed_colors;
	d_indexed_color_names = d_app_settings->d_indexed_color_names;
	d_symbols_list = d_app_settings->d_symbols_list;
	workspaceColor = d_app_settings->workspaceColor;
	panelsColor = d_app_settings->panelsColor;
	panelsTextColor = d_app_settings->panelsTextColor;
	d_open_project_filter = d_app_settings->d_open_project_filter;
	d_latex_compiler = d_app_settings->d_latex_compiler;

	d_extended_open_dialog = d_app_settings->d_extended_open_dialog;
	d_extended_export_dialog = d_app_settings->d_extended_export_dialog;
	d_extended_import_ASCII_dialog = d_app_settings->d_extended_import_ASCII_dialog;
	d_extended_plot_dialog = d_app_settings->d_extended_plot_dialog;
	d_add_curves_dialog_size = d_app_settings->d_add_curves_dialog_size;
	d_show_current_folder = d_app_settings->d_show_current_folder;
	d_stats_significance_level = d_app_settings->d_stats_significance_level;
	d_stats_result_table = d_app_settings->d_stats_result_table;
	d_stats_result_log = d_app_settings->d_stats_result_log;
	d_stats_result_notes = d_app_settings->d_stats_result_notes;
	d_descriptive_stats = d_app_settings->d_descriptive_stats;
	d_stats_confidence = d_app_settings->d_stats_confidence;
	d_stats_power = d_app_settings->d_stats_power;
	d_stats_output = d_app_settings->d_stats_output;
	d_int_sort_data = d_app_settings->d_int_sort_data;
	d_int_show_plot = d_app_settings->d_int_show_plot;
	d_int_results_table = d_app_settings->d_int_results_table;
	d_fft_norm_amp = d_app_settings->d_fft_norm_amp;
	d_fft_shift_res = d_app_settings->d_fft_shift_res;
	d_fft_power2 = d_app_settings->d_fft_power2;

	workingDir = d_app_settings->workingDir;
	templatesDir = d_app_settings->templatesDir;
	helpFilePath = d_app_settings->helpFilePath;
	fitPluginsPath = d_app_settings->fitPluginsPath;
	asciiDirPath = d_app_settings->asciiDirPath;
	imagesDirPath = d_app_settings->imagesDirPath;
	scriptsDirPath = d_app_settings->scriptsDirPath;
	fitModelsPath = d_app_settings->fitModelsPath;
	customActionsDirPath = d_app_settings->customActionsDirPath;
	d_translations_folder = d_app_settings->d_translations_folder;
	d_python_config_folder = d_app_settings->d_python_config_folder;
	d_latex_compiler_path = d_app_settings->d_latex_compiler_path;
	d_startup_scripts_folder = d_app_settings->d_startup_scripts_folder;
	d_soffice_path = d_app_settings->d_soffice_path;
	d_java_path = d_app_settings->d_java_path;
	d_jodconverter_path = d_app_settings->d_jodconverter_path;

	surfaceFunc = d_app_settings->surfaceFunc;
	xFunctions = d_app_settings->xFunctions;
	yFunctions = d_app_settings->yFunctions;
	rFunctions = d_app_settings->rFunctions;
	thetaFunctions = d_app_settings->thetaFunctions;
	d_param_surface_func = d_app_settings->d_param_surface_func;
	d_recent_functions = d_app_settings->d_recent_functions;

	confirmCloseFolder = d_app_settings->confirmCloseFolder;
	confirmCloseTable = d_app_settings->confirmCloseTable;
	confirmCloseMatrix = d_app_settings->confirmCloseMatrix;
	confirmClosePlot2D = d_app_settings->confirmClosePlot2D;
	confirmClosePlot3D = d_app_settings->confirmClosePlot3D;
	confirmCloseNotes = d_app_settings->confirmCloseNotes;
	d_inform_rename_table = d_app_settings->d_inform_rename_table;
	d_confirm_overwrite = d_app_settings->d_confirm_overwrite;
	d_ask_web_connection = d_app_settings->d_ask_web_connection;
	d_confirm_modif_2D_points = d_app_settings->d_confirm_modif_2D_points;

	d_show_table_comments = d_app_settings->d_show_table_comments;
	d_auto_update_table_values = d_app_settings->d_auto_update_table_values;
	d_show_table_paste_dialog = d_app_settings->d_show_table_paste_dialog;
	tableTextFont = d_app_settings->tableTextFont;
	tableHeaderFont = d_app_settings->tableHeaderFont;
	tableBkgdColor = d_app_settings->tableBkgdColor;
	tableTextColor = d_app_settings->tableTextColor;
	tableHeaderColor = d_app_settings->tableHeaderColor;

	titleOn = d_app_settings->titleOn;
	canvasFrameWidth = d_app_settings->canvasFrameWidth;
	defaultPlotMargin = d_app_settings->defaultPlotMargin;
	drawBackbones = d_app_settings->drawBackbones;
	axesLineWidth = d_app_settings->axesLineWidth;
	autoscale2DPlots = d_app_settings->autoscale2DPlots;
	autoScaleFonts = d_app_settings->autoScaleFonts;
	autoResizeLayers = d_app_settings->autoResizeLayers;
	antialiasing2DPlots = d_app_settings->antialiasing2DPlots;
	d_scale_plots_on_print = d_app_settings->d_scale_plots_on_print;
	d_print_cropmarks = d_app_settings->d_print_cropmarks;
	d_layer_geometry_unit = d_app_settings->d_layer_geometry_unit;
	d_layer_canvas_width = d_app_settings->d_layer_canvas_width;
	d_layer_canvas_height = d_app_settings->d_layer_canvas_height;
	plotAxesFont = d_app_settings->plotAxesFont;
	plotNumbersFont = d_app_settings->plotNumbersFont;
	plotLegendFont = d_app_settings->plotLegendFont;
	plotTitleFont = d_app_settings->plotTitleFont;
	d_in_place_editing = d_app_settings->d_in_place_editing;
	d_graph_background_color = d_app_settings->d_graph_background_color;
	d_graph_canvas_color = d_app_settings->d_graph_canvas_color;
	d_graph_border_color = d_app_settings->d_graph_border_color;
	d_graph_background_opacity = d_app_settings->d_graph_background_opacity;
	d_graph_canvas_opacity = d_app_settings->d_graph_canvas_opacity;
	d_graph_border_width = d_app_settings->d_graph_border_width;
	d_canvas_frame_color = d_app_settings->d_canvas_frame_color;
	d_graph_axes_labels_dist = d_app_settings->d_graph_axes_labels_dist;
	d_graph_tick_labels_dist = d_app_settings->d_graph_tick_labels_dist;
	d_show_axes = d_app_settings->d_show_axes;
	d_show_axes_labels = d_app_settings->d_show_axes_labels;
	d_graph_legend_display = d_app_settings->d_graph_legend_display;
	d_graph_axis_labeling = d_app_settings->d_graph_axis_labeling;
	d_keep_aspect_ration = d_app_settings->d_keep_aspect_ration;
	d_synchronize_graph_scales = d_app_settings->d_synchronize_graph_scales;
	d_show_empty_cell_gap = d_app_settings->d_show_empty_cell_gap;
	d_graph_attach_policy = d_app_settings->d_graph_attach_policy;
	defaultCurveStyle = d_app_settings->defaultCurveStyle;
	defaultCurveLineWidth = d_app_settings->defaultCurveLineWidth;
	d_curve_line_style = d_app_settings->d_curve_line_style;
	defaultSymbolSize = d_app_settings->defaultSymbolSize;
	defaultSymbolEdge = d_app_settings->defaultSymbolEdge;
	d_fill_symbols = d_app_settings->d_fill_symbols;
	d_symbol_style = d_app_settings->d_symbol_style;
	d_indexed_symbols = d_app_settings->d_indexed_symbols;
	defaultCurveBrush = d_app_settings->defaultCurveBrush;
	defaultCurveAlpha = d_app_settings->defaultCurveAlpha;
	d_disable_curve_antialiasing = d_app_settings->d_disable_curve_antialiasing;
	d_curve_max_antialising_size = d_app_settings->d_curve_max_antialising_size;
	d_Douglas_Peuker_tolerance = d_app_settings->d_Douglas_Peuker_tolerance;
	d_decimation_method = d_app_settings->d_decimation_method;
	d_speed_mode_points = d_app_settings->d_speed_mode_points;
	d_speed_mode_export = d_app_settings->d_speed_mode_export;
	majTicksStyle = d_app_settings->majTicksStyle;
	minTicksStyle = d_app_settings->minTicksStyle;
	minTicksLength = d_app_settings->minTicksLength;
	majTicksLength = d_app_settings->majTicksLength;
	legendFrameStyle = d_app_settings->legendFrameStyle;
	d_frame_widget_pen = d_app_settings->d_frame_widget_pen;
	legendTextColor = d_app_settings->legendTextColor;
	legendBackground = d_app_settings->legendBackground;
	d_legend_default_angle = d_app_settings->d_legend_default_angle;
	d_frame_geometry_unit = d_app_settings->d_frame_geometry_unit;
	defaultArrowLineWidth = d_app_settings->defaultArrowLineWidth;
	defaultArrowColor = d_app_settings->defaultArrowColor;
	defaultArrowHeadLength = d_app_settings->defaultArrowHeadLength;
	defaultArrowHeadAngle = d_app_settings->defaultArrowHeadAngle;
	defaultArrowHeadFill = d_app_settings->defaultArrowHeadFill;
	defaultArrowLineStyle = d_app_settings->defaultArrowLineStyle;
	d_rect_default_background = d_app_settings->d_rect_default_background;
	d_rect_default_brush = d_app_settings->d_rect_default_brush;

	// Grid
	d_default_2D_grid->setRenderHint(QwtPlotItem::RenderAntialiased, d_app_settings->d_grid_antialiased);
	d_default_2D_grid->enableX(d_app_settings->d_grid_major_x_enabled);
	QPen pen = d_default_2D_grid->majPenX();
	pen.setColor(d_app_settings->d_grid_major_x_color);
	pen.setStyle(PenStyleBox::penStyle(d_app_settings->d_grid_major_x_style));
	pen.setWidthF(d_app_settings->d_grid_major_x_thickness);
	d_default_2D_grid->setMajPenX(pen);

	d_default_2D_grid->enableXMin(d_app_settings->d_grid_minor_x_enabled);
	pen = d_default_2D_grid->minPenX();
	pen.setColor(d_app_settings->d_grid_minor_x_color);
	pen.setStyle(PenStyleBox::penStyle(d_app_settings->d_grid_minor_x_style));
	pen.setWidthF(d_app_settings->d_grid_minor_x_thickness);
	d_default_2D_grid->setMinPenX(pen);

	d_default_2D_grid->enableY(d_app_settings->d_grid_major_y_enabled);
	pen = d_default_2D_grid->majPenY();
	pen.setColor(d_app_settings->d_grid_major_y_color);
	pen.setStyle(PenStyleBox::penStyle(d_app_settings->d_grid_major_y_style));
	pen.setWidthF(d_app_settings->d_grid_major_y_thickness);
	d_default_2D_grid->setMajPenY(pen);

	d_default_2D_grid->enableYMin(d_app_settings->d_grid_minor_y_enabled);
	pen = d_default_2D_grid->minPenY();
	pen.setColor(d_app_settings->d_grid_minor_y_color);
	pen.setStyle(PenStyleBox::penStyle(d_app_settings->d_grid_minor_y_style));
	pen.setWidthF(d_app_settings->d_grid_minor_y_thickness);
	d_default_2D_grid->setMinPenY(pen);

	// 3D Plots
	d_3D_legend = d_app_settings->d_3D_legend;
	d_3D_projection = d_app_settings->d_3D_projection;
	d_3D_shading = d_app_settings->d_3D_shading;
	d_3D_smooth_mesh = d_app_settings->d_3D_smooth_mesh;
	d_3D_resolution = d_app_settings->d_3D_resolution;
	d_3D_orthogonal = d_app_settings->d_3D_orthogonal;
	d_3D_autoscale = d_app_settings->d_3D_autoscale;
	d_3D_scale_fonts = d_app_settings->d_3D_scale_fonts;
	d_3D_title_font = d_app_settings->d_3D_title_font;
	d_3D_numbers_font = d_app_settings->d_3D_numbers_font;
	d_3D_axes_font = d_app_settings->d_3D_axes_font;
	d_3D_labels_color = d_app_settings->d_3D_labels_color;
	d_3D_mesh_color = d_app_settings->d_3D_mesh_color;
	d_3D_numbers_color = d_app_settings->d_3D_numbers_color;
	d_3D_axes_color = d_app_settings->d_3D_axes_color;
	d_3D_background_color = d_app_settings->d_3D_background_color;
	d_3D_color_map = d_app_settings->d_3D_color_map;
	d_3D_major_grids = d_app_settings->d_3D_major_grids;
	d_3D_grid_color = d_app_settings->d_3D_grid_color;
	d_3D_major_style = d_app_settings->d_3D_major_style;
	d_3D_major_width = d_app_settings->d_3D_major_width;
	d_3D_minor_grids = d_app_settings->d_3D_minor_grids;
	d_3D_minor_grid_color = d_app_settings->d_3D_minor_grid_color;
	d_3D_minor_style = d_app_settings->d_3D_minor_style;
	d_3D_minor_width = d_app_settings->d_3D_minor_width;

	// Fitting
	fit_output_precision = d_app_settings->fit_output_precision;
	pasteFitResultsToPlot = d_app_settings->pasteFitResultsToPlot;
	d_write_fit_results_to_log = d_app_settings->d_write_fit_results_to_log;
	generateUniformFitPoints = d_app_settings->generateUniformFitPoints;
	fitPoints = d_app_settings->fitPoints;
	generatePeakCurves = d_app_settings->generatePeakCurves;
	peakCurvesColor = d_app_settings->peakCurvesColor;
	fit_scale_errors = d_app_settings->fit_scale_errors;
	d_2_linear_fit_points = d_app_settings->d_2_linear_fit_points;
	d_multi_peak_messages = d_app_settings->d_multi_peak_messages;

	// ASCII Import/Export
	columnSeparator = d_app_settings->columnSeparator;
	ignoredLines = d_app_settings->ignoredLines;
	renameColumns = d_app_settings->renameColumns;
	strip_spaces = d_app_settings->strip_spaces;
	simplify_spaces = d_app_settings->simplify_spaces;
	d_ASCII_file_filter = d_app_settings->d_ASCII_file_filter;
	d_ASCII_import_locale = d_app_settings->d_ASCII_import_locale;
	d_ASCII_import_mode = d_app_settings->d_ASCII_import_mode;
	d_ASCII_comment_string = d_app_settings->d_ASCII_comment_string;
	d_ASCII_import_comments = d_app_settings->d_ASCII_import_comments;
	d_ASCII_import_read_only = d_app_settings->d_ASCII_import_read_only;
	d_ASCII_import_preview = d_app_settings->d_ASCII_import_preview;
	d_preview_lines = d_app_settings->d_preview_lines;
	d_ASCII_end_line = (EndLineChar)d_app_settings->d_ASCII_end_line;
	d_ASCII_import_first_row_role = d_app_settings->d_ASCII_import_first_row_role;
	d_import_ASCII_dialog_size = d_app_settings->d_import_ASCII_dialog_size;

	d_export_col_separator = d_app_settings->d_export_col_separator;
	d_export_col_names = d_app_settings->d_export_col_names;
	d_export_col_comment = d_app_settings->d_export_col_comment;
	d_export_table_selection = d_app_settings->d_export_table_selection;
	d_export_ASCII_file_filter = d_app_settings->d_export_ASCII_file_filter;

	// Image Export
	d_image_export_filter = d_app_settings->d_image_export_filter;
	d_export_transparency = d_app_settings->d_export_transparency;
	d_export_quality = d_app_settings->d_export_quality;
	d_export_vector_resolution = d_app_settings->d_export_vector_resolution;
	d_export_color = d_app_settings->d_export_color;
	d_3D_export_text_mode = d_app_settings->d_3D_export_text_mode;
	d_3D_export_sort = d_app_settings->d_3D_export_sort;
	d_export_bitmap_resolution = d_app_settings->d_export_bitmap_resolution;
	d_export_raster_size = d_app_settings->d_export_raster_size;
	d_export_size_unit = d_app_settings->d_export_size_unit;
	d_scale_fonts_factor = d_app_settings->d_scale_fonts_factor;
	d_export_escape_tex_strings = d_app_settings->d_export_escape_tex_strings;
	d_export_tex_font_sizes = d_app_settings->d_export_tex_font_sizes;
	d_export_compression = d_app_settings->d_export_compression;

	// Script Window & Notes
	d_script_win_on_top = d_app_settings->d_script_win_on_top;
	d_script_win_rect = d_app_settings->d_script_win_rect;
	d_note_line_numbers = d_app_settings->d_note_line_numbers;
	d_notes_tab_length = d_app_settings->d_notes_tab_length;
	d_notes_font = d_app_settings->d_notes_font;
	d_comment_highlight_color = d_app_settings->d_comment_highlight_color;
	d_keyword_highlight_color = d_app_settings->d_keyword_highlight_color;
	d_quotation_highlight_color = d_app_settings->d_quotation_highlight_color;
	d_numeric_highlight_color = d_app_settings->d_numeric_highlight_color;
	d_function_highlight_color = d_app_settings->d_function_highlight_color;
	d_class_highlight_color = d_app_settings->d_class_highlight_color;

	// ToolBars
	d_file_tool_bar = d_app_settings->d_file_tool_bar;
	d_edit_tool_bar = d_app_settings->d_edit_tool_bar;
	d_table_tool_bar = d_app_settings->d_table_tool_bar;
	d_column_tool_bar = d_app_settings->d_column_tool_bar;
	d_matrix_tool_bar = d_app_settings->d_matrix_tool_bar;
	d_plot_tool_bar = d_app_settings->d_plot_tool_bar;
	d_plot3D_tool_bar = d_app_settings->d_plot3D_tool_bar;
	d_display_tool_bar = d_app_settings->d_display_tool_bar;
	d_format_tool_bar = d_app_settings->d_format_tool_bar;
	d_notes_tool_bar = d_app_settings->d_notes_tool_bar;

	d_print_paper_size = d_app_settings->d_print_paper_size;
	d_printer_orientation = d_app_settings->d_printer_orientation;

	// GUI state restores
	updateRecentProjectsList();
	changeAppStyle(appStyle);

	if (!qApp->arguments().contains("-X"))
		restoreState(settings.value("/General/DockWindows").toByteArray());
	explorerSplitter->restoreState(settings.value("/General/ExplorerSplitter").toByteArray());
	QList<int> lst = explorerSplitter->sizes();
	for (int i=0; i< lst.count(); i++){
		if (lst[i] == 0){
			lst[i] = 45;
			explorerSplitter->setSizes(lst);
		}
	}

	if (!d_app_settings->d_proxy_host.isEmpty()){
		QNetworkProxy proxy;
		proxy.setType(QNetworkProxy::NoProxy);
		proxy.setHostName(d_app_settings->d_proxy_host);
		proxy.setPort(d_app_settings->d_proxy_port);
		proxy.setUser(d_app_settings->d_proxy_user);
		QNetworkProxy::setApplicationProxy(proxy);
	}
}

void ApplicationWindow::saveSettings()
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#else
	QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#endif

	// Sync current state into d_app_settings
	d_app_rect = QRect(this->pos(), this->size());
	if (this->isMaximized())
		d_app_rect = QRect();
	d_app_settings->d_app_rect = d_app_rect;

	d_app_settings->autoSearchUpdates = autoSearchUpdates;
	d_app_settings->appLanguage = appLanguage;
	d_app_settings->show_windows_policy = (ApplicationSettings::ShowWindowsPolicy)show_windows_policy;
	d_app_settings->recentProjects = recentProjects;
	d_app_settings->d_excel_import_method = (ApplicationSettings::ExcelImportMethod)d_excel_import_method;
	d_app_settings->appStyle = appStyle;
	d_app_settings->autoSave = autoSave;
	d_app_settings->autoSaveTime = autoSaveTime;
	d_app_settings->d_backup_files = d_backup_files;
	d_app_settings->d_init_window_type = (ApplicationSettings::WindowType)d_init_window_type;
	d_app_settings->d_completion = d_completion;
	d_app_settings->d_open_last_project = d_open_last_project;
	d_app_settings->defaultScriptingLang = defaultScriptingLang;
	d_app_settings->d_locale = locale();
	d_app_settings->d_decimal_digits = d_decimal_digits;
	d_app_settings->d_clipboard_locale = d_clipboard_locale;
	d_app_settings->d_muparser_c_locale = d_muparser_c_locale;
	d_app_settings->d_force_muParser = d_force_muParser;
	d_app_settings->d_matrix_undo_stack_size = d_matrix_undo_stack_size;
	d_app_settings->d_table_undo_stack_size = d_table_undo_stack_size;
	d_app_settings->d_undo_memory_budget_mb = d_undo_memory_budget_mb;
	d_app_settings->d_eol = (ApplicationSettings::EndLineChar)d_eol;
	d_app_settings->appFont = appFont;
	d_app_settings->d_indexed_colors = d_indexed_colors;
	d_app_settings->d_indexed_color_names = d_indexed_color_names;
	d_app_settings->d_symbols_list = d_symbols_list;
	d_app_settings->workspaceColor = workspaceColor;
	d_app_settings->panelsColor = panelsColor;
	d_app_settings->panelsTextColor = panelsTextColor;
	d_app_settings->d_open_project_filter = d_open_project_filter;
	d_app_settings->d_latex_compiler = d_latex_compiler;

	d_app_settings->d_extended_open_dialog = d_extended_open_dialog;
	d_app_settings->d_extended_export_dialog = d_extended_export_dialog;
	d_app_settings->d_extended_import_ASCII_dialog = d_extended_import_ASCII_dialog;
	d_app_settings->d_extended_plot_dialog = d_extended_plot_dialog;
	d_app_settings->d_add_curves_dialog_size = d_add_curves_dialog_size;
	d_app_settings->d_show_current_folder = d_show_current_folder;
	d_app_settings->d_stats_significance_level = d_stats_significance_level;
	d_app_settings->d_stats_result_table = d_stats_result_table;
	d_app_settings->d_stats_result_log = d_stats_result_log;
	d_app_settings->d_stats_result_notes = d_stats_result_notes;
	d_app_settings->d_descriptive_stats = d_descriptive_stats;
	d_app_settings->d_stats_confidence = d_stats_confidence;
	d_app_settings->d_stats_power = d_stats_power;
	d_app_settings->d_stats_output = d_stats_output;
	d_app_settings->d_int_sort_data = d_int_sort_data;
	d_app_settings->d_int_show_plot = d_int_show_plot;
	d_app_settings->d_int_results_table = d_int_results_table;
	d_app_settings->d_fft_norm_amp = d_fft_norm_amp;
	d_app_settings->d_fft_shift_res = d_fft_shift_res;
	d_app_settings->d_fft_power2 = d_fft_power2;

	d_app_settings->workingDir = workingDir;
	d_app_settings->templatesDir = templatesDir;
	d_app_settings->helpFilePath = helpFilePath;
	d_app_settings->fitPluginsPath = fitPluginsPath;
	d_app_settings->asciiDirPath = asciiDirPath;
	d_app_settings->imagesDirPath = imagesDirPath;
	d_app_settings->scriptsDirPath = scriptsDirPath;
	d_app_settings->fitModelsPath = fitModelsPath;
	d_app_settings->customActionsDirPath = customActionsDirPath;
	d_app_settings->d_translations_folder = d_translations_folder;
	d_app_settings->d_python_config_folder = d_python_config_folder;
	d_app_settings->d_latex_compiler_path = d_latex_compiler_path;
	d_app_settings->d_startup_scripts_folder = d_startup_scripts_folder;
	d_app_settings->d_soffice_path = d_soffice_path;
	d_app_settings->d_java_path = d_java_path;
	d_app_settings->d_jodconverter_path = d_jodconverter_path;

	d_app_settings->surfaceFunc = surfaceFunc;
	d_app_settings->xFunctions = xFunctions;
	d_app_settings->yFunctions = yFunctions;
	d_app_settings->rFunctions = rFunctions;
	d_app_settings->thetaFunctions = thetaFunctions;
	d_app_settings->d_param_surface_func = d_param_surface_func;
	d_app_settings->d_recent_functions = d_recent_functions;

	d_app_settings->confirmCloseFolder = confirmCloseFolder;
	d_app_settings->confirmCloseTable = confirmCloseTable;
	d_app_settings->confirmCloseMatrix = confirmCloseMatrix;
	d_app_settings->confirmClosePlot2D = confirmClosePlot2D;
	d_app_settings->confirmClosePlot3D = confirmClosePlot3D;
	d_app_settings->confirmCloseNotes = confirmCloseNotes;
	d_app_settings->d_inform_rename_table = d_inform_rename_table;
	d_app_settings->d_confirm_overwrite = d_confirm_overwrite;
	d_app_settings->d_ask_web_connection = d_ask_web_connection;
	d_app_settings->d_confirm_modif_2D_points = d_confirm_modif_2D_points;

	d_app_settings->d_show_table_comments = d_show_table_comments;
	d_app_settings->d_auto_update_table_values = d_auto_update_table_values;
	d_app_settings->d_show_table_paste_dialog = d_show_table_paste_dialog;
	d_app_settings->tableTextFont = tableTextFont;
	d_app_settings->tableHeaderFont = tableHeaderFont;
	d_app_settings->tableBkgdColor = tableBkgdColor;
	d_app_settings->tableTextColor = tableTextColor;
	d_app_settings->tableHeaderColor = tableHeaderColor;

	d_app_settings->titleOn = titleOn;
	d_app_settings->canvasFrameWidth = canvasFrameWidth;
	d_app_settings->defaultPlotMargin = defaultPlotMargin;
	d_app_settings->drawBackbones = drawBackbones;
	d_app_settings->axesLineWidth = axesLineWidth;
	d_app_settings->autoscale2DPlots = autoscale2DPlots;
	d_app_settings->autoScaleFonts = autoScaleFonts;
	d_app_settings->autoResizeLayers = autoResizeLayers;
	d_app_settings->antialiasing2DPlots = antialiasing2DPlots;
	d_app_settings->d_scale_plots_on_print = d_scale_plots_on_print;
	d_app_settings->d_print_cropmarks = d_print_cropmarks;
	d_app_settings->d_layer_geometry_unit = d_layer_geometry_unit;
	d_app_settings->d_layer_canvas_width = d_layer_canvas_width;
	d_app_settings->d_layer_canvas_height = d_layer_canvas_height;
	d_app_settings->plotAxesFont = plotAxesFont;
	d_app_settings->plotNumbersFont = plotNumbersFont;
	d_app_settings->plotLegendFont = plotLegendFont;
	d_app_settings->plotTitleFont = plotTitleFont;
	d_app_settings->d_in_place_editing = d_in_place_editing;
	d_app_settings->d_graph_background_color = d_graph_background_color;
	d_app_settings->d_graph_canvas_color = d_graph_canvas_color;
	d_app_settings->d_graph_border_color = d_graph_border_color;
	d_app_settings->d_graph_background_opacity = d_graph_background_opacity;
	d_app_settings->d_graph_canvas_opacity = d_graph_canvas_opacity;
	d_app_settings->d_graph_border_width = d_graph_border_width;
	d_app_settings->d_canvas_frame_color = d_canvas_frame_color;
	d_app_settings->d_graph_axes_labels_dist = d_graph_axes_labels_dist;
	d_app_settings->d_graph_tick_labels_dist = d_graph_tick_labels_dist;
	d_app_settings->d_show_axes = d_show_axes;
	d_app_settings->d_show_axes_labels = d_show_axes_labels;
	d_app_settings->d_graph_legend_display = d_graph_legend_display;
	d_app_settings->d_graph_axis_labeling = d_graph_axis_labeling;
	d_app_settings->d_keep_aspect_ration = d_keep_aspect_ration;
	d_app_settings->d_synchronize_graph_scales = d_synchronize_graph_scales;
	d_app_settings->d_show_empty_cell_gap = d_show_empty_cell_gap;
	d_app_settings->d_graph_attach_policy = d_graph_attach_policy;
	d_app_settings->defaultCurveStyle = defaultCurveStyle;
	d_app_settings->defaultCurveLineWidth = defaultCurveLineWidth;
	d_app_settings->d_curve_line_style = d_curve_line_style;
	d_app_settings->defaultSymbolSize = defaultSymbolSize;
	d_app_settings->defaultSymbolEdge = defaultSymbolEdge;
	d_app_settings->d_fill_symbols = d_fill_symbols;
	d_app_settings->d_symbol_style = d_symbol_style;
	d_app_settings->d_indexed_symbols = d_indexed_symbols;
	d_app_settings->defaultCurveBrush = defaultCurveBrush;
	d_app_settings->defaultCurveAlpha = defaultCurveAlpha;
	d_app_settings->d_disable_curve_antialiasing = d_disable_curve_antialiasing;
	d_app_settings->d_curve_max_antialising_size = d_curve_max_antialising_size;
	d_app_settings->d_Douglas_Peuker_tolerance = d_Douglas_Peuker_tolerance;
	d_app_settings->d_decimation_method = d_decimation_method;
	d_app_settings->d_speed_mode_points = d_speed_mode_points;
	d_app_settings->d_speed_mode_export = d_speed_mode_export;
	d_app_settings->majTicksStyle = majTicksStyle;
	d_app_settings->minTicksStyle = minTicksStyle;
	d_app_settings->minTicksLength = minTicksLength;
	d_app_settings->majTicksLength = majTicksLength;
	d_app_settings->legendFrameStyle = legendFrameStyle;
	d_app_settings->d_frame_widget_pen = d_frame_widget_pen;
	d_app_settings->legendTextColor = legendTextColor;
	d_app_settings->legendBackground = legendBackground;
	d_app_settings->d_legend_default_angle = d_legend_default_angle;
	d_app_settings->d_frame_geometry_unit = d_frame_geometry_unit;
	d_app_settings->defaultArrowLineWidth = defaultArrowLineWidth;
	d_app_settings->defaultArrowColor = defaultArrowColor;
	d_app_settings->defaultArrowHeadLength = defaultArrowHeadLength;
	d_app_settings->defaultArrowHeadAngle = defaultArrowHeadAngle;
	d_app_settings->defaultArrowHeadFill = defaultArrowHeadFill;
	d_app_settings->defaultArrowLineStyle = defaultArrowLineStyle;
	d_app_settings->d_rect_default_background = d_rect_default_background;
	d_app_settings->d_rect_default_brush = d_rect_default_brush;

	d_app_settings->d_grid_antialiased = d_default_2D_grid->testRenderHint(QwtPlotItem::RenderAntialiased);
	d_app_settings->d_grid_major_x_enabled = d_default_2D_grid->xEnabled();
	d_app_settings->d_grid_major_x_color = d_default_2D_grid->majPenX().color();
	d_app_settings->d_grid_major_x_style = PenStyleBox::styleIndex(d_default_2D_grid->majPenX().style());
	d_app_settings->d_grid_major_x_thickness = d_default_2D_grid->majPenX().widthF();
	d_app_settings->d_grid_minor_x_enabled = d_default_2D_grid->xMinEnabled();
	d_app_settings->d_grid_minor_x_color = d_default_2D_grid->minPenX().color();
	d_app_settings->d_grid_minor_x_style = PenStyleBox::styleIndex(d_default_2D_grid->minPenX().style());
	d_app_settings->d_grid_minor_x_thickness = d_default_2D_grid->minPenX().widthF();
	d_app_settings->d_grid_major_y_enabled = d_default_2D_grid->yEnabled();
	d_app_settings->d_grid_major_y_color = d_default_2D_grid->majPenY().color();
	d_app_settings->d_grid_major_y_style = PenStyleBox::styleIndex(d_default_2D_grid->majPenY().style());
	d_app_settings->d_grid_major_y_thickness = d_default_2D_grid->majPenY().widthF();
	d_app_settings->d_grid_minor_y_enabled = d_default_2D_grid->yMinEnabled();
	d_app_settings->d_grid_minor_y_color = d_default_2D_grid->minPenY().color();
	d_app_settings->d_grid_minor_y_style = PenStyleBox::styleIndex(d_default_2D_grid->minPenY().style());
	d_app_settings->d_grid_minor_y_thickness = d_default_2D_grid->minPenY().widthF();

	d_app_settings->d_3D_legend = d_3D_legend;
	d_app_settings->d_3D_projection = d_3D_projection;
	d_app_settings->d_3D_shading = d_3D_shading;
	d_app_settings->d_3D_smooth_mesh = d_3D_smooth_mesh;
	d_app_settings->d_3D_resolution = d_3D_resolution;
	d_app_settings->d_3D_orthogonal = d_3D_orthogonal;
	d_app_settings->d_3D_autoscale = d_3D_autoscale;
	d_app_settings->d_3D_scale_fonts = d_3D_scale_fonts;
	d_app_settings->d_3D_title_font = d_3D_title_font;
	d_app_settings->d_3D_numbers_font = d_3D_numbers_font;
	d_app_settings->d_3D_axes_font = d_3D_axes_font;
	d_app_settings->d_3D_labels_color = d_3D_labels_color;
	d_app_settings->d_3D_mesh_color = d_3D_mesh_color;
	d_app_settings->d_3D_numbers_color = d_3D_numbers_color;
	d_app_settings->d_3D_axes_color = d_3D_axes_color;
	d_app_settings->d_3D_background_color = d_3D_background_color;
	d_app_settings->d_3D_color_map = d_3D_color_map;
	d_app_settings->d_3D_major_grids = d_3D_major_grids;
	d_app_settings->d_3D_grid_color = d_3D_grid_color;
	d_app_settings->d_3D_major_style = d_3D_major_style;
	d_app_settings->d_3D_major_width = d_3D_major_width;
	d_app_settings->d_3D_minor_grids = d_3D_minor_grids;
	d_app_settings->d_3D_minor_grid_color = d_3D_minor_grid_color;
	d_app_settings->d_3D_minor_style = d_3D_minor_style;
	d_app_settings->d_3D_minor_width = d_3D_minor_width;

	d_app_settings->fit_output_precision = fit_output_precision;
	d_app_settings->pasteFitResultsToPlot = pasteFitResultsToPlot;
	d_app_settings->d_write_fit_results_to_log = d_write_fit_results_to_log;
	d_app_settings->generateUniformFitPoints = generateUniformFitPoints;
	d_app_settings->fitPoints = fitPoints;
	d_app_settings->generatePeakCurves = generatePeakCurves;
	d_app_settings->peakCurvesColor = peakCurvesColor;
	d_app_settings->fit_scale_errors = fit_scale_errors;
	d_app_settings->d_2_linear_fit_points = d_2_linear_fit_points;
	d_app_settings->d_multi_peak_messages = d_multi_peak_messages;

	d_app_settings->columnSeparator = columnSeparator;
	d_app_settings->ignoredLines = ignoredLines;
	d_app_settings->renameColumns = renameColumns;
	d_app_settings->strip_spaces = strip_spaces;
	d_app_settings->simplify_spaces = simplify_spaces;
	d_app_settings->d_ASCII_file_filter = d_ASCII_file_filter;
	d_app_settings->d_ASCII_import_locale = d_ASCII_import_locale;
	d_app_settings->d_ASCII_import_mode = d_ASCII_import_mode;
	d_app_settings->d_ASCII_comment_string = d_ASCII_comment_string;
	d_app_settings->d_ASCII_import_comments = d_ASCII_import_comments;
	d_app_settings->d_ASCII_import_read_only = d_ASCII_import_read_only;
	d_app_settings->d_ASCII_import_preview = d_ASCII_import_preview;
	d_app_settings->d_preview_lines = d_preview_lines;
	d_app_settings->d_ASCII_end_line = (ApplicationSettings::EndLineChar)d_ASCII_end_line;
	d_app_settings->d_ASCII_import_first_row_role = d_ASCII_import_first_row_role;
	d_app_settings->d_import_ASCII_dialog_size = d_import_ASCII_dialog_size;

	d_app_settings->d_export_col_separator = d_export_col_separator;
	d_app_settings->d_export_col_names = d_export_col_names;
	d_app_settings->d_export_col_comment = d_export_col_comment;
	d_app_settings->d_export_table_selection = d_export_table_selection;
	d_app_settings->d_export_ASCII_file_filter = d_export_ASCII_file_filter;

	d_app_settings->d_image_export_filter = d_image_export_filter;
	d_app_settings->d_export_transparency = d_export_transparency;
	d_app_settings->d_export_quality = d_export_quality;
	d_app_settings->d_export_vector_resolution = d_export_vector_resolution;
	d_app_settings->d_export_color = d_export_color;
	d_app_settings->d_3D_export_text_mode = d_3D_export_text_mode;
	d_app_settings->d_3D_export_sort = d_3D_export_sort;
	d_app_settings->d_export_bitmap_resolution = d_export_bitmap_resolution;
	d_app_settings->d_export_raster_size = d_export_raster_size;
	d_app_settings->d_export_size_unit = d_export_size_unit;
	d_app_settings->d_scale_fonts_factor = d_scale_fonts_factor;
	d_app_settings->d_export_escape_tex_strings = d_export_escape_tex_strings;
	d_app_settings->d_export_tex_font_sizes = d_export_tex_font_sizes;
	d_app_settings->d_export_compression = d_export_compression;

	d_app_settings->d_script_win_on_top = d_script_win_on_top;
	d_app_settings->d_script_win_rect = d_script_win_rect;
	d_app_settings->d_note_line_numbers = d_note_line_numbers;
	d_app_settings->d_notes_tab_length = d_notes_tab_length;
	d_app_settings->d_notes_font = d_notes_font;
	d_app_settings->d_comment_highlight_color = d_comment_highlight_color;
	d_app_settings->d_keyword_highlight_color = d_keyword_highlight_color;
	d_app_settings->d_quotation_highlight_color = d_quotation_highlight_color;
	d_app_settings->d_numeric_highlight_color = d_numeric_highlight_color;
	d_app_settings->d_function_highlight_color = d_function_highlight_color;
	d_app_settings->d_class_highlight_color = d_class_highlight_color;

	d_app_settings->d_file_tool_bar = d_file_tool_bar;
	d_app_settings->d_edit_tool_bar = d_edit_tool_bar;
	d_app_settings->d_table_tool_bar = d_table_tool_bar;
	d_app_settings->d_column_tool_bar = d_column_tool_bar;
	d_app_settings->d_matrix_tool_bar = d_matrix_tool_bar;
	d_app_settings->d_plot_tool_bar = d_plot_tool_bar;
	d_app_settings->d_plot3D_tool_bar = d_plot3D_tool_bar;
	d_app_settings->d_display_tool_bar = d_display_tool_bar;
	d_app_settings->d_format_tool_bar = d_format_tool_bar;
	d_app_settings->d_notes_tool_bar = d_notes_tool_bar;

	d_app_settings->d_print_paper_size = d_print_paper_size;
	d_app_settings->d_printer_orientation = d_printer_orientation;

	QNetworkProxy proxy = QNetworkProxy::applicationProxy();
	d_app_settings->d_proxy_host = proxy.hostName();
	d_app_settings->d_proxy_port = proxy.port();
	d_app_settings->d_proxy_user = proxy.user();

	// Save all values through ApplicationSettings
	d_app_settings->save(settings);

	// Save GUI state (dock windows & splitter)
	settings.beginGroup("/General");
	settings.setValue("/DockWindows", saveState());
	settings.setValue("/ExplorerSplitter", explorerSplitter->saveState());
	settings.endGroup();
}





QString ApplicationWindow::windowGeometryInfo(MdiSubWindow *w)
{
	QString s = "geometry\t";
	if (w->status() == MdiSubWindow::Maximized){
		if (w == w->folder()->activeWindow())
			return s + "maximized\tactive\n";
		else
			return s + "maximized\n";
	}

	s += QString::number(w->x()) + "\t";
	s += QString::number(w->y()) + "\t";

	QSize restoreSize = w->restoreSize();
	if (w->status() == MdiSubWindow::Hidden &&
		restoreSize.width() > w->width() &&
		restoreSize.height() > w->height()){
		// the window was minimized and afterwards hidden
		s += QString::number(restoreSize.width()) + "\t";
		s += QString::number(restoreSize.height()) + "\t";
	} else if (w->status() != MdiSubWindow::Minimized){
		s += QString::number(w->width()) + "\t";
		s += QString::number(w->height()) + "\t";
	} else {
		s += QString::number(restoreSize.width()) + "\t";
		s += QString::number(restoreSize.height()) + "\t";
		s += "minimized\t";
	}

	bool hide = hidden(w);
	if (w == w->folder()->activeWindow() && !hide)
		s += "active\n";
	else if(hide)
		s += "hidden\n";
	else
		s += "\n";
	return s;
}

void ApplicationWindow::restoreWindowGeometry(MdiSubWindow *w, const QString s)
{
	if (!w)
		return;

	ApplicationWindow *app = w->applicationWindow();
	if (qobject_cast<Graph3D *>(w))
		w->hide();

	QString caption = w->objectName();
	if (s.contains ("minimized")) {
		QStringList lst = s.split("\t");
		if (lst.count() > 4){
			int width = lst[3].toInt();
			int height = lst[4].toInt();
			if(width > 0 && height > 0)
				w->resize(width, height);
		}
		w->setStatus(MdiSubWindow::Minimized);
		if (app)
			app->setListView(caption, tr("Minimized"));
	} else if (s.contains ("maximized")){
		w->setMaximized();
	} else {
		QStringList lst = s.split("\t");
		if (lst.count() > 4){
			w->resize(lst[3].toInt(), lst[4].toInt());
			w->move(lst[1].toInt(), lst[2].toInt());
		}
		w->setStatus(MdiSubWindow::Normal);
		if (lst.count() > 5) {
			if (app && lst[5] == "hidden")
				app->hideWindow(w);
		}
	}

	if (s.contains ("active")){
		Folder *f = w->folder();
		if (f)
			f->setActiveWindow(w);
	}
}

Folder* ApplicationWindow::projectFolder()
{
	return ((FolderListItem *)folders->topLevelItem(0))->folder();
}

bool ApplicationWindow::saveProject(bool compress)
{
	if (d_project_manager)
		return d_project_manager->saveProject(compress);
	return false;
}

QString ApplicationWindow::getFileName(QWidget *parent, const QString & caption, const QString & dir, const QString & filter,
									   QString * selectedFilter, bool save, bool confirmOverwrite)
{
	QFileDialog fd(parent, caption, dir, filter);
	if (filter.contains(";"))
		fd.setNameFilters(filter.split(";", Qt::SkipEmptyParts));

	if (save)
		fd.setAcceptMode(QFileDialog::AcceptSave);
	else
		fd.setAcceptMode(QFileDialog::AcceptOpen);

	fd.setOption(QFileDialog::DontConfirmOverwrite, true);
	fd.setFileMode(QFileDialog::AnyFile);

	if (fd.exec() != QDialog::Accepted )
		return QString();

	if (fd.selectedFiles().isEmpty())
		return QString();

	if (selectedFilter)
		*selectedFilter = fd.selectedNameFilter();

	if (save){
		QString file_name = fd.selectedFiles()[0];
		QString selected_filter = fd.selectedNameFilter();
		int pos1 = selected_filter.indexOf("*");
		selected_filter = selected_filter.mid(pos1 + 1, selected_filter.length() - pos1 - 2);
		if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
			file_name.append(selected_filter);

		if (confirmOverwrite && QFileInfo(file_name).exists() &&
			QMessageBox::warning(parent, tr("QtiPlot") + " - " + tr("Overwrite file?"),
			tr("%1 already exists.").arg(file_name) + "\n" + tr("Do you want to replace it?"),
			QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
			return QString();

		QFile file(file_name);
		if(!file.open(QIODevice::WriteOnly)){
			QMessageBox::critical(parent, tr("QtiPlot - Export error"),
			tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(file_name));
			return QString();
		}
		file.close();
		file.remove();
	}

	return fd.selectedFiles()[0];
}

QString ApplicationWindow::getSaveProjectName(const QString& fileName, bool *compress, int scope)
{
	if (d_project_manager)
		return d_project_manager->getSaveProjectName(fileName, compress, scope);
	return QString();
}

void ApplicationWindow::saveProjectAs(const QString& fileName, bool compress)
{
	if (d_project_manager)
		d_project_manager->saveProjectAs(fileName, compress);
}

void ApplicationWindow::saveWindowAs(const QString& fileName, bool compress)
{
	MdiSubWindow *w = this->activeWindow();
	if (!w)
		return;

	QString fn = getSaveProjectName(fileName, &compress, 2);
	if (!fn.isEmpty()){
		/*if (w->inherits("Table") && fn.endsWith(".ogw")){
			ImportExportPlugin *ep = exportPlugin("ogw");
			if (!ep)
				return;
			ep->exportTable((Table *)w, fn, true, true, false);
			return;
		}*/
		if (saveWindow(w, fn, compress))
			updateRecentProjectsList(fn);
	}
}

bool ApplicationWindow::saveWindow(MdiSubWindow *w, const QString& fn, bool compress)
{
	if (!w)
		return false;

	QString tempFn = fn + ".tmp";
	QFile::remove(tempFn);

	QFile f(tempFn);
	if (!f.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, tr("QtiPlot - File save error"),
			tr("Cannot write to temporary file: <br><b>%1</b>").arg(tempFn));
		return false;
	}
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QTextStream t(&f);
	t.setEncoding(QStringConverter::Utf8);
	t << "QtiPlot " + QString::number(maj_version) + "." + QString::number(min_version) + "." + QString::number(patch_version) + " project file\n";
	t << "<scripting-lang>\t" + QString(scriptEnv->objectName()) + "\n";

	int windows = 1;
	QStringList tbls;
	if (qobject_cast<MultiLayer *>(w)){
		tbls = multilayerDependencies(w);
		windows = tbls.size() + 1;
	}

	Graph3D *g = qobject_cast<Graph3D *>(w);
	if (g && (g->table() || g->matrix()))
		windows++;

	t << "<windows>\t" + QString::number(windows) + "\n";
	t.flush();
	if (t.status() != QTextStream::Ok || f.error() != QFile::NoError) {
		f.close();
		QFile::remove(tempFn);
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("QtiPlot - File save error"), tr("Error writing window header to <b>%1</b>.").arg(fn));
		return false;
	}
	f.close();

	for (QString s : tbls){
		Table *t = table(s);
		if (t)
			t->save(tempFn, windowGeometryInfo(t));
	}

	if (g){
		Matrix *m = g->matrix();
		if (m)
			m->save(tempFn, windowGeometryInfo(m));
		Table *t = g->table();
		if (t)
			t->save(tempFn, windowGeometryInfo(t));
	}

	w->save(tempFn, windowGeometryInfo(w));

	if (compress)
		file_compress(tempFn.toUtf8().data(), (char*)"wb9");

	if (QFile::exists(fn)) {
		if (!QFile::remove(fn)) {
			QFile::remove(tempFn);
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(this, tr("QtiPlot - File save error"), tr("Cannot overwrite existing file <b>%1</b>.").arg(fn));
			return false;
		}
	}

	if (!QFile::rename(tempFn, fn)) {
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("QtiPlot - File save error"),
			tr("Cannot rename temporary file <b>%1</b> to <b>%2</b>.").arg(tempFn).arg(fn));
		return false;
	}

	QApplication::restoreOverrideCursor();
	return true;
}

void ApplicationWindow::saveNoteAs()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;
	w->exportASCII();
}

void ApplicationWindow::increaseNoteIndent()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->setTabStopDistance(w->currentEditor()->tabStopDistance() + 5);
	modifiedProject();
}

void ApplicationWindow::decreaseNoteIndent()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->setTabStopDistance(w->currentEditor()->tabStopDistance() - 5);
	modifiedProject();
}

void ApplicationWindow::showNoteLineNumbers(bool show)
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;
	w->showLineNumbers(show);
	modifiedProject();
}

void ApplicationWindow::noteFindDialogue()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->currentEditor()->showFindDialog();
}

void ApplicationWindow::noteFindNext()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->currentEditor()->findNext();
}

void ApplicationWindow::noteFindPrev()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->currentEditor()->findPrevious();
}

void ApplicationWindow::noteReplaceDialogue()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;

	w->currentEditor()->showFindDialog(true);
}

void ApplicationWindow::renameCurrentNoteTab()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;
	w->renameCurrentTab();
	modifiedProject();
}

void ApplicationWindow::addNoteTab()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;
	w->addTab();
	modifiedProject();
}

void ApplicationWindow::closeNoteTab()
{
	Note* w = (Note*)activeWindow(NoteWindow);
	if (!w)
		return;
	w->removeTab();
	modifiedProject();
}

void ApplicationWindow::saveAsTemplate(MdiSubWindow* w, const QString& fileName)
{
	if (!w) {
		w = activeWindow();
		if (!w)
			return;
	}

	QString fn = fileName;
	if (fn.isEmpty()){
		QString filter;
		if (w->inherits("Matrix"))
			filter = tr("QtiPlot Matrix Template")+" (*.qmt)";
		else if (w->inherits("MultiLayer"))
			filter = tr("QtiPlot 2D Graph Template")+" (*.qpt)";
		else if (w->inherits("Table"))
			filter = tr("QtiPlot Table Template")+" (*.qtt)";
		else if (w->inherits("Graph3D"))
			filter = tr("QtiPlot 3D Surface Template")+" (*.qst)";

		QString selectedFilter;
		fn = getFileName(this, tr("Save Window As Template"), templatesDir + "/" + w->objectName(), filter, &selectedFilter, true, d_confirm_overwrite);

		if (!fn.isEmpty()){
			QFileInfo fi(fn);
			templatesDir = fi.absolutePath();
			QString baseName = fi.fileName();
			if (!baseName.contains(".")){
				selectedFilter = selectedFilter.right(5).left(4);
				fn.append(selectedFilter);
			}
		} else
			return;
	}

	QFile f(fn);
	if ( !f.open( QIODevice::WriteOnly ) ){
		QMessageBox::critical(this, tr("QtiPlot - Export error"),
		tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fn));
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QTextStream t( &f );
	t.setEncoding(QStringConverter::Utf8);
	t << "QtiPlot " + QString::number(maj_version)+"."+ QString::number(min_version)+"."+
				QString::number(patch_version) + " template file\n";
	f.close();
	w->save(fn, windowGeometryInfo(w), true);
	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::rename()
{
	MdiSubWindow* m = activeWindow();
	if (!m)
		return;

	RenameWindowDialog *rwd = new RenameWindowDialog(this);
	rwd->setWidget(m);
	rwd->exec();
}

void ApplicationWindow::renameWindow()
{
	WindowListItem *it = (WindowListItem *)lv->currentItem();
	MdiSubWindow *w = it->window();
	if (!w)
		return;

	RenameWindowDialog *rwd = new RenameWindowDialog(this);
	rwd->setWidget(w);
	rwd->exec();
}

void ApplicationWindow::renameWindow(QTreeWidgetItem *item, int, const QString &text)
{
	if (!item)
		return;

	MdiSubWindow *w = ((WindowListItem *)item)->window();
	if (!w || text == w->objectName())
		return;

	if(!setWindowName(w, text)){
		lv->blockSignals(true);
		item->setText(0, w->objectName());
		lv->blockSignals(false);
	}
}

bool ApplicationWindow::setWindowName(MdiSubWindow *w, const QString &text)
{
	if (!w)
		return false;

	QString name = w->objectName();
	if (name == text)
		return true;

	QString newName = text;
	if (newName.isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please enter a valid name!"));
		return false;
	} else if (QString(newName).remove("-").contains(QRegularExpression("\\W"))){
		QMessageBox::critical(this, tr("QtiPlot - Error"),
				tr("The name you chose is not valid: only letters and digits are allowed!")+
				"<p>" + tr("Please choose another name!"));
		return false;
	}

	while(alreadyUsedName(newName)){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Name <b>%1</b> already exists!").arg(newName) + "<p>" + tr("Please choose another name!"));
		return false;
	}

	if (w->inherits("Table"))
		updateTableNames(name, newName);
	else if (w->inherits("Matrix"))
		changeMatrixName(name, newName);

	w->setObjectName(newName);
	renameListViewItem(name, newName);
	updateCompleter(name, false, newName);
	emit modified();
	return true;
}

QStringList ApplicationWindow::columnsList(Table::PlotDesignation plotType)
{
	QStringList list;
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (!w->inherits("Table"))
			continue;

		Table *t = (Table *)w;
		for (int i=0; i < t->numCols(); i++)
		{
			if (t->colPlotDesignation(i) == plotType || plotType == Table::All)
				list << QString(t->objectName()) + "_" + t->colLabel(i);
		}
	}
	return list;
}

void ApplicationWindow::showCurvesDialog()
{
	Graph* g = activePlotLayer(false);
	if (!g)
		return;

	if (g->isPiePlot()){
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("This functionality is not available for pie plots!"));
	} else {
		CurvesDialog* crvDialog = new CurvesDialog(this);
		crvDialog->setGraph(g);
		crvDialog->resize(d_add_curves_dialog_size);
		crvDialog->setModal(true);
		crvDialog->show();
	}
}

bool ApplicationWindow::hasTable()
{
    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
            if (w->inherits("Table"))
                return true;
		}
		f = f->folderBelow();
	}
	return false;
}

QStringList ApplicationWindow::tableNames()
{
    QStringList lst = QStringList();
    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
            if (w->inherits("Table"))
                lst << w->objectName();
		}
		f = f->folderBelow();
	}
	return lst;
}

QList<MdiSubWindow*> ApplicationWindow::tableList()
{
	QList<MdiSubWindow*> lst;
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
            if (w->inherits("Table"))
                lst << w;
		}
		f = f->folderBelow();
	}
	return lst;
}

AssociationsDialog* ApplicationWindow::showPlotAssociations(int curve)
{
	MdiSubWindow* w = activeWindow(MultiLayerWindow);
	if (!w)
		return 0;

	Graph *g = ((MultiLayer*)w)->activeLayer();
	if (!g)
		return 0;

	AssociationsDialog* ad = new AssociationsDialog(this);
	ad->setAttribute(Qt::WA_DeleteOnClose);
	ad->setGraph(g);
	ad->initTablesList(tableList(), curve);
	ad->show();
	return ad;
}

void ApplicationWindow::showTitleDialog()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;

	if (w->inherits("MultiLayer")){
		Graph* g = ((MultiLayer*)w)->activeLayer();
		if (g){
			TextDialog* td= new TextDialog(TextDialog::LayerTitle, this, {});
			td->setGraph(g);
			td->exec();
		}
	} else if (w->inherits("Graph3D")) {
		Plot3DDialog* pd = (Plot3DDialog*)showPlot3dDialog();
		if (pd)
			pd->showTitleTab();
	}
}

void ApplicationWindow::showAxisTitleDialog()
{
	MdiSubWindow* w = activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph* g = ((MultiLayer*)w)->activeLayer();
	if (!g)
		return;

	TextDialog* td = new TextDialog(TextDialog::AxisTitle, this, {});
	td->setGraph(g);
	td->exec();
}



void ApplicationWindow::showRowsDialog()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	bool ok;
	int rows = QInputDialog::getInt(this, tr("QtiPlot - Enter rows number"), tr("Rows"),
			t->numRows(), 0, 1000000, 1, &ok);
	if ( ok )
		t->resizeRows(rows);
}

void ApplicationWindow::showDeleteRowsDialog()
{
    Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	bool ok;
	int start_row = QInputDialog::getInt(this, tr("QtiPlot - Delete rows"), tr("Start row"),
                    1, 1, t->numRows(), 1, &ok);
    if (ok){
        int end_row = QInputDialog::getInt(this, tr("QtiPlot - Delete rows"), tr("End row"),
                        t->numRows(), 1, t->numRows(), 1, &ok);
        if (ok)
            t->deleteRows(start_row, end_row);
	}
}

void ApplicationWindow::showColsDialog()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	bool ok;
	int cols = QInputDialog::getInt(this, tr("QtiPlot - Enter columns number"), tr("Columns"),
			t->numCols(), 0, 1000000, 1, &ok);
	if ( ok )
		t->resizeCols(cols);
}

void ApplicationWindow::showColumnOptionsDialog()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	if (t->selectedColumns().count()>0){
		TableDialog* td = new TableDialog(t, this);
		td->exec();
	} else
		QMessageBox::warning(this, tr("QtiPlot"), tr("Please select a column first!"));
}

void ApplicationWindow::showColumnValuesDialog()
{
	Table *w = (Table*)activeWindow(TableWindow);
	if (!w)
		return;

	if (w->selectedColumns().count()>0 || w->table()->currentSelection() >= 0){
		SetColValuesDialog* vd = new SetColValuesDialog(scriptEnv, this);
		vd->setTable(w);
		if (d_completion)
			vd->setCompleter(d_completer);
		vd->exec();
	} else
		QMessageBox::warning(this, tr("QtiPlot - Column selection error"), tr("Please select a column first!"));
}

void ApplicationWindow::showExtractDataDialog()
{
	Table *w = (Table*)activeWindow(TableWindow);
	if (!w)
		return;

	ExtractDataDialog* edd = new ExtractDataDialog(scriptEnv, this);
	edd->setTable(w);
	if (d_completion)
		edd->setCompleter(d_completer);
	edd->exec();
}

void ApplicationWindow::recalculateTable()
{
	MdiSubWindow* w = activeWindow();
	if (!w)
		return;

	if (qobject_cast<TableStatistics *>(w))
		((TableStatistics*)w)->update();
	else if (qobject_cast<Table *>(w))
		((Table*)w)->calculate();
	else if (qobject_cast<Matrix *>(w))
		((Matrix*)w)->calculate();
}

void ApplicationWindow::sortActiveTable()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	t->sortTableDialog();
}

void ApplicationWindow::sortSelection()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	t->sortColumnsDialog();
}

void ApplicationWindow::normalizeActiveTable()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (t)
		t->normalize();
}

void ApplicationWindow::normalizeSelection()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	if (int(t->selectedColumns().count())>0)
		t->normalizeSelection();
	else
		QMessageBox::warning(this, tr("QtiPlot - Column selection error"), tr("Please select a column first!"));
}

void ApplicationWindow::correlate()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QStringList s = t->selectedColumns();
	if ((int)s.count() != 2){
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("Please select two columns for this operation!"));
		return;
	}

	Correlation *cor = new Correlation(this, t, s[0], s[1]);
	cor->run();
	delete cor;
}

void ApplicationWindow::autoCorrelate()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QStringList s = t->selectedColumns();
	if ((int)s.count() != 1)
	{
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("Please select exactly one columns for this operation!"));
		return;
	}

	Correlation *cor = new Correlation(this, t, s[0], s[0]);
	cor->run();
	delete cor;
}

void ApplicationWindow::convolute()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QStringList s = t->selectedColumns();
	if ((int)s.count() != 2)
	{
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("Please select two columns for this operation:\n the first represents the signal and the second the response function!"));
		return;
	}

	Convolution *cv = new Convolution(this, t, s[0], s[1]);
	cv->run();
	delete cv;
}

void ApplicationWindow::deconvolute()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QStringList s = t->selectedColumns();
	if ((int)s.count() != 2)
	{
		QMessageBox::warning(this, tr("QtiPlot - Error"), tr("Please select two columns for this operation:\n the first represents the signal and the second the response function!"));
		return;
	}

	Deconvolution *dcv = new Deconvolution(this, t, s[0], s[1]);
	dcv->run();
	delete dcv;
}

void ApplicationWindow::showColStatistics()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QList<int> targets;
	for (int i = 0; i < t->numCols(); i++)
		if (t->isColumnSelected(i))
			targets << i;

	QTableWidgetSelectionRange select = t->getSelection();
	newTableStatistics(t, TableStatistics::column, targets, select.topRow(), select.bottomRow())->showNormal();
}

void ApplicationWindow::showRowStatistics()
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QList<int> targets;
	for (int i = 0; i < t->numRows(); i++)
		if (t->isRowSelected(i))
			targets << i;

	QTableWidgetSelectionRange select = t->getSelection();
	newTableStatistics(t, TableStatistics::row, targets, select.leftColumn(), select.rightColumn())->showNormal();
}

void ApplicationWindow::showColMenu(int c)
{
	Table *w = (Table*)activeWindow(TableWindow);
	if (!w)
		return;

	QMenu contextMenu(this);
	QMenu plot(this);
	QMenu specialPlot(this);
	QMenu specialBarMenu(this);
	QMenu fill(this);
	QMenu sorting(this);
	QMenu colType(this);
	
	QMenu panels(this);
	QMenu gridMenu(this);
	QMenu stat(this);
	QMenu norm(this);

	if (w->selectedColumns().count() == 1){
		w->setSelectedCol(c);
		plot.addAction(actionPlotL);
		plot.addAction(actionPlotP);
		plot.addAction(actionPlotLP);

		specialPlot.addAction(actionPlotVerticalDropLines);
		specialPlot.addAction(actionPlotSpline);
		specialPlot.addAction(actionPlotVertSteps);
		specialPlot.addAction(actionPlotHorSteps);
		specialPlot.addSeparator();
        specialPlot.addAction(actionAddZoomPlot);
		specialPlot.setTitle(tr("Special Line/Symb&ol"));
		plot.addMenu(&specialPlot);
		plot.addSeparator();
		plot.addAction(actionPlotVerticalBars);
		plot.addAction(actionPlotHorizontalBars);
		specialBarMenu.addAction(actionStackBars);
		specialBarMenu.addAction(actionStackColumns);
		specialBarMenu.setTitle(tr("Spec&ial Bar/Column"));
		plot.addMenu (&specialBarMenu);
		plot.addAction(actionPlotArea);
		plot.addAction(actionPlotPie);
		plot.addSeparator();

		plot.addAction(actionPlot3DRibbon);
		plot.addAction(actionPlot3DBars);
		plot.addAction(actionPlot3DScatter);
		plot.addAction(actionPlot3DTrajectory);

		plot.addSeparator();

		stat.addAction(actionBoxPlot);
		stat.addAction(actionPlotHistogram);
		stat.addAction(actionPlotStackedHistograms);
		stat.addSeparator();
		stat.addAction(actionStemPlot);
		stat.setTitle(tr("Statistical &Graphs"));
		plot.addMenu(&stat);

		plot.setTitle(tr("&Plot"));
		contextMenu.addMenu(&plot);
		contextMenu.addSeparator();

		contextMenu.addAction(QIcon(":/cut.png"),tr("Cu&t"), static_cast<Table *>(w), &Table::cutSelection);
		contextMenu.addAction(QIcon(":/copy.png"),tr("&Copy"), static_cast<Table *>(w), &Table::copySelection);
		contextMenu.addAction(QIcon(":/paste.png"),tr("Past&e"), static_cast<Table *>(w), &Table::pasteSelection);
		contextMenu.addSeparator();

		QAction * xColID=colType.addAction(QIcon(":/x_col.png"), tr("&X"), this, &ApplicationWindow::setXCol);
		
		QAction * yColID=colType.addAction(QIcon(":/y_col.png"), tr("&Y"), this, &ApplicationWindow::setYCol);
        
		QAction * zColID=colType.addAction(QIcon(":/z_col.png"), tr("&Z"), this, &ApplicationWindow::setZCol);
        
        colType.addSeparator();
		QAction * labelID = colType.addAction(QIcon(":/set_label_col.png"), tr("&Label"), this, &ApplicationWindow::setLabelCol);
        
		QAction * noneID=colType.addAction(QIcon(":/disregard_col.png"), tr("&None"), this, &ApplicationWindow::disregardCol);
        
        colType.addSeparator();
        QAction * xErrColID =colType.addAction(tr("X E&rror"), this, &ApplicationWindow::setXErrCol);
        
		QAction * yErrColID = colType.addAction(QIcon(":/errors.png"), tr("Y &Error"), this, &ApplicationWindow::setYErrCol);
        
        colType.addSeparator();


        if (w->colPlotDesignation(c) == Table::X)
            xColID->setChecked(true);
        else if (w->colPlotDesignation(c) == Table::Y)
            yColID->setChecked(true);
        else if (w->colPlotDesignation(c) == Table::Z)
            zColID->setChecked(true);
        else if (w->colPlotDesignation(c) == Table::xErr)
            xErrColID->setChecked(true);
        else if (w->colPlotDesignation(c) == Table::yErr)
            yErrColID->setChecked(true);
        else if (w->colPlotDesignation(c) == Table::Label)
            labelID->setChecked(true);
        else
            noneID->setChecked(true);

        colType.addAction(actionReadOnlyCol);
        
        actionReadOnlyCol->setChecked(w->isReadOnlyColumn(c));

		colType.setTitle(tr("Set As"));
		contextMenu.addMenu(&colType);

		if (w){
			contextMenu.addSeparator();

			contextMenu.addAction(actionShowColumnValuesDialog);
			contextMenu.addAction(actionTableRecalculate);
			fill.addAction(actionSetAscValues);
			fill.addAction(actionSetRandomValues);
			fill.addAction(actionSetRandomNormalValues);
			fill.setTitle(tr("&Fill Column With"));
			contextMenu.addMenu(&fill);

			norm.addAction(tr("&Column"), static_cast<Table *>(w), &Table::normalizeSelection);
			norm.addAction(actionNormalizeTable);
			norm.setTitle(tr("&Normalize"));
			contextMenu.addMenu(& norm);

			contextMenu.addSeparator();
			contextMenu.addAction(actionFrequencyCount);
			contextMenu.addAction(actionShowColStatistics);

			contextMenu.addSeparator();

			contextMenu.addAction(QIcon(":/erase.png"), tr("Clea&r"), static_cast<Table *>(w), &Table::clearSelection);
			contextMenu.addAction(QIcon(":/delete_column.png"), tr("&Delete"), static_cast<Table *>(w), qOverload<>(&Table::removeCol));
			contextMenu.addAction(actionHideSelectedColumns);
			contextMenu.addAction(actionShowAllColumns);
			contextMenu.addSeparator();
			contextMenu.addAction(QIcon(":/insert_column.png"), tr("&Insert"), static_cast<Table *>(w), &Table::insertCol);
			contextMenu.addAction(actionAddColToTable);
			contextMenu.addSeparator();

			sorting.addAction(QIcon(":/sort_ascending.png"), tr("&Ascending"), static_cast<Table *>(w), &Table::sortColAsc);
			sorting.addAction(QIcon(":/sort_descending.png"), tr("&Descending"), static_cast<Table *>(w), &Table::sortColDesc);
			sorting.setTitle(tr("Sort Colu&mn"));
			contextMenu.addMenu(&sorting);

			contextMenu.addAction(actionSortTable);
		}
		contextMenu.addSeparator();
		contextMenu.addAction(actionAdjustColumnWidth);
		contextMenu.addAction(actionShowColumnOptionsDialog);
	} else if (w->selectedColumns().count() > 1){
		plot.addAction(actionPlotL);
		plot.addAction(actionPlotP);
		plot.addAction(actionPlotLP);

		specialPlot.addAction(actionPlotVerticalDropLines);
		specialPlot.addAction(actionPlotSpline);
		specialPlot.addAction(actionPlotVertSteps);
		specialPlot.addAction(actionPlotHorSteps);
		specialPlot.addSeparator();
        specialPlot.addAction(actionPlotDoubleYAxis);
        specialPlot.addAction(actionWaterfallPlot);
        specialPlot.addAction(actionAddZoomPlot);
		specialPlot.setTitle(tr("Special Line/Symb&ol"));
		plot.addMenu(&specialPlot);
		plot.addSeparator();
		plot.addAction(actionPlotVerticalBars);
		plot.addAction(actionPlotHorizontalBars);

		specialBarMenu.addAction(actionStackBars);
		specialBarMenu.addAction(actionStackColumns);
		specialBarMenu.setTitle(tr("Spec&ial Bar/Column"));
		plot.addMenu (&specialBarMenu);;

		plot.addAction(actionPlotArea);
		plot.addAction(actionPlotVectXYXY);
		plot.addAction(actionPlotVectXYAM);
		plot.addSeparator();
		stat.addAction(actionBoxPlot);
		stat.addAction(actionPlotHistogram);
		stat.addAction(actionPlotStackedHistograms);
		stat.addSeparator();
		stat.addAction(actionStemPlot);
		stat.setTitle(tr("Statistical &Graphs"));
		plot.addMenu(&stat);

		panels.addAction(actionPlot2VerticalLayers);
		panels.addAction(actionPlot2HorizontalLayers);
		panels.addAction(actionPlot4Layers);
		panels.addAction(actionPlotStackedLayers);
		panels.addAction(actionCustomLayout);
		panels.setTitle(tr("Pa&nel"));
		plot.addMenu(&panels);

		gridMenu.addAction(actionVertSharedAxisLayers);
		gridMenu.addAction(actionHorSharedAxisLayers);
		gridMenu.addAction(actionSharedAxesLayers);
		gridMenu.addAction(actionStackSharedAxisLayers);
		gridMenu.addAction(actionCustomSharedAxisLayers);
		gridMenu.setTitle(tr("Shared A&xes Panel"));
		plot.addMenu(&gridMenu);

		plot.setTitle(tr("&Plot"));
		contextMenu.addMenu(&plot);
		contextMenu.addSeparator();
		contextMenu.addAction(QIcon(":/cut.png"), tr("Cu&t"), static_cast<Table *>(w), &Table::cutSelection);
		contextMenu.addAction(QIcon(":/copy.png"), tr("&Copy"), static_cast<Table *>(w), &Table::copySelection);
		contextMenu.addAction(QIcon(":/paste.png"), tr("Past&e"), static_cast<Table *>(w), &Table::pasteSelection);
		contextMenu.addSeparator();

		if (w){
			contextMenu.addAction(QIcon(":/erase.png"), tr("Clea&r"), static_cast<Table *>(w), &Table::clearSelection);
			contextMenu.addAction(QIcon(":/close.png"), tr("&Delete"), static_cast<Table *>(w), qOverload<>(&Table::removeCol));
			contextMenu.addAction(actionHideSelectedColumns);
			contextMenu.addAction(actionShowAllColumns);
			contextMenu.addSeparator();
			contextMenu.addAction(tr("&Insert"), static_cast<Table *>(w), &Table::insertCol);
			contextMenu.addAction(actionAddColToTable);
			contextMenu.addSeparator();
		}

		colType.addAction(actionSetXCol);
		colType.addAction(actionSetYCol);
		colType.addAction(actionSetZCol);
		colType.addSeparator();
		colType.addAction(actionSetLabelCol);
		colType.addAction(actionDisregardCol);
		colType.addSeparator();
		colType.addAction(actionSetXErrCol);
		colType.addAction(actionSetYErrCol);
		colType.addSeparator();
		colType.addAction(tr("&Read-only"), this, &ApplicationWindow::setReadOnlyColumns);
		colType.addAction(tr("Read/&Write"), this, &ApplicationWindow::setReadWriteColumns);
		colType.setTitle(tr("Set As"));
		contextMenu.addMenu(&colType);

		if (w){
			contextMenu.addSeparator();

			fill.addAction(actionSetAscValues);
			fill.addAction(actionSetRandomValues);
			fill.addAction(actionSetRandomNormalValues);
			fill.setTitle(tr("&Fill Columns With"));
			contextMenu.addMenu(&fill);

			norm.addAction(actionNormalizeSelection);
			norm.addAction(actionNormalizeTable);
			norm.setTitle(tr("&Normalize"));
			contextMenu.addMenu(&norm);

			contextMenu.addSeparator();
			sorting.addAction(QIcon(":/sort_ascending.png"), tr("&Ascending"), static_cast<Table *>(w), &Table::sortColAsc);
			sorting.addAction(QIcon(":/sort_descending.png"), tr("&Descending"), static_cast<Table *>(w), &Table::sortColDesc);
			sorting.addAction(actionSortSelection);
			sorting.setTitle("&" + tr("Sort Columns"));
			contextMenu.addMenu(&sorting);
			contextMenu.addAction(actionSortTable);
			contextMenu.addSeparator();
			contextMenu.addAction(actionShowColStatistics);
			contextMenu.addAction(actionShowRowStatistics);
			contextMenu.addSeparator();
			contextMenu.addAction(actionAdjustColumnWidth);
		}
	}

	QPoint posMouse=QCursor::pos();
	contextMenu.exec(posMouse);
}













void ApplicationWindow::showMatrixDialog()
{
	Matrix *m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	MatrixDialog* md = new MatrixDialog(this);
	md->setMatrix (m);
	md->exec();
}

void ApplicationWindow::showMatrixSizeDialog()
{
	Matrix *m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	MatrixSizeDialog* md = new MatrixSizeDialog(m, this);
	md->exec();
}

void ApplicationWindow::showMatrixValuesDialog()
{
	Matrix *m = nullptr;
	MultiLayer *ml = (MultiLayer*)activeWindow(MultiLayerWindow);
	if (ml){
		int curveIndex = actionSetMatrixValues->data().toInt();
		Graph *g = ml->activeLayer();
		if (g){
			Spectrogram *sp = (Spectrogram *)g->plotItem(curveIndex);
			if (sp)
				m = sp->matrix();
		}
	} else
		m = (Matrix*)activeWindow(MatrixWindow);

	if (!m)
		return;

	MatrixValuesDialog* md = new MatrixValuesDialog(scriptEnv, this);
	md->setMatrix(m);
	if (d_completion)
		md->setCompleter(d_completer);
	md->exec();
}

void ApplicationWindow::showGeneralPlotDialog()
{
	MdiSubWindow* plot = activeWindow();
	if (!plot)
		return;

	if (plot->inherits("MultiLayer") && ((MultiLayer*)plot)->numLayers())
		showPlotDialog();
	else if (plot->inherits("Graph3D")){
	    QDialog* gd = showScaleDialog();
		((Plot3DDialog*)gd)->showGeneralTab();
	}
}

void ApplicationWindow::showAxisDialog()
{
	MdiSubWindow* plot = activeWindow();
	if (!plot)
		return;

	QDialog* gd = showScaleDialog();
	if (gd && plot->inherits("MultiLayer") && ((MultiLayer*)plot)->numLayers())
		((AxesDialog*)gd)->showAxesPage();
	else if (gd && plot->inherits("Graph3D"))
		((Plot3DDialog*)gd)->showAxisTab();
}

void ApplicationWindow::showGridDialog()
{
	AxesDialog* gd = (AxesDialog*)showScaleDialog();
	if (gd)
		gd->showGridPage();
}

QDialog* ApplicationWindow::showScaleDialog()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return 0;

	if (w->inherits("MultiLayer")){
		if (((MultiLayer*)w)->isEmpty())
			return 0;

		Graph* g = ((MultiLayer*)w)->activeLayer();
		if (g->isPiePlot()){
            QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		    return 0;
		}

		AxesDialog* ad = new AxesDialog(this);
        ad->setGraph(g);
        ad->exec();
        return ad;
	} else if (w->inherits("Graph3D"))
		return showPlot3dDialog();

	return 0;
}

AxesDialog* ApplicationWindow::showScalePageFromAxisDialog(int axisPos)
{
	AxesDialog* gd = (AxesDialog*)showScaleDialog();
	if (gd)
		gd->setCurrentScale(axisPos);

	return gd;
}

AxesDialog* ApplicationWindow::showAxisPageFromAxisDialog(int axisPos)
{
	AxesDialog* gd = (AxesDialog*)showScaleDialog();
	if (gd){
		gd->showAxesPage();
		gd->setCurrentScale(axisPos);
	}
	return gd;
}

QDialog* ApplicationWindow::showPlot3dDialog()
{
	Graph3D *g = (Graph3D*)activeWindow(Plot3DWindow);
	if (!g)
		return 0;

	if (!g->hasData()){
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, tr("QtiPlot - Warning"),
					tr("Not available for empty 3D surface plots!"));
		return 0;
	}

	Plot3DDialog* pd = new Plot3DDialog(this);
	pd->setPlot(g);
	pd->show();
	return pd;
}

void ApplicationWindow::showPlotDialog(int curveIndex)
{
	MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	PlotDialog* pd = new PlotDialog(d_extended_plot_dialog, this);
	pd->insertColumnsList(columnsList(Table::All));
	pd->setMultiLayer(w);
    if (curveIndex >= 0){
		Graph *g = w->activeLayer();
		if (g)
			pd->selectCurve(curveIndex);
	} else if (curveIndex == -100)
		pd->selectMultiLayerItem();

    pd->initFonts(plotTitleFont, plotAxesFont, plotNumbersFont, plotLegendFont);
	pd->showAll(d_extended_plot_dialog);
	pd->show();
}

void ApplicationWindow::showCurvePlotDialog()
{
	showPlotDialog(actionShowCurvePlotDialog->data().toInt());
}

void ApplicationWindow::showCurveContextMenu(QwtPlotItem *cv)
{
	if (!cv || !cv->isVisible())
		return;

	MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph *g = w->activeLayer();
	if (!g)
		return;

	int curveIndex = g->curveIndex(cv);
	if (curveIndex < 0 || curveIndex >= g->curveCount())
		return;

	QMenu curveMenu(this);
	curveMenu.addAction(cv->title().text(), this, &ApplicationWindow::showCurvePlotDialog);
	curveMenu.addSeparator();

	curveMenu.addAction(actionHideCurve);
	actionHideCurve->setData(curveIndex);

	int type = ((PlotCurve *)cv)->rtti();
	bool spectrogram = (cv->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) ? true : false;
    if (g->visibleCurves() > 1 && (type == Graph::Function || spectrogram)){
        curveMenu.addAction(actionHideOtherCurves);
        actionHideOtherCurves->setData(curveIndex);
    } else if (type != Graph::Function && !spectrogram) {
        if ((g->visibleCurves() - ((DataCurve *)cv)->errorBarsList().count()) > 1) {
            curveMenu.addAction(actionHideOtherCurves);
            actionHideOtherCurves->setData(curveIndex);
        }
    }

	if (g->visibleCurves() != g->curveCount())
		curveMenu.addAction(actionShowAllCurves);
	curveMenu.addSeparator();

	if (g->rangeSelectorsEnabled() || (g->activeTool() &&
		g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker))
		curveMenu.addAction(actionCopySelection);
	if (spectrogram){
		curveMenu.addSeparator();
		curveMenu.addAction(actionSetMatrixValues);
		actionSetMatrixValues->setData(curveIndex);
	} else if (type != Graph::Function && type != Graph::ErrorBars){
		if (g->rangeSelectorsEnabled() || (g->activeTool() &&
			g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)){
			curveMenu.addAction(actionCutSelection);
			curveMenu.addAction(actionPasteSelection);
			if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker){
				DataPickerTool *dpt = (DataPickerTool *)g->activeTool();
				if (dpt){
					QAction *act = new QAction(tr("Paste Selection as Te&xt"), this);
					connect(act, &QAction::triggered, dpt, &DataPickerTool::pasteSelectionAsLayerText);
					curveMenu.addAction(act);
				}
			}

			curveMenu.addAction(actionClearSelection);
			curveMenu.addSeparator();
			if (g->rangeSelectorsEnabled()){
				QAction *act = new QAction(tr("Set Display Range"), this);
				connect(act, &QAction::triggered, g->rangeSelectorTool(), &RangeSelectorTool::setCurveRange);
				curveMenu.addAction(act);
			}
		}

		curveMenu.addAction(actionEditCurveRange);
		actionEditCurveRange->setData(curveIndex);

		curveMenu.addAction(actionCurveFullRange);
		if (((DataCurve *)cv)->isFullRange())
			actionCurveFullRange->setDisabled(true);
		else
			actionCurveFullRange->setEnabled(true);
		actionCurveFullRange->setData(curveIndex);

		curveMenu.addSeparator();
	}

	curveMenu.addAction(actionShowCurveWorksheet);
	actionShowCurveWorksheet->setData(curveIndex);

	curveMenu.addAction(actionShowCurvePlotDialog);
	actionShowCurvePlotDialog->setData(curveIndex);

	curveMenu.addSeparator();

	curveMenu.addAction(actionRemoveCurve);
	actionRemoveCurve->setData(curveIndex);
	curveMenu.exec(QCursor::pos());
}

void ApplicationWindow::showAllCurves()
{
    MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph* g = w->activeLayer();
	if (!g)
		return;

	for(int i=0; i< g->curveCount(); i++)
		g->showCurve(i);
	g->replot();
}

void ApplicationWindow::hideOtherCurves()
{
    MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph* g = w->activeLayer();
	if (!g)
		return;

	for(int i=0; i< g->curveCount(); i++)
		g->showCurve(i, false);

	g->showCurve(actionHideOtherCurves->data().toInt());
	g->replot();
}

void ApplicationWindow::hideCurve()
{
    MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph* g = w->activeLayer();
	if (!g)
		return;

	g->showCurve(actionHideCurve->data().toInt(), false);
}

void ApplicationWindow::removeCurve()
{
    MultiLayer *w = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!w)
		return;

	Graph* g = w->activeLayer();
	if (!g)
		return;

	g->removeCurve(actionRemoveCurve->data().toInt());
	g->updatePlot();
}

void ApplicationWindow::showCurveWorksheet(Graph *g, int curveIndex)
{
	if (!g)
		return;

    const QwtPlotItem *it = g->plotItem(curveIndex);
	if (!it)
		return;

	if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
		Spectrogram *sp = (Spectrogram *)it;
		if (sp->matrix())
			sp->matrix()->showMaximized();
	} else if (((PlotCurve *)it)->rtti() == Graph::Function)
		g->createTable((PlotCurve *)it);
    else {
		showTable(((DataCurve *)it)->table(), it->title().text());
		if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)
            ((DataPickerTool *)g->activeTool())->selectTableRow();
    }
}

void ApplicationWindow::showCurveWorksheet()
{
	Graph *g = activePlotLayer(false);
	if (!g)
		return;

	showCurveWorksheet(g, actionShowCurveWorksheet->data().toInt());
}

void ApplicationWindow::magnify(int mode)
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	QList<Graph *> layers = g->multiLayer()->layersList();
    for (Graph *g : layers)
		g->enablePanningMagnifier(true, mode);
}

void ApplicationWindow::zoomIn()
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	if (g->isPiePlot()){
		if (btnZoomIn->isChecked())
			QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		btnPointer->setChecked(true);
		return;
	}

	QList<Graph *> layers = g->multiLayer()->layersList();
    for (Graph *g : layers){
		if (!g->isPiePlot())
			g->zoom(true);
	}
}

void ApplicationWindow::zoomOut()
{
	Graph *g = activePlotLayer(false);
	if (!g || g->isPiePlot())
		return;

	g->zoomOut();
	btnPointer->setChecked(true);
}

void ApplicationWindow::setAutoScale()
{
	Graph* g = activePlotLayer(false);
	if (g)
		g->setAutoScale();
}

void ApplicationWindow::removePoints()
{
	Graph* g = activePlotLayer();
	if (!g || !g->validCurvesDataSize()){
		btnPointer->setChecked(true);
		return;
	}

	if (g->isPiePlot()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		btnPointer->setChecked(true);
		return;
	}

	if (d_confirm_modif_2D_points){
		QMessageBox msgBox(QMessageBox::Question, tr("QtiPlot"),
		tr("This will modify the data in the worksheets!\nAre you sure you want to continue?"));

		QPushButton *yesButton = msgBox.addButton(tr("Yes, don't ask me again"), QMessageBox::YesRole);
		msgBox.addButton(QMessageBox::Cancel);
		msgBox.setDefaultButton(msgBox.addButton(QMessageBox::Yes));
		msgBox.setEscapeButton(QMessageBox::Cancel);
		msgBox.setWindowIcon(this->windowIcon());
		msgBox.exec();
		if (msgBox.clickedButton() == yesButton || msgBox.clickedButton() == msgBox.defaultButton()){
			DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Remove);
			connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
			g->setActiveTool(tool);
			displayBar->show();
			if (msgBox.clickedButton() == yesButton)
				d_confirm_modif_2D_points = false;
		} else
			btnPointer->setChecked(true);
	} else {
		DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Remove);
		connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
		g->setActiveTool(tool);
		displayBar->show();
	}
}

void ApplicationWindow::movePoints(bool wholeCurve)
{
	Graph* g = activePlotLayer();
	if (!g || !g->validCurvesDataSize()){
		btnPointer->setChecked(true);
		return;
	}

	if (g->isPiePlot()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		btnPointer->setChecked(true);
		return;
	}

	if (d_confirm_modif_2D_points){
		QMessageBox msgBox(QMessageBox::Question, tr("QtiPlot"),
		tr("This will modify the data in the worksheets!\nAre you sure you want to continue?"));

		QPushButton *yesButton = msgBox.addButton(tr("Yes, don't ask me again"), QMessageBox::YesRole);
		msgBox.addButton(QMessageBox::Cancel);
		msgBox.setDefaultButton(msgBox.addButton(QMessageBox::Yes));
		msgBox.setEscapeButton(QMessageBox::Cancel);
		msgBox.setWindowIcon(this->windowIcon());
		msgBox.exec();
		if (msgBox.clickedButton() == yesButton || msgBox.clickedButton() == msgBox.defaultButton()){
			DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Move);
			connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
			if (wholeCurve)
				tool->setMode(DataPickerTool::MoveCurve);
			g->setActiveTool(tool);
			displayBar->show();
			if (msgBox.clickedButton() == yesButton)
				d_confirm_modif_2D_points = false;
		} else
			btnPointer->setChecked(true);
	} else {
		DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Move);
		connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
		if (wholeCurve)
			tool->setMode(DataPickerTool::MoveCurve);
		g->setActiveTool(tool);
		displayBar->show();
	}

	/*switch(QMessageBox::warning (this, tr("QtiPlot"),
				tr("This will modify the data in the worksheets!\nAre you sure you want to continue?"),
				tr("Continue"), tr("Cancel"), 0, 1))
	{
		case 0:
			if (g){
			DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Move);
			connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
				if (wholeCurve)
					tool->setMode(DataPickerTool::MoveCurve);
				g->setActiveTool(tool);
				displayBar->show();
			}
			break;

		case 1:
			btnPointer->setChecked(true);
			break;
	}*/
}


//print active window

//print preview for active window























void ApplicationWindow::updateLog(const QString& result)
{
	if ( !result.isEmpty() ){
		current_folder->appendLogInfo(result);
		showResults(true);
		emit modified();
	}
}


void ApplicationWindow::showResults(bool ok)
{
	if (ok){
		if (!current_folder->logInfo().isEmpty())
			results->setText(current_folder->logInfo());
		else
			results->setText(tr("Sorry, there are no results to display!"));

		logWindow->show();
		QTextCursor cur = results->textCursor();
		cur.movePosition(QTextCursor::End);
		results->setTextCursor(cur);
	} else
		logWindow->hide();
}

void ApplicationWindow::showResults(const QString& s, bool ok)
{
	current_folder->appendLogInfo(s);

	QString logInfo = current_folder->logInfo();
	if (!logInfo.isEmpty())
		results->setText(logInfo);
	showResults(ok);
}

Graph* ApplicationWindow::activePlotLayer(bool resetPointerBtn)
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return 0;

	Graph* g = (Graph*)plot->activeLayer();
	if (!g){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("There are no plot layers available in this window!"));
		if (resetPointerBtn)
			btnPointer->setChecked(true);
		return 0;
	}
	return g;
}

void ApplicationWindow::showScreenReader()
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	QList<Graph *> layers = g->multiLayer()->layersList();
	for (Graph *g : layers){
		ScreenPickerTool *tool = new ScreenPickerTool(g);
		connect(tool, &ScreenPickerTool::statusText, info, &QLineEdit::setText);
		g->setActiveTool(tool);
	}

	displayBar->show();
}

void ApplicationWindow::drawPoints()
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	QStringList curves = g->analysableCurvesList();
	if (curves.size() > 0){
		QString newItemString = tr("New");
		curves.prepend(newItemString);
		bool ok;
		QString txt = QInputDialog::getItem(this, tr("QtiPlot - Choose data set"), tr("Curve") + ": ", curves, 0, false, &ok);
		if (!ok){
			btnPointer->setChecked(true);
			return;
		}

		DrawPointTool *tool = new DrawPointTool(this, g);
		connect(tool, &DrawPointTool::statusText, info, &QLineEdit::setText);
		if (ok && !txt.isEmpty() && txt != newItemString)
			tool->setDataCurve(g->dataCurve(txt));
		g->setActiveTool(tool);
	} else {
		DrawPointTool *tool = new DrawPointTool(this, g);
		connect(tool, &DrawPointTool::statusText, info, &QLineEdit::setText);
		g->setActiveTool(tool);
	}

	displayBar->show();
}

void ApplicationWindow::showRangeSelectors()
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	if (!g->curveCount()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("There are no curves available on this plot!"));
		btnPointer->setChecked(true);
		return;
	} else if (g->isPiePlot()) {
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		btnPointer->setChecked(true);
		return;
	}

	displayBar->show();
	if (g->enableRangeSelectors()){
		RangeSelectorTool *tool = g->rangeSelectorTool();
		if (tool)
			connect(tool, &RangeSelectorTool::statusText, info, &QLineEdit::setText);
	}
}

void ApplicationWindow::showCursor()
{
	Graph *g = activePlotLayer();
	if (!g)
		return;

	if (g->isPiePlot()){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("This functionality is not available for pie plots!"));
		btnPointer->setChecked(true);
		return;
	}

	QList<Graph *> layers = g->multiLayer()->layersList();
	for (Graph *g : layers){
		if (g->isPiePlot() || !g->curveCount())
			continue;
		if (g->validCurvesDataSize()){
			DataPickerTool *tool = new DataPickerTool(g, this, DataPickerTool::Display);
			connect(tool, &DataPickerTool::statusText, info, &QLineEdit::setText);
			g->setActiveTool(tool);
		}
	}
	displayBar->show();
}

void ApplicationWindow::newLegend()
{
	Graph *g = activePlotLayer();
	if (g)
		g->newLegend();
}

void ApplicationWindow::addTimeStamp()
{
	Graph *g = activePlotLayer(false);
	if (g)
		g->addTimeStamp();
}

void ApplicationWindow::addRectangle()
{
	Graph *g = activePlotLayer(false);
	if (!g){
		actionAddRectangle->setChecked(false);
		return;
	}

	AddWidgetTool *tool = new AddWidgetTool(AddWidgetTool::Rectangle, g, actionAddRectangle);
	connect(tool, &AddWidgetTool::statusText, info, &QLineEdit::setText);
	g->setActiveTool(tool);
	btnPointer->setChecked(false);
}

void ApplicationWindow::addEllipse()
{
	Graph *g = activePlotLayer(false);
	if (!g){
		actionAddEllipse->setChecked(false);
		return;
	}

    AddWidgetTool *tool = new AddWidgetTool(AddWidgetTool::Ellipse, g, actionAddEllipse);
	connect(tool, &AddWidgetTool::statusText, info, &QLineEdit::setText);
	g->setActiveTool(tool);
	btnPointer->setChecked(false);
}

void ApplicationWindow::addTexFormula()
{
	Graph *g = activePlotLayer(false);
	if (!g){
		actionAddFormula->setChecked(false);
		return;
	}

	AddWidgetTool *tool = new AddWidgetTool(AddWidgetTool::TexEquation, g, actionAddFormula);
	connect(tool, &AddWidgetTool::statusText, info, &QLineEdit::setText);
	g->setActiveTool(tool);
	btnPointer->setChecked(false);
}

void ApplicationWindow::addText()
{
	Graph *g = activePlotLayer(false);
	if (!g){
		actionAddText->setChecked(false);
		return;
	}

	AddWidgetTool *tool = new AddWidgetTool(AddWidgetTool::Text, g, actionAddText);
	connect(tool, &AddWidgetTool::statusText, info, &QLineEdit::setText);
	g->setActiveTool(tool);
	btnPointer->setChecked(false);
}

void ApplicationWindow::addImage()
{
	Graph *g = activePlotLayer(false);
	if (!g)
		return;

	QString fn = getFileName(this, tr("QtiPlot - Insert image from file"), imagesDirPath, imageFilter(), 0, false);
	if (!fn.isEmpty()){
		QFileInfo fi(fn);
		imagesDirPath = fi.absolutePath();

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		g->addImage(fn);
		QApplication::restoreOverrideCursor();
	}
}

void ApplicationWindow::drawLine()
{
	Graph *g = activePlotLayer();
	if (g){
		g->drawLine(true);
		emit modified();
	}
}

void ApplicationWindow::drawArrow()
{
	Graph *g = activePlotLayer();
	if (g){
		g->drawLine(true, 1);
		emit modified();
	}
}

void ApplicationWindow::showLayerDialog()
{
	Graph *g = activePlotLayer(false);
	if (!g)
		return;

	LayerDialog *id = new LayerDialog(this);
	id->setMultiLayer(g->multiLayer());
	id->exec();
}

void ApplicationWindow::showEnrichementDialog()
{
	Graph *g = activePlotLayer(false);
	if (!g)
		return;

	FrameWidget *w = g->activeEnrichment();
	EnrichmentDialog::WidgetType wt = EnrichmentDialog::Text;
	if (qobject_cast<RectangleWidget *>(w))
		wt = EnrichmentDialog::Frame;
	else if (qobject_cast<EllipseWidget *>(w))
		wt = EnrichmentDialog::Ellipse;
	else if (qobject_cast<ImageWidget *>(w))
		wt = EnrichmentDialog::Image;
	else if (qobject_cast<TexWidget *>(w))
		wt = EnrichmentDialog::Tex;

	EnrichmentDialog *ed = new EnrichmentDialog(wt, g, this, this);
	ed->setWidget(w);
	ed->exec();
}

void ApplicationWindow::showLineDialog()
{
	Graph *g = activePlotLayer(false);
	if (g){
		ArrowMarker *lm = g->selectedArrow();
		if (!lm)
			return;

		LineDialog *ld = new LineDialog(lm, this);
		ld->exec();

		g->deselectMarker();
	}
}

void ApplicationWindow::addColToTable()
{
	Table* m = (Table*)activeWindow(TableWindow);
	if ( m )
		m->addCol();
}

void ApplicationWindow::clearSelection()
{
	if (results->hasFocus()){
		clearLogInfo();
		return;
	} else if(lv->hasFocus()){
		deleteSelectedItems();
		return;
	}

	MdiSubWindow* m = activeWindow();
	if (!m)
		return;

	if (m->inherits("Table"))
		((Table*)m)->clearSelection();
	else if (m->inherits("Matrix"))
		((Matrix*)m)->clearSelection();
	else if (m->inherits("MultiLayer")){
		Graph* g = ((MultiLayer*)m)->activeLayer();
		if (!g)
			return;

        if (((MultiLayer*)m)->hasSelectedLayers()){
            ((MultiLayer*)m)->confirmRemoveLayer();
            emit modified();
            return;
        }

		if (g->rangeSelectorsEnabled())
			g->rangeSelectorTool()->clearSelection();
		else if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)
			((DataPickerTool *)g->activeTool())->removePoint();
        else if (g->titleSelected())
			g->clearTitle();
		else if (g->axisTitleSelected()){
			QwtScaleWidget *axis = g->currentScale();
			if (axis)
				axis->setTitle(" ");
		} else if (g->selectedCurveLabels()){
			QwtPlotItem *i = g->selectedCurveLabels();
			if(i->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
				((Spectrogram *)i)->showContourLineLabels(false);
			else if(i->rtti() == QwtPlotItem::Rtti_PlotCurve && ((PlotCurve *)i)->rtti() != Graph::Function)
				((DataCurve *)i)->clearLabels();
			g->replot();
		} else
			g->removeMarker();
	}
	else if (m->inherits("Note"))
		((Note*)m)->currentEditor()->textCursor().removeSelectedText();
	emit modified();
}

void ApplicationWindow::copySelection()
{
	if(results->hasFocus()){
		results->copy();
		return;
	} else if(info->hasFocus()) {
		info->copy();
		return;
	}

	MdiSubWindow* m = activeWindow();
	if (!m)
		return;

	if (m->inherits("Table"))
		((Table*)m)->copySelection();
	else if (m->inherits("Matrix"))
		((Matrix*)m)->copySelection();
	else if (m->inherits("MultiLayer")){
		MultiLayer* plot = (MultiLayer*)m;
		if (!plot || plot->numLayers() == 0)
			return;

		Graph* g = (Graph*)plot->activeLayer();
		if (!g)
            return;

		if (g->rangeSelectorsEnabled())
			g->rangeSelectorTool()->copySelection();
		else if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)
			((DataPickerTool *)g->activeTool())->copySelection();
		else if (g->markerSelected())
			copyMarker();
		else
			d_plot_controller_2d->copyActiveLayer();
	} else if (m->inherits("Note"))
		((Note*)m)->currentEditor()->copy();
}

void ApplicationWindow::cutSelection()
{
	MdiSubWindow* m = activeWindow();
	if (!m)
		return;

	if (m->inherits("Table"))
		((Table*)m)->cutSelection();
	else if (m->inherits("Matrix"))
		((Matrix*)m)->cutSelection();
	else if(m->inherits("MultiLayer")){
		MultiLayer* plot = (MultiLayer*)m;
		if (!plot || plot->numLayers() == 0)
			return;

		Graph* g = (Graph*)plot->activeLayer();
		if (!g)
            return;

		if (g->rangeSelectorsEnabled())
			g->rangeSelectorTool()->cutSelection();
		else if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)
			((DataPickerTool *)g->activeTool())->cutSelection();
		else {
            copyMarker();
            g->removeMarker();
        }
	} else if (m->inherits("Note"))
		((Note*)m)->currentEditor()->cut();

	emit modified();
}

void ApplicationWindow::copyMarker()
{
    lastCopiedLayer = nullptr;

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (g && g->markerSelected()){
		d_enrichement_copy = nullptr;
		d_arrow_copy = nullptr;
		if (g->activeEnrichment())
			d_enrichement_copy = g->activeEnrichment();
		else if (g->arrowMarkerSelected())
			d_arrow_copy = g->selectedArrow();
	}
}

void ApplicationWindow::pasteSelection()
{
	MdiSubWindow* m = activeWindow();
	if (!m)
		return;

	if (m->inherits("Table"))
		((Table*)m)->pasteSelection();
	else if (m->inherits("Matrix"))
		((Matrix*)m)->pasteSelection();
	else if (m->inherits("Note"))
		((Note*)m)->currentEditor()->paste();
	else if (m->inherits("MultiLayer")){
		MultiLayer* plot = (MultiLayer*)m;
		if (!plot)
			return;
		plot->deselect();

		if (lastCopiedLayer){
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			Graph* g = plot->addLayer();
			g->copy(lastCopiedLayer);
			QPoint pos = plot->canvas()->mapFromGlobal(QCursor::pos());
			g->setCanvasGeometry(pos.x(), pos.y(), lastCopiedLayer->canvas()->width(), lastCopiedLayer->canvas()->height());
			if (g->isWaterfallPlot())
				g->updateDataCurves();
			QApplication::restoreOverrideCursor();
		} else {
			if (plot->numLayers() == 0)
				return;

			Graph* g = (Graph*)plot->activeLayer();
			if (!g)
				return;

			if (g->rangeSelectorsEnabled())
				g->rangeSelectorTool()->pasteSelection();
            else if (g->activeTool() && g->activeTool()->rtti() == PlotToolInterface::Rtti_DataPicker)
				((DataPickerTool *)g->activeTool())->pasteSelection();
            else if (d_enrichement_copy){
				FrameWidget *t = g->add(d_enrichement_copy);
				QPoint pos = g->mapFromGlobal(QCursor::pos());
				if (g->geometry().contains(pos))
					t->move(pos);
				else
					t->move(g->pos());
			} else if (d_arrow_copy){
                ArrowMarker *a = g->addArrow(d_arrow_copy);
                a->setStartPoint(d_arrow_copy->startPointCoord().x(), d_arrow_copy->startPointCoord().y());
                a->setEndPoint(d_arrow_copy->endPointCoord().x(), d_arrow_copy->endPointCoord().y());
                //translate the new arrow 10 pixels to the right;
                a->setStartPoint(a->startPoint() + QPoint(10, 0));
                a->setEndPoint(a->endPoint() + QPoint(10, 0));
                g->replot();
                g->deselectMarker();
			}
		}
	}
	emit modified();
}

MdiSubWindow* ApplicationWindow::clone(MdiSubWindow* w)
{
	if (!w) {
		w = activeWindow();
		if (!w){
			QMessageBox::critical(this,tr("QtiPlot - Duplicate window error"),
				tr("There are no windows available in this folder!"));
			return 0;
		}
	}

	MdiSubWindow* nw = 0;
	MdiSubWindow::Status status = w->status();
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (w->inherits("MultiLayer")){
		MultiLayer *g = (MultiLayer *)w;
		nw = multilayerPlot(generateUniqueName(tr("Graph")), 0, g->getRows(), g->getCols());
		((MultiLayer *)nw)->copy(g);
	} else if (w->inherits("Table")){
		Table *t = (Table *)w;
		QString caption = generateUniqueName(tr("Table"));
    	nw = newTable(caption, t->numRows(), t->numCols());
    	((Table *)nw)->copy(t);
	} else if (w->inherits("Graph3D")){
		Graph3D *g = (Graph3D *)w;
		if (!g->hasData()){
        	QApplication::restoreOverrideCursor();
        	QMessageBox::warning(this, tr("QtiPlot - Duplicate error"), tr("Empty 3D surface plots cannot be duplicated!"));
        	return 0;
    	}
		nw = newPlot3D();
		if (!nw)
			return 0;
        if (status == MdiSubWindow::Maximized)
            nw->hide();
		((Graph3D *)nw)->copy(g);
		customToolBars(nw);
	} else if (w->inherits("Matrix")){
		nw = newMatrix(((Matrix *)w)->numRows(), ((Matrix *)w)->numCols());
    	((Matrix *)nw)->copy((Matrix *)w);
	} else if (w->inherits("Note")){
		nw = newNote();
		if (nw){
			((Note*)nw)->setText(((Note*)w)->text());
			((Note*)nw)->showLineNumbers(((Note*)w)->hasLineNumbers());
		}
	}

	if (nw){
		if (w->inherits("MultiLayer")){
			if (status == MdiSubWindow::Maximized)
				nw->showMaximized();
		} else if (w->inherits("Graph3D")){
			if (status != MdiSubWindow::Maximized){
				bool scale3DFonts = d_3D_scale_fonts;
				d_3D_scale_fonts = false;
				nw->resize(w->size());
				nw->showNormal();
				d_3D_scale_fonts = scale3DFonts;
			} else
				nw->showMaximized();
		} else {
			nw->resize(w->size());
			nw->showNormal();
		}

		nw->setWindowLabel(w->windowLabel());
		nw->setCaptionPolicy(w->captionPolicy());
	}

	QApplication::restoreOverrideCursor();
	customMenu(nw);
	return nw;
}

void ApplicationWindow::undo()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (qobject_cast<Note*>(w)) {
		if (ScriptEdit *editor = ((Note*)w)->currentEditor())
			editor->undo();
	} else if (QUndoStack *stack = w->undoStack())
		stack->undo();

	QApplication::restoreOverrideCursor();

	// Keep toolbar/menu state in sync after executing undo.
	updateUndoRedoActions();
}

void ApplicationWindow::redo()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (qobject_cast<Note*>(w)) {
		if (ScriptEdit *editor = ((Note*)w)->currentEditor())
			editor->redo();
	} else if (QUndoStack *stack = w->undoStack())
		stack->redo();

	QApplication::restoreOverrideCursor();

	// Keep toolbar/menu state in sync after executing redo.
	updateUndoRedoActions();
}

bool ApplicationWindow::hidden(QWidget* window)
{
	if (hiddenWindows->contains(window))
		return true;

	return false;
}

void ApplicationWindow::updateWindowStatus(MdiSubWindow* w)
{
	setListView(w->objectName(), w->aspect());
	if (w->status() == MdiSubWindow::Maximized){
		QList<MdiSubWindow *> windows = current_folder->windowsList();
		for (MdiSubWindow *oldMaxWindow : windows){
			if (oldMaxWindow != w && oldMaxWindow->status() == MdiSubWindow::Maximized)
				oldMaxWindow->setStatus(MdiSubWindow::Normal);
		}
	}
	modifiedProject();
}

void ApplicationWindow::hideActiveWindow()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;

	hideWindow(w);
}

void ApplicationWindow::hideWindow(MdiSubWindow* w)
{
	hiddenWindows->append(w);
	w->setHidden();
	emit modified();
}

void ApplicationWindow::hideWindow()
{
	WindowListItem *it = (WindowListItem *)lv->currentItem();
	MdiSubWindow *w = it->window();
	if (!w)
		return;

	hideWindow(w);
}

void ApplicationWindow::resizeActiveWindow()
{
	MdiSubWindow* w = activeWindow();
	if (!w)
		return;

	EnrichmentDialog *ed = new EnrichmentDialog(EnrichmentDialog::MDIWindow, nullptr, this, this);
    ed->setWidget(w);
    ed->exec();
}

void ApplicationWindow::resizeWindow()
{
	WindowListItem *it = (WindowListItem *)lv->currentItem();
	MdiSubWindow *w = it->window();
	if (!w)
		return;

	d_workspace->setActiveSubWindow(w);

	EnrichmentDialog *ed = new EnrichmentDialog(EnrichmentDialog::MDIWindow, nullptr, this, this);
    ed->setWidget(w);
    ed->exec();
}

void ApplicationWindow::activateWindow()
{
	WindowListItem *it = (WindowListItem *)lv->currentItem();
	activateWindow(it->window());
}

void ApplicationWindow::activateWindow(MdiSubWindow *w)
{
	if (!w)
		return;

	w->setNormal();
	d_workspace->setActiveSubWindow(w);

	updateWindowLists(w);
	emit modified();
}

void ApplicationWindow::maximizeWindow(QTreeWidgetItem * lbi)
{
	if (!lbi)
		lbi = lv->currentItem();

	if (!lbi || lbi->type() == FolderListItem::RTTI)
		return;

	maximizeWindow(((WindowListItem*)lbi)->window());
}

void ApplicationWindow::maximizeWindow(MdiSubWindow *w)
{
	if (!w || w->status() == MdiSubWindow::Maximized)
		return;

	QList<MdiSubWindow *> windows = current_folder->windowsList();
	for (MdiSubWindow *ow : windows){
		if (ow != w && ow->status() == MdiSubWindow::Maximized){
			ow->setNormal();
			break;
		}
	}

	w->setMaximized();
	updateWindowLists(w);
	emit modified();
}

void ApplicationWindow::minimizeWindow(MdiSubWindow *w)
{
	if (!w)
		w = ((WindowListItem *)lv->currentItem())->window();

	if (!w)
		return;

	updateWindowLists(w);
	w->setMinimized();
	emit modified();
}

void ApplicationWindow::updateWindowLists(MdiSubWindow *w)
{
	if (!w)
		return;

	if (hiddenWindows->contains(w))
		hiddenWindows->takeAt(hiddenWindows->indexOf(w));
}

void ApplicationWindow::closeActiveWindow()
{
	MdiSubWindow *w = activeWindow();
	if (w)
		w->close();
}

void ApplicationWindow::removeWindowFromLists(MdiSubWindow* w)
{
	if (!w)
		return;

	QString caption = w->objectName();
	if (w->inherits("Table")){
		Table* m = (Table*)w;
		for (int i = 0; i < m->numCols(); i++)
			removeCurves(m->colName(i));
	} else if (w->inherits("MultiLayer")){
		MultiLayer *ml = (MultiLayer*)w;
		Graph *g = ml->activeLayer();
		if (g)
			btnPointer->setChecked(true);
	} else if (w->inherits("Matrix"))
		d_plot_controller_3d->remove3DMatrixPlots((Matrix*)w);

	if (hiddenWindows->contains(w))
		hiddenWindows->takeAt(hiddenWindows->indexOf(w));

	updateCompleter(caption, true);
}

void ApplicationWindow::closeWindow(MdiSubWindow* window)
{
	if (!window)
		return;

	if (d_active_window == window)
		d_active_window = nullptr;

	removeWindowFromLists(window);
	Folder *f = window->folder();
	f->removeWindow(window);

	//update list view in project explorer
	QTreeWidgetItem *it = lv->findItem (window->objectName(), 0, Qt::MatchExactly | Qt::MatchCaseSensitive);
	if (it){
		if (it->parent())
			it->parent()->removeChild(it);
		else
			lv->takeTopLevelItem(lv->indexOfTopLevelItem(it));
		delete it;
	}

	window->close();

	if (show_windows_policy == ActiveFolder && !f->windowsList().count()){
		customMenu(0);
		customToolBars(0);
	} else if (show_windows_policy == SubFolders && !(current_folder->children()).isEmpty()){
		FolderListItem *fi = current_folder->folderListItem();
		FolderListItem *item = (FolderListItem *)fi->treeWidget()->topLevelItem(0);
		int initial_depth = item->depth();
		bool emptyFolder = true;
		while (item && item->depth() >= initial_depth){
			QList<MdiSubWindow *> lst = item->folder()->windowsList();
			if (lst.count() > 0){
				emptyFolder = false;
				break;
			}
			item = (FolderListItem *)item->itemBelow();
		}
		if (emptyFolder){
			customMenu(0);
			customToolBars(0);
		}
	}
	emit modified();
}

QMessageBox * ApplicationWindow::about(bool dialog)
{
	QString htmlText, plainText;

	// Basic Info
	htmlText += "<h2>"+ versionString() + "</h2>";
	plainText += versionString() + "\n";

	htmlText +=	"<h3>" + QString(copyright_string).replace("\n", "<br>") + "</h3>";
	plainText += QString(copyright_string) + "\n";

	QString released = tr("Released") + ": " + QString(release_date);
	htmlText += "<h3>" + released + "</h3>";
	plainText += released + "\n";
	
	// Libraries
	htmlText += "<h3>" + tr("Libraries") + "</h3><p>";
	plainText += "\n" + tr("Libraries") + ":\n";

	auto addLib = [&](const QString& name, const QString& version, const QString& url) {
		htmlText += QString("<a href=\"%1\">%2</a> %3<br>").arg(url, name, version);
		plainText += QString("%1 %2\n").arg(name, version);
	};

	addLib("Qt", qVersion(), "https://www.qt.io");
	addLib("Qwt", QWT_VERSION_STR, "https://qwt.sourceforge.io");
	addLib("QwtPlot3D", QString("%1.%2.%3").arg(QWT3D_MAJOR_VERSION).arg(QWT3D_MINOR_VERSION).arg(QWT3D_PATCH_VERSION), "http://qwtplot3d.sourceforge.net");
	addLib("GSL", GSL_VERSION, "https://www.gnu.org/software/gsl");
#ifdef _UNICODE
	addLib("muParser", QString::fromStdWString(mu::ParserVersion), "https://beltoforion.de/en/muparser");
#else
	addLib("muParser", QString::fromStdString(mu::ParserVersion), "https://beltoforion.de/en/muparser");
#endif
	addLib("zlib", ZLIB_VERSION, "https://zlib.net");
#ifdef SCRIPTING_PYTHON
	addLib("Python", QString(Py_GetVersion()).split(" ")[0], "https://www.python.org");
#endif
	htmlText += "</p>";

	// Compiler
	htmlText += "<h3>" + tr("Compiler") + "</h3><p>";
	plainText += "\n" + tr("Compiler") + ":\n";

	QString compiler;
#if defined(__clang__)
	compiler = "Clang " + QString::number(__clang_major__) + "." + QString::number(__clang_minor__) + "." + QString::number(__clang_patchlevel__);
#elif defined(__GNUC__)
	compiler = "GCC " + QString::number(__GNUC__) + "." + QString::number(__GNUC_MINOR__) + "." + QString::number(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
	compiler = "MSVC " + QString::number(_MSC_VER);
#else
	compiler = "Unknown Compiler";
#endif
	htmlText += compiler + "</p>";
	plainText += compiler + "\n";

	// System
	htmlText += "<h3>" + tr("System") + "</h3><p>";
	plainText += "\n" + tr("System") + ":\n";

	QString arch;
#if defined(_M_X64) || defined(__x86_64__)
	arch = "x86_64";
#elif defined(_M_IX86) || defined(__i386__)
	arch = "x86";
#elif defined(_M_ARM64) || defined(__aarch64__)
	arch = "ARM64";
#elif defined(_M_ARM) || defined(__arm__)
	arch = "ARM";
#else
	arch = "Unknown Architecture";
#endif
	arch += " " + QString::number(sizeof(void*) * 8) + "-bit";
	QString built_on = tr("Built on") + ": " + __DATE__ + " " + __TIME__;

	htmlText += arch + "<br>" + built_on + "</p>";
	plainText += arch + "\n" + built_on + "\n";

	if (dialog){
		QMessageBox *mb = new QMessageBox();
		mb->setAttribute(Qt::WA_DeleteOnClose);
		mb->setWindowTitle (tr("About QtiPlot"));
		mb->setWindowIcon(QIcon(":/logo.png"));
		mb->setIconPixmap(QPixmap(":/logo.png"));
		mb->setText(htmlText);
		mb->exec();
		return mb;
	} else {
		printf("%s", plainText.toLocal8Bit().constData());
		exit(0);
	}
	return nullptr;
}

void ApplicationWindow::scriptingMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->scriptingMenuAboutToShow();
}

void ApplicationWindow::analysisMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->analysisMenuAboutToShow();
}

void ApplicationWindow::matrixMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->matrixMenuAboutToShow();
}

void ApplicationWindow::fileMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->fileMenuAboutToShow();
}

void ApplicationWindow::updateUndoRedoActions()
{
	MdiSubWindow *w = activeWindow();
	if (!w) {
		actionUndo->setEnabled(false);
		actionRedo->setEnabled(false);
		return;
	}

	if (qobject_cast<Note *>(w)) {
		ScriptEdit *editor = ((Note *)w)->currentEditor();
		QTextDocument *doc = editor ? editor->document() : nullptr;
		actionUndo->setEnabled(doc && doc->isUndoAvailable());
		actionRedo->setEnabled(doc && doc->isRedoAvailable());
	} else {
		// Table, Matrix, MultiLayer/Graph — all expose undoStack().
		QUndoStack *stack = w->undoStack();
		actionUndo->setEnabled(stack && stack->canUndo());
		actionRedo->setEnabled(stack && stack->canRedo());
	}
}

void ApplicationWindow::editMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->editMenuAboutToShow();
}

void ApplicationWindow::windowsMenuAboutToShow()
{
	if (d_action_manager)
		d_action_manager->windowsMenuAboutToShow();
}

void ApplicationWindow::showMarkerPopupMenu()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	QMenu markerMenu(this);

	if (g->imageMarkerSelected()){
		markerMenu.addAction(QIcon(":/pixelProfile.png"), tr("&View Pixel Line profile"), this, &ApplicationWindow::pixelLineProfile);
		markerMenu.addAction(tr("&Intensity Matrix"), this, &ApplicationWindow::intensityTable);
		markerMenu.addSeparator();
	}
	if (!g->activeEnrichment())
		markerMenu.addAction(QIcon(":/cut.png"), tr("&Cut"), this, &ApplicationWindow::cutSelection);
	markerMenu.addAction(QIcon(":/copy.png"), tr("&Copy"), this, &ApplicationWindow::copySelection);
	markerMenu.addAction(QIcon(":/delete.png"), tr("&Delete"), this, &ApplicationWindow::clearSelection);
	markerMenu.addSeparator();

	if (g->activeEnrichment()){
		markerMenu.addAction(actionRaiseEnrichment);
		markerMenu.addAction(actionLowerEnrichment);
		markerMenu.addSeparator();
	}

	if (g->arrowMarkerSelected())
		markerMenu.addAction(tr("&Properties..."), this, &ApplicationWindow::showLineDialog);
	else
		markerMenu.addAction(tr("&Properties..."), this, &ApplicationWindow::showEnrichementDialog);

	markerMenu.exec(QCursor::pos());
}

void ApplicationWindow::lowerActiveEnrichment()
{
	raiseActiveEnrichment(false);
}

void ApplicationWindow::raiseActiveEnrichment(bool on)
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	if (g->selectionMoveResizer())
		g->selectionMoveResizer()->raiseTargets(on);
}

void ApplicationWindow::alignTop()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	if (g->selectionMoveResizer())
		g->selectionMoveResizer()->alignTargetsTop();
}

void ApplicationWindow::alignBottom()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	if (g->selectionMoveResizer())
		g->selectionMoveResizer()->alignTargetsBottom();
}

void ApplicationWindow::alignLeft()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	if (g->selectionMoveResizer())
		g->selectionMoveResizer()->alignTargetsLeft();
}

void ApplicationWindow::alignRight()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	if (g->selectionMoveResizer())
		g->selectionMoveResizer()->alignTargetsRight();
}

void ApplicationWindow::graphSelectionChanged(SelectionMoveResizer *s)
{
	if (s){
		actionRaiseEnrichment->setEnabled(true);
		actionLowerEnrichment->setEnabled(true);
		if (s->widgetsList().size() > 1){
			actionAlignTop->setEnabled(true);
			actionAlignBottom->setEnabled(true);
			actionAlignLeft->setEnabled(true);
			actionAlignRight->setEnabled(true);
		}
	} else {
		actionRaiseEnrichment->setEnabled(false);
		actionLowerEnrichment->setEnabled(false);
		actionAlignTop->setEnabled(false);
		actionAlignBottom->setEnabled(false);
		actionAlignLeft->setEnabled(false);
		actionAlignRight->setEnabled(false);
	}
}

void ApplicationWindow::showMoreWindows()
{
	if (explorerWindow->isVisible())
		QMessageBox::information(this, "QtiPlot",tr("Please use the project explorer to select a window!"));
	else
		explorerWindow->show();
}

void ApplicationWindow::windowsMenuActivated( int id )
{
	QList<MdiSubWindow *> windows = current_folder->windowsList();
	MdiSubWindow* w = windows.at( id );
	if ( w ){
		MdiSubWindow* aw = (MdiSubWindow*)d_workspace->activeSubWindow();
		bool maximize = aw && aw->isMaximized();

		d_workspace->setActiveSubWindow(w);
		if (maximize)
			w->showMaximized();
		else
			w->showNormal();
		w->setFocus();

		if(hidden(w))
			hiddenWindows->takeAt(hiddenWindows->indexOf(w));
	}
}

void ApplicationWindow::foldersMenuTriggered( QAction *a )
{
	if (a) foldersMenuActivated(a->data().toInt());
}

void ApplicationWindow::windowsMenuTriggered( QAction *a )
{
	if (a) windowsMenuActivated(a->data().toInt());
}

void ApplicationWindow::foldersMenuActivated( int id )
{
	int folder_param = 0;
	Folder *f = projectFolder();
	while (f){
		if (folder_param == id){
			changeFolder (f);
			return;
		}

		folder_param++;
		f = f->folderBelow();
	}
}

void ApplicationWindow::newProject()
{
	if (d_project_manager)
		d_project_manager->newProject();
}

void ApplicationWindow::savedProject()
{
	if (d_project_manager)
		d_project_manager->savedProject();
}

void ApplicationWindow::modifiedProject()
{
	if (d_project_manager)
		d_project_manager->modifiedProject();
}

void ApplicationWindow::modifiedProject(MdiSubWindow *w)
{
	if (d_project_manager)
		d_project_manager->modifiedProject(w);
}

void ApplicationWindow::timerEvent(QTimerEvent *e)
{
	if (d_project_manager && d_project_manager->handleTimerEvent(e))
		return;
	QWidget::timerEvent(e);
}

void ApplicationWindow::dropEvent( QDropEvent* e )
{
	if (!e->mimeData()->hasImage() && !e->mimeData()->hasUrls())
		return;

	MdiSubWindow *destWindow = nullptr;
	QList<QMdiSubWindow *> windows = d_workspace->subWindowList(QMdiArea::StackingOrder);
	QListIterator<QMdiSubWindow *> it(windows);
	it.toBack();
	QPoint pos = d_workspace->mapFromGlobal(e->position().toPoint());
	while (it.hasPrevious()){
		QMdiSubWindow *w = it.previous();
		if (w->frameGeometry().contains(pos)){
			destWindow = (MdiSubWindow*)w;
			break;
		}
	}

	if (e->mimeData()->hasImage()){
		QImage image = qvariant_cast<QImage>(e->mimeData()->imageData());
		MultiLayer *ml = qobject_cast<MultiLayer *>(destWindow);
        if (ml){
            Graph *l = ml->layerAt(pos);
            if (l)
                l->addImage(image);
            else if (ml->activeLayer())
                ml->activeLayer()->addImage(image);
            else
                ml->addLayer()->addImage(image);
            return;
        }

		Matrix *m = qobject_cast<Matrix *>(destWindow);
        if (m)
			m->importImage(image);
		else {
			m = new Matrix(scriptEnv, image, "", this);
        	initMatrix(m, generateUniqueName(tr("Matrix")));
        	m->show();
		}
		return;
	}

	if (e->mimeData()->hasUrls()){
		QList<QUrl> urls = e->mimeData()->urls();
		QStringList fileNames;
		for (QUrl url : urls)
			fileNames << url.toLocalFile();

		QList<QByteArray> lst = QImageReader::supportedImageFormats() << "JPG";
		QStringList asciiFiles;

		for(int i = 0; i<(int)fileNames.count(); i++){
			QString fn = fileNames[i];
			QFileInfo fi (fn);
			QString ext = fi.suffix();
			QStringList tempList;
			// convert QList<QByteArray> to QStringList to be able to 'filter'
			for (QByteArray temp : lst)
				tempList.append(QString(temp));
			QStringList l = tempList.filter(ext, Qt::CaseInsensitive);
			if (l.count() > 0){
			    MultiLayer *ml = qobject_cast<MultiLayer *>(destWindow);
			    if (ml){
			        Graph *l = ml->layerAt(pos);
			        if (l)
                        l->addImage(fn);
			        else if (ml->activeLayer())
                        ml->activeLayer()->addImage(fn);
                    else
                        ml->addLayer()->addImage(fn);
			    } else {
			        Matrix *m = qobject_cast<Matrix *>(destWindow);
                    if (m)
                        m->importImage(fn);
                    else
                        importImage(fn, true);
			    }
			} else if ( ext == "opj" || ext == "qti")
				open(fn);
			else
				asciiFiles << fn;
		}

		if (asciiFiles.count() == 1)
			return importASCII(asciiFiles[0]);
		else
			importASCII(asciiFiles, ImportASCIIDialog::NewTables, columnSeparator, ignoredLines,
					renameColumns, strip_spaces, simplify_spaces, d_ASCII_import_comments,
					d_ASCII_import_locale, d_ASCII_comment_string, d_ASCII_import_read_only, d_ASCII_end_line);
	}
}

void ApplicationWindow::dragEnterEvent( QDragEnterEvent* e )
{
	if (e->source()){
		e->ignore();
		return;
	}

	if (e->mimeData()->hasUrls())
		e->accept();
	else
		e->ignore();
}

void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
	switch(showSaveProjectMessage()){
		case QMessageBox::Yes:
			if (!saveProject()){
				ce->ignore();
				break;
			}
			CrashHandler::cleanSessionRecoveryFiles();
			saveSettings();
			ce->accept();
			break;

		case QMessageBox::No:
		default:
			CrashHandler::cleanSessionRecoveryFiles();
			saveSettings();
			ce->accept();
			break;

		case QMessageBox::Cancel:
			ce->ignore();
			break;
	}
}

#ifdef Q_WS_MAC
void ApplicationWindow::hideEvent (QHideEvent * event)
{
	if (event->spontaneous())
		((QtiPlotApplication *)QCoreApplication::instance())->updateDockMenu();
	event->accept();
}
#endif

QMessageBox::StandardButton ApplicationWindow::showSaveProjectMessage()
{
	if (d_project_manager)
		return d_project_manager->showSaveProjectMessage();
	return QMessageBox::No;
}

void ApplicationWindow::closeProject()
{
	if (d_project_manager)
		d_project_manager->closeProject();
}

bool ApplicationWindow::isLedgerClean() const
{
#ifdef QTIPLOT_LEDGER
	return AllocLedger::instance().clean();
#else
	return true;
#endif
}

QString ApplicationWindow::ledgerReport() const
{
#ifdef QTIPLOT_LEDGER
	return AllocLedger::instance().report();
#else
	return QString();
#endif
}

void ApplicationWindow::resetLedger()
{
#ifdef QTIPLOT_LEDGER
	AllocLedger::instance().reset();
#endif
}

void ApplicationWindow::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

void ApplicationWindow::deleteSelectedItems()
{
	if (folders->hasFocus() && folders->currentItem() != folders->topLevelItem(0))
	{//we never allow the user to delete the project folder item
		deleteFolder();
		return;
	}

	QList<Folder *> folderList;
	QList<MdiSubWindow *> windowList;
	QList<QTreeWidgetItem *> selected = lv->selectedItems();
    for (QTreeWidgetItem *item : selected){
		if (item->type() == FolderListItem::RTTI)
			folderList << ((FolderListItem *)item)->folder();
		else
			windowList << ((WindowListItem *)item)->window();
	}

	folders->blockSignals(true);
	for (MdiSubWindow *w : windowList)
		w->close();
	for (Folder *f : folderList)
		deleteFolder(f);
	folders->blockSignals(false);
}

void ApplicationWindow::showListViewSelectionMenu(const QPoint &p)
{
	QMenu cm(this);
	cm.addAction(tr("&Show All Windows"), this, &ApplicationWindow::showSelectedWindows);
	cm.addAction(tr("&Hide All Windows"), this, &ApplicationWindow::hideSelectedWindows);
	cm.addSeparator();
	cm.addAction(tr("&Delete Selection"), Qt::Key_F8, this, &ApplicationWindow::deleteSelectedItems);
	cm.exec(p);
}

void ApplicationWindow::showListViewPopupMenu(const QPoint &p)
{
	QMenu cm(this);
	QMenu window(this);

	window.addAction(actionNewTable);
	window.addAction(actionNewMatrix);
	window.addAction(actionNewNote);
	window.addAction(actionNewGraph);
	window.addAction(actionNewFunctionPlot);
	window.addAction(actionNewSurfacePlot);
	window.setTitle(tr("New &Window"));
	cm.addMenu(&window);

	cm.addAction(actionNewFolder);
	cm.addSeparator();
	cm.addAction(tr("Auto &Column Width"), lv, &FolderListView::adjustColumns);
	cm.exec(p);
}

void ApplicationWindow::showWindowPopupMenu(QTreeWidgetItem *it, const QPoint &p, int)
{
	if (folders->isRenaming())
		return;

	if (!it){
		showListViewPopupMenu(p);
		return;
	}

	int selected = 0;
	for (int i = 0; i < lv->topLevelItemCount(); ++i){
		QTreeWidgetItem *item = lv->topLevelItem(i);
		if (item->isSelected())
			selected++;

		if (selected>1){
			showListViewSelectionMenu(p);
			return;
		}
	}

	if (it->type() == FolderListItem::RTTI){
		current_folder = ((FolderListItem *)it)->folder();
		showFolderPopupMenu(it, p, false);
		return;
	}

	MdiSubWindow *w = ((WindowListItem *)it)->window();
	if (w){
		QMenu cm(this);
		QMenu plots(this);

		cm.addAction(actionActivateWindow);
		cm.addAction(actionMinimizeWindow);
		cm.addAction(actionMaximizeWindow);
		cm.addSeparator();
		if (!hidden(w))
			cm.addAction(actionHideWindow);
		cm.addAction(QIcon(":/close.png"), tr("&Delete Window"), Qt::Key_F8, w, &MdiSubWindow::close);
		cm.addSeparator();
		QAction *a = cm.addAction(tr("&Rename Window"), this, qOverload<>(&ApplicationWindow::renameWindow));
	a->setShortcut(Qt::Key_F2);
		cm.addAction(actionResizeWindow);
		cm.addSeparator();
		cm.addAction(QIcon(":/fileprint.png"), tr("&Print Window"), w, qOverload<>(&MdiSubWindow::print));
		cm.addSeparator();
		cm.addAction(tr("&Properties..."), this, &ApplicationWindow::windowProperties);

		if (w->inherits("Table")){
			QStringList graphs = dependingPlots(w->objectName());
			if (int(graphs.count())>0){
				cm.addSeparator();
				for (int i=0;i<int(graphs.count());i++)
					plots.addAction(graphs[i], window(graphs[i]), &MdiSubWindow::showMaximized);

				plots.setTitle(tr("D&epending Graphs"));
				cm.addMenu(&plots);
			}
		} else if (w->inherits("Matrix")){
			QStringList graphs = depending3DPlots((Matrix*)w);
			if (int(graphs.count())>0){
				cm.addSeparator();
				for (int i=0;i<int(graphs.count());i++)
					plots.addAction(graphs[i], window(graphs[i]), &MdiSubWindow::showMaximized);

				plots.setTitle(tr("D&epending 3D Graphs"));
				cm.addMenu(&plots);
			}
		} else if (w->inherits("MultiLayer")) {
			tablesDepend->clear();
			QStringList tbls = multilayerDependencies(w);
			int n = int(tbls.count());
			if (n > 0){
				cm.addSeparator();
				for (int i=0; i<n; i++)
					tablesDepend->addAction(tbls[i]);

				tablesDepend->setTitle(tr("D&epends on"));
				cm.addMenu(tablesDepend);
			}
		} else if (w->inherits("Graph3D")){
			cm.addSeparator();
			Graph3D *sp = qobject_cast<Graph3D*>(w);
			Matrix *m = sp->matrix();
			if (m){
				plots.addAction(m->objectName(), m, &MdiSubWindow::showMaximized);
				plots.setTitle(tr("D&epends on"));
				cm.addMenu(&plots);
			} else if (sp->table()){
				tablesDepend->clear();
				tablesDepend->addAction(sp->table()->objectName());
				tablesDepend->setTitle(tr("D&epends on"));
				cm.addMenu(tablesDepend);
			} else if (!sp->formula().isEmpty()){
				plots.addAction(sp->formula());
				plots.setTitle(tr("Function"));
				cm.addMenu(&plots);
			}
		}
		cm.exec(p);
	}
}

void ApplicationWindow::showTable(QAction *action)
{
	if (!action)
		return;
	Table *t = table(action->text());
	if (!t)
		return;

	updateWindowLists(t);

	t->showMaximized();
	QTreeWidgetItem *it = lv->findItem (t->objectName(), 0, Qt::MatchExactly | Qt::MatchCaseSensitive );
	if (it)
		it->setText(2, tr("Maximized"));
}

void ApplicationWindow::showTable(Table *w, const QString& curve)
{
	if (!w)
		return;

	updateWindowLists(w);
	int colIndex = w->colIndex(curve);
	w->setSelectedCol(colIndex);
	w->table()->clearSelection();
	w->table()->selectColumn(colIndex);
	w->table()->ensureCellVisible(0, colIndex);
	w->showMaximized();
	QTreeWidgetItem *it = lv->findItem (w->objectName(), 0, Qt::MatchExactly | Qt::MatchCaseSensitive );
	if (it)
		it->setText(2, tr("Maximized"));
	emit modified();
}

QStringList ApplicationWindow::depending3DPlots(Matrix *m)
{
	QStringList plots;
	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Graph3D") && ((Graph3D *)w)->matrix() == m)
			plots << w->objectName();
	}
	return plots;
}

QStringList ApplicationWindow::dependingPlots(const QString& name)
{
	QStringList plots;

	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			for (Graph *g : layers){
				QStringList onPlot = g->curveNamesList();
				onPlot = onPlot.filter (name);
				if (int(onPlot.count()) && plots.contains(w->objectName())<=0)
					plots << w->objectName();
			}
		}else if (w->inherits("Graph3D")){
			if ((((Graph3D*)w)->formula()).contains(name) && plots.contains(w->objectName())<=0)
				plots << w->objectName();
		}
	}
	return plots;
}

QStringList ApplicationWindow::multilayerDependencies(QWidget *w)
{
	QStringList tables;
	MultiLayer *g=(MultiLayer*)w;
	QList<Graph *> layers = g->layersList();
    for (Graph *ag : layers){
		QStringList onPlot = ag->curveNamesList();
		for (int j=0; j<onPlot.count(); j++)
		{
			QStringList tl = onPlot[j].split("_", Qt::SkipEmptyParts);
			if (tables.contains(tl[0])<=0)
				tables << tl[0];
		}
	}
	return tables;
}

void ApplicationWindow::showGraphContextMenu()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* ag = (Graph*)plot->activeLayer();
	if (!ag)
		return;

	QMenu cm(this);
	if (plot->isLayerSelected(ag)){
		cm.addAction(QPixmap(":/copy.png"), tr("&Copy"), d_plot_controller_2d, &PlotController2D::copyActiveLayer);
		if (lastCopiedLayer)
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste Layer"), this, &ApplicationWindow::pasteSelection);
		else if (d_enrichement_copy){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		} else if (d_arrow_copy){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Line/Arrow"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		}
		cm.addAction(actionDeleteLayer);
		cm.addSeparator();
		cm.addAction(tr("P&roperties..."), this, &ApplicationWindow::showGeneralPlotDialog);
		cm.exec(QCursor::pos());
		return;
	}

	QMenu addMenu(this);
	if (ag->isPiePlot()){
		cm.addAction(tr("Re&move Pie Curve"), ag, &Graph::removePie);
		addMenu.addAction(actionNewLegend);
	} else {
		if (ag->visibleCurves() != ag->curveCount()){
			cm.addAction(actionShowAllCurves);
			cm.addSeparator();
		}

		addMenu.addAction(actionShowCurvesDialog);
		addMenu.addAction(actionAddFunctionCurve);
		addMenu.addAction(actionAddErrorBars);
		addMenu.addAction(actionNewLegend);
		addMenu.addSeparator();
	}

	addMenu.addAction(actionAddFormula);
	addMenu.addAction(actionAddText);
	addMenu.addAction(btnArrow);
	addMenu.addAction(btnLine);
	addMenu.addAction(actionAddRectangle);
	addMenu.addAction(actionAddEllipse);
	addMenu.addAction(actionTimeStamp);
	addMenu.addAction(actionAddImage);
	addMenu.addSeparator();
	addMenu.addAction(actionAddLayer);
	addMenu.addAction(actionAddInsetLayer);
	addMenu.addAction(actionAddInsetCurveLayer);
	addMenu.setTitle(tr("&Add"));
	cm.addMenu(&addMenu);

	QMenu paletteMenu(this);
	if (!ag->isPiePlot()){
		analysisMenu->setTitle(tr("Anal&yze"));
		cm.addMenu(analysisMenu);
		plotDataMenu->setTitle(tr("&Data"));
		cm.addMenu(plotDataMenu);

		paletteMenu.addAction(tr("&Gray Scale"), ag, &Graph::setGrayScale);
		paletteMenu.addAction(tr("&Indexed Colors"), ag, &Graph::setIndexedColors);
		paletteMenu.setTitle(tr("Pale&tte"));
		cm.addMenu(&paletteMenu);
		cm.addSeparator();
	}

	QMenu copy(this);
	copy.addAction(tr("&Layer"), d_plot_controller_2d, &PlotController2D::copyActiveLayer);
	copy.addAction(tr("&Window"), static_cast<MultiLayer *>(plot), &MultiLayer::copyAllLayers);
	copy.setTitle(tr("&Copy"));
	copy.setIcon(QIcon(":/copy.png"));
	cm.addMenu(&copy);

	if (lastCopiedLayer)
		cm.addAction(QPixmap(":/paste.png"), tr("&Paste Layer"), this, &ApplicationWindow::pasteSelection);
	else if (d_enrichement_copy){
		if (qobject_cast<LegendWidget *>(d_enrichement_copy)){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Text"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		} else if (qobject_cast<TexWidget *>(d_enrichement_copy)){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Tex Formula"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		} else if (qobject_cast<ImageWidget *>(d_enrichement_copy)){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Image"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		} else if (qobject_cast<RectangleWidget *>(d_enrichement_copy)){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Rectangle"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		} else if (qobject_cast<EllipseWidget *>(d_enrichement_copy)){
			QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Ellipse"));
			connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
		}
	} else if (d_arrow_copy){
		QAction *a = cm.addAction(QPixmap(":/paste.png"), tr("&Paste Line/Arrow"));
		connect(a, &QAction::triggered, plot, &MultiLayer::pasteMarker);
	}

	QMenu exports(this);
	exports.addAction(actionExportLayer);
	exports.addAction(actionExportGraph);
	exports.setTitle(tr("E&xport"));
	cm.addMenu(&exports);

	QMenu prints(this);
	prints.addAction(tr("&Layer") + "...", static_cast<MultiLayer *>(plot), &MultiLayer::printActiveLayer);
	prints.addAction(tr("&Window") + "...", static_cast<MultiLayer *>(plot), qOverload<>(&MultiLayer::print));
	prints.setTitle(tr("&Print"));
	prints.setIcon(QIcon(":/fileprint.png"));
	cm.addMenu(&prints);

	cm.addSeparator();

	cm.addAction(tr("P&roperties..."), this, &ApplicationWindow::showGeneralPlotDialog);
	cm.addSeparator();
	cm.addAction(actionDeleteLayer);
	cm.exec(QCursor::pos());
}

void ApplicationWindow::showWindowContextMenu()
{
	MdiSubWindow* w = activeWindow();
	if (!w)
		return;

	QMenu cm(this);
	QMenu plot3D(this);
	if (qobject_cast<MultiLayer *>(w)){
		MultiLayer *g = (MultiLayer*)w;
		if (!g->geometry().contains(d_workspace->mapFromGlobal(QCursor::pos())))
			return;

		if (lastCopiedLayer){
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste Layer"), this, &ApplicationWindow::pasteSelection);
			cm.addSeparator();
		}

		cm.addAction(actionAddLayer);
		if (g->numLayers() != 0)
			cm.addAction(actionDeleteLayer);

		cm.addAction(actionShowLayerDialog);
		cm.addSeparator();
		cm.addAction(actionRename);
		cm.addAction(actionCopyWindow);
		cm.addSeparator();
		cm.addAction(QPixmap(":/copy.png"), tr("&Copy Page"), static_cast<MultiLayer *>(g), &MultiLayer::copyAllLayers);
		cm.addAction(tr("E&xport Page"), this, [this]{ d_export_manager->exportGraph(); });
		cm.addAction(actionPrint);
		cm.addSeparator();
		cm.addAction(tr("&Properties..."), this, &ApplicationWindow::showGeneralPlotDialog);
		cm.addSeparator();
		cm.addAction(actionCloseWindow);
	} else if (w->inherits("Graph3D")){
		Graph3D *g = (Graph3D*)w;
		if (!g->hasData()){
			plot3D.setTitle(tr("3D &Plot"));
			cm.addMenu(&plot3D);
			if (hasTable())
				plot3D.addAction(actionAdd3DData);
			if (matrixNames().count())
				plot3D.addAction(tr("&Matrix..."), d_plot_controller_3d, &PlotController3D::add3DMatrixPlot);
			plot3D.addAction(actionEditSurfacePlot);
		} else {
			if (g->table())
				cm.addAction(tr("Choose &Data Set..."), d_plot_controller_3d, qOverload<>(&PlotController3D::change3DData));
			else if (g->matrix())
				cm.addAction(tr("Choose &Matrix..."), d_plot_controller_3d, qOverload<>(&PlotController3D::change3DMatrix));
			else if (g->userFunction() || g->parametricSurface())
				cm.addAction(actionEditSurfacePlot);
			cm.addAction(QPixmap(":/erase.png"), tr("C&lear"), static_cast<Graph3D *>(g), &Graph3D::clearData);
		}
		cm.addMenu(format);
		cm.addSeparator();
		cm.addAction(actionRename);
		cm.addAction(actionCopyWindow);
		cm.addSeparator();
		cm.addAction(tr("&Copy Graph"), static_cast<Graph3D *>(g), &Graph3D::copyImage);
		cm.addAction(tr("&Export") + "...", this, [this]{ d_export_manager->exportGraph(); });
		cm.addAction(actionPrint);
		cm.addSeparator();
		cm.addAction(actionAnimate);
		cm.addSeparator();
		cm.addAction(actionCloseWindow);
	} else if (qobject_cast<Matrix *>(w)){
		Matrix *t = (Matrix *)w;
		cm.addMenu(plot3DMenu);
		cm.addSeparator();
		cm.addAction(actionSetMatrixProperties);
		cm.addAction(actionSetMatrixDimensions);
		cm.addSeparator();
		cm.addAction(actionSetMatrixValues);
		cm.addAction(actionTableRecalculate);
		cm.addSeparator();

		if (t->viewType() == Matrix::TableView){
			cm.addAction(QPixmap(":/cut.png"), tr("Cu&t"), static_cast<Matrix *>(t), &Matrix::cutSelection);
			cm.addAction(QPixmap(":/copy.png"), tr("&Copy"), static_cast<Matrix *>(t), &Matrix::copySelection);
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste"), static_cast<Matrix *>(t), &Matrix::pasteSelection);
            cm.addSeparator();
			cm.addAction(QPixmap(":/insert_row.png"), tr("&Insert Row"), static_cast<Matrix *>(t), &Matrix::insertRow);
			cm.addAction(QPixmap(":/insert_column.png"), tr("&Insert Column"), static_cast<Matrix *>(t), &Matrix::insertColumn);
            if (t->numSelectedRows() > 0)
				cm.addAction(QPixmap(":/delete_row.png"), tr("&Delete Rows"), static_cast<Matrix *>(t), &Matrix::deleteSelectedRows);
            else if (t->numSelectedColumns() > 0)
				cm.addAction(QPixmap(":/delete_column.png"), tr("&Delete Columns"), static_cast<Matrix *>(t), &Matrix::deleteSelectedColumns);

			cm.addAction(QPixmap(":/erase.png"), tr("Clea&r"), static_cast<Matrix *>(t), &Matrix::clearSelection);
            cm.addSeparator();
            cm.addAction(actionViewMatrixImage);
		} else if (t->viewType() == Matrix::ImageView){
		    cm.addAction(actionImportImage);
            cm.addAction(actionExportMatrix);
            cm.addSeparator();
            cm.addAction(actionRotateMatrix);
            cm.addAction(actionRotateMatrixMinus);
            cm.addSeparator();
            cm.addAction(actionFlipMatrixVertically);
            cm.addAction(actionFlipMatrixHorizontally);
            cm.addSeparator();
		#ifdef HAVE_ALGLIB
			cm.addAction(actionExpandMatrix);
			cm.addAction(actionShrinkMatrix);
			cm.addAction(actionSmoothMatrix);
			cm.addSeparator();
		#endif
            cm.addAction(actionTransposeMatrix);
            cm.addAction(actionInvertMatrix);
            cm.addSeparator();

			actionViewMatrix->setChecked(false);
			cm.addAction(actionViewMatrix);

			QMenu *paletteMenu = cm.addMenu (tr("&Palette"));
			paletteMenu->addAction(actionMatrixDefaultScale);
			paletteMenu->addAction(actionMatrixGrayScale);
			paletteMenu->addAction(actionMatrixRainbowScale);
			paletteMenu->addAction(actionMatrixCustomScale);

			actionMatrixDefaultScale->setChecked(t->colorMapType() == Matrix::Default);
			actionMatrixGrayScale->setChecked(t->colorMapType() == Matrix::GrayScale);
			actionMatrixRainbowScale->setChecked(t->colorMapType() == Matrix::Rainbow);
			actionMatrixCustomScale->setChecked(t->colorMapType() == Matrix::Custom);
		}
	}
	cm.exec(QCursor::pos());
}

void ApplicationWindow::customWindowTitleBarMenu(MdiSubWindow *w, QMenu *menu)
{
	menu->addSeparator();

	menu->addAction(actionSaveWindow);
	menu->addAction(actionPrint);
	menu->addSeparator();
	menu->addAction(actionCopyWindow);
	menu->addAction(actionRename);
	menu->addSeparator();

	if (w->inherits("Table") || w->inherits("Matrix")){
		menu->addAction(actionLoad);
		QMenu *exportMenu = menu->addMenu(tr("Export"));
		exportMenu->addAction(actionShowExportASCIIDialog);
		exportMenu->addAction(actionExportExcel);
		exportMenu->addAction(actionExportOds);
		exportMenu->addAction(actionExportPDF);
		if (w->inherits("Matrix"))
			exportMenu->addAction(actionExportMatrix);
		menu->addSeparator();
	}

	if (w->inherits("Note"))
		menu->addAction(actionSaveNote);
	else
		menu->addAction(actionSaveTemplate);
	menu->addSeparator();
	menu->addAction(actionHideActiveWindow);
}

void ApplicationWindow::showTableContextMenu(bool selection)
{
	Table *t = (Table*)activeWindow(TableWindow);
	if (!t)
		return;

	QMenu cm(this);
	QMenu moveRow(this);
	if (selection){
		if ((int)t->selectedColumns().count() > 0){
			showColMenu(t->firstSelectedColumn());
			return;
		} else if (t->numSelectedRows() == 1) {
			cm.addAction(actionShowColumnValuesDialog);
			cm.addAction(actionTableRecalculate);
			cm.addSeparator();
			cm.addAction(QPixmap(":/cut.png"), tr("Cu&t"), static_cast<Table *>(t), &Table::cutSelection);
			cm.addAction(QPixmap(":/copy.png"), tr("&Copy"), static_cast<Table *>(t), &Table::copySelection);
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste"), static_cast<Table *>(t), &Table::pasteSelection);
			cm.addSeparator();
			moveRow.addAction(actionMoveRowUp);
			moveRow.addAction(actionMoveRowDown);
			moveRow.setTitle(tr("Move Row"));
			cm.addMenu (&moveRow);
			cm.addAction(QPixmap(":/insert_row.png"), tr("&Insert Row"), static_cast<Table *>(t), static_cast<void (Table::*)()>(&Table::insertRow));
			cm.addAction(QPixmap(":/delete_row.png"), tr("&Delete Row"), static_cast<Table *>(t), &Table::deleteSelectedRows);
			cm.addAction(QPixmap(":/erase.png"), tr("Clea&r Row"), static_cast<Table *>(t), &Table::clearSelection);
		} else if (t->numSelectedRows() > 1) {
			cm.addAction(actionShowColumnValuesDialog);
			cm.addAction(QPixmap(":/cut.png"), tr("Cu&t"), static_cast<Table *>(t), &Table::cutSelection);
			cm.addAction(QPixmap(":/copy.png"), tr("&Copy"), static_cast<Table *>(t), &Table::copySelection);
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste"), static_cast<Table *>(t), &Table::pasteSelection);
			cm.addSeparator();
			cm.addAction(actionTableRecalculate);
			cm.addAction(QPixmap(":/delete_row.png"), tr("&Delete Rows"), static_cast<Table *>(t), &Table::deleteSelectedRows);
			cm.addAction(QPixmap(":/erase.png"), tr("Clea&r Rows"), static_cast<Table *>(t), &Table::clearSelection);
		} else if (t->numRows() > 0 && t->numCols() > 0){
			cm.addAction(actionShowColumnValuesDialog);
			cm.addAction(QPixmap(":/cut.png"), tr("Cu&t"), static_cast<Table *>(t), &Table::cutSelection);
			cm.addAction(QPixmap(":/copy.png"), tr("&Copy"), static_cast<Table *>(t), &Table::copySelection);
			cm.addAction(QPixmap(":/paste.png"), tr("&Paste"), static_cast<Table *>(t), &Table::pasteSelection);
			cm.addSeparator();
			cm.addAction(actionTableRecalculate);
			cm.addAction(QPixmap(":/erase.png"), tr("Clea&r"), static_cast<Table *>(t), &Table::clearSelection);
		}
		cm.addSeparator();
		cm.addAction(actionShowColStatistics);
		cm.addAction(actionShowRowStatistics);
	} else {
		cm.addAction(actionShowExportASCIIDialog);
		cm.addSeparator();
		cm.addAction(actionAddColToTable);
		cm.addAction(actionClearTable);
		cm.addSeparator();
		cm.addAction(actionGoToRow);
		cm.addAction(actionGoToColumn);
	}
	cm.exec(QCursor::pos());
}

void ApplicationWindow::chooseHelpFolder()
{
	QFileInfo hfi(helpFilePath);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the QtiPlot help folder!"),
		hfi.dir().absolutePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		helpFilePath = QDir(dir).absoluteFilePath ("index.html");
		if (!QFile(helpFilePath).exists()){
			QMessageBox::critical(this, tr("QtiPlot - index.html File Not Found!"),
					tr("There is no file called <b>index.html</b> in this folder.<br>Please choose another folder!"));
		}
	}
}

void ApplicationWindow::showStandAloneHelp()
{
#ifdef Q_OS_MAC // Mac
	QSettings settings(QSettings::IniFormat,QSettings::UserScope, "ProIndependent", "QtiPlot");
#else
	QSettings settings(QSettings::NativeFormat,QSettings::UserScope, "ProIndependent", "QtiPlot");
#endif

	settings.beginGroup("/General");
	settings.beginGroup("/Paths");
	QString helpPath = settings.value("/HelpFile", qApp->applicationDirPath()+"/manual/index.html").toString();
	settings.endGroup();
	settings.endGroup();

	QFile helpFile(helpPath);
	if (!helpPath.isEmpty() && !helpFile.exists())
	{
		QMessageBox::critical(0, tr("QtiPlot - Help Files Not Found!"),
				tr("The manual can be downloaded from the following internet address:")+
				"<p><a href = http://soft.proindependent.com/manuals.html>http://soft.proindependent.com/manuals.html</a></p>");
		exit(0);
	}

	QFileInfo fi(helpPath);
	QString profilePath = QString(fi.absolutePath()+"/qtiplot.adp");
	if (!QFile(profilePath).exists())
	{
		QMessageBox::critical(0, tr("QtiPlot - Help Profile Not Found!"),
				tr("The assistant could not start because the file <b>%1</b> was not found in the help file directory!").arg("qtiplot.adp")+"<br>"+
				tr("This file is provided with the QtiPlot manual which can be downloaded from the following internet address:")+
				"<p><a href = http://soft.proindependent.com/manuals.html>http://soft.proindependent.com/manuals.html</a></p>");
		exit(0);
	}

	// QAssistantClient is deprecated in Qt5.
	// Use QDesktopServices to open the help file directly.
	QDesktopServices::openUrl(QUrl::fromLocalFile(helpFile.fileName()));
	// connect(assist, SIGNAL(assistantClosed()), qApp, SLOT(quit()) );
}

void ApplicationWindow::showHelp()
{
	QFile helpFile(helpFilePath);
	if (!helpFile.exists()){
		QMessageBox::critical(this, tr("QtiPlot - Help Files Not Found!"),
				tr("Please indicate the location of the help file!")+"<br>"+
				tr("The manual can be downloaded from the following internet address:")+
				"<p><a href = http://soft.proindependent.com/manuals.html>http://soft.proindependent.com/manuals.html</a></p>");
		QString fn = getFileName(this, tr("QtiPlot - Help Files Not Found!"), QDir::currentPath(), "*.html", 0, false);
		if (!fn.isEmpty()){
			QFileInfo fi(fn);
			helpFilePath = fi.absoluteFilePath();
			saveSettings();
		}
	}

	QFileInfo fi(helpFilePath);
	QString profilePath = QString(fi.absolutePath()+"/qtiplot.adp");
	if (!QFile(profilePath).exists())
	{
		QMessageBox::critical(this,tr("QtiPlot - Help Profile Not Found!"),
				tr("The assistant could not start because the file <b>%1</b> was not found in the help file directory!").arg("qtiplot.adp")+"<br>"+
				tr("This file is provided with the QtiPlot manual which can be downloaded from the following internet address:")+
				"<p><a href = http://soft.proindependent.com/manuals.html>http://soft.proindependent.com/manuals.html</a></p>");
		return;
	}

	// QStringList cmdLst = QStringList() << "-profile" << profilePath;
	// assistant->setArguments( cmdLst );
	QDesktopServices::openUrl(QUrl::fromLocalFile(helpFilePath));
}

void ApplicationWindow::showPlotWizard()
{
    QStringList lst = tableNames();
	if (lst.count() > 0){
		PlotWizard* pw = new PlotWizard(this);
		pw->insertTablesList(lst);
		pw->setColumnsList(columnsList(Table::All));
		pw->changeColumnsList(lst[0]);
		pw->exec();
	} else
		QMessageBox::warning(this,tr("QtiPlot - Warning"),
				tr("<h4>There are no tables available in this project.</h4>"
					"<p><h4>Please create a table and try again!</h4>"));
}

void ApplicationWindow::setCurveFullRange()
{
    MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	g->setCurveFullRange(actionCurveFullRange->data().toInt());
}

void ApplicationWindow::showCurveRangeDialog()
{
    MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	showCurveRangeDialog(g, actionEditCurveRange->data().toInt());
}

CurveRangeDialog* ApplicationWindow::showCurveRangeDialog(Graph *g, int curve)
{
	if (!g)
		return 0;

	CurveRangeDialog* crd = new CurveRangeDialog(this);
	crd->setCurveToModify(g, curve);
	crd->exec();
	return crd;
}

FunctionDialog* ApplicationWindow::showFunctionDialog(Graph *g, int curve)
{
	if ( !g )
		return 0;

	FunctionDialog* fd = functionDialog();
	fd->setWindowTitle(tr("QtiPlot - Edit function"));
	fd->setCurveToModify(g, curve);
	return fd;
}

FunctionDialog* ApplicationWindow::functionDialog()
{
	FunctionDialog* fd = new FunctionDialog(this);
	fd->setModal(true);
	fd->show();
	fd->activateWindow();
	return fd;
}

void ApplicationWindow::addFunctionCurve()
{
	Graph *g = activePlotLayer(false);
	if (g){
		FunctionDialog* fd = functionDialog();
		if (fd)
			fd->setGraph(g);
	}
}

void ApplicationWindow::updateFunctionLists(int type, QStringList &formulas)
{
	int maxListSize = 10;
	if (type == 2 && formulas.size() >= 2){
		rFunctions.removeAll(formulas[0]);
		rFunctions.push_front(formulas[0]);

		thetaFunctions.removeAll(formulas[1]);
		thetaFunctions.push_front(formulas[1]);

		while ((int)rFunctions.size() > maxListSize)
			rFunctions.pop_back();
		while ((int)thetaFunctions.size() > maxListSize)
			thetaFunctions.pop_back();
	} else if (type == 1 && formulas.size() >= 2){
		xFunctions.removeAll(formulas[0]);
		xFunctions.push_front(formulas[0]);

		yFunctions.removeAll(formulas[1]);
		yFunctions.push_front(formulas[1]);

		while ((int)xFunctions.size() > maxListSize)
			xFunctions.pop_back();
		while ((int)yFunctions.size() > maxListSize)
			yFunctions.pop_back();
	} else if (type == 0 && formulas.size() >= 1){
		d_recent_functions.removeAll(formulas[0]);
		d_recent_functions.push_front(formulas[0]);

		while ((int)d_recent_functions.size() > maxListSize)
			d_recent_functions.pop_back();
	}
}

MultiLayer* ApplicationWindow::newFunctionPlot(QStringList &formulas, double start, double end, int points, const QString& var, int type)
{
	return d_plot_controller_2d ? d_plot_controller_2d->newFunctionPlot(formulas, start, end, points, var, type) : nullptr;
}

void ApplicationWindow::clearLogInfo()
{
	if (!current_folder->logInfo().isEmpty()){
		current_folder->clearLogInfo();
		results->setText("");
		emit modified();
	}
}



























void ApplicationWindow::initPlot3DToolBar()
{
	if (d_action_manager)
		d_action_manager->initPlot3DToolBar();
}

void ApplicationWindow::pixelLineProfile()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (!g)
		return;

	bool ok;
	int res = QInputDialog::getInt(this,
			tr("QtiPlot - Set the number of pixels to average"), tr("Number of averaged pixels"),1, 1, 2000, 2,
			&ok);
	if ( !ok )
		return;

	LineProfileTool *lpt = new LineProfileTool(g, this, res);
	g->setActiveTool(lpt);
}

void ApplicationWindow::intensityTable()
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	Graph* g = plot->activeLayer();
	if (g){
		ImageWidget *im = qobject_cast<ImageWidget *>(g->activeEnrichment());
        if (im){
            QString fn = im->fileName();
            if (!fn.isEmpty())
                importImage(fn);
        }
	}
}








Note* ApplicationWindow::openNote(ApplicationWindow* app, const QStringList &flist)
{
	return ProjectSerializer::openNote(app, flist, app ? app->d_file_version : 0);
}

Matrix* ApplicationWindow::openMatrix(ApplicationWindow* app, const QStringList &flist)
{
	return ProjectSerializer::openMatrix(app, flist, app ? app->d_file_version : 0);
}

Table* ApplicationWindow::openTable(ApplicationWindow* app, const QStringList &flist)
{
	return ProjectSerializer::openTable(app, flist, app ? app->d_file_version : 0);
}

TableStatistics* ApplicationWindow::openTableStatistics(const QStringList &flist)
{
	return ProjectSerializer::openTableStatistics(this, flist, d_file_version);
}

Graph* ApplicationWindow::openGraph(ApplicationWindow* app, MultiLayer *plot, const QStringList &list)
{
	return ProjectSerializer::openGraph(app, plot, list, app ? app->d_file_version : 0);
}












void ApplicationWindow::pickPointerCursor()
{
	btnPointer->setChecked(true);
}

void ApplicationWindow::disableTools()
{
	if (displayBar && displayBar->isVisible())
		displayBar->hide();

	QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
		MultiLayer *ml = qobject_cast<MultiLayer *>(w);
		if (ml){
			ml->deselect();
			QList<Graph *> layers = ml->layersList();
			for (Graph *g : layers)
				g->disableTools();
		}
	}
}

void ApplicationWindow::pickDataTool( QAction* action )
{
	if (!action)
		return;

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;

	plot->deselect();

	QList<Graph *> layers = plot->layersList();
	for (Graph *g : layers)
		g->disableTools();

	if (action == btnCursor)
		showCursor();
	else if (action == btnSelect)
		showRangeSelectors();
	else if (action == btnPicker)
		showScreenReader();
	else if (action == btnMovePoints)
		movePoints();
	else if (action == actionDragCurve)
		movePoints(true);
	else if (action == btnRemovePoints)
		removePoints();
	else if (action == actionDrawPoints)
		drawPoints();
	else if (action == btnZoomIn)
		zoomIn();
	else if (action == btnZoomOut)
		zoomOut();
	else if (action == btnArrow)
		drawArrow();
	else if (action == btnLine)
		drawLine();
	else if (action == actionMagnify)
		magnify();
	else if (action == actionMagnifyHor)
		magnify(2);
	else if (action == actionMagnifyVert)
		magnify(1);
}




void ApplicationWindow::connectTable(Table* w)
{
    connect (w->table(), &QTableWidget::itemSelectionChanged, this, &ApplicationWindow::customColumnActions);
	connect (w, &Table::statusChanged, this, &ApplicationWindow::updateWindowStatus);
	connect (w, &Table::hiddenWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::hideWindow));
	connect (w, &Table::closedWindow, this, &ApplicationWindow::closeWindow);
	connect (w, qOverload<const QString&>(&Table::removedCol), this, &ApplicationWindow::removeCurves);
	connect (w, qOverload<const QString&>(&Table::removedCol), this, &ApplicationWindow::removeColumnNameFromCompleter);
	connect (w, &Table::addedCol, this, [this](const QString& s){ addColumnNameToCompleter(s); });
	connect (w, &Table::modifiedData, this, &ApplicationWindow::updateCurves);
	connect (w, &Table::resizedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect (w, &Table::modifiedWindow, this, qOverload<MdiSubWindow*>(&ApplicationWindow::modifiedProject));
	connect (w, &Table::optionsDialog, this, &ApplicationWindow::showColumnOptionsDialog);
	connect (w, &Table::colValuesDialog, this, &ApplicationWindow::showColumnValuesDialog);
	connect (w, &Table::showContextMenu, this, &ApplicationWindow::showTableContextMenu);
	connect (w, &Table::changedColHeader, this, &ApplicationWindow::updateColNames);

	w->askOnCloseEvent(confirmCloseTable);
}

void ApplicationWindow::setAppColors(const QColor& wc, const QColor& pc, const QColor& tpc, bool force)
{
	if (force || workspaceColor != wc){
		workspaceColor = wc;
		d_workspace->setBackground(wc);
	}

	if (!force && panelsColor == pc && panelsTextColor == tpc)
		return;

	panelsColor = pc;
	panelsTextColor = tpc;

	QPalette palette;
	palette.setColor(QPalette::Base, QColor(panelsColor));
	qApp->setPalette(palette);

	palette.setColor(QPalette::Text, QColor(panelsTextColor));
	palette.setColor(QPalette::WindowText, QColor(panelsTextColor));

	lv->setPalette(palette);
	results->setPalette(palette);
	folders->setPalette(palette);
}


void ApplicationWindow::createActions()
{
	if (d_action_manager)
		d_action_manager->createActions();
}

void ApplicationWindow::translateActionsStrings()
{
	if (d_action_manager)
		d_action_manager->translateActionsStrings();
}

Graph3D * ApplicationWindow::plot3DMatrix(Matrix *m, int style)
{
	return d_plot_controller_3d ? d_plot_controller_3d->plot3DMatrix(m, style) : nullptr;
}





MultiLayer* ApplicationWindow::plotSpectrogram(Matrix *m, Graph::CurveType type)
{
	return d_plot_controller_2d ? d_plot_controller_2d->plotSpectrogram(m, type) : nullptr;
}

MultiLayer* ApplicationWindow::plotImageProfiles(Matrix *m)
{
	return d_plot_controller_2d ? d_plot_controller_2d->plotImageProfiles(m) : nullptr;
}

ApplicationWindow* ApplicationWindow::importOPJ(const QString& filename, bool factorySettings, bool newProject)
{	
	ImportExportPlugin *op = importPlugin(filename);
	if (!op)
		return 0;

	if (filename.endsWith(".opj", Qt::CaseInsensitive) || filename.endsWith(".ogg", Qt::CaseInsensitive)){
		ApplicationWindow *app = this;
		if (newProject)
			app = new ApplicationWindow(factorySettings);

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		app->setWindowTitle("QtiPlot - " + filename);
		app->restoreApplicationGeometry();
		app->projectname = filename;
		app->updateRecentProjectsList(filename);

		op->setApplicationWindow(app);
		op->import(filename);

		QApplication::restoreOverrideCursor();
		return app;
	} else if (filename.endsWith(".ogm", Qt::CaseInsensitive) || filename.endsWith(".ogw", Qt::CaseInsensitive)){
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		op->import(filename);
		updateRecentProjectsList(filename);
		QApplication::restoreOverrideCursor();
		return this;
	}
	return 0;
}


QList<MdiSubWindow *> ApplicationWindow::windowsList()
{
	QList<MdiSubWindow *> lst;

    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows)
			lst << w;
		f = f->folderBelow();
	}
	return lst;
}

void ApplicationWindow::updateRecentProjectsList(const QString& fn)
{
	if (d_project_manager)
		d_project_manager->updateRecentProjectsList(fn);
}

void ApplicationWindow::translateCurveHor()
{
	translateCurve(TranslateCurveTool::Horizontal);
}

void ApplicationWindow::translateCurve(TranslateCurveTool::Direction direction)
{
	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (!plot)
		return;
	if (plot->isEmpty()){
		QMessageBox::warning(this,tr("QtiPlot - Warning"),
				tr("<h4>There are no plot layers available in this window.</h4>"
					"<p><h4>Please add a layer and try again!</h4>"));
		btnPointer->setChecked(true);
		return;
	}

	Graph* g = (Graph*)plot->activeLayer();
	if (!g)
		return;

	if (g->isPiePlot()){
		QMessageBox::warning(this,tr("QtiPlot - Warning"),
				tr("This functionality is not available for pie plots!"));

		btnPointer->setChecked(true);
		return;
	} else if (g->validCurvesDataSize()) {
		btnPointer->setChecked(true);
		TranslateCurveTool *tool = new TranslateCurveTool(g, this, direction);
		connect(tool, &TranslateCurveTool::statusText, info, &QLineEdit::setText);
		g->setActiveTool(tool);
		displayBar->show();
	}
}

void ApplicationWindow::setReadOnlyCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;
    QStringList list = t->selectedColumns();
	for (int i=0; i<(int) list.count(); i++)
		t->setReadOnlyColumn(t->colIndex(list[i]), actionReadOnlyCol->isChecked());
}

void ApplicationWindow::setReadOnlyColumns()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;
    QStringList list = t->selectedColumns();
	for (int i=0; i<(int) list.count(); i++)
		t->setReadOnlyColumn(t->colIndex(list[i]));
}

void ApplicationWindow::setReadWriteColumns()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;
    QStringList list = t->selectedColumns();
	for (int i=0; i<(int) list.count(); i++)
		t->setReadOnlyColumn(t->colIndex(list[i]), false);
}

void ApplicationWindow::setAscValues()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

    t->setAscValues();
}

void ApplicationWindow::setRandomValues()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

    t->setRandomValues();
}

void ApplicationWindow::setNormalRandomValues()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setNormalRandomValues();
}

void ApplicationWindow::setXErrCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::xErr);
}

void ApplicationWindow::setYErrCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::yErr);
}

void ApplicationWindow::setXCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::X);
}

void ApplicationWindow::setYCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::Y);
}

void ApplicationWindow::setZCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::Z);
}

void ApplicationWindow::setLabelCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::Label);
}

void ApplicationWindow::disregardCol()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	t->setPlotDesignation(Table::None);
}







void ApplicationWindow::showSupportPage()
{
	QDesktopServices::openUrl(QUrl("http://soft.proindependent.com/contracts.html"));
}

void ApplicationWindow::showDonationsPage()
{
	QDesktopServices::openUrl(QUrl("http://soft.proindependent.com/why_donate.html"));
}

void ApplicationWindow::downloadManual()
{
	QDesktopServices::openUrl(QUrl("http://soft.proindependent.com/manuals.html"));
}

void ApplicationWindow::downloadTranslation()
{
	QDesktopServices::openUrl(QUrl("http://soft.proindependent.com/translations.html"));
}

void ApplicationWindow::showHomePage()
{
	QDesktopServices::openUrl(QUrl("http://www.qtiplot.ro"));
}

void ApplicationWindow::showForums()
{
	QDesktopServices::openUrl(QUrl("https://developer.berlios.de/forum/?group_id=6626"));
}

void ApplicationWindow::showBugTracker()
{
	QDesktopServices::openUrl(QUrl("https://developer.berlios.de/bugs/?group_id=6626"));
}

void ApplicationWindow::showDonationDialog()
{
	if (qApp->arguments().contains("-X"))
		return;
	QString s = tr("<font size=+2, color = darkBlue><b>QtiPlot is open-source software and its development required hundreds of hours of work.<br><br>\
				If you like it, you're using it in your work and you would like to see it \
				constantly improved, please support its authors by making a donation.</b></font>");

	QMessageBox *msg = new QMessageBox(this);
	msg->setText(s);
	msg->setWindowTitle(tr("Please support QtiPlot!"));
	msg->addButton(tr("Make a donation"), QMessageBox::AcceptRole);
	msg->exec();

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString qtiplotWeb = "www.qtiplot.ro";
	QHostInfo hostInfo = QHostInfo::fromName(qtiplotWeb);
	if (hostInfo.error() != QHostInfo::NoError){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("QtiPlot - Error"), qtiplotWeb + ": " + hostInfo.errorString());
		QApplication::restoreOverrideCursor();
	}
	else {
		QApplication::restoreOverrideCursor();
		showDonationsPage();
	}
}

void ApplicationWindow::parseCommandLineArguments(const QStringList& args)
{
	int num_args = args.count();
	if(num_args == 0){
		if (d_open_last_project && !recentProjects.isEmpty()){
			ApplicationWindow *app = open(recentProjects[0]);
			if (app && app != this){
				savedProject();
				close();
			} else
				savedProject();
		} else
			initWindow();
		return;
	}

	QString str;
	bool exec = false;
	bool noGui = false;
	bool default_settings = false;
	bool console = false;
	for (auto str : args){
		if( (str == "-a" || str == "--about") ||
				(str == "-m" || str == "--manual") ){
			QMessageBox::critical(this, tr("QtiPlot - Error"),
			tr("<b> %1 </b>: This command line option must be used without other arguments!").arg(str));
		} else if (str == "-c" || str == "--console") {
			d_mdi_windows_area = false;
			showScriptWindow(false);
			setScriptingLanguage("Python");
			hide();
			console = true;
		}
		else if( (str == "-d" || str == "--default-settings"))
			default_settings = true;
		else if (str == "-v" || str == "--version"){
			QString s = versionString() + "\n";
			s += QString(copyright_string) + "\n";
			s += tr("Released") + ": " + release_date + "\n";
			#ifdef Q_OS_WIN
                hide();
				QMessageBox::information(this, tr("QtiPlot") + " - " + tr("Version"), s);
			#else
				std::wcout << s.toStdWString();
			#endif
			exit(0);
		}
		else if (str == "-h" || str == "--help"){
			QString s = "\n" + tr("Usage") + ": ";
			s += "qtiplot [" + tr("options") + "] [" + tr("file") + "_" + tr("name") + "]\n\n";
			s += tr("Valid options are") + ":\n";
			s += "-a " + tr("or") + " --about: " + tr("show about dialog and exit") + "\n";
			s += "-c " + tr("or") + " --console: " + tr("show standalone scripting window") + "\n";
			s += "-d " + tr("or") + " --default-settings: " + tr("start QtiPlot with the default settings") + "\n";
			s += "-h " + tr("or") + " --help: " + tr("show command line options") + "\n";
			s += "-l=XX " + tr("or") + " --lang=XX: " + tr("start QtiPlot in language") + " XX ('en', 'fr', 'de', ...)\n";
			s += "-m " + tr("or") + " --manual: " + tr("show QtiPlot manual in a standalone window") + "\n";
			s += "-v " + tr("or") + " --version: " + tr("print QtiPlot version and release date") + "\n";
			s += "-x " + tr("or") + " --execute: " + tr("execute the script file given as argument") + "\n";
			s += "-X: " + tr("execute the script file given as argument without displying the user interface. Warning: 2D plots are not correctly handled in this functioning mode!") + "\n\n";
			s += "'" + tr("file") + "_" + tr("name") + "' " + tr("can be any .qti, qti.gz, .ods, .opj, .ogm, .ogw, .ogg, .py, .xls or ASCII file") + "\n";
			#ifdef Q_OS_WIN
                hide();
				QMessageBox::information(this, tr("QtiPlot") + " - " + tr("Help"), s);
			#else
				std::wcout << s.toStdWString();
			#endif
			exit(0);
		}
		else if (str.startsWith("--lang=") || str.startsWith("-l=")){
			QString locale = str.mid(str.indexOf('=')+1);
			if (locales.contains(locale))
				switchToLanguage(locale);

			if (!locales.contains(locale))
				QMessageBox::critical(this, tr("QtiPlot - Error"),
						tr("<b> %1 </b>: Wrong locale option or no translation available!").arg(locale));
		}
		else if (str.startsWith("--execute") || str.startsWith("-x"))
			exec = true;
		else if (str.startsWith("-X")) {
			noGui = true;
		}
		else if (str.startsWith("-") || str.startsWith("--")){
			QMessageBox::critical(this, tr("QtiPlot - Error"),
			tr("<b> %1 </b> unknown command line option!").arg(str) + "\n" + tr("Type %1 to see the list of the valid options.").arg("'qtiplot -h'"));
		}
	}

	QString file_name = args[num_args-1]; // last argument
	if(file_name.startsWith("-")){// no file name given
		if (console)
			return;

		if (d_open_last_project && !recentProjects.isEmpty()){
			ApplicationWindow *app = open(recentProjects[0], default_settings);
			if (app && app != this){
				savedProject();
				close();
			}
		}
		else
			initWindow();
		return;
	}

	savedProject();

	if (!file_name.isEmpty()){
		if (!this->isFileReadable(file_name))
			return;

		workingDir = QFileInfo(file_name).absolutePath();
		saveSettings();//the recent projects must be saved

		if (console){
			scriptWindow->open(file_name);
			if (exec)
				scriptWindow->executeAll();
		} else if (exec || noGui) {
			loadScript(file_name, exec, noGui);
		}
		else {
			ApplicationWindow *app = open(file_name, default_settings);
			if (app && app != this)
				close();
		}
	}
}

void ApplicationWindow::createLanguagesList()
{
	locales.clear();

	appTranslator = new QTranslator(this);
	qtTranslator = new QTranslator(this);
	qApp->installTranslator(appTranslator);
	qApp->installTranslator(qtTranslator);

	QString qmPath = d_translations_folder;
	QDir dir(qmPath);
	QStringList fileNames = dir.entryList(QStringList() << "qtiplot_*.qm");
	for (int i=0; i < (int)fileNames.size(); i++)
	{
		QString locale = fileNames[i];
		locale = locale.mid(locale.indexOf('_')+1);
		locale.truncate(locale.indexOf('.'));
		locales.push_back(locale);
	}
	locales.push_back("en");
	locales.sort();

	if (appLanguage != "en")
	{
		(void)appTranslator->load("qtiplot_" + appLanguage, qmPath);
		(void)qtTranslator->load("qt_" + appLanguage, qmPath+"/qt");
	}
}

void ApplicationWindow::switchToLanguage(int param)
{
	if (param < (int)locales.size())
		switchToLanguage(locales[param]);
}

void ApplicationWindow::switchToLanguage(const QString& locale)
{
	if (!locales.contains(locale) || appLanguage == locale)
		return;

	appLanguage = locale;
	if (locale == "en")
	{
		qApp->removeTranslator(appTranslator);
		qApp->removeTranslator(qtTranslator);
		delete appTranslator;
		delete qtTranslator;
		appTranslator = new QTranslator(this);
		qtTranslator = new QTranslator(this);
		qApp->installTranslator(appTranslator);
		qApp->installTranslator(qtTranslator);
	}
	else
	{
		QString qmPath = d_translations_folder;
		(void)appTranslator->load("qtiplot_" + locale, qmPath);
		(void)qtTranslator->load("qt_" + locale, qmPath+"/qt");
	}
	insertTranslatedStrings();
}

QStringList ApplicationWindow::matrixNames()
{
	QStringList names;
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if (w->inherits("Matrix"))
				names << w->objectName();
		}
		f = f->folderBelow();
	}
	return names;
}

bool ApplicationWindow::alreadyUsedName(const QString& label)
{
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if (w->objectName() == label)
				return true;
		}
		f = f->folderBelow();
	}
	return false;
}

bool ApplicationWindow::projectHas2DPlots()
{
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if (w->inherits("MultiLayer"))
				return true;
		}
		f = f->folderBelow();
	}
	return false;
}

void ApplicationWindow::appendProject()
{
	OpenProjectDialog *open_dialog = new OpenProjectDialog(this, false);
	open_dialog->setDirectory(workingDir);
	open_dialog->setExtensionWidget(0);
	if (open_dialog->exec() != QDialog::Accepted || open_dialog->selectedFiles().isEmpty())
		return;
	workingDir = open_dialog->directory().path();
	appendProject(open_dialog->selectedFiles()[0]);
}

Folder* ApplicationWindow::appendProject(const QString& fn, Folder* parentFolder)
{
	if (d_project_manager)
		return d_project_manager->appendProject(fn, parentFolder);
	return nullptr;
}

bool ApplicationWindow::saveFolder(Folder *folder, const QString& fn, bool compress)
{
	if (d_project_manager)
		return d_project_manager->saveFolder(folder, fn, compress);
	return false;
}

void ApplicationWindow::saveAsProject()
{
	saveFolderAsProject(current_folder);
}

bool ApplicationWindow::saveFolderAsProject(Folder *f)
{
	if (d_project_manager)
		return d_project_manager->saveFolderAsProject(f);
	return false;
}

void ApplicationWindow::showFolderPopupMenu(QTreeWidgetItem *it, const QPoint &p, int)
{
	showFolderPopupMenu(it, p, true);
}

void ApplicationWindow::showFolderPopupMenu(QTreeWidgetItem *it, const QPoint &p, bool fromFolders)
{
	if (!it || folders->isRenaming())
		return;

	QMenu cm(this);
	QMenu window(this);
	QMenu viewWindowsMenu(this);


	cm.addAction(actionFindWindow);
	cm.addSeparator();
	cm.addAction(actionAppendProject);
	if (((FolderListItem *)it)->folder()->parent())
		cm.addAction(QIcon(":/filesaveas.png"), tr("Save &As Project..."), this, &ApplicationWindow::saveAsProject);
	else
		cm.addAction(actionSaveProjectAs);
	cm.addSeparator();

	if (fromFolders && show_windows_policy != HideAll)
	{
		cm.addAction(tr("&Show All Windows"), this, &ApplicationWindow::showAllFolderWindows);
		cm.addAction(tr("&Hide All Windows"), this, &ApplicationWindow::hideAllFolderWindows);
		cm.addSeparator();
	}

	{
		QAction *a = cm.addAction(QPixmap(":/close.png"), tr("&Delete Folder"), this, qOverload<>(&ApplicationWindow::deleteFolder));
		a->setShortcut(Qt::Key_F8);
		a = cm.addAction(tr("&Rename"), this, qOverload<>(&ApplicationWindow::startRenameFolder));
		a->setShortcut(Qt::Key_F2);
		cm.addSeparator();
	}

	if (fromFolders)
	{
		window.addAction(actionNewTable);
		window.addAction(actionNewMatrix);
		window.addAction(actionNewNote);
		window.addAction(actionNewGraph);
		window.addAction(actionNewFunctionPlot);
		window.addAction(actionNewSurfacePlot);
		cm.addMenu(&window)->setText(tr("New &Window"));
	}

	cm.addAction(actionNewFolder);
	cm.addSeparator();

	QStringList lst;
	lst << tr("&None") << tr("&Windows in Active Folder") << tr("Windows in &Active Folder && Subfolders");
	for (int i = 0; i < 3; ++i)
	{
		QAction *a = viewWindowsMenu.addAction(lst[i], this, [this, i]() { setShowWindowsPolicy(i); });
		a->setData( i );
		
		a->setChecked( show_windows_policy == i );
	}
	cm.addMenu(&viewWindowsMenu)->setText(tr("&View Windows"));
	cm.addSeparator();
	cm.addAction(tr("&Properties..."), this, &ApplicationWindow::folderProperties);
	cm.exec(p);
}

void ApplicationWindow::setShowWindowsPolicy(int p)
{
	if (show_windows_policy == (ShowWindowsPolicy)p)
		return;

	show_windows_policy = (ShowWindowsPolicy)p;
	if (show_windows_policy == HideAll){
		QList<MdiSubWindow *> windows = windowsList();
		for (MdiSubWindow *w : windows){
			hiddenWindows->append(w);
			w->hide();
			setListView(w->objectName(), tr("Hidden"));
		}
	} else
		showAllFolderWindows();
}

void ApplicationWindow::showFindDialogue()
{
	FindDialog *fd = new FindDialog(this);
	fd->exec();
}

void ApplicationWindow::startRenameFolder()
{
	FolderListItem *fi = current_folder->folderListItem();
	if (!fi)
		return;

	disconnect(folders, &QTreeWidget::currentItemChanged, this, &ApplicationWindow::folderItemChanged);
	fi->setFlags(fi->flags() | Qt::ItemIsEditable);
	folders->editItem(fi, 0);
}

void ApplicationWindow::startRenameFolder(QTreeWidgetItem *item)
{
	if (!item || item == folders->topLevelItem(0))
		return;

	if (item->treeWidget() == lv && item->type() == FolderListItem::RTTI) {
        disconnect(folders, &QTreeWidget::currentItemChanged, this, &ApplicationWindow::folderItemChanged);
		current_folder = ((FolderListItem *)item)->folder();
		FolderListItem *it = current_folder->folderListItem();
		it->setFlags(it->flags() | Qt::ItemIsEditable);
		folders->editItem(it, 0);
	} else {
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		lv->editItem(item, 0);
	}
}

void ApplicationWindow::renameFolder(QTreeWidgetItem *it, int col, const QString &text)
{
	Q_UNUSED(col)

	if (!it)
		return;

	Folder *f = ((FolderListItem *)it)->folder();
	if (!f)
		return;

	Folder *parent = (Folder *)f->parent();
	if (!parent)//the parent folder is the project folder (it always exists)
		parent = projectFolder();

	while(text.isEmpty()){
		QMessageBox::critical(this,tr("QtiPlot - Error"), tr("Please enter a valid name!"));
		it->setFlags(it->flags() & ~Qt::ItemIsEditable); // disable edit? or just let it close
		folders->blockSignals(true);
		it->setText(0, f->objectName());
		folders->blockSignals(false);
		return;
	}

	QStringList lst = parent->subfolders();
	lst.removeAll(f->objectName());
	while(lst.contains(text)){
		QMessageBox::critical(this,tr("QtiPlot - Error"),
				tr("Name already exists!")+"\n"+tr("Please choose another name!"));

		folders->blockSignals(true);
		it->setText(0, f->objectName());
		folders->blockSignals(false);
		return;
	}

	f->setObjectName(text);
	connect(folders, &QTreeWidget::currentItemChanged,
			this, &ApplicationWindow::folderItemChanged);
	folders->setCurrentItem(parent->folderListItem());//update the list views
}

void ApplicationWindow::showAllFolderWindows()
{
	QList<MdiSubWindow *> lst = current_folder->windowsList();
	for (MdiSubWindow *w : lst){//force show all windows in current folder
		if (w){
			updateWindowLists(w);
			w->restoreWindow();
		}
	}

	if ((current_folder->children()).isEmpty())
		return;

	FolderListItem *fi = current_folder->folderListItem();
	FolderListItem *item = (FolderListItem *)fi->treeWidget()->topLevelItem(0);
	int initial_depth = item->depth();
	while (item && item->depth() >= initial_depth){//show/hide windows in all subfolders
		lst = ((Folder *)item->folder())->windowsList();
		for (MdiSubWindow *w : lst){
			if (w && show_windows_policy == SubFolders){
				updateWindowLists(w);
				w->restoreWindow();
			} else
				w->hide();
		}

		item = (FolderListItem *)item->itemBelow();
	}
}

void ApplicationWindow::hideAllFolderWindows()
{
	QList<MdiSubWindow *> lst = current_folder->windowsList();
	for (MdiSubWindow *w : lst)
		hideWindow(w);

	if ((current_folder->children()).isEmpty())
		return;

	if (show_windows_policy == SubFolders){
		FolderListItem *fi = current_folder->folderListItem();
		FolderListItem *item = (FolderListItem *)fi->treeWidget()->topLevelItem(0);
		int initial_depth = item->depth();
		while (item && item->depth() >= initial_depth){
			lst = item->folder()->windowsList();
			for (MdiSubWindow *w : lst)
				hideWindow(w);

			item = (FolderListItem *)item->itemBelow();
		}
	}
}

void ApplicationWindow::projectProperties()
{
	QString s = QString(current_folder->objectName()) + "\n\n";
	s += "\n\n\n";
	s += tr("Type") + ": " + tr("Project")+"\n\n";
	if (projectname != "untitled")
	{
		s += tr("Path") + ": " + projectname + "\n\n";

		QFileInfo fi(projectname);
		s += tr("Size") + ": " + QString::number(fi.size()) + " " + tr("bytes")+ "\n\n";
	}

	s += tr("Contents") + ": " + QString::number(windowsList().size()) + " " + tr("windows");
	s += ", " + QString::number(current_folder->subfolders().count()) + " " + tr("folders") + "\n\n";
	s += "\n\n\n";

	if (projectname != "untitled")
	{
		QFileInfo fi(projectname);
		s += tr("Created") + ": " + fi.birthTime().toString(Qt::TextDate) + "\n\n";
		s += tr("Modified") + ": " + fi.lastModified().toString(Qt::TextDate) + "\n\n";
	}
	else
		s += tr("Created") + ": " + current_folder->birthDate() + "\n\n";

	QMessageBox mbox(QMessageBox::NoIcon, tr("Properties"), s, QMessageBox::Ok, this);

	mbox.setIconPixmap(QPixmap(":/qtiplot_logo.png" ));
	mbox.exec();
}

void ApplicationWindow::folderProperties()
{
	if (!current_folder->parent())
	{
		projectProperties();
		return;
	}

	QString s = QString(current_folder->objectName()) + "\n\n";
	s += "\n\n\n";
	s += tr("Type") + ": " + tr("Folder")+"\n\n";
	s += tr("Path") + ": " + current_folder->path() + "\n\n";
	s += tr("Size") + ": " + current_folder->sizeToString() + "\n\n";
	s += tr("Contents") + ": " + QString::number(current_folder->windowsList().count()) + " " + tr("windows");
	s += ", " + QString::number(current_folder->subfolders().count()) + " " + tr("folders") + "\n\n";
	//s += "\n\n\n";
	s += tr("Created") + ": " + current_folder->birthDate() + "\n\n";
	//s += tr("Modified") + ": " + current_folder->modificationDate() + "\n\n";

	QMessageBox mbox(QMessageBox::NoIcon, tr("Properties"), s, QMessageBox::Ok, this);

	mbox.setIconPixmap(QPixmap(":/folder_open.png" ));
	mbox.exec();
}

void ApplicationWindow::addFolder()
{
    if (!explorerWindow->isVisible())
		explorerWindow->show();

	QStringList lst = current_folder->subfolders();
	QString name =  tr("New Folder");
	lst = lst.filter( name );
	if (!lst.isEmpty())
		name += " ("+ QString::number(lst.size()+1)+")";

	Folder *f = new Folder(current_folder, name);
	addFolderListViewItem(f);

	FolderListItem *fi = new FolderListItem(current_folder->folderListItem(), f);
	if (fi){
		f->setFolderListItem(fi);
		fi->setFlags(fi->flags() | Qt::ItemIsEditable);
		folders->editItem(fi, 0);
	}
}

Folder* ApplicationWindow::addFolder(QString name, Folder* parent)
{
    if(!parent){
		if (current_folder)
			parent = current_folder;
		else
        	parent = projectFolder();
	}

    QStringList lst = parent->subfolders();
    lst = lst.filter( name );
    if (!lst.isEmpty())
        name += " ("+ QString::number(lst.size()+1)+")";

    Folder *f = new Folder(parent, name);
    addFolderListViewItem(f);

    FolderListItem *fi = new FolderListItem(parent->folderListItem(), f);
    if (fi)
        f->setFolderListItem(fi);

    return f;
}

bool ApplicationWindow::deleteFolder(Folder *f)
{
	if (!f)
		return false;

	if (confirmCloseFolder && QMessageBox::No == QMessageBox::information(this, tr("QtiPlot - Delete folder?"),
				tr("Delete folder '%1' and all the windows it contains?").arg(f->objectName()),
				QMessageBox::Yes | QMessageBox::No))
		return false;
	else {
		Folder *parent = projectFolder();
		if (current_folder && current_folder != parent){
			if (current_folder->parent())
				parent = (Folder *)current_folder->parent();
		}

		folders->blockSignals(true);

		FolderListItem *fi = f->folderListItem();
		for (MdiSubWindow *w : f->windowsList()){
			w->askOnCloseEvent(false);
            closeWindow(w);
		}

		if (!(f->children()).isEmpty()){
			Folder *subFolder = f->folderBelow();
			int initial_depth = f->depth();
			while (subFolder && subFolder->depth() > initial_depth){
			    for (MdiSubWindow *w : subFolder->windowsList()){
					removeWindowFromLists(w);
					subFolder->removeWindow(w);
					delete w;
				}
				delete subFolder->folderListItem();
				delete subFolder;

				subFolder = f->folderBelow();
			}
		}

		delete f;
		delete fi;

		current_folder = parent;
		folders->setCurrentItem(parent->folderListItem());
		changeFolder(parent, true);

		folders->blockSignals(false);
		folders->setFocus();
		return true;
	}
}

void ApplicationWindow::deleteFolder()
{
	Folder *parent = (Folder *)current_folder->parent();
	if (!parent)
		parent = projectFolder();

	folders->blockSignals(true);

	if (deleteFolder(current_folder)){
		current_folder = parent;
		folders->setCurrentItem(parent->folderListItem());
		changeFolder(parent, true);
	}

	folders->blockSignals(false);
	folders->setFocus();
}

void ApplicationWindow::folderItemDoubleClicked(QTreeWidgetItem *it)
{
	if (!it || it->type() != FolderListItem::RTTI)
		return;

	FolderListItem *item = ((FolderListItem *)it)->folder()->folderListItem();
	folders->setCurrentItem(item);
}

void ApplicationWindow::folderItemChanged(QTreeWidgetItem *it)
{
	if (!it)
		return;

	changeFolder (((FolderListItem *)it)->folder());
	folders->setFocus();
}

void ApplicationWindow::hideFolderWindows(Folder *f)
{
	if (!f)
		return;

	QList<MdiSubWindow *> lst = f->windowsList();
	for (MdiSubWindow *w : lst)
		w->hide();

	if ((f->children()).isEmpty())
		return;

	Folder *dir = f->folderBelow();
	int initial_depth = f->depth();
	while (dir && dir->depth() > initial_depth){
		lst = dir->windowsList();
		for (MdiSubWindow *w : lst)
			w->hide();

		dir = dir->folderBelow();
	}
}

void ApplicationWindow::goToParentFolder()
{
	if (current_folder &&  current_folder->parent())
		current_folder = (Folder *)current_folder->parent();
	else
		current_folder = projectFolder();
}

bool ApplicationWindow::changeFolder(Folder *newFolder, bool force)
{
	if (!newFolder)
		return false;

	if (current_folder == newFolder && !force)
		return false;

	disconnect(d_workspace, &QMdiArea::subWindowActivated,
			this, &ApplicationWindow::windowActivated);

	desactivateFolders();
	newFolder->folderListItem()->setActive(true);

	folders->blockSignals(true);
	folders->setCurrentItem(newFolder->folderListItem());
	folders->blockSignals(false);

	Folder *oldFolder = current_folder;
	current_folder = newFolder;

	MdiSubWindow::Status old_active_window_state = MdiSubWindow::Normal;
	MdiSubWindow *old_active_window = oldFolder->activeWindow();
	if (old_active_window)
		old_active_window_state = old_active_window->status();

	MdiSubWindow::Status active_window_state = MdiSubWindow::Normal;
	MdiSubWindow *active_window = newFolder->activeWindow();

	QList<MdiSubWindow *> lst = newFolder->windowsList();
	for (MdiSubWindow *w : lst){
		if (w->status() == MdiSubWindow::Maximized)
			active_window = w;
	}

	if (active_window)
		active_window_state = active_window->status();

	hideFolderWindows(oldFolder);

	results->setText(current_folder->logInfo());

	lv->clear();

	QObjectList folderLst = newFolder->children();
	if(!folderLst.isEmpty()){
		for (QObject *f : folderLst)
			addFolderListViewItem(static_cast<Folder *>(f));
	}

	for (MdiSubWindow *w : lst){
		if (!hiddenWindows->contains(w) && show_windows_policy != HideAll){
			//show only windows in the current folder which are not hidden by the user
			w->restoreWindow();
		} else
			w->setStatus(MdiSubWindow::Hidden);

		addListViewItem(w);
	}

	if (!(newFolder->children()).isEmpty()){
		Folder *f = newFolder->folderBelow();
		int initial_depth = newFolder->depth();
		while (f && f->depth() > initial_depth){//show/hide windows in subfolders
			lst = f->windowsList();
			for (MdiSubWindow *w : lst){
				if (!hiddenWindows->contains(w)){
					if (show_windows_policy == SubFolders){
						if (w->status() == MdiSubWindow::Normal || w->status() == MdiSubWindow::Maximized)
							w->showNormal();
						else if (w->status() == MdiSubWindow::Minimized)
							w->showMinimized();
					} else
						w->hide();
				}
			}
		f = f->folderBelow();
		}
	}

	if (active_window){
		d_active_window = active_window;
		d_workspace->setActiveSubWindow(active_window);

		if (active_window_state == MdiSubWindow::Minimized)
			active_window->showMinimized();//ws->setActiveWindow() makes minimized windows to be shown normally
		else if (active_window_state == MdiSubWindow::Maximized){
			active_window->setMaximized();

			MultiLayer *ml = qobject_cast<MultiLayer *>(active_window);
			if (ml)
				ml->adjustLayersToCanvasSize();
		}
	} else
		d_active_window = (MdiSubWindow *)d_workspace->activeSubWindow();

	customMenu(d_active_window);
	customToolBars(d_active_window);

	if (old_active_window){
		old_active_window->setStatus(old_active_window_state);
		oldFolder->setActiveWindow(old_active_window);
	}

	connect(d_workspace, &QMdiArea::subWindowActivated,
		this, &ApplicationWindow::windowActivated);

	if (!d_opening_file)
		modifiedProject();
	return true;
}

void ApplicationWindow::desactivateFolders()
{
    QTreeWidgetItemIterator it(folders);
    while (*it) {
        if ((*it)->type() == FolderListItem::RTTI)
            ((FolderListItem *)(*it))->setActive(false);
        ++it;
    }
}

void ApplicationWindow::addListViewItem(MdiSubWindow *w)
{
	if (!w)
		return;

	WindowListItem* it = new WindowListItem(lv, w);
	if (w->inherits("Matrix")){
		it->setIcon(0, QIcon(":/matrix.png"));
		it->setText(1, tr("Matrix"));
	}
	else if (w->inherits("Table")){
		it->setIcon(0, QIcon(":/worksheet.png"));
		it->setText(1, tr("Table"));
	}
	else if (w->inherits("Note")){
		it->setIcon(0, QIcon(":/note.png"));
		it->setText(1, tr("Note"));
	}
	else if (w->inherits("MultiLayer")){
		it->setIcon(0, QIcon(":/graph.png"));
		it->setText(1, tr("Graph"));
	}
	else if (w->inherits("Graph3D")){
		it->setIcon(0, QIcon(":/trajectory.png"));
		it->setText(1, tr("3D Graph"));
	}

	it->setText(0, w->objectName());
    it->setText(2, w->aspect());
	//it->setText(3, w->sizeToString());
	it->setText(3, w->birthDate());
	it->setText(4, w->windowLabel().replace("\n", " "));

	updateCompleter(w->objectName());
}

void ApplicationWindow::windowProperties()
{
	WindowListItem *it = (WindowListItem *)lv->currentItem();
	MdiSubWindow *w = it->window();
	if (!w)
		return;

	QMessageBox *mbox = new QMessageBox (QMessageBox::NoIcon, tr("Properties"), QString(), QMessageBox::Ok, this);

	QString s = QString(w->objectName()) + "\n\n";
	s += "\n\n\n";

	s += tr("Label") + ": " + ((MdiSubWindow *)w)->windowLabel() + "\n\n";

	if (w->inherits("Matrix")){
		mbox->setIconPixmap(QPixmap(":/matrix.png"));
		s +=  tr("Type") + ": " + tr("Matrix") + "\n\n";
	}else if (w->inherits("Table")){
		mbox->setIconPixmap(QPixmap(":/worksheet.png"));
		s +=  tr("Type") + ": " + tr("Table") + "\n\n";
	}else if (w->inherits("Note")){
		mbox->setIconPixmap(QPixmap(":/note.png"));
		s +=  tr("Type") + ": " + tr("Note") + "\n\n";
	}else if (w->inherits("MultiLayer")){
		mbox->setIconPixmap(QPixmap(":/graph.png"));
		s +=  tr("Type") + ": " + tr("Graph") + "\n\n";
	}else if (w->inherits("Graph3D")){
		mbox->setIconPixmap(QPixmap(":/trajectory.png"));
		s +=  tr("Type") + ": " + tr("3D Graph") + "\n\n";
	}
	s += tr("Path") + ": " + current_folder->path() + "\n\n";
	//s += tr("Size") + ": " + w->sizeToString() + "\n\n";
	s += tr("Created") + ": " + w->birthDate() + "\n\n";
	s += tr("Status") + ": " + it->text(2) + "\n\n";
	mbox->setText(s);
	mbox->exec();
	delete mbox;
}

void ApplicationWindow::addFolderListViewItem(Folder *f)
{
	if (!f)
		return;

	FolderListItem* it = new FolderListItem(lv, f);
	it->setActive(false);
	it->setText(0, f->objectName());
	it->setText(1, tr("Folder"));
	it->setText(3, f->birthDate());
}

void ApplicationWindow::find(const QString& s, bool windowNames, bool labels,
		bool folderNames, bool caseSensitive, bool partialMatch, bool subfolders)
{
	if (windowNames || labels){
		MdiSubWindow *w = current_folder->findWindow(s, windowNames, labels, caseSensitive, partialMatch);
		if (w){
			activateWindow(w);
			return;
		}

		if (subfolders){
			FolderListItem *item = (FolderListItem *)folders->topLevelItem(0);
			while (item){
				Folder *f = item->folder();
				MdiSubWindow *w = f->findWindow(s,windowNames,labels,caseSensitive,partialMatch);
				if (w){
					folders->setCurrentItem(f->folderListItem());
					activateWindow(w);
					return;
				}
				item = (FolderListItem *)item->itemBelow();
			}
		}
	}

	if (folderNames){
		Folder *f = current_folder->findSubfolder(s, caseSensitive, partialMatch);
		if (f){
			folders->setCurrentItem(f->folderListItem());
			return;
		}

		if (subfolders){
			FolderListItem *item = (FolderListItem *)folders->topLevelItem(0);
			while (item){
				Folder *f = item->folder()->findSubfolder(s, caseSensitive, partialMatch);
				if (f){
					folders->setCurrentItem(f->folderListItem());
					return;
				}

				item = (FolderListItem *)item->itemBelow();
			}
		}
	}

	QMessageBox::warning(this, tr("QtiPlot - No match found"),
			tr("Sorry, no match found for string: '%1'").arg(s));
}

void ApplicationWindow::dropFolderItems(QTreeWidgetItem *dest)
{
	if (!dest || draggedItems.isEmpty ())
		return;

	Folder *dest_f = ((FolderListItem *)dest)->folder();

	QTreeWidgetItem *it;
	QStringList subfolders = dest_f->subfolders();

	for (auto it : draggedItems){
		if (it->type() == FolderListItem::RTTI){
			Folder *f = ((FolderListItem *)it)->folder();
			FolderListItem *src = f->folderListItem();
			if (dest_f == f){
				QMessageBox::critical(this, "QtiPlot - Error", tr("Cannot move an object to itself!"));
				return;
			}

			if (((FolderListItem *)dest)->isChildOf(src)){
				QMessageBox::critical(this,"QtiPlot - Error",tr("Cannot move a parent folder into a child folder!"));
				draggedItems.clear();
				folders->setCurrentItem(current_folder->folderListItem());
				return;
			}

			Folder *parent = (Folder *)f->parent();
			if (!parent)
				parent = projectFolder();
			if (dest_f == parent)
				return;

			if (subfolders.contains(f->objectName())){
				QMessageBox::critical(this, tr("QtiPlot") +" - " + tr("Skipped moving folder"),
						tr("The destination folder already contains a folder called '%1'! Folder skipped!").arg(f->objectName()));
			} else
				moveFolder(src, (FolderListItem *)dest);
		} else {
			if (dest_f == current_folder)
				return;

			hideFolderWindows(current_folder);

			MdiSubWindow *w = ((WindowListItem *)it)->window();
			if (w){
				current_folder->removeWindow(w);
				w->hide();
				dest_f->addWindow(w);
				dest_f->setActiveWindow(w);
				delete it;
			}
		}
	}

	draggedItems.clear();
	current_folder = dest_f;
	folders->setCurrentItem(dest_f->folderListItem());
	changeFolder(dest_f, true);
	folders->setFocus();
}

void ApplicationWindow::moveFolder(FolderListItem *src, FolderListItem *dest)
{
	folders->blockSignals(true);
	if (copyFolder(src->folder(), dest->folder())){
		delete src->folder();
		delete src;
	}
	folders->blockSignals(false);
}

bool ApplicationWindow::copyFolder(Folder *src, Folder *dest)
{
    if (!src || !dest) {
        return false;
    }

	if (dest->subfolders().contains(src->objectName())){
		QMessageBox::critical(this, tr("QtiPlot") + " - " + tr("Error"),
		tr("The destination folder already contains a folder called '%1'! Folder skipped!").arg(src->objectName()));
		return false;
	}

	Folder *dest_f = new Folder(dest, src->objectName());
	dest_f->setBirthDate(src->birthDate());
	dest_f->setModificationDate(src->modificationDate());

	FolderListItem *copy_item = new FolderListItem(dest->folderListItem(), dest_f);
	copy_item->setText(0, src->objectName());
	copy_item->setOpen(src->folderListItem()->isOpen());
	dest_f->setFolderListItem(copy_item);

	QList<MdiSubWindow *> lst = QList<MdiSubWindow *>(src->windowsList());
	for (MdiSubWindow *w : lst)
		dest_f->addWindow(w);

	if (!(src->children()).isEmpty()){
		int initial_depth = src->depth();
		Folder *parentFolder = dest_f;
        src = src->folderBelow();
		while (src && parentFolder && src->depth() > initial_depth){
			dest_f = new Folder(parentFolder, src->objectName());
			dest_f->setBirthDate(src->birthDate());
			dest_f->setModificationDate(src->modificationDate());

			copy_item = new FolderListItem(parentFolder->folderListItem(), dest_f);
			copy_item->setText(0, src->objectName());
			copy_item->setOpen(src->folderListItem()->isOpen());
			dest_f->setFolderListItem(copy_item);

			lst = QList<MdiSubWindow *>(src->windowsList());
			for (MdiSubWindow *w : lst)
				dest_f->addWindow(w);

            int depth = src->depth();
			src = src->folderBelow();
			if (src){
				int next_folder_depth = src->depth();
            	if (next_folder_depth > depth)
                	parentFolder = dest_f;
				else if (next_folder_depth < depth && next_folder_depth > initial_depth)
                	parentFolder = (Folder*)parentFolder->parent();
			}
		}
	}
	return true;
}

void ApplicationWindow::searchForUpdates()
{
	if (d_ask_web_connection){
		QMessageBox msgBox(QMessageBox::Question, tr("QtiPlot"),
		tr("QtiPlot will try to download necessary information about the last available updates. Please modify your firewall settings in order to allow QtiPlot to connect to the internet!"));
		msgBox.setInformativeText(tr("Do you wish to continue?"));
		QPushButton *yesButton = msgBox.addButton(tr("Yes, don't ask me again"), QMessageBox::YesRole);
		msgBox.addButton(QMessageBox::Cancel);
		msgBox.setDefaultButton(yesButton);
		msgBox.setEscapeButton(QMessageBox::Cancel);
		msgBox.setWindowIcon(this->windowIcon());
		msgBox.exec();
		if (msgBox.clickedButton() == yesButton){
			initSearchForUpdates();
			d_ask_web_connection = false;
		}
	} else
		initSearchForUpdates();
}

void ApplicationWindow::initSearchForUpdates()
{
	/*version_buffer.open(IO_WriteOnly);
	http = new QHttp(this);
	connect(http, &QHttp::done, this, &ApplicationWindow::receivedVersionFile);

	QNetworkProxy proxy = QNetworkProxy::applicationProxy();
	if (!proxy.hostName().isEmpty())
		http->setProxy(proxy.hostName(), proxy.port(), proxy.user(), proxy.password());

	http->setHost("soft.proindependent.com");
	http->get("/version.txt", &version_buffer);*/
    QMessageBox::information(this, tr("QtiPlot - Updates"), tr("Update checking is disabled in this version."));
}

void ApplicationWindow::receivedVersionFile(bool)
{
}

/*!
  Turns 3D animation on or off
  */

QString ApplicationWindow::generateUniqueName(const QString& name, bool increment)
{
	int index = 0;
	QStringList lst;
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			lst << QString(w->objectName());
			if (QString(w->objectName()).startsWith(name))
				index++;
		}
		f = f->folderBelow();
	}

	QString newName = name;
	if (increment)//force return of a different name
		newName += QString::number(++index);
	else if (index>0)
		newName += QString::number(index);

	while(lst.contains(newName))
		newName = name + QString::number(++index);

	return newName;
}

void ApplicationWindow::clearTable()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;

	if (QMessageBox::question(this, tr("QtiPlot - Warning"),
				tr("This will clear the contents of all the data associated with the table. Are you sure?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
		return;
	else
		t->clear();
}

void ApplicationWindow::goToRow()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;
	if (!w->inherits("Table") && !w->inherits("Matrix"))
		return;

	int rows = 0;
	if (w->inherits("Table"))
		rows = ((Table *)w)->numRows();
	else if (w->inherits("Matrix"))
		rows = ((Matrix *)w)->numRows();

	bool ok;
	int row = QInputDialog::getInt(this, tr("QtiPlot - Enter row number"), tr("Row"),
			1, 1, rows, 1, &ok, windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMinMaxButtonsHint );
	if ( !ok )
		return;

	if (w->inherits("Table"))
		((Table *)w)->goToRow(row);
	else if (w->inherits("Matrix"))
		((Matrix *)w)->goToRow(row);
}

void ApplicationWindow::goToColumn()
{
	MdiSubWindow *w = activeWindow();
	if (!w)
		return;
	if (!w->inherits("Table") && !w->inherits("Matrix"))
		return;

	int columns = 0;
	if (w->inherits("Table"))
		columns = ((Table *)w)->numCols();
	else if (w->inherits("Matrix"))
		columns = ((Matrix *)w)->numCols();

	bool ok;
	int col = QInputDialog::getInt(this, tr("QtiPlot - Enter column number"), tr("Column"),
			1, 1, columns, 1, &ok, windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMinMaxButtonsHint );
	if ( !ok )
		return;

	if (w->inherits("Table"))
		((Table *)w)->goToColumn(col);
	else if (w->inherits("Matrix"))
		((Matrix *)w)->goToColumn(col);
}

void ApplicationWindow::showScriptWindow(bool parent)
{
	if (!scriptWindow){
		scriptWindow = new ScriptWindow(scriptEnv, this);
		if (d_completion && d_completer)
            scriptWindow->editor()->setCompleter(d_completer);
        scriptWindow->showLineNumbers(d_note_line_numbers);
        scriptWindow->editor()->setTabStopDistance(d_notes_tab_length);
        scriptWindow->editor()->setCurrentFont(d_notes_font);
		scriptWindow->resize(d_script_win_rect.size());
		scriptWindow->move(d_script_win_rect.topLeft());
		connect(scriptWindow, &ScriptWindow::visibilityChanged, actionShowScriptWindow, &QAction::setChecked);
	}

	if (!parent)
		scriptWindow->setAttribute(Qt::WA_DeleteOnClose);

	if (!scriptWindow->isVisible()){
		Qt::WindowFlags flags = {};
		if (d_script_win_on_top)
			flags |= Qt::WindowStaysOnTopHint;
		scriptWindow->setWindowFlags(flags);
		scriptWindow->show();
		scriptWindow->setFocus();
	} else
		scriptWindow->hide();
}

/*!
  Turns perspective mode on or off
  */

/*!
  Resets rotation of 3D plots to default values
  */

/*!
  Finds best layout for the 3D plot
  */

ApplicationWindow::~ApplicationWindow()
{
	disableTools();//avoids crash if a plot tol is still active

    QList<MdiSubWindow *> windows = windowsList();
	for (MdiSubWindow *w : windows){
	    MultiLayer *ml = qobject_cast<MultiLayer *>(w);
		if (ml && ml->hasSelectedLayers())
			delete ml;
	}

	delete hiddenWindows;

	if (scriptWindow)
		delete scriptWindow;

    if (d_text_editor)
		delete d_text_editor;

	QApplication::clipboard()->clear(QClipboard::Clipboard);
	QApplication::restoreOverrideCursor();

#ifdef Q_WS_MAC
	((QtiPlotApplication *)QCoreApplication::instance ())->remove(this);
#endif
}

//Added svn_revision number to end of version string. (SRB 10/01/2010 )
QString ApplicationWindow::versionString()
{
	return "QtiPlot " + QString::number(maj_version) + "." +
		QString::number(min_version) + "." + QString::number(patch_version) + extra_version + QString(svn_revision);
}


int ApplicationWindow::convertOldToNewColorIndex(int cindex)
{
	if( (cindex == 13) || (cindex == 14) ) // white and light gray
		return cindex + 4;

	if(cindex == 15) // dark gray
		return cindex + 8;

	return cindex;
}

void ApplicationWindow::cascade()
{
    const int xoffset = 13;
    const int yoffset = 20;
    int x = 0;
    int y = 0;
	QList<QMdiSubWindow*> windows = d_workspace->subWindowList(QMdiArea::StackingOrder);
    for (QMdiSubWindow *w : windows){
		if (!w->isVisible())
			continue;

        d_workspace->setActiveSubWindow(w);
		((MdiSubWindow *)w)->setNormal();
		w->setGeometry(x, y, w->geometry().width(), w->geometry().height());
        w->raise();
        x += xoffset;
        y += yoffset;
    }
    modifiedProject();
}

ApplicationWindow * ApplicationWindow::loadScript(const QString& fn, bool execute, bool noGui)
{
#ifdef SCRIPTING_PYTHON
	if (noGui){
		hide();
		setScriptingLanguage("Python");

		ScriptUndoScope::setBatchRunnerActive(true);
		ScriptEdit *se = new ScriptEdit(scriptEnv, this);
		se->importASCII(fn);
		se->executeAll();

		int status = scriptEnv ? scriptEnv->lastExitStatus() : 0;
		delete se;
		QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
		QCoreApplication::processEvents();

#ifdef QTIPLOT_LEDGER
		const AllocLedger &l = AllocLedger::instance();
		if (!l.clean()) {
			fprintf(stderr, "LEDGER: objects outliving project:\n%s\n",
					l.report().toUtf8().constData());
			if (status == 0)
				status = 70;
		}
#endif

		::exit(status);
	} else {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		setScriptingLanguage("Python");
		restoreApplicationGeometry();

		showScriptWindow();
		scriptWindow->open(fn);

		QApplication::restoreOverrideCursor();

		if (execute){
			scriptWindow->hide();
    		scriptWindow->executeAll();
			if (scriptWindow->editor()->hasError())
				showScriptWindow();
		}
		return this;
	}
#else
    QMessageBox::critical(this, tr("QtiPlot") + " - " + tr("Error"),
    tr("QtiPlot was not built with Python scripting support included!"));
#endif
	return 0;
}




void ApplicationWindow::hideSelectedWindows()
{
    QList<QTreeWidgetItem *> lst = lv->selectedItems();
	folders->blockSignals(true);
	for (QTreeWidgetItem *item : lst){
		if (item->type() != FolderListItem::RTTI)
			hideWindow(((WindowListItem *)item)->window());
	}
	folders->blockSignals(false);
}

void ApplicationWindow::showSelectedWindows()
{
    QList<QTreeWidgetItem *> lst = lv->selectedItems();

	folders->blockSignals(true);
	for (QTreeWidgetItem *item : lst){
		if (item->type() != FolderListItem::RTTI)
			activateWindow(((WindowListItem *)item)->window());
	}
	folders->blockSignals(false);
}

void ApplicationWindow::swapColumns()
{
    Table *t = (Table *)activeWindow(TableWindow);
	if (!t)
		return;
	QStringList lst = t->selectedColumns();
	if(lst.count() != 2)
        return;

	t->swapColumns(t->colIndex(lst[0]), t->colIndex(lst[1]));
}

void ApplicationWindow::moveColumnRight()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveColumnBy(1);
}

void ApplicationWindow::moveColumnLeft()
{
    Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveColumnBy(-1);
}

void ApplicationWindow::moveColumnFirst()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveColumnBy(-t->selectedColumn());
}

void ApplicationWindow::moveColumnLast()
{
    Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveColumnBy(t->numCols() - t->selectedColumn() - 1);
}

void ApplicationWindow::adjustColumnWidth()
{
    Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->adjustColumnsWidth();
}

void ApplicationWindow::moveTableRowUp()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveRow();
}

void ApplicationWindow::moveTableRowDown()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
    	t->moveRow(false);
}

void ApplicationWindow::restoreApplicationGeometry()
{
	if (qApp->arguments().contains("-X"))
		return;

	if (d_app_rect.isNull()){
		showMaximized();
	} else {
		resize(d_app_rect.size());
		move(d_app_rect.topLeft());
		show();
	}

	MultiLayer *ml = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (ml && ml->isMaximized())
		ml->adjustLayersToCanvasSize();
}

void ApplicationWindow::setVisible(bool visible)
{
    if (visible && qApp->arguments().contains("-X"))
        return;
    QMainWindow::setVisible(visible);
}

void ApplicationWindow::scriptsDirPathChanged(const QString& path)
{
	scriptsDirPath = path;

	QList<MdiSubWindow*> windows = windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Note"))
			((Note*)w)->setDirPath(path);
	}
}

void ApplicationWindow::showToolBarsMenu()
{
	QMenu toolBarsMenu;

	QAction *actionFileTools = new QAction(fileTools->windowTitle(), this);
	
	actionFileTools->setChecked(fileTools->isVisible());
	connect(actionFileTools, &QAction::toggled, fileTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionFileTools);

	QAction *actionEditTools = new QAction(editTools->windowTitle(), this);
	
	actionEditTools->setChecked(editTools->isVisible());
	connect(actionEditTools, &QAction::toggled, editTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionEditTools);

	QAction *actionNoteTools = new QAction(noteTools->windowTitle(), this);
	
	actionNoteTools->setChecked(noteTools->isVisible());
	connect(actionNoteTools, &QAction::toggled, noteTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionNoteTools);

	QAction *actionTableTools = new QAction(tableTools->windowTitle(), this);
	
	actionTableTools->setChecked(tableTools->isVisible());
	connect(actionTableTools, &QAction::toggled, tableTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionTableTools);

	QAction *actionColumnTools = new QAction(columnTools->windowTitle(), this);
	
	actionColumnTools->setChecked(columnTools->isVisible());
	connect(actionColumnTools, &QAction::toggled, columnTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionColumnTools);

	QAction *actionPlotTools = new QAction(plotTools->windowTitle(), this);
	
	actionPlotTools->setChecked(plotTools->isVisible());
	connect(actionPlotTools, &QAction::toggled, plotTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionPlotTools);

	QAction *actionMatrixTools = new QAction(plotMatrixBar->windowTitle(), this);
	
	actionMatrixTools->setChecked(plotMatrixBar->isVisible());
	connect(actionMatrixTools, &QAction::toggled, plotMatrixBar, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionMatrixTools);

	QAction *actionPlot3DTools = new QAction(plot3DTools->windowTitle(), this);
	
	actionPlot3DTools->setChecked(plot3DTools->isVisible());
	connect(actionPlot3DTools, &QAction::toggled, plot3DTools, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionPlot3DTools);

	QAction *actionDisplayBar = new QAction(displayBar->windowTitle(), this);
	
	actionDisplayBar->setChecked(displayBar->isVisible());
	connect(actionDisplayBar, &QAction::toggled, displayBar, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionDisplayBar);

	QAction *actionFormatToolBar = new QAction(formatToolBar->windowTitle(), this);
	
	actionFormatToolBar->setChecked(formatToolBar->isVisible());
	connect(actionFormatToolBar, &QAction::toggled, formatToolBar, &QToolBar::setVisible);
	toolBarsMenu.addAction(actionFormatToolBar);

	QAction *action = toolBarsMenu.exec(QCursor::pos());
	if (!action)
		return;

	MdiSubWindow *w = activeWindow();

	if (action->text() == plotMatrixBar->windowTitle()){
		d_matrix_tool_bar = action->isChecked();
		plotMatrixBar->setEnabled(w && w->inherits("Matrix"));
	} else if (action->text() == tableTools->windowTitle()){
		d_table_tool_bar = action->isChecked();
		tableTools->setEnabled(w && w->inherits("Table"));
	} else if (action->text() == columnTools->windowTitle()){
		d_column_tool_bar = action->isChecked();
		columnTools->setEnabled(w && w->inherits("Table"));
	} else if (action->text() == plotTools->windowTitle()){
		d_plot_tool_bar = action->isChecked();
		plotTools->setEnabled(w && w->inherits("MultiLayer"));
	} else if (action->text() == plot3DTools->windowTitle()){
		d_plot3D_tool_bar = action->isChecked();
		plot3DTools->setEnabled(w && w->inherits("Graph3D"));
	} else if (action->text() == fileTools->windowTitle()){
		d_file_tool_bar = action->isChecked();
	} else if (action->text() == editTools->windowTitle()){
		d_edit_tool_bar = action->isChecked();
	} else if (action->text() == displayBar->windowTitle()){
		d_display_tool_bar = action->isChecked();
	} else if (action->text() == formatToolBar->windowTitle()){
		d_format_tool_bar = action->isChecked();
	} else if (action->text() == noteTools->windowTitle()){
		d_notes_tool_bar = action->isChecked();
	}
}


void ApplicationWindow::matrixDirectFFT()
{
    Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	if (!m->isEmpty())
		m->fft();
	else
		showNoDataMessage();
}

void ApplicationWindow::matrixInverseFFT()
{
    Matrix* m = (Matrix*)activeWindow(MatrixWindow);
	if (!m)
		return;

	if (!m->isEmpty())
		m->fft(true);
	else
		showNoDataMessage();
}

void ApplicationWindow::setFormatBarColor(const QColor& color)
{
	formatToolBar->setEnabled(true);

	ColorButton *cb = (ColorButton *)formatToolBar->widgetForAction(actionTextColor);
	cb->blockSignals(true);
	cb->setColor(color);
	cb->blockSignals(false);
}

void ApplicationWindow::setFormatBarFont(const QFont& font)
{
	formatToolBar->setEnabled(true);

	QFontComboBox *fb = (QFontComboBox *)formatToolBar->widgetForAction(actionFontBox);
	fb->blockSignals(true);
	fb->setCurrentFont(font);
	fb->blockSignals(false);
	fb->setEnabled(true);

	QSpinBox *sb = (QSpinBox *)formatToolBar->widgetForAction(actionFontSize);
	sb->blockSignals(true);
	sb->setValue(font.pointSize());
	sb->blockSignals(false);
	sb->setEnabled(true);

    actionFontBold->blockSignals(true);
	actionFontBold->setChecked(font.bold());
	actionFontBold->blockSignals(false);

	actionFontItalic->blockSignals(true);
	actionFontItalic->setChecked(font.italic());
    actionFontItalic->blockSignals(false);

    actionSubscript->setEnabled(false);
    actionSuperscript->setEnabled(false);
    actionUnderline->setEnabled(false);
    actionGreekSymbol->setEnabled(false);
    actionGreekMajSymbol->setEnabled(false);
    actionMathSymbol->setEnabled(false);
}

void ApplicationWindow::setTextColor()
{
	ColorButton *cb = (ColorButton *)formatToolBar->widgetForAction(actionTextColor);

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (plot){
		Graph* g = plot->activeLayer();
		if (g)
			g->setCurrentColor(cb->color());
	}
}

void ApplicationWindow::setFontSize(int size)
{
    QFontComboBox *fb = (QFontComboBox *)formatToolBar->widgetForAction(actionFontBox);
	QFont f(fb->currentFont().family(), size);
	f.setBold(actionFontBold->isChecked());
	f.setItalic(actionFontItalic->isChecked());

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (plot){
        Graph* g = plot->activeLayer();
        if (g)
            g->setCurrentFont(f);
	}

	Note *n = (Note *)activeWindow(NoteWindow);
	if (n){
	    d_notes_font = f;
        QList<MdiSubWindow *> windows = windowsList();
        for (MdiSubWindow *w : windows){
            Note *m = qobject_cast<Note *>(w);
            if (m)
                m->setFont(f);
        }
	}
}

void ApplicationWindow::setFontFamily(const QFont& font)
{
    QSpinBox *sb = (QSpinBox *)formatToolBar->widgetForAction(actionFontSize);
    QFont f(font.family(), sb->value());
    f.setBold(actionFontBold->isChecked());
    f.setItalic(actionFontItalic->isChecked());

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (plot){
        Graph* g = plot->activeLayer();
        if (g)
            g->setCurrentFont(f);
	}

	Note *n = (Note *)activeWindow(NoteWindow);
	if (n){
	    d_notes_font = f;
        QList<MdiSubWindow *> windows = windowsList();
        for (MdiSubWindow *w : windows){
            Note *m = qobject_cast<Note *>(w);
            if (m)
                m->setFont(f);
        }
	}
}

void ApplicationWindow::setItalicFont(bool italic)
{
    QFontComboBox *fb = (QFontComboBox *)formatToolBar->widgetForAction(actionFontBox);
	QSpinBox *sb = (QSpinBox *)formatToolBar->widgetForAction(actionFontSize);
	QFont f(fb->currentFont().family(), sb->value());
	f.setBold(actionFontBold->isChecked());
	f.setItalic(italic);

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (plot){
        Graph* g = plot->activeLayer();
        if (g)
            g->setCurrentFont(f);
	}

    Note *n = (Note *)activeWindow(NoteWindow);
	if (n){
	    d_notes_font = f;
        QList<MdiSubWindow *> windows = windowsList();
        for (MdiSubWindow *w : windows){
            Note *m = qobject_cast<Note *>(w);
            if (m)
                m->setFont(f);
        }
	}
}

void ApplicationWindow::setBoldFont(bool bold)
{
    QFontComboBox *fb = (QFontComboBox *)formatToolBar->widgetForAction(actionFontBox);
	QSpinBox *sb = (QSpinBox *)formatToolBar->widgetForAction(actionFontSize);
	QFont f(fb->currentFont().family(), sb->value());
	f.setBold(bold);
	f.setItalic(actionFontItalic->isChecked());

	MultiLayer *plot = (MultiLayer *)activeWindow(MultiLayerWindow);
	if (plot){
        Graph* g = plot->activeLayer();
        if (g)
            g->setCurrentFont(f);
	}

    Note *n = (Note *)activeWindow(NoteWindow);
	if (n){
	    d_notes_font = f;
        QList<MdiSubWindow *> windows = windowsList();
        for (MdiSubWindow *w : windows){
            Note *m = qobject_cast<Note *>(w);
            if (m)
                m->setFont(f);
        }
	}
}

void ApplicationWindow::enableTextEditor(Graph *g)
{
	if (!g){
        formatToolBar->setEnabled(false);
	    if (d_text_editor){
            d_text_editor->close();
            d_text_editor = nullptr;
	    }
	} else if (g) {
	    if (!g->activeText() && !g->selectedScale() && !g->titleSelected())
            return;

        d_text_editor = new TextEditor(g);

        formatToolBar->setEnabled(true);
        actionSubscript->setEnabled(true);
        actionSuperscript->setEnabled(true);
        actionUnderline->setEnabled(true);
        actionGreekSymbol->setEnabled(true);
        actionGreekMajSymbol->setEnabled(true);
        actionMathSymbol->setEnabled(true);
	}
}

void ApplicationWindow::insertSuperscript()
{
    if (!d_text_editor)
        return;

    d_text_editor->formatText("<sup>","</sup>");
}

void ApplicationWindow::insertSubscript()
{
    if (!d_text_editor)
        return;

    d_text_editor->formatText("<sub>","</sub>");
}

void ApplicationWindow::underline()
{
    if (!d_text_editor)
        return;

    d_text_editor->formatText("<u>","</u>");
}

void ApplicationWindow::insertGreekSymbol()
{
    if (!d_text_editor)
        return;

    SymbolDialog *greekLetters = new SymbolDialog(SymbolDialog::lowerGreek, this);
	connect(greekLetters, &SymbolDialog::addLetter, d_text_editor, &TextEditor::addSymbol);
	greekLetters->exec();
}

void ApplicationWindow::insertGreekMajSymbol()
{
    if (!d_text_editor)
        return;

    SymbolDialog *greekLetters = new SymbolDialog(SymbolDialog::upperGreek, this);
	connect(greekLetters, &SymbolDialog::addLetter, d_text_editor, &TextEditor::addSymbol);
	greekLetters->exec();
}

void ApplicationWindow::insertMathSymbol()
{
    if (!d_text_editor)
        return;

    SymbolDialog *ms = new SymbolDialog(SymbolDialog::mathSymbols, this);
	connect(ms, &SymbolDialog::addLetter, d_text_editor, &TextEditor::addSymbol);
	ms->exec();
}

void ApplicationWindow::showCustomActionDialog()
{
	if (d_action_manager)
		d_action_manager->showCustomActionDialog();
}

void ApplicationWindow::addCustomAction(QAction *action, const QString& parentName, int index)
{
	if (d_action_manager)
		d_action_manager->addCustomAction(action, parentName, index);
}

void ApplicationWindow::reloadCustomActions()
{
	if (d_action_manager)
		d_action_manager->reloadCustomActions();
}

void ApplicationWindow::removeCustomAction(QAction *action)
{
	if (d_action_manager)
		d_action_manager->removeCustomAction(action);
}

void ApplicationWindow::performCustomAction(QAction *action)
{
	if (d_action_manager)
		d_action_manager->performCustomAction(action);
}

void ApplicationWindow::loadCustomActions()
{
	if (d_action_manager)
		d_action_manager->loadCustomActions();
}

QList<QMenu *> ApplicationWindow::customizableMenusList()
{
	return d_action_manager ? d_action_manager->customizableMenusList() : QList<QMenu *>();
}

QList<QMenu *> ApplicationWindow::menusList()
{
	return d_action_manager ? d_action_manager->menusList() : QList<QMenu *>();
}

QList<QToolBar *> ApplicationWindow::toolBarsList()
{
	return d_action_manager ? d_action_manager->toolBarsList() : QList<QToolBar *>();
}

void ApplicationWindow::hideSelectedColumns()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
		t->hideSelectedColumns();
}

void ApplicationWindow::showAllColumns()
{
	Table *t = (Table *)activeWindow(TableWindow);
	if (t)
		t->showAllColumns();
}

void ApplicationWindow::setMatrixUndoStackSize(int size)
{
    if (d_matrix_undo_stack_size == size)
        return;

    d_matrix_undo_stack_size = size;
    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
		    if (w->inherits("Matrix")){
				QUndoStack *stack = ((Matrix *)w)->undoStack();
				if (!stack->count())// undo limit can only be changed for empty stacks
                	stack->setUndoLimit(size);
			}
		}
		f = f->folderBelow();
	}
}

void ApplicationWindow::setTableUndoStackSize(int size)
{
    if (d_table_undo_stack_size == size)
        return;

    d_table_undo_stack_size = size;
    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
		    if (w->inherits("Table")){
				QUndoStack *stack = ((Table *)w)->undoStack();
				if (!stack->count())// undo limit can only be changed for empty stacks
                	stack->setUndoLimit(size);
			}
		}
		f = f->folderBelow();
	}
}

QString ApplicationWindow::guessEndOfLine(const QString& sample)
{//Try to guess which end-of-line character is used:
    if (sample.indexOf("\r\n") != -1)//Try \r\n first
        return "\r\n";
    else if (sample.indexOf("\r") != -1)//then look for \r
        return "\r";
	// use \n if neither \r\n nor \r have been found
    return "\n";
}

QString ApplicationWindow::endOfLine()
{
	switch(d_eol){
		case LF:
			return "\n";
		break;
		case CRLF:
			return "\r\n";
		break;
		case CR:
			return "\r";
		break;
	}
	return "\n";
}

QStringList ApplicationWindow::windowsNameList()
{
	QStringList names;
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows)
			names << w->objectName();
		f = f->folderBelow();
	}
	return names;
}

void ApplicationWindow::initCompleter()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QStringList words;
	words.append("col");
	words.append("tablecol");
	words.append("cell");
#ifdef SCRIPTING_PYTHON
	if (scriptEnv && scriptEnv->objectName() == QString("Python")){
		QString fn = d_python_config_folder + "/qti_wordlist.txt";
		QFile file(fn);
		if (!file.open(QFile::ReadOnly)){
			QApplication::restoreOverrideCursor();
			if (!qApp->arguments().contains("-X")) {
				QMessageBox::critical(this, tr("QtiPlot - Warning"),
				tr("Couldn't load file: %1.\nAutocompletion will not be available!").arg(QFileInfo(file).absoluteFilePath()));
			}
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		} else {
			while (!file.atEnd()){
				QByteArray line = file.readLine();
				if (!line.isEmpty()){
					QString s = line.trimmed();
					if (!words.contains(s))
						words << s;
				}
			}
			file.close();
		}
		words.append(PythonSyntaxHighlighter::keywordsList());
	}
#endif

	words.append(windowsNameList());

	QList<MdiSubWindow*> lst = tableList();
	for (MdiSubWindow* mw : lst){
		Table *t = (Table*)mw;
		for (int i = 0; i < t->numCols(); i++)
			words.append(t->colName(i));
	}

	QStringList functions = scriptEnv->mathFunctions();
	for (QString s : functions)
		words.append(s);

    words.sort();

	if (!d_completer){
		d_completer = new QCompleter(this);
		d_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
		d_completer->setCompletionMode(QCompleter::PopupCompletion);
	}
	d_completer->setModel(new QStringListModel(words, d_completer));

	QApplication::restoreOverrideCursor();
}

void ApplicationWindow::enableCompletion(bool on)
{
    if (!d_completer || d_completion == on)
        return;

    d_completion = on;

    if (scriptWindow){
		if (d_completion)
            scriptWindow->editor()->setCompleter(d_completer);
        else
            scriptWindow->editor()->setCompleter(0);
    }

    Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			if(w->inherits("Note")){
                if (d_completion)
                    ((Note *)w)->currentEditor()->setCompleter(d_completer);
                else
                    ((Note *)w)->currentEditor()->setCompleter(0);
			}
        }
		f = f->folderBelow();
	}
}



QString ApplicationWindow::stemPlot(Table *t, const QString& colName, int power, int startRow, int endRow)
{
	return d_plot_controller_2d ? d_plot_controller_2d->stemPlot(t, colName, power, startRow, endRow) : QString();
}

QMenu* ApplicationWindow::addCustomMenu(const QString& title, const QString& parentName)
{
	return d_action_manager ? d_action_manager->addCustomMenu(title, parentName) : nullptr;
}

void ApplicationWindow::removeCustomMenu(const QString& title)
{
	if (d_action_manager)
		d_action_manager->removeCustomMenu(title);
}

void ApplicationWindow::reloadCustomMenus()
{
	if (d_action_manager)
		d_action_manager->reloadCustomMenus();
}

QColor ApplicationWindow::readColorFromProject(const QString& name)
{
	QColor c = QColor();

	if (name.contains(",")){
		QStringList lst = name.split(",");
		if (lst.size() == 2){
			c = QColor(lst[0]);
			c.setAlphaF(lst[1].toDouble());
		}
		return c;
	}

	bool canConvertToInt = false;
	int fillColIndex = name.toInt(&canConvertToInt);
	if (canConvertToInt){
		if (fillColIndex < 0)
			return c;
		if (d_file_version <= 89)
			fillColIndex = convertOldToNewColorIndex(fillColIndex);
		c = ColorBox::defaultColor(fillColIndex);
	} else
		c = QColor(name);

	return c;
}

void ApplicationWindow::enableMdiArea(bool on)
{
	if (d_mdi_windows_area == on)
		return;

	d_mdi_windows_area = on;

	if (on){
		QList<MdiSubWindow *> windows = windowsList();
		for (MdiSubWindow *w : windows)
			d_workspace->addSubWindow(w);
	} else {
		QList<QMdiSubWindow *> windows = d_workspace->subWindowList();
		for (QMdiSubWindow *w : windows){
			d_workspace->removeSubWindow(w);
			w->show();
		}
	}
}

void ApplicationWindow::memoryAllocationError()
{
	QMessageBox::critical(0, tr("QtiPlot") + " - " + tr("Memory Allocation Error"),
		tr("Not enough memory, operation aborted!"));
}

void ApplicationWindow::increasePrecision()
{
	Matrix *m = (Matrix *)activeWindow(MatrixWindow);
	if (!m)
		return;

	int oldPrec = m->precision();
	if (oldPrec == 13)
		return;

	int prec = oldPrec + 1;
	QChar format = m->textFormat();
	m->undoStack()->push(new MatrixSetPrecisionCommand(m, format, format,
					oldPrec, prec, tr("Set Precision %1 digits").arg(prec)));
	m->setNumericPrecision(prec);
}

void ApplicationWindow::decreasePrecision()
{
	Matrix *m = (Matrix *)activeWindow(MatrixWindow);
	if (!m)
		return;

	int oldPrec = m->precision();
	if (oldPrec == 0)
		return;

	int prec = oldPrec - 1;
	QChar format = m->textFormat();
	m->undoStack()->push(new MatrixSetPrecisionCommand(m, format, format,
					oldPrec, prec, tr("Set Precision %1 digits").arg(prec)));
	m->setNumericPrecision(prec);
}

void ApplicationWindow::commentSelection()
{
	Note *note = (Note *)activeWindow(NoteWindow);
	if (!note || !note->currentEditor())
		return;

	note->currentEditor()->commentSelection();
}

void ApplicationWindow::uncommentSelection()
{
	Note *note = (Note *)activeWindow(NoteWindow);
	if (!note || !note->currentEditor())
		return;

	note->currentEditor()->uncommentSelection();
}

void ApplicationWindow::execute()
{
	Note *note = (Note *)activeWindow(NoteWindow);
    if (!note)
		return;

	note->execute();
}

void ApplicationWindow::executeAll()
{
	Note *note = (Note *)activeWindow(NoteWindow);
    if (!note)
		return;

	note->executeAll();
}

void ApplicationWindow::evaluate()
{
	Note *note = (Note *)activeWindow(NoteWindow);
    if (!note)
		return;

	note->evaluate();
}

void ApplicationWindow::stopExecution()
{
	if (scriptEnv)
		scriptEnv->stopExecution();
}

void ApplicationWindow::addWindowsListToCompleter()
{
#ifdef SCRIPTING_PYTHON
	if (scriptEnv->objectName() != QString("Python"))
		return;

	if (!d_completer)
		return;

	QStringListModel *model = qobject_cast<QStringListModel *> (d_completer->model());
	if (!model)
		return;

	QStringList lst = model->stringList();
	Folder *f = projectFolder();
	while (f){
		QList<MdiSubWindow *> folderWindows = f->windowsList();
		for (MdiSubWindow *w : folderWindows){
			lst << w->objectName();

			if (w->inherits("Table")){
				Table *t = (Table*)w;
				for (int i = 0; i < t->numCols(); i++)
					lst.append(t->colName(i));
			}
		}
		f = f->folderBelow();
	}
	lst.sort();
	model->setStringList(lst);
#endif
}

void ApplicationWindow::updateCompleter(const QString& windowName, bool remove, const QString& newName)
{
	if (!d_completer || d_is_appending_file || d_opening_file)
		return;

	QStringListModel *model = qobject_cast<QStringListModel *> (d_completer->model());
	if (!model)
		return;

	QStringList lst = model->stringList();

	if (newName.isEmpty()){
		Table *t = table(windowName);
		if (remove){
			lst.removeAll(windowName);
			if (t){
				for (int i = 0; i < t->numCols(); i++)
					lst.removeAll(t->colName(i));
			}
		} else {
			lst.append(windowName);
			if (t){
				for (int i = 0; i < t->numCols(); i++)
					lst.append(t->colName(i));
			}
		}
	} else {
		int index = lst.indexOf(windowName);
		if (index >= 0)
			lst.replace(index, newName);

		Table *t = table(newName);
		if (t){
			for (int i = 0; i < t->numCols(); i++){
				int index = lst.indexOf(windowName + "_" + t->colLabel(i));
				if (index >= 0)
					lst.replace(index, t->colName(i));
			}
		}
	}

	lst.sort();
	model->setStringList(lst);
}

void ApplicationWindow::addColumnNameToCompleter(const QString& colName, bool remove)
{
	if (!d_completer || d_is_appending_file || d_opening_file)
		return;

	QStringListModel *model = qobject_cast<QStringListModel *> (d_completer->model());
	if (!model)
		return;

	QStringList lst = model->stringList();

	if (remove)
		lst.removeAll(colName);
	else
		lst.append(colName);

	lst.sort();
	model->setStringList(lst);
}

#ifdef SCRIPTING_PYTHON
void ApplicationWindow::openQtDesignerUi()
{
	QString fn = getFileName(this, tr("QtiPlot") + " - " + tr("Choose custom user interface"), workingDir, "*.ui", 0, false);
	if (!fn.isEmpty()){
		QFileInfo fi(fn);
		workingDir = fi.absolutePath();
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		setScriptingLanguage("Python");

		QString script = "# " + tr("Custom user interfaces can be created using the QtDesigner application provided by the Qt framework") + ":\n";
		script += "# http://qt.nokia.com\n";
		script += "# " + tr("For more details about how to use .ui files in your Python scripts please read the PyQt4 documentation") + ":\n";
		script += "# http://www.riverbankcomputing.co.uk/static/Docs/PyQt4/pyqt4ref.html#using-the-generated-code\n";
		script += "##############################################################################################\n";
		script += "from PyQt4 import uic\n";
		script += "global ui\n";
		script += "ui = uic.loadUi(\"" + fn + "\")\n";
		script += "ui.show()\n";

		Note *note = newNote();
		note->setText(script);
		note->executeAll();

		QApplication::restoreOverrideCursor();
	}
}

void ApplicationWindow::executeStartupScripts()
{
	QDir dir(d_startup_scripts_folder);
	if (!dir.exists() || !dir.isReadable())
		return;

	QFileInfoList lst = dir.entryInfoList(QStringList("*.py"), QDir::Files|QDir::NoSymLinks|QDir::Readable);
	if (lst.isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	ScriptEdit *se = new ScriptEdit(scriptEnv, this);
	for (int i = 0; i < lst.count(); i++){
		se->importASCII(lst[i].absoluteFilePath());
		se->executeAll();
	}
	delete se;

	QApplication::restoreOverrideCursor();
}
#endif

#ifdef BROWSER_PLUGIN
QTNPFACTORY_BEGIN("QtiPlot Browser Plugin", "A Qt-based NSAPI plug-in application that graphs numeric data");
    QTNPCLASS(ApplicationWindow)
QTNPFACTORY_END()
#endif

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>
QAXFACTORY_BEGIN("{89ab08da-df8c-4bd0-8327-72f73741c1a6}", "{082bd921-0832-4ca7-ab5a-ec06ca7f3350}")
    QAXCLASS(ApplicationWindow)
QAXFACTORY_END()
#endif

void ApplicationWindow::showProVersionMessage()
{
	QMessageBox::critical(this, tr("Feature not available"),
	tr("The requested functionality is not available because the required plugin could not be loaded."));
}

ImportExportPlugin * ApplicationWindow::exportPlugin(const QString& suffix)
{
	for (ImportExportPlugin *plugin : d_import_export_plugins){
		if (plugin->exportFormats().contains(suffix))
			return plugin;
	}

	showProVersionMessage();
	return 0;
}

ImportExportPlugin * ApplicationWindow::importPlugin(const QString& fileName)
{
	for (ImportExportPlugin *plugin : d_import_export_plugins){
		if (plugin->importFormats().contains(QFileInfo(fileName).suffix()))
			return plugin;
	}

	showProVersionMessage();
	return 0;
}

void ApplicationWindow::loadPlugins()
{
	for (QObject *plugin : QPluginLoader::staticInstances()){
		ImportExportPlugin *p = qobject_cast<ImportExportPlugin *>(plugin);
		if (p){
			p->setApplicationWindow(this);
			d_import_export_plugins << p;
		}
	}

	QDir pluginsDir = QDir(qApp->applicationDirPath());
	bool hasPlugins = pluginsDir.cd("plugins");
	if (!hasPlugins) {
		pluginsDir = QDir(QDir::currentPath() + "/build_linux/qtiplot/plugins");
		hasPlugins = pluginsDir.exists();
	}
	if (!hasPlugins) {
		pluginsDir = QDir(QDir::currentPath() + "/plugins");
		hasPlugins = pluginsDir.exists();
	}

	if (hasPlugins) {
		QStringList filters;
#if defined(Q_OS_WIN)
		filters << "*.dll";
#elif defined(Q_OS_MAC)
		filters << "*.dylib";
#else
		filters << "*.so";
#endif
		for (QString fileName : pluginsDir.entryList(filters, QDir::Files)){
			QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
			QObject *plugin = loader.instance();
			if (!plugin) {
				fprintf(stderr, "Plugin load error for %s: %s\n", fileName.toUtf8().constData(), loader.errorString().toUtf8().constData());
				if (!qApp->arguments().contains("-X")) {
					QMessageBox::critical(this, "Plugin Load Error", 
						QString("Failed to load plugin: %1\nError: %2")
						.arg(fileName)
						.arg(loader.errorString()));
				}
			}
			if (plugin){
				ImportExportPlugin *p = qobject_cast<ImportExportPlugin *>(plugin);
				if (p){
					p->setApplicationWindow(this);
					d_import_export_plugins << p;
				} else {
					fprintf(stderr, "Plugin cast failed for %s\n", fileName.toUtf8().constData());
					if (!qApp->arguments().contains("-X")) {
						QMessageBox::warning(this, "Plugin Cast Failed", "Loaded " + fileName + " but cast to ImportExportPlugin failed.");
					}
				}
			}
		}
	}
}
