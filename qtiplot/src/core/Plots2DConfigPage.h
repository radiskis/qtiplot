/***************************************************************************
    File                 : Plots2DConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D plots preferences page

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
#ifndef PLOTS2D_CONFIG_PAGE_H
#define PLOTS2D_CONFIG_PAGE_H

#include "ConfigPage.h"
#include <QFont>
#include <QList>
#include <QColor>
#include <QStringList>
#include "Graph.h"
#include "FrameWidget.h"

class ApplicationWindow;
class ApplicationSettings;

class QTabWidget;
class QWidget;
class QCheckBox;
class QLabel;
class QSpinBox;
class DoubleSpinBox;
class QComboBox;
class ColorButton;
class QSlider;
class QPushButton;
class QGroupBox;
class QTableWidget;
class QListWidget;
class SymbolBox;
class PatternBox;
class PenStyleBox;
class QGridLayout;

class Plots2DConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit Plots2DConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("2D Plots"); }
    QIcon pageIcon() const override { return QIcon(":/config_curves.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

private slots:
    void showFrameWidth(bool ok);
    void pickAxesFont();
    void pickNumbersFont();
    void pickLegendFont();
    void pickTitleFont();

    void moveColor(bool up = true);
    void moveColorDown() { moveColor(false); }
    void removeColor();
    void newColor();
    void loadDefaultColors();
    void showColorDialog(int row, int col);
    void changeColorName(int row, int col);

    void updateSymbolsList(int style);
    void setCurrentSymbol(SymbolBox *sb);
    void loadDefaultSymbols();
    void moveSymbol(bool up = true);
    void moveSymbolDown() { moveSymbol(false); }

    void showGridOptions(int axis);
    void majorGridEnabled(bool on);
    void minorGridEnabled(bool on);
    void updateGrid();

    void updateCanvasSize(int unit);
    void adjustCanvasHeight(double width);
    void adjustCanvasWidth(double height);

    void enableCurveAntialiasingSizeBox(bool on);

private:
    void initCurvesTab();
    void initAxesTab();
    void initGridTab();
    void initLayerGeometryTab();
    void initLayerSpeedTab();
    void initTicksTab();
    void initFontsTab();
    void initPrintTab();

    int curveStyle();
    void setSymbolsList(const QList<int>& symbList);
    void setColorsList(const QList<QColor>& colList, const QStringList& colNames);
    int convertToPixels(double w, FrameWidget::Unit unit, int dimension);
    double convertFromPixels(int w, FrameWidget::Unit unit, int dimension);

    ApplicationWindow *d_app;

    QTabWidget *plotsTabWidget;
    QWidget *plotOptions;
    QWidget *curves;
    QWidget *axesPage;
    QWidget *plotTicks;
    QWidget *gridPage;
    QWidget *plotGeometryPage;
    QWidget *plotSpeedPage;
    QWidget *plotFonts;
    QWidget *plotPrint;

    // Options tab
    QCheckBox *boxAutoscaling;
    QCheckBox *boxScaleFonts;
    QCheckBox *boxTitle;
    QCheckBox *boxFrame;
    QLabel *labelFrameWidth;
    QSpinBox *boxFrameWidth;
    QLabel *lblMargin;
    QSpinBox *boxMargin;
    QLabel *legendDisplayLabel;
    QComboBox *legendDisplayBox;
    QLabel *lblAxisLabeling;
    QComboBox *axisLabelingBox;
    QLabel *attachToLabel;
    QComboBox *attachToBox;

    QGroupBox *groupBackgroundOptions;
    QLabel *labelGraphBkgColor;
    ColorButton *boxBackgroundColor;
    QLabel *labelGraphBkgOpacity;
    QSpinBox *boxBackgroundTransparency;
    QSlider *bkgOpacitySlider;
    QLabel *labelGraphCanvasColor;
    ColorButton *boxCanvasColor;
    QLabel *labelGraphCanvasOpacity;
    QSpinBox *boxCanvasTransparency;
    QSlider *canvasOpacitySlider;
    QLabel *labelGraphFrameColor;
    ColorButton *boxBorderColor;
    QLabel *labelGraphFrameWidth;
    QSpinBox *boxBorderWidth;

    QCheckBox *boxResize;
    QCheckBox *boxLabelsEditing;
    QCheckBox *boxEmptyCellGap;

    // Curves tab
    QLabel *lblCurveStyle;
    QComboBox *boxCurveStyle;
    QLabel *lblLineWidth;
    DoubleSpinBox *boxCurveLineWidth;
    QLabel *lblLineStyle;
    PenStyleBox *lineStyleBox;
    QLabel *lblPattern;
    PatternBox *patternBox;
    QLabel *lblCurveAlpha;
    QSpinBox *curveAlphaBox;

    QLabel *lblSymbBox;
    SymbolBox *symbolBox;
    QLabel *lblSymbSize;
    QSpinBox *boxSymbolSize;
    QLabel *lblSymbEdge;
    DoubleSpinBox *symbolEdgeBox;
    QCheckBox *fillSymbolsBox;

    QGroupBox *groupIndexedColors;
    QTableWidget *colorsList;
    QPushButton *btnColorUp;
    QPushButton *btnColorDown;
    QPushButton *btnRemoveColor;
    QPushButton *btnNewColor;
    QPushButton *btnLoadDefaultColors;
    QList<QColor> d_indexed_colors;
    QStringList d_indexed_color_names;

    QGroupBox *groupIndexedSymbols;
    QTableWidget *symbolsList;
    QPushButton *btnSymbolUp;
    QPushButton *btnSymbolDown;
    QPushButton *btnLoadDefaultSymbols;
    QList<int> d_indexed_symbols;

    // Axes tab
    QLabel *lblAxesLineWidth;
    QSpinBox *boxLineWidth;
    QCheckBox *boxBackbones;
    QCheckBox *boxSynchronizeScales;
    QGridLayout *enabledAxesGrid;
    QLabel *labelGraphAxesLabelsDist;
    QSpinBox *boxAxesLabelsDist;
    QLabel *labelTickLabelsDist;
    QSpinBox *boxTickLabelsDist;

    // Ticks tab
    QLabel *lblMajTicks;
    QComboBox *boxMajTicks;
    QLabel *lblMajTicksLength;
    QSpinBox *boxMajTicksLength;
    QLabel *lblMinTicks;
    QComboBox *boxMinTicks;
    QLabel *lblMinTicksLength;
    QSpinBox *boxMinTicksLength;

    // Grid tab
    QListWidget *axesGridList;
    QCheckBox *boxMajorGrid;
    QLabel *lblGridLineColor;
    ColorButton *boxColorMajor;
    QLabel *lblGridLineType;
    PenStyleBox *boxTypeMajor;
    QLabel *lblGridLineWidth;
    DoubleSpinBox *boxWidthMajor;
    QCheckBox *boxMinorGrid;
    ColorButton *boxColorMinor;
    PenStyleBox *boxTypeMinor;
    DoubleSpinBox *boxWidthMinor;
    QComboBox *boxGridXAxis;
    QComboBox *boxGridYAxis;
    QCheckBox *boxAntialiseGrid;

    // Geometry tab
    QLabel *unitLabel;
    QComboBox *unitBox;
    QLabel *canvasWidthLabel;
    DoubleSpinBox *boxCanvasWidth;
    QLabel *canvasHeightLabel;
    DoubleSpinBox *boxCanvasHeight;
    QCheckBox *keepRatioBox;

    // Speed tab
    QGroupBox *speedModeBox;
    QLabel *decimationMethodLabel;
    QComboBox *boxDecimationMethod;
    QLabel *maxPointsLabel;
    QSpinBox *boxMaxPoints;
    QLabel *toleranceLabel;
    DoubleSpinBox *boxDouglasPeukerTolerance;
    QCheckBox *applySpeedExportBox;
    QGroupBox *antialiasingGroupBox;
    QCheckBox *disableAntialiasingBox;
    QSpinBox *curveSizeBox;

    // Fonts tab
    QPushButton *buttonTitleFont;
    QPushButton *buttonLegendFont;
    QPushButton *buttonAxesFont;
    QPushButton *buttonNumbersFont;

    // Print tab
    QCheckBox *boxScaleLayersOnPrint;
    QCheckBox *boxPrintCropmarks;

    QFont axesFont, numbersFont, legendFont, titleFont;
};

#endif // PLOTS2D_CONFIG_PAGE_H
