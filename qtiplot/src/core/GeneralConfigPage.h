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

    ApplicationWindow *d_app;

    QTabWidget *appTabWidget;
    QWidget *application;
    QWidget *confirm;
    QWidget *appColors;
    QWidget *numericFormatPage;
    QWidget *fileLocationsPage;
    QWidget *proxyPage;

    // App tab
    QLabel *lblLanguage;
    QComboBox *boxLanguage;
    QLabel *lblStyle;
    QComboBox *boxStyle;
    QLabel *lblFonts;
    QPushButton *fontsBtn;
    QLabel *lblScriptingLanguage;
    QComboBox *boxScriptingLanguage;
    QLabel *lblUndoStackSize;
    QSpinBox *undoStackSizeBox;
    QLabel *lblEndOfLine;
    QComboBox *boxEndLine;
    QLabel *lblInitWindow;
    QComboBox *boxInitWindow;
    QCheckBox *boxSave;
    QSpinBox *boxMinutes;
    QCheckBox *boxBackupProject;
    QCheckBox *boxSearchUpdates;
    QCheckBox *completionBox;
    QCheckBox *openLastProjectBox;
    QLabel *excelImportMethodLabel;
    QComboBox *excelImportMethod;

    // Confirmations tab
    QGroupBox *groupBoxConfirm;
    QCheckBox *boxFolders;
    QCheckBox *boxTables;
    QCheckBox *boxMatrices;
    QCheckBox *boxPlots2D;
    QCheckBox *boxPlots3D;
    QCheckBox *boxNotes;
    QCheckBox *boxPromptRenameTables;
    QCheckBox *boxTablePasteDialog;
    QCheckBox *boxConfirmOverwrite;
    QCheckBox *boxConfirmModifyDataPoints;

    // Colors tab
    QLabel *lblWorkspace;
    ColorButton *btnWorkspace;
    QLabel *lblPanels;
    ColorButton *btnPanels;
    QLabel *lblPanelsText;
    ColorButton *btnPanelsText;

    // Numeric format tab
    QLabel *lblAppPrecision;
    QSpinBox *boxAppPrecision;
    QLabel *lblDecimalSeparator;
    QComboBox *boxDecimalSeparator;
    QCheckBox *boxThousandsSeparator;
    QLabel *lblClipboardSeparator;
    QComboBox *boxClipboardLocale;
    QCheckBox *boxMuParserCLocale;

    // File locations tab
    QLabel *lblTranslationsPath;
    QLineEdit *translationsPathLine;
    QPushButton *browseTranslationsBtn;
    QLabel *lblHelpPath;
    QLineEdit *helpPathLine;
    QPushButton *browseHelpBtn;
    QLabel *texCompilerLabel;
    QLineEdit *texCompilerPathBox;
    QPushButton *browseTexCompilerBtn;
    QLabel *officeLabel;
    QLineEdit *sofficePathBox;
    QPushButton *browseOfficeBtn;
    QLabel *javaLabel;
    QLineEdit *javaPathBox;
    QPushButton *browseJavaBtn;
    QLabel *jodconverterLabel;
    QLineEdit *jodconverterPathBox;
    QPushButton *browseJODConverterBtn;

#ifdef SCRIPTING_PYTHON
    QLabel *lblPythonConfigDir;
    QLineEdit *pythonConfigDirLine;
    QPushButton *browsePythonConfigBtn;
    QLabel *lblPythonScriptsDir;
    QLineEdit *pythonScriptsDirLine;
    QPushButton *browsePythonScriptsBtn;
#endif

    // Proxy tab
    QGroupBox *proxyGroupBox;
    QLabel *proxyHostLabel;
    QLineEdit *proxyHostLine;
    QLabel *proxyPortLabel;
    QSpinBox *proxyPortBox;
    QLabel *proxyUserLabel;
    QLineEdit *proxyUserNameLine;
    QLabel *proxyPasswordLabel;
    QLineEdit *proxyPasswordLine;

    QFont appFont;
};

#endif // GENERAL_CONFIG_PAGE_H
