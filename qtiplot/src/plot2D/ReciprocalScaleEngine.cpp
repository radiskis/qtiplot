/***************************************************************************
    File                 : ReciprocalScaleEngine.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Return a transformation for reciprocal (1/t) scales

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

#include "ReciprocalScaleEngine.h"
#include <qwt_interval.h>
#include <qwt_transform.h>
#include <qwt_math.h>
#include <qmath.h>
#include <cmath>
#include <algorithm>
#include <limits>

static double qwtReciprocal(double value)
{
    if (value == 0.0)
        return std::numeric_limits<double>::max(); // Or some other appropriate handling for 1/0
    return 1.0 / value;
}

static double qwtReciprocalInverse(double value)
{
    if (value == std::numeric_limits<double>::max()) // Or other handling for 1/0
        return 0.0;
    return 1.0 / value;
}

static double floorEps(double value, double stepSize)
{
    if (stepSize == 0.0) return value;
    return std::floor(value / stepSize + 1.0e-9) * stepSize;
}

static double ceilEps(double value, double stepSize)
{
    if (stepSize == 0.0) return value;
    return std::ceil(value / stepSize - 1.0e-9) * stepSize;
}

static int compareEps(double v1, double v2, double intervalSize)
{
    const double eps = std::abs(1.0e-9 * intervalSize);
    if (v1 < v2 - eps) return -1;
    if (v1 > v2 + eps) return 1;
    return 0;
}

/*!
  Return a dummy transformation
*/
QwtTransform *ReciprocalScaleEngine::transformation() const
{
    return new ReciprocalScaleTransformation((const ScaleEngine *)this);
}

/*!
    Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)
*/
void ReciprocalScaleEngine::autoScale(int maxNumSteps,
    double &x1, double &x2, double &stepSize) const
{
    QwtInterval interval(x1, x2);
    interval = interval.normalized();

    interval.setMinValue(interval.minValue() - lowerMargin());
    interval.setMaxValue(interval.maxValue() + upperMargin());

    if (testAttribute(QwtScaleEngine::Symmetric))
        interval = interval.symmetrize(reference());

    if (testAttribute(QwtScaleEngine::IncludeReference))
        interval = interval.extend(reference());

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = divideInterval(interval.width(), qMax(maxNumSteps, 1));

    if ( !testAttribute(QwtScaleEngine::Floating) )
        interval = align(interval, stepSize);

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if (testAttribute(QwtScaleEngine::Inverted))
    {
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

   \sa QwtScaleEngine::stepSize(), QwtScaleEngine::subDivide()
*/
QwtScaleDiv ReciprocalScaleEngine::divideScale(double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize) const
{
    QwtInterval interval = QwtInterval(x1, x2).normalized();
    if (interval.width() <= 0 )
        return QwtScaleDiv();

    stepSize = qAbs(stepSize);
    if ( stepSize == 0.0 )
    {
        if ( maxMajSteps < 1 )
            maxMajSteps = 1;

        stepSize = divideInterval(interval.width(), maxMajSteps);
    }

    QwtScaleDiv scaleDiv;

    if ( stepSize != 0.0 )
    {
        QList<double> ticks[QwtScaleDiv::NTickTypes];
        buildTicks(interval, stepSize, maxMinSteps, ticks);

        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

void ReciprocalScaleEngine::buildTicks(
    const QwtInterval& interval, double stepSize, int maxMinSteps,
    QList<double> ticks[QwtScaleDiv::NTickTypes]) const
{
    const QwtInterval boundingInterval =
        align(interval, stepSize);

    ticks[QwtScaleDiv::MajorTick] =
        buildMajorTicks(boundingInterval, stepSize);

    if ( maxMinSteps > 0 )
    {
        buildMinorTicks(ticks[QwtScaleDiv::MajorTick], maxMinSteps, stepSize,
            ticks[QwtScaleDiv::MinorTick], ticks[QwtScaleDiv::MediumTick]);
    }

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        ticks[i] = strip(ticks[i], interval);

        // ticks very close to 0.0 are
        // explicitely set to 0.0

        for ( int j = 0; j < (int)ticks[i].count(); j++ )
        {
            if ( compareEps(ticks[i][j], 0.0, stepSize) == 0 )
                ticks[i][j] = 0.0;
        }
    }
}

QList<double> ReciprocalScaleEngine::buildMajorTicks(
    const QwtInterval &interval, double stepSize) const
{
    int numTicks = qRound(interval.width() / stepSize) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    QList<double> ticks;

    ticks += interval.minValue();
    for (int i = 1; i < numTicks - 1; i++)
        ticks += interval.minValue() + i * stepSize;
    ticks += interval.maxValue();

    return ticks;
}

void ReciprocalScaleEngine::buildMinorTicks(
    const QList<double>& majorTicks,
    int maxMinSteps, double stepSize,
    QList<double> &minorTicks,
    QList<double> &mediumTicks) const
{
    double minStep = divideInterval(stepSize, maxMinSteps);
    if (minStep == 0.0)
        return;

    // # ticks per interval
    int numTicks = (int)::ceil(qAbs(stepSize / minStep)) - 1;

    // Do the minor steps fit into the interval?
    if ( compareEps((numTicks +  1) * qAbs(minStep),
        qAbs(stepSize), stepSize) > 0)
    {
        numTicks = 1;
        minStep = stepSize * 0.5;
    }

    int medIndex = -1;
    if ( numTicks % 2 )
        medIndex = numTicks / 2;

    // calculate minor ticks

    for (int i = 0; i < (int)majorTicks.count(); i++)
    {
        double val = majorTicks[i];
        for (int k = 0; k < numTicks; k++)
        {
            val += minStep;

            double alignedValue = val;
            if (compareEps(val, 0.0, stepSize) == 0)
                alignedValue = 0.0;

            if ( k == medIndex )
                mediumTicks += alignedValue;
            else
                minorTicks += alignedValue;
        }
    }
}

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
QwtInterval ReciprocalScaleEngine::align(
    const QwtInterval &interval, double stepSize) const
{
    const double x1 = floorEps(interval.minValue(), stepSize);
    const double x2 = ceilEps(interval.maxValue(), stepSize);

    return QwtInterval(x1, x2);
}

//! Create a clone of the transformation
QwtTransform *ReciprocalScaleTransformation::copy() const
{
	return new ReciprocalScaleTransformation(d_engine);
}

double ReciprocalScaleTransformation::transform(double s) const
{
	return 1.0/s;
}

double ReciprocalScaleTransformation::invTransform(double p) const
{
	return 1.0/p;
}

