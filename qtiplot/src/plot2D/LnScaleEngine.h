/***************************************************************************
    File                 : LnScaleEngine.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Engine for ln scales

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

#ifndef LN_SCALE_ENGINE_H
#define LN_SCALE_ENGINE_H

#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>

class LnScaleEngine: public QwtScaleEngine
{
public:
    virtual void autoScale(int maxSteps,
        double &x1, double &x2, double &stepSize) const;

    virtual QwtScaleDiv divideScale(double x1, double x2,
        int numMajorSteps, int numMinorSteps,
        double stepSize = 0.0) const;

    virtual QwtScaleTransformation *transformation() const;

protected:
    QwtDoubleInterval ln(const QwtDoubleInterval&) const;

private:
    QwtDoubleInterval align(const QwtDoubleInterval&,
        double stepSize) const;

    void buildTicks(
        const QwtDoubleInterval &, double stepSize, int maxMinSteps,
        QwtValueList ticks[QwtScaleDiv::NTickTypes]) const;

    QwtValueList buildMinorTicks(const QwtValueList& majorTicks,
        int maxMinMark, double step) const;

    QwtValueList buildMajorTicks(
        const QwtDoubleInterval &interval, double stepSize) const;
};

#endif
