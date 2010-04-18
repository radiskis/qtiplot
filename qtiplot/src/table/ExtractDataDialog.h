/***************************************************************************
	File                 : ExtractDataDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Extract data values dialog

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
#ifndef EXTRACTDATADIALOG_H
#define EXTRACTDATADIALOG_H

#include <ScriptingEnv.h>
#include <Script.h>
#include <QDialog>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QCompleter;
class Table;
class ScriptingEnv;
class ScriptEdit;


//! Set column values dialog
class ExtractDataDialog : public QDialog, public scripted
{
    Q_OBJECT

public:
	ExtractDataDialog( ScriptingEnv *env, QWidget* parent = 0, Qt::WFlags fl = 0 );
	void setTable(Table* w);
	void setCompleter(QCompleter *);

private slots:
	void apply();
	void insertFunction();
	void insertCol();
	void insertOp();
	void insertExplain(int index);
	void clearFormulas();

private:
	Table* table;

	QSize sizeHint() const ;

    QComboBox* functions;
    QComboBox* boxColumn;
	QComboBox *boxOperators;

    QPushButton* btnAddFunction;
    QPushButton* btnAddCol;
    QPushButton* btnCancel;
    QPushButton *btnApply;
	QPushButton* buttonClearFormulas;
	QPushButton* btnAddOp;

    ScriptEdit* commands;
    QTextEdit* explain;
	QSpinBox* start, *end;
	QLineEdit *destNameBox;
};

#endif //
