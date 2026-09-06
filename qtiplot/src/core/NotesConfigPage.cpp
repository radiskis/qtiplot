/***************************************************************************
    File                 : NotesConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Notes preferences page implementation

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
#include "NotesConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>
#include <Note.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QFontComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QIcon>

NotesConfigPage::NotesConfigPage(QWidget *parent)
    : ConfigPage(parent), d_app(nullptr)
{
	QGroupBox *gb1 = new QGroupBox();
	QGridLayout * gl1 = new QGridLayout(gb1);
	gl1->setSpacing(5);

	labelTabLength = new QLabel();
	gl1->addWidget(labelTabLength, 0, 0);

	boxTabLength = new QSpinBox();
	boxTabLength->setRange(0, 1000);
	boxTabLength->setSingleStep(5);
	connect(boxTabLength, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(boxTabLength, 0, 1);

	labelNotesFont = new QLabel();
	gl1->addWidget(labelNotesFont, 1, 0);

	boxFontFamily = new QFontComboBox();
	connect(boxFontFamily, QOverload<int>::of(&QFontComboBox::activated), this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(boxFontFamily, 1, 1);

	boxFontSize = new QSpinBox();
	boxFontSize->setRange(0, 1000);
	connect(boxFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(boxFontSize, 1, 2);

	buttonBoldFont = new QPushButton(tr("&B"));
	QFont font = QFont();
	font.setBold(true);
	buttonBoldFont->setFont(font);
	buttonBoldFont->setCheckable(true);
	connect(buttonBoldFont, &QAbstractButton::clicked, this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(buttonBoldFont, 1, 3);

	buttonItalicFont = new QPushButton(tr("&It"));
	font = QFont();
	font.setItalic(true);
	buttonItalicFont->setFont(font);
	buttonItalicFont->setCheckable(true);
	connect(buttonItalicFont, &QAbstractButton::clicked, this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(buttonItalicFont, 1, 4);

	lineNumbersBox = new QCheckBox();
	connect(lineNumbersBox, &QAbstractButton::toggled, this, &NotesConfigPage::customizeNotes);
	gl1->addWidget(lineNumbersBox, 2, 0);
	gl1->setColumnStretch(5, 1);

	QVBoxLayout* vl = new QVBoxLayout(this);
	vl->addWidget(gb1);

	groupSyntaxHighlighter = new QGroupBox();
	QGridLayout *gl = new QGridLayout(groupSyntaxHighlighter);

	buttonCommentLabel = new QLabel;
	gl->addWidget(buttonCommentLabel, 0, 0);
	buttonCommentColor = new ColorButton();
	connect(buttonCommentColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonCommentColor, 0, 1);
	buttonCommentLabel->setBuddy(buttonCommentColor);

	buttonKeywordLabel = new QLabel;
	gl->addWidget(buttonKeywordLabel, 1, 0);
	buttonKeywordColor = new ColorButton();
	connect(buttonKeywordColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonKeywordColor, 1, 1);
	buttonKeywordLabel->setBuddy(buttonKeywordColor);

	buttonQuotationLabel = new QLabel;
	gl->addWidget(buttonQuotationLabel, 2, 0);
	buttonQuotationColor = new ColorButton();
	connect(buttonQuotationColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonQuotationColor, 2, 1);
	buttonQuotationLabel->setBuddy(buttonQuotationColor);

	buttonNumericLabel = new QLabel;
	gl->addWidget(buttonNumericLabel, 3, 0);
	buttonNumericColor = new ColorButton();
	connect(buttonNumericColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonNumericColor, 3, 1);
	buttonNumericLabel->setBuddy(buttonNumericColor);

	buttonFunctionLabel = new QLabel;
	gl->addWidget(buttonFunctionLabel, 4, 0);
	buttonFunctionColor = new ColorButton();
	connect(buttonFunctionColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonFunctionColor, 4, 1);
	buttonFunctionLabel->setBuddy(buttonFunctionColor);

	buttonClassLabel = new QLabel;
	gl->addWidget(buttonClassLabel, 5, 0);
	buttonClassColor = new ColorButton();
	connect(buttonClassColor, &ColorButton::colorChanged, this, &NotesConfigPage::rehighlight);
	gl->addWidget(buttonClassColor, 5, 1);
	buttonClassLabel->setBuddy(buttonClassColor);
	gl->setRowStretch(6, 1);

	vl->addWidget(groupSyntaxHighlighter);
	vl->addStretch();
}

void NotesConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	boxTabLength->blockSignals(true);
	boxTabLength->setValue(app->d_notes_tab_length);
	boxTabLength->blockSignals(false);

	boxFontFamily->blockSignals(true);
	boxFontFamily->setCurrentFont(app->d_notes_font);
	boxFontFamily->blockSignals(false);

	boxFontSize->blockSignals(true);
	boxFontSize->setValue(app->d_notes_font.pointSize());
	boxFontSize->blockSignals(false);

	buttonBoldFont->blockSignals(true);
	buttonBoldFont->setChecked(app->d_notes_font.bold());
	buttonBoldFont->blockSignals(false);

	buttonItalicFont->blockSignals(true);
	buttonItalicFont->setChecked(app->d_notes_font.italic());
	buttonItalicFont->blockSignals(false);

	lineNumbersBox->blockSignals(true);
	lineNumbersBox->setChecked(app->d_note_line_numbers);
	lineNumbersBox->blockSignals(false);

	buttonCommentColor->setColor(app->d_comment_highlight_color);
	buttonKeywordColor->setColor(app->d_keyword_highlight_color);
	buttonQuotationColor->setColor(app->d_quotation_highlight_color);
	buttonNumericColor->setColor(app->d_numeric_highlight_color);
	buttonFunctionColor->setColor(app->d_function_highlight_color);
	buttonClassColor->setColor(app->d_class_highlight_color);
}

void NotesConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	d_app = app;
	if (!app)
		return;

	app->d_note_line_numbers = lineNumbersBox->isChecked();
	app->d_notes_tab_length = boxTabLength->value();
	QFont f = QFont(boxFontFamily->currentFont().family(), boxFontSize->value());
	f.setBold(buttonBoldFont->isChecked());
	f.setItalic(buttonItalicFont->isChecked());
	app->d_notes_font = f;

	app->d_comment_highlight_color = buttonCommentColor->color();
	app->d_keyword_highlight_color = buttonKeywordColor->color();
	app->d_quotation_highlight_color = buttonQuotationColor->color();
	app->d_numeric_highlight_color = buttonNumericColor->color();
	app->d_function_highlight_color = buttonFunctionColor->color();
	app->d_class_highlight_color = buttonClassColor->color();

	QList<MdiSubWindow *> windows = app->windowsList();
	for (MdiSubWindow *w : windows){
		Note *n = qobject_cast<Note *>(w);
		if (n){
			n->showLineNumbers(app->d_note_line_numbers);
			n->setTabStopDistance(app->d_notes_tab_length);
			n->setFont(f);
			for (int i = 0; i < n->tabs(); i++)
				n->editor(i)->rehighlight();
		}
	}
	app->setFormatBarFont(f);
}

void NotesConfigPage::retranslateUi()
{
	labelTabLength->setText(tr("Tab length"));
	labelNotesFont->setText(tr("Font"));
	lineNumbersBox->setText(tr("Line Numbers"));
	groupSyntaxHighlighter->setTitle(tr("Syntax Highlighting"));
	buttonCommentLabel->setText(tr("Comments"));
	buttonKeywordLabel->setText(tr("Keywords"));
	buttonQuotationLabel->setText(tr("Quotations"));
	buttonNumericLabel->setText(tr("Numbers"));
	buttonFunctionLabel->setText(tr("Functions"));
	buttonClassLabel->setText(tr("Classes"));
}

void NotesConfigPage::rehighlight()
{
	if (!d_app)
		return;

	d_app->d_comment_highlight_color = buttonCommentColor->color();
	d_app->d_keyword_highlight_color = buttonKeywordColor->color();
	d_app->d_quotation_highlight_color = buttonQuotationColor->color();
	d_app->d_numeric_highlight_color = buttonNumericColor->color();
	d_app->d_function_highlight_color = buttonFunctionColor->color();
	d_app->d_class_highlight_color = buttonClassColor->color();

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		Note *n = qobject_cast<Note *>(w);
		if (n){
			for (int i = 0; i < n->tabs(); i++)
				n->editor(i)->rehighlight();
		}
	}
}

void NotesConfigPage::customizeNotes()
{
	if (!d_app)
		return;

	d_app->d_note_line_numbers = lineNumbersBox->isChecked();
	d_app->d_notes_tab_length = boxTabLength->value();
	QFont f = QFont(boxFontFamily->currentFont().family(), boxFontSize->value());
	f.setBold(buttonBoldFont->isChecked());
	f.setItalic(buttonItalicFont->isChecked());
	d_app->d_notes_font = f;
	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		Note *n = qobject_cast<Note *>(w);
		if (n){
			n->showLineNumbers(d_app->d_note_line_numbers);
			n->setTabStopDistance(d_app->d_notes_tab_length);
			n->setFont(f);
		}
	}
	d_app->setFormatBarFont(f);
}
