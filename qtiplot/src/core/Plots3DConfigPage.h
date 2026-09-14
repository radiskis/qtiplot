/***************************************************************************
    File                 : Plots3DConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 3D plots preferences page

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
#ifndef PLOTS3D_CONFIG_PAGE_H
#define PLOTS3D_CONFIG_PAGE_H

#include "ConfigPage.h"
#include <QFont>

class QGroupBox;
class ColorButton;
class ColorMapEditor;
class QPushButton;
class QLabel;
class QSpinBox;
class DoubleSpinBox;
class QComboBox;
class QCheckBox;

class Plots3DConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit Plots3DConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("3D Plots"); }
    QIcon pageIcon() const override { return QIcon(":/logo.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

private slots:
    void pick3DTitleFont();
    void pick3DNumbersFont();
    void pick3DAxesFont();
    void enableMajorGrids(bool on);
    void enableMinorGrids(bool on);

private:
    QGroupBox *groupBox3DCol = nullptr;
    ColorButton *btnBackground3D = nullptr;
    ColorButton *btnMesh = nullptr;
    ColorButton *btnAxes = nullptr;
    ColorButton *btnLabels = nullptr;
    ColorButton *btnNumbers = nullptr;
    ColorButton *btnGrid = nullptr;
    ColorButton *btnGridMinor = nullptr;

    QGroupBox *colorMapBox = nullptr;
    ColorMapEditor *colorMapEditor = nullptr;

    QGroupBox *groupBox3DFonts = nullptr;
    QPushButton *btnTitleFnt = nullptr;
    QPushButton *btnLabelsFnt = nullptr;
    QPushButton *btnNumFnt = nullptr;

    QGroupBox *gridsGroup = nullptr;
    QCheckBox *boxMajorGrids = nullptr;
    QCheckBox *boxMinorGrids = nullptr;
    QLabel *lblMajorGridWidth = nullptr;
    QLabel *lblMinorGridWidth = nullptr;
    QLabel *lblMajorGridStyle = nullptr;
    QLabel *lblMinorGridStyle = nullptr;
    DoubleSpinBox *boxMajorGridWidth = nullptr;
    DoubleSpinBox *boxMinorGridWidth = nullptr;
    QComboBox *boxMajorGridStyle = nullptr;
    QComboBox *boxMinorGridStyle = nullptr;

    QLabel *lblResolution = nullptr;
    QSpinBox *boxResolution = nullptr;
    QCheckBox *boxShowLegend = nullptr;
    QCheckBox *boxSmoothMesh = nullptr;
    QCheckBox *boxOrthogonal = nullptr;
    QCheckBox *boxAutoscale3DPlots = nullptr;
    QCheckBox *boxScaleFonts3DPlots = nullptr;
    QLabel *lblProjection = nullptr;
    QComboBox *boxProjection = nullptr;
    QLabel *lblShadingStyle = nullptr;
    QComboBox *boxShading = nullptr;

    QFont d_3D_title_font;
    QFont d_3D_numbers_font;
    QFont d_3D_axes_font;
};

#endif // PLOTS3D_CONFIG_PAGE_H
