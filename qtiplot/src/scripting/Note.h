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
	void setTabText(int index, const QString & label){d_tab_widget->setTabText(index, label);};
	int indexOf(ScriptEdit* editor);
	ScriptEdit* editor(int index);
	ScriptEdit* currentEditor();
	int tabs(){return d_tab_widget->count();};

public slots:
	void save(const QString& fn, const QString &info, bool = false);
	void restore(const QStringList&);

	bool autoexec() const { return autoExec; }
	void setAutoexec(bool);
	void modifiedNote();

	// ScriptEdit methods
	QString text() { return currentEditor()->text(); };
	void setText(const QString &s) { currentEditor()->setText(s); };
	void print() { currentEditor()->print(); };
	void print(QPrinter *printer) { currentEditor()->print(printer); };
	void exportPDF(const QString& fileName){currentEditor()->exportPDF(fileName);};
	QString exportASCII(const QString &file=QString::null) { return currentEditor()->exportASCII(file); };
	QString importASCII(const QString &file=QString::null){ return currentEditor()->importASCII(file);};
	void execute() { currentEditor()->execute(); };
	void executeAll() { currentEditor()->executeAll(); };
	void evaluate() { currentEditor()->evaluate(); };
	void setDirPath(const QString& path){currentEditor()->setDirPath(path);};

	//! Enables/Disables the line number display
	void showLineNumbers(bool show = true);
	bool hasLineNumbers(){return d_line_number_enabled;};

    void setFont(const QFont& f);
    void addTab();
    void removeTab(int);

 signals:
	void dirPathChanged(const QString& path);

private:
	void saveTab(int index, const QString &fn);

	ScriptingEnv *d_env;
	QWidget *d_frame;
	QTabWidget *d_tab_widget;
	bool d_line_number_enabled;
	bool autoExec;
};

#endif
