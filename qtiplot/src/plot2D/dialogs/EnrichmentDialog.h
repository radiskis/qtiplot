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
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QComboBox;
class QGroupBox;
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

	EnrichmentDialog(WidgetType wt, Graph *g, ApplicationWindow *app, QWidget *parent = nullptr);
	~EnrichmentDialog() override;

	void setWidget(QWidget *w);
	void accept () override{return apply();};

private slots:
    void clearForm();
    void fetchImage();
    void updateForm(QNetworkReply *reply);
	void apply();
	void customButtons(QWidget *w);
	void chooseImageFile(const QString& fn = QString());
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

	ApplicationWindow *d_app = nullptr;
    QNetworkAccessManager *d_network_manager = nullptr;
    QProcess *compileProcess = nullptr, *dvipngProcess = nullptr;

    QLabel *outputLabel = nullptr;
    QPushButton *clearButton = nullptr;
    QPushButton *updateButton = nullptr;
	QPushButton *cancelButton = nullptr;
	QPushButton *bestSizeButton = nullptr;
    QTextEdit *equationEditor = nullptr, *textEditBox = nullptr;
	QComboBox *frameBox = nullptr;
	QTabWidget* tabWidget = nullptr;
	QWidget *editPage = nullptr, *framePage = nullptr, *geometryPage = nullptr, *imagePage = nullptr, *patternPage = nullptr, *textPage = nullptr;
	ColorButton *frameColorBtn = nullptr, *backgroundColorBtn = nullptr, *patternColorBtn = nullptr;
	QCheckBox *boxSaveImagesInternally = nullptr;
	QLineEdit *imagePathBox = nullptr;
	DoubleSpinBox *xBox = nullptr, *yBox = nullptr, *widthBox = nullptr, *heightBox = nullptr, *boxFrameWidth = nullptr;
	QComboBox *unitBox = nullptr;
	PenStyleBox *boxFrameLineStyle = nullptr;
	QCheckBox *keepAspectBox = nullptr, *useFrameColorBox = nullptr;
	QSpinBox *boxTransparency = nullptr, *boxTextAngle = nullptr;
	PatternBox *patternBox = nullptr;
	QPushButton *textApplyToBtn = nullptr;
	QComboBox *frameApplyToBox = nullptr, *patternApplyToBox = nullptr, *textApplyToBox = nullptr;
	ColorButton *textColorBtn = nullptr, *textBackgroundBtn = nullptr;
	QPushButton *textFontBtn = nullptr;
	QSpinBox *boxBackgroundTransparency = nullptr;
	QSlider *transparencySlider = nullptr, *fillTransparencySlider = nullptr;
	TextFormatButtons *formatButtons = nullptr, *texFormatButtons = nullptr;
	QPushButton *textDefaultBtn = nullptr, *frameDefaultBtn = nullptr, *rectangleDefaultBtn = nullptr;
	QCheckBox *autoUpdateTextBox = nullptr, *texOutputBox = nullptr;
    QComboBox *texCompilerBox = nullptr;
    QComboBox* attachToBox = nullptr;

	QFont textFont;

	Graph *d_plot = nullptr;
	QWidget *d_widget = nullptr;
	WidgetType d_widget_type = Frame;
	double aspect_ratio = 0.0;
};

#endif
