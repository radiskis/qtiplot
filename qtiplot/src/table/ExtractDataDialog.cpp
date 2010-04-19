/***************************************************************************
	File                 : ExtractDataDialog.cpp
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
#include "ExtractDataDialog.h"
#include "Table.h"
#include <ScriptEdit.h>
#include <ApplicationWindow.h>
#include "muParserScripting.h"

#include <QTableWidget>
#include <QTableWidgetSelectionRange>
#include <QList>
#include <QLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QTextCursor>
#include <QCompleter>
#include <QLineEdit>

ExtractDataDialog::ExtractDataDialog( ScriptingEnv *env, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), scripted(env)
{
	setName( "ExtractDataDialog" );
	setWindowTitle( tr( "QtiPlot") + " - " + tr("Extract Data"));
	setSizeGripEnabled(true);
    setAttribute(Qt::WA_DeleteOnClose);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(new QLabel(tr("For row (i)")));
	start = new QSpinBox();
	start->setMinValue(1);
    start->setMaxValue(INT_MAX);
	hbox1->addWidget(start);

	hbox1->addWidget(new QLabel(tr("to")));

	end = new QSpinBox();
	end->setMinValue(1);
    end->setMaxValue(INT_MAX);
	hbox1->addWidget(end);

	QGridLayout *gl1 = new QGridLayout();
	functions = new QComboBox();
	functions->addItems(muParserScripting::functionsList(true));
	gl1->addWidget(functions, 0, 0);
	btnAddFunction = new QPushButton(tr( "Add &function" ));
	gl1->addWidget(btnAddFunction, 0, 1);
	boxColumn = new QComboBox();
	gl1->addWidget(boxColumn, 1, 0);
	btnAddCol = new QPushButton(tr( "Add co&lumn" ));
	gl1->addWidget(btnAddCol, 1, 1);
	boxOperators = new QComboBox();
	boxOperators->addItems(QStringList() << ">" << ">=" << "<" << "<=" << "==" << "!=");
	gl1->addWidget(boxOperators, 2, 0);
	btnAddOp = new QPushButton(tr( "Add &operator" ));
	gl1->addWidget(btnAddOp, 2, 1);

	QGroupBox *gb = new QGroupBox();
	QVBoxLayout *vbox1 = new QVBoxLayout();
	vbox1->addLayout(hbox1);
	vbox1->addLayout(gl1);
	gb->setLayout(vbox1);
	gb->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));

	explain = new QTextEdit();
	explain->setReadOnly (true);
	explain->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));
	QPalette palette = explain->palette();
	palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
	explain->setPalette(palette);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(explain);
	hbox2->addWidget(gb);

	commands = new ScriptEdit(scriptEnv);
	commands->setTabStopWidth(((ApplicationWindow *)parent)->d_notes_tab_length);
    commands->setFont(((ApplicationWindow *)parent)->d_notes_font);

	QVBoxLayout *vbox2 = new QVBoxLayout();
	btnApply = new QPushButton(tr( "&Apply" ));
	vbox2->addWidget(btnApply);
	buttonClearFormulas = new QPushButton(tr("Clea&r" ));
	vbox2->addWidget(buttonClearFormulas);
	btnCancel = new QPushButton(tr( "&Close" ));
	vbox2->addWidget(btnCancel);
	vbox2->addStretch();

	QHBoxLayout *hbox4 = new QHBoxLayout();
	hbox4->addWidget(commands);
	hbox4->addLayout(vbox2);

	QHBoxLayout *hbox5 = new QHBoxLayout();
	QLabel *l = new QLabel(tr("&Put into table"));
	hbox5->addWidget(l);

	destNameBox = new QLineEdit;
	destNameBox->setText(tr("Table"));
	l->setBuddy(destNameBox);
	hbox5->addWidget(destNameBox);

	QVBoxLayout* vbox3 = new QVBoxLayout();
	vbox3->addLayout(hbox2);
	vbox3->addLayout(hbox5);

	QLabel *l2 = new QLabel(tr("Cond&ition:"));
	l2->setBuddy(commands);
	vbox3->addWidget(l2);
	vbox3->addLayout(hbox4);

	setLayout(vbox3);
	setFocusProxy (commands);
	commands->setFocus();

	if (functions->count() > 0)
		insertExplain(0);

	connect(btnAddFunction, SIGNAL(clicked()),this, SLOT(insertFunction()));
	connect(btnAddCol, SIGNAL(clicked()),this, SLOT(insertCol()));
	connect(btnAddOp, SIGNAL(clicked()),this, SLOT(insertOp()));
	connect(btnApply, SIGNAL(clicked()),this, SLOT(apply()));
	connect(btnCancel, SIGNAL(clicked()),this, SLOT(close()));
	connect(functions, SIGNAL(activated(int)),this, SLOT(insertExplain(int)));
	connect(buttonClearFormulas, SIGNAL(clicked()), this, SLOT(clearFormulas()));
}

QSize ExtractDataDialog::sizeHint() const
{
	return QSize( 400, 190 );
}

void ExtractDataDialog::apply()
{
	Table *t = table->extractData(destNameBox->text(), commands->text(), start->value() - 1, end->value() - 1);
	if (!t)
		return;

	t->show();
}

void ExtractDataDialog::insertExplain(int index)
{

	explain->setText(muParserScripting::explainFunction(functions->text(index)));
}

void ExtractDataDialog::insertFunction()
{
	commands->insertFunction(functions->currentText());
}

void ExtractDataDialog::insertCol()
{
	commands->insert(boxColumn->currentText());
}

void ExtractDataDialog::insertOp()
{
	commands->insert(boxOperators->currentText());
}

void ExtractDataDialog::setTable(Table* w)
{
	table = w;
	QStringList colNames = w->colNames();
	int cols = w->numCols();
	for (int i=0; i<cols; i++)
		boxColumn->insertItem("col(\""+colNames[i]+"\")", i);

	int s = w->table()->currentSelection();
	if (s >= 0) {
		Q3TableSelection sel = w->table()->selection(s);
		w->setSelectedCol(sel.leftCol());

		start->setValue(sel.topRow() + 1);
		end->setValue(sel.bottomRow() + 1);
	} else {
		start->setValue(1);
		end->setValue(w->numRows());
	}

	commands->setContext(w);
}

void ExtractDataDialog::setCompleter(QCompleter *completer)
{
    if (!completer)
        return;

    commands->setCompleter(completer);
}

void ExtractDataDialog::clearFormulas()
{
	if (!table)
		return;

	table->clearCommands();
	commands->clear();
}
