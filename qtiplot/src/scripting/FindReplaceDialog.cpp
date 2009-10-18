/***************************************************************************
    File                 : FindReplaceDialog.cpp
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
#include "FindReplaceDialog.h"
#include "ScriptEdit.h"
#include <ApplicationWindow.h>
#include <Folder.h>

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QRegExp>
#include <QLabel>
#include <QLayout>
#include <QGroupBox>
#include <QMessageBox>

FindReplaceDialog::FindReplaceDialog(ScriptEdit *editor, bool replace, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), d_editor(editor)
{
	setWindowTitle (tr("QtiPlot") + " - " + tr("Find"));
	setSizeGripEnabled( true );

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout * topLayout = new QGridLayout(gb1);

	topLayout->addWidget( new QLabel(tr( "Find" )), 0, 0);
	boxFind = new QComboBox();
	boxFind->setEditable(true);
	boxFind->setDuplicatesEnabled(false);
	boxFind->setInsertPolicy( QComboBox::InsertAtTop );
	boxFind->setAutoCompletion(true);
	boxFind->setMaxCount ( 10 );
	boxFind->setMaxVisibleItems ( 10 );
	boxFind->setMinimumWidth(250);
	boxFind->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

	if (editor->textCursor().hasSelection()){
		QString text = editor->textCursor().selectedText();
		boxFind->setEditText(text);
		boxFind->addItem(text);
	}

	topLayout->addWidget(boxFind, 0, 1);

	if (replace){
		setWindowTitle (tr("QtiPlot") + " - " + tr("Find and Replace"));
		topLayout->addWidget(new QLabel(tr( "Replace with" )), 1, 0);
		boxReplace = new QComboBox();
		boxReplace->setEditable(true);
		boxReplace->setDuplicatesEnabled(false);
		boxReplace->setInsertPolicy( QComboBox::InsertAtTop );
		boxReplace->setAutoCompletion(true);
		boxReplace->setMaxCount ( 10 );
		boxReplace->setMaxVisibleItems ( 10 );
		boxReplace->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
		topLayout->addWidget( boxReplace, 1, 1);
		topLayout->setColumnStretch(1, 10);
	}

	QGroupBox *gb2 = new QGroupBox();
	QGridLayout * bottomLayout = new QGridLayout(gb2);

	boxCaseSensitive = new QCheckBox(tr("&Match case"));
    boxCaseSensitive->setChecked(false);
	bottomLayout->addWidget( boxCaseSensitive, 0, 0);

	boxWholeWords = new QCheckBox(tr("&Whole word"));
    boxWholeWords->setChecked(false);
	bottomLayout->addWidget(boxWholeWords, 1, 0);
	bottomLayout->setRowStretch(2, 1);

    QVBoxLayout *vb1 = new QVBoxLayout();
    vb1->addWidget(gb1);
	vb1->addWidget(gb2);

	QVBoxLayout *vb2 = new QVBoxLayout();

    buttonNext = new QPushButton(tr("&Next"));
    buttonNext->setDefault(true);
	vb2->addWidget(buttonNext);

	buttonPrevious = new QPushButton(tr("&Previous"));
	vb2->addWidget(buttonPrevious);

	if (replace){
        buttonReplace = new QPushButton(tr("&Replace"));
        connect(buttonReplace, SIGNAL(clicked()), this, SLOT(replace()));
		vb2->addWidget(buttonReplace);

		buttonReplaceAll = new QPushButton(tr("Replace &all"));
		connect(buttonReplaceAll, SIGNAL(clicked()), this, SLOT(replaceAll()));
		vb2->addWidget(buttonReplaceAll);
	}

    buttonCancel = new QPushButton(tr("&Close"));
	vb2->addWidget(buttonCancel);
	vb2->addStretch();

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->addLayout(vb1);
	hb->addLayout(vb2);

    connect(buttonNext, SIGNAL(clicked()), this, SLOT(find()));
    connect(buttonPrevious, SIGNAL(clicked()), this, SLOT(findPrevious()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

bool FindReplaceDialog::find(bool previous)
{
	QString searchString = boxFind->currentText();
	if (searchString.isEmpty()){
        QMessageBox::warning(this, tr("Empty Search Field"),
                tr("The search field is empty. Please enter some text and try again."));
		boxFind->setFocus();
		return false;
    }

	if(boxFind->findText(searchString) == -1)
		boxFind->addItem (searchString);
	
	return d_editor->find(searchString, searchFlags(), previous);
}

void FindReplaceDialog::replace()
{
    QString searchString = boxFind->currentText();
	if (searchString.isEmpty()){
        QMessageBox::warning(this, tr("Empty Search Field"),
                tr("The search field is empty. Please enter some text and try again."));
		boxFind->setFocus();
		return;
    }

    QTextCursor cursor = d_editor->textCursor();
    if (!cursor.hasSelection() || cursor.selectedText() != searchString){
        find();//find and select next match
        return;
    }

    QString replaceString = boxReplace->currentText();
    cursor.insertText(replaceString);
    find();//find and select next match

    if(boxReplace->findText(replaceString) == -1)
        boxReplace->addItem(replaceString);
}

void FindReplaceDialog::replaceAll()
{
	QString searchString = boxFind->currentText();
	if (searchString.isEmpty()){
        QMessageBox::warning(this, tr("Empty Search Field"),
                tr("The search field is empty. Please enter some text and try again."));
		boxFind->setFocus();
		return;
    }

	if(boxFind->findText(searchString) == -1)
		boxFind->addItem (searchString);

	QString replaceString = boxReplace->currentText();
	if(boxReplace->findText(replaceString) == -1)
		boxReplace->addItem(replaceString);

	QTextDocument::FindFlags flags = searchFlags();
	QTextDocument *document = d_editor->document();
	QTextCursor cursor(document);
    bool found = false;
	while (!cursor.isNull() && !cursor.atEnd()){
		cursor = document->find(searchString, cursor, flags);
		if (!cursor.isNull()){
			found = true;
			cursor.insertText(replaceString);
		}
	}

	if (!found)
		QMessageBox::information(this, tr("QtiPlot"), tr("QtiPlot has finished searching the document."));
}

QTextDocument::FindFlags FindReplaceDialog::searchFlags()
{
	QTextDocument::FindFlags flags;
	if (boxCaseSensitive->isChecked())
		flags |= QTextDocument::FindCaseSensitively;
	if (boxWholeWords->isChecked())
		flags |= QTextDocument::FindWholeWords;
	return flags;
}
