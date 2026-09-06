/***************************************************************************
    File                 : FittingConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Curve fitting preferences page

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
#ifndef FITTING_CONFIG_PAGE_H
#define FITTING_CONFIG_PAGE_H

#include "ConfigPage.h"

class QGroupBox;
class QRadioButton;
class QLabel;
class QSpinBox;
class QCheckBox;
class ColorButton;

class FittingConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit FittingConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("Fitting"); }
    QIcon pageIcon() const override { return QIcon(":/fit.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

private slots:
    void showPointsBox(bool);

private:
    QGroupBox *groupBoxFittingCurve;
    QRadioButton *samePointsBtn;
    QRadioButton *generatePointsBtn;
    QLabel *lblPoints;
    QSpinBox *generatePointsBox;
    QCheckBox *linearFit2PointsBox;
    QGroupBox *groupBoxFitParameters;
    QCheckBox *scaleErrorsBox;
    QCheckBox *logBox;
    QCheckBox *plotLabelBox;
    QLabel *lblPrecision;
    QSpinBox *boxPrecision;
    QGroupBox *groupBoxMultiPeak;
    QCheckBox *generatePeaksBox;
    QLabel *lblPeaksColor;
    ColorButton *boxPeaksColor;
    QCheckBox *boxMultiPeakMsgs;
};

#endif // FITTING_CONFIG_PAGE_H
