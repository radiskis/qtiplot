/***************************************************************************
    File                 : ImageExportDialog.h
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
#ifndef IMAGEEXPORTDIALOG_H
#define IMAGEEXPORTDIALOG_H

#include <ExtensibleFileDialog.h>

#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPrinter>

class QLabel;
class QStackedWidget;
class QGroupBox;
class MdiSubWindow;
class DoubleSpinBox;
class Graph;

//! QFileDialog extended with options for image export
class ImageExportDialog: public ExtensibleFileDialog
{
	Q_OBJECT

private:
	//! Create #d_advanced_options and everything it contains.
	void initAdvancedOptions();

	//! Pointer to the window to be exported.
	MdiSubWindow *d_window;
	//! Container widget for all advanced options.
	QWidget *d_advanced_options;
	// vector format options
	//! Container widget for all options available for vector formats.
	QGroupBox *d_vector_options;
	QSpinBox *d_vector_resolution;
	QCheckBox *d_color;
	// raster format options
	//! Container widget for all options available for raster formats.
	QGroupBox *d_raster_options;
	QSpinBox *d_quality;
	QCheckBox *d_transparency;
	QSpinBox *d_bitmap_resolution;
	QComboBox *d_compression;

	QComboBox *d_3D_text_export_mode;
    QComboBox *d_3D_export_sort;

    //! Container widget for user defined export size.
	QGroupBox *d_custom_size_box;
	DoubleSpinBox *widthBox, *heightBox, *scaleFontsBox;
	QComboBox *unitBox;
	QLabel *resolutionLabel, *compressionLabel;
	QCheckBox *keepRatioBox;
	QCheckBox *d_escape_tex_strings;
	QCheckBox *d_tex_font_sizes;

	QPushButton *d_preview_button;

	double aspect_ratio;
	Graph *d_layer;

public:
	//! Constructor
	/**
	 * \param window window to be exported
	 * \param parent parent widget
	 * \param vector_options whether advanced options are to be provided for export to vector formats
	 * \param extended flag: show/hide the advanced options on start-up
	 * \param flags window flags
	 */
	ImageExportDialog(MdiSubWindow *window, QWidget * parent = 0, bool extended = true, Graph *g = 0, Qt::WFlags flags = 0 );
	//! For vector formats: returns the output resolution the user selected, defaulting to the screen resolution.
	int vectorResolution() const { return d_vector_resolution->value(); };
	//! For vector formats: returns whether colors should be enabled for ouput (default: true).
	bool color() const { return d_color->isChecked(); };
	//! For TeX format: returns whether special characters should be escaped for ouput (default: true).
	bool escapeStrings(){return d_escape_tex_strings->isChecked();};
	//! For TeX format: returns whether font sizes should be exported (default: true).
	bool exportFontSizes(){return d_tex_font_sizes->isChecked();};

	//! For raster formats: returns the output resolution the user selected, defaulting to the screen resolution.
	int bitmapResolution() const { return d_bitmap_resolution->value(); };

	//! For raster formats supporting this option returns the output image compression, 0 means no compression.
	int compression() const {return d_compression->currentIndex();};

	//! Return the quality (in percent) the user selected for export to raster formats.
	int quality() const { return d_quality->value(); };
	//! Return whether the output's background should be transparent.
	bool transparency() const;
	//! Return the text export mode for 3D plots.
	int textExportMode() const {return d_3D_text_export_mode->currentIndex();};
	//! Return the sort export mode for 3D plots.
	int sortMode() const {return d_3D_export_sort->currentIndex();};

	void selectFilter(const QString & filter);
	//! Return the custom export size.
	QSizeF customExportSize();
	//! Return the unit used for the custom export size
	int sizeUnit(){return unitBox->currentIndex();};
	//! Return the scale fonts factor
	double scaleFontsFactor();

public slots:
	void accept();

protected slots:
	void closeEvent(QCloseEvent*);
	//! Update which options are visible and enabled based on the output format.
	void updateAdvancedOptions (const QString &filter);
	void adjustWidth(double height);
	void adjustHeight(double width);
	void preview();
	void drawPreview(QPrinter *);
	void drawVectorPreview(QPrinter *);
};

#endif
