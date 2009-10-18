/***************************************************************************
    File                 : FindReplaceDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Find/Replace dialog for ScriptEdit

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
#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextDocument>

class QPushButton;
class QCheckBox;
class QComboBox;
class ScriptEdit;

//! Find/Replace dialog
class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    FindReplaceDialog(ScriptEdit *editor, bool replace = false, QWidget* parent = 0, Qt::WFlags fl = 0 );

private:
	QTextDocument::FindFlags searchFlags();

	ScriptEdit *d_editor;

	QPushButton* buttonNext;
	QPushButton* buttonPrevious;
	QPushButton* buttonReplace;
	QPushButton* buttonReplaceAll;
	QPushButton* buttonCancel;

	QComboBox* boxFind;
	QComboBox* boxReplace;

	QCheckBox* boxCaseSensitive;
	QCheckBox* boxWholeWords;

protected slots:
	bool find(bool previous = false);
	bool findPrevious(){return find(true);};
	void replace();
	void replaceAll();
};

#endif
