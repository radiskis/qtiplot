/***************************************************************************
    File                 : LegendCommand.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Legend undo/redo commands
 ***************************************************************************/

#include "LegendCommand.h"
#include <MultiLayer.h>

/*************************************************************************/
/*           Class LegendSetTextCommand                                  */
/*************************************************************************/
LegendSetTextCommand::LegendSetTextCommand(LegendWidget *l, const QString& oldText,
						const QString& newText, const QString & text):
QUndoCommand(text),
d_legend(l),
d_old_text(oldText),
d_new_text(newText)
{
	setText(l->objectName() + ": " + text);
}

void LegendSetTextCommand::redo()
{
	d_legend->setText(d_new_text, false);
}

void LegendSetTextCommand::undo()
{
	d_legend->setText(d_old_text, false);
}

bool LegendSetTextCommand::mergeWith(const QUndoCommand *other)
{
	if (other->id() != id())
		return false;

	const LegendSetTextCommand *cmd = static_cast<const LegendSetTextCommand *>(other);
	if (cmd->d_legend != d_legend)
		return false;

	d_new_text = cmd->d_new_text;
	return true;
}

/*************************************************************************/
/*           Class LegendSetColorCommand                                 */
/*************************************************************************/
LegendSetColorCommand::LegendSetColorCommand(LegendWidget *l, const QColor& oldColor,
						const QColor& newColor, const QString & text):
QUndoCommand(text),
d_legend(l),
d_old_color(oldColor),
d_new_color(newColor)
{
	setText(l->objectName() + ": " + text);
}

void LegendSetColorCommand::redo()
{
	d_legend->setTextColor(d_new_color, false);
}

void LegendSetColorCommand::undo()
{
	d_legend->setTextColor(d_old_color, false);
}

bool LegendSetColorCommand::mergeWith(const QUndoCommand *other)
{
	if (other->id() != id())
		return false;

	const LegendSetColorCommand *cmd = static_cast<const LegendSetColorCommand *>(other);
	if (cmd->d_legend != d_legend)
		return false;

	d_new_color = cmd->d_new_color;
	return true;
}

/*************************************************************************/
/*           Class LegendSetFontCommand                                  */
/*************************************************************************/
LegendSetFontCommand::LegendSetFontCommand(LegendWidget *l, const QFont& oldFont,
						const QFont& newFont, const QString & text):
QUndoCommand(text),
d_legend(l),
d_old_font(oldFont),
d_new_font(newFont)
{
	setText(l->objectName() + ": " + text);
}

void LegendSetFontCommand::redo()
{
	d_legend->setFont(d_new_font, false);
}

void LegendSetFontCommand::undo()
{
	d_legend->setFont(d_old_font, false);
}

bool LegendSetFontCommand::mergeWith(const QUndoCommand *other)
{
	if (other->id() != id())
		return false;

	const LegendSetFontCommand *cmd = static_cast<const LegendSetFontCommand *>(other);
	if (cmd->d_legend != d_legend)
		return false;

	d_new_font = cmd->d_new_font;
	return true;
}
