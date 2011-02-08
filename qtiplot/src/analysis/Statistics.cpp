/***************************************************************************
	File                 : Statistics.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 - 2011 by Ion Vasilief
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
: QObject(parent),
d_col_name(QString::null),
d_result_log(true),
d_n(0),
d_data(0),
d_table(0)
{
	setData(colName);
}

bool Statistics::run()
{
	if (d_n < 0){
		QMessageBox::critical((ApplicationWindow *)parent(), tr("QtiPlot") + " - " + tr("Error"),
				tr("You didn't specify a valid data set for this operation!"));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (d_result_log)
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
	d_variance = gsl_stats_variance(d_data, 1, d_n);
	d_sd = gsl_stats_sd(d_data, 1, d_n);
	d_se = d_sd/sqrt(d_n);

	return true;
}

QString Statistics::logInfo(bool header)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QLocale l = app->locale();
	int p = app->d_decimal_digits;

	QStringList lst;
	lst << QObject::tr("Sample");
	lst << QObject::tr("N");
	lst << QObject::tr("Mean");
	lst << QObject::tr("Standard Deviation");
	lst << QObject::tr("Variance");
	lst << QObject::tr("Standard Error");
	lst << d_col_name;
	lst << QString::number(d_n);
	lst << l.toString(d_mean, 'g', p);
	lst << l.toString(d_sd, 'g', p);
	lst << l.toString(d_variance, 'g', p);
	lst << l.toString(d_se, 'g', p);

	QFontMetrics fm(app->font());
	int width = 0;
	foreach(QString s, lst){
		int aw = fm.width(s);
		if (aw > width)
			width = aw;
	}
	width += 6;

	QString s;
	QString lineSep;
	for (int i = 0; i < 6; i++){
		QString aux = lst[i];
		int spaces = ceil((double)(width - fm.width(aux))/(double)fm.width(QLatin1Char(' '))) + 1;
		s += aux + QString(spaces, QLatin1Char(' '));
		if (i == 5){
			int scores = ceil((double)fm.width(s)/(double)fm.width(QLatin1Char('-')));
			lineSep = "\n" + QString(scores, QLatin1Char('-')) + "\n";
			s += lineSep;
		}
	}

	if (!header)
		s = QString();

	for (int i = 6; i < lst.size(); i++){
		QString aux = lst[i];
		int spaces = ceil((double)(width - fm.width(aux))/(double)fm.width(QLatin1Char(' '))) + 1;
		s += aux + QString(spaces, QLatin1Char(' '));
	}

	return s + lineSep;
}

void Statistics::memoryErrorMessage()
{
	QApplication::restoreOverrideCursor();

	QMessageBox::critical((ApplicationWindow *)parent(),
	tr("QtiPlot") + " - " + tr("Memory Allocation Error"), tr("Not enough memory, operation aborted!"));
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
