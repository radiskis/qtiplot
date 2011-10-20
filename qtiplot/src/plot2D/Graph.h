/***************************************************************************
    File                 : Graph.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Graph widget

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
#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QPointer>
#include <QPrinter>
#include <QVector>
#include <QEvent>
#include <QMap>

#include <qwt_text.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>

#include <AxesDialog.h>
#include <PlotToolInterface.h>
#include <ScaleDraw.h>
#include <FrameWidget.h>
#include <float.h>

class QwtPlotPanner;
class QwtPlotMagnifier;
class QwtPlotCurve;
class QwtPlotZoomer;
class PieCurve;
class Table;
class ArrowMarker;
class ImageWidget;
class TitlePicker;
class ScalePicker;
class CanvasPicker;
class ApplicationWindow;
class Matrix;
class SelectionMoveResizer;
class RangeSelectorTool;
class ImageProfilesTool;
class DataCurve;
class PlotCurve;
class ErrorBarsCurve;
class MultiLayer;
class Spectrogram;
class FunctionCurve;
class VectorCurve;
class BoxCurve;
class QwtHistogram;
class Grid;
class TexWidget;
class LegendWidget;

//! Structure containing curve layout parameters
typedef struct{
  QColor lCol;     //!< line color
  float lWidth;    //!< line width
  int lStyle;      //!< line style
  double filledArea;  //!< flag: toggles area filling under curve; if > 0 it containts the alphaF value of the brush color
  QColor aCol;     //!< curve area color
  int aStyle;      //!< area filling style
  QColor symCol;   //!< symbol outline color
  QColor fillCol;  //!< symbol fill color
  float penWidth;  //!< symbol outline width
  int sSize;       //!< symbol size
  int sType;       //!< symbol type (shape)
  int connectType; //!< symbol connection type
}  CurveLayout;

/**
 * \brief A 2D-plotting widget.
 *
 * Graphs are managed by a MultiLayer, where they are sometimes referred to as "graphs" and sometimes as "layers".
 * Other parts of the code also call them "plots", regardless of the fact that there's also a class Plot.
 * Within the user interface, they are quite consistently called "layers".
 *
 * Each graph owns a Plot called #d_plot, which handles parts of the curve, axis and marker management (similarly to QwtPlot),
 * as well as the pickers #d_zoomer (a QwtPlotZoomer), #titlePicker (a TitlePicker), #scalePicker (a ScalePicker) and #cp (a CanvasPicker),
 * which handle various parts of the user interaction.
 *
 * Graph contains support for various curve types (see #CurveType),
 * some of them relying on Qtiplot-specific QwtPlotCurve subclasses for parts of the functionality.
 *
 * %Note that some of Graph's methods are implemented in analysis.cpp.
 *
 * \section future Future Plans
 * Merge with Plot and CanvasPicker.
 * Think about merging in TitlePicker and ScalePicker.
 * On the other hand, things like range selection, peak selection or (re)moving data points could be split out into tool classes
 * like QwtPlotZoomer or SelectionMoveResizer.
 *
 * What definitely should be split out are plot types like histograms and pie charts (TODO: which others?).
 * We need a generic framework for this in any case so that new plot types can be implemented in plugins,
 * and Graph could do with a little diet. Especially after merging in Plot and CanvasPicker.
 * [ Framework needs to support plug-ins; assigned to ion ]
 */

class Graph: public QwtPlot
{
	Q_OBJECT

	public:
		Graph (int x = 0, int y = 0, int width = 500, int height = 400, QWidget* parent=0, Qt::WFlags f=0);
		~Graph();

		enum Axis{Left, Right, Bottom, Top};
		enum Scale{Linear, Log10, Ln, Log2, Reciprocal, Probability, Logit};
		enum Ticks{NoTicks = 0, Out = 1, InOut = 2, In = 3};
		enum MarkerType{None = -1, Text = 0, Arrow = 1, Image = 2};
		enum CurveType{Line, Scatter, LineSymbols, VerticalBars, Area, Pie, VerticalDropLines,
			Spline, HorizontalSteps, Histogram, HorizontalBars, VectXYXY, ErrorBars,
			Box, VectXYAM, VerticalSteps, ColorMap, GrayScale, Contour, Function, ImagePlot,
			StackBar, StackColumn};
		enum LegendDisplayMode{Auto, ColumnName, ColumnComment, TableName, TableLegend, DataSetName};
		enum AxisTitlePolicy{Default, ColName, ColComment, NameAndComment};

		//! Returns a pointer to the parent MultiLayer object.
		MultiLayer *multiLayer() const;

		//! Change the active tool, deleting the old one if it exists.
		void setActiveTool(PlotToolInterface *tool);
		//! Return the active tool, or NULL if none is active.
		PlotToolInterface* activeTool() const { return d_active_tool; }
		//! Returns true if a plot/data tool is enabled.
		bool hasActiveTool();

		QList <LegendWidget *> textsList();
		LegendWidget *activeText();
		void setActiveText(LegendWidget *l){d_active_enrichment = (FrameWidget *)l;};
		void select(QWidget *l, bool add = false);

		FrameWidget *activeEnrichment(){return d_active_enrichment;};
		QList <FrameWidget *> enrichmentsList(){return d_enrichments;};
		QList <FrameWidget *> increasingAreaEnrichmentsList();

		bool hasSeletedItems();
		void deselect();
		void deselect(QWidget *);
		void selectCanvas();

		QPointer<SelectionMoveResizer> selectionMoveResizer(){return d_markers_selector;};

		QwtPlotItem* selectedCurveLabels();
        //! Used when restoring DataCurve curveID from a project file
        void restoreCurveLabels(int curveID, const QStringList& lst);

		Grid *grid(){return (Grid *)d_grid;};
		QList<QwtPlotItem *> curvesList(){return d_curves;};
		void setCurvesList(QList<QwtPlotItem *> lst){d_curves = lst;};

		QwtPlotItem* closestCurve(int xpos, int ypos, int &dist, int &point);

		void insertMarker(QwtPlotMarker *m);

		QList<int> getMajorTicksType();
		void setMajorTicksType(int axis, int type);

		QList<int> getMinorTicksType();
		void setMinorTicksType(int axis, int type);

		int minorTickLength() const;
		int majorTickLength() const;
		void setTickLength (int minLength, int majLength);

		int axesLinewidth() const;
		void setAxesLinewidth(int width);

    	void axisLabelFormat(int axis, char &f, int &prec) const;

		int axisLabelFormat(int axis);
		int axisLabelPrecision(int axis);

		QColor frameColor();
		const QColor & paletteBackgroundColor() const;

		void print(QPainter *, const QRect &rect, const QwtPlotPrintFilter & = QwtPlotPrintFilter());
		void updateLayout();
		void setCanvasGeometry(const QRect &canvasRect);
		//!Convenience function for scripts
		void setCanvasGeometry(int x, int y, int w, int h){setCanvasGeometry(QRect(x, y, w, h));};
		void setCanvasSize(const QSize &size);
		//!Convenience function for scripts
		void setCanvasSize(int w, int h){setCanvasSize(QSize(w, h));};

    	void updateCurveLabels();

		TexWidget* addTexFormula(const QString& s, const QPixmap& pix);

		FrameWidget* add(FrameWidget* fw, bool copy = true);
		void remove(FrameWidget*);

		QRect boundingRect();
		void raiseEnrichements();
		void addLegendItem();
		bool isPrinting(){return d_is_printing;};

		void enablePanningMagnifier(bool on = true, int mode = 0);
		bool hasPanningMagnifierEnabled(){if (d_magnifier && d_panner) return true; return false;};
		QwtPlotMagnifier* magnifyTool(){return d_magnifier;};

#ifdef TEX_OUTPUT
		static QString escapeTeXSpecialCharacters(const QString &);
		static QString texSuperscripts(const QString &);
#endif
		void changeCurveIndex(int fromIndex, int toIndex);
		void enableDouglasPeukerSpeedMode(double tolerance, int maxPoints = 3000);

		int speedModeMaxPoints(){return d_speed_mode_points;};
		double getDouglasPeukerTolerance(){return d_Douglas_Peuker_tolerance;};

		AxisTitlePolicy axisTitlePolicy(){return d_axis_title_policy;};
		void setAxisTitlePolicy(const AxisTitlePolicy& policy){d_axis_title_policy = policy;};

		bool hasSynchronizedScaleDivisions(){return d_synchronize_scales;};
		void setSynchronizedScaleDivisions(bool on){d_synchronize_scales = on;};

		QRectF pageGeometry(){return d_page_rect;}
		void setPageGeometry(const QRectF& r){d_page_rect = r;}

	public slots:
		void copy(Graph* g);
		void copyCurves(Graph* g);
		void copyEnrichments(Graph* g);
		void copyScaleWidget(Graph* g, int i);
		void copyScaleDraw(Graph* g, int i);

		//! \name Pie Curves
		//@{
		//! Returns true if this Graph is a pie plot, false otherwise.
		bool isPiePlot();
		//! Used when creating a pie plot.
		PieCurve* plotPie(Table* w,const QString& name, int startRow = 0, int endRow = -1);
		//! Used when restoring a pie plot from a project file.
		PieCurve* plotPie(Table* w, const QString& name, const QPen& pen, int brush, int size,
			int firstColor, int startRow = 0, int endRow = -1, bool visible = true,
			double d_start_azimuth = 270, double d_view_angle = 90, double d_thickness = 33,
			double d_horizontal_offset = 0.0, double d_edge_dist = 25, bool d_counter_clockwise = false,
			bool d_auto_labeling = true, bool d_values = false, bool d_percentages = true,
			bool d_categories = false, bool d_fixed_labels_pos = true);

		void removePie();
		QString pieLegendText();
		QString savePieCurveLayout();
		//@}

		bool addCurves(Table* w, const QStringList& names, int style = 0, double lWidth = 1, int sSize = 3, int startRow = 0, int endRow = -1);
		DataCurve* insertCurve(Table* w, const QString& name, int style, int startRow = 0, int endRow = -1);
		DataCurve* insertCurve(Table* w, int xcol, const QString& name, int style);
		DataCurve* insertCurve(Table* w, const QString& xColName, const QString& yColName, int style, int startRow = 0, int endRow = -1);
		DataCurve* insertCurve(Table* xt, const QString& xColName, Table* yt, const QString& yColName, int style, int startRow = 0, int endRow = -1);
		void insertPlotItem(QwtPlotItem *i, int type);
		void insertCurve(QwtPlotItem *c);

		//! Shows/Hides a curve defined by its index.
		void showCurve(int index, bool visible = true);
		int visibleCurves();

		void removeCurve(QwtPlotItem *it);
		//! Removes a curve defined by its index.
		void removeCurve(int index);
		/**
		 * \brief Removes a curve defined by its title string s.
		 */
		void removeCurve(const QString& s);
		/**
		 * \brief Removes all curves defined by the title/plot association string s.
		 */
		void removeCurves(const QString& s);

		void updateCurvesData(Table* w, const QString& yColName);
		void reloadCurvesData();

		int curveCount(){return d_curves.size();};
		bool validCurvesDataSize();
		double selectedXStartValue();
		double selectedXEndValue();

		//! Map curve pointer to index.
		int curveIndex(QwtPlotItem *c){return d_curves.indexOf(c);};
		//! map curve title to index
		int curveIndex(const QString &title){return plotItemsList().indexOf(title);}
		DataCurve* dataCurve(int index);
		FunctionCurve* functionCurve(int index);
		//! get curve by index
		PlotCurve* curve(int index);
		//! get curve by name
		PlotCurve* curve(const QString &title){return curve(curveIndex(title));};
		//! get curve title string by inde (convenience function for scripts)
		QString curveTitle(int index);
		//! returns the curve range information as a string: "curve_name [start:end]"
		QString curveRange(QwtPlotCurve *c);

		//! Returns the names of all the curves suitable for data analysis, as a string list. The list excludes error bars and spectrograms.
		QStringList analysableCurvesList();
		//! Returns the names of all the QwtPlotCurve items on the plot, as a string list
  		QStringList curveNamesList();
  	    //! Returns the names of all plot items, including spectrograms, as a string list
  		QStringList plotItemsList();
  		 //! get plotted item by index
  	    QwtPlotItem* plotItem(int index);

        void updateCurveNames(const QString& oldName, const QString& newName, bool updateTableName = true);

		//! \name Customizing plot curves
		//@{
		void setCurveStyle(int index, int s);
		void setCurveFullRange(int curveIndex);
		void setCurveLineColor(int curveIndex, int colorIndex);
		void setCurveLineColor(int curveIndex, QColor qColor);
		void setCurveLineStyle(int curveIndex, Qt::PenStyle style);
		void setCurveLineWidth(int curveIndex, double width);
		void setGrayScale();
		void setIndexedColors();
		//@}

		//! \name Output: Copy/Export/Print
		//@{
		void print();
		void copyImage();
		QPixmap graphPixmap(const QSize& size = QSize(), double scaleFontsFactor = 1.0, bool transparent = false);
		//! Provided for convenience in scripts
		void exportToFile(const QString& fileName);
		void exportSVG(const QString& fname, const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
		void exportEMF(const QString& fname, const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
		void exportTeX(const QString& fname, bool color = true, bool escapeStrings = true, bool fontSizes = true,
						const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
#ifdef TEX_OUTPUT
		bool isExportingTeX(){return d_is_exporting_tex;};
		void setTeXExportingMode(bool on = true){d_is_exporting_tex = on;};
		bool escapeTeXStrings(){return d_tex_escape_strings;};
		void setEscapeTeXStringsMode(bool on = true){d_tex_escape_strings = on;};
#endif
		void exportVector(const QString& fileName, int res = 0, bool color = true,
						const QSizeF& customSize = QSizeF (), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
		void exportVector(QPrinter *printer, int res = 0, bool color = true,
						  const QSizeF& customSize = QSizeF (), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
		void exportImage(const QString& fileName, int quality = 100, bool transparent = false,
						 int dpi = 0, const QSizeF& customSize = QSizeF (),
						 int unit = FrameWidget::Pixel, double fontsFactor = 1.0, int compression = 0);

		void draw(QPaintDevice *, const QSize& size, double fontsFactor = 1.0);
		static QSize customPrintSize(const QSizeF& customSize, int unit, int dpi);
		//@}

		void updatePlot();

		//! \name Error Bars
		//@{
		ErrorBarsCurve* addErrorBars(const QString& xColName, const QString& yColName, Table *errTable,
				const QString& errColName, int type = 1, double width = 1, int cap = 8, const QColor& color = QColor(Qt::black),
				bool through = true, bool minus = true, bool plus = true);

		ErrorBarsCurve* addErrorBars(const QString& yColName, Table *errTable, const QString& errColName,
				int type = 1, double width = 1, int cap = 8, const QColor& color = QColor(Qt::black),
				bool through = true, bool minus = true, bool plus = true);

		ErrorBarsCurve* addErrorBars(DataCurve *c, Table *errTable, const QString& errColName,
				int type = 1, double width = 1, int cap = 8, const QColor& color = QColor(Qt::black),
				bool through = true, bool minus = true, bool plus = true);

		void updateErrorBars(ErrorBarsCurve *er, bool xErr, double width, int cap, const QColor& c, bool plus, bool minus, bool through);
		//! Used when restoring project files
		void loadErrorBars(QList<ErrorBarsCurve *> errBars, QList<int> mcIndexes);

		//! Returns a valid master curve for the error bars curve.
		DataCurve* masterCurve(ErrorBarsCurve *er);
		//! Returns a valid master curve for a plot association.
		DataCurve* masterCurve(const QString& xColName, const QString& yColName);
		//@}

		//! \name Event Handlers
		//@{
		bool mousePressed(QEvent *);
		void contextMenuEvent(QContextMenuEvent *);
		void closeEvent(QCloseEvent *e);
		bool focusNextPrevChild ( bool next );
		//@}

		//! Set axis scale
		void invertScale(int axis);
		void setScale(int axis, double start, double end, double step = 0.0,
				int majorTicks = 5, int minorTicks = 5, int type = 0, bool inverted = false,
				double left_break = -DBL_MAX, double right_break = DBL_MAX, int pos = 50,
				double stepBeforeBreak = 0.0, double stepAfterBreak = 0.0, int minTicksBeforeBreak = 4,
				int minTicksAfterBreak = 4, bool log10AfterBreak = false, int breakWidth = 4, bool breakDecoration = true);
		double axisStep(int axis){return d_user_step[axis];};
		void setAxisStep(int axis, double step){d_user_step[axis] = step;};
		void setCanvasCoordinates(const QRectF&);

		//! \name Curves Layout
		//@{
		CurveLayout initCurveLayout(int style, int curves = 0, bool guessLayout = true);
		static CurveLayout initCurveLayout();
		void updateCurveLayout(PlotCurve* c, const CurveLayout *cL);
		//! Tries to guess not already used curve color and symbol style
		void guessUniqueCurveLayout(int& colorIndex, int& symbolIndex);
		//@}

		//! \name Zoom
		//@{
		void zoomed (const QwtDoubleRect &);
		void zoom(bool on);
		void zoomOut();
		bool zoomOn();
		//@}

		void setAutoScale();
		void updateScale();

		//! \name Saving/Restoring to/from file
		//@{
		QString saveToString(bool saveAsTemplate = false);
		QString saveScale();
		QString saveScaleTitles();
		QString saveFonts();
		QString saveMarkers();
		QString saveCurveLayout(int index);
		QString saveAxesTitleColors();
		QString saveAxesColors();
		QString saveEnabledAxes();
		QString saveCanvas();
		QString saveTitle();
		QString saveAxesTitleAlignement();
		QString saveEnabledTickLabels();
		QString saveTicksType();
		QString saveCurves();
		QString saveLabelsFormat();
		QString saveLabelsRotation();
		QString saveAxesLabelsType();
		QString saveAxesBaseline();
		QString saveAxesFormulas();
		QString saveAxesBackbones();
		QString saveTickLabelsSpace();
		QString saveLabelsPrefixAndSuffix();
		QString saveBackgroundImage();
		void restoreBackgroundImage(const QStringList& lst);
		void restoreSymbolImage(int index, const QStringList& lst);
		static QString rgbaName(const QColor& color);
		//@}

		//! \name Texts
		//@{
		LegendWidget* addText(LegendWidget*);
		//! Used when opening a project file
		LegendWidget* insertText(const QStringList& list, int fileVersion);

		LegendWidget* addTimeStamp();
		void removeLegendItem(int index);
		void insertLegend(const QStringList& lst, int fileVersion);

		LegendWidget* newLegend(const QString& text = QString());
		//! Creates a new legend text using the curves titles
		QString legendText(bool layerSpec = false, int fromIndex = 0);
		//@}

		//! \name Obsolete functions provided to keep existing Python scripts working
		//@{
		LegendWidget* legend();
		void setLegend(const QString&);
		void removeLegend();
		//@}

		//! \name Line Markers
		//@{
		ArrowMarker* addArrow(ArrowMarker* mrk);
		void remove(ArrowMarker* arrow);

		//! Used when opening a project file
		void addArrow(QStringList list, int fileVersion);
		QList<ArrowMarker *> arrowsList(){return d_lines;};
		int numArrows(){return d_lines.count();};

		//!Draws a line/arrow depending on the value of "arrow"
		void drawLine(bool on, bool arrow = false);
		bool drawArrow(){return drawArrowOn;};
		bool drawLineActive(){return drawLineOn;};
        bool arrowMarkerSelected();
		//@}

		//! \name Image Markers
		//@{
		ImageWidget* addImage(ImageWidget* i);
		ImageWidget* addImage(const QString& fileName);
		ImageWidget* addImage(const QImage& image);

		void insertImageMarker(const QStringList& lst, int fileVersion);
		bool imageMarkerSelected();
		//@}

		//! \name Common to all Markers
		//@{
		void removeMarker();
		//! Keep the markers on screen each time the scales are modified by adding/removing curves
		void updateMarkersBoundingRect(bool rescaleEvent = true);

		/*!\brief Set the selected arrow.
		 * \param mrk key of the arrow to be selected.
		 * \param add whether the arrow is to be added to an existing selection.
		 * If <i>add</i> is false (the default) or there is no existing selection, a new SelectionMoveResizer is
		 * created and stored in #d_markers_selector.
		 */
		void setSelectedArrow(ArrowMarker* mrk, bool add = false);
		ArrowMarker* selectedArrow(){return d_selected_arrow;};
		bool markerSelected();
		//! Reset any selection states on markers.
		void deselectMarker();
		//@}

		//! \name Axes
		//@{
		QwtScaleWidget* currentScale();
		QwtScaleWidget* selectedScale();
		QRect axisTitleRect(const QwtScaleWidget *scale);
		bool axisTitleSelected();

		ScaleDraw::ScaleType axisType(int axis);

		void setXAxisTitle(const QString& text);
		void setYAxisTitle(const QString& text);
		void setRightAxisTitle(const QString& text);
		void setTopAxisTitle(const QString& text);

		QString axisTitleString(int axis);
		void setAxisTitleString(int axis, const QString& text);
		void setAxisTitle(int axis, const QString& text);
		void updateAxesTitles();
		void updateAxisTitle(int axis);
		//! TODO: eliminate this function in version 0.9.1 (used only when restoring project files)
		void setScaleTitle(int axis, const QString& text);

		QFont axisTitleFont(int axis);
		void setAxisTitleFont(int axis,const QFont &fnt);

		void setAxisFont(int axis, const QFont &fnt);
		void initFonts(const QFont &scaleTitleFnt,const QFont &numbersFnt);

		QColor axisTitleColor(int axis);
		void setAxisTitleColor(int axis, const QColor& c);

		int axisTitleAlignment (int axis);
		void setAxisTitleAlignment(int axis, int align);

		int axisTitleDistance(int axis);
		void setAxisTitleDistance(int axis, int dist);

        QColor axisColor(int axis);
		void setAxisColor(int axis, const QColor& color);

        QColor axisLabelsColor(int axis);
		void setAxisLabelsColor(int axis, const QColor& color);

		void showAxis(int axis, int type, const QString& formatInfo, Table *table, bool axisOn,
				int majTicksType, int minTicksType, bool labelsOn, const QColor& c, int format,
				int prec, int rotation, int baselineDist, const QString& formula, const QColor& labelsColor,
				int spacing = 4, bool backbone = true, const ScaleDraw::ShowTicksPolicy& showTicks = ScaleDraw::ShowAll,
				const QString& prefix = QString::null, const QString& suffix = QString::null);

		void enableAxis(int axis, bool on = true);
		void enableAxisLabels(int axis, bool on = true);

		int labelsRotation(int axis);
		void setAxisLabelRotation(int axis, int rotation);

		//! used when opening a project file
		void loadAxesLinewidth(int width);

		void drawAxesBackbones(bool yes);
		bool axesBackbones(){return drawAxesBackbone;};
		//! used when opening a project file
		void loadAxesOptions(const QStringList& lst);

		void setAxisMargin(int axis, int margin);

		void setMajorTicksType(const QList<int>& lst);
		void setMajorTicksType(const QStringList& lst);

		void setMinorTicksType(const QList<int>& lst);
		void setMinorTicksType(const QStringList& lst);

		void setAxisTicksLength(int axis, int majTicksType, int minTicksType, int minLength, int majLength);
		void setTicksLength(int minLength, int majLength);
		void changeTicksLength(int minLength, int majLength);
        //! Used for restoring project files
		void setLabelsNumericFormat(const QStringList& l);
		void setLabelsNumericFormat(int axis, int format, int prec = 6, const QString& formula = QString());
		void setLabelsDateTimeFormat(int axis, int type, const QString& formatInfo);
		void setLabelsDayFormat(int axis, int format);
		void setLabelsMonthFormat(int axis, int format);
		void recoverObsoleteDateTimeScale(int axis, int type, const QString& origin, const QString& format);

		QString axisFormatInfo(int axis);

		void setLabelsTextFormat(int axis, int type, const QString& name, const QStringList& lst);
		void setLabelsTextFormat(int axis, int type, const QString& labelsColName, Table *table);

		QString axisFormula(int axis);
		void setAxisFormula(int axis, const QString &);
		//@}

		//! \name Canvas Frame
		//@{
		void setCanvasFrame(int width = 1, const QColor& color =  QColor(Qt::black));
		QColor canvasFrameColor();
		int canvasFrameWidth();
		//@}

		//! \name Canvas Image Background
		//@{
		QString canvasBackgroundFileName(){return d_canvas_bkg_path;};
		void setCanvasBackgroundImage (const QString & fn = QString(), bool update = true);
		QPixmap backgroundPixmap(){return d_canvas_bkg_pix;};
		//@}

		//! \name Plot Title
		//@{
		void setTitleFont(const QFont &fnt);
		void setTitleColor(const QColor &c);
		void setTitleAlignment(int align);

		bool titleSelected();
		void selectTitle(bool select = true);
		//! Sets the title to an "almost empty" string: " ", thus keeping the QwtText object visible
		void clearTitle();
		//! Sets title to an empty string and hides the text label
		void removeTitle();
		void initTitle( bool on, const QFont& fnt);
		//@}

		void disableTools();
		void disableImageProfilesTool();
		QPointer<ImageProfilesTool> imageProfilesTool(){return d_image_profiles_tool;}

		/*! Enables the data range selector tool.
		 *
		 * This one is a bit special, because other tools can depend upon an existing selection.
		 * Therefore, range selection (like zooming) has to be provided in addition to the generic
		 * tool interface.
		 */
		bool enableRangeSelectors(const QObject *status_target=NULL, const char *status_slot="");
		bool rangeSelectorsEnabled();
		QPointer<RangeSelectorTool> rangeSelectorTool(){return d_range_selector;};
		//! \name Border and background
		//@{
		void setFrame(int width = 1, const QColor& color = Qt::black);
		void setBackgroundColor(const QColor& color);
		//@}

		void addFitCurve(QwtPlotCurve *c);
		void deleteFitCurves();
		QList<QwtPlotCurve *> fitCurvesList(){return d_fit_curves;};
		/*! Set start and end to selected X range of curve "curveTitle" or, if there's no selection, to the curve's total range.
		 *
		 * \return the number of selected or total points
		 */
		int range(const QString& curveTitle, double *start, double *end);
		/*! Set start and end to selected X range of curve "curve" or, if there's no selection, to the curve's total range.
		 *
		 * \return the number of selected or total points
		 */
		int range(QwtPlotCurve *c, double *start, double *end);

		//!  Used for VerticalBars, HorizontalBars and Histograms
		void setBarsGap(int curve, int gapPercent, int offset);

		//! \name User-defined Functions
		//@{
		void modifyFunctionCurve(int curve, int type, const QStringList &formulas, const QString &var,
			double start, double end, int points, const QMap<QString, double>& constants);
		FunctionCurve* addFunction(const QStringList &formulas, double start, double end, int points = 100, const QString &var = "x", int type = 0, const QString& title = QString::null);
		//! Used when reading from a project file with version < 0.9.5.
		FunctionCurve* insertFunctionCurve(const QString& formula, int points, int fileVersion);

		//! Returns an unique function name
        QString generateFunctionName(const QString& name = tr("F"));
		//@}

        //! Provided for convenience in scripts.
		void createTable(const QString& curveName);
        void createTable(const QwtPlotCurve* curve);
		void activateGraph();

		//! \name Vector Curves
		//@{
		VectorCurve* plotVectors(Table* w, const QStringList& colList, int style, int startRow = 0, int endRow = -1);
		void updateVectorsLayout(int curve, const QColor& color, double width, int arrowLength, int arrowAngle, bool filled, int position,
				const QString& xEndColName = QString(), const QString& yEndColName = QString());
		//@}

		//! \name Box Plots
		//@{
		BoxCurve* openBoxDiagram(Table *w, const QStringList& l, int fileVersion);
		void plotBox(Table *w, const QStringList& names, int startRow = 0, int endRow = -1);
		BoxCurve * boxCurve(int index);
		//@}

		//! \name Resizing
		//@{
		void resizeEvent(QResizeEvent *e);
		void scaleFonts(double factor);
		//@}

		void notifyChanges();

		void updateSecondaryAxis(int axis, bool changeFormat = false);
		int oppositeAxis(int axis);
		void updateOppositeScaleDiv(int axis);

		bool isAutoscalingEnabled(){return d_auto_scale;};
		void enableAutoscaling(bool on = true){d_auto_scale = on;};

		bool autoscaleFonts(){return autoScaleFonts;};
		void setAutoscaleFonts(bool on = true){autoScaleFonts = on;};

		static int obsoleteSymbolStyle(int type);
		static QString penStyleName(Qt::PenStyle style);
		static Qt::PenStyle getPenStyle(const QString& s);
		static Qt::PenStyle getPenStyle(int style);
		static void showPlotErrorMessage(QWidget *parent, const QStringList& emptyColumns);

		void showTitleContextMenu();
		void copyTitle();
		void cutTitle();

		void clearAxisTitle();
		void removeAxisTitle();
		void cutAxisTitle();
		void copyAxisTitle();
		void showAxisTitleMenu();
		void showAxisContextMenu(int axis);
		void hideSelectedAxis();
		void showGrids();

		//! Convenience function enabling the grid for QwtScaleDraw::Left and Bottom Scales
		void showGrid();
		//! Convenience function enabling the grid for a user defined axis
		void showGrid(int axis);
		void setGridOnTop(bool on = true, bool update = true);
		bool hasGridOnTop(){return d_grid_on_top;}

		void showAxisDialog();
		void showScaleDialog();

		//! Returns a pointer to the spectrogram which data source is matrix m (the pointer can be NULL)
		Spectrogram* spectrogram(Matrix *m);
		//! Add a spectrogram to the graph
  		Spectrogram* plotSpectrogram(Matrix *m, CurveType type);
		//! Restores a spectrogram. Used when opening a project file.
  		void restoreSpectrogram(ApplicationWindow *app, const QStringList& lst);
        //! Add a matrix histogram  to the graph
        QwtHistogram* addHistogram(Matrix *m);
        //! Restores a histogram from a project file.
        QwtHistogram* restoreHistogram(Matrix *m, const QStringList& l);

		bool antialiasing(){return d_antialiasing;};
		//! Enables/Disables antialiasing of plot items.
		void setAntialiasing(bool on = true, bool update = true);

		void disableCurveAntialiasing(bool disable, int maxPoints);
		bool isCurveAntialiasingEnabled(QwtPlotItem *it);
		bool isCurveAntialiasingDisabled(){return d_disable_curve_antialiasing;};
		int maxAntialisingSize(){return d_max_antialising_size;};

		void setCurrentColor(const QColor& c);
		void notifyColorChange(const QColor& c){emit currentColorChanged(c);};
		void setCurrentFont(const QFont& f);
		void notifyFontChange(const QFont& f){emit currentFontChanged(f);};
        void enableTextEditor();

		void showMissingDataGap(bool on = true, bool update = true);
		bool isMissingDataGapEnabled(){return d_missing_data_gap;}

		//! \name Waterfall
		//@{
		bool isWaterfallPlot(){return d_waterfall_offset_x != 0 || d_waterfall_offset_y != 0;};
		int waterfallXOffset(){return d_waterfall_offset_x;};
		int waterfallYOffset(){return d_waterfall_offset_y;};
		void setWaterfallOffset(int x, int y, bool update = false);
		void setWaterfallXOffset(int);
		void setWaterfallYOffset(int);
		void setWaterfallSideLines(bool on = true);
		void setWaterfallFillColor(const QColor&);
		void updateWaterfallFill(bool on);
		//@}
		void updateDataCurves();
		void reverseCurveOrder();

signals:
		void selectedGraph(Graph*);
		void selectedCanvas(Graph*);
		void closedGraph();
		void drawLineEnded(bool);
		void cursorInfo(const QString&);
		void showPlotDialog(int);

		void viewLineDialog();
		void viewTitleDialog();
		void modifiedGraph();
		void hiddenPlot(QWidget*);

		void showContextMenu();
		void showCurveContextMenu(QwtPlotItem *);
		void showMarkerPopupMenu();

		void showAxisDialog(int);
		void axisDblClicked(int);

		void showAxisTitleDialog();

		void dataRangeChanged();
		void showFitResults(const QString&);
		void currentFontChanged(const QFont&);
		void currentColorChanged(const QColor&);
        void enableTextEditor(Graph *);
        void axisDivChanged(Graph *, int);
		void updatedLayout(Graph *);
		void selectionChanged(SelectionMoveResizer *);

	private slots:
		void selectorDeleted();

	private:
		QString parseAxisTitle(int axis);
		QList<FrameWidget*> stackingOrderEnrichmentsList() const;
		//! Finds bounding interval of the plot data.
		QwtDoubleInterval axisBoundingInterval(int axis);
		void deselectCurves();

		void dropEvent(QDropEvent*);
		void dragEnterEvent(QDragEnterEvent*);
		void showEvent (QShowEvent * event);
    	void printFrame(QPainter *painter, const QRect &rect) const;
		void printCanvas(QPainter *painter, const QRect &canvasRect,
   			 const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const;
		virtual void printScale (QPainter *, int axisId, int startDist, int endDist,
			int baseDist, const QRect &) const;
		virtual void drawItems (QPainter *painter, const QRect &rect,
			const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const;

		void drawInwardTicks(QPainter *painter, const QRect &rect,
							const QwtScaleMap&map, int axis, bool min, bool maj) const;
   	 	void drawBreak(QPainter *painter, const QRect &rect, const QwtScaleMap &map, int axis) const;

		QwtPlotZoomer *d_zoomer[2];
		TitlePicker *titlePicker;
		ScalePicker *scalePicker;
		CanvasPicker* cp;
		//! Pointer to the grid
		Grid *d_grid;
		//! List storing pointers to the curves on the plot.
		QList<QwtPlotItem*> d_curves;
		//! List storing pointers to the curves resulting after a fit session, in case the user wants to delete them later on.
		QList<QwtPlotCurve *>d_fit_curves;
		//! Render hint for plot items.
		bool d_antialiasing;
		bool d_disable_curve_antialiasing;
		int d_max_antialising_size;
		bool autoScaleFonts;
		bool drawLineOn, drawArrowOn, drawAxesBackbone;
		//! Flag telling if we are performing a print operation
		bool d_is_printing;
		//! Flag telling if the grid should be drawn on top of data
		bool d_grid_on_top;
		//! Flag telling if the curves line should be connected across missing data
		bool d_missing_data_gap;
		//! Stores the step the user specified for the four scale. If step = 0.0, the step will be calculated automatically by the Qwt scale engine.
		QVector<double> d_user_step;
		//! Arrows/lines on plot
		QList<ArrowMarker*> d_lines;
		//! Pointer to the currently selected line/image
		ArrowMarker* d_selected_arrow;
		//! The markers selected for move/resize operations or NULL if none are selected.
		QPointer<SelectionMoveResizer> d_markers_selector;
		//! The current curve selection, or NULL if none is active.
		QPointer<RangeSelectorTool> d_range_selector;
		QPointer<ImageProfilesTool> d_image_profiles_tool;
		//! The currently active tool, or NULL for default (pointer).
		PlotToolInterface *d_active_tool, *d_peak_fit_tool;
		//! Pointer to the currently selected text/legend
		FrameWidget *d_active_enrichment;
        //! Flag indicating if the axes limits should be changed in order to show all data each time a curva data change occurs
		bool d_auto_scale;
		//! Axes tick lengths
		int d_min_tick_length, d_maj_tick_length;
#ifdef TEX_OUTPUT
		bool d_is_exporting_tex;
		bool d_tex_escape_strings;
#endif
		QList<FrameWidget*> d_enrichments;
		QwtPlotMagnifier *d_magnifier;
		QwtPlotPanner *d_panner;

		double d_Douglas_Peuker_tolerance;
		int d_speed_mode_points;
		AxisTitlePolicy d_axis_title_policy;
		bool d_synchronize_scales;
		QStringList d_axis_titles;

		QString d_canvas_bkg_path;
		QPixmap d_canvas_bkg_pix;

		int d_waterfall_offset_x, d_waterfall_offset_y;
		QRectF d_page_rect;
};

class ScaledFontsPrintFilter: public QwtPlotPrintFilter
{

public:
	ScaledFontsPrintFilter(double factor){d_factor = factor;};
	virtual QFont font(const QFont &f, Item item) const
	{
		if (d_factor == 1.0 || d_factor <= 0.0)
			return f;

		if (item == Title || item == AxisScale || item == AxisTitle || item == Marker){
			QFont fnt(f);
			fnt.setPointSizeF(d_factor*f.pointSizeF());
			return fnt;
		}
		return f;
	}

	double scaleFontsFactor(){return d_factor;}

private:
	double d_factor;
};
#endif // GRAPH_H
