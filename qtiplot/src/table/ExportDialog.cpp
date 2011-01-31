/***************************************************************************
    File                 : ExportDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Export ASCII dialog

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
#include "ExportDialog.h"
#include "ApplicationWindow.h"
#include <MdiSubWindow.h>
#include <Matrix.h>

#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>

ExportDialog::ExportDialog(MdiSubWindow *window, QWidget * parent, bool extended, Qt::WFlags flags)
: ExtensibleFileDialog( parent, extended, flags ), d_window(window)
{
	setWindowTitle( tr( "QtiPlot - Export ASCII" ) );
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled( true );
	setAcceptMode(QFileDialog::AcceptSave);

	initAdvancedOptions();
	setExtensionWidget((QWidget *)d_advanced_options);

	setFileTypeFilters();
	setFileMode(QFileDialog::AnyFile);
	if (d_window){
		boxTable->setCurrentIndex(boxTable->findText(d_window->objectName()));
		selectFile(d_window->objectName());
	}

#if QT_VERSION >= 0x040300
	connect(this, SIGNAL(filterSelected ( const QString & )),
			this, SLOT(updateAdvancedOptions ( const QString & )));
#else
	QList<QComboBox*> combo_boxes = findChildren<QComboBox*>();
	if (combo_boxes.size() >= 2)
		connect(combo_boxes[1], SIGNAL(currentIndexChanged ( const QString & )),
				this, SLOT(updateAdvancedOptions ( const QString & )));
#endif

	selectNameFilter(((ApplicationWindow *)parent)->d_export_ASCII_file_filter);
	updateAdvancedOptions(selectedNameFilter());
}

void ExportDialog::initAdvancedOptions()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	d_advanced_options = new QGroupBox();

	QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Table")), 0, 0);
	boxTable = new QComboBox();
	QStringList tables = app->tableNames() + app->matrixNames();
	boxTable->addItems(tables);

	boxTable->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	gl1->addWidget(boxTable, 0, 1);

	boxAllTables = new QCheckBox(tr( "&All" ));
    boxAllTables->setChecked(false);
	gl1->addWidget(boxAllTables, 0, 2);

    separatorLbl = new QLabel( tr( "Separator" ) );
	gl1->addWidget(separatorLbl, 1, 0);

    boxSeparator = new QComboBox();
	boxSeparator->addItem(tr("TAB"));
    boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem(";" + tr("SPACE"));
	boxSeparator->addItem("," + tr("SPACE"));
    boxSeparator->addItem(";");
    boxSeparator->addItem(",");
	boxSeparator->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	boxSeparator->setEditable( true );
	gl1->addWidget(boxSeparator, 1, 1);
	setColumnSeparator(app->d_export_col_separator);

	buttonHelp = new QPushButton(tr( "&Help" ));
	gl1->addWidget( buttonHelp, 1, 2);

	QString help = tr("The column separator can be customized. The following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: 0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	separatorLbl->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	separatorLbl->setToolTip(help);

	boxNames = new QCheckBox(tr( "Include Column &Names" ));
    boxNames->setChecked( app->d_export_col_names );
	boxNames->setVisible(d_window && d_window->inherits("Table"));

	boxComments = new QCheckBox(tr( "Include Column Co&mments" ));
    boxComments->setChecked( app->d_export_col_comment );
	boxComments->setVisible(d_window && d_window->inherits("Table"));

    boxSelection = new QCheckBox(tr( "Export &Selection" ));
    boxSelection->setChecked( app->d_export_table_selection );

	QVBoxLayout *vl1 = new QVBoxLayout(d_advanced_options);
	vl1->addLayout( gl1 );
	vl1->addWidget( boxNames );
	vl1->addWidget( boxComments );
	vl1->addWidget( boxSelection );

    // signals and slots connections
    connect( boxTable, SIGNAL(activated(const QString &)), this, SLOT(updateOptions(const QString &)));
    connect( buttonHelp, SIGNAL(clicked()), this, SLOT(help()));
	connect( boxAllTables, SIGNAL(toggled(bool)), this, SLOT( enableTableName(bool)));
}

void ExportDialog::updateAdvancedOptions (const QString & filter)
{
	bool on = !filter.contains(".tex") && !filter.contains(".odf") && !filter.contains(".html") &&
			  !filter.contains(".xls") && !filter.contains(".ods") && !filter.contains(".csv");
	separatorLbl->setVisible(on);
	boxSeparator->setVisible(on);
	buttonHelp->setVisible(on);
}

void ExportDialog::help()
{
	QString s = tr("The column separator can be customized. The following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	s += "\n"+tr("The separator must not contain the following characters: 0-9eE.+-");
	QMessageBox::about((ApplicationWindow *)parent(), tr("QtiPlot - Help"), s);
}

void ExportDialog::enableTableName(bool ok)
{
	QString selected_filter = selectedNameFilter();
	boxTable->setEnabled(!ok);
	if (!ok){
		setFileMode(QFileDialog::AnyFile);

		if (d_window){
			boxTable->setCurrentIndex(boxTable->findText(d_window->objectName()));
			selectFile(d_window->objectName());
		}
	} else
		setFileMode(QFileDialog::Directory);

	setFileTypeFilters();
	selectNameFilter(selected_filter);
}

void ExportDialog::setFileTypeFilters()
{
	QList<QByteArray> list;
	list << "CSV";
	list << "DAT";
	list << "TXT";
	list << "TEX";
#if QT_VERSION >= 0x040500
	list << "ODF";
	list << "HTML";
#endif
	list << "ODS";
	list << "XLS";

	QStringList filters;
	for(int i = 0 ; i < list.count() ; i++)
		filters << "*." + list[i].toLower();

	filters.sort();
	setFilters(filters);
}

void ExportDialog::accept()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");
	sep.replace("\\t", "\t");

	if (sep.contains(QRegExp("[0-9.eE+-]"))){
		QMessageBox::warning(0, tr("QtiPlot - Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	app->asciiDirPath = directory().path();
	if (selectedFiles().isEmpty())
		return;

	QString selected_filter = selectedNameFilter().remove("*");
	if (boxAllTables->isChecked())
		app->exportAllTables(directory().absolutePath(), selected_filter, sep, boxNames->isChecked(), boxComments->isChecked(), boxSelection->isChecked());
	else {
		QString file_name = selectedFiles()[0];
		if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
			file_name.append(selected_filter);

		if (app->d_confirm_overwrite && QFileInfo(file_name).exists() &&
			QMessageBox::warning(this, tr("QtiPlot") + " - " + tr("Overwrite file?"),
			tr("%1 already exists.").arg(file_name) + "\n" + tr("Do you want to replace it?"),
			QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
			return;

		QFile file(file_name);
		if ( !file.open( QIODevice::WriteOnly ) ){
			QMessageBox::critical(this, tr("QtiPlot - Export error"),
					tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(file_name));
			return;
		}
		file.close();

		MdiSubWindow* w = app->window(boxTable->currentText());
		if (!w)
			return;

		if (w->inherits("Table"))
			((Table *)w)->exportASCII(file_name, sep, boxNames->isChecked(), boxComments->isChecked(), boxSelection->isChecked());
		else if (qobject_cast<Matrix *>(w))
			((Matrix *)w)->exportASCII(file_name, sep, boxSelection->isChecked());
	}

	close();
}

void ExportDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep==" ")
		boxSeparator->setCurrentIndex(1);
	else if (sep==";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep==",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep=="; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep==", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep==";")
		boxSeparator->setCurrentIndex(6);
	else if (sep==",")
		boxSeparator->setCurrentIndex(7);
	else {
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

void ExportDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (app){
		app->d_export_col_names = boxNames->isChecked();
		app->d_export_table_selection = boxSelection->isChecked();
		app->d_export_col_comment = boxComments->isChecked();
		app->d_export_ASCII_file_filter = selectedNameFilter();

		QString sep = boxSeparator->currentText();
		sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);
		sep.replace(tr("SPACE"), " ");
		sep.replace("\\s", " ");
		sep.replace("\\t", "\t");
		app->d_export_col_separator = sep;
	}
	e->accept();
}

void ExportDialog::updateOptions(const QString & name)
{
    ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
        return;

    MdiSubWindow* w = app->window(name);
    if (!w)
		return;

    boxComments->setVisible(w->inherits("Table"));
    boxNames->setVisible(w->inherits("Table"));
}
