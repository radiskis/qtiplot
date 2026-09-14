/***************************************************************************
    File                 : BoxCurve.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Box curve

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
#ifndef BOXCURVE_H
#define BOXCURVE_H

#include "PlotCurve.h"
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <vector>

//! Box curve
class BoxCurve: public DataCurve
{
public:
	enum BoxStyle{NoBox, Rect, Diamond, WindBox, Notch};
	enum Range{None, SD, SE, r25_75, r10_90, r5_95, r1_99, MinMax, UserDef};
	enum LabelsDisplayPolicy{Percentage, Value, PercentageValue, ValuePercentage};

	BoxCurve(Table *t, const QString& name, int startRow = 0, int endRow = -1);

	void copy(BoxCurve *b);

	QRectF boundingRect() const override;

	QwtSymbol::Style minStyle(){return min_style;};
	void setMinStyle(QwtSymbol::Style s){min_style = s;};

	QwtSymbol::Style maxStyle(){return max_style;};
	void setMaxStyle(QwtSymbol::Style s){max_style = s;};

	void setMeanStyle(QwtSymbol::Style s){mean_style = s;};
	QwtSymbol::Style meanStyle(){return mean_style;};

	void setP99Style(QwtSymbol::Style s){p99_style = s;};
	QwtSymbol::Style p99Style(){return p99_style;};

	void setP1Style(QwtSymbol::Style s){p1_style = s;};
	QwtSymbol::Style p1Style(){return p1_style;};

	int boxStyle(){return b_style;};
	void setBoxStyle(int style);

	int boxWidth(){return b_width;};
	void setBoxWidth(int width){b_width=width;};

	double boxRange(){return b_coeff;};
	int boxRangeType(){return b_range;};
	void setBoxRange(int type, double coeff);

	double whiskersRange(){return w_coeff;};
	int whiskersRangeType(){return w_range;};
	void setWhiskersRange(int type, double coeff = 0.0);

	LabelsDisplayPolicy labelsDisplayPolicy() const {return d_labels_display;};
	void setLabelsDisplayPolicy(const LabelsDisplayPolicy& policy);

	bool hasBoxLabels() const {return d_box_labels;};
	void showBoxLabels(bool on = true);

	bool hasWhiskerLabels() const {return d_whiskers_labels;};
	void showWhiskerLabels(bool on = true);

    void loadData() override;

	QString statistics();

	double median(){return quantile(0.5);};
	double quantile(double f);

private:
	void draw(QPainter *painter,const QwtScaleMap &xMap,
		const QwtScaleMap &yMap, int from, int to) const;
	void drawBox(QPainter *painter, const QwtScaleMap &xMap,
				const QwtScaleMap &yMap, double *dat, int size) const;
	void drawSymbols(QPainter *painter, const QwtScaleMap &xMap,
				const QwtScaleMap &yMap, double *dat, int size) const;

	std::vector<double> statisticValues();
	QString labelText(int index, double val);
	QString labelPercentage(int index);
	void createLabel(double val);
	void loadLabels() override;
	void updateLabels(bool updateText = true);
	void updateLabelsPosition(){updateLabels(false);};

	QwtSymbol::Style min_style = QwtSymbol::NoSymbol;
	QwtSymbol::Style max_style = QwtSymbol::NoSymbol;
	QwtSymbol::Style mean_style = QwtSymbol::NoSymbol;
	QwtSymbol::Style p99_style = QwtSymbol::NoSymbol;
	QwtSymbol::Style p1_style = QwtSymbol::NoSymbol;
	double b_coeff = 0.0;
	double w_coeff = 0.0;
	int b_range = None;
	int w_range = None;
	int b_style = Rect;
	int b_width = 0;
	bool d_box_labels = false;
	bool d_whiskers_labels = false;
	LabelsDisplayPolicy d_labels_display = Percentage;
};


//! Single array data (extension to QwtSeriesData)
class QwtSingleArrayData: public QwtSeriesData<QPointF>
{
public:
    QwtSingleArrayData(const double x, const QVector<double>& y, size_t)
	{
		d_y = y;
		d_x = x;
	};

    size_t size() const override {return d_y.size();};
    QPointF sample(size_t i) const override {return QPointF(d_x, d_y[int(i)]);};
    QRectF boundingRect() const override {return qwtBoundingRect(*this);};

private:
    QVector<double> d_y;
	double d_x = 0.0;
};

#endif
