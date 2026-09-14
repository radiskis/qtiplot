/***************************************************************************
    File                 : GeneralConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : General application preferences page

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
#ifndef GENERAL_CONFIG_PAGE_H
#define GENERAL_CONFIG_PAGE_H

#include "ConfigPage.h"
#include <QFont>
#include <QNetworkProxy>

class ApplicationWindow;
class ApplicationSettings;

class QTabWidget;
class QWidget;
class QLabel;
class QComboBox;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;
class ColorButton;

class GeneralConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit GeneralConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("General"); }
    QIcon pageIcon() const override { return QIcon(":/general.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

private slots:
    void pickApplicationFont();
    void switchToLanguage(int param);
    void insertLanguagesList();

    void chooseTranslationsFolder();
    void chooseHelpFolder();
#ifdef SCRIPTING_PYTHON
    void choosePythonConfigFolder();
    void chooseStartupScriptsFolder();
    void showStartupScriptsFolder(const QString &);
#endif
    void chooseTexCompiler();
    bool validateTexCompiler();
    void chooseOffice();
    bool validateOffice();
    void chooseJava();
    bool validateJava();
    void chooseJODConverter();
    bool validateJODConverter();

private:
    void initConfirmationsTab();
    void initColorsTab();
    void initNumericFormatTab();
    void initFileLocationsTab();
    void initProxyTab();

    bool validFolderPath(const QString& path);
    QNetworkProxy setApplicationCustomProxy();

    ApplicationWindow *d_app = nullptr;

    QTabWidget *appTabWidget = nullptr;
    QWidget *application = nullptr;
    QWidget *confirm = nullptr;
    QWidget *appColors = nullptr;
    QWidget *numericFormatPage = nullptr;
    QWidget *fileLocationsPage = nullptr;
    QWidget *proxyPage = nullptr;

    // App tab
    QLabel *lblLanguage = nullptr;
    QComboBox *boxLanguage = nullptr;
    QLabel *lblStyle = nullptr;
    QComboBox *boxStyle = nullptr;
    QLabel *lblFonts = nullptr;
    QPushButton *fontsBtn = nullptr;
    QLabel *lblScriptingLanguage = nullptr;
    QComboBox *boxScriptingLanguage = nullptr;
    QLabel *lblUndoStackSize = nullptr;
    QSpinBox *undoStackSizeBox = nullptr;
    QLabel *lblEndOfLine = nullptr;
    QComboBox *boxEndLine = nullptr;
    QLabel *lblInitWindow = nullptr;
    QComboBox *boxInitWindow = nullptr;
    QCheckBox *boxSave = nullptr;
    QSpinBox *boxMinutes = nullptr;
    QCheckBox *boxBackupProject = nullptr;
    QCheckBox *boxSearchUpdates = nullptr;
    QCheckBox *completionBox = nullptr;
    QCheckBox *openLastProjectBox = nullptr;
    QLabel *excelImportMethodLabel = nullptr;
    QComboBox *excelImportMethod = nullptr;

    // Confirmations tab
    QGroupBox *groupBoxConfirm = nullptr;
    QCheckBox *boxFolders = nullptr;
    QCheckBox *boxTables = nullptr;
    QCheckBox *boxMatrices = nullptr;
    QCheckBox *boxPlots2D = nullptr;
    QCheckBox *boxPlots3D = nullptr;
    QCheckBox *boxNotes = nullptr;
    QCheckBox *boxPromptRenameTables = nullptr;
    QCheckBox *boxTablePasteDialog = nullptr;
    QCheckBox *boxConfirmOverwrite = nullptr;
    QCheckBox *boxConfirmModifyDataPoints = nullptr;

    // Colors tab
    QLabel *lblWorkspace = nullptr;
    ColorButton *btnWorkspace = nullptr;
    QLabel *lblPanels = nullptr;
    ColorButton *btnPanels = nullptr;
    QLabel *lblPanelsText = nullptr;
    ColorButton *btnPanelsText = nullptr;

    // Numeric format tab
    QLabel *lblAppPrecision = nullptr;
    QSpinBox *boxAppPrecision = nullptr;
    QLabel *lblDecimalSeparator = nullptr;
    QComboBox *boxDecimalSeparator = nullptr;
    QCheckBox *boxThousandsSeparator = nullptr;
    QLabel *lblClipboardSeparator = nullptr;
    QComboBox *boxClipboardLocale = nullptr;
    QCheckBox *boxMuParserCLocale = nullptr;

    // File locations tab
    QLabel *lblTranslationsPath = nullptr;
    QLineEdit *translationsPathLine = nullptr;
    QPushButton *browseTranslationsBtn = nullptr;
    QLabel *lblHelpPath = nullptr;
    QLineEdit *helpPathLine = nullptr;
    QPushButton *browseHelpBtn = nullptr;
    QLabel *texCompilerLabel = nullptr;
    QLineEdit *texCompilerPathBox = nullptr;
    QPushButton *browseTexCompilerBtn = nullptr;
    QLabel *officeLabel = nullptr;
    QLineEdit *sofficePathBox = nullptr;
    QPushButton *browseOfficeBtn = nullptr;
    QLabel *javaLabel = nullptr;
    QLineEdit *javaPathBox = nullptr;
    QPushButton *browseJavaBtn = nullptr;
    QLabel *jodconverterLabel = nullptr;
    QLineEdit *jodconverterPathBox = nullptr;
    QPushButton *browseJODConverterBtn = nullptr;

#ifdef SCRIPTING_PYTHON
    QLabel *lblPythonConfigDir = nullptr;
    QLineEdit *pythonConfigDirLine = nullptr;
    QPushButton *browsePythonConfigBtn = nullptr;
    QLabel *lblPythonScriptsDir = nullptr;
    QLineEdit *pythonScriptsDirLine = nullptr;
    QPushButton *browsePythonScriptsBtn = nullptr;
#endif

    // Proxy tab
    QGroupBox *proxyGroupBox = nullptr;
    QLabel *proxyHostLabel = nullptr;
    QLineEdit *proxyHostLine = nullptr;
    QLabel *proxyPortLabel = nullptr;
    QSpinBox *proxyPortBox = nullptr;
    QLabel *proxyUserLabel = nullptr;
    QLineEdit *proxyUserNameLine = nullptr;
    QLabel *proxyPasswordLabel = nullptr;
    QLineEdit *proxyPasswordLine = nullptr;

    QFont appFont;
};

#endif // GENERAL_CONFIG_PAGE_H
