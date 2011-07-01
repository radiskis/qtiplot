/***************************************************************************
	File                 : Spectrogram.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : QtiPlot's Spectrogram Class
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
#include <Spectrogram.h>
#include <Graph.h>
#include <MultiLayer.h>
#include <ApplicationWindow.h>
#include <PlotCurve.h>
#include <PenStyleBox.h>

#include <math.h>
#include <QPen>
#include <QPainter>
#include <qwt_scale_widget.h>
#include <qwt_painter.h>

Spectrogram::Spectrogram(Graph *graph, Matrix *m):
	QwtPlotSpectrogram(QString(m->objectName())),
	d_graph(graph),
	d_matrix(m),
	color_axis(QwtPlot::yRight),
	color_map_policy(Default),
	color_map(LinearColorMap()),
	d_show_labels(true),
	d_labels_color(Qt::black),
	d_labels_font(QFont()),
	d_white_out_labels(false),
	d_labels_angle(0.0),
	d_labels_x_offset(0),
	d_labels_y_offset(0),
	d_selected_label(NULL),
	d_use_matrix_formula(false),
	d_color_map_pen(false)
{
	setData(MatrixData(m));

	double step = fabs(data().range().maxValue() - data().range().minValue())/5.0;

	QwtValueList contourLevels;
	for ( double level = data().range().minValue() + step;
		level < data().range().maxValue(); level += step )
		contourLevels += level;

	setContourLevels(contourLevels);
}

void Spectrogram::updateData()
{
	if (!d_matrix || !d_graph)
		return;

	setData(MatrixData(d_matrix, d_use_matrix_formula));

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		colorAxis->setColorMap(range(), colorMap());

	d_graph->setAxisScale(color_axis, range().minValue(), range().maxValue());
	d_graph->replot();
}

QwtDoubleInterval Spectrogram::range() const
{
	if (color_map.intensityRange().isValid())
		return color_map.intensityRange();

	double mmin, mmax;
	d_matrix->range(&mmin, &mmax);
	return QwtDoubleInterval(mmin, mmax);
}

bool Spectrogram::setMatrix(Matrix *m, bool useFormula)
{
	if (!m)
		return false;

	if (m == d_matrix && useFormula == d_use_matrix_formula)
		return true;

	bool changedUseFormula = (useFormula != d_use_matrix_formula) ? true : false;
	bool canCalculate = false;
	if (useFormula){
		canCalculate = m->canCalculate();
		if (!canCalculate){
			QMessageBox::warning(d_graph->multiLayer(), QObject::tr("QtiPlot - Script Error"),
			QObject::tr("Python-like syntax is not supported in this case since it severely reduces drawing speed!"));
			changedUseFormula = false;
			d_use_matrix_formula = false;
		}
	}

	if (changedUseFormula)
		d_use_matrix_formula = useFormula;

	bool changedMatrix = (d_matrix != m) ? true : false;
	if (changedMatrix)
		d_matrix = m;

	if (changedMatrix || changedUseFormula)
		updateData();

	if (!canCalculate)
		return false;
	return true;
}

void Spectrogram::setLevelsNumber(int levels)
{
	if (levels <= 0)
		return;

	double step = fabs(range().maxValue() - range().minValue())/(double)levels;

	QwtValueList contourLevels;
	for ( double level = range().minValue() + 0.5*step;
		level < range().maxValue(); level += step )
		contourLevels += level;

	setContourLevels(contourLevels);
}

void Spectrogram::setContourLevels (const QwtValueList & levels)
{
	QwtPlotSpectrogram::setContourLevels(levels);
	createLabels();
}

bool Spectrogram::hasColorScale()
{
	if (!d_graph)
		return false;

	if (!d_graph->axisEnabled (color_axis))
		return false;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		return colorAxis->isColorBarEnabled();

	return false;
}

void Spectrogram::showColorScale(int axis, bool on)
{
if (hasColorScale() == on && color_axis == axis)
	return;

if (!d_graph)
	return;

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
colorAxis->setColorBarEnabled(false);

color_axis = axis;

// We must switch main and the color scale axes and their respective scales
	int xAxis = this->xAxis();
	int yAxis = this->yAxis();
	int oldMainAxis = QwtPlot::xBottom;
	if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop){
		oldMainAxis = xAxis;
		xAxis = 5 - color_axis;
	} else if (axis == QwtPlot::yLeft || axis == QwtPlot::yRight){
		oldMainAxis = yAxis;
		yAxis = 1 - color_axis;
	}

// First we switch axes
setAxis(xAxis, yAxis);

// Next we switch axes scales
QwtScaleDiv *scDiv = d_graph->axisScaleDiv(oldMainAxis);
if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	d_graph->setAxisScale(xAxis, scDiv->lowerBound(), scDiv->upperBound());
else if (axis == QwtPlot::yLeft || color_axis == QwtPlot::yRight)
	d_graph->setAxisScale(yAxis, scDiv->lowerBound(), scDiv->upperBound());

colorAxis = d_graph->axisWidget(color_axis);
d_graph->setAxisScale(color_axis, range().minValue(), range().maxValue());
colorAxis->setColorBarEnabled(on);
colorAxis->setColorMap(range(), colorMap());
if (!d_graph->axisEnabled(color_axis))
	d_graph->enableAxis(color_axis);
colorAxis->show();
d_graph->updateLayout();
}

int Spectrogram::colorBarWidth()
{
	if (!d_graph)
		return 0;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	return colorAxis->colorBarWidth();
}

void Spectrogram::setColorBarWidth(int width)
{
	if (!d_graph)
		return;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	colorAxis->setColorBarWidth(width);
}

Spectrogram* Spectrogram::copy(Graph *g)
{
	Spectrogram *new_s = new Spectrogram(g, matrix());
	new_s->setDisplayMode(QwtPlotSpectrogram::ImageMode, testDisplayMode(QwtPlotSpectrogram::ImageMode));
	new_s->setDisplayMode(QwtPlotSpectrogram::ContourMode, testDisplayMode(QwtPlotSpectrogram::ContourMode));
	new_s->setCustomColorMap(color_map);
	new_s->setAxis(xAxis(), yAxis());
	new_s->setDefaultContourPen(defaultContourPen());
	new_s->color_map_policy = color_map_policy;
	new_s->d_show_labels = d_show_labels;
	new_s->d_labels_angle = d_labels_angle;
	new_s->d_labels_color = d_labels_color;
	new_s->d_white_out_labels = d_white_out_labels;
	new_s->d_labels_font = d_labels_font;
	new_s->d_labels_x_offset = d_labels_x_offset;
	new_s->d_labels_y_offset = d_labels_y_offset;
	new_s->setContourLevels(contourLevels());

	if (defaultContourPen().style() == Qt::NoPen && !d_color_map_pen)
		new_s->setContourPenList(d_pen_list);
	else
		new_s->d_color_map_pen = d_color_map_pen;

	if (d_labels_list.isEmpty()){
		new_s->clearLabels();
		return new_s;
	}

	QList <PlotMarker *> lst = new_s->labelsList();
	int count = lst.size();
	for(int i = 0; i < count; i++){
		PlotMarker *m = lst[i];
		PlotMarker *mrk = d_labels_list[i];
		if (m && mrk)
			m->setLabelOffset(mrk->xLabelOffset(), mrk->yLabelOffset());
	}

	return new_s;
}

void Spectrogram::setGrayScale()
{
	color_map = LinearColorMap(Qt::black, Qt::white);
	setColorMap(color_map);
	color_map_policy = GrayScale;

	if (!d_graph)
		return;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		colorAxis->setColorMap(range(), colorMap());
}

void Spectrogram::setDefaultColorMap()
{
	if (!d_graph)
		return;

	color_map = d_graph->multiLayer()->applicationWindow()->d_3D_color_map;
	setColorMap(color_map);
	color_map_policy = Default;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		colorAxis->setColorMap(range(), colorMap());
}

void Spectrogram::setCustomColorMap(const LinearColorMap& map)
{
	setColorMap(map);
	color_map = map;
	color_map_policy = Custom;

	if (!d_graph)
		return;

	QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
	if (colorAxis)
		colorAxis->setColorMap(range(), colorMap());
}

QString Spectrogram::saveToString()
{
QString s = "<spectrogram>\n";
s += "\t<matrix>" + QString(d_matrix->objectName()) + "</matrix>\n";
s += "\t<useMatrixFormula>" + QString::number(d_use_matrix_formula) + "</useMatrixFormula>\n";
s += "\t<xAxis>" + QString::number(xAxis()) + "</xAxis>\n";
s += "\t<yAxis>" + QString::number(yAxis()) + "</yAxis>\n";

if (color_map_policy != Custom)
	s += "\t<ColorPolicy>" + QString::number(color_map_policy) + "</ColorPolicy>\n";
else
	s += color_map.toXmlString();
s += "\t<Image>"+QString::number(testDisplayMode(QwtPlotSpectrogram::ImageMode))+"</Image>\n";

bool contourLines = testDisplayMode(QwtPlotSpectrogram::ContourMode);
s += "\t<ContourLines>"+QString::number(contourLines)+"</ContourLines>\n";
if (contourLines){
	QwtValueList levels = contourLevels();
	s += "\t\t<Levels>" + QString::number(levels.size()) + "</Levels>\n";
	for (int i = 0; i < levels.size(); i++)
		s += "\t\t<z>" + QString::number(levels[i]) + "</z>\n";

	bool defaultPen = defaultContourPen().style() != Qt::NoPen;
	s += "\t\t<DefaultPen>" + QString::number(defaultPen) + "</DefaultPen>\n";
	if (defaultPen){
		s += "\t\t\t<PenColor>" + defaultContourPen().color().name() + "</PenColor>\n";
		s += "\t\t\t<PenWidth>" + QString::number(defaultContourPen().widthF()) + "</PenWidth>\n";
		s += "\t\t\t<PenStyle>" + QString::number(defaultContourPen().style() - 1) + "</PenStyle>\n";
	} else if (!d_color_map_pen && !d_pen_list.isEmpty()){
		s += "\t\t<PenList>\n";
		for (int i = 0; i < d_pen_list.size(); i++){
			QPen pen = d_pen_list[i];
			s += "\t\t\t<pen>" + pen.color().name () + "," + QString::number(pen.widthF()) + ",";
			s += QString::number(PenStyleBox::styleIndex(pen.style())) + "</pen>\n";
		}
		s += "\t\t</PenList>\n";
	}

	if (d_show_labels){
		s += "\t\t<Labels>\n";
		s += "\t\t\t<Color>" + d_labels_color.name() + "</Color>\n";
		s += "\t\t\t<WhiteOut>" + QString::number(d_white_out_labels) +"</WhiteOut>\n";
		s += "\t\t\t<Angle>" + QString::number(d_labels_angle) + "</Angle>\n";
		s += "\t\t\t<xOffset>" + QString::number(d_labels_x_offset) + "</xOffset>\n";
		s += "\t\t\t<yOffset>" + QString::number(d_labels_y_offset) + "</yOffset>\n";
		s += "\t\t\t<Font>" + d_labels_font.toString() + "</Font>\n";
		foreach(PlotMarker *m, d_labels_list){
			if (m->xLabelOffset() != 0.0 || m->xLabelOffset() != 0.0){
				s += "\t\t\t<offset>" + QString::number(m->index()) + ",";
				s += QString::number(m->xLabelOffset()) + ",";
				s += QString::number(m->yLabelOffset()) + "</offset>\n";
			}
		}
		s += "\t\t</Labels>\n";
	}
}

QwtScaleWidget *colorAxis = d_graph->axisWidget(color_axis);
if (colorAxis && colorAxis->isColorBarEnabled()){
	s += "\t<ColorBar>\n\t\t<axis>" + QString::number(color_axis) + "</axis>\n";
	s += "\t\t<width>" + QString::number(colorAxis->colorBarWidth()) + "</width>\n";
	s += "\t</ColorBar>\n";
	}
s += "\t<Visible>"+ QString::number(isVisible()) + "</Visible>\n";
return s+"</spectrogram>\n";
}

QwtDoubleRect Spectrogram::boundingRect() const
{
	return d_matrix->boundingRect();
}

void Spectrogram::createLabels()
{
	clearLabels();

	QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
    for (int l = 0; l < numLevels; l++){
		PlotMarker *m = new PlotMarker(l, d_labels_angle);

		QwtText t = QwtText(QString::number(levels[l]));
		t.setColor(d_labels_color);
		t.setFont(d_labels_font);

		if (d_white_out_labels)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);

        int x_axis = xAxis();
        int y_axis = yAxis();
		m->setAxis(x_axis, y_axis);

        if (d_graph && d_show_labels)
			m->attach(d_graph);
		d_labels_list << m;
	}
}

void Spectrogram::showContourLineLabels(bool show)
{
	if (show == d_show_labels)
        return;

    d_show_labels = show;

    foreach(PlotMarker *m, d_labels_list){
    	if (d_show_labels)
			m->attach(d_graph);
		else
			m->detach();
	}
}

void Spectrogram::drawContourLines (QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtRasterData::ContourLines &contourLines) const
{
	QwtValueList levels = contourLevels();
    const int numLevels = (int)levels.size();
    for (int l = 0; l < numLevels; l++){
        const double level = levels[l];

        QPen pen = defaultContourPen();
        if ( pen.style() == Qt::NoPen )
            pen = contourPen(level);

        if ( pen.style() == Qt::NoPen )
            continue;

        p->setPen(QwtPainter::scaledPen(pen));

        const QPolygonF &lines = contourLines[level];
        for ( int i = 0; i < (int)lines.size(); i += 2 ){
            const QPointF p1( xMap.xTransform(lines[i].x()),
                yMap.transform(lines[i].y()) );
            const QPointF p2( xMap.xTransform(lines[i + 1].x()),
                yMap.transform(lines[i + 1].y()) );

            p->drawLine(p1, p2);
        }
    }

	if (d_show_labels)
		updateLabels(p, xMap, yMap, contourLines);
}

void Spectrogram::updateLabels(QPainter *p, const QwtScaleMap &, const QwtScaleMap &,
		const QwtRasterData::ContourLines &contourLines) const
{
	QwtValueList levels = contourLevels();
	const int numLevels = levels.size();
	int x_axis = xAxis();
	int y_axis = yAxis();
	for (int l = 0; l < numLevels; l++){
		const double level = levels[l];
		const QPolygonF &lines = contourLines[level];
		if (lines.isEmpty())
			continue;

		PlotMarker *mrk = d_labels_list[l];
		if (!mrk)
			return;

		QSize size = mrk->label().textSize();
		int dx = int((d_labels_x_offset + mrk->xLabelOffset())*0.01*size.height());
		int dy = -int(((d_labels_y_offset + mrk->yLabelOffset())*0.01 + 0.5)*size.height());

		int i = (int)lines.size()/2;
		double x = lines[i].x();
		double y = lines[i].y();

		int x2 = d_graph->transform(x_axis, x) + dx;
		int y2 = d_graph->transform(y_axis, y) + dy;

		if (p->device()->logicalDpiX() == plot()->logicalDpiX() ||
			p->device()->logicalDpiY() == plot()->logicalDpiY())
			mrk->setValue(d_graph->invTransform(x_axis, x2),
							d_graph->invTransform(y_axis, y2));
	}
}

void Spectrogram::setLabelsFont(const QFont& font)
{
	if (font == d_labels_font)
        return;

    d_labels_font = font;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setFont(font);
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsColor(const QColor& c)
{
    if (c == d_labels_color)
        return;

    d_labels_color = c;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setColor(c);
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsWhiteOut(bool whiteOut)
{
    if (whiteOut == d_white_out_labels)
        return;

    d_white_out_labels = whiteOut;

    foreach(QwtPlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if (whiteOut)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);
	}
}

void Spectrogram::setLabelsOffset(double x, double y)
{
    if (x == d_labels_x_offset && y == d_labels_y_offset)
        return;

    d_labels_x_offset = x;
    d_labels_y_offset = y;
}

void Spectrogram::setLabelOffset(int index, double x, double y)
{
	if (index < 0 || index >= d_labels_list.size())
		return;

	PlotMarker *m = d_labels_list[index];
	if (!m)
		return;

	m->setLabelOffset(x, y);
}

void Spectrogram::setLabelsRotation(double angle)
{
    if (angle == d_labels_angle)
        return;

    d_labels_angle = angle;

    foreach(PlotMarker *m, d_labels_list)
		m->setAngle(angle);
}

bool Spectrogram::selectedLabels(const QPoint& pos)
{
	d_selected_label = NULL;

	if (d_graph->hasActiveTool())
		return false;

    foreach(PlotMarker *m, d_labels_list){
        int x = d_graph->transform(xAxis(), m->xValue());
        int y = d_graph->transform(yAxis(), m->yValue());

        QMatrix wm;
        wm.translate(x, y);
		wm.rotate(-d_labels_angle);
        if (wm.mapToPolygon(QRect(QPoint(0, 0), m->label().textSize())).containsPoint(pos, Qt::OddEvenFill)){
			d_selected_label = m;
			d_click_pos_x = d_graph->invTransform(xAxis(), pos.x());
			d_click_pos_y = d_graph->invTransform(yAxis(), pos.y());
            selectLabel(true);
            return true;
        }
	}
	return false;
}

void Spectrogram::selectLabel(bool on)
{
	if (on){
		d_graph->deselect();
		d_graph->notifyFontChange(d_labels_font);
		d_graph->notifyColorChange(d_labels_color);
	}

	foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if(t.text().isEmpty())
			return;

		if (d_selected_label && m == d_selected_label && on)
			t.setBackgroundPen(QPen(Qt::blue));
		else
			t.setBackgroundPen(QPen(Qt::NoPen));

		m->setLabel(t);
	}

	d_graph->replot();
}

bool Spectrogram::hasSelectedLabels()
{
    /*if (d_labels_list.isEmpty())
        return false;

    foreach(PlotMarker *m, d_labels_list){
        if (m->label().backgroundPen() == QPen(Qt::blue))
            return true;
        else
            return false;
    }
    return false;*/

    if (d_selected_label)
		return true;
	return false;
}

void Spectrogram::moveLabel(const QPoint& pos)
{
	if (!d_selected_label || d_labels_list.isEmpty())
		return;

    d_graph->replot();
    int x_axis = xAxis();
    int y_axis = yAxis();

    int d_x = pos.x() - d_graph->transform(x_axis, d_click_pos_x);
	int d_y = pos.y() - d_graph->transform(y_axis, d_click_pos_y);

	int height = d_selected_label->label().textSize().height();
	double x_offset = d_selected_label->xLabelOffset() + d_x*100.0/(double)height;
    double y_offset = d_selected_label->yLabelOffset() - d_y*100.0/(double)height;

	d_selected_label->setLabelOffset(x_offset, y_offset);

	d_graph->replot();
    d_graph->notifyChanges();

	d_click_pos_x = d_graph->invTransform(x_axis, pos.x());
	d_click_pos_y = d_graph->invTransform(y_axis, pos.y());
}

void Spectrogram::clearLabels()
{
	foreach(PlotMarker *m, d_labels_list){
		m->detach();
		delete m;
	}
	d_labels_list.clear();
}

void Spectrogram::setVisible(bool on)
{
	QwtPlotItem::setVisible(on);
	foreach(PlotMarker *m, d_labels_list)
		m->setVisible(on);
}

QPen Spectrogram::contourPen (double level) const
{
	if (d_color_map_pen)
		return QwtPlotSpectrogram::contourPen(level);

	QwtValueList levels = contourLevels();
	int index = levels.indexOf (level);
	if (index >= 0 && index < d_pen_list.size())
		return d_pen_list[index];

	return QPen();
}

void Spectrogram::setColorMapPen(bool on)
{
	d_color_map_pen = on;
	if (on) {
		setDefaultContourPen(Qt::NoPen);
		d_pen_list.clear();
	}
}

void Spectrogram::setContourPenList(QList<QPen> lst)
{
	d_pen_list = lst;
	setDefaultContourPen(Qt::NoPen);
	d_color_map_pen = false;
}

void Spectrogram::setContourLinePen(int index, const QPen &pen)
{
	QwtValueList levels = contourLevels();
	if (index < 0 || index >= levels.size())
		return;

	if (d_pen_list.isEmpty()){
		QPen p = defaultContourPen();
		for (int i = 0; i < levels.size(); i++){
			if (p.style() == Qt::NoPen)
				d_pen_list << contourPen(levels[i]);
			else
				d_pen_list << p;
		}
	}

	d_pen_list[index] = pen;
	setDefaultContourPen(Qt::NoPen);
	d_color_map_pen = false;
	d_graph->replot();
}

bool Spectrogram::setUseMatrixFormula(bool on)
{
	if (on && !d_matrix->canCalculate())
		return false;

	d_use_matrix_formula = on;
	updateData();
	return true;
}

QImage Spectrogram::renderImage(const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QwtDoubleRect &area) const
{
	if (area.isEmpty())
		return QImage();

	QRect rect = transform(xMap, yMap, area);

	QwtScaleMap xxMap = xMap;
	QwtScaleMap yyMap = yMap;

	MatrixData *d_data = (MatrixData *)data().copy();
	const QSize res = d_data->rasterHint(area);
	if (res.isValid()){
		rect.setSize(rect.size().boundedTo(res));

		int px1 = rect.x();
		int px2 = rect.x() + rect.width();
		if ( xMap.p1() > xMap.p2() )
			qSwap(px1, px2);

		double sx1 = area.x();
		double sx2 = area.x() + area.width();
		if ( xMap.s1() > xMap.s2() )
			qSwap(sx1, sx2);

		int py1 = rect.y();
		int py2 = rect.y() + rect.height();
		if ( yMap.p1() > yMap.p2() )
			qSwap(py1, py2);

		double sy1 = area.y();
		double sy2 = area.y() + area.height();
		if ( yMap.s1() > yMap.s2() )
			qSwap(sy1, sy2);

		xxMap.setPaintInterval(px1, px2);
		xxMap.setScaleInterval(sx1, sx2);
		yyMap.setPaintInterval(py1, py2);
		yyMap.setScaleInterval(sy1, sy2);
	}

	QImage image(rect.size(), QImage::Format_ARGB32);

	const QwtDoubleInterval intensityRange = range();
	if(!intensityRange.isValid())
		return image;

	d_data->initRaster(area, rect.size());

	for (int y = rect.top(); y <= rect.bottom(); y++){
		const double ty = yyMap.invTransform(y);
		QRgb *line = (QRgb *)image.scanLine(y - rect.top());
		for (int x = rect.left(); x <= rect.right(); x++){
			const double tx = xxMap.invTransform(x);
			*line++ = color_map.rgb(intensityRange, d_data->value(tx, ty));
		}
	}

	d_data->discardRaster();

	// Mirror the image in case of inverted maps
	const bool hInvert = xxMap.p1() > xxMap.p2();
	const bool vInvert = yyMap.p1() < yyMap.p2();
	if (hInvert || vInvert)
		image = image.mirrored(hInvert, vInvert);

	return image;
}

double MatrixData::value(double x, double y) const
{
	x += 0.5*dx;
	y -= 0.5*dy;

	int i = qRound(fabs((y - y_start)/dy));
	int j = qRound(fabs((x - x_start)/dx));

	if (d_mup){
		*d_y = y;
		*d_x = x;
		*d_ri = i;
		*d_rr = i;
		*d_cj = j;
		*d_cc = j;

		if (d_mup->codeLines() == 1)
			return d_mup->evalSingleLine();
		else
			return d_mup->eval().toDouble();
	} else
		return d_matrix->cell(i, j);

	return 0.0;
}
