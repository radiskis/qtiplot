/***************************************************************************
    File                 : ImportASCIIDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
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

#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRegExp>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include <QApplication>
#include <QProgressDialog>
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

	d_current_path = QString::null;

	initAdvancedOptions();
	d_import_mode->setEnabled(import_mode_enabled);
	setExtensionWidget(d_advanced_options);

	// get rembered option values
	ApplicationWindow *app = (ApplicationWindow *)parent;
	setLocale(app->locale());
	
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
	d_preview_button->setChecked(app->d_ASCII_import_preview);
	d_preview_table->setNumericPrecision(app->d_decimal_digits);
	if (!app->d_ASCII_import_preview)
		d_preview_table->hide();

    connect(d_preview_lines_box, SIGNAL(valueChanged(int)), this, SLOT(preview()));
    connect(d_rename_columns, SIGNAL(clicked()), this, SLOT(preview()));
    connect(d_import_comments, SIGNAL(clicked()), this, SLOT(preview()));
    connect(d_strip_spaces, SIGNAL(clicked()), this, SLOT(preview()));
    connect(d_simplify_spaces, SIGNAL(clicked()), this, SLOT(preview()));
    connect(d_ignored_lines, SIGNAL(valueChanged(int)), this, SLOT(preview()));
    connect(d_import_dec_separators, SIGNAL(clicked()), this, SLOT(preview()));
    connect(d_column_separator, SIGNAL(currentIndexChanged(int)), this, SLOT(preview()));
    connect(boxDecimalSeparator, SIGNAL(currentIndexChanged(int)), this, SLOT(preview()));
    connect(d_comment_string, SIGNAL(textChanged(const QString&)), this, SLOT(preview()));
    connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(changePreviewFile(const QString&)));
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

	d_preview_button = new QCheckBox(tr("&Preview Lines"));
	connect(d_preview_button, SIGNAL(clicked()), this, SLOT(preview()));
	advanced_layout->addWidget(d_preview_button, 5, 0);

	d_preview_lines_box = new QSpinBox();
	d_preview_lines_box->setMaximum (INT_MAX);
	d_preview_lines_box->setValue(100);
	d_preview_lines_box->setSingleStep(10);
	d_preview_lines_box->setSpecialValueText(tr("All"));
	advanced_layout->addWidget(d_preview_lines_box, 5, 1);

    d_read_only = new QCheckBox(tr("Import as &read-only"));
	advanced_layout->addWidget(d_read_only, 5, 2);

	d_help_button = new QPushButton(tr("&Help"));
	connect(d_help_button, SIGNAL(clicked()), this, SLOT(displayHelp()));
	advanced_layout->addWidget(d_help_button, 5, 3);

	d_preview_table = new PreviewTable(30, 2, this);
	main_layout->addWidget(d_preview_table);
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
		app->d_ASCII_import_preview = d_preview_button->isChecked();
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
    if (!d_preview_button->isChecked()){
        d_preview_table->hide();
        return;
    }

	if (d_current_path.trimmed().isEmpty()){
		d_preview_table->clear();
		d_preview_table->resetHeader();
        return;
    }

	QString fileName = d_current_path;
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
							d_simplify_spaces->isChecked(), d_import_comments->isChecked(),
                            d_comment_string->text());

	if (d_import_dec_separators->isChecked())
		d_preview_table->updateDecimalSeparators(decimalSeparators());
    if (!d_preview_table->isVisible())
        d_preview_table->show();

	tempFile.close();
}

void ImportASCIIDialog::changePreviewFile(const QString& path)
{
	if (path.isEmpty() || !QFileInfo(path).exists())
		return;

	d_current_path = path;
	preview();
}

/*****************************************************************************
 *
 * Class PreviewTable
 *
 *****************************************************************************/

PreviewTable::PreviewTable(int numRows, int numCols, QWidget * parent, const char * name)
:Q3Table(numRows, numCols, parent, name)
{
	setAttribute(Qt::WA_DeleteOnClose);	
	setSelectionMode(Q3Table::NoSelection);
	setReadOnly(true);
	setRowMovingEnabled(false);
	setColumnMovingEnabled(false);
	verticalHeader()->setResizeEnabled(false);
	
	for (int i=0; i<numCols; i++){
		comments << "";
		col_label << QString::number(i+1);
	}
	setHeader();
#ifdef Q_OS_MAC
	setMinimumHeight(4*horizontalHeader()->height());
#else
	setMinimumHeight(2*horizontalHeader()->height());
#endif
}

void PreviewTable::importASCII(const QString &fname, const QString &sep, int ignoredLines, bool renameCols,
    bool stripSpaces, bool simplifySpaces, bool importComments, const QString& commentString)
{
	QFile f(fname);
	if (f.open(QIODevice::ReadOnly)) //| QIODevice::Text | QIODevice::Unbuffered ))
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        Q3TextStream t(&f);//TODO: use QTextStream instead and find a way to make it read the end-of-line char correctly.
                         //Opening the file with the above combination doesn't seem to help: problems on Mac OS X generated ASCII files!

		int c, rows = 0, cols = 0;
		for (int i=0; i<ignoredLines; i++)
			t.readLine();

		QString s = t.readLine();//read first line after the ignored ones
		while (!t.atEnd()){
            if (commentString.isEmpty() || !s.startsWith(commentString)){
                rows++;
                break;
            } else
                s = t.readLine();
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}

		while (!t.atEnd()){
			QString aux = t.readLine();
			if (commentString.isEmpty() || !aux.startsWith(commentString))
                rows++;
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}

		if (simplifySpaces)
			s = s.simplifyWhiteSpace();
		else if (stripSpaces)
			s = s.stripWhiteSpace();

		QStringList line = s.split(sep);
		cols = (int)line.count();

		bool allNumbers = true;
		for (int i=0; i<cols; i++)
		{//verify if the strings in the line used to rename the columns are not all numbers
			locale().toDouble(line[i], &allNumbers);
			if (!allNumbers)
				break;
		}

		if (renameCols && !allNumbers)
			rows--;
        if (importComments)
            rows--;

		int steps = int(rows/1000);

		QProgressDialog progress(this);
		progress.setWindowTitle("Qtiplot - Reading file...");
		progress.setLabelText(fname);
		progress.setActiveWindow();
		progress.setAutoClose(true);
		progress.setAutoReset(true);
		progress.setRange(0, steps+1);

		QApplication::restoreOverrideCursor();

		QStringList oldHeader;
		if (numRows() != rows)
			setNumRows(rows);

		c = numCols();
		oldHeader = col_label;
		if (c != cols){
			if (c < cols)
				addColumns(cols - c);
			else{
				setNumCols(cols);
                for (int i=c-1; i>=cols; i--){
                	comments.removeLast();            
					col_label.removeLast();
				}
			}
		}

		f.reset();
		for (int i=0; i<ignoredLines; i++)
			t.readLine();

		if (renameCols && !allNumbers){//use first line to set the table header
			s = t.readLine();//read first line after the ignored ones
			while (!t.atEnd()){
            	if (commentString.isEmpty() || !s.startsWith(commentString))
                	break;
             	else
                	s = t.readLine();//ignore all commented lines
				qApp->processEvents(QEventLoop::ExcludeUserInput);
			}

			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.stripWhiteSpace();
			line = s.split(sep, QString::SkipEmptyParts);
			for (int i=0; i<(int)line.count(); i++)
  	        	col_label[i] = QString::null;

			for (int i=0; i<(int)line.count(); i++){
			    if (!importComments)
                    comments[i] = line[i];
				s = line[i].replace("-","_").remove(QRegExp("\\W")).replace("_","-");
				int n = col_label.count(s);
				if(n){//avoid identical col names
					while (col_label.contains(s+QString::number(n)))
						n++;
					s += QString::number(n);
				}
				col_label[i] = s;
			}
		}

        if (importComments){//import comments
            s = t.readLine();//read 2nd line after the ignored ones
            while (!t.atEnd()){
                if (commentString.isEmpty() || !s.startsWith(commentString))
                	break;
             	else
					s = t.readLine();//ignore all commented lines
                qApp->processEvents(QEventLoop::ExcludeUserInput);
            }

            if (simplifySpaces)
                s = s.simplifyWhiteSpace();
            else if (stripSpaces)
                s = s.stripWhiteSpace();
            line = s.split(sep, QString::SkipEmptyParts);
			for (int i=0; i<(int)line.count(); i++)
                comments[i] = line[i];
        }

		setHeader();

		int start = 0;
		for (int i=0; i<steps; i++){
			if (progress.wasCanceled()){
				f.close();
				return;
			}

			start = i*1000;
			for (int k=0; k<1000; k++){
				s = t.readLine();
				if (!commentString.isEmpty() && s.startsWith(commentString)){
				    k--;
                    continue;
				}

				if (simplifySpaces)
					s = s.simplifyWhiteSpace();
				else if (stripSpaces)
					s = s.stripWhiteSpace();
				line = s.split(sep);
				int lc = (int)line.count();
				if (lc > cols){
					addColumns(lc - cols);
					cols = lc;
				}
				for (int j=0; j<cols && j<lc; j++)
					setText(start + k, j, line[j]);
			}
			progress.setValue(i);
			qApp->processEvents();
		}

		start = steps*1000;
		for (int i=start; i<rows; i++){
			s = t.readLine();
			if (!commentString.isEmpty() && s.startsWith(commentString)){
			    i--;
                continue;
			}

			if (simplifySpaces)
				s = s.simplifyWhiteSpace();
			else if (stripSpaces)
				s = s.stripWhiteSpace();
			line = s.split(sep);
			int lc = (int)line.count();
			if (lc > cols) {
				addColumns(lc - cols);
				cols = lc;
			}
			for (int j=0; j<cols && j<lc; j++)
				setText(i, j, line[j]);
		}
		progress.setValue(steps+1);
		qApp->processEvents();
		f.close();
	}
}

void PreviewTable::resetHeader()
{
	for (int i=0; i<numCols(); i++){
	    comments[i] = QString::null;
		col_label[i] = QString::number(i+1);
	}
}

void PreviewTable::clear()
{
	for (int i=0; i<numCols(); i++){
		for (int j=0; j<numRows(); j++)
			setText(j, i, QString::null);
	}
}

void PreviewTable::updateDecimalSeparators(const QLocale& oldSeparators)
{		
	QLocale locale = ((QWidget *)parent())->locale();
	for (int i=0; i<numCols(); i++){
        for (int j=0; j<numRows(); j++){
            if (!text(j, i).isEmpty()){
				double val = oldSeparators.toDouble(text(j, i));
                setText(j, i, locale.toString(val, 'g', d_numeric_precision));
			}
		}
	}
}

void PreviewTable::setHeader()
{
	Q3Header *head = horizontalHeader();
	for (int i=0; i<numCols(); i++){
		QString s = col_label[i];
		int lines = columnWidth(i)/head->fontMetrics().averageCharWidth();
	#ifdef Q_OS_MAC
		head->setLabel(i, s.remove("\n"));
	#else
		head->setLabel(i, s.remove("\n") + "\n" + QString(lines, '_') + "\n" + comments[i]);
	#endif
	} 
}

void PreviewTable::addColumns(int c)
{
	int max=0, cols = numCols();
	for (int i=0; i<cols; i++){
		if (!col_label[i].contains(QRegExp ("\\D"))){
			int index=col_label[i].toInt();
			if (index>max)
				max=index;
		}
	}
	max++;
	insertColumns(cols, c);
	for (int i=0; i<c; i++){
		comments << QString();
		col_label<< QString::number(max+i);
	}
}
