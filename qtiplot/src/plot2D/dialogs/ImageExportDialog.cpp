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
#include "../../ApplicationWindow.h"

#include <QStackedWidget>
#include <QImageWriter>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QPrinter>
#include <QLabel>
#include <QComboBox>

ImageExportDialog::ImageExportDialog(QWidget * parent, bool vector_options, bool extended, Qt::WFlags flags)
	: ExtensibleFileDialog( parent, extended, flags )
{
	setWindowTitle( tr( "QtiPlot - Choose a filename to save under" ) );
	setAcceptMode(QFileDialog::AcceptSave);

	QList<QByteArray> list = QImageWriter::supportedImageFormats();
	list<<"EPS";
	list<<"PS";
	list<<"PDF";
	#if QT_VERSION >= 0x040300
		list<<"SVG";
	#endif

    #if EMF_OUTPUT
		list<<"EMF";
	#endif

	QStringList filters;
	for(int i=0 ; i<list.count() ; i++)
		filters << "*."+list[i].toLower();

	filters.sort();
	setFilters(filters);
	setFileMode( QFileDialog::AnyFile );

	initAdvancedOptions();
	d_vector_options->setEnabled(vector_options);
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
	d_advanced_options = new QStackedWidget();

	d_vector_options = new QGroupBox();
	QGridLayout *vector_layout = new QGridLayout(d_vector_options);
	d_advanced_options->addWidget(d_vector_options);

    QLabel *resLabel = new QLabel(tr("Resolution (DPI)"));
	vector_layout->addWidget(resLabel, 1, 0);
	resLabel->hide();
	d_resolution = new QSpinBox();
	d_resolution->setRange(0, 1000);
	//d_resolution->setValue(app->d_export_resolution);
	//only printing with screen resolution works correctly for the moment
	d_resolution->setValue(QPrinter().resolution());
	vector_layout->addWidget(d_resolution, 1, 1);
	d_resolution->hide();

	d_color = new QCheckBox();
	d_color->setText(tr("&Export in &color"));
	d_color->setChecked(app->d_export_color);
	vector_layout->addWidget(d_color, 2, 0, 1, 2);

	d_raster_options = new QGroupBox();
	QGridLayout *raster_layout = new QGridLayout(d_raster_options);
	d_advanced_options->addWidget(d_raster_options);

	raster_layout->addWidget(new QLabel(tr("Image quality")), 1, 0);
	d_quality = new QSpinBox();
	d_quality->setRange(1, 100);
	d_quality->setValue(app->d_export_quality);
	raster_layout->addWidget(d_quality, 1, 1);

	d_transparency = new QCheckBox();
	d_transparency->setText(tr("Save transparency"));
	d_transparency->setChecked(app->d_export_transparency);
	raster_layout->addWidget(d_transparency, 2, 0, 1, 2);
}

void ImageExportDialog::updateAdvancedOptions (const QString & filter)
{
	if (filter.contains("*.svg") || filter.contains("*.emf")) {
		d_extension_toggle->setChecked(false);
		d_extension_toggle->setEnabled(false);
		return;
	}
	d_extension_toggle->setEnabled(true);
	if (filter.contains("*.eps") || filter.contains("*.ps") || filter.contains("*.pdf"))
		d_advanced_options->setCurrentIndex(0);
	else {
		d_advanced_options->setCurrentIndex(1);
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

        app->d_export_resolution = d_resolution->value();
        app->d_export_color = d_color->isChecked();
	}

	e->accept();
}

void ImageExportDialog::selectFilter(const QString & filter)
{
	QFileDialog::selectFilter(filter);
	updateAdvancedOptions(filter);
}
