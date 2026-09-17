/***************************************************************************
    File                 : PlotDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Custom curves dialog

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
#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <MultiLayer.h>

class QCheckBox;
class QComboBox;
class QCompleter;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QWidget;
#include <QStringList>
class QGroupBox;
class QDoubleSpinBox;
class QRadioButton;
class QTreeWidget;
class QSlider;

class LayerItem;
class CurveTreeItem;
class ColorBox;
class PatternBox;
class ColorButton;
class MultiLayer;
class SymbolBox;
class ColorMapEditor;
class QwtPlotItem;
class DoubleSpinBox;
class PenStyleBox;
class Spectrogram;
class ErrorBarsCurve;
class BoxCurve;
class DataCurve;
class ContourLinesEditor;
class FunctionDialog;
class EnrichmentDialog;
class ApplicationWindow;

//! Custom plot/curves dialog
class PlotDialog : public QDialog
{
    Q_OBJECT

public:
    PlotDialog(bool showExtended, QWidget* parent = nullptr, Qt::WindowFlags fl = {} );
    ApplicationWindow *app() const;
    void initFonts(const QFont& titlefont, const QFont& axesfont, const QFont& numbersfont, const QFont& legendfont);
	void insertColumnsList(const QStringList& names){columnNames = names;};
	void setMultiLayer(MultiLayer *ml);
	void selectMultiLayerItem();

public slots:
	void showAll(bool all);
	void selectCurve(int index);

private slots:
	void showBoxStatistics();
	void showStatistics();
	void customVectorsPage(bool angleMag);
	void updateEndPointColumns(const QString& text);

	void fillBoxSymbols();
	void fillSymbols();
	bool acceptParams();
	void showWorksheet();
	void quit();

	int setPlotType(CurveTreeItem *item);
	void changePlotType(int plotType);
	void setActiveCurve(CurveTreeItem *item);

	void raiseCurve();
	void shiftCurveBy(int offset = 1);

	void insertTabs(int plot_type);
	void updateTabWindow(QTreeWidgetItem *currentItem, QTreeWidgetItem *previousItem);
	void showAreaColor(bool show);

	void removeSelectedObject();

	void chooseBackgroundImageFile(const QString& fn = QString());
	void resizeCanvasToFitImage();

	void chooseSymbolImageFile();

	void pickErrorBarsColor();
	void enableBoxApplyColor(int);

	void setAutomaticBinning(bool on = true);

	//box plots
	void setBoxType(int index);
	void setBoxRangeType(int index);
	void setWhiskersRange(int index);
	void enableLabelsPage();

	//spectrograms
  	void showDefaultContourLinesBox(bool show);
	void showColorMapEditor(bool show);

	//layer geometry
	void adjustLayerHeight(double width);
	void adjustLayerWidth(double height);
	void displayCoordinates(int unit, Graph *g = nullptr);
	//plot window geometry
	void displayPlotCoordinates(int unit);
	void adjustPlotWidth(double height);
	void adjustPlotHeight(double width);

	void setCanvasDefaultValues();

    void setActiveLayer(LayerItem *item);
    void updateTreeWidgetItem(QTreeWidgetItem *item);
	void updateVisibility();
	void updateVisibility(QTreeWidgetItem *item, int column);
    void updateBackgroundTransparency(int alpha);
    void updateCanvasTransparency(int alpha);
	void setTitlesFont();
	void setAxesLabelsFont();
	void setAxesNumbersFont();
	void setLegendsFont();
	void editCurve();
	void editCurveRange();
	void chooseLabelsFont();
	void applyLayerFormat();
	void applyCanvasFormat();
	void setLayerDefaultValues();
    void setEquidistantLevels();
    void showCustomPenColumn(bool on);
	void plotAssociationsDialogClosed();

private:
	void resizeLayerToFitImage(Graph *g);
	void applyCanvasFormatToLayer(Graph *g);
	void applyCanvasSize();

	void applyFormatToLayer(Graph *g);
	void applySymbolsFormatToCurve(QwtPlotCurve *c, bool fillColor = true, bool penColor = true);
	void applySymbolsFormatToLayer(Graph *g);
	void applySymbolsFormat(QwtPlotCurve *c);

	void applyLineFormatToLayer(Graph *g);
	void applyLineFormat(QwtPlotCurve *c);

	void applyErrorBarFormatToCurve(ErrorBarsCurve *err, bool color = true);
	void applyErrorBarFormatToLayer(Graph *g);
	void applyErrorBarFormat(ErrorBarsCurve *c);

	void applyBoxWhiskersFormatToCurve(BoxCurve *b);
	void applyBoxWhiskersFormatToLayer(Graph *g);
	void applyBoxWhiskersFormat(BoxCurve *c);

	void applyPercentileFormatToCurve(BoxCurve *b);
	void applyPercentileFormatToLayer(Graph *g);
	void applyPercentileFormat(BoxCurve *c);

	void applyLabelsFormatToItem(QwtPlotItem *);
	void applyLabelsFormatToLayer(Graph *);
	void applyLabelsFormat(QwtPlotItem *);

	void applyGapToLayer(Graph *g);
	void applyGap(Graph *g);

	void setLabelsFont(const QFont& font, Graph *, const QwtPlotItem *);
	void setLabelsFontToPlotItem(const QFont& font, const QwtPlotItem *);
	void setLabelsFontToLayer(const QFont& font, Graph *);

    int labelsAlignment() const;
	void closeEvent(QCloseEvent* e) override;

    void clearTabWidget();
	void initAxesPage();
	void initLinePage();
	void initSymbolsPage();
	void initHistogramPage();
	void initErrorsPage();
	void initSpacingPage();
	void initVectPage();
	void initBoxPage();
	void initPercentilePage();
	void initSpectrogramPage();
	void initSpectrogramValuesPage();
	void initContourLinesPage();
	void initLayerPage();
	void initCanvasPage();
	void initLayerGeometryPage();
	void initPlotGeometryPage();
	void initLayerDisplayPage();
	void initLayerSpeedPage();
	void initFontsPage();
	void initMiscPage();
	void initPiePage();
	void initPieGeometryPage();
	void initPieLabelsPage();
	void initPrintPage();
	void initLabelsPage();
	void initFunctionPage();
    void contextMenuEvent(QContextMenuEvent *e) override;
	void showAllLabelControls(bool show = true, int curveType = 0);
    void updateContourLevelsDisplay(Spectrogram *sp);
    QRect layerCanvasRect(QWidget *widget, double x, double y, double w, double h, FrameWidget::Unit unit) const;

	double aspect_ratio = 0.0, plot_aspect_ratio = 0.0;

    QFont titleFont, legendFont, axesFont, numbersFont;

    MultiLayer *d_ml = nullptr;
	QStringList columnNames;

	DoubleSpinBox* boxX = nullptr, *boxY = nullptr, *boxLayerWidth = nullptr, *boxLayerHeight = nullptr;
	QCheckBox *keepRatioBox = nullptr;

    QPushButton *btnTitle = nullptr, *btnAxesLabels = nullptr, *btnAxesNumbers = nullptr, *btnLegend = nullptr;
	ColorMapEditor *colorMapEditor = nullptr;
	QWidget *curvePlotTypeBox = nullptr, *layerPage = nullptr, *layerGeometryPage = nullptr, *piePage = nullptr, *fontsPage = nullptr, *printPage = nullptr;
	QWidget *layerDisplayPage = nullptr, *speedPage = nullptr, *functionPage = nullptr, *canvasPage = nullptr;
    QTreeWidget* listBox = nullptr;
	QCheckBox *boxAntialiasing = nullptr, *boxScaleLayers = nullptr, *boxPrintCrops = nullptr, *boxAutoscaling = nullptr, *boxSynchronizeScales = nullptr, *boxGridPosition = nullptr, *boxMissingData = nullptr;
    ColorButton *boxBorderColor = nullptr, *boxBackgroundColor = nullptr, *boxCanvasColor = nullptr;
	QSpinBox *boxBackgroundTransparency = nullptr, *boxCanvasTransparency = nullptr, *boxBorderWidth = nullptr, *boxMargin = nullptr;
	QSpinBox *boxRadius = nullptr;
	DoubleSpinBox *boxPieLineWidth = nullptr;
    ColorBox *boxFirstColor = nullptr;
    ColorButton *boxPieLineColor = nullptr;
    PatternBox *boxPiePattern = nullptr;
    PenStyleBox* boxPieLineStyle = nullptr;
	QLineEdit *imagePathBox = nullptr;
	QComboBox *imageApplyToBox = nullptr;
	QRadioButton *colorBtn = nullptr, *imageBtn = nullptr;
	QGroupBox *canvasColorBox = nullptr, *canvasImageBox = nullptr;

    QPushButton* buttonApply = nullptr, *btnWorksheet = nullptr;
    QPushButton* buttonOk = nullptr, *btnMore = nullptr;
    QPushButton* buttonCancel = nullptr;
    QComboBox* boxPlotType = nullptr;
    QWidget* linePage = nullptr;
    QComboBox* boxConnect = nullptr;
    PenStyleBox* boxLineStyle = nullptr;
    DoubleSpinBox *boxLineWidth = nullptr, *boxPenWidth = nullptr;
    ColorButton* boxLineColor = nullptr, *boxAreaColor = nullptr;
    QWidget* symbolPage = nullptr;
    QSpinBox* boxSymbolSize = nullptr;
    ColorButton *boxSymbolColor = nullptr, *boxFillColor = nullptr;
    SymbolBox* boxSymbolStyle = nullptr;
    PatternBox *boxPattern = nullptr;
	QTabWidget* privateTabWidget = nullptr;
	QWidget *errorsPage = nullptr, *spectrogramPage = nullptr, *contourLinesPage = nullptr;
	QGroupBox* fillGroupBox = nullptr;
    QCheckBox* plusBox = nullptr;
    QCheckBox* minusBox = nullptr;
    QCheckBox* xBox = nullptr;
    ColorButton *colorBox = nullptr, *levelsColorBox = nullptr, *vectColorBox = nullptr;
    DoubleSpinBox* widthBox = nullptr;
    QComboBox* capBox = nullptr;
    QCheckBox* throughBox = nullptr;
	QLabel *labelPosition = nullptr, *labelXEnd = nullptr, *labelYEnd = nullptr;
	QGroupBox* GroupBoxH = nullptr;
	QWidget *histogramPage = nullptr, *spacingPage = nullptr;
	DoubleSpinBox *binSizeBox = nullptr, *histogramBeginBox = nullptr, *histogramEndBox = nullptr;
	QCheckBox *automaticBox = nullptr;
	QPushButton* buttonStatistics = nullptr, *btnEditCurve = nullptr, *buttonBoxStatistics = nullptr, *btnEditCurveRange = nullptr;
	QSpinBox* gapBox = nullptr, *offsetBox = nullptr, *boxWidth = nullptr;
	QWidget *vectPage = nullptr, *boxPage = nullptr, *percentilePage = nullptr, *axesPage = nullptr;
	QComboBox *xEndBox = nullptr, *yEndBox = nullptr, *boxType = nullptr, *boxWhiskersType = nullptr, *boxWhiskersRange = nullptr, *boxRange = nullptr;
	QSpinBox* headAngleBox = nullptr, *headLengthBox = nullptr, *boxPercSize = nullptr;
	DoubleSpinBox *vectWidthBox = nullptr, *boxEdgeWidth = nullptr;
	QCheckBox *filledHeadBox = nullptr;
	QSpinBox *boxCoef = nullptr, *boxWhiskersCoef = nullptr;
	QCheckBox *boxFillSymbols = nullptr, *boxFillSymbol = nullptr;
	ColorButton *boxPercFillColor = nullptr, *boxEdgeColor = nullptr;
	QLabel 	*whiskerCoeffLabel = nullptr, *whiskerRangeLabel = nullptr, *boxCoeffLabel = nullptr;
	QLabel *boxRangeLabel = nullptr, *whiskerCntLabel = nullptr, *boxCntLabel = nullptr;
	QGroupBox *GroupBoxVectEnd = nullptr;
	QComboBox *vectPosBox = nullptr, *boxXAxis = nullptr, *boxYAxis = nullptr, *colorScaleBox = nullptr;
	PenStyleBox	*boxContourStyle = nullptr;
  	QSpinBox *levelsBox = nullptr, *colorScaleWidthBox = nullptr;
	DoubleSpinBox *contourWidthBox = nullptr;
  	QGroupBox *levelsGroupBox = nullptr, *axisScaleBox = nullptr, *imageGroupBox = nullptr;
  	QGroupBox *defaultPenBox = nullptr;
  	QRadioButton *defaultScaleBox = nullptr, *grayScaleBox = nullptr, *customScaleBox = nullptr, *defaultContourBox = nullptr, *autoContourBox = nullptr;

	QCheckBox *boxWhiskerLabels = nullptr, *boxBoxLabels = nullptr;
	QComboBox *boxWhiskersFormatApplyToBox = nullptr, *boxLabelsFormatApplyToBox = nullptr, *percentileFormatApplyToBox = nullptr;
    SymbolBox *boxMaxStyle = nullptr, *boxMinStyle = nullptr, *boxMeanStyle = nullptr, *box99Style = nullptr, *box1Style = nullptr;
    QDoubleSpinBox *whiskerCnt = nullptr, *boxCnt = nullptr;
    //!Labels page
    QGroupBox *labelsGroupBox = nullptr;
    DoubleSpinBox *boxLabelsAngle = nullptr;
    QSpinBox *boxLabelsXOffset = nullptr, *boxLabelsYOffset = nullptr;
    QCheckBox *boxLabelsWhiteOut = nullptr;
    QPushButton *btnLabelsFont = nullptr;
    QComboBox *boxLabelsAlign = nullptr, *boxLabelsColumn = nullptr;
    ColorButton* boxLabelsColor = nullptr;
    QWidget *labelsPage = nullptr;

    QGroupBox *pieAutoLabelsBox = nullptr, *boxPieWedge = nullptr;
	DoubleSpinBox *boxPieStartAzimuth = nullptr, *boxPieEdgeDist = nullptr, *boxPieViewAngle = nullptr, *boxPieThickness = nullptr;
	QCheckBox *boxPieConterClockwise = nullptr, *boxPieValues = nullptr, *boxPiePercentages = nullptr, *boxPieCategories = nullptr;
	QWidget *pieLabelsPage = nullptr;
    QSpinBox *boxPieOffset = nullptr;
	QWidget *pieGeometryPage = nullptr;

	QComboBox *unitBox = nullptr;
	QComboBox *backgroundApplyToBox = nullptr;
	QPushButton *layerDefaultBtn = nullptr;

	DoubleSpinBox *firstContourLineBox = nullptr, *contourLinesDistanceBox = nullptr;
	QLabel *justifyLabelsLbl = nullptr, *labelsColumnLbl = nullptr;

	QWidget *spectroValuesPage = nullptr;
	QComboBox *boxSpectroMatrix = nullptr;
	QCheckBox *boxUseMatrixFormula = nullptr;
	ContourLinesEditor *contourLinesEditor = nullptr;
	QPushButton *btnSetEquidistantLevels = nullptr;
	QRadioButton *customPenBtn = nullptr;

	QSpinBox *boxSkipSymbols = nullptr, *boxSkipErrorBars = nullptr;
	QComboBox *symbolsFormatApplyToBox = nullptr, *lineFormatApplyToBox = nullptr, *errorBarsFormatApplyToBox = nullptr, *sizeApplyToBox = nullptr;
	QSpinBox *boxMaxPoints = nullptr;
	DoubleSpinBox *boxDouglasPeukerTolerance = nullptr;
	QGroupBox *speedModeBox = nullptr;
	QCheckBox *boxApplyColorTo = nullptr;

	QWidget *miscPage = nullptr, *plotGeometryPage = nullptr;
	QCheckBox *boxLinkXAxes = nullptr;
	QComboBox *boxLinkAllXAxes = nullptr;

	FunctionDialog *functionEdit = nullptr;
	QSpinBox *boxCurveOpacity = nullptr, *boxSymbolTransparency = nullptr, *boxLineTransparency = nullptr, *boxPercentileTransparency = nullptr;
	QSlider *curveOpacitySlider = nullptr, *symbTransparencySlider = nullptr, *lineTransparencySlider = nullptr, *percentileTransparencySlider = nullptr;

	DoubleSpinBox *boxPlotX = nullptr, *boxPlotY = nullptr, *boxPlotWidth = nullptr, *boxPlotHeight = nullptr;
	QComboBox *plotUnitBox = nullptr, *gapApplyToBox = nullptr;
	QCheckBox *boxResizeLayers = nullptr, *keepPlotRatioBox = nullptr, *layerScaleFonts = nullptr;
	EnrichmentDialog *enrichmentDialog = nullptr;
	QPushButton *btnUp = nullptr, *btnDown = nullptr;
	QLabel *barsOffsetLabel = nullptr;

	QRadioButton *standardSymbolBtn = nullptr, *imageSymbolBtn = nullptr;
	QGroupBox *standardSymbolFormatBox = nullptr, *imageSymBolFormatBox = nullptr;
	QLineEdit *imageSymbolPathBox = nullptr;
	QLabel *symbolImageLabel = nullptr;

	QGroupBox *boxFramed = nullptr;
	ColorButton *boxFrameColor = nullptr;
	QSpinBox *boxFrameWidth = nullptr;
	QPushButton *canvasDefaultBtn = nullptr;
	QSlider *canvasOpacitySlider = nullptr, *bkgOpacitySlider = nullptr;

	QCheckBox *boxLeftAxis = nullptr, *boxRightAxis = nullptr, *boxBottomAxis = nullptr, *boxTopAxis = nullptr, *boxClipData = nullptr;
	QCompleter *completer = nullptr;
};

/*****************************************************************************
 *
 * Class LayerItem
 *
 *****************************************************************************/
//! LayerItem tree widget item class
class LayerItem : public QTreeWidgetItem
{
public:
    enum {LayerTreeItem = 1001};
    LayerItem(Graph *g, QTreeWidgetItem *parent, const QString& s);

    Graph *graph() const { return d_graph; };
    void setActive(bool select);

protected:
    void insertCurvesList();
	void insertEnrichmentsList();
    Graph *d_graph = nullptr;
};

/*****************************************************************************
 *
 * Class CurveTreeItem
 *
 *****************************************************************************/
//! CurveTreeItem tree widget item class
class CurveTreeItem : public QTreeWidgetItem
{
public:
    enum {PlotCurveTreeItem = 1002};
    CurveTreeItem(QwtPlotItem *curve, LayerItem *parent, const QString& s);

    Graph* graph() const {
        const LayerItem *p = static_cast<const LayerItem *>(parent());
        return p ? p->graph() : nullptr;
    }
    void setActive(bool on);

    QwtPlotItem *plotItem() { return d_curve; };
    const QwtPlotItem *plotItem() const { return d_curve; };
    int plotItemType() const;
    int plotItemStyle() const;
    int plotItemIndex() const;

protected:
    QwtPlotItem *d_curve = nullptr;
};

/*****************************************************************************
 *
 * Class FrameWidgetTreeItem
 *
 *****************************************************************************/
//! FrameWidgetTreeItem tree widget item class
class FrameWidgetTreeItem : public QTreeWidgetItem
{
public:
	enum {FrameWidgetItem = 1003};
	FrameWidgetTreeItem(FrameWidget *w, LayerItem *parent, const QString& s);

	Graph* graph() const {
        const LayerItem *p = static_cast<const LayerItem *>(parent());
        return p ? p->graph() : nullptr;
    }
	FrameWidget *frameWidget() const {return d_widget;};

	 void setActive(bool on);

protected:
	QPixmap frameWidgetPixmap() const;

	FrameWidget *d_widget = nullptr;
};
#endif
