/***************************************************************************
    File                 : Note.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Hoener zu Siederdissen,
					  Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
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

#include <QLayout>
#include <QFile>
#include <QTextStream>

Note::Note(ScriptingEnv *env, const QString& label, ApplicationWindow* parent, const QString& name, Qt::WFlags f)
		 : MdiSubWindow(label, parent, name, f)
{
init(env);
}

void Note::init(ScriptingEnv *env)
{
autoExec = false;
te = new ScriptEdit(env, this, name());
te->setContext(this);

d_line_number = new LineNumberDisplay(te, this);
d_frame = new QWidget(this);

QHBoxLayout *hbox = new QHBoxLayout(d_frame);
hbox->setMargin(0);
hbox->setSpacing(0);
hbox->addWidget(d_line_number);
hbox->addWidget(te);
setWidget(d_frame);

setGeometry(0, 0, 500, 200);
connect(te, SIGNAL(textChanged()), this, SLOT(modifiedNote()));
connect(te, SIGNAL(dirPathChanged(const QString& )), this, SIGNAL(dirPathChanged(const QString&)));
}

void Note::setName(const QString& name)
{
	te->setObjectName(name);
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
	t << "<LineNumbers>" + QString::number(d_line_number->isVisible()) + "</LineNumbers>\n";
	t << "<content>\n" + te->text().stripWhiteSpace() + "\n</content>";
	t << "\n</note>\n";
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

  if (*line == "<content>") line++;
  while (line != data.end() && *line != "</content>")
    te->insertPlainText((*line++)+"\n");

  d_line_number->setVisible(lineNumbers);
  te->moveCursor(QTextCursor::Start);
}

void Note::setAutoexec(bool exec)
{
  autoExec = exec;
  if (autoExec)
    te->setPaletteBackgroundColor(QColor(255,239,185));
  else
    te->unsetPalette();
}

void Note::setFont(const QFont& f)
{
    if (te->toPlainText().isEmpty()){
        te->setCurrentFont(f);
        d_line_number->setCurrentFont(f);
        return;
    }

    te->selectAll();
    te->setCurrentFont(f);
    d_line_number->setCurrentFont(f);
    d_line_number->updateLineNumbers(true);

    QTextCursor cursor = te->textCursor();
    cursor.clearSelection();
    te->setTextCursor(cursor);
}
