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

#include <QApplication>
#include <Matrix.h>
#include <muParserScript.h>
#include <LinearColorMap.h>

#include <qwt_raster_data.h>
#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
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
	Graph * graph(){return d_graph;};

	Matrix * matrix(){return d_matrix;};
	bool setMatrix(Matrix *m, bool useFormula = false);

	int levels(){return (int)contourLevels().size();};
	void setLevelsNumber(int levels);
	void setContourLevels (const QwtValueList & levels);

	bool hasColorScale();
	int colorScaleAxis(){return color_axis;};
	void setColorScaleAxis(int axis){color_axis = axis;};
	void showColorScale(int axis, bool on = true);

	int colorBarWidth();
	void setColorBarWidth(int width);

	void setGrayScale();
	void setDefaultColorMap();

	LinearColorMap colorMap(){return color_map;};
	LinearColorMap *colorMapPointer(){return &color_map;};
	void setCustomColorMap(const LinearColorMap& map);

	//! Used when saving a project file
	QString saveToString();

	ColorMapPolicy colorMapPolicy(){return color_map_policy;};

	virtual QwtDoubleRect boundingRect() const;

	bool hasLabels(){return d_show_labels;};
	QList <PlotMarker *> labelsList(){return d_labels_list;};

	void showContourLineLabels(bool show = true);

	QFont labelsFont(){return d_labels_font;};
	void setLabelsFont(const QFont& font);

	QColor labelsColor(){return d_labels_color;};
	void setLabelsColor(const QColor& c);

	bool labelsWhiteOut(){return d_white_out_labels;};
	void setLabelsWhiteOut(bool whiteOut);

	double labelsXOffset(){return d_labels_x_offset;};
    double labelsYOffset(){return d_labels_y_offset;};
    void setLabelsOffset(double x, double y);
	void setLabelOffset(int index, double x, double y);

	double labelsRotation(){return d_labels_angle;};
    void setLabelsRotation(double angle);

    bool selectedLabels(const QPoint& pos);
    void selectLabel(bool on);
    bool hasSelectedLabels();
	void moveLabel(const QPoint& pos);
	void clearLabels();

    virtual void setVisible(bool on);
    virtual QPen contourPen (double level) const;
    void setColorMapPen(bool on = true);
    bool useColorMapPen(){return d_color_map_pen;};

    QList<QPen> contourPenList(){return d_pen_list;};
    void setContourPenList(QList<QPen> lst);

    void setContourLinePen(int index, const QPen &pen);

    bool useMatrixFormula(){return d_use_matrix_formula;};
    bool setUseMatrixFormula(bool on = true);

    void updateData();
	QwtDoubleInterval range() const;

protected:
	virtual QImage renderImage(const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtDoubleRect &rect) const;
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

	LinearColorMap color_map;
	//! Flag telling if we display the labels
	bool d_show_labels;
	//! Labels color
	QColor d_labels_color;
	//! Labels font
	QFont d_labels_font;
	//! Flag telling if we paint a white background for the labels
	bool d_white_out_labels;
	double d_labels_angle;
	double d_labels_x_offset, d_labels_y_offset;

	//! List of the text labels associated to this spectrogram.
	QList <PlotMarker *> d_labels_list;
	//! Keeps track of the plot marker on which the user clicked when selecting the labels.
	PlotMarker *d_selected_label;
	//! Keep track of the coordinates of the point where the user clicked when selecting the labels.
	double d_click_pos_x, d_click_pos_y;

	//! Flag telling that we evaluate the matrix expression instead of using the matrix data.
	bool d_use_matrix_formula;

	//! Flag telling if we use the color map to calculate the pen (QwtPlotSpectrogram::contourPen()).
	bool d_color_map_pen;

	QList<QPen> d_pen_list;
};

class MatrixData: public QwtRasterData
{
public:
    MatrixData(Matrix *m, bool useMatrixFormula = false):
        QwtRasterData(m->boundingRect()),
		d_matrix(m)
    {
		n_rows = d_matrix->numRows();
		n_cols = d_matrix->numCols();

		x_start = d_matrix->xStart();
		dx = d_matrix->dx();
		y_start = d_matrix->yStart();
		dy = d_matrix->dy();

		d_mup = NULL;
		if (useMatrixFormula && d_matrix->canCalculate()){
			d_mup = new muParserScript(d_matrix->scriptingEnv(), d_matrix->formula(),
					d_matrix, QString("<%1>").arg(d_matrix->objectName()));

			d_x = d_mup->defineVariable("x");
			d_y = d_mup->defineVariable("y");
			d_ri = d_mup->defineVariable("i");
			d_rr = d_mup->defineVariable("row");
			d_cj = d_mup->defineVariable("j");
			d_cc = d_mup->defineVariable("col");

			if (!d_mup->compile()){
				delete d_mup;
				d_mup = NULL;
			}

			if (d_mup){//calculate z range
				*d_ri = 1.0;
				*d_rr = 1.0;
				*d_y = y_start;
				*d_cj = 1.0;
				*d_cc = 1.0;
				*d_x = x_start;

				if (d_mup->codeLines() == 1)
					min_z = d_mup->evalSingleLine();
				else
					min_z = d_mup->eval().toDouble();

				max_z = min_z;

				if (d_mup->codeLines() == 1){
					for(int row = 0; row < n_rows; row++){
						double r = row + 1.0;
						*d_ri = r; *d_rr = r;
						*d_y = y_start + row*dy;
						for(int col = 0; col < n_cols; col++){
							double c = col + 1.0;
							*d_cj = c; *d_cc = c;
							*d_x = x_start + col*dx;
							double aux = d_mup->evalSingleLine();
							if (aux <= min_z)
								min_z = aux;
							if (aux >= max_z)
								max_z = aux;
						}
					}
				} else {
					for(int row = 0; row < n_rows; row++){
						double r = row + 1.0;
						*d_ri = r; *d_rr = r;
						*d_y = y_start + row*dy;
						for(int col = 0; col < n_cols; col++){
							double c = col + 1.0;
							*d_cj = c; *d_cc = c;
							*d_x = x_start + col*dx;
							double aux = d_mup->eval().toDouble();
							if (aux <= min_z)
								min_z = aux;
							if (aux >= max_z)
								max_z = aux;
						}
						qApp->processEvents();
					}
				}
			}
		} else
			m->range(&min_z, &max_z);
    }

	~MatrixData()
	{
		if (d_mup)
			delete d_mup;
	}

    virtual QwtRasterData *copy() const
    {
    	if (d_mup)
			return new MatrixData(d_matrix, true);

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

	//! Pointer to a muParserScript
	muParserScript *d_mup;
	//! Pointers to internal variables of the muParserScript
	double *d_x, *d_y, *d_ri, *d_rr, *d_cj, *d_cc;
};

#endif
