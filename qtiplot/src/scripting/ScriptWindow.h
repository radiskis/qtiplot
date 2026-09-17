/***************************************************************************
    File                 : ScriptWindow.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Python script window

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
#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include "ScriptEdit.h"

#include <QMainWindow>
#include <QMenu>
#include <QCloseEvent>
class ScriptingEnv;
class ApplicationWindow;
class LineNumberDisplay;
class QAction;

//! Python script window
class ScriptWindow: public QMainWindow
{
	Q_OBJECT

public:
		ScriptWindow(ScriptingEnv *env, ApplicationWindow *app);
        ~ScriptWindow() override = default;

public slots:
		void newScript();
		void open(const QString& fn = QString());
		void save();
		void saveAs();
		void languageChange();
		void setVisible(bool visible) override;

		ScriptEdit* editor() const {return te;};
		void executeAll(){te->executeAll();};

		//! Enables/Disables the line number display
		void showLineNumbers(bool show = true);
		void stopExecution();

private slots:
		void setAlwaysOnTop(bool on);
		void redirectOutput(bool);
		void printPreview();
		void showWorkspace(bool on = true);
		void find();
		void findNext();
		void findPrevious();
		void replace();
		void increaseIndent();
		void decreaseIndent();
		void enableActions();

signals:
		void visibilityChanged(bool visible);

private:
		void moveEvent( QMoveEvent* ) override;
		void resizeEvent( QResizeEvent* ) override;

		void initMenu();
		void initActions();
		ScriptEdit *te = nullptr;
		ApplicationWindow *d_app = nullptr;
		LineNumberDisplay *d_line_number = nullptr;
		QWidget *d_frame = nullptr;

		QString fileName;

		QMenu *file = nullptr, *edit = nullptr, *run = nullptr, *windowMenu = nullptr;
		QAction *actionNew = nullptr, *actionUndo = nullptr, *actionRedo = nullptr, *actionCut = nullptr, *actionCopy = nullptr, *actionPaste = nullptr;
		QAction *actionExecute = nullptr, *actionExecuteAll = nullptr, *actionEval = nullptr, *actionPrint = nullptr, *actionOpen = nullptr;
		QAction *actionSave = nullptr, *actionSaveAs = nullptr;
		QAction *actionAlwaysOnTop = nullptr, *actionHide = nullptr, *actionShowLineNumbers = nullptr;
		QAction *actionShowConsole = nullptr, *actionRedirectOutput = nullptr, *actionPrintPreview = nullptr;
		QAction *actionShowWorkspace = nullptr;
		QAction *actionFind = nullptr, *actionFindNext = nullptr, *actionFindPrev = nullptr, *actionReplace = nullptr;
		QAction *actionIncreaseIndent = nullptr, *actionDecreaseIndent = nullptr;
		QDockWidget *consoleWindow = nullptr;
		QTextEdit *console = nullptr;
		QAction *actionStop = nullptr;
};

#endif
