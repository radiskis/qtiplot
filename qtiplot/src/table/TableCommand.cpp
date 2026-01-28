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
	d_table->setText(d_row, d_col, d_new_text);
	d_table->notifyChanges(d_table->colName(d_col));
}

void TableEditCellCommand::undo()
{
	d_table->setText(d_row, d_col, d_old_text);
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
	d_table->setColNames(d_start_col, d_new_names);
}

void TableSetColNamesCommand::undo()
{
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
	d_table->setColName(d_col, d_new_name, false, false);
}

void TableSetColNameCommand::undo()
{
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
	d_table->setColumnType(d_col, d_new_type, false);
}

void TableSetColTypeCommand::undo()
{
	d_table->setColumnType(d_col, d_old_type, false);
}

/*************************************************************************/
/*           Class TableSetColFormatCommand                              */
/*************************************************************************/
TableSetColFormatCommand::TableSetColFormatCommand(Table *t, int col, const QString& oldFormat,
						const QString& newFormat, const QString & text):
QUndoCommand(text),
d_table(t),
d_col(col),
d_old_format(oldFormat),
d_new_format(newFormat)
{
	setText(t->objectName() + ": " + text);
}

void TableSetColFormatCommand::redo()
{
	// Need a generic setColFormat(col, format, pushUndo)
	// For now, assume format contains "type/prec" or just format
    // This is a bit tricky since Table has different setters for different types.
}

void TableSetColFormatCommand::undo()
{
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
	d_table->setColComment(d_col, d_new_comment);
}

void TableSetColCommentCommand::undo()
{
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
	d_table->setColPlotDesignation(d_col, d_new_pd, false);
	d_table->setHeaderColType();
}

void TableSetPlotDesignationCommand::undo()
{
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
	d_table->deleteRows(d_start_row, d_end_row, false);
}

void TableDeleteRowsCommand::undo()
{
	d_table->insertRows(d_start_row, d_data.count(), false);
	for (int i = 0; i < d_data.count(); i++){
		QStringList rowData = d_data[i];
		for (int j = 0; j < rowData.count(); j++){
			d_table->setText(d_start_row + i - 1, j, rowData[j]);
		}
	}
}

/*************************************************************************/
/*           Class TableInsertRowCommand                                 */
/*************************************************************************/
TableInsertRowCommand::TableInsertRowCommand(Table *t, int row, const QString& text):
QUndoCommand(text),
d_table(t),
d_row(row)
{
	setText(t->objectName() + ": " + text);
}

void TableInsertRowCommand::redo()
{
	d_table->insertRow(d_row, false);
}

void TableInsertRowCommand::undo()
{
	d_table->deleteRows(d_row, d_row, false);
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
	d_table->insertCols(d_start_col, d_count, false);
    for (int i = 0; i < d_names.count(); i++)
        d_table->setColName(d_start_col + i, d_names[i], false, false);
}

void TableAddColsCommand::undo()
{
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
	d_table->deleteColumns(d_names, false);
}

void TableDeleteColsCommand::undo()
{
	int count = d_names.count();
	for (int i = 0; i < count; i++){
		int col = d_start_col + i;
		d_table->insertColumn(col, false);
		d_table->setColName(col, d_names[i], false, false);
		d_table->setColComment(col, d_comments[i], false);
		d_table->setColumnType(col, (Table::ColType)d_types[i], false);
		d_table->setColPlotDesignation(col, (Table::PlotDesignation)d_plot_types[i], false);
		d_table->setColumnWidth(col, d_widths[i].toInt()); // This needs pushUndo = false too
        
        QStringList colData = d_cell_data[i];
        for (int row = 0; row < colData.count(); row++)
            d_table->setText(row, col, colData[row]);
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
	d_table->insertColumn(d_col, false);
}

void TableInsertColCommand::undo()
{
	d_table->deleteColumns(QStringList() << d_table->colName(d_col), false);
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
	MyTable *table = d_table->table();
	table->blockSignals(true);
	for (int i = 0; i < d_cols.count(); i++){
		int col = d_cols[i];
		QStringList data = d_new_data[i];
		for (int j = d_start_row; j <= d_end_row; j++)
			d_table->setText(j, col, data[j - d_start_row]);
	}
	table->blockSignals(false);

	for (int i = 0; i < d_cols.count(); i++)
		d_table->notifyChanges(d_table->colName(d_cols[i]));
}

void TableSetValuesCommand::undo()
{
	MyTable *table = d_table->table();
	table->blockSignals(true);
	for (int i = 0; i < d_cols.count(); i++){
		int col = d_cols[i];
		QStringList data = d_old_data[i];
		for (int j = d_start_row; j <= d_end_row; j++)
			d_table->setText(j, col, data[j - d_start_row]);
	}
	table->blockSignals(false);

	for (int i = 0; i < d_cols.count(); i++)
		d_table->notifyChanges(d_table->colName(d_cols[i]));
}
