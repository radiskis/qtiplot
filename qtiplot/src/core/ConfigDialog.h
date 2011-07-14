/***************************************************************************
    File                 : ConfigDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Preferences dialog

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
#ifndef ConfigDialog_H
#define ConfigDialog_H

#include <FrameWidget.h>

#include <QDialog>
#include <QCheckBox>

class QNetworkProxy;
class QLineEdit;
class QGroupBox;
class QGridLayout;
class QPushButton;
class QTabWidget;
class QStackedWidget;
class QWidget;
class QComboBox;
class QSpinBox;
class QLabel;
class QRadioButton;
class QListWidget;
class QSlider;
class ColorButton;
class DoubleSpinBox;
class QFontComboBox;
class ColorMapEditor;
class QTableWidget;
class SymbolBox;
class PatternBox;
class PenStyleBox;
class ApplicationWindow;

//! Preferences dialog
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget (must be the application window!=
	 * \param fl window flags
	 */
    ConfigDialog( QWidget* parent, Qt::WFlags fl = 0 );
	void setColumnSeparator(const QString& sep);

private slots:
    virtual void languageChange();
	void insertLanguagesList();

	void accept();
	void apply();
	void resetDefaultSettings();

	void setCurrentPage(int index);

	//table fonts
	void pickTextFont();
	void pickHeaderFont();

	//graph fonts
	void pickAxesFont();
	void pickNumbersFont();
	void pickLegendFont();
	void pickTitleFont();

	void showFrameWidth(bool ok);

	//application
	void pickApplicationFont();

	//2D curves
	int curveStyle();
	void pick3DTitleFont();
	void pick3DNumbersFont();
	void pick3DAxesFont();

	//Fitting
	void showPointsBox(bool);

	void switchToLanguage(int param);

	void chooseTranslationsFolder();
	void chooseHelpFolder();
#ifdef SCRIPTING_PYTHON
	void choosePythonConfigFolder();
	void chooseStartupScriptsFolder();
	void showStartupScriptsFolder(const QString &);
#endif
	void rehighlight();
    void customizeNotes();
	void chooseTexCompiler();
	bool validateTexCompiler();

	void chooseOffice();
	bool validateOffice();
	void chooseJava();
	bool validateJava();
	void chooseJODConverter();
	bool validateJODConverter();

	void enableMajorGrids(bool on);
	void enableMinorGrids(bool on);

	void updateCanvasSize(int unit);
	void adjustCanvasHeight(double width);
	void adjustCanvasWidth(double height);

	void moveColor(bool up = true);
	void moveColorDown();
	void removeColor();
	void newColor();
	void loadDefaultColors();
	void showColorDialog(int, int);
	void changeColorName(int, int);
	void updateSymbolsList(int);
	void setCurrentSymbol(SymbolBox *);
	void loadDefaultSymbols();
	void moveSymbol(bool up = true);
	void moveSymbolDown(){moveSymbol(false);};
	void showGridOptions(int axis);
	void updateGrid();
	void majorGridEnabled(bool);
	void minorGridEnabled(bool);
	void enableCurveAntialiasingSizeBox(bool);

private:
	void setApplication(ApplicationWindow *app);
	void setSymbolsList(const QList<int>& symbList);
	void setColorsList(const QList<QColor>& colList, const QStringList& colNames);
	void initPlotsPage();
	void initAppPage();
	void initCurvesPage();
	void initAxesPage();
	void initGridPage();
	void initPlots3DPage();
	void initTablesPage();
	void initConfirmationsPage();
	void initFileLocationsPage();
	void initFittingPage();
	void initNotesPage();
	void initProxyPage();
	void initLayerGeometryPage();
	void initLayerSpeedPage();
	//! Calculates a sensible width for the items list
	void updateMenuList();
	bool validFolderPath(const QString& path);
	QNetworkProxy setApplicationCustomProxy();
	int convertToPixels(double w, FrameWidget::Unit unit, int dimension);
	double convertFromPixels(int w, FrameWidget::Unit unit, int dimension);

	QFont textFont, headerFont, axesFont, numbersFont, legendFont, titleFont, appFont;
	QFont d_3D_title_font, d_3D_numbers_font, d_3D_axes_font;

	QCheckBox *boxScaleLayersOnPrint, *boxPrintCropmarks, *linearFit2PointsBox;
	QTabWidget *plotsTabWidget, *appTabWidget;
	ColorButton *btnBackground3D, *btnMesh, *btnAxes, *btnLabels, *btnNumbers;
	QGroupBox *colorMapBox;
	ColorMapEditor *colorMapEditor;
	QPushButton	*btnTitleFnt, *btnLabelsFnt, *btnNumFnt;
	ColorButton *buttonBackground, *buttonText, *buttonHeader;
    QPushButton *buttonOk, *buttonCancel, *buttonApply;
	QPushButton* buttonTextFont, *buttonHeaderFont;
	QStackedWidget * generalDialog;
	QWidget *appColors, *tables, *plotOptions, *plotTicks, *plotFonts, *confirm, *plotPrint;
	QWidget *application, *curves, *axesPage, *plots3D, *fitPage, *numericFormatPage, *notesPage, *plotGeometryPage, *plotSpeedPage;
	QPushButton* buttonAxesFont, *buttonNumbersFont, *buttonLegendFont, *buttonTitleFont, *fontsBtn;
	QCheckBox *boxSearchUpdates, *boxOrthogonal, *logBox, *plotLabelBox, *scaleErrorsBox;
	QCheckBox *boxTitle, *boxFrame, *boxPlots3D, *boxPlots2D, *boxTables, *boxNotes, *boxFolders;
	QCheckBox *boxSave, *boxBackbones, *boxShowLegend, *boxSmoothMesh;
	QCheckBox *boxAutoscaling, *boxMatrices, *boxScaleFonts, *boxResize;
	QComboBox *boxMajTicks, *boxMinTicks, *boxStyle, *boxCurveStyle, *boxSeparator, *boxLanguage, *boxDecimalSeparator;
	QComboBox *boxClipboardLocale, *boxProjection;
	QLabel *lblClipboardSeparator, *lblFloorStyle;
	QSpinBox *boxMinutes, *boxLineWidth, *boxFrameWidth, *boxResolution, *boxMargin, *boxPrecision, *boxAppPrecision;
	QSpinBox *boxSymbolSize, *boxMinTicksLength, *boxMajTicksLength, *generatePointsBox;
	DoubleSpinBox *boxCurveLineWidth;
	ColorButton *btnWorkspace, *btnPanels, *btnPanelsText;
	QListWidget * itemsList;
	QLabel *labelFrameWidth, *lblLanguage, *lblWorkspace, *lblPanels, *lblPageHeader;
	QLabel *lblPanelsText, *lblFonts, *lblStyle, *lblDecimalSeparator, *lblAppPrecision;
	QGroupBox *groupBoxConfirm;
	QGroupBox *groupBoxTableFonts, *groupBoxTableCol;
	QLabel *lblSeparator, *lblTableBackground, *lblTextColor, *lblHeaderColor;
	QLabel *lblSymbSize, *lblAxesLineWidth, *lblCurveStyle, *lblResolution, *lblPrecision;
	QGroupBox *groupBox3DFonts, *groupBox3DCol;
	QLabel *lblMargin, *lblMajTicks, *lblMajTicksLength, *lblLineWidth, *lblMinTicks, *lblMinTicksLength, *lblPoints, *lblPeaksColor;
	QGroupBox *groupBoxFittingCurve, *groupBoxFitParameters;
	QRadioButton *samePointsBtn, *generatePointsBtn;
    QGroupBox *groupBoxMultiPeak;
	ColorButton *boxPeaksColor;
	QLabel *lblScriptingLanguage, *lblInitWindow;
	QComboBox *boxScriptingLanguage, *boxInitWindow;
	QCheckBox *boxAutoscale3DPlots, *boxTableComments, *boxThousandsSeparator;
	QCheckBox *boxPromptRenameTables, *boxBackupProject, *boxLabelsEditing, *boxEmptyCellGap;
	QWidget *fileLocationsPage;
	QLabel *lblTranslationsPath, *lblHelpPath, *lblUndoStackSize, *lblEndOfLine;
	QLineEdit *translationsPathLine, *helpPathLine;
	QSpinBox *undoStackSizeBox;
	QComboBox *boxEndLine;
#ifdef SCRIPTING_PYTHON
	QLabel *lblPythonConfigDir;
	QLineEdit *pythonConfigDirLine;
	QPushButton *browsePythonScriptsBtn;
	QLabel *lblPythonScriptsDir;
	QLineEdit *pythonScriptsDirLine;
#endif
	QCheckBox *boxUpdateTableValues, *boxTablePasteDialog;
	QGroupBox *groupBackgroundOptions;
	QLabel *labelGraphFrameColor, *labelGraphFrameWidth;
	QLabel *labelGraphBkgColor, *labelGraphCanvasColor;
	QLabel *labelGraphBkgOpacity, *labelGraphCanvasOpacity;
	ColorButton *boxBackgroundColor, *boxCanvasColor, *boxBorderColor;
	QSpinBox *boxBackgroundTransparency, *boxCanvasTransparency, *boxBorderWidth, *boxTabLength;
	QSlider *bkgOpacitySlider, *canvasOpacitySlider, *curveOpacitySlider;
	QCheckBox *completionBox, *lineNumbersBox;
	QLabel *labelTabLength, *labelNotesFont;
	QFontComboBox *boxFontFamily;
	QSpinBox *boxFontSize;
	QPushButton *buttonItalicFont, *buttonBoldFont;
	QLabel *labelGraphAxesLabelsDist, *labelTickLabelsDist;
	QSpinBox *boxAxesLabelsDist, *boxTickLabelsDist;
	QLabel *xBottomLabel, *xTopLabel, *yLeftLabel, *yRightLabel, *enableAxisLabel, *showNumbersLabel;
	QCheckBox *boxEnableAxis, *boxShowAxisLabels;
	QGroupBox * enabledAxesGroupBox;
	QGridLayout *enabledAxesGrid;

	QWidget *proxyPage;
	QGroupBox *proxyGroupBox;
    QLineEdit *proxyHostLine, *proxyUserNameLine, *proxyPasswordLine;
    QSpinBox *proxyPortBox;
    QLabel *proxyHostLabel, *proxyPortLabel, *proxyUserLabel, *proxyPasswordLabel;

    QLineEdit *texCompilerPathBox;
    QPushButton *browseTexCompilerBtn;
    QLabel *texCompilerLabel;

	QComboBox *legendDisplayBox, *attachToBox;
	QLabel *legendDisplayLabel, *attachToLabel;

	DoubleSpinBox *boxMajorGridWidth, *boxMinorGridWidth;
	QComboBox *boxMajorGridStyle, *boxMinorGridStyle;
	QCheckBox *boxMajorGrids, *boxMinorGrids;
	ColorButton *btnGrid, *btnGridMinor;
	QLabel *label3DGridsColor, *label3DGridsWidth, *label3DGridsStyle;
	QGroupBox *groupBox3DGrids;

	QGroupBox *groupSyntaxHighlighter;
	ColorButton *buttonCommentColor, *buttonNumericColor, *buttonQuotationColor;
	ColorButton *buttonKeywordColor, *buttonFunctionColor, *buttonClassColor;
	QLabel *buttonCommentLabel, *buttonNumericLabel, *buttonQuotationLabel;
	QLabel *buttonKeywordLabel, *buttonFunctionLabel, *buttonClassLabel;

	QCheckBox *boxMuParserCLocale, *boxConfirmOverwrite, *boxConfirmModifyDataPoints;
	DoubleSpinBox *boxCanvasHeight, *boxCanvasWidth;
	QComboBox *unitBox;
	QLabel *unitBoxLabel, *canvasWidthLabel, *canvasHeightLabel;
	QCheckBox *keepRatioBox, *boxMultiPeakMsgs;

	double aspect_ratio;

	QGroupBox *groupIndexedColors, *symbolGroupBox, *groupIndexedSymbols, *fillCurvesGroupBox;
	QTableWidget *colorsList, *symbolsList;
	QPushButton *btnColorUp, *btnColorDown, *btnRemoveColor, *btnNewColor, *btnLoadDefaultColors;
	QList<QColor> d_indexed_colors;
	QStringList d_indexed_color_names;
	QCheckBox *fillSymbolsBox;
	DoubleSpinBox *symbolEdgeBox;
	QLabel *lblSymbEdge, *lblSymbBox;
	SymbolBox *symbolBox;
	QList<int> d_indexed_symbols;
	QPushButton *btnLoadDefaultSymbols, *btnSymbolUp, *btnSymbolDown;
	PatternBox *patternBox;
	QLabel *lblPattern, *lblCurveAlpha, *lblLineStyle;
	QSpinBox *curveAlphaBox;
	PenStyleBox *lineStyleBox;
	QGroupBox * curvesGroupBox;
	QLabel *lblAxisLabeling;
	QComboBox *axisLabelingBox;
	QCheckBox *boxSynchronizeScales;

	QWidget *gridPage;
	QCheckBox *boxMinorGrid, *boxMajorGrid;
	ColorButton *boxColorMinor, *boxColorMajor;
	PenStyleBox *boxTypeMinor, *boxTypeMajor;
	DoubleSpinBox *boxWidthMinor, *boxWidthMajor;
	QComboBox *boxGridYAxis, *boxGridXAxis;
	QCheckBox *boxAntialiseGrid;
	QListWidget *axesGridList;
	QLabel *gridLineColorLbl, *gridLineWidthLbl, *gridLineTypeLbl, *gridAxesLbl;
	QGroupBox * antialiasingGroupBox;
	QCheckBox *disableAntialiasingBox, *openLastProjectBox;
	QSpinBox *curveSizeBox;
	QPushButton *btnDefaultSettings;

	QLineEdit *sofficePathBox;
	QPushButton *browseOfficeBtn;
	QLabel *officeLabel;

	QLineEdit *javaPathBox;
	QPushButton *browseJavaBtn;
	QLabel *javaLabel;

	QLineEdit *jodconverterPathBox;
	QPushButton *browseJODConverterBtn;
	QLabel *jodconverterLabel;

	QLabel *excelImportMethodLabel;
	QComboBox *excelImportMethod;
};

#endif // CONFIGDIALOG_H
