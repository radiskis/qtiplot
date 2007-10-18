/***************************************************************************
	File                 : MatrixModel.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2007 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's matrix model

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
#include <QtGui>

#include "Matrix.h"
#include "MatrixModel.h"
#include <gsl/gsl_math.h>
#include <qwt_color_map.h>

MatrixModel::MatrixModel(int rows, int cols, QObject *parent)
     : QAbstractTableModel(parent),
	 d_matrix((Matrix*)parent),
     d_rows(rows),
     d_cols(cols)
{
    d_data.resize(d_rows*d_cols);
	d_data.fill(GSL_NAN);
}

MatrixModel::MatrixModel(const QImage& image, QObject *parent)
     : QAbstractTableModel(parent),
	 d_matrix((Matrix*)parent)
{
    d_rows = image.height();
    d_cols = image.width();
    d_data.resize(d_rows*d_cols);

    for (int i=0; i<d_rows; i++ ){
		for (int j=0; j<d_cols; j++){
		    int index = i*d_cols + j;
            d_data[index] = qGray(image.pixel (j, i));
		}
    }
}

Qt::ItemFlags MatrixModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

int MatrixModel::rowCount(const QModelIndex & /* parent */) const
{
    return d_rows;
}

int MatrixModel::columnCount(const QModelIndex & /* parent */) const
{
    return d_cols;
}

double MatrixModel::cell(int row, int col)
{
    int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size() || gsl_isnan (d_data[i]))
        return 0.0;

	return d_data[i];
}

void MatrixModel::setCell(int row, int col, double val)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
		return;

    d_data[i] = val;
}

QString MatrixModel::text(int row, int col)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size() || gsl_isnan (d_data[i]))
        return "";

	QLocale locale = d_matrix->locale();
	return locale.toString(d_data[i], d_matrix->textFormat().toAscii(), d_matrix->precision());
}

void MatrixModel::setText(int row, int col, const QString& text)
{
	int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
		return;

 	if (text.isEmpty())
		d_data[i] = GSL_NAN;
	else
		d_data[i] = text.toDouble();
}

double MatrixModel::data(int row, int col) const
{
    int i = d_cols*row + col;
    if (i < 0 || i>= d_data.size())
        return 0.0;

	return d_data[i];
}

QVariant MatrixModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
		return QVariant();

    int i = d_cols*index.row() + index.column();
    if (gsl_isnan (d_data[i]))
        return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole){
		QLocale locale = d_matrix->locale();
		return QVariant(locale.toString(d_data[i], d_matrix->textFormat().toAscii(), d_matrix->precision()));
	}else
		return QVariant();
}

bool MatrixModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	int i = d_cols*index.row() + index.column();
 	if(role == Qt::EditRole){
		if (value.toString().isEmpty())
			d_data[i] = GSL_NAN;
		else
			d_data[i] = value.toDouble();
	}

	if(index.row() + 1 >= d_rows){
        insertRows(d_rows, 1);
		d_matrix->resetView();
	}

	d_matrix->moveCell(index);
	return false;
}

bool MatrixModel::removeColumns(int column, int count, const QModelIndex & parent)
{
	beginRemoveColumns(parent, column, column + count - 1);

	int new_cols = d_cols - count;
	QVector <double> d_data_new(d_rows*new_cols);
	for (int i=0; i <d_rows; i++){
		int aux1 = d_cols * i;
		int aux2 = new_cols * i;
		for (int j=0; j < column; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];

		aux1 += count;
		for (int j = column; j < new_cols; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];
	}

	d_data.resize(0);
	d_data = d_data_new;
	d_cols = new_cols;

	endRemoveColumns();
	return true;
}

bool MatrixModel::insertColumns(int column, int count, const QModelIndex & parent)
{
	beginInsertColumns(parent, column, column + count - 1);

	int new_cols = d_cols + count;
	QVector <double> d_data_new(d_rows*new_cols);
	d_data_new.fill(GSL_NAN);

	for (int i=0; i <d_rows; i++){
		int aux1 = d_cols * i;
		int aux2 = new_cols * i;
		for (int j=0; j < column; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];

		aux2 += count;
		for (int j = column; j < d_cols; j++)
			d_data_new[aux2 + j] = d_data[aux1 + j];
	}

	d_data.resize(0);
	d_data = d_data_new;
	d_cols = new_cols;

	endInsertColumns();
	return true;
}

bool MatrixModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count - 1);

	d_data.insert(row * d_cols, count*d_cols, GSL_NAN);
	d_rows += count;

	endInsertRows();
	return true;
}

bool MatrixModel::removeRows(int row, int count, const QModelIndex & parent)
{
	beginRemoveRows(parent, row, row + count - 1);

	d_data.remove(row * d_cols, count*d_cols);
	d_rows -= count;

	endRemoveRows();
	return true;
}

QString MatrixModel::saveToString()
{
	QString s = "<data>\n";
	int cols = d_cols - 1;
	for(int i = 0; i < d_rows; i++){
		int aux = d_cols*i;
		bool emptyRow = true;
		for(int j = 0; j < d_cols; j++){
			if (gsl_finite(d_data[aux + j])){
				emptyRow = false;
				break;
			}
		}
		if (emptyRow)
			continue;

		s += QString::number(i) + "\t";
		for(int j = 0; j < cols; j++){
			double val = d_data[aux + j];
			if (gsl_finite(val))
				s += QString::number(val, 'e', 16);
			s += "\t";
		}
		double val = d_data[aux + cols];
		if (gsl_finite(val))
			s += QString::number(val, 'e', 16);
		s += "\n";
	}
	return s + "</data>\n";
}

QImage MatrixModel::renderImage()
{
	QImage image(QSize(d_cols, d_rows), QImage::Format_RGB32);
	QwtLinearColorMap color_map = d_matrix->colorMap();

	double minValue = 0.0, maxValue = 0.0;
	d_matrix->range(&minValue, &maxValue);
    const QwtDoubleInterval intensityRange = QwtDoubleInterval (minValue, maxValue);
    for ( int i = 0; i < d_rows; i++ ){
    	QRgb *line = (QRgb *)image.scanLine(i);
		for ( int j = 0; j < d_cols; j++)
			*line++ = color_map.rgb(intensityRange, d_data[i*d_cols + j]);
     }
	 return image;
}
