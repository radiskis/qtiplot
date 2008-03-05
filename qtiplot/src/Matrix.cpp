/***************************************************************************
    File                 : Matrix.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2007 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
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
#include "Matrix.h"
#include "MatrixCommand.h"
#include "Graph.h"
#include "ApplicationWindow.h"
#include "MyParser.h"

#include <QtGlobal>
#include <QTextStream>
#include <QList>
#include <QEvent>
#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QHeaderView>
#include <QApplication>
#include <QMessageBox>
#include <QVarLengthArray>
#include <QClipboard>
#include <QShortcut>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QLocale>
#include <QItemDelegate>
#include <QLabel>
#include <QStackedWidget>
#include <QImageWriter>
#include <QSvgGenerator>
#include <QFile>
#include <QUndoStack>

#include <stdlib.h>
#include <stdio.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>

#include "analysis/fft2D.h"

Matrix::Matrix(ScriptingEnv *env, int r, int c, const QString& label, ApplicationWindow* parent, const QString& name, Qt::WFlags f)
: MdiSubWindow(label, parent, name, f), scripted(env)
{
	initTable(r, c);
}

Matrix::Matrix(ScriptingEnv *env, const QImage& image, const QString& label, ApplicationWindow* parent, const QString& name, Qt::WFlags f)
: MdiSubWindow(label, parent, name, f), scripted(env)
{
	initImage(image);
}

void Matrix::initGlobals()
{
    d_table_view = NULL;
    imageLabel = NULL;

    d_header_view_type = ColumnRow;
	d_color_map_type = GrayScale;
	d_color_map = QwtLinearColorMap(Qt::black, Qt::white);
    d_column_width = 100;

	formula_str = "";
	txt_format = 'f';
	num_precision = 6;
	x_start = 1.0;
	x_end = 10.0;
	y_start = 1.0;
	y_end = 10.0;

    d_stack = new QStackedWidget();
    d_stack->setFocusPolicy(Qt::StrongFocus);
	setWidget(d_stack);

	d_undo_stack = new QUndoStack();
	d_undo_stack->setUndoLimit(applicationWindow()->matrixUndoStackSize());
}

void Matrix::initTable(int rows, int cols)
{
    initGlobals();
	d_view_type = TableView;

    d_matrix_model = new MatrixModel(rows, cols, this);
    initTableView();

	// resize the table
	setGeometry(50, 50, QMIN(_Matrix_initial_columns_, cols)*d_table_view->horizontalHeader()->sectionSize(0) + 55,
                (QMIN(_Matrix_initial_rows_,rows)+1)*d_table_view->verticalHeader()->sectionSize(0));
}

void Matrix::initImage(const QImage& image)
{
    initGlobals();
	d_view_type = ImageView;

    d_matrix_model = new MatrixModel(image, this);
    initImageView();

	int w = image.width();
	int h = image.height();
	if (w <= 500 && h <= 400){
		int size = QMAX(w, h);
        resize(size, size);
    } else
		resize(500, 500);

	imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
}

double Matrix::cell(int row, int col)
{
	return d_matrix_model->cell(row, col);
}

void Matrix::setCell(int row, int col, double value)
{
	d_matrix_model->setCell(row, col, value);
}

QString Matrix::text(int row, int col)
{
    return d_matrix_model->text(row, col);
}

void Matrix::setText (int row, int col, const QString & new_text )
{
	d_matrix_model->setText(row, col, new_text);
}

void Matrix::setCoordinates(double xs, double xe, double ys, double ye)
{
	if (x_start == xs && x_end == xe &&	y_start == ys && y_end == ye)
		return;

	x_start = xs;
	x_end = xe;
	y_start = ys;
	y_end = ye;

	emit modifiedWindow(this);
}

QString Matrix::saveToString(const QString &info, bool saveAsTemplate)
{
    bool notTemplate = !saveAsTemplate;
	QString s = "<matrix>\n";
	if (notTemplate)
        s += QString(objectName()) + "\t";
	s += QString::number(numRows())+"\t";
	s += QString::number(numCols())+"\t";
	if (notTemplate)
        s += birthDate() + "\n";
	s += info;
	s += "ColWidth\t" + QString::number(d_column_width)+"\n";
	s += "<formula>\n" + formula_str + "\n</formula>\n";
	s += "TextFormat\t" + QString(txt_format) + "\t" + QString::number(num_precision) + "\n";
	if (notTemplate)
        s += "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	s += "Coordinates\t" + QString::number(x_start,'g',15) + "\t" +QString::number(x_end,'g',15) + "\t";
	s += QString::number(y_start,'g',15) + "\t" + QString::number(y_end,'g',15) + "\n";
	s += "ViewType\t" + QString::number((int)d_view_type) + "\n";
    s += "HeaderViewType\t" + QString::number((int)d_header_view_type) + "\n";

	if (d_color_map_type != Custom)
		s += "ColorPolicy\t" + QString::number(d_color_map_type) + "\n";
	else {
		s += "<ColorMap>\n";
		s += "\t<Mode>" + QString::number(d_color_map.mode()) + "</Mode>\n";
		s += "\t<MinColor>" + d_color_map.color1().name() + "</MinColor>\n";
		s += "\t<MaxColor>" + d_color_map.color2().name() + "</MaxColor>\n";
		QwtArray <double> colors = d_color_map.colorStops();
		int stops = (int)colors.size();
		s += "\t<ColorStops>" + QString::number(stops - 2) + "</ColorStops>\n";
		for (int i = 1; i < stops - 1; i++){
			s += "\t<Stop>" + QString::number(colors[i]) + "\t";
			s += QColor(d_color_map.rgb(QwtDoubleInterval(0,1), colors[i])).name();
			s += "</Stop>\n";
		}
		s += "</ColorMap>\n";
	}

    if (notTemplate)
        s += d_matrix_model->saveToString();
    s +="</matrix>\n";
	return s;
}

QString Matrix::saveAsTemplate(const QString &info)
{
	return saveToString(info, true);
}

void Matrix::restore(const QStringList &lst)
{
	QStringList l;
	QStringList::const_iterator i = lst.begin();

	l = (*i++).split("\t");
	setColumnsWidth(l[1].toInt());

	l = (*i++).split("\t");
	if (l[0] == "Formula")
		formula_str = l[1];
	else if (l[0] == "<formula>"){
		for(formula_str=""; i != lst.end() && *i != "</formula>"; i++)
			formula_str += *i + "\n";
		formula_str.truncate(formula_str.length()-1);
		i++;
	}

	l = (*i++).split("\t");
	if (l[1] == "f")
		setTextFormat('f', l[2].toInt());
	else
		setTextFormat('e', l[2].toInt());

	l = (*i++).split("\t");
	x_start = l[1].toDouble();
	x_end = l[2].toDouble();
	y_start = l[3].toDouble();
	y_end = l[4].toDouble();

	l = (*i++).split("\t");
	d_view_type = (Matrix::ViewType)l[1].toInt();
	l = (*i++).split("\t");
	d_header_view_type = (Matrix::HeaderViewType)l[1].toInt();
	l = (*i++).split("\t");
	d_color_map_type = (Matrix::ColorMapType)l[1].toInt();

    if (lst.contains ("<ColorMap>")){
        QStringList aux;
        while (*i != "</ColorMap>"){
            aux << *i;
            i++;
        }
        setColorMap(aux);
    }

    if (d_view_type == ImageView){
	    if (d_table_view)
            delete d_table_view;
        if (d_select_all_shortcut)
            delete d_select_all_shortcut;
	    initImageView();
		d_stack->setCurrentWidget(imageLabel);
		if (d_color_map_type == Rainbow)
            setRainbowColorMap();
	}
    resetView();
}

void Matrix::setNumericFormat(const QChar& f, int prec)
{
	if (txt_format == f && num_precision == prec)
		return;

	txt_format = f;
	num_precision = prec;

	resetView();
	emit modifiedWindow(this);
	QApplication::restoreOverrideCursor();
}

void Matrix::setTextFormat(const QChar &format, int precision)
{
	txt_format = format;
	num_precision = precision;
}

void Matrix::setColumnsWidth(int width)
{
	if (d_column_width == width)
		return;

    d_column_width = width;
    d_table_view->horizontalHeader()->setDefaultSectionSize(d_column_width);

    if (d_view_type == TableView){
        int cols = numCols();
        for(int i=0; i<cols; i++)
            d_table_view->setColumnWidth(i, width);
    }

	emit modifiedWindow(this);
}

void Matrix::setDimensions(int rows, int cols)
{
	int r = numRows();
	int c = numCols();

	if (r == rows && c == cols)
		return;

	if (rows < r || cols < c){
		QString msg_text = tr("Deleting rows/columns from the matrix!","set matrix dimensions");
		msg_text += tr("<p>Do you really want to continue?","set matrix dimensions");
		switch( QMessageBox::information(0, tr("QtiPlot"), msg_text, tr("Yes"), tr("Cancel"), 0, 1 ) )
		{
			case 0: {// Yes
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				MatrixModel *new_model = d_matrix_model->copy();
				if (cols != c)
					new_model->setColumnCount(cols);
				if (rows != r)
					new_model->setRowCount(rows);

                d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Set Dimensions") +
                " " + QString::number(rows) + "x" + QString::number(cols)));
                setMatrixModel(new_model);
				QApplication::restoreOverrideCursor();
				break;
			}
			case 1: // Cancel
				return;
				break;
		}
	} else {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		MatrixModel *new_model = d_matrix_model->copy();
        if (cols != c)
            new_model->setColumnCount(cols);
        if (rows != r)
            new_model->setRowCount(rows);

        d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Set Dimensions") +
                " " + QString::number(rows) + "x" + QString::number(cols)));
        setMatrixModel(new_model);
        QApplication::restoreOverrideCursor();
	}
}

double Matrix::determinant()
{
	int rows = numRows();
	int cols = numCols();

	if (rows != cols){
		QMessageBox::critical(0,tr("QtiPlot - Error"),
				tr("Calculation failed, the matrix is not square!"));
		return GSL_POSINF;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	gsl_matrix *A = gsl_matrix_alloc(rows, cols);
	int i;
	for(i=0; i<rows; i++)
		for(int j=0; j<cols; j++)
			gsl_matrix_set(A, i, j, d_matrix_model->cell(i, j));

	gsl_permutation * p = gsl_permutation_alloc(rows);
	gsl_linalg_LU_decomp(A, p, &i);

	double det = gsl_linalg_LU_det(A, i);

	gsl_matrix_free(A);
	gsl_permutation_free(p);

	QApplication::restoreOverrideCursor();
	return det;
}

void Matrix::invert()
{
	int rows = numRows();
	int cols = numCols();

	if (rows != cols){
		QMessageBox::critical(0,tr("QtiPlot - Error"),
				tr("Inversion failed, the matrix is not square!"));
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	gsl_matrix *A = gsl_matrix_alloc(rows, cols);
	int i;
	for(i=0; i<rows; i++){
		for(int j=0; j<cols; j++)
			gsl_matrix_set(A, i, j, d_matrix_model->cell(i, j));
	}

	gsl_permutation * p = gsl_permutation_alloc(cols);
	gsl_linalg_LU_decomp(A, p, &i);

	gsl_matrix *inverse = gsl_matrix_alloc(rows, cols);
	gsl_linalg_LU_invert(A, p, inverse);

	gsl_matrix_free(A);
	gsl_permutation_free(p);

    MatrixModel *new_matrix_model = new MatrixModel(rows, cols, this);
	for(i=0; i<rows; i++){
		for(int j=0; j<cols; j++)
			new_matrix_model->setCell(i, j, gsl_matrix_get(inverse, i, j));
	}

	gsl_matrix_free(inverse);

    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_matrix_model, tr("Invert")));
    setMatrixModel(new_matrix_model);

	QApplication::restoreOverrideCursor();
}

void Matrix::transpose()
{
	int rows = numRows();
	int cols = numCols();

	MatrixModel *new_matrix_model = new MatrixModel(cols, rows, this);
	for(int i = 0; i < cols; i++){
		for(int j = 0; j < rows; j++)
			new_matrix_model->setCell(i, j, d_matrix_model->data(j, i));
	}

    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_matrix_model, tr("Transpose")));
    setMatrixModel(new_matrix_model);
}

void Matrix::flipVertically()
{
	int rows = numRows();
	int cols = numCols();

	MatrixModel *new_matrix_model = new MatrixModel(rows, cols, this);
	for(int i = 0; i < rows; i++){
		int row = rows - i - 1;
		for(int j = 0; j < cols; j++)
			new_matrix_model->setCell(i, j, d_matrix_model->data(row, j));
	}

    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_matrix_model, tr("Flip Vertically")));
    setMatrixModel(new_matrix_model);
}

void Matrix::flipHorizontally()
{
	int rows = numRows();
	int cols = numCols();

	MatrixModel *new_matrix_model = new MatrixModel(rows, cols, this);
	for(int i = 0; i < cols; i++){
		int col = cols - i - 1;
		for(int j = 0; j < rows; j++)
			new_matrix_model->setCell(j, i, d_matrix_model->data(j, col));
	}

    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_matrix_model, tr("Flip Horizontally")));
    setMatrixModel(new_matrix_model);
}

void Matrix::rotate90(bool clockwise)
{
	int rows = numRows();
	int cols = numCols();

	MatrixModel *new_matrix_model = new MatrixModel(cols, rows, this);

	if (clockwise){
        int aux = rows - 1;
        for(int i = 0; i < rows; i++){
            int row = aux - i;
            for(int j = 0; j < cols; j++)
                new_matrix_model->setCell(j, i, d_matrix_model->data(row, j));
        }
	} else {
	    int aux = cols - 1;
        for(int i = 0; i < cols; i++){
            int col = aux - i;
            for(int j = 0; j < rows; j++)
                new_matrix_model->setCell(i, j, d_matrix_model->data(j, col));
        }
	}
    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_matrix_model, tr("Rotate 90°")));
    setMatrixModel(new_matrix_model);
}

bool Matrix::muParserCalculate(int startRow, int endRow, int startCol, int endCol)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	double dx = fabs(x_end-x_start)/(double)(numRows()-1);
	double dy = fabs(y_end-y_start)/(double)(numCols()-1);
	double r = startRow + 1.0;
	double c = startCol + 1.0;
	double x = x_start + startCol*dx;
	double y = y_start + startRow*dy;

	MyParser parser;
	parser.SetExpr(formula_str.ascii());
	parser.DefineVar("i", &r);
	parser.DefineVar("row", &r);
	parser.DefineVar("y", &y);
	parser.DefineVar("j", &c);
	parser.DefineVar("col", &c);
	parser.DefineVar("x", &x);
	try {
		parser.Eval();
	} catch(mu::ParserError &e){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("QtiPlot - Input error"), QString::fromStdString(e.GetMsg()));
		return false;		
	}

	int rows = numRows();
	int cols = numCols();

	if (endRow < 0)
		endRow = rows - 1;
	if (endCol < 0)
		endCol = cols - 1;
    if (endCol >= cols)
		cols = endCol + 1;
	if (endRow >= rows)
        rows = endRow + 1;

    MatrixModel *new_model = new MatrixModel(rows, cols, this);
	double *data = new_model->dataVector();		
	for(int row = startRow; row <= endRow; row++){
		r = row + 1.0;
		y = y_start + row*dy;
		int aux = cols*row + startCol;
		for(int col = startCol; col <= endCol; col++){
			c = col + 1.0;
			x = x_start + col*dx;
			data[aux] = parser.Eval();
			++aux;
		}
	}
			
    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Calculate Values")));
    setMatrixModel(new_model);

	QApplication::restoreOverrideCursor();
	return true;
}

bool Matrix::calculate(int startRow, int endRow, int startCol, int endCol, bool forceMuParser)
{
	if (QString(scriptEnv->name()) == "muParser" || forceMuParser)
		return muParserCalculate(startRow, endRow, startCol, endCol);
	
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	
	Script *script = scriptEnv->newScript(formula_str, this, QString("<%1>").arg(objectName()));
	connect(script, SIGNAL(error(const QString&,const QString&,int)), scriptEnv, SIGNAL(error(const QString&,const QString&,int)));
	connect(script, SIGNAL(print(const QString&)), scriptEnv, SIGNAL(print(const QString&)));
	if (!script->compile()){
		QApplication::restoreOverrideCursor();
		return false;
	}

	int rows = numRows();
	int cols = numCols();

	if (endRow < 0)
		endRow = rows - 1;
	if (endCol < 0)
		endCol = cols - 1;
    if (endCol >= cols)
		cols = endCol + 1;
	if (endRow >= rows)
        rows = endRow + 1;

    MatrixModel *new_model = new MatrixModel(rows, cols, this);
	
	QVariant ret;
	double dx = fabs(x_end-x_start)/(double)(numRows()-1);
	double dy = fabs(y_end-y_start)/(double)(numCols()-1);
	for(int row = startRow; row <= endRow; row++){
		script->setInt(row+1, "i");
		script->setInt(row+1, "row");
		script->setDouble(y_start+row*dy, "y");
		for(int col = startCol; col <= endCol; col++){
			script->setInt(col+1, "j");
			script->setInt(col+1, "col");
			script->setDouble(x_start+col*dx, "x");
			ret = script->eval();
			if (ret.canConvert(QVariant::Double))
				new_model->setCell(row, col, ret.toDouble());
			else {
				new_model->setText(row, col, "");
				QApplication::restoreOverrideCursor();
				return false;
			}
		}
	}
			
    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Calculate Values")));
    setMatrixModel(new_model);

	QApplication::restoreOverrideCursor();
	return true;
}

void Matrix::clearSelection()
{
    if (d_view_type == ImageView)
        return;

	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

    MatrixModel *old_model = d_matrix_model->copy();

	QModelIndexList lst = selModel->selection().indexes();
    foreach(QModelIndex index, lst)
		d_matrix_model->setText(index.row(), index.column(), "");

	d_undo_stack->push(new MatrixCommand(old_model, d_matrix_model, tr("Clear Selection")));

	d_table_view->reset();
    emit modifiedWindow(this);
}


void Matrix::copySelection()
{
    if (d_view_type == ImageView)
        return;

	QItemSelectionModel *selModel = d_table_view->selectionModel();
	QString s = "";
	if (!selModel->hasSelection()){
		QModelIndex index = selModel->currentIndex();
		s = text(index.row(), index.column());
	} else {
		QItemSelection sel = selModel->selection();
		QListIterator<QItemSelectionRange> it(sel);
		if(!it.hasNext())
			return;

		QItemSelectionRange cur = it.next();
		int top = cur.top();
		int bottom = cur.bottom();
		int left = cur.left();
		int right = cur.right();
		for(int i=top; i<=bottom; i++){
			for(int j=left; j<right; j++)
				s += d_matrix_model->text(i, j) + "\t";
			s += d_matrix_model->text(i,right) + "\n";
		}
	}
	// Copy text into the clipboard
	QApplication::clipboard()->setText(s.trimmed());
}

void Matrix::pasteSelection()
{
     if (d_view_type == ImageView)
        return;

	QString text = QApplication::clipboard()->text();
	if (text.isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QTextStream ts( &text, QIODevice::ReadOnly );
	QString s = ts.readLine();
	QStringList cellTexts = s.split("\t");
	int cols = cellTexts.count();
	int rows = 1;
	while(!ts.atEnd()){
		rows++;
		s = ts.readLine();
		int aux = s.split("\t").count();
		if (aux > cols)
            cols = aux;
	}
	ts.reset();

    int top = 0, left = 0;
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (selModel->hasSelection()){
		QItemSelection sel = selModel->selection();
		QListIterator<QItemSelectionRange> it(sel);
		if(!it.hasNext())
			return;

		QItemSelectionRange cur = it.next();
		top = cur.top();
		left = cur.left();
	}

	QTextStream ts2(&text, QIODevice::ReadOnly);

    MatrixModel *new_model = d_matrix_model->copy();
    if (top + rows > numRows())
        new_model->setRowCount(top + rows);
    if (left + cols > numCols())
        new_model->setColumnCount(left + cols);

	bool numeric;
	QLocale system_locale = QLocale::system();
	for(int i=top; i<top+rows; i++){
		s = ts2.readLine();
		cellTexts = s.split("\t");
		for(int j = left; j<left+cols; j++){
		    int colIndex = j-left;
            if (colIndex >= cellTexts.count())
                break;

			double value = system_locale.toDouble(cellTexts[colIndex], &numeric);
			if (numeric)
				new_model->setCell(i, j, value);
			else
				new_model->setText(i, j, cellTexts[colIndex]);
		}
	}
	d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Paste")));
    setMatrixModel(new_model);
	QApplication::restoreOverrideCursor();
}

void Matrix::cutSelection()
{
	copySelection();
	clearSelection();
}

void Matrix::deleteSelectedRows()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

	int startRow = 0;
	int rows = numRows();
	for (int i=0; i<rows; i++){
		if (selModel->isRowSelected (i, QModelIndex())){
			startRow = i;
			break;
		}
	}

	int count = 0;
	for (int i = startRow; i<rows; i++){
		if (selModel->isRowSelected (i, QModelIndex()))
			count++;
	}

    MatrixModel *old_model = d_matrix_model->copy();
	d_matrix_model->removeRows(startRow, count, QModelIndex());
	d_undo_stack->push(new MatrixCommand(old_model, d_matrix_model, tr("Delete Rows") + " " +
                      QString::number(startRow + 1) + " - " + QString::number(startRow + count)));
	d_table_view->reset();
	emit modifiedWindow(this);
}

void Matrix::deleteSelectedColumns()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

	int startCol = 0;
	int cols = numCols();
	for (int i=0; i<cols; i++){
		if (selModel->isColumnSelected(i, QModelIndex())){
			startCol = i;
			break;
		}
	}

	int count = 0;
	for (int i = startCol; i<cols; i++){
		if (selModel->isColumnSelected (i, QModelIndex()))
			count++;
	}

    MatrixModel *old_model = d_matrix_model->copy();
	d_matrix_model->removeColumns(startCol, count, QModelIndex());
	d_undo_stack->push(new MatrixCommand(old_model, d_matrix_model, tr("Delete Columns") + " " +
                      QString::number(startCol + 1) + " - " + QString::number(startCol + count)));

	d_table_view->reset();
	emit modifiedWindow(this);
}

int Matrix::numSelectedRows()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return 0;

	int rows = numRows();
	int count = 0;
	for (int i = 0; i<rows; i++){
		if (selModel->isRowSelected (i, QModelIndex()))
			count++;
	}
	return count;
}

int Matrix::numSelectedColumns()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return 0;

	int cols = numCols();
	int count = 0;
	for (int i = 0; i<cols; i++){
		if (selModel->isColumnSelected (i, QModelIndex()))
			count++;
	}
	return count;
}

void Matrix::insertRow()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

	QModelIndex index = selModel->currentIndex();
	if (!index.isValid())
		return;

    MatrixModel *old_model = d_matrix_model->copy();
	d_matrix_model->insertRows(index.row(), 1);
	d_table_view->reset();
	emit modifiedWindow(this);

    d_undo_stack->push(new MatrixCommand(old_model, d_matrix_model, tr("Insert Row") + " " +
                      QString::number(index.row() + 1)));
}

void Matrix::insertColumn()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

	QModelIndex index = selModel->currentIndex();
	if (!index.isValid())
		return;

    MatrixModel *old_model = d_matrix_model->copy();
	d_matrix_model->insertColumns(index.column(), 1);
	d_table_view->reset();
	emit modifiedWindow(this);

    d_undo_stack->push(new MatrixCommand(old_model, d_matrix_model, tr("Insert Column") + " " +
                      QString::number(index.column() + 1)));
}

void Matrix::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

void Matrix::exportRasterImage(const QString& fileName, int quality)
{
	d_matrix_model->renderImage().save(fileName, 0, quality);
}

void Matrix::exportToFile(const QString& fileName)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

	if (fileName.contains(".eps") || fileName.contains(".pdf") || fileName.contains(".ps")){
		exportVector(fileName);
		return;
	} else if(fileName.contains(".svg")){
		exportSVG(fileName);
		return;
	} else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
    	for(int i=0 ; i<list.count() ; i++){
			if (fileName.contains( "." + list[i].toLower())){
				d_matrix_model->renderImage().save(fileName, list[i], 100);
				return;
			}
		}
    	QMessageBox::critical(this, tr("QtiPlot - Error"), tr("File format not handled, operation aborted!"));
	}
}

void Matrix::exportSVG(const QString& fileName)
{
	#if QT_VERSION >= 0x040300
		if (d_view_type != ImageView)
			return;

		int width = numRows();
		int height = numCols();

		QSvgGenerator svg;
        svg.setFileName(fileName);
        svg.setSize(QSize(width, height));

		QPainter p(&svg);
        p.drawImage (QRect(0, 0, width, height), d_matrix_model->renderImage());
		p.end();
	#endif
}

void Matrix::exportPDF(const QString& fileName)
{
	print(fileName);
}

void Matrix::print()
{
    print(QString());
}

void Matrix::print(const QString& fileName)
{
	QPrinter printer;
	printer.setColorMode (QPrinter::GrayScale);

	if (!fileName.isEmpty()){
	    printer.setCreator("QtiPlot");
	    printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
	} else {
        QPrintDialog printDialog(&printer);
        if (printDialog.exec() != QDialog::Accepted)
            return;
    }
		printer.setFullPage( true );
		QPainter p;
		if ( !p.begin(&printer ) )
			return; // paint on printer
		int dpiy = printer.logicalDpiY();
		const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

        if (d_view_type == ImageView){
            p.drawImage (printer.pageRect(), d_matrix_model->renderImage());
            return;
        }

		QHeaderView *vHeader = d_table_view->verticalHeader();

		int rows = numRows();
		int cols = numCols();
		int height = margin;
		int i, vertHeaderWidth = vHeader->width();
		int right = margin + vertHeaderWidth;

		// print header
		p.setFont(QFont());
		QString header_label = d_matrix_model->headerData(0, Qt::Horizontal).toString();
		QRect br = p.boundingRect(br, Qt::AlignCenter, header_label);
		p.drawLine(right, height, right, height+br.height());
		QRect tr(br);

		for(i=0; i<cols; i++){
			int w = d_table_view->columnWidth(i);
			tr.setTopLeft(QPoint(right,height));
			tr.setWidth(w);
			tr.setHeight(br.height());
			header_label = d_matrix_model->headerData(i, Qt::Horizontal).toString();
			p.drawText(tr, Qt::AlignCenter, header_label,-1);
			right += w;
			p.drawLine(right, height, right, height+tr.height());

			if (right >= printer.width()-2*margin )
				break;
		}

		p.drawLine(margin + vertHeaderWidth, height, right-1, height);//first horizontal line
		height += tr.height();
		p.drawLine(margin, height, right-1, height);

		// print table values
		for(i=0;i<rows;i++){
			right = margin;
			QString cell_text = d_matrix_model->headerData(i, Qt::Horizontal).toString()+"\t";
			tr = p.boundingRect(tr, Qt::AlignCenter, cell_text);
			p.drawLine(right, height, right, height+tr.height());

			br.setTopLeft(QPoint(right,height));
			br.setWidth(vertHeaderWidth);
			br.setHeight(tr.height());
			p.drawText(br,Qt::AlignCenter,cell_text,-1);
			right += vertHeaderWidth;
			p.drawLine(right, height, right, height+tr.height());

			for(int j=0; j<cols; j++){
				int w = d_table_view->columnWidth (j);
				cell_text = text(i,j)+"\t";
				tr = p.boundingRect(tr,Qt::AlignCenter,cell_text);
				br.setTopLeft(QPoint(right,height));
				br.setWidth(w);
				br.setHeight(tr.height());
				p.drawText(br, Qt::AlignCenter, cell_text, -1);
				right += w;
				p.drawLine(right, height, right, height+tr.height());

				if (right >= printer.width()-2*margin )
					break;
			}
			height += br.height();
			p.drawLine(margin, height, right-1, height);

			if (height >= printer.height()-margin ){
				printer.newPage();
				height = margin;
				p.drawLine(margin, height, right, height);
			}
		}
}

void Matrix::exportVector(const QString& fileName, int res, bool color, bool keepAspect, QPrinter::PageSize pageSize)
{
    if (d_view_type != ImageView)
        return;

	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

	QPrinter printer;
    printer.setCreator("QtiPlot");
	printer.setFullPage(true);
	if (res)
		printer.setResolution(res);

    printer.setOutputFileName(fileName);
    if (fileName.contains(".eps"))
    	printer.setOutputFormat(QPrinter::PostScriptFormat);

    if (color)
		printer.setColorMode(QPrinter::Color);
	else
		printer.setColorMode(QPrinter::GrayScale);

    int cols = numCols();
    int rows = numRows();
    QRect rect = QRect(0, 0, cols, rows);
    if (pageSize == QPrinter::Custom)
        printer.setPageSize(Graph::minPageSize(printer, rect));
    else
        printer.setPageSize(pageSize);

    double aspect = (double)cols/(double)rows;
	if (aspect < 1)
		printer.setOrientation(QPrinter::Portrait);
	else
		printer.setOrientation(QPrinter::Landscape);

    if (keepAspect){// export should preserve aspect ratio
        double page_aspect = double(printer.width())/double(printer.height());
        if (page_aspect > aspect){
            int margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
            int height = printer.height() - 2*margin;
            int width = int(height*aspect);
            int x = (printer.width()- width)/2;
            rect = QRect(x, margin, width, height);
        } else if (aspect >= page_aspect){
            int margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
            int width = printer.width() - 2*margin;
            int height = int(width/aspect);
            int y = (printer.height()- height)/2;
            rect = QRect(margin, y, width, height);
        }
	} else {
	    int x_margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
        int y_margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
        int width = printer.width() - 2*x_margin;
        int height = printer.height() - 2*y_margin;
        rect = QRect(x_margin, y_margin, width, height);
	}

    QPainter paint(&printer);
    paint.drawImage(rect, d_matrix_model->renderImage());
    paint.end();
}

void Matrix::range(double *min, double *max)
{
	double d_min = cell(0, 0);
	double d_max = d_min;
	int rows = numRows();
	int cols = numCols();

	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			double aux = cell(i, j);
			if (aux <= d_min)
				d_min = aux;

			if (aux >= d_max)
				d_max = aux;
		}
	}

	*min = d_min;
	*max = d_max;
}

double** Matrix::allocateMatrixData(int rows, int columns)
{
	double** data = new double* [rows];
	for ( int i = 0; i < rows; ++i)
		data[i] = new double [columns];

	return data;
}

void Matrix::freeMatrixData(double **data, int rows)
{
	for ( int i = 0; i < rows; i++)
		delete [] data[i];

	delete [] data;
}

void Matrix::goToRow(int row)
{
	if(row < 1 || row > numRows())
		return;

	d_table_view->selectRow(row - 1);
}

void Matrix::moveCell(const QModelIndex& index)
{
	if (!index.isValid())
		return;

	d_table_view->setCurrentIndex(d_matrix_model->index(index.row() + 1, index.column()));
}

void Matrix::copy(Matrix *m)
{
	if (!m)
        return;

	x_start = m->xStart();
	x_end = m->xEnd();
	y_start = m->yStart();
	y_end = m->yEnd();

	int rows = numRows();
	int cols = numCols();

	MatrixModel *mModel = m->matrixModel();
	if (!mModel)
		return;

    txt_format = m->textFormat();
	num_precision = m->precision();

    for (int i=0; i<rows; i++)
        for (int j=0; j<cols; j++)
            d_matrix_model->setCell(i, j, mModel->data(i, j));

	d_header_view_type = m->headerViewType();
    d_view_type = m->viewType();
	setColumnsWidth(m->columnsWidth());
	formula_str = m->formula();
    d_color_map_type = m->colorMapType();
    d_color_map = m->colorMap();

    if (d_view_type == ImageView){
	    if (d_table_view)
            delete d_table_view;
        if (d_select_all_shortcut)
            delete d_select_all_shortcut;
	    initImageView();
		d_stack->setCurrentWidget(imageLabel);
	}
	resetView();
}

void Matrix::setViewType(ViewType type)
{
	if (d_view_type == type)
		return;

	d_view_type = type;

	if (d_view_type == ImageView){
	    if (d_table_view)
            delete d_table_view;
        if (d_select_all_shortcut)
            delete d_select_all_shortcut;
	    initImageView();
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
		d_stack->setCurrentWidget(imageLabel);
	} else if (d_view_type == TableView){
	    if (imageLabel)
            delete imageLabel;
	    initTableView();
	    d_stack->setCurrentWidget(d_table_view);
	}
	emit modifiedWindow(this);
}

void Matrix::initImageView()
{
    imageLabel = new QLabel();
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    d_stack->addWidget(imageLabel);
}

void Matrix::initTableView()
{
    d_table_view = new QTableView();
    d_table_view->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    d_table_view->setSelectionMode(QAbstractItemView::ContiguousSelection);// only one contiguous selection supported
    d_table_view->setModel(d_matrix_model);
    d_table_view->setEditTriggers(QAbstractItemView::DoubleClicked);
    d_table_view->setFocusPolicy(Qt::StrongFocus);
    d_table_view->setFocus();

    QPalette pal = d_table_view->palette();
	pal.setColor(QColorGroup::Base, QColor(255, 255, 128));
	d_table_view->setPalette(pal);

	// set header properties
	QHeaderView* hHeader = (QHeaderView*)d_table_view->horizontalHeader();
	hHeader->setMovable(false);
	hHeader->setResizeMode(QHeaderView::Fixed);
	hHeader->setDefaultSectionSize(d_column_width);

    int cols = numCols();
	for(int i=0; i<cols; i++)
		d_table_view->setColumnWidth(i, d_column_width);

	QHeaderView* vHeader = (QHeaderView*)d_table_view->verticalHeader();
	vHeader->setMovable(false);
	vHeader->setResizeMode(QHeaderView::ResizeToContents);

    d_stack->addWidget(d_table_view);

    // recreate keyboard shortcut
	d_select_all_shortcut = new QShortcut(QKeySequence(tr("Ctrl+A", "Matrix: select all")), this);
	connect(d_select_all_shortcut, SIGNAL(activated()), d_table_view, SLOT(selectAll()));
}

QImage Matrix::image()
{
	return d_matrix_model->renderImage();
}

void Matrix::setImage(const QImage& image)
{
    if (d_table_view)
        delete d_table_view;

    d_view_type = ImageView;
    initImageView();

    MatrixModel *new_model = new MatrixModel(image, this);
    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, tr("Import Image")));
    d_matrix_model = new_model;

    imageLabel->setPixmap(QPixmap::fromImage(image));
    d_stack->setCurrentWidget(imageLabel);
    emit modifiedWindow(this);
}

void Matrix::importImage(const QString& fn)
{
	QImage image(fn);
    if (image.isNull())
        return;

	setImage(image);
}

void Matrix::setGrayScale()
{
    d_color_map_type = GrayScale;
	d_color_map = QwtLinearColorMap(Qt::black, Qt::white);
	if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
	emit modifiedWindow(this);
}

void Matrix::setRainbowColorMap()
{
    d_color_map_type = Rainbow;

	d_color_map = QwtLinearColorMap(Qt::blue, Qt::red);
	d_color_map.addColorStop(0.25, Qt::cyan);
	d_color_map.addColorStop(0.5, Qt::green);
	d_color_map.addColorStop(0.75, Qt::yellow);

	if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
	emit modifiedWindow(this);
}

void Matrix::setColorMap(const QwtLinearColorMap& map)
{
	d_color_map_type = Custom;
	d_color_map = map;
	if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));

	emit modifiedWindow(this);
}

void Matrix::setColorMap(const QStringList& lst)
{
	d_color_map_type = Custom;

	QStringList::const_iterator line = lst.begin();
  	QString s = (*line).stripWhiteSpace();

	int mode = s.remove("<Mode>").remove("</Mode>").stripWhiteSpace().toInt();
    s = *(++line);
    QColor color1 = QColor(s.remove("<MinColor>").remove("</MinColor>").stripWhiteSpace());
    s = *(++line);
    QColor color2 = QColor(s.remove("<MaxColor>").remove("</MaxColor>").stripWhiteSpace());

	d_color_map = QwtLinearColorMap(color1, color2);
	d_color_map.setMode((QwtLinearColorMap::Mode)mode);

	s = *(++line);
	int stops = s.remove("<ColorStops>").remove("</ColorStops>").stripWhiteSpace().toInt();
	for (int i = 0; i < stops; i++){
		s = (*(++line)).stripWhiteSpace();
		QStringList l = QStringList::split("\t", s.remove("<Stop>").remove("</Stop>"));
		d_color_map.addColorStop(l[0].toDouble(), QColor(l[1]));
	}
}

void Matrix::setColorMapType(ColorMapType mapType)
{
	d_color_map_type = mapType;

	if (d_color_map_type == GrayScale)
        setGrayScale();
    else if (d_color_map_type == Rainbow)
        setRainbowColorMap();
}

void Matrix::resetView()
{
    if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
    else if (d_view_type == TableView){
        d_table_view->setModel(d_matrix_model);
        d_table_view->horizontalHeader()->setDefaultSectionSize(d_column_width);
        d_table_view->horizontalHeader()->reset();
        d_table_view->verticalHeader()->reset();
        d_table_view->reset();
        QSize size = this->size();
        this->resize(QSize(size.width() + 1, size.height()));
        this->resize(size);
    }
}

void Matrix::setHeaderViewType(HeaderViewType type)
{
    if (d_header_view_type == type)
        return;

    d_header_view_type = type;

    if (d_view_type == TableView)
        resetView();
	emit modifiedWindow(this);
}

QwtDoubleRect Matrix::boundingRect()
{
    int rows = numRows();
    int cols = numCols();
    double dx = fabs(x_end - x_start)/(double)(cols - 1);
    double dy = fabs(y_end - y_start)/(double)(rows - 1);

    return QwtDoubleRect(QMIN(x_start, x_end) - 0.5*dx, QMIN(y_start, y_end) - 0.5*dy,
						 fabs(x_end - x_start) + dx, fabs(y_end - y_start) + dy).normalized();
}

void Matrix::fft(bool inverse)
{
	int width = numCols();
    int height = numRows();

    MatrixModel *new_model = new MatrixModel(height, width, this);
    double **x_int_re = allocateMatrixData(height, width); /* real coeff matrix */
    double **x_int_im = allocateMatrixData(height, width); /* imaginary coeff  matrix*/
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            x_int_re[i][j] = cell(i, j);
            x_int_im[i][j] = 0.0;
        }
    }

    QString commandText = tr("Forward FFT");
    if (inverse){
        double **x_fin_re = allocateMatrixData(height, width);
        double **x_fin_im = allocateMatrixData(height, width);
        fft2d_inv(x_int_re, x_int_im, x_fin_re, x_fin_im, width, height);

        for (int i = 0; i < height; i++){
            for (int j = 0; j < width; j++){
                double re = x_fin_re[i][j];
                double im = x_fin_im[i][j];
                new_model->setCell(i, j, sqrt(re*re + im*im));
            }
        }
        freeMatrixData(x_fin_re, height);
        freeMatrixData(x_fin_im, height);
        commandText = tr("Inverse FFT");
    } else {
        fft2d(x_int_re, x_int_im, width, height);

        for (int i = 0; i < height; i++){
            for (int j = 0; j < width; j++){
                double re = x_int_re[i][j];
                double im = x_int_im[i][j];
                new_model->setCell(i, j, sqrt(re*re + im*im));
            }
        }
    }

    freeMatrixData(x_int_re, height);
    freeMatrixData(x_int_im, height);

    d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model, commandText));
    setMatrixModel(new_model);
}

bool Matrix::exportASCII(const QString& fname, const QString& separator, bool exportSelection)
{
	QFile f(fname);
	if ( !f.open( QIODevice::WriteOnly ) ){
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("QtiPlot - ASCII Export Error"),
				tr("Could not write to file: <br><h4>%1</h4><p>Please verify that you have the right to write to this location!").arg(fname));
		return false;
	}

	int rows = numRows();
	int cols = numCols();
	QTextStream t( &f );

	if (exportSelection && d_view_type == TableView){
        QModelIndexList selectedIndexes = d_table_view->selectionModel()->selectedIndexes();
        int topRow = selectedIndexes[0].row();
        int bottomRow = topRow;
        int leftCol = selectedIndexes[0].column();
        int rightCol = leftCol;
        foreach(QModelIndex index, selectedIndexes){
            int row = index.row();
            if (row < topRow)
                topRow = row;
            if (row > bottomRow)
                bottomRow = row;

            int col = index.column();
            if (col < leftCol)
                leftCol = col;
            if (col > rightCol)
                rightCol = col;
        }

		for (int i = topRow; i <= bottomRow; i++){
			for (int j = leftCol; j < rightCol; j++){
				t << d_matrix_model->text(i, j);
				t << separator;
			}
			t << d_matrix_model->text(i, rightCol);
			t << "\n";
		}
	} else {
		for (int i=0; i<rows; i++) {
			for (int j=0; j<cols-1; j++){
				t << d_matrix_model->text(i,j);
				t << separator;
			}
			t << d_matrix_model->text(i, cols-1);
			t << "\n";
		}
	}
	f.close();
	return true;
}

void Matrix::importASCII(const QString &fname, const QString &sep, int ignoredLines,
    	bool stripSpaces, bool simplifySpaces, const QString& commentString,
		ImportMode importAs, const QLocale& locale, int endLineChar, int maxRows)
{
    MatrixModel *new_model = new MatrixModel(numRows(), numCols(), this);
    if (new_model->importASCII(fname, sep, ignoredLines, stripSpaces,
		simplifySpaces, commentString, importAs, locale, endLineChar, maxRows)){
        d_undo_stack->push(new MatrixCommand(d_matrix_model, new_model,
                    tr("Import ASCII File") + " \"" + fname + "\""));
		setMatrixModel(new_model);
	} else
        delete new_model;
}

void Matrix::setMatrixModel(MatrixModel *model)
{
    if(!model)
        return;

	d_matrix_model = model;
	resetView();
	emit modifiedWindow(this);
}

Matrix::~Matrix()
{
    delete d_undo_stack;
}
