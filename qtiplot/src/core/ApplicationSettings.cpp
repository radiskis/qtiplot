#include "ApplicationSettings.h"
#include <QApplication>
#include <QDir>
#include <QRegularExpression>
#include <ColorBox.h>
#include <SymbolBox.h>
#include <PatternBox.h>
#include <PenStyleBox.h>
#include <qwt3d_types.h>

ApplicationSettings::ApplicationSettings(QObject *parent)
    : QObject(parent),
      d_3D_color_map(LinearColorMap::rainbow())
{
    init();
}

void ApplicationSettings::init()
{
    QString aux = qApp->applicationDirPath();

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
    plotNumbersFont = QFont(family, pointSize);
    plotLegendFont = appFont;
    plotTitleFont = QFont(family, pointSize + 2, QFont::Bold, false);

    autoSearchUpdates = false;
    appLanguage = QLocale::system().name().section('_', 0, 0);
    show_windows_policy = ActiveFolder;

    workspaceColor = QColor("darkGray");
    panelsColor = QColor("#ffffff");
    panelsTextColor = QColor("#000000");
    tableBkgdColor = QColor("#ffffff");
    tableTextColor = QColor("#000000");
    tableHeaderColor = QColor("#000000");

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
    d_show_axes = QVector<bool>(QwtPlot::axisCnt, true);
    d_show_axes_labels = QVector<bool>(QwtPlot::axisCnt, true);
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
    d_graph_attach_policy = (int)FrameWidget::Scales;
    d_graph_axis_labeling = Graph::Default;
    d_synchronize_graph_scales = true;
    d_print_paper_size = (int)QPageSize::A4;
    d_printer_orientation = (int)QPageLayout::Landscape;
    defaultCurveStyle = int(Graph::LineSymbols);
    defaultCurveLineWidth = 1;
    d_curve_line_style = 0;
    defaultCurveBrush = 0;
    defaultCurveAlpha = 100;
    defaultSymbolSize = 7;
    defaultSymbolEdge = 1.0;
    d_fill_symbols = true;
    d_symbol_style = 0;
    d_indexed_symbols = true;
    d_keep_aspect_ration = true;
    d_show_empty_cell_gap = true;

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
    legendBackground.setAlpha(0);
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
    defaultArrowLineStyle = Qt::SolidLine;

    // 2D grid defaults
    d_grid_antialiased = false;
    d_grid_major_x_enabled = false;
    d_grid_major_x_color = Qt::black;
    d_grid_major_x_style = (int)Qt::SolidLine;
    d_grid_major_x_thickness = 1.0;
    d_grid_minor_x_enabled = false;
    d_grid_minor_x_color = Qt::black;
    d_grid_minor_x_style = (int)Qt::DotLine;
    d_grid_minor_x_thickness = 1.0;
    d_grid_major_y_enabled = false;
    d_grid_major_y_color = Qt::black;
    d_grid_major_y_style = (int)Qt::SolidLine;
    d_grid_major_y_thickness = 1.0;
    d_grid_minor_y_enabled = false;
    d_grid_minor_y_color = Qt::black;
    d_grid_minor_y_style = (int)Qt::DotLine;
    d_grid_minor_y_thickness = 1.0;

    d_3D_legend = true;
    d_3D_projection = Qwt3D::NOFLOOR;
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
    d_3D_major_style = 0;
    d_3D_major_width = 1.0;
    d_3D_minor_style = 0;
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
    d_ASCII_import_first_row_role = 0;
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
    d_export_bitmap_resolution = 96;
    d_export_color = true;
    d_export_escape_tex_strings = true;
    d_export_tex_font_sizes = true;
    d_3D_export_text_mode = 1;
    d_3D_export_sort = 1;

    d_stats_significance_level = 0.05;
    d_stats_result_table = false;
    d_stats_result_log = true;
    d_stats_result_notes = false;
    d_descriptive_stats = false;
    d_stats_confidence = false;
    d_stats_power = false;
    d_stats_output = false;
    d_int_sort_data = true;
    d_int_show_plot = true;
    d_int_results_table = false;
    d_fft_norm_amp = false;
    d_fft_shift_res = true;
    d_fft_power2 = true;

    d_init_window_type = TableWindow;
    d_excel_import_method = Custom;
    d_completion = true;
    d_open_last_project = false;
    thousandsSep = true;
    d_locale = QLocale::system();
    d_clipboard_locale = QLocale::system();
    d_force_muParser = false;
    d_matrix_undo_stack_size = 10;
    d_table_undo_stack_size = 1000;
    d_undo_memory_budget_mb = 64;

    d_indexed_colors = ColorBox::defaultColors();
    d_indexed_color_names = ColorBox::defaultColorNames();
    d_symbols_list = SymbolBox::defaultSymbols();

    d_open_project_filter = "*";
    d_latex_compiler = 0;

    d_ask_web_connection = true;
    d_confirm_modif_2D_points = true;
    d_auto_update_table_values = true;
    d_show_table_paste_dialog = true;

    d_script_win_on_top = false;
    d_script_win_rect = QRect(0, 0, 500, 300);
    d_note_line_numbers = true;
    d_notes_tab_length = 8;
    d_comment_highlight_color = Qt::darkGreen;
    d_keyword_highlight_color = Qt::darkRed;
    d_quotation_highlight_color = Qt::darkBlue;
    d_numeric_highlight_color = Qt::blue;
    d_function_highlight_color = Qt::darkYellow;
    d_class_highlight_color = Qt::darkCyan;

    d_file_tool_bar = true;
    d_edit_tool_bar = true;
    d_table_tool_bar = true;
    d_column_tool_bar = true;
    d_matrix_tool_bar = true;
    d_plot_tool_bar = true;
    d_plot3D_tool_bar = true;
    d_display_tool_bar = false;
    d_format_tool_bar = true;
    d_notes_tool_bar = true;
    d_proxy_port = 8080;
}

void ApplicationSettings::load()
{
#ifdef Q_OS_MAC
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#endif
    load(settings);
}

void ApplicationSettings::load(QSettings &settings)
{
    /* ---------------- group General --------------- */
    settings.beginGroup("/General");
    settings.beginGroup("/ApplicationGeometry");
    d_app_rect = QRect(settings.value("/x", 0).toInt(), settings.value("/y", 0).toInt(),
                       settings.value("/width", 0).toInt(), settings.value("/height", 0).toInt());
    settings.endGroup();

    autoSearchUpdates = settings.value("/AutoSearchUpdates", false).toBool();
    appLanguage = settings.value("/Language", QLocale::system().name().section('_', 0, 0)).toString();
    show_windows_policy = (ShowWindowsPolicy)settings.value("/ShowWindowsPolicy", ActiveFolder).toInt();

    recentProjects = settings.value("/RecentProjects").toStringList();
#ifdef Q_OS_WIN
    if (!recentProjects.isEmpty() && recentProjects[0].contains("^e"))
        recentProjects = recentProjects[0].split("^e", Qt::SkipEmptyParts);
    else if (recentProjects.count() == 1){
        QString s = recentProjects[0];
        if (s.remove(QRegularExpression("\\s")).isEmpty())
            recentProjects = QStringList();
    }
#endif
    d_excel_import_method = (ExcelImportMethod)settings.value("/ExcelImportMethod", d_excel_import_method).toInt();
    appStyle = settings.value("/Style", appStyle).toString();
    autoSave = settings.value("/AutoSave", true).toBool();
    autoSaveTime = settings.value("/AutoSaveTime", 15).toInt();
    d_backup_files = settings.value("/BackupProjects", true).toBool();
    d_init_window_type = (WindowType)settings.value("/InitWindow", TableWindow).toInt();
    d_completion = settings.value("/Completion", true).toBool();
    d_open_last_project = settings.value("/OpenLastProject", d_open_last_project).toBool();
    defaultScriptingLang = settings.value("/ScriptingLang", "muParser").toString();

    thousandsSep = settings.value("/ThousandsSeparator", true).toBool();
    d_locale = QLocale(settings.value("/Locale", QLocale::system().name()).toString());
    if (!thousandsSep)
        d_locale.setNumberOptions(QLocale::OmitGroupSeparator);

    d_decimal_digits = settings.value("/DecimalDigits", 13).toInt();
    d_clipboard_locale = QLocale(settings.value("/ClipboardLocale", QLocale::system().name()).toString());
    d_muparser_c_locale = settings.value("/MuParserCLocale", true).toBool();
    d_force_muParser = settings.value("/ForceMuParser", d_force_muParser).toBool();

    d_matrix_undo_stack_size = settings.value("/MatrixUndoStackSize", 10).toInt();
    d_table_undo_stack_size = settings.value("/TableUndoStackSize", 1000).toInt();
    d_undo_memory_budget_mb = settings.value("/UndoMemoryBudgetMB", 64).toInt();
    d_eol = (EndLineChar)settings.value("/EndOfLine", d_eol).toInt();

    QStringList applicationFont = settings.value("/Font").toStringList();
    if (applicationFont.size() == 4)
        appFont = QFont(applicationFont[0], applicationFont[1].toInt(), applicationFont[2].toInt(), applicationFont[3].toInt());

    QStringList colors = settings.value("/IndexedColors").toStringList();
    if (!colors.isEmpty()){
        d_indexed_colors.clear();
        for (int i = 0; i < colors.size(); i++)
            d_indexed_colors << QColor(colors[i]);
    }
    d_indexed_color_names = settings.value("/IndexedColorNames", d_indexed_color_names).toStringList();

    QStringList symbols = settings.value("/IndexedSymbolsList").toStringList();
    if (!symbols.isEmpty()){
        d_symbols_list.clear();
        for (int i = 0; i < symbols.size(); i++)
            d_symbols_list << symbols[i].toInt();
    }

    settings.beginGroup("/Dialogs");
    d_extended_open_dialog = settings.value("/ExtendedOpenDialog", true).toBool();
    d_extended_export_dialog = settings.value("/ExtendedExportDialog", true).toBool();
    d_extended_import_ASCII_dialog = settings.value("/ExtendedImportAsciiDialog", true).toBool();
    d_extended_plot_dialog = settings.value("/ExtendedPlotDialog", true).toBool();

    settings.beginGroup("/AddRemoveCurves");
    d_add_curves_dialog_size = QSize(settings.value("/Width", 700).toInt(), settings.value("/Height", 400).toInt());
    d_show_current_folder = settings.value("/ShowCurrentFolder", false).toBool();
    settings.endGroup();

    settings.beginGroup("/Statistics");
    d_stats_significance_level = settings.value("/SignificanceLevel", d_stats_significance_level).toDouble();
    d_stats_result_table = settings.value("/ResultTable", d_stats_result_table).toBool();
    d_stats_result_log = settings.value("/ResultLog", d_stats_result_log).toBool();
    d_stats_result_notes = settings.value("/Notes", d_stats_result_notes).toBool();
    d_descriptive_stats = settings.value("/DescriptiveStats", d_descriptive_stats).toBool();
    d_stats_confidence = settings.value("/ConfidenceIntervals", d_stats_confidence).toBool();
    d_stats_power = settings.value("/PowerAnalysis", d_stats_power).toBool();
    d_stats_output = settings.value("/OutputSettings", d_stats_output).toBool();
    settings.endGroup();

    settings.beginGroup("/Integration");
    d_int_sort_data = settings.value("/SortData", d_int_sort_data).toBool();
    d_int_show_plot = settings.value("/ShowPlot", d_int_show_plot).toBool();
    d_int_results_table = settings.value("/ResultsTable", d_int_results_table).toBool();
    settings.endGroup();

    settings.beginGroup("/FFT");
    d_fft_norm_amp = settings.value("/NormalizeAmplitude", d_fft_norm_amp).toBool();
    d_fft_shift_res = settings.value("/ShiftResults", d_fft_shift_res).toBool();
    d_fft_power2 = settings.value("/Power2", d_fft_power2).toBool();
    settings.endGroup();

    settings.endGroup(); // Dialogs

    settings.beginGroup("/Colors");
    workspaceColor = settings.value("/Workspace", "darkGray").value<QColor>();
    panelsColor = settings.value("/Panels", "#ffffff").value<QColor>();
    panelsTextColor = settings.value("/PanelsText", "#000000").value<QColor>();
    settings.endGroup();

    settings.beginGroup("/Paths");
    QString appPath = qApp->applicationDirPath();
    workingDir = settings.value("/WorkingDir", appPath).toString();
    fitPluginsPath = settings.value("/FitPlugins", fitPluginsPath).toString();
#ifdef Q_OS_WIN
    templatesDir = settings.value("/TemplatesDir", appPath).toString();
    asciiDirPath = settings.value("/ASCII", appPath).toString();
    imagesDirPath = settings.value("/Images", appPath).toString();
#else
    templatesDir = settings.value("/TemplatesDir", QDir::homePath()).toString();
    asciiDirPath = settings.value("/ASCII", QDir::homePath()).toString();
    imagesDirPath = settings.value("/Images", QDir::homePath()).toString();
    workingDir = settings.value("/WorkingDir", QDir::homePath()).toString();
#endif
    scriptsDirPath = settings.value("/ScriptsDir", appPath).toString();
    fitModelsPath = settings.value("/FitModelsDir", "").toString();
    customActionsDirPath = settings.value("/CustomActionsDir", "").toString();
    helpFilePath = settings.value("/HelpFile", helpFilePath).toString();
    d_translations_folder = settings.value("/Translations", d_translations_folder).toString();
    d_python_config_folder = settings.value("/PythonConfigDir", d_python_config_folder).toString();
    d_latex_compiler_path = settings.value("/LaTeXCompiler", d_latex_compiler_path).toString();
    d_startup_scripts_folder = settings.value("/StartupScripts", d_startup_scripts_folder).toString();
    d_soffice_path = settings.value("/OpenOffice", d_soffice_path).toString();
    d_java_path = settings.value("/Java", d_java_path).toString();
    d_jodconverter_path = settings.value("/JoDConverter", d_jodconverter_path).toString();
    settings.endGroup(); // Paths

    d_open_project_filter = settings.value("/OpenProjectFilter", d_open_project_filter).toString();
    d_latex_compiler = settings.value("/TeXCompilerPolicy", d_latex_compiler).toInt();
    settings.endGroup(); // General

    /* ---------------- group UserFunctions --------------- */
    settings.beginGroup("/UserFunctions");
    surfaceFunc = settings.value("/SurfaceFunctions").toStringList();
    xFunctions = settings.value("/xFunctions").toStringList();
    yFunctions = settings.value("/yFunctions").toStringList();
    rFunctions = settings.value("/rFunctions").toStringList();
    thetaFunctions = settings.value("/thetaFunctions").toStringList();
    d_param_surface_func = settings.value("/ParametricSurfaces").toStringList();
    d_recent_functions = settings.value("/Functions").toStringList();
    settings.endGroup();

    /* ---------------- group Confirmations --------------- */
    settings.beginGroup("/Confirmations");
    confirmCloseFolder = settings.value("/Folder", true).toBool();
    confirmCloseTable = settings.value("/Table", true).toBool();
    confirmCloseMatrix = settings.value("/Matrix", true).toBool();
    confirmClosePlot2D = settings.value("/Plot2D", true).toBool();
    confirmClosePlot3D = settings.value("/Plot3D", true).toBool();
    confirmCloseNotes = settings.value("/Note", true).toBool();
    d_inform_rename_table = settings.value("/RenameTable", true).toBool();
    d_confirm_overwrite = settings.value("/Overwrite", true).toBool();
    d_ask_web_connection = settings.value("/WebConnection", d_ask_web_connection).toBool();
    d_confirm_modif_2D_points = settings.value("/ModifyDataPoints", d_confirm_modif_2D_points).toBool();
    settings.endGroup();

    /* ---------------- group Tables --------------- */
    settings.beginGroup("/Tables");
    d_show_table_comments = settings.value("/DisplayComments", false).toBool();
    d_auto_update_table_values = settings.value("/AutoUpdateValues", true).toBool();
    d_show_table_paste_dialog = settings.value("/EnablePasteDialog", d_show_table_paste_dialog).toBool();

    QStringList tableFonts = settings.value("/Fonts").toStringList();
    if (tableFonts.size() == 8) {
        tableTextFont = QFont(tableFonts[0], tableFonts[1].toInt(), tableFonts[2].toInt(), tableFonts[3].toInt());
        tableHeaderFont = QFont(tableFonts[4], tableFonts[5].toInt(), tableFonts[6].toInt(), tableFonts[7].toInt());
    }

    settings.beginGroup("/Colors");
    tableBkgdColor = settings.value("/Background", "#ffffff").value<QColor>();
    tableTextColor = settings.value("/Text", "#000000").value<QColor>();
    tableHeaderColor = settings.value("/Header", "#000000").value<QColor>();
    settings.endGroup();
    settings.endGroup(); // Tables

    /* ---------------- group 2D Plots --------------- */
    settings.beginGroup("/2DPlots");
    settings.beginGroup("/General");
    titleOn = settings.value("/Title", true).toBool();
    canvasFrameWidth = settings.value("/CanvasFrameWidth", 0).toInt();
    defaultPlotMargin = settings.value("/Margin", 0).toInt();
    drawBackbones = settings.value("/AxesBackbones", true).toBool();
    axesLineWidth = settings.value("/AxesLineWidth", 1).toInt();
    autoscale2DPlots = settings.value("/Autoscale", true).toBool();
    autoScaleFonts = settings.value("/AutoScaleFonts", true).toBool();
    autoResizeLayers = settings.value("/AutoResizeLayers", true).toBool();
    antialiasing2DPlots = settings.value("/Antialiasing", antialiasing2DPlots).toBool();
    d_scale_plots_on_print = settings.value("/ScaleLayersOnPrint", false).toBool();
    d_print_cropmarks = settings.value("/PrintCropmarks", false).toBool();
    d_layer_geometry_unit = settings.value("/GeometryUnit", d_layer_geometry_unit).toInt();
    d_layer_canvas_width = settings.value("/LayerCanvasWidth", d_layer_canvas_width).toInt();
    d_layer_canvas_height = settings.value("/LayerCanvasHeight", d_layer_canvas_height).toInt();

    QStringList graphFonts = settings.value("/Fonts").toStringList();
    if (graphFonts.size() == 16) {
        plotAxesFont = QFont(graphFonts[0], graphFonts[1].toInt(), graphFonts[2].toInt(), graphFonts[3].toInt());
        plotNumbersFont = QFont(graphFonts[4], graphFonts[5].toInt(), graphFonts[6].toInt(), graphFonts[7].toInt());
        plotLegendFont = QFont(graphFonts[8], graphFonts[9].toInt(), graphFonts[10].toInt(), graphFonts[11].toInt());
        plotTitleFont = QFont(graphFonts[12], graphFonts[13].toInt(), graphFonts[14].toInt(), graphFonts[15].toInt());
    }
    d_in_place_editing = settings.value("/InPlaceEditing", true).toBool();
    d_graph_background_color = settings.value("/BackgroundColor", d_graph_background_color).value<QColor>();
    d_graph_canvas_color = settings.value("/CanvasColor", d_graph_canvas_color).value<QColor>();
    d_graph_border_color = settings.value("/FrameColor", d_graph_border_color).value<QColor>();
    d_graph_background_opacity = settings.value("/BackgroundOpacity", d_graph_background_opacity).toInt();
    d_graph_canvas_opacity = settings.value("/BackgroundOpacity", d_graph_canvas_opacity).toInt();
    d_graph_border_width = settings.value("/FrameWidth", d_graph_border_width).toInt();
    d_canvas_frame_color = settings.value("/FrameColor", QColor(Qt::black)).value<QColor>();
    d_graph_axes_labels_dist = settings.value("/LabelsAxesDist", d_graph_axes_labels_dist).toInt();
    d_graph_tick_labels_dist = settings.value("/TickLabelsDist", d_graph_tick_labels_dist).toInt();
    int size = settings.beginReadArray("EnabledAxes");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        d_show_axes[i] = settings.value("enabled", true).toBool();
        d_show_axes_labels[i] = settings.value("labels", true).toBool();
    }
    settings.endArray();
    d_graph_legend_display = (Graph::LegendDisplayMode)settings.value("/LegendDisplayMode", d_graph_legend_display).toInt();
    d_graph_axis_labeling = (Graph::AxisTitlePolicy)settings.value("/AxisTitlePolicy", d_graph_axis_labeling).toInt();
    d_keep_aspect_ration = settings.value("/KeepAspectRatio", d_keep_aspect_ration).toBool();
    d_synchronize_graph_scales = settings.value("/SynchronizeScales", d_synchronize_graph_scales).toBool();
    d_show_empty_cell_gap = settings.value("/ShowEmptyCellGap", d_show_empty_cell_gap).toBool();
    d_graph_attach_policy = settings.value("/AttachPolicy", d_graph_attach_policy).toInt();
    settings.endGroup(); // General

    settings.beginGroup("/Curves");
    defaultCurveStyle = settings.value("/Style", Graph::LineSymbols).toInt();
    defaultCurveLineWidth = settings.value("/LineWidth", 1).toDouble();
    d_curve_line_style = settings.value("/LineType", d_curve_line_style).toInt();
    defaultSymbolSize = settings.value("/SymbolSize", 7).toInt();
    defaultSymbolEdge = settings.value("/SymbolEdge", defaultSymbolEdge).toDouble();
    d_fill_symbols = settings.value("/FillSymbols", d_fill_symbols).toBool();
    d_symbol_style = settings.value("/SymbolStyle", d_symbol_style).toInt();
    d_indexed_symbols = settings.value("/IndexedSymbols", d_indexed_symbols).toBool();
    defaultCurveBrush = settings.value("/BrushStyle", defaultCurveBrush).toInt();
    defaultCurveAlpha = settings.value("/BrushAlpha", defaultCurveAlpha).toInt();
    d_disable_curve_antialiasing = settings.value("/DisableAntialiasing", d_disable_curve_antialiasing).toBool();
    d_curve_max_antialising_size = settings.value("/MaxCurveAntialisingSize", d_curve_max_antialising_size).toInt();
    d_Douglas_Peuker_tolerance = settings.value("/DouglasPeukerTolerance", d_Douglas_Peuker_tolerance).toDouble();
    d_speed_mode_points = settings.value("/MaxPoints", d_speed_mode_points).toInt();
    d_speed_mode_export = settings.value("/SpeedModeExport", d_speed_mode_export).toBool();
    settings.endGroup(); // Curves

    settings.beginGroup("/Ticks");
    majTicksStyle = settings.value("/MajTicksStyle", ScaleDraw::Out).toInt();
    minTicksStyle = settings.value("/MinTicksStyle", ScaleDraw::Out).toInt();
    minTicksLength = settings.value("/MinTicksLength", 5).toInt();
    majTicksLength = settings.value("/MajTicksLength", 9).toInt();
    settings.endGroup(); // Ticks

    settings.beginGroup("/Legend");
    legendFrameStyle = settings.value("/FrameStyle", LegendWidget::Line).toInt();
    d_frame_widget_pen.setColor(settings.value("/FrameColor", QColor(Qt::black)).value<QColor>());
    d_frame_widget_pen.setWidthF(settings.value("/FrameWidth", 1).toDouble());
    d_frame_widget_pen.setStyle(PenStyleBox::penStyle(settings.value("/FramePenStyle", 0).toInt()));

    legendTextColor = settings.value("/TextColor", "#000000").value<QColor>();
    legendBackground = settings.value("/BackgroundColor", QColor(Qt::white)).value<QColor>();
    legendBackground.setAlpha(settings.value("/Transparency", 0).toInt());
    d_legend_default_angle = settings.value("/Angle", 0).toInt();
    d_frame_geometry_unit = settings.value("/DefaultGeometryUnit", FrameWidget::Scale).toInt();
    settings.endGroup(); // Legend

    settings.beginGroup("/Arrows");
    defaultArrowLineWidth = settings.value("/Width", 1).toDouble();
    defaultArrowColor = settings.value("/Color", "#000000").value<QColor>();
    defaultArrowHeadLength = settings.value("/HeadLength", 4).toInt();
    defaultArrowHeadAngle = settings.value("/HeadAngle", 45).toInt();
    defaultArrowHeadFill = settings.value("/HeadFill", true).toBool();
    defaultArrowLineStyle = Graph::getPenStyle(settings.value("/LineStyle", "SolidLine").toString());
    settings.endGroup(); // Arrows

    settings.beginGroup("/Rectangle");
    d_rect_default_background = settings.value("/BackgroundColor", QColor(Qt::white)).value<QColor>();
    d_rect_default_background.setAlpha(settings.value("/Transparency", 255).toInt());
    d_rect_default_brush.setColor(settings.value("/BrushColor", d_rect_default_brush).value<QColor>());
    d_rect_default_brush.setStyle(PatternBox::brushStyle(settings.value("/Pattern", 0).toInt()));
    settings.endGroup(); // Rectangle

    settings.beginGroup("/Grid");
    d_grid_antialiased = settings.value("/Antialiased", false).toBool();
    d_grid_major_x_enabled = settings.value("/MajorX", false).toBool();
    d_grid_major_x_color = settings.value("/MajorXColor", QColor(Qt::black)).value<QColor>();
    d_grid_major_x_style = settings.value("/MajorXStyle", 0).toInt();
    d_grid_major_x_thickness = settings.value("/MajorXThickness", 1.0).toDouble();

    d_grid_minor_x_enabled = settings.value("/MinorX", false).toBool();
    d_grid_minor_x_color = settings.value("/MinorXColor", QColor(Qt::black)).value<QColor>();
    d_grid_minor_x_style = settings.value("/MinorXStyle", 2).toInt();
    d_grid_minor_x_thickness = settings.value("/MinorXThickness", 1.0).toDouble();

    d_grid_major_y_enabled = settings.value("/MajorY", false).toBool();
    d_grid_major_y_color = settings.value("/MajorYColor", QColor(Qt::black)).value<QColor>();
    d_grid_major_y_style = settings.value("/MajorYStyle", 0).toInt();
    d_grid_major_y_thickness = settings.value("/MajorYThickness", 1.0).toDouble();

    d_grid_minor_y_enabled = settings.value("/MinorY", false).toBool();
    d_grid_minor_y_color = settings.value("/MinorYColor", QColor(Qt::black)).value<QColor>();
    d_grid_minor_y_style = settings.value("/MinorYStyle", 2).toInt();
    d_grid_minor_y_thickness = settings.value("/MinorYThickness", 1.0).toDouble();
    settings.endGroup(); // Grid

    settings.endGroup(); // 2DPlots

    /* ---------------- group 3D Plots --------------- */
    settings.beginGroup("/3DPlots");
    d_3D_legend = settings.value("/Legend", true).toBool();
    d_3D_projection = settings.value("/Projection", d_3D_projection).toInt();
    d_3D_shading = settings.value("/Shading", d_3D_shading).toInt();
    d_3D_smooth_mesh = settings.value("/Antialiasing", true).toBool();
    d_3D_resolution = settings.value("/Resolution", 1).toInt();
    d_3D_orthogonal = settings.value("/Orthogonal", false).toBool();
    d_3D_autoscale = settings.value("/Autoscale", true).toBool();
    d_3D_scale_fonts = settings.value("/ScaleFonts", true).toBool();

    QStringList plot3DFonts = settings.value("/Fonts").toStringList();
    if (plot3DFonts.size() == 12) {
        d_3D_title_font = QFont(plot3DFonts[0], plot3DFonts[1].toInt(), plot3DFonts[2].toInt(), plot3DFonts[3].toInt());
        d_3D_numbers_font = QFont(plot3DFonts[4], plot3DFonts[5].toInt(), plot3DFonts[6].toInt(), plot3DFonts[7].toInt());
        d_3D_axes_font = QFont(plot3DFonts[8], plot3DFonts[9].toInt(), plot3DFonts[10].toInt(), plot3DFonts[11].toInt());
    }

    settings.beginGroup("/Colors");
    QColor max_color = settings.value("/MaxData", QColor(Qt::red)).value<QColor>();
    d_3D_labels_color = settings.value("/Labels", d_3D_labels_color).value<QColor>();
    d_3D_mesh_color = settings.value("/Mesh", d_3D_mesh_color).value<QColor>();
    QColor min_color = settings.value("/MinData", QColor(Qt::blue)).value<QColor>();
    d_3D_numbers_color = settings.value("/Numbers", d_3D_numbers_color).value<QColor>();
    d_3D_axes_color = settings.value("/Axes", d_3D_axes_color).value<QColor>();
    d_3D_background_color = settings.value("/Background", d_3D_background_color).value<QColor>();

    d_3D_color_map = LinearColorMap(min_color, max_color);
    d_3D_color_map.setMode((QwtLinearColorMap::Mode)settings.value("/ColorMapMode", QwtLinearColorMap::ScaledColors).toInt());
    QList<QVariant> stop_values = settings.value("/ColorMapStops").toList();
    QStringList stop_colors = settings.value("/ColorMapColors").toStringList();
    for (int i = 0; i < stop_colors.size(); i++)
        d_3D_color_map.addColorStop(stop_values[i].toDouble(), QColor(stop_colors[i]));
    settings.endGroup(); // Colors

    settings.beginGroup("/Grids");
    d_3D_major_grids = settings.value("/EnableMajor", d_3D_major_grids).toBool();
    d_3D_grid_color = settings.value("/MajorColor", d_3D_grid_color).value<QColor>();
    d_3D_major_style = settings.value("/MajorStyle", d_3D_major_style).toInt();
    d_3D_major_width = settings.value("/MajorWidth", d_3D_major_width).toDouble();

    d_3D_minor_grids = settings.value("/EnableMinor", d_3D_minor_grids).toBool();
    d_3D_minor_grid_color = settings.value("/MinorColor", d_3D_minor_grid_color).value<QColor>();
    d_3D_minor_style = settings.value("/MinorStyle", d_3D_minor_style).toInt();
    d_3D_minor_width = settings.value("/MinorWidth", d_3D_minor_width).toDouble();
    settings.endGroup(); // Grids
    settings.endGroup(); // 3DPlots

    /* ---------------- group Fitting --------------- */
    settings.beginGroup("/Fitting");
    fit_output_precision = settings.value("/OutputPrecision", 15).toInt();
    pasteFitResultsToPlot = settings.value("/PasteResultsToPlot", false).toBool();
    d_write_fit_results_to_log = settings.value("/WriteResultsToLog", true).toBool();
    generateUniformFitPoints = settings.value("/GenerateFunction", true).toBool();
    fitPoints = settings.value("/Points", 100).toInt();
    generatePeakCurves = settings.value("/GeneratePeakCurves", true).toBool();
    peakCurvesColor = QColor(settings.value("/PeakColor", peakCurvesColor.name()).toString());
    fit_scale_errors = settings.value("/ScaleErrors", true).toBool();
    d_2_linear_fit_points = settings.value("/TwoPointsLinearFit", true).toBool();
    d_multi_peak_messages = settings.value("/MultiPeakToolMsg", d_multi_peak_messages).toBool();
    settings.endGroup(); // Fitting

    /* ---------------- group ImportASCII --------------- */
    settings.beginGroup("/ImportASCII");
    columnSeparator = settings.value("/ColumnSeparator", "\\t").toString();
    columnSeparator.replace("\\t", "\t").replace("\\s", " ");
    ignoredLines = settings.value("/IgnoreLines", 0).toInt();
    renameColumns = settings.value("/RenameColumns", true).toBool();
    strip_spaces = settings.value("/StripSpaces", false).toBool();
    simplify_spaces = settings.value("/SimplifySpaces", false).toBool();
    d_ASCII_file_filter = settings.value("/AsciiFileTypeFilter", "*").toString();
    d_ASCII_import_locale = QLocale(settings.value("/AsciiImportLocale", QLocale::system().name()).toString());
    if (settings.value("/OmitGroupSeparator", false).toBool())
        d_ASCII_import_locale.setNumberOptions(QLocale::OmitGroupSeparator);

    d_ASCII_import_mode = settings.value("/ImportMode", ImportASCIIDialog::NewTables).toInt();
    d_ASCII_comment_string = settings.value("/CommentString", "#").toString();
    d_ASCII_import_comments = settings.value("/ImportComments", false).toBool();
    d_ASCII_import_read_only = settings.value("/ImportReadOnly", false).toBool();
    d_ASCII_import_preview = settings.value("/Preview", true).toBool();
    d_preview_lines = settings.value("/PreviewLines", 100).toInt();
    d_ASCII_end_line = (EndLineChar)settings.value("/EndLineCharacter", d_ASCII_end_line).toInt();
    d_ASCII_import_first_row_role = settings.value("/FirstLineRole", 0).toInt();
    d_import_ASCII_dialog_size = settings.value("/DialogSize", d_import_ASCII_dialog_size).toSize();
    settings.endGroup(); // ImportASCII

    /* ---------------- group ExportASCII --------------- */
    settings.beginGroup("/ExportASCII");
    d_export_col_separator = settings.value("/ColumnSeparator", "\\t").toString();
    d_export_col_separator.replace("\\t", "\t").replace("\\s", " ");
    d_export_col_names = settings.value("/ExportLabels", false).toBool();
    d_export_col_comment = settings.value("/ExportComments", false).toBool();
    d_export_table_selection = settings.value("/ExportSelection", false).toBool();
    d_export_ASCII_file_filter = settings.value("/ExportAsciiFilter", d_export_ASCII_file_filter).toString();
    settings.endGroup(); // ExportASCII

    /* ---------------- group ExportImage --------------- */
    settings.beginGroup("/ExportImage");
    d_image_export_filter = settings.value("/ImageFileTypeFilter", ".png").toString();
    d_export_transparency = settings.value("/ExportTransparency", false).toBool();
    d_export_quality = settings.value("/ImageQuality", 100).toInt();
    d_export_vector_resolution = settings.value("/Resolution", d_export_vector_resolution).toInt();
    d_export_color = settings.value("/ExportColor", true).toBool();
    d_3D_export_text_mode = settings.value("/3DTextMode", d_3D_export_text_mode).toInt();
    d_3D_export_sort = settings.value("/3DSortMode", d_3D_export_sort).toInt();
    d_export_bitmap_resolution = settings.value("/BitmapResolution", d_export_bitmap_resolution).toInt();
    d_export_raster_size = settings.value("/RasterSize", d_export_raster_size).toSizeF();
    d_export_size_unit = settings.value("/SizeUnit", d_export_size_unit).toInt();
    d_scale_fonts_factor = settings.value("/ScaleFontsFactor", d_scale_fonts_factor).toDouble();
    d_export_escape_tex_strings = settings.value("/EscapeTeXStrings", true).toBool();
    d_export_tex_font_sizes = settings.value("/ExportTeXFontSize", true).toBool();
    d_export_compression = settings.value("/Compression", d_export_compression).toBool();
    settings.endGroup(); // ExportImage

    /* ---------------- group ScriptWindow --------------- */
    settings.beginGroup("/ScriptWindow");
    d_script_win_on_top = settings.value("/AlwaysOnTop", false).toBool();
    d_script_win_rect = QRect(settings.value("/x", 0).toInt(), settings.value("/y", 0).toInt(),
                              settings.value("/width", 500).toInt(), settings.value("/height", 300).toInt());
    settings.endGroup();

    /* ---------------- group ToolBars --------------- */
    settings.beginGroup("/ToolBars");
    d_file_tool_bar = settings.value("/FileToolBar", true).toBool();
    d_edit_tool_bar = settings.value("/EditToolBar", true).toBool();
    d_table_tool_bar = settings.value("/TableToolBar", true).toBool();
    d_column_tool_bar = settings.value("/ColumnToolBar", true).toBool();
    d_matrix_tool_bar = settings.value("/MatrixToolBar", true).toBool();
    d_plot_tool_bar = settings.value("/PlotToolBar", true).toBool();
    d_plot3D_tool_bar = settings.value("/Plot3DToolBar", true).toBool();
    d_display_tool_bar = settings.value("/DisplayToolBar", false).toBool();
    d_format_tool_bar = settings.value("/FormatToolBar", true).toBool();
    d_notes_tool_bar = settings.value("/NotesToolBar", true).toBool();
    settings.endGroup();

    /* ---------------- group Notes --------------- */
    settings.beginGroup("/Notes");
    d_note_line_numbers = settings.value("/LineNumbers", true).toBool();
    d_notes_tab_length = settings.value("/TabLength", d_notes_tab_length).toInt();
    d_notes_font.setFamily(settings.value("/FontFamily", d_notes_font.family()).toString());
    d_notes_font.setPointSize(settings.value("/FontSize", d_notes_font.pointSize()).toInt());
    d_notes_font.setBold(settings.value("/FontBold", d_notes_font.bold()).toBool());
    d_notes_font.setItalic(settings.value("/FontItalic", d_notes_font.italic()).toBool());

    settings.beginGroup("/SyntaxHighlighting");
    d_comment_highlight_color = settings.value("/Comments", d_comment_highlight_color).value<QColor>();
    d_keyword_highlight_color = settings.value("/Keywords", d_keyword_highlight_color).value<QColor>();
    d_quotation_highlight_color = settings.value("/Quotations", d_quotation_highlight_color).value<QColor>();
    d_numeric_highlight_color = settings.value("/Numbers", d_numeric_highlight_color).value<QColor>();
    d_function_highlight_color = settings.value("/Functions", d_function_highlight_color).value<QColor>();
    d_class_highlight_color = settings.value("/QtClasses", d_class_highlight_color).value<QColor>();
    settings.endGroup();
    settings.endGroup(); // Notes

    /* ---------------- group PrintPreview --------------- */
    settings.beginGroup("/PrintPreview");
    d_print_paper_size = settings.value("/PaperSize", (int)d_print_paper_size).toInt();
    d_printer_orientation = settings.value("/Orientation", (int)d_printer_orientation).toInt();
    settings.endGroup();

    /* ---------------- group Proxy --------------- */
    settings.beginGroup("/Proxy");
    d_proxy_host = settings.value("/Host", QString()).toString();
    d_proxy_port = settings.value("/Port", 8080).toInt();
    d_proxy_user = settings.value("/Username", QString()).toString();
    settings.endGroup();
}

void ApplicationSettings::save()
{
#ifdef Q_OS_MAC
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ProIndependent", "QtiPlot");
#endif
    save(settings);
}

void ApplicationSettings::save(QSettings &settings)
{
    /* ---------------- group General --------------- */
    settings.beginGroup("/General");
    settings.beginGroup("/ApplicationGeometry");
    settings.setValue("/x", d_app_rect.x());
    settings.setValue("/y", d_app_rect.y());
    settings.setValue("/width", d_app_rect.width());
    settings.setValue("/height", d_app_rect.height());
    settings.endGroup();

    settings.setValue("/AutoSearchUpdates", autoSearchUpdates);
    settings.setValue("/Language", appLanguage);
    settings.setValue("/ShowWindowsPolicy", (int)show_windows_policy);
    settings.setValue("/RecentProjects", recentProjects);
    settings.setValue("/ExcelImportMethod", (int)d_excel_import_method);
    settings.setValue("/Style", appStyle);
    settings.setValue("/AutoSave", autoSave);
    settings.setValue("/AutoSaveTime", autoSaveTime);
    settings.setValue("/BackupProjects", d_backup_files);
    settings.setValue("/InitWindow", (int)d_init_window_type);
    settings.setValue("/Completion", d_completion);
    settings.setValue("/OpenLastProject", d_open_last_project);
    settings.setValue("/ScriptingLang", defaultScriptingLang);

    bool omitGroup = (d_locale.numberOptions() & QLocale::OmitGroupSeparator) ? true : false;
    settings.setValue("/ThousandsSeparator", !omitGroup);

    settings.setValue("/Locale", d_locale.name());
    settings.setValue("/DecimalDigits", d_decimal_digits);
    settings.setValue("/ClipboardLocale", d_clipboard_locale.name());
    settings.setValue("/MuParserCLocale", d_muparser_c_locale);
    settings.setValue("/ForceMuParser", d_force_muParser);
    settings.setValue("/MatrixUndoStackSize", d_matrix_undo_stack_size);
    settings.setValue("/TableUndoStackSize", d_table_undo_stack_size);
    settings.setValue("/UndoMemoryBudgetMB", d_undo_memory_budget_mb);
    settings.setValue("/EndOfLine", (int)d_eol);

    QStringList applicationFont;
    applicationFont << appFont.family()
                    << QString::number(appFont.pointSize())
                    << QString::number(appFont.weight())
                    << QString::number(appFont.italic());
    settings.setValue("/Font", applicationFont);

    QStringList indexedColors;
    for (int i = 0; i < d_indexed_colors.size(); i++)
        indexedColors << d_indexed_colors[i].name();
    settings.setValue("/IndexedColors", indexedColors);
    settings.setValue("/IndexedColorNames", d_indexed_color_names);

    QStringList indexedSymbols;
    for (int i = 0; i < d_symbols_list.size(); i++)
        indexedSymbols << QString::number(d_symbols_list[i]);
    settings.setValue("/IndexedSymbolsList", indexedSymbols);

    settings.beginGroup("/Dialogs");
    settings.setValue("/ExtendedOpenDialog", d_extended_open_dialog);
    settings.setValue("/ExtendedExportDialog", d_extended_export_dialog);
    settings.setValue("/ExtendedImportAsciiDialog", d_extended_import_ASCII_dialog);
    settings.setValue("/ExtendedPlotDialog", d_extended_plot_dialog);
    settings.beginGroup("/AddRemoveCurves");
    settings.setValue("/Width", d_add_curves_dialog_size.width());
    settings.setValue("/Height", d_add_curves_dialog_size.height());
    settings.setValue("/ShowCurrentFolder", d_show_current_folder);
    settings.endGroup();

    settings.beginGroup("/Statistics");
    settings.setValue("/SignificanceLevel", d_stats_significance_level);
    settings.setValue("/ResultTable", d_stats_result_table);
    settings.setValue("/ResultLog", d_stats_result_log);
    settings.setValue("/Notes", d_stats_result_notes);
    settings.setValue("/DescriptiveStats", d_descriptive_stats);
    settings.setValue("/ConfidenceIntervals", d_stats_confidence);
    settings.setValue("/PowerAnalysis", d_stats_power);
    settings.setValue("/OutputSettings", d_stats_output);
    settings.endGroup();

    settings.beginGroup("/Integration");
    settings.setValue("/SortData", d_int_sort_data);
    settings.setValue("/ShowPlot", d_int_show_plot);
    settings.setValue("/ResultsTable", d_int_results_table);
    settings.endGroup();

    settings.beginGroup("/FFT");
    settings.setValue("/NormalizeAmplitude", d_fft_norm_amp);
    settings.setValue("/ShiftResults", d_fft_shift_res);
    settings.setValue("/Power2", d_fft_power2);
    settings.endGroup();
    settings.endGroup(); // Dialogs

    settings.beginGroup("/Colors");
    settings.setValue("/Workspace", workspaceColor);
    settings.setValue("/Panels", panelsColor);
    settings.setValue("/PanelsText", panelsTextColor);
    settings.endGroup();

    settings.beginGroup("/Paths");
    settings.setValue("/WorkingDir", workingDir);
    settings.setValue("/TemplatesDir", templatesDir);
    settings.setValue("/HelpFile", helpFilePath);
    settings.setValue("/FitPlugins", fitPluginsPath);
    settings.setValue("/ASCII", asciiDirPath);
    settings.setValue("/Images", imagesDirPath);
    settings.setValue("/ScriptsDir", scriptsDirPath);
    settings.setValue("/FitModelsDir", fitModelsPath);
    settings.setValue("/CustomActionsDir", customActionsDirPath);
    settings.setValue("/Translations", d_translations_folder);
    settings.setValue("/PythonConfigDir", d_python_config_folder);
    settings.setValue("/LaTeXCompiler", d_latex_compiler_path);
    settings.setValue("/StartupScripts", d_startup_scripts_folder);
    settings.setValue("/OpenOffice", d_soffice_path);
    settings.setValue("/Java", d_java_path);
    settings.setValue("/JoDConverter", d_jodconverter_path);
    settings.endGroup();

    settings.setValue("/OpenProjectFilter", d_open_project_filter);
    settings.setValue("/TeXCompilerPolicy", d_latex_compiler);
    settings.endGroup(); // General

    /* ---------------- group UserFunctions --------------- */
    settings.beginGroup("/UserFunctions");
    settings.setValue("/SurfaceFunctions", surfaceFunc);
    settings.setValue("/xFunctions", xFunctions);
    settings.setValue("/yFunctions", yFunctions);
    settings.setValue("/rFunctions", rFunctions);
    settings.setValue("/thetaFunctions", thetaFunctions);
    settings.setValue("/ParametricSurfaces", d_param_surface_func);
    settings.setValue("/Functions", d_recent_functions);
    settings.endGroup();

    /* ---------------- group Confirmations --------------- */
    settings.beginGroup("/Confirmations");
    settings.setValue("/Folder", confirmCloseFolder);
    settings.setValue("/Table", confirmCloseTable);
    settings.setValue("/Matrix", confirmCloseMatrix);
    settings.setValue("/Plot2D", confirmClosePlot2D);
    settings.setValue("/Plot3D", confirmClosePlot3D);
    settings.setValue("/Note", confirmCloseNotes);
    settings.setValue("/RenameTable", d_inform_rename_table);
    settings.setValue("/Overwrite", d_confirm_overwrite);
    settings.setValue("/WebConnection", d_ask_web_connection);
    settings.setValue("/ModifyDataPoints", d_confirm_modif_2D_points);
    settings.endGroup();

    /* ---------------- group Tables --------------- */
    settings.beginGroup("/Tables");
    settings.setValue("/DisplayComments", d_show_table_comments);
    settings.setValue("/AutoUpdateValues", d_auto_update_table_values);
    settings.setValue("/EnablePasteDialog", d_show_table_paste_dialog);
    QStringList tableFonts;
    tableFonts << tableTextFont.family()
               << QString::number(tableTextFont.pointSize())
               << QString::number(tableTextFont.weight())
               << QString::number(tableTextFont.italic())
               << tableHeaderFont.family()
               << QString::number(tableHeaderFont.pointSize())
               << QString::number(tableHeaderFont.weight())
               << QString::number(tableHeaderFont.italic());
    settings.setValue("/Fonts", tableFonts);

    settings.beginGroup("/Colors");
    settings.setValue("/Background", tableBkgdColor);
    settings.setValue("/Text", tableTextColor);
    settings.setValue("/Header", tableHeaderColor);
    settings.endGroup();
    settings.endGroup(); // Tables

    /* ---------------- group 2D Plots --------------- */
    settings.beginGroup("/2DPlots");
    settings.beginGroup("/General");
    settings.setValue("/Title", titleOn);
    settings.setValue("/CanvasFrameWidth", canvasFrameWidth);
    settings.setValue("/Margin", defaultPlotMargin);
    settings.setValue("/AxesBackbones", drawBackbones);
    settings.setValue("/AxesLineWidth", axesLineWidth);
    settings.setValue("/Autoscale", autoscale2DPlots);
    settings.setValue("/AutoScaleFonts", autoScaleFonts);
    settings.setValue("/AutoResizeLayers", autoResizeLayers);
    settings.setValue("/Antialiasing", antialiasing2DPlots);
    settings.setValue("/ScaleLayersOnPrint", d_scale_plots_on_print);
    settings.setValue("/PrintCropmarks", d_print_cropmarks);
    settings.setValue("/GeometryUnit", d_layer_geometry_unit);
    settings.setValue("/LayerCanvasWidth", d_layer_canvas_width);
    settings.setValue("/LayerCanvasHeight", d_layer_canvas_height);

    QStringList graphFonts;
    graphFonts << plotAxesFont.family()
               << QString::number(plotAxesFont.pointSize())
               << QString::number(plotAxesFont.weight())
               << QString::number(plotAxesFont.italic())
               << plotNumbersFont.family()
               << QString::number(plotNumbersFont.pointSize())
               << QString::number(plotNumbersFont.weight())
               << QString::number(plotNumbersFont.italic())
               << plotLegendFont.family()
               << QString::number(plotLegendFont.pointSize())
               << QString::number(plotLegendFont.weight())
               << QString::number(plotLegendFont.italic())
               << plotTitleFont.family()
               << QString::number(plotTitleFont.pointSize())
               << QString::number(plotTitleFont.weight())
               << QString::number(plotTitleFont.italic());
    settings.setValue("/Fonts", graphFonts);

    settings.setValue("/InPlaceEditing", d_in_place_editing);
    settings.setValue("/BackgroundColor", d_graph_background_color);
    settings.setValue("/CanvasColor", d_graph_canvas_color);
    settings.setValue("/FrameColor", d_graph_border_color);
    settings.setValue("/BackgroundOpacity", d_graph_background_opacity);
    settings.setValue("/FrameWidth", d_graph_border_width);
    settings.setValue("/LabelsAxesDist", d_graph_axes_labels_dist);
    settings.setValue("/TickLabelsDist", d_graph_tick_labels_dist);
    settings.beginWriteArray("EnabledAxes");
    for (int i = 0; i < QwtPlot::axisCnt; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("axis", i);
        settings.setValue("enabled", d_show_axes[i]);
        settings.setValue("labels", d_show_axes_labels[i]);
    }
    settings.endArray();

    settings.setValue("/LegendDisplayMode", (int)d_graph_legend_display);
    settings.setValue("/AxisTitlePolicy", (int)d_graph_axis_labeling);
    settings.setValue("/KeepAspectRatio", d_keep_aspect_ration);
    settings.setValue("/SynchronizeScales", d_synchronize_graph_scales);
    settings.setValue("/ShowEmptyCellGap", d_show_empty_cell_gap);
    settings.setValue("/AttachPolicy", d_graph_attach_policy);
    settings.endGroup(); // General

    settings.beginGroup("/Curves");
    settings.setValue("/Style", defaultCurveStyle);
    settings.setValue("/LineWidth", defaultCurveLineWidth);
    settings.setValue("/LineType", d_curve_line_style);
    settings.setValue("/SymbolSize", defaultSymbolSize);
    settings.setValue("/SymbolEdge", defaultSymbolEdge);
    settings.setValue("/FillSymbols", d_fill_symbols);
    settings.setValue("/SymbolStyle", d_symbol_style);
    settings.setValue("/IndexedSymbols", d_indexed_symbols);
    settings.setValue("/BrushStyle", defaultCurveBrush);
    settings.setValue("/BrushAlpha", defaultCurveAlpha);
    settings.setValue("/DisableAntialiasing", d_disable_curve_antialiasing);
    settings.setValue("/MaxCurveAntialisingSize", d_curve_max_antialising_size);
    settings.setValue("/DouglasPeukerTolerance", d_Douglas_Peuker_tolerance);
    settings.setValue("/MaxPoints", d_speed_mode_points);
    settings.setValue("/SpeedModeExport", d_speed_mode_export);
    settings.endGroup(); // Curves

    settings.beginGroup("/Ticks");
    settings.setValue("/MajTicksStyle", majTicksStyle);
    settings.setValue("/MinTicksStyle", minTicksStyle);
    settings.setValue("/MinTicksLength", minTicksLength);
    settings.setValue("/MajTicksLength", majTicksLength);
    settings.endGroup(); // Ticks

    settings.beginGroup("/Legend");
    settings.setValue("/FrameStyle", legendFrameStyle);
    settings.setValue("/FrameColor", d_frame_widget_pen.color().name());
    settings.setValue("/FrameWidth", d_frame_widget_pen.widthF());
    settings.setValue("/FramePenStyle", PenStyleBox::styleIndex(d_frame_widget_pen.style()));
    settings.setValue("/TextColor", legendTextColor);
    settings.setValue("/BackgroundColor", legendBackground);
    settings.setValue("/Transparency", legendBackground.alpha());
    settings.setValue("/Angle", d_legend_default_angle);
    settings.setValue("/DefaultGeometryUnit", d_frame_geometry_unit);
    settings.endGroup(); // Legend

    settings.beginGroup("/Arrows");
    settings.setValue("/Width", defaultArrowLineWidth);
    settings.setValue("/Color", defaultArrowColor.name());
    settings.setValue("/HeadLength", defaultArrowHeadLength);
    settings.setValue("/HeadAngle", defaultArrowHeadAngle);
    settings.setValue("/HeadFill", defaultArrowHeadFill);
    settings.setValue("/LineStyle", Graph::penStyleName(defaultArrowLineStyle));
    settings.endGroup(); // Arrows

    settings.beginGroup("/Rectangle");
    settings.setValue("/BackgroundColor", d_rect_default_background);
    settings.setValue("/Transparency", d_rect_default_background.alpha());
    settings.setValue("/BrushColor", d_rect_default_brush.color());
    settings.setValue("/Pattern", PatternBox::patternIndex(d_rect_default_brush.style()));
    settings.endGroup(); // Rectangle

    settings.beginGroup("/Grid");
    settings.setValue("/Antialiased", d_grid_antialiased);
    settings.setValue("/MajorX", d_grid_major_x_enabled);
    settings.setValue("/MajorXColor", d_grid_major_x_color);
    settings.setValue("/MajorXStyle", d_grid_major_x_style);
    settings.setValue("/MajorXThickness", d_grid_major_x_thickness);
    settings.setValue("/MinorX", d_grid_minor_x_enabled);
    settings.setValue("/MinorXColor", d_grid_minor_x_color);
    settings.setValue("/MinorXStyle", d_grid_minor_x_style);
    settings.setValue("/MinorXThickness", d_grid_minor_x_thickness);
    settings.setValue("/MajorY", d_grid_major_y_enabled);
    settings.setValue("/MajorYColor", d_grid_major_y_color);
    settings.setValue("/MajorYStyle", d_grid_major_y_style);
    settings.setValue("/MajorYThickness", d_grid_major_y_thickness);
    settings.setValue("/MinorY", d_grid_minor_y_enabled);
    settings.setValue("/MinorYColor", d_grid_minor_y_color);
    settings.setValue("/MinorYStyle", d_grid_minor_y_style);
    settings.setValue("/MinorYThickness", d_grid_minor_y_thickness);
    settings.endGroup(); // Grid

    settings.endGroup(); // 2DPlots

    /* ---------------- group 3D Plots --------------- */
    settings.beginGroup("/3DPlots");
    settings.setValue("/Legend", d_3D_legend);
    settings.setValue("/Projection", d_3D_projection);
    settings.setValue("/Shading", d_3D_shading);
    settings.setValue("/Antialiasing", d_3D_smooth_mesh);
    settings.setValue("/Resolution", d_3D_resolution);
    settings.setValue("/Orthogonal", d_3D_orthogonal);
    settings.setValue("/Autoscale", d_3D_autoscale);
    settings.setValue("/ScaleFonts", d_3D_scale_fonts);

    QStringList plot3DFonts;
    plot3DFonts << d_3D_title_font.family()
                << QString::number(d_3D_title_font.pointSize())
                << QString::number(d_3D_title_font.weight())
                << QString::number(d_3D_title_font.italic())
                << d_3D_numbers_font.family()
                << QString::number(d_3D_numbers_font.pointSize())
                << QString::number(d_3D_numbers_font.weight())
                << QString::number(d_3D_numbers_font.italic())
                << d_3D_axes_font.family()
                << QString::number(d_3D_axes_font.pointSize())
                << QString::number(d_3D_axes_font.weight())
                << QString::number(d_3D_axes_font.italic());
    settings.setValue("/Fonts", plot3DFonts);

    settings.beginGroup("/Colors");
    settings.setValue("/MaxData", d_3D_color_map.color2());
    settings.setValue("/Labels", d_3D_labels_color);
    settings.setValue("/Mesh", d_3D_mesh_color);
    settings.setValue("/MinData", d_3D_color_map.color1());
    settings.setValue("/Numbers", d_3D_numbers_color);
    settings.setValue("/Axes", d_3D_axes_color);
    settings.setValue("/Background", d_3D_background_color);

    settings.setValue("/ColorMapMode", d_3D_color_map.mode());
    QList<QVariant> stop_values;
    QStringList stop_colors;
    QVector<double> colors = d_3D_color_map.colorStops();
    int stops = (int)colors.size() - 1;
    for (int i = 1; i < stops; i++){
        stop_values << QVariant(colors[i]);
        stop_colors << d_3D_color_map.color(i).name();
    }
    settings.setValue("/ColorMapStops", QVariant(stop_values));
    settings.setValue("/ColorMapColors", stop_colors);
    settings.endGroup(); // Colors

    settings.beginGroup("/Grids");
    settings.setValue("/EnableMajor", d_3D_major_grids);
    settings.setValue("/MajorColor", d_3D_grid_color);
    settings.setValue("/MajorStyle", d_3D_major_style);
    settings.setValue("/MajorWidth", d_3D_major_width);
    settings.setValue("/EnableMinor", d_3D_minor_grids);
    settings.setValue("/MinorColor", d_3D_minor_grid_color);
    settings.setValue("/MinorStyle", d_3D_minor_style);
    settings.setValue("/MinorWidth", d_3D_minor_width);
    settings.endGroup(); // Grids
    settings.endGroup(); // 3DPlots

    /* ---------------- group Fitting --------------- */
    settings.beginGroup("/Fitting");
    settings.setValue("/OutputPrecision", fit_output_precision);
    settings.setValue("/PasteResultsToPlot", pasteFitResultsToPlot);
    settings.setValue("/WriteResultsToLog", d_write_fit_results_to_log);
    settings.setValue("/GenerateFunction", generateUniformFitPoints);
    settings.setValue("/Points", fitPoints);
    settings.setValue("/GeneratePeakCurves", generatePeakCurves);
    settings.setValue("/PeakColor", peakCurvesColor.name());
    settings.setValue("/ScaleErrors", fit_scale_errors);
    settings.setValue("/TwoPointsLinearFit", d_2_linear_fit_points);
    settings.setValue("/MultiPeakToolMsg", d_multi_peak_messages);
    settings.endGroup(); // Fitting

    /* ---------------- group ImportASCII --------------- */
    settings.beginGroup("/ImportASCII");
    QString sep = columnSeparator;
    settings.setValue("/ColumnSeparator", sep.replace("\t", "\\t").replace(" ", "\\s"));
    settings.setValue("/IgnoreLines", ignoredLines);
    settings.setValue("/RenameColumns", renameColumns);
    settings.setValue("/StripSpaces", strip_spaces);
    settings.setValue("/SimplifySpaces", simplify_spaces);
    settings.setValue("/AsciiFileTypeFilter", d_ASCII_file_filter);
    settings.setValue("/AsciiImportLocale", d_ASCII_import_locale.name());

    bool omitGroupSep = (d_ASCII_import_locale.numberOptions() & QLocale::OmitGroupSeparator) ? true : false;
    settings.setValue("/OmitGroupSeparator", omitGroupSep);

    settings.setValue("/ImportMode", d_ASCII_import_mode);
    settings.setValue("/CommentString", d_ASCII_comment_string);
    settings.setValue("/ImportComments", d_ASCII_import_comments);
    settings.setValue("/ImportReadOnly", d_ASCII_import_read_only);
    settings.setValue("/Preview", d_ASCII_import_preview);
    settings.setValue("/PreviewLines", d_preview_lines);
    settings.setValue("/EndLineCharacter", (int)d_ASCII_end_line);
    settings.setValue("/FirstLineRole", d_ASCII_import_first_row_role);
    settings.setValue("/DialogSize", d_import_ASCII_dialog_size);
    settings.endGroup(); // ImportASCII

    /* ---------------- group ExportASCII --------------- */
    settings.beginGroup("/ExportASCII");
    sep = d_export_col_separator;
    settings.setValue("/ColumnSeparator", sep.replace("\t", "\\t").replace(" ", "\\s"));
    settings.setValue("/ExportLabels", d_export_col_names);
    settings.setValue("/ExportComments", d_export_col_comment);
    settings.setValue("/ExportSelection", d_export_table_selection);
    settings.setValue("/ExportAsciiFilter", d_export_ASCII_file_filter);
    settings.endGroup(); // ExportASCII

    /* ---------------- group ExportImage --------------- */
    settings.beginGroup("/ExportImage");
    settings.setValue("/ImageFileTypeFilter", d_image_export_filter);
    settings.setValue("/ExportTransparency", d_export_transparency);
    settings.setValue("/ImageQuality", d_export_quality);
    settings.setValue("/Resolution", d_export_vector_resolution);
    settings.setValue("/ExportColor", d_export_color);
    settings.setValue("/3DTextMode", d_3D_export_text_mode);
    settings.setValue("/3DSortMode", d_3D_export_sort);
    settings.setValue("/BitmapResolution", d_export_bitmap_resolution);
    settings.setValue("/RasterSize", d_export_raster_size);
    settings.setValue("/SizeUnit", d_export_size_unit);
    settings.setValue("/ScaleFontsFactor", d_scale_fonts_factor);
    settings.setValue("/EscapeTeXStrings", d_export_escape_tex_strings);
    settings.setValue("/ExportTeXFontSize", d_export_tex_font_sizes);
    settings.setValue("/Compression", d_export_compression);
    settings.endGroup(); // ExportImage

    /* ---------------- group ScriptWindow --------------- */
    settings.beginGroup("/ScriptWindow");
    settings.setValue("/AlwaysOnTop", d_script_win_on_top);
    settings.setValue("/x", d_script_win_rect.x());
    settings.setValue("/y", d_script_win_rect.y());
    settings.setValue("/width", d_script_win_rect.width());
    settings.setValue("/height", d_script_win_rect.height());
    settings.endGroup();

    /* ---------------- group ToolBars --------------- */
    settings.beginGroup("/ToolBars");
    settings.setValue("/FileToolBar", d_file_tool_bar);
    settings.setValue("/EditToolBar", d_edit_tool_bar);
    settings.setValue("/TableToolBar", d_table_tool_bar);
    settings.setValue("/ColumnToolBar", d_column_tool_bar);
    settings.setValue("/MatrixToolBar", d_matrix_tool_bar);
    settings.setValue("/PlotToolBar", d_plot_tool_bar);
    settings.setValue("/Plot3DToolBar", d_plot3D_tool_bar);
    settings.setValue("/DisplayToolBar", d_display_tool_bar);
    settings.setValue("/FormatToolBar", d_format_tool_bar);
    settings.setValue("/NotesToolBar", d_notes_tool_bar);
    settings.endGroup();

    /* ---------------- group Notes --------------- */
    settings.beginGroup("/Notes");
    settings.setValue("/LineNumbers", d_note_line_numbers);
    settings.setValue("/TabLength", d_notes_tab_length);
    settings.setValue("/FontFamily", d_notes_font.family());
    settings.setValue("/FontSize", d_notes_font.pointSize());
    settings.setValue("/FontBold", d_notes_font.bold());
    settings.setValue("/FontItalic", d_notes_font.italic());
    settings.beginGroup("/SyntaxHighlighting");
    settings.setValue("/Comments", d_comment_highlight_color.name());
    settings.setValue("/Keywords", d_keyword_highlight_color.name());
    settings.setValue("/Quotations", d_quotation_highlight_color.name());
    settings.setValue("/Numbers", d_numeric_highlight_color.name());
    settings.setValue("/Functions", d_function_highlight_color.name());
    settings.setValue("/QtClasses", d_class_highlight_color.name());
    settings.endGroup();
    settings.endGroup(); // Notes

    /* ---------------- group PrintPreview --------------- */
    settings.beginGroup("/PrintPreview");
    settings.setValue("/PaperSize", d_print_paper_size);
    settings.setValue("/Orientation", d_printer_orientation);
    settings.endGroup();

    /* ---------------- group Proxy --------------- */
    if (!d_proxy_host.isEmpty()){
        settings.beginGroup("/Proxy");
        settings.setValue("/Host", d_proxy_host);
        settings.setValue("/Port", d_proxy_port);
        settings.setValue("/Username", d_proxy_user);
        settings.endGroup();
    } else {
        settings.remove("/Proxy");
    }
}
