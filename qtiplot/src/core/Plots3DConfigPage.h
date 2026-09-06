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
    QGroupBox *groupBox3DCol;
    ColorButton *btnBackground3D;
    ColorButton *btnMesh;
    ColorButton *btnAxes;
    ColorButton *btnLabels;
    ColorButton *btnNumbers;
    ColorButton *btnGrid;
    ColorButton *btnGridMinor;

    QGroupBox *colorMapBox;
    ColorMapEditor *colorMapEditor;

    QGroupBox *groupBox3DFonts;
    QPushButton *btnTitleFnt;
    QPushButton *btnLabelsFnt;
    QPushButton *btnNumFnt;

    QGroupBox *gridsGroup;
    QCheckBox *boxMajorGrids;
    QCheckBox *boxMinorGrids;
    QLabel *lblMajorGridWidth;
    QLabel *lblMinorGridWidth;
    QLabel *lblMajorGridStyle;
    QLabel *lblMinorGridStyle;
    DoubleSpinBox *boxMajorGridWidth;
    DoubleSpinBox *boxMinorGridWidth;
    QComboBox *boxMajorGridStyle;
    QComboBox *boxMinorGridStyle;

    QLabel *lblResolution;
    QSpinBox *boxResolution;
    QCheckBox *boxShowLegend;
    QCheckBox *boxSmoothMesh;
    QCheckBox *boxOrthogonal;
    QCheckBox *boxAutoscale3DPlots;
    QCheckBox *boxScaleFonts3DPlots;
    QLabel *lblProjection;
    QComboBox *boxProjection;
    QLabel *lblShadingStyle;
    QComboBox *boxShading;

    QFont d_3D_title_font;
    QFont d_3D_numbers_font;
    QFont d_3D_axes_font;
};

#endif // PLOTS3D_CONFIG_PAGE_H
