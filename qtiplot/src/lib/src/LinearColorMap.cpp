/***************************************************************************
File                 : LinearColorMap.cpp
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2011 by Ion Vasilief
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : A wrapper around QwtLinearColorMap from Qwt
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
#include "LinearColorMap.h"

LinearColorMap::LinearColorMap() : QwtLinearColorMap(),
d_range(QwtInterval())
{}

LinearColorMap::LinearColorMap(const QColor &from, const QColor &to) : QwtLinearColorMap(from, to),
d_range(QwtInterval())
{}

LinearColorMap::LinearColorMap(const LinearColorMap& other) : QwtLinearColorMap(other.color1(), other.color2(), other.format()),
d_range(other.intensityRange())
{
	QVector<double> stops = other.colorStops();
	for (int i = 1; i < stops.size() - 1; i++)
		addColorStop(stops[i], QColor(other.rgb(QwtInterval(0, 1), stops[i])));
}

LinearColorMap& LinearColorMap::operator=(const LinearColorMap& other)
{
	if (this != &other) {
		setColorInterval(other.color1(), other.color2());
		setMode(other.mode());
		setFormat(other.format());
		d_range = other.intensityRange();
		QVector<double> stops = other.colorStops();
		for (int i = 1; i < stops.size() - 1; i++)
			addColorStop(stops[i], QColor(other.rgb(QwtInterval(0, 1), stops[i])));
	}
	return *this;
}

QwtInterval LinearColorMap::intensityRange() const
{
	return d_range;
}

QColor LinearColorMap::color(int index) const
{
	QVector<double> stops = colorStops();
	if (index <= 0)
		return color1();
	if (index >= (int)stops.size() - 1)
		return color2();

	return QwtColorMap::color(QwtInterval(0, 1), stops[index]);
}

QString LinearColorMap::toXmlString()
{
	QString s = "<ColorMap>\n";
	s += "\t<Mode>" + QString::number(mode()) + "</Mode>\n";
	s += "\t<MinColor>" + color1().name() + "</MinColor>\n";
	s += "\t<MaxColor>" + color2().name() + "</MaxColor>\n";
	if (d_range.isValid())
		s += "\t<Range>" + QString::number(d_range.minValue(), 'g', 15) + "\t" + QString::number(d_range.maxValue(), 'g', 15) + "</Range>\n";
	QVector <double> colors = colorStops();
	int stops = (int)colors.size();
	s += "\t<ColorStops>" + QString::number(stops - 2) + "</ColorStops>\n";
	for (int i = 1; i < stops - 1; i++){
		s += "\t<Stop>" + QString::number(colors[i], 'g', 15) + "\t";
		s += color(i).name();
		s += "</Stop>\n";
	}
	return s += "</ColorMap>\n";
}

LinearColorMap LinearColorMap::fromXmlStringList(const QStringList& lst)
{
	QStringList::const_iterator line = lst.begin();
	QString s = (*line).trimmed();

	int mode = s.remove("<Mode>").remove("</Mode>").trimmed().toInt();
	s = *(++line);
	QColor color1 = QColor(s.remove("<MinColor>").remove("</MinColor>").trimmed());
	s = *(++line);
	QColor color2 = QColor(s.remove("<MaxColor>").remove("</MaxColor>").trimmed());

	LinearColorMap colorMap = LinearColorMap(color1, color2);
	colorMap.setMode((QwtLinearColorMap::Mode)mode);

	s = *(++line);
	if (s.contains("<Range>")){
		QStringList l = s.trimmed().remove("<Range>").remove("</Range>").split("\t");
		if (l.size() == 2)
			colorMap.setIntensityRange(QwtInterval(l[0].toDouble(), l[1].toDouble()));
		 s = *(++line);
	}

	int stops = s.remove("<ColorStops>").remove("</ColorStops>").trimmed().toInt();
	for (int i = 0; i < stops; i++){
		s = (*(++line)).trimmed();
		QStringList l = s.remove("<Stop>").remove("</Stop>").split("\t");
		if (l.size() >= 2)
			colorMap.addColorStop(l[0].toDouble(), QColor(l[1]));
	}

	return colorMap;
}

LinearColorMap LinearColorMap::rainbow()
{
	LinearColorMap colorMap = LinearColorMap(Qt::black, Qt::white);
	colorMap.addColorStop(0.0, Qt::blue);
	colorMap.addColorStop(0.125, QColor(0, 128, 255));
	colorMap.addColorStop(0.25, Qt::cyan);
	colorMap.addColorStop(0.375, QColor(0, 255, 128));
	colorMap.addColorStop(0.5, Qt::green);
	colorMap.addColorStop(0.625, Qt::yellow);
	colorMap.addColorStop(0.75, QColor(255, 128, 0));
	colorMap.addColorStop(0.875, Qt::red);
	return colorMap;
}
