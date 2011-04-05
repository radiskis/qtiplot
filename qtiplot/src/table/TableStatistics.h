/***************************************************************************
	File                 : TableStatistics.h
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke
	Email (use @ for *)  : knut.franke*gmx.de
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
#ifndef TABLE_STATISTICS_H
#define TABLE_STATISTICS_H

#include "Table.h"

/*!\brief Table that computes and displays statistics on another Table.
 *
 * \section future Future Plans
 * Make it possible to add new columns/rows to be monitored.
 */
class TableStatistics : public Table
{
	Q_OBJECT

	public:
		//! supported statistics types
		enum Type { row, column };
		enum StatisticsColType {NoStats, Row, Col, Rows, Cols, Mean, StandardDev, StandardError, Variance, Sum, iMax, Max, iMin, Min, N, Median};
		TableStatistics(ScriptingEnv *env, ApplicationWindow *parent, Table *base, Type, QList<int> targets, int start = 0, int end = -1);
		//! return the type of statistics
		Type type() const { return d_type; }
		//! return the base table of which statistics are displayed
		Table *base() const { return d_base; }
		void setBase(Table *t);
		// saving
		virtual void save(const QString&, const QString &geometry, bool = false);
		void setColumnStatsTypes(const QList<int>& colStatTypes);
		void setRange(int start, int end);

		QString baseName(){return d_base_name;}
		void setBaseName(const QString& name){d_base_name = name;}

    public slots:
        //! update statistics when the user triggers a recaculate action
        void update();
        //! update statistics after a column has changed (to be connected with Table::modifiedData)
        void update(Table*, const QString& colName);
		//! handle renaming of columns (to be connected with Table::changedColHeader)
		void renameCol(const QString&, const QString&);
		//! remove statistics of removed columns (to be connected with Table::removedCol)
		void removeCol(const QString&);

	private slots:
		void closedBase();
		void removeStatsCol(int);
		void moveColumn(int, int, int);
		void changeColIndex(int, int);
		void insertCols(int start, int count);
		void addCol(PlotDesignation pd = Y);
	
	private:
		Table *d_base;
		Type d_type;
		QList<int> d_targets;
		QList<int> d_stats_col_type;
		int d_start, d_end;
		QString d_base_name;
};

#endif
