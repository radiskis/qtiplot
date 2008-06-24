/***************************************************************************
    File                 : EnrichmentDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A general properties dialog for the FrameWidget, using article
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
#include "../RectangleWidget.h"
#include "../../ColorButton.h"
#include "../../ApplicationWindow.h"
#include "../../DoubleSpinBox.h"
#include "../../PatternBox.h"
#include "../../PenStyleBox.h"

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
	patternPage = NULL;

	if (wt == Tex){
		setWindowTitle(tr("QtiPlot") + " - " + tr("Tex Equation Editor"));

    	clearButton = buttonBox->addButton(tr("Clea&r"), QDialogButtonBox::ResetRole);
		connect(clearButton, SIGNAL(clicked()), this, SLOT(clearForm()));
	} else if (wt == MDIWindow)
        setWindowTitle(tr("QtiPlot") + " - " + tr("Window Geometry"));
	else
		setWindowTitle(tr("QtiPlot") + " - " + tr("Object Properties"));

    updateButton = buttonBox->addButton(tr("&Apply"), QDialogButtonBox::ApplyRole);
	connect(updateButton, SIGNAL(clicked()), this, SLOT(apply()));

	cancelButton = buttonBox->addButton(tr("&Close"), QDialogButtonBox::RejectRole);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	tabWidget = new QTabWidget();
	if (wt == Tex)
		initEditorPage();
	else if (wt == Image)
		initImagePage();
	else if (wt == Frame)
		initPatternPage();

    if (wt != MDIWindow)
        initFramePage();
	initGeometryPage();

    QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

	connect(tabWidget, SIGNAL(currentChanged (QWidget *)), this, SLOT(customButtons(QWidget *)));
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
	
	gl->addWidget(new QLabel(tr( "Line Style" )), 2, 0);
	boxFrameLineStyle = new PenStyleBox();
	gl->addWidget(boxFrameLineStyle, 2, 1);
	
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(3, 1);

	if (d_widget_type == Frame){
		gl->addWidget(new QLabel(tr("Width")), 3, 0);
		boxFrameWidth = new QSpinBox();
		gl->addWidget(boxFrameWidth, 3, 1);
		gl->setRowStretch(4, 1);
	}

	QVBoxLayout *layout = new QVBoxLayout(framePage);
    layout->addWidget(gb);

	tabWidget->addTab(framePage, tr( "&Frame" ) );
}

void EnrichmentDialog::initPatternPage()
{
	patternPage = new QWidget();

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);
    gl->addWidget(new QLabel( tr("Fill Color")), 0, 0);

	backgroundColorBtn = new ColorButton();
    gl->addWidget(backgroundColorBtn, 0, 1);
	
	gl->addWidget(new QLabel(tr("Opacity")), 1, 0);
	boxTransparency = new QSpinBox();
	boxTransparency->setRange(0, 255);
    boxTransparency->setSingleStep(5);
	boxTransparency->setWrapping(true);
    boxTransparency->setSpecialValueText(tr("Transparent"));
	gl->addWidget(boxTransparency, 1, 1);
	
	gl->addWidget(new QLabel(tr("Pattern")), 2, 0);
	patternBox = new PatternBox();
	gl->addWidget(patternBox, 2, 1);
	
	gl->addWidget(new QLabel(tr("Pattern Color")), 3, 0);
	patternColorBtn = new ColorButton();
	gl->addWidget(patternColorBtn, 3, 1);
	
	useFrameColorBox = new QCheckBox(tr("Use &Frame Color"));
	connect(useFrameColorBox, SIGNAL(toggled(bool)), patternColorBtn, SLOT(setDisabled(bool)));
	gl->addWidget(useFrameColorBox, 3, 2);
	
	gl->setColumnStretch(1, 1);
	gl->setRowStretch(4, 1);

	QVBoxLayout *layout = new QVBoxLayout(patternPage);
    layout->addWidget(gb);

	tabWidget->addTab(patternPage, tr("Fill &Pattern"));
}

void EnrichmentDialog::initGeometryPage()
{
    geometryPage = new QWidget();

	unitBox = new QComboBox();
	unitBox->insertItem(tr("inch"));
	unitBox->insertItem(tr("mm"));
	unitBox->insertItem(tr("cm"));
	unitBox->insertItem(tr("point"));
	unitBox->insertItem(tr("pixel"));
	if (d_widget_type != MDIWindow)
        unitBox->insertItem(tr("scale"));

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
	xBox->setMinimumWidth(80);
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
	if (d_widget_type == Tex && editPage && w == editPage && clearButton){
		clearButton->show();
		return;
	} else if (clearButton)
		clearButton->hide();

	if (w == framePage)
		updateButton->setEnabled(true);
}

void EnrichmentDialog::setWidget(QWidget *w)
{
	if (!w)
		return;

	d_widget = w;

    FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
    if (fw){
        frameBox->setCurrentIndex(fw->frameStyle());
        frameColorBtn->setColor(fw->frameColor());
		boxFrameLineStyle->setStyle(fw->framePen().style());
		if (d_widget_type == Frame)
			boxFrameWidth->setValue(fw->framePen().width());
    }

    unitBox->setCurrentIndex(FrameWidget::Pixel);
	displayCoordinates(FrameWidget::Pixel);

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
	} else if (d_widget_type == Frame){
		RectangleWidget *r = qobject_cast<RectangleWidget *>(d_widget);
		if (r){
			backgroundColorBtn->setColor(r->backgroundColor());
			boxTransparency->setValue(r->backgroundColor().alpha());
			patternBox->setPattern(r->brush().style());
			patternColorBtn->setColor(r->brush().color());
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
	    FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
        if (fw){
			fw->setFrameStyle(frameBox->currentIndex());
			QPen pen = QPen(frameColorBtn->color(), 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
			pen.setStyle(boxFrameLineStyle->style());
			if (d_widget_type == Frame)
				pen.setWidth(boxFrameWidth->value());
			fw->setFramePen(pen);
			fw->repaint();
			d_plot->multiLayer()->notifyChanges();
		}
	} else if (imagePage && tabWidget->currentPage() == imagePage)
		chooseImageFile(imagePathBox->text());
	else if (tabWidget->currentPage() == geometryPage)
		setCoordinates(unitBox->currentIndex());
	else if (patternPage && tabWidget->currentPage() == patternPage){
		RectangleWidget *r = qobject_cast<RectangleWidget *>(d_widget);
        if (r){
			QColor c = backgroundColorBtn->color();
			c.setAlpha(boxTransparency->value());
			r->setBackgroundColor(c);
			
			QColor patternColor = patternColorBtn->color();
			if (useFrameColorBox->isChecked())
				patternColor = frameColorBtn->color();
			r->setBrush(QBrush(patternColor, patternBox->getSelectedPattern()));
			
			r->repaint();
			d_plot->multiLayer()->notifyChanges();
		}
	}
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

	if (unit == FrameWidget::Scale){//ScaleCoordinates
		double left = xBox->value();
		double top = yBox->value();
		FrameWidget *fw = qobject_cast<FrameWidget *>(d_widget);
        if (fw)
            fw->setCoordinates(left, top, left + widthBox->value(), top - heightBox->value());
	} else
		FrameWidget::setRect(d_widget, xBox->value(), yBox->value(),
        widthBox->value(), heightBox->value(), (FrameWidget::Unit)unit);

    if (d_plot)
        d_plot->multiLayer()->notifyChanges();
}

void EnrichmentDialog::displayCoordinates(int unit)
{
	if (!d_widget)
		return;

	if (unit == FrameWidget::Pixel || unit == FrameWidget::Point){
		xBox->setFormat('f', 0);
		yBox->setFormat('f', 0);
		widthBox->setFormat('f', 0);
		heightBox->setFormat('f', 0);

		xBox->setSingleStep(1.0);
		yBox->setSingleStep(1.0);
		widthBox->setSingleStep(1.0);
		heightBox->setSingleStep(1.0);
	} else {
		xBox->setFormat('g', 6);
		yBox->setFormat('g', 6);
		widthBox->setFormat('g', 6);
		heightBox->setFormat('g', 6);

		xBox->setSingleStep(0.1);
		yBox->setSingleStep(0.1);
		widthBox->setSingleStep(0.1);
		heightBox->setSingleStep(0.1);
	}

	xBox->setValue(FrameWidget::xIn(d_widget, (FrameWidget::Unit)unit));
	yBox->setValue(FrameWidget::yIn(d_widget, (FrameWidget::Unit)unit));
	widthBox->setValue(FrameWidget::widthIn(d_widget, (FrameWidget::Unit)unit));
	heightBox->setValue(FrameWidget::heightIn(d_widget, (FrameWidget::Unit)unit));

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
		d_plot->remove(tw);
		d_plot->setActiveTool(NULL);
		d_widget->close();
		d_widget = NULL;
	}
}
