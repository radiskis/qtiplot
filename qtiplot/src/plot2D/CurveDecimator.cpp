#include "CurveDecimator.h"
#include <cmath>

QVector<QPointF> CurveDecimator::decimateLTTB(const QVector<QPointF> &points, int targetPoints)
{
	int n = points.size();
	if (n <= targetPoints || targetPoints <= 2)
		return points;

	QVector<QPointF> sampled;
	sampled.reserve(targetPoints);

	double bucketSize = (double)(n - 2) / (double)(targetPoints - 2);

	sampled.append(points[0]);
	int a = 0;

	for (int i = 0; i < targetPoints - 2; ++i) {
		int rangeOffs = (int)std::floor((i + 0) * bucketSize) + 1;
		int rangeTo   = (int)std::floor((i + 1) * bucketSize) + 1;
		if (rangeTo > n - 1)
			rangeTo = n - 1;

		int nextOffs = (int)std::floor((i + 1) * bucketSize) + 1;
		int nextTo   = (int)std::floor((i + 2) * bucketSize) + 1;
		if (nextTo > n)
			nextTo = n;

		double avgX = 0.0;
		double avgY = 0.0;
		int nextCount = nextTo - nextOffs;
		if (nextCount > 0) {
			for (int j = nextOffs; j < nextTo; ++j) {
				avgX += points[j].x();
				avgY += points[j].y();
			}
			avgX /= (double)nextCount;
			avgY /= (double)nextCount;
		} else {
			avgX = points[n - 1].x();
			avgY = points[n - 1].y();
		}

		double pointAX = points[a].x();
		double pointAY = points[a].y();

		double maxArea = -1.0;
		int maxAreaIndex = rangeOffs;

		for (int j = rangeOffs; j < rangeTo; ++j) {
			double bx = points[j].x();
			double by = points[j].y();

			double area = std::abs((pointAX - avgX) * (by - pointAY) - (pointAX - bx) * (avgY - pointAY));
			if (area > maxArea) {
				maxArea = area;
				maxAreaIndex = j;
			}
		}

		sampled.append(points[maxAreaIndex]);
		a = maxAreaIndex;
	}

	sampled.append(points[n - 1]);
	return sampled;
}

QVector<QPointF> CurveDecimator::decimateLTTB(const double *x, const double *y, int count, int targetPoints)
{
	if (!x || !y || count <= 0)
		return QVector<QPointF>();

	if (count <= targetPoints || targetPoints <= 2) {
		QVector<QPointF> pts;
		pts.reserve(count);
		for (int i = 0; i < count; ++i)
			pts.append(QPointF(x[i], y[i]));
		return pts;
	}

	QVector<QPointF> sampled;
	sampled.reserve(targetPoints);

	double bucketSize = (double)(count - 2) / (double)(targetPoints - 2);

	sampled.append(QPointF(x[0], y[0]));
	int a = 0;

	for (int i = 0; i < targetPoints - 2; ++i) {
		int rangeOffs = (int)std::floor((i + 0) * bucketSize) + 1;
		int rangeTo   = (int)std::floor((i + 1) * bucketSize) + 1;
		if (rangeTo > count - 1)
			rangeTo = count - 1;

		int nextOffs = (int)std::floor((i + 1) * bucketSize) + 1;
		int nextTo   = (int)std::floor((i + 2) * bucketSize) + 1;
		if (nextTo > count)
			nextTo = count;

		double avgX = 0.0;
		double avgY = 0.0;
		int nextCount = nextTo - nextOffs;
		if (nextCount > 0) {
			for (int j = nextOffs; j < nextTo; ++j) {
				avgX += x[j];
				avgY += y[j];
			}
			avgX /= (double)nextCount;
			avgY /= (double)nextCount;
		} else {
			avgX = x[count - 1];
			avgY = y[count - 1];
		}

		double pointAX = x[a];
		double pointAY = y[a];

		double maxArea = -1.0;
		int maxAreaIndex = rangeOffs;

		for (int j = rangeOffs; j < rangeTo; ++j) {
			double bx = x[j];
			double by = y[j];

			double area = std::abs((pointAX - avgX) * (by - pointAY) - (pointAX - bx) * (avgY - pointAY));
			if (area > maxArea) {
				maxArea = area;
				maxAreaIndex = j;
			}
		}

		sampled.append(QPointF(x[maxAreaIndex], y[maxAreaIndex]));
		a = maxAreaIndex;
	}

	sampled.append(QPointF(x[count - 1], y[count - 1]));
	return sampled;
}

QVector<QPointF> CurveDecimator::decimateMinMax(const QVector<QPointF> &points, int targetPoints)
{
	int n = points.size();
	if (n <= targetPoints || targetPoints <= 3)
		return points;

	QVector<QPointF> sampled;
	sampled.reserve(targetPoints);

	sampled.append(points[0]);

	int numBuckets = (targetPoints - 2) / 2;
	if (numBuckets < 1)
		numBuckets = 1;

	double bucketSize = (double)(n - 2) / (double)numBuckets;

	for (int i = 0; i < numBuckets; ++i) {
		int rangeOffs = (int)std::floor(i * bucketSize) + 1;
		int rangeTo   = (int)std::floor((i + 1) * bucketSize) + 1;
		if (rangeTo > n - 1)
			rangeTo = n - 1;
		if (rangeOffs >= rangeTo)
			continue;

		int minIdx = rangeOffs;
		int maxIdx = rangeOffs;
		double minY = points[rangeOffs].y();
		double maxY = points[rangeOffs].y();

		for (int j = rangeOffs + 1; j < rangeTo; ++j) {
			double y = points[j].y();
			if (y < minY) {
				minY = y;
				minIdx = j;
			}
			if (y > maxY) {
				maxY = y;
				maxIdx = j;
			}
		}

		if (minIdx < maxIdx) {
			sampled.append(points[minIdx]);
			sampled.append(points[maxIdx]);
		} else if (minIdx > maxIdx) {
			sampled.append(points[maxIdx]);
			sampled.append(points[minIdx]);
		} else {
			sampled.append(points[minIdx]);
		}
	}

	sampled.append(points[n - 1]);
	return sampled;
}

QVector<QPointF> CurveDecimator::decimateMinMax(const double *x, const double *y, int count, int targetPoints)
{
	if (!x || !y || count <= 0)
		return QVector<QPointF>();

	if (count <= targetPoints || targetPoints <= 3) {
		QVector<QPointF> pts;
		pts.reserve(count);
		for (int i = 0; i < count; ++i)
			pts.append(QPointF(x[i], y[i]));
		return pts;
	}

	QVector<QPointF> sampled;
	sampled.reserve(targetPoints);

	sampled.append(QPointF(x[0], y[0]));

	int numBuckets = (targetPoints - 2) / 2;
	if (numBuckets < 1)
		numBuckets = 1;

	double bucketSize = (double)(count - 2) / (double)numBuckets;

	for (int i = 0; i < numBuckets; ++i) {
		int rangeOffs = (int)std::floor(i * bucketSize) + 1;
		int rangeTo   = (int)std::floor((i + 1) * bucketSize) + 1;
		if (rangeTo > count - 1)
			rangeTo = count - 1;
		if (rangeOffs >= rangeTo)
			continue;

		int minIdx = rangeOffs;
		int maxIdx = rangeOffs;
		double minY = y[rangeOffs];
		double maxY = y[rangeOffs];

		for (int j = rangeOffs + 1; j < rangeTo; ++j) {
			double valY = y[j];
			if (valY < minY) {
				minY = valY;
				minIdx = j;
			}
			if (valY > maxY) {
				maxY = valY;
				maxIdx = j;
			}
		}

		if (minIdx < maxIdx) {
			sampled.append(QPointF(x[minIdx], y[minIdx]));
			sampled.append(QPointF(x[maxIdx], y[maxIdx]));
		} else if (minIdx > maxIdx) {
			sampled.append(QPointF(x[maxIdx], y[maxIdx]));
			sampled.append(QPointF(x[minIdx], y[minIdx]));
		} else {
			sampled.append(QPointF(x[minIdx], y[minIdx]));
		}
	}

	sampled.append(QPointF(x[count - 1], y[count - 1]));
	return sampled;
}
