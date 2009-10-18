/***************************************************************************
    File                 : PythonSyntaxHighlighter.h
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

#ifndef PYTHON_HIGHLIGHTER_H
#define PYTHON_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

#include "ScriptEdit.h"

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(ScriptEdit * parent);

protected:
    void highlightBlock(const QString &text);

	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> highlightingRules;

	QTextCharFormat commentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat numericFormat;
};

class PythonSyntaxHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    PythonSyntaxHighlighter(ScriptEdit *parent);

    static QStringList keywordsList(){return d_keywords;};

protected:
    void highlightBlock(const QString &text);

private:
	QVector<HighlightingRule> pythonHighlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;

    static const QStringList d_keywords;
};

#endif
