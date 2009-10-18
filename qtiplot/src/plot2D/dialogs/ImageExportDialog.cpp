/***************************************************************************
    File                 : ImageExportDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : QFileDialog extended with options for image export

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
#include "ImageExportDialog.h"
#include <ApplicationWindow.h>
#include <MdiSubWindow.h>
#include <Graph3D.h>
#include <MultiLayer.h>
#include <DoubleSpinBox.h>

#include <QStackedWidget>
#include <QImageWriter>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QPrinter>
#include <QLabel>

ImageExportDialog::ImageExportDialog(MdiSubWindow *window, QWidget * parent, bool extended, Graph *g, Qt::WFlags flags)
	: ExtensibleFileDialog( parent, extended, flags ), d_window(window), d_layer(g)
{
	setWindowTitle( tr( "QtiPlot - Choose a filename to save under" ) );
	setAcceptMode(QFileDialog::AcceptSave);

	QList<QByteArray> list = QImageWriter::supportedImageFormats();
	list << "EPS";
	list << "ODF";
	list << "PS";
	list << "PDF";
	list << "SVG";

    #if EMF_OUTPUT
    if (!qobject_cast<Graph3D *> (d_window))
		list << "EMF";
	#endif

	if (qobject_cast<MultiLayer *> (d_window))
        list << "TEX";
	if (qobject_cast<Graph3D *> (d_window))
        list << "PGF";

	QStringList filters;
	for(int i=0 ; i<list.count() ; i++)
		filters << "*."+list[i].toLower();

	filters.sort();
	setFilters(filters);
	setFileMode( QFileDialog::AnyFile );

	initAdvancedOptions();
	setExtensionWidget(d_advanced_options);

#if QT_VERSION >= 0x040300
	connect(this, SIGNAL(filterSelected ( const QString & )),
			this, SLOT(updateAdvancedOptions ( const QString & )));
#else
	QList<QComboBox*> combo_boxes = findChildren<QComboBox*>();
	if (combo_boxes.size() >= 2)
		connect(combo_boxes[1], SIGNAL(currentIndexChanged ( const QString & )),
				this, SLOT(updateAdvancedOptions ( const QString & )));
#endif
	updateAdvancedOptions(selectedFilter());
}

void ImageExportDialog::initAdvancedOptions()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	d_advanced_options = new QWidget();
	QVBoxLayout *vert_layout = new QVBoxLayout(d_advanced_options);

	d_vector_options = new QGroupBox();
	QGridLayout *vector_layout = new QGridLayout(d_vector_options);

	resolutionLabel = new QLabel(tr("Resolution (DPI)"));
	vector_layout->addWidget(resolutionLabel, 1, 0);

	d_vector_resolution = new QSpinBox();
	d_vector_resolution->setRange(0, 10000);
	d_vector_resolution->setValue(app->d_export_vector_resolution);
	vector_layout->addWidget(d_vector_resolution, 1, 1);

	d_color = new QCheckBox();
	d_color->setText(tr("Export in &color"));
	d_color->setChecked(app->d_export_color);
	vector_layout->addWidget(d_color, 2, 1);

	d_escape_tex_strings = new QCheckBox();
	d_escape_tex_strings->setText(tr("&Escape special characters in title/axis labels"));
	d_escape_tex_strings->setChecked(app->d_export_escape_tex_strings);
	vector_layout->addWidget(d_escape_tex_strings, 3, 1);
	d_escape_tex_strings->hide();

	d_tex_font_sizes = new QCheckBox();
	d_tex_font_sizes->setText(tr("Export &font sizes"));
	d_tex_font_sizes->setChecked(app->d_export_tex_font_sizes);
	vector_layout->addWidget(d_tex_font_sizes, 4, 1);
	d_tex_font_sizes->hide();

	QLabel *text3DLabel = new QLabel(tr("Export 3D texts as"));
	vector_layout->addWidget(text3DLabel, 4, 0);

	d_3D_text_export_mode = new QComboBox();
	d_3D_text_export_mode->addItem(tr("Bitmap images"));
	d_3D_text_export_mode->addItem(tr("Native fonts"));
	d_3D_text_export_mode->addItem(tr("LaTeX file"));
	d_3D_text_export_mode->setCurrentIndex(app->d_3D_export_text_mode);
	vector_layout->addWidget(d_3D_text_export_mode, 4, 1);

	QLabel *sort3DLabel = new QLabel(tr("3D Sort mode"));
	vector_layout->addWidget(sort3DLabel, 5, 0);
	d_3D_export_sort = new QComboBox();
	d_3D_export_sort->addItem(tr("No sort"));
	d_3D_export_sort->addItem(tr("Simple sort"));
	d_3D_export_sort->addItem(tr("BSP sort"));
	d_3D_export_sort->setCurrentIndex(app->d_3D_export_sort);
	vector_layout->addWidget(d_3D_export_sort, 5, 1);

	d_vector_options->hide();
	vert_layout->addWidget(d_vector_options);

	d_raster_options = new QGroupBox();
	QGridLayout *raster_layout = new QGridLayout(d_raster_options);

	raster_layout->addWidget(new QLabel(tr("Image quality")), 1, 0);
	d_quality = new QSpinBox();
	d_quality->setRange(1, 100);
	d_quality->setValue(app->d_export_quality);
	raster_layout->addWidget(d_quality, 1, 1);

	QLabel *rasterResLabel = new QLabel(tr("Print Resolution (DPI)"));
	raster_layout->addWidget(rasterResLabel, 2, 0);

	d_bitmap_resolution = new QSpinBox();
	d_bitmap_resolution->setRange(0, 10000);
	d_bitmap_resolution->setValue(app->d_export_bitmap_resolution);
	raster_layout->addWidget(d_bitmap_resolution, 2, 1);

	d_transparency = new QCheckBox();
	d_transparency->setText(tr("Save transparency"));
	d_transparency->setChecked(app->d_export_transparency);
	raster_layout->addWidget(d_transparency, 3, 1, 1, 2);

	d_raster_options->hide();
	vert_layout->addWidget(d_raster_options);

	QSizeF customSize = app->d_export_raster_size;

	d_custom_size_box = new QGroupBox(tr("Custom print size"));
	d_custom_size_box->setCheckable(true);
	d_custom_size_box->setChecked(customSize.isValid());
	QGridLayout *size_layout = new QGridLayout(d_custom_size_box);

	unitBox = new QComboBox();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));
	unitBox->setCurrentIndex(app->d_export_size_unit);

	size_layout->addWidget(new QLabel(tr( "Unit" )), 0, 0);
	size_layout->addWidget(unitBox, 0, 1);

	if (!customSize.isValid()){
		if (d_layer)
			customSize = d_layer->size();
		else if (d_window){
			if (qobject_cast<MultiLayer *> (d_window))
				customSize = ((MultiLayer *)d_window)->canvas()->size();
			else
				customSize = d_window->widget()->size();
		}
	}

	if (d_layer)
		aspect_ratio = (double)d_layer->width()/(double)d_layer->height();
	else if (d_window){
		MultiLayer *ml = qobject_cast<MultiLayer *> (d_window);
		if (ml)
			aspect_ratio = (double)ml->canvas()->width()/(double)ml->canvas()->height();
		else
			aspect_ratio = (double)d_window->widget()->width()/(double)d_window->widget()->height();
	}

	size_layout->addWidget(new QLabel( tr("Width")), 1, 0);
	widthBox = new DoubleSpinBox();
	widthBox->setLocale(app->locale());
	widthBox->setDecimals(6);
	widthBox->setValue(customSize.width());

	size_layout->addWidget(widthBox, 1, 1);

	size_layout->addWidget(new QLabel(tr("Height")), 2, 0);
	heightBox = new DoubleSpinBox();
	heightBox->setLocale(app->locale());
	heightBox->setDecimals(6);
	heightBox->setValue(customSize.height());
	size_layout->addWidget(heightBox, 2, 1);

	connect(widthBox, SIGNAL(valueChanged (double)), this, SLOT(adjustHeight(double)));
	connect(heightBox, SIGNAL(valueChanged (double)), this, SLOT(adjustWidth(double)));

	size_layout->addWidget(new QLabel(tr("Scale Fonts Factor")), 3, 0);
	scaleFontsBox = new DoubleSpinBox();
	scaleFontsBox->setMinimum (0.0);
	scaleFontsBox->setSpecialValueText (tr("Automatic"));
	scaleFontsBox->setLocale(app->locale());
	scaleFontsBox->setFormat('f', 1);
	if (customSize.isValid())
		scaleFontsBox->setValue(app->d_scale_fonts_factor);
	else
		scaleFontsBox->setValue(0.0);
	size_layout->addWidget(scaleFontsBox, 3, 1);

	keepRatioBox = new QCheckBox(tr("&Keep aspect ratio"));
	keepRatioBox->setChecked(true);
    size_layout->addWidget(keepRatioBox, 4, 1);

	vert_layout->addWidget(d_custom_size_box);

	if (!d_window)
		return;

	if (qobject_cast<Graph3D *> (d_window)){
		resolutionLabel->hide();
		d_vector_resolution->hide();
		d_color->hide();
		d_escape_tex_strings->hide();
		d_tex_font_sizes->hide();
	} else {
		if (qobject_cast<Matrix *> (d_window))
			d_custom_size_box->hide();
    	text3DLabel->hide();
		d_3D_text_export_mode->hide();
		sort3DLabel->hide();
		d_3D_export_sort->hide();
	}
}

void ImageExportDialog::updateAdvancedOptions (const QString & filter)
{
	d_vector_options->hide();
	d_raster_options->hide();
	d_custom_size_box->hide();

#if EMF_OUTPUT
	if (filter.contains("*.emf") && !qobject_cast<MultiLayer *> (d_window)){
		d_extension_toggle->setChecked(false);
		d_extension_toggle->setEnabled(false);
		return;
	}
#endif

	if (filter.contains("*.svg")){
		if (qobject_cast<Graph3D *> (d_window)){
			d_extension_toggle->setEnabled(true);
			d_vector_options->show();
			d_custom_size_box->show();
			return;
		} else if (!qobject_cast<MultiLayer *> (d_window)){
			d_extension_toggle->setChecked(false);
			d_extension_toggle->setEnabled(false);
			return;
		}
	}

	if (filter.contains("*.pgf") && qobject_cast<Graph3D *> (d_window)){
		d_vector_options->show();
		return;
	}

	d_extension_toggle->setEnabled(true);
	if (filter.contains("*.eps") || filter.contains("*.emf") ||
		filter.contains("*.ps") || filter.contains("*.pdf") ||
		filter.contains("*.svg") || filter.contains("*.tex")){
		d_vector_options->show();
		if (qobject_cast<MultiLayer *> (d_window)){
			d_custom_size_box->show();
			d_vector_options->setVisible(!filter.contains("*.svg") && !filter.contains("*.emf"));

			bool texOutput = filter.contains("*.tex");
			d_vector_resolution->setVisible(!texOutput);
			resolutionLabel->setVisible(!texOutput);

			d_escape_tex_strings->setVisible(texOutput);
			d_tex_font_sizes->setVisible(texOutput);
		} else if (qobject_cast<Graph3D *> (d_window))
			d_custom_size_box->show();
	} else {
		d_raster_options->show();
		if (!qobject_cast<Matrix *> (d_window))
			d_custom_size_box->show();
		d_transparency->setEnabled(filter.contains("*.tif") || filter.contains("*.tiff") || filter.contains("*.png") || filter.contains("*.xpm"));
	}
}

void ImageExportDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (app){
		app->d_extended_export_dialog = this->isExtended();
		app->d_image_export_filter = this->selectedFilter();
		app->d_export_transparency = d_transparency->isChecked();
        app->d_export_quality = d_quality->value();
        app->d_export_bitmap_resolution = d_bitmap_resolution->value();
		app->d_export_size_unit = unitBox->currentIndex();
        app->d_export_vector_resolution = d_vector_resolution->value();
        app->d_export_color = d_color->isChecked();
        app->d_export_escape_tex_strings = d_escape_tex_strings->isChecked();
        app->d_export_tex_font_sizes = d_tex_font_sizes->isChecked();
        app->d_export_raster_size = customExportSize();
        app->d_scale_fonts_factor = scaleFontsBox->value();

		app->d_3D_export_text_mode = d_3D_text_export_mode->currentIndex();
		app->d_3D_export_sort = d_3D_export_sort->currentIndex();
	}

	e->accept();
}

void ImageExportDialog::selectFilter(const QString & filter)
{
	QFileDialog::selectFilter(filter);
	updateAdvancedOptions(filter);
}

QSizeF ImageExportDialog::customExportSize()
{
	if (!d_custom_size_box->isChecked())
		return QSizeF();

	return QSizeF(widthBox->value(), heightBox->value());
}

bool ImageExportDialog::transparency() const
{
	if (d_transparency->isEnabled())
		return d_transparency->isChecked();

	return false;
}

double ImageExportDialog::scaleFontsFactor()
{
	if (!d_custom_size_box->isChecked())
		return 1.0;

	return scaleFontsBox->value();
}

void ImageExportDialog::adjustWidth(double height)
{
	if (keepRatioBox->isChecked()){
		widthBox->blockSignals(true);
		double val = height*aspect_ratio;
		if (unitBox->currentIndex() >= 3)// point and pixel
			widthBox->setValue(qRound(val));
		else
			widthBox->setValue(val);
		widthBox->blockSignals(false);
	}
}

void ImageExportDialog::adjustHeight(double width)
{
	if (keepRatioBox->isChecked()){
		heightBox->blockSignals(true);
		double val = width/aspect_ratio;
		if (unitBox->currentIndex() >= 3)// point and pixel
			heightBox->setValue(qRound(val));
		else
			heightBox->setValue(val);
		heightBox->blockSignals(false);
	}
}
