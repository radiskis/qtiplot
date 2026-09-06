/***************************************************************************
    File                 : ConfigDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Preferences dialog implementation

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
#include "ConfigDialog.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include "GeneralConfigPage.h"
#include "TablesConfigPage.h"
#include "Plots2DConfigPage.h"
#include "Plots3DConfigPage.h"
#include "NotesConfigPage.h"
#include "FittingConfigPage.h"

#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

ConfigDialog::ConfigDialog( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl ),
      d_app( qobject_cast<ApplicationWindow *>(parent) ),
      d_settings( d_app ? d_app->settings() : nullptr )
{
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled(true);

	generalDialog = new QStackedWidget();
	itemsList = new QListWidget();
	itemsList->setSpacing(10);
	itemsList->setIconSize(QSize(32, 32));

	GeneralConfigPage *generalPage = new GeneralConfigPage(this);
	TablesConfigPage *tablesPage = new TablesConfigPage(this);
	Plots2DConfigPage *plots2DPage = new Plots2DConfigPage(this);
	Plots3DConfigPage *plots3DPage = new Plots3DConfigPage(this);
	NotesConfigPage *notesPage = new NotesConfigPage(this);
	FittingConfigPage *fitPage = new FittingConfigPage(this);

	d_pages << generalPage << tablesPage << plots2DPage << plots3DPage << notesPage << fitPage;

	for (ConfigPage *page : d_pages){
		page->init(d_app, d_settings);
		generalDialog->addWidget(page);
	}

	QVBoxLayout * rightLayout = new QVBoxLayout();
	lblPageHeader = new QLabel();
	QFont fnt = this->font();
	fnt.setPointSize(fnt.pointSize() + 3);
	fnt.setBold(true);
	lblPageHeader->setFont(fnt);
	lblPageHeader->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	if (d_app){
		QPalette pal = lblPageHeader->palette();
		pal.setColor( QPalette::Window, d_app->panelsColor );
		lblPageHeader->setPalette(pal);
		lblPageHeader->setAutoFillBackground( true );
	}

	rightLayout->setSpacing(10);
	rightLayout->addWidget( lblPageHeader );
	rightLayout->addWidget( generalDialog );

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);
	topLayout->setContentsMargins(5, 5, 5, 5);
	topLayout->addWidget(itemsList, 0);
	topLayout->addLayout(rightLayout, 1);
	topLayout->addStretch();

	QHBoxLayout * bottomButtons = new QHBoxLayout();
	bottomButtons->addStretch();

	btnDefaultSettings = new QPushButton();
	btnDefaultSettings->setAutoDefault( false );
	bottomButtons->addWidget( btnDefaultSettings );

	buttonApply = new QPushButton();
	buttonApply->setAutoDefault( true );
	bottomButtons->addWidget( buttonApply );

	buttonOk = new QPushButton();
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );
	bottomButtons->addWidget( buttonOk );

	buttonCancel = new QPushButton();
	buttonCancel->setAutoDefault( true );
	bottomButtons->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout( this );
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomButtons);

	languageChange();

	connect( itemsList, &QListWidget::currentRowChanged, this, &ConfigDialog::setCurrentPage);
	connect(buttonOk, &QAbstractButton::clicked, this, &ConfigDialog::accept);
	connect(buttonApply, &QAbstractButton::clicked, this, &ConfigDialog::apply);
	connect(buttonCancel, &QAbstractButton::clicked, this, &ConfigDialog::reject);
	connect(btnDefaultSettings, &QAbstractButton::clicked, this, &ConfigDialog::resetDefaultSettings);

	setCurrentPage(0);
}

void ConfigDialog::setCurrentPage(int index)
{
	generalDialog->setCurrentIndex(index);
	if (itemsList->currentItem())
		lblPageHeader->setText(itemsList->currentItem()->text());
}

void ConfigDialog::accept()
{
	apply();
	close();
}

void ConfigDialog::apply()
{
	if (!d_app)
		return;

	for (ConfigPage *page : d_pages)
		page->apply(d_app, d_settings);

	d_app->saveSettings();
}

void ConfigDialog::languageChange()
{
	setWindowTitle( tr( "QtiPlot - Choose default settings" ) );
	btnDefaultSettings->setText(tr("&Default options"));
	buttonApply->setText(tr("&Apply"));
	buttonOk->setText(tr("&OK"));
	buttonCancel->setText(tr("&Cancel"));

	itemsList->clear();
	for (ConfigPage *page : d_pages){
		itemsList->addItem(new QListWidgetItem(page->pageIcon(), page->pageTitle()));
		page->retranslateUi();
	}
	if (itemsList->count() > 0)
		itemsList->setCurrentRow(generalDialog->currentIndex());
	if (itemsList->currentItem())
		lblPageHeader->setText(itemsList->currentItem()->text());
}

void ConfigDialog::resetDefaultSettings()
{
	if (!d_app)
		return;

	d_app->setDefaultOptions();
	for (ConfigPage *page : d_pages)
		page->init(d_app, d_settings);

	QString msg = tr("You need to restart QtiPlot before your changes become effective, would you like to do it now?");
	if (QMessageBox::question(this, tr("QtiPlot"), msg, QMessageBox::Ok, QMessageBox::No) == QMessageBox::Ok){
		connect(this, &QObject::destroyed, d_app, &ApplicationWindow::newProject);
		close();
	}
}

void ConfigDialog::setColumnSeparator(const QString& sep)
{
	for (ConfigPage *page : d_pages){
		TablesConfigPage *tp = qobject_cast<TablesConfigPage *>(page);
		if (tp){
			tp->setColumnSeparator(sep);
			break;
		}
	}
}
