#ifndef APPLICATION_SETTINGS_H
#define APPLICATION_SETTINGS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QFont>
#include <QLocale>
#include <QRect>
#include <QSize>
#include <QSizeF>
#include <QPen>
#include <QBrush>
#include <QSettings>
#include <QVector>
#include <QPageSize>
#include <QPageLayout>

#include "Graph.h"
#include "LegendWidget.h"
#include "FrameWidget.h"
#include "ScaleDraw.h"
#include "LinearColorMap.h"
#include "ImportASCIIDialog.h"

class Grid;

class ApplicationSettings : public QObject
{
    Q_OBJECT

public:
    enum ShowWindowsPolicy { HideAll, ActiveFolder, SubFolders };
    enum WindowType { TableWindow, MatrixWindow, Plot2DWindow, Plot3DWindow, NoteWindow };
    enum EndLineChar { LF, CRLF, CR };
    enum ExcelImportMethod { Custom, OLE, Native };

    explicit ApplicationSettings(QObject *parent = nullptr);
    ~ApplicationSettings() override = default;

    void init();
    void load();
    void load(QSettings &settings);
    void save();
    void save(QSettings &settings);

    // General settings
    QRect d_app_rect;
    bool autoSearchUpdates;
    QString appLanguage;
    ShowWindowsPolicy show_windows_policy;
    QStringList recentProjects;
    ExcelImportMethod d_excel_import_method;
    QString appStyle;
    bool autoSave;
    int autoSaveTime;
    bool d_backup_files;
    WindowType d_init_window_type;
    bool d_completion;
    bool d_open_last_project;
    QString defaultScriptingLang;
    bool thousandsSep;
    QLocale d_locale;
    int d_decimal_digits;
    QLocale d_clipboard_locale;
    bool d_muparser_c_locale;
    bool d_force_muParser;
    int d_matrix_undo_stack_size;
    int d_table_undo_stack_size;
    int d_undo_memory_budget_mb;
    EndLineChar d_eol;
    QFont appFont;
    QList<QColor> d_indexed_colors;
    QStringList d_indexed_color_names;
    QList<int> d_symbols_list;
    QColor workspaceColor;
    QColor panelsColor;
    QColor panelsTextColor;
    QString d_open_project_filter;
    int d_latex_compiler;

    // Dialogs
    bool d_extended_open_dialog;
    bool d_extended_export_dialog;
    bool d_extended_import_ASCII_dialog;
    bool d_extended_plot_dialog;
    QSize d_add_curves_dialog_size;
    bool d_show_current_folder;
    double d_stats_significance_level;
    bool d_stats_result_table;
    bool d_stats_result_log;
    bool d_stats_result_notes;
    bool d_descriptive_stats;
    bool d_stats_confidence;
    bool d_stats_power;
    bool d_stats_output;
    bool d_int_sort_data;
    bool d_int_show_plot;
    bool d_int_results_table;
    bool d_fft_norm_amp;
    bool d_fft_shift_res;
    bool d_fft_power2;

    // Paths
    QString workingDir;
    QString templatesDir;
    QString helpFilePath;
    QString fitPluginsPath;
    QString asciiDirPath;
    QString imagesDirPath;
    QString scriptsDirPath;
    QString fitModelsPath;
    QString customActionsDirPath;
    QString d_translations_folder;
    QString d_python_config_folder;
    QString d_latex_compiler_path;
    QString d_startup_scripts_folder;
    QString d_soffice_path;
    QString d_java_path;
    QString d_jodconverter_path;

    // User Functions
    QStringList surfaceFunc;
    QStringList xFunctions;
    QStringList yFunctions;
    QStringList rFunctions;
    QStringList thetaFunctions;
    QStringList d_param_surface_func;
    QStringList d_recent_functions;

    // Confirmations
    bool confirmCloseFolder;
    bool confirmCloseTable;
    bool confirmCloseMatrix;
    bool confirmClosePlot2D;
    bool confirmClosePlot3D;
    bool confirmCloseNotes;
    bool d_inform_rename_table;
    bool d_confirm_overwrite;
    bool d_ask_web_connection;
    bool d_confirm_modif_2D_points;

    // Tables
    bool d_show_table_comments;
    bool d_auto_update_table_values;
    bool d_show_table_paste_dialog;
    QFont tableTextFont;
    QFont tableHeaderFont;
    QColor tableBkgdColor;
    QColor tableTextColor;
    QColor tableHeaderColor;

    // 2D Plots
    bool titleOn;
    int canvasFrameWidth;
    int defaultPlotMargin;
    bool drawBackbones;
    int axesLineWidth;
    bool autoscale2DPlots;
    bool autoScaleFonts;
    bool autoResizeLayers;
    bool antialiasing2DPlots;
    bool d_scale_plots_on_print;
    bool d_print_cropmarks;
    int d_layer_geometry_unit;
    int d_layer_canvas_width;
    int d_layer_canvas_height;
    QFont plotAxesFont;
    QFont plotNumbersFont;
    QFont plotLegendFont;
    QFont plotTitleFont;
    bool d_in_place_editing;
    QColor d_graph_background_color;
    QColor d_graph_canvas_color;
    QColor d_graph_border_color;
    int d_graph_background_opacity;
    int d_graph_canvas_opacity;
    int d_graph_border_width;
    QColor d_canvas_frame_color;
    int d_graph_axes_labels_dist;
    int d_graph_tick_labels_dist;
    QVector<bool> d_show_axes;
    QVector<bool> d_show_axes_labels;
    Graph::LegendDisplayMode d_graph_legend_display;
    Graph::AxisTitlePolicy d_graph_axis_labeling;
    bool d_keep_aspect_ration;
    bool d_synchronize_graph_scales;
    bool d_show_empty_cell_gap;
    int d_graph_attach_policy;
    int defaultCurveStyle;
    double defaultCurveLineWidth;
    int d_curve_line_style;
    int defaultSymbolSize;
    double defaultSymbolEdge;
    bool d_fill_symbols;
    int d_symbol_style;
    bool d_indexed_symbols;
    int defaultCurveBrush;
    int defaultCurveAlpha;
    bool d_disable_curve_antialiasing;
    int d_curve_max_antialising_size;
    double d_Douglas_Peuker_tolerance;
    Graph::DecimationMethod d_decimation_method;
    int d_speed_mode_points;
    bool d_speed_mode_export;
    int majTicksStyle;
    int minTicksStyle;
    int minTicksLength;
    int majTicksLength;
    int legendFrameStyle;
    QPen d_frame_widget_pen;
    QColor legendTextColor;
    QColor legendBackground;
    int d_legend_default_angle;
    int d_frame_geometry_unit;
    double defaultArrowLineWidth;
    QColor defaultArrowColor;
    int defaultArrowHeadLength;
    int defaultArrowHeadAngle;
    bool defaultArrowHeadFill;
    Qt::PenStyle defaultArrowLineStyle;
    QColor d_rect_default_background;
    QBrush d_rect_default_brush;

    // 2D Grid defaults
    bool d_grid_antialiased;
    bool d_grid_major_x_enabled;
    QColor d_grid_major_x_color;
    int d_grid_major_x_style;
    double d_grid_major_x_thickness;
    bool d_grid_minor_x_enabled;
    QColor d_grid_minor_x_color;
    int d_grid_minor_x_style;
    double d_grid_minor_x_thickness;
    bool d_grid_major_y_enabled;
    QColor d_grid_major_y_color;
    int d_grid_major_y_style;
    double d_grid_major_y_thickness;
    bool d_grid_minor_y_enabled;
    QColor d_grid_minor_y_color;
    int d_grid_minor_y_style;
    double d_grid_minor_y_thickness;

    // 3D Plots
    bool d_3D_legend;
    int d_3D_projection;
    int d_3D_shading;
    bool d_3D_smooth_mesh;
    int d_3D_resolution;
    bool d_3D_orthogonal;
    bool d_3D_autoscale;
    bool d_3D_scale_fonts;
    QFont d_3D_axes_font;
    QFont d_3D_numbers_font;
    QFont d_3D_title_font;
    LinearColorMap d_3D_color_map;
    QColor d_3D_labels_color;
    QColor d_3D_mesh_color;
    QColor d_3D_numbers_color;
    QColor d_3D_axes_color;
    QColor d_3D_background_color;
    bool d_3D_major_grids;
    QColor d_3D_grid_color;
    int d_3D_major_style;
    double d_3D_major_width;
    bool d_3D_minor_grids;
    QColor d_3D_minor_grid_color;
    int d_3D_minor_style;
    double d_3D_minor_width;

    // Fitting
    int fit_output_precision;
    bool pasteFitResultsToPlot;
    bool d_write_fit_results_to_log;
    bool generateUniformFitPoints;
    int fitPoints;
    bool generatePeakCurves;
    QColor peakCurvesColor;
    bool fit_scale_errors;
    bool d_2_linear_fit_points;
    bool d_multi_peak_messages;

    // ASCII Import/Export
    QString columnSeparator;
    int ignoredLines;
    bool renameColumns;
    bool strip_spaces;
    bool simplify_spaces;
    QString d_ASCII_file_filter;
    QLocale d_ASCII_import_locale;
    int d_ASCII_import_mode;
    QString d_ASCII_comment_string;
    bool d_ASCII_import_comments;
    bool d_ASCII_import_read_only;
    bool d_ASCII_import_preview;
    int d_preview_lines;
    EndLineChar d_ASCII_end_line;
    int d_ASCII_import_first_row_role;
    QSize d_import_ASCII_dialog_size;
    QString d_export_col_separator;
    bool d_export_col_names;
    bool d_export_col_comment;
    bool d_export_table_selection;
    QString d_export_ASCII_file_filter;

    // Image Export
    QString d_image_export_filter;
    bool d_export_transparency;
    int d_export_quality;
    int d_export_vector_resolution;
    bool d_export_color;
    int d_3D_export_text_mode;
    int d_3D_export_sort;
    int d_export_bitmap_resolution;
    QSizeF d_export_raster_size;
    int d_export_size_unit;
    double d_scale_fonts_factor;
    bool d_export_escape_tex_strings;
    bool d_export_tex_font_sizes;
    bool d_export_compression;

    // Notes & Script Window
    bool d_script_win_on_top;
    QRect d_script_win_rect;
    bool d_note_line_numbers;
    int d_notes_tab_length;
    QFont d_notes_font;
    QColor d_comment_highlight_color;
    QColor d_keyword_highlight_color;
    QColor d_quotation_highlight_color;
    QColor d_numeric_highlight_color;
    QColor d_function_highlight_color;
    QColor d_class_highlight_color;

    // Toolbars
    bool d_file_tool_bar;
    bool d_edit_tool_bar;
    bool d_table_tool_bar;
    bool d_column_tool_bar;
    bool d_matrix_tool_bar;
    bool d_plot_tool_bar;
    bool d_plot3D_tool_bar;
    bool d_display_tool_bar;
    bool d_format_tool_bar;
    bool d_notes_tool_bar;

    // Print Preview
    int d_print_paper_size;
    int d_printer_orientation;

    // Proxy
    QString d_proxy_host;
    int d_proxy_port;
    QString d_proxy_user;
};

#endif // APPLICATION_SETTINGS_H
