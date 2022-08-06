/***************************************************************************
    File                 : MatrixValuesDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Set matrix values dialog

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
#include "MatrixValuesDialog.h"
#include "MatrixCommand.h"
#include <ApplicationWindow.h>
#include "muParserScripting.h"

#include <QLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetSelectionRange>
#ifdef SCRIPTING_PYTHON
#include <QCheckBox>
#endif

MatrixValuesDialog::MatrixValuesDialog( ScriptingEnv *env, QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl ), scripted(env)
{
    setName( "MatrixValuesDialog" );
	setWindowTitle( tr( "QtiPlot - Set Matrix Values" ) );
	setSizeGripEnabled(true);
    setAttribute(Qt::WA_DeleteOnClose);

	QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("For row (i)")), 0, 0);
	startRow = new QSpinBox();
	startRow->setRange(1, INT_MAX);
	gl1->addWidget(startRow, 0, 1);
	gl1->addWidget(new QLabel(tr("to")), 0, 2);
	endRow =  new QSpinBox();
	endRow->setRange(1, INT_MAX);
	gl1->addWidget(endRow, 0, 3);
	gl1->addWidget(new QLabel(tr("For col (j)")), 1, 0);
	startCol = new QSpinBox();
	startCol->setRange(1, INT_MAX);
	gl1->addWidget(startCol, 1, 1);
	gl1->addWidget(new QLabel(tr("to")), 1, 2);
	endCol = new QSpinBox();
	endCol->setRange(1, INT_MAX);
	gl1->addWidget(endCol, 1, 3);
	gl1->setColumnStretch(1, 1);
	gl1->setColumnStretch(3, 1);

	functions = new QComboBox();
	functions->setEditable(false);
	functions->addItems(scriptEnv->mathFunctions());
	btnAddFunction = new QPushButton(tr( "Add &Function" ));
	btnAddCell = new QPushButton(tr( "Add Ce&ll" ));

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(functions, 1);
	hbox1->addWidget(btnAddFunction);
	hbox1->addWidget(btnAddCell);

	QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addLayout(gl1);
	vbox1->addLayout(hbox1);
	vbox1->addStretch();
	QGroupBox *gb = new QGroupBox();
    gb->setLayout(vbox1);
    gb->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));

	explain = new QTextEdit();
	explain->setReadOnly(true);
	explain->setMaximumHeight(100);
	explain->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));
    QPalette palette = explain->palette();
    palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
    explain->setPalette(palette);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(explain);
	hbox2->addWidget(gb);

	QHBoxLayout *hbox3 = new QHBoxLayout();

	commands = new ScriptEdit( scriptEnv);
	commands->setTabStopWidth(((ApplicationWindow *)parent)->d_notes_tab_length);
    commands->setFont(((ApplicationWindow *)parent)->d_notes_font);
	commands->setFocus();
	hbox3->addWidget(commands);

	QVBoxLayout *vbox2 = new QVBoxLayout();
	btnApply = new QPushButton(tr( "&Apply" ));
    vbox2->addWidget(btnApply);

	buttonProperties = new QPushButton();
	buttonProperties->setIcon(QIcon(":/configure.png"));
	buttonProperties->setToolTip(tr("Open Properties Dialog"));
	vbox2->addWidget(buttonProperties);

	btnCancel = new QPushButton(tr( "&Close" ));
	btnCancel->setIcon(QIcon(":/delete.png"));
    vbox2->addWidget(btnCancel);
    vbox2->addStretch();

	hbox3->addLayout(vbox2);

	QVBoxLayout* vbox3 = new QVBoxLayout(this);
	vbox3->addLayout(hbox2);
#ifdef SCRIPTING_PYTHON
	boxMuParser = NULL;
	if (scriptEnv->name() != QString("muParser")){
		boxMuParser = new QCheckBox(tr("Use built-in muParser (much faster)"));
		boxMuParser->setChecked(((ApplicationWindow *)parent)->d_force_muParser);
		connect(boxMuParser, SIGNAL(toggled(bool)), this, SLOT(updateFunctionsList(bool)));
		updateFunctionsList(boxMuParser->isChecked());
		vbox3->addWidget(boxMuParser);
	}
#endif
	vbox3->addWidget(new QLabel(tr( "Cell(i,j)=" )));
	vbox3->addLayout(hbox3, 1);

	insertExplain(0);

	connect(btnAddCell, SIGNAL(clicked()), this, SLOT(addCell()));
	connect(btnAddFunction, SIGNAL(clicked()), this, SLOT(insertFunction()));
	connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
	connect(buttonProperties, SIGNAL(clicked()), this, SLOT(openPropertiesDialog()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(functions, SIGNAL(activated(int)), this, SLOT(insertExplain(int)));
}

void MatrixValuesDialog::openPropertiesDialog()
{
	ApplicationWindow *app = qobject_cast<ApplicationWindow *>(this->parent());
	if (!app)
		return;

	connect(this, SIGNAL(destroyed()), app, SLOT(showMatrixDialog()));
	this->apply();
	this->close();
}

QSize MatrixValuesDialog::sizeHint() const
{
	return QSize( 400, 190 );
}

void MatrixValuesDialog::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

bool MatrixValuesDialog::apply()
{
	QString formula = commands->text();
	QString oldFormula = matrix->formula();

	matrix->setFormula(formula);

	bool useMuParser = true;
#ifdef SCRIPTING_PYTHON
	if (boxMuParser)
		useMuParser = boxMuParser->isChecked();
#endif

	if (matrix->canCalculate(useMuParser)){
		matrix->undoStack()->push(new MatrixSetFormulaCommand(matrix, oldFormula, formula,
							tr("Set New Formula") + " \"" + formula + "\""));

		if (matrix->calculate(startRow->value()-1, endRow->value()-1,
			startCol->value()-1, endCol->value()-1, useMuParser))
			return true;
	}

	matrix->setFormula(oldFormula);
	return false;
}

void MatrixValuesDialog::setMatrix(Matrix* m)
{
    if (!m)
        return;

	matrix = m;
	commands->setText(m->formula());
	commands->setContext(m);

    endCol->setValue(m->numCols());
    endRow->setValue(m->numRows());

    if (m->viewType() == Matrix::TableView){
        QItemSelectionModel *selModel = m->selectionModel();
        if (selModel->hasSelection()){
            QItemSelectionRange selection = selModel->selection().first();
            if (selection.width() > 1 || selection.height() > 1){
                startCol->setValue(selection.left()+1);
                startRow->setValue(selection.top()+1);
                endCol->setValue(selection.right()+1);
                endRow->setValue(selection.bottom()+1);
            }
        }
    }
}

void MatrixValuesDialog::insertExplain(int index)
{
#ifdef SCRIPTING_PYTHON
	if (boxMuParser && boxMuParser->isChecked())
		explain->setText(muParserScripting::explainFunction(functions->text(index)));
	else
		explain->setText(scriptEnv->mathFunctionDoc(functions->text(index)));
#else
	explain->setText(scriptEnv->mathFunctionDoc(functions->text(index)));
#endif
}

void MatrixValuesDialog::insertFunction()
{
	commands->insertFunction(functions->currentText());
}

void MatrixValuesDialog::addCell()
{
	commands->insert("cell(i, j)");
}

void MatrixValuesDialog::setCompleter(QCompleter *completer)
{
    if (!completer)
        return;

    commands->setCompleter(completer);
}

void MatrixValuesDialog::closeEvent(QCloseEvent* e)
{
#ifdef SCRIPTING_PYTHON
	if (boxMuParser){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		if (app)
			app->d_force_muParser = boxMuParser->isChecked();
	}
#endif
	e->accept();
}

#ifdef SCRIPTING_PYTHON
void MatrixValuesDialog::updateFunctionsList(bool muParser)
{
	functions->clear();
	if (muParser)
		functions->addItems(muParserScripting::functionsList());
	else
		functions->addItems(scriptingEnv()->mathFunctions());

	if (functions->count() > 0)
		insertExplain(0);
}
#endif
