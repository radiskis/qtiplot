#include "qwt3d_scale.h"
#include <QLocale>

using namespace Qwt3D;

Scale::Scale() 
: start_p(0.), stop_p(0.), 
  majorintervals_p(0), minorintervals_p(0),
  mstart_p(0.), mstop_p(0.),
  d_numeric_precision(6),
  d_numeric_format(Default)
{
}

/*! The function maps the double value at tic-position idx to a final
representation. The default return value is simply the tic values QString 
representation. Overwrite this function, if you plan to transform the value 
in some way. See e.g. LogScale::ticLabel.
\param idx the current major tic index
\return The QString representation for the value corresponding to a valid index, 
an empty QString else.
*/
QString Scale::ticLabel(unsigned int idx) const
{
	if (idx >= majors_p.size())
		return QString::null;

	double val = majors_p[idx];
	QLocale locale = QLocale();
	switch (d_numeric_format){
		case Default:
			return locale.toString(val, 'g', d_numeric_precision);
		break;

		case Decimal:
			return locale.toString(val, 'f', d_numeric_precision);
		break;

		case Scientific:
			return locale.toString(val, 'e', d_numeric_precision);
		break;

		case Engineering:{
			QString eng_suff;
			double new_value = val;

			if(fabs(new_value) >= 1e18){
				eng_suff = 'E';
				new_value /= 1e18;
			} else if(fabs(new_value) >= 1e15){
				eng_suff = 'P';
				new_value /= 1e15;
			} else if(fabs(new_value) >= 1e12){
				eng_suff = 'T';
				new_value /= 1e12;
			} else if(fabs(new_value) >= 1e9){
				eng_suff = 'G';
				new_value /= 1e9;
			} else if(fabs(new_value) >= 1e6){
				eng_suff = 'M';
				new_value /= 1e6;
			} else if(fabs(new_value) >= 1e3){
				eng_suff = 'k';
				new_value /= 1e3;
			} else if(fabs(new_value) >= 1){
				eng_suff = "";
				new_value /= 1.0;
			} else if(fabs(new_value) >= 1e-3){
				eng_suff = 'm';
				new_value /= 1e-3;
			} else if(fabs(new_value) >= 1e-6){
				eng_suff = 'µ';
				new_value /= 1e-6;
			} else if(fabs(new_value) >= 1e-9){
				eng_suff = 'n';
				new_value /= 1e-9;
			} else if(fabs(new_value) >= 1e-12){
				eng_suff = 'p';
				new_value /= 1e-12;
			} else if(fabs(new_value) >= 1e-15){
				eng_suff = 'f';
				new_value /= 1e-15;
			} else {
				eng_suff = 'a';
				new_value /= 1e-18;
			}

			QString txt = locale.toString((new_value), 'f', d_numeric_precision);
			if(txt.contains(QRegExp("^0[\\.,]?0*$")))
				return "0";

			return txt + eng_suff;
		}
		break;
	}
	return QString::null;
}

double Scale::ticValue(unsigned int idx) const
{
	if (idx < majors_p.size())
		return majors_p[idx];

	return 0.0;
}

//! Sets start and stop value for the scale;
void Scale::setLimits(double start, double stop) 
{
	if (start < stop){
		start_p = start;
		stop_p = stop;
		return;
	}
	start_p = stop;
	stop_p = start;
}

//! Sets value of first major tic
void Scale::setMajorLimits(double start, double stop) 
{
	if (start < stop){
		mstart_p = start;
		mstop_p = stop;
		return;
	}
	mstart_p = stop;
	mstop_p = start;
} 

/*!
  \param a First major tic after applying autoscaling
  \param b Last major tic after applying autoscaling
  \param start Scale begin
  \param stop Scale end
  \param ivals Requested number of major intervals
  \return Number of major intervals after autoscaling\n
  
  The default implementation sets a=start, b=stop and returns ivals.
*/
int Scale::autoscale(double& a, double& b, double start, double stop, int ivals)
{
	a = start;
	b = stop;
	return ivals;
}

/***************************
*
* linear scales
*
***************************/


//! Applies LinearAutoScaler::execute()
int LinearScale::autoscale(double& a, double& b, double start, double stop, int ivals)
{
	return autoscaler_p.execute(a, b, start, stop, ivals);
}

//! Creates the major and minor vector for the scale
void LinearScale::calculate()
{
	majors_p.clear();
	minors_p.clear();

	if (!majorintervals_p)
		return;

	double interval = stop_p - start_p;
	if (interval <= 0)
		return;

	double stepSize = divideInterval(fabs(interval), majorintervals_p + 1);
	majors_p = buildMajorTicks(start_p, stop_p, stepSize);
	majorintervals_p = (int)majors_p.size();
	for (int i = 0; i < majorintervals_p; i++){
		if (compareEps(majors_p[i], 0.0, stepSize) == 0)
			majors_p[i] = 0.0;
	}
	majorintervals_p = majors_p.size();
	if (majorintervals_p >= 1)
		minors_p = strip(buildMinorTicks(majors_p, minorintervals_p, stepSize), start_p, stop_p);

	majors_p = strip(majors_p, start_p, stop_p);
	majorintervals_p = majors_p.size();
}

/*! Creates major and minor vectors for the scale.
\warning If the interval is too small, the scale becomes empty
or will contain only a single major tic. There is no automatism 
(also not planned for now) for an 'intelligent' guess, what to do. 
Better switch manually to linear to scales in such cases.
*/
void LogScale::calculate()
{
	majors_p.clear();
	minors_p.clear();

	if (majorintervals_p < 1) // not even a single major tic
		return;

	if (start_p < DBL_MIN_10_EXP)
		start_p = DBL_MIN_10_EXP;

	double interval = stop_p - start_p;
	if (interval <= 0)
		return;

	double stepSize = divideInterval(fabs(log10(start_p) - log10(stop_p)), majorintervals_p + 1);
	if (stepSize < 1.0)
		stepSize = 1.0; // major step must be >= 1 decade

	majors_p = buildMajorTicks(start_p, stop_p, stepSize);
	majorintervals_p = majors_p.size();
	if (majorintervals_p >= 1)
		minors_p = strip(buildMinorTicks(majors_p, minorintervals_p - 1, stepSize), start_p, stop_p);

	majors_p = strip(majors_p, start_p, stop_p);
	majorintervals_p = majors_p.size();
}

/*!
Sets the minor intervals for the logarithmic scale. Only values of 9,5,3 or 2 
are accepted as arguments. They will produce mantissa sets of {2,3,4,5,6,7,8,9}, 
{2,4,6,8}, {2,5} or {5} respectively.
*/
void LogScale::setMinors(int val)
{
	if ((val == 2) || (val == 3) || (val == 5) || (val == 9))
		minorintervals_p = val;
	else if (val > 9)
		minorintervals_p = 9;
	else if (val < 2)
		minorintervals_p = 0;
}

LogScale::LogScale()
{
	minorintervals_p = 9;
}

//The code bellow was adapted from Qwt library (http://qwt.sourceforge.net/)
static const double _eps = 1.0e-6;

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
void LinearScale::align(double *minValue, double *maxValue, double stepSize)
{
	double x1 = floorEps(*minValue, stepSize);
	if (compareEps(*minValue, x1, stepSize) == 0)
		x1 = *minValue;

	double x2 = ceilEps(*maxValue, stepSize);
	if (compareEps(*maxValue, x2, stepSize) == 0)
		x2 = *maxValue;

	*minValue = x1;
	*maxValue = x2;
}

std::vector<double> LinearScale::buildMajorTicks(double minValue, double maxValue, double stepSize)
{
	double start = minValue, stop = maxValue;
	align(&minValue, &maxValue, stepSize);

	int numTicks = qRound(fabs(maxValue - minValue)/stepSize) + 1;
	if (numTicks > 100)
		numTicks = 100;

	std::vector<double> ticks;
	ticks.push_back(minValue);
	for (int i = 0; i < numTicks; i++){
		double val = minValue + i*stepSize;
		if (val >= start && val <= stop)
			ticks.push_back(val);
	}
	if (ticks.back() != maxValue)
		ticks.push_back(maxValue);
	return ticks;
}

std::vector<double> LinearScale::buildMinorTicks(const std::vector<double> &majorTicks, int maxMinSteps, double stepSize) const
{
	std::vector<double> minorTicks;
	if (maxMinSteps < 1)
		return minorTicks;

	double minStep = stepSize/(double)maxMinSteps;
	if (minStep == 0.0)
		return minorTicks;

	int numTicks = (int)::ceil(qAbs(stepSize/minStep));//ticks per interval
	int majors = (int)majorTicks.size() - 1;
	for (int i = 0; i < majors; i++){
		double val = majorTicks[i];
		for (int k = 0; k < numTicks; k++){
			val += minStep;
			double alignedValue = val;
			if (compareEps(val, 0.0, stepSize) == 0)
				alignedValue = 0.0;
			minorTicks.push_back(alignedValue);
		}
	}
	return minorTicks;
}

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
void LogScale::align(double *minValue, double *maxValue, double stepSize)
{
	const double lxmin = log10(*minValue);
	double x1 = floorEps(lxmin, stepSize);
	if (compareEps(lxmin, x1, stepSize) == 0)
		x1 = lxmin;

	const double lxmax = log10(*maxValue);
	double x2 = ceilEps(lxmax, stepSize);
	if (compareEps(lxmax, x2, stepSize) == 0)
		x2 = lxmax;

	*minValue = pow(10.0, x1);
	*maxValue = pow(10.0, x2);
}

std::vector<double> LogScale::buildMajorTicks(double minValue, double maxValue, double stepSize)
{
	double start = minValue, stop = maxValue;
	align(&minValue, &maxValue, stepSize);

	const double lxmin = log10(minValue);
	const double lxmax = log10(maxValue);
	int numTicks = qRound(fabs(lxmax - lxmin)/stepSize) + 1;
	if (numTicks > 100)
		numTicks = 100;

	const double lstep = fabs(lxmax - lxmin)/double(numTicks - 1);

	std::vector<double> ticks;
	ticks.push_back(minValue);
	for (int i = 0; i < numTicks; i++){
		double val = pow(10, lxmin + double(i) * lstep);
		if (val >= start && val <= stop)
			ticks.push_back(val);
	}
	if (ticks.back() != maxValue)
		ticks.push_back(maxValue);
	return ticks;
}

std::vector<double> LogScale::buildMinorTicks(const std::vector<double> &majorTicks, int maxMinSteps, double stepSize) const
{
	std::vector<double> minorTicks;
	if (maxMinSteps < 1)
		return minorTicks;

	int majors = (int)majorTicks.size() - 1;
	if (stepSize < 1.1){// major step width is one decade
		int k0, kstep, kmax;

		if (maxMinSteps >= 8){
			k0 = 2;
			kmax = 9;
			kstep = 1;
		} else if (maxMinSteps >= 4){
			k0 = 2;
			kmax = 8;
			kstep = 2;
		} else if (maxMinSteps >= 2){
			k0 = 2;
			kmax = 5;
			kstep = 3;
		} else {
			k0 = 5;
			kmax = 5;
			kstep = 1;
		}

		for (int i = 0; i < majors; i++){
			const double v = majorTicks[i];
			for (int k = k0; k <= kmax; k += kstep)
				minorTicks.push_back(v * double(k));
		}
		return minorTicks;
	} else {// major step > one decade
		double minStep = divideInterval(stepSize, maxMinSteps);
		if (minStep == 0.0)
			return minorTicks;

		if (minStep < 1.0)
			minStep = 1.0;

		// # subticks per interval
		int nMin = qRound(stepSize / minStep) - 1;

		// Do the minor steps fit into the interval?

		if (compareEps((nMin +  1) * minStep, qAbs(stepSize), stepSize) > 0)
			nMin = 0;

		if (nMin < 1)
			return minorTicks;// no subticks

		// substep factor = 10^substeps
		const double minFactor = qMax(pow(10.0, minStep), 10.0);

		for (int i = 0; i < majors; i++){
			double val = majorTicks[i];
			for (int k = 0; k < nMin; k++){
				val *= minFactor;
				minorTicks.push_back(val);
			}
		}
		return minorTicks;
	}
}

/*!
  Remove ticks from a list, that are not inside an interval

  \param ticks Tick list
  \param start Interval start
  \param stop Interval end

  \return Stripped tick list
*/
std::vector<double> Scale::strip(const std::vector<double>& ticks, double start, double stop)
{
	std::vector<double> strippedTicks;
	int n = (int)ticks.size();
	if (n == 0)
		return strippedTicks;

	for (int i = 0; i < n; i++){
		double v = ticks[i];
		if (v >= start && v <= stop)
			strippedTicks.push_back(v);
	}
	return strippedTicks;
}

/*!
  Calculate a step size for an interval size

  \param intervalSize Interval size
  \param numSteps Number of steps

  \return Step size
*/
double Scale::divideInterval(double intervalSize, int numSteps)
{
	if (numSteps <= 0)
		return 0.0;

	double v = divideEps(intervalSize, numSteps);
	return ceil125(v);
}

/*!
  \brief Compare 2 values, relative to an interval

  Values are "equal", when :
  \f$\cdot value2 - value1 <= abs(intervalSize * 10e^{-6})\f$

  \param value1 First value to compare
  \param value2 Second value to compare
  \param intervalSize interval size

  \return 0: if equal, -1: if value2 > value1, 1: if value1 > value2
*/
int Scale::compareEps(double value1, double value2, double intervalSize)
{
	const double eps = qAbs(_eps * intervalSize);

	if ( value2 - value1 > eps )
		return -1;

	if ( value1 - value2 > eps )
		return 1;

	return 0;
}

/*!
  Ceil a value, relative to an interval

  \param value Value to ceil
  \param intervalSize Interval size

  \sa floorEps()
*/
double Scale::ceilEps(double value, double intervalSize)
{
	const double eps = _eps * intervalSize;

	value = (value - eps) / intervalSize;
	return ceil(value) * intervalSize;
}

/*!
  Floor a value, relative to an interval

  \param value Value to floor
  \param intervalSize Interval size

  \sa floorEps()
*/
double Scale::floorEps(double value, double intervalSize)
{
	const double eps = _eps * intervalSize;

	value = (value + eps) / intervalSize;
	return floor(value) * intervalSize;
}

/*!
  \brief Divide an interval into steps

  \f$stepSize = (intervalSize - intervalSize * 10e^{-6}) / numSteps\f$

  \param intervalSize Interval size
  \param numSteps Number of steps
  \return Step size
*/
double Scale::divideEps(double intervalSize, double numSteps)
{
	if ( numSteps == 0.0 || intervalSize == 0.0 )
		return 0.0;

	return (intervalSize - (_eps * intervalSize)) / numSteps;
}

/*!
  Find the smallest value out of {1,2,5}*10^n with an integer number n
  which is greater than or equal to x

  \param x Input value
*/
double Scale::ceil125(double x)
{
	if (x == 0.0)
		return 0.0;

	const double sign = (x > 0) ? 1.0 : -1.0;
	const double lx = log10(fabs(x));
	const double p10 = floor(lx);

	double fr = pow(10.0, lx - p10);
	if (fr <=1.0)
		fr = 1.0;
	else if (fr <= 2.0)
		fr = 2.0;
	else if (fr <= 5.0)
		fr = 5.0;
	else
		fr = 10.0;

	return sign * fr * pow(10.0, p10);
}
