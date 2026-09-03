/***************************************************************************
    File                 : TableCommand.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Table undo/redo commands
 ***************************************************************************/

#include "TableCommand.h"
#include <ApplicationWindow.h>

TableEditCellCommand::TableEditCellCommand(Table *t, int row, int col, const QString& oldText,
						const QString& newText, const QString & text):
QUndoCommand(text),
d_table(t),
d_row(row),
d_col(col),
d_old_text(oldText),
d_new_text(newText)
{
	setText(t->objectName() + ": " + text);
}

void TableEditCellCommand::redo()
{
	if (!d_table)
		return;
	bool blocked = d_table->table()->blockSignals(true);
	d_table->setText(d_row, d_col, d_new_text, false);
	d_table->table()->blockSignals(blocked);
	d_table->notifyChanges(d_table->colName(d_col));
}

void TableEditCellCommand::undo()
{
	if (!d_table)
		return;
	bool blocked = d_table->table()->blockSignals(true);
	d_table->setText(d_row, d_col, d_old_text, false);
	d_table->table()->blockSignals(blocked);
	d_table->notifyChanges(d_table->colName(d_col));
}

/*************************************************************************/
/*           Class TableSetColNamesCommand                               */
/*************************************************************************/
TableSetColNamesCommand::TableSetColNamesCommand(Table *t, int startCol, const QStringList& oldNames,
						const QStringList& newNames, const QString & text):
QUndoCommand(text),
d_table(t),
d_start_col(startCol),
d_old_names(oldNames),
d_new_names(newNames)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColNamesCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColNames(d_start_col, d_new_names);
}

void TableSetColNamesCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColNames(d_start_col, d_old_names);
}

/*************************************************************************/
/*           Class TableSetColNameCommand                                */
/*************************************************************************/
TableSetColNameCommand::TableSetColNameCommand(Table *t, int col, const QString& oldName,
						const QString& newName, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_name(oldName),
d_new_name(newName)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColNameCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColName(d_col, d_new_name, false, false);
}

void TableSetColNameCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColName(d_col, d_old_name, false, false);
}

/*************************************************************************/
/*           Class TableSetColTypeCommand                                */
/*************************************************************************/
TableSetColTypeCommand::TableSetColTypeCommand(Table *t, int col, Table::ColType oldType,
						Table::ColType newType, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_type(oldType),
d_new_type(newType)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColTypeCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColumnType(d_col, d_new_type, false);
}

void TableSetColTypeCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColumnType(d_col, d_old_type, false);
}

/*************************************************************************/
/*           Class TableSetColFormatCommand                              */
/*************************************************************************/
TableSetColFormatCommand::TableSetColFormatCommand(Table *t, int col, Table::ColType oldType, Table::ColType newType,
						const QString& oldFormat, const QString& newFormat, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_type(oldType),
d_new_type(newType),
d_old_format(oldFormat),
d_new_format(newFormat)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColFormatCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColFormat(d_col, d_new_type, d_new_format);
}

void TableSetColFormatCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColFormat(d_col, d_old_type, d_old_format);
}

/*************************************************************************/
/*           Class TableSetColCommentCommand                             */
/*************************************************************************/
TableSetColCommentCommand::TableSetColCommentCommand(Table *t, int col, const QString& oldComment,
						const QString& newComment, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_comment(oldComment),
d_new_comment(newComment)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColCommentCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColComment(d_col, d_new_comment, false);
}

void TableSetColCommentCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColComment(d_col, d_old_comment, false);
}

/*************************************************************************/
/*           Class TableSetPlotDesignationCommand                        */
/*************************************************************************/
TableSetPlotDesignationCommand::TableSetPlotDesignationCommand(Table *t, int col,
						Table::PlotDesignation oldPD, Table::PlotDesignation newPD, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_pd(oldPD),
d_new_pd(newPD)
{
	setText(t->objectName() + ": " + text);
}

void TableSetPlotDesignationCommand::redo()
{
	if (!d_table)
		return;
	d_table->setColPlotDesignation(d_col, d_new_pd, false);
	d_table->setHeaderColType();
}

void TableSetPlotDesignationCommand::undo()
{
	if (!d_table)
		return;
	d_table->setColPlotDesignation(d_col, d_old_pd, false);
	d_table->setHeaderColType();
}



/*************************************************************************/
/*           Class TableDeleteRowsCommand                                */
/*************************************************************************/
TableDeleteRowsCommand::TableDeleteRowsCommand(Table *t, int startRow, int endRow,
						const QList<QStringList>& data, const QString& text):
QUndoCommand(text),
d_table(t),
d_start_row(startRow),
d_end_row(endRow),
d_data(data)
{
	setText(t->objectName() + ": " + text);
}

void TableDeleteRowsCommand::redo()
{
	if (!d_table)
		return;
	d_table->deleteRows(d_start_row, d_end_row, false);
}

void TableDeleteRowsCommand::undo()
{
	if (!d_table)
		return;
	// d_start_row is 1-based (stored as start+1 in Table::deleteRows).
	// Table::insertRows(row, count, false) inserts at 0-based index (row-1+i),
	// so the first restored row lands at d_start_row-1 (0-based).
	// setText(d_start_row + i - 1, ...) == setText(d_start_row - 1 + i, ...)
	// which matches the insertion point exactly.
	d_table->insertRows(d_start_row, d_data.count(), false);
	for (int i = 0; i < d_data.count(); i++){
		QStringList rowData = d_data[i];
		for (int j = 0; j < rowData.count(); j++){
			d_table->setText(d_start_row + i - 1, j, rowData[j], false);
		}
	}
}

/*************************************************************************/
/*           Class TableInsertRowCommand                                 */
/*************************************************************************/
TableInsertRowCommand::TableInsertRowCommand(Table *t, int row, int count, const QString& text):
QUndoCommand(text.isEmpty() ? QObject::tr("Insert Rows") : text),
d_table(t),
d_row(row),
d_count(count)
{
	setText(t->objectName() + ": " + (text.isEmpty() ? QObject::tr("Insert Rows") : text));
}

void TableInsertRowCommand::redo()
{
	if (!d_table)
		return;
	d_table->insertRows(d_row, d_count, false);
}

void TableInsertRowCommand::undo()
{
	if (!d_table)
		return;
	d_table->deleteRows(d_row, d_row + d_count - 1, false);
}

/*************************************************************************/
/*           Class TableAddColsCommand                                   */
/*************************************************************************/
TableAddColsCommand::TableAddColsCommand(Table *t, int startCol, int count, const QStringList& names, const QString& text):
QUndoCommand(text),
d_table(t),
d_start_col(startCol),
d_count(count),
d_names(names)
{
	setText(t->objectName() + ": " + text);
}

void TableAddColsCommand::redo()
{
	if (!d_table)
		return;
	d_table->insertCols(d_start_col, d_count, false);
    for (int i = 0; i < d_names.count(); i++)
        d_table->setColName(d_start_col + i, d_names[i], false, false);
}

void TableAddColsCommand::undo()
{
	if (!d_table)
		return;
	d_table->deleteColumns(d_names, false);
}

/*************************************************************************/
/*           Class TableDeleteColsCommand                                */
/*************************************************************************/
TableDeleteColsCommand::TableDeleteColsCommand(Table *t, int startCol, int endCol,
						const QList<QStringList>& cellData, const QStringList& names,
						const QStringList& comments, const QStringList& formats,
						const QList<int>& types, const QList<int>& plotTypes,
						const QStringList& widths, const QStringList& commands, const QString& text):
QUndoCommand(text),
d_table(t),
d_start_col(startCol),
d_end_col(endCol),
d_cell_data(cellData),
d_names(names),
d_comments(comments),
d_formats(formats),
d_types(types),
d_plot_types(plotTypes),
d_widths(widths),
d_commands(commands)
{
	setText(t->objectName() + ": " + text);
}

void TableDeleteColsCommand::redo()
{
	if (!d_table)
		return;
	d_table->deleteColumns(d_names, false);
}

void TableDeleteColsCommand::undo()
{
	if (!d_table)
		return;
	int count = d_names.count();
	for (int i = 0; i < count; i++){
		int col = d_start_col + i;
		d_table->insertColumn(col, false);
		d_table->setColName(col, d_names[i], false, false);
		d_table->setColComment(col, d_comments[i], false);
		d_table->setColumnType(col, (Table::ColType)d_types[i], false);
		d_table->setColPlotDesignation(col, (Table::PlotDesignation)d_plot_types[i], false);
		d_table->setColumnWidth(col, d_widths[i].toInt());
		if (i < d_formats.size())
			d_table->setColumnFormat(col, d_formats[i]);
		if (i < d_commands.size())
			d_table->setCommand(col, d_commands[i]);
        
        QStringList colData = d_cell_data[i];
        for (int row = 0; row < colData.count(); row++)
            d_table->setText(row, col, colData[row], false);
	}
    d_table->setHeaderColType();
}

/*************************************************************************/
/*           Class TableInsertColCommand                                 */
/*************************************************************************/
TableInsertColCommand::TableInsertColCommand(Table *t, int col, const QString& text):
QUndoCommand(text),
d_table(t),
d_col(col)
{
	setText(t->objectName() + ": " + text);
}

void TableInsertColCommand::redo()
{
	if (!d_table)
		return;
	d_table->insertColumn(d_col, false);
}

void TableInsertColCommand::undo()
{
	if (!d_table)
		return;
	d_table->deleteColumns(QStringList() << d_table->colLabel(d_col), false);
}

/*************************************************************************/
/*           Class TableSetValuesCommand                                 */
/*************************************************************************/
TableSetValuesCommand::TableSetValuesCommand(Table *t, int startRow, int endRow, const QList<int>& cols,
						const QList<QStringList>& oldData, const QList<QStringList>& newData, const QString & text):
QUndoCommand(text),
d_table(t),
d_start_row(startRow),
d_end_row(endRow),
d_cols(cols),
d_old_data(oldData),
d_new_data(newData)
{
	setText(t->objectName() + ": " + text);
}

void TableSetValuesCommand::redo()
{
	if (!d_table)
		return;
	MyTable *table = d_table->table();
	table->blockSignals(true);
	for (int i = 0; i < d_cols.count(); i++){
		int col = d_cols[i];
		QStringList data = d_new_data[i];
		for (int j = d_start_row; j <= d_end_row; j++)
			d_table->setText(j, col, data[j - d_start_row], false);
	}
	table->blockSignals(false);

	for (int i = 0; i < d_cols.count(); i++)
		d_table->notifyChanges(d_table->colName(d_cols[i]));
}

void TableSetValuesCommand::undo()
{
	if (!d_table)
		return;
	MyTable *table = d_table->table();
	table->blockSignals(true);
	for (int i = 0; i < d_cols.count(); i++){
		int col = d_cols[i];
		QStringList data = d_old_data[i];
		for (int j = d_start_row; j <= d_end_row; j++)
			d_table->setText(j, col, data[j - d_start_row], false);
	}
	table->blockSignals(false);

	for (int i = 0; i < d_cols.count(); i++)
		d_table->notifyChanges(d_table->colName(d_cols[i]));
}

/*************************************************************************/
/*           Class TableSwapColumnsCommand                               */
/*************************************************************************/
TableSwapColumnsCommand::TableSwapColumnsCommand(Table *t, int col1, int col2, const QString &text):
QUndoCommand(text.isEmpty() ? QObject::tr("Swap Columns") : text),
d_table(t),
d_col1(col1),
d_col2(col2)
{
	if (t)
		setText(t->objectName() + ": " + this->text());
}

void TableSwapColumnsCommand::redo()
{
	if (!d_table)
		return;
	d_table->swapColumns(d_col1, d_col2, false);
}

void TableSwapColumnsCommand::undo()
{
	if (!d_table)
		return;
	d_table->swapColumns(d_col1, d_col2, false);
}

/*************************************************************************/
/*           Class TableMoveColumnCommand                                */
/*************************************************************************/
TableMoveColumnCommand::TableMoveColumnCommand(Table *t, int from, int to, const QString &text):
QUndoCommand(text.isEmpty() ? QObject::tr("Move Column") : text),
d_table(t),
d_from(from),
d_to(to)
{
	if (t)
		setText(t->objectName() + ": " + this->text());
}

void TableMoveColumnCommand::redo()
{
	if (!d_table)
		return;
	d_table->moveColumnBy(d_to - d_from, false);
}

void TableMoveColumnCommand::undo()
{
	if (!d_table)
		return;
	d_table->moveColumnBy(d_from - d_to, false);
}

/*************************************************************************/
/*           Class TableSetColumnWidthCommand                            */
/*************************************************************************/
TableSetColumnWidthCommand::TableSetColumnWidthCommand(Table *t, int col, int oldWidth, int newWidth, bool allCols,
													   const QList<int>& oldWidths, const QString &text):
QUndoCommand(text.isEmpty() ? QObject::tr("Set Column Width") : text),
d_table(t),
d_col(col),
d_old_width(oldWidth),
d_new_width(newWidth),
d_all_cols(allCols),
d_old_widths(oldWidths)
{
	if (t)
		setText(t->objectName() + ": " + this->text());
}

void TableSetColumnWidthCommand::redo()
{
	if (!d_table)
		return;
	if (d_all_cols)
		d_table->setColumnWidth(d_new_width, true, false);
	else
		d_table->setColumnWidth(d_col, d_new_width, false);
}

void TableSetColumnWidthCommand::undo()
{
	if (!d_table)
		return;
	if (d_all_cols) {
		for (int i = 0; i < d_old_widths.size() && i < d_table->numCols(); i++)
			d_table->setColumnWidth(i, d_old_widths[i], false);
	} else {
		d_table->setColumnWidth(d_col, d_old_width, false);
	}
}

/*************************************************************************/
/*           Class TableSetReadOnlyCommand                               */
/*************************************************************************/
TableSetReadOnlyCommand::TableSetReadOnlyCommand(Table *t, int col, bool oldState, bool newState, const QString &text):
QUndoCommand(text.isEmpty() ? QObject::tr("Set Read Only") : text),
d_table(t),
d_col(col),
d_old_state(oldState),
d_new_state(newState)
{
	if (t)
		setText(t->objectName() + ": " + this->text());
}

void TableSetReadOnlyCommand::redo()
{
	if (!d_table)
		return;
	d_table->setReadOnlyColumn(d_col, d_new_state, false);
}

void TableSetReadOnlyCommand::undo()
{
	if (!d_table)
		return;
	d_table->setReadOnlyColumn(d_col, d_old_state, false);
}
