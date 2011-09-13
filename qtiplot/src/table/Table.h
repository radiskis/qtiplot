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

#include <q3table.h>
#include <q3header.h>
#include <QVarLengthArray>
#include <QLocale>

#include <MdiSubWindow.h>
#include <ScriptingEnv.h>
#include <Script.h>

class MyTable : public Q3Table
{
public:
    MyTable(QWidget * parent = 0, const char * name = 0);
    MyTable(int numRows, int numCols, QWidget * parent = 0, const char * name = 0);

private:
    void activateNextCell();
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

	Table(ScriptingEnv *env, int r,int c, const QString &label, ApplicationWindow* parent, const QString& name = QString(), Qt::WFlags f=0);

	Q3TableSelection getSelection();

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
	void copy(Table *m, bool values = true);
	int numRows();
	int numCols();
	void setNumRows(int rows);
	void setNumCols(int cols);
	void resizeRows(int);
	void resizeCols(int);

	//! Return the value of the cell as a double
	double cell(int row, int col);
	void setCell(int row, int col, double val);

	QString text(int row, int col);
	QStringList columnsList();
	QStringList colNames(){return col_label;}
	QString colName(int col);
	void setColName(int col, const QString& text, bool enumerateRight = false, bool warn = true);
	QString colLabel(int col);
	int colIndex(const QString& name);

	int colPlotDesignation(int col){return col_plot_type[col];};
	void setColPlotDesignation(int col, PlotDesignation pd);
	void setPlotDesignation(PlotDesignation pd, bool rightColumns = false);
	QList<int> plotDesignations(){return col_plot_type;};

	void setHeader(QStringList header);
	void loadHeader(QStringList header);
	void setHeaderColType();
	void setText(int row,int col,const QString & text);
	void setRandomValues();
	void setRandomValues(int col, int startRow = 0, int endRow = -1);
	void setNormalRandomValues();
	void setNormalRandomValues(int col, int startRow = 0, int endRow = -1, double sigma = 1.0);
	void setAscValues();

	void cellEdited(int,int col);
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
	//@}v

	//! \name Column Operations
	//@{
	void removeCol();
	void removeCol(const QStringList& list);
	void insertCol();
	virtual void insertCols(int start, int count);
	virtual void addCol(PlotDesignation pd = Y);
	void addColumns(int c);
	virtual void moveColumn(int, int, int);
	void swapColumns(int, int);
	void moveColumnBy(int cols);
	void hideSelectedColumns();
	void showAllColumns();
	void hideColumn(int col, bool = true);
	bool isColumnHidden(int col){return d_table->isColumnHidden(col);};
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
	void deleteRows(int startRow, int endRow);
	void insertRow();
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

	void setColumnWidth(int width, bool allCols);
	void setColumnWidth(int col, int width);
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
	//! Scroll to row (row starts with 1)
	void goToRow(int row);
	//! Scroll to column (column starts with 1)
	void goToColumn(int col);

	void columnNumericFormat(int col, char *f, int *precision);
	void columnNumericFormat(int col, int *f, int *precision);
	int columnType(int col){return colTypes[col];};

	QList<int> columnTypes(){return colTypes;};
	void setColumnTypes(QList<int> ctl){colTypes = ctl;};
	void setColumnTypes(const QStringList& ctl);
	void setColumnType(int col, ColType val) { colTypes[col] = val; }

    void saveToMemory(double **cells){d_saved_cells = cells;};
	void saveToMemory();
	void freeMemory();

    bool isReadOnlyColumn(int col);
    void setReadOnlyColumn(int col, bool on = true);

	QString columnFormat(int col){return col_format[col];};
	QStringList getColumnsFormat(){return col_format;};
	void setColumnsFormat(const QStringList& lst);

	void setTextFormat(int col);
	void setColNumericFormat(int col);
	void setColNumericFormat(int f, int prec, int col, bool updateCells = true);
	bool setDateFormat(const QString& format, int col, bool updateCells = true);
	bool setTimeFormat(const QString& format, int col, bool updateCells = true);
	void setMonthFormat(const QString& format, int col, bool updateCells = true);
	void setDayFormat(const QString& format, int col, bool updateCells = true);

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
	void restore(const QStringList& lst);

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
	void setColComment(int col, const QString& s);
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

	//! Internal function to change the column header
	void setColumnHeader(int index, const QString& label);
};

#endif
