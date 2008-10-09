/***************************************************************************
    File                 : ScriptEdit.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Editor widget for scripting code

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
#include "ScriptEdit.h"
#include "Note.h"
#include "PythonSyntaxHighlighter.h"
#include "FindReplaceDialog.h"

#include <QAction>
#include <QMenu>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>
#include <QShortcut>

ScriptEdit::ScriptEdit(ScriptingEnv *env, QWidget *parent, const char *name)
  : QTextEdit(parent, name), scripted(env), d_error(false), d_completer(0),
  d_file_name(QString::null), d_highlighter(0)
{
	myScript = scriptEnv->newScript("", this, name);
	connect(myScript, SIGNAL(error(const QString&, const QString&, int)), this, SLOT(insertErrorMsg(const QString&)));
	connect(myScript, SIGNAL(print(const QString&)), this, SLOT(scriptPrint(const QString&)));

	setLineWrapMode(NoWrap);
	setUndoRedoEnabled(true);
	setTextFormat(Qt::PlainText);
	setAcceptRichText (false);

	if (scriptEnv->name() == QString("Python"))
		d_highlighter = new PythonSyntaxHighlighter(this);

	d_fmt_default.setBackground(palette().brush(QPalette::Base));
	d_fmt_failure.setBackground(QBrush(QColor(255,128,128)));

	printCursor = textCursor();
	scriptsDirPath = qApp->applicationDirPath();

	actionExecute = new QAction(tr("E&xecute"), this);
	actionExecute->setShortcut( tr("Ctrl+J") );
	connect(actionExecute, SIGNAL(activated()), this, SLOT(execute()));

	actionExecuteAll = new QAction(tr("Execute &All"), this);
	actionExecuteAll->setShortcut( tr("Ctrl+Shift+J") );
	connect(actionExecuteAll, SIGNAL(activated()), this, SLOT(executeAll()));

	actionEval = new QAction(tr("&Evaluate Expression"), this);
	actionEval->setShortcut( tr("Ctrl+Return") );
	connect(actionEval, SIGNAL(activated()), this, SLOT(evaluate()));

	actionPrint = new QAction(tr("&Print"), this);
	connect(actionPrint, SIGNAL(activated()), this, SLOT(print()));

	actionImport = new QAction(tr("&Import..."), this);
	actionImport->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_O));
	connect(actionImport, SIGNAL(activated()), this, SLOT(import()));

	QShortcut *accelImport = new QShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_O), this);
	connect(accelImport, SIGNAL(activated()), this, SLOT(import()));

	actionSave = new QAction(tr("&Save"), this);
	actionSave->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_S));
	connect(actionSave, SIGNAL(activated()), this, SLOT(save()));

	QShortcut *accelSave = new QShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_S), this);
	connect(accelSave, SIGNAL(activated()), this, SLOT(save()));

	actionExport = new QAction(tr("Sa&ve as..."), this);
	connect(actionExport, SIGNAL(activated()), this, SLOT(exportASCII()));

	actionFind = new QAction(tr("&Find..."), this);
	actionFind->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_F));
	connect(actionFind, SIGNAL(activated()), this, SLOT(find()));

	QShortcut *accelFind = new QShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_F), this);
	connect(accelFind, SIGNAL(activated()), this, SLOT(find()));

	actionReplace = new QAction(tr("&Replace..."), this);
	actionReplace->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_R));
	connect(actionReplace, SIGNAL(activated()), this, SLOT(replace()));

	QShortcut *accelReplace = new QShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_R), this);
	connect(accelReplace, SIGNAL(activated()), this, SLOT(replace()));

	functionsMenu = new QMenu(this);
	Q_CHECK_PTR(functionsMenu);
	connect(functionsMenu, SIGNAL(triggered(QAction *)), this, SLOT(insertFunction(QAction *)));
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(handleContentsChange(int,int,int)));
}

void ScriptEdit::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
	{
		scriptingChangeEvent((ScriptingChangeEvent*)e);
		delete myScript;
		myScript = scriptEnv->newScript("", this, name());
		connect(myScript, SIGNAL(error(const QString&, const QString&, int)), this, SLOT(insertErrorMsg(const QString&)));
		connect(myScript, SIGNAL(print(const QString&)), this, SLOT(scriptPrint(const QString&)));

		if (scriptEnv->name() == QString("Python") && !d_highlighter)
			d_highlighter = new PythonSyntaxHighlighter(this);
		else {
			if (d_highlighter)
				delete d_highlighter;
			d_highlighter = 0;
		}
	}
}

void ScriptEdit::focusInEvent(QFocusEvent *e)
{
     if (d_completer)
         d_completer->setWidget(this);
     QTextEdit::focusInEvent(e);
}

void ScriptEdit::keyPressEvent(QKeyEvent *e)
{
    if (d_completer && d_completer->popup()->isVisible()) {
         // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
             e->ignore();
             return; // let the completer do default behavior
        default:
            break;
        }
     }

    QTextEdit::keyPressEvent(e);
    if (d_completer && !d_completer->popup()->isVisible() && e->key() == Qt::Key_Return)
        updateIndentation();

     bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_U); // CTRL+U
     const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
     if (!d_completer || (ctrlOrShift && e->text().isEmpty()))
         return;

     static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
     bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
     QString completionPrefix = textUnderCursor();

     if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
                       || eow.contains(e->text().right(1)))) {
         d_completer->popup()->hide();
         return;
     }

     if (completionPrefix != d_completer->completionPrefix()) {
         d_completer->setCompletionPrefix(completionPrefix);
         d_completer->popup()->setCurrentIndex(d_completer->completionModel()->index(0, 0));
     }

     QRect cr = cursorRect();
     cr.setWidth(d_completer->popup()->sizeHintForColumn(0)
                 + d_completer->popup()->verticalScrollBar()->sizeHint().width());
     d_completer->complete(cr); // popup it up!
}

void ScriptEdit::contextMenuEvent(QContextMenuEvent *e)
{
	QMenu *menu = createStandardContextMenu();
	Q_CHECK_PTR(menu);

	menu->insertSeparator();
	menu->addAction(actionFind);
	menu->addAction(actionReplace);
	menu->insertSeparator();
	menu->addAction(actionPrint);
	menu->addAction(actionImport);
	menu->insertSeparator();
	menu->addAction(actionSave);
	menu->addAction(actionExport);
	menu->insertSeparator();

	menu->addAction(actionExecute);
	menu->addAction(actionExecuteAll);
	menu->addAction(actionEval);
	if (parent()->isA("Note")){
		Note *sp = (Note*) parent();
		QAction *actionAutoexec = new QAction(tr("Auto&exec"), menu);
		actionAutoexec->setToggleAction(true);
		actionAutoexec->setOn(sp->autoexec());
		connect(actionAutoexec, SIGNAL(toggled(bool)), sp, SLOT(setAutoexec(bool)));
		menu->addAction(actionAutoexec);
	}

	functionsMenu->clear();
	functionsMenu->setTearOffEnabled(true);
	QStringList flist = scriptEnv->mathFunctions();
	QMenu *submenu=NULL;
	for (int i=0; i<flist.size(); i++)
	{
		QAction *newAction;
		QString menupart;
		// group by text before "_" (would make sense if we renamed several functions...)
		/*if (flist[i].contains("_") || (i<flist.size()-1 && flist[i+1].split("_")[0]==flist[i]))
			menupart = flist[i].split("_")[0];
		else
			menupart = "";*/
		// group by first letter, avoiding submenus with only one entry
		if ((i==0 || flist[i-1][0] != flist[i][0]) && (i==flist.size()-1 || flist[i+1][0] != flist[i][0]))
			menupart = "";
		else
			menupart = flist[i].left(1);
		if (!menupart.isEmpty()) {
			if (!submenu || menupart != submenu->title())
				submenu = functionsMenu->addMenu(menupart);
			newAction = submenu->addAction(flist[i]);
		} else
			newAction = functionsMenu->addAction(flist[i]);
		newAction->setData(i);
		newAction->setWhatsThis(scriptEnv->mathFunctionDoc(flist[i]));
	}
	functionsMenu->setTitle(tr("&Functions"));
	menu->addMenu(functionsMenu);

	menu->exec(e->globalPos());
	delete menu;
}

void ScriptEdit::insertErrorMsg(const QString &message)
{
	QString err = message;
	err.prepend("\n").replace("\n","\n#> ");
	int start = printCursor.position();
	printCursor.insertText(err);
	printCursor.setPosition(start, QTextCursor::KeepAnchor);
	setTextCursor(printCursor);
	d_error = true;
}

void ScriptEdit::scriptPrint(const QString &text)
{
	if(lineNumber(printCursor.position()) == lineNumber(textCursor().selectionEnd()))
		printCursor.insertText("\n");
	printCursor.insertText(text);
}

void ScriptEdit::insertFunction(const QString &fname)
{
	QTextCursor cursor = textCursor();
	QString markedText = cursor.selectedText();
	cursor.insertText(fname+"("+markedText+")");
	if(markedText.isEmpty()){
		// if no text is selected, place cursor inside the ()
		// instead of after it
		cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
		// the next line makes the selection visible to the user
		// (the line above only changes the selection in the
		// underlying QTextDocument)
		setTextCursor(cursor);
	}
}

void ScriptEdit::insertFunction(QAction *action)
{
	insertFunction(scriptEnv->mathFunctions()[action->data().toInt()]);
}

int ScriptEdit::lineNumber(int pos) const
{
	int n = 1;
	for(QTextBlock i=document()->begin(); !i.contains(pos) && i!=document()->end(); i=i.next())
		n++;
	return n;
}

void ScriptEdit::handleContentsChange(int position, int, int)
{
	if (d_changing_fmt) return; // otherwise we overwrite our own changes
	QTextCursor cursor = textCursor();
	cursor.setPosition(position);
	cursor.mergeBlockFormat(d_fmt_default);
	textChanged();
}

void ScriptEdit::execute()
{
	QString fname = "<%1:%2>";
	fname = fname.arg(name());
	QTextCursor codeCursor = textCursor();
	if (codeCursor.selectedText().isEmpty()){
		codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	}
	fname = fname.arg(lineNumber(codeCursor.selectionStart()));

	myScript->setName(fname);
	myScript->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator,"\n"));
	//printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
	//printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
	myScript->exec();

	d_changing_fmt = true;
	if (d_error)
		codeCursor.mergeBlockFormat(d_fmt_failure);
	else
		codeCursor.mergeBlockFormat(d_fmt_default);

	d_changing_fmt = false;
	d_error = false;
}

void ScriptEdit::executeAll()
{
	QTextCursor codeCursor = textCursor();
	codeCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	codeCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

	QString fname = "<%1>";
	fname = fname.arg(name());
	myScript->setName(fname);
	myScript->setCode(text());
	myScript->exec();

	d_changing_fmt = true;
	if (d_error)
		codeCursor.mergeBlockFormat(d_fmt_failure);
	else
		codeCursor.mergeBlockFormat(d_fmt_default);

	d_changing_fmt = false;
	d_error = false;
}

void ScriptEdit::evaluate()
{
	QString fname = "<%1:%2>";
	fname = fname.arg(name());
	QTextCursor codeCursor = textCursor();
	if (codeCursor.selectedText().isEmpty()){
		codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	}
	fname = fname.arg(lineNumber(codeCursor.selectionStart()));

	myScript->setName(fname);
	myScript->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator,"\n"));
	//printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
	//printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
	QVariant res = myScript->eval();

	d_changing_fmt = true;
	if (d_error)
		codeCursor.mergeBlockFormat(d_fmt_failure);
	else
		codeCursor.mergeBlockFormat(d_fmt_default);

	if (res.isValid())
		if (!res.isNull() && res.canConvert(QVariant::String)){
			QString strVal = res.toString();
			strVal.replace("\n", "\n#> ");

			printCursor.insertText("\n");
			printCursor.mergeBlockFormat(d_fmt_default);
			if (!strVal.isEmpty())
				 printCursor.insertText("#> "+strVal+"\n");
		}

	d_changing_fmt = false;
	d_error = false;
	setTextCursor(printCursor);
}

void ScriptEdit::exportPDF(const QString& fileName)
{
	QTextDocument *doc = document();
	QPrinter printer;
	printer.setColorMode(QPrinter::GrayScale);
	printer.setCreator("QtiPlot");
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
	doc->print(&printer);
}

void ScriptEdit::print()
{
	QTextDocument *doc = document();
	QPrinter printer;
	printer.setColorMode(QPrinter::GrayScale);
	QPrintDialog printDialog(&printer);
	// TODO: Write a dialog to use more features of Qt4's QPrinter class
	if (printDialog.exec() == QDialog::Accepted)
		doc->print(&printer);
}

QString ScriptEdit::import(const QString &filename)
{
	QString filter = tr("Text") + " (*.txt *.TXT);;";
	filter += scriptEnv->fileFilter();
	filter += tr("All Files")+" (*)";

	QString f;
	if (filename.isEmpty())
		f = QFileDialog::getOpenFileName(this, tr("QtiPlot - Import Text From File"), scriptsDirPath, filter);
	else
		f = filename;
	if (f.isEmpty()) return QString::null;

	QFile file(f);
	if (!file.open(IO_ReadOnly)){
		QMessageBox::critical(this, tr("QtiPlot - Error Opening File"), tr("Could not open file \"%1\" for reading.").arg(f));
		return QString::null;
	}

	setFileName(f);

	QFileInfo fi(f);
	if (scriptsDirPath != fi.absolutePath()){
		scriptsDirPath = fi.absolutePath();
		emit dirPathChanged(scriptsDirPath);
	}

	clear();
	QTextStream ts(&file);
	ts.setEncoding(QTextStream::UnicodeUTF8);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qApp->processEvents(QEventLoop::ExcludeUserInput);

    blockSignals(true);
    insertPlainText(ts.readAll());
    blockSignals(false);
    textChanged();

	file.close();
	QApplication::restoreOverrideCursor();
	return f;
}

QString ScriptEdit::save()
{
	return exportASCII(d_file_name);
}

QString ScriptEdit::exportASCII(const QString &filename)
{
	QString filter = tr("Text") + " (*.txt *.TXT);;";
	filter += scriptEnv->fileFilter();
	filter += tr("All Files")+" (*)";

	QString selectedFilter;
	QString fn;
	if (filename.isEmpty())
		fn = QFileDialog::getSaveFileName(this, tr("Save Text to File"), scriptsDirPath, filter, &selectedFilter);
	else
		fn = filename;

	if (!fn.isEmpty()){
		QFileInfo fi(fn);
		scriptsDirPath = fi.absolutePath();

		QString baseName = fi.fileName();
		if (!baseName.contains(".")){
			if (selectedFilter.contains(".txt"))
				fn.append(".txt");
			else if (selectedFilter.contains(".py"))
				fn.append(".py");
		}

		QFile f(fn);
		if (!f.open(IO_WriteOnly)){
			QMessageBox::critical(0, tr("QtiPlot - File Save Error"),
						tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fn));
			return QString::null;
		}

		QTextStream t( &f );
		t.setEncoding(QTextStream::UnicodeUTF8);
		t << text();
		f.close();

		setFileName(fn);
	}
	return fn;
}

void ScriptEdit::setFileName(const QString& fn)
{
	if (d_file_name == fn)
		return;

	d_file_name = fn;
	Note *note = qobject_cast<Note *>(myScript->context());
	if (note)
		note->setWindowLabel(d_file_name);
}

void ScriptEdit::updateIndentation()
{
	QTextCursor cursor = textCursor();
	QTextBlock para = cursor.block();
	QString prev = para.previous().text();
	int i;
	for (i=0; prev[i].isSpace(); i++);
	QString indent = prev.mid(0, i);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
	cursor.insertText(indent);
}

void ScriptEdit::setDirPath(const QString& path)
{
	QFileInfo fi(path);
	if (!fi.exists() || !fi.isDir())
		return;

	scriptsDirPath = path;
}

 void ScriptEdit::setCompleter(QCompleter *completer)
 {
     if (d_completer)
         QObject::disconnect(d_completer, 0, this, 0);

     d_completer = completer;

     if (!d_completer)
         return;

     d_completer->setWidget(this);
     QObject::connect(d_completer, SIGNAL(activated(const QString&)),
                      this, SLOT(insertCompletion(const QString&)));
 }

 void ScriptEdit::insertCompletion(const QString& completion)
 {
     if (d_completer->widget() != this)
         return;

	 QTextCursor tc = textCursor();
     int extra = completion.length() - d_completer->completionPrefix().length();
     tc.movePosition(QTextCursor::Left);
     tc.movePosition(QTextCursor::EndOfWord);
     tc.insertText(completion.right(extra));
     setTextCursor(tc);
 }

 QString ScriptEdit::textUnderCursor() const
 {
     QTextCursor tc = textCursor();
     tc.select(QTextCursor::WordUnderCursor);
     return tc.selectedText();
 }

void ScriptEdit::rehighlight()
{
	if (scriptEnv->name() != QString("Python"))
		return;

	if (d_highlighter)
		delete d_highlighter;

	d_highlighter = new PythonSyntaxHighlighter(this);
}

void ScriptEdit::find(bool replace)
{
	if (toPlainText().isEmpty())
		return;

	FindReplaceDialog *frd = new FindReplaceDialog(this, replace, (QWidget *)scriptingEnv()->application());
	frd->exec();
}
