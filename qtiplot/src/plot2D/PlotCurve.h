/***************************************************************************
    File                 : PlotCurve.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : AbstractPlotCurve and DataCurve classes

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
#ifndef PLOTCURVE_H
#define PLOTCURVE_H

#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <Table.h>

class PlotMarker;
class Table;
class ErrorBarsCurve;

//! Abstract 2D plot curve class
class PlotCurve: public QwtPlotCurve
{

public:
    PlotCurve(const QString& name = QString());

	//! Sort of rtti()
	int type(){return d_type;};
	void setType(int t){d_type = t;};

	/*!\brief The plot style of the curve.
	 * \sa Graph::CurveType
	 */
	int plotStyle(){return d_plot_style;};
	void setPlotStyle(int s){d_plot_style = s;};

	double xOffset(){return d_x_offset;};
	void setXOffset(double dx){d_x_offset = dx;};

	double yOffset(){return d_y_offset;};
	void setYOffset(double dy){d_y_offset = dy;};

	bool sideLinesEnabled(){return d_side_lines;};
	void enableSideLines(bool on){d_side_lines = on;};

	QString saveCurveSymbolImage();
	QString saveCurveLayout();
	void restoreCurveLayout(const QStringList& lst);

	//! Set the number of symbols not to be drawn: usefull for large data sets
	void setSkipSymbolsCount(int count);
	//! Returns the number of symbols not to be drawn
	int skipSymbolsCount(){return d_skip_symbols;};

protected:
	virtual void drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;
	void drawSideLines(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;

    virtual void drawSymbols(QPainter *p, const QwtSymbol &,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;

	void drawSticks(QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;

	QwtDoubleRect boundingRect() const;

	// Rtti
	int d_type;
	// The plot style of the curve
	int d_plot_style;
	double d_x_offset, d_y_offset;
	bool d_side_lines;
	int d_skip_symbols;
};

class DataCurve: public PlotCurve
{

public:

    struct DataRange
    {
            unsigned int from;
            unsigned int to;
    };

	DataCurve(Table *t, const QString& xColName, const QString& name, int startRow = 0, int endRow = -1);
	DataCurve(Table *xt, const QString& xColName, Table *yt, const QString& name, int startRow = 0, int endRow = -1);
    void clone(DataCurve* c);

    virtual QString saveToString();
    void restoreLabels(const QStringList& lst);

	QString xColumnName(){return d_x_column;};
	void setXColumnName(const QString& name){d_x_column = name;};

	bool hasLabels(){return !d_labels_list.isEmpty();};
	QString labelsColumnName(){return d_labels_column;};
	void setLabelsColumnName(const QString& name);

    int labelsAlignment(){return d_labels_align;};
    void setLabelsAlignment(int flags);

    int labelsXOffset(){return d_labels_x_offset;};
    int labelsYOffset(){return d_labels_y_offset;};
    void setLabelsOffset(int x, int y);

    double labelsRotation(){return d_labels_angle;};
    void setLabelsRotation(double angle);

    QFont labelsFont(){return d_labels_font;};
    void setLabelsFont(const QFont& font);

    QColor labelsColor(){return d_labels_color;};
    void setLabelsColor(const QColor& c);

    bool labelsWhiteOut(){return d_white_out_labels;};
    void setLabelsWhiteOut(bool whiteOut = true);

	Table* table(){return d_table;};
	Table* xTable(){return d_x_table;};
	void setDataSource(Table *yt, int ycol = 1, Table *xt = NULL, int xcol = 0);

	int startRow(){return d_start_row;};
	int endRow(){return d_end_row;};
	void setRowRange(int startRow, int endRow);

	bool isFullRange();
	void setFullRange();

	virtual bool updateData(Table *t, const QString& colName);
	virtual void loadData();
	void enableSpeedMode();

	//! Returns the row index in the data source table corresponding to the data point index.
	int tableRow(int point);

	void remove();

        /**
		 * \brief A list of data sources for this curve.
		 *
		 * Elements must be in either of the following forms:
		 *  - &lt;id of X column> "(X)," &lt;id of Y column> "(Y)" [ "," &lt;id of error column> ("(xErr)" | "(yErr)") ]
		 *  - &lt;id of Xstart column> "(X)," &lt;id of Ystart column>"(Y)," &lt;id of Xend column> "(X)," &lt;id of Yend column> "(Y)"\n
		 *    (denoting start and end coordinates for the #VectXYXY style)
		 *  - &lt;id of Xstart column> "(X)," &lt;id of Ystart column> "(Y)," &lt;id of angle column> "(A)," &lt;id of magnitude column> "(M)"\n
		 *    (denoting start coordinates, angle in radians and length for the #VectXYAM style)
		 *
		 * Column ids are of the form '&lt;name of table> "_" &lt;name of column>'.
		 */
	virtual QStringList plotAssociation();
	virtual void updateColumnNames(const QString& oldName, const QString& newName, bool updateTableName);

	//! The list of attached error bars.
	QList<ErrorBarsCurve *> errorBarsList(){return d_error_bars;};
	//! Adds a single error bars curve to the list of attached error bars.
	void addErrorBars(ErrorBarsCurve *c){if (c) d_error_bars << c;};
	//! Remove a single error bars curve from the list of attached error bars.
	void removeErrorBars(ErrorBarsCurve *c);
	//! Clears the list of attached error bars.
	void clearErrorBars();
	//! Clears the list of attached text labels.
	void clearLabels();

	void setVisible(bool on);

	void setVisibleLabels(bool on = true){d_show_labels = on;};
	bool hasVisibleLabels(){return d_show_labels;};
	bool selectedLabels(const QPoint& pos);
	bool hasSelectedLabels();
	void setLabelsSelected(bool on = true);

	void moveLabels(const QPoint& pos);
	virtual void updateLabelsPosition();

protected:
        virtual void drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;

	bool validCurveType();
	virtual void loadLabels();

	//! List of the error bar curves associated to this curve.
	QList <ErrorBarsCurve *> d_error_bars;
	//! The data source table.
	Table *d_table;
	//! The data source table in case X and Y datasets are from different tables.
	Table *d_x_table;
	//!\brief The name of the column used for abscissae values.
	/*
	 *The column name used for Y values is stored in title().text().
	 */
	QString d_x_column;

	int d_start_row;
	int d_end_row;

	//!\brief The name of the column used for text labels.
	QString d_labels_column;

	//! List of the text labels associated to this curve.
	QList <PlotMarker *> d_labels_list;
	//! Keep track of the coordinates of the point where the user clicked when selecting the labels.
	double d_click_pos_x, d_click_pos_y;

	QColor d_labels_color;
	QFont d_labels_font;
	double d_labels_angle;
	bool d_white_out_labels;
	bool d_show_labels;
	int d_labels_align, d_labels_x_offset, d_labels_y_offset;
	//! Keeps track of the plot marker on which the user clicked when selecting the labels.
	PlotMarker *d_selected_label;
	std::vector<DataRange> d_data_ranges;
};

class PlotMarker: public QwtPlotMarker
{
public:
	PlotMarker(int index, double angle);

	int index(){return d_index;};
	void setIndex(int i){d_index = i;};

	double angle(){return d_angle;};
	void setAngle(double a){d_angle = a;};

	double xLabelOffset(){return d_label_x_offset;};
	double yLabelOffset(){return d_label_y_offset;};
	void setLabelOffset(double xOffset, double yOffset){d_label_x_offset = xOffset; d_label_y_offset = yOffset;};

protected:
	//! Does the actual drawing; see QwtPlotItem::draw.
	void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect &r) const;

	int d_index;
	double d_angle;
	//! x coordinate offset
	double d_label_x_offset;
	//! y coordinate offset
	double d_label_y_offset;
};
#endif
