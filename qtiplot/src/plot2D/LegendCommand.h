/***************************************************************************
    File                 : LegendCommand.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Legend undo/redo commands
 ***************************************************************************/

#ifndef LEGENDCOMMAND_H
#define LEGENDCOMMAND_H

#include <QUndoCommand>
#include <QString>
#include <QColor>
#include <QFont>
#include "LegendWidget.h"

class LegendSetTextCommand: public QUndoCommand
{
public:
	LegendSetTextCommand(LegendWidget *l, const QString& oldText, const QString& newText, const QString & text);
	virtual void redo();
	virtual void undo();
	int id() const override {return 1;};
	bool mergeWith(const QUndoCommand *other) override;

private:
	LegendWidget *d_legend;
	QString d_old_text, d_new_text;
};

class LegendSetColorCommand: public QUndoCommand
{
public:
	LegendSetColorCommand(LegendWidget *l, const QColor& oldColor, const QColor& newColor, const QString & text);
	virtual void redo();
	virtual void undo();
	int id() const override {return 2;};
	bool mergeWith(const QUndoCommand *other) override;

private:
	LegendWidget *d_legend;
	QColor d_old_color, d_new_color;
};

class LegendSetFontCommand: public QUndoCommand
{
public:
	LegendSetFontCommand(LegendWidget *l, const QFont& oldFont, const QFont& newFont, const QString & text);
	virtual void redo();
	virtual void undo();
	int id() const override {return 3;};
	bool mergeWith(const QUndoCommand *other) override;

private:
	LegendWidget *d_legend;
	QFont d_old_font, d_new_font;
};

#endif
