/***************************************************************************
    File                 : Matrix.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Hoener zu Siederdissen
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Matrix worksheet class

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
#ifndef MATRIX_H
#define MATRIX_H

#include <QHeaderView>
#include <QTableView>
#include <QPrinter>
#include "MyWidget.h"
#include "ScriptingEnv.h"
#include "Script.h"
#include <qwt_double_rect.h>
#include <qwt_color_map.h>

#include <math.h>

// (maximum) initial matrix size
#define _Matrix_initial_rows_ 10
#define _Matrix_initial_columns_ 3

class MatrixModel;
class QLabel;
class QStackedWidget;
class QShortcut;

//! Matrix worksheet class
class Matrix: public MyWidget, public scripted
{
    Q_OBJECT

public:

	/*!
	 * \brief Constructor
	 *
	 * \param env scripting interpreter
	 * \param r initial number of rows
	 * \param c initial number of columns
	 * \param label window label
	 * \param parent parent object
	 * \param name
	 * \param name window name
	 * \param f window flags
	 */
	Matrix(ScriptingEnv *env, int r, int c, const QString& label, QWidget* parent=0, const QString& name = QString(), Qt::WFlags f=0);
    Matrix(ScriptingEnv *env, const QImage& image, const QString& label, QWidget* parent=0, const QString& name = QString(), Qt::WFlags f=0);

    enum HeaderViewType{ColumnRow, XY};
	enum ViewType{TableView, ImageView};
	enum ColorMapType{GrayScale, Rainbow, Custom};

	void setViewType(ViewType);
	ViewType viewType(){return d_view_type;};

    HeaderViewType headerViewType(){return d_header_view_type;};
    void setHeaderViewType(HeaderViewType type);

	QImage image();
	void setImage(const QImage& image);
	void importImage(const QString& fn);
	void exportRasterImage(const QString& fileName, int quality = 100);
	void exportSVG(const QString& fileName);
	void exportToFile(const QString& fileName);
	void exportVector(const QString& fileName, int res = 0, bool color = true, bool keepAspect = true, QPrinter::PageSize pageSize = QPrinter::Custom);

	MatrixModel * matrixModel(){return d_matrix_model;};
	QItemSelectionModel * selectionModel(){return d_table_view->selectionModel();};

	//! Return the number of rows
	int numRows();
	void setNumRows(int rows);

	//! Return the number of columns
	int numCols();
	void setNumCols(int cols);

	//event handlers
	/*!
	 * \brief Event filter
	 *
	 * Currently only reacts to events of the
	 * title bar.
	 */
	bool eventFilter(QObject *object, QEvent *e);
	//! Context menu event handler
	void contextMenuEvent(QContextMenuEvent *e);
	//! Custom event handler
	/**
	 * Currently handles SCRIPTING_CHANGE_EVENT only.
	 */
	void customEvent(QEvent *e);

	void resetView();
	void moveCell(const QModelIndex& index);

	void flipVertically();
	void flipHorizontally();
	void rotate90(bool clockwise = true);

#ifdef QTIPLOT_PRO
    void fft(bool inverse = false);
#endif

    ColorMapType colorMapType(){return d_color_map_type;};
	void setColorMapType(ColorMapType mapType);

	QwtLinearColorMap colorMap(){return d_color_map;};
	void setColorMap(const QwtLinearColorMap& map);
	//! Used when restoring from project files
	void setColorMap(const QStringList& lst);

	void setGrayScale();
	void setRainbowColorMap();

public slots:
	void exportPDF(const QString& fileName);
	//! Print the Matrix
	void print();
	//! Print the Matrix to fileName
	void print(const QString& fileName);

	//! Return the width of all columns
	int columnsWidth(){return d_column_width;};
	//! Set the width of all columns
	void setColumnsWidth(int width);

	//! Set the Matrix size
	void setDimensions(int rows, int cols);
	//! Transpose the matrix
	void transpose();
	//! Invert the matrix
	void invert();
	//! Calculate the determinant of the matrix
	double determinant();

	//! Calculate matrix values using the #formula_str
	bool calculate(int startRow = 0, int endRow = -1, int startCol = 0, int endCol = -1);

	//! Return the content of the cell as a string
	QString text(int row, int col);
	//! Set the content of the cell as a string
	void setText(int row, int col, const QString & new_text );
	//! Return the value of the cell as a double
	double cell(int row, int col);
	//! Set the value of the cell
	void setCell(int row, int col, double value );

	/*!
	 * \brief Return the text format code ('e', 'f', ...)
	 *
	 * \sa setNumerFormat(), setTextFormat()
	 */
	QChar textFormat(){return txt_format;};
	/*!
	 * \brief Return the number precision digits
	 *
	 * See arguments of setNumericFormat().
	 * \sa setNumericFormat(), setTextFormat()
	 */
	int precision(){return num_precision;};
	/*!
	 * \brief Set the number of significant digits
	 *
	 * \sa precision(), setNumericFormat(), setTextFormat()
	 */
	void setNumericPrecision(int prec){num_precision = prec;};

	/*!
	 * \brief Set the number format for the cells
	 *
	 * This method should only be called before any user
	 * interaction was done. Use setTextFormat() if you
	 * want to change it from a dialog.
	 * \sa setTextFormat()
	 */
	void setTextFormat(const QChar &format, int precision);
	void setNumericFormat(const QChar & f, int prec);

	//! Return the matrix formula
	QString formula(){return formula_str;};
	//! Set the matrix forumla
	void setFormula(const QString &s){formula_str = s;};

	//! Load the matrix from a string list (i.e. lines from a project file)
	void restore(const QStringList &l);
	//! Format the matrix format in a string to save it in a template file
	QString saveAsTemplate(const QString &info);

	//! Return a string to save the matrix in a project file (\<matrix\> section)
	QString saveToString(const QString &info, bool saveAsTemplate = false);

	// selection operations
	//! Standard cut operation
	void cutSelection();
	//! Standard copy operation
	void copySelection();
	//! Clear cells
	void clearSelection();
	//! Standard paste operation
	void pasteSelection();

	//! Insert a row before the current cell
	void insertRow();
	//! Delete the selected rows
	void deleteSelectedRows();
	//! Return the number of selected rows
	int numSelectedRows();

	//! Insert a column before the current cell
	void insertColumn();
	//! Delte the selected columns
	void deleteSelectedColumns();
	//! Return the number of selected columns
	int numSelectedColumns();

	//! Returns the X value corresponding to column 1
	double xStart(){return x_start;};
	//! Returns the X value corresponding to the last column
	double xEnd(){return x_end;};
	//! Returns the Y value corresponding to row 1
	double yStart(){return y_start;};
	//! Returns the Y value corresponding to the last row
	double yEnd(){return y_end;};

	//! Returns the step of the X axis
	double dx(){return fabs(x_end - x_start)/(double)(numCols() - 1);};
	//! Returns the step of the Y axis
	double dy(){return fabs(y_end - y_start)/(double)(numRows() - 1);};

	//! Returns the bounding rect of the matrix coordinates
  	QwtDoubleRect boundingRect();
	//! Set the X and Y coordinate intervals
	void setCoordinates(double xs, double xe, double ys, double ye);

	 //! Min and max values of the matrix.
  	void range(double *min, double *max);

	//! Scroll to row (row starts with 1)
	void goToRow(int row);

	//! Allocate memory for a matrix buffer
	static double** allocateMatrixData(int rows, int columns);
	//! Free memory used for a matrix buffer
	static void freeMatrixData(double **data, int rows);

	int verticalHeaderWidth(){return d_table_view->verticalHeader()->width();}

    void copy(Matrix *m);

signals:
	//! Show the context menu
	void showContextMenu();

private:
	//! Initialize the matrix
	void initTable(int rows, int cols);
	void initImage(const QImage& image);
	void initImageView();
	void initTableView();
    void initGlobals();

    QStackedWidget *d_stack;
    MatrixModel *d_matrix_model;
	//! Pointer to the table view
    QTableView *d_table_view;
	//! Used to display the image view
	QLabel *imageLabel;
	//! Last formula used to calculate cell values
	QString formula_str;
	//! Format code for displaying numbers
	QChar txt_format;
	//! Number of significant digits
	int num_precision;
	double x_start, //!< X value corresponding to column 1
	x_end,  //!< X value corresponding to the last column
	y_start,  //!< Y value corresponding to row 1
	y_end;  //!< Y value corresponding to the last row

    //! Keeps track of the view type;
	ViewType d_view_type;
	//! Keeps track of the header view type;
	HeaderViewType d_header_view_type;

	QwtLinearColorMap d_color_map;
	ColorMapType d_color_map_type;

	//! Column width in pixels;
	int d_column_width;

	QShortcut *d_select_all_shortcut;
};

#endif
