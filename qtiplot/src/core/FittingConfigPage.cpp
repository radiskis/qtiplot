/***************************************************************************
    File                 : FittingConfigPage.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Curve fitting preferences page implementation

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
#include "FittingConfigPage.h"
#include <ApplicationWindow.h>
#include <ApplicationSettings.h>
#include <ColorButton.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QIcon>

FittingConfigPage::FittingConfigPage(QWidget *parent)
    : ConfigPage(parent)
{
	groupBoxFittingCurve = new QGroupBox();
	QGridLayout * fittingCurveLayout = new QGridLayout( groupBoxFittingCurve );

	samePointsBtn = new QRadioButton();
	fittingCurveLayout->addWidget( samePointsBtn, 0, 0 );

	generatePointsBtn = new QRadioButton();
	fittingCurveLayout->addWidget( generatePointsBtn, 1, 0 );

	lblPoints = new QLabel();
	fittingCurveLayout->addWidget( lblPoints, 1, 1 );

	generatePointsBox = new QSpinBox();
	generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(100);
	fittingCurveLayout->addWidget( generatePointsBox, 1, 2 );

	linearFit2PointsBox = new QCheckBox();
	fittingCurveLayout->addWidget( linearFit2PointsBox, 2, 0, 1, 3 );

	groupBoxFitParameters = new QGroupBox();
	QGridLayout * fitParamsLayout = new QGridLayout( groupBoxFitParameters );

	scaleErrorsBox = new QCheckBox();
	fitParamsLayout->addWidget( scaleErrorsBox, 0, 0 );

	logBox = new QCheckBox();
	fitParamsLayout->addWidget( logBox, 1, 0 );

	plotLabelBox = new QCheckBox();
	fitParamsLayout->addWidget( plotLabelBox, 2, 0 );

	lblPrecision = new QLabel();
	fitParamsLayout->addWidget( lblPrecision, 3, 0 );

	boxPrecision = new QSpinBox();
	boxPrecision->setRange(0, 16);
	fitParamsLayout->addWidget( boxPrecision, 3, 1 );

	groupBoxMultiPeak = new QGroupBox();
	QGridLayout * multiPeakLayout = new QGridLayout( groupBoxMultiPeak );

	generatePeaksBox = new QCheckBox();
	multiPeakLayout->addWidget( generatePeaksBox, 0, 0 );

	lblPeaksColor = new QLabel();
	multiPeakLayout->addWidget( lblPeaksColor, 0, 1 );

	boxPeaksColor = new ColorButton();
	multiPeakLayout->addWidget( boxPeaksColor, 0, 2 );

	boxMultiPeakMsgs = new QCheckBox();
	multiPeakLayout->addWidget( boxMultiPeakMsgs, 1, 0, 1, 3 );

	QVBoxLayout * fitPageLayout = new QVBoxLayout( this );
	fitPageLayout->addWidget(groupBoxFittingCurve);
	fitPageLayout->addWidget(groupBoxMultiPeak);
	fitPageLayout->addWidget(groupBoxFitParameters);
	fitPageLayout->addStretch();

	connect(generatePointsBtn, &QAbstractButton::toggled, this, &FittingConfigPage::showPointsBox);
}

void FittingConfigPage::init(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	boxPrecision->setValue(app->fit_output_precision);
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	logBox->setChecked(app->writeFitResultsToLog());
	generatePointsBox->setValue(app->fitPoints);
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	showPointsBox(app->generateUniformFitPoints);

	generatePeaksBox->setChecked(app->generatePeakCurves);
	groupBoxMultiPeak->setChecked(app->generatePeakCurves);
	boxPeaksColor->setColor(app->peakCurvesColor);
	scaleErrorsBox->setChecked(app->fit_scale_errors);
	linearFit2PointsBox->setChecked(app->d_2_linear_fit_points);
	boxMultiPeakMsgs->setChecked(app->d_multi_peak_messages);
}

void FittingConfigPage::apply(ApplicationWindow *app, ApplicationSettings *settings)
{
	Q_UNUSED(settings);
	if (!app)
		return;

	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->setWriteFitResultsToLog(logBox->isChecked());
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->generatePeakCurves = groupBoxMultiPeak->isChecked();
	app->peakCurvesColor = boxPeaksColor->color();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->d_2_linear_fit_points = linearFit2PointsBox->isChecked();
	app->d_multi_peak_messages = boxMultiPeakMsgs->isChecked();
}

void FittingConfigPage::retranslateUi()
{
	groupBoxFittingCurve->setTitle(tr("Generated Fit Curve"));
	samePointsBtn->setText(tr("Uniform X Function"));
	generatePointsBtn->setText(tr("Uniform &Points"));
	lblPoints->setText(tr("Points"));
	linearFit2PointsBox->setText(tr("2 points for &linear fits"));
	groupBoxFitParameters->setTitle(tr("Parameters"));
	scaleErrorsBox->setText(tr("Scale Errors with sqrt(Chi^2/doF)"));
	logBox->setText(tr("Write Parameters to Result &Log"));
	plotLabelBox->setText(tr("Paste Parameters to &Plot"));
	lblPrecision->setText(tr("Significant Digits"));
	groupBoxMultiPeak->setTitle(tr("Multi-peak Fits"));
	generatePeaksBox->setText(tr("&Generate Peaks"));
	lblPeaksColor->setText(tr("Color"));
	boxMultiPeakMsgs->setText(tr("Display &Information Messages"));
}

void FittingConfigPage::showPointsBox(bool on)
{
	lblPoints->setEnabled(on);
	generatePointsBox->setEnabled(on);
}
