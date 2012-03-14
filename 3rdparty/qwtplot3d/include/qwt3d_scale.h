#ifndef qwt3d_scale_h__2004_06_02_22_02_begin_guarded_code
#define qwt3d_scale_h__2004_06_02_22_02_begin_guarded_code

#include <qstring.h>
#include "qwt3d_types.h"
#include "qwt3d_autoscaler.h"
#include "qwt3d_autoptr.h"

namespace Qwt3D
{

/*! 
The class encapsulates non-visual scales. 
She is utilized by Axis and also collaborates closely with AutoScaler.
A Scale allows control over all aspects of tic generation including 
arbitrary transformations of tic values into corresponding strings. 
The strings contain what eventually will be shown as tic labels.\n
Standard linear and logarithmic scales have been integrated yet into the Axis 
interface. User-defined axes can be derived from Scale, LinearScale et al.
*/
class QWT3D_EXPORT Scale
{
	friend class Axis;
	friend class qwt3d_ptr<Scale>;

public:
	enum NumericFormat{Default, Decimal, Scientific, Engineering};

	int numericPrecision(){return d_numeric_precision;};//!< Returns numeric precision for tick labels
	NumericFormat numericFormat(){return d_numeric_format;};//!< Returns numeric format for tick labels
	void setNumericFormat(const NumericFormat& fmt, int prec){d_numeric_format = fmt; d_numeric_precision = prec;};//!< Set numeric format and precision for tick labels

protected:
	Scale();
	virtual ~Scale(){}
	QString ticLabel(unsigned int idx) const;
	double ticValue(unsigned int idx) const;

	virtual void setLimits(double start, double stop);
	virtual void setMajors(int val) {majorintervals_p=val;} //!< Sets number of major intervals
	virtual void setMinors(int val) {minorintervals_p=val;} //!< Sets number of minor intervals per major interval
	virtual void setMajorLimits(double start, double stop);

	int majors() const {return majorintervals_p;} //!< Returns major intervals
	int minors() const {return minorintervals_p;} //!< Returns minor intervals

	//! Derived classes should return a new heap based object here.
	virtual Scale* clone() const = 0;
	//! This function should setup the 2 vectors for major and minor positions;
	virtual void calculate() = 0;
	virtual int autoscale(double& a, double& b, double start, double stop, int ivals);
	virtual double transform(double x, double start, double stop) const = 0;
	virtual double transformRatio(double x) const = 0;

	static std::vector<double> strip(const std::vector<double>& ticks, double start, double stop);
	static double divideInterval(double intervalSize, int numSteps);
	static int compareEps(double value1, double value2, double intervalSize);
	static double ceilEps(double value, double intervalSize);
	static double floorEps(double value, double intervalSize);
	static double divideEps(double interval, double steps);
	static double ceil125(double x);

	std::vector<double> majors_p, minors_p;
	double start_p, stop_p;
	int majorintervals_p, minorintervals_p;
	double mstart_p, mstop_p;

private:
	void destroy() const {delete this;} //!< Used by qwt3d_ptr
	int d_numeric_precision;
	NumericFormat d_numeric_format;
};

//! The standard (1:1) mapping class for axis numbering
class QWT3D_EXPORT LinearScale : public Scale
{
	friend class Axis;
	friend class qwt3d_ptr<Scale>;
protected:
	int autoscale(double& a, double& b, double start, double stop, int ivals);
	//! Returns a new heap based object utilized from qwt3d_ptr
	Scale* clone() const {return new LinearScale(*this);}
	void calculate();
	double transform(double x, double, double) const {return x;}
	double transformRatio(double x) const {return (x - start_p)/(stop_p - start_p);}
	LinearAutoScaler autoscaler_p;

private:
	std::vector<double> buildMajorTicks(double minValue, double maxValue, double stepSize);
	std::vector<double> buildMinorTicks(const std::vector<double> &majorTicks, int maxMinSteps, double stepSize) const;
	static void align(double *minValue, double *maxValue, double stepSize);
};

//! log10 scale
class QWT3D_EXPORT LogScale : public Scale
{
	friend class Axis;
	friend class qwt3d_ptr<Scale>;
protected:
	void setMinors(int val);
	//! Standard ctor
	LogScale();
	//! Returns a new heap based object utilized from qwt3d_ptr
	Scale* clone() const {return new LogScale;}
	void calculate();
	double transform(double x, double start, double stop) const {return x <= 0.0 ? 1e-150 : start + (stop - start)*log10(x/start)/log10(stop/start);}
	double transformRatio(double x) const {return x <= 0.0 ? 1e-150 : log10(x/start_p)/log10(stop_p/start_p);}

private:
	std::vector<double> buildMajorTicks(double minValue, double maxValue, double stepSize);
	std::vector<double> buildMinorTicks(const std::vector<double> &majorTicks, int maxMinSteps, double stepSize) const;
	static void align(double *minValue, double *maxValue, double stepSize);
};

} // namespace Qwt3D


#endif /* include guarded */
