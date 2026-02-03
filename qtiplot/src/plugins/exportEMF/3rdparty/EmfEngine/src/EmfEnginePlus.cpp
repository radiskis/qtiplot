/***************************************************************************
    File                 : EmfEnginePlus.cpp
    Project              : EmfEngine
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Enables the export of QPainter grafics to
						   Windows Enhanced Metafiles (.emf) by using GDI+ calls
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 3 of the License, or      *
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

#include "EmfEngine.h"
#include <QDir>

EmfPaintEngine::EmfPaintEngine(const QString& f) : QPaintEngine(QPaintEngine::AllFeatures)
{
	fname = f;
}

bool EmfPaintEngine::begin(QPaintDevice* p)
{
	setPaintDevice(p);

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);	// Initialize GDI+.
	metafile = new Metafile(fname.toStdWString().c_str(), GetDC(NULL),
				RectF(0, 0, paintDevice()->width(), paintDevice()->height()), MetafileFrameUnitPixel);

	d_grx = new Graphics(metafile);
	d_grx->SetCompositingQuality(CompositingQualityHighQuality);
	d_grx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
#ifdef HAVE_ANTIALIASED_TEXT
	d_grx->SetTextRenderingHint(TextRenderingHintAntiAlias);
#endif

	if (metafile && d_grx)
		return true;

	return false;
}

bool EmfPaintEngine::end()
{
	delete d_grx;
	delete metafile;

	GdiplusShutdown(gdiplusToken);
 	return true;
}

void EmfPaintEngine::drawPoints ( const QPointF * points, int pointCount )
{
	setClipping();

	QPen pen = painter()->pen();
	REAL lw = pen.widthF();
	QColor color = pen.color();
	SolidBrush *br = new SolidBrush(Color(color.red(), color.green(), color.blue()));

	QTransform m = painter()->worldTransform();
	for (int i = 0; i < pointCount; i++) {
		QPointF p = m.map(points[i]);
		d_grx->FillRectangle(br, RectF(p.x(), p.y(), lw, lw));
	}

	delete br;
	resetClipping();
}

void EmfPaintEngine::drawLines ( const QLineF * lines, int lineCount )
{
	setClipping();

	Pen *pen = convertPen(painter()->pen());

	QTransform m = painter()->worldTransform();
	for (int i = 0; i < lineCount; i++) {
		QPointF p1 = m.map(lines[i].p1());
		QPointF p2 = m.map(lines[i].p2());

		PointF wp1(p1.x(), p1.y());
		PointF wp2(p2.x(), p2.y());

		d_grx->DrawLine(pen, wp1, wp2);
	}

	delete pen;
	resetClipping();
}

void EmfPaintEngine::drawPolygon ( const QPointF * points, int pointCount, PolygonDrawMode mode )
{
	setClipping();

	PointF *pts = new PointF[pointCount];

	QTransform m = painter()->worldTransform();
	for (int i = 0; i < pointCount; i++){
		QPointF p = m.map (points[i]);
		pts[i] = PointF(p.x(), p.y());
	}

	Pen *pen = convertPen(painter()->pen());
	Qt::BrushStyle brushStyle = painter()->brush().style();

	if (mode == QPaintEngine::PolylineMode)
		d_grx->DrawLines(pen, pts, pointCount);
	else {
		if (brushStyle != Qt::NoBrush){
			GraphicsPath path;
			if (brushStyle == Qt::RadialGradientPattern)
				path.AddPolygon(pts, pointCount);

			Brush *brush = convertBrush(painter()->brush(), &path);
			if (mode == QPaintEngine::OddEvenMode)
				d_grx->FillPolygon(brush, pts, pointCount, FillModeAlternate);
			else if (mode == QPaintEngine::WindingMode)
				d_grx->FillPolygon(brush, pts, pointCount, FillModeWinding);
			else
				d_grx->FillPolygon(brush, pts, pointCount);
			delete brush;
		}
		d_grx->DrawPolygon(pen, pts, pointCount);
	}

	delete pen;
	delete [] pts;
	resetClipping();
}

void EmfPaintEngine::drawTextItem ( const QPointF & p, const QTextItem & textItem )
{
	QFont f = textItem.font();

	FontFamily *fontFamily = new FontFamily(f.family().toStdWString().c_str());
	if (!fontFamily->IsAvailable()){
		qWarning("EmfEngine::drawTextItem(): font family %ls is not available.", f.family().toStdWString().c_str());

		InstalledFontCollection installedFontCollection;
		int count = installedFontCollection.GetFamilyCount();
		if (!count)
			return;

		// Allocate a buffer to hold the array of FontFamily objects available.
		FontFamily *pFontFamily = new FontFamily[count];
		int found = 0;
		installedFontCollection.GetFamilies(count, pFontFamily, &found);

		WCHAR familyName[LF_FACESIZE];
		pFontFamily[0].GetFamilyName(familyName);

		delete fontFamily;
		fontFamily = new FontFamily(familyName);

		qWarning("Font replaced with: %ls", familyName);
		delete pFontFamily;
	}

	INT fontStyle = FontStyleRegular;
	if (f.italic())
		fontStyle = fontStyle | FontStyleItalic;
	if (f.bold())
		fontStyle = fontStyle | FontStyleBold;
	if (f.underline())
		fontStyle = fontStyle | FontStyleUnderline;
	if (f.strikeOut())
		fontStyle = fontStyle | FontStyleStrikeout;

	Font font(fontFamily, f.pointSizeF(), fontStyle, UnitPoint);

	QString text = textItem.text();
	int size = text.size();

	wchar_t *wtext = (wchar_t *)malloc(size*sizeof(wchar_t));
	if (!wtext){
		qWarning("EmfEngine: Not enough memory in drawTextItem().");
		return;
	}

	setClipping();

	size = text.toWCharArray(wtext);

	QColor c = painter()->pen().color();
	SolidBrush brush(Color(c.red(), c.green(), c.blue()));

	QTransform m = painter()->worldTransform();
	Matrix wm(m.m11(), m.m12(), m.m21(), m.m22(), m.dx(), m.dy());
	d_grx->SetTransform(&wm);

	double y_offset = textItem.ascent();
	int height = QFontMetrics(f).boundingRect(text).height();
	if (int(textItem.ascent() + textItem.descent()) > height)
		y_offset = 0.75*height;

	d_grx->DrawString(wtext, size, &font, PointF(p.x() - 0.5*QFontMetrics(f).averageCharWidth(), p.y() - y_offset), &brush);
	d_grx->ResetTransform();

	free(wtext);
	delete fontFamily;
	resetClipping();
}

void EmfPaintEngine::drawRects ( const QRectF * rects, int rectCount )
{
	setClipping();

	Pen *pen = convertPen(painter()->pen());
	Qt::BrushStyle brushStyle = painter()->brush().style();

	QColor c = Qt::yellow;
	Color color(c.alpha(), c.red(), c.green(), c.blue());
	Brush *br = new SolidBrush(color);

	QTransform m = painter()->worldTransform();
	for (int i = 0; i < rectCount; i++){
		PointF *pts = new PointF[4];

		QPolygonF poly(rects[i]);
		for (int j = 0; j < 4; j++){
			QPointF p = m.map(poly[j]);
			pts[j] = PointF(p.x(), p.y());
		}

		GraphicsPath path;
		path.AddPolygon(pts, 4);

		if (brushStyle != Qt::NoBrush){
			Brush *brush = convertBrush(painter()->brush(), &path);
			d_grx->FillPath(brush, &path);
			delete brush;
		}

		d_grx->DrawPath(pen, &path);
		delete [] pts;
	}

	resetClipping();
	delete pen;
	delete br;
}

void EmfPaintEngine::drawEllipse ( const QRectF & rect )
{
	setClipping();

	QRectF r = painter()->worldTransform().mapRect(rect);
	RectF rf((REAL)r.left(), (REAL)r.top(), (REAL)r.width(), (REAL)r.height());

	if (painter()->brush().style() != Qt::NoBrush){
		Brush *brush = convertBrush(painter()->brush());
		d_grx->FillEllipse(brush, rf);
		delete brush;
	}

	Pen *pen = convertPen(painter()->pen());
	d_grx->DrawEllipse(pen, rf);
	delete pen;

	resetClipping();
}

void EmfPaintEngine::drawPath ( const QPainterPath & path )
{
	setClipping();

	GraphicsPath *wpath = convertPath(path);
	if (!wpath)
		return;

	if (painter()->brush().style() != Qt::NoBrush){
		Brush *brush = convertBrush(painter()->brush(), wpath);
		d_grx->FillPath(brush, wpath);
		delete brush;
	}

	Pen *pen = convertPen(painter()->pen());
	d_grx->DrawPath(pen, wpath);
	delete pen;

	delete wpath;
	resetClipping();
}

void EmfPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
	setClipping();

	QTransform m = painter()->worldTransform();
	QPointF p = m.map(r.topLeft());

	drawPixmap(pm.copy(sr.toAlignedRect()),
			   RectF(PointF(p.x(), p.y()), SizeF(r.width(), r.height())));

	resetClipping();
}

void EmfPaintEngine::drawPixmap(const QPixmap &pix, const RectF &r)
{
	QString path = QDir::tempPath();
	QString name = path + "/" + "EmfEngine_temp.png";
	name = QDir::cleanPath(name);
	pix.save(name);

	Image *image = Image::FromFile(name.toStdWString().c_str());
	d_grx->DrawImage(image, r);
	delete image;

	QFile::remove(name);
}

void EmfPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap & pix, const QPointF &)
{
	setClipping();

	TextureBrush *tBrush = textureBrush(pix);

	QRectF dr = painter()->worldTransform().mapRect(r);
	d_grx->FillRectangle(tBrush, RectF((REAL)dr.left(), (REAL)dr.top(), (REAL)dr.width(), (REAL)dr.height()));

	delete tBrush;
	resetClipping();
}

TextureBrush * EmfPaintEngine::textureBrush(const QPixmap & pix)
{
	QString path = QDir::tempPath();
	QString name = path + "/" + "EmfEngine_temp.png";
	name = QDir::cleanPath(name);
	pix.save(name);

	Image *image = Image::FromFile(name.toStdWString().c_str());
	TextureBrush *tBrush = new TextureBrush(image);

	delete image;
	QFile::remove(name);
	return tBrush;
}

void EmfPaintEngine::drawImage(const QRectF & r, const QImage & image, const QRectF & sr, Qt::ImageConversionFlags flags)
{
	setClipping();

	QTransform m = painter()->worldTransform();
	QPointF p = m.map(r.topLeft());

	drawPixmap(QPixmap::fromImage(image, flags).copy(sr.toAlignedRect()),
	           RectF(PointF(p.x(), p.y()), SizeF(r.width(), r.height())));

	resetClipping();
}

void EmfPaintEngine::setClipping()
{
	if(painter()->testRenderHint (QPainter::Antialiasing))
		d_grx->SetSmoothingMode(SmoothingModeAntiAlias);

	if (painter()->hasClipping()){
		GraphicsPath *path = convertPath(painter()->clipPath());
		Region region(path);// Set the clipping region of the Graphics object
		d_grx->SetClip(&region);
		delete path;
	}
}

void EmfPaintEngine::resetClipping()
{
	d_grx->ResetClip();
	d_grx->SetSmoothingMode(SmoothingModeDefault);
}

Pen *EmfPaintEngine::convertPen(const QPen& pen)
{
	if (pen.style() == Qt::NoPen)
		return new Pen(Color::Transparent);

	QColor color = pen.color();
	Pen *wpen = new Pen(Color(color.red(), color.green(), color.blue()), pen.widthF());

	DashStyle style = DashStyleSolid;
	switch (pen.style()){
		case Qt::SolidLine:
			style = DashStyleSolid;
		break;

		case Qt::DashLine:
			style = DashStyleDash;
		break;

		case Qt::DotLine:
			style = DashStyleDot;
		break;

		case Qt::DashDotLine:
			style = DashStyleDashDot;
		break;

		case Qt::DashDotDotLine:
			style = DashStyleDashDotDot;
		break;

		case Qt::CustomDashLine:
		{
			style = DashStyleCustom;
			QVector<qreal> pattern = pen.dashPattern();
			int count = pattern.count();
			REAL *dashVals = new REAL[count];
			for (int i = 0; i < count; i++)
				dashVals[i] = pattern[i];

			wpen->SetDashPattern(dashVals, count);
			delete [] dashVals;
			break;
		}

		default:
		break;
	}
	wpen->SetDashStyle(style);

	LineCap capStyle = LineCapFlat;
	DashCap dashStyle = DashCapFlat;
	switch (pen.capStyle()){
		case Qt::FlatCap:
		break;

		case Qt::SquareCap:
			capStyle = LineCapSquare;
		break;

		case Qt::RoundCap:
			capStyle = LineCapRound;
			dashStyle = DashCapRound;
		break;

		default:
		break;
	}
	wpen->SetStartCap(capStyle);
	wpen->SetEndCap(capStyle);
	wpen->SetLineCap(capStyle, capStyle, dashStyle);

	LineJoin joinStyle = LineJoinMiter;
	switch (pen.joinStyle()){
		case Qt::MiterJoin:
		break;

		case Qt::BevelJoin:
			joinStyle = LineJoinBevel;
		break;

		case Qt::RoundJoin:
			joinStyle = LineJoinRound;
		break;

		case Qt::SvgMiterJoin:
			joinStyle = LineJoinMiter;
		break;

		default:
			break;
	}
	wpen->SetLineJoin(joinStyle);
	return wpen;
}

Brush *EmfPaintEngine::convertBrush(const QBrush& brush, GraphicsPath *origPath)
{
	QColor c = brush.color();
	Color color(c.alpha(), c.red(), c.green(), c.blue());

	switch(brush.style()){
		case Qt::NoBrush:
			return new SolidBrush(Color::Transparent);
		break;

		case Qt::SolidPattern:
			return new SolidBrush(color);
		break;
		case Qt::Dense1Pattern:
			return new HatchBrush(HatchStyle10Percent, Color::White, color);
		break;
		case Qt::Dense2Pattern:
			return new HatchBrush(HatchStyle20Percent, Color::White, color);
		break;
		case Qt::Dense3Pattern:
			return new HatchBrush(HatchStyle30Percent, Color::White, color);
		break;
		case Qt::Dense4Pattern:
			return new HatchBrush(HatchStyle50Percent, Color::White, color);
		break;
		case Qt::Dense5Pattern:
			return new HatchBrush(HatchStyle60Percent, Color::White, color);
		break;
		case Qt::Dense6Pattern:
			return new HatchBrush(HatchStyle75Percent, Color::White, color);
		break;
		case Qt::Dense7Pattern:
			return new HatchBrush(HatchStyle80Percent, Color::White, color);
		break;
		case Qt::HorPattern:
			return new HatchBrush(HatchStyleHorizontal, color, Color::Transparent);
		break;
		case Qt::VerPattern:
			return new HatchBrush(HatchStyleVertical, color, Color::Transparent);
		break;
		case Qt::CrossPattern:
			return new HatchBrush(HatchStyleCross, color, Color::Transparent);
		break;
		case Qt::BDiagPattern:
			return new HatchBrush(HatchStyleBackwardDiagonal, color, Color::Transparent);
		break;
		case Qt::FDiagPattern:
			return new HatchBrush(HatchStyleForwardDiagonal, color, Color::Transparent);
		break;
		case Qt::DiagCrossPattern:
			return new HatchBrush(HatchStyleDiagonalCross, color, Color::Transparent);
		break;

		case Qt::LinearGradientPattern:
		{
			const QLinearGradient *qtgradient = (const QLinearGradient *)brush.gradient();
			QGradientStops stops = qtgradient->stops();
			QColor fc = stops.first().second;
			QColor sc = stops.last().second;

			QTransform m = painter()->worldTransform();
			QPointF sp = m.map(qtgradient->start());
			QPointF ep = m.map(qtgradient->finalStop());

			LinearGradientBrush *linGrBrush = new LinearGradientBrush(
				PointF(sp.x(), sp.y()), PointF(ep.x(), ep.y()),
				Color(fc.alpha(), fc.red(), fc.green(), fc.blue()),
				Color(sc.alpha(), sc.red(), sc.green(), sc.blue()));
			return linGrBrush;
		}
		break;

		case Qt::ConicalGradientPattern:
		{
			qWarning("EmfEngine: Qt::ConicalGradientPattern is not supported.");
			return new SolidBrush(Color::Transparent);
		}
		break;

		case Qt::RadialGradientPattern:
		{
			const QRadialGradient *qtgradient = (const QRadialGradient *)brush.gradient();
			QGradientStops stops = qtgradient->stops();

			QTransform m = painter()->worldTransform();
			QPointF center = m.map(qtgradient->center());
			QPointF focalPoint = m.map(qtgradient->focalPoint());
			REAL radius = qtgradient->radius();
			REAL diameter = 2*qtgradient->radius();

			GraphicsPath path;
			path.AddEllipse(center.x() - radius, center.y() - radius, diameter, diameter);

			// Use the path to construct a brush.
			PathGradientBrush *pthGrBrush = new PathGradientBrush(&path);
			pthGrBrush->SetCenterPoint(PointF(focalPoint.x(), focalPoint.y()));

			int count = stops.count();
			Color *colors = new Color[count];
			REAL *interpPositions = new REAL[count];
			for (int i = 0; i < count; i++){
				QGradientStop stop = stops[i];
				interpPositions[i] = stop.first;
				QColor sc = stop.second;
				colors[count - i - 1] = Color(sc.alpha(), sc.red(), sc.green(), sc.blue());
			}
			pthGrBrush->SetInterpolationColors(colors, interpPositions, count);

			if (origPath){
				SolidBrush br(colors[0]);
				d_grx->FillPath(&br, origPath);
			}
			delete [] colors;
			delete [] interpPositions;
			return pthGrBrush;
		}
		break;

		case Qt::TexturePattern:
			return textureBrush(brush.texture());
		break;

		default:
		break;
	}

	return new SolidBrush(color);
}

GraphicsPath * EmfPaintEngine::convertPath ( const QPainterPath & path )
{
	int points = path.elementCount();
	PointF *pts = new PointF[points];
	BYTE *types = new BYTE[points];

	QTransform m = painter()->worldTransform();
	for (int i = 0; i < points; i++){
		QPainterPath::Element el = path.elementAt(i);
		QPointF p = m.map(QPointF(el.x, el.y));
		pts[i] = PointF(p.x(), p.y());

		switch(el.type){
			case QPainterPath::MoveToElement:
				types[i] = PathPointTypeStart;
			break;

			case QPainterPath::LineToElement:
				types[i] = PathPointTypeLine;
			break;

			case QPainterPath::CurveToElement:
				types[i] = PathPointTypeBezier;
			break;

			case QPainterPath::CurveToDataElement:
				types[i] = PathPointTypeBezier;
			break;
		}
	}

	GraphicsPath *wpath = new GraphicsPath(pts, types, points);
	delete [] pts;
	delete [] types;
	return wpath;
}
