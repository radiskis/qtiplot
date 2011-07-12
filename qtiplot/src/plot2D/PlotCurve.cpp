/***************************************************************************
    File                 : DataCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : AbstractPlotCurve and DataCurve classes

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
#include "PlotCurve.h"
#include "ErrorBarsCurve.h"
#include "BoxCurve.h"
#include "Graph.h"
#include "Grid.h"
#include "MultiLayer.h"
#include "ScaleDraw.h"
#include "ScaleEngine.h"
#include <ApplicationWindow.h>
#include <SymbolBox.h>
#include <PatternBox.h>
#include <ImageSymbol.h>

#include <QDateTime>
#include <QPainter>
#include <QBuffer>

#include <qwt_symbol.h>
#include <qwt_painter.h>
#include <qwt_curve_fitter.h>
#include <qwt_plot_canvas.h>

PlotCurve::PlotCurve(const QString& name): QwtPlotCurve(name),
d_type(0),
d_plot_style(0),
d_x_offset(0.0),
d_y_offset(0.0),
d_side_lines(false),
d_skip_symbols(1)
{
    setPaintAttribute(PaintFiltered);
    setPaintAttribute(ClipPolygons);
	setCurveAttribute(QwtPlotCurve::Fitted, false);
}

QwtDoubleRect PlotCurve::boundingRect() const
{
	QwtDoubleRect r = QwtPlotCurve::boundingRect();

	double percent = 0.01;

	double dw = percent*fabs(r.right() - r.left());
	double left = r.left() - dw;
	if (left <= 0.0){
		ScaleEngine *sc_engine = (ScaleEngine *)this->plot()->axisScaleEngine(xAxis());
		if (sc_engine && (sc_engine->type() == ScaleTransformation::Log10 ||
			sc_engine->type() == ScaleTransformation::Log2 ||
			sc_engine->type() == ScaleTransformation::Ln))
			left = r.left();
	}

	r.setLeft(left);
	r.setRight(r.right() + dw);

	double dh = percent*fabs(r.top() - r.bottom());
	r.setBottom(r.bottom() + dh);

	double top = r.top() - dh;
	if (top <= 0.0){
		ScaleEngine *sc_engine = (ScaleEngine *)this->plot()->axisScaleEngine(yAxis());
		if (sc_engine && (sc_engine->type() == ScaleTransformation::Log10 ||
			sc_engine->type() == ScaleTransformation::Log2 ||
			sc_engine->type() == ScaleTransformation::Ln))
			top = r.top();
	}
	r.setTop(top);

	return r;
}

QString PlotCurve::saveCurveSymbolImage()
{
	if (symbol().style() != QwtSymbol::Image)
		return QString();

	ImageSymbol *is = (ImageSymbol *)(&symbol());
	if (!is)
		return QString();

	QString s = "<SymbolImage>\n";
	s += "<path>" + is->imagePath() + "</path>\n";
	s += "<xpm>\n";
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	is->pixmap().save(&buffer, "XPM");
	s += QString(bytes);
	s += "</xpm>\n";

	return s + "</SymbolImage>\n";
}

QString PlotCurve::saveCurveLayout()
{
	QString s = "<Style>" + QString::number(d_plot_style) + "</Style>\n";
	if (d_plot_style == Graph::Spline)
		s += "<LineStyle>5</LineStyle>\n";
	else if (d_plot_style == Graph::VerticalSteps)
		s += "<LineStyle>6</LineStyle>\n";
	else
		s += "<LineStyle>" + QString::number(this->style()) + "</LineStyle>\n";

	QPen pen = this->pen();
	if (pen.style() != Qt::NoPen){
		s += "<Pen>\n";
		s += "\t<Color>" + pen.color().name() + "</Color>\n";
		if (pen.color().alpha() != 255)
			s += "\t<Alpha>" + QString::number(pen.color().alpha()) + "</Alpha>\n";
		s += "\t<Style>" + QString::number(pen.style()-1) + "</Style>\n";
		s += "\t<Width>" + QString::number(pen.widthF()) + "</Width>\n";
		s += "</Pen>\n";
	}

	QBrush brush = this->brush();
	if (brush.style() != Qt::NoBrush){
		s += "<Brush>\n";
		s += "\t<Color>" + brush.color().name() + "</Color>\n";
		if (brush.color().alpha() != 255)
			s += "\t<Alpha>" + QString::number(brush.color().alpha()) + "</Alpha>\n";
		s += "\t<Style>" + QString::number(PatternBox::patternIndex(brush.style())) + "</Style>\n";
		s += "</Brush>\n";
	}

	const QwtSymbol symbol = this->symbol();
	if (symbol.style() != QwtSymbol::NoSymbol){
		s += "<Symbol>\n";
		s += "\t<Style>" + QString::number(SymbolBox::symbolIndex(symbol.style())) + "</Style>\n";
		s += "\t<Size>" + QString::number(symbol.size().width()) + "</Size>\n";

		s += "\t<SymbolPen>\n";
		s += "\t\t<Color>" + symbol.pen().color().name() + "</Color>\n";
		if (symbol.pen().color().alpha() != 255)
			s += "\t<Alpha>" + QString::number(symbol.pen().color().alpha()) + "</Alpha>\n";
		s += "\t\t<Width>" + QString::number(symbol.pen().widthF()) + "</Width>\n";
		s += "\t</SymbolPen>\n";

		brush = this->brush();
		if (brush.style() != Qt::NoBrush){
			s += "\t<SymbolBrush>\n";
			s += "\t\t<Color>" + symbol.brush().color().name() + "</Color>\n";
			if (symbol.brush().color().alpha() != 255)
				s += "\t<Alpha>" + QString::number(symbol.brush().color().alpha()) + "</Alpha>\n";
			s += "\t\t<Style>" + QString::number(PatternBox::patternIndex(symbol.brush().style())) + "</Style>\n";
			s += "\t</SymbolBrush>\n";
		}
		s += "</Symbol>\n";
	}
	s += "<xAxis>" + QString::number(xAxis()) + "</xAxis>\n";
	s += "<yAxis>" + QString::number(yAxis()) + "</yAxis>\n";
	s += "<CurveType>" + QString::number(curveType()) + "</CurveType>\n";
	s += "<Visible>" + QString::number(isVisible()) + "</Visible>\n";
	return s;
}

void PlotCurve::restoreCurveLayout(const QStringList& lst)
{
	QStringList::const_iterator line = lst.begin();
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = (*line).stripWhiteSpace();
        if (s == "<Pen>"){
			QPen pen;
			pen.setCosmetic(true);
			while(s != "</Pen>"){
				s = (*(++line)).stripWhiteSpace();
				if (s.contains("<Color>"))
					pen.setColor(QColor(s.remove("<Color>").remove("</Color>")));
				else if (s.contains("<Alpha>")){
					QColor c = pen.color();
					c.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
					pen.setColor(c);
				} else if (s.contains("<Style>"))
					pen.setStyle(Graph::getPenStyle(s.remove("<Style>").remove("</Style>").toInt()));
				else if (s.contains("<Width>"))
					pen.setWidthF(s.remove("<Width>").remove("</Width>").toDouble());
			}
			setPen(pen);
		} else if (s == "<Brush>"){
			QBrush brush;
			while(s != "</Brush>"){
				s = (*(++line)).stripWhiteSpace();
				if (s.contains("<Color>"))
					brush.setColor(QColor(s.remove("<Color>").remove("</Color>")));
				else if (s.contains("<Alpha>")){
					QColor c = brush.color();
					c.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
					brush.setColor(c);
				} else if (s.contains("<Style>"))
					brush.setStyle(PatternBox::brushStyle(s.remove("<Style>").remove("</Style>").toInt()));
			}
			setBrush(brush);
		} else if (s == "<Symbol>"){
			QwtSymbol symbol;
			while(s != "</Symbol>"){
				s = (*(++line)).stripWhiteSpace();
				if (s.contains("<Style>"))
					symbol.setStyle(SymbolBox::style(s.remove("<Style>").remove("</Style>").toInt()));
				else if (s.contains("<Size>"))
					symbol.setSize((QwtSymbol::Style)s.remove("<Size>").remove("</Size>").toInt());
				else if (s == "<SymbolPen>"){
					QPen pen;
					while(s != "</SymbolPen>"){
						s = (*(++line)).stripWhiteSpace();
						if (s.contains("<Color>"))
							pen.setColor(QColor(s.remove("<Color>").remove("</Color>")));
						else if (s.contains("<Alpha>")){
							QColor c = pen.color();
							c.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
							pen.setColor(c);
						} else if (s.contains("<Style>"))
							pen.setStyle(Graph::getPenStyle(s.remove("<Style>").remove("</Style>").toInt()));
						else if (s.contains("<Width>"))
							pen.setWidthF(s.remove("<Width>").remove("</Width>").toDouble());
					}
					pen.setCosmetic(true);
					symbol.setPen(pen);
				} else if (s == "<SymbolBrush>"){
					QBrush brush;
					while(s != "</SymbolBrush>"){
						s = (*(++line)).stripWhiteSpace();
						if (s.contains("<Color>"))
							brush.setColor(QColor(s.remove("<Color>").remove("</Color>")));
						else if (s.contains("<Alpha>")){
							QColor c = brush.color();
							c.setAlpha(s.remove("<Alpha>").remove("</Alpha>").toInt());
							brush.setColor(c);
						} else if (s.contains("<Style>"))
							brush.setStyle(PatternBox::brushStyle(s.remove("<Style>").remove("</Style>").toInt()));
					}
					symbol.setBrush(brush);
				}
				setSymbol(symbol);
			}
		} else if (s.contains("<xAxis>"))
			setXAxis(s.remove("<xAxis>").remove("</xAxis>").toInt());
		else if (s.contains("<yAxis>"))
			setYAxis(s.remove("<yAxis>").remove("</yAxis>").toInt());
		else if (s.contains("<CurveType>"))
			setCurveType((QwtPlotCurve::CurveType)s.remove("<CurveType>").remove("</CurveType>").toInt());
		else if (s.contains("<Visible>"))
			setVisible(s.remove("<Visible>").remove("</Visible>").toInt());
	}
}

void PlotCurve::drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if(d_side_lines)
		drawSideLines(p, xMap, yMap, from, to);
	QwtPlotCurve::drawCurve(p, style, xMap, yMap, from, to);
}

/*!
  Draw sticks

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa draw(), drawCurve(), drawDots()
*/
void PlotCurve::drawSticks(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
	if (d_skip_symbols < 2){
		QwtPlotCurve::drawSticks(painter, xMap, yMap, from, to);
		return;
	}

    int x0 = xMap.transform(baseline());
    int y0 = yMap.transform(baseline());

    for (int i = from; i <= to; i += d_skip_symbols){
        const int xi = xMap.transform(x(i));
        const int yi = yMap.transform(y(i));

        if (curveType() == Xfy)
            QwtPainter::drawLine(painter, x0, yi, xi, yi);
        else
            QwtPainter::drawLine(painter, xi, y0, xi, yi);
    }
}

void PlotCurve::setSkipSymbolsCount(int count)
{
	if (count < 1 || count > dataSize())
		return;

	d_skip_symbols = count;
}

/*!
  \brief Draw symbols
  \param painter Painter
  \param symbol Curve symbol
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa setSymbol(), draw(), drawCurve()
*/
void PlotCurve::drawSymbols(QPainter *painter, const QwtSymbol &symbol,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
	if (d_skip_symbols < 2){
		QwtPlotCurve::drawSymbols(painter, symbol, xMap, yMap, from, to);
		return;
	}

    painter->setBrush(symbol.brush());
    painter->setPen(QwtPainter::scaledPen(symbol.pen()));

    const QwtMetricsMap &metricsMap = QwtPainter::metricsMap();

    QRect rect;
    rect.setSize(metricsMap.screenToLayout(symbol.size()));

	for (int i = from; i <= to; i += d_skip_symbols){
		const int xi = xMap.transform(x(i));
		const int yi = yMap.transform(y(i));

		rect.moveCenter(QPoint(xi, yi));
		symbol.draw(painter, rect);
	}
}

void PlotCurve::drawSideLines(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	if (!p || dataSize() <= 0)
		return;

	if (to < 0)
		to = dataSize() - 1;

	p->save();
	QPen pen = p->pen();
	pen.setCapStyle(Qt::FlatCap);
	pen.setJoinStyle(Qt::MiterJoin);
	p->setPen(pen);

	double lw = 0.5*pen.widthF();
	const double xl = xMap.xTransform(x(from)) - lw;
	const double xr = xMap.xTransform(x(to)) + lw;
	const double yl = yMap.xTransform(y(from)) - lw;
	const double yr = yMap.xTransform(y(to)) - lw;
	const double base = yMap.xTransform(baseline());

	p->drawLine(QPointF(xl, yl), QPointF(xl, base));
	p->drawLine(QPointF(xr, yr), QPointF(xr, base));

	p->restore();
}

DataCurve::DataCurve(Table *t, const QString& xColName, const QString& name, int startRow, int endRow):
    PlotCurve(name),
	d_table(t),
	d_x_table(t),
	d_x_column(xColName),
	d_start_row(startRow),
	d_end_row(endRow),
	d_labels_column(QString()),
	d_labels_color(Qt::black),
	d_labels_font(QFont()),
	d_labels_angle(0.0),
	d_white_out_labels(false),
	d_show_labels(false),
	d_labels_align(Qt::AlignHCenter),
	d_labels_x_offset(0),
	d_labels_y_offset(50),
	d_selected_label(NULL)
{
	if (t && d_end_row < 0)
		d_end_row = t->numRows() - 1;
}

DataCurve::DataCurve(Table *xt, const QString& xColName, Table *yt, const QString& name, int startRow, int endRow):
	PlotCurve(name),
	d_table(yt),
	d_x_table(xt),
	d_x_column(xColName),
	d_start_row(startRow),
	d_end_row(endRow),
	d_labels_column(QString()),
	d_labels_color(Qt::black),
	d_labels_font(QFont()),
	d_labels_angle(0.0),
	d_white_out_labels(false),
	d_show_labels(false),
	d_labels_align(Qt::AlignHCenter),
	d_labels_x_offset(0),
	d_labels_y_offset(50),
	d_selected_label(NULL)
{
	if (yt && d_end_row < 0)
		d_end_row = yt->numRows() - 1;
}

void DataCurve::setRowRange(int startRow, int endRow)
{
	if (d_start_row == startRow && d_end_row == endRow)
		return;

	d_start_row = startRow;
	d_end_row = endRow;

	loadData();

	foreach(ErrorBarsCurve *c, d_error_bars)
		c->loadData();
}

void DataCurve::setFullRange()
{
	d_start_row = 0;
	d_end_row = d_table->numRows() - 1;

	loadData();

	foreach(ErrorBarsCurve *c, d_error_bars)
		c->loadData();
}

bool DataCurve::isFullRange()
{
	if (d_start_row != 0 || d_end_row != d_table->numRows() - 1)
		return false;
	else
		return true;
}

QStringList DataCurve::plotAssociation()
{
	QStringList lst = QStringList() << title().text();
	if (!d_x_column.isEmpty())
		lst = QStringList() << d_x_column + "(X)" << title().text() + "(Y)";

	if (!d_labels_column.isEmpty())
		lst << d_labels_column + "(L)";

	return lst;
}

void DataCurve::updateColumnNames(const QString& oldName, const QString& newName, bool updateTableName)
{
	if (updateTableName){
		QString s = title().text();
		QStringList lst = s.split("_", QString::SkipEmptyParts);
		if (lst.size() < 2)
			return;

		if (lst[0] == oldName)
			setTitle(newName + "_" + lst[1]);

		if (!d_x_column.isEmpty()){
			lst = d_x_column.split("_", QString::SkipEmptyParts);
			if (lst[0] == oldName)
				d_x_column = newName + "_" + lst[1];
		}
	} else {
		if (title().text() == oldName)
			setTitle(newName);
		if (d_x_column == oldName)
			d_x_column = newName;
	}
}

bool DataCurve::updateData(Table *t, const QString& colName)
{
	if (d_table != t ||
       (colName != title().text() &&
       colName != d_x_column &&
       colName != d_labels_column))
        return false;

	loadData();
	return true;
}

void DataCurve::setDataSource(Table *yt, int ycol, Table *xt, int xcol)
{
	if (!yt)
		return;

	d_table = yt;
	d_x_table = (xt != NULL) ? xt : yt;
	d_x_column = d_x_table->colName(xcol);
	setTitle(yt->colName(ycol));

	loadData();
}

void DataCurve::enableSpeedMode()
{
	Graph *g = (Graph *)plot();
	if (!g)
		return;
	if (g->getDouglasPeukerTolerance() == 0.0 || dataSize() < g->speedModeMaxPoints())
		return;

	setCurveAttribute(QwtPlotCurve::Fitted);

	QwtWeedingCurveFitter *fitter = new QwtWeedingCurveFitter(g->getDouglasPeukerTolerance());
	setCurveFitter(fitter);
}

void DataCurve::drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
	Graph *g = (Graph *)plot();
	if (!g)
		return;

	if (d_data_ranges.empty() || !g->isMissingDataGapEnabled())
		return PlotCurve::drawCurve(p, style, xMap, yMap, from, to);

	if(d_side_lines)
		drawSideLines(p, xMap, yMap, from, to);

	for (unsigned int i = 0; i < d_data_ranges.size(); i++)
		QwtPlotCurve::drawCurve(p, style, xMap, yMap, d_data_ranges[i].from, d_data_ranges[i].to);
}

void DataCurve::loadData()
{
	Graph *g = (Graph *)plot();
	if (!g)
		return;

	int xcol = d_x_table->colIndex(d_x_column);
	int ycol = d_table->colIndex(title().text());

	if (xcol < 0 || ycol < 0){
		remove();
		return;
	}

	int r = abs(d_end_row - d_start_row) + 1;
    QVarLengthArray<double> X(r), Y(r);
	int xColType = d_x_table->columnType(xcol);
	int yColType = d_table->columnType(ycol);

	QStringList xLabels, yLabels;// store text labels

	int xAxis = QwtPlot::xBottom;
	if (d_type == Graph::HorizontalBars)
		xAxis = QwtPlot::yLeft;

	QString date_time_fmt = d_table->columnFormat(xcol);
	int size = 0, from = 0;
	d_data_ranges.clear();
	for (int i = d_start_row; i <= d_end_row; i++ ){
		QString xval = d_x_table->text(i, xcol);
		QString yval = d_table->text(i, ycol);
		if (!xval.isEmpty() && !yval.isEmpty()){
			bool valid_data = true;
			if (xColType == Table::Text){
				xLabels << xval;
				X[size] = (double)(size + 1);
			} else if (xColType == Table::Time)
				X[size] = Table::fromTime(QTime::fromString(xval.trimmed(), date_time_fmt));
			  else if (xColType == Table::Date)
				X[size] = Table::fromDateTime(QDateTime::fromString(xval.trimmed(), date_time_fmt));
			  else
				X[size] = g->locale().toDouble(xval, &valid_data);

			if (yColType == Table::Text){
				yLabels << yval;
				Y[size] = (double)(size + 1);
			} else
				Y[size] = g->locale().toDouble(yval, &valid_data);

			if (valid_data)
				size++;
		} else if (from < size){
			DataRange range;
			range.from = from;
			range.to = size - 1;
			d_data_ranges.push_back(range);
			from = size;
		}
	}

	if (d_data_ranges.size() && from < size){
		DataRange range;
		range.from = from;
		range.to = size - 1;
		d_data_ranges.push_back(range);
	}

	X.resize(size);
	Y.resize(size);

	if (g->isWaterfallPlot()){
		int index = g->curveIndex(this);
		int curves = g->curveCount();
		DataCurve *c = g->dataCurve(0);
		if (index > 0 && c){
			double xmin = c->minXValue();
			double dx = index*g->waterfallXOffset()*0.01*g->canvas()->width()/(double)(curves - 1);
			d_x_offset = g->invTransform(xAxis, g->transform(xAxis, xmin) + dx) - xmin;

			double ymin = c->minYValue();
			double dy = index*g->waterfallYOffset()*0.01*g->canvas()->height()/(double)(curves - 1);
			d_y_offset = ymin - g->invTransform(yAxis(), g->transform(yAxis(), ymin) + dy);
			
			setZ(-index);
			setBaseline(d_y_offset);

			for (int i = 0; i < size; i++){
				X[i] = X[i] + d_x_offset;
				Y[i] = Y[i] + d_y_offset;
			}
		} else {
			setZ(0);
			setBaseline(0.0);
		}
		if (g->grid())
			g->grid()->setZ(-g->curveCount() - 1);
	}

	if (!size){
		remove();
		return;
	} else {
		if (d_type == Graph::HorizontalBars){
			setData(Y.data(), X.data(), size);
			foreach(ErrorBarsCurve *c, d_error_bars)
                c->setData(Y.data(), X.data(), size);
		} else {
			setData(X.data(), Y.data(), size);
			foreach(ErrorBarsCurve *c, d_error_bars)
                c->setData(X.data(), Y.data(), size);
		}

		if (xColType == Table::Text)
			g->setLabelsTextFormat(xAxis, ScaleDraw::Text, d_x_column, xLabels);
		if (yColType == Table::Text)
			g->setLabelsTextFormat(QwtPlot::yLeft, ScaleDraw::Text, title().text(), yLabels);
	}

	enableSpeedMode();

	setRenderHint(QwtPlotItem::RenderAntialiased, g->isCurveAntialiasingEnabled(this));

	if (!d_labels_list.isEmpty()){
		((Graph*)plot())->updatePlot();
		loadLabels();
	}
}

void DataCurve::removeErrorBars(ErrorBarsCurve *c)
{
	if (!c || d_error_bars.isEmpty())
		return;

	int index = d_error_bars.indexOf(c);
	if (index >= 0 && index < d_error_bars.size())
		d_error_bars.removeAt(index);
}

void DataCurve::clearErrorBars()
{
	if (d_error_bars.isEmpty())
		return;

	foreach(ErrorBarsCurve *c, d_error_bars)
		c->remove();
}

void DataCurve::remove()
{
	Graph *g = (Graph *)plot();
	if (!g)
		return;

	g->removeCurve(title().text());
}

void DataCurve::setVisible(bool on)
{
	QwtPlotCurve::setVisible(on);
	foreach(ErrorBarsCurve *c, d_error_bars)
		c->setVisible(on);
	foreach(PlotMarker *m, d_labels_list)
		m->setVisible(on);
}

int DataCurve::tableRow(int point)
{
	if (!d_table)
		return -1;

	if (d_type == Graph::Pie){
		double y_val = y(point);
		int ycol = d_table->colIndex(title().text());
		for (int i = d_start_row; i <= d_end_row; i++ ){
			if (d_table->cell(i, ycol) == y_val)
				return i;
		}
	}

	int xcol = d_table->colIndex(d_x_column);
	int ycol = d_table->colIndex(title().text());

	if (xcol < 0 || ycol < 0)
		return -1;

	int xColType = d_table->columnType(xcol);
	if (xColType == Table::Date){
		QString format = d_table->columnFormat(xcol);
		QDateTime date0 = QDateTime::fromString (d_table->text(d_start_row, xcol), format);
		for (int i = d_start_row; i <= d_end_row; i++ ){
			QDateTime d = QDateTime::fromString (d_table->text(i, xcol), format);
			if (d.isValid()){
				if (d_type == Graph::HorizontalBars && date0.secsTo(d) == y(point) && d_table->cell(i, ycol) == x(point))
					return i;
				else if (date0.secsTo(d) == x(point) && d_table->cell(i, ycol) == y(point))
					return i;
			}
		}
	} else if (xColType == Table::Time){
		QString format = d_table->columnFormat(xcol);
		QTime t0 = QTime::fromString (d_table->text(d_start_row, xcol), format);
		for (int i = d_start_row; i <= d_end_row; i++ ){
			QTime t = QTime::fromString (d_table->text(i, xcol), format);
			if (t.isValid()){
				if (d_type == Graph::HorizontalBars && t0.msecsTo(t) == y(point) && d_table->cell(i, ycol) == x(point))
					return i;
				if (t0.msecsTo(t) == x(point) && d_table->cell(i, ycol) == y(point))
					return i;
			}
		}
	} else if (xColType == Table::Text){
		double y_val = y(point);
		for (int i = d_start_row; i <= d_end_row; i++ ){
			if (d_table->cell(i, ycol) == y_val)
				return i;
		}
	}

	double x_val = x(point);
	double y_val = y(point);
	for (int i = d_start_row; i <= d_end_row; i++ ){
		if (d_table->cell(i, xcol) == x_val && d_table->cell(i, ycol) == y_val)
			return i;
	}

	return point;
}

void DataCurve::setLabelsColumnName(const QString& name)
{
	if (!validCurveType())
		return;

	if (d_labels_column == name && !d_labels_list.isEmpty())
		return;

	d_labels_column = name;
	loadLabels();
}

void DataCurve::loadLabels()
{
	if (!validCurveType())
		return;

    clearLabels();

    int xcol = d_table->colIndex(d_x_column);
    int ycol = d_table->colIndex(title().text());
    int labelsCol = d_table->colIndex(d_labels_column);
    int cols = d_table->numCols();
	if (xcol < 0 || ycol < 0 || labelsCol < 0 ||
        xcol >= cols || ycol >= cols || labelsCol >= cols)
		return;

	QwtPlot *d_plot = plot();
	if (!d_plot)
		return;

    int index = 0;
	for (int i = d_start_row; i <= d_end_row; i++){
		if (d_table->text(i, xcol).isEmpty() ||
            d_table->text(i, ycol).isEmpty())
            continue;

		PlotMarker *m = new PlotMarker(index, d_labels_angle);

		QwtText t = QwtText(d_table->text(i, labelsCol));
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

		QSize size = t.textSize();
        int dx = int(d_labels_x_offset*0.01*size.height());
        int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());
        int x2 = d_plot->transform(x_axis, x(index)) + dx;
        int y2 = d_plot->transform(y_axis, y(index)) + dy;
        switch(d_labels_align){
            case Qt::AlignLeft:
            break;
            case Qt::AlignHCenter:
                x2 -= size.width()/2;
            break;
            case Qt::AlignRight:
                x2 -= size.width();
            break;
        }
        m->setXValue(d_plot->invTransform(x_axis, x2));
        m->setYValue(d_plot->invTransform(y_axis, y2));
		m->attach(d_plot);
		d_labels_list << m;
		index++;
	}

	d_show_labels = true;
}

void DataCurve::clearLabels()
{
	if (!validCurveType())
		return;

	d_show_labels = false;

	foreach(PlotMarker *m, d_labels_list){
		m->detach();
		delete m;
	}
	d_labels_list.clear();
}

void DataCurve::setLabelsFont(const QFont& font)
{
	if (!validCurveType())
		return;

     if (font == d_labels_font)
        return;

    d_labels_font = font;

    foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setFont(font);
		m->setLabel(t);
	}
	updateLabelsPosition();
}

void DataCurve::setLabelsColor(const QColor& c)
{
	if (!validCurveType())
		return;

    if (c == d_labels_color)
        return;

    d_labels_color = c;

    foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		t.setColor(c);
		m->setLabel(t);
	}
}

void DataCurve::setLabelsAlignment(int flags)
{
	if (!validCurveType())
		return;

    if (flags == d_labels_align)
        return;

    d_labels_align = flags;
    updateLabelsPosition();
}

void DataCurve::updateLabelsPosition()
{
	if (!validCurveType())
		return;

    QwtPlot *d_plot = plot();
    if (!d_plot)
        return;

    foreach(PlotMarker *m, d_labels_list){
        int index = m->index();
        QSize size = m->label().textSize();
        int x_axis = xAxis();
        int y_axis = yAxis();
        int dx = int(d_labels_x_offset*0.01*size.height());
        int dy = -int((d_labels_y_offset*0.01 + 0.5)*size.height());
        int x2 = d_plot->transform(x_axis, x(index)) + dx;
        int y2 = d_plot->transform(y_axis, y(index)) + dy;
        switch(d_labels_align){
            case Qt::AlignLeft:
            break;
            case Qt::AlignHCenter:
                x2 -= size.width()/2;
            break;
            case Qt::AlignRight:
                x2 -= size.width();
            break;
        }
        m->setXValue(d_plot->invTransform(x_axis, x2));
        m->setYValue(d_plot->invTransform(y_axis, y2));
	}
}

void DataCurve::setLabelsOffset(int x, int y)
{
	if (!validCurveType())
		return;

    if (x == d_labels_x_offset && y == d_labels_y_offset)
        return;

    d_labels_x_offset = x;
    d_labels_y_offset = y;
    updateLabelsPosition();
}

void DataCurve::setLabelsRotation(double angle)
{
	if (!validCurveType())
		return;

    if (angle == d_labels_angle)
        return;

    d_labels_angle = angle;

    foreach(PlotMarker *m, d_labels_list)
		m->setAngle(angle);
}

void DataCurve::setLabelsWhiteOut(bool whiteOut)
{
	if (!validCurveType())
		return;

    if (whiteOut == d_white_out_labels)
        return;

    d_white_out_labels = whiteOut;

    foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if (whiteOut)
			t.setBackgroundBrush(QBrush(Qt::white));
        else
            t.setBackgroundBrush(QBrush(Qt::transparent));
		m->setLabel(t);
	}
}

void DataCurve::clone(DataCurve* c)
{
	if (!validCurveType())
		return;

    d_labels_color = c->labelsColor();
    d_labels_font = c->labelsFont();
	d_labels_angle = c->labelsRotation();
	d_white_out_labels = c->labelsWhiteOut();
	d_labels_align = c->labelsAlignment();
	d_labels_x_offset = c->labelsXOffset();
	d_labels_y_offset = c->labelsYOffset();
	d_skip_symbols = c->skipSymbolsCount();
	d_show_labels = c->hasVisibleLabels();
	if (d_show_labels){
        plot()->replot();
        setLabelsColumnName(c->labelsColumnName());
    }
}

QString DataCurve::saveToString()
{
	if (!validCurveType())
		return QString();

	QString s = QString::null;
	if (d_skip_symbols > 1)
		s += "<SkipPoints>" + QString::number(d_skip_symbols) + "</SkipPoints>\n";

	if (d_labels_list.isEmpty() || type() == Graph::Function)
        return s;

    s += "<CurveLabels>\n";
    s += "\t<column>" + d_labels_column + "</column>\n";
    s += "\t<color>" + d_labels_color.name() + "</color>\n";
    s += "\t<whiteOut>" + QString::number(d_white_out_labels) + "</whiteOut>\n";
    s += "\t<Font>" + d_labels_font.toString() + "</Font>\n";
    s += "\t<angle>" + QString::number(d_labels_angle) + "</angle>\n";
    s += "\t<justify>" + QString::number(d_labels_align) + "</justify>\n";
    if (d_labels_x_offset != 0.0)
        s += "\t<xoffset>" + QString::number(d_labels_x_offset) + "</xoffset>\n";
    if (d_labels_y_offset != 0.0)
        s += "\t<yoffset>" + QString::number(d_labels_y_offset) + "</yoffset>\n";

	if (type() == Graph::Box){
		s += "\t<boxLabels>" + QString::number(((BoxCurve *)this)->hasBoxLabels()) + "</boxLabels>\n";
		s += "\t<whiskerLabels>" + QString::number(((BoxCurve *)this)->hasWhiskerLabels()) + "</whiskerLabels>\n";
		s += "\t<display>" + QString::number(((BoxCurve *)this)->labelsDisplayPolicy()) + "</display>\n";
	}

	s += "</CurveLabels>\n";
	return s + saveCurveSymbolImage();
}

void DataCurve::restoreLabels(const QStringList& lst)
{
    QString labelsColumn = QString();
    int xoffset = 0, yoffset = 0;
  	QStringList::const_iterator line = lst.begin();
  	QString s = *line;
    if (s.contains("<column>"))
        labelsColumn = s.remove("<column>").remove("</column>").trimmed();

  	for (line++; line != lst.end(); line++){
        s = *line;
        if (s.contains("<color>"))
            setLabelsColor(QColor(s.remove("<color>").remove("</color>").trimmed()));
        else if (s.contains("<whiteOut>"))
            setLabelsWhiteOut(s.remove("<whiteOut>").remove("</whiteOut>").toInt());
        else if (s.contains("<font>")){//versions older than 0.9.7.8
            QStringList fontList = s.remove("<font>").remove("</font>").trimmed().split("\t");
            QFont font = QFont(fontList[0], fontList[1].toInt());
            if (fontList.count() >= 3)
                font.setBold(fontList[2].toInt());
            if (fontList.count() >= 4)
                font.setItalic(fontList[3].toInt());
            if (fontList.count() >= 5)
                font.setUnderline(fontList[4].toInt());
            setLabelsFont(font);
        } else if (s.contains("<Font>")){
            s.remove("<Font>").remove("</Font>").trimmed();
            QFont fnt;
			fnt.fromString(s);
            setLabelsFont(fnt);
        }else if (s.contains("<angle>"))
            setLabelsRotation(s.remove("<angle>").remove("</angle>").toDouble());
        else if (s.contains("<justify>"))
            setLabelsAlignment(s.remove("<justify>").remove("</justify>").toInt());
        else if (s.contains("<xoffset>"))
            xoffset = s.remove("<xoffset>").remove("</xoffset>").toInt();
        else if (s.contains("<yoffset>"))
            yoffset = s.remove("<yoffset>").remove("</yoffset>").toInt();
		else if (s.contains("<boxLabels>"))
			((BoxCurve *)this)->showBoxLabels(s.remove("<boxLabels>").remove("</boxLabels>").toInt());
		else if (s.contains("<whiskerLabels>"))
			((BoxCurve *)this)->showWhiskerLabels(s.remove("<whiskerLabels>").remove("</whiskerLabels>").toInt());
		else if (s.contains("<display>"))
			((BoxCurve *)this)->setLabelsDisplayPolicy((BoxCurve::LabelsDisplayPolicy)s.remove("<display>").remove("</display>").toInt());
    }
    setLabelsOffset(xoffset, yoffset);
    setLabelsColumnName(labelsColumn);
}

bool DataCurve::selectedLabels(const QPoint& pos)
{
	if (!validCurveType())
		return false;

    QwtPlot *d_plot = plot();
    if (!d_plot || ((Graph *)d_plot)->hasActiveTool())
        return false;

    bool selected = false;
	d_selected_label = NULL;
    foreach(PlotMarker *m, d_labels_list){
        int x = d_plot->transform(xAxis(), m->xValue());
        int y = d_plot->transform(yAxis(), m->yValue());

        QMatrix wm;
        wm.translate(x, y);
		wm.rotate(-d_labels_angle);
        if (wm.mapToPolygon(QRect(QPoint(0, 0), m->label().textSize())).containsPoint(pos, Qt::OddEvenFill)){
			d_selected_label = m;
			d_click_pos_x = d_plot->invTransform(xAxis(), pos.x());
			d_click_pos_y = d_plot->invTransform(yAxis(), pos.y());
            setLabelsSelected();
            return true;
        }
	}
	return selected;
}

bool DataCurve::hasSelectedLabels()
{
	if (!validCurveType())
		return false;

	if (d_labels_list.isEmpty())
		return false;

	foreach(PlotMarker *m, d_labels_list){
		if (m->label().backgroundPen().color() == Qt::blue)
			return true;
	}
	return false;
}

void DataCurve::setLabelsSelected(bool on)
{
	if (!validCurveType())
		return;

    foreach(PlotMarker *m, d_labels_list){
		QwtText t = m->label();
		if(t.text().isEmpty())
            continue;

        if (on){
            t.setBackgroundPen(QPen(Qt::blue));
        } else
            t.setBackgroundPen(QPen(Qt::NoPen));
        m->setLabel(t);
    }
    if (on){
        Graph *g = (Graph *)plot();
        g->selectTitle(false);
        g->deselectMarker();
        g->notifyFontChange(d_labels_font);
		g->notifyColorChange(d_labels_color);
    }
    plot()->replot();
}

bool DataCurve::validCurveType()
{
	int style = type();
	if (style == Graph::Function || style == Graph::Pie ||
		style == Graph::ColorMap || style == Graph::GrayScale ||
		style == Graph::Contour || style == Graph::ImagePlot)
		return false;
	return true;
}

void DataCurve::moveLabels(const QPoint& pos)
{
	if (!validCurveType() || !d_selected_label || d_labels_list.isEmpty())
		return;

    QwtPlot *d_plot = plot();
    if (!d_plot)
        return;

    d_plot->replot();
    int d_x = pos.x() - d_plot->transform(xAxis(), d_click_pos_x);
	int d_y = pos.y() - d_plot->transform(yAxis(), d_click_pos_y);

	int height = d_selected_label->label().textSize().height();
	d_labels_x_offset += int(d_x*100.0/(double)height);
    d_labels_y_offset -= int(d_y*100.0/(double)height);

	updateLabelsPosition();
	d_plot->replot();

    ((Graph *)d_plot->parent())->notifyChanges();

	d_click_pos_x = d_plot->invTransform(xAxis(), pos.x());
	d_click_pos_y = d_plot->invTransform(yAxis(), pos.y());
}

PlotMarker::PlotMarker(int index, double angle):QwtPlotMarker(),
	d_index(index),
	d_angle(angle),
	d_label_x_offset(0.0),
	d_label_y_offset(0.0)
{}

void PlotMarker::draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect &) const
{
	p->save();
	int x = xMap.transform (xValue());
	int y = yMap.transform (yValue());

	p->translate(x, y);
	p->rotate(-d_angle);

	QwtText text = label();
	text.draw(p, QRect(QPoint(0, 0), text.textSize()));
	p->restore();
}
