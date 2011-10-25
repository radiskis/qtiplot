/***************************************************************************
    File                 : ConfigDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Preferences dialog

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
#include "ConfigDialog.h"
#include <ApplicationWindow.h>
#include <Note.h>
#include <MultiLayer.h>
#include <Graph.h>
#include <Grid.h>
#include <Matrix.h>
#include <ColorButton.h>
#include <ColorBox.h>
#include <DoubleSpinBox.h>
#include <ColorMapEditor.h>
#include <SymbolBox.h>
#include <PatternBox.h>
#include <PenStyleBox.h>

#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QFontDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStyleFactory>
#include <QRegExp>
#include <QMessageBox>
#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QFontMetrics>
#include <QFileDialog>
#include <QFontComboBox>
#include <QNetworkProxy>
#include <QCompleter>
#include <QDirModel>
#include <QTableWidget>
#include <QColorDialog>

ConfigDialog::ConfigDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled(true);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	d_3D_title_font = app->d_3D_title_font;
	d_3D_numbers_font = app->d_3D_numbers_font;
	d_3D_axes_font = app->d_3D_axes_font;
	textFont = app->tableTextFont;
	headerFont = app->tableHeaderFont;
	appFont = app->appFont;
	axesFont = app->plotAxesFont;
	numbersFont = app->plotNumbersFont;
	legendFont = app->plotLegendFont;
	titleFont = app->plotTitleFont;

	generalDialog = new QStackedWidget();
	itemsList = new QListWidget();
	itemsList->setSpacing(10);
	itemsList->setIconSize(QSize(32, 32));

	initAppPage();
	initTablesPage();
	initPlotsPage();
	initPlots3DPage();
	initNotesPage();
	initFittingPage();

	generalDialog->addWidget(appTabWidget);
	generalDialog->addWidget(tables);
	generalDialog->addWidget(plotsTabWidget);
	generalDialog->addWidget(plots3D);
	generalDialog->addWidget(notesPage);
	generalDialog->addWidget(fitPage);

	QVBoxLayout * rightLayout = new QVBoxLayout();
	lblPageHeader = new QLabel();
	QFont fnt = this->font();
	fnt.setPointSize(fnt.pointSize() + 3);
	fnt.setBold(true);
	lblPageHeader->setFont(fnt);
	lblPageHeader->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QPalette pal = lblPageHeader->palette();
	pal.setColor( QPalette::Window, app->panelsColor );
	lblPageHeader->setPalette(pal);
	lblPageHeader->setAutoFillBackground( true );

	rightLayout->setSpacing(10);
	rightLayout->addWidget( lblPageHeader );
	rightLayout->addWidget( generalDialog );

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);
	topLayout->setMargin(5);
	topLayout->addWidget(itemsList, 0);
	topLayout->addLayout(rightLayout, 1);
	topLayout->addStretch();

	QHBoxLayout * bottomButtons = new QHBoxLayout();
	bottomButtons->addStretch();

	btnDefaultSettings = new QPushButton();
	btnDefaultSettings->setAutoDefault( false );
	bottomButtons->addWidget( btnDefaultSettings );

	buttonApply = new QPushButton();
	buttonApply->setAutoDefault( true );
	bottomButtons->addWidget( buttonApply );

	buttonOk = new QPushButton();
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );
	bottomButtons->addWidget( buttonOk );

	buttonCancel = new QPushButton();
	buttonCancel->setAutoDefault( true );
	bottomButtons->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout( this );
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomButtons);

	languageChange();

	// signals and slots connections
	connect( itemsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentPage(int)));
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( btnDefaultSettings, SIGNAL( clicked() ), this, SLOT( resetDefaultSettings() ) );
	connect( buttonTextFont, SIGNAL( clicked() ), this, SLOT( pickTextFont() ) );
	connect( buttonHeaderFont, SIGNAL( clicked() ), this, SLOT( pickHeaderFont() ) );

	setCurrentPage(0);
}

void ConfigDialog::setCurrentPage(int index)
{
	generalDialog->setCurrentIndex(index);
	if(itemsList->currentItem())
		lblPageHeader->setText(itemsList->currentItem()->text());
}

void ConfigDialog::initTablesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	tables = new QWidget();

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);

	lblSeparator = new QLabel();
	topLayout->addWidget( lblSeparator );
	boxSeparator = new QComboBox();
	boxSeparator->setEditable( true );
	topLayout->addWidget( boxSeparator );

	QString help = tr("The column separator can be customized. \nThe following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: \n0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	lblSeparator->setWhatsThis(help);
	lblSeparator->setToolTip(help);

	groupBoxTableCol = new QGroupBox();
	QGridLayout * colorsLayout = new QGridLayout(groupBoxTableCol);

	lblTableBackground = new QLabel();
	colorsLayout->addWidget( lblTableBackground, 0, 0 );
	buttonBackground= new ColorButton();
	buttonBackground->setColor(app->tableBkgdColor);
	colorsLayout->addWidget( buttonBackground, 0, 1 );

	lblTextColor = new QLabel();
	colorsLayout->addWidget( lblTextColor, 1, 0 );
	buttonText = new ColorButton();
	buttonText->setColor(app->tableTextColor);
	colorsLayout->addWidget( buttonText, 1, 1 );

	lblHeaderColor = new QLabel();
	colorsLayout->addWidget( lblHeaderColor, 2, 0 );
	buttonHeader= new ColorButton();
	buttonHeader->setColor(app->tableHeaderColor);
	colorsLayout->addWidget( buttonHeader, 2, 1 );

	groupBoxTableFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBoxTableFonts );

	buttonTextFont= new QPushButton();
	buttonTextFont->setIcon(QIcon(":/font.png"));
	bottomLayout->addWidget( buttonTextFont );
	buttonHeaderFont= new QPushButton();
	buttonHeaderFont->setIcon(QIcon(":/font.png"));
	bottomLayout->addWidget( buttonHeaderFont );

	boxTableComments = new QCheckBox();
	boxTableComments->setChecked(app->d_show_table_comments);

	boxUpdateTableValues = new QCheckBox();
	boxUpdateTableValues->setChecked(app->autoUpdateTableValues());

	QVBoxLayout * tablesPageLayout = new QVBoxLayout( tables );
	tablesPageLayout->addLayout(topLayout,1);
	tablesPageLayout->addWidget(groupBoxTableCol);
	tablesPageLayout->addWidget(groupBoxTableFonts);
    tablesPageLayout->addWidget(boxTableComments);
	tablesPageLayout->addWidget(boxUpdateTableValues);
	tablesPageLayout->addStretch();
}

void ConfigDialog::initPlotsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	plotsTabWidget = new QTabWidget();
	plotOptions = new QWidget();

	QVBoxLayout * optionsTabLayout = new QVBoxLayout( plotOptions );
	optionsTabLayout->setSpacing(5);

	QGroupBox * groupBoxOptions = new QGroupBox();
	optionsTabLayout->addWidget( groupBoxOptions );

	QGridLayout * optionsLayout = new QGridLayout( groupBoxOptions );

	boxAutoscaling = new QCheckBox();
	boxAutoscaling->setChecked(app->autoscale2DPlots);
	optionsLayout->addWidget( boxAutoscaling, 0, 0);

	boxScaleFonts = new QCheckBox();
	boxScaleFonts->setChecked(app->autoScaleFonts);
	optionsLayout->addWidget( boxScaleFonts, 0, 1);

	boxTitle = new QCheckBox();
	boxTitle->setChecked(app->titleOn);
	optionsLayout->addWidget(boxTitle, 1, 0);

	boxFrame = new QCheckBox();
	boxFrame->setChecked(app->canvasFrameWidth > 0);
	optionsLayout->addWidget(boxFrame, 1, 1);

	labelFrameWidth = new QLabel();
	optionsLayout->addWidget(labelFrameWidth, 4, 0);
	boxFrameWidth= new QSpinBox();
	optionsLayout->addWidget(boxFrameWidth, 4, 1);
	boxFrameWidth->setRange(1, 100);
	boxFrameWidth->setValue(app->canvasFrameWidth);
	if (!app->canvasFrameWidth){
		labelFrameWidth->hide();
		boxFrameWidth->hide();
	}

	lblMargin = new QLabel();
	optionsLayout->addWidget(lblMargin, 5, 0);
	boxMargin = new QSpinBox();
	boxMargin->setRange(0, 1000);
	boxMargin->setSingleStep(5);
	boxMargin->setValue(app->defaultPlotMargin);
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
	boxBackgroundColor->setColor(app->d_graph_background_color);
    graphBackgroundLayout->addWidget(boxBackgroundColor, 0, 1 );

	labelGraphBkgOpacity = new QLabel(tr("Opacity" ));
    graphBackgroundLayout->addWidget(labelGraphBkgOpacity, 0, 2 );
    boxBackgroundTransparency = new QSpinBox();
	boxBackgroundTransparency->setRange(0, 100);
	boxBackgroundTransparency->setSuffix(" %");
    boxBackgroundTransparency->setWrapping(true);
	boxBackgroundTransparency->setSpecialValueText(" " + tr("Transparent"));
	boxBackgroundTransparency->setValue(app->d_graph_background_opacity);

	bkgOpacitySlider = new QSlider();
	bkgOpacitySlider->setOrientation(Qt::Horizontal);
	bkgOpacitySlider->setRange(0, 100);
	bkgOpacitySlider->setValue(app->d_graph_background_opacity);

	connect(bkgOpacitySlider, SIGNAL(valueChanged(int)), boxBackgroundTransparency, SLOT(setValue(int)));
	connect(boxBackgroundTransparency, SIGNAL(valueChanged(int)), bkgOpacitySlider, SLOT(setValue(int)));

	QHBoxLayout* hb = new QHBoxLayout();
	hb->addWidget(bkgOpacitySlider);
	hb->addWidget(boxBackgroundTransparency);
	graphBackgroundLayout->addLayout(hb, 0, 3 );

	labelGraphCanvasColor = new QLabel(tr("Canvas Color" ));
    graphBackgroundLayout->addWidget(labelGraphCanvasColor, 1, 0);
    boxCanvasColor = new ColorButton();
	boxCanvasColor->setColor(app->d_graph_canvas_color);
    graphBackgroundLayout->addWidget( boxCanvasColor, 1, 1 );

	labelGraphCanvasOpacity = new QLabel(tr("Opacity"));
    graphBackgroundLayout->addWidget(labelGraphCanvasOpacity, 1, 2 );
    boxCanvasTransparency = new QSpinBox();
	boxCanvasTransparency->setRange(0, 100);
	boxCanvasTransparency->setSuffix(" %");
    boxCanvasTransparency->setWrapping(true);
	boxCanvasTransparency->setSpecialValueText(" " + tr("Transparent"));
	boxCanvasTransparency->setValue(app->d_graph_canvas_opacity);

	canvasOpacitySlider = new QSlider();
	canvasOpacitySlider->setOrientation(Qt::Horizontal);
	canvasOpacitySlider->setRange(0, 100);
	canvasOpacitySlider->setValue(app->d_graph_canvas_opacity);

	connect(canvasOpacitySlider, SIGNAL(valueChanged(int)), boxCanvasTransparency, SLOT(setValue(int)));
	connect(boxCanvasTransparency, SIGNAL(valueChanged(int)), canvasOpacitySlider, SLOT(setValue(int)));

	QHBoxLayout* hb1 = new QHBoxLayout();
	hb1->addWidget(canvasOpacitySlider);
	hb1->addWidget(boxCanvasTransparency);
	graphBackgroundLayout->addLayout(hb1, 1, 3);

	labelGraphFrameColor = new QLabel(tr("Border Color"));
    graphBackgroundLayout->addWidget(labelGraphFrameColor, 2, 0);
    boxBorderColor = new ColorButton();
	boxBorderColor->setColor(app->d_graph_border_color);
    graphBackgroundLayout->addWidget(boxBorderColor, 2, 1);

	labelGraphFrameWidth = new QLabel(tr( "Width" ));
    graphBackgroundLayout->addWidget(labelGraphFrameWidth, 2, 2);
    boxBorderWidth = new QSpinBox();
	boxBorderWidth->setValue(app->d_graph_border_width);
    graphBackgroundLayout->addWidget(boxBorderWidth, 2, 3);

	graphBackgroundLayout->setRowStretch(4, 1);
	graphBackgroundLayout->setColumnStretch(4, 1);

	boxResize = new QCheckBox();
	boxResize->setChecked(!app->autoResizeLayers);
	optionsTabLayout->addWidget( boxResize );

    boxLabelsEditing = new QCheckBox();
    boxLabelsEditing->setChecked(!app->d_in_place_editing);
    optionsTabLayout->addWidget(boxLabelsEditing);

	boxEmptyCellGap = new QCheckBox();
	boxEmptyCellGap->setChecked(!app->d_show_empty_cell_gap);
	optionsTabLayout->addWidget(boxEmptyCellGap);

	plotsTabWidget->addTab( plotOptions, QString() );

	initCurvesPage();
	plotsTabWidget->addTab( curves, QString() );

	initAxesPage();
	plotsTabWidget->addTab( axesPage, QString() );

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
	boxMajTicksLength->setValue(app->majTicksLength);
	ticksLayout->addWidget( boxMajTicksLength, 0, 3 );

	lblMinTicks = new QLabel();
	ticksLayout->addWidget( lblMinTicks, 1, 0 );
	boxMinTicks = new QComboBox();
	ticksLayout->addWidget( boxMinTicks, 1, 1 );

	lblMinTicksLength = new QLabel();
	ticksLayout->addWidget( lblMinTicksLength, 1, 2 );
	boxMinTicksLength = new QSpinBox();
	boxMinTicksLength->setRange(0, 100);
	boxMinTicksLength->setValue(app->minTicksLength);
	ticksLayout->addWidget( boxMinTicksLength, 1, 3 );

	ticksLayout->setRowStretch( 4, 1 );

	plotsTabWidget->addTab( plotTicks, QString() );

	initGridPage();
	plotsTabWidget->addTab(gridPage, QString());

	initLayerGeometryPage();
	plotsTabWidget->addTab(plotGeometryPage, QString());

	initLayerSpeedPage();
	plotsTabWidget->addTab(plotSpeedPage, QString());

	plotFonts = new QWidget();
	QVBoxLayout * plotFontsLayout = new QVBoxLayout( plotFonts );

	QGroupBox * groupBox2DFonts = new QGroupBox();
	plotFontsLayout->addWidget( groupBox2DFonts );
	QVBoxLayout * fontsLayout = new QVBoxLayout( groupBox2DFonts );
	buttonTitleFont= new QPushButton();
	buttonTitleFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonTitleFont );
	buttonLegendFont= new QPushButton();
	buttonLegendFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonLegendFont );
	buttonAxesFont= new QPushButton();
	buttonAxesFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonAxesFont );
	buttonNumbersFont= new QPushButton();
	buttonNumbersFont->setIcon(QIcon(":/font.png"));
	fontsLayout->addWidget( buttonNumbersFont );
	fontsLayout->addStretch();

	plotsTabWidget->addTab( plotFonts, QString() );

	plotPrint = new QWidget();
	QVBoxLayout *printLayout = new QVBoxLayout( plotPrint );

	boxScaleLayersOnPrint = new QCheckBox();
	boxScaleLayersOnPrint->setChecked(app->d_scale_plots_on_print);
	printLayout->addWidget( boxScaleLayersOnPrint );

	boxPrintCropmarks = new QCheckBox();
	boxPrintCropmarks->setChecked(app->d_print_cropmarks);
	printLayout->addWidget( boxPrintCropmarks );
	printLayout->addStretch();
	plotsTabWidget->addTab(plotPrint, QString());

	connect( boxFrame, SIGNAL( toggled(bool) ), this, SLOT( showFrameWidth(bool) ) );
	connect( buttonAxesFont, SIGNAL( clicked() ), this, SLOT( pickAxesFont() ) );
	connect( buttonNumbersFont, SIGNAL( clicked() ), this, SLOT( pickNumbersFont() ) );
	connect( buttonLegendFont, SIGNAL( clicked() ), this, SLOT( pickLegendFont() ) );
	connect( buttonTitleFont, SIGNAL( clicked() ), this, SLOT( pickTitleFont() ) );
}

void ConfigDialog::showFrameWidth(bool ok)
{
	if (!ok)
	{
		boxFrameWidth->hide();
		labelFrameWidth->hide();
	}
	else
	{
		boxFrameWidth->show();
		labelFrameWidth->show();
	}
}

void ConfigDialog::initPlots3DPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	plots3D = new QWidget();

	QGroupBox * topBox = new QGroupBox();
	QGridLayout * topLayout = new QGridLayout( topBox );
	topLayout->setSpacing(5);

	lblResolution = new QLabel();
	topLayout->addWidget( lblResolution, 0, 0 );
	boxResolution = new QSpinBox();
	boxResolution->setRange(1, 100);
	boxResolution->setValue(app->d_3D_resolution);
	topLayout->addWidget( boxResolution, 0, 1 );
    lblResolution->setBuddy(boxResolution);

    lblFloorStyle = new QLabel();
	topLayout->addWidget( lblFloorStyle, 1, 0 );

    boxProjection = new QComboBox();
	boxProjection->setCurrentIndex(app->d_3D_projection);
	topLayout->addWidget(boxProjection, 1, 1);
	lblFloorStyle->setBuddy(boxProjection);

	boxShowLegend = new QCheckBox();
	boxShowLegend->setChecked(app->d_3D_legend);
	topLayout->addWidget(boxShowLegend, 2, 0);

	boxSmoothMesh = new QCheckBox();
	boxSmoothMesh->setChecked(app->d_3D_smooth_mesh);
	topLayout->addWidget(boxSmoothMesh, 2, 1);

	boxOrthogonal = new QCheckBox();
	boxOrthogonal->setChecked(app->d_3D_orthogonal);
	topLayout->addWidget(boxOrthogonal, 3, 1);

	boxAutoscale3DPlots = new QCheckBox();
	boxAutoscale3DPlots->setChecked(app->d_3D_autoscale);
	topLayout->addWidget(boxAutoscale3DPlots, 3, 0);

    colorMapBox = new QGroupBox();
    QHBoxLayout *colorMapLayout = new QHBoxLayout( colorMapBox );
    colorMapLayout->setMargin(0);
    colorMapLayout->setSpacing(0);

    colorMapEditor = new ColorMapEditor(app->locale());
    colorMapEditor->setColorMap(app->d_3D_color_map);
    colorMapLayout->addWidget(colorMapEditor);

	groupBox3DCol = new QGroupBox();
	QGridLayout * middleLayout = new QGridLayout( groupBox3DCol );
	btnAxes = new ColorButton();
	btnAxes->setColor(app->d_3D_axes_color);
	middleLayout->addWidget(btnAxes, 0, 0);
	btnLabels = new ColorButton();
	btnLabels->setColor(app->d_3D_labels_color);
	middleLayout->addWidget(btnLabels, 0, 1);
	btnNumbers = new ColorButton();
	btnNumbers->setColor(app->d_3D_numbers_color);
	middleLayout->addWidget(btnNumbers, 0, 2);
	btnMesh = new ColorButton();
	btnMesh->setColor(app->d_3D_mesh_color);
	middleLayout->addWidget(btnMesh, 1, 0);
	btnBackground3D = new ColorButton();
	btnBackground3D->setColor(app->d_3D_background_color);
	middleLayout->addWidget(btnBackground3D, 1, 1);

    groupBox3DFonts = new QGroupBox();
	QGridLayout * fl = new QGridLayout( groupBox3DFonts );
	btnTitleFnt = new QPushButton();
	btnTitleFnt->setIcon(QIcon(":/font.png"));
	fl->addWidget( btnTitleFnt, 0, 0);
	btnLabelsFnt = new QPushButton();
	btnLabelsFnt->setIcon(QIcon(":/font.png"));
	fl->addWidget( btnLabelsFnt, 0, 1);
	btnNumFnt = new QPushButton();
	btnNumFnt->setIcon(QIcon(":/font.png"));
	fl->addWidget( btnNumFnt, 0, 2);
	fl->setRowStretch(1, 1);

	groupBox3DGrids = new QGroupBox(tr("Grids"));
	QGridLayout *gl1 = new QGridLayout(groupBox3DGrids);

	boxMajorGrids = new QCheckBox(tr("Ma&jor Grids"));
	boxMajorGrids->setChecked(app->d_3D_major_grids);
	connect(boxMajorGrids, SIGNAL(toggled(bool)), this, SLOT(enableMajorGrids(bool)));
	gl1->addWidget(boxMajorGrids, 0, 1);

	boxMinorGrids = new QCheckBox(tr("Mi&nor Grids"));
	boxMinorGrids->setChecked(app->d_3D_minor_grids);
	connect(boxMinorGrids, SIGNAL(toggled(bool)), this, SLOT(enableMinorGrids(bool)));
	gl1->addWidget(boxMinorGrids, 0, 2);

	label3DGridsColor = new QLabel(tr("Color"));
    gl1->addWidget(label3DGridsColor, 1, 0);

	btnGrid = new ColorButton();
	btnGrid->setColor(app->d_3D_grid_color);
    gl1->addWidget(btnGrid, 1, 1);

    btnGridMinor = new ColorButton();
    btnGridMinor->setColor(app->d_3D_minor_grid_color);
    gl1->addWidget(btnGridMinor, 1, 2);

	label3DGridsStyle = new QLabel(tr("Style"));
    gl1->addWidget(label3DGridsStyle, 2, 0);

	boxMajorGridStyle = new QComboBox();
	gl1->addWidget(boxMajorGridStyle, 2, 1);

	boxMinorGridStyle = new QComboBox();
	gl1->addWidget(boxMinorGridStyle, 2, 2);

	label3DGridsWidth = new QLabel(tr("Width"));
	gl1->addWidget(label3DGridsWidth, 3, 0);

	boxMajorGridWidth = new DoubleSpinBox();
	boxMajorGridWidth->setLocale(app->locale());
	boxMajorGridWidth->setMinimum(0.0);
	boxMajorGridWidth->setValue(app->d_3D_major_width);
	gl1->addWidget(boxMajorGridWidth, 3, 1);

	boxMinorGridWidth = new DoubleSpinBox();
	boxMinorGridWidth->setLocale(app->locale());
	boxMinorGridWidth->setMinimum(0.0);
	boxMinorGridWidth->setValue(app->d_3D_minor_width);
	gl1->addWidget(boxMinorGridWidth, 3, 2);

	gl1->setRowStretch(4, 1);
	gl1->setColumnStretch(3, 1);

	enableMajorGrids(app->d_3D_major_grids);
	enableMinorGrids(app->d_3D_minor_grids);

    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(groupBox3DCol);
    vl->addWidget(groupBox3DFonts);
    vl->addWidget(groupBox3DGrids);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->addWidget(colorMapBox);
    hb->addLayout(vl);

	QVBoxLayout * plots3DPageLayout = new QVBoxLayout( plots3D );
	plots3DPageLayout->addWidget(topBox);
	plots3DPageLayout->addLayout(hb);

	connect( btnNumFnt, SIGNAL( clicked() ), this, SLOT(pick3DNumbersFont() ) );
	connect( btnTitleFnt, SIGNAL( clicked() ), this, SLOT(pick3DTitleFont() ) );
	connect( btnLabelsFnt, SIGNAL( clicked() ), this, SLOT(pick3DAxesFont() ) );
}

void ConfigDialog::initAppPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	appTabWidget = new QTabWidget(generalDialog);
	appTabWidget->setUsesScrollButtons(false);

	application = new QWidget();
	QVBoxLayout * applicationLayout = new QVBoxLayout( application );
	QGroupBox * groupBoxApp = new QGroupBox();
	applicationLayout->addWidget(groupBoxApp);
	QGridLayout * topBoxLayout = new QGridLayout( groupBoxApp );

	lblLanguage = new QLabel();
	topBoxLayout->addWidget( lblLanguage, 0, 0 );
	boxLanguage = new QComboBox();
	insertLanguagesList();
	topBoxLayout->addWidget( boxLanguage, 0, 1 );

	lblStyle = new QLabel();
	topBoxLayout->addWidget( lblStyle, 1, 0 );
	boxStyle = new QComboBox();
	topBoxLayout->addWidget( boxStyle, 1, 1 );
	QStringList styles = QStyleFactory::keys();
	styles.sort();
	boxStyle->addItems(styles);
	boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle,Qt::MatchWildcard));

	lblFonts = new QLabel();
	topBoxLayout->addWidget( lblFonts, 2, 0 );
	fontsBtn= new QPushButton();
	topBoxLayout->addWidget( fontsBtn, 2, 1 );

	lblScriptingLanguage = new QLabel();
	topBoxLayout->addWidget( lblScriptingLanguage, 3, 0 );
	boxScriptingLanguage = new QComboBox();
	QStringList llist = ScriptingLangManager::languages();
	boxScriptingLanguage->insertStringList(llist);
	boxScriptingLanguage->setCurrentItem(llist.indexOf(app->defaultScriptingLang));
	topBoxLayout->addWidget( boxScriptingLanguage, 3, 1 );

    lblUndoStackSize = new QLabel();
	topBoxLayout->addWidget( lblUndoStackSize, 4, 0 );
    undoStackSizeBox = new QSpinBox();
    undoStackSizeBox->setValue(app->matrixUndoStackSize());
    topBoxLayout->addWidget( undoStackSizeBox, 4, 1 );

	lblEndOfLine = new QLabel();
	topBoxLayout->addWidget(lblEndOfLine, 5, 0 );
	boxEndLine = new QComboBox();
	topBoxLayout->addWidget(boxEndLine, 5, 1);

	lblInitWindow = new QLabel();
	topBoxLayout->addWidget( lblInitWindow, 6, 0 );
	boxInitWindow = new QComboBox();
	topBoxLayout->addWidget( boxInitWindow, 6, 1 );

    boxSave= new QCheckBox();
	boxSave->setChecked(app->autoSave);
	topBoxLayout->addWidget( boxSave, 7, 0 );

	boxMinutes = new QSpinBox();
	boxMinutes->setRange(1, 100);
	boxMinutes->setValue(app->autoSaveTime);
	boxMinutes->setEnabled(app->autoSave);
	topBoxLayout->addWidget( boxMinutes, 7, 1 );

    boxBackupProject = new QCheckBox();
	boxBackupProject->setChecked(app->d_backup_files);
	topBoxLayout->addWidget( boxBackupProject, 8, 0, 1, 2 );

	boxSearchUpdates = new QCheckBox();
	boxSearchUpdates->setChecked(app->autoSearchUpdates);
	topBoxLayout->addWidget( boxSearchUpdates, 9, 0, 1, 2 );

    completionBox = new QCheckBox();
	completionBox->setChecked(app->d_completion);
	topBoxLayout->addWidget(completionBox, 10, 0);

	openLastProjectBox = new QCheckBox();
	openLastProjectBox->setChecked(app->d_open_last_project);
	topBoxLayout->addWidget(openLastProjectBox, 11, 0);

	topBoxLayout->setRowStretch(12, 1);

	excelImportMethodLabel = new QLabel;
	topBoxLayout->addWidget(excelImportMethodLabel, 12, 0);
	excelImportMethod = new QComboBox;
	topBoxLayout->addWidget(excelImportMethod, 12, 1);
	topBoxLayout->setRowStretch(13, 1);

	appTabWidget->addTab(application, QString());

	initConfirmationsPage();

	appTabWidget->addTab( confirm, QString() );

	appColors = new QWidget();
	QVBoxLayout * appColorsLayout = new QVBoxLayout( appColors );
	QGroupBox * groupBoxAppCol = new QGroupBox();
	appColorsLayout->addWidget( groupBoxAppCol );
	QGridLayout * colorsBoxLayout = new QGridLayout( groupBoxAppCol );

	lblWorkspace = new QLabel();
	colorsBoxLayout->addWidget( lblWorkspace, 0, 0 );
	btnWorkspace = new ColorButton();
	btnWorkspace->setColor(app->workspaceColor);
	colorsBoxLayout->addWidget( btnWorkspace, 0, 1 );

	lblPanels = new QLabel();
	colorsBoxLayout->addWidget( lblPanels, 1, 0 );
	btnPanels = new ColorButton();
	colorsBoxLayout->addWidget( btnPanels, 1, 1 );
	btnPanels->setColor(app->panelsColor);

	lblPanelsText = new QLabel();
	colorsBoxLayout->addWidget( lblPanelsText, 2, 0 );
	btnPanelsText = new ColorButton();
	colorsBoxLayout->addWidget( btnPanelsText, 2, 1 );
	btnPanelsText->setColor(app->panelsTextColor);

	colorsBoxLayout->setRowStretch( 3, 1 );

	appTabWidget->addTab( appColors, QString() );

	numericFormatPage = new QWidget();
	QVBoxLayout *numLayout = new QVBoxLayout( numericFormatPage );
	QGroupBox *numericFormatBox = new QGroupBox();
	numLayout->addWidget( numericFormatBox );
	QGridLayout *numericFormatLayout = new QGridLayout( numericFormatBox );

	lblAppPrecision = new QLabel();
	numericFormatLayout->addWidget(lblAppPrecision, 0, 0);
	boxAppPrecision = new QSpinBox();
	boxAppPrecision->setRange(0, 14);
	boxAppPrecision->setValue(app->d_decimal_digits);
	numericFormatLayout->addWidget(boxAppPrecision, 0, 1);

    lblDecimalSeparator = new QLabel();
    numericFormatLayout->addWidget(lblDecimalSeparator, 1, 0 );
	boxDecimalSeparator = new QComboBox();
	boxDecimalSeparator->addItem(tr("System Locale Setting"));
	boxDecimalSeparator->addItem("1,000.0");
	boxDecimalSeparator->addItem("1.000,0");
	boxDecimalSeparator->addItem("1 000,0");

	numericFormatLayout->addWidget(boxDecimalSeparator, 1, 1);

    boxThousandsSeparator = new QCheckBox();
    boxThousandsSeparator->setChecked(app->locale().numberOptions() & QLocale::OmitGroupSeparator);
    numericFormatLayout->addWidget(boxThousandsSeparator, 1, 2);

	lblClipboardSeparator = new QLabel();
    numericFormatLayout->addWidget(lblClipboardSeparator, 2, 0 );
	boxClipboardLocale = new QComboBox();
	boxClipboardLocale->addItem(tr("System Locale Setting"));
	boxClipboardLocale->addItem("1,000.0");
	boxClipboardLocale->addItem("1.000,0");
	boxClipboardLocale->addItem("1 000,0");
	numericFormatLayout->addWidget(boxClipboardLocale, 2, 1);

	boxMuParserCLocale = new QCheckBox();
	boxMuParserCLocale->setChecked(app->d_muparser_c_locale);
	numericFormatLayout->addWidget(boxMuParserCLocale, 3, 0);

	numericFormatLayout->setRowStretch(4, 1);
	appTabWidget->addTab( numericFormatPage, QString() );

	initFileLocationsPage();
	initProxyPage();

	connect( boxLanguage, SIGNAL( activated(int) ), this, SLOT( switchToLanguage(int) ) );
	connect( fontsBtn, SIGNAL( clicked() ), this, SLOT( pickApplicationFont() ) );
	connect( boxSave, SIGNAL( toggled(bool) ), boxMinutes, SLOT( setEnabled(bool) ) );
}

void ConfigDialog::initNotesPage()
{
    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	notesPage = new QWidget();

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout * gl1 = new QGridLayout(gb1);
	gl1->setSpacing(5);

	labelTabLength = new QLabel();
	gl1->addWidget(labelTabLength, 0, 0);

	boxTabLength = new QSpinBox();
	boxTabLength->setRange(0, 1000);
	boxTabLength->setSingleStep(5);
	boxTabLength->setValue(app->d_notes_tab_length);
    connect(boxTabLength, SIGNAL(valueChanged(int)), this, SLOT(customizeNotes()));
	gl1->addWidget(boxTabLength, 0, 1);

	labelNotesFont = new QLabel();
	gl1->addWidget(labelNotesFont, 1, 0);

    boxFontFamily = new QFontComboBox();
    boxFontFamily->setCurrentFont(app->d_notes_font);
    connect(boxFontFamily, SIGNAL(activated(int)), this, SLOT(customizeNotes()));
    gl1->addWidget(boxFontFamily, 1, 1);

    boxFontSize = new QSpinBox();
	boxFontSize->setRange(0, 1000);
	boxFontSize->setValue(app->d_notes_font.pointSize());
    connect(boxFontSize, SIGNAL(valueChanged(int)), this, SLOT(customizeNotes()));
	gl1->addWidget(boxFontSize, 1, 2);

    buttonBoldFont = new QPushButton(tr("&B"));
    QFont font = QFont();
    font.setBold(true);
    buttonBoldFont->setFont(font);
    buttonBoldFont->setCheckable(true);
    buttonBoldFont->setChecked(app->d_notes_font.bold());
    connect(buttonBoldFont, SIGNAL(clicked()), this, SLOT(customizeNotes()));
	gl1->addWidget(buttonBoldFont, 1, 3);

    buttonItalicFont = new QPushButton(tr("&It"));
    font = QFont();
    font.setItalic(true);
    buttonItalicFont->setFont(font);
    buttonItalicFont->setCheckable(true);
    buttonItalicFont->setChecked(app->d_notes_font.italic());
    connect(buttonItalicFont, SIGNAL(clicked()), this, SLOT(customizeNotes()));
	gl1->addWidget(buttonItalicFont, 1, 4);

    lineNumbersBox = new QCheckBox();
	lineNumbersBox->setChecked(app->d_note_line_numbers);
    connect(lineNumbersBox, SIGNAL(toggled(bool)), this, SLOT(customizeNotes()));
	gl1->addWidget(lineNumbersBox, 2, 0);
	gl1->setColumnStretch(5, 1);

	QVBoxLayout* vl = new QVBoxLayout(notesPage);
	vl->addWidget(gb1);

	groupSyntaxHighlighter = new QGroupBox();
	QGridLayout *gl = new QGridLayout(groupSyntaxHighlighter);

	buttonCommentLabel = new QLabel;
	gl->addWidget(buttonCommentLabel, 0, 0);

	buttonCommentColor = new ColorButton();
	buttonCommentColor->setColor(app->d_comment_highlight_color);
	connect(buttonCommentColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonCommentColor, 0, 1);
	buttonCommentLabel->setBuddy (buttonCommentColor);

	buttonKeywordLabel = new QLabel;
	gl->addWidget(buttonKeywordLabel, 1, 0);

	buttonKeywordColor = new ColorButton();
	buttonKeywordColor->setColor(app->d_keyword_highlight_color);
	connect(buttonKeywordColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonKeywordColor, 1, 1);
	buttonKeywordLabel->setBuddy (buttonKeywordColor);

	buttonQuotationLabel = new QLabel;
	gl->addWidget(buttonQuotationLabel, 2, 0);

	buttonQuotationColor = new ColorButton();
	buttonQuotationColor->setColor(app->d_quotation_highlight_color);
	connect(buttonQuotationColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonQuotationColor, 2, 1);
	buttonQuotationLabel->setBuddy (buttonQuotationColor);

	buttonNumericLabel = new QLabel;
	gl->addWidget(buttonNumericLabel, 3, 0);

	buttonNumericColor = new ColorButton();
	buttonNumericColor->setColor(app->d_numeric_highlight_color);
	connect(buttonNumericColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonNumericColor, 3, 1);
	buttonNumericLabel->setBuddy (buttonNumericColor);

	buttonFunctionLabel = new QLabel;
	gl->addWidget(buttonFunctionLabel, 4, 0);

	buttonFunctionColor = new ColorButton();
	buttonFunctionColor->setColor(app->d_function_highlight_color);
	connect(buttonFunctionColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonFunctionColor, 4, 1);
	buttonFunctionLabel->setBuddy (buttonFunctionColor);

	buttonClassLabel = new QLabel;
	gl->addWidget(buttonClassLabel, 5, 0);

	buttonClassColor = new ColorButton();
	buttonClassColor->setColor(app->d_class_highlight_color);
	connect(buttonClassColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonClassColor, 5, 1);
	buttonClassLabel->setBuddy (buttonClassColor);
	gl->setRowStretch(6, 1);

	vl->addWidget(groupSyntaxHighlighter);

	vl->addStretch();
}

void ConfigDialog::initFittingPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	fitPage = new QWidget();

	groupBoxFittingCurve = new QGroupBox();
	QGridLayout * fittingCurveLayout = new QGridLayout(groupBoxFittingCurve);
	fittingCurveLayout->setSpacing(5);

	generatePointsBtn = new QRadioButton();
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(generatePointsBtn, 0, 0);

	lblPoints = new QLabel();
	fittingCurveLayout->addWidget(lblPoints, 0, 1);
	generatePointsBox = new QSpinBox();
	generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(10);
	generatePointsBox->setValue(app->fitPoints);
	fittingCurveLayout->addWidget(generatePointsBox, 0, 2);

    linearFit2PointsBox = new QCheckBox();
    linearFit2PointsBox->setChecked(app->d_2_linear_fit_points);
    fittingCurveLayout->addWidget(linearFit2PointsBox, 0, 3);

	showPointsBox(!app->generateUniformFitPoints);

	samePointsBtn = new QRadioButton();
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(samePointsBtn, 1, 0);

	groupBoxMultiPeak = new QGroupBox();
	groupBoxMultiPeak->setCheckable(true);
	groupBoxMultiPeak->setChecked(app->generatePeakCurves);

	QHBoxLayout * multiPeakLayout = new QHBoxLayout(groupBoxMultiPeak);

	lblPeaksColor = new QLabel();
	multiPeakLayout->addWidget(lblPeaksColor);
	boxPeaksColor = new ColorButton();
	boxPeaksColor->setColor(app->peakCurvesColor);
	multiPeakLayout->addWidget(boxPeaksColor);

	groupBoxFitParameters = new QGroupBox();
	QGridLayout * fitParamsLayout = new QGridLayout(groupBoxFitParameters);

	lblPrecision = new QLabel();
	fitParamsLayout->addWidget(lblPrecision, 0, 0);
	boxPrecision = new QSpinBox();
	fitParamsLayout->addWidget(boxPrecision, 0, 1);
	boxPrecision->setValue(app->fit_output_precision);

	logBox = new QCheckBox();
	logBox->setChecked(app->writeFitResultsToLog);
	fitParamsLayout->addWidget(logBox, 1, 0);

	plotLabelBox = new QCheckBox();
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	fitParamsLayout->addWidget(plotLabelBox, 2, 0);

	scaleErrorsBox = new QCheckBox();
	fitParamsLayout->addWidget(scaleErrorsBox, 3, 0);
	scaleErrorsBox->setChecked(app->fit_scale_errors);

	boxMultiPeakMsgs = new QCheckBox();
	boxMultiPeakMsgs->setChecked(app->d_multi_peak_messages);

	QVBoxLayout* fitPageLayout = new QVBoxLayout(fitPage);
	fitPageLayout->addWidget(groupBoxFittingCurve);
	fitPageLayout->addWidget(groupBoxMultiPeak);
	fitPageLayout->addWidget(groupBoxFitParameters);
	fitPageLayout->addWidget(boxMultiPeakMsgs);
	fitPageLayout->addStretch();

	connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
}

void ConfigDialog::initLayerSpeedPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	plotSpeedPage = new QWidget();

	antialiasingGroupBox = new QGroupBox();
	antialiasingGroupBox->setCheckable(true);
	antialiasingGroupBox->setChecked(app->antialiasing2DPlots);

	disableAntialiasingBox = new QCheckBox();
	disableAntialiasingBox->setChecked(app->d_disable_curve_antialiasing);

	curveSizeBox = new QSpinBox();
	curveSizeBox->setMinimum(2);
	curveSizeBox->setMaximum(INT_MAX);
	curveSizeBox->setValue(app->d_curve_max_antialising_size);

	connect(disableAntialiasingBox, SIGNAL(toggled(bool)), curveSizeBox, SLOT(setEnabled(bool)));
	connect(antialiasingGroupBox, SIGNAL(toggled(bool)), this, SLOT(enableCurveAntialiasingSizeBox(bool)));

	QGridLayout * gl = new QGridLayout(antialiasingGroupBox);
	gl->addWidget(disableAntialiasingBox, 0, 0);
	gl->addWidget(curveSizeBox, 0, 1);
	gl->setRowStretch(1, 1);

	QHBoxLayout * hl = new QHBoxLayout(plotSpeedPage);
	hl->addWidget(antialiasingGroupBox);
}

void ConfigDialog::enableCurveAntialiasingSizeBox(bool on)
{
	curveSizeBox->setEnabled(on && disableAntialiasingBox->isChecked());
}

void ConfigDialog::initLayerGeometryPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	plotGeometryPage = new QWidget();

	QGroupBox * gb = new QGroupBox();
	QGridLayout * gl = new QGridLayout( gb );

	unitBoxLabel = new QLabel();
	gl->addWidget(unitBoxLabel, 0, 0);

	unitBox = new QComboBox();
	connect(unitBox, SIGNAL(activated(int)), this, SLOT(updateCanvasSize(int)));
	gl->addWidget(unitBox, 0, 1);

	QLocale locale = QLocale();

	canvasWidthLabel = new QLabel();
	gl->addWidget(canvasWidthLabel, 1, 0);

	FrameWidget::Unit unit = (FrameWidget::Unit)app->d_layer_geometry_unit;

	boxCanvasWidth = new DoubleSpinBox();
	boxCanvasWidth->setRange(0, 10000);
	boxCanvasWidth->setLocale(locale);
	boxCanvasWidth->setDecimals(6);
	//boxCanvasWidth->setValue(convertFromPixels(app->d_layer_canvas_width, unit,0));
	connect(boxCanvasWidth, SIGNAL(valueChanged (double)), this, SLOT(adjustCanvasHeight(double)));
	gl->addWidget(boxCanvasWidth, 1, 1);

	canvasHeightLabel = new QLabel();
	gl->addWidget(canvasHeightLabel, 2, 0);

	boxCanvasHeight = new DoubleSpinBox();
	boxCanvasHeight->setRange(0, 10000);
	boxCanvasHeight->setLocale(locale);
	boxCanvasHeight->setDecimals(6);
	//boxCanvasHeight->setValue(convertFromPixels(app->d_layer_canvas_height, unit, 1));
	connect(boxCanvasHeight, SIGNAL(valueChanged (double)), this, SLOT(adjustCanvasWidth(double)));
	gl->addWidget(boxCanvasHeight, 2, 1);

	keepRatioBox = new QCheckBox(tr("&Keep aspect ratio"));
	keepRatioBox->setChecked(app->d_keep_aspect_ration);
	gl->addWidget(keepRatioBox, 3, 1);

	gl->setRowStretch(4, 1);

	updateCanvasSize(unit);

	QHBoxLayout * hl = new QHBoxLayout(plotGeometryPage);
	hl->addWidget(gb);
}

void ConfigDialog::initCurvesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	curves = new QWidget();

	curvesGroupBox = new QGroupBox();
	QGridLayout * curvesBoxLayout = new QGridLayout( curvesGroupBox );

	lblCurveStyle = new QLabel();
	curvesBoxLayout->addWidget( lblCurveStyle, 0, 0 );
	boxCurveStyle = new QComboBox();
	curvesBoxLayout->addWidget( boxCurveStyle, 0, 1 );
	lblCurveStyle->setBuddy(boxCurveStyle);

	lblLineWidth = new QLabel();
	curvesBoxLayout->addWidget( lblLineWidth, 1, 0 );
	boxCurveLineWidth = new DoubleSpinBox('f');
	boxCurveLineWidth->setLocale(app->locale());
	boxCurveLineWidth->setSingleStep(0.1);
	boxCurveLineWidth->setRange(0.1, 100);
	boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
	curvesBoxLayout->addWidget( boxCurveLineWidth, 1, 1 );
	lblLineWidth->setBuddy(boxCurveLineWidth);

	lblLineStyle = new QLabel();
	curvesBoxLayout->addWidget(lblLineStyle, 2, 0);
	lineStyleBox = new PenStyleBox();
	lineStyleBox->setCurrentIndex(app->d_curve_line_style);
	curvesBoxLayout->addWidget(lineStyleBox, 2, 1);

	fillCurvesGroupBox = new QGroupBox();
	QGridLayout * fillCurvesBoxLayout = new QGridLayout( fillCurvesGroupBox );

	lblPattern = new QLabel();
	fillCurvesBoxLayout->addWidget(lblPattern, 0, 0);
	patternBox = new PatternBox();
	patternBox->setCurrentIndex(app->defaultCurveBrush);
	fillCurvesBoxLayout->addWidget(patternBox, 0, 1);

	lblPattern->setBuddy(patternBox);

	curveOpacitySlider = new QSlider();
	curveOpacitySlider->setOrientation(Qt::Horizontal);
	curveOpacitySlider->setRange(0, 100);
	curveOpacitySlider->setValue(app->defaultCurveAlpha);

	curveAlphaBox = new QSpinBox();
	curveAlphaBox->setRange(0, 100);
	curveAlphaBox->setWrapping(true);
	curveAlphaBox->setSuffix(" %");
	curveAlphaBox->setValue(app->defaultCurveAlpha);

	connect(curveOpacitySlider, SIGNAL(valueChanged(int)), curveAlphaBox, SLOT(setValue(int)));
	connect(curveAlphaBox, SIGNAL(valueChanged(int)), curveOpacitySlider, SLOT(setValue(int)));

	QHBoxLayout* hb1 = new QHBoxLayout();
	hb1->addWidget(curveOpacitySlider);
	hb1->addWidget(curveAlphaBox);
	fillCurvesBoxLayout->addLayout(hb1, 1, 1);

	lblCurveAlpha = new QLabel();
	lblCurveAlpha->setBuddy(curveAlphaBox);
	fillCurvesBoxLayout->addWidget(lblCurveAlpha, 1, 0);

	QHBoxLayout *hl0 = new QHBoxLayout();
	hl0->addWidget(curvesGroupBox);
	hl0->addWidget(fillCurvesGroupBox);

	symbolGroupBox = new QGroupBox();
	QGridLayout * symbLayout = new QGridLayout(symbolGroupBox);

	lblSymbBox = new QLabel();
	symbLayout->addWidget(lblSymbBox, 0, 0);

	symbolBox = new SymbolBox();
	symbolBox->setCurrentIndex(app->d_symbol_style);
	symbolBox->setDisabled(app->d_indexed_symbols);
	symbLayout->addWidget(symbolBox, 0, 1);

	lblSymbSize = new QLabel();
	symbLayout->addWidget( lblSymbSize, 1, 0 );
	boxSymbolSize = new QSpinBox();
	boxSymbolSize->setRange(1,100);
	boxSymbolSize->setValue(app->defaultSymbolSize/2);
	symbLayout->addWidget( boxSymbolSize, 1, 1 );

	lblSymbSize->setBuddy(boxSymbolSize);

	lblSymbEdge = new QLabel();
	symbLayout->addWidget(lblSymbEdge, 0, 2);

	symbolEdgeBox = new DoubleSpinBox('f');
	symbolEdgeBox->setLocale(app->locale());
	symbolEdgeBox->setSingleStep(0.1);
	symbolEdgeBox->setRange(0.1, 100);
	symbolEdgeBox->setValue(app->defaultSymbolEdge);
	symbLayout->addWidget(symbolEdgeBox, 0, 3);

	lblSymbEdge->setBuddy(symbolEdgeBox);

	fillSymbolsBox = new QCheckBox();
	fillSymbolsBox->setChecked(app->d_fill_symbols);
	symbLayout->addWidget(fillSymbolsBox, 1, 3);

	QVBoxLayout *curvesPageLayout = new QVBoxLayout(curves);
	curvesPageLayout->addLayout(hl0);
	curvesPageLayout->addWidget(symbolGroupBox);

	colorsList = new QTableWidget();
	colorsList->setColumnCount(2);
	colorsList->horizontalHeader()->setClickable(false);
	colorsList->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
	colorsList->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
	colorsList->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	connect(colorsList, SIGNAL(cellClicked(int, int)), this, SLOT(showColorDialog(int, int)));
	connect(colorsList, SIGNAL(cellChanged(int, int)), this, SLOT(changeColorName(int, int)));

	d_indexed_colors = app->indexedColors();
	d_indexed_color_names = app->indexedColorNames();
	setColorsList(d_indexed_colors, d_indexed_color_names);

	QHBoxLayout *hl2 = new QHBoxLayout();
	hl2->setSpacing(0);
	hl2->addStretch();
	btnNewColor = new QPushButton(QIcon(":/plus.png"), QString::null);
	connect(btnNewColor, SIGNAL(clicked()), this, SLOT(newColor()));
	hl2->addWidget(btnNewColor);

	btnRemoveColor = new QPushButton(QIcon(":/delete.png"), QString::null);
	connect(btnRemoveColor, SIGNAL(clicked()), this, SLOT(removeColor()));
	hl2->addWidget(btnRemoveColor);

	btnColorUp = new QPushButton(QIcon(":/arrow_up.png"), QString::null);
	connect(btnColorUp, SIGNAL(clicked()), this, SLOT(moveColor()));
	hl2->addWidget(btnColorUp);

	btnColorDown = new QPushButton(QIcon(":/arrow_down.png"), QString::null);
	connect(btnColorDown, SIGNAL(clicked()), this, SLOT(moveColorDown()));
	hl2->addWidget(btnColorDown);

	btnLoadDefaultColors = new QPushButton();
	connect(btnLoadDefaultColors, SIGNAL(clicked()), this, SLOT(loadDefaultColors()));
	hl2->addWidget(btnLoadDefaultColors);

	hl2->addStretch();

	groupIndexedColors = new QGroupBox();
	QVBoxLayout *vl = new QVBoxLayout(groupIndexedColors);
	vl->addWidget(colorsList);
	vl->addLayout(hl2);

	symbolsList = new QTableWidget();
	symbolsList->setColumnCount(1);
	symbolsList->horizontalHeader()->setClickable(false);
	symbolsList->horizontalHeader()->setResizeMode (0, QHeaderView::Stretch);
	symbolsList->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	d_indexed_symbols = app->indexedSymbols();
	setSymbolsList(d_indexed_symbols);

	groupIndexedSymbols = new QGroupBox();
	groupIndexedSymbols->setCheckable(true);
	groupIndexedSymbols->setChecked(app->d_indexed_symbols);
	connect(groupIndexedSymbols, SIGNAL(clicked(bool)), symbolBox, SLOT(setDisabled(bool)));

	QVBoxLayout *vl2 = new QVBoxLayout(groupIndexedSymbols);
	vl2->addWidget(symbolsList);

	btnSymbolUp = new QPushButton(QIcon(":/arrow_up.png"), QString::null);
	connect(btnSymbolUp, SIGNAL(clicked()), this, SLOT(moveSymbol()));

	btnSymbolDown = new QPushButton(QIcon(":/arrow_down.png"), QString::null);
	connect(btnSymbolDown, SIGNAL(clicked()), this, SLOT(moveSymbolDown()));

	btnLoadDefaultSymbols = new QPushButton();
	connect(btnLoadDefaultSymbols, SIGNAL(clicked()), this, SLOT(loadDefaultSymbols()));

	QHBoxLayout *hl4 = new QHBoxLayout();
	hl4->setSpacing(0);
	hl4->addStretch();
	hl4->addWidget(btnSymbolUp);
	hl4->addWidget(btnSymbolDown);
	hl4->addWidget(btnLoadDefaultSymbols);
	hl4->addStretch();
	vl2->addLayout(hl4);

	QHBoxLayout *hl3 = new QHBoxLayout();
	hl3->addWidget(groupIndexedColors);
	hl3->addWidget(groupIndexedSymbols);

	curvesPageLayout->addLayout(hl3);
}

void ConfigDialog::setSymbolsList(const QList<int>& symbList)
{
	symbolsList->clearContents();
	int rows = symbList.size();
	symbolsList->setRowCount(rows);
	for (int i = 0; i < rows; i++){
		SymbolBox *sb = new SymbolBox(false);
		sb->setCurrentIndex(symbList[i]);
		connect(sb, SIGNAL(activated(SymbolBox *)), this, SLOT(setCurrentSymbol(SymbolBox *)));
		connect(sb, SIGNAL(activated(int)), this, SLOT(updateSymbolsList(int)));
		symbolsList->setCellWidget(i, 0, sb);
	}
}

void ConfigDialog::loadDefaultSymbols()
{
	d_indexed_symbols = SymbolBox::defaultSymbols();
	setSymbolsList(d_indexed_symbols);
}

void ConfigDialog::setCurrentSymbol(SymbolBox *sb)
{
	if (!sb)
		return;

	int rows = symbolsList->rowCount();
	for (int i = 0; i < rows; i++){
		SymbolBox *b = (SymbolBox*)symbolsList->cellWidget(i, 0);
		if (b && sb == b){
			symbolsList->setCurrentCell(i, 0);
			break;
		}
	}
}

void ConfigDialog::updateSymbolsList(int style)
{
	int row = symbolsList->currentRow();
	if (row >= 0 && row < d_indexed_symbols.size())
		d_indexed_symbols[row] = style;
}

void ConfigDialog::moveSymbol(bool up)
{
	int row = symbolsList->currentRow();
	if (row < 0 || row >= d_indexed_symbols.size())
		return;

	int destRow = up ? row - 1 : row + 1;
	if (destRow < 0 || destRow >= d_indexed_symbols.size())
		return;

	d_indexed_symbols.swap(row, destRow);
	setSymbolsList(d_indexed_symbols);

	symbolsList->selectRow(destRow);
}

void ConfigDialog::setColorsList(const QList<QColor>& colList, const QStringList& colNames)
{
	colorsList->clearContents();

	int colors = colList.size();
	colorsList->setRowCount(colors);
	for (int i = 0; i < colors; i++){
		QTableWidgetItem *it = new QTableWidgetItem();
		it->setFlags(!Qt::ItemIsEditable);
		it->setBackground(QBrush(colList[i]));
		colorsList->setItem(i, 0, it);

		it = new QTableWidgetItem(colNames[i]);
		it->setForeground(Qt::black);
		colorsList->setItem(i, 1, it);
	}
}

void ConfigDialog::moveColor(bool up)
{
	int row = colorsList->currentRow();
	if (row < 0 || row >= d_indexed_colors.size())
		return;

	int destRow = up ? row - 1 : row + 1;
	if (destRow < 0 || destRow >= d_indexed_colors.size())
		return;

	d_indexed_colors.swap(row, destRow);
	d_indexed_color_names.swap(row, destRow);
	setColorsList(d_indexed_colors, d_indexed_color_names);

	colorsList->selectRow(destRow);
}

void ConfigDialog::moveColorDown()
{
	moveColor(false);
}

void ConfigDialog::removeColor()
{
	int row = colorsList->currentRow();
	if (row < 0 || row >= d_indexed_colors.size())
		return;

	d_indexed_colors.removeAt(row);
	d_indexed_color_names.removeAt(row);
	setColorsList(d_indexed_colors, d_indexed_color_names);

	colorsList->selectRow(row);
}

void ConfigDialog::newColor()
{
	d_indexed_colors << Qt::black;
	d_indexed_color_names << tr("New Color");
	setColorsList(d_indexed_colors, d_indexed_color_names);

	int row = colorsList->rowCount() - 1;
	colorsList->selectRow(row);
	showColorDialog(row, 0);
}

void ConfigDialog::loadDefaultColors()
{
	d_indexed_colors = ColorBox::defaultColors();
	d_indexed_color_names = ColorBox::defaultColorNames();
	setColorsList(d_indexed_colors, d_indexed_color_names);
	colorsList->selectRow(0);
}

void ConfigDialog::showColorDialog(int row, int col)
{
	if (col)
		return;

	QColor c = QColor(d_indexed_colors[row]);
	QColor color = QColorDialog::getColor(c, this);
	if (!color.isValid() || color == c)
		return;

	d_indexed_colors[row] = color;
	setColorsList(d_indexed_colors, d_indexed_color_names);
}

void ConfigDialog::changeColorName(int row, int col)
{
	if (col != 1)
		return;

	QTableWidgetItem *it = colorsList->item(row, 1);
	if (!it)
		return;

	d_indexed_color_names[row] = it->text();
}

void ConfigDialog::initAxesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	axesPage = new QWidget();

	QGroupBox * axisOptions = new QGroupBox();
	QGridLayout * axisOptionsLayout = new QGridLayout( axisOptions );

	boxBackbones = new QCheckBox();
	boxBackbones->setChecked(app->drawBackbones);
	axisOptionsLayout->addWidget(boxBackbones, 0, 0);

	boxSynchronizeScales = new QCheckBox();
	boxSynchronizeScales->setChecked(app->d_synchronize_graph_scales);
	axisOptionsLayout->addWidget(boxSynchronizeScales, 0, 1);

	lblAxesLineWidth = new QLabel();
	axisOptionsLayout->addWidget(lblAxesLineWidth, 1, 0);
	boxLineWidth= new QSpinBox();
	boxLineWidth->setRange(0, 100);
	boxLineWidth->setValue(app->axesLineWidth);
	axisOptionsLayout->addWidget(boxLineWidth, 1, 1);

	labelGraphAxesLabelsDist = new QLabel();
	axisOptionsLayout->addWidget(labelGraphAxesLabelsDist, 2, 0);
	boxAxesLabelsDist = new QSpinBox();
	boxAxesLabelsDist->setRange(0, 1000);
	boxAxesLabelsDist->setValue(app->d_graph_axes_labels_dist);
	axisOptionsLayout->addWidget(boxAxesLabelsDist, 2, 1);

	labelTickLabelsDist = new QLabel();
	axisOptionsLayout->addWidget(labelTickLabelsDist, 3, 0);
	boxTickLabelsDist = new QSpinBox();
	boxTickLabelsDist->setRange(0, 1000);
	boxTickLabelsDist->setValue(app->d_graph_tick_labels_dist);
	axisOptionsLayout->addWidget(boxTickLabelsDist, 3, 1);
	axisOptionsLayout->setRowStretch(4, 1);

	enabledAxesGroupBox = new QGroupBox();
	enabledAxesGrid = new QGridLayout( enabledAxesGroupBox );

	enableAxisLabel = new QLabel();
	enabledAxesGrid->addWidget(enableAxisLabel, 0, 2);
	showNumbersLabel = new QLabel();
	enabledAxesGrid->addWidget(showNumbersLabel, 0, 3);

	QLabel *pixLabel = new QLabel();
	pixLabel->setPixmap (QPixmap (":/left_axis.png"));
	enabledAxesGrid->addWidget(pixLabel, 1, 0);
	yLeftLabel = new QLabel();
	enabledAxesGrid->addWidget(yLeftLabel, 1, 1);

	pixLabel = new QLabel();
	pixLabel->setPixmap (QPixmap (":/right_axis.png"));
	enabledAxesGrid->addWidget(pixLabel, 2, 0);
	yRightLabel = new QLabel();
	enabledAxesGrid->addWidget(yRightLabel, 2, 1);

	pixLabel = new QLabel();
	pixLabel->setPixmap (QPixmap (":/bottom_axis.png"));
	enabledAxesGrid->addWidget(pixLabel, 3, 0);
	xBottomLabel = new QLabel();
	enabledAxesGrid->addWidget(xBottomLabel, 3, 1);

	pixLabel = new QLabel();
	pixLabel->setPixmap (QPixmap (":/top_axis.png"));
	enabledAxesGrid->addWidget(pixLabel, 4, 0);
	xTopLabel = new QLabel();
	enabledAxesGrid->addWidget(xTopLabel, 4, 1);

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		QCheckBox *box1 = new QCheckBox();
		int row = i + 1;

		enabledAxesGrid->addWidget(box1, row, 2);
		bool enabledAxis = app->d_show_axes[i];
		box1->setChecked(enabledAxis);

		QCheckBox *box2 = new QCheckBox();
		enabledAxesGrid->addWidget(box2, row, 3);
		box2->setChecked(app->d_show_axes_labels[i]);
		box2->setEnabled(enabledAxis);

		connect(box1, SIGNAL(toggled(bool)), box2, SLOT(setEnabled(bool)));
	}
	enabledAxesGrid->setColumnStretch (0, 0);
	enabledAxesGrid->setColumnStretch (1, 1);
	enabledAxesGrid->setColumnStretch (2, 1);

	QVBoxLayout * axesPageLayout = new QVBoxLayout( axesPage );
	axesPageLayout->addWidget(axisOptions);
	axesPageLayout->addWidget(enabledAxesGroupBox);
}

void ConfigDialog::initGridPage()
{
	gridPage = new QWidget();

	QGroupBox * rightBox = new QGroupBox(QString());
	QGridLayout * rightLayout = new QGridLayout(rightBox);

	boxMajorGrid = new QCheckBox();
	boxMajorGrid->setChecked(true);
	rightLayout->addWidget( boxMajorGrid, 0, 1);

	boxMinorGrid = new QCheckBox();
	boxMinorGrid->setChecked(false);
	rightLayout->addWidget( boxMinorGrid, 0, 2);

	gridLineColorLbl = new QLabel();
	rightLayout->addWidget(gridLineColorLbl, 1, 0 );

	boxColorMajor = new ColorButton();
	rightLayout->addWidget( boxColorMajor, 1, 1);

	boxColorMinor = new ColorButton();
	boxColorMinor->setDisabled(true);
	rightLayout->addWidget( boxColorMinor, 1, 2);

	gridLineTypeLbl = new QLabel();
	rightLayout->addWidget(gridLineTypeLbl, 2, 0 );

	boxTypeMajor = new PenStyleBox();
	rightLayout->addWidget( boxTypeMajor, 2, 1);

	boxTypeMinor = new PenStyleBox();
	rightLayout->addWidget( boxTypeMinor, 2, 2);

	gridLineWidthLbl = new QLabel();
	rightLayout->addWidget(gridLineWidthLbl, 3, 0);

	boxWidthMajor = new DoubleSpinBox('f');
	boxWidthMajor->setLocale(((ApplicationWindow *)parent())->locale());
	boxWidthMajor->setSingleStep(0.1);
	boxWidthMajor->setRange(0.1, 20);
	boxWidthMajor->setValue(1);
	rightLayout->addWidget( boxWidthMajor, 3, 1);

	boxWidthMinor = new DoubleSpinBox('f');
	boxWidthMinor->setLocale(((ApplicationWindow *)parent())->locale());
	boxWidthMinor->setSingleStep(0.1);
	boxWidthMinor->setRange(0.1, 20);
	boxWidthMinor->setValue(1);
	boxWidthMinor->setDisabled(true);
	rightLayout->addWidget( boxWidthMinor, 3, 2);

	gridAxesLbl = new QLabel();
	rightLayout->addWidget(gridAxesLbl, 4, 0 );

	boxGridXAxis = new QComboBox();
	rightLayout->addWidget( boxGridXAxis, 4, 1);

	boxGridYAxis = new QComboBox();
	rightLayout->addWidget(boxGridYAxis, 4, 2);

	boxAntialiseGrid = new QCheckBox();
	rightLayout->addWidget(boxAntialiseGrid, 5, 0);

	rightLayout->setRowStretch(6, 1);
	rightLayout->setColumnStretch(4, 1);

	axesGridList = new QListWidget();

	QHBoxLayout* mainLayout2 = new QHBoxLayout(gridPage);
	mainLayout2->addWidget(axesGridList);
	mainLayout2->addWidget(rightBox);

	showGridOptions(0);

	//grid page slot connections
	connect(axesGridList, SIGNAL(currentRowChanged(int)), this, SLOT(showGridOptions(int)));
	connect(boxMajorGrid,SIGNAL(toggled(bool)), this, SLOT(majorGridEnabled(bool)));
	connect(boxMinorGrid,SIGNAL(toggled(bool)), this, SLOT(minorGridEnabled(bool)));
	connect(boxAntialiseGrid,SIGNAL(toggled(bool)), this, SLOT(updateGrid()));
	connect(boxColorMajor, SIGNAL(colorChanged(const QColor &)),this, SLOT(updateGrid()));
	connect(boxColorMinor, SIGNAL(colorChanged(const QColor &)),this, SLOT(updateGrid()));
	connect(boxTypeMajor,SIGNAL(activated(int)),this, SLOT(updateGrid()));
	connect(boxTypeMinor,SIGNAL(activated(int)),this, SLOT(updateGrid()));
	connect(boxWidthMajor,SIGNAL(valueChanged(double)),this, SLOT(updateGrid()));
	connect(boxWidthMinor,SIGNAL(valueChanged(double)),this, SLOT(updateGrid()));
}

void ConfigDialog::initConfirmationsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	confirm = new QWidget();

	groupBoxConfirm = new QGroupBox();
	QVBoxLayout * layout = new QVBoxLayout( groupBoxConfirm );

	boxFolders = new QCheckBox();
	boxFolders->setChecked(app->confirmCloseFolder);
	layout->addWidget( boxFolders );

	boxTables = new QCheckBox();
	boxTables->setChecked(app->confirmCloseTable);
	layout->addWidget( boxTables );

	boxMatrices = new QCheckBox();
	boxMatrices->setChecked(app->confirmCloseMatrix);
	layout->addWidget( boxMatrices );

	boxPlots2D = new QCheckBox();
	boxPlots2D->setChecked(app->confirmClosePlot2D);
	layout->addWidget( boxPlots2D );

	boxPlots3D = new QCheckBox();
	boxPlots3D->setChecked(app->confirmClosePlot3D);
	layout->addWidget( boxPlots3D );

	boxNotes = new QCheckBox();
	boxNotes->setChecked(app->confirmCloseNotes);
	layout->addWidget( boxNotes );
	layout->addStretch();

	boxPromptRenameTables = new QCheckBox();
	boxPromptRenameTables->setChecked(app->d_inform_rename_table);

	boxTablePasteDialog = new QCheckBox();
	boxTablePasteDialog->setChecked(app->d_show_table_paste_dialog);

	boxConfirmOverwrite = new QCheckBox();
	boxConfirmOverwrite->setChecked(app->d_confirm_overwrite);

	boxConfirmModifyDataPoints = new QCheckBox();
	boxConfirmModifyDataPoints->setChecked(app->d_confirm_modif_2D_points);

	QVBoxLayout * confirmPageLayout = new QVBoxLayout( confirm );
	confirmPageLayout->addWidget(groupBoxConfirm);
	confirmPageLayout->addWidget(boxPromptRenameTables);
	confirmPageLayout->addWidget(boxTablePasteDialog);
	confirmPageLayout->addWidget(boxConfirmOverwrite);
	confirmPageLayout->addWidget(boxConfirmModifyDataPoints);
	confirmPageLayout->addStretch();
}

void ConfigDialog::initFileLocationsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	fileLocationsPage = new QWidget();

	QCompleter *completer = new QCompleter(this);
	completer->setModel(new QDirModel(completer));
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode(QCompleter::InlineCompletion);

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);

	lblTranslationsPath = new QLabel;
	lblTranslationsPath->setOpenExternalLinks(true);
	gl->addWidget(lblTranslationsPath , 0, 0);

	translationsPathLine = new QLineEdit();
	translationsPathLine->setText(QDir::toNativeSeparators(app->d_translations_folder));
	translationsPathLine->setCompleter(completer);
	gl->addWidget(translationsPathLine, 0, 1);

	QPushButton *browseTranslationsBtn = new QPushButton();
	browseTranslationsBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browseTranslationsBtn, 0, 2);

	lblHelpPath = new QLabel;
	lblHelpPath->setOpenExternalLinks(true);
	gl->addWidget(lblHelpPath, 1, 0 );

	QFileInfo hfi(app->helpFilePath);
	helpPathLine = new QLineEdit(QDir::toNativeSeparators(hfi.dir().absolutePath()));
	helpPathLine->setCompleter(completer);
	gl->addWidget( helpPathLine, 1, 1);

	QPushButton *browseHelpBtn = new QPushButton();
	browseHelpBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browseHelpBtn, 1, 2);

	texCompilerLabel = new QLabel;
	texCompilerLabel->setOpenExternalLinks(true);
	gl->addWidget(texCompilerLabel, 2, 0);

	texCompilerPathBox = new QLineEdit(QDir::toNativeSeparators(app->d_latex_compiler_path));
	texCompilerPathBox->setCompleter(completer);
	connect(texCompilerPathBox, SIGNAL(editingFinished ()), this, SLOT(validateTexCompiler()));

	gl->addWidget(texCompilerPathBox, 2, 1);

    browseTexCompilerBtn = new QPushButton;
	browseTexCompilerBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseTexCompilerBtn, SIGNAL(clicked()), this, SLOT(chooseTexCompiler()));

    gl->addWidget(browseTexCompilerBtn, 2, 2);

	officeLabel = new QLabel;
	officeLabel->setOpenExternalLinks(true);
	gl->addWidget(officeLabel, 3, 0);

	sofficePathBox = new QLineEdit(QDir::toNativeSeparators(app->d_soffice_path));
	sofficePathBox->setCompleter(completer);
	connect(sofficePathBox, SIGNAL(editingFinished ()), this, SLOT(validateOffice()));
	gl->addWidget(sofficePathBox, 3, 1);

	browseOfficeBtn = new QPushButton;
	browseOfficeBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseOfficeBtn, SIGNAL(clicked()), this, SLOT(chooseOffice()));
	gl->addWidget(browseOfficeBtn, 3, 2);

	javaLabel = new QLabel(tr("Java"));
	javaLabel->setOpenExternalLinks(true);
	gl->addWidget(javaLabel, 4, 0);

	javaPathBox = new QLineEdit(QDir::toNativeSeparators(app->d_java_path));
	javaPathBox->setCompleter(completer);
	connect(javaPathBox, SIGNAL(editingFinished ()), this, SLOT(validateJava()));
	gl->addWidget(javaPathBox, 4, 1);

	browseJavaBtn = new QPushButton;
	browseJavaBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseJavaBtn, SIGNAL(clicked()), this, SLOT(chooseJava()));
	gl->addWidget(browseJavaBtn, 4, 2);

	jodconverterLabel = new QLabel;
	jodconverterLabel->setOpenExternalLinks(true);
	gl->addWidget(jodconverterLabel, 5, 0);

	jodconverterPathBox = new QLineEdit(QDir::toNativeSeparators(app->d_jodconverter_path));
	jodconverterPathBox->setCompleter(completer);
	connect(jodconverterPathBox, SIGNAL(editingFinished ()), this, SLOT(validateJODConverter()));
	gl->addWidget(jodconverterPathBox, 5, 1);

	browseJODConverterBtn = new QPushButton;
	browseJODConverterBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseJODConverterBtn, SIGNAL(clicked()), this, SLOT(chooseJODConverter()));
	gl->addWidget(browseJODConverterBtn, 5, 2);

	gl->setRowStretch(6, 1);

#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir = new QLabel;
	gl->addWidget(lblPythonConfigDir, 6, 0);

	pythonConfigDirLine = new QLineEdit(QDir::toNativeSeparators(app->d_python_config_folder));
	pythonConfigDirLine->setCompleter(completer);
	gl->addWidget(pythonConfigDirLine, 6, 1);

	QPushButton *browsePythonConfigBtn = new QPushButton();
	browsePythonConfigBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browsePythonConfigBtn, SIGNAL(clicked()), this, SLOT(choosePythonConfigFolder()));
	gl->addWidget(browsePythonConfigBtn, 6, 2);

	bool showScriptsFolder = (app->defaultScriptingLang == QString("Python"));
	lblPythonScriptsDir = new QLabel;
	lblPythonScriptsDir->setVisible(showScriptsFolder);
	gl->addWidget(lblPythonScriptsDir, 7, 0);

	pythonScriptsDirLine = new QLineEdit(QDir::toNativeSeparators(app->d_startup_scripts_folder));
	pythonScriptsDirLine->setCompleter(completer);
	pythonScriptsDirLine->setVisible(showScriptsFolder);
	gl->addWidget(pythonScriptsDirLine, 7, 1);

	browsePythonScriptsBtn = new QPushButton();
	browsePythonScriptsBtn->setIcon(QIcon(":/folder_open.png"));
	browsePythonScriptsBtn->setVisible(showScriptsFolder);
	connect(browsePythonScriptsBtn, SIGNAL(clicked()), this, SLOT(chooseStartupScriptsFolder()));
	gl->addWidget(browsePythonScriptsBtn, 7, 2);
	gl->setRowStretch(8, 1);

	connect(boxScriptingLanguage, SIGNAL(activated(const QString &)), this, SLOT(showStartupScriptsFolder(const QString &)));
#endif

	QVBoxLayout *vl = new QVBoxLayout(fileLocationsPage);
	vl->addWidget(gb);

	appTabWidget->addTab(fileLocationsPage, QString());

	connect(browseTranslationsBtn, SIGNAL(clicked()), this, SLOT(chooseTranslationsFolder()));
	connect(browseHelpBtn, SIGNAL(clicked()), this, SLOT(chooseHelpFolder()));
}

void ConfigDialog::languageChange()
{
	setWindowTitle( tr( "QtiPlot - Choose default settings" ) );
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	btnDefaultSettings->setText(tr("&Default options"));

	// pages list
	itemsList->clear();
	itemsList->addItem( tr( "General" ) );
	itemsList->addItem( tr( "Tables" ) );
	itemsList->addItem( tr( "2D Plots" ) );
	itemsList->addItem( tr( "3D Plots" ) );
	itemsList->addItem( tr( "Notes" ) );
	itemsList->addItem( tr( "Fitting" ) );
	itemsList->setCurrentRow(0);
	itemsList->item(0)->setIcon(QIcon(":/general.png"));
	itemsList->item(1)->setIcon(QIcon(":/configTable.png"));
	itemsList->item(2)->setIcon(QIcon(":/config_curves.png"));
	itemsList->item(3)->setIcon(QIcon(":/logo.png"));
	itemsList->item(4)->setIcon(QIcon(":/notes_32.png"));
	itemsList->item(5)->setIcon(QIcon(":/fit.png"));

	//plots 2D page
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotOptions), tr("Options"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(curves), tr("Curves"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(axesPage), tr("Axes"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotTicks), tr("Ticks"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(gridPage), tr("Grid"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotGeometryPage), tr("Geometry"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotSpeedPage), tr("Speed"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotFonts), tr("Fonts"));

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
	legendDisplayBox->clear();
	legendDisplayBox->addItem(tr("Default"));
	legendDisplayBox->addItem(tr("Column name"));
	legendDisplayBox->addItem(tr("Column comment"));
	legendDisplayBox->addItem(tr("Table name"));
	legendDisplayBox->addItem(tr("Table legend"));
	legendDisplayBox->addItem(tr("Full dataset name"));
	legendDisplayBox->setCurrentIndex(app->d_graph_legend_display);

	attachToLabel->setText(tr("Attach objects to"));
	attachToBox->clear();
	attachToBox->addItem(tr("Page"));
	attachToBox->addItem(tr("Layer Scales"));
	attachToBox->setCurrentIndex(app->d_graph_attach_policy);

	lblAxisLabeling->setText(tr("Axis title" ));
	axisLabelingBox->clear();
	axisLabelingBox->addItem(tr("Default"));
	axisLabelingBox->addItem(tr("Column name"));
	axisLabelingBox->addItem(tr("Column comment"));
	axisLabelingBox->addItem(tr("Column name") + " (" + tr("Column comment") + ")");
	axisLabelingBox->setCurrentIndex(app->d_graph_axis_labeling);

	groupBackgroundOptions->setTitle(tr("Background"));
	labelGraphBkgColor->setText(tr("Background Color"));
	labelGraphBkgOpacity->setText(tr( "Opacity" ));
	labelGraphCanvasColor->setText(tr("Canvas Color" ));
	labelGraphCanvasOpacity->setText(tr("Opacity"));
	labelGraphFrameColor->setText(tr("Border Color"));
	labelGraphFrameWidth->setText(tr( "Width" ));
	boxBackgroundTransparency->setSpecialValueText(tr("Transparent"));
	boxCanvasTransparency->setSpecialValueText(tr("Transparent"));

	unitBoxLabel->setText(tr("Unit"));
	unitBox->clear();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));
	unitBox->setCurrentIndex(app->d_layer_geometry_unit);

	canvasWidthLabel->setText(tr("Canvas Width"));
	canvasHeightLabel->setText(tr("Canvas Height"));
	keepRatioBox->setText(tr("&Keep aspect ratio"));

	// axes page
	boxBackbones->setText(tr("Axes &backbones"));
	boxSynchronizeScales->setText(tr("Synchronize scale &divisions"));
	lblAxesLineWidth->setText(tr("Axes linewidth" ));

	yLeftLabel->setText(tr("Left"));
	yRightLabel->setText(tr("Right"));
	xBottomLabel->setText(tr("Bottom"));
	xTopLabel->setText(tr("Top"));

	enabledAxesGroupBox->setTitle(tr("Enabled axes" ));
	enableAxisLabel->setText(tr( "Show" ));
	showNumbersLabel->setText(tr( "Labels" ));

	boxMajTicks->clear();
	boxMajTicks->addItem(tr("None"));
	boxMajTicks->addItem(tr("Out"));
	boxMajTicks->addItem(tr("In & Out"));
	boxMajTicks->addItem(tr("In"));

	boxMinTicks->clear();
	boxMinTicks->addItem(tr("None"));
	boxMinTicks->addItem(tr("Out"));
	boxMinTicks->addItem(tr("In & Out"));
	boxMinTicks->addItem(tr("In"));

	boxMajTicks->setCurrentIndex(app->majTicksStyle);
	boxMinTicks->setCurrentIndex(app->minTicksStyle);

	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotPrint), tr("Print"));
	boxPrintCropmarks->setText(tr("Print Crop&marks"));
	boxScaleLayersOnPrint->setText(tr("&Scale layers to paper size"));

	//grid page
	boxMajorGrid->setText( tr( "Major Grids" ) );
	boxMinorGrid->setText( tr( "Minor Grids" ) );
	boxAntialiseGrid->setText(tr("An&tialised"));

	gridLineColorLbl->setText(tr( "Line Color" ));
	gridLineWidthLbl->setText(tr( "Thickness" ));
	gridAxesLbl->setText(tr( "Axes" ));
	gridLineTypeLbl->setText(tr( "Line Type" ));

	boxGridXAxis->clear();
	boxGridXAxis->insertItem(tr("Bottom"));
	boxGridXAxis->insertItem(tr("Top"));

	boxGridYAxis->clear();
	boxGridYAxis->insertItem(tr("Left"));
	boxGridYAxis->insertItem(tr("Right"));

	QPixmap image2(":/vertical_grid.png");
	QPixmap image3(":/horizontal_grid.png");
	axesGridList->clear();
	axesGridList->addItem( new QListWidgetItem(image3, tr( "Horizontal" )) );
	axesGridList->addItem( new QListWidgetItem(image2, tr( "Vertical" )) );
	axesGridList->setIconSize(image3.size());
	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(axesGridList->font());
	int width = 32,i;
	for(i=0 ; i<axesGridList->count() ; i++)
		if( fm.width(axesGridList->item(i)->text()) > width)
			width = fm.width(axesGridList->item(i)->text());

	axesGridList->setMaximumWidth( axesGridList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	axesGridList->resize(axesGridList->maximumWidth(),axesGridList->height());
	axesGridList->setCurrentRow(0);

	//speed page
	antialiasingGroupBox->setTitle(tr("Antia&liasing"));
	disableAntialiasingBox->setText(tr("&Disable for curves with more than"));
	curveSizeBox->setSuffix(" " + tr("data points"));

	//confirmations page
	groupBoxConfirm->setTitle(tr("Prompt on closing"));
	boxFolders->setText(tr("Folders"));
	boxTables->setText(tr("Tables"));
	boxPlots3D->setText(tr("3D Plots"));
	boxPlots2D->setText(tr("2D Plots"));
	boxMatrices->setText(tr("Matrices"));
	boxNotes->setText(tr("&Notes"));

	buttonOk->setText( tr( "&OK" ) );
	buttonCancel->setText( tr( "&Cancel" ) );
	buttonApply->setText( tr( "&Apply" ) );
	buttonTextFont->setText( tr( "&Text Font" ) );
	buttonHeaderFont->setText( tr( "&Labels Font" ) );
	buttonAxesFont->setText( tr( "A&xes Labels" ) );
	buttonNumbersFont->setText( tr( "Axes &Numbers" ) );
	buttonLegendFont->setText( tr( "&Legend" ) );
	buttonTitleFont->setText( tr( "T&itle" ) );
	boxPromptRenameTables->setText( tr( "Prompt on &renaming tables when appending projects" ) );
	boxTablePasteDialog->setText(tr( "Prompt on pasting &values to tables" ));
	boxConfirmOverwrite->setText( tr( "Ask before over&writing files" ) );
	boxConfirmModifyDataPoints->setText( tr( "Ask before modifying data &points" ) );

	//application page
	appTabWidget->setTabText(appTabWidget->indexOf(application), tr("Application"));
	appTabWidget->setTabText(appTabWidget->indexOf(confirm), tr("Confirmations"));
	appTabWidget->setTabText(appTabWidget->indexOf(appColors), tr("Colors"));
	appTabWidget->setTabText(appTabWidget->indexOf(numericFormatPage), tr("Numeric Format"));
	appTabWidget->setTabText(appTabWidget->indexOf(fileLocationsPage), tr("File Locations"));
	appTabWidget->setTabText(appTabWidget->indexOf(proxyPage), tr("&Internet Connection"));

	lblLanguage->setText(tr("Language"));
	lblStyle->setText(tr("Style"));
	lblFonts->setText(tr("Main Font"));
	fontsBtn->setText(tr("Choose &font"));
	lblWorkspace->setText(tr("Workspace"));
	lblPanelsText->setText(tr("Panels text"));
	lblPanels->setText(tr("Panels"));
	boxSave->setText(tr("Save every"));
	boxBackupProject->setText(tr("&Backup project before saving"));
	boxSearchUpdates->setText(tr("Check for new versions at startup"));
	boxMinutes->setSuffix(tr(" minutes"));
	lblScriptingLanguage->setText(tr("Default scripting language"));
	lblUndoStackSize->setText(tr("Matrix Undo Stack Size"));
	lblEndOfLine->setText(tr("Endline character"));
	boxEndLine->clear();
	boxEndLine->addItem(tr("LF (Unix)"));
	boxEndLine->addItem(tr("CRLF (Windows)"));
	boxEndLine->addItem(tr("CR (Mac)"));
	boxEndLine->setCurrentIndex((int)app->d_eol);

	lblInitWindow->setText(tr("Start New Project"));
	boxInitWindow->clear();
	boxInitWindow->addItem(tr("Empty"));
	boxInitWindow->addItem(tr("Table"));
	boxInitWindow->addItem(tr("Matrix"));
	boxInitWindow->addItem(tr("Empty Graph"));
	boxInitWindow->addItem(tr("Note"));
	boxInitWindow->setCurrentIndex((int)app->d_init_window_type);
    completionBox->setText(tr("&Enable autocompletion (Ctrl+U)"));
	openLastProjectBox->setText(tr("Open &last project at startup"));

	excelImportMethodLabel->setText(tr("Import Excel files using"));
	excelImportMethod->clear();
	excelImportMethod->addItem(tr("Excel Format Library"));
	excelImportMethod->addItem(tr("Locally Installed OpenOffice/LibreOffice"));
#ifdef Q_OS_WIN
	excelImportMethod->addItem(tr("Locally Installed Excel"));
#endif
	excelImportMethod->setCurrentIndex((int)app->excelImportMethod());

	lblAppPrecision->setText(tr("Number of Decimal Digits"));
	lblDecimalSeparator->setText(tr("Decimal Separators"));
	boxDecimalSeparator->clear();
	boxDecimalSeparator->addItem(tr("System Locale Setting"));
	boxDecimalSeparator->addItem("1,000.0");
	boxDecimalSeparator->addItem("1.000,0");
	boxDecimalSeparator->addItem("1 000,0");
	boxThousandsSeparator->setText(tr("Omit &Thousands Separator"));
	boxMuParserCLocale->setText(tr("mu&Parser uses C locale settings"));

    QLocale locale = app->locale();
    if (locale.name() == QLocale::c().name())
        boxDecimalSeparator->setCurrentIndex(1);
    else if (locale.name() == QLocale(QLocale::German).name())
        boxDecimalSeparator->setCurrentIndex(2);
    else if (locale.name() == QLocale(QLocale::French).name())
        boxDecimalSeparator->setCurrentIndex(3);

	lblClipboardSeparator->setText(tr("Clipboard Decimal Separators"));
	boxClipboardLocale->clear();
	boxClipboardLocale->addItem(tr("System Locale Setting"));
	boxClipboardLocale->addItem("1,000.0");
	boxClipboardLocale->addItem("1.000,0");
	boxClipboardLocale->addItem("1 000,0");

    if (app->clipboardLocale().name() == QLocale::c().name())
        boxClipboardLocale->setCurrentIndex(1);
    else if (app->clipboardLocale().name() == QLocale(QLocale::German).name())
        boxClipboardLocale->setCurrentIndex(2);
    else if (app->clipboardLocale().name() == QLocale(QLocale::French).name())
        boxClipboardLocale->setCurrentIndex(3);

	lblTranslationsPath->setText("<a href=\"http://soft.proindependent.com/translations.html\">" + tr("Translations") + "</a>");
	lblHelpPath->setText("<a href=\"http://soft.proindependent.com/manuals.html\">" + tr("Help") + "</a>");
	texCompilerLabel->setText("<a href=\"http://www.latex-project.org/\">" + tr("LaTeX Compiler") + "</a>");
	officeLabel->setText("<a href=\"http://www.openoffice.org/\">" + tr("OpenOffice.org") + "</a>/" +
						 "<a href=\"http://www.documentfoundation.org/\">" + tr("LibreOffice") + "</a>");
	javaLabel->setText("<a href=\"http://www.java.com/\">" + tr("Java") + "</a>");
	jodconverterLabel->setText("<a href=\"http://www.artofsolving.com/opensource/jodconverter\">" + tr("JODConverter") + "</a>");
#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir->setText(tr("Python Configuration Files"));
	lblPythonScriptsDir->setText(tr("Startup Scripts"));
#endif

	//proxy tab
	proxyGroupBox->setTitle(tr("&Proxy"));
    proxyHostLabel->setText(tr("Host"));
    proxyPortLabel->setText(tr("Port"));
    proxyUserLabel->setText(tr("Username"));
    proxyPasswordLabel->setText(tr("Password"));

	//tables page
	boxUpdateTableValues->setText(tr("Automatically &Recalculate Column Values"));
	boxTableComments->setText(tr("&Display Comments in Header"));
	groupBoxTableCol->setTitle(tr("Colors"));
	lblSeparator->setText(tr("Default Column Separator"));
	boxSeparator->clear();
	boxSeparator->addItem(tr("TAB"));
	boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem(";" + tr("SPACE"));
	boxSeparator->addItem("," + tr("SPACE"));
	boxSeparator->addItem(";");
	boxSeparator->addItem(",");
	setColumnSeparator(app->columnSeparator);

	lblTableBackground->setText(tr( "Background" ));
	lblTextColor->setText(tr( "Text" ));
	lblHeaderColor->setText(tr("Labels"));
	groupBoxTableFonts->setTitle(tr("Fonts"));

	//curves page
	lblCurveStyle->setText("&" + tr( "Default curve style" ));
	lblLineWidth->setText("&" + tr( "Line width" ));
	lblSymbSize->setText("&" + tr( "Symbol size" ));
	btnLoadDefaultColors->setText(tr("&Load Default"));
	btnNewColor->setToolTip(tr("New Color"));
	btnRemoveColor->setToolTip(tr("Delete Color"));
	btnColorUp->setToolTip(tr("Move Color Up"));
	btnColorDown->setToolTip(tr("Move Color Down"));
	groupIndexedColors->setTitle(tr("Indexed Colors"));
	groupIndexedSymbols->setTitle(tr("Inde&xed Symbols"));
	btnLoadDefaultSymbols->setText(tr("&Load Default"));
	btnSymbolUp->setToolTip(tr("Move Symbol Up"));
	btnSymbolDown->setToolTip(tr("Move Symbol Down"));
	lblPattern->setText("&" + tr("Pattern"));
	lblCurveAlpha->setText("&" + tr("Opacity"));
	curveAlphaBox->setSpecialValueText(" " + tr("Transparent"));
	lblLineStyle->setText(tr("Line style"));
	fillCurvesGroupBox->setTitle(tr("Fill area under curve"));
	curvesGroupBox->setTitle(tr("Default Line Style"));

	symbolGroupBox->setTitle(tr("Default Symbol"));
	lblSymbBox->setText(tr("Style"));
	lblSymbEdge->setText("&" + tr("Edge width"));
	fillSymbolsBox->setText(tr("&Fill Symbol"));

	QStringList header = QStringList() << tr("Color") << tr("Name");
	colorsList->setHorizontalHeaderLabels(header);
	header = QStringList() << tr("Symbol");
	symbolsList->setHorizontalHeaderLabels(header);

	boxCurveStyle->clear();
	boxCurveStyle->addItem( QPixmap(":/lPlot.png"), tr( " Line" ) );
	boxCurveStyle->addItem( QPixmap(":/pPlot.png"), tr( " Scatter" ) );
	boxCurveStyle->addItem( QPixmap(":/lpPlot.png"), tr( " Line + Symbol" ) );
	boxCurveStyle->addItem( QPixmap(":/dropLines.png"), tr( " Vertical drop lines" ) );
	boxCurveStyle->addItem( QPixmap(":/spline.png"), tr( " Spline" ) );
	boxCurveStyle->addItem( QPixmap(":/vert_steps.png"), tr( " Vertical steps" ) );
	boxCurveStyle->addItem( QPixmap(":/hor_steps.png"), tr( " Horizontal steps" ) );
	boxCurveStyle->addItem( QPixmap(":/area.png"), tr( " Area" ) );
	boxCurveStyle->addItem( QPixmap(":/vertBars.png"), tr( " Vertical Bars" ) );
	boxCurveStyle->addItem( QPixmap(":/hBars.png"), tr( " Horizontal Bars" ) );

	int style = app->defaultCurveStyle;
	if (style == Graph::Line)
		boxCurveStyle->setCurrentItem(0);
	else if (style == Graph::Scatter)
		boxCurveStyle->setCurrentItem(1);
	else if (style == Graph::LineSymbols)
		boxCurveStyle->setCurrentItem(2);
	else if (style == Graph::VerticalDropLines)
		boxCurveStyle->setCurrentItem(3);
	else if (style == Graph::Spline)
		boxCurveStyle->setCurrentItem(4);
	else if (style == Graph::VerticalSteps)
		boxCurveStyle->setCurrentItem(5);
	else if (style == Graph::HorizontalSteps)
		boxCurveStyle->setCurrentItem(6);
	else if (style == Graph::Area)
		boxCurveStyle->setCurrentItem(7);
	else if (style == Graph::VerticalBars)
		boxCurveStyle->setCurrentItem(8);
	else if (style == Graph::HorizontalBars)
		boxCurveStyle->setCurrentItem(9);

	//plots 3D
	lblResolution->setText(tr("&Resolution"));
	boxResolution->setSpecialValueText( "1 " + tr("(all data shown)") );
	boxShowLegend->setText(tr( "&Show Legend" ));
	lblFloorStyle->setText(tr( "&Floor style" ));
	boxProjection->clear();
	boxProjection->addItem(tr( "Empty" ));
	boxProjection->addItem(tr( "Isolines" ));
	boxProjection->addItem(tr( "Projection" ));
	boxProjection->setCurrentIndex(app->d_3D_projection);

	boxSmoothMesh->setText(tr( "Smoot&h Line" ));
	boxOrthogonal->setText(tr( "O&rthogonal" ));
	btnLabels->setText( tr( "Lab&els" ) );
	btnMesh->setText( tr( "&Mesh" ) );
	btnGrid->setText( tr( "&Grid" ) );
	btnNumbers->setText( tr( "&Numbers" ) );
	btnAxes->setText( tr( "A&xes" ) );
	btnBackground3D->setText( tr( "&Background" ) );
	groupBox3DCol->setTitle(tr("Colors" ));
	colorMapBox->setTitle(tr("Default Color Map" ));

	groupBox3DFonts->setTitle(tr("Fonts" ));
	btnTitleFnt->setText( tr( "&Title" ) );
	btnLabelsFnt->setText( tr( "&Axes Labels" ) );
	btnNumFnt->setText( tr( "&Numbers" ) );
	boxAutoscale3DPlots->setText( tr( "Autosca&ling" ) );

	groupBox3DGrids->setTitle(tr("Grids"));
	boxMajorGrids->setText(tr("Ma&jor Grids"));
	boxMinorGrids->setText(tr("Mi&nor Grids"));

	boxMajorGridStyle->clear();
	boxMajorGridStyle->addItem(tr("Solid"));
	boxMajorGridStyle->addItem(tr("Dash"));
	boxMajorGridStyle->addItem(tr("Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMajorGridStyle->addItem(tr("Short Dash"));
	boxMajorGridStyle->addItem(tr("Short Dot"));
	boxMajorGridStyle->addItem(tr("Short Dash Dot"));
	boxMajorGridStyle->setCurrentIndex(app->d_3D_major_style);

	boxMinorGridStyle->clear();
	boxMinorGridStyle->addItem(tr("Solid"));
	boxMinorGridStyle->addItem(tr("Dash"));
	boxMinorGridStyle->addItem(tr("Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMinorGridStyle->addItem(tr("Short Dash"));
	boxMinorGridStyle->addItem(tr("Short Dot"));
	boxMinorGridStyle->addItem(tr("Short Dash Dot"));
	boxMinorGridStyle->setCurrentIndex(app->d_3D_minor_style);

	label3DGridsColor->setText(tr("Color"));
	label3DGridsWidth->setText(tr("Width"));
	label3DGridsStyle->setText(tr("Style"));

    //Notes page
    labelTabLength->setText(tr("Tab length (pixels)"));
    labelNotesFont->setText(tr("Font"));
    lineNumbersBox->setText(tr("&Display line numbers"));

	groupSyntaxHighlighter->setTitle(tr("Syntax Highlighting"));
	buttonCommentLabel->setText(tr("Co&mments"));
	buttonKeywordLabel->setText(tr("&Keywords"));
	buttonNumericLabel->setText(tr("&Numbers"));
	buttonQuotationLabel->setText(tr("&Quotations"));
	buttonFunctionLabel->setText(tr("&Functions"));
	buttonClassLabel->setText(tr("Q&t Classes"));

	//Fitting page
	groupBoxFittingCurve->setTitle(tr("Generated Fit Curve"));
	generatePointsBtn->setText(tr("Uniform X Function"));
	lblPoints->setText( tr("Points") );
	samePointsBtn->setText( tr( "Same X as Fitting Data" ) );
	linearFit2PointsBox->setText( tr( "2 points for linear fits" ) );

	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));

	groupBoxFitParameters->setTitle(tr("Parameters Output"));
	lblPrecision->setText(tr("Significant Digits"));
	logBox->setText(tr("Write Parameters to Result Log"));
	plotLabelBox->setText(tr("Paste Parameters to Plot"));
	scaleErrorsBox->setText(tr("Scale Errors with sqrt(Chi^2/doF)"));
	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));
	lblPeaksColor->setText(tr("Peaks Color"));

	boxMultiPeakMsgs->setText(tr("Display Confirmation &Messages for Multi-peak Fits"));

	updateMenuList();
}

void ConfigDialog::accept()
{
	apply();
	close();
}

void ConfigDialog::apply()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	// tables page
	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", false);
	sep.replace("\\t", "\t");
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");

	if (sep.contains(QRegExp("[0-9.eE+-]"))!=0){
		QMessageBox::warning(0, tr("QtiPlot - Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	app->columnSeparator = sep;
	app->setAutoUpdateTableValues(boxUpdateTableValues->isChecked());
	app->d_show_table_paste_dialog = boxTablePasteDialog->isChecked();

	app->tableBkgdColor = buttonBackground->color();
	app->tableTextColor = buttonText->color();
	app->tableHeaderColor = buttonHeader->color();
	app->tableTextFont = textFont;
	app->tableHeaderFont = headerFont;
	app->d_show_table_comments = boxTableComments->isChecked();

    QColorGroup cg;
	cg.setColor(QColorGroup::Base, buttonBackground->color());
	cg.setColor(QColorGroup::Text, buttonText->color());
	QPalette palette(cg, cg, cg);

	QList<MdiSubWindow *> windows = app->windowsList();
	foreach(MdiSubWindow *w, windows){
		if (w->inherits("Table")){
			Table *t = (Table*)w;
            w->setPalette(palette);
			t->setHeaderColor(buttonHeader->color());
            t->setTextFont(textFont);
            t->setHeaderFont(headerFont);
            t->showComments(boxTableComments->isChecked());
		}
	}

	app->d_graph_background_color = boxBackgroundColor->color();
	app->d_graph_background_opacity = boxBackgroundTransparency->value();
	app->d_graph_canvas_color = boxCanvasColor->color();
	app->d_graph_canvas_opacity = boxCanvasTransparency->value();
	app->d_graph_border_color = boxBorderColor->color();
	app->d_graph_border_width = boxBorderWidth->value();

	// 2D plots page: options tab
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
	// 2D plots page: curves tab
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

	// 2D plots page: axes tab
	if (generalDialog->currentWidget() == plotsTabWidget &&
		plotsTabWidget->currentWidget() == axesPage){
		app->drawBackbones = boxBackbones->isChecked();
		app->axesLineWidth = boxLineWidth->value();
		app->d_synchronize_graph_scales = boxSynchronizeScales->isChecked();

		for (int i = 0; i < QwtPlot::axisCnt; i++){
			int row = i + 1;
			QLayoutItem *item = enabledAxesGrid->itemAtPosition(row, 2);
			QCheckBox *box = qobject_cast<QCheckBox *>(item->widget());
			app->d_show_axes[i] = box->isChecked();

			item = enabledAxesGrid->itemAtPosition(row, 3);
			box = qobject_cast<QCheckBox *>(item->widget());
			app->d_show_axes_labels[i] = box->isChecked();
		}
	}

	if (generalDialog->currentWidget() == plotsTabWidget &&
		plotsTabWidget->currentWidget() == plotGeometryPage){
		app->d_layer_geometry_unit = unitBox->currentIndex();
		FrameWidget::Unit unit = (FrameWidget::Unit)unitBox->currentIndex();
		app->d_layer_canvas_width = convertToPixels(boxCanvasWidth->value(), unit, 0);
		app->d_layer_canvas_height = convertToPixels(boxCanvasHeight->value(), unit, 1);
		app->d_keep_aspect_ration = keepRatioBox->isChecked();
	}

	// 2D plots page: ticks tab
	app->majTicksLength = boxMajTicksLength->value();
	app->minTicksLength = boxMinTicksLength->value();
	app->majTicksStyle = boxMajTicks->currentItem();
	app->minTicksStyle = boxMinTicks->currentItem();
	// 2D plots page: fonts tab
	app->plotAxesFont=axesFont;
	app->plotNumbersFont=numbersFont;
	app->plotLegendFont=legendFont;
	app->plotTitleFont=titleFont;
	// 2D plots page: print tab
	app->d_print_cropmarks = boxPrintCropmarks->isChecked();
	app->d_scale_plots_on_print = boxScaleLayersOnPrint->isChecked();
	//2D plots page: speed tab
	app->d_curve_max_antialising_size = curveSizeBox->value();
	app->d_disable_curve_antialiasing = disableAntialiasingBox->isChecked();

	foreach(MdiSubWindow *w, windows){
		MultiLayer *ml = qobject_cast<MultiLayer *>(w);
		if (ml){
			ml->setScaleLayersOnPrint(boxScaleLayersOnPrint->isChecked());
			ml->printCropmarks(boxPrintCropmarks->isChecked()); 
			foreach(Graph *g, ml->layersList()){
				g->setSynchronizedScaleDivisions(app->d_synchronize_graph_scales);
				g->setAxisTitlePolicy(app->d_graph_axis_labeling);
				g->disableCurveAntialiasing(app->d_disable_curve_antialiasing, app->d_curve_max_antialising_size);
			}
		}
	}
	// general page: application tab
	app->changeAppFont(appFont);
	setFont(appFont);
	app->changeAppStyle(boxStyle->currentText());
	app->autoSearchUpdates = boxSearchUpdates->isChecked();
	app->setSaveSettings(boxSave->isChecked(), boxMinutes->value());
	app->d_backup_files = boxBackupProject->isChecked();
	app->defaultScriptingLang = boxScriptingLanguage->currentText();
	app->d_init_window_type = (ApplicationWindow::WindowType)boxInitWindow->currentIndex();
	app->setMatrixUndoStackSize(undoStackSizeBox->value());
	app->d_eol = (ApplicationWindow::EndLineChar)boxEndLine->currentIndex();
    app->enableCompletion(completionBox->isChecked());
	app->d_open_last_project = openLastProjectBox->isChecked();
	app->setExcelImportMethod((ApplicationWindow::ExcelImportMethod)excelImportMethod->currentIndex());

	// general page: numeric format tab
	app->d_decimal_digits = boxAppPrecision->value();
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex()){
        case 0:
            locale = QLocale::system();
        break;
        case 1:
            locale = QLocale::c();
        break;
        case 2:
            locale = QLocale(QLocale::German);
        break;
        case 3:
            locale = QLocale(QLocale::French);
        break;
    }
    if (boxThousandsSeparator->isChecked())
        locale.setNumberOptions(QLocale::OmitGroupSeparator);

    QLocale oldLocale = app->locale();
    app->setLocale(locale);
	QLocale::setDefault(locale);
	app->d_muparser_c_locale = boxMuParserCLocale->isChecked();

	if (generalDialog->currentWidget() == appTabWidget &&
		appTabWidget->currentWidget() == numericFormatPage){
    	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QList<MdiSubWindow *> windows = app->windowsList();
        foreach(MdiSubWindow *w, windows){
            w->setLocale(locale);

            if(w->isA("Table"))
                ((Table *)w)->updateDecimalSeparators(oldLocale);
            else if(w->isA("Matrix"))
                ((Matrix *)w)->resetView();
        }

		switch (boxClipboardLocale->currentIndex()){
        	case 0:
            	app->setClipboardLocale(QLocale::system());
        	break;
        	case 1:
            	app->setClipboardLocale(QLocale::c());
        	break;
        	case 2:
            	app->setClipboardLocale(QLocale(QLocale::German));
        	break;
        	case 3:
            	app->setClipboardLocale(QLocale(QLocale::French));
        	break;
    	}

        app->modifiedProject();
    	QApplication::restoreOverrideCursor();
	}

	// general page: file locations tab
	if (generalDialog->currentWidget() == appTabWidget &&
		appTabWidget->currentWidget() == fileLocationsPage){
		QString path = translationsPathLine->text();
		if (path != app->d_translations_folder && validFolderPath(path)){
			app->d_translations_folder = QFileInfo(path).absoluteFilePath();
			app->createLanguagesList();
			insertLanguagesList();
		}

		if (validFolderPath(helpPathLine->text())){
			path = helpPathLine->text() + "/index.html";
			if (path != app->helpFilePath){
				QFileInfo fi(path);
				if (fi.exists() && fi.isFile())
					app->helpFilePath = fi.absoluteFilePath();
				else
					QMessageBox::critical(this, tr("QtiPlot - index.html File Not Found!"),
					tr("There is no file called <b>index.html</b> in folder %1.<br>Please choose another folder!").
					arg(helpPathLine->text()));
			}
		}

#ifdef SCRIPTING_PYTHON
		path = pythonConfigDirLine->text();
		if (path != app->d_python_config_folder && validFolderPath(path))
			app->d_python_config_folder = QFileInfo(path).absoluteFilePath();

		path = pythonScriptsDirLine->text();
		if (path != app->d_startup_scripts_folder && validFolderPath(path))
			app->d_startup_scripts_folder = QFileInfo(path).absoluteFilePath();
#endif
	}

	if (generalDialog->currentWidget() == appTabWidget &&
		appTabWidget->currentWidget() == proxyPage){
		setApplicationCustomProxy();
	}

	// general page: confirmations tab
	app->d_inform_rename_table = boxPromptRenameTables->isChecked();
	app->d_confirm_overwrite = boxConfirmOverwrite->isChecked();
	app->d_confirm_modif_2D_points = boxConfirmModifyDataPoints->isChecked();
	app->confirmCloseFolder = boxFolders->isChecked();
	app->updateConfirmOptions(boxTables->isChecked(), boxMatrices->isChecked(),
			boxPlots2D->isChecked(), boxPlots3D->isChecked(),
			boxNotes->isChecked());
	// general page: colors tab
	app->setAppColors(btnWorkspace->color(), btnPanels->color(), btnPanelsText->color());
	// 3D plots page
	app->d_3D_color_map = colorMapEditor->colorMap();
	app->d_3D_axes_color = btnAxes->color();
	app->d_3D_numbers_color = btnNumbers->color();
	app->d_3D_grid_color = btnGrid->color();
	app->d_3D_mesh_color = btnMesh->color();
	app->d_3D_background_color = btnBackground3D->color();
	app->d_3D_labels_color = btnLabels->color();
	app->d_3D_legend = boxShowLegend->isChecked();
	app->d_3D_projection = boxProjection->currentIndex();
	app->d_3D_resolution = boxResolution->value();
	app->d_3D_title_font = d_3D_title_font;
	app->d_3D_numbers_font = d_3D_numbers_font;
	app->d_3D_axes_font = d_3D_axes_font;
	app->d_3D_orthogonal = boxOrthogonal->isChecked();
	app->d_3D_smooth_mesh = boxSmoothMesh->isChecked();
	app->d_3D_autoscale = boxAutoscale3DPlots->isChecked();
	app->setPlot3DOptions();

	app->d_3D_grid_color = btnGrid->color();
	app->d_3D_minor_grid_color = btnGridMinor->color();
	app->d_3D_minor_grids = boxMajorGrids->isChecked();
	app->d_3D_major_grids = boxMinorGrids->isChecked();
	app->d_3D_major_style = boxMajorGridStyle->currentIndex();
	app->d_3D_minor_style = boxMinorGridStyle->currentIndex();
	app->d_3D_major_width = boxMajorGridWidth->value();
	app->d_3D_minor_width = boxMinorGridWidth->value();

	// fitting page
	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->generatePeakCurves = groupBoxMultiPeak->isChecked();
	app->peakCurvesColor = boxPeaksColor->color();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->d_2_linear_fit_points = linearFit2PointsBox->isChecked();
	app->d_multi_peak_messages = boxMultiPeakMsgs->isChecked();
	app->saveSettings();

	updateMenuList();
}

int ConfigDialog::curveStyle()
{
	int style = 0;
	switch (boxCurveStyle->currentItem())
	{
		case 0:
			style = Graph::Line;
			break;
		case 1:
			style = Graph::Scatter;
			break;
		case 2:
			style = Graph::LineSymbols;
			break;
		case 3:
			style = Graph::VerticalDropLines;
			break;
		case 4:
			style = Graph::Spline;
			break;
		case 5:
			style = Graph::VerticalSteps;
			break;
		case 6:
			style = Graph::HorizontalSteps;
			break;
		case 7:
			style = Graph::Area;
			break;
		case 8:
			style = Graph::VerticalBars;
			break;
		case 9:
			style = Graph::HorizontalBars;
			break;
	}
	return style;
}

void ConfigDialog::pickTextFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,textFont,this);
	if ( ok ) {
		textFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickHeaderFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,headerFont,this);
	if ( ok ) {
		headerFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickLegendFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,legendFont,this);
	if ( ok ) {
		legendFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,axesFont,this);
	if ( ok ) {
		axesFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,numbersFont,this);
	if ( ok ) {
		numbersFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,titleFont,this);
	if ( ok )
		titleFont = font;
	else
		return;
}

void ConfigDialog::pickApplicationFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, appFont,this);
	if ( ok )
		appFont = font;
	else
		return;

	((ApplicationWindow *)parentWidget())->changeAppFont(appFont);
}

void ConfigDialog::pick3DTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_title_font,this);
	if ( ok )
		d_3D_title_font = font;
	else
		return;
}

void ConfigDialog::pick3DNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_numbers_font,this);
	if ( ok )
		d_3D_numbers_font = font;
	else
		return;
}

void ConfigDialog::pick3DAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_axes_font,this);
	if ( ok )
		d_3D_axes_font = font;
	else
		return;
}

void ConfigDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep==" ")
		boxSeparator->setCurrentIndex(1);
	else if (sep==";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep==",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep=="; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep==", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep==";")
		boxSeparator->setCurrentIndex(6);
	else if (sep==",")
		boxSeparator->setCurrentIndex(7);
	else
	{
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

void ConfigDialog::switchToLanguage(int param)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	app->switchToLanguage(param);
	languageChange();
}

void ConfigDialog::insertLanguagesList()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if(!app)
		return;

	boxLanguage->clear();
	QString qmPath = app->d_translations_folder;
	QDir dir(qmPath);
	QStringList locales = app->locales;
	QStringList languages;
	int lang = 0;
	for (int i=0; i < (int)locales.size(); i++)
	{
		if (locales[i] == "en")
			languages.push_back("English");
		else
		{
			QTranslator translator;
			translator.load("qtiplot_"+locales[i], qmPath);

			QString language = translator.translate("ApplicationWindow", "English");
			if (!language.isEmpty())
				languages.push_back(language);
			else
				languages.push_back(locales[i]);
		}

		if (locales[i] == app->appLanguage)
			lang = i;
	}
	boxLanguage->addItems(languages);
	boxLanguage->setCurrentIndex(lang);
}


void ConfigDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked()){
		lblPoints->show();
		generatePointsBox->show();
		linearFit2PointsBox->show();
	} else {
		lblPoints->hide();
		generatePointsBox->hide();
		linearFit2PointsBox->hide();
	}
}

void ConfigDialog::chooseTranslationsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo tfi(app->d_translations_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the QtiPlot translations folder!"),
		tfi.absoluteFilePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		app->d_translations_folder = QDir::toNativeSeparators(dir);
		translationsPathLine->setText(app->d_translations_folder);
		app->createLanguagesList();
		insertLanguagesList();
	}
}

void ConfigDialog::chooseHelpFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo hfi(app->helpFilePath);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the QtiPlot help folder!"),
		hfi.dir().absolutePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		QString helpFilePath = QDir(dir).absoluteFilePath ("index.html");
		if (!QFile(helpFilePath).exists()){
			QMessageBox::critical(this, tr("QtiPlot - index.html File Not Found!"),
					tr("There is no file called <b>index.html</b> in this folder.<br>Please choose another folder!"));
		} else
			app->helpFilePath = helpFilePath;
	}

	helpPathLine->setText(QDir::toNativeSeparators(QFileInfo(app->helpFilePath).dir().absolutePath()));
}

#ifdef SCRIPTING_PYTHON
void ConfigDialog::choosePythonConfigFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo tfi(app->d_python_config_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the Python configuration files!"),
		tfi.absoluteFilePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		app->d_python_config_folder = QDir::toNativeSeparators(dir);
		pythonConfigDirLine->setText(app->d_python_config_folder);

		if (app->scriptingEnv()->name() == QString("Python"))
			app->setScriptingLanguage(QString("Python"), true);
	}
}

void ConfigDialog::chooseStartupScriptsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo tfi(app->d_startup_scripts_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the startup scripts folder!"),
		tfi.absoluteFilePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		app->d_startup_scripts_folder = QDir::toNativeSeparators(dir);
		pythonScriptsDirLine->setText(app->d_startup_scripts_folder);
	}
}

void ConfigDialog::showStartupScriptsFolder(const QString & s)
{
	bool showScriptsFolder = (s == QString("Python"));
	lblPythonScriptsDir->setVisible(showScriptsFolder);
	pythonScriptsDirLine->setVisible(showScriptsFolder);
	browsePythonScriptsBtn->setVisible(showScriptsFolder);
}
#endif

void ConfigDialog::rehighlight()
{
    if (generalDialog->currentWidget() != notesPage)
        return;

    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

    app->d_comment_highlight_color = buttonCommentColor->color();
	app->d_keyword_highlight_color = buttonKeywordColor->color();
	app->d_quotation_highlight_color = buttonQuotationColor->color();
	app->d_numeric_highlight_color = buttonNumericColor->color();
	app->d_function_highlight_color = buttonFunctionColor->color();
	app->d_class_highlight_color = buttonClassColor->color();

    QList<MdiSubWindow *> windows = app->windowsList();
    foreach(MdiSubWindow *w, windows){
        Note *n = qobject_cast<Note *>(w);
        if (n){
        	for (int i = 0; i < n->tabs(); i++)
				n->editor(i)->rehighlight();
        }
    }
}

void ConfigDialog::customizeNotes()
{
    if (generalDialog->currentWidget() != notesPage)
        return;

    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

    app->d_note_line_numbers = lineNumbersBox->isChecked();
    app->d_notes_tab_length = boxTabLength->value();
    QFont f = QFont(boxFontFamily->currentFont().family(), boxFontSize->value());
    f.setBold(buttonBoldFont->isChecked());
    f.setItalic(buttonItalicFont->isChecked());
    app->d_notes_font = f;
    QList<MdiSubWindow *> windows = app->windowsList();
    foreach(MdiSubWindow *w, windows){
        Note *n = qobject_cast<Note *>(w);
        if (n){
            n->showLineNumbers(app->d_note_line_numbers);
            n->setTabStopWidth(app->d_notes_tab_length);
            n->setFont(f);
        }
    }
	app->setFormatBarFont(f);
}

void ConfigDialog::updateMenuList()
{
	QFontMetrics fm(itemsList->font());
	int width = 0;
	for(int i = 0; i<itemsList->count() ; i++){
		int itemWidth = fm.boundingRect(itemsList->item(i)->text()).width();
		if(itemWidth > width)
			width = itemWidth;
	}

	itemsList->setFixedWidth(itemsList->iconSize().width() + width + 50);
}

bool ConfigDialog::validFolderPath(const QString& path)
{
	QFileInfo fi(path);
	if (!fi.exists()){
		QMessageBox::critical(this, tr("QtiPlot - Folder Not Found!"),
		tr("The folder %1 doesn't exist.<br>Please choose another folder!").arg(path));
		return false;
	}

	if (!fi.isDir()){
		QMessageBox::critical(this, tr("QtiPlot - Folder Not Found!"),
		tr("%1 is not a folder.<br>Please choose another folder!").arg(path));
		return false;
	}

	if (!fi.isReadable()){
		QMessageBox::critical(this, tr("QtiPlot"),
		tr("You don't have read access rights to folder %1.<br>Please choose another folder!").arg(path));
		return false;
	}
	return true;
}

void ConfigDialog::initProxyPage()
{
	QNetworkProxy proxy = QNetworkProxy::applicationProxy();

	proxyPage = new QWidget();

	proxyGroupBox = new QGroupBox (tr("&Proxy"));
	proxyGroupBox->setCheckable(true);
	proxyGroupBox->setChecked(!proxy.hostName().isEmpty());

	QGridLayout *gl = new QGridLayout(proxyGroupBox);

	proxyHostLabel = new QLabel( tr("Host"));
    gl->addWidget(proxyHostLabel, 0, 0);
    proxyHostLine = new QLineEdit(proxy.hostName ());
    gl->addWidget(proxyHostLine, 0, 1);

	proxyPortLabel = new QLabel( tr("Port"));
    gl->addWidget(proxyPortLabel, 1, 0);
    proxyPortBox = new QSpinBox;
    proxyPortBox->setMaximum(10000000);
	proxyPortBox->setValue(proxy.port());
    gl->addWidget(proxyPortBox, 1, 1);

	proxyUserLabel = new QLabel( tr("Username"));
    gl->addWidget(proxyUserLabel, 2, 0);
    proxyUserNameLine = new QLineEdit(proxy.user());
    gl->addWidget(proxyUserNameLine, 2, 1);

	proxyPasswordLabel = new QLabel( tr("Password"));
    gl->addWidget(proxyPasswordLabel, 3, 0);
    proxyPasswordLine = new QLineEdit;

    gl->addWidget(proxyPasswordLine, 3, 1);

	gl->setRowStretch(4, 1);

	QVBoxLayout *layout = new QVBoxLayout(proxyPage);
    layout->addWidget(proxyGroupBox);

	appTabWidget->addTab(proxyPage, tr( "&Internet Connection" ) );
}

QNetworkProxy ConfigDialog::setApplicationCustomProxy()
{
	QNetworkProxy proxy;
	proxy.setType(QNetworkProxy::NoProxy);
	if (proxyGroupBox->isChecked())
		proxy.setHostName(proxyHostLine->text());
	else
		proxy.setHostName(QString::null);

	proxy.setPort(proxyPortBox->value());
	proxy.setUser(proxyUserNameLine->text());
	proxy.setPassword(proxyPasswordLine->text());
	QNetworkProxy::setApplicationProxy(proxy);
	return proxy;
}

void ConfigDialog::chooseTexCompiler()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString filter = QString();
#ifdef Q_WS_WIN
	filter = "*.exe";
#endif
	QString compiler = ApplicationWindow::getFileName(this, tr("Choose the location of the LaTeX compiler!"),
	app->d_latex_compiler_path, filter, 0, false);
	if (!compiler.isEmpty()){
		app->d_latex_compiler_path = QDir::toNativeSeparators(compiler);
		texCompilerPathBox->setText(app->d_latex_compiler_path);
	}
}

void ConfigDialog::chooseOffice()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString filter = QString();
#ifdef Q_WS_WIN
	filter = "*.exe";
#endif
	QString compiler = ApplicationWindow::getFileName(this, tr("Choose location"), app->d_soffice_path, filter, 0, false);
	if (!compiler.isEmpty()){
		app->d_soffice_path = QDir::toNativeSeparators(compiler);
		sofficePathBox->setText(app->d_soffice_path);
	}
}

bool ConfigDialog::validateOffice()
{
	QString path = sofficePathBox->text();
	if (path.isEmpty())
		return false;

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	QFileInfo fi(path);
	if (!fi.exists()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("The file %1 doesn't exist.<br>Please choose another file!").arg(path));
		sofficePathBox->setText(app->d_soffice_path);
		return false;
	}

	if (fi.isDir()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("%1 is a folder.<br>Please choose a file!").arg(path));
		sofficePathBox->setText(app->d_soffice_path);
		return false;
	}

	app->d_soffice_path = QDir::toNativeSeparators(path);
	sofficePathBox->setText(app->d_soffice_path);
	return true;
}

void ConfigDialog::chooseJava()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString filter = QString();
#ifdef Q_WS_WIN
	filter = "*.exe";
#endif
	QString compiler = ApplicationWindow::getFileName(this, tr("Choose location"), app->d_java_path, filter, 0, false);
	if (!compiler.isEmpty()){
		app->d_java_path = QDir::toNativeSeparators(compiler);
		javaPathBox->setText(app->d_java_path);
	}
}

bool ConfigDialog::validateJava()
{
	QString path = javaPathBox->text();
	if (path.isEmpty())
		return false;

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	QFileInfo fi(path);
	if (!fi.exists()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("The file %1 doesn't exist.<br>Please choose another file!").arg(path));
		javaPathBox->setText(app->d_java_path);
		return false;
	}

	if (fi.isDir()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("%1 is a folder.<br>Please choose a file!").arg(path));
		javaPathBox->setText(app->d_java_path);
		return false;
	}

	app->d_java_path = QDir::toNativeSeparators(path);
	javaPathBox->setText(app->d_java_path);
	return true;
}

void ConfigDialog::chooseJODConverter()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString compiler = ApplicationWindow::getFileName(this, tr("Choose location"), app->d_jodconverter_path, "*.jar", 0, false);
	if (!compiler.isEmpty()){
		app->d_jodconverter_path = QDir::toNativeSeparators(compiler);
		jodconverterPathBox->setText(app->d_jodconverter_path);
	}
}

bool ConfigDialog::validateJODConverter()
{
	QString path = jodconverterPathBox->text();
	if (path.isEmpty())
		return false;

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	QFileInfo fi(path);
	if (!fi.exists()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("The file %1 doesn't exist.<br>Please choose another file!").arg(path));
		jodconverterPathBox->setText(app->d_jodconverter_path);
		return false;
	}

	if (fi.isDir()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("%1 is a folder.<br>Please choose a file!").arg(path));
		jodconverterPathBox->setText(app->d_jodconverter_path);
		return false;
	}

	app->d_jodconverter_path = QDir::toNativeSeparators(path);
	jodconverterPathBox->setText(app->d_jodconverter_path);
	return true;
}

bool ConfigDialog::validateTexCompiler()
{
	QString path = texCompilerPathBox->text();
	if (path.isEmpty())
		return false;

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	QFileInfo fi(path);
	if (!fi.exists()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("The file %1 doesn't exist.<br>Please choose another file!").arg(path));
		texCompilerPathBox->setText(app->d_latex_compiler_path);
		return false;
	}

	if (fi.isDir()){
		QMessageBox::critical(this, tr("QtiPlot - File Not Found!"),
		tr("%1 is a folder.<br>Please choose a file!").arg(path));
		texCompilerPathBox->setText(app->d_latex_compiler_path);
		return false;
	}

	if (!fi.isReadable()){
		QMessageBox::critical(this, tr("QtiPlot"),
		tr("You don't have read access rights to file %1.<br>Please choose another file!").arg(path));
		texCompilerPathBox->setText(app->d_latex_compiler_path);
		return false;
	}

	app->d_latex_compiler_path = QDir::toNativeSeparators(path);
	texCompilerPathBox->setText(app->d_latex_compiler_path);
	return true;
}

void ConfigDialog::enableMajorGrids(bool on)
{
	btnGrid->setEnabled(on);
	boxMajorGridStyle->setEnabled(on);
	boxMajorGridWidth->setEnabled(on);
}

void ConfigDialog::enableMinorGrids(bool on)
{
	btnGridMinor->setEnabled(on);
	boxMinorGridStyle->setEnabled(on);
	boxMinorGridWidth->setEnabled(on);
}

int ConfigDialog::convertToPixels(double w, FrameWidget::Unit unit, int dimension)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return qRound(w);

	double dpi = (double)app->logicalDpiX();
	if (dimension)
		dpi = (double)app->logicalDpiY();

	switch(unit){
		case FrameWidget::Pixel:
		default:
			return qRound(w);
		break;
		case FrameWidget::Inch:
			return qRound(w*dpi);
		break;
		case FrameWidget::Millimeter:
			return qRound(w*dpi/25.4);
		break;
		case FrameWidget::Centimeter:
			return qRound(w*dpi/2.54);
		break;
		case FrameWidget::Point:
			return qRound(w*dpi/72.0);
		break;
	}
	return qRound(w);
}

double ConfigDialog::convertFromPixels(int w, FrameWidget::Unit unit, int dimension)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return w;

	double dpi = (double)app->logicalDpiX();
	if (dimension)
		dpi = (double)app->logicalDpiY();

	double val = 0.0;
	switch(unit){
		case FrameWidget::Pixel:
		default:
			val = w;
		break;
		case FrameWidget::Inch:
			val = (double)w/dpi;
		break;
		case FrameWidget::Millimeter:
			val = 25.4*w/dpi;
		break;
		case FrameWidget::Centimeter:
			val = 2.54*w/dpi;
		break;
		case FrameWidget::Point:
			val = 72.0*w/dpi;
		break;
	}
	return val;
}

void ConfigDialog::updateCanvasSize(int unit)
{
	if (unit == FrameWidget::Pixel || unit == FrameWidget::Point){
		boxCanvasWidth->setFormat('f', 0);
		boxCanvasHeight->setFormat('f', 0);

		boxCanvasWidth->setSingleStep(1.0);
		boxCanvasHeight->setSingleStep(1.0);
	} else {
		boxCanvasWidth->setFormat('g', 6);
		boxCanvasHeight->setFormat('g', 6);

		boxCanvasWidth->setSingleStep(0.1);
		boxCanvasHeight->setSingleStep(0.1);
	}

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	aspect_ratio = (double)app->d_layer_canvas_width/(double)app->d_layer_canvas_height;

	boxCanvasWidth->setValue(convertFromPixels(app->d_layer_canvas_width, (FrameWidget::Unit)unit, 0));
	boxCanvasHeight->setValue(convertFromPixels(app->d_layer_canvas_height, (FrameWidget::Unit)unit, 1));
}

void ConfigDialog::adjustCanvasHeight(double width)
{
	if (keepRatioBox->isChecked()){
		boxCanvasHeight->blockSignals(true);
		boxCanvasHeight->setValue(width/aspect_ratio);
		boxCanvasHeight->blockSignals(false);
	} else
		aspect_ratio = width/boxCanvasHeight->value();
}

void ConfigDialog::adjustCanvasWidth(double height)
{
	if (keepRatioBox->isChecked()){
		boxCanvasWidth->blockSignals(true);
		boxCanvasWidth->setValue(height*aspect_ratio);
		boxCanvasWidth->blockSignals(false);
	} else
		aspect_ratio = boxCanvasWidth->value()/height;
}

void ConfigDialog::showGridOptions(int axis)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	Grid *grd = app->d_default_2D_grid;
	if (!grd)
		return;

	boxMajorGrid->blockSignals(true);
	boxMinorGrid->blockSignals(true);
	boxWidthMajor->blockSignals(true);
	boxWidthMinor->blockSignals(true);
	boxColorMajor->blockSignals(true);
	boxColorMinor->blockSignals(true);
	boxTypeMajor->blockSignals(true);
	boxTypeMinor->blockSignals(true);
	boxAntialiseGrid->blockSignals(true);

	if (axis == 1) {
		boxMajorGrid->setChecked(grd->xEnabled());
		boxMinorGrid->setChecked(grd->xMinEnabled());

		boxGridXAxis->setEnabled(true);
		boxGridYAxis->setDisabled(true);

		QPen majPenX = grd->majPenX();
		boxTypeMajor->setStyle(majPenX.style());
		boxColorMajor->setColor(majPenX.color());
		boxWidthMajor->setValue(majPenX.widthF());

		QPen minPenX = grd->minPenX();
		boxTypeMinor->setStyle(minPenX.style());
		boxColorMinor->setColor(minPenX.color());
		boxWidthMinor->setValue(minPenX.widthF());
	} else if (axis == 0) {
		boxMajorGrid->setChecked(grd->yEnabled());
		boxMinorGrid->setChecked(grd->yMinEnabled());

		boxGridXAxis->setDisabled(true);
		boxGridYAxis->setEnabled(true);

		QPen majPenY = grd->majPenY();
		boxTypeMajor->setCurrentIndex(majPenY.style() - 1);
		boxColorMajor->setColor(majPenY.color());
		boxWidthMajor->setValue(majPenY.widthF());

		QPen minPenY = grd->minPenY();
		boxTypeMinor->setCurrentItem(minPenY.style() - 1);
		boxColorMinor->setColor(minPenY.color());
		boxWidthMinor->setValue(minPenY.widthF());
	}

	bool majorOn = boxMajorGrid->isChecked();
	boxTypeMajor->setEnabled(majorOn);
	boxColorMajor->setEnabled(majorOn);
	boxWidthMajor->setEnabled(majorOn);

	bool minorOn = boxMinorGrid->isChecked();
	boxTypeMinor->setEnabled(minorOn);
	boxColorMinor->setEnabled(minorOn);
	boxWidthMinor->setEnabled(minorOn);

	boxGridXAxis->setCurrentIndex(grd->xAxis() - 2);
	boxGridYAxis->setCurrentIndex(grd->yAxis());

	boxAntialiseGrid->setChecked(grd->testRenderHint(QwtPlotItem::RenderAntialiased));

	boxAntialiseGrid->blockSignals(false);
	boxWidthMajor->blockSignals(false);
	boxWidthMinor->blockSignals(false);
	boxColorMajor->blockSignals(false);
	boxColorMinor->blockSignals(false);
	boxTypeMajor->blockSignals(false);
	boxTypeMinor->blockSignals(false);
	boxMajorGrid->blockSignals(false);
	boxMinorGrid->blockSignals(false);
}

void ConfigDialog::majorGridEnabled(bool on)
{
	boxTypeMajor->setEnabled(on);
	boxColorMajor->setEnabled(on);
	boxWidthMajor->setEnabled(on);

	updateGrid();
}

void ConfigDialog::minorGridEnabled(bool on)
{
	boxTypeMinor->setEnabled(on);
	boxColorMinor->setEnabled(on);
	boxWidthMinor->setEnabled(on);

	updateGrid();
}

void ConfigDialog::updateGrid()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	Grid *grid = app->d_default_2D_grid;
	if (!grid)
		return;

	if (axesGridList->currentRow() == 1){
		grid->enableX(boxMajorGrid->isChecked());
		grid->enableXMin(boxMinorGrid->isChecked());

		grid->setMajPenX(QPen(boxColorMajor->color(), boxWidthMajor->value(), boxTypeMajor->style()));
		grid->setMinPenX(QPen(boxColorMinor->color(), boxWidthMinor->value(), boxTypeMinor->style()));
	} else {
		grid->enableY(boxMajorGrid->isChecked());
		grid->enableYMin(boxMinorGrid->isChecked());

		grid->setMajPenY(QPen(boxColorMajor->color(), boxWidthMajor->value(), boxTypeMajor->style()));
		grid->setMinPenY(QPen(boxColorMinor->color(), boxWidthMinor->value(), boxTypeMinor->style()));
	}

	grid->setAxis(boxGridXAxis->currentIndex() + 2, boxGridYAxis->currentIndex());
	grid->setRenderHint(QwtPlotItem::RenderAntialiased, boxAntialiseGrid->isChecked());
}

void ConfigDialog::resetDefaultSettings()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	app->setDefaultOptions();
	setApplication(app);

	QString msg = tr("You need to restart QtiPlot before your changes become effective, would you like to do it now?");
	if (QMessageBox::question(this, tr("QtiPlot"), msg, QMessageBox::Ok, QMessageBox::No) == QMessageBox::Ok){
		connect(this, SIGNAL(destroyed()), app, SLOT(newProject()));
		close();
	}
}

void ConfigDialog::setApplication(ApplicationWindow *app)
{
	if (!app)
		return;

	blockSignals(true);

	d_3D_title_font = app->d_3D_title_font;
	d_3D_numbers_font = app->d_3D_numbers_font;
	d_3D_axes_font = app->d_3D_axes_font;
	textFont = app->tableTextFont;
	headerFont = app->tableHeaderFont;
	appFont = app->appFont;
	axesFont = app->plotAxesFont;
	numbersFont = app->plotNumbersFont;
	legendFont = app->plotLegendFont;
	titleFont = app->plotTitleFont;

	//app page
	QStringList styles = QStyleFactory::keys();
	styles.sort();
	boxStyle->clear();
	boxStyle->addItems(styles);
	boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle,Qt::MatchWildcard));

	QStringList llist = ScriptingLangManager::languages();
	boxScriptingLanguage->clear();
	boxScriptingLanguage->insertStringList(llist);
	boxScriptingLanguage->setCurrentItem(llist.indexOf(app->defaultScriptingLang));

	undoStackSizeBox->setValue(app->matrixUndoStackSize());
	boxEndLine->setCurrentIndex((int)app->d_eol);
	boxInitWindow->setCurrentIndex((int)app->d_init_window_type);
	excelImportMethod->setCurrentIndex((int)app->excelImportMethod());

	boxSave->setChecked(app->autoSave);
	boxMinutes->setValue(app->autoSaveTime);
	boxMinutes->setEnabled(app->autoSave);
	boxBackupProject->setChecked(app->d_backup_files);
	boxSearchUpdates->setChecked(app->autoSearchUpdates);
	completionBox->setChecked(app->d_completion);
	openLastProjectBox->setChecked(app->d_open_last_project);

	//confirmations page
	boxFolders->setChecked(app->confirmCloseFolder);
	boxTables->setChecked(app->confirmCloseTable);
	boxMatrices->setChecked(app->confirmCloseMatrix);
	boxPlots2D->setChecked(app->confirmClosePlot2D);
	boxPlots3D->setChecked(app->confirmClosePlot3D);
	boxNotes->setChecked(app->confirmCloseNotes);
	boxPromptRenameTables->setChecked(app->d_inform_rename_table);
	boxTablePasteDialog->setChecked(app->d_show_table_paste_dialog);
	boxConfirmOverwrite->setChecked(app->d_confirm_overwrite);
	boxConfirmModifyDataPoints->setChecked(app->d_confirm_modif_2D_points);

	btnWorkspace->setColor(app->workspaceColor);
	btnPanels->setColor(app->panelsColor);
	btnPanelsText->setColor(app->panelsTextColor);
	boxAppPrecision->setValue(app->d_decimal_digits);
	boxThousandsSeparator->setChecked(app->locale().numberOptions() & QLocale::OmitGroupSeparator);
	boxMuParserCLocale->setChecked(app->d_muparser_c_locale);

	//file locations page
	translationsPathLine->setText(QDir::toNativeSeparators(app->d_translations_folder));
	helpPathLine->setText(QDir::toNativeSeparators(QFileInfo(app->helpFilePath).dir().absolutePath()));
	texCompilerPathBox->setText(QDir::toNativeSeparators(app->d_latex_compiler_path));
	sofficePathBox->setText(QDir::toNativeSeparators(app->d_soffice_path));
	javaPathBox->setText(QDir::toNativeSeparators(app->d_java_path));
	jodconverterPathBox->setText(QDir::toNativeSeparators(app->d_jodconverter_path));
#ifdef SCRIPTING_PYTHON
	pythonConfigDirLine->setText(QDir::toNativeSeparators(app->d_python_config_folder));
	pythonScriptsDirLine->setText(QDir::toNativeSeparators(app->d_startup_scripts_folder));
#endif

	//proxy page
	QNetworkProxy proxy = QNetworkProxy::applicationProxy();
	proxyGroupBox->setChecked(!proxy.hostName().isEmpty());
	proxyHostLine->setText(proxy.hostName ());
	proxyPortBox->setValue(proxy.port());
	proxyUserNameLine->setText(proxy.user());

	//tables page
	buttonBackground->setColor(app->tableBkgdColor);
	buttonText->setColor(app->tableTextColor);
	buttonHeader->setColor(app->tableHeaderColor);
	boxTableComments->setChecked(app->d_show_table_comments);
	boxUpdateTableValues->setChecked(app->autoUpdateTableValues());

	//plots page
	boxAutoscaling->setChecked(app->autoscale2DPlots);
	boxScaleFonts->setChecked(app->autoScaleFonts);
	boxTitle->setChecked(app->titleOn);
	boxFrame->setChecked(app->canvasFrameWidth > 0);
	boxFrameWidth->setValue(app->canvasFrameWidth);
	if (!app->canvasFrameWidth){
		labelFrameWidth->hide();
		boxFrameWidth->hide();
	}

	boxMargin->setValue(app->defaultPlotMargin);
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
	legendDisplayBox->setCurrentIndex(app->d_graph_legend_display);
	attachToBox->setCurrentIndex(app->d_graph_attach_policy);

	//curves page
	boxCurveLineWidth->setLocale(app->locale());
	boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
	lineStyleBox->setCurrentIndex(app->d_curve_line_style);
	patternBox->setCurrentIndex(app->defaultCurveBrush);
	curveAlphaBox->setValue(app->defaultCurveAlpha);
	symbolBox->setCurrentIndex(app->d_symbol_style);
	symbolBox->setDisabled(app->d_indexed_symbols);
	boxSymbolSize->setValue(app->defaultSymbolSize/2);
	symbolEdgeBox->setValue(app->defaultSymbolEdge);
	fillSymbolsBox->setChecked(app->d_fill_symbols);

	d_indexed_colors = app->indexedColors();
	d_indexed_color_names = app->indexedColorNames();
	setColorsList(d_indexed_colors, d_indexed_color_names);

	d_indexed_symbols = app->indexedSymbols();
	setSymbolsList(d_indexed_symbols);
	groupIndexedSymbols->setChecked(app->d_indexed_symbols);

	//axes page
	boxBackbones->setChecked(app->drawBackbones);
	boxSynchronizeScales->setChecked(app->d_synchronize_graph_scales);
	boxLineWidth->setValue(app->axesLineWidth);
	boxAxesLabelsDist->setValue(app->d_graph_axes_labels_dist);
	boxTickLabelsDist->setValue(app->d_graph_tick_labels_dist);

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		int row = i + 1;
		bool enabledAxis = app->d_show_axes[i];

		QLayoutItem *item = enabledAxesGrid->itemAtPosition(row, 2);
		if (item && item->widget())
			((QCheckBox *)item->widget())->setChecked(enabledAxis);

		item = enabledAxesGrid->itemAtPosition(row, 3);
		if (item && item->widget()){
			QCheckBox *box = qobject_cast<QCheckBox *>(item->widget());
			box->setChecked(app->d_show_axes_labels[i]);
			box->setEnabled(enabledAxis);
		}
	}

	showGridOptions(0);

	//layer speed page
	antialiasingGroupBox->setChecked(app->antialiasing2DPlots);
	disableAntialiasingBox->setChecked(app->d_disable_curve_antialiasing);
	curveSizeBox->setValue(app->d_curve_max_antialising_size);

	//layer geometry page
	keepRatioBox->setChecked(app->d_keep_aspect_ration);
	updateCanvasSize((FrameWidget::Unit)app->d_layer_geometry_unit);

	boxMajTicksLength->setValue(app->majTicksLength);
	boxMinTicksLength->setValue(app->minTicksLength);
	boxScaleLayersOnPrint->setChecked(app->d_scale_plots_on_print);
	boxPrintCropmarks->setChecked(app->d_print_cropmarks);

	//3D plots page
	boxResolution->setValue(app->d_3D_resolution);
	boxProjection->setCurrentIndex(app->d_3D_projection);
	boxShowLegend->setChecked(app->d_3D_legend);
	boxSmoothMesh->setChecked(app->d_3D_smooth_mesh);
	boxOrthogonal->setChecked(app->d_3D_orthogonal);
	boxAutoscale3DPlots->setChecked(app->d_3D_autoscale);
	colorMapEditor = new ColorMapEditor(app->locale());
	colorMapEditor->setColorMap(app->d_3D_color_map);
	btnAxes->setColor(app->d_3D_axes_color);
	btnLabels->setColor(app->d_3D_labels_color);
	btnNumbers->setColor(app->d_3D_numbers_color);
	btnMesh->setColor(app->d_3D_mesh_color);
	btnBackground3D->setColor(app->d_3D_background_color);
	boxMajorGrids->setChecked(app->d_3D_major_grids);
	boxMinorGrids->setChecked(app->d_3D_minor_grids);
	btnGrid->setColor(app->d_3D_grid_color);
	btnGridMinor->setColor(app->d_3D_minor_grid_color);
	boxMajorGridWidth->setLocale(app->locale());
	boxMajorGridWidth->setValue(app->d_3D_major_width);
	boxMinorGridWidth->setLocale(app->locale());
	boxMinorGridWidth->setValue(app->d_3D_minor_width);
	enableMajorGrids(app->d_3D_major_grids);
	enableMinorGrids(app->d_3D_minor_grids);

	//notes page
	boxTabLength->setValue(app->d_notes_tab_length);
	boxFontFamily->setCurrentFont(app->d_notes_font);
	boxFontSize->setValue(app->d_notes_font.pointSize());
	buttonBoldFont->setChecked(app->d_notes_font.bold());
	buttonItalicFont->setChecked(app->d_notes_font.italic());
	lineNumbersBox->setChecked(app->d_note_line_numbers);
	buttonCommentColor->setColor(app->d_comment_highlight_color);
	buttonKeywordColor->setColor(app->d_keyword_highlight_color);
	buttonQuotationColor->setColor(app->d_quotation_highlight_color);
	buttonNumericColor->setColor(app->d_numeric_highlight_color);
	buttonFunctionColor->setColor(app->d_function_highlight_color);
	buttonClassColor->setColor(app->d_class_highlight_color);

	//fits page
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	generatePointsBox->setValue(app->fitPoints);
	linearFit2PointsBox->setChecked(app->d_2_linear_fit_points);
	showPointsBox(!app->generateUniformFitPoints);
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	groupBoxMultiPeak->setChecked(app->generatePeakCurves);
	boxPeaksColor->setColor(app->peakCurvesColor);
	boxPrecision->setValue(app->fit_output_precision);
	logBox->setChecked(app->writeFitResultsToLog);
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	scaleErrorsBox->setChecked(app->fit_scale_errors);
	boxMultiPeakMsgs->setChecked(app->d_multi_peak_messages);

	languageChange();

	blockSignals(false);
}
