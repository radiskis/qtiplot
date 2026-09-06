/***************************************************************************
    File                 : TablesConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Table preferences page implementation

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
#include "TablesConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>
#include <Table.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFontDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QIcon>

TablesConfigPage::TablesConfigPage(QWidget *parent)
    : ConfigPage(parent)
{
	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);

	lblSeparator = new QLabel();
	topLayout->addWidget( lblSeparator );
	boxSeparator = new QComboBox();
	boxSeparator->setEditable( true );
	topLayout->addWidget( boxSeparator );

	QString help = tr("The column separator can be customized. \nThe following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: \n0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	lblSeparator->setWhatsThis(help);
	lblSeparator->setToolTip(help);

	groupBoxTableCol = new QGroupBox();
	QGridLayout * colorsLayout = new QGridLayout(groupBoxTableCol);

	lblTableBackground = new QLabel();
	colorsLayout->addWidget( lblTableBackground, 0, 0 );
	buttonBackground = new ColorButton();
	colorsLayout->addWidget( buttonBackground, 0, 1 );

	lblTextColor = new QLabel();
	colorsLayout->addWidget( lblTextColor, 1, 0 );
	buttonText = new ColorButton();
	colorsLayout->addWidget( buttonText, 1, 1 );

	lblHeaderColor = new QLabel();
	colorsLayout->addWidget( lblHeaderColor, 2, 0 );
	buttonHeader = new ColorButton();
	colorsLayout->addWidget( buttonHeader, 2, 1 );

	groupBoxTableFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBoxTableFonts );

	buttonTextFont = new QPushButton();
	buttonTextFont->setIcon(QIcon(":/font.png"));
	bottomLayout->addWidget( buttonTextFont );
	buttonHeaderFont = new QPushButton();
	buttonHeaderFont->setIcon(QIcon(":/font.png"));
	bottomLayout->addWidget( buttonHeaderFont );

	boxTableComments = new QCheckBox();
	boxUpdateTableValues = new QCheckBox();
	boxTablePasteDialog = new QCheckBox();

	QVBoxLayout * tablesPageLayout = new QVBoxLayout( this );
	tablesPageLayout->addLayout(topLayout,1);
	tablesPageLayout->addWidget(groupBoxTableCol);
	tablesPageLayout->addWidget(groupBoxTableFonts);
	tablesPageLayout->addWidget(boxTableComments);
	tablesPageLayout->addWidget(boxUpdateTableValues);
	tablesPageLayout->addWidget(boxTablePasteDialog);
	tablesPageLayout->addStretch();

	connect(buttonTextFont, &QAbstractButton::clicked, this, &TablesConfigPage::pickTextFont);
	connect(buttonHeaderFont, &QAbstractButton::clicked, this, &TablesConfigPage::pickHeaderFont);
}

void TablesConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	textFont = app->tableTextFont;
	headerFont = app->tableHeaderFont;

	buttonBackground->setColor(app->tableBkgdColor);
	buttonText->setColor(app->tableTextColor);
	buttonHeader->setColor(app->tableHeaderColor);
	boxTableComments->setChecked(app->d_show_table_comments);
	boxUpdateTableValues->setChecked(app->autoUpdateTableValues());
	boxTablePasteDialog->setChecked(app->d_show_table_paste_dialog);

	setColumnSeparator(app->columnSeparator);
}

void TablesConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);
	sep.replace("\\t", "\t");
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");

	if (sep.contains(QRegularExpression("[0-9.eE+-]")) != 0){
		QMessageBox::warning(this, tr("QtiPlot - Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	app->columnSeparator = sep;
	app->setAutoUpdateTableValues(boxUpdateTableValues->isChecked());
	app->d_show_table_paste_dialog = boxTablePasteDialog->isChecked();

	app->tableBkgdColor = buttonBackground->color();
	app->tableTextColor = buttonText->color();
	app->tableHeaderColor = buttonHeader->color();
	app->tableTextFont = textFont;
	app->tableHeaderFont = headerFont;
	app->d_show_table_comments = boxTableComments->isChecked();

	QPalette palette;
	palette.setColor(QPalette::Base, buttonBackground->color());
	palette.setColor(QPalette::Text, buttonText->color());

	QList<MdiSubWindow *> windows = app->windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Table")){
			Table *t = (Table*)w;
			w->setPalette(palette);
			t->setHeaderColor(buttonHeader->color());
			t->setTextFont(textFont);
			t->setHeaderFont(headerFont);
			t->showComments(boxTableComments->isChecked());
		}
	}
}

void TablesConfigPage::retranslateUi()
{
	lblSeparator->setText(tr("Default Column Separator"));
	groupBoxTableCol->setTitle(tr("Colors"));
	groupBoxTableFonts->setTitle(tr("Fonts"));
	buttonTextFont->setText(tr("&Text"));
	buttonHeaderFont->setText(tr("&Header"));
	lblTableBackground->setText(tr("Background"));
	lblTextColor->setText(tr("Text"));
	lblHeaderColor->setText(tr("Header"));
	boxTableComments->setText(tr("Display &Comments in Header"));
	boxUpdateTableValues->setText(tr("Automatically &Recalculate Column Values"));
	boxTablePasteDialog->setText(tr("Show &Prompt on Paste to Multiple Cells"));

	QString sep = boxSeparator->currentText();
	boxSeparator->clear();
	boxSeparator->addItem(tr("TAB"));
	boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem("; " + tr("SPACE"));
	boxSeparator->addItem(", " + tr("SPACE"));
	boxSeparator->addItem(";");
	boxSeparator->addItem(",");
	setColumnSeparator(sep);
}

void TablesConfigPage::setColumnSeparator(const QString& sep)
{
	if (sep == "\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep == " ")
		boxSeparator->setCurrentIndex(1);
	else if (sep == ";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep == ",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep == "; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep == ", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep == ";")
		boxSeparator->setCurrentIndex(6);
	else if (sep == ",")
		boxSeparator->setCurrentIndex(7);
	else {
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ", "\\s").replace("\t", "\\t"));
	}
}

void TablesConfigPage::pickTextFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, textFont, this);
	if (ok)
		textFont = font;
}

void TablesConfigPage::pickHeaderFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, headerFont, this);
	if (ok)
		headerFont = font;
}
