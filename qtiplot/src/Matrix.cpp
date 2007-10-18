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
#include "MatrixModel.h"

#include <QtGlobal>
#include <QTextStream>
#include <QList>
#include <QEvent>
#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QHeaderView>
#include <QDateTime>
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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>

Matrix::Matrix(ScriptingEnv *env, int r, int c, const QString& label, QWidget* parent, const QString& name, Qt::WFlags f)
: MyWidget(label, parent, name, f), scripted(env)
{
	initTable(r, c);
}

Matrix::Matrix(ScriptingEnv *env, const QImage& image, const QString& label, QWidget* parent, const QString& name, Qt::WFlags f)
: MyWidget(label, parent, name, f), scripted(env)
{
	initImage(image);
}

void Matrix::initGlobals()
{
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

	QDateTime dt = QDateTime::currentDateTime();
	setBirthDate(dt.toString(Qt::LocalDate));

    d_stack = new QStackedWidget();
	QVBoxLayout *d_main_layout = new QVBoxLayout(this);
	d_main_layout->setMargin(0);
    d_main_layout->addWidget(d_stack);
}

void Matrix::initTable(int rows, int cols)
{
    initGlobals();
	d_view_type = TableView;

    d_matrix_model = new MatrixModel(rows, cols, this);
    initTableView();

	// resize the table
	setGeometry(50, 50, qMin(_Matrix_initial_columns_, cols)*d_table_view->horizontalHeader()->sectionSize(0) + 55,
                (qMin(_Matrix_initial_rows_,rows)+1)*d_table_view->verticalHeader()->sectionSize(0));
}

void Matrix::initImage(const QImage& image)
{
    initGlobals();
	d_view_type = ImageView;

    d_matrix_model = new MatrixModel(image, this);
    initImageView();
    imageLabel->setPixmap(QPixmap::fromImage(image));

	int w = image.width();
	int h = image.height();
	if (w <= 500 && h <= 400)
        resize(w, h);
    else // resize the image and keep aspect ratio
        resize(400*(double)w/(double)h, 400);
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

QString Matrix::saveToString(const QString &info)
{
	QString s = "<matrix>\n";
	s += QString(name()) + "\t";
	s += QString::number(numRows())+"\t";
	s += QString::number(numCols())+"\t";
	s += birthDate() + "\n";
	s += info;
	s += "ColWidth\t" + QString::number(d_column_width)+"\n";
	s += "<formula>\n" + formula_str + "\n</formula>\n";
	s += "TextFormat\t" + QString(txt_format) + "\t" + QString::number(num_precision) + "\n";
	s += "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	s += "Coordinates\t" + QString::number(x_start,'g',15) + "\t" +QString::number(x_end,'g',15) + "\t";
	s += QString::number(y_start,'g',15) + "\t" + QString::number(y_end,'g',15) + "\n";
	s += d_matrix_model->saveToString();
	s +="</matrix>\n";
	return s;
}

QString Matrix::saveAsTemplate(const QString &info)
{
	QString s= "<matrix>\t";
	s+= QString::number(numRows())+"\t";
	s+= QString::number(numCols())+"\n";
	s+= info;
	s+= "ColWidth\t" + QString::number(d_table_view->columnWidth(0))+"\n";
	s+= "<formula>\n" + formula_str + "\n</formula>\n";
	s+= "TextFormat\t" + QString(txt_format) + "\t" + QString::number(num_precision) + "\n";
	s+= "Coordinates\t" + QString::number(x_start,'g',15) + "\t" +QString::number(x_end,'g',15) + "\t";
	s+= QString::number(y_start,'g',15) + "\t" + QString::number(y_end,'g',15) + "\n";
	return s;
}

void Matrix::restore(const QStringList &lst)
{
	QStringList l;
	QStringList::const_iterator i = lst.begin();

	l= (*i++).split("\t");
	setColumnsWidth(l[1].toInt());

	l= (*i++).split("\t");
	if (l[0] == "Formula")
		formula_str = l[1];
	else if (l[0] == "<formula>"){
		for(formula_str=""; i != lst.end() && *i != "</formula>"; i++)
			formula_str += *i + "\n";
		formula_str.truncate(formula_str.length()-1);
		i++;
	}

	l= (*i++).split("\t");
	if (l[1] == "f")
		setTextFormat('f', l[2].toInt());
	else
		setTextFormat('e', l[2].toInt());

	l= (*i++).split("\t");
	x_start = l[1].toDouble();
	x_end = l[2].toDouble();
	y_start = l[3].toDouble();
	y_end = l[4].toDouble();
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
		switch( QMessageBox::information(0, tr("QtiPlot"), msg_text,tr("Yes"), tr("Cancel"), 0, 1 ) )
		{
			case 0: // Yes
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				if (cols != c)
					setNumCols(cols);
				if (rows != r)
					setNumRows(rows);

                resetView();
				QApplication::restoreOverrideCursor();
				emit modifiedWindow(this);
				break;

			case 1: // Cancel
				return;
				break;
		}
	} else {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		if (cols != c)
			setNumCols(cols);
		if (rows != r)
			setNumRows(rows);

        resetView();
		QApplication::restoreOverrideCursor();
		emit modifiedWindow(this);
	}
}

int Matrix::numRows()
{
	return d_matrix_model->rowCount();
}

void Matrix::setNumRows(int rows)
{
	int old_rows = numRows();
	if (old_rows == rows)
		return;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int diff = abs(rows - old_rows);
	if (rows > old_rows )
		d_matrix_model->insertRows(old_rows, diff);
    else if (rows < old_rows )
		d_matrix_model->removeRows(rows, diff);

	QApplication::restoreOverrideCursor();
}

int Matrix::numCols()
{
	return d_matrix_model->columnCount();
}

void Matrix::setNumCols(int cols)
{
	int old_cols = numCols();
	if (old_cols == cols)
		return;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int diff = abs(cols - old_cols);
	if (cols > old_cols )
		d_matrix_model->insertColumns(old_cols, diff);
    else if (cols < old_cols )
		d_matrix_model->removeColumns(cols, diff);

	QApplication::restoreOverrideCursor();
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

	for(i=0; i<rows; i++){
		for(int j=0; j<cols; j++)
			d_matrix_model->setCell(i, j, gsl_matrix_get(inverse, i, j));
	}

	gsl_matrix_free(inverse);

	resetView();
	QApplication::restoreOverrideCursor();
	emit modifiedWindow(this);
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

	delete d_matrix_model;
	d_matrix_model = new_matrix_model;

	resetView();
	emit modifiedWindow(this);
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

	delete d_matrix_model;
	d_matrix_model = new_matrix_model;

	resetView();
	emit modifiedWindow(this);
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

	delete d_matrix_model;
	d_matrix_model = new_matrix_model;

	resetView();
	emit modifiedWindow(this);
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

	delete d_matrix_model;
	d_matrix_model = new_matrix_model;

	resetView();
	emit modifiedWindow(this);
}

bool Matrix::calculate(int startRow, int endRow, int startCol, int endCol)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	Script *script = scriptEnv->newScript(formula_str, this, QString("<%1>").arg(name()));
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
		setNumCols(endCol+1);
	if (endRow >= rows)
		setNumRows(endRow+1);

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
			/*if (ret.type()==QVariant::Int || ret.type()==QVariant::UInt || ret.type()==QVariant::LongLong
					|| ret.type()==QVariant::ULongLong)
				setCell(row, col, ret.toDouble());
			else */
			if (ret.canConvert(QVariant::Double))
				d_matrix_model->setCell(row, col, ret.toDouble());
			else {
				d_matrix_model->setText(row, col, "");
				QApplication::restoreOverrideCursor();
				return false;
			}
		}
	}

	resetView();
	emit modifiedWindow(this);
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

	QModelIndexList lst = selModel->selection().indexes();
	foreach(QModelIndex index, lst)
		d_matrix_model->setText(index.row(), index.column(), "");

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

	if (top + rows > numRows())
        setNumRows(top + rows);
    if (left + cols > numCols())
        setNumCols(left + cols);

	bool numeric;
	QLocale system_locale = QLocale::system();
	for(int i=top; i<top+rows; i++){
		s = ts2.readLine();
		cellTexts = s.split("\t");
		for(int j=left; j<left+cols; j++){
			double value = system_locale.toDouble(cellTexts[j-left], &numeric);
			if (numeric)
				d_matrix_model->setCell(i, j, value);
			else
				d_matrix_model->setText(i, j, cellTexts[j-left]);
		}
	}
	d_table_view->reset();
	emit modifiedWindow(this);
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

	d_matrix_model->removeRows(startRow, count, QModelIndex());
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

	d_matrix_model->removeColumns(startCol, count, QModelIndex());
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

	d_matrix_model->insertRows(index.row(), 1);
	d_table_view->reset();
	emit modifiedWindow(this);
}

void Matrix::insertColumn()
{
	QItemSelectionModel *selModel = d_table_view->selectionModel();
	if (!selModel || !selModel->hasSelection())
		return;

	QModelIndex index = selModel->currentIndex();
	if (!index.isValid())
		return;

	d_matrix_model->insertColumns(index.column(), 1);
	d_table_view->reset();
	emit modifiedWindow(this);
}

void Matrix::contextMenuEvent(QContextMenuEvent *e)
{
	emit showContextMenu();
	e->accept();
}

void Matrix::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

bool Matrix::eventFilter(QObject *object, QEvent *e)
{
	if (e->type()==QEvent::ContextMenu && object == titleBar){
		emit showTitleBarMenu();
		((QContextMenuEvent*)e)->accept();
		return true;
	}

	return MyWidget::eventFilter(object, e);
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

    for (int i=0; i<rows; i++)
        for (int j=0; j<cols; j++)
            d_matrix_model->setCell(i, j, mModel->data(i, j));

	setColumnsWidth(m->columnsWidth());
	formula_str = m->formula();
	setTextFormat(m->textFormat(), m->precision());
	setViewType(m->viewType());
	if (m->colorMapType() == GrayScale)
        setGrayScale();
    else if (m->colorMapType() == Rainbow)
        setRainbowColorMap();
}

void Matrix::setViewType(ViewType type)
{
	if (d_view_type == type)
		return;

	d_view_type = type;

	if (d_view_type == ImageView){
	    if (d_table_view)
            delete d_table_view;
	    initImageView();
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
		d_stack->setCurrentWidget(imageLabel);
	} else if (d_view_type == TableView){
	    if (imageLabel)
            delete imageLabel;
	    initTableView();
	    d_stack->setCurrentWidget(d_table_view);
	}
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

    int cols = numCols();
	for(int i=0; i<cols; i++)
		d_table_view->setColumnWidth(i, d_column_width);

    QPalette pal = d_table_view->palette();
	pal.setColor(QColorGroup::Base, QColor(255, 255, 128));
	d_table_view->setPalette(pal);

	// set header properties
	QHeaderView* hHeader = (QHeaderView*)d_table_view->horizontalHeader();
	hHeader->setMovable(false);
	hHeader->setResizeMode(QHeaderView::Fixed);
	QHeaderView* vHeader = (QHeaderView*)d_table_view->verticalHeader();
	vHeader->setMovable(false);
	vHeader->setResizeMode(QHeaderView::ResizeToContents);

    d_stack->addWidget(d_table_view);

    // keyboard shortcuts
	QShortcut * sel_all = new QShortcut(QKeySequence(tr("Ctrl+A", "Matrix: select all")), this);
	connect(sel_all, SIGNAL(activated()), d_table_view, SLOT(selectAll()));
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
    if (d_matrix_model)
        delete d_matrix_model;
    d_matrix_model = new MatrixModel(image, this);
    imageLabel->setPixmap(QPixmap::fromImage(image));
    d_stack->setCurrentWidget(imageLabel);
}

void Matrix::setGrayScale()
{
    d_color_map_type = GrayScale;
	d_color_map = QwtLinearColorMap(Qt::black, Qt::white);
	if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
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
}

void Matrix::resetView()
{
    if (d_view_type == ImageView)
		imageLabel->setPixmap(QPixmap::fromImage(d_matrix_model->renderImage()));
    else if (d_view_type == TableView)
        d_table_view->setModel(d_matrix_model);
}
