#include "ApplicationWindow.h"
#include "MultiLayer.h"
#include "PlotCurve.h"
#include "Grid.h"
#include "ScaleEngine.h"
#include "QwtBarCurve.h"
#include "LegendWidget.h"
#include "RectangleWidget.h"
#include "ImageWidget.h"
#include "QwtPieCurve.h"
#include "ArrowMarker.h"

#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include <QApplication>
#include <QClipboard>

#define xOffset 100
#define yOffset 10

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
	pen.setWidth(border->property("Weight").toInt() - 1);

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
	QColor bc = colorToQColor(curve->property("MarkerBackgroundColor").toInt());
	QColor lc = colorToQColor(curve->property("MarkerForegroundColor").toInt());

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

void importText(QAxObject* text, Graph *g)
{
	if (!text)
		return;

	LegendWidget *l = g->newLegend(text->property("Text").toString());

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
	l->move(QPoint(x + xOffset, y + yOffset));
	l->setOnTop();
}

void importRectangle(QAxObject* r, Graph *g)
{
	if (!r)
		return;

	RectangleWidget *fw = new RectangleWidget(g);

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();
	int x = qRound(r->property("Left").toDouble()*dpiX/72.0);
	int y = qRound(r->property("Top").toDouble()*dpiY/72.0);
	int w = qRound(r->property("Width").toDouble()*dpiX/72.0);
	int h = qRound(r->property("Height").toDouble()*dpiY/72.0);

	fw->move(QPoint(x + xOffset, y + yOffset));
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

void importPicture(QAxObject* r, Graph *g)
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

	i->move(QPoint(x + xOffset, y + yOffset));
	i->setSize(w, h);
	i->setFrameStyle(0);

	g->add(i, false);
}

void importLine(QAxObject* l, Graph *g)
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

	QWidget *canvas = g->multiLayer()->canvas();
	QPoint pos = g->canvas()->mapFrom(canvas, QPoint(x + xOffset, y + yOffset + h));
	mrk.setStartPoint(g->invTransform(QwtPlot::xBottom, pos.x()), g->invTransform(QwtPlot::yLeft, pos.y()));

	pos = g->canvas()->mapFrom(canvas, QPoint(x + xOffset + w, y + yOffset));
	mrk.setEndPoint(g->invTransform(QwtPlot::xBottom, pos.x()), g->invTransform(QwtPlot::yLeft, pos.y()));

	mrk.setLinePen(linePen(l));

	g->addArrow(&mrk);
}

void importShape(QAxObject* shape, Graph *g)
{
	if (!shape)
		return;

	QAxObject* o = shape->querySubObject("DrawingObject");
	if (!o)
		return;

	int type = shape->dynamicCall("Type").toInt();
	if (type == 17)
		importText(o, g);
	else if (type == 13)
		importPicture(o, g);
	else if (type == 9)
		importLine(shape, g);
	else if (type == 1){
		QAxObject* fill = o->querySubObject("Interior");
		if (fill)
			importRectangle(o, g);
		else
			importLine(shape, g);
	}
}

void importShapes(QAxObject* chart, Graph *g)
{
	QAxObject* shapes = chart->querySubObject( "Shapes()");
	if (!shapes)
		return;

	int count = shapes->property("Count").toInt();
	for (int i = 1; i <= count; i++)
		importShape(chart->querySubObject( "Shapes(const QVariant&)", QVariant(i)), g);
}

void importGroupBoxes(QAxObject* chart, Graph *g)
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
			importShape(shapes->querySubObject("Item(const QVariant&)", QVariant(j)), g);
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
	Grid *grid = g->grid();

	int dpiX = g->physicalDpiX();
	int dpiY = g->physicalDpiY();

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

		g->setMargin(10);
		g->setGeometry(5, 5 + yOffset, w, h);
		ml->resize(w + 2*xOffset, h + LayerButton::btnSize() + yOffset + 100);
	}

	QAxObject* plotArea = chart->querySubObject( "PlotArea");
	if (plotArea){
		QAxObject* fill = plotArea->querySubObject( "Interior");
		if (fill)
			g->setCanvasBackground(colorToQColor(fill->property("Color").toInt()));
		QAxObject* border = plotArea->querySubObject( "Border");
		if (border)
			g->setCanvasFrame(border->property("Weight").toInt(), colorToQColor(border->property("Color").toInt()));

		int x = qRound(plotArea->property("Left").toDouble()*dpiX/72.0);
		int y = qRound(plotArea->property("Top").toDouble()*dpiY/72.0);
		int w = qRound(plotArea->property("Width").toDouble()*dpiX/72.0);
		int h = qRound(plotArea->property("Height").toDouble()*dpiY/72.0);
		g->setCanvasGeometry(x + xOffset, g->y() + y + yOffset, w, h);
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
		g->setTitle(" ");

	if (!chart->dynamicCall("RightAngleAxes()").toBool()){
		g->enableAxis(QwtPlot::yRight, false);
		g->enableAxis(QwtPlot::xTop, false);
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
				bool hasMarkers = false;
				switch(curveType){
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
					case -4169:
						hasMarkers = true;
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
					QwtPieCurve *pc = (QwtPieCurve *)c;
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
						style == Graph::Histogram || style == Graph::Pie || style == Graph::Box)
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

					if (hasMarkers)
						setCurveMarkersFormat(c, curve);
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

	if (!chart->dynamicCall("HasLegend()").toBool())
		g->removeLegend();
	else {
		LegendWidget *l = g->legend();
		QAxObject* legend = chart->querySubObject( "Legend()");
		if (l && legend){
			QAxObject* fill = legend->querySubObject( "Interior");
			if (fill)
				l->setBackgroundColor(colorToQColor(fill->property("Color").toInt()));
			QAxObject* border = legend->querySubObject( "Border");
			if (border){
				QPen pen(colorToQColor(border->property("Color").toInt()), border->property("Weight").toInt(),
						 Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
				l->setFramePen(pen);
			}
			l->setFont(qFont(legend));
			QAxObject* font = legend->querySubObject("Font");
			if (font)
				l->setTextColor(colorToQColor(font->property("Color").toInt()));

			int x = qRound(legend->property("Left").toDouble()*dpiX/72.0);
			int y = qRound(legend->property("Top").toDouble()*dpiY/72.0);
			l->move(QPoint(x + xOffset, y + yOffset));
		}
	}

	importShapes(chart, g);
	importGroupBoxes(chart, g);

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
		} else
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

	QApplication::restoreOverrideCursor();
	return t;
}
