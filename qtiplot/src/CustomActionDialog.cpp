/***************************************************************************
    File                 : CustomActionDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Custom Action dialog

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
#include "CustomActionDialog.h"
#include "ApplicationWindow.h"

#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QListWidget>
#include <QLayout>
#include <QFileDialog>
#include <QToolBar>
#include <QMenu>
#include <QImageReader>
#include <QMessageBox>

CustomActionDialog::CustomActionDialog(QWidget* parent, Qt::WFlags fl)
    : QDialog(parent, fl)
{
    setWindowTitle(tr("QtiPlot") + " - " + tr("Add Custom Action"));

    d_actions.clear();

    ApplicationWindow *app = (ApplicationWindow *)parent;
    QStringList toolBars, menus;
    foreach (QWidget *w, QApplication::allWidgets()){
        if (w->isA("QToolBar"))
            toolBars << ((QToolBar*)w)->windowTitle();
        else if (w->isA("QMenu") && !((QMenu*)w)->title().isEmpty())
            menus << ((QMenu*)w)->title();
    }
    toolBars.sort();
    menus.sort();

    itemsList = new QListWidget();
    itemsList->setSelectionMode(QAbstractItemView::SingleSelection);
	itemsList->setSpacing(2);

	QList<QAction *> actionsList = app->customActionsList();
	foreach(QAction *action, actionsList){
	    QString text = action->text();
        QString shortcut = action->shortcut().toString();
	    if (!shortcut.isEmpty())
            text += " (" + shortcut + ")";

        QListWidgetItem *it = new QListWidgetItem(text, itemsList);
        if (!action->icon().isNull())
            it->setIcon(action->icon());
        itemsList->addItem(it);
        d_actions << action;
	}

    QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);

	gl1->addWidget(new QLabel(tr("Folder")), 0, 0);
	folderBox = new QLineEdit();
	folderBox->setText(app->customActionsDirPath);

	gl1->addWidget(folderBox, 0, 1);
	folderBtn = new QPushButton(tr("Browse"));
	gl1->addWidget(folderBtn, 0, 2);

	gl1->addWidget(new QLabel(tr("Script File")), 1, 0);
	fileBox = new QLineEdit();

	gl1->addWidget(fileBox, 1, 1);
	fileBtn = new QPushButton(tr("Browse"));
	gl1->addWidget(fileBtn, 1, 2);

	gl1->addWidget(new QLabel(tr("Icon")), 2, 0);
	iconBox = new QLineEdit();
	gl1->addWidget(iconBox, 2, 1);
	iconBtn = new QPushButton(tr("Browse"));
	gl1->addWidget(iconBtn, 2, 2);

	gl1->addWidget(new QLabel(tr("Text")), 3, 0);
	textBox = new QLineEdit();
	gl1->addWidget(textBox, 3, 1);

	gl1->addWidget(new QLabel(tr("Tool Tip Text")), 4, 0);
	toolTipBox = new QLineEdit();
	gl1->addWidget(toolTipBox, 4, 1);

	gl1->addWidget(new QLabel(tr("Shortcut")), 5, 0);
	shortcutBox = new QLineEdit();
	gl1->addWidget(shortcutBox, 5, 1);

    menuBtn = new QRadioButton(tr("Menu"));
    gl1->addWidget(menuBtn, 6, 0);
    menuBox = new QComboBox();
    menuBox->addItems(menus);
    gl1->addWidget(menuBox, 6, 1);

    toolBarBtn = new QRadioButton(tr("Tool Bar"));
    toolBarBtn->setChecked(true);
    gl1->addWidget(toolBarBtn, 7, 0);
    toolBarBox = new QComboBox();
    toolBarBox->addItems(toolBars);
    gl1->addWidget(toolBarBox, 7, 1);
    gl1->setRowStretch(8, 1);

	QHBoxLayout * bottomButtons = new QHBoxLayout();
	bottomButtons->addStretch();
	buttonAdd = new QPushButton(tr("Add"));
	buttonAdd->setAutoDefault( true );
	bottomButtons->addWidget(buttonAdd);

	buttonRemove = new QPushButton(tr("Remove"));
	buttonRemove->setAutoDefault(true);
	bottomButtons->addWidget(buttonRemove);

	buttonSave = new QPushButton(tr("Save"));
	buttonSave->setAutoDefault(true);
	bottomButtons->addWidget(buttonSave);

	buttonCancel = new QPushButton(tr("Close"));
	buttonCancel->setAutoDefault( true );
	bottomButtons->addWidget( buttonCancel );

	QHBoxLayout *vl = new QHBoxLayout();
	vl->addWidget(itemsList);
	vl->addWidget(gb1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(vl);
	mainLayout->addLayout(bottomButtons);

    setCurrentAction(0);
    itemsList->setCurrentRow(0);

	connect(buttonAdd, SIGNAL(clicked()), this, SLOT(addAction()));
	connect(buttonRemove, SIGNAL(clicked()), this, SLOT(removeAction()));
	connect(buttonSave, SIGNAL(clicked()), this, SLOT(saveAction()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(iconBtn, SIGNAL(clicked()), this, SLOT(chooseIcon()));
	connect(fileBtn, SIGNAL(clicked()), this, SLOT(chooseFile()));
	connect(folderBtn, SIGNAL(clicked()), this, SLOT(chooseFolder()));
	connect(itemsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentAction(int)));
}

QAction* CustomActionDialog::addAction()
{
    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
    if (!app)
        return 0;

    QString folder = folderBox->text();
    if (folder.isEmpty() || !QFile::exists(folder))
        chooseFolder();

    foreach(QAction *action, d_actions){
        if(action->text() == textBox->text()){
            QMessageBox::critical(app, tr("QtiPlot") + " - " + tr("Error"),
            tr("You have already defined an action having menu text: %1, please choose another menu text!").arg(textBox->text()));
            textBox->setFocus();
            return 0;
        }
    }

    QString file = fileBox->text();
    QFileInfo fi(file);
    if (file.isEmpty() || !fi.exists()){
        QMessageBox::critical(app, tr("QtiPlot") + " - " + tr("Error"),
        tr("The file you have specified doesn't exist, please choose a valid script file!"));
        fileBox->setFocus();
        return 0;
    }

    QString iconPath = iconBox->text();
    QFileInfo iconInfo(iconPath);
    if (!iconPath.isEmpty() && (!iconInfo.exists() || !iconInfo.isFile() || !iconInfo.isReadable())){
        iconPath = QString();
        QMessageBox::critical(app, tr("QtiPlot") + " - " + tr("Error"),
        tr("The image file you have specified doesn't exist or can't be read, please choose another file!"));
        iconBox->setFocus();
        return 0;
    }

    QAction *action = new QAction(textBox->text(), app);
    action->setData(fi.absoluteFilePath());

    QIcon icon = QIcon();
    if (!iconPath.isEmpty()){
        icon = QIcon(iconPath);
        action->setIcon(icon);
        action->setIconText(iconPath);
    }

    if (!toolTipBox->text().isEmpty())
        action->setToolTip(toolTipBox->text());

    if (!shortcutBox->text().isEmpty())
        action->setShortcut(shortcutBox->text());

    if (toolBarBtn->isChecked())
        app->addCustomAction(action, toolBarBox->currentText(), true);
    else if (menuBtn->isChecked())
        app->addCustomAction(action, menuBox->currentText(), false);

    QListWidgetItem *it = new QListWidgetItem(textBox->text() + " (" + shortcutBox->text() + ")", itemsList);
    if (!icon.isNull())
        it->setIcon(icon);
    itemsList->addItem(it);
    itemsList->setCurrentItem(it);

    d_actions << action;
    saveAction(action);
    return action;
}

void CustomActionDialog::removeAction()
{
    int row = itemsList->currentRow();
    if (row < 0 || row >= d_actions.count())
        return;

    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
    QString s = tr("Are you sure you want to remove this action?");
    if (QMessageBox::Ok != QMessageBox::question(app, tr("QtiPlot") + " - " + tr("Remove Action"), s))
        return;

    QAction *action = d_actions.at(row);
    QFile f(app->customActionsDirPath + "/" + action->text() + ".qca");
    f.remove();

    QList<QAction *> actionsList = app->customActionsList();
    foreach (QAction *a, actionsList){
        if(a->text() == action->text()){
            app->removeCustomAction(a);
            break;
        }
    }
    d_actions.removeAt(row);
    itemsList->takeItem(row);
}

void CustomActionDialog::saveAction()
{
    QAction *action = NULL;
    foreach(QAction *act, d_actions){
        if(act->text() == textBox->text()){
            action = act;
            break;
        }
    }

    if (!action)
        action = addAction();
    else
        saveAction(action);
}

void CustomActionDialog::saveAction(QAction *action)
{
    if (!action)
        return;

    ApplicationWindow *app = (ApplicationWindow *)parent();
    QString fileName = app->customActionsDirPath + "/" + action->text() + ".qca";
    QFile f(fileName);
	if (!f.open( QIODevice::WriteOnly)){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(app, tr("QtiPlot") + " - " + tr("File Save Error"),
				tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fileName));
		return;
	}

    QTextStream out( &f );
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE action>\n"
         << "<action version=\"1.0\">\n";

     out << "<text>" + action->text() + "</text>\n";
     out << "<file>" + action->data().toString() + "</file>\n";
     out << "<icon>" + action->iconText() + "</icon>\n";
     out << "<tooltip>" + action->toolTip() + "</tooltip>\n";
     out << "<shortcut>" + action->shortcut().toString() + "</shortcut>\n";

     QList<QWidget *> list = action->associatedWidgets();
     QWidget *w = list[0];
     bool type = toolBarBtn->isChecked();
     out << "<type>" + QString::number(type) + "</type>\n";
     if (type)
         out << "<location>" + ((QToolBar*)w)->windowTitle() + "</location>\n";
     else
         out << "<location>" + ((QMenu*)w)->title() + "</location>\n";
     out << "</action>\n";
}

void CustomActionDialog::chooseIcon()
{
    QList<QByteArray> list = QImageReader::supportedImageFormats();
	QString filter = tr("Images") + " (", aux1, aux2;
	for (int i=0; i<(int)list.count(); i++){
		aux1 = " *." + list[i] + " ";
		aux2 += " *." + list[i] + ";;";
		filter += aux1;
	}
	filter+=");;" + aux2;

	QString fn = QFileDialog::getOpenFileName(this, tr("QtiPlot - Load icon from file"), iconBox->text(), filter);
	if (!fn.isEmpty())
		iconBox->setText(fn);
}

void CustomActionDialog::chooseFile()
{
    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
    QString fileName = QFileDialog::getOpenFileName(app, tr("Choose script file"), fileBox->text());
    if (!fileName.isEmpty())
        fileBox->setText(fileName);
}

void CustomActionDialog::chooseFolder()
{
    ApplicationWindow *app = (ApplicationWindow *)parentWidget();

    QString dir = QFileDialog::getExistingDirectory(app, tr("Choose the custom actions folder"), app->customActionsDirPath);
    if (!dir.isEmpty()){
        app->customActionsDirPath = dir;
        loadCustomActions();
        folderBox->setText(dir);
    }
}

void CustomActionDialog::setCurrentAction(int row)
{
    if (d_actions.isEmpty() || row < 0 || row >= d_actions.count())
        return;

    QAction *action = d_actions.at(row);
    if (!action)
        return;

    fileBox->setText(action->data().toString());
    textBox->setText(action->text());
    iconBox->setText(action->iconText());
    toolTipBox->setText(action->toolTip());
    shortcutBox->setText(action->shortcut().toString());

    QList<QWidget *> list = action->associatedWidgets();
    QWidget *w = NULL;
    if (!list.isEmpty())
        w = list[0];

    int index = toolBarBox->findText(((QToolBar*)w)->windowTitle());
    if (index >= 0){
        toolBarBox->setCurrentIndex(index);
        toolBarBtn->setChecked(true);
    } else {
        index = menuBox->findText(((QMenu*)w)->title());
        if (index >= 0){
            menuBox->setCurrentIndex(index);
            menuBtn->setChecked(true);
        }
    }
}

void CustomActionDialog::loadCustomActions()
{
    ApplicationWindow *app = (ApplicationWindow *)this->parent();
    QString path = app->customActionsDirPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks, QDir::Name);
	for (int i=0; i<lst.count(); i++){
	    QString fileName = path + lst[i];
        QFile file(fileName);
        QFileInfo fi(file);
        if (!file.open(QFile::ReadOnly | QFile::Text)){
            QMessageBox::warning(((ApplicationWindow *)parent()), tr("QtiPlot Custom Action"),
            tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
            continue;
        }

        QAction *action = new QAction(app);
        CustomActionHandler handler(action);
        QXmlSimpleReader reader;
        reader.setContentHandler(&handler);
        reader.setErrorHandler(&handler);

        QXmlInputSource xmlInputSource(&file);
        if (reader.parse(xmlInputSource)){
            d_actions << action;

            QListWidgetItem *it = new QListWidgetItem(action->text() + " (" + action->shortcut().toString() + ")", itemsList);
            if (!action->icon().isNull()){
                it->setIcon(action->icon());
                itemsList->addItem(it);
                itemsList->setCurrentItem(it);
            }
        }
	}
}

/*****************************************************************************
 *
 * Class CustomActionHandler
 *
 *****************************************************************************/

CustomActionHandler::CustomActionHandler(QAction *action)
     : d_action(action)
 {
     metFitTag = false;
     toolBar = false;
     filePath = QString();
 }

bool CustomActionHandler::startElement(const QString & /* namespaceURI */,
                                const QString & /* localName */,
                                const QString &qName,
                                const QXmlAttributes &attributes)
{
     if (!metFitTag && qName != "action") {
         errorStr = QObject::tr("The file is not a QtiPlot custom action file.");
         return false;
     }

     if (qName == "action") {
         QString version = attributes.value("version");
         if (!version.isEmpty() && version != "1.0") {
             errorStr = QObject::tr("The file is not an QtiPlot custom action version 1.0 file.");
             return false;
         }
         metFitTag = true;
     }

     currentText.clear();
     return true;
}

bool CustomActionHandler::endElement(const QString & /* namespaceURI */,
                              const QString & /* localName */,
                              const QString &qName)
{
    if (qName == "text")
        d_action->setText(currentText);
    else if (qName == "file")
        filePath = currentText;
    else if (qName == "icon" && !currentText.isEmpty()){
        d_action->setIcon(QIcon(currentText));
        d_action->setIconText(currentText);
    } else if (qName == "tooltip")
        d_action->setToolTip(currentText);
    else if (qName == "shortcut")
        d_action->setShortcut(currentText);
    else if (qName == "type")
        toolBar = (currentText == "1") ? true:false;
    else if (qName == "location"){
        foreach (QWidget *w, QApplication::allWidgets()){
            if (toolBar && w->isA("QToolBar") && ((QToolBar*)w)->windowTitle() == currentText)
                d_action->addTo((QToolBar*)w);
            else if (!toolBar && w->isA("QMenu") && ((QMenu*)w)->title() == currentText)
                d_action->addTo((QMenu*)w);
        }
    } else if (qName == "action")
        d_action->setData(filePath);

    return true;
}

bool CustomActionHandler::characters(const QString &str)
{
     currentText += str;
     return true;
}

bool CustomActionHandler::fatalError(const QXmlParseException &exception)
{
     QMessageBox::information(((ApplicationWindow *)d_action->parent()), QObject::tr("QtiPlot Custom Action"),
                              QObject::tr("Parse error at line %1, column %2:\n"
                                          "%3")
                              .arg(exception.lineNumber())
                              .arg(exception.columnNumber())
                              .arg(exception.message()));
     return false;
}

QString CustomActionHandler::errorString() const
{
     return errorStr;
}
