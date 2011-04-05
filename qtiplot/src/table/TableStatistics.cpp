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

TableStatistics::TableStatistics(ScriptingEnv *env, ApplicationWindow *parent, Table *base, Type t, QList<int> targets, int start, int end)
	: Table(env, 1, 1, "", parent, ""),
	d_base(NULL), d_type(t), d_targets(targets), d_start(start), d_end(end)
{
	d_table->setReadOnly(true);
	setCaptionPolicy(MdiSubWindow::Both);
	if (d_start < 0)
		d_start = 0;

	QStringList header = QStringList(), comments = QStringList();
	if (d_type == row){
		resizeRows(d_targets.size());
		resizeCols(11);

		header << tr("Row") << tr("Cols") << tr("Mean") << tr("StandardDev") << tr("StandardError") << tr("Variance")
		<< tr("Sum") << tr("Max") << tr("Min") << "N" << tr("Median");

		comments << tr("Index of Row") << tr("Number of Columns") << tr("Mean") << tr("Standard Deviation") << tr("Standard Error") << tr("Variance")
		<< tr("Sum") << tr("Maximum Value") << tr("Minimum Value") << tr("Number of Points") << tr("Median");

		d_stats_col_type << Row << Cols << Mean << StandardDev << StandardError << Variance << Sum << Max << Min << N << Median;

		for (int i = 0; i < d_targets.size(); i++)
			setText(i, 0, QString::number(d_targets[i]+1));
	} else if (d_type == column){
		resizeRows(d_targets.size());
		resizeCols(13);

		header << tr("Col") << tr("Rows") << tr("Mean") << tr("StandardDev") << tr("StandardError")
		<< tr("Variance") << tr("Sum") << tr("iMax") << tr("Max") << tr("iMin") << tr("Min") << "N" << tr("Median");

		comments << tr("Column Name") << tr("Rows Included") << tr("Mean") << tr("Standard Deviation") << tr("Standard Error")
		<< tr("Variance") << tr("Sum") << tr("Index of Maximum Value") << tr("Maximum Value") << tr("Index of Minimum Value") << tr("Minimum Value") << tr("Number of Points") << tr("Median");

		d_stats_col_type << Col << Rows << Mean << StandardDev << StandardError << Variance << Sum << iMax << Max << iMin << Min << N << Median;

		setColumnType(1, Text);
	}
	int w = 9*(d_table->horizontalHeader())->sectionSize(0);
	int h = 0;
	if (numRows()>11)
		h = 11*(d_table->verticalHeader())->sectionSize(0);
	else
		h = (numRows()+1)*(d_table->verticalHeader())->sectionSize(0);
	setGeometry(50, 50, w + 45, h + 45);

	setColPlotDesignation(0, Table::X);
	setColPlotDesignation(4, Table::yErr);
	setHeader(header);
	setColComments(comments);

	setBase(base);

	connect(this, SIGNAL(removedCol(int)), this, SLOT(removeStatsCol(int)));
	connect(this, SIGNAL(colIndexChanged(int, int)), this, SLOT(changeColIndex(int, int)));
}

void TableStatistics::setBase(Table *t)
{
	if (!t)
		return;

	d_base = t;
	d_base_name = d_base->objectName();

	if (d_type == row){
		if (d_end < 0)
			d_end = d_base->numCols() - 1;

		for (int i = 0; i < d_targets.size(); i++)
			setText(i, 0, QString::number(d_targets[i]+1));

		if (d_base){
			setName(d_base_name + "-" + tr("RowStats"));
			setWindowLabel(tr("Row Statistics of %1").arg(d_base_name));
			update(d_base, QString::null);
		}
	} else if (d_type == column){
		if (d_end < 0)
			d_end = d_base->numRows() - 1;

		setName(d_base_name + "-" + tr("ColStats"));
		setWindowLabel(tr("Column Statistics of %1").arg(d_base_name));
		for (int i = 0; i < d_targets.size(); i++){
			setText(i, 0, d_base->colLabel(d_targets[i]));
			update(d_base, d_base->colName(d_targets[i]));
		}
	}

	connect(d_base, SIGNAL(modifiedData(Table*, const QString&)), this, SLOT(update(Table*, const QString&)));
	connect(d_base, SIGNAL(changedColHeader(const QString&, const QString&)), this, SLOT(renameCol(const QString&, const QString&)));
	connect(d_base, SIGNAL(removedCol(const QString&)), this, SLOT(removeCol(const QString&)));
	connect(d_base, SIGNAL(destroyed()), this, SLOT(closedBase()));
}

void TableStatistics::setColumnStatsTypes(const QList<int>& colStatTypes)
{
	setNumCols(colStatTypes.size());
	d_stats_col_type = colStatTypes;
}

void TableStatistics::changeColIndex(int fromIndex, int toIndex)
{
	d_stats_col_type.swap(fromIndex, toIndex);
}

void TableStatistics::moveColumn(int, int fromIndex, int toIndex)
{
	Table::moveColumn(0, fromIndex, toIndex);

	int to = toIndex;
	if (fromIndex < toIndex)
		to = toIndex - 1;

	d_stats_col_type.move(fromIndex, to);
}

void TableStatistics::insertCols(int start, int count)
{
	Table::insertCols(start, count);

	if (start < 0)
		start = 0;

	for(int i = 0; i<count; i++ ){
		int j = start + i;
		d_stats_col_type.insert(j, NoStats);
	}
}

void TableStatistics::addCol(PlotDesignation pd)
{
	Table::addCol(pd);
	d_stats_col_type << NoStats;
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
		for (int r = 0; r < d_targets.size(); r++){
			int i = d_targets[r];
			int m = 0;
			for (j = d_start; j <= d_end; j++){
				if (!d_base->text(i, j).isEmpty() && d_base->columnType(j) == Numeric && !d_base->isColumnHidden(j))
					m++;
			}

			if (!m){//clear row statistics
				for (j = 1; j < numCols(); j++)
					setText(r, j, QString::null);
			}

			if (m > 0){
				double *dat = new double[m];
				gsl_vector *y = gsl_vector_alloc (m);
				int aux = 0;
				for (j = d_start; j <= d_end; j++){
					QString text = d_base->text(i,j);
					if (!text.isEmpty() && d_base->columnType(j) == Numeric && !d_base->isColumnHidden(j)){
						double val = d_base->cell(i, j);
						gsl_vector_set (y, aux, val);
						dat[aux] = val;
						aux++;
					}
				}
				double mean = gsl_stats_mean (dat, 1, m);
				double sd = gsl_stats_sd(dat, 1, m);
				double min, max;
				gsl_vector_minmax (y, &min, &max);

				for (int k = 0; k < d_stats_col_type.size(); k++){
					switch (d_stats_col_type[k]){
						case Cols:
							setText(r, k, QString::number(m));
						break;
						case Mean:
							setCell(r, k, mean);
						break;
						case StandardDev:
							setCell(r, k, sd);
						break;
						case StandardError:
							setCell(r, k, sd/sqrt((double)m));
						break;
						case Variance:
							setCell(r, k, gsl_stats_variance(dat, 1, m));
						break;
						case Sum:
							setCell(r, k, mean*m);
						break;
						case Max:
							setCell(r, k, max);
						break;
						case Min:
							setCell(r, k, min);
						break;
						case N:
							setCell(r, k, m);
						break;
						default:
						break;
					}
				}
				//after everything else is done, calculate median
				for (int k = 0; k < d_stats_col_type.size(); k++){
					if (d_stats_col_type[k] != Median)
						continue;
					gsl_sort(dat, 1, m); //sort data
					double median = gsl_stats_median_from_sorted_data(dat, 1, m); //get median
					setCell(r, k, median);
					break;
				}

				gsl_vector_free (y);
				delete[] dat;
			}
		}
	} else if (d_type == column){
		for (int c = 0; c < d_targets.size(); c++){
			if (colName == QString(d_base->objectName()) + "_" + d_base->colLabel(d_targets[c])){
				int i = d_base->colIndex(colName);
				if (d_base->columnType(i) != Numeric) return;

				int start = -1, m = 0;
				for (j = d_start; j <= d_end; j++){
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
				for (j = start + 1; j <= d_end; j++){
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
				double sd = gsl_stats_sd(dat, 1, m);
				for (int k = 0; k < d_stats_col_type.size(); k++){
					switch (d_stats_col_type[k]){
						case Col:
							setText(c, k, d_base->colLabel(d_targets[c]));
						break;
						case Rows:
							setText(c, k, "[" + QString::number(start + 1) + ":" + QString::number(d_end + 1) + "]");
						break;
						case Mean:
							setCell(c, k, mean);
						break;
						case StandardDev:
							setCell(c, k, sd);
						break;
						case StandardError:
							setCell(c, k, sd/sqrt((double)m));
						break;
						case Variance:
							setCell(c, k, gsl_stats_variance(dat, 1, m));
						break;
						case Sum:
							setCell(c, k, mean*m);
						break;
						case iMax:
							setCell(c, k, max_index + 1);
						break;
						case Max:
							setCell(c, k, max);
						break;
						case iMin:
							setCell(c, k, min_index + 1);
						break;
						case Min:
							setCell(c, k, min);
						break;
						case N:
							setCell(c, k, m);
						break;
						default:
						break;
					}
				}

				//after everything else is done, calculate median
				for (int k = 0; k < d_stats_col_type.size(); k++){
					if (d_stats_col_type[k] != Median)
						continue;
					gsl_sort(dat, 1, m); //sort data
					double median = gsl_stats_median_from_sorted_data(dat, 1, m); //get median
					setCell(c, k, median);
					break;
				}

				gsl_vector_free (y);
				delete[] dat;
			}
		}
	}

for (int i = 0; i < numCols(); i++)
	emit modifiedData(this, Table::colName(i));
}

void TableStatistics::renameCol(const QString &from, const QString &to)
{
	if (!d_base || d_type == row)
		return;

	for (int c=0; c < d_targets.size(); c++)
		if (from == QString(d_base->objectName()) + "_" + text(c, 0)){
			setText(c, 0, to.section('_', 1, 1));
			return;
		}
}

void TableStatistics::removeCol(const QString &col)
{
	if (!d_base)
		return;

	if (d_type == row){
		update(d_base, col);
		return;
	}

	for (int c = 0; c < d_targets.size(); c++)
		if (col == QString(d_base->objectName()) + "_" + text(c, 0)){
			d_targets.remove(d_targets.at(c));
			d_table->removeRow(c);
			return;
		}
}

void TableStatistics::removeStatsCol(int col)
{
	if (col < d_stats_col_type.size())
		d_stats_col_type.removeAt(col);
}

void TableStatistics::setRange(int start, int end)
{
	d_start = start;
	d_end = end;
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
	if (d_start != 0 || (d_type == row && d_end != d_base->numCols() - 1) ||
		(d_type == column && d_end != d_base->numRows() - 1))
		t << "Range\t" + QString::number(d_start) + "\t" + QString::number(d_end) + "\n";
	t << "ColStatType";
	for (int i = 0; i < d_stats_col_type.size(); ++i)
		t << "\t" + QString::number(d_stats_col_type[i]);
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
