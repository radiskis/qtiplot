/***************************************************************************
    File                 : Graph3D.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004-2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 3D graph widget

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
#ifndef GRAPH3D_H
#define GRAPH3D_H

#include <qwt3d_curve.h>
#include <qwt3d_surfaceplot.h>
#include <qwt3d_function.h>
#include <qwt3d_parametricsurface.h>

#include <QTimer>
#include <QVector>
#include <QEvent>

#include <Table.h>
#include <Matrix.h>
#include <FrameWidget.h>

using namespace Qwt3D;

class QTextDocument;
class UserFunction;
class UserParametricSurface;
class ConstFunction;

/*!\brief 3D graph widget.
 *
 * This provides 3D plotting using Qwt3D.
 *
 * \section future Future Plans
 * If MultiLayer is extended to accept any QWidget, Graph3D wouldn't have to inherit from MdiSubWindow any more.
 * It could also make sense to unify the interface with other plot types; see documentation of Graph.
 * Big problem here: export to vector formats. Qwt3D's export filters write directly to a file, so they
 * can't be combined with output generated via QPrinter.
 */
class Graph3D: public MdiSubWindow
{
	Q_OBJECT

public:
	Graph3D (const QString& label, ApplicationWindow* parent, const char* name=0, Qt::WFlags f=0);
	~Graph3D();

	enum PlotType{NoTable = -1, Scatter = 0, Trajectory = 1, Bars = 2, Ribbon =  3};
	enum PointStyle{None = 0, Dots = 1, VerticalBars = 2, HairCross = 3, Cones = 4};
	enum AxisNumericFormat{Default = 0, Decimal = 1, Scientific = 2, Engineering = 3};

	static Graph3D* restore(ApplicationWindow* app, const QStringList &lst, int fileVersion);

	Qwt3D::Plot3D* surface(){return sp;};

	bool scaleOnPrint(){return d_scale_on_print;};
	void setScaleOnPrint(bool on){d_scale_on_print = on;};

	bool printCropmarksEnabled(){return d_print_cropmarks;};
	void printCropmarks(bool on){d_print_cropmarks = on;};

public slots:
	void copy(Graph3D* g);
	void initPlot();
	void initCoord();
	void addFunction(const QString& s, double xl, double xr, double yl,
					double yr, double zl, double zr, int columns = 40, int rows = 30);
	void addParametricSurface(const QString& xFormula, const QString& yFormula,
						const QString& zFormula, double ul, double ur, double vl, double vr,
						int columns, int rows, bool uPeriodic, bool vPeriodic);
	void insertNewData(Table* table, const QString& colName);

	Matrix * matrix(){return d_matrix;};
	void addMatrixData(Matrix* m);//used to plot matrixes
	void addMatrixData(Matrix* m,double xl,double xr,double yl,double yr,double zl,double zr);
	void updateMatrixData(Matrix* m);

	void addRibbon(Table* table,const QString& xColName,const QString& yColName);
	void addRibbon(Table* table,const QString& xColName,const QString& yColName,
                double xl, double xr, double yl, double yr, double zl, double zr);
	void addData(Table* table, int xCol, int yCol, int zCol, int type = 0);
	void loadData(Table* table, int xCol, int yCol, int zCol,
				double xl=0.0, double xr=0.0, double yl=0.0, double yr=0.0, double zl=0.0, double zr=0.0, int axis = -1);

    PlotType tablePlotType(){return d_table_plot_type;};

	void clearData();
	bool hasData(){return sp->hasData();};

	void updateData(Table* table);
	void updateDataXY(Table* table, int xCol, int yCol);

	void changeDataColumn(Table* table, const QString& colName, int type = 0);

	//! \name User Functions
	//@{
	UserParametricSurface *parametricSurface(){return d_surface;};
	//@}

	//! \name User Functions
	//@{
	UserFunction* userFunction(){return d_func;};
	QString formula();
	//@}

	//! \name Event Handlers
	//@{
	void dropEvent(QDropEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	bool eventFilter(QObject *object, QEvent *e);
	void resizeEvent (QResizeEvent *);
	void scaleFonts(double factor);
	void setIgnoreFonts(bool ok){ignoreFonts = ok;};
	//@}

	//! \name Axes
	//@{
	void setFramed();
	void setBoxed();
	void setNoAxes();
	bool isOrthogonal(){return sp->ortho();};
	void setOrthogonal(bool on = true){sp->setOrtho(on);};

	QStringList axesLabels(){return labels;};
	void setAxesLabels(const QStringList& lst);
	void resetAxesLabels();

    void setXAxisLabel(const QString&);
    void setYAxisLabel(const QString&);
	void setZAxisLabel(const QString&);

	QFont xAxisLabelFont();
	QFont yAxisLabelFont();
	QFont zAxisLabelFont();

	void setXAxisLabelFont(const QFont& fnt);
	void setYAxisLabelFont(const QFont& fnt);
	void setZAxisLabelFont(const QFont& fnt);

	void setXAxisLabelFont(const QStringList& lst);
	void setYAxisLabelFont(const QStringList& lst);
	void setZAxisLabelFont(const QStringList& lst);

	QFont numbersFont();
	void setNumbersFont(const QFont& font);

	double xStart();
	double xStop();
	double yStart();
	double yStop();
	double zStart();
	double zStop();

	int axisType(int axis){return scaleType[axis];};
	void setAxisType(int axis, int type);

	int axisNumericFormat(int axis);
	int axisNumericPrecision(int axis);
	void setAxisNumericFormat(int axis, int format, int precision);

	void setScales(double xl, double xr, double yl, double yr, double zl, double zr, int axis = -1);
	void updateScales(double xl, double xr, double yl, double yr,
				  		double zl, double zr, int xcol, int ycol);
	void updateScalesFromMatrix(double xl,double xr,double yl,double yr,double zl,double zr);

	QStringList scaleTicks();
	void setTicks(const QStringList& options);

	void setXAxisTickLength(double majorLength, double minorLength);
    void setYAxisTickLength(double majorLength, double minorLength);
	void setZAxisTickLength(double majorLength, double minorLength);

	void setAxisTickLength(int axis, double majorLength, double minorLength);
	void setLabelsDistance(int val);
	int labelsDistance(){return labelsDist;};

	QStringList axisTickLengths();
	void setTickLengths(const QStringList& lst);
	//@}

	//! \name Mesh
	//@{
	void setPolygonStyle();
	void setHiddenLineStyle();
	void setWireframeStyle();
	void setFilledMeshStyle();
	void setDotStyle();
	void setBarStyle();
	void setFloorData();
	void setFloorIsolines();
	void setEmptyFloor();

	void setMeshLineWidth(double lw);
	double meshLineWidth(){if (d_active_curve) return d_active_curve->meshLineWidth(); return 0.0;};
	//@}

	//! \name Grid
	//@{
	int grids();
	void setGrid(int s, bool b);
	void setGrid(int grids);

	void setLeftGrid(bool b = true);
	void setRightGrid(bool b = true);
	void setCeilGrid(bool b = true);
	void setFloorGrid(bool b = true);
	void setFrontGrid(bool b = true);
	void setBackGrid(bool b = true);
	//@}

	void setStyle(const QStringList& st);
	void customPlotStyle(int style);
	void resetNonEmptyStyle();

	void setRotation(double  xVal,double  yVal,double  zVal);
	void setScale(double  xVal,double  yVal,double  zVal);
	void setShift(double  xVal,double  yVal,double  zVal);

	double xRotation(){return sp->xRotation();};
	double yRotation(){return sp->yRotation();};
	double zRotation(){return sp->zRotation();};

	double xScale(){return sp->xScale();};
	double yScale(){return sp->yScale();};
	double zScale(){return sp->zScale();};

	double xShift(){return sp->xShift();};
	double yShift(){return sp->yShift();};
	double zShift(){return sp->zShift();};

	double zoom(){return sp->zoom();};
	void setZoom(double val);

	Qwt3D::PLOTSTYLE plotStyle();
	Qwt3D::FLOORSTYLE floorStyle();
	Qwt3D::COORDSTYLE coordStyle();

	void print();
	void print(QPrinter *printer);
	void copyImage();
#if QT_VERSION >= 0x040500
	void exportImage(QTextDocument *document, int quality, bool transparent,
						int dpi, const QSizeF& customSize, int unit, double fontsFactor);
#endif
	QPixmap pixmap(int dpi = 0, const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
	void exportImage(const QString& fileName, int quality = 100, bool transparent = false, int dpi = 0,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0, int compression = 0);
    void exportPDF(const QString& fileName);
    void exportVector(const QString& fileName, int textExportMode = 0, int sortMode = 1,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
    void exportToFile(const QString& fileName);

	void save(const QString& fn, const QString& geometry, bool = false);

	void zoomChanged(double);
	void rotationChanged(double, double, double);
	void scaleChanged(double, double, double);
	void shiftChanged(double, double, double);

	//! \name Colors
	//@{
	void setDataColors(const QColor& cMin, const QColor& cMax){setDataColorMap(LinearColorMap(cMin, cMax));};
	void setDataColorMap(const LinearColorMap& colorMap);
	void setDataColorMap(const QString& fileName);
	void setDataColorMap(const ColorVector& colors);
	void setDataColorMap(const ColorVector& colors, const LinearColorMap& colorMap);

	void changeTransparency(double t);
	void setTransparency(double t);
	double transparency(){return d_alpha;};

	QColor meshColor(){return meshCol;};
	QColor axesColor(){return axesCol;};
	QColor labelColor(){return labelsCol;};
	QColor numColor(){return numCol;};
	QColor bgColor(){return bgCol;};
	QColor gridColor(){return gridCol;};

	QString colorMapFile(){return d_color_map_file;};
	LinearColorMap colorMap(){return d_color_map;};
	LinearColorMap *colorMapPointer(){return &d_color_map;};

	static bool openColorMapFile(ColorVector& cv, QString fname);

	void setMeshColor(const QColor&);
	void setAxesColor(const QColor&);
	void setNumbersColor(const QColor&);
	void setLabelsColor(const QColor&);
	void setBackgroundColor(const QColor&);
	void setGridColor(const QColor&);
	//@}

	//! \name Title
	//@{
	QFont titleFont(){return titleFnt;};
	void setTitleFont(const QFont& font);
	QString plotTitle(){return title;};
	QColor titleColor(){return titleCol;};
	void setTitle(const QStringList& lst);
	void setTitle(const QString& s, const QColor& color = QColor(Qt::black), const QFont& font = QFont());
	//@}

	//! \name Resolution
	//@{
	void setResolution(int r);
	int resolution(){if (d_active_curve) return d_active_curve->resolution(); return 0;};
	//@}

	//! \name Legend
	//@{
	void showColorLegend(bool show = true);
	bool isLegendOn(){return legendOn;};
	//@}

	void setOptions(bool legend, int r, int dist);
	void setOptions(const QStringList& lst);
	void update();

	//! \name Bars
	//@{
	double barsRadius();
	void setBarRadius(double rad);
	bool barLines(){return d_bar_lines;};
	void setBarLines(bool lines = true);
	bool filledBars(){return d_filled_bars;};
	void setFilledBars(bool filled = true);
	//@}

	//! \name Scatter Plots
	//@{
	double pointsSize(){return d_point_size;};
	bool smoothPoints(){return d_smooth_points;};
	void setDotOptions(double size, bool smooth);

	bool smoothCrossHair(){return crossHairSmooth;};
	bool boxedCrossHair(){return crossHairBoxed;};
	double crossHairRadius(){return crossHairRad;};
	double crossHairLinewidth(){return crossHairLineWidth;};
	void setCrossOptions(double rad, double linewidth, bool smooth, bool boxed);
	void setCrossStyle();

	double coneRadius(){return conesRad;};
	int coneQuality(){return conesQuality;};
	void setConeOptions(double rad, int quality);
	void setConeStyle();

	PointStyle pointType(){return pointStyle;};
	//@}

	Table* table(){return d_table;};
	void showWorksheet();
	void setPlotAssociation(const QString& s){plotAssociation = s;};

	void setAntialiasing(bool smooth = true);
	bool antialiasing(){if (d_active_curve) return d_active_curve->smoothDataMesh(); return false;};

	//! Used for the animation: rotates the scene with 1/360 degrees
	void rotate();
	void animate(bool on = true);
	bool isAnimated(){return d_timer->isActive();};

    void findBestLayout();
	bool autoscale(){return d_autoscale;};
	//! Enables/Disables autoscaling using findBestLayout().
	void setAutoscale(bool on = true){d_autoscale = on;};

	Qwt3D::CoordinateSystem* coordinateSystem() {return sp->coordinates();};
	void setScale(int axis, double start, double end, int majorTicks, int minorTicks, Qwt3D::SCALETYPE type);

signals:
	void showOptionsDialog();
	void modified();

private:
	void addHiddenConstantCurve(double xl, double xr, double yl, double yr, double zl, double zr);
	void changeScales(double xl, double xr, double yl, double yr, double zl, double zr);

	Curve* addCurve();
	void removeCurve();

	void resetAxesType();
	//! Wait this many msecs before redraw 3D plot (used for animations)
  	int animation_redraw_wait;
	//! File name of the color map used for the data (if any)
  	QString d_color_map_file;
	LinearColorMap d_color_map;

	QTimer *d_timer;
	QString title, plotAssociation;
	QStringList labels;
	QFont titleFnt;
	bool legendOn, d_autoscale;
	bool d_scale_on_print, d_print_cropmarks;
	QVector<int> scaleType;
	QColor axesCol, labelsCol, titleCol, meshCol, bgCol, numCol, gridCol;
	int labelsDist, legendMajorTicks;
	bool ignoreFonts;
	Qwt3D::StandardColor* col_;
	//! Transparency
	double d_alpha;

	//! \name Bar options
	//@{
	double d_bars_rad;
	bool d_filled_bars, d_bar_lines;
	//@}

	double d_point_size, crossHairRad, crossHairLineWidth, conesRad;
	//! Draw 3D points with smoothed angles.
	bool d_smooth_points;
	bool crossHairSmooth, crossHairBoxed;
	int conesQuality;
	PointStyle pointStyle;
	Table *d_table;
	Matrix *d_matrix;
	Qwt3D::Plot3D* sp;
	UserFunction *d_func;
	UserParametricSurface *d_surface;
	Qwt3D::PLOTSTYLE style_;
	PlotType d_table_plot_type;
	Curve * d_active_curve;
	ConstFunction *d_const_func;
	Curve * d_const_curve;
};

//! Class for constant z surfaces
class ConstFunction : public Function
{
public:
	ConstFunction(Qwt3D::Curve *pw);
	double operator()(double x, double y);
};

//! Class for user defined surfaces
class UserFunction : public Function
{
public:
	UserFunction(const QString& s, Qwt3D::Curve *pw);

    double operator()(double x, double y);
	QString function(){return formula;};

	unsigned int rows(){return d_rows;};
	unsigned int columns(){return d_columns;};
	void setMesh (unsigned int columns, unsigned int rows);

private:
	  QString formula;
	  unsigned int d_rows, d_columns;
};

//! Class for user defined parametric surfaces
class UserParametricSurface : public ParametricSurface
{
public:
    UserParametricSurface(const QString& xFormula, const QString& yFormula,
						  const QString& zFormula, Qwt3D::Curve *pw);
    Triple operator()(double u, double v);

	unsigned int rows(){return d_rows;};
	unsigned int columns(){return d_columns;};
	void setMesh (unsigned int columns, unsigned int rows);

	bool uPeriodic(){return d_u_periodic;};
	bool vPeriodic(){return d_v_periodic;};
	void setPeriodic (bool u, bool v);

	double uStart(){return d_ul;};
	double uEnd(){return d_ur;};
	double vStart(){return d_vl;};
	double vEnd(){return d_vr;};
	void setDomain(double ul, double ur, double vl, double vr);

	QString xFormula(){return d_x_formula;};
	QString yFormula(){return d_y_formula;};
	QString zFormula(){return d_z_formula;};

private:
	QString d_x_formula, d_y_formula, d_z_formula;
	unsigned int d_rows, d_columns;
	bool d_u_periodic, d_v_periodic;
	double d_ul, d_ur, d_vl, d_vr;
};
#endif // Plot3D_H
