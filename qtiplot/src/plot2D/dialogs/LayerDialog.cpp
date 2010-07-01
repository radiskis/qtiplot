/***************************************************************************
    File                 : LayerDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004-2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Arrange layers dialog

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
#include "LayerDialog.h"
#include <ApplicationWindow.h>
#include <DoubleSpinBox.h>

#include <QLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QFontDialog>
#include <QFont>
#include <QMessageBox>

LayerDialog::LayerDialog( QWidget* parent, bool okMode, Qt::WFlags fl )
: QDialog(parent, fl),
multi_layer(NULL)
{
	setName("LayerDialog");
	setWindowTitle(tr( "QtiPlot - Arrange Layers" ));
	setAttribute(Qt::WA_DeleteOnClose);

	QGroupBox *gb1 = new QGroupBox(tr("Layers"));
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Number")), 0, 0);
	layersBox = new QSpinBox();
	layersBox->setRange(0, 100);
	gl1->addWidget(layersBox, 0, 1);

	linkXAxesBox = new QCheckBox(tr("Link &X axes"));
	linkXAxesBox->setChecked(false);
	gl1->addWidget(linkXAxesBox, 1, 0);

	fitBox = new QCheckBox(tr("Automatic &layout"));
	fitBox->setChecked(false);
	gl1->addWidget(fitBox, 1, 1);
	gl1->setRowStretch(2, 1);

    QGroupBox *gb2 = new QGroupBox(tr("Alignment"));
	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->addWidget(new QLabel(tr("Horizontal")), 0, 0);

	alignHorBox = new QComboBox( false );
	alignHorBox->insertItem( tr( "Center" ) );
	alignHorBox->insertItem( tr( "Left" ) );
	alignHorBox->insertItem( tr( "Right" ) );
	gl2->addWidget(alignHorBox, 0, 1);

	gl2->addWidget(new QLabel( tr( "Vertical" )), 1, 0 );
	alignVertBox = new QComboBox( false );
	alignVertBox->insertItem( tr( "Center" ) );
	alignVertBox->insertItem( tr( "Top" ) );
	alignVertBox->insertItem( tr( "Bottom" ) );
	gl2->addWidget(alignVertBox, 1, 1);
	gl2->setRowStretch(2, 1);

    GroupGrid = new QGroupBox(tr("Grid"));
	QGridLayout *gl3 = new QGridLayout(GroupGrid);
	gl3->addWidget(new QLabel(tr("Columns")), 0, 0);
	boxX = new QSpinBox();
	boxX->setRange(1, 100);
	gl3->addWidget(boxX, 0, 1);
	gl3->addWidget(new QLabel( tr( "Rows" )), 1, 0);
	boxY = new QSpinBox();
	boxY->setRange(1, 100);
	gl3->addWidget(boxY, 1, 1);
	gl3->setRowStretch(2, 1);

	GroupCanvasSize = new QGroupBox(tr("&Layer Canvas Size"));
	GroupCanvasSize->setCheckable(true);
	GroupCanvasSize->setChecked(false);

	QGridLayout *gl5 = new QGridLayout(GroupCanvasSize);

	gl5->addWidget(new QLabel(tr("Unit")), 0, 0);

	unitBox = new QComboBox();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));
	gl5->addWidget(unitBox, 0, 1);

	QLocale locale = QLocale();
	gl5->addWidget(new QLabel(tr("Width")), 1, 0);
	boxCanvasWidth = new DoubleSpinBox();
	boxCanvasWidth->setRange(0, 10000);
	boxCanvasWidth->setLocale(locale);
	boxCanvasWidth->setDecimals(6);
	gl5->addWidget(boxCanvasWidth, 1, 1);

	gl5->addWidget(new QLabel( tr( "Height" )), 2, 0);
	boxCanvasHeight = new DoubleSpinBox();
	boxCanvasHeight->setRange(0, 10000);
	boxCanvasHeight->setLocale(locale);
	boxCanvasHeight->setDecimals(6);
	gl5->addWidget(boxCanvasHeight, 2, 1);

	keepRatioBox = new QCheckBox(tr("&Keep aspect ratio"));
	keepRatioBox->setChecked(((ApplicationWindow*)parent)->d_keep_aspect_ration);
	gl5->addWidget(keepRatioBox, 3, 1);

	fixedSizeBox = new QCheckBox(tr("&Fixed size"));
	gl5->addWidget(fixedSizeBox, 3, 0);

    QGroupBox *gb4 = new QGroupBox(tr("Spacing"));
	QGridLayout *gl4 = new QGridLayout(gb4);

	gl4->addWidget(new QLabel(tr("Align")), 0, 0);
	alignPolicyBox = new QComboBox();
	alignPolicyBox->addItem(tr("Layers"));
	alignPolicyBox->addItem(tr("Canvases"));
	gl4->addWidget(alignPolicyBox, 0, 1);

	gl4->addWidget(new QLabel(tr("Columns gap")), 1, 0);
	boxColsGap = new QSpinBox();
	boxColsGap->setRange(0, 1000);
	boxColsGap->setSingleStep(5);
	boxColsGap->setSuffix(tr(" pixels"));
	gl4->addWidget(boxColsGap, 1, 1);
	gl4->addWidget(new QLabel( tr( "Rows gap" )), 2, 0);
	boxRowsGap = new QSpinBox();
	boxRowsGap->setRange(0, 1000);
	boxRowsGap->setSingleStep(5);
	boxRowsGap->setSuffix(tr(" pixels"));
	gl4->addWidget(boxRowsGap, 2, 1);

	commonAxesBox = new QCheckBox(tr("Co&mmon axes"));
	commonAxesBox->setDisabled(true);
	gl4->addWidget(commonAxesBox, 3, 1);

	QGroupBox *gb7 = new QGroupBox(tr("Margins"));
	gl4 = new QGridLayout(gb7);
	gl4->addWidget(new QLabel( tr( "Left margin" )), 0, 0);
	boxLeftSpace = new QSpinBox();
	boxLeftSpace->setRange(0, 1000);
	boxLeftSpace->setSingleStep(5);
	boxLeftSpace->setSuffix(tr(" pixels"));
	gl4->addWidget(boxLeftSpace, 0, 1);
	gl4->addWidget(new QLabel( tr( "Right margin" )), 1, 0);
	boxRightSpace = new QSpinBox();
	boxRightSpace->setRange(0, 1000);
	boxRightSpace->setSingleStep(5);
	boxRightSpace->setSuffix(tr(" pixels"));
	gl4->addWidget(boxRightSpace, 1, 1);
	gl4->addWidget(new QLabel( tr( "Top margin" )), 2, 0);
	boxTopSpace = new QSpinBox();
	boxTopSpace->setRange(0, 1000);
	boxTopSpace->setSingleStep(5);
	boxTopSpace->setSuffix(tr(" pixels"));
	gl4->addWidget(boxTopSpace, 2, 1);
	gl4->addWidget(new QLabel( tr( "Bottom margin") ), 3, 0);
	boxBottomSpace = new QSpinBox();
	boxBottomSpace->setRange(0, 1000);
	boxBottomSpace->setSingleStep(5);
	boxBottomSpace->setSuffix(tr(" pixels"));
	gl4->addWidget(boxBottomSpace, 3, 1);
	gl4->setRowStretch(4, 1);

	buttonOk = new QPushButton(tr( "&OK" ));
	buttonCancel = new QPushButton(tr( "&Cancel" ));
	buttonApply = NULL;

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addStretch();

	if (!okMode){
		buttonApply = new QPushButton(tr( "&Apply" ));
		connect( buttonApply, SIGNAL( clicked() ), this, SLOT(update() ) );
		hbox1->addWidget(buttonApply);
	}

	hbox1->addWidget(buttonOk);
	hbox1->addWidget(buttonCancel);
	hbox1->addStretch();

	QGroupBox *gb5 = new QGroupBox(tr("Swap Layers"));
	QHBoxLayout *hbox2 = new QHBoxLayout(gb5);
	hbox2->addWidget(new QLabel( tr( "Source Layer") ));

	boxLayerSrc = new QSpinBox();
	hbox2->addWidget(boxLayerSrc);

	hbox2->addWidget(new QLabel( tr( "Destination Layer") ));
	boxLayerDest = new QSpinBox();
	hbox2->addWidget(boxLayerDest);

	buttonSwapLayers = new QPushButton(tr( "&Swap" ));
	hbox2->addWidget(buttonSwapLayers);

	QGridLayout *gl6 = new QGridLayout();
	gl6->addWidget(gb1, 0, 0);
	gl6->addWidget(gb2, 0, 1);
	gl6->addWidget(GroupGrid, 1, 0);
	gl6->addWidget(gb4, 1, 1);
	gl6->addWidget(GroupCanvasSize, 2, 0);
	gl6->addWidget(gb7, 2, 1);
	gl6->setRowStretch(3, 1);

	QVBoxLayout *vbox2 = new QVBoxLayout(this);
	vbox2->addLayout(gl6);
	vbox2->addWidget(gb5);
	vbox2->addStretch();
	vbox2->addLayout(hbox1);

	connect( buttonSwapLayers, SIGNAL( clicked() ), this, SLOT( swapLayers() ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( fitBox, SIGNAL( toggled(bool) ), this, SLOT(enableLayoutOptions(bool) ) );
	connect(unitBox, SIGNAL(activated(int)), this, SLOT(updateSizes(int)));
	connect(boxCanvasWidth, SIGNAL(valueChanged (double)), this, SLOT(adjustCanvasHeight(double)));
	connect(boxCanvasHeight, SIGNAL(valueChanged (double)), this, SLOT(adjustCanvasWidth(double)));

	connect(boxColsGap, SIGNAL(valueChanged(int)), this, SLOT(showCommonAxesBox()));
	connect(boxRowsGap, SIGNAL(valueChanged(int)), this, SLOT(showCommonAxesBox()));
	connect(alignPolicyBox, SIGNAL(activated(int)), this, SLOT(showCommonAxesBox()));
}

void LayerDialog::enableLayoutOptions(bool ok)
{
	GroupGrid->setEnabled(!ok);
	GroupCanvasSize->setEnabled(!ok);
}

void LayerDialog::setMultiLayer(MultiLayer *g)
{
	if (!g)
		return;

	multi_layer = g;

	layersBox->setValue(g->numLayers());
	boxX->setValue(g->getCols());
	boxY->setValue(g->getRows());

	linkXAxesBox->setChecked(g->hasLinkedXLayerAxes());
	commonAxesBox->setChecked(g->hasCommonAxes());

	alignPolicyBox->setCurrentIndex(g->alignPolicy());
	boxColsGap->setValue(g->colsSpacing());
	boxRowsGap->setValue(g->rowsSpacing());
	boxLeftSpace->setValue(g->leftMargin());
	boxRightSpace->setValue(g->rightMargin());
	boxTopSpace->setValue(g->topMargin());
	boxBottomSpace->setValue(g->bottomMargin());

	FrameWidget::Unit unit = (FrameWidget::Unit)g->applicationWindow()->d_layer_geometry_unit;
	unitBox->blockSignals(true);
	unitBox->setCurrentIndex(unit);
	unitBox->blockSignals(false);

	updateSizes(unit);

	GroupCanvasSize->setChecked(g->sizePolicy() == MultiLayer::UserSize);

	fixedSizeBox->setChecked(!g->scaleLayersOnResize());

	alignHorBox->setCurrentItem(g->horizontalAlignement());
	alignVertBox->setCurrentItem(g->verticalAlignement());

	boxLayerSrc->setRange(1, g->numLayers());
	boxLayerDest->setRange(1, g->numLayers());
	boxLayerDest->setValue(g->numLayers());

	showCommonAxesBox();
}

void LayerDialog::update()
{
	if (!multi_layer){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		multi_layer = app->multilayerPlot(1, -1, app->defaultCurveStyle, MultiLayer::AlignLayers);
		QList<Graph *> layersList = multi_layer->layersList();
		foreach(Graph *g, layersList)
			g->removeLegend();
	}

	int graphs = layersBox->value();
	int old_graphs = multi_layer->numLayers();
	int dn = multi_layer->numLayers() - graphs;
	if (dn > 0 && QMessageBox::question(0, tr("QtiPlot - Delete Layers?"),
				tr("You are about to delete %1 existing layers.").arg(dn)+"\n"+
				tr("Are you sure you want to continue this operation?"),
				tr("&Continue"), tr("&Cancel"), QString(), 0, 1 )) return;

	multi_layer->setNumLayers(graphs);

	if (!graphs)
		return;

	if (dn < 0){// Customize new layers with user default settings
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		for (int i = old_graphs+1; i <= graphs; i++)
			app->setPreferences(multi_layer->layer(i));
	}

	int cols = boxX->value();
	int rows = boxY->value();

	if (cols>graphs && !fitBox->isChecked()){
		QMessageBox::about(this, tr("QtiPlot - Columns input error"),
				tr("The number of columns you've entered is greater than the number of graphs (%1)!").arg(graphs));
		boxX->setFocus();
		return;
	}

	if (rows>graphs && !fitBox->isChecked()){
		QMessageBox::about(this, tr("QtiPlot - Rows input error"),
				tr("The number of rows you've entered is greater than the number of graphs (%1)!").arg(graphs));
		boxY->setFocus();
		return;
	}

	if (!fitBox->isChecked()){
		multi_layer->setCols(cols);
		multi_layer->setRows(rows);
	}

	FrameWidget::Unit unit = (FrameWidget::Unit)unitBox->currentIndex();
	if (GroupCanvasSize->isChecked()){
		ApplicationWindow *app = multi_layer->applicationWindow();
		if (app)
			app->d_layer_geometry_unit = unitBox->currentIndex();

		multi_layer->setLayerCanvasSize(convertToPixels(boxCanvasWidth->value(), unit, 0), convertToPixels(boxCanvasHeight->value(), unit, 1));
	}

	multi_layer->setCommonAxesLayout(commonAxesBox->isEnabled() && commonAxesBox->isChecked());
	if (commonAxesBox->isEnabled() && commonAxesBox->isChecked())
		multi_layer->setCommonLayerAxes(boxColsGap->value() == 0, boxRowsGap->value() == 0);

	multi_layer->setAlignement(alignHorBox->currentItem(), alignVertBox->currentItem());

	multi_layer->setMargins(boxLeftSpace->value(), boxRightSpace->value(),
			boxTopSpace->value(), boxBottomSpace->value());

	multi_layer->setScaleLayersOnResize(!fixedSizeBox->isChecked());
	multi_layer->linkXLayerAxes(linkXAxesBox->isChecked());

	multi_layer->setAlignPolicy((MultiLayer::AlignPolicy)alignPolicyBox->currentIndex());
	multi_layer->setSpacing(boxRowsGap->value(), boxColsGap->value());
	multi_layer->arrangeLayers(fitBox->isChecked(), GroupCanvasSize->isChecked());
	if (!buttonApply){
		QList<Graph *> layersList = multi_layer->layersList();
		foreach(Graph *g, layersList)
			g->newLegend();
	}

	if (!GroupCanvasSize->isChecked()){//show new layer canvas size
		boxCanvasWidth->blockSignals(true);
		boxCanvasWidth->setValue(convertFromPixels(multi_layer->layerCanvasSize().width(), unit, 0));
		boxCanvasWidth->blockSignals(false);

		boxCanvasHeight->blockSignals(true);
		boxCanvasHeight->setValue(convertFromPixels(multi_layer->layerCanvasSize().height(), unit, 1));
		boxCanvasHeight->blockSignals(false);
	}

	if (fitBox->isChecked()){//show new grid settings
		boxX->setValue(multi_layer->getCols());
		boxY->setValue(multi_layer->getRows());
	}
}

void LayerDialog::accept()
{
	update();
	close();
}

void LayerDialog::swapLayers()
{
	if(boxLayerSrc->value() == boxLayerDest->value()){
		QMessageBox::warning(this, tr("QtiPlot - Error"),
		tr("Please enter different indexes for the source and destination layers!"));
		return;
	}

	multi_layer->swapLayers(boxLayerSrc->value(), boxLayerDest->value());
}

int LayerDialog::convertToPixels(double w, FrameWidget::Unit unit, int dimension)
{
	if (!multi_layer)
		return qRound(w);

	double dpi = (double)multi_layer->logicalDpiX();
	if (dimension)
		dpi = (double)multi_layer->logicalDpiY();

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

double LayerDialog::convertFromPixels(int w, FrameWidget::Unit unit, int dimension)
{
	if (!multi_layer)
		return w;

	double dpi = (double)multi_layer->logicalDpiX();
	if (dimension)
		dpi = (double)multi_layer->logicalDpiY();

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

void LayerDialog::updateSizes(int unit)
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

	if (!multi_layer)
		return;

	aspect_ratio = (double)multi_layer->layerCanvasSize().width()/(double)multi_layer->layerCanvasSize().height();

	boxCanvasWidth->setValue(convertFromPixels(multi_layer->layerCanvasSize().width(), (FrameWidget::Unit)unit, 0));
	boxCanvasHeight->setValue(convertFromPixels(multi_layer->layerCanvasSize().height(), (FrameWidget::Unit)unit, 1));
}

void LayerDialog::adjustCanvasHeight(double width)
{
	if (keepRatioBox->isChecked()){
		boxCanvasHeight->blockSignals(true);
		boxCanvasHeight->setValue(width/aspect_ratio);
		boxCanvasHeight->blockSignals(false);
	} else
		aspect_ratio = width/boxCanvasHeight->value();
}

void LayerDialog::adjustCanvasWidth(double height)
{
	if (keepRatioBox->isChecked()){
		boxCanvasWidth->blockSignals(true);
		boxCanvasWidth->setValue(height*aspect_ratio);
		boxCanvasWidth->blockSignals(false);
	} else
		aspect_ratio = boxCanvasWidth->value()/height;
}

void LayerDialog::showCommonAxesBox()
{
	commonAxesBox->setEnabled(alignPolicyBox->currentIndex() == MultiLayer::AlignCanvases &&
							  (boxColsGap->value() == 0 || boxRowsGap->value() == 0));
}

void LayerDialog::setLayers(int layers)
{
	layersBox->setValue(layers);
}

void LayerDialog::setLayerCanvasSize(int w, int h, int unit)
{
	boxCanvasWidth->blockSignals(true);
	boxCanvasWidth->setValue(convertFromPixels(w, (FrameWidget::Unit)unit, 0));
	boxCanvasWidth->blockSignals(false);
	boxCanvasHeight->blockSignals(true);
	boxCanvasHeight->setValue(convertFromPixels(h, (FrameWidget::Unit)unit, 1));
	boxCanvasHeight->blockSignals(false);
	unitBox->blockSignals(true);
	unitBox->setCurrentIndex(unit);
	unitBox->blockSignals(false);
	aspect_ratio = boxCanvasWidth->value()/boxCanvasHeight->value();
	GroupCanvasSize->setChecked(true);
}

void LayerDialog::setRows(int r)
{
	boxY->setValue(r);
}

void LayerDialog::setColumns(int c)
{
	boxX->setValue(c);
}

void LayerDialog::setMargins(int l, int t, int r, int b)
{
	boxLeftSpace->setValue(l);
	boxRightSpace->setValue(r);
	boxTopSpace->setValue(t);
	boxBottomSpace->setValue(b);
}

void LayerDialog::setSharedAxes(bool on)
{
	alignPolicyBox->setCurrentIndex(MultiLayer::AlignCanvases);
	commonAxesBox->setChecked(on);
	commonAxesBox->setEnabled(true);
	boxColsGap->setValue(0);
	boxRowsGap->setValue(0);
}

void LayerDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (app)
		app->d_keep_aspect_ration = keepRatioBox->isChecked();

	e->accept();
}
