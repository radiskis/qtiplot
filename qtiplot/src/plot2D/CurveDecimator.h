#ifndef CURVE_DECIMATOR_H
#define CURVE_DECIMATOR_H

#include <QVector>
#include <QPointF>

//! High-performance O(N) curve decimation algorithms for large scientific datasets.
class CurveDecimator
{
public:
	enum Algorithm {
		NoDecimation = 0,
		LTTB = 1,
		MinMax = 2,
		DouglasPeucker = 3
	};

	//! Decimate using Largest Triangle Three Buckets (LTTB) algorithm in O(N) time.
	//! Preserves visual peaks, troughs, and data trends.
	static QVector<QPointF> decimateLTTB(const QVector<QPointF> &points, int targetPoints);
	static QVector<QPointF> decimateLTTB(const double *x, const double *y, int count, int targetPoints);

	//! Decimate using Min-Max algorithm in O(N) time.
	//! Preserves signal envelopes, extreme spikes, and bounds.
	static QVector<QPointF> decimateMinMax(const QVector<QPointF> &points, int targetPoints);
	static QVector<QPointF> decimateMinMax(const double *x, const double *y, int count, int targetPoints);
};

#endif // CURVE_DECIMATOR_H
