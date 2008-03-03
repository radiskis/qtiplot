/***************************************************************************
	File                 : MatrixModel.h
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

#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QLocale>

class Matrix;

class MatrixModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MatrixModel(int rows = 32, int cols = 32, QObject *parent = 0);
    MatrixModel(const QImage& image, QObject *parent);
    MatrixModel* copy();

    Matrix *matrix(){return d_matrix;};

	Qt::ItemFlags flags( const QModelIndex & index ) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
	void setRowCount(int rows);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	void setColumnCount(int cols);

	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
	bool removeColumns ( int column, int count, const QModelIndex & parent = QModelIndex());
	bool insertColumns ( int column, int count, const QModelIndex & parent = QModelIndex());

	double x(int col) const;
	double y(int row) const;

	double cell(int row, int col);
	void setCell(int row, int col, double val);

	QString text(int row, int col);
	void setText(int row, int col, const QString&);

	QString saveToString();
	QImage renderImage();

	double data(int row, int col) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex & index, const QVariant & value, int role);

    double* dataVector();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	void setDataVector(const QVector<double>& data);

	bool importASCII(const QString &fname, const QString &sep, int ignoredLines, bool stripSpaces,
					bool simplifySpaces, const QString& commentString, int importAs,
					const QLocale& locale, int endLineChar = 0, int maxRows = -1);

	void setLocale(const QLocale& locale){d_locale = locale;};
	void setNumericFormat(char f, int prec);

private:
    int d_rows, d_cols;
    QVector<double> d_data;
	Matrix *d_matrix;
	//! Format code for displaying numbers
	char d_txt_format;
	//! Number of significant digits
	int d_num_precision;
	//! Locale used to display data
	QLocale d_locale;
};

#endif
