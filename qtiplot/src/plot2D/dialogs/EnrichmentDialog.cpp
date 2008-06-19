/***************************************************************************
    File                 : EnrichmentDialog.cpp
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

#include "EnrichmentDialog.h"
#include "../Graph.h"
#include "../TexWidget.h"
#include "../FrameWidget.h"
#include "../ImageWidget.h"
#include "../../ColorButton.h"
#include "../../ApplicationWindow.h"
#include "../../DoubleSpinBox.h"

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

EnrichmentDialog::EnrichmentDialog(WidgetType wt, Graph *g, QWidget *parent)
    : QDialog(parent), d_plot(g), d_widget(NULL), d_widget_type(wt)
{
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);
	
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
	clearButton = NULL;
	editPage = NULL;
	imagePage = NULL;
		
	if (wt == Tex){
		setWindowTitle(tr("QtiPlot") + " - " + tr("Tex Equation Editor"));

    	clearButton = buttonBox->addButton(tr("Clea&r"), QDialogButtonBox::ResetRole);
		connect(clearButton, SIGNAL(clicked()), this, SLOT(clearForm()));
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
	layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

	connect(tabWidget, SIGNAL(currentChanged (QWidget *)),
			this, SLOT(customButtons(QWidget *)));
}

void EnrichmentDialog::initEditorPage()
{
	http = new QHttp(this);
    connect(http, SIGNAL(done(bool)), this, SLOT(updateForm(bool)));

	editPage = new QWidget();

    equationEditor = new QTextEdit;
	outputLabel = new QLabel;
    outputLabel->setFrameShape(QFrame::StyledPanel);

	QVBoxLayout *layout = new QVBoxLayout(editPage);
    layout->addWidget(equationEditor, 1);
	layout->addWidget(new QLabel(tr("Preview:")));
	layout->addWidget(outputLabel);

	tabWidget->addTab(editPage, tr( "&Text" ) );
}

void EnrichmentDialog::initImagePage()
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
	connect(boxSaveImagesInternally, SIGNAL(toggled(bool)), this, SLOT(saveImagesInternally(bool)));
	
	gl->addWidget(boxSaveImagesInternally, 1, 1);
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(2, 1);

	QVBoxLayout *layout = new QVBoxLayout(imagePage);
    layout->addWidget(gb);
	tabWidget->addTab(imagePage, tr( "&Image" ) );
}

void EnrichmentDialog::initFramePage()
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
    layout->addWidget(gb);

	tabWidget->addTab(framePage, tr( "&Frame" ) );
}

void EnrichmentDialog::initGeometryPage()
{
    geometryPage = new QWidget();

	unitBox = new QComboBox();
	unitBox->insertItem(tr("Scale Coordinates"));
	unitBox->insertItem(tr("Pixels"));

	QBoxLayout *bl1 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl1->addWidget(new QLabel(tr( "Unit" )));
	bl1->addWidget(unitBox);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale locale = QLocale();
	if (app)
		locale = app->locale();
	
    QGroupBox *gb1 = new QGroupBox(tr("Position"));
	xBox = new DoubleSpinBox();
	xBox->setLocale(locale);
	xBox->setDecimals(6);
	yBox = new DoubleSpinBox();
	yBox->setLocale(locale);
	yBox->setDecimals(6);
	
    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel( tr("X")), 0, 0);
    gl1->addWidget(xBox, 0, 1);
    gl1->addWidget(new QLabel(tr("Y")), 1, 0);
    gl1->addWidget(yBox, 1, 1);
	gl1->setColumnStretch(1, 10);
	gl1->setRowStretch(2, 1);
    gb1->setLayout(gl1);

    QGroupBox *gb2 = new QGroupBox(tr("Size"));
    widthBox = new DoubleSpinBox();
	widthBox->setLocale(locale);
	widthBox->setDecimals(6);
	heightBox = new DoubleSpinBox();
	heightBox->setLocale(locale);
	heightBox->setDecimals(6);
	
    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("Width")), 0, 0);
    gl2->addWidget(widthBox, 0, 1);
	
    gl2->addWidget(new QLabel(tr("Height")), 1, 0);
    gl2->addWidget(heightBox, 1, 1);
	
	keepAspectBox = new QCheckBox(tr("&Keep aspect ratio"));
	gl2->addWidget(keepAspectBox, 2, 1);
		
	bestSizeButton = new QPushButton(tr("&Best size"));
	bestSizeButton->hide();
	gl2->addWidget(bestSizeButton, 3, 1);
	
	gl2->setColumnStretch(1, 10);
	gl2->setRowStretch(4, 1);
    gb2->setLayout(gl2);

    QBoxLayout *bl2 = new QBoxLayout (QBoxLayout::LeftToRight);
	bl2->addWidget(gb1);
	bl2->addWidget(gb2);

	QVBoxLayout* vl = new QVBoxLayout(geometryPage);
    vl->addLayout(bl1);
    vl->addLayout(bl2);

	connect(unitBox, SIGNAL(activated(int)), this, SLOT(displayCoordinates(int)));
	connect(widthBox, SIGNAL(valueChanged(double)), this, SLOT(adjustHeight(double)));
	connect(heightBox, SIGNAL(valueChanged(double)), this, SLOT(adjustWidth(double)));
	connect(bestSizeButton, SIGNAL(clicked()), this, SLOT(setBestSize()));

	tabWidget->addTab(geometryPage, tr( "&Geometry" ) );
}

void EnrichmentDialog::customButtons(QWidget *w)
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

void EnrichmentDialog::setWidget(FrameWidget *w)
{
	if (!w)
		return;

	d_widget = w;

	frameBox->setCurrentIndex(w->frameStyle());
	frameColorBtn->setColor(w->frameColor());
	
	displayCoordinates(0);

	if (d_widget_type == Tex){
		TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
		if (tw){
			equationEditor->setText(tw->formula());
			outputLabel->setPixmap(tw->pixmap());
			bestSizeButton->show();
		}
		return;
	} else if (d_widget_type == Image){
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i){
			imagePathBox->setText(i->fileName());
			boxSaveImagesInternally->blockSignals(true);
			boxSaveImagesInternally->setChecked(i->saveInternally());
			boxSaveImagesInternally->blockSignals(false);
		}
	}
}

void EnrichmentDialog::clearForm()
{
    outputLabel->setPixmap(QPixmap());
    equationEditor->clear();
}

void EnrichmentDialog::apply()
{
	if (tabWidget->currentPage() == editPage)
		fetchImage();
	else if (tabWidget->currentPage() == framePage){
		if (d_widget){
			d_widget->setFrameStyle(frameBox->currentIndex());
			d_widget->setFrameColor(frameColorBtn->color());
			d_widget->repaint();
			d_plot->multiLayer()->notifyChanges();
		}
	} else if (imagePage && tabWidget->currentPage() == imagePage)
		chooseImageFile(imagePathBox->text());	
	else if (tabWidget->currentPage() == geometryPage)
		setCoordinates(unitBox->currentIndex());
}

void EnrichmentDialog::fetchImage()
{
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw && tw->formula() == equationEditor->toPlainText() && !tw->pixmap().isNull())
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

void EnrichmentDialog::updateForm(bool error)
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

    clearButton->setEnabled(true);
    updateButton->setEnabled(true);
    equationEditor->setReadOnly(false);
}

void EnrichmentDialog::chooseImageFile(const QString& fn)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QString path = fn;
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this, tr("QtiPlot - Import image from file"), app->imagesDirPath);

	if (!path.isEmpty()){
		ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
		if (i && i->load(path)){
			imagePathBox->setText(path);
			QFileInfo fi(path);
			app->imagesDirPath = fi.dirPath(true);
			app->modifiedProject();
		}
	}
}

void EnrichmentDialog::saveImagesInternally(bool save)
{
	ImageWidget *i = qobject_cast<ImageWidget *>(d_widget);
	if (i)
		i->setSaveInternally(boxSaveImagesInternally->isChecked());
		
	d_plot->multiLayer()->notifyChanges();
	
	if (save)
		return;
	
	QString fn = imagePathBox->text();	
	if (fn.isEmpty() || !QFile::exists(fn)){
		QMessageBox::warning((ApplicationWindow *)parentWidget(), tr("QtiPlot - Warning"),
		tr("The file %1 doesn't exist. The image cannot be restored when reloading the project file!").arg(fn));
		chooseImageFile();
	}
}

void EnrichmentDialog::setCoordinates(int unit)
{
	if (!d_widget)
		return;
		
	if (unit == 0){//ScaleCoordinates
		double left = xBox->value();
		double top = yBox->value();
		d_widget->setCoordinates(left, top, left + widthBox->value(), top - heightBox->value());
	} else
		d_widget->setRect((int)xBox->value(), (int)yBox->value(), (int)widthBox->value(), (int)heightBox->value());
	
	d_plot->multiLayer()->notifyChanges();
}

void EnrichmentDialog::displayCoordinates(int unit)
{
	if (!d_widget)
		return;
	
	if (unit == 0){//ScaleCoordinates
		xBox->setFormat('g', 6);
		yBox->setFormat('g', 6);
		widthBox->setFormat('g', 6);
		heightBox->setFormat('g', 6);

		xBox->setSingleStep(0.1);
		yBox->setSingleStep(0.1);
		widthBox->setSingleStep(0.1);
		heightBox->setSingleStep(0.1);
				
		xBox->setValue(d_widget->xValue());
		yBox->setValue(d_widget->yValue());

		QRectF r = d_widget->boundingRect();
		widthBox->setValue(r.width());
		heightBox->setValue(r.height());
	} else {
		xBox->setFormat('f', 0);
		yBox->setFormat('f', 0);
		widthBox->setFormat('f', 0);
		heightBox->setFormat('f', 0);
		
		xBox->setSingleStep(1.0);
		yBox->setSingleStep(1.0);
		widthBox->setSingleStep(1.0);
		heightBox->setSingleStep(1.0);
		
		xBox->setValue(d_widget->x());
		yBox->setValue(d_widget->y());
		widthBox->setValue(d_widget->width());
		heightBox->setValue(d_widget->height());
	}
	
	aspect_ratio = widthBox->value()/heightBox->value();
}

void EnrichmentDialog::adjustHeight(double width)
{
	if (keepAspectBox->isChecked()){
		heightBox->blockSignals(true);
		heightBox->setValue(width/aspect_ratio);
		heightBox->blockSignals(false);
	} else
		aspect_ratio = width/heightBox->value();
}

void EnrichmentDialog::adjustWidth(double height)
{
	if (keepAspectBox->isChecked()){
		widthBox->blockSignals(true);
		widthBox->setValue(height*aspect_ratio);
		widthBox->blockSignals(false);
	} else
		aspect_ratio = widthBox->value()/height;
}

void EnrichmentDialog::setBestSize()
{
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw){
		tw->setBestSize();
		displayCoordinates(unitBox->currentIndex());
		d_plot->multiLayer()->notifyChanges();
	}
}

EnrichmentDialog::~EnrichmentDialog()
{
	QApplication::restoreOverrideCursor();
	
	TexWidget *tw = qobject_cast<TexWidget *>(d_widget);
	if (tw && (tw->formula().isEmpty() || tw->pixmap().isNull())){
		d_plot->remove(d_widget);
		d_plot->setActiveTool(NULL);
		d_widget->close();
		d_widget = NULL;
	}
}
