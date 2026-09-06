/***************************************************************************
    File                 : GeneralConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : General application preferences page implementation

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
#include "GeneralConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>
#include <Script.h>
#include <Table.h>
#include <Matrix.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QFontDialog>
#include <QStyleFactory>
#include <QFileDialog>
#include <QMessageBox>
#include <QCompleter>
#include <QFileSystemModel>
#include <QProcess>
#include <QApplication>
#include <QTranslator>

GeneralConfigPage::GeneralConfigPage(QWidget *parent)
    : ConfigPage(parent), d_app(nullptr)
{
	appTabWidget = new QTabWidget(this);
	appTabWidget->setUsesScrollButtons(false);

	application = new QWidget();
	QVBoxLayout * applicationLayout = new QVBoxLayout( application );
	QGroupBox * groupBoxApp = new QGroupBox();
	applicationLayout->addWidget(groupBoxApp);
	QGridLayout * topBoxLayout = new QGridLayout( groupBoxApp );

	lblLanguage = new QLabel();
	topBoxLayout->addWidget( lblLanguage, 0, 0 );
	boxLanguage = new QComboBox();
	topBoxLayout->addWidget( boxLanguage, 0, 1 );

	lblStyle = new QLabel();
	topBoxLayout->addWidget( lblStyle, 1, 0 );
	boxStyle = new QComboBox();
	topBoxLayout->addWidget( boxStyle, 1, 1 );
	QStringList styles = QStyleFactory::keys();
	styles.sort();
	boxStyle->addItems(styles);

	lblFonts = new QLabel();
	topBoxLayout->addWidget( lblFonts, 2, 0 );
	fontsBtn = new QPushButton();
	topBoxLayout->addWidget( fontsBtn, 2, 1 );

	lblScriptingLanguage = new QLabel();
	topBoxLayout->addWidget( lblScriptingLanguage, 3, 0 );
	boxScriptingLanguage = new QComboBox();
	QStringList llist = ScriptingLangManager::languages();
	boxScriptingLanguage->addItems(llist);
	topBoxLayout->addWidget( boxScriptingLanguage, 3, 1 );

    lblUndoStackSize = new QLabel();
	topBoxLayout->addWidget( lblUndoStackSize, 4, 0 );
    undoStackSizeBox = new QSpinBox();
    topBoxLayout->addWidget( undoStackSizeBox, 4, 1 );

	lblEndOfLine = new QLabel();
	topBoxLayout->addWidget(lblEndOfLine, 5, 0 );
	boxEndLine = new QComboBox();
	topBoxLayout->addWidget(boxEndLine, 5, 1);

	lblInitWindow = new QLabel();
	topBoxLayout->addWidget( lblInitWindow, 6, 0 );
	boxInitWindow = new QComboBox();
	topBoxLayout->addWidget( boxInitWindow, 6, 1 );

    boxSave = new QCheckBox();
	topBoxLayout->addWidget( boxSave, 7, 0 );

	boxMinutes = new QSpinBox();
	boxMinutes->setRange(1, 100);
	topBoxLayout->addWidget( boxMinutes, 7, 1 );

    boxBackupProject = new QCheckBox();
	topBoxLayout->addWidget( boxBackupProject, 8, 0, 1, 2 );

	boxSearchUpdates = new QCheckBox();
	topBoxLayout->addWidget( boxSearchUpdates, 9, 0, 1, 2 );

    completionBox = new QCheckBox();
	topBoxLayout->addWidget(completionBox, 10, 0);

	openLastProjectBox = new QCheckBox();
	topBoxLayout->addWidget(openLastProjectBox, 11, 0);

	topBoxLayout->setRowStretch(12, 1);

	excelImportMethodLabel = new QLabel;
	topBoxLayout->addWidget(excelImportMethodLabel, 12, 0);
	excelImportMethod = new QComboBox;
	topBoxLayout->addWidget(excelImportMethod, 12, 1);
	topBoxLayout->setRowStretch(13, 1);

	appTabWidget->addTab(application, QString());

	initConfirmationsTab();
	initColorsTab();
	initNumericFormatTab();
	initFileLocationsTab();
	initProxyTab();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(appTabWidget);

	connect(boxLanguage, QOverload<int>::of(&QComboBox::activated), this, &GeneralConfigPage::switchToLanguage);
	connect(fontsBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::pickApplicationFont);
	connect(boxSave, &QAbstractButton::toggled, boxMinutes, &QWidget::setEnabled);
}

void GeneralConfigPage::initConfirmationsTab()
{
	confirm = new QWidget();

	groupBoxConfirm = new QGroupBox();
	QVBoxLayout * layout = new QVBoxLayout( groupBoxConfirm );

	boxFolders = new QCheckBox();
	layout->addWidget( boxFolders );

	boxTables = new QCheckBox();
	layout->addWidget( boxTables );

	boxMatrices = new QCheckBox();
	layout->addWidget( boxMatrices );

	boxPlots2D = new QCheckBox();
	layout->addWidget( boxPlots2D );

	boxPlots3D = new QCheckBox();
	layout->addWidget( boxPlots3D );

	boxNotes = new QCheckBox();
	layout->addWidget( boxNotes );
	layout->addStretch();

	boxPromptRenameTables = new QCheckBox();
	boxTablePasteDialog = new QCheckBox();
	boxConfirmOverwrite = new QCheckBox();
	boxConfirmModifyDataPoints = new QCheckBox();

	QVBoxLayout * confirmPageLayout = new QVBoxLayout( confirm );
	confirmPageLayout->addWidget(groupBoxConfirm);
	confirmPageLayout->addWidget(boxPromptRenameTables);
	confirmPageLayout->addWidget(boxTablePasteDialog);
	confirmPageLayout->addWidget(boxConfirmOverwrite);
	confirmPageLayout->addWidget(boxConfirmModifyDataPoints);
	confirmPageLayout->addStretch();

	appTabWidget->addTab(confirm, QString());
}

void GeneralConfigPage::initColorsTab()
{
	appColors = new QWidget();
	QVBoxLayout * appColorsLayout = new QVBoxLayout( appColors );
	QGroupBox * groupBoxAppCol = new QGroupBox();
	appColorsLayout->addWidget( groupBoxAppCol );
	QGridLayout * colorsBoxLayout = new QGridLayout( groupBoxAppCol );

	lblWorkspace = new QLabel();
	colorsBoxLayout->addWidget( lblWorkspace, 0, 0 );
	btnWorkspace = new ColorButton();
	colorsBoxLayout->addWidget( btnWorkspace, 0, 1 );

	lblPanels = new QLabel();
	colorsBoxLayout->addWidget( lblPanels, 1, 0 );
	btnPanels = new ColorButton();
	colorsBoxLayout->addWidget( btnPanels, 1, 1 );

	lblPanelsText = new QLabel();
	colorsBoxLayout->addWidget( lblPanelsText, 2, 0 );
	btnPanelsText = new ColorButton();
	colorsBoxLayout->addWidget( btnPanelsText, 2, 1 );

	colorsBoxLayout->setRowStretch( 3, 1 );

	appTabWidget->addTab( appColors, QString() );
}

void GeneralConfigPage::initNumericFormatTab()
{
	numericFormatPage = new QWidget();
	QVBoxLayout *numLayout = new QVBoxLayout( numericFormatPage );
	QGroupBox *numericFormatBox = new QGroupBox();
	numLayout->addWidget( numericFormatBox );
	QGridLayout *numericFormatLayout = new QGridLayout( numericFormatBox );

	lblAppPrecision = new QLabel();
	numericFormatLayout->addWidget(lblAppPrecision, 0, 0);
	boxAppPrecision = new QSpinBox();
	boxAppPrecision->setRange(0, 16);
	numericFormatLayout->addWidget(boxAppPrecision, 0, 1);

    lblDecimalSeparator = new QLabel();
    numericFormatLayout->addWidget(lblDecimalSeparator, 1, 0 );
	boxDecimalSeparator = new QComboBox();
	numericFormatLayout->addWidget(boxDecimalSeparator, 1, 1);

    boxThousandsSeparator = new QCheckBox();
    numericFormatLayout->addWidget(boxThousandsSeparator, 1, 2);

	lblClipboardSeparator = new QLabel();
    numericFormatLayout->addWidget(lblClipboardSeparator, 2, 0 );
	boxClipboardLocale = new QComboBox();
	numericFormatLayout->addWidget(boxClipboardLocale, 2, 1);

	boxMuParserCLocale = new QCheckBox();
	numericFormatLayout->addWidget(boxMuParserCLocale, 3, 0);

	numericFormatLayout->setRowStretch(4, 1);
	appTabWidget->addTab( numericFormatPage, QString() );
}

void GeneralConfigPage::initFileLocationsTab()
{
	fileLocationsPage = new QWidget();

	QCompleter *completer = new QCompleter(this);
	completer->setModel(new QFileSystemModel(completer));
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode(QCompleter::InlineCompletion);

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);

	lblTranslationsPath = new QLabel;
	lblTranslationsPath->setOpenExternalLinks(true);
	gl->addWidget(lblTranslationsPath , 0, 0);

	translationsPathLine = new QLineEdit();
	translationsPathLine->setCompleter(completer);
	gl->addWidget(translationsPathLine, 0, 1);

	browseTranslationsBtn = new QPushButton();
	browseTranslationsBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browseTranslationsBtn, 0, 2);

	lblHelpPath = new QLabel;
	lblHelpPath->setOpenExternalLinks(true);
	gl->addWidget(lblHelpPath, 1, 0 );

	helpPathLine = new QLineEdit();
	helpPathLine->setCompleter(completer);
	gl->addWidget( helpPathLine, 1, 1);

	browseHelpBtn = new QPushButton();
	browseHelpBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browseHelpBtn, 1, 2);

	texCompilerLabel = new QLabel;
	texCompilerLabel->setOpenExternalLinks(true);
	gl->addWidget(texCompilerLabel, 2, 0);

	texCompilerPathBox = new QLineEdit();
	texCompilerPathBox->setCompleter(completer);
	connect(texCompilerPathBox, &QLineEdit::editingFinished, this, &GeneralConfigPage::validateTexCompiler);
	gl->addWidget(texCompilerPathBox, 2, 1);

    browseTexCompilerBtn = new QPushButton;
	browseTexCompilerBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseTexCompilerBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseTexCompiler);
    gl->addWidget(browseTexCompilerBtn, 2, 2);

	officeLabel = new QLabel;
	officeLabel->setOpenExternalLinks(true);
	gl->addWidget(officeLabel, 3, 0);

	sofficePathBox = new QLineEdit();
	sofficePathBox->setCompleter(completer);
	connect(sofficePathBox, &QLineEdit::editingFinished, this, &GeneralConfigPage::validateOffice);
	gl->addWidget(sofficePathBox, 3, 1);

	browseOfficeBtn = new QPushButton;
	browseOfficeBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseOfficeBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseOffice);
	gl->addWidget(browseOfficeBtn, 3, 2);

	javaLabel = new QLabel;
	javaLabel->setOpenExternalLinks(true);
	gl->addWidget(javaLabel, 4, 0);

	javaPathBox = new QLineEdit();
	javaPathBox->setCompleter(completer);
	connect(javaPathBox, &QLineEdit::editingFinished, this, &GeneralConfigPage::validateJava);
	gl->addWidget(javaPathBox, 4, 1);

	browseJavaBtn = new QPushButton;
	browseJavaBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseJavaBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseJava);
	gl->addWidget(browseJavaBtn, 4, 2);

	jodconverterLabel = new QLabel;
	jodconverterLabel->setOpenExternalLinks(true);
	gl->addWidget(jodconverterLabel, 5, 0);

	jodconverterPathBox = new QLineEdit();
	jodconverterPathBox->setCompleter(completer);
	connect(jodconverterPathBox, &QLineEdit::editingFinished, this, &GeneralConfigPage::validateJODConverter);
	gl->addWidget(jodconverterPathBox, 5, 1);

	browseJODConverterBtn = new QPushButton;
	browseJODConverterBtn->setIcon(QIcon(":/folder_open.png"));
	connect(browseJODConverterBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseJODConverter);
	gl->addWidget(browseJODConverterBtn, 5, 2);

#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir = new QLabel;
	gl->addWidget(lblPythonConfigDir, 6, 0);
	pythonConfigDirLine = new QLineEdit();
	pythonConfigDirLine->setCompleter(completer);
	gl->addWidget(pythonConfigDirLine, 6, 1);
	browsePythonConfigBtn = new QPushButton();
	browsePythonConfigBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browsePythonConfigBtn, 6, 2);
	connect(browsePythonConfigBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::choosePythonConfigFolder);

	lblPythonScriptsDir = new QLabel;
	gl->addWidget(lblPythonScriptsDir, 7, 0);
	pythonScriptsDirLine = new QLineEdit();
	pythonScriptsDirLine->setCompleter(completer);
	gl->addWidget(pythonScriptsDirLine, 7, 1);
	browsePythonScriptsBtn = new QPushButton();
	browsePythonScriptsBtn->setIcon(QIcon(":/folder_open.png"));
	gl->addWidget(browsePythonScriptsBtn, 7, 2);
	connect(browsePythonScriptsBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseStartupScriptsFolder);
#endif

	gl->setRowStretch(8, 1);

	QVBoxLayout *fileLocationsLayout = new QVBoxLayout(fileLocationsPage);
	fileLocationsLayout->addWidget(gb);

	connect(browseTranslationsBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseTranslationsFolder);
	connect(browseHelpBtn, &QAbstractButton::clicked, this, &GeneralConfigPage::chooseHelpFolder);

	appTabWidget->addTab(fileLocationsPage, QString());
}

void GeneralConfigPage::initProxyTab()
{
	proxyPage = new QWidget();

	proxyGroupBox = new QGroupBox (tr("&Proxy"));
	proxyGroupBox->setCheckable(true);

	QGridLayout *gl = new QGridLayout(proxyGroupBox);

	proxyHostLabel = new QLabel( tr("Host"));
    gl->addWidget(proxyHostLabel, 0, 0);
    proxyHostLine = new QLineEdit();
    gl->addWidget(proxyHostLine, 0, 1);

	proxyPortLabel = new QLabel( tr("Port"));
    gl->addWidget(proxyPortLabel, 1, 0);
    proxyPortBox = new QSpinBox;
    proxyPortBox->setMaximum(10000000);
    gl->addWidget(proxyPortBox, 1, 1);

	proxyUserLabel = new QLabel( tr("Username"));
    gl->addWidget(proxyUserLabel, 2, 0);
    proxyUserNameLine = new QLineEdit();
    gl->addWidget(proxyUserNameLine, 2, 1);

	proxyPasswordLabel = new QLabel( tr("Password"));
    gl->addWidget(proxyPasswordLabel, 3, 0);
    proxyPasswordLine = new QLineEdit;
    proxyPasswordLine->setEchoMode(QLineEdit::Password);
    gl->addWidget(proxyPasswordLine, 3, 1);

	gl->setRowStretch(4, 1);

	QVBoxLayout *layout = new QVBoxLayout(proxyPage);
    layout->addWidget(proxyGroupBox);

	appTabWidget->addTab(proxyPage, QString());
}

void GeneralConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	appFont = app->appFont;
	fontsBtn->setText(appFont.family() + " " + QString::number(appFont.pointSize()) + "pt");

	boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle, Qt::MatchWildcard));
	QStringList llist = ScriptingLangManager::languages();
	boxScriptingLanguage->setCurrentIndex(llist.indexOf(app->defaultScriptingLang));

	undoStackSizeBox->setValue(app->matrixUndoStackSize());
	boxEndLine->setCurrentIndex((int)app->d_eol);
	boxInitWindow->setCurrentIndex((int)app->d_init_window_type);
	excelImportMethod->setCurrentIndex((int)app->excelImportMethod());

	boxSave->setChecked(app->autoSave);
	boxMinutes->setValue(app->autoSaveTime);
	boxMinutes->setEnabled(app->autoSave);
	boxBackupProject->setChecked(app->d_backup_files);
	boxSearchUpdates->setChecked(app->autoSearchUpdates);
	completionBox->setChecked(app->d_completion);
	openLastProjectBox->setChecked(app->d_open_last_project);

	boxFolders->setChecked(app->confirmCloseFolder);
	boxTables->setChecked(app->confirmCloseTable);
	boxMatrices->setChecked(app->confirmCloseMatrix);
	boxPlots2D->setChecked(app->confirmClosePlot2D);
	boxPlots3D->setChecked(app->confirmClosePlot3D);
	boxNotes->setChecked(app->confirmCloseNotes);
	boxPromptRenameTables->setChecked(app->d_inform_rename_table);
	boxTablePasteDialog->setChecked(app->d_show_table_paste_dialog);
	boxConfirmOverwrite->setChecked(app->d_confirm_overwrite);
	boxConfirmModifyDataPoints->setChecked(app->d_confirm_modif_2D_points);

	btnWorkspace->setColor(app->workspaceColor);
	btnPanels->setColor(app->panelsColor);
	btnPanelsText->setColor(app->panelsTextColor);

	boxAppPrecision->setValue(app->d_decimal_digits);
	boxThousandsSeparator->setChecked(app->locale().numberOptions() & QLocale::OmitGroupSeparator);
	boxMuParserCLocale->setChecked(app->d_muparser_c_locale);

	translationsPathLine->setText(QDir::toNativeSeparators(app->d_translations_folder));
	helpPathLine->setText(QDir::toNativeSeparators(QFileInfo(app->helpFilePath).dir().absolutePath()));
	texCompilerPathBox->setText(QDir::toNativeSeparators(app->d_latex_compiler_path));
	sofficePathBox->setText(QDir::toNativeSeparators(app->d_soffice_path));
	javaPathBox->setText(QDir::toNativeSeparators(app->d_java_path));
	jodconverterPathBox->setText(QDir::toNativeSeparators(app->d_jodconverter_path));

#ifdef SCRIPTING_PYTHON
	pythonConfigDirLine->setText(QDir::toNativeSeparators(app->d_python_config_folder));
	pythonScriptsDirLine->setText(QDir::toNativeSeparators(app->d_startup_scripts_folder));
#endif

	QNetworkProxy proxy = QNetworkProxy::applicationProxy();
	proxyGroupBox->setChecked(!proxy.hostName().isEmpty());
	proxyHostLine->setText(proxy.hostName());
	proxyPortBox->setValue(proxy.port());
	proxyUserNameLine->setText(proxy.user());
}

void GeneralConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	app->changeAppFont(appFont);
	app->changeAppStyle(boxStyle->currentText());
	app->autoSearchUpdates = boxSearchUpdates->isChecked();
	app->setSaveSettings(boxSave->isChecked(), boxMinutes->value());
	app->d_backup_files = boxBackupProject->isChecked();
	app->defaultScriptingLang = boxScriptingLanguage->currentText();
	app->d_init_window_type = (ApplicationWindow::WindowType)boxInitWindow->currentIndex();
	app->setMatrixUndoStackSize(undoStackSizeBox->value());
	app->d_eol = (ApplicationWindow::EndLineChar)boxEndLine->currentIndex();
    app->enableCompletion(completionBox->isChecked());
	app->d_open_last_project = openLastProjectBox->isChecked();
	app->setExcelImportMethod((ApplicationWindow::ExcelImportMethod)excelImportMethod->currentIndex());

	app->d_decimal_digits = boxAppPrecision->value();
	QLocale locale;
	switch (boxDecimalSeparator->currentIndex()){
		case 0:
			locale = QLocale::system();
		break;
		case 1:
			locale = QLocale::c();
		break;
		case 2:
			locale = QLocale(QLocale::German);
		break;
		case 3:
			locale = QLocale(QLocale::French);
		break;
	}
	if (boxThousandsSeparator->isChecked())
		locale.setNumberOptions(QLocale::OmitGroupSeparator);

    QLocale oldLocale = app->locale();
    app->setLocale(locale);
	QLocale::setDefault(locale);
	app->d_muparser_c_locale = boxMuParserCLocale->isChecked();

	if (appTabWidget->currentWidget() == numericFormatPage){
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QList<MdiSubWindow *> windows = app->windowsList();
		for (MdiSubWindow *w : windows){
			w->setLocale(locale);

			if(w->inherits("Table"))
				((Table *)w)->updateDecimalSeparators(oldLocale);
			else if(w->inherits("Matrix"))
				((Matrix *)w)->resetView();
		}

		switch (boxClipboardLocale->currentIndex()){
			case 0:
				app->setClipboardLocale(QLocale::system());
			break;
			case 1:
				app->setClipboardLocale(QLocale::c());
			break;
			case 2:
				app->setClipboardLocale(QLocale(QLocale::German));
			break;
			case 3:
				app->setClipboardLocale(QLocale(QLocale::French));
			break;
		}

		app->modifiedProject();
		QApplication::restoreOverrideCursor();
	}

	if (appTabWidget->currentWidget() == fileLocationsPage){
		QString path = translationsPathLine->text();
		if (path != app->d_translations_folder && validFolderPath(path)){
			app->d_translations_folder = QFileInfo(path).absoluteFilePath();
			app->createLanguagesList();
			insertLanguagesList();
		}

		if (validFolderPath(helpPathLine->text())){
			path = helpPathLine->text() + "/index.html";
			if (path != app->helpFilePath){
				QFileInfo fi(path);
				if (fi.exists() && fi.isFile())
					app->helpFilePath = fi.absoluteFilePath();
				else
					QMessageBox::critical(this, tr("QtiPlot - index.html File Not Found!"),
					tr("There is no file called <b>index.html</b> in folder %1.<br>Please choose another folder!").
					arg(helpPathLine->text()));
			}
		}

#ifdef SCRIPTING_PYTHON
		path = pythonConfigDirLine->text();
		if (path != app->d_python_config_folder && validFolderPath(path))
			app->d_python_config_folder = QFileInfo(path).absoluteFilePath();

		path = pythonScriptsDirLine->text();
		if (path != app->d_startup_scripts_folder && validFolderPath(path))
			app->d_startup_scripts_folder = QFileInfo(path).absoluteFilePath();
#endif
	}

	if (appTabWidget->currentWidget() == proxyPage){
		setApplicationCustomProxy();
	}

	app->d_inform_rename_table = boxPromptRenameTables->isChecked();
	app->d_confirm_overwrite = boxConfirmOverwrite->isChecked();
	app->d_confirm_modif_2D_points = boxConfirmModifyDataPoints->isChecked();
	app->confirmCloseFolder = boxFolders->isChecked();
	app->updateConfirmOptions(boxTables->isChecked(), boxMatrices->isChecked(),
			boxPlots2D->isChecked(), boxPlots3D->isChecked(),
			boxNotes->isChecked());

	app->setAppColors(btnWorkspace->color(), btnPanels->color(), btnPanelsText->color());
}

void GeneralConfigPage::retranslateUi()
{
	appTabWidget->setTabText(appTabWidget->indexOf(application), tr("Application"));
	appTabWidget->setTabText(appTabWidget->indexOf(confirm), tr("Confirmations"));
	appTabWidget->setTabText(appTabWidget->indexOf(appColors), tr("Colors"));
	appTabWidget->setTabText(appTabWidget->indexOf(numericFormatPage), tr("Numeric Format"));
	appTabWidget->setTabText(appTabWidget->indexOf(fileLocationsPage), tr("File Locations"));
	appTabWidget->setTabText(appTabWidget->indexOf(proxyPage), tr("Internet Connection"));

	lblLanguage->setText(tr("Language"));
	lblStyle->setText(tr("Main Window &Style"));
	lblFonts->setText(tr("Fonts"));
	fontsBtn->setText(tr("&Choose font..."));
	lblScriptingLanguage->setText(tr("Default Scripting &Language"));
    lblUndoStackSize->setText(tr("&Undo Stack Size"));
	lblEndOfLine->setText(tr("End of line"));
	boxEndLine->clear();
	boxEndLine->addItem(tr("LF (Unix)"));
	boxEndLine->addItem(tr("CRLF (Windows)"));
	boxEndLine->addItem(tr("CR (Mac)"));
	boxEndLine->setCurrentIndex(d_app ? (int)d_app->d_eol : 0);

	lblInitWindow->setText(tr("&Initial Window Type"));
	boxInitWindow->clear();
	boxInitWindow->addItem(tr("Folder"));
	boxInitWindow->addItem(tr("Table"));
	boxInitWindow->addItem(tr("Matrix"));
	boxInitWindow->addItem(tr("Empty"));
	boxInitWindow->addItem(tr("Note"));
	boxInitWindow->setCurrentIndex(d_app ? (int)d_app->d_init_window_type : 0);

	boxSave->setText(tr("&Save every"));
	boxMinutes->setSuffix(" " + tr("minutes"));
	boxBackupProject->setText(tr("&Backup project file before saving"));
	boxSearchUpdates->setText(tr("Check &updates at startup"));
	completionBox->setText(tr("Enable &Auto-completion"));
	openLastProjectBox->setText(tr("Open &last project at startup"));

	excelImportMethodLabel->setText(tr("Excel Import Method"));
	excelImportMethod->clear();
	excelImportMethod->addItem(tr("Native"));
#if defined(Q_OS_WIN)
	excelImportMethod->addItem(tr("OLE Automation"));
#endif
	excelImportMethod->addItem(tr("OpenOffice / LibreOffice"));

	groupBoxConfirm->setTitle(tr("Prompt on closing"));
	boxFolders->setText(tr("Folders"));
	boxTables->setText(tr("Tables"));
	boxPlots3D->setText(tr("3D Plots"));
	boxPlots2D->setText(tr("2D Plots"));
	boxMatrices->setText(tr("Matrices"));
	boxNotes->setText(tr("Notes"));
	boxPromptRenameTables->setText(tr("Prompt on &renaming tables"));
	boxTablePasteDialog->setText(tr("Show &Prompt on Paste to Multiple Cells"));
	boxConfirmOverwrite->setText(tr("Confirm &overwrite"));
	boxConfirmModifyDataPoints->setText(tr("Confirm modifying 2D plot data points"));

	lblWorkspace->setText(tr("Workspace"));
	lblPanels->setText(tr("Panels"));
	lblPanelsText->setText(tr("Panels Text"));

	lblAppPrecision->setText(tr("Decimal Digits"));
	lblDecimalSeparator->setText(tr("Decimal Separators"));
	boxThousandsSeparator->setText(tr("Omit Group Separator"));
	lblClipboardSeparator->setText(tr("Clipboard Decimal Separators"));
	boxMuParserCLocale->setText(tr("Use C Locale for muParser"));

	lblTranslationsPath->setText(tr("Translations Path"));
	lblHelpPath->setText(tr("Help Path"));
	texCompilerLabel->setText(tr("LaTeX Compiler"));
	officeLabel->setText(tr("LibreOffice Path"));
	javaLabel->setText(tr("Java Path"));
	jodconverterLabel->setText(tr("JODConverter Path"));

#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir->setText(tr("Python Configuration Path"));
	lblPythonScriptsDir->setText(tr("Startup Scripts Path"));
#endif

	proxyGroupBox->setTitle(tr("&Proxy"));
	proxyHostLabel->setText(tr("Host"));
	proxyPortLabel->setText(tr("Port"));
	proxyUserLabel->setText(tr("Username"));
	proxyPasswordLabel->setText(tr("Password"));

	insertLanguagesList();
}

void GeneralConfigPage::pickApplicationFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, appFont, this);
	if (ok){
		appFont = font;
		fontsBtn->setText(font.family() + " " + QString::number(font.pointSize()) + "pt");
	}
}

void GeneralConfigPage::switchToLanguage(int param)
{
	if (d_app)
		d_app->switchToLanguage(param);
}

void GeneralConfigPage::insertLanguagesList()
{
	if (!d_app)
		return;

	boxLanguage->clear();
	QString qmPath = d_app->d_translations_folder;
	QDir dir(qmPath);
	QStringList locales = d_app->locales;
	QStringList languages;
	int lang = 0;
	for (int i = 0; i < locales.size(); i++){
		if (locales[i] == "en")
			languages.push_back("English");
		else {
			QTranslator translator;
			(void)translator.load("qtiplot_" + locales[i], qmPath);

			QString language = translator.translate("ApplicationWindow", "English");
			if (!language.isEmpty())
				languages.push_back(language);
			else
				languages.push_back(locales[i]);
		}

		if (locales[i] == d_app->appLanguage)
			lang = i;
	}
	boxLanguage->addItems(languages);
	boxLanguage->setCurrentIndex(lang);
}

bool GeneralConfigPage::validFolderPath(const QString& path)
{
	QFileInfo fi(path);
	return (fi.exists() && fi.isDir());
}

void GeneralConfigPage::chooseTranslationsFolder()
{
	if (!d_app)
		return;

	QFileInfo tfi(d_app->d_translations_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the QtiPlot translations folder!"),
		tfi.absoluteFilePath(), QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty()){
		d_app->d_translations_folder = QDir::toNativeSeparators(dir);
		translationsPathLine->setText(d_app->d_translations_folder);
		d_app->createLanguagesList();
		insertLanguagesList();
	}
}

void GeneralConfigPage::chooseHelpFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the help folder"), helpPathLine->text());
	if (!dir.isEmpty())
		helpPathLine->setText(QDir::toNativeSeparators(dir));
}

#ifdef SCRIPTING_PYTHON
void GeneralConfigPage::choosePythonConfigFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Python configuration folder"), pythonConfigDirLine->text());
	if (!dir.isEmpty())
		pythonConfigDirLine->setText(QDir::toNativeSeparators(dir));
}

void GeneralConfigPage::chooseStartupScriptsFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose startup scripts folder"), pythonScriptsDirLine->text());
	if (!dir.isEmpty())
		pythonScriptsDirLine->setText(QDir::toNativeSeparators(dir));
}

void GeneralConfigPage::showStartupScriptsFolder(const QString & s)
{
	Q_UNUSED(s);
}
#endif

void GeneralConfigPage::chooseTexCompiler()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Choose LaTeX compiler"), texCompilerPathBox->text());
	if (!file.isEmpty())
		texCompilerPathBox->setText(QDir::toNativeSeparators(file));
}

bool GeneralConfigPage::validateTexCompiler()
{
	return true;
}

void GeneralConfigPage::chooseOffice()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Choose LibreOffice / OpenOffice executable"), sofficePathBox->text());
	if (!file.isEmpty())
		sofficePathBox->setText(QDir::toNativeSeparators(file));
}

bool GeneralConfigPage::validateOffice()
{
	return true;
}

void GeneralConfigPage::chooseJava()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Choose Java executable"), javaPathBox->text());
	if (!file.isEmpty())
		javaPathBox->setText(QDir::toNativeSeparators(file));
}

bool GeneralConfigPage::validateJava()
{
	return true;
}

void GeneralConfigPage::chooseJODConverter()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Choose JODConverter JAR file"), jodconverterPathBox->text(), tr("JAR files (*.jar)"));
	if (!file.isEmpty())
		jodconverterPathBox->setText(QDir::toNativeSeparators(file));
}

bool GeneralConfigPage::validateJODConverter()
{
	return true;
}

QNetworkProxy GeneralConfigPage::setApplicationCustomProxy()
{
	QNetworkProxy proxy;
	proxy.setType(QNetworkProxy::NoProxy);
	if (proxyGroupBox->isChecked())
		proxy.setHostName(proxyHostLine->text());
	else
		proxy.setHostName(QString());

	proxy.setPort(proxyPortBox->value());
	proxy.setUser(proxyUserNameLine->text());
	proxy.setPassword(proxyPasswordLine->text());
	QNetworkProxy::setApplicationProxy(proxy);
	return proxy;
}
