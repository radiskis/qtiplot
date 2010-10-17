/***************************************************************************
    File                 : PlotDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
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

	void pickErrorBarsColor();
	void enableBoxApplyColor(int);

	void setAutomaticBinning(bool on = true);
	void showPlotAssociations(QTreeWidgetItem *item, int);

	//box plots
	void setBoxType(int index);
	void setBoxRangeType(int index);
	void setWhiskersRange(int index);

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

protected slots:
    void setActiveLayer(LayerItem *item);
    void updateTreeWidgetItem(QTreeWidgetItem *item);
    void updateBackgroundTransparency(int alpha);
    void updateCanvasTransparency(int alpha);
	void setTitlesFont();
	void setAxesLabelsFont();
	void setAxesNumbersFont();
	void setLegendsFont();
	void editCurve();
	void chooseLabelsFont();
	void applyLayerFormat();
	void setLayerDefaultValues();
    void setEquidistantLevels();
    void showCustomPenColumn(bool on);

private:
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

	void applyGapToLayer(Graph *g);
	void applyGap(Graph *g);

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
	void initLayerGeometryPage();
	void initPlotGeometryPage();
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
    void showAllLabelControls(bool show = true);
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
    QWidget *curvePlotTypeBox, *layerPage, *layerGeometryPage, *piePage, *fontsPage, *printPage, *speedPage, *functionPage;
    QTreeWidget* listBox;
    QCheckBox *boxAntialiasing, *boxScaleLayers, *boxPrintCrops;
    ColorButton *boxBorderColor, *boxBackgroundColor, *boxCanvasColor;
	QSpinBox *boxBackgroundTransparency, *boxCanvasTransparency, *boxBorderWidth, *boxMargin;
	QSpinBox *boxRadius;
	DoubleSpinBox *boxPieLineWidth;
    ColorBox *boxFirstColor;
    ColorButton *boxPieLineColor;
    PatternBox *boxPiePattern;
    PenStyleBox* boxPieLineStyle;

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
	QPushButton* buttonStatistics, *btnEditCurve, *buttonBoxStatistics;
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
	QSpinBox *boxCurveOpacity;
	QSlider *curveOpacitySlider;

	DoubleSpinBox *boxPlotX, *boxPlotY, *boxPlotWidth, *boxPlotHeight;
	QComboBox *plotUnitBox, *gapApplyToBox;
	QCheckBox *boxResizeLayers, *keepPlotRatioBox, *layerScaleFonts;
	EnrichmentDialog *enrichmentDialog;
	QPushButton *btnUp, *btnDown;
	QLabel *barsOffsetLabel;
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
