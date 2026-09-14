/***************************************************************************
    File                 : LayerDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004-2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Arrange layers dialog

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
#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <MultiLayer.h>

#include <QDialog>

class DoubleSpinBox;
class QGroupBox;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QComboBox;

//! Arrange layers dialog
class LayerDialog : public QDialog
{
    Q_OBJECT

public:
	LayerDialog(QWidget* parent, bool okMode = false, Qt::WindowFlags fl = {});
	void setMultiLayer(MultiLayer *g);
	void setLayers(int layers);
	void setLayerCanvasSize(int w, int h, int unit);
	void setMargins(int, int, int, int);
	void setRows(int);
	void setColumns(int);
	void setSharedAxes(bool = true);

protected slots:
	void accept();
	void update();
	void enableLayoutOptions(bool ok);
	void swapLayers();
	void updateSizes(int unit);
	void adjustCanvasHeight(double width);
	void adjustCanvasWidth(double height);
	void showCommonAxesBox();

private:
	void closeEvent(QCloseEvent*);
	int convertToPixels(double w, FrameWidget::Unit unit, int dimension);
	double convertFromPixels(int w, FrameWidget::Unit unit, int dimension);

	MultiLayer *multi_layer = nullptr;

	QPushButton* buttonOk = nullptr;
	QPushButton* buttonCancel = nullptr;
	QPushButton* buttonApply = nullptr;
	QPushButton* buttonSwapLayers = nullptr;
    QGroupBox *GroupCanvasSize = nullptr, *GroupGrid = nullptr;
    QSpinBox *boxX = nullptr, *boxY = nullptr, *boxColsGap = nullptr, *boxRowsGap = nullptr;
	QSpinBox *boxRightSpace = nullptr, *boxLeftSpace = nullptr, *boxTopSpace = nullptr, *boxBottomSpace = nullptr;
	DoubleSpinBox *boxCanvasWidth = nullptr, *boxCanvasHeight = nullptr;
	QSpinBox *layersBox = nullptr;
	QSpinBox *boxLayerDest = nullptr, *boxLayerSrc = nullptr;
	QCheckBox *fitBox = nullptr;
	QComboBox *alignHorBox = nullptr, *alignVertBox = nullptr;
	QComboBox *unitBox = nullptr;
	QCheckBox *keepRatioBox = nullptr;
	QComboBox *alignPolicyBox = nullptr;
	QCheckBox *commonAxesBox = nullptr;
	QCheckBox *fixedSizeBox = nullptr;
	QCheckBox *linkXAxesBox = nullptr;

	double aspect_ratio = 0.0;
};

#endif
