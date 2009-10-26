/***************************************************************************
    File                 : TextFormatButtons.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Widget with text format buttons (connected to a QTextEdit)

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

#include "TextFormatButtons.h"
#include "SymbolDialog.h"
#include "pixmaps.h"
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>

static const char * lineSymbol_xpm[] = {
"16 16 4 1",
" 	c None",
".	c #8C2727",
"+	c #272787",
"@	c #FFFFFF",
"                ",
"                ",
"                ",
"      ...       ",
"  ............  ",
"      ...       ",
"                ",
"                ",
"                ",
"      +++       ",
"  +++++@++++++  ",
"      +++       ",
"                ",
"                ",
"                ",
"                "};

/* XPM */
static const char * fraction_xpm[] = {
"18 21 5 1",
" 	c None",
".	c #000000",
"+	c #121212",
"@	c #090909",
"#	c #FC0F0F",
"       ..  ..     ",
"      + .@@...    ",
"     @   @. .     ",
"        ..        ",
"        ..        ",
"     @ @@   .     ",
"    @.@@.. .      ",
"     @.  ..       ",
"                  ",
"##################",
"                  ",
"      .@@  .@     ",
"     . .@  .@     ",
"       .  .@      ",
"      ..  .@      ",
"      .   .@      ",
"      .@ ..       ",
"       ....       ",
"     @   @        ",
"     .. ..        ",
"      ..@         "};

/* XPM */
static const char * square_root_xpm[] = {
"22 19 8 1",
" 	c None",
".	c #FC0F0F",
"+	c #000000",
"@	c #1A1A1A",
"#	c #060606",
"$	c #010101",
"%	c #070707",
"&	c #090909",
"         .............",
"         ..          .",
"         .           .",
"        ..            ",
"        ..            ",
"        ..  +@++ +++  ",
"        .  +# ++++ $+ ",
"  .    ..  +   ++  ++ ",
" ..    ..      +#     ",
"....   ..      +      ",
". ..   .      #+      ",
"   .  ..  ++  %+   +  ",
"   .. ..  ++ ++++ ++  ",
"   .. ..   +++ +++&   ",
"    . .               ",
"    ...               ",
"    ...               ",
"     .                ",
"     .                "};

TextFormatButtons::TextFormatButtons(QTextEdit * textEdit, Buttons buttons, QWidget * parent)
: QWidget(parent),
connectedTextEdit(textEdit),
d_buttons(buttons)
{
	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	init(buttons);
}

void TextFormatButtons::init(Buttons buttons)
{
	QHBoxLayout *layout = (QHBoxLayout*)this->layout();
	QLayoutItem *child;
	while ((child = layout->takeAt(0)) != 0){
		if (child->widget())
			delete child->widget();
	}

	QFont font = QFont();
	int btnSize = 32;
#ifdef Q_OS_MAC
	btnSize = 38;
#endif
	if (buttons == Legend || buttons == TexLegend){
		QPushButton *buttonCurve = new QPushButton( QPixmap(lineSymbol_xpm), QString());
		buttonCurve->setFixedWidth(btnSize);
		buttonCurve->setFixedHeight(btnSize);
		buttonCurve->setFont(font);
		layout->addWidget(buttonCurve);
		connect( buttonCurve, SIGNAL(clicked()), this, SLOT(addCurve()) );
	}

	QPushButton *buttonSubscript = new QPushButton(QPixmap(index_xpm), QString());
	buttonSubscript->setFixedWidth(btnSize);
	buttonSubscript->setFixedHeight(btnSize);
	buttonSubscript->setFont(font);
	layout->addWidget(buttonSubscript);
	connect( buttonSubscript, SIGNAL(clicked()), this, SLOT(addSubscript()) );

	QPushButton *buttonSuperscript = new QPushButton(QPixmap(exp_xpm), QString());
	buttonSuperscript->setFixedWidth(btnSize);
	buttonSuperscript->setFixedHeight(btnSize);
	buttonSuperscript->setFont(font);
	layout->addWidget(buttonSuperscript);
	connect( buttonSuperscript, SIGNAL(clicked()), this, SLOT(addSuperscript()));

	if (buttons == Equation || buttons == TexLegend){
		QPushButton *buttonFraction = new QPushButton(QPixmap(fraction_xpm), QString());
		buttonFraction->setFixedWidth(btnSize);
		buttonFraction->setFixedHeight(btnSize);
		buttonFraction->setFont(font);
		layout->addWidget(buttonFraction);
		connect(buttonFraction, SIGNAL(clicked()), this, SLOT(addFraction()));

		QPushButton *buttonSquareRoot = new QPushButton(QPixmap(square_root_xpm), QString());
		buttonSquareRoot->setFixedWidth(btnSize);
		buttonSquareRoot->setFixedHeight(btnSize);
		buttonSquareRoot->setFont(font);
		layout->addWidget(buttonSquareRoot);
		connect(buttonSquareRoot, SIGNAL(clicked()), this, SLOT(addSquareRoot()));
	}

	QPushButton *buttonLowerGreek = new QPushButton(QString(QChar(0x3B1)));
	buttonLowerGreek->setFont(font);
	buttonLowerGreek->setFixedWidth(btnSize);
	buttonLowerGreek->setFixedHeight(btnSize);
	layout->addWidget(buttonLowerGreek);
	connect( buttonLowerGreek, SIGNAL(clicked()), this, SLOT(showLowerGreek()));

	if (buttons != Equation && buttons != TexLegend){
		QPushButton *buttonUpperGreek = new QPushButton(QString(QChar(0x393)));
		buttonUpperGreek->setFont(font);
		buttonUpperGreek->setFixedWidth(btnSize);
		buttonUpperGreek->setFixedHeight(btnSize);
		layout->addWidget(buttonUpperGreek);
		connect( buttonUpperGreek, SIGNAL(clicked()), this, SLOT(showUpperGreek()));

		QPushButton *buttonMathSymbols = new QPushButton(QString(QChar(0x222B)));
		buttonMathSymbols->setFont(font);
		buttonMathSymbols->setFixedWidth(btnSize);
		buttonMathSymbols->setFixedHeight(btnSize);
		layout->addWidget(buttonMathSymbols);
		connect( buttonMathSymbols, SIGNAL(clicked()), this, SLOT(showMathSymbols()));

		QPushButton *buttonArrowSymbols = new QPushButton(QString(QChar(0x2192)));
		buttonArrowSymbols->setFont(font);
		buttonArrowSymbols->setFixedWidth(btnSize);
		buttonArrowSymbols->setFixedHeight(btnSize);
		layout->addWidget(buttonArrowSymbols);
		connect( buttonArrowSymbols, SIGNAL(clicked()), this, SLOT(showArrowSymbols()));
	} else
		layout->addStretch();

	if (buttons != Plot3D && buttons != Equation && buttons != TexLegend){
		font = this->font();
		font.setBold(true);

		QPushButton *buttonBold = new QPushButton(tr("B","Button bold"));
		buttonBold->setFont(font);
		buttonBold->setFixedWidth(btnSize);
		buttonBold->setFixedHeight(btnSize);
		layout->addWidget(buttonBold);
		connect( buttonBold, SIGNAL(clicked()), this, SLOT(addBold()));

		font = this->font();
		font.setItalic(true);

		QPushButton *buttonItalics = new QPushButton(tr("It","Button italics"));
		buttonItalics->setFont(font);
		buttonItalics->setFixedWidth(btnSize);
		buttonItalics->setFixedHeight(btnSize);
		layout->addWidget(buttonItalics);
		connect( buttonItalics, SIGNAL(clicked()), this, SLOT(addItalics()));

		font = this->font();
		font.setUnderline(true);

		QPushButton *buttonUnderline = new QPushButton(tr("U","Button underline"));
		buttonUnderline->setFont(font);
		buttonUnderline->setFixedWidth(btnSize);
		buttonUnderline->setFixedHeight(btnSize);
		layout->addWidget(buttonUnderline);
   		layout->addStretch();
		connect( buttonUnderline, SIGNAL(clicked()), this, SLOT(addUnderline()));
	}
}

void TextFormatButtons::showLowerGreek()
{
	SymbolDialog *greekLetters = new SymbolDialog(SymbolDialog::lowerGreek, this, Qt::Tool);
	greekLetters->setAttribute(Qt::WA_DeleteOnClose);
	QFont f = connectedTextEdit->font();
	f.setPointSize(12);
	greekLetters->setFont(f);
	connect(greekLetters, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));
	greekLetters->show();
	greekLetters->setFocus();
}

void TextFormatButtons::showUpperGreek()
{
	SymbolDialog *greekLetters = new SymbolDialog(SymbolDialog::upperGreek, this, Qt::Tool);
	greekLetters->setAttribute(Qt::WA_DeleteOnClose);
	QFont f = connectedTextEdit->font();
	f.setPointSize(12);
	greekLetters->setFont(f);
	connect(greekLetters, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));
	greekLetters->show();
	greekLetters->setFocus();
}

void TextFormatButtons::showMathSymbols()
{
	SymbolDialog *mathSymbols = new SymbolDialog(SymbolDialog::mathSymbols, this, Qt::Tool);
	mathSymbols->setAttribute(Qt::WA_DeleteOnClose);
	QFont f = connectedTextEdit->font();
	f.setPointSize(12);
	mathSymbols->setFont(f);
	connect(mathSymbols, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));
	mathSymbols->show();
	mathSymbols->setFocus();
}

void TextFormatButtons::showArrowSymbols()
{
	SymbolDialog *arrowSymbols = new SymbolDialog(SymbolDialog::arrowSymbols, this, Qt::Tool);
	arrowSymbols->setAttribute(Qt::WA_DeleteOnClose);
	arrowSymbols->setFont(connectedTextEdit->font());
	QFont f = connectedTextEdit->font();
	f.setPointSize(12);
	arrowSymbols->setFont(f);
	connect(arrowSymbols, SIGNAL(addLetter(const QString&)), this, SLOT(addSymbol(const QString&)));
	arrowSymbols->show();
	arrowSymbols->setFocus();
}

void TextFormatButtons::addSymbol(const QString & letter)
{
	if (d_buttons == Equation || d_buttons == TexLegend){
		int alpha = 0x3B1;
		if (letter == QString(QChar(alpha)))
			connectedTextEdit->textCursor().insertText("\\alpha");
		else if (letter == QString(QChar(1 + alpha)))
			connectedTextEdit->textCursor().insertText("\\beta");
		else if (letter == QString(QChar(2 + alpha)))
			connectedTextEdit->textCursor().insertText("\\gamma");
		else if (letter == QString(QChar(3 + alpha)))
			connectedTextEdit->textCursor().insertText("\\delta");
		else if (letter == QString(QChar(4 + alpha)))
			connectedTextEdit->textCursor().insertText("\\epsilon");
		else if (letter == QString(QChar(5 + alpha)))
			connectedTextEdit->textCursor().insertText("\\zeta");
		else if (letter == QString(QChar(6 + alpha)))
			connectedTextEdit->textCursor().insertText("\\eta");
		else if (letter == QString(QChar(7 + alpha)))
			connectedTextEdit->textCursor().insertText("\\theta");
		else if (letter == QString(QChar(8 + alpha)))
			connectedTextEdit->textCursor().insertText("\\iota");
		else if (letter == QString(QChar(9 + alpha)))
			connectedTextEdit->textCursor().insertText("\\kappa");
		else if (letter == QString(QChar(10 + alpha)))
			connectedTextEdit->textCursor().insertText("\\lambda");
		else if (letter == QString(QChar(11 + alpha)))
			connectedTextEdit->textCursor().insertText("\\mu");
		else if (letter == QString(QChar(12 + alpha)))
			connectedTextEdit->textCursor().insertText("\\nu");
		else if (letter == QString(QChar(13 + alpha)))
			connectedTextEdit->textCursor().insertText("\\xi");
		else if (letter == QString(QChar(14 + alpha)))
			connectedTextEdit->textCursor().insertText("\\\\o");
		else if (letter == QString(QChar(15 + alpha)))
			connectedTextEdit->textCursor().insertText("\\pi");
		else if (letter == QString(QChar(16 + alpha)))
			connectedTextEdit->textCursor().insertText("\\rho");
		else if (letter == QString(QChar(17 + alpha)))
			connectedTextEdit->textCursor().insertText("\\varsigma");
		else if (letter == QString(QChar(18 + alpha)))
			connectedTextEdit->textCursor().insertText("\\sigma");
		else if (letter == QString(QChar(19 + alpha)))
			connectedTextEdit->textCursor().insertText("\\tau");
		else if (letter == QString(QChar(20 + alpha)))
			connectedTextEdit->textCursor().insertText("\\upsilon");
		else if (letter == QString(QChar(21 + alpha)))
			connectedTextEdit->textCursor().insertText("\\varphi");
		else if (letter == QString(QChar(22 + alpha)))
			connectedTextEdit->textCursor().insertText("\\chi");
		else if (letter == QString(QChar(23 + alpha)))
			connectedTextEdit->textCursor().insertText("\\psi");
		else if (letter == QString(QChar(24 + alpha)))
			connectedTextEdit->textCursor().insertText("\\omega");
	} else
		connectedTextEdit->textCursor().insertText(letter);
}

void TextFormatButtons::addCurve()
{
	formatText("\\l(",")");
}

void TextFormatButtons::addUnderline()
{
	formatText("<u>","</u>");
}

void TextFormatButtons::addItalics()
{
	formatText("<i>","</i>");
}

void TextFormatButtons::addBold()
{
	formatText("<b>","</b>");
}

void TextFormatButtons::addSubscript()
{
	if (d_buttons == TexLegend || d_buttons == Equation || d_buttons == Plot3D)
		formatText("_{","}");
	else
		formatText("<sub>","</sub>");
}

void TextFormatButtons::addSuperscript()
{
	if (d_buttons == TexLegend || d_buttons == Equation  || d_buttons == Plot3D)
		formatText("^{","}");
	else
		formatText("<sup>","</sup>");
}

void TextFormatButtons::addFraction()
{
	if (d_buttons == TexLegend || d_buttons == Equation)
		formatText("\\frac{","}{}");
}

void TextFormatButtons::addSquareRoot()
{
	if (d_buttons == TexLegend || d_buttons == Equation)
		formatText("\\sqrt{","}");
}

void TextFormatButtons::formatText(const QString & prefix, const QString & postfix)
{
	QTextCursor cursor = connectedTextEdit->textCursor();
	QString markedText = cursor.selectedText();
	cursor.insertText(prefix+markedText+postfix);
	if(markedText.isEmpty())
	{
		// if no text is marked, place cursor inside the <..></..> statement
		// instead of after it
		cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,postfix.size());
		// the next line makes the selection visible to the user
		// (the line above only changes the selection in the
		// underlying QTextDocument)
		connectedTextEdit->setTextCursor(cursor);
	}
	// give focus back to text edit
	connectedTextEdit->setFocus();
}

void TextFormatButtons::setButtons(Buttons btns)
{
	if (btns == d_buttons)
		return;

	d_buttons = btns;
	init(d_buttons);
}
