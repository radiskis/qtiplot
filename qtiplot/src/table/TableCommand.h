/***************************************************************************
    File                 : TableCommand.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Table undo/redo commands
 ***************************************************************************/

#ifndef TABLE_COMMAND_H
#define TABLE_COMMAND_H

#include "Table.h"
#include <QUndoCommand>
#include <QPointer>
#include <QStringList>

class TableSetColNamesCommand: public QUndoCommand
{
public:
	TableSetColNamesCommand(Table *t, int startCol, const QStringList& oldNames, const QStringList& newNames, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_col;
	QStringList d_old_names, d_new_names;
};

class TableEditCellCommand: public QUndoCommand
{
public:
	TableEditCellCommand(Table *t, int row, int col, const QString& oldText, const QString& newText, const QString & text,
	                     bool hasOldVal = false, double oldVal = 0.0, bool hasNewVal = false, double newVal = 0.0);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_row, d_col;
	QString d_old_text, d_new_text;
	bool d_has_old_val, d_has_new_val;
	double d_old_val, d_new_val;
};

class TableSetColNameCommand: public QUndoCommand
{
public:
	TableSetColNameCommand(Table *t, int col, const QString& oldName, const QString& newName, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	QString d_old_name, d_new_name;
};

class TableSetColTypeCommand: public QUndoCommand
{
public:
	TableSetColTypeCommand(Table *t, int col, Table::ColType oldType, Table::ColType newType, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	Table::ColType d_old_type, d_new_type;
};

class TableSetColFormatCommand: public QUndoCommand
{
public:
	TableSetColFormatCommand(Table *t, int col, Table::ColType oldType, Table::ColType newType,
							 const QString& oldFormat, const QString& newFormat, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	Table::ColType d_old_type, d_new_type;
	QString d_old_format, d_new_format;
};

class TableSetColCommentCommand: public QUndoCommand
{
public:
	TableSetColCommentCommand(Table *t, int col, const QString& oldComment, const QString& newComment, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	QString d_old_comment, d_new_comment;
};

class TableSetPlotDesignationCommand: public QUndoCommand
{
public:
	TableSetPlotDesignationCommand(Table *t, int col, Table::PlotDesignation oldPD, Table::PlotDesignation newPD, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	Table::PlotDesignation d_old_pd, d_new_pd;
};



class TableDeleteRowsCommand: public QUndoCommand
{
public:
	TableDeleteRowsCommand(Table *t, int startRow, int endRow, const QList<QStringList>& data, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_row, d_end_row;
	QList<QStringList> d_data;
};

class TableInsertRowCommand: public QUndoCommand
{
public:
	TableInsertRowCommand(Table *t, int row, int count = 1, const QString& text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_row;
	int d_count;
};

class TableAddColsCommand: public QUndoCommand
{
public:
	TableAddColsCommand(Table *t, int startCol, int count, const QStringList& names, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_col, d_count;
	QStringList d_names;
};

class TableDeleteColsCommand: public QUndoCommand
{
public:
	TableDeleteColsCommand(Table *t, int startCol, int endCol, const QList<QStringList>& cellData,
							const QStringList& names, const QStringList& comments,
							const QStringList& formats, const QList<int>& types,
							const QList<int>& plotTypes, const QStringList& widths,
							const QStringList& commands, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_col, d_end_col;
	QList<QStringList> d_cell_data;
	QStringList d_names, d_comments, d_formats, d_widths, d_commands;
	QList<int> d_types, d_plot_types;
};

class TableInsertColCommand: public QUndoCommand
{
public:
	TableInsertColCommand(Table *t, int col, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
};

class TableSetValuesCommand: public QUndoCommand
{
public:
	TableSetValuesCommand(Table *t, int startRow, int endRow, const QList<int>& cols,
						const QList<QStringList>& oldData, const QList<QStringList>& newData, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_row, d_end_row;
	QList<int> d_cols;
	QList<QStringList> d_old_data, d_new_data;
};

class TableSwapColumnsCommand: public QUndoCommand
{
public:
	TableSwapColumnsCommand(Table *t, int col1, int col2, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col1, d_col2;
};

class TableMoveColumnCommand: public QUndoCommand
{
public:
	TableMoveColumnCommand(Table *t, int from, int to, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_from, d_to;
};

class TableSetColumnWidthCommand: public QUndoCommand
{
public:
	TableSetColumnWidthCommand(Table *t, int col, int oldWidth, int newWidth, bool allCols = false,
							   const QList<int>& oldWidths = QList<int>(), const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	int d_old_width, d_new_width;
	bool d_all_cols;
	QList<int> d_old_widths;
};

class TableSetReadOnlyCommand: public QUndoCommand
{
public:
	TableSetReadOnlyCommand(Table *t, int col, bool oldState, bool newState, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col;
	bool d_old_state, d_new_state;
};

#endif
