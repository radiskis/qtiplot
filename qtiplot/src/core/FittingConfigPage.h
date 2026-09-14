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
    QGroupBox *groupBoxFittingCurve = nullptr;
    QRadioButton *samePointsBtn = nullptr;
    QRadioButton *generatePointsBtn = nullptr;
    QLabel *lblPoints = nullptr;
    QSpinBox *generatePointsBox = nullptr;
    QCheckBox *linearFit2PointsBox = nullptr;
    QGroupBox *groupBoxFitParameters = nullptr;
    QCheckBox *scaleErrorsBox = nullptr;
    QCheckBox *logBox = nullptr;
    QCheckBox *plotLabelBox = nullptr;
    QLabel *lblPrecision = nullptr;
    QSpinBox *boxPrecision = nullptr;
    QGroupBox *groupBoxMultiPeak = nullptr;
    QCheckBox *generatePeaksBox = nullptr;
    QLabel *lblPeaksColor = nullptr;
    ColorButton *boxPeaksColor = nullptr;
    QCheckBox *boxMultiPeakMsgs = nullptr;
};

#endif // FITTING_CONFIG_PAGE_H
