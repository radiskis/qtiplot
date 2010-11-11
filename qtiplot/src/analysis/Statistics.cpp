/***************************************************************************
	File                 : Statistics.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Abstract base class for statistics data analysis

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
#include "Statistics.h"
#include <Table.h>

#include <QApplication>
#include <QMessageBox>
#include <QLocale>

#include <gsl/gsl_statistics.h>

Statistics::Statistics(ApplicationWindow *parent, const QString& colName)
: QObject(parent)
{
	init();
	setData(colName);
}

void Statistics::init()
{
	d_n = 0;
	d_data = NULL;
	d_table = NULL;
	d_col_name = QString::null;
}

bool Statistics::run()
{
	if (d_n < 0){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("You didn't specify a valid data set for this operation!"));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	((ApplicationWindow *)parent())->updateLog(logInfo());
	QApplication::restoreOverrideCursor();
	return true;
}

bool Statistics::setData(const QString& colName)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return false;

	d_table = app->table(colName);
	if (!d_table)
		return false;

	int col = d_table->colIndex(colName);
	if (col < 0){
		QMessageBox::information((ApplicationWindow *)parent(), QObject::tr("Attention!"),
				tr("There is no sample dataset called %1 in this project.").arg(colName));
		return false;
	}
	d_col_name = colName;

	unsigned int rows = d_table->numRows();
	if (!rows)
		return false;

	if (d_n > 0)//delete previousely allocated memory
		freeMemory();

	for (unsigned int i = 0; i < rows; i++){
		if (!d_table->text(i, col).isEmpty())
			d_n++;
	}
	if (!d_n){
		QMessageBox::information((ApplicationWindow *)parent(), QObject::tr("Attention!"),
		QObject::tr("The sample dataset (%1) must have at least one data point.").arg(colName));
		return false;
	}

	d_data = (double *)malloc(d_n*sizeof(double));
	if (!d_data){
		memoryErrorMessage();
		return false;
	}

	int aux = 0;
	for (unsigned int i = 0; i < rows; i++){
		if (!d_table->text(i, col).isEmpty()){
			d_data[aux] = d_table->cell(i, col);
			aux++;
		}
	}
	d_mean = gsl_stats_mean (d_data, 1, d_n);
	d_sd = gsl_stats_sd(d_data, 1, d_n);
	d_se = d_sd/sqrt(d_n);

	return true;
}

QString Statistics::logInfo()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;
	QString sep = "\t";
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";

	QString s = QObject::tr("Sample") + sep + QObject::tr("N") + sep + QObject::tr("Mean") + sep;
	s += QObject::tr("Standard Deviation") + sep + QObject::tr("Standard Error") + "\n";
	s += sep1;
	s += d_col_name + sep + QString::number(d_n) + sep + l.toString(d_mean, 'g', p) + sep;
	s += l.toString(d_sd, 'g', p) + "\t\t" + l.toString(d_se, 'g', p) + "\n";
	return s;
}

void Statistics::memoryErrorMessage()
{
	QApplication::restoreOverrideCursor();

	QMessageBox::critical((ApplicationWindow *)parent(),
		tr("QtiPlot") + " - " + tr("Memory Allocation Error"),
		tr("Not enough memory, operation aborted!"));
}

void Statistics::freeMemory()
{
	if (d_data && d_n > 0) {
		free(d_data);
		d_data = NULL;
		d_n = 0;
	}
}

Statistics::~Statistics()
{
	freeMemory();
}
