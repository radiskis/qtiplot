/***************************************************************************
	File                 : GriddingDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Random XYZ gridding options dialog

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
#ifndef GRIDDINGDIALOG_H
#define GRIDDINGDIALOG_H

#include <qwt3d_curve.h>
#include <qwt3d_surfaceplot.h>

#include <QDialog>
#include <ap.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSpinBox;
class Table;
class DoubleSpinBox;

using namespace Qwt3D;

//! Random XYZ gridding options dialog
class GriddingDialog : public QDialog
{
    Q_OBJECT

public:
	GriddingDialog(Table* t, const QString& colName, int nodes, QWidget* parent = 0, Qt::WFlags fl = 0 );
	~GriddingDialog();

private slots:
	void accept();
	void preview();
	void setPlotStyle(int);
	void showMethodParameters(int);

private:
	void loadDataFromTable();
	void resetAxesLabels();
	void findBestLayout();

	Table *d_table;
	Qwt3D::Plot3D* sp;

	QString d_col_name;
	int d_nodes;
	ap::real_2d_array xy;

	QPushButton* buttonFit;
	QPushButton* buttonCancel;
	QLabel* boxName;
	QComboBox* boxMethod;
	QSpinBox* boxCols, *boxRows;
	DoubleSpinBox* boxXStart;
	DoubleSpinBox* boxXEnd;
	DoubleSpinBox* boxYStart;
	DoubleSpinBox* boxYEnd;
	DoubleSpinBox* boxRadius;
	QComboBox* boxPlotStyle;
	QCheckBox *showPlotBox;
	QGroupBox *previewBox;
	QGroupBox *gbRadius;
	QGroupBox *gbModel;
	QComboBox* boxModel;
	QSpinBox* boxNQ;
	QSpinBox* boxNW;
};

#endif
