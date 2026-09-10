/***************************************************************************
    File                 : Table.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Table worksheet class

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
#ifndef TABLE_H
#define TABLE_H

#include <algorithm>
#include <functional>

#include <QTableWidget>
#include <QHeaderView>
#include <QItemSelection>
#include <QVarLengthArray>
#include <QLocale>

#include <MdiSubWindow.h>
#include <ScriptingEnv.h>
#include <Script.h>
#include <QUndoStack>
#include "TableModel.h"
#include <QTableView>
#include <QTableWidgetSelectionRange>
#include <QItemSelectionModel>
#include <QHeaderView>

class Table;

class MyTable : public QTableView
{
    Q_OBJECT

public:
    MyTable(QWidget * parent = nullptr, const char * name = nullptr);
    MyTable(int numRows, int numCols, QWidget * parent = nullptr, const char * name = nullptr);

    TableModel* tableModel() const { return d_model; }

    int numRows() const { return d_model ? d_model->rowCount() : 0; }
    int numCols() const { return d_model ? d_model->columnCount() : 0; }
    int rowCount() const { return numRows(); }
    int columnCount() const { return numCols(); }
    void setNumRows(int r) { if (d_model) d_model->setRowCount(r); }
    void setNumCols(int c) { if (d_model) d_model->setColumnCount(c); }
    void setRowCount(int r) { setNumRows(r); }
    void setColumnCount(int c) { setNumCols(c); }

    int currentRow() const { return currentIndex().isValid() ? currentIndex().row() : -1; }
    int currentColumn() const { return currentIndex().isValid() ? currentIndex().column() : -1; }

    QString text(int r, int c) const { return d_model ? d_model->text(r, c) : QString(); }
    void setText(int r, int c, const QString &t) { if (d_model) d_model->setText(r, c, t); }
    double rawValue(int r, int c) const { return d_model ? d_model->rawValue(r, c) : 0.0; }
    void setRawValue(int r, int c, double val) { if (d_model) d_model->setRawValue(r, c, val); }
    bool hasRawValue(int r, int c) const { return d_model ? d_model->hasRawValue(r, c) : false; }
    void clearRawValue(int r, int c) { if (d_model) d_model->clearRawValue(r, c); }

    bool isColumnReadOnly(int col) const { return d_model ? d_model->isColumnReadOnly(col) : false; }
    void setColumnReadOnly(int col, bool ro) { if (d_model) d_model->setColumnReadOnly(col, ro); }

    void setHeaderText(int col, const QString &text) { if (d_model) d_model->setHeaderText(col, text); }
    CellState previousCellState(int r, int c) const { return d_model ? d_model->previousCellState(r, c) : CellState{}; }
    void setPreviousCellState(int r, int c, const QString &text, double val, bool hasVal) {
        if (d_model) d_model->setPreviousCellState(r, c, text, val, hasVal);
    }

    void setPaletteBackgroundColor(const QColor &c) { QPalette p = palette(); p.setColor(QPalette::Base, c); setPalette(p); }
    void setPaletteForegroundColor(const QColor &c) { QPalette p = palette(); p.setColor(QPalette::Text, c); setPalette(p); }
    void setLeftMargin(int m) { verticalHeader()->setFixedWidth(m); }
    void adjustColumn(int col) { resizeColumnToContents(col); }

    QList<QTableWidgetSelectionRange> selectedRanges() const {
        QList<QTableWidgetSelectionRange> ranges;
        if (selectionModel()) {
            for (const QItemSelectionRange &r : selectionModel()->selection())
                ranges.append(QTableWidgetSelectionRange(r.top(), r.left(), r.bottom(), r.right()));
        }
        return ranges;
    }

    void setRangeSelected(const QTableWidgetSelectionRange &range, bool select) {
        if (!d_model || !selectionModel()) return;
        int top = qMax(0, range.topRow()), bottom = qMin(d_model->rowCount() - 1, range.bottomRow());
        int left = qMax(0, range.leftColumn()), right = qMin(d_model->columnCount() - 1, range.rightColumn());
        if (top <= bottom && left <= right) {
            selectionModel()->select(QItemSelection(d_model->index(top, left), d_model->index(bottom, right)),
                                     select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
        }
    }

    bool isRowSelected(int row, bool full = false) {
        for (const auto &r : selectedRanges()) {
            if (r.topRow() <= row && r.bottomRow() >= row)
                return !full || (r.leftColumn() == 0 && r.rightColumn() == numCols() - 1);
        }
        return false;
    }
    
    bool isColumnSelected(int col, bool full = false) {
        for (const auto &r : selectedRanges()) {
            if (r.leftColumn() <= col && r.rightColumn() >= col)
                return !full || (r.topRow() == 0 && r.bottomRow() == numRows() - 1);
        }
        return false;
    }

    void ensureCellVisible(int row, int col) { if (d_model) scrollTo(d_model->index(row, col)); }
    void setCurrentCell(int r, int c) { if (d_model) setCurrentIndex(d_model->index(r, c)); }
    int currentSelection() { return selectedRanges().isEmpty() ? -1 : 0; }

    void insertColumn(int col) { if (d_model) d_model->insertColumn(col); }
    void removeColumn(int col) { if (d_model) d_model->removeColumn(col); }
    void insertColumns(int col, int count = 1) { if (d_model) d_model->insertColumns(col, count); }
    void insertRows(int row, int count = 1) { if (d_model) d_model->insertRows(row, count); }
    void insertRow(int row) { insertRows(row, 1); }
    void removeRow(int row) { if (d_model) d_model->removeRows(row, 1); }
    void swapColumns(int col1, int col2) { if (d_model) d_model->swapColumns(col1, col2); }
    void swapRows(int row1, int row2) { if (d_model) d_model->swapRows(row1, row2); }
    void updateContents() { viewport()->update(); }
    void activateNextCell();

    QTableWidgetSelectionRange selection(int index) {
        auto ranges = selectedRanges();
        return (index >= 0 && index < ranges.count()) ? ranges[index] : QTableWidgetSelectionRange();
    }
    void addSelection(const QTableWidgetSelectionRange &sel) { setRangeSelected(sel, true); }
    void removeRows(const QVector<int> &rows) {
        QList<int> sorted = rows.toList();
        std::sort(sorted.begin(), sorted.end(), std::greater<int>());
        for (int r : sorted) removeRow(r);
    }
    bool isSelected(int r, int c) { return d_model && selectionModel() && selectionModel()->isSelected(d_model->index(r, c)); }
    void setReadOnly(bool ro) {
        setEditTriggers(ro ? QAbstractItemView::NoEditTriggers
                           : (QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed));
    }

signals:
    void itemSelectionChanged();
    void cellChanged(int row, int col);
    void cellDoubleClicked(int row, int col);
    void cellPressed(int row, int col);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

private:
    void setupConnections();
    TableModel *d_model;
};



/*!\brief MDI window providing a spreadsheet table with column logic.
 *
 * \section future Future Plans
 * Port to the Model/View approach used in Qt4 and get rid of the Qt3Support dependancy.
 * [ assigned to thzs ]
 */
class Table: public MdiSubWindow, public scripted
{
    Q_OBJECT

public:
	enum PlotDesignation{All = -1, None = 0, X = 1, Y = 2, Z = 3, xErr = 4, yErr = 5, Label = 6};
	enum ColType{Numeric = 0, Text = 1, Date = 2, Time = 3, Month = 4, Day = 5};
	enum NumericFormat{Default = 0, Decimal = 1, Scientific = 2};
	enum ImportMode {
		NewColumns, //!< add file as new columns to this table
		NewRows, //!< add file as new rows to this table
		Overwrite //!< replace content of table with the imported file
	};

	Table(ScriptingEnv *env, int r,int c, const QString &label, ApplicationWindow* parent, const QString& name = QString(), Qt::WindowFlags f= {});
	~Table();

	QTableWidgetSelectionRange getSelection();

	//! Sets the number of significant digits
	void setNumericPrecision(int prec);
	//! Updates the decimal separators when importing ASCII files on user request
	void updateDecimalSeparators(const QLocale& oldSeparators);
	void setAutoUpdateValues(bool on = true);
	virtual QString sizeToString();

	double avg(int col, int startRow = 0, int endRow = -1);
	double sum(int col, int startRow = 0, int endRow = -1);
	double minColumnValue(int col, int startRow = 0, int endRow = -1);
	double maxColumnValue(int col, int startRow = 0, int endRow = -1);
	Table* extractData(const QString& name, const QString& condition, int startRow = 0, int endRow = -1);
	static QDateTime dateTime(double val);
	static double fromDateTime(const QDateTime& dt);
	static double fromTime(const QTime& t);

public slots:
	MyTable* table(){return d_table;};
	QUndoStack *undoStack() const override {return d_undo_stack;};
	void pushUndoCommand(QUndoCommand *cmd);
	size_t undoMemoryUsage() const;
	void copy(Table *m, bool values = true);
	int numRows();
	int numCols();
	void setNumRows(int rows);
	void setNumCols(int cols);
	void resizeRows(int);
	void resizeCols(int);

	//! Return the value of the cell as a double
	double cell(int row, int col);
	void setCell(int row, int col, double val, bool pushUndo = true);

	QString text(int row, int col);
	QStringList columnsList();
	QStringList colNames(){return col_label;}
	QString colName(int col);
	void setColName(int col, const QString& text, bool enumerateRight = false, bool warn = true);
	void setColNames(int startCol, const QStringList& names);
	QString colLabel(int col);
	int colIndex(const QString& name);

	int colPlotDesignation(int col){return col_plot_type[col];};
	void setColPlotDesignation(int col, PlotDesignation pd, bool pushUndo = true);
	void setPlotDesignation(PlotDesignation pd, bool rightColumns = false);
	QList<int> plotDesignations(){return col_plot_type;};

	void setHeader(QStringList header);
	void loadHeader(QStringList header);
	void setHeaderColType();
	void setText(int row,int col,const QString & text, bool pushUndo = true, const double *exact = nullptr);
	void setRandomValues();
	void setRandomValues(int col, int startRow = 0, int endRow = -1);
	void setNormalRandomValues();
	void setNormalRandomValues(int col, int startRow = 0, int endRow = -1, double sigma = 1.0);
	void setAscValues();

	void cellEdited(int,int col);
	void cellDoubleClicked(int, int);
	void moveCurrentCell();
	void clearCell(int row, int col);
	bool isEmptyRow(int row);
	bool isEmptyColumn(int col);
	int nonEmptyRows();

	void print();
	void print(QPrinter *);
	void print(const QString& fileName);
	void exportPDF(const QString& fileName);

	//! \name Event Handlers
	//@{
	bool eventFilter(QObject *object, QEvent *e);
	void customEvent( QEvent* e);
	//@}

	//! \name Column Operations
	//@{
	void removeCol();
	void deleteColumns(const QStringList& list, bool pushUndo = true);
	void insertCol();
	virtual void insertColumn(int col, bool pushUndo = true);
	virtual void insertCols(int start, int count, bool pushUndo = true);
	virtual void addCol(PlotDesignation pd = Y);
	void addColumns(int c);
	virtual void moveColumn(int, int, int);
	void swapColumns(int col1, int col2, bool pushUndo = true);
	void moveColumnBy(int cols, bool pushUndo = true);
	void hideSelectedColumns();
	void showAllColumns();
	void hideColumn(int col, bool = true);
	bool isColumnHidden(int col){return d_table->isColumnHidden(col);};
    // QTableWidget doesn't have currentColumn(), it has currentColumn() (same name)
    // Q3Table::currentColumn() -> QTableWidget::currentColumn().
	//@}

	//! \name Sorting
	//@{
	/*!\brief Sort the current column in ascending order.
	 * \sa sortColDesc(), sortColumn(), Q3Table::currentColumn()
	 */
	void sortColAsc();
	/*!\brief Sort the current column in descending order.
	 * \sa sortColAsc(), sortColumn(), Q3Table::currentColumn()
	 */
	void sortColDesc();
	/*!\brief Sort the specified column.
	 * \param col the column to be sorted
	 * \param order 0 means ascending, anything else means descending
	 */
	void sortColumn(int col = -1, int order = 0);
	/*!\brief Display a dialog with some options for sorting all columns.
	 *
	 * The sorting itself is done using sort(int,int,const QString&).
	 */
	void sortTableDialog();
	//! Sort all columns as in sortColumns(const QStringList&,int,int,const QString&).
	void sort(int type = 0, int order  = 0, const QString& leadCol = QString());
	//! Sort selected columns as in sortColumns(const QStringList&,int,int,const QString&).
	void sortColumns(int type = 0, int order = 0, const QString& leadCol = QString());
	/*!\brief Sort the specified columns.
	 * \param cols the columns to be sorted
	 * \param type 0 means sort individually (as in sortColumn()), anything else means together
	 * \param order 0 means ascending, anything else means descending
	 * \param leadCol for sorting together, the column which determines the permutation
	 */
	void sortColumns(const QStringList& cols, int type = 0, int order = 0, const QString& leadCol = QString());
	/*!\brief Display a dialog with some options for sorting the selected columns.
	 *
	 * The sorting itself is done using sortColumns(int,int,const QString&).
	 */
	void sortColumnsDialog();
	//@}

	//! \name Normalization
	//@{
	void normalizeCol(int col=-1);
	void normalizeSelection();
	void normalize();
	//@}

	QVarLengthArray<double> col(int ycol);
	void columnRange(int c, double *min, double *max);

	int firstXCol();
	bool noXColumn();
	bool noYColumn();
	int colX(int col);
	int colY(int col, int xCol = -1, const QStringList& lst = QStringList());

	QStringList getCommands(){return commands;};
	//! Clear all column formulae.
	void clearCommands();
	//! Set all column formulae.
	void setCommands(const QStringList& com);
	//! Set all column formulae.
	void setCommands(const QString& com);
	//! Set formula for column col.
	void setCommand(int col, const QString& com);
	//! Compute specified cells from column formula.
	bool calculate(int col, int startRow, int endRow, bool forceMuParser = false, bool notifyChanges = true);
	//! Compute specified cells from column formula (optimized for muParser).
	bool muParserCalculate(int col, int startRow, int endRow, bool notifyChanges = true);
	//! Compute selected cells from column formulae; use current cell if there's no selection.
	bool calculate();
	//! Recalculates values in all columns with formulas containing \param columnName
	void updateValues(Table*, const QString& columnName);

	//! \name Row Operations
	//@{
	void deleteSelectedRows();
	void deleteRows(int startRow, int endRow, bool pushUndo = true);
	void insertRow();
	void insertRow(int row, bool pushUndo = true);
	void insertRows(int row, int count, bool pushUndo = true);
	void moveRow(bool up = true);
	//@}

	//! Selection Operations
	//@{
	void cutSelection();
	void copySelection();
	void clearSelection();
	void pasteSelection();
	void selectAllTable();
	void deselect();
	void clear();
	//@}

	void init(int rows, int cols);
	QStringList writableSelectedColumns();
	QStringList selectedColumns();
	QStringList selectedYColumns();
	QStringList selectedErrColumns();
	QStringList selectedYLabels();
	QStringList drawableColumnSelection();
	QStringList YColumns();
	int selectedColsNumber();

	void setColumnWidth(int width, bool allCols, bool pushUndo);
	void setColumnWidth(int width, bool allCols = false) { setColumnWidth(width, allCols, true); }
	void setColumnWidth(int col, int width, bool pushUndo = true);
	int columnWidth(int col);
	QStringList columnWidths();
	void setColWidths(const QStringList& widths);
	void adjustColumnsWidth(bool selection = true);

	void setSelectedCol(int col){selectedCol = col;};
	int selectedColumn(){return selectedCol;};
	int firstSelectedColumn();
	int numSelectedRows();
	bool isRowSelected(int row, bool full=false) { return d_table->isRowSelected(row, full); }
	bool isColumnSelected(int col, bool full=false) { return d_table->isColumnSelected(col, full); }
	void setSelectedRange(int topRow, int leftCol, int bottomRow, int rightCol, bool sel = true) {
		if (d_table) d_table->setRangeSelected(QTableWidgetSelectionRange(topRow, leftCol, bottomRow, rightCol), sel);
	}
	//! Scroll to row (row starts with 1)
	void goToRow(int row);
	//! Scroll to column (column starts with 1)
	void goToColumn(int col);

	void columnNumericFormat(int col, char *f, int *precision);
	void columnNumericFormat(int col, int *f, int *precision);
	int columnType(int col){return colTypes[col];};

	QList<int> columnTypes(){return colTypes;};
	void setColumnTypes(const QStringList& ctl);
	void setColumnTypes(const QList<int>& ctl);
	void setColumnType(int col, ColType val, bool pushUndo = true);

    void saveToMemory(double **cells){d_saved_cells = cells;};
	void saveToMemory();
	void freeMemory();

    bool isReadOnlyColumn(int col);
    void setReadOnlyColumn(int col, bool on = true, bool pushUndo = true);
	void setColFormat(int col, ColType type, const QString &format);

	QString columnFormat(int col){return col_format[col];};
	QStringList getColumnsFormat(){return col_format;};
	void setColumnsFormat(const QStringList& lst);
	void setColumnFormat(int col, const QString &f){if (col >= 0 && col < col_format.size()) col_format[col] = f;};

	void setTextFormat(int col, bool pushUndo = true);
	void setColNumericFormat(int col, bool pushUndo = true);
	void setColNumericFormat(int f, int prec, int col, bool updateCells = true, bool pushUndo = true);
	bool setDateFormat(const QString& format, int col, bool updateCells = true, bool pushUndo = true);
	bool setTimeFormat(const QString& format, int col, bool updateCells = true, bool pushUndo = true);
	void setMonthFormat(const QString& format, int col, bool updateCells = true, bool pushUndo = true);
	void setDayFormat(const QString& format, int col, bool updateCells = true, bool pushUndo = true);

	bool exportExcel(const QString& fname, bool withLabels, bool exportComments, bool exportSelection);
	bool exportOdsSpreadsheet(const QString& fname, bool withLabels, bool exportComments, bool exportSelection);
	bool exportODF(const QString& fname, bool withLabels, bool exportComments, bool exportSelection);
	bool exportASCII(const QString& fname, const QString& separator, bool withLabels = false,
                     bool exportComments = false, bool exportSelection = false);
	void importASCII(const QString &fname, const QString &sep = "\t", int ignoredLines = 0, bool renameCols = false,
					bool stripSpaces = false, bool simplifySpaces = false, bool importComments = false,
					const QString& commentString = "", bool readOnly = false,
					ImportMode importAs = Overwrite, const QLocale& importLocale = QLocale(), int endLine = 0, int maxRows = -1,
					const QList<int>& newColTypes = QList<int>(), const QStringList& colFormats = QStringList());

	//! \name Saving and Restoring
	//@{
	virtual void save(const QString &fn, const QString& geometry, bool = false);
	void restore(const QStringList& lst, int fileVersion, bool fromTemplate = false);

	QString saveHeader();
	QString saveComments();
	QString saveCommands();
	QString saveColumnWidths();
	QString saveColumnTypes();
	QString saveReadOnlyInfo();
	QString saveHiddenColumnsInfo();

	void setBackgroundColor(const QColor& col);
	void setTextColor(const QColor& col);
	void setHeaderColor(const QColor& col);
	void setTextFont(const QFont& fnt);
	void setHeaderFont(const QFont& fnt);

	int verticalHeaderWidth(){return d_table->verticalHeader()->width();};

	QString comment(int col);
	void setColComment(int col, const QString& s, bool pushUndo = true);
	QStringList colComments(){return comments;};
	void setColComments(const QStringList& lst){comments = lst;};
	void showComments(bool on = true);
	bool commentsEnabled(){return d_show_comments;}

	//! This slot notifies the main application that the table has been modified. Triggers the update of 2D plots.
	void notifyChanges();
	void notifyChanges(const QString& colName);

	//! Notifies the main application that the width of a table column has been modified by the user.
	void colWidthModified(int, int, int);

signals:
	void changedColHeader(const QString&, const QString&);
	void removedCol(const QString&);
	void addedCol(const QString&);
	void removedCol(int);
	void colIndexChanged(int, int);
	void modifiedData(Table *, const QString&);
	void optionsDialog();
	void colValuesDialog();
	void resizedTable(QWidget*);
	void showContextMenu(bool selection);

protected:
	MyTable *d_table;

private:
	void clearCol();

	bool d_show_comments;
	QStringList commands, col_format, comments, col_label;
	QList<int> colTypes, col_plot_type;
	int selectedCol;
	int d_numeric_precision;
	double **d_saved_cells;

	QUndoStack *d_undo_stack;

	//! Internal function to change the column header
	void setColumnHeader(int index, const QString& label);
};

#endif
