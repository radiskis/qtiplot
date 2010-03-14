/***************************************************************************
    File                 : ScriptEdit.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Scripting classes

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
#ifndef SCRIPTEDIT_H
#define SCRIPTEDIT_H

#include "ScriptingEnv.h"
#include "Script.h"

#include <QMenu>
#include <QTextEdit>

class QAction;
class QMenu;
class QCompleter;

class SyntaxHighlighter;

/*!\brief Editor widget with support for evaluating expressions and executing code.
 *
 * \section future Future Plans
 * - Display line numbers.
 * - syntax highlighting, indentation, auto-completion etc. (maybe using QScintilla)
 */
class ScriptEdit: public QTextEdit, public scripted
{
  Q_OBJECT

  public:
    ScriptEdit(ScriptingEnv *env, QWidget *parent=0, const char *name=0);
  	~ScriptEdit();
	//! Handle changing of scripting environment.
    void customEvent(QEvent*);
  	//! Map cursor positions to line numbers.
    int lineNumber(int pos) const;
	bool error(){return d_error;};

    void setCompleter(QCompleter *c);
	void setFileName(const QString& fn);
	void rehighlight();
	void redirectOutputTo(QTextEdit *);
	void enableShortcuts();

  public slots:
    void execute();
    void executeAll();
    void evaluate();
    void print();
    void print(QPrinter*);
    void exportPDF(const QString& fileName);
  	QString save();
    QString exportASCII(const QString &file=QString::null);
    QString importASCII(const QString &file=QString::null);
    void insertFunction(const QString &);
    void insertFunction(QAction * action);
    void setContext(QObject *context) { myScript->setContext(context); }
    void scriptPrint(const QString&);

    void updateIndentation();
	void setDirPath(const QString& path);
	void showFindDialog(bool replace = false);
	void replace(){showFindDialog(true);};
	bool find(const QString& searchString, QTextDocument::FindFlags flags, bool previous = false);
	void findNext();
	void findPrevious();
	void commentSelection();
	void uncommentSelection();

  signals:
	void dirPathChanged(const QString& path);
	void error(const QString&, const QString&, int);
	void activated(ScriptEdit *);

  protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

  private:
	void clearErrorHighlighting();
	void highlightErrorLine(int offset);

    Script *myScript;
    QAction *actionExecute, *actionExecuteAll, *actionEval, *actionPrint, *actionImport, *actionSave, *actionExport;
    QAction *actionFind, *actionReplace, *actionFindNext, *actionFindPrevious;
  	//! Submenu of context menu with mathematical functions.
  	QMenu *functionsMenu;
  	//! Cursor used for output of evaluation results and error messages.
  	QTextCursor printCursor;
  	QString scriptsDirPath;

    //! Format used for resetting success/failure markers.
	QTextBlockFormat d_fmt_default;
	//! True if we are inside evaluate(), execute() or executeAll() there were errors.
	bool d_error;
	QString d_err_message;

	QCompleter *d_completer;
	SyntaxHighlighter *d_highlighter;
	QString d_file_name;
	QString d_search_string;
	QTextDocument::FindFlags d_search_flags;
	QTextEdit *d_output_widget;

  private slots:
	  //! Insert an error message from the scripting system at printCursor.
		/**
		* After insertion, the text cursor will have the error message selected, allowing the user to
		* delete it and fix the error.
		*/
    void insertErrorMsg(const QString &message);
	void insertCompletion(const QString &completion);
	void matchParentheses();

  private:
    QString textUnderCursor() const;
	bool matchLeftParenthesis(QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);
};

//! Structure used for parentheses matching
struct ParenthesisInfo
{
    char character;
    int position;
};

//! Help class used for parentheses matching (code taken from Qt Quarterly Issue 31 · Q3 2009)
class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData(){};

    QVector<ParenthesisInfo *> parentheses(){return m_parentheses;};
    void insert(ParenthesisInfo *info)
	{
		int i = 0;
		while (i < m_parentheses.size() &&
			info->position > m_parentheses.at(i)->position)
			++i;

		m_parentheses.insert(i, info);
	}

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

#endif
