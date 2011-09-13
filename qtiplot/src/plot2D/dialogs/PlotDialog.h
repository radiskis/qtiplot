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
class QStringList;
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

//! Custom plot/curves dialog
class PlotDialog : public QDialog
{
    Q_OBJECT

public:
    PlotDialog(bool showExtended, QWidget* parent = 0, Qt::WFlags fl = 0 );
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
	void displayCoordinates(int unit, Graph *g = 0);
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

    int labelsAlignment();
	void closeEvent(QCloseEvent* e);

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
    void contextMenuEvent(QContextMenuEvent *e);
	void showAllLabelControls(bool show = true, int curveType = 0);
    void updateContourLevelsDisplay(Spectrogram *sp);
    QRect layerCanvasRect(QWidget *widget, double x, double y, double w, double h, FrameWidget::Unit unit);

	double aspect_ratio, plot_aspect_ratio;

    QFont titleFont, legendFont, axesFont, numbersFont;

    MultiLayer *d_ml;
	QStringList columnNames;

	DoubleSpinBox* boxX, *boxY, *boxLayerWidth, *boxLayerHeight;
	QCheckBox *keepRatioBox;

    QPushButton *btnTitle, *btnAxesLabels, *btnAxesNumbers, *btnLegend;
	ColorMapEditor *colorMapEditor;
	QWidget *curvePlotTypeBox, *layerPage, *layerGeometryPage, *piePage, *fontsPage, *printPage;
	QWidget *layerDisplayPage, *speedPage, *functionPage, *canvasPage;
    QTreeWidget* listBox;
	QCheckBox *boxAntialiasing, *boxScaleLayers, *boxPrintCrops, *boxAutoscaling, *boxGridPosition, *boxMissingData;
    ColorButton *boxBorderColor, *boxBackgroundColor, *boxCanvasColor;
	QSpinBox *boxBackgroundTransparency, *boxCanvasTransparency, *boxBorderWidth, *boxMargin;
	QSpinBox *boxRadius;
	DoubleSpinBox *boxPieLineWidth;
    ColorBox *boxFirstColor;
    ColorButton *boxPieLineColor;
    PatternBox *boxPiePattern;
    PenStyleBox* boxPieLineStyle;
	QLineEdit *imagePathBox;
	QComboBox *imageApplyToBox;
	QRadioButton *colorBtn, *imageBtn;
	QGroupBox *canvasColorBox, *canvasImageBox;

    QPushButton* buttonApply, *btnWorksheet;
    QPushButton* buttonOk, *btnMore;
    QPushButton* buttonCancel;
    QComboBox* boxPlotType;
    QWidget* linePage;
    QComboBox* boxConnect;
    PenStyleBox* boxLineStyle;
    DoubleSpinBox *boxLineWidth, *boxPenWidth;
    ColorButton* boxLineColor, *boxAreaColor;
    QWidget* symbolPage;
    QSpinBox* boxSymbolSize;
    ColorButton *boxSymbolColor, *boxFillColor;
    SymbolBox* boxSymbolStyle;
    PatternBox *boxPattern;
	QTabWidget* privateTabWidget;
	QWidget *errorsPage, *spectrogramPage, *contourLinesPage;
	QGroupBox* fillGroupBox;
    QCheckBox* plusBox;
    QCheckBox* minusBox;
    QCheckBox* xBox;
    ColorButton *colorBox, *levelsColorBox, *vectColorBox;
    DoubleSpinBox* widthBox;
    QComboBox* capBox;
    QCheckBox* throughBox;
	QLabel *labelPosition, *labelXEnd, *labelYEnd;
	QGroupBox* GroupBoxH;
	QWidget *histogramPage, *spacingPage;
	DoubleSpinBox *binSizeBox, *histogramBeginBox, *histogramEndBox;
	QCheckBox *automaticBox;
	QPushButton* buttonStatistics, *btnEditCurve, *buttonBoxStatistics, *btnEditCurveRange;
	QSpinBox* gapBox, *offsetBox, *boxWidth;
	QWidget *vectPage, *boxPage, *percentilePage, *axesPage;
	QComboBox *xEndBox, *yEndBox, *boxType, *boxWhiskersType, *boxWhiskersRange, *boxRange;
	QSpinBox* headAngleBox, *headLengthBox, *boxPercSize;
	DoubleSpinBox *vectWidthBox, *boxEdgeWidth;
	QCheckBox *filledHeadBox;
	QSpinBox *boxCoef, *boxWhiskersCoef;
	QCheckBox *boxFillSymbols, *boxFillSymbol;
	ColorButton *boxPercFillColor, *boxEdgeColor;
	QLabel 	*whiskerCoeffLabel, *whiskerRangeLabel, *boxCoeffLabel;
	QLabel *boxRangeLabel, *whiskerCntLabel, *boxCntLabel;
	QGroupBox *GroupBoxVectEnd;
	QComboBox *vectPosBox, *boxXAxis, *boxYAxis, *colorScaleBox;
	PenStyleBox	*boxContourStyle;
  	QSpinBox *levelsBox, *colorScaleWidthBox;
	DoubleSpinBox *contourWidthBox;
  	QGroupBox *levelsGroupBox, *axisScaleBox, *imageGroupBox;
  	QGroupBox *defaultPenBox;
  	QRadioButton *defaultScaleBox, *grayScaleBox, *customScaleBox, *defaultContourBox, *autoContourBox;

	QCheckBox *boxWhiskerLabels, *boxBoxLabels;
	QComboBox *boxWhiskersFormatApplyToBox, *boxLabelsFormatApplyToBox, *percentileFormatApplyToBox;
    SymbolBox *boxMaxStyle, *boxMinStyle, *boxMeanStyle, *box99Style, *box1Style;
    QDoubleSpinBox *whiskerCnt, *boxCnt;
    //!Labels page
    QGroupBox *labelsGroupBox;
    DoubleSpinBox *boxLabelsAngle;
    QSpinBox *boxLabelsXOffset, *boxLabelsYOffset;
    QCheckBox *boxLabelsWhiteOut;
    QPushButton *btnLabelsFont;
    QComboBox *boxLabelsAlign, *boxLabelsColumn;
    ColorButton* boxLabelsColor;
    QWidget *labelsPage;

    QGroupBox *pieAutoLabelsBox, *boxPieWedge;
	DoubleSpinBox *boxPieStartAzimuth, *boxPieEdgeDist, *boxPieViewAngle, *boxPieThickness;
	QCheckBox *boxPieConterClockwise, *boxPieValues, *boxPiePercentages, *boxPieCategories;
	QWidget *pieLabelsPage;
    QSpinBox *boxPieOffset;
	QWidget *pieGeometryPage;

	QComboBox *unitBox;
	QComboBox *backgroundApplyToBox;
	QPushButton *layerDefaultBtn;

	DoubleSpinBox *firstContourLineBox, *contourLinesDistanceBox;
	QLabel *justifyLabelsLbl, *labelsColumnLbl;

	QWidget *spectroValuesPage;
	QComboBox *boxSpectroMatrix;
	QCheckBox *boxUseMatrixFormula;
	ContourLinesEditor *contourLinesEditor;
	QPushButton *btnSetEquidistantLevels;
	QRadioButton *customPenBtn;

	QSpinBox *boxSkipSymbols, *boxSkipErrorBars;
	QComboBox *symbolsFormatApplyToBox, *lineFormatApplyToBox, *errorBarsFormatApplyToBox, *sizeApplyToBox;
	QSpinBox *boxMaxPoints;
	DoubleSpinBox *boxDouglasPeukerTolerance;
	QGroupBox *speedModeBox;
	QCheckBox *boxApplyColorTo;

	QWidget *miscPage, *plotGeometryPage;
	QCheckBox *boxLinkXAxes;
	QComboBox *boxLinkAllXAxes;

	FunctionDialog *functionEdit;
	QSpinBox *boxCurveOpacity, *boxSymbolTransparency, *boxLineTransparency, *boxPercentileTransparency;
	QSlider *curveOpacitySlider, *symbTransparencySlider, *lineTransparencySlider, *percentileTransparencySlider;

	DoubleSpinBox *boxPlotX, *boxPlotY, *boxPlotWidth, *boxPlotHeight;
	QComboBox *plotUnitBox, *gapApplyToBox;
	QCheckBox *boxResizeLayers, *keepPlotRatioBox, *layerScaleFonts;
	EnrichmentDialog *enrichmentDialog;
	QPushButton *btnUp, *btnDown;
	QLabel *barsOffsetLabel;

	QRadioButton *standardSymbolBtn, *imageSymbolBtn;
	QGroupBox *standardSymbolFormatBox, *imageSymBolFormatBox;
	QLineEdit *imageSymbolPathBox;
	QLabel *symbolImageLabel;

	QGroupBox *boxFramed;
	ColorButton *boxFrameColor;
	QSpinBox *boxFrameWidth;
	QPushButton *canvasDefaultBtn;
	QSlider *canvasOpacitySlider, *bkgOpacitySlider;

	QCheckBox *boxLeftAxis, *boxRightAxis, *boxBottomAxis, *boxTopAxis;
	QCompleter *completer;
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

    Graph *graph() { return d_graph; };
    void setActive(bool select);

protected:
    void insertCurvesList();
	void insertEnrichmentsList();
    Graph *d_graph;
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

    Graph* graph(){return ((LayerItem *)parent())->graph();};
    void setActive(bool on);

    const QwtPlotItem *plotItem() { return d_curve; };
    int plotItemType();
    int plotItemStyle();
    int plotItemIndex();

protected:
    QwtPlotItem *d_curve;
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

	Graph* graph(){return ((LayerItem *)parent())->graph();};
	FrameWidget *frameWidget(){return d_widget;};

	 void setActive(bool on);

protected:
	QPixmap frameWidgetPixmap();

	FrameWidget *d_widget;
};
#endif
