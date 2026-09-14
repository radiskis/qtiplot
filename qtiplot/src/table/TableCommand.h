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

class TableCommand : public QUndoCommand
{
public:
	TableCommand(const QString &text = QString(), QUndoCommand *parent = nullptr)
		: QUndoCommand(text, parent) {}
	virtual ~TableCommand() = default;

	virtual size_t byteSize() const {
		size_t sz = sizeof(*this) + text().length() * sizeof(QChar);
		for (int i = 0; i < childCount(); ++i) {
			if (auto *tc = dynamic_cast<const TableCommand*>(child(i)))
				sz += tc->byteSize();
			else
				sz += 64;
		}
		return sz;
	}
};

class TableSetColNamesCommand: public TableCommand
{
public:
	TableSetColNamesCommand(Table *t, int startCol, const QStringList& oldNames, const QStringList& newNames, const QString & text);
	virtual void redo();
	virtual void undo();
	size_t byteSize() const override {
		size_t sz = sizeof(*this);
		for (const QString &s : d_old_names) sz += s.length() * sizeof(QChar);
		for (const QString &s : d_new_names) sz += s.length() * sizeof(QChar);
		return sz;
	}

private:
	QPointer<Table> d_table;
	int d_start_col = 0;
	QStringList d_old_names, d_new_names;
};

class TableEditCellCommand: public TableCommand
{
public:
	TableEditCellCommand(Table *t, int row, int col, const QString& oldText, const QString& newText, const QString & text,
	                     bool hasOldVal = false, double oldVal = 0.0, bool hasNewVal = false, double newVal = 0.0);
	virtual void redo();
	virtual void undo();
	size_t byteSize() const override {
		return sizeof(*this) + (d_old_text.length() + d_new_text.length()) * sizeof(QChar);
	}

private:
	QPointer<Table> d_table;
	int d_row = 0, d_col = 0;
	QString d_old_text, d_new_text;
	bool d_has_old_val = false, d_has_new_val = false;
	double d_old_val = 0.0, d_new_val = 0.0;
};

class TableSetColNameCommand: public TableCommand
{
public:
	TableSetColNameCommand(Table *t, int col, const QString& oldName, const QString& newName, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	QString d_old_name, d_new_name;
};

class TableSetColTypeCommand: public TableCommand
{
public:
	TableSetColTypeCommand(Table *t, int col, Table::ColType oldType, Table::ColType newType, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	Table::ColType d_old_type = Table::Numeric, d_new_type = Table::Numeric;
};

class TableSetColFormatCommand: public TableCommand
{
public:
	TableSetColFormatCommand(Table *t, int col, Table::ColType oldType, Table::ColType newType,
							 const QString& oldFormat, const QString& newFormat, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	Table::ColType d_old_type = Table::Numeric, d_new_type = Table::Numeric;
	QString d_old_format, d_new_format;
};

class TableSetColCommentCommand: public TableCommand
{
public:
	TableSetColCommentCommand(Table *t, int col, const QString& oldComment, const QString& newComment, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	QString d_old_comment, d_new_comment;
};

class TableSetPlotDesignationCommand: public TableCommand
{
public:
	TableSetPlotDesignationCommand(Table *t, int col, Table::PlotDesignation oldPD, Table::PlotDesignation newPD, const QString & text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	Table::PlotDesignation d_old_pd = Table::None, d_new_pd = Table::None;
};



class TableDeleteRowsCommand: public TableCommand
{
public:
	TableDeleteRowsCommand(Table *t, int startRow, int endRow, const QList<QStringList>& data, const QString& text);
	virtual void redo();
	virtual void undo();
	size_t byteSize() const override {
		size_t sz = sizeof(*this);
		for (const QStringList &lst : d_data)
			for (const QString &s : lst) sz += s.length() * sizeof(QChar) + sizeof(QString);
		return sz;
	}

private:
	QPointer<Table> d_table;
	int d_start_row = 0, d_end_row = 0;
	QList<QStringList> d_data;
};

class TableInsertRowCommand: public TableCommand
{
public:
	TableInsertRowCommand(Table *t, int row, int count = 1, const QString& text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_row = 0;
	int d_count = 1;
};

class TableAddColsCommand: public TableCommand
{
public:
	TableAddColsCommand(Table *t, int startCol, int count, const QStringList& names, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_start_col = 0, d_count = 0;
	QStringList d_names;
};

class TableDeleteColsCommand: public TableCommand
{
public:
	TableDeleteColsCommand(Table *t, int startCol, int endCol, const QList<QStringList>& cellData,
							const QStringList& names, const QStringList& comments,
							const QStringList& formats, const QList<int>& types,
							const QList<int>& plotTypes, const QStringList& widths,
							const QStringList& commands, const QString& text);
	virtual void redo();
	virtual void undo();
	size_t byteSize() const override {
		size_t sz = sizeof(*this);
		for (const QStringList &lst : d_cell_data)
			for (const QString &s : lst) sz += s.length() * sizeof(QChar) + sizeof(QString);
		return sz;
	}

private:
	QPointer<Table> d_table;
	int d_start_col = 0, d_end_col = 0;
	QList<QStringList> d_cell_data;
	QStringList d_names, d_comments, d_formats, d_widths, d_commands;
	QList<int> d_types, d_plot_types;
};

class TableInsertColCommand: public TableCommand
{
public:
	TableInsertColCommand(Table *t, int col, const QString& text);
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
};

class TableSetValuesCommand: public TableCommand
{
public:
	TableSetValuesCommand(Table *t, int startRow, int endRow, const QList<int>& cols,
						const QList<QStringList>& oldData, const QList<QStringList>& newData, const QString & text);
	virtual void redo();
	virtual void undo();
	size_t byteSize() const override {
		size_t sz = sizeof(*this);
		for (const QStringList &lst : d_old_data)
			for (const QString &s : lst) sz += s.length() * sizeof(QChar) + sizeof(QString);
		for (const QStringList &lst : d_new_data)
			for (const QString &s : lst) sz += s.length() * sizeof(QChar) + sizeof(QString);
		return sz;
	}

private:
	QPointer<Table> d_table;
	int d_start_row = 0, d_end_row = 0;
	QList<int> d_cols;
	QList<QStringList> d_old_data, d_new_data;
};

class TableSwapColumnsCommand: public TableCommand
{
public:
	TableSwapColumnsCommand(Table *t, int col1, int col2, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col1 = 0, d_col2 = 0;
};

class TableMoveColumnCommand: public TableCommand
{
public:
	TableMoveColumnCommand(Table *t, int from, int to, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_from = 0, d_to = 0;
};

class TableSetColumnWidthCommand: public TableCommand
{
public:
	TableSetColumnWidthCommand(Table *t, int col, int oldWidth, int newWidth, bool allCols = false,
							   const QList<int>& oldWidths = QList<int>(), const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	int d_old_width = 0, d_new_width = 0;
	bool d_all_cols = false;
	QList<int> d_old_widths;
};

class TableSetReadOnlyCommand: public TableCommand
{
public:
	TableSetReadOnlyCommand(Table *t, int col, bool oldState, bool newState, const QString &text = QString());
	virtual void redo();
	virtual void undo();

private:
	QPointer<Table> d_table;
	int d_col = 0;
	bool d_old_state = false, d_new_state = false;
};

#endif
