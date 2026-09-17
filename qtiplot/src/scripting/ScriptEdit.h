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
#include <QTextBlock>

#include <QMenu>
#include <QTextEdit>

class QAction;
class QMenu;
class QCompleter;
class QPrinter;

class QSyntaxHighlighter;

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
    ScriptEdit(ScriptingEnv *env, QWidget *parent = nullptr, const QString& name = QString());
  	~ScriptEdit() override;
	//! Handle changing of scripting environment.
    void customEvent(QEvent*) override;
  	//! Map cursor positions to line numbers.
    int lineNumber(int pos) const;
	bool hasError() const {return d_error;};
	QString text() const {return toPlainText();};

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
    QString exportASCII(const QString &file=QString());
    QString importASCII(const QString &file=QString());
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
	void stopExecution();

  signals:
	void dirPathChanged(const QString& path);
	void error(const QString&, const QString&, int);
	void activated(ScriptEdit *);

  protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

  private:
	void clearErrorHighlighting();
	void highlightErrorLine(int offset);

    Script *myScript = nullptr;
    QAction *actionExecute = nullptr, *actionExecuteAll = nullptr, *actionEval = nullptr, *actionPrint = nullptr, *actionImport = nullptr, *actionSave = nullptr, *actionExport = nullptr;
    QAction *actionFind = nullptr, *actionReplace = nullptr, *actionFindNext = nullptr, *actionFindPrevious = nullptr;
  	//! Submenu of context menu with mathematical functions.
  	QMenu *functionsMenu = nullptr;
  	//! Cursor used for output of evaluation results and error messages.
  	QTextCursor printCursor;
  	QString scriptsDirPath;

    //! Format used for resetting success/failure markers.
	QTextBlockFormat d_fmt_default;
	//! True if we are inside evaluate(), execute() or executeAll() there were errors.
	bool d_error = false;
	QString d_err_message;

	QCompleter *d_completer = nullptr;
	QSyntaxHighlighter *d_highlighter = nullptr;
	QString d_file_name;
	QString d_search_string;
	QTextDocument::FindFlags d_search_flags;
	QTextEdit *d_output_widget = nullptr;

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
    ~TextBlockData() override { qDeleteAll(m_parentheses); }

    QVector<ParenthesisInfo *> parentheses() const {return m_parentheses;};
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
