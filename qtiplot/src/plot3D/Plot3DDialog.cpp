/***************************************************************************
    File                 : Plot3DDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Surface plot options dialog

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
#include "Plot3DDialog.h"
#include "ColorMapPreviewDialog.h"
#include <MyParser.h>
#include <SymbolDialog.h>
#include <ApplicationWindow.h>
#include <TextFormatButtons.h>
#include <DoubleSpinBox.h>
#include <ColorMapEditor.h>
#include <ColorButton.h>

#include <QListWidget>
#include <QTextEdit>
#include <QLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QWidget>
#include <QMessageBox>
#include <QComboBox>
#include <QWidgetList>
#include <QFileDialog>
#include <QGroupBox>
#include <QFontDialog>
#include <QApplication>

#include <qwt3d_color.h>

Plot3DDialog::Plot3DDialog( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setName( "Plot3DDialog" );
	setWindowTitle( tr( "QtiPlot - Surface Plot Options" ) );
	setSizeGripEnabled(true);

	bars=0; points=0;

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addStretch();
	btnTable = new QPushButton();
    hbox->addWidget(btnTable);
	buttonApply = new QPushButton(tr( "&Apply" ));
    hbox->addWidget(buttonApply);
	buttonOk = new QPushButton(tr( "&OK" ));
	buttonOk->setDefault( true );
    hbox->addWidget(buttonOk);
	buttonCancel = new QPushButton(tr( "&Cancel" ));
    hbox->addWidget(buttonCancel);

    generalDialog = new QTabWidget();

    initScalesPage();
	initAxesPage();
	initGridPage();
	initTitlePage();
	initColorsPage();
	initGeneralPage();
	initPrintPage();

	QVBoxLayout* vl = new QVBoxLayout(this);
	vl->addWidget(generalDialog);
	vl->addLayout(hbox);

	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT(updatePlot() ) );
	connect( btnTable, SIGNAL( clicked() ), this, SLOT(worksheet() ) );
}

void Plot3DDialog::initPrintPage()
{
	printPage = new QWidget();
	QVBoxLayout *printLayout = new QVBoxLayout( printPage );

	boxScaleOnPrint = new QCheckBox(tr("Scale to paper si&ze"));
	printLayout->addWidget( boxScaleOnPrint );

	boxPrintCropmarks = new QCheckBox(tr("Print Crop&marks"));
	printLayout->addWidget( boxPrintCropmarks );
	printLayout->addStretch();

	generalDialog->addTab(printPage, tr( "&Print" ) );
}

void Plot3DDialog::initScalesPage()
{
	axesList = new QListWidget();
	axesList->addItem(tr( "X" ) );
	axesList->addItem(tr( "Y" ) );
	axesList->addItem(tr( "Z" ) );
	axesList->setFixedWidth(50);
	axesList->setCurrentRow(0);

	ApplicationWindow *app = (ApplicationWindow *)parent();

	QGridLayout *gl1 = new QGridLayout();
	gl1->addWidget(new QLabel(tr("From")), 0, 0);
	boxFrom = new DoubleSpinBox();
	boxFrom->setLocale(app->locale());
	boxFrom->setDecimals(app->d_decimal_digits);
	gl1->addWidget(boxFrom, 0, 1);

	gl1->addWidget(new QLabel(tr("To")), 1, 0);
	boxTo = new DoubleSpinBox();
	boxTo->setLocale(app->locale());
	boxTo->setDecimals(app->d_decimal_digits);
	gl1->addWidget(boxTo, 1, 1);

	gl1->addWidget(new QLabel(tr("Type")), 2, 0);
	boxType=new QComboBox();
	boxType->addItem(tr("linear"));
	boxType->addItem(tr("logarithmic"));
	boxType->setMaximumWidth(150);
	gl1->addWidget(boxType, 2, 1);
	gl1->setRowStretch(3, 1);

	QGroupBox *gb1 = new QGroupBox();
	gb1->setLayout(gl1);

	QGridLayout *gl2 = new QGridLayout();
	gl2->addWidget(new QLabel(tr("Major Ticks")), 0, 0);
	boxMajors = new QSpinBox();
	gl2->addWidget(boxMajors, 0, 1);
	gl2->addWidget(new QLabel(tr("Minor Ticks")), 1, 0);
	boxMinors = new QSpinBox();
	gl2->addWidget(boxMinors, 1, 1);
	gl2->setRowStretch(2, 1);

	TicksGroupBox = new QGroupBox();
	TicksGroupBox->setLayout(gl2);

	QGroupBox *tickLabelsGroupBox = new QGroupBox(tr("Tick Labels"));
	QGridLayout *gl3 = new QGridLayout(tickLabelsGroupBox);
	gl3->addWidget(new QLabel(tr("Format")), 0, 0);
	boxTickLabelsFormat = new QComboBox();
	boxTickLabelsFormat->insertItem(tr("Automatic"));
	boxTickLabelsFormat->insertItem(tr("Decimal: 10000.0"));
	boxTickLabelsFormat->insertItem(tr("Scientific: 1e4"));
	boxTickLabelsFormat->insertItem(tr("Engineering: 10k"));

	gl3->addWidget(boxTickLabelsFormat, 0, 1);
	gl3->addWidget(new QLabel(tr("Precision")), 1, 0);
	boxPrecision = new QSpinBox();
	boxPrecision->setMaximum(14);
	gl3->addWidget(boxPrecision, 1, 1);
	gl3->setRowStretch(2, 1);

	QVBoxLayout* vb = new QVBoxLayout();
	vb->addWidget(TicksGroupBox);
	vb->addWidget(tickLabelsGroupBox);

	QHBoxLayout* hb = new QHBoxLayout();
	hb->addWidget(axesList);
	hb->addWidget(gb1, 1);
	hb->addLayout(vb, 1);

	scale = new QWidget();
	scale->setLayout(hb);
	generalDialog->insertTab(scale, tr( "&Scale" ) );
}

void Plot3DDialog::initAxesPage()
{
	axesList2 = new QListWidget();
	axesList2->addItem(tr( "X" ) );
	axesList2->addItem(tr( "Y" ) );
	axesList2->addItem(tr( "Z" ) );
	axesList2->setFixedWidth(50);
	axesList2->setCurrentRow(0);

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Title")), 0, 0);
	boxLabel = new QTextEdit();
	boxLabel->setMaximumHeight(60);
    gl1->addWidget(boxLabel, 0, 1);
    gl1->addWidget(new QLabel(tr("Axis Font")), 1, 0);

    QHBoxLayout* hb1 = new QHBoxLayout();
	btnLabelFont = new QPushButton(tr( "&Font" ));
	btnLabelFont->setIcon(QIcon(":/font.png"));
    hb1->addWidget(btnLabelFont);

	axisTitleFormatButtons = new TextFormatButtons(boxLabel);
	hb1->addWidget(axisTitleFormatButtons);

    hb1->addStretch();
    gl1->addLayout(hb1, 1, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent();

    gl1->addWidget(new QLabel(tr("Major Ticks Length")), 2, 0);
	boxMajorLength = new DoubleSpinBox();
	boxMajorLength->setLocale(app->locale());
    boxMajorLength->setDecimals(app->d_decimal_digits);
	boxMajorLength->setMinimum(0.0);
    gl1->addWidget(boxMajorLength, 2, 1);

    gl1->addWidget(new QLabel(tr("Minor Ticks Length")), 3, 0);
	boxMinorLength = new DoubleSpinBox();
	boxMinorLength->setLocale(app->locale());
    boxMinorLength->setDecimals(app->d_decimal_digits);
	boxMinorLength->setMinimum(0.0);

    gl1->addWidget(boxMinorLength, 3, 1);
    gl1->setRowStretch(4, 1);

    QGroupBox *gb1 = new QGroupBox();
    gb1->setLayout(gl1);

	QHBoxLayout* hb2 = new QHBoxLayout();
	hb2->addWidget(axesList2);
	hb2->addWidget(gb1);

    axes = new QWidget();
    axes->setLayout(hb2);
	generalDialog->insertTab(axes, tr( "&Axis" ) );

	connect( btnLabelFont, SIGNAL(clicked()), this, SLOT(pickAxisLabelFont()));
}

void Plot3DDialog::initTitlePage()
{
    QHBoxLayout* hb1 = new QHBoxLayout();
    hb1->addStretch();
    QLabel *colorLabel = new QLabel(tr( "Co&lor" ));
    hb1->addWidget(colorLabel);
    btnTitleColor = new ColorButton();
    hb1->addWidget(btnTitleColor);
    colorLabel->setBuddy(btnTitleColor);

    btnTitleFont = new QPushButton(tr( "&Font" ));
	btnTitleFont->setIcon(QIcon(":/font.png"));
    hb1->addWidget(btnTitleFont);

	QVBoxLayout* vl = new QVBoxLayout();
	boxTitle = new QTextEdit();
	boxTitle->setMaximumHeight(80);
	vl->addWidget(boxTitle);

	titleFormatButtons = new TextFormatButtons(boxTitle);
	hb1->addWidget(titleFormatButtons);
    hb1->addStretch();

    vl->addLayout(hb1);
    vl->addStretch();

    title = new QWidget();
    title->setLayout(vl);
	generalDialog->insertTab(title, tr( "&Title" ) );

	connect( btnTitleFont, SIGNAL(clicked()), this, SLOT(pickTitleFont() ) );
}

void Plot3DDialog::initColorsPage()
{
	linearColorMapGroupBox = new QGroupBox(tr( "Linea&r color map" ));
	linearColorMapGroupBox->setCheckable(true);
	connect(linearColorMapGroupBox, SIGNAL(clicked(bool)),
			this, SLOT(updateColorMapFileGroupBox(bool)));

	QHBoxLayout* hb = new QHBoxLayout(linearColorMapGroupBox);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	d_color_map_editor = new ColorMapEditor(app->locale(), app->d_decimal_digits);
	hb->addWidget(d_color_map_editor);

    colorMapFileGroupBox = new QGroupBox(tr( "Color map &file" ));
	colorMapFileGroupBox->setCheckable(true);
	connect(colorMapFileGroupBox, SIGNAL(clicked(bool)), this,
			SLOT(updateLinearColorMapGroupBox(bool)));

	QGridLayout* layout = new QGridLayout(colorMapFileGroupBox);

	btnColorMap = new QPushButton();
	btnColorMap->setIcon(QIcon(":/folder_open.png"));
	layout->addWidget(btnColorMap, 0, 0);

	colorMapPreviewLabel = new QLabel(tr("None"));
	colorMapPreviewLabel->setScaledContents(true);
	colorMapPreviewLabel->setFrameShape( QFrame::StyledPanel );
	colorMapPreviewLabel->setFrameShadow( QFrame::Sunken );
	layout->addWidget(colorMapPreviewLabel, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setColumnStretch(1, 1);

    QGridLayout* vl2 = new QGridLayout();

	transparencySlider = new QSlider();
	transparencySlider->setOrientation(Qt::Horizontal);
	transparencySlider->setRange(0, 100);

	boxTransparency = new QSpinBox();
	boxTransparency->setRange(0, 100);
	boxTransparency->setSuffix(" %");

	QHBoxLayout* hb0 = new QHBoxLayout();
	hb0->addWidget(transparencySlider);
	hb0->addWidget(boxTransparency);
	vl2->addLayout(hb0, 0, 1);

	QLabel *l1 = new QLabel("&" + tr( "Opacity" ));
	l1->setBuddy(boxTransparency);
	vl2->addWidget(l1, 0, 0);

    btnMesh = new ColorButton();
    QLabel *meshLabel = new QLabel(tr( "&Line" ));
    meshLabel->setBuddy(btnMesh);
    vl2->addWidget(meshLabel, 1, 0);
    vl2->addWidget(btnMesh, 1, 1);

    btnBackground = new ColorButton();
    QLabel *backgroundLabel = new QLabel(tr( "&Background" ));
    backgroundLabel->setBuddy(btnBackground);
    vl2->addWidget(backgroundLabel, 2, 0);
    vl2->addWidget(btnBackground, 2, 1);
    vl2->setRowStretch(3, 1);

    QGroupBox *gb2 = new QGroupBox(tr( "General" ));
    gb2->setLayout(vl2);

    QGridLayout *gl1 = new QGridLayout();
    btnAxes = new ColorButton();
    QLabel *axesLabel = new QLabel(tr( "A&xes" ));
    axesLabel->setBuddy(btnAxes);
    gl1->addWidget(axesLabel, 0, 0);
    gl1->addWidget(btnAxes, 0, 1);

    btnLabels = new ColorButton();
    QLabel *labLabels = new QLabel(tr( "Lab&els" ));
    labLabels->setBuddy(btnLabels);
    gl1->addWidget(labLabels, 1, 0);
    gl1->addWidget(btnLabels, 1, 1);

	btnNumbers = new ColorButton();
	QLabel *numbersLabel = new QLabel(tr( "&Numbers" ));
    numbersLabel->setBuddy(btnNumbers);
	gl1->addWidget(numbersLabel, 2, 0);
    gl1->addWidget(btnNumbers, 2, 1);
    gl1->setRowStretch(3, 1);

    AxesColorGroupBox = new QGroupBox(tr( "Coordinate System" ));
    AxesColorGroupBox->setLayout(gl1);

	QVBoxLayout *vl1 = new QVBoxLayout();
	vl1->addWidget(gb2);
	vl1->addWidget(AxesColorGroupBox);

    QHBoxLayout* hb1 = new QHBoxLayout();
	hb1->addWidget(linearColorMapGroupBox);
	hb1->addLayout(vl1);

    QVBoxLayout *vl0 = new QVBoxLayout();
	vl0->addLayout(hb1);
	vl0->addWidget(colorMapFileGroupBox);

    colors = new QWidget();
    colors->setLayout(vl0);
	generalDialog->insertTab(colors, tr( "&Colors" ) );

	connect(btnColorMap, SIGNAL(clicked()), this, SLOT(pickDataColorMap()));
	connect(transparencySlider, SIGNAL(valueChanged(int)), boxTransparency, SLOT(setValue(int)));
	connect(boxTransparency, SIGNAL(valueChanged(int)), transparencySlider, SLOT(setValue(int)));
	connect(boxTransparency, SIGNAL(valueChanged(int)), this, SLOT(changeTransparency(int)));
}

void Plot3DDialog::initGeneralPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	QGridLayout *gl1 = new QGridLayout();
	boxLegend = new QCheckBox(tr("Show Legend"));
	gl1->addWidget(boxLegend, 0, 0);
	boxOrthogonal = new QCheckBox(tr("Orthogonal"));
	gl1->addWidget(boxOrthogonal, 0, 1);

	gl1->addWidget(new QLabel(tr( "Line Width" )), 1, 0);
	boxMeshLineWidth = new QDoubleSpinBox();
	boxMeshLineWidth->setDecimals(1);
	boxMeshLineWidth->setSingleStep(0.1);
	boxMeshLineWidth->setRange(0, 100);
	gl1->addWidget(boxMeshLineWidth, 1, 1);

	gl1->addWidget(new QLabel( tr( "Resolution" )), 2, 0);
	boxResolution = new QSpinBox();
	boxResolution->setRange(1, 100);
	boxResolution->setSpecialValueText( "1 (all data)" );
	gl1->addWidget(boxResolution, 2, 1);

	gl1->addWidget(new QLabel( tr( "Numbers Font" )), 3, 0);
	btnNumbersFont = new QPushButton(tr( "&Font" ));
	btnNumbersFont->setIcon(QIcon(":/font.png"));
	gl1->addWidget(btnNumbersFont, 3, 1);

	gl1->addWidget(new QLabel( tr( "Distance labels - axis" )), 4, 0);
	boxDistance = new QSpinBox();
	boxDistance->setRange(-1000, 1000);
	boxDistance->setSingleStep(5);
	gl1->addWidget(boxDistance, 4, 1);

	boxZoom = new QSpinBox();
	boxZoom->setRange(1, 10000);
	boxZoom->setSingleStep(10);
	gl1->addWidget(boxZoom, 5, 1);

	QLabel *l1 = new QLabel("&" + tr("Zoom (%)"));
	l1->setBuddy(boxZoom);
	gl1->addWidget(l1, 5, 0);
	gl1->setRowStretch(6, 1);

	QGroupBox *gb1 = new QGroupBox();
	gb1->setLayout(gl1);

	QGridLayout *gl2 = new QGridLayout();

	gl2->addWidget(new QLabel(tr( "Zoom (%)" )), 0, 1);
	gl2->addWidget(new QLabel(tr( "Rotation" )), 0, 2);

	QLabel *xLabel = new QLabel("&" + tr("X"));
	gl2->addWidget(xLabel, 1, 0);
	boxXScale = new QSpinBox();
	boxXScale->setRange(1, 10000);
	boxXScale->setSingleStep(10);
	xLabel->setBuddy(boxXScale);
	gl2->addWidget(boxXScale, 1, 1);

	boxXRotation = new DoubleSpinBox('f');
	boxXRotation->setDecimals(1);
	boxXRotation->setSingleStep(1.0);
	boxXRotation->setLocale(app->locale());
	boxXRotation->setRange(-360, 360);
	gl2->addWidget(boxXRotation, 1, 2);

	QLabel *yLabel = new QLabel("&" + tr("Y"));
	gl2->addWidget(yLabel, 2, 0);
	boxYScale = new QSpinBox();
	boxYScale->setRange(1, 10000);
	boxYScale->setSingleStep(10);
	gl2->addWidget(boxYScale, 2, 1);
	yLabel->setBuddy(boxYScale);

	boxYRotation = new DoubleSpinBox('f');
	boxYRotation->setDecimals(1);
	boxYRotation->setSingleStep(1.0);
	boxYRotation->setLocale(app->locale());
	boxYRotation->setRange(-360, 360);
	gl2->addWidget(boxYRotation, 2, 2);

	QLabel *zLabel = new QLabel("&" + tr("Z"));
	gl2->addWidget(zLabel, 3, 0);
	boxZScale = new QSpinBox();
	boxZScale->setRange(1, 10000);
	boxZScale->setSingleStep(10);
	gl2->addWidget(boxZScale, 3, 1);
	zLabel->setBuddy(boxZScale);

	gl2->setRowStretch(4, 1);

	boxZRotation = new DoubleSpinBox('f');
	boxZRotation->setDecimals(1);
	boxZRotation->setSingleStep(1.0);
	boxZRotation->setLocale(app->locale());
	boxZRotation->setRange(-360, 360);
	gl2->addWidget(boxZRotation, 3, 2);

	QGroupBox *gb2 = new QGroupBox();
	gb2->setLayout(gl2);

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(gb1);
	hl->addWidget(gb2);

	general = new QWidget();
	general->setLayout(hl);
	generalDialog->insertTab(general, tr("&General"));

	connect( boxZoom, SIGNAL(valueChanged(int)), this, SLOT(changeZoom(int)));
	connect( boxXScale, SIGNAL(valueChanged(int)), this, SLOT(changeZoom(int)));
	connect( boxYScale, SIGNAL(valueChanged(int)), this, SLOT(changeZoom(int)));
	connect( boxZScale, SIGNAL(valueChanged(int)), this, SLOT(changeZoom(int)));
	connect( btnNumbersFont, SIGNAL(clicked()), this, SLOT(pickNumbersFont()));
	connect( boxXRotation, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect( boxYRotation, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect( boxZRotation, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
}

void Plot3DDialog::enableMajorGrids(bool on)
{
	btnGrid->setEnabled(on);
	boxMajorGridStyle->setEnabled(on);
	boxMajorGridWidth->setEnabled(on);
}

void Plot3DDialog::enableMinorGrids(bool on)
{
	btnGridMinor->setEnabled(on);
	boxMinorGridStyle->setEnabled(on);
	boxMinorGridWidth->setEnabled(on);
}

void Plot3DDialog::initGridPage()
{
	QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);

	boxMajorGrids = new QCheckBox(tr("Ma&jor Grids"));
	gl1->addWidget(boxMajorGrids, 0, 1);

	boxMinorGrids = new QCheckBox(tr("Mi&nor Grids"));
	gl1->addWidget(boxMinorGrids, 0, 2);

    gl1->addWidget(new QLabel(tr("Color")), 1, 0);

	btnGrid = new ColorButton();
    gl1->addWidget(btnGrid, 1, 1);

    btnGridMinor = new ColorButton();
    gl1->addWidget(btnGridMinor, 1, 2);

    gl1->addWidget(new QLabel(tr("Style")), 2, 0);

	boxMajorGridStyle = new QComboBox();
	boxMajorGridStyle->addItem(tr("Solid"));
	boxMajorGridStyle->addItem(tr("Dash"));
	boxMajorGridStyle->addItem(tr("Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMajorGridStyle->addItem(tr("Short Dash"));
	boxMajorGridStyle->addItem(tr("Short Dot"));
	boxMajorGridStyle->addItem(tr("Short Dash Dot"));

	gl1->addWidget(boxMajorGridStyle, 2, 1);

	boxMinorGridStyle = new QComboBox();
	boxMinorGridStyle->addItem(tr("Solid"));
	boxMinorGridStyle->addItem(tr("Dash"));
	boxMinorGridStyle->addItem(tr("Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMinorGridStyle->addItem(tr("Short Dash"));
	boxMinorGridStyle->addItem(tr("Short Dot"));
	boxMinorGridStyle->addItem(tr("Short Dash Dot"));

	gl1->addWidget(boxMinorGridStyle, 2, 2);

	gl1->addWidget(new QLabel(tr("Width")), 3, 0);

	ApplicationWindow *app = (ApplicationWindow *)parent();

	boxMajorGridWidth = new DoubleSpinBox();
	boxMajorGridWidth->setLocale(app->locale());
	boxMajorGridWidth->setMinimum(0.0);
	gl1->addWidget(boxMajorGridWidth, 3, 1);

	boxMinorGridWidth = new DoubleSpinBox();
	boxMinorGridWidth->setLocale(app->locale());
	boxMinorGridWidth->setMinimum(0.0);
	gl1->addWidget(boxMinorGridWidth, 3, 2);

	gl1->setRowStretch(4, 1);
	gl1->setColumnStretch(3, 1);

    QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(gb1);

    gridPage = new QWidget();
    gridPage->setLayout(hl);
	generalDialog->insertTab(gridPage, tr("G&rid"));
}

void Plot3DDialog::initPointsOptionsStack()
{
    QHBoxLayout* hl1 = new QHBoxLayout();
    hl1->addStretch();
	hl1->addWidget(new QLabel( tr( "Style" )));
	boxPointStyle = new QComboBox();
	boxPointStyle->addItem(tr("Dot"));
	boxPointStyle->addItem(tr("Cross Hair"));
	boxPointStyle->addItem(tr("Cone"));
    hl1->addWidget(boxPointStyle);

	optionStack = new QStackedWidget();
	optionStack->setFrameShape( QFrame::StyledPanel );
	optionStack->setFrameShadow( QStackedWidget::Plain );

	ApplicationWindow *app = (ApplicationWindow *)parent();

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel( tr( "Width" )), 0, 0);
	boxSize = new DoubleSpinBox();
	boxSize->setLocale(app->locale());
    boxSize->setDecimals(app->d_decimal_digits);
    boxSize->setMinimum(0.0);
	boxSize->setValue(5);
    gl1->addWidget(boxSize, 0, 1);

	gl1->addWidget(new QLabel( tr( "Smooth angles" )), 1, 0);
	boxSmooth = new QCheckBox();
	boxSmooth->setChecked(false);
    gl1->addWidget(boxSmooth, 1, 1);
    gl1->setRowStretch(2, 1);

    dotsPage = new QWidget();
    dotsPage->setLayout(gl1);
	optionStack->addWidget(dotsPage);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr( "Radius" )), 0, 0);
	boxCrossRad = new DoubleSpinBox();
	boxCrossRad->setLocale(app->locale());
    boxCrossRad->setDecimals(app->d_decimal_digits);
	boxCrossRad->setValue(0.01);
	boxCrossRad->setMinimum(0.0);
    gl2->addWidget(boxCrossRad, 0, 1);

	gl2->addWidget(new QLabel(tr( "Line Width")), 1, 0);
	boxCrossLinewidth = new DoubleSpinBox();
	boxCrossLinewidth->setLocale(app->locale());
    boxCrossLinewidth->setDecimals(app->d_decimal_digits);
	boxCrossLinewidth->setValue(1);
	boxCrossLinewidth->setMinimum(0.0);
    gl2->addWidget(boxCrossLinewidth, 1, 1);

	gl2->addWidget(new QLabel(tr( "Smooth line" )), 2, 0);
	boxCrossSmooth = new QCheckBox();
    boxCrossSmooth->setChecked(true);
    gl2->addWidget(boxCrossSmooth, 2, 1);
	gl2->addWidget(new QLabel(tr( "Boxed" )), 3, 0);
	boxBoxed = new QCheckBox();
	boxBoxed->setChecked(false);
    gl2->addWidget(boxBoxed, 3, 1);
    gl2->setRowStretch(4, 1);

	crossPage = new QWidget();
    crossPage->setLayout(gl2);
	optionStack->addWidget(crossPage);

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel( tr( "Width" )), 0, 0);
	boxConesRad = new DoubleSpinBox();
	boxConesRad->setLocale(app->locale());
    boxConesRad->setDecimals(app->d_decimal_digits);
	boxConesRad->setValue(0.5);
	boxConesRad->setMinimum(0.0);
    gl3->addWidget(boxConesRad, 0, 1);

    gl3->addWidget(new QLabel( tr( "Quality" )), 1, 0);
	boxQuality = new QSpinBox();
    boxQuality->setRange(0, 40);
	boxQuality->setValue(32);
    gl3->addWidget(boxQuality, 1, 1);
    gl3->setRowStretch(2, 1);

    conesPage = new QWidget();
    conesPage->setLayout(gl3);
	optionStack->addWidget(conesPage);

	QVBoxLayout* vl = new QVBoxLayout();
    vl->addLayout(hl1);
    vl->addWidget(optionStack);

    points = new QWidget();
    points->setLayout(vl);

	generalDialog->insertTab(points, tr( "Points" ),4 );
	connect( boxPointStyle, SIGNAL( activated(int) ), optionStack, SLOT( setCurrentIndex(int) ) );
}

void Plot3DDialog::setPlot(Graph3D *g)
{
	if (!g)
		return;

	d_plot = g;

	d_color_map_editor->setRange(g->zStart(), g->zStop());
	d_color_map_editor->setColorMap(g->colorMap());
	btnTitleColor->setColor(g->titleColor());
	btnMesh->setColor(g->meshColor());
	btnAxes->setColor(g->axesColor());
	btnNumbers->setColor(g->numColor());
	btnLabels->setColor(g->labelColor());
	btnBackground->setColor(g->bgColor());

	//Set major grid properties
	Qwt3D::GridLine majorGridLine = g->surface()->coordinates()->majorGridLine(Qwt3D::X1);
	boxMajorGrids->setChecked(majorGridLine.visible_);
	RGBA color = majorGridLine.color_;
	btnGrid->setColor(GL2Qt(color.r, color.g, color.b));
	boxMajorGridWidth->setValue(majorGridLine.width_);
	boxMajorGridStyle->setCurrentIndex(majorGridLine.style_);

	//Set minor grid properties
	Qwt3D::GridLine minorGridLine = g->surface()->coordinates()->minorGridLine(Qwt3D::X1);
	boxMinorGrids->setChecked(minorGridLine.visible_);
	color = minorGridLine.color_;
	btnGridMinor->setColor(GL2Qt(color.r, color.g, color.b));
	boxMinorGridWidth->setValue(minorGridLine.width_);
	boxMinorGridStyle->setCurrentIndex(minorGridLine.style_);

	d_color_map_file = g->colorMapFile();
	setColorMapPreview(d_color_map_file);
	linearColorMapGroupBox->setChecked(d_color_map_file.isEmpty());
	colorMapFileGroupBox->setChecked(!d_color_map_file.isEmpty());

	boxMeshLineWidth->setValue(g->meshLineWidth());

	boxTransparency->blockSignals(true);
	boxTransparency->setValue(int(100*g->transparency()));
	boxTransparency->blockSignals(false);
	transparencySlider->blockSignals(true);
	transparencySlider->setValue(boxTransparency->value());
	transparencySlider->blockSignals(false);

	boxTitle->setText(g->plotTitle());
	titleFont = g->titleFont();

    xScale = d_plot->xScale();
    yScale = d_plot->yScale();
    zScale = d_plot->zScale();
    zoom = d_plot->zoom();

    boxZoom->setValue(100);
	boxXScale->setValue(100);
	boxYScale->setValue(100);
	boxZScale->setValue(100);

	boxXRotation->blockSignals(true);
	boxXRotation->setValue(g->xRotation());
	boxXRotation->blockSignals(false);
	boxYRotation->blockSignals(true);
	boxYRotation->setValue(g->yRotation());
	boxYRotation->blockSignals(false);
	boxZRotation->blockSignals(true);
	boxZRotation->setValue(g->zRotation());
	boxZRotation->blockSignals(false);

	boxResolution->setValue(g->resolution());
	boxLegend->setChecked(g->isLegendOn());
	boxOrthogonal->setChecked(g->isOrthogonal());

	labels = g->axesLabels();
	boxLabel->setText(labels[0]);

	QStringList tickLengths = g->axisTickLengths();
	boxMajorLength->setValue(tickLengths[0].toDouble());
	boxMinorLength->setValue(tickLengths[1].toDouble());

	xAxisFont = g->xAxisLabelFont();
	yAxisFont = g->yAxisLabelFont();
	zAxisFont = g->zAxisLabelFont();

	viewScaleLimits(0);
	viewAxisOptions(0);

	boxDistance->setValue(g->labelsDistance());
	numbersFont = g->numbersFont();

	if (g->coordStyle() == Qwt3D::NOCOORD){
		TicksGroupBox->setDisabled(true);
		generalDialog->setTabEnabled(axes,false);
		AxesColorGroupBox->setDisabled(true);
		boxDistance->setDisabled(true);
		btnNumbersFont->setDisabled(true);
	}

	Qwt3D::PLOTSTYLE style = g->plotStyle();
	Graph3D::PointStyle pt = g->pointType();

	if ( style == Qwt3D::USER ){
			switch (pt)
			{
				case Graph3D::None :
					break;

				case Graph3D::Dots :
					disableMeshOptions();
					initPointsOptionsStack();
					showPointsTab (g->pointsSize(), g->smoothPoints());
					break;

				case Graph3D::VerticalBars :
					showBarsTab();
					break;

				case Graph3D::HairCross :
					disableMeshOptions();
					initPointsOptionsStack();
					showCrossHairTab (g->crossHairRadius(), g->crossHairLinewidth(),
							g->smoothCrossHair(), g->boxedCrossHair());
					break;

				case Graph3D::Cones :
					disableMeshOptions();
					initPointsOptionsStack();
					showConesTab(g->coneRadius(), g->coneQuality());
					break;
			}
		}
		else if ( style == Qwt3D::FILLED )
			disableMeshOptions();
		else if (style == Qwt3D::HIDDENLINE || style == Qwt3D::WIREFRAME)
			boxLegend->setDisabled(true);

		if (g->grids() == 0)
			gridPage->setDisabled(true);

		if (g->userFunction() || g->parametricSurface())
			btnTable->hide();
		else if (g->table())
            btnTable->setText(tr("&Worksheet"));
		else if (g->matrix())
			btnTable->setText(tr("&Matrix"));

	boxPrintCropmarks->setChecked(g->printCropmarksEnabled());
	boxScaleOnPrint->setChecked(g->scaleOnPrint());

	initConnections();
}

void Plot3DDialog::initConnections()
{
	// scales page connections
	connect(boxFrom, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
    connect(boxTo, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect(boxMajors, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
	connect(boxMinors, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
	connect(boxPrecision, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
	connect(boxTickLabelsFormat, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(axesList, SIGNAL(currentRowChanged(int)), this, SLOT(viewScaleLimits(int)));

	// axes page connections
	connect(boxMajorLength, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect(boxMinorLength, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect(axesList2, SIGNAL(currentRowChanged(int)), this, SLOT(viewAxisOptions(int)));

	//grid page connections
	connect(boxMajorGrids, SIGNAL(toggled(bool)), this, SLOT(enableMajorGrids(bool)));
	connect(boxMajorGrids, SIGNAL(toggled(bool)), this, SLOT(updatePlot()));
	connect(boxMinorGrids, SIGNAL(toggled(bool)), this, SLOT(enableMinorGrids(bool)));
	connect(boxMinorGrids, SIGNAL(toggled(bool)), this, SLOT(updatePlot()));
	connect(boxMajorGridStyle, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(boxMinorGridStyle, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(boxMajorGridWidth, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect(boxMinorGridWidth, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
	connect(btnGrid, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(btnGridMinor, SIGNAL(colorChanged()), this, SLOT(updatePlot()));

	//color page connections
	connect(btnAxes, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(btnLabels, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(btnNumbers, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(btnMesh, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(btnBackground, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(d_color_map_editor, SIGNAL(scalingChanged()), this, SLOT(updatePlot()));

	connect(boxMeshLineWidth, SIGNAL(valueChanged(double)), d_plot, SLOT(setMeshLineWidth(double)));
	connect(boxOrthogonal, SIGNAL(toggled(bool)), d_plot, SLOT(setOrthogonal(bool)));
	connect(boxLegend, SIGNAL(toggled(bool)), d_plot, SLOT(showColorLegend(bool)));
    connect(boxResolution, SIGNAL(valueChanged(int)), d_plot, SLOT(setResolution(int)));
	connect(boxDistance, SIGNAL(valueChanged(int)), d_plot, SLOT(setLabelsDistance(int)));

	//title page connections
	connect(btnTitleColor, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
}

void Plot3DDialog::worksheet()
{
	d_plot->showWorksheet();
	close();
}

void Plot3DDialog::showBarsTab()
{
	bars = new QWidget( generalDialog );

	QGridLayout* hb = new QGridLayout(bars);
	hb->addWidget(new QLabel( tr( "Width" )), 0, 0);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	boxBarsRad = new DoubleSpinBox();
	boxBarsRad->setLocale(app->locale());
    boxBarsRad->setDecimals(app->d_decimal_digits);
	boxBarsRad->setValue(d_plot->barsRadius());
	boxBarsRad->setMinimum(0.0);
	hb->addWidget(boxBarsRad, 0, 1);

	boxBarLines = new QCheckBox(tr("Draw lines"));
	boxBarLines->setChecked(d_plot->barLines());
	hb->addWidget(boxBarLines, 1, 0);

	boxFilledBars = new QCheckBox(tr("Filled bars"));
	boxFilledBars->setChecked(d_plot->filledBars());
	hb->addWidget(boxFilledBars, 2, 0);
    hb->setRowStretch(3, 1);

	generalDialog->insertTab(bars, tr( "Bars" ), 4);
}

void Plot3DDialog::showPointsTab(double rad, bool smooth)
{
	boxPointStyle->setCurrentIndex(0);
	boxSize->setValue(rad);
	boxSmooth->setChecked(smooth);
	optionStack->setCurrentIndex(0);
}

void Plot3DDialog::showConesTab(double rad, int quality)
{
	boxPointStyle->setCurrentIndex(2);
	boxConesRad->setValue(rad);
	boxQuality->setValue(quality);
	optionStack->setCurrentIndex (2);
}

void Plot3DDialog::showCrossHairTab(double rad, double linewidth, bool smooth, bool boxed)
{
	boxPointStyle->setCurrentIndex(1);
	boxCrossRad->setValue(rad);
	boxCrossLinewidth->setValue(linewidth);
	boxCrossSmooth->setChecked(smooth);
	boxBoxed->setChecked(boxed);
	optionStack->setCurrentIndex(1);
}

void Plot3DDialog::disableMeshOptions()
{
	btnMesh->setDisabled(true);
	boxMeshLineWidth->setDisabled(true);
}

void Plot3DDialog::pickDataColorMap()
{
	ColorMapPreviewDialog *pd = new ColorMapPreviewDialog(this);
	pd->selectFile(d_color_map_file);
	pd->updatePreview(d_color_map_file);
	if (pd->exec() != QDialog::Accepted)
		return;

	QString fn = pd->selectedFiles()[0];
	if (!fn.isEmpty()){
   		d_plot->setDataColorMap(fn);
		d_color_map_file = fn;
		setColorMapPreview(d_color_map_file);
	}
}

void Plot3DDialog::setColorMapPreview(const QString& fileName)
{
	if (fileName.isEmpty()){
		colorMapPreviewLabel->setText(tr("None"));
   		return;
	}

	ColorVector cv;
	if (!Graph3D::openColorMapFile(cv, fileName)){
		colorMapPreviewLabel->setText(tr("None"));
   		return;
	}

	int height = 20;
	QPixmap pix;
	pix.resize(cv.size(), height);
	QPainter p(&pix);
	for (unsigned i = 0; i != cv.size(); ++i){
		RGBA rgb = cv[i];
		p.setPen(GL2Qt(rgb.r, rgb.g, rgb.b));
		p.drawLine(QPoint(0, 0), QPoint(0, height));
   		p.translate(1, 0);
	}
  	p.end();
	colorMapPreviewLabel->setPixmap(pix);
}

void Plot3DDialog::pickTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,titleFont,this);
	if (ok){
		titleFont = font;
		updatePlot();
	} else
		return;
}

void Plot3DDialog::pickNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,numbersFont,this);
	if ( ok ) {
		numbersFont = font;
		updatePlot();
	} else
		return;
}

void Plot3DDialog::viewAxisOptions(int axis)
{
	boxLabel->setText(labels[axis]);

	QStringList tickLengths = d_plot->axisTickLengths();

	boxMajorLength->blockSignals(true);
	boxMajorLength->setValue(tickLengths[2*axis+0].toDouble());
	boxMajorLength->blockSignals(false);

	boxMinorLength->blockSignals(true);
	boxMinorLength->setValue(tickLengths[2*axis+1].toDouble());
	boxMinorLength->blockSignals(false);

	axesList->setCurrentRow(axis);

	boxTickLabelsFormat->blockSignals(true);
	boxTickLabelsFormat->setCurrentIndex(d_plot->axisNumericFormat(axis));
	boxTickLabelsFormat->blockSignals(false);
	boxPrecision->blockSignals(true);
	boxPrecision->setValue(d_plot->axisNumericPrecision(axis));
	boxPrecision->blockSignals(false);
}

void Plot3DDialog::viewScaleLimits(int axis)
{
	Qwt3D::Axis a = d_plot->surface()->coordinates()->axes[axis];

	double start, end;
	a.limits(start, end);

	boxFrom->blockSignals(true);
	boxFrom->setValue(start);
	boxFrom->blockSignals(false);

	boxTo->blockSignals(true);
	boxTo->setValue(end);
	boxTo->blockSignals(false);

	boxMajors->blockSignals(true);
	boxMajors->setValue(a.majors());
	boxMajors->blockSignals(false);

	boxMinors->blockSignals(true);
	boxMinors->setValue(a.minors());
	boxMinors->blockSignals(false);

	boxType->setCurrentIndex(d_plot->axisType(axis));

	axesList2->setCurrentRow(axis);
}

void Plot3DDialog::accept()
{
	if (updatePlot())
		close();
}

void Plot3DDialog::changeZoom(int)
{
	if (generalDialog->currentPage() != (QWidget*)general)
		return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	d_plot->setZoom(zoom*boxZoom->value()*0.01);
    d_plot->setScale(xScale*boxXScale->value()*0.01, yScale*boxYScale->value()*0.01, zScale*boxZScale->value()*0.01);
    d_plot->update();
    QApplication::restoreOverrideCursor();
}

void Plot3DDialog::changeTransparency(int val)
{
	if (generalDialog->currentPage() != (QWidget*)colors)
		return;

	d_plot->changeTransparency(val*0.01);
	d_plot->update();
}

bool Plot3DDialog::updatePlot()
{
	if (!d_plot)
		return false;

    ApplicationWindow *app = (ApplicationWindow *)this->parent();
    if (!app)
        return false;

	if (generalDialog->currentPage() == bars){
		d_plot->setBarRadius(boxBarsRad->text().toDouble());
		d_plot->setBarLines(boxBarLines->isChecked());
		d_plot->setFilledBars(boxFilledBars->isChecked());
		d_plot->setBarStyle();
	} else if (generalDialog->currentPage() == points){
		if (boxPointStyle->currentIndex() == 0) {
			d_plot->setDotOptions(boxSize->text().toDouble(), boxSmooth->isChecked());
			d_plot->setDotStyle();
		} else if (boxPointStyle->currentIndex() == 1){
			d_plot->setCrossOptions(boxCrossRad->text().toDouble(), boxCrossLinewidth->text().toDouble(),
					boxCrossSmooth->isChecked(), boxBoxed->isChecked());
            d_plot->setCrossStyle();
        } else if (boxPointStyle->currentIndex() == 2) {
			d_plot->setConeOptions(boxConesRad->text().toDouble(), boxQuality->value());
			d_plot->setConeStyle();
        }
        app->custom3DActions(d_plot);
	} else if (generalDialog->currentPage() == title){
		d_plot->setTitle(boxTitle->text().remove("\n"), btnTitleColor->color(), titleFont);
	} else if (generalDialog->currentPage() == colors){
		d_plot->changeTransparency(boxTransparency->value()*0.01);
		if (linearColorMapGroupBox->isChecked())
			d_plot->setDataColorMap(d_color_map_editor->colorMap());
		else if (colorMapFileGroupBox->isChecked() && !d_color_map_file.isEmpty()){
			d_plot->setDataColorMap(d_color_map_file);
			setColorMapPreview(d_color_map_file);
		}

		d_plot->setMeshColor(btnMesh->color());
		d_plot->setAxesColor(btnAxes->color());
		d_plot->setNumbersColor(btnNumbers->color());
		d_plot->setLabelsColor(btnLabels->color());
		d_plot->setBackgroundColor(btnBackground->color());
	} else if (generalDialog->currentPage() == gridPage){
		Qwt3D::GridLine majorGrid(boxMajorGrids->isChecked(), Qt2GL(btnGrid->color()), (Qwt3D::LINESTYLE)boxMajorGridStyle->currentIndex(), boxMajorGridWidth->value());
		Qwt3D::GridLine minorGrid(boxMinorGrids->isChecked(), Qt2GL(btnGridMinor->color()), (Qwt3D::LINESTYLE)boxMinorGridStyle->currentIndex(), boxMinorGridWidth->value());
		for (int i = 0; i < 12; i++){
			d_plot->coordinateSystem()->setMajorGridLines((Qwt3D::AXIS)i, majorGrid);
			d_plot->coordinateSystem()->setMinorGridLines((Qwt3D::AXIS)i, minorGrid);
		}
	} else if (generalDialog->currentPage() == general){
		d_plot->showColorLegend(boxLegend->isChecked());
		d_plot->setResolution(boxResolution->value());
		d_plot->setMeshLineWidth(boxMeshLineWidth->value());
		d_plot->setLabelsDistance(boxDistance->value());
		d_plot->setNumbersFont(numbersFont);
		d_plot->setZoom(zoom*boxZoom->value()*0.01);
		d_plot->setScale(xScale*boxXScale->value()*0.01, yScale*boxYScale->value()*0.01, zScale*boxZScale->value()*0.01);
		d_plot->setRotation(boxXRotation->value(), boxYRotation->value(), boxZRotation->value());
	} else if (generalDialog->currentPage() == (QWidget*)scale){
		double start = qMin(boxFrom->value(), boxTo->value());
		double end = qMax(boxFrom->value(), boxTo->value());
		d_plot->setScale(axesList->currentRow(), start, end, boxMajors->value(), boxMinors->value(), (Qwt3D::SCALETYPE)boxType->currentIndex());
		d_plot->setAxisNumericFormat(axesList->currentRow(), boxTickLabelsFormat->currentIndex(), boxPrecision->value());
		viewScaleLimits(axesList->currentRow());
	} else if (generalDialog->currentPage() == axes){
		int axis = axesList2->currentRow();
		labels[axis] = boxLabel->text();

		switch(axis)
		{
		case 0:
			d_plot->setXAxisLabel(boxLabel->text().remove("\n"));
			d_plot->setXAxisLabelFont(axisFont(axis));
			d_plot->setXAxisTickLength(boxMajorLength->text().toDouble(), boxMinorLength->text().toDouble());
		break;
		case 1:
			d_plot->setYAxisLabel(boxLabel->text().remove("\n"));
			d_plot->setYAxisLabelFont(axisFont(axis));
			d_plot->setYAxisTickLength(boxMajorLength->text().toDouble(), boxMinorLength->text().toDouble());
		break;
		case 2:
			d_plot->setZAxisLabel(boxLabel->text().remove("\n"));
			d_plot->setZAxisLabelFont(axisFont(axis));
			d_plot->setZAxisTickLength(boxMajorLength->text().toDouble(), boxMinorLength->text().toDouble());
		break;
		}
	} else if (generalDialog->currentPage() == printPage){
		d_plot->printCropmarks(boxPrintCropmarks->isChecked());
		d_plot->setScaleOnPrint(boxScaleOnPrint->isChecked());
	}

    d_plot->update();
    app->modifiedProject(d_plot);
	return true;
}

void Plot3DDialog::pickAxisLabelFont()
{
	bool ok;
	QFont font;
	switch(axesList2->currentRow())
	{
		case 0:
			font = QFontDialog::getFont(&ok, xAxisFont, this);
			if ( ok ){
				xAxisFont=font;
				updatePlot();
			} else
				return;
			break;

		case 1:
			font = QFontDialog::getFont(&ok, yAxisFont, this);
			if ( ok ){
				yAxisFont = font;
				updatePlot();
			} else
				return;
			break;

		case 2:
			font = QFontDialog::getFont(&ok, zAxisFont, this);
			if ( ok ){
				zAxisFont = font;
				updatePlot();
			} else
				return;
			break;
	}
}

QFont Plot3DDialog::axisFont(int axis)
{
	QFont f;
	switch(axis)
	{
		case 0:
			f=xAxisFont;
			break;

		case 1:
			f=yAxisFont;
			break;

		case 2:
			f=zAxisFont;
			break;
	}
	return f;
}

void Plot3DDialog::showGeneralTab()
{
	generalDialog->setCurrentWidget (general);
}

void Plot3DDialog::showTitleTab()
{
	generalDialog->setCurrentWidget (title);
}

void Plot3DDialog::showAxisTab()
{
	generalDialog->setCurrentWidget (axes);
}

void Plot3DDialog::updateColorMapFileGroupBox(bool checked)
{
	colorMapFileGroupBox->setChecked(!checked);
}

void Plot3DDialog::updateLinearColorMapGroupBox(bool checked)
{
	linearColorMapGroupBox->setChecked(!checked);
}
