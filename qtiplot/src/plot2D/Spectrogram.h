/***************************************************************************
	File                 : Spectrogram.h
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's Spectrogram Class
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

#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include <Matrix.h>
#include <qwt_raster_data.h>
#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_plot_marker.h>

class MatrixData;
class Graph;
class PlotMarker;

class Spectrogram: public QwtPlotSpectrogram
{
public:
    Spectrogram(Graph *graph, Matrix *m);

	enum ColorMapPolicy{GrayScale, Default, Custom};

	Spectrogram* copy(Graph *g);
	Matrix * matrix(){return d_matrix;};

	int levels(){return (int)contourLevels().size() + 1;};
	void setLevelsNumber(int levels);

	bool hasColorScale();
	int colorScaleAxis(){return color_axis;};
	void showColorScale(int axis, bool on = true);

	int colorBarWidth();
	void setColorBarWidth(int width);

	void setGrayScale();
	void setDefaultColorMap();

	QwtLinearColorMap colorMap(){return color_map;};
	void setCustomColorMap(const QwtLinearColorMap& map);
	void updateData(Matrix *m);

	//! Used when saving a project file
	QString saveToString();

	ColorMapPolicy colorMapPolicy(){return color_map_policy;};

	virtual QwtDoubleRect boundingRect() const;
	void setContourLevels (const QwtValueList & levels);

	bool hasLabels(){return d_show_labels;};
	QList <PlotMarker *> labelsList(){return d_labels_list;};

	void showContourLineLabels(bool show = true);

	QFont labelsFont(){return d_labels_font;};
	void setLabelsFont(const QFont& font);

	QColor labelsColor(){return d_labels_color;};
	void setLabelsColor(const QColor& c);

	bool labelsWhiteOut(){return d_white_out_labels;};
	void setLabelsWhiteOut(bool whiteOut);

	int labelsXOffset(){return d_labels_x_offset;};
    int labelsYOffset(){return d_labels_y_offset;};
    void setLabelsOffset(int x, int y);

	double labelsRotation(){return d_labels_angle;};
    void setLabelsRotation(double angle);

    bool selectedLabels(const QPoint& pos);
    void selectLabel(bool on);
    bool hasSelectedLabels();
	void moveLabel(const QPoint& pos);

    virtual void setVisible(bool on);

protected:
	virtual void drawContourLines (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtRasterData::ContourLines &lines) const;
	void updateLabels(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtRasterData::ContourLines &lines) const;
	void createLabels();
	//! Pointer to the parent plot
	Graph *d_graph;
	//! Pointer to the source data matrix
	Matrix *d_matrix;

	//! Axis used to display the color scale
	int color_axis;

	//! Flags
	ColorMapPolicy color_map_policy;

	QwtLinearColorMap color_map;
	//! Flag telling if we display the labels
	bool d_show_labels;
	//! Labels color
	QColor d_labels_color;
	//! Labels font
	QFont d_labels_font;
	//! Flag telling if we paint a white background for the labels
	bool d_white_out_labels;
	double d_labels_angle;
	int d_labels_x_offset, d_labels_y_offset;

	//! List of the text labels associated to this spectrogram.
	QList <PlotMarker *> d_labels_list;
	//! Keeps track of the plot marker on which the user clicked when selecting the labels.
	PlotMarker *d_selected_label;
	//! Keep track of the coordinates of the point where the user clicked when selecting the labels.
	double d_click_pos_x, d_click_pos_y;
};


class MatrixData: public QwtRasterData
{
public:
    MatrixData(Matrix *m):
        QwtRasterData(m->boundingRect()),
		d_matrix(m)
    {
	n_rows = d_matrix->numRows();
	n_cols = d_matrix->numCols();

	d_m = new double* [n_rows];
	for ( int l = 0; l < n_rows; ++l)
		d_m[l] = new double [n_cols];

	for (int i = 0; i < n_rows; i++)
         for (int j = 0; j < n_cols; j++)
           d_m[i][j] = d_matrix->cell(i, j);

	m->range(&min_z, &max_z);

	x_start = d_matrix->xStart();
	dx = d_matrix->dx();
	y_start = d_matrix->yStart();
	dy = d_matrix->dy();
    }

	~MatrixData()
	{
	for (int i = 0; i < n_rows; i++)
		delete [] d_m[i];

	delete [] d_m;
	};

    virtual QwtRasterData *copy() const
    {
        return new MatrixData(d_matrix);
    }

    virtual QwtDoubleInterval range() const
    {
        return QwtDoubleInterval(min_z, max_z);
    }

	virtual QSize rasterHint (const QwtDoubleRect &) const
	{
		return QSize(n_cols, n_rows);
	}

    virtual double value(double x, double y) const;

private:
	//! Pointer to the source data matrix
	Matrix *d_matrix;

	//! Vector used to store in memory the data from the source matrix window
	double** d_m;

	//! Data size
	int n_rows, n_cols;

	//! Min and max values in the source data matrix
	double min_z, max_z;

	//! Data resolution in x(columns) and y(rows)
	double dx, dy;

	//! X axis left value in the data matrix
	double x_start;

	//! Y axis bottom value in the data matrix
	double y_start;
};

#endif
