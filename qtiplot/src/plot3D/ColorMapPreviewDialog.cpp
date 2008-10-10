/***************************************************************************
    File                 : ColorMapPreviewDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Open file dialog providing a color map preview.

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

#include "ColorMapPreviewDialog.h"
#include "Graph3D.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ColorMapPreviewDialog::ColorMapPreviewDialog(QWidget *parent, Qt::WFlags flags)
	: ExtensibleFileDialog(parent, flags)
{
	setCaption(tr("QtiPlot - Color Map Preview Dialog"));
	setFileMode(ExistingFile);
	QStringList filters;
	filters << tr("Colormap files") + " (*.map *.MAP)" << tr("All files") + " (*)";
	setFilters(filters);

	setExtentionToggleButtonText(tr("<< &Preview"));
	setExtended(true);
	
	QWidget *advanced_options = new QWidget();
	QHBoxLayout *advanced_layout = new QHBoxLayout();
	advanced_options->setLayout(advanced_layout);
	
	d_preview_label = new QLabel(tr("None"));
	d_preview_label->setScaledContents(true);
	d_preview_label->setFrameShape( QFrame::StyledPanel );
	d_preview_label->setFrameShadow( QFrame::Sunken );
	advanced_layout->addWidget(d_preview_label);
	
	setExtensionWidget(advanced_options);
	connect(this, SIGNAL(currentChanged(const QString&)), 
			this, SLOT(updatePreview(const QString&)));
}

void ColorMapPreviewDialog::updatePreview(const QString& fileName)
{
	if (fileName.isEmpty()){
		d_preview_label->setText(tr("None"));
   		return;
	}
	
	QFileInfo fi(fileName);
	if (!fi.isFile () || !fi.isReadable ()){
		d_preview_label->setText(tr("None"));
   		return;
	}
	
	ColorVector cv;
	if (!Graph3D::openColorMapFile(cv, fileName)){
		d_preview_label->setText(tr("None"));
   		return;
	}
		
	int height = 40;
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
	d_preview_label->setPixmap(pix);
}
