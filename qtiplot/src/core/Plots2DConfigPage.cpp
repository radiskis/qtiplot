/***************************************************************************
    File                 : Plots2DConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D plots preferences page implementation

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
#include "Plots2DConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>
#include <ColorBox.h>
#include <DoubleSpinBox.h>
#include <SymbolBox.h>
#include <PatternBox.h>
#include <PenStyleBox.h>
#include <MultiLayer.h>
#include <Graph.h>
#include <Grid.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QTableWidget>
#include <QListWidget>
#include <QFontDialog>
#include <QColorDialog>
#include <QIcon>
#include <QHeaderView>

Plots2DConfigPage::Plots2DConfigPage(QWidget *parent)
    : ConfigPage(parent), d_app(nullptr)
{
	plotsTabWidget = new QTabWidget(this);

	plotOptions = new QWidget();
	QVBoxLayout * optionsTabLayout = new QVBoxLayout( plotOptions );
	optionsTabLayout->setSpacing(5);

	QGroupBox * groupBoxOptions = new QGroupBox();
	optionsTabLayout->addWidget( groupBoxOptions );

	QGridLayout * optionsLayout = new QGridLayout( groupBoxOptions );

	boxAutoscaling = new QCheckBox();
	optionsLayout->addWidget( boxAutoscaling, 0, 0);

	boxScaleFonts = new QCheckBox();
	optionsLayout->addWidget( boxScaleFonts, 0, 1);

	boxTitle = new QCheckBox();
	optionsLayout->addWidget(boxTitle, 1, 0);

	boxFrame = new QCheckBox();
	optionsLayout->addWidget(boxFrame, 1, 1);

	labelFrameWidth = new QLabel();
	optionsLayout->addWidget(labelFrameWidth, 4, 0);
	boxFrameWidth= new QSpinBox();
	optionsLayout->addWidget(boxFrameWidth, 4, 1);
	boxFrameWidth->setRange(1, 100);

	lblMargin = new QLabel();
	optionsLayout->addWidget(lblMargin, 5, 0);
	boxMargin = new QSpinBox();
	boxMargin->setRange(0, 1000);
	boxMargin->setSingleStep(5);
	optionsLayout->addWidget(boxMargin, 5, 1);

	legendDisplayLabel = new QLabel;
	optionsLayout->addWidget(legendDisplayLabel, 6, 0);
	legendDisplayBox = new QComboBox;
	optionsLayout->addWidget(legendDisplayBox, 6, 1);

	lblAxisLabeling = new QLabel;
	optionsLayout->addWidget(lblAxisLabeling, 7, 0);
	axisLabelingBox = new QComboBox;
	optionsLayout->addWidget(axisLabelingBox, 7, 1);

	attachToLabel = new QLabel;
	optionsLayout->addWidget(attachToLabel, 8, 0);
	attachToBox = new QComboBox;
	optionsLayout->addWidget(attachToBox, 8, 1);

	optionsLayout->setRowStretch(9, 1);

	groupBackgroundOptions = new QGroupBox(tr("Background"));
	optionsTabLayout->addWidget( groupBackgroundOptions );
	QGridLayout *graphBackgroundLayout = new QGridLayout( groupBackgroundOptions );

	labelGraphBkgColor = new QLabel(tr("Background Color"));
    graphBackgroundLayout->addWidget(labelGraphBkgColor, 0, 0 );
    boxBackgroundColor = new ColorButton();
    graphBackgroundLayout->addWidget(boxBackgroundColor, 0, 1 );

	labelGraphBkgOpacity = new QLabel(tr("Opacity" ));
    graphBackgroundLayout->addWidget(labelGraphBkgOpacity, 0, 2 );
    boxBackgroundTransparency = new QSpinBox();
	boxBackgroundTransparency->setRange(0, 100);
	boxBackgroundTransparency->setSuffix(" %");
    boxBackgroundTransparency->setWrapping(true);
	boxBackgroundTransparency->setSpecialValueText(" " + tr("Transparent"));

	bkgOpacitySlider = new QSlider();
	bkgOpacitySlider->setOrientation(Qt::Horizontal);
	bkgOpacitySlider->setRange(0, 100);

	connect(bkgOpacitySlider, &QSlider::valueChanged, boxBackgroundTransparency, &QSpinBox::setValue);
	connect(boxBackgroundTransparency, QOverload<int>::of(&QSpinBox::valueChanged), bkgOpacitySlider, &QSlider::setValue);

	QHBoxLayout* hb = new QHBoxLayout();
	hb->addWidget(bkgOpacitySlider);
	hb->addWidget(boxBackgroundTransparency);
	graphBackgroundLayout->addLayout(hb, 0, 3 );

	labelGraphCanvasColor = new QLabel(tr("Canvas Color" ));
    graphBackgroundLayout->addWidget(labelGraphCanvasColor, 1, 0);
    boxCanvasColor = new ColorButton();
    graphBackgroundLayout->addWidget( boxCanvasColor, 1, 1 );

	labelGraphCanvasOpacity = new QLabel(tr("Opacity"));
    graphBackgroundLayout->addWidget(labelGraphCanvasOpacity, 1, 2 );
    boxCanvasTransparency = new QSpinBox();
	boxCanvasTransparency->setRange(0, 100);
	boxCanvasTransparency->setSuffix(" %");
    boxCanvasTransparency->setWrapping(true);
	boxCanvasTransparency->setSpecialValueText(" " + tr("Transparent"));

	canvasOpacitySlider = new QSlider();
	canvasOpacitySlider->setOrientation(Qt::Horizontal);
	canvasOpacitySlider->setRange(0, 100);

	connect(canvasOpacitySlider, &QSlider::valueChanged, boxCanvasTransparency, &QSpinBox::setValue);
	connect(boxCanvasTransparency, QOverload<int>::of(&QSpinBox::valueChanged), canvasOpacitySlider, &QSlider::setValue);

	QHBoxLayout* hb1 = new QHBoxLayout();
	hb1->addWidget(canvasOpacitySlider);
	hb1->addWidget(boxCanvasTransparency);
	graphBackgroundLayout->addLayout(hb1, 1, 3);

	labelGraphFrameColor = new QLabel(tr("Border Color"));
    graphBackgroundLayout->addWidget(labelGraphFrameColor, 2, 0);
    boxBorderColor = new ColorButton();
    graphBackgroundLayout->addWidget(boxBorderColor, 2, 1);

	labelGraphFrameWidth = new QLabel(tr( "Width" ));
    graphBackgroundLayout->addWidget(labelGraphFrameWidth, 2, 2);
    boxBorderWidth = new QSpinBox();
    graphBackgroundLayout->addWidget(boxBorderWidth, 2, 3);

	graphBackgroundLayout->setRowStretch(4, 1);
	graphBackgroundLayout->setColumnStretch(4, 1);

	boxResize = new QCheckBox();
	optionsTabLayout->addWidget( boxResize );

    boxLabelsEditing = new QCheckBox();
    optionsTabLayout->addWidget(boxLabelsEditing);

	boxEmptyCellGap = new QCheckBox();
	optionsTabLayout->addWidget(boxEmptyCellGap);

	plotsTabWidget->addTab( plotOptions, QString() );

	initCurvesTab();
	initAxesTab();
	initTicksTab();
	initGridTab();
	initLayerGeometryTab();
	initLayerSpeedTab();
	initFontsTab();
	initPrintTab();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(plotsTabWidget);

	connect(boxFrame, &QAbstractButton::toggled, this, &Plots2DConfigPage::showFrameWidth);
	connect(buttonAxesFont, &QAbstractButton::clicked, this, &Plots2DConfigPage::pickAxesFont);
	connect(buttonNumbersFont, &QAbstractButton::clicked, this, &Plots2DConfigPage::pickNumbersFont);
	connect(buttonLegendFont, &QAbstractButton::clicked, this, &Plots2DConfigPage::pickLegendFont);
	connect(buttonTitleFont, &QAbstractButton::clicked, this, &Plots2DConfigPage::pickTitleFont);
}

void Plots2DConfigPage::initCurvesTab()
{
	curves = new QWidget();
	QHBoxLayout * curvesTabLayout = new QHBoxLayout( curves );

	QGroupBox *curvesGroupBox = new QGroupBox();
	QGridLayout * curvesLayout = new QGridLayout( curvesGroupBox );

	lblCurveStyle = new QLabel();
	curvesLayout->addWidget( lblCurveStyle, 0, 0 );
	boxCurveStyle = new QComboBox();
	curvesLayout->addWidget( boxCurveStyle, 0, 1 );

	lblLineWidth = new QLabel();
	curvesLayout->addWidget( lblLineWidth, 1, 0 );
	boxCurveLineWidth = new DoubleSpinBox();
	boxCurveLineWidth->setRange(0.1, 100);
	boxCurveLineWidth->setSingleStep(1);
	curvesLayout->addWidget( boxCurveLineWidth, 1, 1 );

	lblLineStyle = new QLabel();
	curvesLayout->addWidget( lblLineStyle, 2, 0 );
	lineStyleBox = new PenStyleBox();
	curvesLayout->addWidget( lineStyleBox, 2, 1 );

	lblPattern = new QLabel();
	curvesLayout->addWidget( lblPattern, 3, 0 );
	patternBox = new PatternBox();
	curvesLayout->addWidget( patternBox, 3, 1 );

	lblCurveAlpha = new QLabel();
	curvesLayout->addWidget( lblCurveAlpha, 4, 0 );
	curveAlphaBox = new QSpinBox();
	curveAlphaBox->setRange(0, 255);
	curveAlphaBox->setSingleStep(5);
	curvesLayout->addWidget( curveAlphaBox, 4, 1 );

	QGroupBox *symbolGroupBox = new QGroupBox();
	QGridLayout * symbolLayout = new QGridLayout( symbolGroupBox );

	lblSymbBox = new QLabel();
	symbolLayout->addWidget( lblSymbBox, 0, 0 );
	symbolBox = new SymbolBox();
	symbolLayout->addWidget( symbolBox, 0, 1 );

	lblSymbSize = new QLabel();
	symbolLayout->addWidget( lblSymbSize, 1, 0 );
	boxSymbolSize = new QSpinBox();
	boxSymbolSize->setRange(1, 100);
	symbolLayout->addWidget( boxSymbolSize, 1, 1 );

	lblSymbEdge = new QLabel();
	symbolLayout->addWidget( lblSymbEdge, 2, 0 );
	symbolEdgeBox = new DoubleSpinBox();
	symbolEdgeBox->setRange(0.1, 100);
	symbolEdgeBox->setSingleStep(1);
	symbolLayout->addWidget( symbolEdgeBox, 2, 1 );

	fillSymbolsBox = new QCheckBox();
	symbolLayout->addWidget( fillSymbolsBox, 3, 0 );

	QVBoxLayout * curvesLeftLayout = new QVBoxLayout();
	curvesLeftLayout->addWidget( curvesGroupBox );
	curvesLeftLayout->addWidget( symbolGroupBox );
	curvesLeftLayout->addStretch();
	curvesTabLayout->addLayout( curvesLeftLayout );

	groupIndexedColors = new QGroupBox();
	groupIndexedColors->setCheckable(false);
	QVBoxLayout * colorsLeftLayout = new QVBoxLayout( groupIndexedColors );

	colorsList = new QTableWidget();
	colorsList->setColumnCount(2);
	colorsList->horizontalHeader()->hide();
	colorsList->verticalHeader()->hide();
	colorsList->setSelectionMode(QAbstractItemView::SingleSelection);
	colorsLeftLayout->addWidget( colorsList );

	QHBoxLayout * colorBtnLayout = new QHBoxLayout();
	btnColorUp = new QPushButton();
	btnColorUp->setIcon(QIcon(":/arrow_up.png"));
	btnColorDown = new QPushButton();
	btnColorDown->setIcon(QIcon(":/arrow_down.png"));
	btnRemoveColor = new QPushButton();
	btnRemoveColor->setIcon(QIcon(":/delete.png"));
	btnNewColor = new QPushButton();
	btnNewColor->setIcon(QIcon(":/plus.png"));
	btnLoadDefaultColors = new QPushButton();
	btnLoadDefaultColors->setIcon(QIcon(":/reload.png"));

	colorBtnLayout->addWidget( btnNewColor );
	colorBtnLayout->addWidget( btnRemoveColor );
	colorBtnLayout->addWidget( btnColorUp );
	colorBtnLayout->addWidget( btnColorDown );
	colorBtnLayout->addWidget( btnLoadDefaultColors );
	colorsLeftLayout->addLayout( colorBtnLayout );

	curvesTabLayout->addWidget( groupIndexedColors );

	groupIndexedSymbols = new QGroupBox();
	groupIndexedSymbols->setCheckable(true);
	QVBoxLayout * symbolsLeftLayout = new QVBoxLayout( groupIndexedSymbols );

	symbolsList = new QTableWidget();
	symbolsList->setColumnCount(1);
	symbolsList->horizontalHeader()->hide();
	symbolsList->verticalHeader()->hide();
	symbolsList->setSelectionMode(QAbstractItemView::SingleSelection);
	symbolsLeftLayout->addWidget( symbolsList );

	QHBoxLayout * symbolBtnLayout = new QHBoxLayout();
	btnSymbolUp = new QPushButton();
	btnSymbolUp->setIcon(QIcon(":/arrow_up.png"));
	btnSymbolDown = new QPushButton();
	btnSymbolDown->setIcon(QIcon(":/arrow_down.png"));
	btnLoadDefaultSymbols = new QPushButton();
	btnLoadDefaultSymbols->setIcon(QIcon(":/reload.png"));

	symbolBtnLayout->addWidget( btnSymbolUp );
	symbolBtnLayout->addWidget( btnSymbolDown );
	symbolBtnLayout->addWidget( btnLoadDefaultSymbols );
	symbolsLeftLayout->addLayout( symbolBtnLayout );

	curvesTabLayout->addWidget( groupIndexedSymbols );

	connect(btnColorUp, &QAbstractButton::clicked, this, &Plots2DConfigPage::moveColor);
	connect(btnColorDown, &QAbstractButton::clicked, this, &Plots2DConfigPage::moveColorDown);
	connect(btnRemoveColor, &QAbstractButton::clicked, this, &Plots2DConfigPage::removeColor);
	connect(btnNewColor, &QAbstractButton::clicked, this, &Plots2DConfigPage::newColor);
	connect(btnLoadDefaultColors, &QAbstractButton::clicked, this, &Plots2DConfigPage::loadDefaultColors);
	connect(colorsList, &QTableWidget::cellDoubleClicked, this, &Plots2DConfigPage::showColorDialog);
	connect(colorsList, &QTableWidget::cellChanged, this, &Plots2DConfigPage::changeColorName);

	connect(btnSymbolUp, &QAbstractButton::clicked, this, &Plots2DConfigPage::moveSymbol);
	connect(btnSymbolDown, &QAbstractButton::clicked, this, &Plots2DConfigPage::moveSymbolDown);
	connect(btnLoadDefaultSymbols, &QAbstractButton::clicked, this, &Plots2DConfigPage::loadDefaultSymbols);

	plotsTabWidget->addTab( curves, QString() );
}

void Plots2DConfigPage::initAxesTab()
{
	axesPage = new QWidget();
	QVBoxLayout * axesPageLayout = new QVBoxLayout( axesPage );

	QGroupBox * groupBoxAxes = new QGroupBox();
	axesPageLayout->addWidget( groupBoxAxes );
	QGridLayout * axesLayout = new QGridLayout( groupBoxAxes );

	lblAxesLineWidth = new QLabel();
	axesLayout->addWidget( lblAxesLineWidth, 0, 0 );
	boxLineWidth = new QSpinBox();
	boxLineWidth->setRange(1, 100);
	axesLayout->addWidget( boxLineWidth, 0, 1 );

	boxBackbones = new QCheckBox();
	axesLayout->addWidget( boxBackbones, 1, 0 );

	boxSynchronizeScales = new QCheckBox();
	axesLayout->addWidget( boxSynchronizeScales, 2, 0 );

	labelGraphAxesLabelsDist = new QLabel();
	axesLayout->addWidget(labelGraphAxesLabelsDist, 3, 0);
	boxAxesLabelsDist = new QSpinBox();
	boxAxesLabelsDist->setRange(0, 1000);
	axesLayout->addWidget(boxAxesLabelsDist, 3, 1);

	labelTickLabelsDist = new QLabel();
	axesLayout->addWidget(labelTickLabelsDist, 4, 0);
	boxTickLabelsDist = new QSpinBox();
	boxTickLabelsDist->setRange(0, 1000);
	axesLayout->addWidget(boxTickLabelsDist, 4, 1);

	axesLayout->setRowStretch( 5, 1 );

	QGroupBox *groupBoxAxesGrid = new QGroupBox(tr("Enabled Axes"));
	axesPageLayout->addWidget(groupBoxAxesGrid);
	enabledAxesGrid = new QGridLayout(groupBoxAxesGrid);

	enabledAxesGrid->addWidget(new QLabel(tr("Left")), 1, 0);
	enabledAxesGrid->addWidget(new QLabel(tr("Right")), 2, 0);
	enabledAxesGrid->addWidget(new QLabel(tr("Bottom")), 3, 0);
	enabledAxesGrid->addWidget(new QLabel(tr("Top")), 4, 0);

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		int row = i + 1;
		enabledAxesGrid->addWidget(new QCheckBox(), row, 2);
		enabledAxesGrid->addWidget(new QCheckBox(), row, 3);
	}

	plotsTabWidget->addTab( axesPage, QString() );
}

void Plots2DConfigPage::initTicksTab()
{
	plotTicks = new QWidget();
	QVBoxLayout * plotTicksLayout = new QVBoxLayout( plotTicks );

	QGroupBox * ticksGroupBox = new QGroupBox();
	QGridLayout * ticksLayout = new QGridLayout( ticksGroupBox );
	plotTicksLayout->addWidget( ticksGroupBox );

	lblMajTicks = new QLabel();
	ticksLayout->addWidget( lblMajTicks, 0, 0 );
	boxMajTicks = new QComboBox();
	ticksLayout->addWidget( boxMajTicks, 0, 1 );

	lblMajTicksLength = new QLabel();
	ticksLayout->addWidget( lblMajTicksLength, 0, 2 );
	boxMajTicksLength = new QSpinBox();
	boxMajTicksLength->setRange(0, 100);
	ticksLayout->addWidget( boxMajTicksLength, 0, 3 );

	lblMinTicks = new QLabel();
	ticksLayout->addWidget( lblMinTicks, 1, 0 );
	boxMinTicks = new QComboBox();
	ticksLayout->addWidget( boxMinTicks, 1, 1 );

	lblMinTicksLength = new QLabel();
	ticksLayout->addWidget( lblMinTicksLength, 1, 2 );
	boxMinTicksLength = new QSpinBox();
	boxMinTicksLength->setRange(0, 100);
	ticksLayout->addWidget( boxMinTicksLength, 1, 3 );

	ticksLayout->setRowStretch( 4, 1 );

	plotsTabWidget->addTab( plotTicks, QString() );
}

void Plots2DConfigPage::initGridTab()
{
	gridPage = new QWidget();
	QVBoxLayout * layout = new QVBoxLayout( gridPage );

	QHBoxLayout * topLayout = new QHBoxLayout();
	axesGridList = new QListWidget();
	topLayout->addWidget(axesGridList);

	QGroupBox * gridOptionsBox = new QGroupBox();
	QGridLayout * gl = new QGridLayout(gridOptionsBox);

	boxMajorGrid = new QCheckBox();
	gl->addWidget(boxMajorGrid, 0, 0);

	lblGridLineColor = new QLabel(tr("Color"));
	gl->addWidget(lblGridLineColor, 0, 1);
	boxColorMajor = new ColorButton();
	gl->addWidget(boxColorMajor, 0, 2);

	lblGridLineType = new QLabel(tr("Type"));
	gl->addWidget(lblGridLineType, 1, 1);
	boxTypeMajor = new PenStyleBox();
	gl->addWidget(boxTypeMajor, 1, 2);

	lblGridLineWidth = new QLabel(tr("Width"));
	gl->addWidget(lblGridLineWidth, 2, 1);
	boxWidthMajor = new DoubleSpinBox();
	boxWidthMajor->setRange(0.1, 100);
	boxWidthMajor->setSingleStep(1);
	gl->addWidget(boxWidthMajor, 2, 2);

	boxMinorGrid = new QCheckBox();
	gl->addWidget(boxMinorGrid, 3, 0);

	gl->addWidget(new QLabel(tr("Color")), 3, 1);
	boxColorMinor = new ColorButton();
	gl->addWidget(boxColorMinor, 3, 2);

	gl->addWidget(new QLabel(tr("Type")), 4, 1);
	boxTypeMinor = new PenStyleBox();
	gl->addWidget(boxTypeMinor, 4, 2);

	gl->addWidget(new QLabel(tr("Width")), 5, 1);
	boxWidthMinor = new DoubleSpinBox();
	boxWidthMinor->setRange(0.1, 100);
	boxWidthMinor->setSingleStep(1);
	gl->addWidget(boxWidthMinor, 5, 2);

	topLayout->addWidget(gridOptionsBox);
	layout->addLayout(topLayout);

	boxAntialiseGrid = new QCheckBox();
	layout->addWidget(boxAntialiseGrid);
	layout->addStretch();

	connect(axesGridList, &QListWidget::currentRowChanged, this, &Plots2DConfigPage::showGridOptions);
	connect(boxMajorGrid, &QAbstractButton::toggled, this, &Plots2DConfigPage::majorGridEnabled);
	connect(boxMinorGrid, &QAbstractButton::toggled, this, &Plots2DConfigPage::minorGridEnabled);
	connect(boxColorMajor, &ColorButton::colorChanged, this, &Plots2DConfigPage::updateGrid);
	connect(boxColorMinor, &ColorButton::colorChanged, this, &Plots2DConfigPage::updateGrid);
	connect(boxTypeMajor, QOverload<int>::of(&PenStyleBox::activated), this, &Plots2DConfigPage::updateGrid);
	connect(boxTypeMinor, QOverload<int>::of(&PenStyleBox::activated), this, &Plots2DConfigPage::updateGrid);
	connect(boxWidthMajor, QOverload<double>::of(&DoubleSpinBox::valueChanged), this, &Plots2DConfigPage::updateGrid);
	connect(boxWidthMinor, QOverload<double>::of(&DoubleSpinBox::valueChanged), this, &Plots2DConfigPage::updateGrid);
	connect(boxAntialiseGrid, &QAbstractButton::toggled, this, &Plots2DConfigPage::updateGrid);

	plotsTabWidget->addTab(gridPage, QString());
}

void Plots2DConfigPage::initLayerGeometryTab()
{
	plotGeometryPage = new QWidget();
	QVBoxLayout * vl = new QVBoxLayout( plotGeometryPage );

	QGroupBox * gb = new QGroupBox(tr("Canvas Size"));
	QGridLayout * gl = new QGridLayout(gb);

	unitLabel = new QLabel(tr("Unit"));
	gl->addWidget(unitLabel, 0, 0);
	unitBox = new QComboBox();
	unitBox->addItem(tr("inch"));
	unitBox->addItem(tr("mm"));
	unitBox->addItem(tr("cm"));
	unitBox->addItem(tr("point"));
	unitBox->addItem(tr("pixel"));
	gl->addWidget(unitBox, 0, 1);

	canvasWidthLabel = new QLabel(tr("Width"));
	gl->addWidget(canvasWidthLabel, 1, 0);
	boxCanvasWidth = new DoubleSpinBox();
	boxCanvasWidth->setRange(0, 10000);
	gl->addWidget(boxCanvasWidth, 1, 1);

	canvasHeightLabel = new QLabel(tr("Height"));
	gl->addWidget(canvasHeightLabel, 2, 0);
	boxCanvasHeight = new DoubleSpinBox();
	boxCanvasHeight->setRange(0, 10000);
	gl->addWidget(boxCanvasHeight, 2, 1);

	keepRatioBox = new QCheckBox(tr("Keep aspect ratio"));
	gl->addWidget(keepRatioBox, 3, 0, 1, 2);

	vl->addWidget(gb);
	vl->addStretch();

	connect(unitBox, QOverload<int>::of(&QComboBox::activated), this, &Plots2DConfigPage::updateCanvasSize);
	connect(boxCanvasWidth, QOverload<double>::of(&DoubleSpinBox::valueChanged), this, &Plots2DConfigPage::adjustCanvasHeight);
	connect(boxCanvasHeight, QOverload<double>::of(&DoubleSpinBox::valueChanged), this, &Plots2DConfigPage::adjustCanvasWidth);

	plotsTabWidget->addTab(plotGeometryPage, QString());
}

void Plots2DConfigPage::initLayerSpeedTab()
{
	plotSpeedPage = new QWidget();
	QVBoxLayout * speedLayout = new QVBoxLayout( plotSpeedPage );

	speedModeBox = new QGroupBox();
	speedModeBox->setCheckable(true);
	QGridLayout * sl = new QGridLayout( speedModeBox );

	decimationMethodLabel = new QLabel(tr("Decimation"));
	sl->addWidget(decimationMethodLabel, 0, 0);
	boxDecimationMethod = new QComboBox();
	boxDecimationMethod->addItem(tr("Douglas-Peucker"), Graph::DouglasPeucker);
	boxDecimationMethod->addItem(tr("Largest-Triangle-Three-Buckets (LTTB)"), Graph::LTTB);
	boxDecimationMethod->addItem(tr("Min-Max"), Graph::MinMax);
	sl->addWidget(boxDecimationMethod, 0, 1);

	maxPointsLabel = new QLabel(tr("Maximum number of points"));
	sl->addWidget(maxPointsLabel, 1, 0);
	boxMaxPoints = new QSpinBox();
	boxMaxPoints->setRange(100, 10000000);
	boxMaxPoints->setSingleStep(1000);
	sl->addWidget(boxMaxPoints, 1, 1);

	toleranceLabel = new QLabel(tr("Tolerance"));
	sl->addWidget(toleranceLabel, 2, 0);
	boxDouglasPeukerTolerance = new DoubleSpinBox();
	boxDouglasPeukerTolerance->setRange(0, 1000);
	boxDouglasPeukerTolerance->setSingleStep(0.1);
	sl->addWidget(boxDouglasPeukerTolerance, 2, 1);

	applySpeedExportBox = new QCheckBox();
	sl->addWidget(applySpeedExportBox, 3, 0, 1, 2);

	speedLayout->addWidget( speedModeBox );

	antialiasingGroupBox = new QGroupBox(tr("Antialiasing"));
	antialiasingGroupBox->setCheckable(true);
	QGridLayout * al = new QGridLayout( antialiasingGroupBox );

	disableAntialiasingBox = new QCheckBox(tr("Disable for curves with more than"));
	al->addWidget(disableAntialiasingBox, 0, 0);
	curveSizeBox = new QSpinBox();
	curveSizeBox->setRange(10, 1000000);
	curveSizeBox->setSingleStep(100);
	al->addWidget(curveSizeBox, 0, 1);

	speedLayout->addWidget( antialiasingGroupBox );
	speedLayout->addStretch();

	connect(disableAntialiasingBox, &QAbstractButton::toggled, this, &Plots2DConfigPage::enableCurveAntialiasingSizeBox);

	plotsTabWidget->addTab(plotSpeedPage, QString());
}

void Plots2DConfigPage::initFontsTab()
{
	plotFonts = new QWidget();
	QVBoxLayout * plotFontsLayout = new QVBoxLayout( plotFonts );

	QGroupBox * groupBox2DFonts = new QGroupBox();
	plotFontsLayout->addWidget( groupBox2DFonts );
	QVBoxLayout * fontsLayout = new QVBoxLayout( groupBox2DFonts );

	buttonTitleFont = new QPushButton();
	buttonTitleFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonTitleFont );

	buttonLegendFont = new QPushButton();
	buttonLegendFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonLegendFont );

	buttonAxesFont = new QPushButton();
	buttonAxesFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonAxesFont );

	buttonNumbersFont = new QPushButton();
	buttonNumbersFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonNumbersFont );
	fontsLayout->addStretch();

	plotsTabWidget->addTab( plotFonts, QString() );
}

void Plots2DConfigPage::initPrintTab()
{
	plotPrint = new QWidget();
	QVBoxLayout *printLayout = new QVBoxLayout( plotPrint );

	boxScaleLayersOnPrint = new QCheckBox();
	printLayout->addWidget( boxScaleLayersOnPrint );

	boxPrintCropmarks = new QCheckBox();
	printLayout->addWidget( boxPrintCropmarks );
	printLayout->addStretch();

	plotsTabWidget->addTab(plotPrint, QString());
}

void Plots2DConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	axesFont = app->plotAxesFont;
	numbersFont = app->plotNumbersFont;
	legendFont = app->plotLegendFont;
	titleFont = app->plotTitleFont;

	boxAutoscaling->setChecked(app->autoscale2DPlots);
	boxScaleFonts->setChecked(app->autoScaleFonts);
	boxTitle->setChecked(app->titleOn);
	boxFrame->setChecked(app->canvasFrameWidth > 0);
	boxFrameWidth->setValue(app->canvasFrameWidth);
	showFrameWidth(app->canvasFrameWidth > 0);

	boxMargin->setValue(app->defaultPlotMargin);
	legendDisplayBox->setCurrentIndex(app->d_graph_legend_display);
	axisLabelingBox->setCurrentIndex(app->d_graph_axis_labeling);
	attachToBox->setCurrentIndex(app->d_graph_attach_policy);

	boxBackgroundColor->setColor(app->d_graph_background_color);
	boxBackgroundTransparency->setValue(app->d_graph_background_opacity);
	bkgOpacitySlider->setValue(app->d_graph_background_opacity);

	boxCanvasColor->setColor(app->d_graph_canvas_color);
	boxCanvasTransparency->setValue(app->d_graph_canvas_opacity);
	canvasOpacitySlider->setValue(app->d_graph_canvas_opacity);

	boxBorderColor->setColor(app->d_graph_border_color);
	boxBorderWidth->setValue(app->d_graph_border_width);

	boxResize->setChecked(!app->autoResizeLayers);
	boxLabelsEditing->setChecked(!app->d_in_place_editing);
	boxEmptyCellGap->setChecked(!app->d_show_empty_cell_gap);

	boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
	boxSymbolSize->setValue((app->defaultSymbolSize - 1) / 2);
	symbolEdgeBox->setValue(app->defaultSymbolEdge);
	fillSymbolsBox->setChecked(app->d_fill_symbols);
	symbolBox->setCurrentIndex(app->d_symbol_style);
	patternBox->setCurrentIndex(app->defaultCurveBrush);
	curveAlphaBox->setValue(app->defaultCurveAlpha);
	lineStyleBox->setCurrentIndex(app->d_curve_line_style);

	d_indexed_colors = app->indexedColors();
	d_indexed_color_names = app->indexedColorNames();
	setColorsList(d_indexed_colors, d_indexed_color_names);

	groupIndexedSymbols->setChecked(app->d_indexed_symbols);
	d_indexed_symbols = app->indexedSymbols();
	setSymbolsList(d_indexed_symbols);

	boxLineWidth->setValue(app->axesLineWidth);
	boxBackbones->setChecked(app->drawBackbones);
	boxSynchronizeScales->setChecked(app->d_synchronize_graph_scales);
	boxAxesLabelsDist->setValue(app->d_graph_axes_labels_dist);
	boxTickLabelsDist->setValue(app->d_graph_tick_labels_dist);

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		int row = i + 1;
		QLayoutItem *item = enabledAxesGrid->itemAtPosition(row, 2);
		QCheckBox *box = qobject_cast<QCheckBox *>(item->widget());
		if (box) box->setChecked(app->d_show_axes[i]);

		item = enabledAxesGrid->itemAtPosition(row, 3);
		box = qobject_cast<QCheckBox *>(item->widget());
		if (box) box->setChecked(app->d_show_axes_labels[i]);
	}

	boxMajTicksLength->setValue(app->majTicksLength);
	boxMinTicksLength->setValue(app->minTicksLength);
	boxMajTicks->setCurrentIndex(app->majTicksStyle);
	boxMinTicks->setCurrentIndex(app->minTicksStyle);

	Grid *defaultGrid = app->d_default_2D_grid;
	if (defaultGrid){
		boxAntialiseGrid->setChecked(defaultGrid->testRenderHint(QwtPlotItem::RenderAntialiased));
		boxMajorGrid->setChecked(defaultGrid->xEnabled());
		boxColorMajor->setColor(defaultGrid->majPenX().color());
		boxTypeMajor->setStyle(defaultGrid->majPenX().style());
		boxWidthMajor->setValue(defaultGrid->majPenX().widthF());

		boxMinorGrid->setChecked(defaultGrid->xMinEnabled());
		boxColorMinor->setColor(defaultGrid->minPenX().color());
		boxTypeMinor->setStyle(defaultGrid->minPenX().style());
		boxWidthMinor->setValue(defaultGrid->minPenX().widthF());
	}

	unitBox->setCurrentIndex(app->d_layer_geometry_unit);
	FrameWidget::Unit unit = (FrameWidget::Unit)app->d_layer_geometry_unit;
	boxCanvasWidth->setValue(convertFromPixels(app->d_layer_canvas_width, unit, 0));
	boxCanvasHeight->setValue(convertFromPixels(app->d_layer_canvas_height, unit, 1));
	keepRatioBox->setChecked(app->d_keep_aspect_ration);

	antialiasingGroupBox->setChecked(app->antialiasing2DPlots);
	disableAntialiasingBox->setChecked(app->d_disable_curve_antialiasing);
	curveSizeBox->setValue(app->d_curve_max_antialising_size);
	enableCurveAntialiasingSizeBox(app->d_disable_curve_antialiasing);

	speedModeBox->setChecked(app->defaultDecimationMethod() != Graph::NoDecimation || app->speedModeMaxPoints() > 0);
	int decIdx = boxDecimationMethod->findData((int)app->defaultDecimationMethod());
	boxDecimationMethod->setCurrentIndex(decIdx >= 0 ? decIdx : 0);
	boxMaxPoints->setValue(app->speedModeMaxPoints());
	boxDouglasPeukerTolerance->setValue(app->getDouglasPeukerTolerance());
	applySpeedExportBox->setChecked(app->speedModeExport());

	boxScaleLayersOnPrint->setChecked(app->d_scale_plots_on_print);
	boxPrintCropmarks->setChecked(app->d_print_cropmarks);
}

void Plots2DConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	app->d_graph_background_color = boxBackgroundColor->color();
	app->d_graph_background_opacity = boxBackgroundTransparency->value();
	app->d_graph_canvas_color = boxCanvasColor->color();
	app->d_graph_canvas_opacity = boxCanvasTransparency->value();
	app->d_graph_border_color = boxBorderColor->color();
	app->d_graph_border_width = boxBorderWidth->value();

	app->d_in_place_editing = !boxLabelsEditing->isChecked();
	app->d_show_empty_cell_gap = !boxEmptyCellGap->isChecked();
	app->titleOn = boxTitle->isChecked();

	if (boxFrame->isChecked())
		app->canvasFrameWidth = boxFrameWidth->value();
	else
		app->canvasFrameWidth = 0;

	app->defaultPlotMargin = boxMargin->value();
	app->d_graph_axes_labels_dist = boxAxesLabelsDist->value();
	app->d_graph_tick_labels_dist = boxTickLabelsDist->value();
	app->d_graph_legend_display = (Graph::LegendDisplayMode)legendDisplayBox->currentIndex();
	app->d_graph_axis_labeling = (Graph::AxisTitlePolicy)axisLabelingBox->currentIndex();
	app->d_graph_attach_policy = (FrameWidget::AttachPolicy)attachToBox->currentIndex();
	app->setGraphDefaultSettings(boxAutoscaling->isChecked(), boxScaleFonts->isChecked(),
		boxResize->isChecked(), antialiasingGroupBox->isChecked());

	app->defaultCurveStyle = curveStyle();
	app->defaultCurveLineWidth = boxCurveLineWidth->value();
	app->defaultSymbolSize = 2*boxSymbolSize->value() + 1;
	app->setIndexedColors(d_indexed_colors);
	app->setIndexedColorNames(d_indexed_color_names);
	app->d_indexed_symbols = groupIndexedSymbols->isChecked();
	app->d_fill_symbols = fillSymbolsBox->isChecked();
	app->defaultSymbolEdge = symbolEdgeBox->value();
	app->d_symbol_style = symbolBox->currentIndex();
	app->setIndexedSymbols(d_indexed_symbols);
	app->defaultCurveBrush = patternBox->currentIndex();
	app->defaultCurveAlpha = curveAlphaBox->value();
	app->d_curve_line_style = lineStyleBox->currentIndex();

	app->drawBackbones = boxBackbones->isChecked();
	app->axesLineWidth = boxLineWidth->value();
	app->d_synchronize_graph_scales = boxSynchronizeScales->isChecked();

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		int row = i + 1;
		QLayoutItem *item = enabledAxesGrid->itemAtPosition(row, 2);
		QCheckBox *box = qobject_cast<QCheckBox *>(item->widget());
		if (box) app->d_show_axes[i] = box->isChecked();

		item = enabledAxesGrid->itemAtPosition(row, 3);
		box = qobject_cast<QCheckBox *>(item->widget());
		if (box) app->d_show_axes_labels[i] = box->isChecked();
	}

	app->d_layer_geometry_unit = unitBox->currentIndex();
	FrameWidget::Unit unit = (FrameWidget::Unit)unitBox->currentIndex();
	app->d_layer_canvas_width = convertToPixels(boxCanvasWidth->value(), unit, 0);
	app->d_layer_canvas_height = convertToPixels(boxCanvasHeight->value(), unit, 1);
	app->d_keep_aspect_ration = keepRatioBox->isChecked();

	app->majTicksLength = boxMajTicksLength->value();
	app->minTicksLength = boxMinTicksLength->value();
	app->majTicksStyle = boxMajTicks->currentIndex();
	app->minTicksStyle = boxMinTicks->currentIndex();

	app->plotAxesFont = axesFont;
	app->plotNumbersFont = numbersFont;
	app->plotLegendFont = legendFont;
	app->plotTitleFont = titleFont;

	app->d_print_cropmarks = boxPrintCropmarks->isChecked();
	app->d_scale_plots_on_print = boxScaleLayersOnPrint->isChecked();

	app->d_curve_max_antialising_size = curveSizeBox->value();
	app->d_disable_curve_antialiasing = disableAntialiasingBox->isChecked();
	app->setSpeedMaxPoints(boxMaxPoints->value());
	Graph::DecimationMethod decMethod = (Graph::DecimationMethod)boxDecimationMethod->currentData().toInt();
	if (!speedModeBox->isChecked())
		decMethod = Graph::NoDecimation;
	app->setDefaultDecimationMethod(decMethod);
	app->setDouglasPeukerTolerance(speedModeBox->isChecked() ? boxDouglasPeukerTolerance->value() : 0.0);
	app->setSpeedModeExport(applySpeedExportBox->isChecked());

	QList<MdiSubWindow *> windows = app->windowsList();
	for (MdiSubWindow *w : windows){
		MultiLayer *ml = qobject_cast<MultiLayer *>(w);
		if (ml){
			ml->setScaleLayersOnPrint(boxScaleLayersOnPrint->isChecked());
			ml->printCropmarks(boxPrintCropmarks->isChecked()); 
			for (Graph *g : ml->layersList()){
				g->setSynchronizedScaleDivisions(app->d_synchronize_graph_scales);
				g->setAxisTitlePolicy(app->d_graph_axis_labeling);
				g->disableCurveAntialiasing(app->d_disable_curve_antialiasing, app->d_curve_max_antialising_size);
			}
		}
	}
}

void Plots2DConfigPage::retranslateUi()
{
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotOptions), tr("Options"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(curves), tr("Curves"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(axesPage), tr("Axes"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotTicks), tr("Ticks"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(gridPage), tr("Grid"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotGeometryPage), tr("Geometry"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotSpeedPage), tr("Speed"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotFonts), tr("Fonts"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotPrint), tr("Print"));

	boxResize->setText(tr("Do not &resize layers when window size changes"));
    boxLabelsEditing->setText(tr("&Disable in-place editing"));
	boxEmptyCellGap->setText(tr("Co&nnect line across missing data"));
	lblMinTicksLength->setText(tr("Length"));

	lblMajTicksLength->setText(tr("Length" ));
	lblMajTicks->setText(tr("Major Ticks" ));
	lblMinTicks->setText(tr("Minor Ticks" ));

	lblMargin->setText(tr("Margin" ));
	labelGraphAxesLabelsDist->setText(tr("Axes title space" ));
	labelTickLabelsDist->setText(tr("Ticks - Labels space" ));
	boxAxesLabelsDist->setSuffix(" " + tr("pixels"));
	boxTickLabelsDist->setSuffix(" " + tr("pixels"));
	labelFrameWidth->setText(tr("Frame width" ));
	boxFrame->setText(tr("Canvas Fra&me"));
	boxTitle->setText(tr("Show &Title"));
	boxScaleFonts->setText(tr("Scale &Fonts"));
	boxAutoscaling->setText(tr("Auto&scaling"));

	legendDisplayLabel->setText(tr("Legend display" ));
	int curLeg = legendDisplayBox->currentIndex();
	legendDisplayBox->clear();
	legendDisplayBox->addItem(tr("Default"));
	legendDisplayBox->addItem(tr("Column name"));
	legendDisplayBox->addItem(tr("Column comment"));
	legendDisplayBox->addItem(tr("Table name"));
	legendDisplayBox->addItem(tr("Table legend"));
	legendDisplayBox->addItem(tr("Full dataset name"));
	legendDisplayBox->setCurrentIndex(curLeg >= 0 ? curLeg : 0);

	attachToLabel->setText(tr("Attach objects to"));
	int curAtt = attachToBox->currentIndex();
	attachToBox->clear();
	attachToBox->addItem(tr("Frame"));
	attachToBox->addItem(tr("Plot Canvas"));
	attachToBox->addItem(tr("Scales"));
	attachToBox->setCurrentIndex(curAtt >= 0 ? curAtt : 0);

	lblAxisLabeling->setText(tr("Axes Titles" ));
	int curLbl = axisLabelingBox->currentIndex();
	axisLabelingBox->clear();
	axisLabelingBox->addItem(tr("None"));
	axisLabelingBox->addItem(tr("Default"));
	axisLabelingBox->addItem(tr("Column name"));
	axisLabelingBox->addItem(tr("Column comment"));
	axisLabelingBox->addItem(tr("Column comment + name"));
	axisLabelingBox->addItem(tr("Table name + column name"));
	axisLabelingBox->setCurrentIndex(curLbl >= 0 ? curLbl : 0);

	lblCurveStyle->setText(tr("Style"));
	int curSt = boxCurveStyle->currentIndex();
	boxCurveStyle->clear();
	boxCurveStyle->addItem(tr("Line"));
	boxCurveStyle->addItem(tr("Scatter"));
	boxCurveStyle->addItem(tr("Line + Symbol"));
	boxCurveStyle->addItem(tr("Vertical Drop Lines"));
	boxCurveStyle->addItem(tr("Spline"));
	boxCurveStyle->addItem(tr("Steps") + ": " + tr("Vertical"));
	boxCurveStyle->addItem(tr("Steps") + ": " + tr("Horizontal"));
	boxCurveStyle->addItem(tr("Area"));
	boxCurveStyle->addItem(tr("Vertical Bars"));
	boxCurveStyle->addItem(tr("Horizontal Bars"));
	boxCurveStyle->setCurrentIndex(curSt >= 0 ? curSt : 0);

	lblLineWidth->setText(tr("Width"));
	lblLineStyle->setText(tr("Line Style"));
	lblPattern->setText(tr("Area pattern"));
	lblCurveAlpha->setText(tr("Opacity"));
	lblSymbBox->setText(tr("Style"));
	lblSymbSize->setText(tr("Size"));
	lblSymbEdge->setText(tr("Edge width"));
	fillSymbolsBox->setText(tr("Fill color"));

	groupIndexedColors->setTitle(tr("Indexed Colors"));
	btnColorUp->setText(tr("Up"));
	btnColorDown->setText(tr("Down"));
	btnNewColor->setText(tr("Add"));
	btnRemoveColor->setText(tr("Delete"));
	btnLoadDefaultColors->setText(tr("Defaults"));

	groupIndexedSymbols->setTitle(tr("Indexed Symbols"));
	btnSymbolUp->setText(tr("Up"));
	btnSymbolDown->setText(tr("Down"));
	btnLoadDefaultSymbols->setText(tr("Defaults"));

	lblAxesLineWidth->setText(tr("Axes linewidth"));
	boxBackbones->setText(tr("Draw backbones"));
	boxSynchronizeScales->setText(tr("Synchronize scale divisions"));

	int curMaj = boxMajTicks->currentIndex();
	boxMajTicks->clear();
	boxMajTicks->addItem(tr("None"));
	boxMajTicks->addItem(tr("In"));
	boxMajTicks->addItem(tr("Out"));
	boxMajTicks->addItem(tr("Both"));
	boxMajTicks->setCurrentIndex(curMaj >= 0 ? curMaj : 0);

	int curMin = boxMinTicks->currentIndex();
	boxMinTicks->clear();
	boxMinTicks->addItem(tr("None"));
	boxMinTicks->addItem(tr("In"));
	boxMinTicks->addItem(tr("Out"));
	boxMinTicks->addItem(tr("Both"));
	boxMinTicks->setCurrentIndex(curMin >= 0 ? curMin : 0);

	axesGridList->clear();
	axesGridList->addItem(tr("Horizontal"));
	axesGridList->addItem(tr("Vertical"));
	axesGridList->setCurrentRow(0);

	boxMajorGrid->setText(tr("Major Grid"));
	boxMinorGrid->setText(tr("Minor Grid"));
	boxAntialiseGrid->setText(tr("Antialiasing"));

	speedModeBox->setTitle(tr("Speed Mode, Maximum Data Points"));
	maxPointsLabel->setText(tr("Maximum number of points"));
	decimationMethodLabel->setText(tr("Decimation"));
	toleranceLabel->setText(tr("Tolerance"));
	applySpeedExportBox->setText(tr("Apply to &export"));

	antialiasingGroupBox->setTitle(tr("Antialiasing"));
	disableAntialiasingBox->setText(tr("Disable for curves with more than"));
	curveSizeBox->setSuffix(" " + tr("points"));

	buttonTitleFont->setText(tr("&Title"));
	buttonLegendFont->setText(tr("&Legend"));
	buttonAxesFont->setText(tr("&Axes Labels"));
	buttonNumbersFont->setText(tr("&Numbers"));

	boxScaleLayersOnPrint->setText(tr("&Scale layers to paper size"));
	boxPrintCropmarks->setText(tr("Print Crop&marks"));
}

void Plots2DConfigPage::showFrameWidth(bool ok)
{
	if (!ok){
		boxFrameWidth->hide();
		labelFrameWidth->hide();
	} else {
		boxFrameWidth->show();
		labelFrameWidth->show();
	}
}

void Plots2DConfigPage::pickAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, axesFont, this);
	if (ok) axesFont = font;
}

void Plots2DConfigPage::pickNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, numbersFont, this);
	if (ok) numbersFont = font;
}

void Plots2DConfigPage::pickLegendFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, legendFont, this);
	if (ok) legendFont = font;
}

void Plots2DConfigPage::pickTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, titleFont, this);
	if (ok) titleFont = font;
}

int Plots2DConfigPage::curveStyle()
{
	int style = 0;
	switch (boxCurveStyle->currentIndex()){
		case 0: style = Graph::Line; break;
		case 1: style = Graph::Scatter; break;
		case 2: style = Graph::LineSymbols; break;
		case 3: style = Graph::VerticalDropLines; break;
		case 4: style = Graph::Spline; break;
		case 5: style = Graph::VerticalSteps; break;
		case 6: style = Graph::HorizontalSteps; break;
		case 7: style = Graph::Area; break;
		case 8: style = Graph::VerticalBars; break;
		case 9: style = Graph::HorizontalBars; break;
	}
	return style;
}

void Plots2DConfigPage::setSymbolsList(const QList<int>& symbList)
{
	d_indexed_symbols = symbList;
	int rows = symbList.count();
	symbolsList->clearContents();
	symbolsList->setRowCount(rows);
	for (int i = 0; i < rows; i++){
		SymbolBox *sb = new SymbolBox(false);
		sb->setCurrentIndex(symbList[i]);
		connect(sb, &SymbolBox::activated, this, &Plots2DConfigPage::setCurrentSymbol);
		connect(sb, QOverload<int>::of(&QComboBox::activated), this, &Plots2DConfigPage::updateSymbolsList);
		symbolsList->setCellWidget(i, 0, sb);
	}
}

void Plots2DConfigPage::loadDefaultSymbols()
{
	d_indexed_symbols = SymbolBox::defaultSymbols();
	setSymbolsList(d_indexed_symbols);
}

void Plots2DConfigPage::setCurrentSymbol(SymbolBox *sb)
{
	if (!sb)
		return;

	int rows = symbolsList->rowCount();
	for (int i = 0; i < rows; i++){
		SymbolBox *b = qobject_cast<SymbolBox *>(symbolsList->cellWidget(i, 0));
		if (b && sb == b){
			symbolsList->setCurrentCell(i, 0);
			break;
		}
	}
}

void Plots2DConfigPage::updateSymbolsList(int style)
{
	int row = symbolsList->currentRow();
	if (row >= 0 && row < d_indexed_symbols.size())
		d_indexed_symbols[row] = style;
}

void Plots2DConfigPage::moveSymbol(bool up)
{
	int row = symbolsList->currentRow();
	if (row < 0 || row >= d_indexed_symbols.size())
		return;

	int destRow = up ? row - 1 : row + 1;
	if (destRow < 0 || destRow >= d_indexed_symbols.size())
		return;

	d_indexed_symbols.swapItemsAt(row, destRow);
	setSymbolsList(d_indexed_symbols);
	symbolsList->setCurrentCell(destRow, 0);
}

void Plots2DConfigPage::setColorsList(const QList<QColor>& colList, const QStringList& colNames)
{
	d_indexed_colors = colList;
	d_indexed_color_names = colNames;
	int rows = colList.count();
	colorsList->blockSignals(true);
	colorsList->setRowCount(rows);
	for (int i = 0; i < rows; i++){
		QTableWidgetItem *it = new QTableWidgetItem();
		it->setBackground(QBrush(colList[i]));
		it->setFlags(it->flags() & ~Qt::ItemIsEditable);
		colorsList->setItem(i, 0, it);
		colorsList->setItem(i, 1, new QTableWidgetItem(colNames[i]));
	}
	colorsList->blockSignals(false);
}

void Plots2DConfigPage::moveColor(bool up)
{
	int row = colorsList->currentRow();
	if (row < 0 || (up && row == 0) || (!up && row == colorsList->rowCount() - 1))
		return;

	int destRow = up ? row - 1 : row + 1;
	d_indexed_colors.swapItemsAt(row, destRow);
	d_indexed_color_names.swapItemsAt(row, destRow);
	setColorsList(d_indexed_colors, d_indexed_color_names);
	colorsList->setCurrentCell(destRow, 0);
}

void Plots2DConfigPage::removeColor()
{
	int row = colorsList->currentRow();
	if (row < 0 || colorsList->rowCount() <= 1)
		return;

	d_indexed_colors.removeAt(row);
	d_indexed_color_names.removeAt(row);
	setColorsList(d_indexed_colors, d_indexed_color_names);
	colorsList->setCurrentCell(qMin(row, colorsList->rowCount() - 1), 0);
}

void Plots2DConfigPage::newColor()
{
	QColor color = QColorDialog::getColor(Qt::black, this);
	if (!color.isValid())
		return;

	d_indexed_colors.append(color);
	d_indexed_color_names.append(color.name());
	setColorsList(d_indexed_colors, d_indexed_color_names);
	colorsList->setCurrentCell(colorsList->rowCount() - 1, 0);
}

void Plots2DConfigPage::loadDefaultColors()
{
	d_indexed_colors = ColorBox::defaultColors();
	d_indexed_color_names = ColorBox::defaultColorNames();
	setColorsList(d_indexed_colors, d_indexed_color_names);
	colorsList->selectRow(0);
}

void Plots2DConfigPage::showColorDialog(int row, int col)
{
	if (col != 0 || row < 0 || row >= d_indexed_colors.count())
		return;

	QColor c = QColor(d_indexed_colors[row]);
	QColor color = QColorDialog::getColor(c, this);
	if (!color.isValid() || color == c)
		return;

	d_indexed_colors[row] = color;
	setColorsList(d_indexed_colors, d_indexed_color_names);
}

void Plots2DConfigPage::changeColorName(int row, int col)
{
	if (col == 1 && row >= 0 && row < d_indexed_color_names.count())
		d_indexed_color_names[row] = colorsList->item(row, col)->text();
}

void Plots2DConfigPage::showGridOptions(int axis)
{
	if (!d_app)
		return;
	Grid *grid = d_app->d_default_2D_grid;
	if (!grid)
		return;

	if (axis == 0){ // Horizontal
		boxMajorGrid->setChecked(grid->xEnabled());
		boxColorMajor->setColor(grid->majPenX().color());
		boxTypeMajor->setStyle(grid->majPenX().style());
		boxWidthMajor->setValue(grid->majPenX().widthF());

		boxMinorGrid->setChecked(grid->xMinEnabled());
		boxColorMinor->setColor(grid->minPenX().color());
		boxTypeMinor->setStyle(grid->minPenX().style());
		boxWidthMinor->setValue(grid->minPenX().widthF());
	} else { // Vertical
		boxMajorGrid->setChecked(grid->yEnabled());
		boxColorMajor->setColor(grid->majPenY().color());
		boxTypeMajor->setStyle(grid->majPenY().style());
		boxWidthMajor->setValue(grid->majPenY().widthF());

		boxMinorGrid->setChecked(grid->yMinEnabled());
		boxColorMinor->setColor(grid->minPenY().color());
		boxTypeMinor->setStyle(grid->minPenY().style());
		boxWidthMinor->setValue(grid->minPenY().widthF());
	}
	majorGridEnabled(boxMajorGrid->isChecked());
	minorGridEnabled(boxMinorGrid->isChecked());
}

void Plots2DConfigPage::majorGridEnabled(bool on)
{
	boxColorMajor->setEnabled(on);
	boxTypeMajor->setEnabled(on);
	boxWidthMajor->setEnabled(on);
	updateGrid();
}

void Plots2DConfigPage::minorGridEnabled(bool on)
{
	boxColorMinor->setEnabled(on);
	boxTypeMinor->setEnabled(on);
	boxWidthMinor->setEnabled(on);
	updateGrid();
}

void Plots2DConfigPage::updateGrid()
{
	if (!d_app)
		return;
	Grid *grid = d_app->d_default_2D_grid;
	if (!grid)
		return;

	grid->setRenderHint(QwtPlotItem::RenderAntialiased, boxAntialiseGrid->isChecked());

	int axis = axesGridList->currentRow();
	if (axis == 0){ // Horizontal
		grid->enableX(boxMajorGrid->isChecked());
		QPen pen = grid->majPenX();
		pen.setColor(boxColorMajor->color());
		pen.setStyle(boxTypeMajor->style());
		pen.setWidthF(boxWidthMajor->value());
		grid->setMajPenX(pen);

		grid->enableXMin(boxMinorGrid->isChecked());
		pen = grid->minPenX();
		pen.setColor(boxColorMinor->color());
		pen.setStyle(boxTypeMinor->style());
		pen.setWidthF(boxWidthMinor->value());
		grid->setMinPenX(pen);
	} else { // Vertical
		grid->enableY(boxMajorGrid->isChecked());
		QPen pen = grid->majPenY();
		pen.setColor(boxColorMajor->color());
		pen.setStyle(boxTypeMajor->style());
		pen.setWidthF(boxWidthMajor->value());
		grid->setMajPenY(pen);

		grid->enableYMin(boxMinorGrid->isChecked());
		pen = grid->minPenY();
		pen.setColor(boxColorMinor->color());
		pen.setStyle(boxTypeMinor->style());
		pen.setWidthF(boxWidthMinor->value());
		grid->setMinPenY(pen);
	}
}

int Plots2DConfigPage::convertToPixels(double w, FrameWidget::Unit unit, int dimension)
{
	if (!d_app)
		return qRound(w);

	double dpi = (double)d_app->logicalDpiX();
	if (dimension)
		dpi = (double)d_app->logicalDpiY();

	switch(unit){
		case FrameWidget::Pixel:
		default:
			return qRound(w);
		case FrameWidget::Inch:
			return qRound(w*dpi);
		case FrameWidget::Millimeter:
			return qRound(w*dpi/25.4);
		case FrameWidget::Centimeter:
			return qRound(w*dpi/2.54);
		case FrameWidget::Point:
			return qRound(w*dpi/72.0);
	}
}

double Plots2DConfigPage::convertFromPixels(int w, FrameWidget::Unit unit, int dimension)
{
	if (!d_app)
		return w;

	double dpi = (double)d_app->logicalDpiX();
	if (dimension)
		dpi = (double)d_app->logicalDpiY();

	switch(unit){
		case FrameWidget::Pixel:
		default:
			return w;
		case FrameWidget::Inch:
			return (double)w/dpi;
		case FrameWidget::Millimeter:
			return 25.4*w/dpi;
		case FrameWidget::Centimeter:
			return 2.54*w/dpi;
		case FrameWidget::Point:
			return 72.0*w/dpi;
	}
}

void Plots2DConfigPage::updateCanvasSize(int unit)
{
	if (!d_app)
		return;
	FrameWidget::Unit u = (FrameWidget::Unit)unit;
	boxCanvasWidth->blockSignals(true);
	boxCanvasHeight->blockSignals(true);
	boxCanvasWidth->setValue(convertFromPixels(d_app->d_layer_canvas_width, u, 0));
	boxCanvasHeight->setValue(convertFromPixels(d_app->d_layer_canvas_height, u, 1));
	boxCanvasWidth->blockSignals(false);
	boxCanvasHeight->blockSignals(false);
}

void Plots2DConfigPage::adjustCanvasHeight(double width)
{
	if (keepRatioBox->isChecked() && d_app && d_app->d_layer_canvas_width > 0){
		double ratio = (double)d_app->d_layer_canvas_height / (double)d_app->d_layer_canvas_width;
		boxCanvasHeight->blockSignals(true);
		boxCanvasHeight->setValue(width * ratio);
		boxCanvasHeight->blockSignals(false);
	}
}

void Plots2DConfigPage::adjustCanvasWidth(double height)
{
	if (keepRatioBox->isChecked() && d_app && d_app->d_layer_canvas_height > 0){
		double ratio = (double)d_app->d_layer_canvas_width / (double)d_app->d_layer_canvas_height;
		boxCanvasWidth->blockSignals(true);
		boxCanvasWidth->setValue(height * ratio);
		boxCanvasWidth->blockSignals(false);
	}
}

void Plots2DConfigPage::enableCurveAntialiasingSizeBox(bool on)
{
	curveSizeBox->setEnabled(on);
}
