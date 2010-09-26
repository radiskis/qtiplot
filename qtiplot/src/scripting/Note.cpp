/***************************************************************************
    File                 : Note.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
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
#include "Note.h"
#include "ScriptEdit.h"
#include <ApplicationWindow.h>

#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QInputDialog>

Note::Note(ScriptingEnv *env, const QString& label, ApplicationWindow* parent, const QString& name, Qt::WFlags f)
		 : MdiSubWindow(label, parent, name, f), d_env(env),
		 d_line_number_enabled(parent->d_note_line_numbers)
{
	init(env);
}

void Note::init(ScriptingEnv *env)
{
	autoExec = false;

	d_tab_widget = new QTabWidget;
#if QT_VERSION >= 0x040500
	d_tab_widget->setTabsClosable(true);
	d_tab_widget->setDocumentMode(true);

	connect(d_tab_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));
#endif
	connect(d_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(notifyChanges()));
	connect(d_tab_widget, SIGNAL(currentChanged(int)), this, SIGNAL(currentEditorChanged()));

	QPushButton *btnAdd = new QPushButton("+");
	btnAdd->setToolTip(tr("Add tab"));
	btnAdd->setMaximumWidth(20);
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(addTab()));

	QWidget *addWidget = new QWidget;
	QHBoxLayout *hb = new QHBoxLayout(addWidget);
	hb->setMargin(0);
	hb->setSpacing(0);
	hb->addWidget(btnAdd);
	hb->addStretch();

	d_tab_widget->setCornerWidget(addWidget);

	setWidget(d_tab_widget);

	addTab();

	resize(500, 200);
}

void Note::showLineNumbers(bool show)
{
	if (d_line_number_enabled == show)
		return;

	d_line_number_enabled = show;

	for (int i = 0; i < d_tab_widget->count(); i++){
		QWidget *w = d_tab_widget->widget(i);
		if (!w)
			continue;

		QObjectList lst = w->children();
		foreach (QObject *obj, lst){
			LineNumberDisplay *display = qobject_cast<LineNumberDisplay *>(obj);
			if (display){
				display->setVisible(show);
				break;
			}
		}
	}
}

void Note::renameCurrentTab()
{
	bool ok;
	int index = d_tab_widget->currentIndex();
	QString title = QInputDialog::getText(this, tr("Please, enter new title:"), tr("Title"),
					QLineEdit::Normal, d_tab_widget->tabText(index), &ok);
	if (ok && !title.isEmpty())
		d_tab_widget->setTabText(index, title);
}

void Note::renameTab(int index, const QString& title)
{
	d_tab_widget->setTabText(index, title);
}

void Note::removeTab(int index)
{
	if (index < 0)
		index = d_tab_widget->currentIndex();

	if (d_tab_widget->count() == 1)
		return;

	d_tab_widget->removeTab(index);
#if QT_VERSION >= 0x040500
	d_tab_widget->setTabsClosable(d_tab_widget->count() != 1);
#endif
}

void Note::addTab()
{
	ApplicationWindow *app = applicationWindow();
	if (!app)
		return;
#if QT_VERSION >= 0x040500
	d_tab_widget->setTabsClosable(d_tab_widget->count() != 0);
#endif
	QFont f = app->d_notes_font;

	ScriptEdit *editor = new ScriptEdit(d_env, this, name());
	editor->setContext(this);
	editor->setCurrentFont(f);
	editor->document()->setDefaultFont(f);
	editor->setTabStopWidth(app->d_notes_tab_length);
	editor->setCompleter(app->completer());
	editor->setDirPath(app->scriptsDirPath);

	app->connectScriptEditor(editor);

	d_tab_widget->setFocusProxy(editor);

	LineNumberDisplay *ln = new LineNumberDisplay(editor, this);
	ln->setCurrentFont(f);

	QWidget *frame = new QWidget(this);

	QHBoxLayout *hbox = new QHBoxLayout(frame);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(ln);
	hbox->addWidget(editor);

	ln->setVisible(d_line_number_enabled);

	d_tab_widget->setCurrentIndex(d_tab_widget->addTab(frame, tr("untitled")));

	connect(editor, SIGNAL(textChanged()), this, SLOT(modifiedNote()));
	connect(editor, SIGNAL(textChanged()), this, SIGNAL(currentEditorChanged()));
	connect(editor, SIGNAL(dirPathChanged(const QString& )), this, SIGNAL(dirPathChanged(const QString&)));
}

int Note::indexOf(ScriptEdit* editor)
{
	if (!editor)
		return -1;

	for (int i = 0; i < d_tab_widget->count(); i++){
		QWidget *w = d_tab_widget->widget(i);
		if (!w)
			continue;

		QObjectList lst = w->children();
		foreach (QObject *obj, lst){
			ScriptEdit *edit = qobject_cast<ScriptEdit *>(obj);
			if (edit && edit == editor){
				return i;
			}
		}
	}

	return -1;
}

ScriptEdit* Note::editor(int index)
{
	if (index < 0 || index >= d_tab_widget->count())
		return 0;

	QWidget *w = d_tab_widget->widget(index);
	if (!w)
		return 0;

	QObjectList lst = w->children();
	foreach (QObject *obj, lst){
		ScriptEdit *edit = qobject_cast<ScriptEdit *>(obj);
		if (edit)
			return edit;
	}
	return 0;
}

ScriptEdit* Note::currentEditor()
{
	QWidget *w = d_tab_widget->currentWidget();
	if (!w)
		return 0;

	QObjectList lst = w->children();
	foreach (QObject *obj, lst){
		ScriptEdit *editor = qobject_cast<ScriptEdit *>(obj);
		if (editor)
			return editor;
	}
	return 0;
}

void Note::setTabStopWidth(int length)
{
	for (int i = 0; i < d_tab_widget->count(); i++){
		QWidget *w = d_tab_widget->widget(i);
		if (!w)
			continue;

		QObjectList lst = w->children();
		foreach (QObject *obj, lst){
			ScriptEdit *edit = qobject_cast<ScriptEdit *>(obj);
			if (edit){
				edit->setTabStopWidth(length);
				break;
			}
		}
	}
}

void Note::setName(const QString& name)
{
	currentEditor()->setObjectName(name);
	MdiSubWindow::setName(name);
}

void Note::modifiedNote()
{
	emit modifiedWindow(this);
}

void Note::save(const QString &fn, const QString &info, bool)
{
	QFile f(fn);
	if (!f.isOpen()){
		if (!f.open(QIODevice::Append))
			return;
	}
	QTextStream t( &f );
	t.setEncoding(QTextStream::UnicodeUTF8);
	t << "<note>\n";
	t << QString(name()) + "\t" + birthDate() + "\n";
	t << info;
	t << "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	t << "AutoExec\t" + QString(autoExec ? "1" : "0") + "\n";
	t << "<LineNumbers>" + QString::number(d_line_number_enabled) + "</LineNumbers>\n";

	f.close();
	for (int i = 0; i < tabs(); i++)
		saveTab(i, fn);

	if (!f.open(QIODevice::Append))
		return;

	t << "</note>\n";
}

void Note::saveTab(int index, const QString &fn)
{
	QFile f(fn);
	if (!f.open(QIODevice::Append))
		return;

	QTextStream t( &f );
	t.setEncoding(QTextStream::UnicodeUTF8);
	t << "<tab>\n";
	if (d_tab_widget->currentIndex() == index)
		t << "<active>1</active>\n";

	t << "<title>" + d_tab_widget->tabText(index) + "</title>\n";
	t << "<content>\n" + editor(index)->text().stripWhiteSpace() + "\n</content>";
	t << "\n</tab>\n";

	f.close();
}

void Note::restore(const QStringList& data)
{
	QStringList::ConstIterator line = data.begin();
	QStringList fields;

	fields = (*line).split("\t");
	if (fields[0] == "AutoExec"){
		setAutoexec(fields[1] == "1");
		line++;
	}

	bool lineNumbers = true;
	if ((*line).startsWith("<LineNumbers>")){
	  QString s = *line;
	  lineNumbers = s.remove("<LineNumbers>").remove("</LineNumbers>").toInt();
	  line++;
	}

	d_tab_widget->clear();

	int activeTab = 0;
	while (line != data.end() && *line != "</note>"){
		QString s = *line;
		if (s == "<tab>"){
			addTab();

			line++;
			s = *line;

			if (s.startsWith("<active>")){
				activeTab = d_tab_widget->currentIndex();
				line++;
				s = *line;
			}

			if (s.startsWith("<title>"))
				d_tab_widget->setTabText(d_tab_widget->currentIndex(), s.remove("<title>").remove("</title>"));

			line++;
			if (*line == "<content>")
				line++;

			currentEditor()->setUndoRedoEnabled(false);
			while (line != data.end() && *line != "</content>")
				currentEditor()->insertPlainText((*line++)+"\n");
			currentEditor()->setUndoRedoEnabled(true);

			line++;
			if (*line == "</content>")
				line++;
			if (*line == "</tab>")
				line++;
		} else if (s == "<content>"){//old style single editor notes
			addTab();
			line++;

			currentEditor()->setUndoRedoEnabled(false);
			while (line != data.end() && *line != "</content>")
				currentEditor()->insertPlainText((*line++)+"\n");
			currentEditor()->setUndoRedoEnabled(true);

			if (*line == "</content>")
				line++;
		} else {//even older style, no <content> tag, versions < 0.8.5
			if (!currentEditor())
				addTab();
			currentEditor()->insertPlainText((*line++) + "\n");
		}
	}

	showLineNumbers(lineNumbers);
	d_tab_widget->setCurrentIndex(activeTab);
	currentEditor()->moveCursor(QTextCursor::Start);
}

void Note::setAutoexec(bool exec)
{
  autoExec = exec;
  if (autoExec)
    currentEditor()->setPaletteBackgroundColor(QColor(255,239,185));
  else
    currentEditor()->unsetPalette();
}

void Note::setFont(const QFont& f)
{
	for (int i = 0; i < d_tab_widget->count(); i++){
		QWidget *w = d_tab_widget->widget(i);
		if (!w)
			continue;

		LineNumberDisplay *display = 0;
		ScriptEdit *editor = 0;

		QObjectList lst = w->children();
		foreach (QObject *obj, lst){
			display = qobject_cast<LineNumberDisplay *>(obj);
			if (display)
				break;
		}

		foreach (QObject *obj, lst){
			editor = qobject_cast<ScriptEdit *>(obj);
			if (editor)
				break;
		}

		if (!editor || !display)
			continue;

		if (editor->toPlainText().isEmpty()){
			editor->setCurrentFont(f);
			editor->document()->setDefaultFont(f);
			display->setCurrentFont(f);
			return;
		}

		editor->selectAll();
		editor->setCurrentFont(f);
		editor->document()->setDefaultFont(f);
		display->selectAll();
		display->setCurrentFont(f);
		display->updateLineNumbers(true);

		QTextCursor cursor = editor->textCursor();
		cursor.clearSelection();
		editor->setTextCursor(cursor);
	}
}
