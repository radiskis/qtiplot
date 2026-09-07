/***************************************************************************
    File                 : AnalysisController.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Analysis, calculus, fitting, and filtering controller

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                   *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                            *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                            *
 *                                                                         *
 ***************************************************************************/

#ifndef ANALYSIS_CONTROLLER_H
#define ANALYSIS_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

class ApplicationWindow;
class Graph;
class MultiLayer;
class Table;
class Matrix;
class Note;
class QwtPlotCurve;

class AnalysisController : public QObject
{
    Q_OBJECT

public:
    explicit AnalysisController(ApplicationWindow *app);
    virtual ~AnalysisController();

    void showChiSquareTestDialog();
    void showStudentTestDialog(bool twoSamples);
    void testNormality();
#ifdef HAVE_TAMUANOVA
    void showANOVADialog(bool twoWay);
#endif
    void showExpGrowthDialog();
    void showExpDecayDialog();
    void showExpDecayDialog(int type);
    void showTwoExpDecayDialog();
    void showExpDecay3Dialog();
    void showFitDialog();
    void showFilterDialog(int filter);
    void lowPassFilterDialog();
    void highPassFilterDialog();
    void bandPassFilterDialog();
    void bandBlockFilterDialog();
    void showFFTDialog();
    void showSmoothDialog(int m);
    void showSmoothSavGolDialog();
    void showSmoothFFTDialog();
    void showSmoothAverageDialog();
    void showSmoothLowessDialog();
    void showInterpolationDialog();
    void showFitPolynomDialog();
    void showFunctionIntegrationDialog();
    void showDataSetDialog(int operation);
    void analyzeCurve(Graph *g,  QwtPlotCurve *c, int operation);
    void analysis(int operation);
    void integrate();
    void differentiate();
    void fitLinear();
    void fitSlope();
    void fitSigmoidal();
    void fitGauss();
    void fitLorentz();
    void deleteFitTables();
    void fitMultiPeakGauss();
    void fitMultiPeakLorentz();
    void fitMultiPeak(int profile);
    void subtractStraightLine();
    void subtractReferenceData();
    void baselineDialog();
    void saveFitFunctions(const QStringList& lst);
    void showFrequencyCountDialog();

private:
    ApplicationWindow *d_app;
};

#endif // ANALYSIS_CONTROLLER_H
