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
#include "../FrameWidget.h"
#include "../ImageWidget.h"
#include "../../ColorButton.h"
#include "../../ApplicationWindow.h"

static const char* choose_folder_xpm[]={
    "16 16 11 1",
    "# c #000000",
    "g c #c0c0c0",
    "e c #303030",
    "a c #ffa858",
    "b c #808080",
    "d c #a0a0a4",
    "f c #585858",
    "c c #ffdca8",
    "h c #dcdcdc",
    "i c #ffffff",
    ". c None",
    "....###.........",
    "....#ab##.......",
    "....#acab####...",
    "###.#acccccca#..",
    "#ddefaaaccccca#.",
    "#bdddbaaaacccab#",
    ".eddddbbaaaacab#",
    ".#bddggdbbaaaab#",
    "..edgdggggbbaab#",
    "..#bgggghghdaab#",
    "...ebhggghicfab#",
    "....#edhhiiidab#",
    "......#egiiicfb#",
    "........#egiibb#",
    "..........#egib#",
    "............#ee#"};
	
TexWidgetDialog::TexWidgetDialog(WidgetType wt, Graph *g, QWidget *parent)
    : QDialog(parent), d_plot(g), d_widget(NULL), d_widget_type(wt)
{	
    http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(updateForm(bool)));
	
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
	clearButton = NULL;
	editPage = NULL;
	imagePage = NULL;
	if (wt == Tex){
		setWindowTitle(tr("QtiPlot") + " - " + tr("Tex Equation Editor"));
		
    	clearButton = buttonBox->addButton(tr("Clea&r"), QDialogButtonBox::ResetRole);
		connect(clearButton, SIGNAL(clicked()), this, SLOT(clearForm()));
		
		addButton = buttonBox->addButton(tr("&Add"), QDialogButtonBox::AcceptRole);
		addButton->setEnabled(false);
		connect(addButton, SIGNAL(clicked()), this, SLOT(addImage()));	
		
		updateButton = buttonBox->addButton(tr("&Update"), QDialogButtonBox::ApplyRole);
	} else {
		setWindowTitle(tr("QtiPlot") + " - " + tr("Properties Editor"));
    	updateButton = buttonBox->addButton(tr("&Apply"), QDialogButtonBox::ApplyRole);
	}
	
	connect(updateButton, SIGNAL(clicked()), this, SLOT(apply()));
	
	cancelButton = buttonBox->addButton(tr("&Cancel"), QDialogButtonBox::RejectRole);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	tabWidget = new QTabWidget();
	if (wt == Tex)
		initEditorPage();
	else if (wt == Image)
		initImagePage();
	
	initFramePage();
	initGeometryPage();
	
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(4);
    layout->setSpacing(4);

	layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);
	
	connect(tabWidget, SIGNAL(currentChanged (QWidget *)), 
			this, SLOT(customButtons(QWidget *)));
}

void TexWidgetDialog::initEditorPage()
{
	editPage = new QWidget();
	
    equationEditor = new QTextEdit;
	outputLabel = new QLabel;
    outputLabel->setFrameShape(QFrame::StyledPanel);
	
	QVBoxLayout *layout = new QVBoxLayout(editPage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(equationEditor, 1);
	layout->addWidget(new QLabel(tr("Preview:")));
	layout->addWidget(outputLabel);
	
	tabWidget->addTab(editPage, tr( "&Text" ) );
}

void TexWidgetDialog::initImagePage()
{
	imagePage = new QWidget();

    QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("File")), 0, 0);
	
	imagePathBox = new QLineEdit();
	gl->addWidget(imagePathBox, 0, 1);
	
	QPushButton *browseBtn = new QPushButton();
	connect(browseBtn, SIGNAL(clicked()), this, SLOT(chooseImageFile()));
	browseBtn->setIcon(QIcon(QPixmap(choose_folder_xpm)));
	gl->addWidget(browseBtn, 0, 2);
	
	boxSaveImagesInternally = new QCheckBox(tr("&Save internally"));
	gl->addWidget(boxSaveImagesInternally, 1, 1);
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(2, 1);
	
	QVBoxLayout *layout = new QVBoxLayout(imagePage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(gb);
	tabWidget->addTab(imagePage, tr( "&Image" ) );
}

void TexWidgetDialog::initFramePage()
{
    framePage = new QWidget();
    
	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("Shape")), 0, 0);
	
	frameBox = new QComboBox();
	frameBox->addItem( tr( "None" ) );
	frameBox->addItem( tr( "Rectangle" ) );
	frameBox->addItem( tr( "Shadow" ) );
    gl->addWidget(frameBox, 0, 1);
	
    gl->addWidget(new QLabel(tr("Color")), 1, 0);
	frameColorBtn = new ColorButton();
    gl->addWidget(frameColorBtn, 1, 1);
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(2, 1);

	QVBoxLayout *layout = new QVBoxLayout(framePage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(gb);
	
	tabWidget->addTab(framePage, tr( "&Frame" ) );
}

void TexWidgetDialog::initGeometryPage()
{
    geometryPage = new QWidget();
    
	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    /*gl->addWidget(new QLabel( tr("Shape")), 0, 0);
	
	frameBox = new QComboBox();
	frameBox->addItem( tr( "None" ) );
	frameBox->addItem( tr( "Rectangle" ) );
	frameBox->addItem( tr( "Shadow" ) );
    gl->addWidget(frameBox, 0, 1);
	
    gl->addWidget(new QLabel(tr("Color")), 1, 0);
	frameColorBtn = new ColorButton();
    gl->addWidget(frameColorBtn, 1, 1);
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(2, 1);*/

	QVBoxLayout *layout = new QVBoxLayout(geometryPage);
    layout->setMargin(4);
    layout->setSpacing(4);
    layout->addWidget(gb);
	
	tabWidget->addTab(geometryPage, tr( "&Geometry" ) );
}

void TexWidgetDialog::customButtons(QWidget *w)
{
	if (editPage && w == editPage){
		if (clearButton)
			clearButton->setEnabled(true);
		return;
	}
	
	if (w == framePage){
		updateButton->setEnabled(true);
		if (clearButton)
			clearButton->setEnabled(false);
		return;
	}
}

void TexWidgetDialog::setWidget(FrameWidget *w)
{
	if (!w)
		return;
	
	d_widget = w;
	
	frameBox->setCurrentIndex(w->frameStyle());
	frameColorBtn->setColor(w->frameColor());
	
	if (d_widget_type == Tex){
		TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
		if (tw){
			equationEditor->setText(tw->formula());
			outputLabel->setPixmap(tw->pixmap());
		}
		return;
	} else if (d_widget_type == Image){
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i)			
			imagePathBox->setText(i->fileName());
	}
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
	else if (tabWidget->currentPage() == framePage){
		if (d_widget){
			d_widget->setFrameStyle(frameBox->currentIndex());
			d_widget->setFrameColor(frameColorBtn->color());
			d_plot->multiLayer()->notifyChanges();
		}
	} else if (imagePage && tabWidget->currentPage() == imagePage){
		chooseImageFile(imagePathBox->text());
	}
}

void TexWidgetDialog::fetchImage()
{
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw && tw->formula() == equationEditor->toPlainText())
		return;
		
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
			TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
			if (tw){
				tw->setPixmap(pixmap);
				tw->setFormula(equationEditor->toPlainText());
				d_plot->multiLayer()->notifyChanges();
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

void TexWidgetDialog::addImage()
{
	const QPixmap *pix = outputLabel->pixmap();
	if (!pix){
		fetchImage();
		pix = outputLabel->pixmap();
	}
	
	if (!pix)
		return;
	
	d_widget = (FrameWidget *)d_plot->addTexFormula(equationEditor->text(), *pix);
}

void TexWidgetDialog::chooseImageFile(const QString& fn)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString path = fn;
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this, tr("QtiPlot - Import image from file"), app->imagesDirPath);
	
	if (!path.isEmpty()){
		QFileInfo fi(path);
		if (!fi.exists ()){
			QMessageBox::critical(this, tr("QtiPlot - File openning error"),
			tr("The file: <b>%1</b> doesn't exist!").arg(path));
			return;
		}
		if (!fi.isReadable()){
			QMessageBox::critical(this, tr("QtiPlot - File openning error"),
			tr("You don't have the permission to open this file: <b>%1</b>").arg(path));
			return;
		}
		
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i && i->setFileName(path)){			
			imagePathBox->setText(path);
			app->imagesDirPath = fi.dirPath(true);
		}
	}
}
