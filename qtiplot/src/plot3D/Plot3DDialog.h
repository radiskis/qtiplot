/***************************************************************************
    File                 : Plot3DDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Surface plot options dialog

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
#ifndef PLOT3DDIALOG_H
#define PLOT3DDIALOG_H

#include "Graph3D.h"
#include <QCheckBox>

class QGroupBox;
class QComboBox;
class QLabel;
class QTextEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTabWidget;
class QWidget;
#include <QStringList>
class QStackedWidget;
class QDoubleSpinBox;
class ColorButton;
class TextFormatButtons;
class DoubleSpinBox;
class ColorMapEditor;

using namespace Qwt3D;

//! Surface plot options dialog
class Plot3DDialog : public QDialog
{
    Q_OBJECT

public:
    Plot3DDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );
	void setPlot(Graph3D *);

	void showTitleTab();
	void showAxisTab();
	void showGeneralTab();

private slots:
	void accept() override;
	bool updatePlot();

	void pickTitleFont();
	void viewAxisOptions(int axis);
	QFont axisFont(int axis);
	void pickAxisLabelFont();
	void pickNumbersFont();

	void viewScaleLimits(int axis);
	void disableMeshOptions();

	void worksheet();

	void initPointsOptionsStack();
	void changeZoom(int);
	void changeTransparency(int val);
    void pickDataColorMap();
	void updateColorMapFileGroupBox(bool);
	void updateLinearColorMapGroupBox(bool);
	void enableMajorGrids(bool on);
	void enableMinorGrids(bool on);

private:
	void initConnections();
    void initScalesPage();
	void initAxesPage();
	void initTitlePage();
	void initGridPage();
	void initColorsPage();
	void initGeneralPage();
	void initPrintPage();
	void setColorMapPreview(const QString& fileName);

	void showBarsTab();
    void showPointsTab(double rad, bool smooth);
	void showConesTab(double rad, int quality);
	void showCrossHairTab(double rad, double linewidth, bool smooth, bool boxed);

    Graph3D *d_plot = nullptr;
	QGroupBox *linearColorMapGroupBox = nullptr;
	QGroupBox *colorMapFileGroupBox = nullptr;
	QLabel *colorMapPreviewLabel = nullptr;
	QFont titleFont, xAxisFont, yAxisFont, zAxisFont, numbersFont;
	QStringList labels;
    QDoubleSpinBox *boxMeshLineWidth = nullptr;
    QPushButton* buttonApply = nullptr;
    QPushButton* buttonOk = nullptr;
    QPushButton* buttonCancel = nullptr;
	QPushButton *btnTitleFont = nullptr;
	QPushButton *btnLabelFont = nullptr;
    QPushButton *btnNumbersFont = nullptr;
	QPushButton *btnTable = nullptr;
	QPushButton *btnColorMap = nullptr;
	ColorButton *btnBackground = nullptr;
	ColorButton *btnMesh = nullptr;
	ColorButton *btnAxes = nullptr;
	ColorButton *btnTitleColor = nullptr;
	ColorButton *btnLabels = nullptr;
	ColorButton *btnNumbers = nullptr;
	ColorButton *btnGrid = nullptr;
	ColorButton *btnGridMinor = nullptr;
	ColorMapEditor *d_color_map_editor = nullptr;
    QTabWidget* generalDialog = nullptr;
	QWidget *scale = nullptr;
	QWidget *colors = nullptr;
	QWidget *general = nullptr;
	QWidget *axes = nullptr;
	QWidget *title = nullptr;
	QWidget *bars = nullptr;
	QWidget *points = nullptr;
	QWidget *gridPage = nullptr;
	QWidget *printPage = nullptr;
	DoubleSpinBox *boxFrom = nullptr;
	DoubleSpinBox *boxTo = nullptr;
	QTextEdit *boxTitle = nullptr;
	QTextEdit *boxLabel = nullptr;
	QSpinBox *boxMajors = nullptr;
	QSpinBox *boxMinors = nullptr;
	QGroupBox *TicksGroupBox = nullptr;
	QGroupBox *AxesColorGroupBox = nullptr;
	QSpinBox *boxResolution = nullptr;
	QSpinBox *boxDistance = nullptr;
	QSpinBox *boxTransparency = nullptr;
	QSlider *transparencySlider = nullptr;
	QCheckBox *boxLegend = nullptr;
	QCheckBox *boxSmooth = nullptr;
	QCheckBox *boxBoxed = nullptr;
	QCheckBox *boxCrossSmooth = nullptr;
	QCheckBox *boxOrthogonal = nullptr;
	QListWidget *axesList = nullptr;
	QListWidget *axesList2 = nullptr;
	QComboBox *boxType = nullptr;
	QComboBox *boxPointStyle = nullptr;
	DoubleSpinBox *boxMajorLength = nullptr;
	DoubleSpinBox *boxMinorLength = nullptr;
	DoubleSpinBox *boxConesRad = nullptr;
	QSpinBox *boxZoom = nullptr;
	QSpinBox *boxXScale = nullptr;
	QSpinBox *boxYScale = nullptr;
	QSpinBox *boxZScale = nullptr;
	QSpinBox *boxQuality = nullptr;
	QSpinBox *boxPrecision = nullptr;
	DoubleSpinBox *boxSize = nullptr;
	DoubleSpinBox *boxBarsRad = nullptr;
	DoubleSpinBox *boxCrossRad = nullptr;
	DoubleSpinBox *boxCrossLinewidth = nullptr;
	QStackedWidget *optionStack = nullptr;
	QWidget *dotsPage = nullptr;
	QWidget *conesPage = nullptr;
	QWidget *crossPage = nullptr;
	QCheckBox *boxBarLines = nullptr;
	QCheckBox *boxFilledBars = nullptr;
	QCheckBox *boxScaleOnPrint = nullptr;
	QCheckBox *boxPrintCropmarks = nullptr;
	TextFormatButtons *titleFormatButtons = nullptr;
	TextFormatButtons *axisTitleFormatButtons = nullptr;
    double zoom = 0.0;
	double xScale = 1.0;
	double yScale = 1.0;
	double zScale = 1.0;
	QString d_color_map_file;
	DoubleSpinBox *boxMajorGridWidth = nullptr;
	DoubleSpinBox *boxMinorGridWidth = nullptr;
	QComboBox *boxMajorGridStyle = nullptr;
	QComboBox *boxMinorGridStyle = nullptr;
	QComboBox *boxTickLabelsFormat = nullptr;
	QCheckBox *boxMajorGrids = nullptr;
	QCheckBox *boxMinorGrids = nullptr;
	DoubleSpinBox *boxXRotation = nullptr;
	DoubleSpinBox *boxYRotation = nullptr;
	DoubleSpinBox *boxZRotation = nullptr;
};

#endif
