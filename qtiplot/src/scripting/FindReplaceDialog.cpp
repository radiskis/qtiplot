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
#include "../ApplicationWindow.h"
#include "../Folder.h"

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QRegExp>
#include <QLabel>
#include <QLayout>
#include <QGroupBox>
#include <QMessageBox>

FindReplaceDialog::FindReplaceDialog(ScriptEdit *editor, bool replace, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
	d_editor(editor), d_replace_mode(replace), d_counter(0)
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
	boxFind->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	
	highlightCursor = QTextCursor(editor->document()); 
	if (editor->textCursor().hasSelection()){
		QString text = editor->textCursor().selectedText();
		boxFind->setEditText(text);
		boxFind->addItem(text);
	}
		
	topLayout->addWidget(boxFind, 0, 1);

	if (d_replace_mode){
		setWindowTitle (tr("QtiPlot") + " - " + tr("Find and Replace"));
		lblReplace = new QLabel(tr( "Replace with" ));
	
		topLayout->addWidget( lblReplace, 1, 0 );
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
	
	boxCaseSensitive = new QCheckBox(tr("Case &Sensitive"));
    boxCaseSensitive->setChecked(false);
	bottomLayout->addWidget( boxCaseSensitive, 0, 0);
		
	boxWholeWords = new QCheckBox(tr("&Whole words"));
    boxWholeWords->setChecked(false);
	bottomLayout->addWidget(boxWholeWords, 1, 0);
	
	boxCircularSearch = new QCheckBox(tr("Circu&lar search"));
    boxCircularSearch->setChecked(true);
	bottomLayout->addWidget(boxCircularSearch, 2, 0);
	bottomLayout->setRowStretch(3, 1);
		
	QHBoxLayout *hb = new QHBoxLayout();
	hb->addStretch();
	
	if (d_replace_mode)
		buttonFind = new QPushButton(tr("&Replace"));
	else
		buttonFind = new QPushButton(tr("&Find"));
    buttonFind->setDefault( true );
	hb->addWidget(buttonFind);
   
	if (d_replace_mode){
		buttonReplaceAll = new QPushButton(tr("Replace &all"));
		connect(buttonReplaceAll, SIGNAL(clicked()), this, SLOT(replaceAll()));
		hb->addWidget(buttonReplaceAll);
	}
	
    buttonCancel = new QPushButton(tr("&Close"));
	hb->addWidget(buttonCancel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(gb1);
	mainLayout->addWidget(gb2);
	mainLayout->addLayout(hb);
   
	connect(boxFind, SIGNAL(editTextChanged(const QString &)), this, SLOT(resetSearch()));
	connect(boxCaseSensitive, SIGNAL(clicked()), this, SLOT(resetSearch()));
	connect(boxWholeWords, SIGNAL(clicked()), this, SLOT(resetSearch()));

    connect(buttonFind, SIGNAL(clicked()), this, SLOT(find()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void FindReplaceDialog::resetSearch()
{	
	d_counter = 0;
	highlightCursor = QTextCursor(d_editor->document());
}

void FindReplaceDialog::find()
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
		
    QTextDocument *document = d_editor->document();	
	QTextCursor cursor(document);
	QTextDocument::FindFlags flags = searchFlags();
	while (!highlightCursor.isNull() && !highlightCursor.atEnd()){
		highlightCursor = document->find(searchString, highlightCursor, flags);
		if (!highlightCursor.isNull()){
			d_counter++;	
			if (d_replace_mode){
				QString replaceString = boxReplace->currentText();
     			highlightCursor.insertText(replaceString);
				if(boxReplace->findText(replaceString) == -1)
					boxReplace->addItem(replaceString);
			}
			
			d_editor->setTextCursor(highlightCursor);
			return;
		}
	}

	if (!d_counter){
		QMessageBox::information(this, tr("Word Not Found"), 
				tr("Sorry, the expression cannot be found."));
	} else {
		if (boxCircularSearch->isChecked()){
			highlightCursor = QTextCursor(d_editor->document());
			if (!d_replace_mode)			
				find();
		} else
			QMessageBox::information(this, tr("End of document"), 
					tr("The end of the document was reached!"));
	} 
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
	
    QTextDocument *document = d_editor->document();
	QTextDocument::FindFlags flags = searchFlags();
    bool found = false;
	while (!highlightCursor.isNull() && !highlightCursor.atEnd()){
		highlightCursor = document->find(searchString, highlightCursor, flags);
		if (!highlightCursor.isNull()){
			found = true;
			highlightCursor.insertText(replaceString);
		}
	}
	
	if (!found){
		QMessageBox::information(this, tr("Word Not Found"), 
				tr("Sorry, the expression cannot be found."));
	}    
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
