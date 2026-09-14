/***************************************************************************
File                 : AxesDialog.h
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2004 - 2012 by Ion Vasilief
					   (C) 2006 Tilman Hoener zu Siederdissen
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Axes preferences dialog

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
#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QList>
#include <QTextEdit>

class QLineEdit;
class QTimeEdit;
class QDateTimeEdit;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QGroupBox;
class QComboBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTabWidget;
class QWidget;
#include <QStringList>
class ColorButton;
class Graph;
class TextFormatButtons;
class DoubleSpinBox;
class Grid;
class PenStyleBox;
class ApplicationWindow;

//! General plot options dialog
/**
 * Remark: Don't use this dialog as a non modal dialog!
 */
class AxesDialog : public QDialog
{
	Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
	AxesDialog( QWidget* parent = 0, Qt::WindowFlags fl = {} );
	ApplicationWindow *app() const;

	void setGraph(Graph *g);

public slots:
	void setCurrentScale(int axisPos);
	void showGeneralPage();
	void showAxesPage();
	void showGridPage();
	void showFormulaBox();

	//! Shows the dialog as a modal dialog
	/**
	 * Show the dialog as a modal dialog and do
	 * some initialization.
	 */
	int exec();

private slots:
	void showAxisSettings(int);
	void showAxisFormula(int);
	void customAxisLabelFont();
	void pickAxisLabelColor();

	void updateAxisType(int);
	bool updatePlot(QWidget *page = nullptr);
	void updateScale();
	void stepEnabled();
	void stepDisabled();
	void majorGridEnabled(bool);
	void minorGridEnabled(bool);
	void showGridSettings(int);
	void accept();
	void customAxisFont();
	void showAxis();

	void updateTickLabelsList(bool);
	void updateMinorTicksList(int);

	void updateGrid();
	void setLabelsNumericFormat(int);

	void showAxisFormatOptions(int);
	void changeMinorTicksLength(int);
	void changeMajorTicksLength(int);
	void pageChanged(QWidget *page);
	void applyCanvasFormat();
	void setFrameDefaultValues();
	void applyAxisFormat();

private:
	void showAxis(int, int, const QString&, bool, int, int, bool, const QColor&, int, int, int, int, const QString&, const QColor&, int, bool, int);
	int mapToQwtAxis(int axis);
	int mapToQwtAxisId();
	void showGeneralSettings();
	void applyCanvasFormatTo(Graph *g);
	//! generate UI for the axes page
	void initAxesPage();
	//! generate UI for the scales page
	void initScalesPage();
	//! generate UI for the grid page
	void initGridPage();
	//! generate UI for the general page
	void initFramePage();
	//! Modifies the grid
	void applyChangesToGrid(Grid *grid);
	//! Customizes graph axes
	void applyAxisFormatToLayer(Graph *g);

	QPushButton* buttonApply = nullptr;
	QPushButton* buttonOk = nullptr;
	QPushButton* buttonCancel = nullptr;
	QTabWidget* generalDialog = nullptr;
	QWidget* scalesPage = nullptr;
	DoubleSpinBox* boxEnd = nullptr;
	DoubleSpinBox* boxStart = nullptr;
	QComboBox* boxScaleType = nullptr;
	QComboBox* boxMinorValue = nullptr;
	DoubleSpinBox* boxStep = nullptr;
	QRadioButton* btnStep = nullptr;
	QCheckBox *btnInvert = nullptr;
	QSpinBox* boxMajorValue = nullptr;
	QRadioButton* btnMajor = nullptr;
	QListWidget* axesList = nullptr;
	QWidget* gridPage = nullptr;
	QCheckBox* boxMajorGrid = nullptr;
	QCheckBox* boxMinorGrid = nullptr;
	PenStyleBox* boxTypeMajor = nullptr;
	ColorButton* boxColorMinor = nullptr;
	ColorButton* boxColorMajor = nullptr;
	ColorButton *boxCanvasColor = nullptr;
	DoubleSpinBox* boxWidthMajor = nullptr;
	PenStyleBox* boxTypeMinor = nullptr;
	DoubleSpinBox* boxWidthMinor = nullptr;
	QCheckBox* boxXLine = nullptr;
	QCheckBox* boxYLine = nullptr;
	QListWidget* axesGridList = nullptr;
	QWidget* axesPage = nullptr, *frame = nullptr;
	QListWidget* axesTitlesList = nullptr;
	QGroupBox *boxShowLabels = nullptr;
	QGroupBox *boxShowAxis = nullptr;

	QTextEdit *boxFormula = nullptr, *boxTitle = nullptr;
	QSpinBox *boxFrameWidth = nullptr, *boxPrecision = nullptr, *boxAngle = nullptr, *boxBaseline = nullptr, *boxAxesLinewidth = nullptr;
	QPushButton* btnAxesFont = nullptr;
	QCheckBox *boxBackbones = nullptr, *boxShowFormula = nullptr;
	ColorButton* boxAxisColor = nullptr;
	QComboBox *boxMajorTicksType = nullptr, *boxMinorTicksType = nullptr, *boxFormat = nullptr, *boxAxisType = nullptr, *boxColName = nullptr;
	QGroupBox *boxFramed = nullptr;
	QLabel *label1 = nullptr, *label2 = nullptr, *label3 = nullptr, *boxScaleTypeLabel = nullptr, *minorBoxLabel = nullptr, *labelTable = nullptr;
	QSpinBox *boxMajorTicksLength = nullptr, *boxMinorTicksLength = nullptr, *boxBorderWidth = nullptr;
	QComboBox *boxUnit = nullptr, *boxTableName = nullptr, *boxGridXAxis = nullptr, *boxGridYAxis = nullptr;
	ColorButton *boxFrameColor = nullptr, *boxAxisNumColor = nullptr;
	QGroupBox  *labelBox = nullptr;
	QPushButton * buttonLabelFont = nullptr;
	TextFormatButtons *formatButtons = nullptr;

	QGroupBox *boxAxesBreaks = nullptr;
	DoubleSpinBox *boxBreakStart = nullptr, *boxBreakEnd = nullptr, *boxStepBeforeBreak = nullptr, *boxStepAfterBreak = nullptr;
	QSpinBox *boxBreakPosition = nullptr, *boxBreakWidth = nullptr;
	QComboBox *boxMinorTicksBeforeBreak = nullptr, *boxMinorTicksAfterBreak = nullptr;
	QCheckBox *boxLog10AfterBreak = nullptr, *boxBreakDecoration = nullptr, *boxAntialiseGrid = nullptr;
	QComboBox *boxApplyGridFormat = nullptr;
	Graph* d_graph = nullptr;
	//! Last selected tab
	QWidget* lastPage = nullptr;
	QDateTimeEdit *boxStartDateTime = nullptr, *boxEndDateTime = nullptr;
	QComboBox *canvasFrameApplyToBox = nullptr;
	QPushButton *frameDefaultBtn = nullptr;
	QSpinBox *boxLabelsDistance = nullptr;
	QGroupBox *axisFormatBox = nullptr;
	QComboBox *axisFormatApplyToBox = nullptr;
	QCheckBox *invertTitleBox = nullptr;
	QCheckBox* boxAxisBackbone = nullptr;
	QSpinBox *boxTickLabelDistance = nullptr;
	QLineEdit *boxPrefix = nullptr, *boxSuffix = nullptr;

	QComboBox *showTicksPolicyBox = nullptr;
	ColorButton *axisLabelColorButton = nullptr;
};

#endif
