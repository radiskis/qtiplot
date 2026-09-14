/***************************************************************************
    File                 : QwtHistogram.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Histogram class

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
#include "QwtBarCurve.h"

class Matrix;

//! Histogram class
class QwtHistogram: public QwtBarCurve
{
public:
	QwtHistogram(Table *t, const QString& name, int startRow = 0, int endRow = -1);
	QwtHistogram(Matrix *m);

	void copy(QwtHistogram *h);

	QRectF boundingRect() const override;

	void setBinning(bool autoBin, double size, double begin, double end);
	//! Convenience function. It disables autobinning
	void setBinning(double binSize, double begin, double end);

	bool autoBinning(){return d_autoBin;};
	//! Convenience function to be used in scripts
	void setAutoBinning(bool autoBin = true);

	double begin(){return d_begin;};
	double end(){return d_end;};
	double binSize(){return d_bin_size;};

	void loadData() override;

	double mean(){return d_mean;};
	double standardDeviation(){return d_standard_deviation;};
	double minimum(){return d_min;};
	double maximum(){return d_max;};

	Matrix* matrix(){return d_matrix;};

private:
	void init();

	void loadDataFromMatrix();
	void loadLabels() override;

	Matrix *d_matrix = nullptr;

	bool d_autoBin = true;
	double d_bin_size = 1.0, d_begin = 0.0, d_end = 0.0;

	//! Variables storing statistical information
	double d_mean = 0.0, d_standard_deviation = 0.0, d_min = 0.0, d_max = 0.0;
};
