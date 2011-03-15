/***************************************************************************
    File                 : PythonSyntaxHighlighter.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Python Syntax Highlighting based on the Qt Syntax Highlighter Example
							(http://doc.trolltech.com/4.4/richtext-syntaxhighlighter.html)

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

#include "PythonSyntaxHighlighter.h"
#include <ApplicationWindow.h>

const QStringList PythonSyntaxHighlighter::d_keywords = QStringList() << "and" << "assert" << "break"
					<< "class" << "continue"  << "def" << "del"
					<< "elif" << "else" << "except" << "exec"
					<< "finally" << "for" << "from" << "global"
					<< "if" << "import" << "in" << "is"
					<< "lambda" << "not" << "or" << "pass"
					<< "print" << "raise" << "return" << "try" << "while" << "True" << "False";

PythonSyntaxHighlighter::PythonSyntaxHighlighter(ScriptEdit *parent)
    : SyntaxHighlighter(parent)
{
    HighlightingRule rule;
	ApplicationWindow *app = parent->scriptingEnv()->application();

	keywordFormat.setForeground(app->d_keyword_highlight_color);
    keywordFormat.setFontWeight(QFont::Bold);

	foreach (QString pattern, d_keywords) {
        rule.pattern = QRegExp("\\b" + pattern + "\\b");
        rule.format = keywordFormat;
        pythonHighlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(app->d_class_highlight_color);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
	pythonHighlightingRules.append(rule);
}

void PythonSyntaxHighlighter::highlightBlock(const QString &text)
{
	QString s = text;
	QRegExp comment = QRegExp("\"{3}");
	s.replace(comment, "   ");

    foreach (HighlightingRule rule, pythonHighlightingRules) {
        QRegExp expression(rule.pattern);
        int index = s.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = s.indexOf(expression, index + length);
        }
    }

	SyntaxHighlighter::highlightBlock(text);//process common rules and parentheses matching

	int startIndex = text.indexOf(comment);
	int prevState = previousBlockState ();
	int comments = text.count(comment);

	if (comments > 1){
			int aux = 1;
			if (prevState == 1)
				setFormat(0, startIndex + 3, commentFormat);

			while (aux < comments) {
				int endIndex = text.indexOf(comment, startIndex + 3);
				aux++;
				if ((!prevState && (aux %2 == 0)) || (prevState == 1 && (aux %2 != 0)))
					setFormat(startIndex, endIndex - startIndex + 3, commentFormat);

				startIndex = endIndex;
			}

		int state = 0;
		if ((!prevState && (comments % 2 != 0)) || (prevState && (comments % 2 == 0))){
			state = 1;
			setFormat(startIndex, text.length() - startIndex, commentFormat);
		}
		setCurrentBlockState(state);
	} else if (comments == 1){
		if (prevState == 1){
			setCurrentBlockState(0);// end of comment block
			setFormat(0, startIndex + 3, commentFormat);
		} else {
			setCurrentBlockState(1);// start of comment block
			setFormat(startIndex, text.length() - startIndex, commentFormat);
		}
	} else {
		if (prevState == 1){
			setCurrentBlockState(1);// inside comment block
			setFormat(0, text.length(), commentFormat);
		} else
			setCurrentBlockState(0);// outside comment block
	}
}

SyntaxHighlighter::SyntaxHighlighter(ScriptEdit * parent) : QSyntaxHighlighter(parent->document())
{
	HighlightingRule rule;
	ApplicationWindow *app = parent->scriptingEnv()->application();

	functionFormat.setFontItalic(true);
	functionFormat.setForeground(app->d_function_highlight_color);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	numericFormat.setForeground(app->d_numeric_highlight_color);
	rule.pattern = QRegExp("\\b\\d+[eE.,]*\\d*\\b");
	rule.format = numericFormat;
	highlightingRules.append(rule);

	quotationFormat.setForeground(app->d_quotation_highlight_color);
	rule.pattern = QRegExp("\".*\"");
	rule.pattern.setMinimal(true);
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	commentFormat.setForeground(app->d_comment_highlight_color);
	rule.pattern = QRegExp("#[^\n]*");
	rule.format = commentFormat;
	highlightingRules.append(rule);
}

//! Parentheses matching code taken from Qt Quarterly Issue 31 · Q3 2009
void SyntaxHighlighter::highlightBlock(const QString &text)
{
	QString s = text;
	foreach (HighlightingRule rule, highlightingRules) {
		QRegExp expression(rule.pattern);
		int index = s.indexOf(expression);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = s.indexOf(expression, index + length);
		}
	}

	TextBlockData *data = new TextBlockData;

    int leftPos = text.indexOf('(');
    while (leftPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = '(';
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf('(', leftPos + 1);
    }

    int rightPos = text.indexOf(')');
    while (rightPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ')';
        info->position = rightPos;

        data->insert(info);

		rightPos = text.indexOf(')', rightPos + 1);
    }

    setCurrentBlockUserData(data);
}
