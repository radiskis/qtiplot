#include "ApplicationWindow.h"
#include "MultiLayer.h"
#include "PlotCurve.h"
#include "Grid.h"
#include "ScaleEngine.h"
#include "QwtBarCurve.h"
#include "LegendWidget.h"
#include "RectangleWidget.h"
#include "EllipseWidget.h"
#include "ImageWidget.h"
#include "PieCurve.h"
#include "ArrowMarker.h"
#include "ErrorBarsCurve.h"

#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include <QApplication>
#include <QClipboard>

#define xOffset 100
#define yOffset 15

QColor colorToQColor(int rgb)
{
	QString colorHex = QString("%1").arg(rgb, 6, 16, QLatin1Char('0'));

	bool ok;
	int r = colorHex.right(2).toInt(&ok, 16);
	int g = colorHex.mid(2, 2).toInt(&ok, 16);
	int b = colorHex.left(2).toInt(&ok, 16);

	return QColor(r, g, b);
}

QPen linePen(QAxObject *o)
{
	QPen pen = QPen();
	if (!o)
		return pen;
	QAxObject* lineFormat = o->querySubObject("Line");
	if (!lineFormat)
		return pen;

	QAxObject* color = lineFormat->querySubObject("ForeColor");
	if (color){
		int schemeColor = color->property("SchemeColor").toInt();
		if (schemeColor != 9){//9: auto color
			int rgb = color->property("RGB").toInt();
			pen.setColor(colorToQColor(rgb));
		}
		double lw = lineFormat->property("Weight").toDouble();
		if (lw >= 0)
			pen.setWidthF(lw);
	}

	int dashStyle = lineFormat->dynamicCall("DashStyle()").toInt();
	Qt::PenStyle penStyle = Qt::SolidLine;
	switch(dashStyle){
		case 2:
		case 3:
		case 10:
		case 11:
			penStyle = Qt::DotLine;
		break;
		case 4:
		case 7:
			penStyle = Qt::DashLine;
		break;
		case 5:
		case 8:
		case 12:
			penStyle = Qt::DashDotLine;
		break;
		case 6:
			penStyle = Qt::DashDotDotLine;
		break;
		default:
			break;
	}
	pen.setStyle(penStyle);
	return pen;
}

QPen qPen(QAxObject *o)
{
	QPen pen = QPen();
	if (!o)
		return pen;

	QAxObject* format = o->querySubObject("Format");
	if (format)
		pen = linePen(format);

	return pen;
}

QPen borderPen(QAxObject *o)
{
	if (!o)
		return QPen();

	QAxObject* border = o->querySubObject( "Border");
	if (!border)
		return QPen();

	QPen pen = QPen(colorToQColor(border->property("Color").toInt()));
	int lw = border->property("Weight").toInt() - 1;
	if (lw >= 1)
		pen.setWidth(lw);

	int dashStyle = border->property("LineStyle").toInt();
	Qt::PenStyle penStyle = Qt::SolidLine;
	switch(dashStyle){
		case -4142:
			penStyle = Qt::NoPen;
		break;
		case -4118:
			penStyle = Qt::DotLine;
		break;
		case -4115:
			penStyle = Qt::DashLine;
		break;
		case 4:
		case 13:
			penStyle = Qt::DashDotLine;
		break;
		case 5:
			penStyle = Qt::DashDotDotLine;
		break;
		default:
			break;
	}
	pen.setStyle(penStyle);
	pen.setCapStyle(Qt::SquareCap);
	pen.setJoinStyle(Qt::MiterJoin);
	return pen;
}

QBrush curveBrush(QAxObject *o)
{
	if (!o)
		return QBrush();

	QBrush br = QBrush();
	QAxObject* fill = o->querySubObject( "Fill");
	if (fill){
		QAxObject* color = fill->querySubObject("ForeColor");
		if (color)
			br = QBrush(colorToQColor(color->property("RGB").toInt()));
	}
	return br;
}

QBrush interiorBrush(QAxObject *o)
{
	if (!o)
		return QBrush();

	QBrush br = QBrush();
	QAxObject* fill = o->querySubObject("Interior");
	if (fill){
		int brStyle = fill->property("Pattern").toInt();
		Qt::BrushStyle brushStyle = Qt::NoBrush;
		switch(brStyle){
			case 1:
				brushStyle = Qt::SolidPattern;
			break;

			case 12:
			case -4166:
				brushStyle = Qt::VerPattern;
			break;

			case 11:
			case -4128:
				brushStyle = Qt::HorPattern;
			break;
			case 15:
				brushStyle = Qt::CrossPattern;
			break;
			case 16:
				brushStyle = Qt::DiagCrossPattern;
			break;

			case 13:
			case -4121:
				brushStyle = Qt::BDiagPattern;
			break;

			case 14:
			case -4162:
				brushStyle = Qt::FDiagPattern;
			break;

			case 10:
			case -4126:
				brushStyle = Qt::Dense1Pattern;
			break;

			case -4125:
				brushStyle = Qt::Dense4Pattern;
			break;
			case -4124:
				brushStyle = Qt::Dense5Pattern;
			break;
			case 17:
				brushStyle = Qt::Dense6Pattern;
			break;
			case 18:
				brushStyle = Qt::Dense7Pattern;
			break;
			default:
				break;
		}
		br.setStyle(brushStyle);
		br.setColor(colorToQColor(fill->property("Color").toInt()));
	}
	return br;
}

QFont qFont(QAxObject *o)
{
	if (!o)
		return QFont();

	QFont fnt = QFont();
	QAxObject* font = o->querySubObject("Font");
	if (font){
		fnt.setFamily(font->property("Name").toString());
		fnt.setPointSizeF(font->property("Size").toDouble());
		fnt.setBold(font->property("Bold").toBool());
		fnt.setItalic(font->property("Italic").toBool());
		fnt.setStrikeOut(font->property("Strikethrough").toBool());
	}
	return fnt;
}

void setAxisFormat(Graph *g, int axisIndex, QAxObject *axis)
{
	if (!axis)
		return;

	bool autoUnit = axis->property("MajorUnitIsAuto").toBool();
	ScaleDraw *sd = (ScaleDraw *)g->axisScaleDraw(axisIndex);
	if (sd->scaleType() == ScaleDraw::Numeric && autoUnit){
		double start = axis->property("MinimumScale").toDouble();
		double end = axis->property("MaximumScale").toDouble();
		double step = axis->property("MajorUnit").toDouble();
		int scaleType = ScaleTransformation::Linear;

		if (axis->dynamicCall("ScaleType()").toInt() == -4133){
			double base = axis->dynamicCall("LogBase()").toDouble();
			if (base == 10)
				scaleType = ScaleTransformation::Log10;
			else if (base == exp(0))
				scaleType = ScaleTransformation::Ln;
			else if (base == 2)
				scaleType = ScaleTransformation::Log2;

			if (step > 0)
				step = log(step)/log(base);
			else
				step = 0.0;
		}
		g->setScale(axisIndex, start, end, step, 5, 5, scaleType, axis->dynamicCall("ReversePlotOrder()").toInt());
	}

	int majorTicks = axis->dynamicCall("MajorTickMark()").toInt();
	int majorTicksType = ScaleDraw::None;
	switch(majorTicks){
		case 2:
			majorTicksType = ScaleDraw::In;
		break;
		case 3:
			majorTicksType = ScaleDraw::Out;
		break;
		case 4:
			majorTicksType = ScaleDraw::Both;
		break;
		default:
			break;
	}

	bool hasTitle = axis->dynamicCall("HasTitle()").toBool();
	if (hasTitle){
		QAxObject* axisTitle = axis->querySubObject("AxisTitle");
		if (axisTitle){
			g->setAxisTitle(axisIndex, axisTitle->property("Text").toString());
			g->setAxisTitleFont(axisIndex, qFont(axisTitle));
			QAxObject* font = axisTitle->querySubObject("Font");
			if (font)
				g->setAxisTitleColor(axisIndex, colorToQColor(font->property("Color").toInt()));
		}
	} else
		g->setAxisTitle(axisIndex, " ");

	int minorTicks = axis->dynamicCall("MinorTickMark()").toInt();
	int minorTicksType = ScaleDraw::None;
	switch(minorTicks){
		case 2:
			minorTicksType = ScaleDraw::In;
		break;
		case 3:
			minorTicksType = ScaleDraw::Out;
		break;
		case 4:
			minorTicksType = ScaleDraw::Both;
		break;
		default:
			break;
	}
	g->setAxisTicksLength(axisIndex, majorTicksType, minorTicksType, g->minorTickLength(), g->majorTickLength());

	QAxObject* tickLabels = axis->querySubObject("TickLabels");
	if (tickLabels){
		g->setAxisFont(axisIndex, qFont(tickLabels));
		QAxObject* font = tickLabels->querySubObject("Font");
		if (font)
			g->setAxisLabelsColor(axisIndex, colorToQColor(font->property("Color").toInt()));

		int angle = tickLabels->dynamicCall("Orientation()").toInt();
		if (angle >= -90 && angle <= 90)
			g->setAxisLabelRotation(axisIndex, -angle);
	}

	g->axisWidget(axisIndex)->repaint();
}

void setCurveMarkersFormat(DataCurve *c, QAxObject *curve)
{
	if (!c || !curve)
		return;

	int size = curve->property("MarkerSize").toInt();
	int fillRgb = curve->property("MarkerBackgroundColor").toInt();
	int lRgb = curve->property("MarkerForegroundColor").toInt();

	QColor bc = Qt::black;
	if (fillRgb >= 0)
		bc = colorToQColor(fillRgb);
	QColor lc = Qt::black;
	if (lRgb >= 0)
		lc = colorToQColor(lRgb);

	int style = curve->dynamicCall("MarkerStyle()").toInt();
	QwtSymbol::Style symbStyle = QwtSymbol::Ellipse;
	switch(style){
		case -4142:
			symbStyle = QwtSymbol::NoSymbol;
		break;
		case -4168:
			symbStyle = QwtSymbol::XCross;
		break;
		case -4115:
		case -4118:
			symbStyle = QwtSymbol::HLine;
		break;
		case 1:
			symbStyle = QwtSymbol::Rect;
		break;
		case 2:
			symbStyle = QwtSymbol::Diamond;
		break;
		case 3:
			symbStyle = QwtSymbol::Triangle;
		break;
		case 5:
			symbStyle = QwtSymbol::Star1;
		break;
		case 9:
			symbStyle = QwtSymbol::Cross;
		break;
	}
	c->setSymbol(QwtSymbol(symbStyle, QBrush(bc), QPen(lc), QSize(size, size)));
}

void setCurveErrorBars(DataCurve *c, QAxObject *curve)
{
	if (!c || !curve)
		return;

	QAxObject* errors = curve->querySubObject("ErrorBars");
	if (!errors)
		return;

	Table *t = c->table();
	t->addCol(Table::yErr);
	int points = t->numRows();
	for (int i = 0; i < points; i++)
		t->setCell(i, 2, 0.2*t->cell(i, 1));

	int cap = 0;
	if (errors->dynamicCall("EndStyle()").toInt() == 1)
		cap = 8;

	QPen pen = qPen(errors);
	((Graph *)c->plot())->addErrorBars(curve->property("Name").toString(), t, t->colName(2),
						ErrorBarsCurve::Vertical, pen.widthF(), cap, pen.color(), false, true, true);
}

void importText(int left, int top, QAxObject* text, Graph *g, LegendWidget *l)
{
	if (!text)
		return;

	if (!l)
		l = g->newLegend(text->property("Text").toString());

	QBrush brush = interiorBrush(text);
	l->setBrush(brush);
	if (brush.style() != Qt::NoBrush)
		l->setBackgroundColor(brush.color());

	int bkg = 0;
	QPen pen = borderPen(text);
	if (pen.style() != Qt::NoPen){
		l->setFramePen(pen);
		bkg = 1;
	}
	l->setFrameStyle(bkg);

	l->setFont(qFont(text));
	QAxObject* font = text->querySubObject("Font");
	if (font)
		l->setTextColor(colorToQColor(font->property("Color").toInt()));

	int x = qRound(text->property("Left").toDouble()*g->physicalDpiX()/72.0);
	int y = qRound(text->property("Top").toDouble()*g->physicalDpiY()/72.0);
	l->move(QPoint(x + left, y + top));
	l->setOnTop();
}

void importRectangle(int left, int top, QAxObject* r, Graph *g, int type)
{
	if (!r)
		return;

	FrameWidget* fw;
	if (type == 9)
		fw = new EllipseWidget(g);
	else
		fw = new RectangleWidget(g);

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();
	int x = qRound(r->property("Left").toDouble()*dpiX/72.0);
	int y = qRound(r->property("Top").toDouble()*dpiY/72.0);
	int w = qRound(r->property("Width").toDouble()*dpiX/72.0);
	int h = qRound(r->property("Height").toDouble()*dpiY/72.0);

	fw->move(QPoint(x + left, y + top));

	fw->setSize(w, h);
	fw->setFramePen(borderPen(r));
	fw->setFrameStyle(1);

	QColor bkg = Qt::white;
	bkg.setAlpha(0);
	fw->setBackgroundColor(bkg);

	QBrush brush = interiorBrush(r);
	if (brush.style() != Qt::NoBrush)
		fw->setBrush(brush);

	g->add(fw, false);
}

void importPicture(int left, int top, QAxObject* r, Graph *g)
{
	if (!r)
		return;

	r->dynamicCall("CopyPicture()");

	ImageWidget *i = new ImageWidget(g);
	i->setPixmap(QApplication::clipboard()->pixmap());
	i->setSaveInternally();

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();
	int x = qRound(r->property("Left").toDouble()*dpiX/72.0);
	int y = qRound(r->property("Top").toDouble()*dpiY/72.0);
	int w = qRound(r->property("Width").toDouble()*dpiX/72.0);
	int h = qRound(r->property("Height").toDouble()*dpiY/72.0);

	i->move(QPoint(x + left, y + top));
	i->setSize(w, h);
	i->setFrameStyle(0);

	g->add(i, false);
}

void importLine(int left, int top, QAxObject* l, Graph *g)
{
	if (!l)
		return;

	QAxObject* o = l->querySubObject("DrawingObject");
	if (!o)
		return;

	QAxObject* lineFormat = l->querySubObject("Line");
	if (!lineFormat)
		return;

	ArrowMarker mrk;
	mrk.setAttachPolicy(ArrowMarker::Page);
	mrk.drawStartArrow(lineFormat->dynamicCall("BeginArrowheadStyle()").toInt() != 1);
	mrk.drawEndArrow(lineFormat->dynamicCall("EndArrowheadStyle()").toInt() != 1);

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();
	int x = qRound(o->property("Left").toDouble()*dpiX/72.0);
	int y = qRound(o->property("Top").toDouble()*dpiY/72.0);
	int w = qRound(o->property("Width").toDouble()*dpiX/72.0);
	int h = qRound(o->property("Height").toDouble()*dpiY/72.0);

	x += left;
	y += top;

	QWidget *canvas = g->multiLayer()->canvas();
	QPoint pos = g->canvas()->mapFrom(canvas, QPoint(x, y + h));
	mrk.setStartPoint(g->invTransform(QwtPlot::xBottom, pos.x()), g->invTransform(QwtPlot::yLeft, pos.y()));

	pos = g->canvas()->mapFrom(canvas, QPoint(x + w, y));
	mrk.setEndPoint(g->invTransform(QwtPlot::xBottom, pos.x()), g->invTransform(QwtPlot::yLeft, pos.y()));

	mrk.setLinePen(linePen(l));

	g->addArrow(&mrk);
}

void importShape(int left, int top, QAxObject* shape, Graph *g)
{
	if (!shape)
		return;

	QAxObject* o = shape->querySubObject("DrawingObject");
	if (!o)
		return;

	int type = shape->dynamicCall("Type").toInt();
	if (type == 17)
		importText(left, top, o, g, NULL);
	else if (type == 13)
		importPicture(left, top, o, g);
	else if (type == 9)
		importLine(left, top, shape, g);
	else if (type == 1){// auto shape
		QAxObject* fill = o->querySubObject("Interior");
		if (fill)
			importRectangle(left, top, o, g, shape->dynamicCall("AutoShapeType()").toInt());
		else
			importLine(left, top, shape, g);
	}
}

void importShapes(int left, int top, QAxObject* chart, Graph *g)
{
	QAxObject* shapes = chart->querySubObject( "Shapes()");
	if (!shapes)
		return;

	int count = shapes->property("Count").toInt();
	for (int i = 1; i <= count; i++)
		importShape(left, top, chart->querySubObject( "Shapes(const QVariant&)", QVariant(i)), g);
}

void importGroupBoxes(int left, int top, QAxObject* chart, Graph *g)
{
	QAxObject* groups = chart->querySubObject("GroupObjects()");
	if (!groups)
		return;

	int count = groups->property("Count").toInt();
	for (int i = 1; i <= count; i++){
		QAxObject* groupObject = groups->querySubObject( "Item(const QVariant&)", QVariant(i));
		if (!groupObject)
			continue;

		QAxObject* range = groupObject->querySubObject( "ShapeRange");
		if (!range)
			continue;

		QAxObject* shapes = range->querySubObject("GroupItems()");
		if (!shapes)
			continue;

		int n = shapes->property("Count").toInt();
		for (int j = 1; j <= n; j++)
			importShape(left, top, shapes->querySubObject("Item(const QVariant&)", QVariant(j)), g);
	}
}

MultiLayer * ApplicationWindow::importExcelChart(QAxObject* chart, const QString& fn, const QString& name)
{
	if (!chart)
		return 0;

	MultiLayer *ml = newGraph();
	ml->setScaleLayersOnResize(false);

	QString chartName = name;
	if (name.isEmpty())
		chartName = chart->property("Name").toString();

	ml->setWindowLabel(fn + ", " + tr("chart") + ": " + chartName);
	ml->setCaptionPolicy(MdiSubWindow::Both);

	Graph *g = ml->activeLayer();
	g->setAntialiasing(true);
	Grid *grid = g->grid();

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();

	if (!chart->dynamicCall("RightAngleAxes()").toBool()){
		g->enableAxis(QwtPlot::yRight, false);
		g->enableAxis(QwtPlot::xTop, false);
	}

	bool hasTitle = chart->dynamicCall("HasTitle()").toBool();
	if (hasTitle){
		QAxObject* chartTitle = chart->querySubObject("ChartTitle");
		if (chartTitle){
			QString title = chartTitle->property("Text").toString();
			g->setTitle(title);
			g->setTitleFont(qFont(chartTitle));

			QAxObject* font = chartTitle->querySubObject("Font");
			if (font)
				g->setTitleColor(colorToQColor(font->property("Color").toInt()));
		}
	} else
		g->removeTitle();

	QAxObject* area = chart->querySubObject( "ChartArea");
	if (area){
		QAxObject* fill = area->querySubObject( "Interior");
		if (fill)
			g->setBackgroundColor(colorToQColor(fill->property("Color").toInt()));
		QAxObject* border = area->querySubObject( "Border");
		if (border)
			g->setFrame(border->property("Weight").toInt(), colorToQColor(border->property("Color").toInt()));

		int w = qRound(area->property("Width").toDouble()*dpiX/72.0);
		int h = qRound(area->property("Height").toDouble()*dpiY/72.0);

		g->setMargin(20);
		ml->resize(w + 2*xOffset, h + LayerButton::btnSize() + yOffset + 100);
	}

	int left = 0, top = 0;
	QAxObject* plotArea = chart->querySubObject( "PlotArea");
	if (plotArea){
		QAxObject* fill = plotArea->querySubObject( "Interior");
		if (fill)
			g->setCanvasBackground(colorToQColor(fill->property("Color").toInt()));
		QAxObject* border = plotArea->querySubObject( "Border");
		if (border)
			g->setCanvasFrame(border->property("Weight").toInt(), colorToQColor(border->property("Color").toInt()));

		int x = qRound(plotArea->property("InsideLeft").toDouble()*dpiX/72.0);
		int y = qRound(plotArea->property("InsideTop").toDouble()*dpiY/72.0);
		int w = qRound(plotArea->property("InsideWidth").toDouble()*dpiX/72.0);
		int h = qRound(plotArea->property("InsideHeight").toDouble()*dpiY/72.0);
		g->setCanvasGeometry(x + xOffset, g->y() + y + yOffset, w, h);

		left = x;
		top = y;
	}

	QAxObject* curves = chart->querySubObject( "SeriesCollection()");
	if (curves){
		int n = curves->property("Count").toInt();
		for (int l = 1; l <= n; l++){
			QAxObject* curve = chart->querySubObject( "SeriesCollection(const QVariant&)", QVariant(l));
			if (curve){
				QString curveName = curve->property("Name").toString();

				QList<QVariant> xValues = curve->dynamicCall( "XValues").toList();
				QList<QVariant> yValues = curve->dynamicCall( "Values").toList();
				int points = xValues.size();
				Table *t = newTable(points, 2);
				t->setColName(1, curveName, false, false);
				hideWindow(t);
				for (int k = 0; k < points; k++){
					QVariant x = xValues[k];
					QString xs = x.toString();
					if (xs.isEmpty())
						continue;
					if (x.convert(QVariant::Double))
						t->setCell(k, 0, x.toDouble());
					else {
						t->setText(k, 0, xs);
						t->setTextFormat(0);
					}

					t->setCell(k, 1, yValues[k].toDouble());
				}

				int curveType = curve->property("ChartType").toInt();
				QString tName = t->objectName();
				int style = Graph::Line;
				switch(curveType){
					case -4169:
						style = Graph::Scatter;
					break;

					case 5:
					case 69:
					case 70:
					case -4102:
						style = Graph::Pie;
					break;

					case 51:
					case 54:
						style = Graph::VerticalBars;
					break;

					case 52:
					case 53:
					case 55:
					case 56:
						style = Graph::StackColumn;
					break;

					case 57:
					case 60:
					case 95:
					case 102:
						style = Graph::HorizontalBars;
					break;

					case 58:
					case 59:
					case 61:
					case 62:
					case 96:
					case 97:
					case 103:
					case 104:
						style = Graph::StackBar;
					break;

					case 65:
					case 66:
					case 67:
					case 72:
					case 74:
						style = Graph::LineSymbols;
					break;

					case 1:
					case -4098:
					case 76:
					case 77:
					case 78:
					case 79:
						style = Graph::Area;
					break;

					default:
						break;
				}

				DataCurve *c = NULL;
				if(style == Graph::Pie){
					g->addCurves(t, QStringList() << QString("%1_%2").arg(tName, curveName), style);
					c = g->dataCurve(l - 1);
					PieCurve *pc = (PieCurve *)c;
					if (curveType == 5 || curveType == 69){
						pc->setViewAngle(90);
						pc->setRadius(75);
					} else
						pc->setRadius(90);
					pc->clearLabels();
					g->setTitle(QwtText(curveName));
					g->setMargin(0);
				} else
					c = (DataCurve *)g->insertCurve(t, QString("%1_%2").arg(tName, "1"), QString("%1_%2").arg(tName, curveName), style);

				if (c){
					c->setTitle(QwtText(curveName));
					c->setPen( qPen(curve));

					if (style == Graph::Area ||style == Graph::VerticalBars || style==Graph::HorizontalBars ||
						style == Graph::Histogram || style == Graph::Pie || style == Graph::Box ||
						style == Graph::StackBar || style == Graph::StackColumn)
						c->setBrush(curveBrush(curve));

					if (style == Graph::VerticalBars || style==Graph::HorizontalBars || style==Graph::Histogram){
						QwtBarCurve *b = (QwtBarCurve*)c;
						if (b){
							b->setGap(70);
							b->setOffset(-50 + (l - 1)*100);
						}
					}

					if (style == Graph::Area)
						c->setZ(n - l);

					if (style == Graph::Scatter || style == Graph::LineSymbols)
						setCurveMarkersFormat(c, curve);

					if (curve->property("HasErrorBars").toBool())
						setCurveErrorBars(c, curve);
				}
			}
		}
	}

	QAxObject* axis = chart->querySubObject( "Axes(const QVariant&)", QVariant(2));
	if (axis){
		bool hasMajorGrid = axis->dynamicCall("HasMajorGridlines()").toBool();
		grid->enableY(hasMajorGrid);
		if (hasMajorGrid)
			grid->setMajPenY(qPen(axis->querySubObject("MajorGridlines")));

		bool hasMinorGrid = axis->dynamicCall("HasMinorGridlines()").toBool();
		grid->enableYMin(hasMinorGrid);
		if (hasMinorGrid)
			grid->setMinPenY(qPen(axis->querySubObject("MinorGridlines")));

		setAxisFormat(g, QwtPlot::yLeft, axis);
	}

	axis = chart->querySubObject( "Axes(const QVariant&)", QVariant(1));
	if (axis){
		bool hasMajorGrid = axis->dynamicCall("HasMajorGridlines()").toBool();
		grid->enableX(hasMajorGrid);
		if (hasMajorGrid)
			grid->setMajPenX(qPen(axis->querySubObject("MajorGridlines")));

		bool hasMinorGrid = axis->dynamicCall("HasMinorGridlines()").toBool();
		grid->enableXMin(hasMinorGrid);
		if (hasMinorGrid)
			grid->setMinPenX(qPen(axis->querySubObject("MinorGridlines")));

		setAxisFormat(g, QwtPlot::xBottom, axis);
	}

	left = g->x() + g->canvas()->x() - left;
	top = g->y() + g->canvas()->y() - top;

	if (chart->property("HasLegend").toBool())
		importText(left, top, chart->querySubObject("Legend"), g, g->legend());
	else
		g->removeLegend();

	importShapes(left, top, chart, g);
	importGroupBoxes(left, top, chart, g);

	return ml;
}

void ApplicationWindow::importExcelCharts(QAxObject* ws, const QString& fn)
{
	if (!ws)
		return;

	QAxObject* charts = ws->querySubObject("ChartObjects()");
	if (!charts)
		return;

	int plots = charts->dynamicCall("Count()").toInt();
	for (int j = 1; j <= plots; j++){
		QAxObject* chartObj = charts->querySubObject("Item( int )", j);
		if (!chartObj)
			continue;

		QAxObject* chart = chartObj->querySubObject("Chart");
		if (!chart)
			continue;

		importExcelChart(chart, fn, chartObj->property("Name").toString());
	}
}

Table * ApplicationWindow::importUsingExcel(const QString& fn, int sheet)
{
	QAxObject *excel = new QAxObject(this);
	if (!excel->setControl("Excel.Application"))
		return NULL;

	QAxObject* workbooks = excel->querySubObject( "Workbooks" );
	if (!workbooks)
		return NULL;

	QAxObject* workbook = workbooks->querySubObject( "Open(const QString&)", fn);
	if (!workbook)
		return NULL;

	QAxObject* sheets = workbook->querySubObject("Worksheets");
	if (!sheets)
		return NULL;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	Table *t = NULL;
	QString dir = excel->property("DefaultFilePath").toString();
	int count = sheets->dynamicCall("Count()").toInt();
	for (int i = 1; i <= count; i++){
		if (sheet > 0 && sheet != i)
			continue;

		QAxObject* ws = sheets->querySubObject( "Item( int )", i);
		if (!ws)
			continue;

		QString tfn = "qtiplot_tmp_sheet" + QString::number(i) + ".txt";
		QString path = dir + "\\" + tfn;

		if (QFile::exists(path))
			QFile::remove(path);

		t = newTable();
		t->setWindowLabel(fn + ", " + tr("sheet") + ": " + ws->property("Name").toString());
		t->setCaptionPolicy(MdiSubWindow::Both);

		ws->dynamicCall("SaveAs(QVariant,QVariant)", QVariant(tfn), QVariant(20));
		t->importASCII(path);

		if (t->numCols() == 1 && t->numRows() == 1 && t->text(0, 0).isEmpty()){
			t->askOnCloseEvent(false);
			t->close();
			continue;
		}

		int emptyRows = 0;
		int rows = t->numRows();
		for (unsigned int i = 0; i < rows; i++){
			if (!t->isEmptyRow(i))
				break;
			emptyRows++;
		}
		if (emptyRows)
			t->deleteRows(0, emptyRows);

		int row = rows - emptyRows - 1;
		QStringList header;
		bool firstLineAllStrings = true;
		for (int col = 1; col <= t->numCols(); col++){
			if (firstLineAllStrings){
				QString s = t->text(0, col - 1);
				if (s.isEmpty()){
					firstLineAllStrings = false;
					break;
				}

				bool ok;
				s.toDouble (&ok);
				if (ok)
					firstLineAllStrings = false;
				else
					header << s;
			}

			QAxObject* cell = ws->querySubObject("Cells( int, int )", row, col);
			if (!cell)
				continue;

			QString format = cell->property("NumberFormat").toString().simplified();
			if (format.contains("y") || format.contains("d")){
				QStringList lst = format.split(" ", QString::SkipEmptyParts);
				if (lst.size() == 1)
					format = lst[0].replace("m", "M");
				else if (lst.size() == 2)
					format = lst[0].replace("m", "M") + " " + lst[1];

				t->setDateFormat(format, col - 1, false);
			} else if (format.contains("h") || format.contains("m") || format.contains("s"))
				t->setTimeFormat(format, col - 1, false);
		}

		if (!header.isEmpty() && firstLineAllStrings){
			t->deleteRows(0, 1);
			t->setHeader(header);
		}

		t->showNormal();

		importExcelCharts(ws, fn);

		if (sheet > 0 && sheet == i)
			break;
	}

	QAxObject* charts = workbook->querySubObject("Charts");
	if (charts){
		count = charts->dynamicCall("Count()").toInt();
		for (int i = 1; i <= count; i++){
			QAxObject* ws = charts->querySubObject( "Item( int )", i);
			if (!ws)
				continue;

			importExcelChart(ws, fn);
		}
	}

	workbook->dynamicCall("SetSaved(bool)", true);
	workbook->dynamicCall("Close()");
	excel->dynamicCall("Quit()");

	delete excel;

	for (int i = 1; i <= count; i++){
		QString tfn = "qtiplot_tmp_sheet" + QString::number(i) + ".txt";
		QFile::remove(dir + "\\" + tfn);
	}

	updateRecentProjectsList(fn);
	QApplication::restoreOverrideCursor();
	return t;
}

bool ApplicationWindow::isExcelInstalled()
{
	QAxObject *excel = new QAxObject();
	if (!excel->setControl("Excel.Application"))
		return false;

	excel->dynamicCall("Quit()");
	delete excel;
	return true;
}
