/***************************************************************************
    File                 : ExportDialog.h
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
#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <ExtensibleFileDialog.h>

class QPushButton;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class MdiSubWindow;

//! Export ASCII dialog
class ExportDialog : public ExtensibleFileDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param window window to be exported
	 * \param parent parent widget
	 * \param extended flag: show/hide the advanced options on start-up
	 * \param fl window flags
	 */
    ExportDialog(MdiSubWindow *window = nullptr, QWidget* parent = nullptr, bool extended = true, Qt::WindowFlags fl = {} );

private:
	//! Pointer to the window to be exported.
	MdiSubWindow *d_window = nullptr;

	void closeEvent(QCloseEvent *e) override;
	//! Create #d_advanced_options and everything it contains.
	void initAdvancedOptions();
	void setFileTypeFilters();
	//! Container widget for all advanced options.
	QGroupBox *d_advanced_options = nullptr;

	QPushButton* buttonHelp = nullptr;
    QCheckBox* boxNames = nullptr;
    QCheckBox* boxComments = nullptr;
    QCheckBox* boxSelection = nullptr;
	QCheckBox* boxAllTables = nullptr;
    QComboBox* boxSeparator = nullptr;
	QComboBox* boxTable = nullptr;
	QLabel *separatorLbl = nullptr;

public slots:
	//! Set the column delimiter
	void setColumnSeparator(const QString& sep);
	//! Enable/disable export options depending if the selected window is a Table or a Matrix.
	void updateOptions(const QString & name);
	//! Update which options are visible and enabled based on the output format.
	void updateAdvancedOptions (const QString &filter);

private slots:
	//! Enable/disable the tables combox box
	/**
	 * The tables combo box is disabled when
	 * the checkbox "all" is selected.
	 */
	void enableTableName(bool ok);

protected slots:
	//! Accept changes
	void accept() override;
	//! Display help
	void help();
};

#endif // ExportDialog_H
