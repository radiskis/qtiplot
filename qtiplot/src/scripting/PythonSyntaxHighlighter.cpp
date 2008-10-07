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

#include <QtGui>
#include "PythonSyntaxHighlighter.h"
#include "../ApplicationWindow.h"

PythonSyntaxHighlighter::PythonSyntaxHighlighter(ScriptEdit *parent)
    : QSyntaxHighlighter(parent->document())
{
    HighlightingRule rule;
	ApplicationWindow *app = parent->scriptingEnv()->application();	
	
	keywordFormat.setForeground(app->d_keyword_highlight_color);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\band\\b" << "\\bassert\\b" << "\\bbreak\\b" 
					<< "\\bclass\\b" << "\\bcontinue\\b"  << "\\bdef\\b" << "\\bdel\\b" 
					<< "\\belif\\b" << "\\belse\\b" << "\\bexcept\\b" << "\\bexec\\b"
					<< "\\bfinally\\b" << "\\bfor\\b" << "\\bfrom\\b" << "\\bglobal\\b" 
					<< "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" << "\\bis\\b"
					<< "\\blambda\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b"
					<< "\\bprint\\b" << "\\braise\\b" << "\\breturn\\b" << "\\try\b" << "\\bwhile\\b";
    foreach (QString pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(app->d_class_highlight_color);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

	quotationFormat.setForeground(app->d_quotation_highlight_color);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
	functionFormat.setForeground(app->d_function_highlight_color);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

	numericFormat.setForeground(app->d_numeric_highlight_color);
    rule.pattern = QRegExp("\\b\\d+[eE.]*\\d*\\b");
    rule.format = numericFormat;
    highlightingRules.append(rule);
	
	commentFormat.setForeground(app->d_comment_highlight_color);
	rule.pattern = QRegExp("#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

void PythonSyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
    setCurrentBlockState(0);

	QRegExp comment = QRegExp("\"{3}");
	
     int startIndex = 0;
     if (previousBlockState() != 1)
         startIndex = text.indexOf(comment);

     while (startIndex >= 0) {
         int endIndex = text.indexOf(comment, startIndex + 3);
         int commentLength;
         if (endIndex == -1) {
             setCurrentBlockState(1);
             commentLength = text.length() - startIndex;
         } else
             commentLength = endIndex - startIndex + comment.matchedLength();
         setFormat(startIndex, commentLength, commentFormat);
         startIndex = text.indexOf(comment, startIndex + commentLength + 3);
     }
}
