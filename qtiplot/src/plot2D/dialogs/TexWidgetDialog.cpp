/***************************************************************************
    File                 : TexWidgetDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A dialog for the TextWidget, based on article 
						  "Using a Simple Web Service with Qt" in Qt Quaterly, Issue 23, Q3 2007

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

#include <QtGui>
#include <QHttp>
#include <QIODevice>

#include "TexWidgetDialog.h"
#include "../Graph.h"
#include "../TexWidget.h"

TexWidgetDialog::TexWidgetDialog(Graph *g, QWidget *parent)
    : QDialog(parent), d_plot(g), d_tex_widget(NULL)
{
	setWindowTitle(tr("QtiPlot") + " - " + tr("Tex Equation Editor"));
	
    http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(updateForm(bool)));
	
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    clearButton = buttonBox->addButton(tr("Clea&r"), QDialogButtonBox::ResetRole);
	addButton = buttonBox->addButton(tr("&Add"), QDialogButtonBox::AcceptRole);
	addButton->setEnabled(false);
		
    updateButton = buttonBox->addButton(tr("&Update"), QDialogButtonBox::ApplyRole);
	cancelButton = buttonBox->addButton(tr("&Cancel"), QDialogButtonBox::RejectRole);
	
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearForm()));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addImage()));		
    connect(updateButton, SIGNAL(clicked()), this, SLOT(apply()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	tabWidget = new QTabWidget();
	initEditorPage();
	initGeometryPage();
	
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(4);
    layout->setSpacing(4);

	layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void TexWidgetDialog::initEditorPage()
{
	editPage = new QWidget();
	
	outputLabel = new QLabel;
    outputLabel->setFrameShape(QFrame::StyledPanel);
    equationEditor = new QTextEdit;

	QVBoxLayout *layout = new QVBoxLayout(editPage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(outputLabel, 1);
    layout->addWidget(equationEditor);
	
	tabWidget->addTab(editPage, tr( "&Text" ) );
}

void TexWidgetDialog::initGeometryPage()
{
    geometryPage = new QWidget();
    
	frameBox = new QComboBox();
	frameBox->addItem( tr( "None" ) );
	frameBox->addItem( tr( "Rectangle" ) );
	frameBox->addItem( tr( "Shadow" ) );

	QHBoxLayout *layout = new QHBoxLayout(geometryPage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(new QLabel(tr("Frame")));
    layout->addWidget(frameBox, 1);
	
	tabWidget->addTab(geometryPage, tr( "&Frame/Geometry" ) );
}

void TexWidgetDialog::setTexWidget(TexWidget *tw)
{
	if (!tw)
		return;
	
	d_tex_widget = tw;
	
	equationEditor->setText(tw->formula());
	outputLabel->setPixmap(tw->pixmap());
	frameBox->setCurrentIndex(tw->frameStyle());
}

void TexWidgetDialog::clearForm()
{
    outputLabel->setPixmap(QPixmap());
    equationEditor->clear();
}

void TexWidgetDialog::apply()
{
	if (tabWidget->currentPage() == editPage)
		fetchImage();
	else if (tabWidget->currentPage() == geometryPage){
		if (d_tex_widget)
			d_tex_widget->setFrameStyle(frameBox->currentIndex());
	}
}

void TexWidgetDialog::fetchImage()
{
    clearButton->setEnabled(false);
    updateButton->setEnabled(false);
    equationEditor->setReadOnly(true);

    QUrl url;
    url.setPath("/cgi-bin/mathtran");
    url.setQueryDelimiters('=', ';');
    url.addQueryItem("D", "3");
    url.addQueryItem("tex", QUrl::toPercentEncoding(
                     equationEditor->toPlainText()));

    http->setHost("mathtran.org");
    http->get(url.toString());
	
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void TexWidgetDialog::updateForm(bool error)
{
	QApplication::restoreOverrideCursor();
	
    if (!error) {
        QImage image;
        if (image.loadFromData(http->readAll())) {
            QPixmap pixmap = QPixmap::fromImage(image);
            outputLabel->setPixmap(pixmap);
			if (d_tex_widget){
				d_tex_widget->setPixmap(pixmap);
				d_tex_widget->setFormula(equationEditor->toPlainText());
			}	
        }
    } else {
		QMessageBox::critical((QWidget *)parent(), tr("QtiPlot") + " - " + tr("Network connection error"), 
		tr("Error while trying to connect to host %1:").arg("mathtran.org") + "\n\n'" +
		http->errorString() + "'\n\n" + tr("Please verify your network connection!"));
	}

	addButton->setEnabled(!error);
	
    clearButton->setEnabled(true);
    updateButton->setEnabled(true);
    equationEditor->setReadOnly(false);
}

/*void TexWidgetDialog::saveImage()
{
	QString fn = QFileDialog::getSaveFileName(this, tr("Save Image to File"), "");
	
	QFile f(fn);
	if (!f.open(QIODevice::WriteOnly)){
		QMessageBox::critical(0, tr("QtiPlot - File Save Error"),
					tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fn));
		return;
	}
	
	outputLabel->pixmap()->save(fn, 0, 100);
}*/

void TexWidgetDialog::addImage()
{
	const QPixmap *pix = outputLabel->pixmap();
	if (!pix){
		fetchImage();
		pix = outputLabel->pixmap();
	}
	
	if (!pix)
		return;
	
	d_tex_widget = d_plot->addTexFormula(equationEditor->text(), *pix);
}
