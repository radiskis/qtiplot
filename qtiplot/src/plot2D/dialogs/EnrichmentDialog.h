/***************************************************************************
    File                 : EnrichmentDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A general properties dialog for the FrameWidget, using article
						  "Using a Simple Web Service with Qt" in Qt Quaterly, Issue 23, Q3 2007

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

#ifndef TEXWIDGETDIALOG_H
#define TEXWIDGETDIALOG_H

#include <QDialog>
#include <QProcess>

class QComboBox;
class QGroupBox;
class QHttp;
class QLabel;
class QPushButton;
class QTextEdit;
class QTabWidget;
class QCheckBox;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QSlider;
class QNetworkProxy;

class Graph;
class FrameWidget;
class LegendWidget;
class ColorButton;
class DoubleSpinBox;
class PatternBox;
class PenStyleBox;
class RectangleWidget;
class TextFormatButtons;
class ApplicationWindow;

class EnrichmentDialog : public QDialog
{
    Q_OBJECT

public:
	enum WidgetType{Frame, Text, Image, Tex, MDIWindow, Ellipse};

	EnrichmentDialog(WidgetType wt, Graph *g, ApplicationWindow *app, QWidget *parent = 0);
	~EnrichmentDialog();

	void setWidget(QWidget *w);
	void accept (){return apply();};

private slots:
    void clearForm();
    void fetchImage();
    void updateForm(bool error);
	void apply();
	void customButtons(QWidget *w);
	void chooseImageFile(const QString& fn = QString::null);
	void displayCoordinates(int unit);
	void adjustHeight(double width);
	void adjustWidth(double height);
	void saveImagesInternally(bool save);
	void setBestSize();
	void frameApplyTo();
	void patternApplyTo();
	void textFormatApplyTo();
	void customFont();
	void updateTransparency(int alpha);
	void setTextDefaultValues();
	void setFrameDefaultValues();
	void setRectangleDefaultValues();
	void finishedCompiling(int, QProcess::ExitStatus);
	void displayCompileError(QProcess::ProcessError error);
	void createImage();
	void updateCompilerInterface(int);
	void updateButtons();

private:
	void initTextPage();
	void initEditorPage();
	void initImagePage();
	void initFramePage();
	void initGeometryPage();
	void initPatternPage();
	void setCoordinates(int unit);
	void setFrameTo(FrameWidget *fw);
	void setPatternTo(FrameWidget *r);
	void setTextFormatTo(LegendWidget *l);
	void setText(QTextEdit *editor, const QString & t);
	QString createTempTexFile();

	ApplicationWindow *d_app;
    QHttp *http;
    QProcess *compileProcess, *dvipngProcess;

    QLabel *outputLabel;
    QPushButton *clearButton;
    QPushButton *updateButton;
	QPushButton *cancelButton;
	QPushButton *bestSizeButton;
    QTextEdit *equationEditor, *textEditBox;
	QComboBox *frameBox;
	QTabWidget* tabWidget;
	QWidget *editPage, *framePage, *geometryPage, *imagePage, *patternPage, *textPage;
	ColorButton *frameColorBtn, *backgroundColorBtn, *patternColorBtn;
	QCheckBox *boxSaveImagesInternally;
	QLineEdit *imagePathBox;
	DoubleSpinBox *xBox, *yBox, *widthBox, *heightBox, *boxFrameWidth;
	QComboBox *unitBox;
	PenStyleBox *boxFrameLineStyle;
	QCheckBox *keepAspectBox, *useFrameColorBox;
	QSpinBox *boxTransparency, *boxTextAngle;
	PatternBox *patternBox;
	QPushButton *textApplyToBtn;
	QComboBox *frameApplyToBox, *patternApplyToBox, *textApplyToBox;
	ColorButton *textColorBtn, *textBackgroundBtn;
	QPushButton *textFontBtn;
	QSpinBox *boxBackgroundTransparency;
	QSlider *transparencySlider, *fillTransparencySlider;
	TextFormatButtons *formatButtons, *texFormatButtons;
	QPushButton *textDefaultBtn, *frameDefaultBtn, *rectangleDefaultBtn;
	QCheckBox *autoUpdateTextBox, *texOutputBox;
    QComboBox *texCompilerBox;
    QComboBox* attachToBox;

	QFont textFont;

	Graph *d_plot;
	QWidget *d_widget;
	WidgetType d_widget_type;
	double aspect_ratio;
};

#endif
