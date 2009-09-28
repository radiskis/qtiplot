/***************************************************************************
	File                 : TableStatistics.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke, 2008 by Ion Vasilief
	Email (use @ for *)  : knut.franke*gmx.de, ion_vasilief*yahoo.fr
	Description          : Table subclass that displays statistics on
	                       columns or rows of another table

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
#include "TableStatistics.h"

#include <QList>
#include <QFile>
#include <QTextStream>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <math.h>

TableStatistics::TableStatistics(ScriptingEnv *env, ApplicationWindow *parent, Table *base, Type t, QList<int> targets)
	: Table(env, 1, 1, "", parent, ""),
	d_base(base), d_type(t), d_targets(targets)
{
    d_table->setReadOnly(true);
	setCaptionPolicy(MdiSubWindow::Both);
	if (d_type == row){
		setName(QString(d_base->objectName())+"-"+tr("RowStats"));
		setWindowLabel(tr("Row Statistics of %1").arg(base->objectName()));
		resizeRows(d_targets.size());
        resizeCols(11);
		setColName(0, tr("Row"));
		setColName(1, tr("Cols"));
		setColName(2, tr("Mean"));
        setColName(3, tr("StandardDev"));
        setColName(4, tr("StandardError"));
        setColName(5, tr("Variance"));
        setColName(6, tr("Sum"));
        setColName(7, tr("Max"));
        setColName(8, tr("Min"));
        setColName(9, "N");
        setColName(10, tr("Median"));

		for (int i=0; i < d_targets.size(); i++)
            setText(i, 0, QString::number(d_targets[i]+1));

		update(d_base, QString::null);
    } else if (d_type == column) {
            setName(QString(d_base->objectName())+"-"+tr("ColStats"));
            setWindowLabel(tr("Column Statistics of %1").arg(base->objectName()));
            resizeRows(d_targets.size());
            resizeCols(13);
            setColName(0, tr("Col"));
            setColName(1, tr("Rows"));
            setColName(2, tr("Mean"));
            setColName(3, tr("StandardDev"));
            setColName(4, tr("StandardError"));

            setColName(5, tr("Variance"));
            setColName(6, tr("Sum"));
            setColName(7, tr("iMax"));
            setColName(8, tr("Max"));
            setColName(9, tr("iMin"));
            setColName(10, tr("Min"));
            setColName(11, "N");
            setColName(12, tr("Median"));

            setColumnType(1, Text);

		for (int i=0; i < d_targets.size(); i++){
            setText(i, 0, d_base->colLabel(d_targets[i]));
            update(d_base, d_base->colName(d_targets[i]));
		}
	}
    int w = 9*(d_table->horizontalHeader())->sectionSize(0);
	int h;
	if (numRows()>11)
        h = 11*(d_table->verticalHeader())->sectionSize(0);
	else
        h = (numRows()+1)*(d_table->verticalHeader())->sectionSize(0);
	setGeometry(50,50,w + 45, h + 45);

    setColPlotDesignation(0, Table::X);
    setColPlotDesignation(4, Table::yErr);
	setHeaderColType();

    connect(d_base, SIGNAL(modifiedData(Table*, const QString&)), this, SLOT(update(Table*, const QString&)));
	connect(d_base, SIGNAL(changedColHeader(const QString&, const QString&)), this, SLOT(renameCol(const QString&, const QString&)));
	connect(d_base, SIGNAL(removedCol(const QString&)), this, SLOT(removeCol(const QString&)));
	connect(d_base, SIGNAL(destroyed()), this, SLOT(closedBase()));
}

void TableStatistics::closedBase()
{
	d_base = NULL;
}

void TableStatistics::update()
{
    if (!d_base)
        return;

    for (int i = 0; i < d_base->numCols (); i++)
        update(d_base, d_base->colName(i));
}

void TableStatistics::update(Table *t, const QString& colName)
{
        if (!d_base || t != d_base)
            return;

	int j;
        if (d_type == row){
            if (numCols () < 11){ //modified columns structure
                d_base = NULL;
                return;
            }

            for (int r=0; r < d_targets.size(); r++){
                int cols = d_base->numCols();
                int i = d_targets[r];
                int m = 0;
                for (j = 0; j < cols; j++)
                        if (!d_base->text(i, j).isEmpty() && d_base->columnType(j) == Numeric)
                                m++;

                if (!m){//clear row statistics
                    for (j = 1; j < numCols(); j++)
                        setText(r, j, QString::null);
                }

                if (m > 0){
                    double *dat = new double[m];
                    gsl_vector *y = gsl_vector_alloc (m);
                    int aux = 0;
                    for (j = 0; j<cols; j++){
                        QString text = d_base->text(i,j);
                        if (!text.isEmpty() && d_base->columnType(j) == Numeric){
                            double val = d_base->cell(i, j);
                            gsl_vector_set (y, aux, val);
                            dat[aux] = val;
                            aux++;
                        }
                    }
                    double mean = gsl_stats_mean (dat, 1, m);
                    double min, max;
                    gsl_vector_minmax (y, &min, &max);

                    setText(r, 1, QString::number(d_base->numCols()));
                    setCell(r, 2, mean);
                    double sd = gsl_stats_sd(dat, 1, m);
                    setCell(r, 3, sd);
                    setCell(r, 4, sd/sqrt((double)m));
                    setCell(r, 5, gsl_stats_variance(dat, 1, m));
                    setCell(r, 5, mean*m);
                    setCell(r, 7, max);
                    setCell(r, 8, min);
                    setText(r, 9, QString::number(m));

                    //after everything else is done, calculate median
                    gsl_sort(dat,1,m); //sort data
                    double median = gsl_stats_median_from_sorted_data(dat,1,m); //get median
                    setCell(r, 10, median);

                    gsl_vector_free (y);
                    delete[] dat;
                }
            }
        } else if (d_type == column){
            if (numCols () < 13){ //modified columns structure
                d_base = NULL;
                return;
            }
            for (int c = 0; c < d_targets.size(); c++){
                if (colName == QString(d_base->objectName()) + "_" + text(c, 0)){
                    int i = d_base->colIndex(colName);
                    if (d_base->columnType(i) != Numeric) return;

                    int rows = d_base->numRows();
                    int start = -1, m = 0;
                    for (j=0; j<rows; j++){
                        if (!d_base->text(j, i).isEmpty()){
                            m++;
                            if (start < 0) start = j;
                        }
                    }

                    if (!m){//clear col statistics
                        for (j = 1; j<numCols(); j++)
                            setText(c, j, QString::null);
                        return;
                    }

                    if (start < 0)
                        return;

                    double *dat = new double[m];
                    gsl_vector *y = gsl_vector_alloc (m);

                    int aux = 0, min_index = start, max_index = start;
                    double val = d_base->cell(start, i);
                    gsl_vector_set (y, 0, val);
                    dat[0] = val;
                    double min = val, max = val;
                    for (j = start + 1; j<rows; j++){
                        if (!d_base->text(j, i).isEmpty()){
                            aux++;
                            val = d_base->cell(j, i);
                            gsl_vector_set (y, aux, val);
                            dat[aux] = val;
                            if (val < min){
                                min = val;
                                min_index = j;
                            }
                            if (val > max){
                                max = val;
                                max_index = j;
                            }
                        }
                    }
                    double mean = gsl_stats_mean (dat, 1, m);

                    setText(c, 1, "[1:"+QString::number(rows)+"]");
                    setCell(c, 2, mean);
                    double sd = gsl_stats_sd(dat, 1, m);
                    setCell(c, 3, sd);
                    setCell(c, 4, sd/sqrt((double)m));
                    setCell(c, 5, gsl_stats_variance(dat, 1, m));
                    setCell(c, 6, mean*m);
                    setText(c, 7, QString::number(max_index + 1));
                    setCell(c, 8, max);
                    setText(c, 9, QString::number(min_index + 1));
                    setCell(c, 10, min);
                    setText(c, 11, QString::number(m));
                    //after everything else is done, calculate median
                    gsl_sort(dat,1,m); //sort data
                    double median = gsl_stats_median_from_sorted_data(dat,1,m); //get median
                    setCell(c, 12, median);

                    gsl_vector_free (y);
                    delete[] dat;
                }
            }
        }

	for (int i=0; i<numCols(); i++)
		emit modifiedData(this, Table::colName(i));
}

void TableStatistics::renameCol(const QString &from, const QString &to)
{
	if (!d_base)
		return;

	if (d_type == row) return;
	for (int c=0; c < d_targets.size(); c++)
		if (from == QString(d_base->objectName())+"_"+text(c, 0))
		{
			setText(c, 0, to.section('_', 1, 1));
			return;
		}
}

void TableStatistics::removeCol(const QString &col)
{
	if (!d_base)
		return;

	if (d_type == row)
	{
		update(d_base, col);
		return;
	}
	for (int c=0; c < d_targets.size(); c++)
		if (col == QString(d_base->objectName())+"_"+text(c, 0))
		{
			d_targets.remove(d_targets.at(c));
			d_table->removeRow(c);
			return;
		}
}

void TableStatistics::save(const QString& fn, const QString &geometry, bool)
{
	if (!d_base){
		Table::save(fn, geometry, false);
		return;
	}

	QFile f(fn);
	if (!f.isOpen()){
		if (!f.open(QIODevice::Append))
			return;
	}

	QTextStream t( &f );
	t.setEncoding(QTextStream::UnicodeUTF8);
	t << "<TableStatistics>\n";
	t << QString(objectName())+"\t";
	t << QString(d_base->objectName()) + "\t";
	t << QString(d_type == row ? "row" : "col") + "\t";
	t << birthDate()+"\n";
	t << "Targets";
	for (QList<int>::iterator i=d_targets.begin(); i!=d_targets.end(); ++i)
		t << "\t" + QString::number(*i);
	t << "\n";
	t << geometry;
	t << saveHeader();
	t << saveColumnWidths();
	t << saveCommands();
	t << saveColumnTypes();
	t << saveReadOnlyInfo();
	t << saveHiddenColumnsInfo();
	t << saveComments();
	t << "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	t << "</TableStatistics>\n";
	f.close();
}
