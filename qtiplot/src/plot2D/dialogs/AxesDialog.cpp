/***************************************************************************
    File                 : AxesDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004 - 2010 by Ion Vasilief,
						   (C) 2006 - June 2007 Tilman Hoener zu Siederdissen
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Axes preferences dialog

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
#include "AxesDialog.h"
#include <ApplicationWindow.h>
#include <MyParser.h>
#include <TextFormatButtons.h>
#include <Table.h>
#include <Graph.h>
#include <MultiLayer.h>
#include <Grid.h>
#include <ScaleDraw.h>
#include <ScaleEngine.h>
#include <FunctionCurve.h>
#include <DoubleSpinBox.h>
#include <ColorButton.h>
#include <PenStyleBox.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QLayout>
#include <QMessageBox>
#include <QFontDialog>
#include <QDate>
#include <QList>
#include <QListWidget>
#include <QVector>
#include <QGroupBox>
#include <QDateTimeEdit>
#include <QTimeEdit>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>

#ifndef M_PI
#define M_PI	3.141592653589793238462643
#endif

AxesDialog::AxesDialog( QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
    setWindowTitle( tr( "QtiPlot - General Plot Options" ) );

    generalDialog = new QTabWidget();

    initScalesPage();
    initGridPage();
    initAxesPage();
    initFramePage();

    QHBoxLayout * bottomButtons = new QHBoxLayout();
    bottomButtons->addStretch();

    buttonApply = new QPushButton();
    buttonApply->setText( tr( "&Apply" ) );
    bottomButtons->addWidget( buttonApply );

    buttonOk = new QPushButton();
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setDefault( true );
    bottomButtons->addWidget( buttonOk );

    buttonCancel = new QPushButton();
    buttonCancel->setText( tr( "&Cancel" ) );
    bottomButtons->addWidget( buttonCancel );

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(generalDialog);
    mainLayout->addLayout(bottomButtons);

    lastPage = scalesPage;

    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonApply, SIGNAL( clicked() ), this, SLOT(updatePlot() ) );
	connect( generalDialog, SIGNAL( currentChanged ( QWidget * ) ), this, SLOT(pageChanged( QWidget *)));
}

void AxesDialog::initScalesPage()
{
    ApplicationWindow *app = (ApplicationWindow *)parent();
	scalesPage = new QWidget();

	QGroupBox * middleBox = new QGroupBox(QString());
	QGridLayout * middleLayout = new QGridLayout(middleBox);

	middleLayout->addWidget(new QLabel(tr( "From" )), 0, 0);
	boxStart = new DoubleSpinBox();
	boxStart->setLocale(app->locale());
    boxStart->setDecimals(app->d_decimal_digits);
	middleLayout->addWidget( boxStart, 0, 1 );

	boxStartDateTime = new QDateTimeEdit();
	boxStartDateTime->setCalendarPopup(true);
	boxStartDateTime->setWrapping(true);
	middleLayout->addWidget( boxStartDateTime, 0, 1 );
	boxStartDateTime->hide();

	middleLayout->addWidget(new QLabel(tr( "To" )), 1, 0);
	boxEnd = new DoubleSpinBox();
	boxEnd->setLocale(app->locale());
	boxEnd->setDecimals(app->d_decimal_digits);
	middleLayout->addWidget( boxEnd, 1, 1);

	boxEndDateTime = new QDateTimeEdit();
	boxEndDateTime->setCalendarPopup(true);
	boxEndDateTime->setWrapping(true);
	middleLayout->addWidget(boxEndDateTime, 1, 1);
	boxEndDateTime->hide();

	boxScaleTypeLabel = new QLabel(tr( "Type" ));
	boxScaleType = new QComboBox();
	boxScaleType->setIconSize(QSize(27, 16));
	boxScaleType->addItem(QIcon(":/liniar_scale.png"), tr( "Linear" ));
	boxScaleType->addItem(QIcon(":/log10.png"), tr( "Log10" ));
	boxScaleType->addItem(QIcon(":/ln_scale.png"), tr( "ln" ));
	boxScaleType->addItem(QIcon(":/log2_scale.png"), tr( "Log2" ));
	boxScaleType->addItem(QIcon(":/reciprocal_scale.png"), tr( "Reciprocal" ));
	boxScaleType->addItem(QIcon(":/probability_scale.png"), tr( "Probability" ));
	boxScaleType->addItem(QIcon(":/logit_scale.png"), tr( "Logit" ));
	middleLayout->addWidget( boxScaleTypeLabel, 2, 0);
	middleLayout->addWidget( boxScaleType, 2, 1);

	btnInvert = new QCheckBox();
	btnInvert->setText( tr( "Inverted" ) );
	btnInvert->setChecked(false);
	middleLayout->addWidget( btnInvert, 3, 1 );
	middleLayout->setRowStretch(4, 1);

	boxAxesBreaks = new QGroupBox(tr("Show Axis &Break"));
	boxAxesBreaks->setCheckable(true);
	boxAxesBreaks->setChecked(false);

	QGridLayout * breaksLayout = new QGridLayout(boxAxesBreaks);
	boxBreakDecoration = new QCheckBox(tr("Draw Break &Decoration"));
	breaksLayout->addWidget(boxBreakDecoration, 0, 1);

	breaksLayout->addWidget(new QLabel(tr("From")), 1, 0);
	boxBreakStart = new DoubleSpinBox();
	boxBreakStart->setLocale(app->locale());
    boxBreakStart->setDecimals(app->d_decimal_digits);
	breaksLayout->addWidget(boxBreakStart, 1, 1);

	breaksLayout->addWidget(new QLabel(tr("To")), 2, 0);
	boxBreakEnd = new DoubleSpinBox();
	boxBreakEnd->setLocale(app->locale());
    boxBreakEnd->setDecimals(app->d_decimal_digits);
	breaksLayout->addWidget(boxBreakEnd, 2, 1);

    breaksLayout->addWidget(new QLabel(tr("Position")), 3, 0);
    boxBreakPosition = new QSpinBox();
	boxBreakPosition->setSuffix(" (" + tr("% of Axis Length") + ")");
    breaksLayout->addWidget(boxBreakPosition, 3, 1);

	breaksLayout->addWidget(new QLabel(tr("Width")), 4, 0);
	boxBreakWidth = new QSpinBox();
	boxBreakWidth->setSuffix(" (" + tr("pixels") + ")");
	breaksLayout->addWidget(boxBreakWidth, 4, 1);

    boxLog10AfterBreak = new QCheckBox(tr("&Log10 Scale After Break"));
	breaksLayout->addWidget(boxLog10AfterBreak, 0, 2);

    breaksLayout->addWidget(new QLabel(tr("Step Before Break")), 1, 2);
    boxStepBeforeBreak = new DoubleSpinBox();
    boxStepBeforeBreak->setMinimum(0.0);
    boxStepBeforeBreak->setSpecialValueText(tr("Guess"));
	boxStepBeforeBreak->setLocale(app->locale());
    boxStepBeforeBreak->setDecimals(app->d_decimal_digits);
	breaksLayout->addWidget(boxStepBeforeBreak, 1, 3);

    breaksLayout->addWidget(new QLabel(tr("Step After Break")), 2, 2);
    boxStepAfterBreak = new DoubleSpinBox();
    boxStepAfterBreak->setMinimum(0.0);
    boxStepAfterBreak->setSpecialValueText(tr("Guess"));
	boxStepAfterBreak->setLocale(app->locale());
    boxStepAfterBreak->setDecimals(app->d_decimal_digits);
	breaksLayout->addWidget(boxStepAfterBreak, 2, 3);

    breaksLayout->addWidget(new QLabel(tr("Minor Ticks Before")), 3, 2);
    boxMinorTicksBeforeBreak = new QComboBox();
	boxMinorTicksBeforeBreak->setEditable(true);
	boxMinorTicksBeforeBreak->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");
    breaksLayout->addWidget(boxMinorTicksBeforeBreak, 3, 3);

    breaksLayout->addWidget(new QLabel(tr("Minor Ticks After")), 4, 2);
    boxMinorTicksAfterBreak  = new QComboBox();
	boxMinorTicksAfterBreak->setEditable(true);
	boxMinorTicksAfterBreak->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");
    breaksLayout->addWidget(boxMinorTicksAfterBreak, 4, 3);

	QGroupBox *rightBox = new QGroupBox(QString());
	QGridLayout *rightLayout = new QGridLayout(rightBox);

	QWidget * stepWidget = new QWidget();
	QHBoxLayout * stepWidgetLayout = new QHBoxLayout( stepWidget );
	stepWidgetLayout->setMargin(0);

	btnStep = new QRadioButton(tr("Step"));
	btnStep->setChecked(true);
	rightLayout->addWidget( btnStep, 0, 0 );

	boxStep = new DoubleSpinBox();
	boxStep->setMinimum(0.0);
	boxStep->setLocale(app->locale());
    boxStep->setDecimals(app->d_decimal_digits);
	stepWidgetLayout->addWidget(boxStep);

	boxUnit = new QComboBox();
	boxUnit->hide();
	stepWidgetLayout->addWidget( boxUnit );

	rightLayout->addWidget( stepWidget, 0, 1 );

	btnMajor = new QRadioButton();
	btnMajor->setText( tr( "Major Ticks" ) );
	rightLayout->addWidget( btnMajor, 1, 0);

	boxMajorValue = new QSpinBox();
	boxMajorValue->setDisabled(true);
	rightLayout->addWidget( boxMajorValue, 1, 1);

	minorBoxLabel = new QLabel( tr( "Minor Ticks" ));
	rightLayout->addWidget( minorBoxLabel, 2, 0);

	boxMinorValue = new QComboBox();
	boxMinorValue->setEditable(true);
	boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");
	rightLayout->addWidget( boxMinorValue, 2, 1);

	rightLayout->setRowStretch( 3, 1 );

    QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(middleBox);
	hl->addWidget(rightBox);

    QVBoxLayout* vl = new QVBoxLayout();
	vl->addLayout(hl);
	vl->addWidget(boxAxesBreaks);

	QPixmap image0(":/bottom_scl.png");
	QPixmap image1(":/left_scl.png");
	QPixmap image2(":/top_scl.png");
	QPixmap image3(":/right_scl.png");

	axesList = new QListWidget();
	axesList->addItem( new QListWidgetItem(image0, tr( "Bottom" )));
	axesList->addItem( new QListWidgetItem(image1, tr( "Left" )));
	axesList->addItem( new QListWidgetItem(image2, tr( "Top" )));
	axesList->addItem( new QListWidgetItem(image3,  tr( "Right" )));
    axesList->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	axesList->setIconSize(image0.size());
	axesList->setCurrentRow(-1);

	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(axesList->font());
	int width = 32;
	for(int i=0 ; i<axesList->count() ; i++)
		if( fm.width(axesList->item(i)->text()) > width)
			width = fm.width(axesList->item(i)->text());

	axesList->setMaximumWidth( axesList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	axesList->resize(axesList->maximumWidth(),axesList->height());

	QHBoxLayout* mainLayout = new QHBoxLayout(scalesPage);
	mainLayout->addWidget(axesList);
	mainLayout->addLayout(vl);

	generalDialog->addTab(scalesPage, tr( "Scale" ));

	connect(btnInvert,SIGNAL(clicked()), this, SLOT(updatePlot()));
	connect(axesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateScale()));
	connect(boxScaleType,SIGNAL(activated(int)), this, SLOT(updateMinorTicksList(int)));
	connect(btnStep,SIGNAL(clicked()), this, SLOT(stepEnabled()));
	connect(btnMajor,SIGNAL(clicked()), this, SLOT(stepDisabled()));
}

void AxesDialog::initGridPage()
{
	gridPage = new QWidget();

    QGroupBox * rightBox = new QGroupBox(QString());
    QGridLayout * rightLayout = new QGridLayout(rightBox);

    boxMajorGrid = new QCheckBox();
    boxMajorGrid->setText( tr( "Major Grids" ) );
    boxMajorGrid->setChecked(true);
    rightLayout->addWidget( boxMajorGrid, 0, 1);

    boxMinorGrid = new QCheckBox();
    boxMinorGrid->setText( tr( "Minor Grids" ) );
    boxMinorGrid->setChecked(false);
    rightLayout->addWidget( boxMinorGrid, 0, 2);

    rightLayout->addWidget( new QLabel(tr( "Line Color" )), 1, 0 );

	boxColorMajor = new ColorButton();
    rightLayout->addWidget( boxColorMajor, 1, 1);

	boxColorMinor = new ColorButton();
    boxColorMinor->setDisabled(true);
    rightLayout->addWidget( boxColorMinor, 1, 2);

    rightLayout->addWidget( new QLabel(tr( "Line Type" )), 2, 0 );

    boxTypeMajor = new PenStyleBox();
    rightLayout->addWidget( boxTypeMajor, 2, 1);

    boxTypeMinor = new PenStyleBox();
    rightLayout->addWidget( boxTypeMinor, 2, 2);

    rightLayout->addWidget( new QLabel(tr( "Thickness" )), 3, 0 );

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

    rightLayout->addWidget( new QLabel(tr( "Axes" )), 4, 0 );

    boxGridXAxis = new QComboBox();
    boxGridXAxis->insertItem(tr("Bottom"));
    boxGridXAxis->insertItem(tr("Top"));
    rightLayout->addWidget( boxGridXAxis, 4, 1);

    boxGridYAxis = new QComboBox();
    boxGridYAxis->insertItem(tr("Left"));
    boxGridYAxis->insertItem(tr("Right"));
    rightLayout->addWidget(boxGridYAxis, 4, 2);

    rightLayout->addWidget( new QLabel(tr( "Additional lines" )), 5, 0);

    boxXLine = new QCheckBox(tr( "X=0" ));
    boxXLine->setDisabled(true);
    rightLayout->addWidget( boxXLine, 5, 1);

    boxYLine = new QCheckBox(tr( "Y=0" ));
    rightLayout->addWidget(boxYLine, 5, 2);

    rightLayout->addWidget( new QLabel(tr( "Apply To" )), 6, 0);
    boxApplyGridFormat = new QComboBox();
    boxApplyGridFormat->insertItem(tr("This Layer"));
    boxApplyGridFormat->insertItem(tr("This Window"));
    boxApplyGridFormat->insertItem(tr("All Windows"));
    rightLayout->addWidget(boxApplyGridFormat, 6, 1);

    boxAntialiseGrid = new QCheckBox(tr("An&tialised"));
    rightLayout->addWidget(boxAntialiseGrid, 6, 2);

    rightLayout->setRowStretch(7, 1);
    rightLayout->setColumnStretch(4, 1);

	QPixmap image2(":/vertical_grid.png");
	QPixmap image3(":/horizontal_grid.png");

    axesGridList = new QListWidget();
    axesGridList->addItem( new QListWidgetItem(image3, tr( "Horizontal" )) );
    axesGridList->addItem( new QListWidgetItem(image2, tr( "Vertical" )) );
    axesGridList->setIconSize(image3.size());
    axesGridList->setCurrentRow(0);

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

    QHBoxLayout* mainLayout2 = new QHBoxLayout(gridPage);
    mainLayout2->addWidget(axesGridList);
    mainLayout2->addWidget(rightBox);

    generalDialog->addTab( gridPage, tr( "Grid" ) );

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
	connect(boxXLine,SIGNAL(clicked()),this, SLOT(updatePlot()));
	connect(boxYLine,SIGNAL(clicked()),this, SLOT(updatePlot()));
}

void AxesDialog::initAxesPage()
{
	//axes page
	QPixmap bottom_axis_pic(":/bottom_axis.png");
	QPixmap left_axis_pic(":/left_axis.png");
	QPixmap top_axis_pic(":/top_axis.png");
	QPixmap right_axis_pic(":/right_axis.png");

	axesPage = new QWidget();

	axesTitlesList = new QListWidget();
	axesTitlesList->addItem( new QListWidgetItem(bottom_axis_pic, tr("Bottom")));
	axesTitlesList->addItem( new QListWidgetItem(left_axis_pic, tr("Left")));
	axesTitlesList->addItem( new QListWidgetItem(top_axis_pic, tr("Top")));
	axesTitlesList->addItem( new QListWidgetItem(right_axis_pic, tr("Right")));
	axesTitlesList->setIconSize(bottom_axis_pic.size());
	axesTitlesList->setMaximumWidth((int)(bottom_axis_pic.width()*1.5));
	axesTitlesList->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	axesTitlesList->setCurrentRow(-1);

	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(axesTitlesList->font());
	int width = 32;
	for(int i=0 ; i<axesTitlesList->count() ; i++)
		if( fm.width(axesTitlesList->item(i)->text()) > width)
			width = fm.width(axesTitlesList->item(i)->text());
	axesTitlesList->setMaximumWidth( axesTitlesList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	axesTitlesList->resize(axesTitlesList->maximumWidth(),axesTitlesList->height());

	QHBoxLayout * topLayout = new QHBoxLayout();

	boxShowAxis = new QCheckBox(tr("Show"));
	boxShowAxis->setChecked(true);
	topLayout->addWidget( boxShowAxis );

	labelBox = new QGroupBox(tr( "Title"));
	topLayout->addWidget( labelBox );

	QVBoxLayout *labelBoxLayout = new QVBoxLayout( labelBox );

	boxTitle = new QTextEdit();
	boxTitle->setTextFormat(Qt::PlainText);
	QFontMetrics metrics(this->font());
	boxTitle->setMaximumHeight(3*metrics.height());
    labelBoxLayout->addWidget(boxTitle);

	formatButtons = new TextFormatButtons(boxTitle, TextFormatButtons::AxisLabel);
	labelBoxLayout->addWidget(formatButtons);

	QHBoxLayout *hl = new QHBoxLayout();
	hl->addWidget(new QLabel(tr("Color")));
	axisLabelColorButton = new ColorButton();
	connect(axisLabelColorButton, SIGNAL(colorChanged()), this, SLOT(pickAxisLabelColor()));
	hl->addWidget(axisLabelColorButton);

	buttonLabelFont = new QPushButton(tr("&Font"));
	buttonLabelFont->setIcon(QIcon(":/font.png"));
	connect(buttonLabelFont, SIGNAL(clicked()), this, SLOT(customAxisLabelFont()));
	hl->addWidget(buttonLabelFont);

	hl->addWidget(new QLabel(tr("Distance to axis")));
	boxLabelsDistance = new QSpinBox();
	boxLabelsDistance->setRange(0, 1000);
	boxLabelsDistance->setSuffix(" " + tr("pixels"));
	connect(boxLabelsDistance, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
	hl->addWidget(boxLabelsDistance);
	hl->addStretch();

	invertTitleBox = new QCheckBox(tr("&Inverted"));
	invertTitleBox->hide();
	connect(invertTitleBox, SIGNAL(toggled(bool)), this, SLOT(updatePlot()));
	hl->addWidget(invertTitleBox);

	hl->addStretch();
	labelBoxLayout->addLayout(hl);

	QHBoxLayout * bottomLayout = new QHBoxLayout();

	axisFormatBox = new QGroupBox(tr("Format"));
	bottomLayout->addWidget( axisFormatBox );
	QGridLayout *leftBoxLayout = new QGridLayout(axisFormatBox);

	leftBoxLayout->addWidget( new QLabel(tr( "Type" )), 0, 0 );

	boxAxisType = new QComboBox();
	boxAxisType->addItem(tr("Numeric"));
	boxAxisType->addItem(tr("Text from table"));
	boxAxisType->addItem(tr("Day of the week"));
	boxAxisType->addItem(tr("Month"));
	boxAxisType->addItem(tr("Time"));
	boxAxisType->addItem(tr("Date") + "/" + tr("Time"));
	boxAxisType->addItem(tr("Column Headings"));
	leftBoxLayout->addWidget( boxAxisType, 0, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Font" )), 1, 0 );

	btnAxesFont = new QPushButton();
	btnAxesFont->setText( tr( "Axis &Font" ) );
	btnAxesFont->setIcon(QIcon(":/font.png"));
	leftBoxLayout->addWidget( btnAxesFont, 1, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Color" )), 2, 0 );
	boxAxisColor = new ColorButton();
	leftBoxLayout->addWidget( boxAxisColor, 2, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Major Ticks" )), 3, 0 );

	boxMajorTicksType= new QComboBox();
	boxMajorTicksType->addItem(tr( "None" ) );
	boxMajorTicksType->addItem(tr( "Out" ) );
	boxMajorTicksType->addItem(tr( "In & Out" ) );
	boxMajorTicksType->addItem(tr( "In" ) );
	leftBoxLayout->addWidget( boxMajorTicksType, 3, 1 );

	leftBoxLayout->addWidget( new QLabel(tr( "Minor Ticks" )), 4, 0 );

	boxMinorTicksType = new QComboBox();
	boxMinorTicksType->addItem(tr( "None" ) );
	boxMinorTicksType->addItem(tr( "Out" ) );
	boxMinorTicksType->addItem(tr( "In & Out" ) );
	boxMinorTicksType->addItem(tr( "In" ) );
	leftBoxLayout->addWidget( boxMinorTicksType, 4, 1);

	leftBoxLayout->addWidget( new QLabel(tr( "Hide Ticks" )), 5, 0 );
	showTicksPolicyBox = new QComboBox();
	showTicksPolicyBox->addItem(tr("None"));
	showTicksPolicyBox->addItem(tr("At Axis Begin"));
	showTicksPolicyBox->addItem(tr("At Axis End"));
	showTicksPolicyBox->addItem(tr("At Axis Begin & End"));
	leftBoxLayout->addWidget(showTicksPolicyBox, 5, 1);

	leftBoxLayout->addWidget( new QLabel(tr("Labels space")), 6, 0);
	boxTickLabelDistance = new QSpinBox();
	boxTickLabelDistance->setRange( 0, 10000);
	boxTickLabelDistance->setSuffix(" " + tr("pixels"));
	leftBoxLayout->addWidget(boxTickLabelDistance, 6, 1);

	leftBoxLayout->addWidget( new QLabel(tr("Stand-off")), 7, 0);
	boxBaseline = new QSpinBox();
	boxBaseline->setRange( 0, 1000 );
	boxBaseline->setSuffix(" " + tr("pixels"));
	leftBoxLayout->addWidget(boxBaseline, 7, 1);

	QLabel *l = new QLabel(tr("Apply &to"));
	leftBoxLayout->addWidget(l, 8, 0);

	axisFormatApplyToBox = new QComboBox();
	axisFormatApplyToBox->insertItem(tr("Axis"));
	axisFormatApplyToBox->insertItem(tr("Layer"));
    axisFormatApplyToBox->insertItem(tr("Window"));
    axisFormatApplyToBox->insertItem(tr("All Windows"));
	leftBoxLayout->addWidget(axisFormatApplyToBox, 8, 1);
	l->setBuddy(axisFormatApplyToBox);

	boxShowLabels = new QGroupBox(tr("Show Labels"));
	boxShowLabels->setCheckable(true);
	boxShowLabels->setChecked(true);

	boxAxisBackbone = new QCheckBox(tr("Show &backbone"));

	QVBoxLayout *vb = new QVBoxLayout;
	vb->addWidget( boxShowLabels );
	vb->addWidget( boxAxisBackbone );

	bottomLayout->addLayout( vb );

	QGridLayout *rightBoxLayout = new QGridLayout( boxShowLabels );

	label1 = new QLabel(tr("Column"));
	rightBoxLayout->addWidget( label1, 0, 0 );

	boxColName = new QComboBox();
	rightBoxLayout->addWidget( boxColName, 0, 1 );

	labelTable = new QLabel(tr("Table"));
	rightBoxLayout->addWidget( labelTable, 1, 0 );

	boxTableName = new QComboBox();
	rightBoxLayout->addWidget( boxTableName, 1, 1 );

	label2 = new QLabel(tr( "Format" ));
	rightBoxLayout->addWidget( label2, 2, 0 );

	boxFormat = new QComboBox();
	boxFormat->setDuplicatesEnabled(false);
	rightBoxLayout->addWidget( boxFormat, 2, 1 );

	label3 = new QLabel(tr( "Precision" ));
	rightBoxLayout->addWidget( label3, 3, 0 );
	boxPrecision = new QSpinBox();
	boxPrecision->setRange( 0, 10 );
	rightBoxLayout->addWidget( boxPrecision, 3, 1 );

	rightBoxLayout->addWidget( new QLabel(tr( "Angle" )), 4, 0 );

	boxAngle = new QSpinBox();
	boxAngle->setRange( -90, 90 );
	boxAngle->setSingleStep(5);
	rightBoxLayout->addWidget( boxAngle, 4, 1 );

    rightBoxLayout->addWidget(new QLabel(tr( "Color" )), 5, 0);
	boxAxisNumColor = new ColorButton();
	rightBoxLayout->addWidget( boxAxisNumColor, 5, 1 );

	rightBoxLayout->addWidget(new QLabel(tr( "Prefix" )), 6, 0);
	boxPrefix = new QLineEdit();
	rightBoxLayout->addWidget( boxPrefix, 6, 1 );
	rightBoxLayout->addWidget(new QLabel(tr( "Suffix" )), 7, 0);
	boxSuffix = new QLineEdit();
	rightBoxLayout->addWidget( boxSuffix, 7, 1 );

	boxShowFormula = new QCheckBox(tr( "For&mula" ));
	rightBoxLayout->addWidget( boxShowFormula, 8, 0 );

	boxFormula = new QTextEdit();
	boxFormula->setTextFormat(Qt::PlainText);
	boxFormula->setMaximumHeight(3*metrics.height());
	boxFormula->hide();
	rightBoxLayout->addWidget( boxFormula, 8, 1 );
	rightBoxLayout->setRowStretch(9, 1);

	QVBoxLayout * rightLayout = new QVBoxLayout();
	rightLayout->addLayout( topLayout );
	rightLayout->addLayout( bottomLayout );
	rightLayout->addStretch(1);

	QHBoxLayout * mainLayout3 = new QHBoxLayout( axesPage );
	mainLayout3->addWidget( axesTitlesList );
	mainLayout3->addLayout( rightLayout );

	generalDialog->addTab( axesPage, tr( "Axis" ) );

	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateShowBox(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateAxisColor(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateTitleBox(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setTicksType(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setAxisType(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(setBaselineDist(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateLabelsFormat(int)));
	connect(axesTitlesList,SIGNAL(currentRowChanged(int)), this, SLOT(updateCurrentAxis()));

	connect(boxShowLabels,SIGNAL(clicked(bool)), this, SLOT(updateTickLabelsList(bool)));

	connect(boxAxisColor, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
    connect(boxAxisNumColor, SIGNAL(colorChanged()), this, SLOT(updatePlot()));
	connect(boxMajorTicksType, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(boxMinorTicksType, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(showTicksPolicyBox, SIGNAL(activated(int)), this, SLOT(updatePlot()));
	connect(boxBaseline, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
	connect(boxAxisBackbone, SIGNAL(clicked(bool)), this, SLOT(updatePlot()));
	connect(boxTickLabelDistance, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));

	connect(boxShowFormula, SIGNAL(clicked()), this, SLOT(showFormulaBox()));
	connect(boxShowAxis, SIGNAL(clicked()), this, SLOT(showAxis()));
	connect(boxFormat, SIGNAL(activated(int)), this, SLOT(setLabelsNumericFormat(int)));
	connect(btnAxesFont, SIGNAL(clicked()), this, SLOT(customAxisFont()));
	connect(boxAxisType, SIGNAL(activated(int)), this, SLOT(showAxisFormatOptions(int)));
	connect(boxPrecision, SIGNAL(valueChanged(int)), this, SLOT(setLabelsNumericFormat(int)));
}

void AxesDialog::initFramePage()
{
	frame = new QWidget();

    boxFramed = new QGroupBox(tr("Canvas frame"));
    boxFramed->setCheckable (true);

    QGridLayout * boxFramedLayout = new QGridLayout( boxFramed );
    boxFramedLayout->addWidget( new QLabel(tr( "Color" )), 0, 0 );
    boxFrameColor= new ColorButton(boxFramed);
    boxFramedLayout->addWidget( boxFrameColor, 0, 1 );

    boxFramedLayout->addWidget( new QLabel(tr( "Width" )), 1, 0 );
    boxFrameWidth= new QSpinBox();
    boxFrameWidth->setMinimum(1);
    boxFramedLayout->addWidget( boxFrameWidth, 1, 1 );

    boxFramedLayout->setRowStretch( 2, 1 );

    QGroupBox * boxAxes = new QGroupBox(tr("Axes"));
    QGridLayout * boxAxesLayout = new QGridLayout( boxAxes );
    boxBackbones = new QCheckBox();
    boxBackbones->setText( tr( "Draw backbones" ) );
    boxAxesLayout->addWidget( boxBackbones, 0, 0 );

    boxAxesLayout->addWidget( new QLabel(tr( "Line Width" )), 1, 0 );
    boxAxesLinewidth = new QSpinBox();
    boxAxesLinewidth->setRange( 1, 100 );
    boxAxesLayout->addWidget( boxAxesLinewidth, 1, 1 );

    boxAxesLayout->addWidget( new QLabel(tr( "Major ticks length" )), 2, 0 );
    boxMajorTicksLength = new QSpinBox();
    boxMajorTicksLength->setRange( 0, 1000 );
    boxAxesLayout->addWidget( boxMajorTicksLength, 2, 1 );

    boxAxesLayout->addWidget( new QLabel(tr( "Minor ticks length" )), 3, 0 );
    boxMinorTicksLength = new QSpinBox();
    boxMinorTicksLength->setRange( 0, 1000 );
    boxAxesLayout->addWidget( boxMinorTicksLength, 3, 1 );

    boxAxesLayout->setRowStretch( 4, 1 );

    QVBoxLayout *vl = new QVBoxLayout();

    frameDefaultBtn = new QPushButton(tr("Set As &Default"));
    connect(frameDefaultBtn, SIGNAL(clicked()), this, SLOT(setFrameDefaultValues()));
	vl->addWidget(frameDefaultBtn);

	QLabel *l = new QLabel(tr("Apply &to..."));
	vl->addWidget(l);

	canvasFrameApplyToBox = new QComboBox();
	canvasFrameApplyToBox->insertItem(tr("Layer"));
    canvasFrameApplyToBox->insertItem(tr("Window"));
    canvasFrameApplyToBox->insertItem(tr("All Windows"));
	vl->addWidget(canvasFrameApplyToBox);
	vl->addStretch();

	l->setBuddy(canvasFrameApplyToBox);

    QHBoxLayout * mainLayout = new QHBoxLayout(frame);
    mainLayout->addWidget(boxFramed, 1);
    mainLayout->addWidget(boxAxes, 1);
    mainLayout->addLayout(vl);

    generalDialog->addTab(frame, tr( "General" ) );

	connect(boxFrameColor, SIGNAL(colorChanged()), this, SLOT(applyCanvasFormat()));
	connect(boxBackbones, SIGNAL(toggled(bool)), this, SLOT(applyCanvasFormat()));
	connect(boxFramed, SIGNAL(toggled(bool)), this, SLOT(applyCanvasFormat()));
	connect(boxFrameWidth, SIGNAL(valueChanged (int)), this, SLOT(applyCanvasFormat()));
	connect(boxAxesLinewidth, SIGNAL(valueChanged (int)), this, SLOT(applyCanvasFormat()));
	connect(boxMajorTicksLength, SIGNAL(valueChanged (int)), this, SLOT(changeMajorTicksLength(int)));
	connect(boxMinorTicksLength, SIGNAL(valueChanged (int)), this, SLOT(changeMinorTicksLength(int)));
}

void AxesDialog::changeMinorTicksLength (int minLength)
{
	applyCanvasFormat();
	boxMajorTicksLength->setMinValue(minLength);
}

void AxesDialog::changeMajorTicksLength (int majLength)
{
	applyCanvasFormat();
	boxMinorTicksLength->setMaxValue(majLength);
}

void AxesDialog::showAxisFormatOptions(int format)
{
	int axis = mapToQwtAxisId();

	boxFormat->clear();
	boxFormat->setEditable(false);
	boxFormat->hide();
	boxPrecision->hide();
	boxColName->hide();
	label1->hide();
	label2->hide();
	label3->hide();
	boxShowFormula->hide();
	boxFormula->hide();
	boxTableName->hide();
	labelTable->hide();

	switch (format)
	{
		case 0:
			label2->show();
			boxFormat->show();
			boxFormat->insertItem(tr( "Automatic" ) );
			boxFormat->insertItem(tr( "Decimal: 10000.0" ) );
			boxFormat->insertItem(tr( "Scientific: 1e4" ) );
			boxFormat->insertItem(tr( "Scientific: 1x10^4" ) );
			boxFormat->insertItem(tr( "Engineering: 10k" ) );
			boxFormat->insertItem(tr( "Scientific: 1·10^4" ) );
			boxFormat->setCurrentIndex(d_graph->axisLabelFormat(axis));

			label3->show();
			label3->setText(tr("Precision"));
			boxPrecision->show();
			boxShowFormula->show();

			showAxisFormula(mapToQwtAxisId());
			break;

		case 1:
			label1->show();
			boxColName->show();
			break;

		case 2:
        {
            int day = (QDate::currentDate()).dayOfWeek();
            label2->show();
            boxFormat->show();
            boxFormat->insertItem(QDate::shortDayName(day));
            boxFormat->insertItem(QDate::longDayName(day));
            boxFormat->insertItem((QDate::shortDayName(day)).left(1));
            boxFormat->setCurrentIndex((d_graph->axisFormatInfo(axis)).toInt());
        }
        break;

		case 3:
        {
            int month = (QDate::currentDate()).month();
            label2->show();
            boxFormat->show();
            boxFormat->insertItem(QDate::shortMonthName(month));
            boxFormat->insertItem(QDate::longMonthName(month));
            boxFormat->insertItem((QDate::shortMonthName(month)).left(1));
            boxFormat->setCurrentIndex((d_graph->axisFormatInfo(axis)).toInt());
        }
        break;

		case 4:
		{
			label2->show();
			boxFormat->show();
			boxFormat->setEditable(true);

			ScaleDraw *scaleDraw = (ScaleDraw *) d_graph->axisScaleDraw(axis);
			if (scaleDraw){
				QString formatInfo = scaleDraw->formatString();
				boxFormat->insertItem(formatInfo);
				boxFormat->setCurrentText(formatInfo);
			}

			boxFormat->insertItem("h");
			boxFormat->insertItem("h ap");
			boxFormat->insertItem("h AP");
			boxFormat->insertItem("M");
			boxFormat->insertItem("S");
			boxFormat->insertItem("h:mm");
			boxFormat->insertItem("h:mm ap");
			boxFormat->insertItem("hh:mm");
			boxFormat->insertItem("h:mm:ss");
			boxFormat->insertItem("h:mm:ss.zzz");
			boxFormat->insertItem("m");
			boxFormat->insertItem("mm");
			boxFormat->insertItem("mm:ss");
			boxFormat->insertItem("mm:ss.zzz");
			boxFormat->insertItem("hmm");
			boxFormat->insertItem("hmmss");
			boxFormat->insertItem("hhmmss");
        }
        break;

		case 5:
		{
			label2->show();
			boxFormat->show();
			boxFormat->setEditable(true);

			ScaleDraw *scaleDraw = (ScaleDraw *) d_graph->axisScaleDraw(axis);
			if (scaleDraw){
				QString formatInfo = scaleDraw->formatString();
				boxFormat->insertItem(formatInfo);
				boxFormat->setCurrentText(formatInfo);
			}
			boxFormat->insertItem("yyyy-MM-dd hh:mm:ss");
			boxFormat->insertItem("yyyy/MM/dd hh:mm:ss");
			boxFormat->insertItem("yyyy.MM.dd hh:mm:ss");
			boxFormat->insertItem("dd-MM-yyyy hh:mm:ss");
			boxFormat->insertItem("dd/MM/yyyy hh:mm:ss");
			boxFormat->insertItem("dd.MM.yyyy hh:mm:ss");

			boxFormat->insertItem("yyyy-MM-dd");
			boxFormat->insertItem("yyyy/MM/dd");
			boxFormat->insertItem("yyyy.MM.dd");
			boxFormat->insertItem("dd-MM-yyyy");
			boxFormat->insertItem("dd/MM/yyyy");
			boxFormat->insertItem("dd.MM.yyyy");
			boxFormat->insertItem("ddd MMMM d yy");
        }
        break;

		case 6:
        {
            labelTable->show();
            QString tableName = d_graph->axisFormatInfo(axis);
			if (((ApplicationWindow *)parent())->tableNames().contains(tableName))
                boxTableName->setCurrentText(tableName);
            boxTableName->show();
        }
        break;
	}
}

void AxesDialog::updateAxisType(int)
{
	boxAxisType->setCurrentIndex(mapToQwtAxisId());
}

void AxesDialog::showAxis()
{
	bool ok = boxShowAxis->isChecked();
	axisFormatBox->setEnabled(ok);
	boxShowLabels->setEnabled(ok);
	labelBox->setEnabled(ok);
	boxAxisBackbone->setEnabled(ok);

    int axis = -1;
    int a = axesTitlesList->currentRow();
    switch(a){
        case 0:
            axis = QwtPlot::xBottom;
        break;
        case 1:
            axis = QwtPlot::yLeft;
        break;
        case 2:
            axis = QwtPlot::xTop;
        break;
        case 3:
            axis = QwtPlot::yRight;
        break;
    }

    bool labels=false;
    if (tickLabelsOn[axis] == "1")
        labels=true;

    boxFormat->setEnabled(labels && ok);
    boxColName->setEnabled(labels && ok);
    boxShowFormula->setEnabled(labels && ok);
	boxFormula->setEnabled(labels && ok);
	boxAngle->setEnabled(labels && ok);
	boxPrecision->setEnabled(labels && ok);

	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
	   formula = QString();

    showAxis(axis, boxAxisType->currentIndex(), boxColName->currentText(),ok, boxMajorTicksType->currentIndex(),
            boxMinorTicksType->currentIndex(), boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
			boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color(),
			boxTickLabelDistance->value(), boxAxisBackbone->isChecked(), showTicksPolicyBox->currentIndex());
}

void AxesDialog::updateShowBox(int axis)
{
	int a = mapToQwtAxis(axis);
	bool axisOn = d_graph->axisEnabled(a);
	boxShowAxis->setChecked(axisOn);

	int labelsOn = tickLabelsOn[a].toInt();
	boxShowLabels->setChecked(labelsOn);
	boxFormat->setEnabled(labelsOn && axisOn);
	boxAngle->setEnabled(labelsOn && axisOn);
	boxAngle->setValue(d_graph->labelsRotation(a));

	bool ok = boxShowAxis->isChecked();
	axisFormatBox->setEnabled(ok);
	boxShowLabels->setEnabled(ok);
	labelBox->setEnabled(ok);
}

void AxesDialog::customAxisFont()
{
	bool ok = false;
	int axis = mapToQwtAxisId();
	QFont fnt = QFontDialog::getFont(&ok, d_graph->axisFont(axis), this);
	if (ok){
		d_graph->setAxisFont(axis, fnt);
		applyAxisFormat();//customize all other layers
	}
}

void AxesDialog::accept()
{
	if (updatePlot())
		close();
}

void AxesDialog::majorGridEnabled(bool on)
{
	boxTypeMajor->setEnabled(on);
	boxColorMajor->setEnabled(on);
	boxWidthMajor->setEnabled(on);

    updateGrid();
}

void AxesDialog::minorGridEnabled(bool on)
{
	boxTypeMinor->setEnabled(on);
	boxColorMinor->setEnabled(on);
	boxWidthMinor->setEnabled(on);

	updateGrid();
}

void AxesDialog::updateGrid()
{
	if (generalDialog->currentWidget() != gridPage)
		return;

    switch(boxApplyGridFormat->currentIndex()){
        case 0:
        {
            applyChangesToGrid(d_graph->grid());
            d_graph->replot();
            d_graph->notifyChanges();
        }
        break;

        case 1:
        {
            MultiLayer *plot = d_graph->multiLayer();
            if (!plot)
                return;

            QList<Graph *> layers = plot->layersList();
			foreach(Graph *g, layers){
                if (g->isPiePlot())
                    continue;

                applyChangesToGrid(g->grid());
                g->replot();
            }
            plot->applicationWindow()->modifiedProject();
        }
        break;

        case 2:
        {
            ApplicationWindow *app = (ApplicationWindow *)parent();
            if (!app)
                return;

            QList<MdiSubWindow *> windows = app->windowsList();
            foreach(MdiSubWindow *w, windows){
                if (w->isA("MultiLayer")){
                    QList<Graph *> layers = ((MultiLayer*)w)->layersList();
                    foreach(Graph *g, layers){
                        if (g->isPiePlot())
                            continue;
                        applyChangesToGrid(g->grid());
                        g->replot();
                    }
                }
            }
            app->modifiedProject();
        }
        break;
    }
}

void AxesDialog::applyChangesToGrid(Grid *grid)
{
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

	grid->enableZeroLineX(boxXLine->isChecked());
	grid->enableZeroLineY(boxYLine->isChecked());
	grid->setAxis(boxGridXAxis->currentIndex() + 2, boxGridYAxis->currentIndex());
	grid->setRenderHint(QwtPlotItem::RenderAntialiased, boxAntialiseGrid->isChecked());
}

void AxesDialog::showGridOptions(int axis)
{
    Grid *grd = (Grid *)d_graph->grid();
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

        boxXLine->setEnabled(true);
        boxYLine->setDisabled(true);

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

        boxXLine->setDisabled(true);
        boxYLine->setEnabled(true);

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

    boxXLine->setChecked(grd->xZeroLineEnabled());
    boxYLine->setChecked(grd->yZeroLineEnabled());

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

void AxesDialog::stepEnabled()
{
	boxStep->setEnabled(btnStep->isChecked ());
	boxUnit->setEnabled(btnStep->isChecked ());
	boxMajorValue->setDisabled(btnStep->isChecked ());
	btnMajor->setChecked(!btnStep->isChecked ());
}

void AxesDialog::stepDisabled()
{
	boxStep->setDisabled(btnMajor->isChecked ());
	boxUnit->setDisabled(btnMajor->isChecked ());
	boxMajorValue->setEnabled(btnMajor->isChecked ());
	btnStep->setChecked(!btnMajor->isChecked ());
}

void AxesDialog::updateAxisColor(int)
{
	int a = mapToQwtAxisId();
	boxAxisColor->blockSignals(true);
	boxAxisColor->setColor(d_graph->axisColor(a));
	boxAxisColor->blockSignals(false);

	boxAxisNumColor->blockSignals(true);
	boxAxisNumColor->setColor(d_graph->axisLabelsColor(a));
	boxAxisNumColor->blockSignals(false);

	axisLabelColorButton->blockSignals(true);
	axisLabelColorButton->setColor(d_graph->axisTitleColor(a));
	axisLabelColorButton->blockSignals(false);
}

bool AxesDialog::updatePlot(QWidget *page)
{
	QWidget *currentWidget = generalDialog->currentWidget();
	if (page)
		currentWidget = page;

	if (currentWidget == scalesPage){
		int a = mapToQwtAxis(axesList->currentRow());
		ScaleDraw::ScaleType type = d_graph->axisType(a);

		double start = 0.0, end = 0.0;
		if (type == ScaleDraw::Date || type == ScaleDraw::Time){
			start = Table::fromDateTime(boxStartDateTime->dateTime());
			end = Table::fromDateTime(boxEndDateTime->dateTime());
			if (type == ScaleDraw::Time && end < start)
				end += 1.0;
		} else {
			start = boxStart->value();
			end = boxEnd->value();
		}

		double step = 0.0;
		if (btnStep->isChecked()){
			step = boxStep->value();
			if (type == ScaleDraw::Time){
				switch (boxUnit->currentIndex()){
					case 0:
						step /= 86400000.0;
					break;
					case 1:
						step /= 86400.0;
					break;
					case 2:
						step /= 1440.0;
					break;
					case 3:
						step /= 24.0;
					break;
				}
			} else if (type == ScaleDraw::Date){
				switch (boxUnit->currentIndex()){
				   case 0:
					   step *= 1.0/1440.0;//min
				   break;
				   case 1:
						step *= 1.0/24.0;//hour
				   break;
				   case 2:
					   step *= 1;//day
				   break;
				   case 3:
						step *= 7;//week
				   break;
				   case 4:
						step *= 30;//month
				   break;
				   case 5:
						step *= 365;//year
				   break;
				}
			}
		}

		double breakLeft = -DBL_MAX, breakRight = DBL_MAX;
		if (boxAxesBreaks->isChecked()){
			breakLeft = qMin(boxBreakStart->value(), boxBreakEnd->value());
			breakRight = qMax(boxBreakStart->value(), boxBreakEnd->value());
		}

		d_graph->setScale(a, start, end, step, boxMajorValue->value(), boxMinorValue->currentText().toInt(),
                          boxScaleType->currentIndex(), btnInvert->isChecked(), breakLeft, breakRight,
                          boxBreakPosition->value(), boxStepBeforeBreak->value(), boxStepAfterBreak->value(),
                          boxMinorTicksBeforeBreak->currentText().toInt(), boxMinorTicksAfterBreak->currentText().toInt(),
                          boxLog10AfterBreak->isChecked(), boxBreakWidth->value(), boxBreakDecoration->isChecked());

		if (d_graph->hasSynchronizedScaleDivisions() && (a == QwtPlot::xTop || a == QwtPlot::yRight)){
			d_graph->updateOppositeScaleDiv(d_graph->oppositeAxis(a));
			d_graph->replot();
			d_graph->updateMarkersBoundingRect();
		}

		d_graph->notifyChanges();
	} else if (currentWidget == gridPage)
		updateGrid();
	else if (currentWidget == axesPage){
		int axis = mapToQwtAxisId();
		int format = boxAxisType->currentIndex();

		QString formatInfo = QString::null;
		if (format == ScaleDraw::Numeric){
			if (boxShowFormula->isChecked()){
				QString formula = boxFormula->text().lower();
				try {
					double value = 1.0;
					MyParser parser;
					if (formula.contains("x"))
						parser.DefineVar("x", &value);
					else if (formula.contains("y"))
						parser.DefineVar("y", &value);
					parser.SetExpr(formula.ascii());
					parser.Eval();
				} catch(mu::ParserError &e) {
					QMessageBox::critical(this, tr("QtiPlot - Formula input error"), QString::fromStdString(e.GetMsg())+"\n"+
							tr("Valid variables are 'x' for Top/Bottom axes and 'y' for Left/Right axes!"));
					boxFormula->setFocus();
					return false;
				}
			}
		} else if (format == ScaleDraw::Date || format == ScaleDraw::Time)
			formatInfo = boxFormat->currentText();
		else if (format == ScaleDraw::Day || format == ScaleDraw::Month)
            formatInfo = QString::number(boxFormat->currentIndex());
          else if (format == ScaleDraw::ColHeader)
            formatInfo = boxTableName->currentText();
          else
            formatInfo = boxColName->currentText();

		if (d_graph->axisTitleString(axis) != boxTitle->text())
			d_graph->setAxisTitle(axis, boxTitle->text());

		d_graph->setAxisTitleDistance(axis, boxLabelsDistance->value());

		QString formula = boxFormula->text();
		if (!boxShowFormula->isChecked())
			formula = QString();
		showAxis(axis, format, formatInfo, boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
				boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
				boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color(),
				boxTickLabelDistance->value(), boxAxisBackbone->isChecked(), showTicksPolicyBox->currentIndex());

		if (axis == QwtPlot::yRight){
			QwtScaleWidget *scale = d_graph->axisWidget(axis);
			if (scale){
				scale->setLayoutFlag(QwtScaleWidget::TitleInverted, invertTitleBox->isChecked());
				scale->repaint();
			}
		}
		applyAxisFormat();
	} else if (currentWidget == frame)
		applyCanvasFormat();

	return true;
}

void AxesDialog::setGraph(Graph *g)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	if (!g)
        return;

	d_graph = g;

	boxTableName->insertStringList(app->tableNames());
	boxColName-> insertStringList(app->columnsList(Table::All));

	updateTitleBox(0);

	for (int axis=0; axis<QwtPlot::axisCnt; axis++){
		const QwtScaleDraw *sd = g->axisScaleDraw (axis);
		tickLabelsOn << QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels));
	}

	boxAxesLinewidth->setValue(g->axesLinewidth());
    boxBackbones->setChecked (d_graph->axesBackbones());

	boxFramed->setChecked(d_graph->canvasFrameWidth()>0);
	boxFrameColor->blockSignals(true);
	boxFrameColor->setColor(d_graph->canvasFrameColor());
	boxFrameColor->blockSignals(false);
	boxFrameWidth->setValue(d_graph->canvasFrameWidth());

	boxMinorTicksLength->setValue(g->minorTickLength());
	boxMajorTicksLength->setValue(g->majorTickLength());

	showGridOptions(axesGridList->currentRow());
}

int AxesDialog::mapToQwtAxisId()
{
	return mapToQwtAxis(axesTitlesList->currentRow());
}

int AxesDialog::mapToQwtAxis(int axis)
{
	int a = -1;
	switch(axis)
		{
		case 0:
			a = QwtPlot::xBottom;
		break;
		case 1:
			a = QwtPlot::yLeft;
		break;
		case 2:
			 a = QwtPlot::xTop;
		break;
		case 3:
			 a = QwtPlot::yRight;
		break;
		}
	return a;
}

void AxesDialog::updateScale()
{
    int axis = axesList->currentRow();

    boxStart->clear();
    boxEnd->clear();
    boxStep->clear();
    boxUnit->hide();
    boxUnit->clear();

    int a = mapToQwtAxis(axis);
    const QwtScaleDiv *scDiv = d_graph->axisScaleDiv(a);
    double start = QMIN(scDiv->lowerBound(), scDiv->upperBound());
    double end = QMAX(scDiv->lowerBound(), scDiv->upperBound());

    ScaleDraw::ScaleType type = d_graph->axisType(a);
	if (type == ScaleDraw::Date){
		QString format = ((ScaleDraw *)d_graph->axisScaleDraw(a))->formatString();

		boxStart->hide();
		boxStartDateTime->show();
		boxStartDateTime->setDisplayFormat(format);
		boxStartDateTime->setDateTime(Table::dateTime(start));

		boxEnd->hide();
		boxEndDateTime->show();
		boxEndDateTime->setDisplayFormat(format);
		boxEndDateTime->setDateTime(Table::dateTime(end));

		boxUnit->show();
		boxUnit->insertItem(tr("min."));
		boxUnit->insertItem(tr("hours"));
		boxUnit->insertItem(tr("days"));
		boxUnit->insertItem(tr("weeks"));
		boxUnit->insertItem(tr("months"));
		boxUnit->insertItem(tr("years"));

		double step = d_graph->axisStep(a);
		if (step < 1.0/24.0){
			boxUnit->setCurrentIndex(0);
			boxStep->setValue(step*24*60);
		} else if (step < 1.0){
			boxUnit->setCurrentIndex(1);
			boxStep->setValue(step*24);
		} else if (step < 30){
			if (int(step) % 7){
				boxUnit->setCurrentIndex(2);
				boxStep->setValue(step);
			} else {
				boxUnit->setCurrentIndex(3);
				boxStep->setValue(step/7.0);
			}
		} else if (step < 365){
			boxUnit->setCurrentIndex(4);
			boxStep->setValue(step/30.0);
		} else {
			boxUnit->setCurrentIndex(5);
			boxStep->setValue(step/365.0);
		}

		boxStep->setSingleStep(1);
	} else if (type == ScaleDraw::Time){
		QString format = ((ScaleDraw *)d_graph->axisScaleDraw(a))->formatString();
		boxStart->hide();
		boxStartDateTime->show();
		boxStartDateTime->setDisplayFormat(format);
		boxStartDateTime->setDateTime(Table::dateTime(start));

		boxEnd->hide();
		boxEndDateTime->show();
		boxEndDateTime->setDisplayFormat(format);
		boxEndDateTime->setDateTime(Table::dateTime(end));

		boxUnit->show();
		boxUnit->insertItem(tr("millisec."));
		boxUnit->insertItem(tr("sec."));
		boxUnit->insertItem(tr("min."));
		boxUnit->insertItem(tr("hours"));

		double step = d_graph->axisStep(a);
		boxStep->setValue(step);
		if (step < 1.0/86400.0){
			boxUnit->setCurrentIndex(0);
			boxStep->setValue(step*864e5);
		} else if (step < 1.0/1440.0){
			boxUnit->setCurrentIndex(1);
			boxStep->setValue(step*864e3);
		} else if (step < 1/24.0){
			boxUnit->setCurrentIndex(2);
			boxStep->setValue(step*1440.0);
		} else {
			boxUnit->setCurrentIndex(3);
			boxStep->setValue(step*24.0);
		}
		boxStep->setSingleStep(1);
	} else {
	    boxStart->show();
        boxStart->setValue(start);
        boxStartDateTime->hide();
        boxEnd->show();
        boxEnd->setValue(end);
        boxEndDateTime->hide();
        boxStep->setValue(d_graph->axisStep(a));
        boxStep->setSingleStep(0.1);
	}

    double range = fabs(scDiv->range());
    ScaleEngine *sc_engine = (ScaleEngine *)d_graph->axisScaleEngine(a);
    if (sc_engine->axisBreakLeft() > -DBL_MAX)
        boxBreakStart->setValue(sc_engine->axisBreakLeft());
    else
        boxBreakStart->setValue(start + 0.25*range);

    if (sc_engine->axisBreakRight() < DBL_MAX)
        boxBreakEnd->setValue(sc_engine->axisBreakRight());
    else
        boxBreakEnd->setValue(start + 0.75*range);

    boxAxesBreaks->setChecked(sc_engine->hasBreak());
    boxBreakPosition->setValue(sc_engine->breakPosition());
    boxBreakWidth->setValue(sc_engine->breakWidth());
    boxStepBeforeBreak->setValue(sc_engine->stepBeforeBreak());
    boxStepAfterBreak->setValue(sc_engine->stepAfterBreak());

    ScaleTransformation::Type scale_type = sc_engine->type();
    boxMinorTicksBeforeBreak->clear();
    if (scale_type == ScaleTransformation::Log10)
        boxMinorTicksBeforeBreak->addItems(QStringList()<<"0"<<"2"<<"4"<<"8");
    else
        boxMinorTicksBeforeBreak->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");
    boxMinorTicksBeforeBreak->setEditText(QString::number(sc_engine->minTicksBeforeBreak()));


    boxMinorTicksAfterBreak->setEditText(QString::number(sc_engine->minTicksAfterBreak()));
    boxLog10AfterBreak->setChecked(sc_engine->log10ScaleAfterBreak());
    boxBreakDecoration->setChecked(sc_engine->hasBreakDecoration());

	QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);
	boxMajorValue->setValue(lst.count());

	if (d_graph->axisStep(a) != 0.0){
		btnStep->setChecked(true);
		boxStep->setEnabled(true);
		boxUnit->setEnabled(true);

		btnMajor->setChecked(false);
		boxMajorValue->setEnabled(false);
	} else{
		btnStep->setChecked(false);
		boxStep->setEnabled(false);
		boxUnit->setEnabled(false);
		btnMajor->setChecked(true);
		boxMajorValue->setEnabled(true);
	}

    btnInvert->setChecked(sc_engine->testAttribute(QwtScaleEngine::Inverted));
    boxScaleType->setCurrentIndex(scale_type);

    boxMinorValue->clear();
    if (scale_type == ScaleTransformation::Log10)
        boxMinorValue->addItems(QStringList()<<"0"<<"2"<<"4"<<"8");
    else
        boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");

    boxMinorValue->setEditText(QString::number(d_graph->axisMaxMinor(a)));
}

void AxesDialog::updateTitleBox(int axis)
{
	int axisId = mapToQwtAxis(axis);
	boxTitle->setText(d_graph->axisTitleString(axisId));
	boxLabelsDistance->blockSignals(true);
	boxLabelsDistance->setValue(d_graph->axisTitleDistance(axisId));
	boxLabelsDistance->blockSignals(false);
	if (axisId == QwtPlot::yRight){
		QwtScaleWidget *scale = d_graph->axisWidget(axisId);
		if (scale){
			invertTitleBox->blockSignals(true);
			invertTitleBox->setChecked(scale->testLayoutFlag(QwtScaleWidget::TitleInverted));
			invertTitleBox->blockSignals(false);
			invertTitleBox->show();
		}
	} else
		invertTitleBox->hide();
}

void AxesDialog::setAxisType(int)
{
	int a = mapToQwtAxisId();
	int style = (int)d_graph->axisType(a);
	boxAxisType->setCurrentIndex(style);
	showAxisFormatOptions(style);

	if (style == 1)
		boxColName->setCurrentText(d_graph->axisFormatInfo(a));
}

void AxesDialog::setBaselineDist(int)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)d_graph->axisWidget(mapToQwtAxisId());
	if (scale)
		boxBaseline->setValue(scale->margin());
	else
		boxBaseline->setValue(0);
}

void AxesDialog::updateCurrentAxis()
{
	int axis = mapToQwtAxisId();
	ScaleDraw *sd = (ScaleDraw *)d_graph->axisScaleDraw (axis);
	if (sd){
		boxAxisBackbone->setChecked(sd->hasComponent(QwtAbstractScaleDraw::Backbone));

		boxTickLabelDistance->blockSignals(true);
		boxTickLabelDistance->setValue(sd->spacing());
		boxTickLabelDistance->blockSignals(false);

		showTicksPolicyBox->blockSignals(true);
		showTicksPolicyBox->setCurrentIndex(sd->showTicksPolicy());
		showTicksPolicyBox->blockSignals(false);

		boxPrefix->setText(sd->prefix());
		boxSuffix->setText(sd->suffix());
	}
	boxShowLabels->repaint();
}

void AxesDialog::setTicksType(int)
{
	int a = mapToQwtAxisId();
	boxMajorTicksType->setCurrentIndex(d_graph->getMajorTicksType()[a]);
	boxMinorTicksType->setCurrentIndex(d_graph->getMinorTicksType()[a]);
}

void AxesDialog::updateTickLabelsList(bool on)
{
	int axis = mapToQwtAxisId();

	boxFormat->setEnabled(on && boxShowAxis->isChecked());
	boxColName->setEnabled(on && boxShowAxis->isChecked());
	boxAngle->setEnabled(on);
	boxPrecision->setEnabled(on);

	if (tickLabelsOn[axis] == QString::number(on))
		return;
	tickLabelsOn[axis] = QString::number(on);

    QString formatInfo = QString::null;
	int type = boxAxisType->currentIndex();
	if (type == ScaleDraw::Day || type == ScaleDraw::Month)
		formatInfo = QString::number(boxFormat->currentIndex());
	else if (type == ScaleDraw::Date|| type == ScaleDraw::Time)
		formatInfo = boxFormat->currentText();
	else
		formatInfo = boxColName->currentText();

	QString formula = boxFormula->text();
	if (!boxShowFormula->isChecked())
	   formula = QString();

    showAxis(axis, type, formatInfo, boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
                  boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(), boxPrecision->value(),
				  boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color(),
				  boxTickLabelDistance->value(), boxAxisBackbone->isChecked(), showTicksPolicyBox->currentIndex());
}

void AxesDialog::setCurrentScale(int axisPos)
{
int axis = -1;
switch (axisPos)
	{
	case QwtScaleDraw::LeftScale:
		axis = 1;
	break;
	case QwtScaleDraw::BottomScale:
		axis = 0;
	break;
	case QwtScaleDraw::RightScale:
		axis = 3;
	break;
	case QwtScaleDraw::TopScale:
		axis = 2;
	break;
	}
if (generalDialog->currentPage()==(QWidget*)scalesPage)
	axesList->setCurrentRow(axis);
else if (generalDialog->currentPage()==(QWidget*)axesPage)
	axesTitlesList->setCurrentRow(axis);
}

void AxesDialog::showAxesPage()
{
	if (generalDialog->currentWidget()!=(QWidget*)axesPage)
		generalDialog->setCurrentWidget(axesPage);
}

void AxesDialog::showGridPage()
{
	if (generalDialog->currentWidget()!=(QWidget*)gridPage)
		generalDialog->setCurrentWidget(gridPage);
}

void AxesDialog::setLabelsNumericFormat(int)
{
	int axis = mapToQwtAxisId();
	int type = boxAxisType->currentIndex();
	int prec = boxPrecision->value();
	int format = boxFormat->currentIndex();

    QString formatInfo = QString::null;
	if (type == ScaleDraw::Numeric){
		if (d_graph->axisLabelFormat(axis) == format &&
			d_graph->axisLabelPrecision(axis) == prec)
			return;

		boxPrecision->setEnabled(true);
    } else if (type == ScaleDraw::Day || type == ScaleDraw::Month)
		formatInfo = QString::number(format);
	else if (type == ScaleDraw::Date || type == ScaleDraw::Time)
		formatInfo = boxFormat->currentText();
	else
		formatInfo = boxColName->currentText();

	QString formula =  boxFormula->text();
	if (!boxShowFormula->isChecked())
		formula = QString();

    showAxis(axis, type, formatInfo, boxShowAxis->isChecked(), boxMajorTicksType->currentIndex(),
			boxMinorTicksType->currentIndex(), boxShowLabels->isChecked(), boxAxisColor->color(),
			format, prec, boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color(),
			boxTickLabelDistance->value(), boxAxisBackbone->isChecked(), showTicksPolicyBox->currentIndex());
}

void AxesDialog::showAxisFormula(int axis)
{
    QString formula = d_graph->axisFormula(axis);
    if (!formula.isEmpty()){
        boxShowFormula->setChecked(true);
        boxFormula->show();
        boxFormula->setText(formula);
    } else {
        boxShowFormula->setChecked(false);
        boxFormula->clear();
        boxFormula->hide();
    }
}

void AxesDialog::updateLabelsFormat(int)
{
        if (boxAxisType->currentIndex() != ScaleDraw::Numeric)
        	return;

		int a = mapToQwtAxisId();
		int format = d_graph->axisLabelFormat(a);
        boxFormat->setCurrentIndex(format);
		boxPrecision->setValue(d_graph->axisLabelPrecision(a));
		boxPrecision->setEnabled(true);

        QString formula = d_graph->axisFormula(a);
        if (!formula.isEmpty())
        {
        	boxShowFormula->setChecked(true);
        	boxFormula->show();
        	boxFormula->setText(formula);
        }
        else
        {
        	boxShowFormula->setChecked(false);
        	boxFormula->clear();
        	boxFormula->hide();
        }
}

void AxesDialog::showGeneralPage()
{
	generalDialog->showPage (frame);
}

void AxesDialog::showFormulaBox()
{
	if (boxShowFormula->isChecked())
		boxFormula->show();
	else
		boxFormula->hide();
}

void AxesDialog::pickAxisLabelColor()
{
	d_graph->setAxisTitleColor(mapToQwtAxisId(), axisLabelColorButton->color());
	d_graph->replot();
}

void AxesDialog::customAxisLabelFont()
{
	int axis = mapToQwtAxisId();
	bool okF;
	QFont oldFont = d_graph->axisTitleFont(axis);
	QFont fnt = QFontDialog::getFont( &okF, oldFont,this);
	if (okF && fnt != oldFont)
		d_graph->setAxisTitleFont(axis, fnt);
}

void AxesDialog::pageChanged ( QWidget *page )
{
	//updatePlot(lastPage);

	if (lastPage == scalesPage && page == axesPage){
		axesTitlesList->setCurrentRow(axesList->currentRow());
	} else if (lastPage == axesPage && page == scalesPage){
		axesList->setCurrentRow(axesTitlesList->currentRow());
		updateScale();
	}

	if (page == axesPage)
		updateCurrentAxis();

	lastPage = page;
}

int AxesDialog::exec()
{
	axesList->setCurrentRow(0);
	axesGridList->setCurrentRow(0);
	axesTitlesList->setCurrentRow(0);

    setModal(true);
    show();
	return 0;
}

void AxesDialog::updateMinorTicksList(int scaleType)
{
	updatePlot();

	boxMinorValue->clear();
	if (scaleType)//log scale
		boxMinorValue->addItems(QStringList()<<"0"<<"2"<<"4"<<"8");
	else
		boxMinorValue->addItems(QStringList()<<"0"<<"1"<<"4"<<"9"<<"14"<<"19");

	int a = mapToQwtAxis(axesList->currentRow());
	boxMinorValue->setEditText(QString::number(d_graph->axisMaxMinor(a)));

	if (!d_graph)
		return;

	int functions = 0;
	QList<QwtPlotItem *> cvs = d_graph->curvesList();
	foreach(QwtPlotItem *item, cvs){
		if(item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		FunctionCurve *c = (FunctionCurve *)item;
		if (c->type() == Graph::Function){
			c->loadData();
			functions++;
		}
	}
	if (functions)
		d_graph->replot();
}

void AxesDialog::showAxis(int axis, int type, const QString& labelsColName, bool axisOn,
		int majTicksType, int minTicksType, bool labelsOn, const QColor& c, int format,
		int prec, int rotation, int baselineDist, const QString& formula, const QColor& labelsColor,
		int spacing, bool backbone, int showTicks)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

	Table *w = app->table(labelsColName);
	if ((type == ScaleDraw::Text || type == ScaleDraw::ColHeader) && !w)
		return;

	if (!d_graph)
		return;
	d_graph->showAxis(axis, type, labelsColName, w, axisOn, majTicksType, minTicksType, labelsOn,
			c, format, prec, rotation, baselineDist, formula, labelsColor, spacing, backbone,
			(ScaleDraw::ShowTicksPolicy)showTicks, boxPrefix->text(), boxSuffix->text());
}

void AxesDialog::applyCanvasFormatTo(Graph *g)
{
    if (!g)
        return;

    g->setAxesLinewidth(boxAxesLinewidth->value());
    g->changeTicksLength(boxMinorTicksLength->value(), boxMajorTicksLength->value());
    if (boxFramed->isChecked())
        g->setCanvasFrame(boxFrameWidth->value(), boxFrameColor->color());
    else
        g->setCanvasFrame(0);
    g->drawAxesBackbones(boxBackbones->isChecked());
    g->replot();
}

void AxesDialog::applyCanvasFormat()
{
    if (generalDialog->currentWidget() != frame)
		return;

    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(canvasFrameApplyToBox->currentIndex()){
		case 0://this layer
			applyCanvasFormatTo(d_graph);
		break;

		case 1://this window
		{
			QList<Graph *> layersLst = d_graph->multiLayer()->layersList();
			foreach(Graph *g, layersLst)
				applyCanvasFormatTo(g);
		}
		break;

		case 2://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applyCanvasFormatTo(g);
			}
		}
		break;

		default:
			break;
	}
	app->modifiedProject();
}

void AxesDialog::setFrameDefaultValues()
{
    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
		return;

    if (boxFramed->isChecked())
		app->canvasFrameWidth = boxFrameWidth->value();
	else
		app->canvasFrameWidth = 0;

    app->d_canvas_frame_color = boxFrameColor->color();
	app->drawBackbones = boxBackbones->isChecked();
	app->axesLineWidth = boxAxesLinewidth->value();
	app->majTicksLength = boxMajorTicksLength->value();
	app->minTicksLength = boxMinorTicksLength->value();

	app->saveSettings();
}

void AxesDialog::applyAxisFormatToLayer(Graph *g)
{
	if (!g)
		return;

	for (int i = 0; i<QwtPlot::axisCnt; i++){
		if (!g->axisEnabled(i))
			continue;

		QwtScaleWidget *axis = g->axisWidget(i);
		if (!axis)
			continue;

		axis->setMargin(boxBaseline->value());
		QPalette pal = axis->palette();
		if (pal.color(QPalette::Active, QColorGroup::Foreground) != boxAxisColor->color())
			pal.setColor(QColorGroup::Foreground, boxAxisColor->color());
		if (pal.color(QPalette::Active, QColorGroup::Text) != boxAxisNumColor->color())
			pal.setColor(QColorGroup::Text, boxAxisNumColor->color());
		axis->setPalette(pal);

		g->setAxisTicksLength(i, boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
							boxMinorTicksLength->value(), boxMajorTicksLength->value());
		g->setAxisFont(i, d_graph->axisFont(mapToQwtAxisId()));

		ScaleDraw *sd = (ScaleDraw *)g->axisScaleDraw(i);
		sd->setSpacing(boxTickLabelDistance->value());
		sd->enableComponent (QwtAbstractScaleDraw::Backbone, boxAxisBackbone->isChecked());
		sd->setShowTicksPolicy((ScaleDraw::ShowTicksPolicy)showTicksPolicyBox->currentIndex());

		axis->repaint();
	}
	g->updateLayout();
	g->replot();
}

void AxesDialog::applyAxisFormat()
{
    if (generalDialog->currentWidget() != axesPage)
		return;

    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	switch(axisFormatApplyToBox->currentIndex()){
		case 0://current axis
		break;
		case 1://this layer
			applyAxisFormatToLayer(d_graph);
		break;
		case 2://this window
		{
			QList<Graph *> layersLst = d_graph->multiLayer()->layersList();
			foreach(Graph *g, layersLst)
				applyAxisFormatToLayer(g);
		}
		break;
		case 3://all windows
		{
			QList<MdiSubWindow *> windows = app->windowsList();
			foreach(MdiSubWindow *w, windows){
				MultiLayer *ml = qobject_cast<MultiLayer *>(w);
				if (!ml)
					continue;

				QList<Graph *> layersLst = ml->layersList();
				foreach(Graph *g, layersLst)
					applyAxisFormatToLayer(g);
			}
		}
		break;
		default:
			break;
	}
	app->modifiedProject();
}
