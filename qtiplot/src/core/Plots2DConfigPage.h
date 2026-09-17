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

    int curveStyle() const;
    void setSymbolsList(const QList<int>& symbList);
    void setColorsList(const QList<QColor>& colList, const QStringList& colNames);
    int convertToPixels(double w, FrameWidget::Unit unit, int dimension) const;
    double convertFromPixels(int w, FrameWidget::Unit unit, int dimension) const;

    ApplicationWindow *d_app = nullptr;

    QTabWidget *plotsTabWidget = nullptr;
    QWidget *plotOptions = nullptr;
    QWidget *curves = nullptr;
    QWidget *axesPage = nullptr;
    QWidget *plotTicks = nullptr;
    QWidget *gridPage = nullptr;
    QWidget *plotGeometryPage = nullptr;
    QWidget *plotSpeedPage = nullptr;
    QWidget *plotFonts = nullptr;
    QWidget *plotPrint = nullptr;

    // Options tab
    QCheckBox *boxAutoscaling = nullptr;
    QCheckBox *boxScaleFonts = nullptr;
    QCheckBox *boxTitle = nullptr;
    QCheckBox *boxFrame = nullptr;
    QLabel *labelFrameWidth = nullptr;
    QSpinBox *boxFrameWidth = nullptr;
    QLabel *lblMargin = nullptr;
    QSpinBox *boxMargin = nullptr;
    QLabel *legendDisplayLabel = nullptr;
    QComboBox *legendDisplayBox = nullptr;
    QLabel *lblAxisLabeling = nullptr;
    QComboBox *axisLabelingBox = nullptr;
    QLabel *attachToLabel = nullptr;
    QComboBox *attachToBox = nullptr;

    QGroupBox *groupBackgroundOptions = nullptr;
    QLabel *labelGraphBkgColor = nullptr;
    ColorButton *boxBackgroundColor = nullptr;
    QLabel *labelGraphBkgOpacity = nullptr;
    QSpinBox *boxBackgroundTransparency = nullptr;
    QSlider *bkgOpacitySlider = nullptr;
    QLabel *labelGraphCanvasColor = nullptr;
    ColorButton *boxCanvasColor = nullptr;
    QLabel *labelGraphCanvasOpacity = nullptr;
    QSpinBox *boxCanvasTransparency = nullptr;
    QSlider *canvasOpacitySlider = nullptr;
    QLabel *labelGraphFrameColor = nullptr;
    ColorButton *boxBorderColor = nullptr;
    QLabel *labelGraphFrameWidth = nullptr;
    QSpinBox *boxBorderWidth = nullptr;

    QCheckBox *boxResize = nullptr;
    QCheckBox *boxLabelsEditing = nullptr;
    QCheckBox *boxEmptyCellGap = nullptr;

    // Curves tab
    QLabel *lblCurveStyle = nullptr;
    QComboBox *boxCurveStyle = nullptr;
    QLabel *lblLineWidth = nullptr;
    DoubleSpinBox *boxCurveLineWidth = nullptr;
    QLabel *lblLineStyle = nullptr;
    PenStyleBox *lineStyleBox = nullptr;
    QLabel *lblPattern = nullptr;
    PatternBox *patternBox = nullptr;
    QLabel *lblCurveAlpha = nullptr;
    QSpinBox *curveAlphaBox = nullptr;

    QLabel *lblSymbBox = nullptr;
    SymbolBox *symbolBox = nullptr;
    QLabel *lblSymbSize = nullptr;
    QSpinBox *boxSymbolSize = nullptr;
    QLabel *lblSymbEdge = nullptr;
    DoubleSpinBox *symbolEdgeBox = nullptr;
    QCheckBox *fillSymbolsBox = nullptr;

    QGroupBox *groupIndexedColors = nullptr;
    QTableWidget *colorsList = nullptr;
    QPushButton *btnColorUp = nullptr;
    QPushButton *btnColorDown = nullptr;
    QPushButton *btnRemoveColor = nullptr;
    QPushButton *btnNewColor = nullptr;
    QPushButton *btnLoadDefaultColors = nullptr;
    QList<QColor> d_indexed_colors;
    QStringList d_indexed_color_names;

    QGroupBox *groupIndexedSymbols = nullptr;
    QTableWidget *symbolsList = nullptr;
    QPushButton *btnSymbolUp = nullptr;
    QPushButton *btnSymbolDown = nullptr;
    QPushButton *btnLoadDefaultSymbols = nullptr;
    QList<int> d_indexed_symbols;

    // Axes tab
    QLabel *lblAxesLineWidth = nullptr;
    QSpinBox *boxLineWidth = nullptr;
    QCheckBox *boxBackbones = nullptr;
    QCheckBox *boxSynchronizeScales = nullptr;
    QGridLayout *enabledAxesGrid = nullptr;
    QLabel *labelGraphAxesLabelsDist = nullptr;
    QSpinBox *boxAxesLabelsDist = nullptr;
    QLabel *labelTickLabelsDist = nullptr;
    QSpinBox *boxTickLabelsDist = nullptr;

    // Ticks tab
    QLabel *lblMajTicks = nullptr;
    QComboBox *boxMajTicks = nullptr;
    QLabel *lblMajTicksLength = nullptr;
    QSpinBox *boxMajTicksLength = nullptr;
    QLabel *lblMinTicks = nullptr;
    QComboBox *boxMinTicks = nullptr;
    QLabel *lblMinTicksLength = nullptr;
    QSpinBox *boxMinTicksLength = nullptr;

    // Grid tab
    QListWidget *axesGridList = nullptr;
    QCheckBox *boxMajorGrid = nullptr;
    QLabel *lblGridLineColor = nullptr;
    ColorButton *boxColorMajor = nullptr;
    QLabel *lblGridLineType = nullptr;
    PenStyleBox *boxTypeMajor = nullptr;
    QLabel *lblGridLineWidth = nullptr;
    DoubleSpinBox *boxWidthMajor = nullptr;
    QCheckBox *boxMinorGrid = nullptr;
    ColorButton *boxColorMinor = nullptr;
    PenStyleBox *boxTypeMinor = nullptr;
    DoubleSpinBox *boxWidthMinor = nullptr;
    QComboBox *boxGridXAxis = nullptr;
    QComboBox *boxGridYAxis = nullptr;
    QCheckBox *boxAntialiseGrid = nullptr;

    // Geometry tab
    QLabel *unitLabel = nullptr;
    QComboBox *unitBox = nullptr;
    QLabel *canvasWidthLabel = nullptr;
    DoubleSpinBox *boxCanvasWidth = nullptr;
    QLabel *canvasHeightLabel = nullptr;
    DoubleSpinBox *boxCanvasHeight = nullptr;
    QCheckBox *keepRatioBox = nullptr;

    // Speed tab
    QGroupBox *speedModeBox = nullptr;
    QLabel *maxPointsLabel = nullptr;
    QSpinBox *boxMaxPoints = nullptr;
    QLabel *toleranceLabel = nullptr;
    DoubleSpinBox *boxDouglasPeukerTolerance = nullptr;
    QCheckBox *applySpeedExportBox = nullptr;
    QGroupBox *antialiasingGroupBox = nullptr;
    QCheckBox *disableAntialiasingBox = nullptr;
    QSpinBox *curveSizeBox = nullptr;

    // Fonts tab
    QPushButton *buttonTitleFont = nullptr;
    QPushButton *buttonLegendFont = nullptr;
    QPushButton *buttonAxesFont = nullptr;
    QPushButton *buttonNumbersFont = nullptr;

    // Print tab
    QCheckBox *boxScaleLayersOnPrint = nullptr;
    QCheckBox *boxPrintCropmarks = nullptr;

    QFont axesFont, numbersFont, legendFont, titleFont;
};

#endif // PLOTS2D_CONFIG_PAGE_H
