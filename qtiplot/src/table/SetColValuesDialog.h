/***************************************************************************
    File                 : SetColValuesDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Set column values dialog

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
#ifndef VALUESDIALOG_H
#define VALUESDIALOG_H

#include <ScriptingEnv.h>
#include <Script.h>
#include <QDialog>

class QComboBox;
class QTextEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QCompleter;
#ifdef SCRIPTING_PYTHON
class QCheckBox;
#endif
class Table;
class ScriptingEnv;
class ScriptEdit;


//! Set column values dialog
class SetColValuesDialog : public QDialog, public scripted
{
    Q_OBJECT

public:
    SetColValuesDialog( ScriptingEnv *env, QWidget* parent = nullptr, Qt::WindowFlags fl = {} );
	void setTable(Table* w);
	void setCompleter(QCompleter *);

private slots:
	bool apply();
	void prevColumn();
	void nextColumn();
	void openColumnPropertiesDialog();
	void insertFunction();
	void insertCol();
	void insertCell();
	void insertExplain(int index);
	void updateColumn(int sc);
	void clearFormulas();
#ifdef SCRIPTING_PYTHON
	void updateFunctionsList(bool);
#endif

private:
	Table* table = nullptr;

	QSize sizeHint() const override;
	void customEvent( QEvent *e ) override;
	void closeEvent(QCloseEvent *e) override;

    QComboBox* functions = nullptr;
    QComboBox* boxColumn = nullptr;
    QPushButton* btnAddFunction = nullptr;
    QPushButton* btnAddCol = nullptr;
    QPushButton* btnCancel = nullptr;
    QPushButton *buttonPrev = nullptr;
    QPushButton *buttonNext = nullptr;
	QPushButton *buttonProperties = nullptr;
    QPushButton *addCellButton = nullptr;
    QPushButton *btnApply = nullptr;
	QPushButton* buttonClearFormulas = nullptr;
    ScriptEdit* commands = nullptr;
    QTextEdit* explain = nullptr;
	QSpinBox* start = nullptr;
	QSpinBox* end = nullptr;
	QLabel *colNameLabel = nullptr;
#ifdef SCRIPTING_PYTHON
	QCheckBox *boxMuParser = nullptr;
#endif
};

#endif //
