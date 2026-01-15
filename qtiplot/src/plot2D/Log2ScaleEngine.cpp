/***************************************************************************
    File                 : Log2ScaleEngine.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Engine for log2 scales

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

#include <Log2ScaleEngine.h>

/*!
  Return a dummy transformation
*/
#include <qwt_transform.h>
#include <qwt_interval.h>
// #include <qwt_scale_arithmetic.h>
#include <cmath>

#ifndef LOG_MIN
#define LOG_MIN 1.0e-100
#endif

#ifndef LOG_MAX
#define LOG_MAX 1.0e100
#endif

/*!
  Return a dummy transformation
*/
QwtTransform *Log2ScaleEngine::transformation() const
{
    return new QwtLogTransform(); 
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

/*!
    Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)
*/
void Log2ScaleEngine::autoScale(int maxNumSteps,
    double &x1, double &x2, double &stepSize) const
{
    if ( x1 > x2 )
        qSwap(x1, x2);

    QwtInterval interval(x1 / exp(lowerMargin()),
        x2 * exp(upperMargin()) );

    double logRef = 1.0;
    if (reference() > LOG_MIN / 2)
        logRef = qMin(reference(), LOG_MAX / 2);

    if (testAttribute(QwtScaleEngine::Symmetric)){
        const double delta = qMax(interval.maxValue() / logRef,
            logRef / interval.minValue());
        interval.setInterval(logRef / delta, logRef * delta);
    }

    if (testAttribute(QwtScaleEngine::IncludeReference))
        interval = interval.extend(logRef);

    interval = interval.limited(LOG_MIN, LOG_MAX);

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = divideInterval(log2(interval).width(), qMax(maxNumSteps, 1));

    if (!testAttribute(QwtScaleEngine::Floating))
        interval = align(interval, stepSize);

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
QwtScaleDiv Log2ScaleEngine::divideScale(double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize) const
{
    QwtInterval interval = QwtInterval(x1, x2).normalized();
    interval = interval.limited(LOG_MIN, LOG_MAX);

    if (interval.width() <= 0 )
        return QwtScaleDiv();

    if (interval.maxValue() / interval.minValue() < 2){
        // scale width is less than 2 -> build linear scale
        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes(attributes());
        linearScaler.setReference(reference());
        linearScaler.setMargins(lowerMargin(), upperMargin());

        return linearScaler.divideScale(x1, x2,
            maxMajSteps, maxMinSteps, stepSize);
    }

    stepSize = qAbs(stepSize);
    if ( stepSize == 0.0 ){
        if ( maxMajSteps < 1 )
            maxMajSteps = 1;

		stepSize = ceil(log2(interval).width()/double(maxMajSteps));
    }

    QwtScaleDiv scaleDiv;
    if ( stepSize != 0.0 ){
        QList<double> ticks[QwtScaleDiv::NTickTypes];
		buildTicks(interval, stepSize, maxMinSteps, ticks);
        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

void Log2ScaleEngine::buildTicks(
    const QwtInterval& interval, double stepSize, int maxMinSteps,
    QList<double> ticks[QwtScaleDiv::NTickTypes]) const
{
    const QwtInterval boundingInterval = align(interval, stepSize);

    ticks[QwtScaleDiv::MajorTick] = buildMajorTicks(boundingInterval, stepSize);
    if ( maxMinSteps > 0 )
        ticks[QwtScaleDiv::MinorTick] = buildMinorTicks(ticks[QwtScaleDiv::MajorTick], maxMinSteps, stepSize);

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
        ticks[i] = strip(ticks[i], interval);
}

QList<double> Log2ScaleEngine::buildMajorTicks(
    const QwtInterval &interval, double stepSize) const
{
    double width = log2(interval).width();

    int numTicks = qRound(width / stepSize) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    const double lxmin = std::log2(interval.minValue());
    const double lxmax = std::log2(interval.maxValue());
    const double lstep = (lxmax - lxmin) / double(numTicks - 1);

    QList<double> ticks;
    ticks += interval.minValue();

    for (int i = 1; i < numTicks; i++)
       ticks += pow(2, lxmin + double(i) * lstep);

    ticks += interval.maxValue();

    return ticks;
}

QList<double> Log2ScaleEngine::buildMinorTicks(const QList<double> &majorTicks,
	int maxMinSteps, double) const
{
	if ( maxMinSteps < 1 )
		return QList<double>();

	int majTicks = (int)majorTicks.count();
    if (majTicks > 1){
        QList<double> minorTicks;
		for (int i = 0; i < majTicks - 1; i++){
			const double v = majorTicks[i];
            const double dv = fabs(majorTicks[i + 1] - majorTicks[i])/double(maxMinSteps - 1);
            for (int j = 0; j < maxMinSteps; j++)
                minorTicks += v + j*dv;
        }
        return minorTicks;
    }
    return QList<double>();
}

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
QwtInterval Log2ScaleEngine::align(
    const QwtInterval &interval, double stepSize) const
{
    const QwtInterval intv = log2(interval);

    const double x1 = floorEps(intv.minValue(), stepSize);
    const double x2 = ceilEps(intv.maxValue(), stepSize);

    return QwtInterval(pow(2, x1), pow(2, x2));
}

/*!
  Return the interval [log2(interval.minValue(), log2(interval.maxValue]
*/

QwtInterval Log2ScaleEngine::log2(
    const QwtInterval &interval) const
{
    return QwtInterval(std::log2(interval.minValue()),
            std::log2(interval.maxValue()));
}
