/***************************************************************************
    File                 : Plots3DConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 3D plots preferences page implementation

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
#include "Plots3DConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>
#include <ColorMapEditor.h>
#include <DoubleSpinBox.h>
#include <PenStyleBox.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QFontDialog>
#include <QIcon>

Plots3DConfigPage::Plots3DConfigPage(QWidget *parent)
    : ConfigPage(parent)
{
	QGroupBox *groupBox3D = new QGroupBox();
	QHBoxLayout * topLayout = new QHBoxLayout( groupBox3D );

	groupBox3DCol = new QGroupBox();
	QGridLayout * colorsLayout = new QGridLayout( groupBox3DCol );

	colorsLayout->addWidget( new QLabel(tr("Background")), 0, 0 );
	btnBackground3D = new ColorButton();
	colorsLayout->addWidget( btnBackground3D, 0, 1 );

	colorsLayout->addWidget( new QLabel(tr("Mesh")), 1, 0 );
	btnMesh = new ColorButton();
	colorsLayout->addWidget( btnMesh, 1, 1 );

	colorsLayout->addWidget( new QLabel(tr("Axes")), 2, 0 );
	btnAxes = new ColorButton();
	colorsLayout->addWidget( btnAxes, 2, 1 );

	colorsLayout->addWidget( new QLabel(tr("Labels")), 3, 0 );
	btnLabels = new ColorButton();
	colorsLayout->addWidget( btnLabels, 3, 1 );

	colorsLayout->addWidget( new QLabel(tr("Numbers")), 4, 0 );
	btnNumbers = new ColorButton();
	colorsLayout->addWidget( btnNumbers, 4, 1 );

	colorsLayout->addWidget( new QLabel(tr("Grids")), 5, 0 );
	btnGrid = new ColorButton();
	colorsLayout->addWidget( btnGrid, 5, 1 );

	colorsLayout->addWidget( new QLabel(tr("Minor Grids")), 6, 0 );
	btnGridMinor = new ColorButton();
	colorsLayout->addWidget( btnGridMinor, 6, 1 );

	colorMapBox = new QGroupBox();
	QHBoxLayout * colorMapLayout = new QHBoxLayout( colorMapBox );
	colorMapEditor = new ColorMapEditor();
	colorMapLayout->addWidget(colorMapEditor);

	groupBox3DFonts = new QGroupBox();
	QHBoxLayout * middleLayout = new QHBoxLayout( groupBox3DFonts );

	btnTitleFnt = new QPushButton();
	btnTitleFnt->setIcon(QIcon(":/font.png"));
	middleLayout->addWidget( btnTitleFnt );

	btnLabelsFnt = new QPushButton();
	btnLabelsFnt->setIcon(QIcon(":/font.png"));
	middleLayout->addWidget( btnLabelsFnt );

	btnNumFnt = new QPushButton();
	btnNumFnt->setIcon(QIcon(":/font.png"));
	middleLayout->addWidget( btnNumFnt );

	topLayout->addWidget(groupBox3DCol);
	topLayout->addWidget(colorMapBox);

	gridsGroup = new QGroupBox();
	QGridLayout * gridsLayout = new QGridLayout( gridsGroup );

	boxMajorGrids = new QCheckBox();
	gridsLayout->addWidget( boxMajorGrids, 0, 0 );

	lblMajorGridWidth = new QLabel();
	gridsLayout->addWidget( lblMajorGridWidth, 0, 1 );
	boxMajorGridWidth = new DoubleSpinBox();
	boxMajorGridWidth->setRange(0, 100);
	boxMajorGridWidth->setSingleStep(1);
	gridsLayout->addWidget( boxMajorGridWidth, 0, 2 );

	lblMajorGridStyle = new QLabel();
	gridsLayout->addWidget( lblMajorGridStyle, 0, 3 );
	boxMajorGridStyle = new QComboBox();
	gridsLayout->addWidget( boxMajorGridStyle, 0, 4 );

	boxMinorGrids = new QCheckBox();
	gridsLayout->addWidget( boxMinorGrids, 1, 0 );

	lblMinorGridWidth = new QLabel();
	gridsLayout->addWidget( lblMinorGridWidth, 1, 1 );
	boxMinorGridWidth = new DoubleSpinBox();
	boxMinorGridWidth->setRange(0, 100);
	boxMinorGridWidth->setSingleStep(1);
	gridsLayout->addWidget( boxMinorGridWidth, 1, 2 );

	lblMinorGridStyle = new QLabel();
	gridsLayout->addWidget( lblMinorGridStyle, 1, 3 );
	boxMinorGridStyle = new QComboBox();
	gridsLayout->addWidget( boxMinorGridStyle, 1, 4 );

	QGroupBox *groupBox3DOptions = new QGroupBox();
	QGridLayout * bottomLayout = new QGridLayout( groupBox3DOptions );

	lblResolution = new QLabel();
	bottomLayout->addWidget( lblResolution, 0, 0 );
	boxResolution = new QSpinBox();
	boxResolution->setRange(1, 100);
	bottomLayout->addWidget( boxResolution, 0, 1 );

	boxShowLegend = new QCheckBox();
	bottomLayout->addWidget( boxShowLegend, 1, 0 );

	boxSmoothMesh = new QCheckBox();
	bottomLayout->addWidget( boxSmoothMesh, 2, 0 );

	boxOrthogonal = new QCheckBox();
	bottomLayout->addWidget( boxOrthogonal, 0, 2 );

	boxAutoscale3DPlots = new QCheckBox();
	bottomLayout->addWidget( boxAutoscale3DPlots, 1, 2 );

	boxScaleFonts3DPlots = new QCheckBox();
	bottomLayout->addWidget( boxScaleFonts3DPlots, 2, 2 );

	lblProjection = new QLabel();
	bottomLayout->addWidget( lblProjection, 3, 0 );
	boxProjection = new QComboBox();
	bottomLayout->addWidget( boxProjection, 3, 1 );

	lblShadingStyle = new QLabel();
	bottomLayout->addWidget( lblShadingStyle, 3, 2 );
	boxShading = new QComboBox();
	bottomLayout->addWidget( boxShading, 3, 3 );

	QVBoxLayout * plots3DPageLayout = new QVBoxLayout( this );
	plots3DPageLayout->addWidget(groupBox3D);
	plots3DPageLayout->addWidget(groupBox3DFonts);
	plots3DPageLayout->addWidget(gridsGroup);
	plots3DPageLayout->addWidget(groupBox3DOptions);
	plots3DPageLayout->addStretch();

	connect(btnTitleFnt, &QAbstractButton::clicked, this, &Plots3DConfigPage::pick3DTitleFont);
	connect(btnNumFnt, &QAbstractButton::clicked, this, &Plots3DConfigPage::pick3DNumbersFont);
	connect(btnLabelsFnt, &QAbstractButton::clicked, this, &Plots3DConfigPage::pick3DAxesFont);
	connect(boxMajorGrids, &QAbstractButton::toggled, this, &Plots3DConfigPage::enableMajorGrids);
	connect(boxMinorGrids, &QAbstractButton::toggled, this, &Plots3DConfigPage::enableMinorGrids);
}

void Plots3DConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	d_3D_title_font = app->d_3D_title_font;
	d_3D_numbers_font = app->d_3D_numbers_font;
	d_3D_axes_font = app->d_3D_axes_font;

	btnBackground3D->setColor(app->d_3D_background_color);
	btnMesh->setColor(app->d_3D_mesh_color);
	btnAxes->setColor(app->d_3D_axes_color);
	btnLabels->setColor(app->d_3D_labels_color);
	btnNumbers->setColor(app->d_3D_numbers_color);
	btnGrid->setColor(app->d_3D_grid_color);
	btnGridMinor->setColor(app->d_3D_minor_grid_color);

	colorMapEditor->setColorMap(app->d_3D_color_map);

	boxMajorGrids->setChecked(app->d_3D_major_grids);
	boxMajorGridWidth->setValue(app->d_3D_major_width);
	boxMajorGridStyle->setCurrentIndex(app->d_3D_major_style);
	enableMajorGrids(app->d_3D_major_grids);

	boxMinorGrids->setChecked(app->d_3D_minor_grids);
	boxMinorGridWidth->setValue(app->d_3D_minor_width);
	boxMinorGridStyle->setCurrentIndex(app->d_3D_minor_style);
	enableMinorGrids(app->d_3D_minor_grids);

	boxResolution->setValue(app->d_3D_resolution);
	boxShowLegend->setChecked(app->d_3D_legend);
	boxSmoothMesh->setChecked(app->d_3D_smooth_mesh);
	boxOrthogonal->setChecked(app->d_3D_orthogonal);
	boxAutoscale3DPlots->setChecked(app->d_3D_autoscale);
	boxScaleFonts3DPlots->setChecked(app->scale3DPlotFonts());

	boxProjection->setCurrentIndex(app->d_3D_projection);
	boxShading->setCurrentIndex(app->d_3D_shading);
}

void Plots3DConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	app->d_3D_color_map = colorMapEditor->colorMap();
	app->d_3D_axes_color = btnAxes->color();
	app->d_3D_numbers_color = btnNumbers->color();
	app->d_3D_grid_color = btnGrid->color();
	app->d_3D_mesh_color = btnMesh->color();
	app->d_3D_background_color = btnBackground3D->color();
	app->d_3D_labels_color = btnLabels->color();
	app->d_3D_legend = boxShowLegend->isChecked();
	app->d_3D_projection = boxProjection->currentIndex();
	app->d_3D_shading = boxShading->currentIndex();
	app->d_3D_resolution = boxResolution->value();
	app->d_3D_title_font = d_3D_title_font;
	app->d_3D_numbers_font = d_3D_numbers_font;
	app->d_3D_axes_font = d_3D_axes_font;
	app->d_3D_orthogonal = boxOrthogonal->isChecked();
	app->d_3D_smooth_mesh = boxSmoothMesh->isChecked();
	app->d_3D_autoscale = boxAutoscale3DPlots->isChecked();
	app->setScale3DPlotFonts(boxScaleFonts3DPlots->isChecked());
	app->setPlot3DOptions();

	app->d_3D_grid_color = btnGrid->color();
	app->d_3D_minor_grid_color = btnGridMinor->color();
	app->d_3D_minor_grids = boxMinorGrids->isChecked();
	app->d_3D_major_grids = boxMajorGrids->isChecked();
	app->d_3D_major_style = boxMajorGridStyle->currentIndex();
	app->d_3D_minor_style = boxMinorGridStyle->currentIndex();
	app->d_3D_major_width = boxMajorGridWidth->value();
	app->d_3D_minor_width = boxMinorGridWidth->value();
}

void Plots3DConfigPage::retranslateUi()
{
	groupBox3DCol->setTitle(tr("Colors"));
	colorMapBox->setTitle(tr("Color Map"));
	groupBox3DFonts->setTitle(tr("Fonts"));
	btnTitleFnt->setText(tr("&Title"));
	btnLabelsFnt->setText(tr("&Axes Labels"));
	btnNumFnt->setText(tr("&Numbers"));

	gridsGroup->setTitle(tr("Grids"));
	boxMajorGrids->setText(tr("Major Grids"));
	boxMinorGrids->setText(tr("Minor Grids"));
	lblMajorGridWidth->setText(tr("Width"));
	lblMinorGridWidth->setText(tr("Width"));
	lblMajorGridStyle->setText(tr("Type"));
	lblMinorGridStyle->setText(tr("Type"));

	int idxMaj = boxMajorGridStyle->currentIndex();
	boxMajorGridStyle->clear();
	boxMajorGridStyle->addItem(tr("Solid"));
	boxMajorGridStyle->addItem(tr("Dash"));
	boxMajorGridStyle->addItem(tr("Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot"));
	boxMajorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMajorGridStyle->setCurrentIndex(idxMaj >= 0 ? idxMaj : 0);

	int idxMin = boxMinorGridStyle->currentIndex();
	boxMinorGridStyle->clear();
	boxMinorGridStyle->addItem(tr("Solid"));
	boxMinorGridStyle->addItem(tr("Dash"));
	boxMinorGridStyle->addItem(tr("Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot"));
	boxMinorGridStyle->addItem(tr("Dash Dot Dot"));
	boxMinorGridStyle->setCurrentIndex(idxMin >= 0 ? idxMin : 0);

	lblResolution->setText(tr("Mesh &resolution"));
	boxShowLegend->setText(tr("Show &Legend"));
	boxSmoothMesh->setText(tr("Smoo&th Mesh"));
	boxOrthogonal->setText(tr("O&rthogonal"));
	boxAutoscale3DPlots->setText(tr("Auto&scaling"));
	boxScaleFonts3DPlots->setText(tr("&Scale Fonts"));

	lblProjection->setText(tr("Projection"));
	int idxProj = boxProjection->currentIndex();
	boxProjection->clear();
	boxProjection->addItem(tr("Perspective"));
	boxProjection->addItem(tr("Orthographic"));
	boxProjection->setCurrentIndex(idxProj >= 0 ? idxProj : 0);

	lblShadingStyle->setText(tr("Shading"));
	int idxShad = boxShading->currentIndex();
	boxShading->clear();
	boxShading->addItem(tr("Flat"));
	boxShading->addItem(tr("Gouraud"));
	boxShading->setCurrentIndex(idxShad >= 0 ? idxShad : 0);
}

void Plots3DConfigPage::pick3DTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_title_font, this);
	if (ok)
		d_3D_title_font = font;
}

void Plots3DConfigPage::pick3DNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_numbers_font, this);
	if (ok)
		d_3D_numbers_font = font;
}

void Plots3DConfigPage::pick3DAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d_3D_axes_font, this);
	if (ok)
		d_3D_axes_font = font;
}

void Plots3DConfigPage::enableMajorGrids(bool on)
{
	lblMajorGridWidth->setEnabled(on);
	boxMajorGridWidth->setEnabled(on);
	lblMajorGridStyle->setEnabled(on);
	boxMajorGridStyle->setEnabled(on);
}

void Plots3DConfigPage::enableMinorGrids(bool on)
{
	lblMinorGridWidth->setEnabled(on);
	boxMinorGridWidth->setEnabled(on);
	lblMinorGridStyle->setEnabled(on);
	boxMinorGridStyle->setEnabled(on);
}
