/***************************************************************************
    File                 : Note.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Notes window class

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
#ifndef NOTE_H
#define NOTE_H

#include <MdiSubWindow.h>
#include <ScriptEdit.h>
#include <LineNumberDisplay.h>
#include <QTextEdit>
#include <QTabWidget>

class ScriptingEnv;

/*!\brief Notes window class.
 *
 */
class Note: public MdiSubWindow
{
    Q_OBJECT

public:

	Note(ScriptingEnv *env, const QString& label, ApplicationWindow* parent, const QString& name = QString(), Qt::WFlags f=0);
	~Note(){};

	void init(ScriptingEnv *env);
	void setName(const QString& name);
	void setTabStopWidth(int length);
	int indexOf(ScriptEdit* editor);
	ScriptEdit* editor(int index);
	ScriptEdit* currentEditor();
	int tabs(){return d_tab_widget->count();};
    void renameTab(int, const QString&);

	void save(const QString& fn, const QString &info, bool = false);
	void restore(const QStringList&);

public slots:
	bool autoexec() const { return autoExec; }
	void setAutoexec(bool);
	void modifiedNote();

	// ScriptEdit methods
        QString text() { if(currentEditor()) return currentEditor()->text(); return QString::null;};
        void setText(const QString &s) { if(currentEditor()) currentEditor()->setText(s); };
        void print() { if(currentEditor()) currentEditor()->print(); };
        void print(QPrinter *printer) { if(currentEditor()) currentEditor()->print(printer); };
        void exportPDF(const QString& fileName){if(currentEditor()) currentEditor()->exportPDF(fileName);};
        QString exportASCII(const QString &file=QString::null) { if(currentEditor()) return currentEditor()->exportASCII(file); return QString::null;};
        QString importASCII(const QString &file=QString::null){ if(currentEditor()) return currentEditor()->importASCII(file); return QString::null;};
        void execute() { if(currentEditor()) currentEditor()->execute(); };
        void executeAll() { if(currentEditor()) currentEditor()->executeAll(); };
        void evaluate() { if(currentEditor()) currentEditor()->evaluate(); };
        void setDirPath(const QString& path){if(currentEditor()) currentEditor()->setDirPath(path);};

	//! Enables/Disables the line number display
	void showLineNumbers(bool show = true);
	bool hasLineNumbers(){return d_line_number_enabled;};

    void setFont(const QFont& f);
    void addTab();
    void removeTab(int = -1);
    void renameCurrentTab();

 signals:
	void dirPathChanged(const QString& path);
	void currentEditorChanged();

private:
	void saveTab(int index, const QString &fn);

	ScriptingEnv *d_env;
	QWidget *d_frame;
	QTabWidget *d_tab_widget;
	bool d_line_number_enabled;
	bool autoExec;
};

#endif
