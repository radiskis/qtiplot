/***************************************************************************
    File                 : ProbabilityScaleEngine.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Engine for normal probability scales

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

#include <ProbabilityScaleEngine.h>
#include <gsl/gsl_cdf.h>

/*!
  Return a dummy transformation
*/
QwtScaleTransformation *ProbabilityScaleEngine::transformation() const
{
    return new QwtScaleTransformation(QwtScaleTransformation::Other);
}

/*!
    Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)
*/
void ProbabilityScaleEngine::autoScale(int,
    double &x1, double &x2, double &stepSize) const
{
    if ( x1 > x2 )
        qSwap(x1, x2);

    QwtDoubleInterval interval(1e-4, 99.999);

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = 1.0;

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if (testAttribute(QwtScaleEngine::Inverted)){
        qSwap(x1, x2);
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division

   \param x1 First interval limit
   \param x2 Second interval limit
   \param maxMajSteps Maximum for the number of major steps
   \param maxMinSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the scaleEngine
                   calculates one.
*/
QwtScaleDiv ProbabilityScaleEngine::divideScale(double x1, double x2,
    int, int, double stepSize) const
{
    QwtDoubleInterval interval = QwtDoubleInterval(x1, x2).normalized();
    interval = interval.limited(1e-4, 99.999);

    if (interval.width() <= 0 )
        return QwtScaleDiv();

    stepSize = fabs(qRound(stepSize));
    if ( stepSize == 0.0 )
        stepSize = 1.0;

    QwtScaleDiv scaleDiv;
    if ( stepSize != 0.0 ){
        QwtValueList ticks[QwtScaleDiv::NTickTypes];
		buildTicks(interval, (int)stepSize, ticks);
        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

void ProbabilityScaleEngine::buildTicks(
    const QwtDoubleInterval& interval, int stepSize,
    QwtValueList ticks[QwtScaleDiv::NTickTypes]) const
{
    ticks[QwtScaleDiv::MajorTick] = buildMajorTicks(interval, stepSize);
    ticks[QwtScaleDiv::MinorTick] = QwtValueList();

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
        ticks[i] = strip(ticks[i], interval);
}

QwtValueList ProbabilityScaleEngine::buildMajorTicks(
    const QwtDoubleInterval &interval, int stepSize) const
{
	QwtValueList baseTicks;

	baseTicks += 1e-4;
	baseTicks += 1e-3;
	baseTicks += 0.01;
	baseTicks += 0.1;
	baseTicks += 0.5;
	baseTicks += 1;
	baseTicks += 2;
	baseTicks += 5;
	for (int i = 1; i <= 5; i++)
		baseTicks += i*10;

	QwtValueList ticks;

	int size = baseTicks.size();
	for (int i = 0; i < size; i += stepSize)
		ticks += baseTicks[i];

	size = ticks.size();
	for (int i = 1; i < size; i++)
		ticks += 100 - ticks[size - i];

    return ticks;
}

//! Create a clone of the transformation
QwtScaleTransformation *ProbabilityScaleTransformation::copy() const
{
	return new ProbabilityScaleTransformation(d_engine);
}

double ProbabilityScaleTransformation::xForm(
    double s, double s1, double s2, double p1, double p2) const
{
	return p1 + (p2 - p1) * (func(s) - func(s1))/(func(s2) - func(s1));
}

double ProbabilityScaleTransformation::invXForm(double p, double p1, double p2,
    double s1, double s2) const
{
	return invFunc(func(s1) + (p - p1)/(p2 - p1)*(func(s2) - func(s1)));
}

double ProbabilityScaleTransformation::func(double x) const
{
	return gsl_cdf_ugaussian_Pinv(0.01*x);
}

double ProbabilityScaleTransformation::invFunc(double x) const
{
	return 100*gsl_cdf_ugaussian_P(x);
}
