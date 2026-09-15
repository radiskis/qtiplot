/***************************************************************************
    File                 : ErrDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Add error bars dialog

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
#ifndef ERRDIALOG_H
#define ERRDIALOG_H

#include <QDialog>
#include <QList>
class QLabel;
class QComboBox;
class QRadioButton;
class DoubleSpinBox;
class QPushButton;
class QGroupBox;
class QButtonGroup;
class MdiSubWindow;

//! Add error bars dialog
class ErrDialog : public QDialog
{
	Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
	ErrDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

private:
	QLabel* textLabel1 = nullptr;
	QComboBox* nameLabel = nullptr, *tableNamesBox = nullptr, *colNamesBox = nullptr;
	QGroupBox *groupBox2 = nullptr;
	QGroupBox *groupBox1 = nullptr, *groupBox3 = nullptr;
	QButtonGroup *buttonGroup1 = nullptr, *buttonGroup2 = nullptr;
	QRadioButton* standardBox = nullptr, *columnBox = nullptr;
	QRadioButton* percentBox = nullptr;
	QRadioButton* standardErrorBox = nullptr;
	DoubleSpinBox* valueBox = nullptr;
	QRadioButton* xErrBox = nullptr;
	QRadioButton* yErrBox = nullptr;
	QPushButton* buttonAdd = nullptr;
	QPushButton* buttonCancel = nullptr;
	QList<MdiSubWindow*> srcTables;

protected slots:
	//! Set all string in the current language
	virtual void languageChange();

public slots:
	//! Add a plot definition
	void add();
	//! Supply the dialog with a curves list
	void setCurveNames(const QStringList& names);
	//! Supply the dialog with a tables list
	void setSrcTables(QList<MdiSubWindow *> tables);
	//! Select a table
	void selectSrcTable(int tabnr);
};

#endif // ERRDIALOG_H
