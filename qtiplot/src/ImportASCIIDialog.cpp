/***************************************************************************
    File                 : ImportASCIIDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Import ASCII file(s) dialog

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

#include "ImportASCIIDialog.h"
#include "ApplicationWindow.h"
#include "Table.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRegExp>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include <Q3TextStream>

ImportASCIIDialog::ImportASCIIDialog(bool import_mode_enabled, QWidget * parent, bool extended, Qt::WFlags flags )
: ExtensibleFileDialog(parent, extended, flags )
{
	setWindowTitle(tr("QtiPlot - Import ASCII File(s)"));

	QStringList filters;
	filters << tr("All files") + " (*)";
	filters << tr("Text files") + " (*.TXT *.txt)";
	filters << tr("Data files") + " (*.DAT *.dat)";
	filters << tr("Comma Separated Values") + " (*.CSV *.csv)";
	setFilters( filters );

	setFileMode( QFileDialog::ExistingFiles );

	initAdvancedOptions();
	d_import_mode->setEnabled(import_mode_enabled);
	setExtensionWidget(d_advanced_options);

	// get rembered option values
	ApplicationWindow *app = (ApplicationWindow *)parent;
	d_strip_spaces->setChecked(app->strip_spaces);
	d_simplify_spaces->setChecked(app->simplify_spaces);
	d_ignored_lines->setValue(app->ignoredLines);
	d_rename_columns->setChecked(app->renameColumns);
	setColumnSeparator(app->columnSeparator);
    d_comment_string->setText(app->d_ASCII_comment_string);
    d_import_comments->setChecked(app->d_ASCII_import_comments);
    d_read_only->setChecked(app->d_ASCII_import_read_only);

	if (app->d_ASCII_import_locale.name() == QLocale::c().name())
        boxDecimalSeparator->setCurrentIndex(1);
    else if (app->d_ASCII_import_locale.name() == QLocale(QLocale::German).name())
        boxDecimalSeparator->setCurrentIndex(2);
    else if (app->d_ASCII_import_locale.name() == QLocale(QLocale::French).name())
        boxDecimalSeparator->setCurrentIndex(3);
	boxDecimalSeparator->setEnabled(app->d_import_dec_separators);
	d_import_dec_separators->setChecked(app->d_import_dec_separators);

	connect(d_import_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImportMode(int)));
	if (import_mode_enabled)
        d_import_mode->setCurrentIndex(app->d_ASCII_import_mode);
	d_preview_lines_box->setValue(app->d_preview_lines);
}

void ImportASCIIDialog::initAdvancedOptions()
{
	d_advanced_options = new QGroupBox();
	QVBoxLayout *main_layout = new QVBoxLayout(d_advanced_options);
	QGridLayout *advanced_layout = new QGridLayout();
	main_layout->addLayout(advanced_layout);

	advanced_layout->addWidget(new QLabel(tr("Import each file as: ")), 0, 0);
	d_import_mode = new QComboBox();
	// Important: Keep this in sync with the ImportMode enum.
	d_import_mode->addItem(tr("New Table"));
	d_import_mode->addItem(tr("New Columns"));
	d_import_mode->addItem(tr("New Rows"));
	d_import_mode->addItem(tr("Overwrite Current Table"));
	advanced_layout->addWidget(d_import_mode, 0, 1);

	QLabel *label_column_separator = new QLabel(tr("Separator:"));
	advanced_layout->addWidget(label_column_separator, 1, 0);
	d_column_separator = new QComboBox();
	d_column_separator->addItem(tr("TAB"));
	d_column_separator->addItem(tr("SPACE"));
	d_column_separator->addItem(";" + tr("TAB"));
	d_column_separator->addItem("," + tr("TAB"));
	d_column_separator->addItem(";" + tr("SPACE"));
	d_column_separator->addItem("," + tr("SPACE"));
	d_column_separator->addItem(";");
	d_column_separator->addItem(",");
	d_column_separator->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	d_column_separator->setEditable( true );
	advanced_layout->addWidget(d_column_separator, 1, 1);
	// context-sensitive help
	QString help_column_separator = tr("The column separator can be customized. \nThe following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help_column_separator += "\n"+tr("The separator must not contain the following characters: \n0-9eE.+-");
	d_column_separator->setWhatsThis(help_column_separator);
	label_column_separator->setToolTip(help_column_separator);
	d_column_separator->setToolTip(help_column_separator);
	label_column_separator->setWhatsThis(help_column_separator);

	QLabel *label_ignore_lines = new QLabel(tr( "Ignore first" ));
	advanced_layout->addWidget(label_ignore_lines, 2, 0);
	d_ignored_lines = new QSpinBox();
	d_ignored_lines->setRange( 0, 10000 );
	d_ignored_lines->setSuffix(" " + tr("lines"));
	d_ignored_lines->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	advanced_layout->addWidget(d_ignored_lines, 2, 1);

	advanced_layout->addWidget(new QLabel(tr("Ignore lines starting with")), 3, 0);
	d_comment_string = new QLineEdit();
    advanced_layout->addWidget(d_comment_string, 3, 1);

	d_rename_columns = new QCheckBox(tr("Use first row to &name columns"));
	advanced_layout->addWidget(d_rename_columns, 0, 2, 1, 2);

    d_import_comments = new QCheckBox(tr("Use second row as &comments"));
	advanced_layout->addWidget(d_import_comments, 1, 2, 1, 2);

	d_strip_spaces = new QCheckBox(tr("&Remove white spaces from line ends"));
	advanced_layout->addWidget(d_strip_spaces, 2, 2, 1, 2);
	// context-sensitive help
	QString help_strip_spaces = tr("By checking this option all white spaces will be \nremoved from the beginning and the end of \nthe lines in the ASCII file.","when translating this check the what's this functions and tool tips to place the '\\n's correctly");
	help_strip_spaces +="\n\n"+tr("Warning: checking this option leads to column \noverlaping if the columns in the ASCII file don't \nhave the same number of rows.");
	help_strip_spaces +="\n"+tr("To avoid this problem you should precisely \ndefine the column separator using TAB and \nSPACE characters.","when translating this check the what's this functions and tool tips to place the '\\n's correctly");
	d_strip_spaces->setWhatsThis(help_strip_spaces);
	d_strip_spaces->setToolTip(help_strip_spaces);

	d_simplify_spaces = new QCheckBox(tr("&Simplify white spaces" ));
	advanced_layout->addWidget(d_simplify_spaces, 3, 2, 1, 2);
	// context-sensitive help
	QString help_simplify_spaces = tr("By checking this option all white spaces will be \nremoved from the beginning and the end of the \nlines and each sequence of internal \nwhitespaces (including the TAB character) will \nbe replaced with a single space.","when translating this check the what's this functions and tool tips to place the '\\n's correctly");
	help_simplify_spaces +="\n\n"+tr("Warning: checking this option leads to column \noverlaping if the columns in the ASCII file don't \nhave the same number of rows.","when translating this check the what's this functions and tool tips to place the '\\n's correctly");
	help_simplify_spaces +="\n"+tr("To avoid this problem you should precisely \ndefine the column separator using TAB and \nSPACE characters.","when translating this check the what's this functions and tool tips to place the '\\n's correctly");
	d_simplify_spaces->setWhatsThis(help_simplify_spaces);
	d_simplify_spaces->setToolTip(help_simplify_spaces);

	advanced_layout->addWidget(new QLabel(tr("Decimal Separators")), 4, 0);
	boxDecimalSeparator = new QComboBox();
	boxDecimalSeparator->addItem(tr("System Locale Setting"));
	boxDecimalSeparator->addItem("1,000.0");
	boxDecimalSeparator->addItem("1.000,0");
	boxDecimalSeparator->addItem("1 000,0");
	advanced_layout->addWidget(boxDecimalSeparator, 4, 1);

	d_import_dec_separators = new QCheckBox(tr("Import &decimal separators"));
	connect(d_import_dec_separators, SIGNAL(toggled(bool)), boxDecimalSeparator, SLOT(setEnabled(bool)));
	advanced_layout->addWidget(d_import_dec_separators, 4, 2, 1, 2);

	QHBoxLayout *meta_options_layout = new QHBoxLayout();
	d_read_only = new QCheckBox(tr("Import as &read-only"));
	meta_options_layout->addWidget(d_read_only);
	d_help_button = new QPushButton(tr("&Help"));
	connect(d_help_button, SIGNAL(clicked()), this, SLOT(displayHelp()));
	meta_options_layout->addWidget(d_help_button);
	d_preview_button = new QPushButton(tr("&Preview"));
	connect(d_preview_button, SIGNAL(clicked()), this, SLOT(preview()));
	meta_options_layout->addWidget(d_preview_button);
	meta_options_layout->addWidget(new QLabel(tr("Lines")));
	d_preview_lines_box = new QSpinBox();
	d_preview_lines_box->setMaximum (INT_MAX);
	d_preview_lines_box->setValue(100);
	d_preview_lines_box->setSingleStep(10);
	d_preview_lines_box->setSpecialValueText(tr("All"));
	meta_options_layout->addWidget(d_preview_lines_box);
	meta_options_layout->addStretch();
	main_layout->addLayout(meta_options_layout);
	
	QHBoxLayout *preview_layout = new QHBoxLayout();
	d_preview_table = new Table(ScriptingLangManager::newEnv((ApplicationWindow *)parent()), 30, 2, tr("Preview"), 0, 0);
	d_preview_table->setAttribute(Qt::WA_DeleteOnClose);
	d_preview_table->showComments(true);
	int height = d_preview_table->table()->horizontalHeader()->height();
	d_preview_table->setMinimumHeight(2*height);
	d_preview_table->setMaximumHeight(5*height);
	preview_layout->addWidget(d_preview_table);
	main_layout->addLayout(preview_layout);
}

void ImportASCIIDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		d_column_separator->setCurrentIndex(0);
	else if (sep==" ")
		d_column_separator->setCurrentIndex(1);
	else if (sep==";\t")
		d_column_separator->setCurrentIndex(2);
	else if (sep==",\t")
		d_column_separator->setCurrentIndex(3);
	else if (sep=="; ")
		d_column_separator->setCurrentIndex(4);
	else if (sep==", ")
		d_column_separator->setCurrentIndex(5);
	else if (sep==";")
		d_column_separator->setCurrentIndex(6);
	else if (sep==",")
		d_column_separator->setCurrentIndex(7);
	else {
		QString separator = sep;
		d_column_separator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

const QString ImportASCIIDialog::columnSeparator() const
{
	QString sep = d_column_separator->currentText();

	if (d_simplify_spaces->isChecked())
		sep.replace(tr("TAB"), " ", Qt::CaseInsensitive);
	else
		sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);

	sep.replace(tr("SPACE"), " ", Qt::CaseInsensitive);
	sep.replace("\\s", " ");
	sep.replace("\\t", "\t");

	/* TODO
	if (sep.contains(QRegExp("[0-9.eE+-]")))
		QMessageBox::warning(this, tr("QtiPlot - Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
	*/

	return sep;
}

void ImportASCIIDialog::displayHelp()
{
	QString s = tr("The column separator can be customized. The following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	s += "\n"+tr("The separator must not contain the following characters: 0-9eE.+-") + "\n\n";
	s += tr( "Remove white spaces from line ends" )+ ":\n";
	s += tr("By checking this option all white spaces will be removed from the beginning and the end of the lines in the ASCII file.") + "\n\n";
	s += tr( "Simplify white spaces" )+ ":\n";
	s += tr("By checking this option each sequence of internal whitespaces (including the TAB character) will be replaced with a single space.");
	s += tr("By checking this option all white spaces will be removed from the beginning and the end of the lines and each sequence of internal whitespaces (including the TAB character) will be replaced with a single space.");

	s +="\n\n"+tr("Warning: using these two last options leads to column overlaping if the columns in the ASCII file don't have the same number of rows.");
	s +="\n"+tr("To avoid this problem you should precisely define the column separator using TAB and SPACE characters.");

	QMessageBox::about(this, tr("QtiPlot - Help"), s);
}

void ImportASCIIDialog::updateImportMode(int mode)
{
	if (mode == Overwrite)
		setFileMode( QFileDialog::ExistingFile );
	else
		setFileMode( QFileDialog::ExistingFiles );
}

void ImportASCIIDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (app){
		app->d_extended_import_ASCII_dialog = this->isExtended();
		app->d_ASCII_file_filter = this->selectedFilter();
		app->d_preview_lines = d_preview_lines_box->value();
	}

	e->accept();
}

QLocale ImportASCIIDialog::decimalSeparators()
{
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
	return locale;
}

void ImportASCIIDialog::preview()
{
	QString fileName = selectedFiles()[0];
	QTemporaryFile tempFile;
	int rows = d_preview_lines_box->value();
	if (rows){
		QFile dataFile(fileName);
		if(tempFile.open() && dataFile.open(QIODevice::ReadOnly)){
			Q3TextStream t(&dataFile);
			QTextStream tt(&tempFile);
			int i = 0;
			while(i<rows && !t.atEnd()){
				tt << t.readLine() + "\n";
				i++;			
			}
			fileName = tempFile.fileName();	
		}
	}
	 
	d_preview_table->resetHeader();
	d_preview_table->importASCII(fileName, columnSeparator(), d_ignored_lines->value(), 
							d_rename_columns->isChecked(), d_strip_spaces->isChecked(), 
							d_simplify_spaces->isChecked(), d_import_comments->isChecked(), false,
                            d_comment_string->text(), d_read_only->isChecked());
	
	if (d_import_dec_separators->isChecked())
		d_preview_table->updateDecimalSeparators(decimalSeparators());
	tempFile.close();			
}
