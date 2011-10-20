/***************************************************************************
    File                 : Graph.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004-2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Graph widget

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

#include <QVarLengthArray>
#include <PenStyleBox.h>
#include <ScreenPickerTool.h>

#include "Graph.h"
#include "MultiLayer.h"
#include "Grid.h"
#include "CanvasPicker.h"
#include "ErrorBarsCurve.h"
#include "TexWidget.h"
#include "LegendWidget.h"
#include "ArrowMarker.h"
#include "ScalePicker.h"
#include "TitlePicker.h"
#include "PieCurve.h"
#include "ImageWidget.h"
#include "QwtBarCurve.h"
#include "BoxCurve.h"
#include "QwtHistogram.h"
#include "VectorCurve.h"
#include "ScaleDraw.h"
#include "FunctionCurve.h"
#include "Spectrogram.h"
#include "SelectionMoveResizer.h"
#include "RangeSelectorTool.h"
#include "PlotCurve.h"
#include "ApplicationWindow.h"
#include "ScaleEngine.h"
#include "RectangleWidget.h"
#include "EllipseWidget.h"
#include <FrameWidget.h>
#include <ImageSymbol.h>
#include <ImportExportPlugin.h>

#ifdef TEX_OUTPUT
	#include <QTeXEngine.h>
#endif

#ifdef Q_OS_WIN
	#include <windows.h>
#endif

#include <ColorBox.h>
#include <PatternBox.h>
#include <SymbolBox.h>
#include <LinearColorMap.h>

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QImage>
#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QPaintEngine>
#include <QMenu>
#include <QTextStream>
#include <QLocale>
#include <QPrintDialog>
#include <QFileInfo>
#include <QSvgGenerator>
#include <QDir>
#if QT_VERSION >= 0x040500
#include <QTextDocumentWriter>
#endif

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_text_label.h>
#include <qwt_curve_fitter.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

Graph::Graph(int x, int y, int width, int height, QWidget* parent, Qt::WFlags f)
: QwtPlot(parent)
{
	setWindowFlags(f);
	setAttribute(Qt::WA_DeleteOnClose);
	setAcceptDrops(true);

	d_canvas_bkg_path = QString();
	d_canvas_bkg_pix = QPixmap();

	d_waterfall_offset_x = 0;
	d_waterfall_offset_y = 0;

	d_active_tool = NULL;
	d_image_profiles_tool = NULL;
	d_range_selector = NULL;
	d_peak_fit_tool = NULL;
	d_active_enrichment = NULL;
	d_selected_arrow = NULL;
	drawLineOn = false;
	drawArrowOn = false;
	drawAxesBackbone = true;
	d_auto_scale = true;
	autoScaleFonts = false;
	d_antialiasing = false;
	d_is_printing = false;
#ifdef TEX_OUTPUT
	d_is_exporting_tex = false;
	d_tex_escape_strings = true;
#endif
	d_axis_title_policy = ColComment;
	d_Douglas_Peuker_tolerance = 0.0;
	d_speed_mode_points = 3000;
	d_synchronize_scales = false;
	d_missing_data_gap = false;
	d_page_rect = QRectF();

	setGeometry(x, y, width, height);
	setAutoReplot(false);

	d_min_tick_length = 5;
	d_maj_tick_length = 9;

	d_axis_titles << "%(?Y)" << "%(?Y)" << "%(?X)" << "";
	updateAxesTitles();

	// grid
	d_grid = new Grid();
	d_grid->attach(this);
	d_grid->setZ(INT_MIN);
	d_grid_on_top = false;

	//custom scale
	d_user_step = QVector<double>(QwtPlot::axisCnt);
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		d_user_step[i] = 0.0;

		QwtScaleWidget *scale = (QwtScaleWidget *) axisWidget(i);
		if (scale){
			scale->setMargin(0);

			if (i == QwtPlot::yRight)
				scale->setLayoutFlag(QwtScaleWidget::TitleInverted, false);

			//the axis title color must be initialized...
			QwtText title = scale->title();
			title.setColor(Qt::black);
			scale->setTitle(title);

            //...same for axis color
            QPalette pal = scale->palette();
            pal.setColor(QPalette::Foreground, QColor(Qt::black));
            scale->setPalette(pal);

			ScaleDraw *sd = new ScaleDraw(this);
			sd->setTickLength(QwtScaleDiv::MinorTick, d_min_tick_length);
			sd->setTickLength(QwtScaleDiv::MediumTick, d_min_tick_length);
			sd->setTickLength(QwtScaleDiv::MajorTick, d_maj_tick_length);

			setAxisScaleDraw (i, sd);
			setAxisScaleEngine (i, new ScaleEngine());
		}
	}

	QwtPlotLayout *pLayout = plotLayout();
	pLayout->setCanvasMargin(0);
	pLayout->setAlignCanvasToScales (true);

	QwtPlotCanvas* plCanvas = canvas();
	plCanvas->setFocusPolicy(Qt::StrongFocus);
	plCanvas->setFocusIndicator(QwtPlotCanvas::ItemFocusIndicator);
	plCanvas->setFocus();
	plCanvas->setFrameShadow(QwtPlot::Plain);
	plCanvas->setCursor(Qt::arrowCursor);
	plCanvas->setLineWidth(0);
	plCanvas->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
	plCanvas->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

    QColor background = QColor(Qt::white);
    background.setAlpha(255);

	QPalette palette;
    palette.setColor(QPalette::Window, background);
    setPalette(palette);

	setCanvasBackground (background);
	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(plCanvas);
	setFrameShape(QFrame::Box);
	setLineWidth(0);
	setMouseTracking(true);

	cp = new CanvasPicker(this);

	titlePicker = new TitlePicker(this);
	scalePicker = new ScalePicker(this);

	d_zoomer[0]= new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPicker::DragSelection | QwtPicker::CornerToCorner, QwtPicker::AlwaysOff, canvas());
	d_zoomer[0]->setRubberBandPen(QPen(Qt::black));
	d_zoomer[1] = new QwtPlotZoomer(QwtPlot::xTop, QwtPlot::yRight,
			QwtPicker::DragSelection | QwtPicker::CornerToCorner,
			QwtPicker::AlwaysOff, canvas());
	zoom(false);

	d_magnifier = NULL;
	d_panner = NULL;

	connect (cp,SIGNAL(viewLineDialog()),this,SIGNAL(viewLineDialog()));
	connect (cp,SIGNAL(showPlotDialog(int)),this,SIGNAL(showPlotDialog(int)));
	connect (cp,SIGNAL(showMarkerPopupMenu()),this,SIGNAL(showMarkerPopupMenu()));
	connect (cp,SIGNAL(modified()), this, SIGNAL(modifiedGraph()));

	connect (titlePicker,SIGNAL(showTitleMenu()),this,SLOT(showTitleContextMenu()));
	connect (titlePicker,SIGNAL(doubleClicked()),this, SLOT(enableTextEditor()));
	connect (titlePicker,SIGNAL(removeTitle()),this,SLOT(removeTitle()));
	connect (titlePicker,SIGNAL(clicked()), this,SLOT(selectTitle()));

	connect (scalePicker,SIGNAL(clicked()),this,SLOT(activateGraph()));
	connect (scalePicker,SIGNAL(clicked()),this,SLOT(deselectMarker()));
	connect (scalePicker,SIGNAL(axisDblClicked(int)),this,SIGNAL(axisDblClicked(int)));
	connect (scalePicker,SIGNAL(axisTicksDblClicked(int)),this,SIGNAL(showAxisDialog(int)));
	connect (scalePicker, SIGNAL(axisTitleDblClicked()), this, SLOT(enableTextEditor()));
	connect (scalePicker,SIGNAL(axisTitleRightClicked()),this,SLOT(showAxisTitleMenu()));
	connect (scalePicker,SIGNAL(axisRightClicked(int)),this,SLOT(showAxisContextMenu(int)));

	connect (d_zoomer[0],SIGNAL(zoomed (const QwtDoubleRect &)),this,SLOT(zoomed (const QwtDoubleRect &)));
}

MultiLayer* Graph::multiLayer() const
{
	if (!parent())
		return NULL;

	return (MultiLayer *)(this->parent()->parent()->parent());
}

void Graph::notifyChanges()
{
	emit modifiedGraph();
}

void Graph::selectCanvas()
{
	selectedCanvas(this);
	setFocus();
}

void Graph::activateGraph()
{
	emit selectedGraph(this);
	setFocus();
}

void Graph::deselectMarker()
{
	if (d_selected_arrow && d_lines.contains(d_selected_arrow))
		d_selected_arrow->setEditable(false);

	deselect(d_active_enrichment);
	d_active_enrichment = NULL;
	d_selected_arrow = NULL;

	if (d_markers_selector){
		delete d_markers_selector;
		d_markers_selector = NULL;
	}

	emit enableTextEditor(NULL);

	cp->disableEditing();
	setFocus();
}

void Graph::enableTextEditor()
{
    ApplicationWindow *app = multiLayer()->applicationWindow();
    if (!app)
        return;

	if (app->d_in_place_editing)
        emit enableTextEditor(this);
    else if (titlePicker->selected())
        viewTitleDialog();
    else
        showAxisTitleDialog();
}

QList <LegendWidget *> Graph::textsList()
{
	QList <LegendWidget *> texts;
	foreach(FrameWidget *f, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(f);
		if (l)
			texts << l;
	}
	return texts;
}

bool areaLessThan(FrameWidget* fw1, FrameWidget* fw2)
{
	return fw1->width()*fw1->height() < fw2->width()*fw2->height();
}

QList <FrameWidget *> Graph::increasingAreaEnrichmentsList()
{
	int size = d_enrichments.size();
	if (size < 2)
		return d_enrichments;

	QList <FrameWidget *> lst;
	foreach(FrameWidget *f, d_enrichments)
		lst << f;

	qSort(lst.begin(), lst.end(), areaLessThan);

	return lst;
}

void Graph::selectorDeleted()
{
	d_markers_selector = NULL;
	selectionChanged(NULL);
}

void Graph::select(QWidget *l, bool add)
{
    if (!l){
        d_active_enrichment = NULL;
		selectionChanged(NULL);
        return;
    }

    selectTitle(false);
    scalePicker->deselect();
    deselectCurves();

    d_active_enrichment = qobject_cast<LegendWidget *>(l);
	if (d_active_enrichment){
		currentFontChanged(((LegendWidget *)l)->font());
		currentColorChanged(((LegendWidget *)l)->textColor());
	} else
        d_active_enrichment = qobject_cast<FrameWidget *>(l);

    if (add){
        if (d_markers_selector && d_markers_selector->contains(l))
            return;
        else if (d_markers_selector)
            d_markers_selector->add(l);
        else {
            d_markers_selector = new SelectionMoveResizer(l);
			connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
			connect(d_markers_selector, SIGNAL(destroyed(QObject*)), this, SLOT(selectorDeleted()));
        }
    } else {
        if (d_markers_selector)
            delete d_markers_selector;

        d_markers_selector = new SelectionMoveResizer(l);
        connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
		connect(d_markers_selector, SIGNAL(destroyed(QObject*)), this, SLOT(selectorDeleted()));
    }

	selectionChanged(d_markers_selector);
}

bool Graph::hasSeletedItems()
{
	if (d_markers_selector || titlePicker->selected() || scalePicker->selectedAxis())
		return true;
	return false;
}

void Graph::deselect(QWidget *l)
{
	if(!l)
		return;

	if (d_markers_selector && d_markers_selector->contains(l))
		d_markers_selector->removeAll(l);
}

void Graph::setSelectedArrow(ArrowMarker *mrk, bool add)
{
	if (!mrk)
		return;

    selectTitle(false);
	scalePicker->deselect();

	d_selected_arrow = mrk;
	if (add){
	    if (d_markers_selector){
            if (d_lines.contains(mrk))
				d_markers_selector->add(mrk);
            else
                return;
        } else {
            if (d_lines.contains(mrk))
				d_markers_selector = new SelectionMoveResizer(mrk);
            else
                return;
            connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
        }
	} else {
	    if (d_lines.contains(mrk)){
			if (mrk->editable()){
	            if (d_markers_selector)
	                delete d_markers_selector;
                return;
	        }

			if (d_markers_selector && d_markers_selector->contains(mrk))
                return;
            else
				d_markers_selector = new SelectionMoveResizer(mrk);
        } else
            return;
        connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
	}
}

void Graph::initFonts(const QFont &scaleTitleFnt, const QFont &numbersFnt)
{
	for (int i = 0; i<QwtPlot::axisCnt; i++){
		setAxisFont (i,numbersFnt);
		QwtText t = axisTitle (i);
		t.setFont (scaleTitleFnt);
		((QwtPlot *)this)->setAxisTitle(i, t);
	}
}

void Graph::setAxisFont(int axis, const QFont &fnt)
{
	((QwtPlot *)this)->setAxisFont (axis, fnt);
	replot();
	emit modifiedGraph();
}

void Graph::enableAxis(int axis, bool on)
{
	((QwtPlot *)this)->enableAxis(axis, on);
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (scale)
		scale->setMargin(0);

	scalePicker->refresh();
}

void Graph::setAxisMargin(int axis, int margin)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (scale)
		scale->setMargin(margin);
}

ScaleDraw::ScaleType Graph::axisType(int axis)
{
	if (!axisEnabled(axis))
		return ScaleDraw::Numeric;

	return ((ScaleDraw *)axisScaleDraw(axis))->scaleType();
}

void Graph::setLabelsNumericFormat(int axis, int format, int prec, const QString& formula)
{
	ScaleDraw *sd = new ScaleDraw(this, formula.ascii());
	sd->setNumericFormat((ScaleDraw::NumericFormat)format);
	sd->setNumericPrecision(prec);
	sd->setScaleDiv(axisScaleDraw(axis)->scaleDiv());
	setAxisScaleDraw (axis, sd);
}

void Graph::setLabelsNumericFormat(const QStringList& l)
{
	for (int axis = 0; axis<4; axis++){
        ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
        if (!sd || !sd->hasComponent(QwtAbstractScaleDraw::Labels))
            continue;

        int aux = 2*axis;
        setLabelsNumericFormat(axis, l[aux].toInt(), l[aux + 1].toInt(), sd->formula());
	}
}

QString Graph::saveAxesLabelsType()
{
	QString s = "AxisType\t";
	for (int i = 0; i < 4; i++){
		if (!axisEnabled(i)){
			s += QString::number((int)ScaleDraw::Numeric) + "\t";
			continue;
		}

		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(i);
		int type = (int) sd->scaleType();
		s += QString::number(type);
		if (type == ScaleDraw::Time || type == ScaleDraw::Date || type == ScaleDraw::Text ||
			type == ScaleDraw::ColHeader || type == ScaleDraw::Day || type == ScaleDraw::Month)
			s += ";" + sd->formatString();
		s += "\t";
	}
	return s+"\n";
}

QString Graph::saveTicksType()
{
	QList<int> ticksTypeList = getMajorTicksType();
	QString s = "MajorTicks";
	for (int i = 0; i < 4; i++)
		s += "\t" + QString::number(ticksTypeList[i]);
	s += "\n";

	ticksTypeList = getMinorTicksType();
	s += "MinorTicks";
	for (int i = 0; i < 4; i++)
		s += "\t" + QString::number(ticksTypeList[i]);

	return s + "\n";
}

QString Graph::saveEnabledTickLabels()
{
	QString s = "EnabledTickLabels";
	for (int axis = 0; axis < QwtPlot::axisCnt; axis++){
		const QwtScaleDraw *sd = axisScaleDraw (axis);
		s += "\t" + QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels));
	}
	return s + "\n";
}

QString Graph::saveLabelsFormat()
{
	QString s = "LabelsFormat";
	for (int axis = 0; axis < QwtPlot::axisCnt; axis++){
		s += "\t" + QString::number(axisLabelFormat(axis));
		s += "\t" + QString::number(axisLabelPrecision(axis));
	}
	return s + "\n";
}

QString Graph::saveAxesBaseline()
{
	QString s = "AxesBaseline\t";
	for (int i = 0; i<QwtPlot::axisCnt; i++){
		QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
		if (scale)
			s += QString::number(scale->margin()) + "\t";
		else
			s += "0\t";
	}
	return s + "\n";
}

QString Graph::saveAxesBackbones()
{
	QString s = "DrawAxesBackbone\t" + QString::number(drawAxesBackbone);
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		const QwtScaleDraw *sd = axisScaleDraw (i);
		s += "\t" + QString::number(sd->hasComponent(QwtAbstractScaleDraw::Backbone));
	}
	return s + "\n";
}

QString Graph::saveTickLabelsSpace()
{
	QString s = "TickLabelsSpace";
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		const QwtScaleDraw *sd = axisScaleDraw (i);
		s += "\t" + QString::number(sd->spacing());
	}
	s += "\n";

	s += "ShowTicksPolicy";
	for (int axis = 0; axis < QwtPlot::axisCnt; axis++){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
		s += "\t" + QString::number(sd->showTicksPolicy());
	}
	return s + "\n";
}

QString Graph::saveLabelsPrefixAndSuffix()
{
	QString s = "LabelsPrefix";
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (i);
		s += "\t" + sd->prefix();
	}
	s += "\n";

	s += "LabelsSuffix";
	for (int axis = 0; axis < QwtPlot::axisCnt; axis++){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
		s += "\t" + sd->suffix();
	}
	return s + "\n";
}

QString Graph::saveLabelsRotation()
{
	QString s = "LabelsRotation\t";
	s += QString::number(labelsRotation(QwtPlot::xBottom)) + "\t";
	s += QString::number(labelsRotation(QwtPlot::xTop)) + "\t";
	s += QString::number(labelsRotation(QwtPlot::yLeft)) + "\t";
	s += QString::number(labelsRotation(QwtPlot::yRight)) + "\n";
	return s;
}

void Graph::enableAxisLabels(int axis, bool on)
{
	QwtScaleWidget *sc = axisWidget(axis);
	if (sc){
		QwtScaleDraw *sd = axisScaleDraw (axis);
		sd->enableComponent (QwtAbstractScaleDraw::Labels, on);
	}
}

void Graph::setMajorTicksType(const QList<int>& lst)
{
	if (getMajorTicksType() == lst)
		return;

	for (int i=0;i<(int)lst.count();i++)
	{
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (i);
		if (lst[i]==ScaleDraw::None || lst[i]==ScaleDraw::In)
			sd->enableComponent (QwtAbstractScaleDraw::Ticks, false);
		else
		{
			sd->enableComponent (QwtAbstractScaleDraw::Ticks);
			sd->setTickLength  	(QwtScaleDiv::MinorTick, minorTickLength());
			sd->setTickLength  	(QwtScaleDiv::MediumTick, minorTickLength());
			sd->setTickLength  	(QwtScaleDiv::MajorTick, majorTickLength());
		}
		sd->setMajorTicksStyle((ScaleDraw::TicksStyle)lst[i]);
	}
}

void Graph::setMajorTicksType(const QStringList& lst)
{
	for (int i=0; i<(int)lst.count(); i++)
		setMajorTicksType(i, lst[i].toInt());
}

void Graph::setMinorTicksType(const QList<int>& lst)
{
	if (getMinorTicksType() == lst)
		return;

	for (int i=0;i<(int)lst.count();i++)
		setMinorTicksType(i, lst[i]);
}

void Graph::setMinorTicksType(const QStringList& lst)
{
	for (int i=0;i<(int)lst.count();i++)
		setMinorTicksType(i,lst[i].toInt());
}

void Graph::setAxisTicksLength(int axis, int majTicksType, int minTicksType,
		int minLength, int majLength)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (!scale)
		return;

	setTickLength(minLength, majLength);

	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
	sd->setMajorTicksStyle((ScaleDraw::TicksStyle)majTicksType);
	sd->setMinorTicksStyle((ScaleDraw::TicksStyle)minTicksType);

	if (majTicksType == ScaleDraw::None && minTicksType == ScaleDraw::None)
		sd->enableComponent (QwtAbstractScaleDraw::Ticks, false);
	else
		sd->enableComponent (QwtAbstractScaleDraw::Ticks);

	if (majTicksType == ScaleDraw::None || majTicksType == ScaleDraw::In)
		majLength = 1;
	if (minTicksType == ScaleDraw::None || minTicksType == ScaleDraw::In)
		minLength = 0;

	sd->setTickLength (QwtScaleDiv::MinorTick, minLength);
	sd->setTickLength (QwtScaleDiv::MediumTick, minLength);
	sd->setTickLength (QwtScaleDiv::MajorTick, majLength);
}

void Graph::setTicksLength(int minLength, int majLength)
{
	QList<int> majTicksType = getMajorTicksType();
	QList<int> minTicksType = getMinorTicksType();

	for (int i=0; i<4; i++)
		setAxisTicksLength (i, majTicksType[i], minTicksType[i], minLength, majLength);
}

void Graph::changeTicksLength(int minLength, int majLength)
{
	if (minorTickLength() == minLength && majorTickLength() == majLength)
		return;

	setTicksLength(minLength, majLength);

	hide();
	for (int i = 0; i < 4; i++){
		if (axisEnabled(i)){
			enableAxis (i,false);
			enableAxis (i,true);
		}
	}
	replot();
	show();

	emit modifiedGraph();
}

void Graph::showAxis(int axis, int type, const QString& formatInfo, Table *table,
		bool axisOn, int majTicksType, int minTicksType, bool labelsOn,
		const QColor& c,  int format, int prec, int rotation, int baselineDist,
		const QString& formula, const QColor& labelsColor, int  spacing, bool backbone,
		const ScaleDraw::ShowTicksPolicy& showTicks, const QString& prefix, const QString& suffix)
{
	enableAxis(axis, axisOn);
	if (!axisOn)
		return;

	QList<int> majTicksTypeList = getMajorTicksType();
	QList<int> minTicksTypeList = getMinorTicksType();

	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);

	if (axisEnabled (axis) == axisOn &&
		majTicksTypeList[axis] == majTicksType &&
		minTicksTypeList[axis] == minTicksType &&
		axisColor(axis) == c &&
		axisLabelsColor(axis) == labelsColor &&
		prec == axisLabelPrecision (axis) &&
		format == axisLabelFormat (axis) &&
		labelsRotation(axis) == rotation &&
		(int)sd->scaleType() == type &&
		sd->formatString() == formatInfo &&
		sd->formula() == formula &&
		scale->margin() == baselineDist &&
		sd->hasComponent(QwtAbstractScaleDraw::Labels) == labelsOn &&
		sd->spacing() == spacing &&
		sd->hasComponent(QwtAbstractScaleDraw::Backbone) == backbone &&
		sd->showTicksPolicy() == showTicks &&
		sd->prefix() == prefix && sd->suffix() == suffix)
		return;

	scale->setMargin(baselineDist);
	QPalette pal = scale->palette();
	if (pal.color(QPalette::Active, QColorGroup::Foreground) != c)
		pal.setColor(QColorGroup::Foreground, c);
    if (pal.color(QPalette::Active, QColorGroup::Text) != labelsColor)
		pal.setColor(QColorGroup::Text, labelsColor);
    scale->setPalette(pal);

	if (axis == yLeft && d_grid->xZeroLineMarker()){
		QPen pen = d_grid->xZeroLineMarker()->linePen();
		pen.setColor(c);
		d_grid->xZeroLineMarker()->setLinePen(pen);
	} else if (axis == xBottom && d_grid->yZeroLineMarker()){
		QPen pen = d_grid->yZeroLineMarker()->linePen();
		pen.setColor(c);
		d_grid->yZeroLineMarker()->setLinePen(pen);
	}

	if (!labelsOn)
		sd->enableComponent (QwtAbstractScaleDraw::Labels, false);
	else {
		if (type == ScaleDraw::Numeric)
			setLabelsNumericFormat(axis, format, prec, formula);
		else if (type == ScaleDraw::Day)
			setLabelsDayFormat (axis, format);
		else if (type == ScaleDraw::Month)
			setLabelsMonthFormat (axis, format);
		else if (type == ScaleDraw::Time || type == ScaleDraw::Date)
			setLabelsDateTimeFormat (axis, type, formatInfo);
		else
			setLabelsTextFormat(axis, type, formatInfo, table);

		setAxisLabelRotation(axis, rotation);
	}

	sd = (ScaleDraw *)axisScaleDraw (axis);
	sd->enableComponent(QwtAbstractScaleDraw::Backbone, backbone);
	sd->setSpacing(spacing);
	sd->setShowTicksPolicy(showTicks);
	sd->setPrefix(prefix);
	sd->setSuffix(suffix);

	setAxisTicksLength(axis, majTicksType, minTicksType, minorTickLength(), majorTickLength());

	if (d_synchronize_scales && axisOn && (axis == QwtPlot::xTop || axis == QwtPlot::yRight))
		updateSecondaryAxis(axis);//synchronize scale divisions

	scalePicker->refresh();
	updateLayout();	//This is necessary in order to enable/disable tick labels
	scale->repaint();
	replot();
	emit modifiedGraph();
}

void Graph::setLabelsDayFormat(int axis, int format)
{
	ScaleDraw *sd = new ScaleDraw(this);
	sd->setDayFormat((ScaleDraw::NameFormat)format);
	sd->setScaleDiv(axisScaleDraw(axis)->scaleDiv());
	setAxisScaleDraw (axis, sd);
}

void Graph::setLabelsMonthFormat(int axis, int format)
{
	ScaleDraw *sd = new ScaleDraw(this);
	sd->setMonthFormat((ScaleDraw::NameFormat)format);
	sd->setScaleDiv(axisScaleDraw(axis)->scaleDiv());
	setAxisScaleDraw (axis, sd);
}

void Graph::setLabelsTextFormat(int axis, int type, const QString& name, const QStringList& lst)
{
	if (type != ScaleDraw::Text && type != ScaleDraw::ColHeader)
		return;

	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(axis);
	if (sd && sd->scaleType() == type && sd->labelsList() == lst && sd->formatString() == name)
		return;

	setAxisScaleDraw(axis, new ScaleDraw(this, lst, name, (ScaleDraw::ScaleType)type));
}

void Graph::setLabelsTextFormat(int axis, int type, const QString& labelsColName, Table *table)
{
	if (type != ScaleDraw::Text && type != ScaleDraw::ColHeader)
		return;

	QStringList list = QStringList();
	if (type == ScaleDraw::Text){
		if (!table)
			return;

		int r = table->numRows();
		int col = table->colIndex(labelsColName);
		for (int i=0; i < r; i++){
		    QString s = table->text(i, col);
		    if (!s.isEmpty())
                list << s;
		}
        setAxisScaleDraw(axis, new ScaleDraw(this, list, labelsColName, ScaleDraw::Text));
	} else if (type == ScaleDraw::ColHeader) {
		if (!table)
			return;

		for (int i=0; i<table->numCols(); i++){
			if (table->colPlotDesignation(i) == Table::Y)
				list << table->colLabel(i);
		}
        setAxisScaleDraw(axis, new ScaleDraw(this, list, table->objectName(), ScaleDraw::ColHeader));
	}
}

void Graph::setLabelsDateTimeFormat(int axis, int type, const QString& formatInfo)
{
	if (type < ScaleDraw::Time)
		return;

	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(axis);
	if (sd->scaleType() == type && sd->formatString() == formatInfo)
		return;

	ScaleDraw *nsd = 0;
	if (sd->scaleType() == type)
		nsd = new ScaleDraw(this, sd);
	else
		nsd = new ScaleDraw(this);

	if (type == ScaleDraw::Time)
		nsd->setTimeFormat(formatInfo);
	else if (type == ScaleDraw::Date)
		nsd->setDateFormat(formatInfo);

	nsd->enableComponent (QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
	setAxisScaleDraw (axis, nsd);
}

void Graph::recoverObsoleteDateTimeScale(int axis, int type, const QString& origin, const QString& format)
{
	QwtScaleDiv *div = this->axisScaleDiv(axis);
	double start = div->lowerBound();
	double end = div->upperBound();
	double step = d_user_step[axis];
	double newStep = 0.0;
	if (type == ScaleDraw::Date){
		QDateTime dt = QDateTime::fromString(origin, Qt::ISODate);
		if (dt.isNull())
			dt = QDateTime::fromString(origin, format);
		QDateTime sdt = dt.addSecs(int(start));
		QDateTime edt = dt.addSecs(int(end));
		if (step != 0.0)
			newStep = step/864.0e2;
		setAxisScale(axis, Table::fromDateTime(sdt), Table::fromDateTime(edt), newStep);
	} else if (type == ScaleDraw::Time){
		QTime t = QTime::fromString(origin, Qt::ISODate);
		QTime st = t.addMSecs(int(start));
		QTime et = t.addMSecs(int(end));
		if (step != 0.0)
			newStep = step/864.0e5;	
		setAxisScale(axis, Table::fromTime(st), Table::fromTime(et), newStep);
	}
	d_user_step[axis] = newStep;
}

void Graph::setAxisLabelRotation(int axis, int rotation)
{
	if (axis == QwtPlot::xBottom){
		if (rotation > 0)
			setAxisLabelAlignment(axis, Qt::AlignRight|Qt::AlignVCenter);
		else if (rotation < 0)
			setAxisLabelAlignment(axis, Qt::AlignLeft|Qt::AlignVCenter);
		else if (rotation == 0)
			setAxisLabelAlignment(axis, Qt::AlignHCenter|Qt::AlignBottom);
	} else if (axis == QwtPlot::xTop){
		if (rotation > 0)
			setAxisLabelAlignment(axis, Qt::AlignLeft|Qt::AlignVCenter);
		else if (rotation < 0)
			setAxisLabelAlignment(axis, Qt::AlignRight|Qt::AlignVCenter);
		else if (rotation == 0)
			setAxisLabelAlignment(axis, Qt::AlignHCenter|Qt::AlignTop);
	}
	((QwtPlot *)this)->setAxisLabelRotation (axis, rotation);
}

int Graph::labelsRotation(int axis)
{
	ScaleDraw *sclDraw = (ScaleDraw *)axisScaleDraw (axis);
	return (int)sclDraw->labelRotation();
}

void Graph::setAxisTitleFont(int axis,const QFont &fnt)
{
	QwtText t = axisTitle (axis);
	t.setFont (fnt);
	((QwtPlot *)this)->setAxisTitle(axis, t);
	replot();
	emit modifiedGraph();
}

QFont Graph::axisTitleFont(int axis)
{
	return axisTitle(axis).font();
}

QColor Graph::axisTitleColor(int axis)
{
	QColor c;
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (scale)
		c = scale->title().color();
	return c;
}

void Graph::setAxisLabelsColor(int axis, const QColor& color)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (scale){
		QPalette pal = scale->palette();
		pal.setColor(QColorGroup::Text, color);
		scale->setPalette(pal);
  	}
}

void Graph::setAxisColor(int axis, const QColor& color)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (!scale)
		return;

	QPalette pal = scale->palette();
	pal.setColor(QColorGroup::Foreground, color);
	scale->setPalette(pal);

	if (axis == yLeft && d_grid->xZeroLineMarker()){
		QPen pen = d_grid->xZeroLineMarker()->linePen();
		pen.setColor(color);
		d_grid->xZeroLineMarker()->setLinePen(pen);
	} else if (axis == xBottom && d_grid->yZeroLineMarker()){
		QPen pen = d_grid->yZeroLineMarker()->linePen();
		pen.setColor(color);
		d_grid->yZeroLineMarker()->setLinePen(pen);
	}
}

QString Graph::saveAxesColors()
{
	QString s="AxesColors\t";
	QStringList colors, numColors;
	QPalette pal;
	int i;
	for (i=0;i<4;i++)
    {
		colors<<QColor(Qt::black).name();
        numColors<<QColor(Qt::black).name();
    }

	for (i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
		if (scale)
		{
			pal=scale->palette();
			colors[i]=pal.color(QPalette::Active, QColorGroup::Foreground).name();
            numColors[i]=pal.color(QPalette::Active, QColorGroup::Text).name();
		}
	}
	s+=colors.join ("\t")+"\n";
    s+="AxesNumberColors\t"+numColors.join ("\t")+"\n";
	return s;
}

QColor Graph::axisColor(int axis)
{
    QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
    if (scale)
  	     return scale->palette().color(QPalette::Active, QColorGroup::Foreground);
  	else
  	     return QColor(Qt::black);
}

QColor Graph::axisLabelsColor(int axis)
{
    QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
 	if (scale)
  	     return scale->palette().color(QPalette::Active, QColorGroup::Text);
  	else
  	     return QColor(Qt::black);
}

void Graph::setTitleColor(const QColor & c)
{
	QwtText t = title();
	t.setColor(c);
	setTitle(t);
	replot();
	emit modifiedGraph();
}

void Graph::setTitleAlignment(int align)
{
	QwtText t = title();
	t.setRenderFlags(align);
	setTitle(t);
	replot();
	emit modifiedGraph();
}

void Graph::setTitleFont(const QFont &fnt)
{
	QwtText t = title();
	t.setFont(fnt);
	setTitle(t);
	replot();
	emit modifiedGraph();
}

void Graph::setYAxisTitle(const QString& text)
{
	setAxisTitle(QwtPlot::yLeft, text);
	replot();
	emit modifiedGraph();
}

void Graph::setXAxisTitle(const QString& text)
{
	setAxisTitle(QwtPlot::xBottom, text);
	replot();
	emit modifiedGraph();
}

void Graph::setRightAxisTitle(const QString& text)
{
	setAxisTitle(QwtPlot::yRight, text);
	replot();
	emit modifiedGraph();
}

void Graph::setTopAxisTitle(const QString& text)
{
	setAxisTitle(QwtPlot::xTop, text);
	replot();
	emit modifiedGraph();
}

int Graph::axisTitleAlignment (int axis)
{
	return axisTitle(axis).renderFlags();
}

void Graph::setAxisTitleAlignment(int axis, int align)
{
	QwtText t = axisTitle(axis);
	t.setRenderFlags(align);
	((QwtPlot *)this)->setAxisTitle(axis, t);
}

int Graph::axisTitleDistance(int axis)
{
	if (!axisEnabled(axis))
		return 0;

	return axisWidget(axis)->spacing();
}

void Graph::setAxisTitleDistance(int axis, int dist)
{
	if (!axisEnabled(axis))
		return;

	QwtScaleWidget *scale = axisWidget(axis);
	if (scale)
		scale->setSpacing(dist);
}

void Graph::setScaleTitle(int axis, const QString& text)
{
	int a = 0;
	switch (axis){
		case 0:
			a=2;
        break;
		case 1:
			a=0;
        break;
		case 2:
			a=3;
        break;
		case 3:
			a=1;
        break;
	}
	setAxisTitle(a, text);
}

QString Graph::parseAxisTitle(int axis)
{
	if (axis < 0 || axis >= QwtPlot::axisCnt)
		return QString::null;

	QString s = d_axis_titles[axis];
	if (s.trimmed().isEmpty() || !(s.contains("%(?X)", Qt::CaseInsensitive) || s.contains("%(?Y)", Qt::CaseInsensitive)))
		return s;

	QString name = QString::null;
	QString comment = QString::null;
	if (s.contains("%(?Y)", Qt::CaseInsensitive)){// parse Origin tag
		PlotCurve *c = NULL;
		int index = -1;
		for (int i = 0; i < d_curves.size(); i++){
			PlotCurve *cv = curve(i);
			if (cv && cv->yAxis() == axis){
				c = cv;
				index = i;
				break;
			}
		}
		if (!c)
			c = curve(0);

		if (c){
			name = c->title().text();
			int pos = name.lastIndexOf("_");
			if (pos > 0)
				name = name.right(name.length() - pos - 1);

			if (d_axis_title_policy > 1){
				DataCurve *dc = dataCurve(index);
				if (dc){
					Table *t = dc->table();
					if (t)
						comment = t->comment(t->colIndex(name)).trimmed().replace("\n", " ");
				}
			}

			switch(d_axis_title_policy){
				case Default:
					s.replace("%(?Y)", tr("Y Axis Title"), Qt::CaseInsensitive);
				break;
				case ColName:
					s.replace("%(?Y)", name, Qt::CaseInsensitive);
				break;
				case ColComment:
					if (!comment.isEmpty())
						s.replace("%(?Y)", comment, Qt::CaseInsensitive);
					else
						s.replace("%(?Y)", name, Qt::CaseInsensitive);
				break;
				case NameAndComment:
					if (!comment.isEmpty())
						name += " (" + comment + ")";
					s.replace("%(?Y)", name, Qt::CaseInsensitive);
				break;
				default:
					break;
			}
		} else
			s.replace("%(?Y)", tr("Y Axis Title"), Qt::CaseInsensitive);
	}

	if (s.contains("%(?X)", Qt::CaseInsensitive)){
		DataCurve *c = dataCurve(0);
		if (c){
			name = c->xColumnName();
			int pos = name.lastIndexOf("_");
			if (pos > 0)
				name = name.right(name.length() - pos - 1);

			if (d_axis_title_policy > 1){
				Table *t = c->table();
				if (t)
					comment = t->comment(t->colIndex(c->xColumnName())).trimmed().replace("\n", " ");
			}

			switch(d_axis_title_policy){
				case Default:
					s.replace("%(?X)", tr("X Axis Title"), Qt::CaseInsensitive);
				break;
				case ColName:
				  s.replace("%(?X)", name, Qt::CaseInsensitive);
				break;
				case ColComment:
					if (!comment.isEmpty())
						s.replace("%(?X)", comment, Qt::CaseInsensitive);
					else
						s.replace("%(?X)", name, Qt::CaseInsensitive);
				break;
				case NameAndComment:
				  if (!comment.isEmpty())
					  name += " (" + comment + ")";
				  s.replace("%(?X)", name, Qt::CaseInsensitive);
				break;
				default:
				  break;
			}
		} else
			s.replace("%(?X)", tr("X Axis Title"), Qt::CaseInsensitive);
	}
	return s;
}

QString Graph::axisTitleString(int axis)
{
	if (axis >= 0 && axis < d_axis_titles.size())
		return d_axis_titles[axis];

	return QString::null;
}

void Graph::setAxisTitleString(int axis, const QString& text)
{
	if (axis >= 0 && axis < d_axis_titles.size())
		d_axis_titles[axis] = text;

	((QwtPlot *)this)->setAxisTitle(axis, parseAxisTitle(axis));
}

void Graph::setAxisTitle(int axis, const QString& text)
{
	setAxisTitleString(axis, text);
	replot();
	emit modifiedGraph();
}

void Graph::updateSecondaryAxis(int axis, bool changeFormat)
{
	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
			Spectrogram *sp = (Spectrogram *)it;
			if (sp->colorScaleAxis() == axis)
				return;
		}

		if ((axis == QwtPlot::yRight && it->yAxis() == QwtPlot::yRight) ||
			(axis == QwtPlot::xTop && it->xAxis () == QwtPlot::xTop))
			return;
	}

	int a = oppositeAxis(axis);
	if (!axisEnabled(a))
		return;

	if (changeFormat && axisScaleDraw(axis)->hasComponent(QwtAbstractScaleDraw::Labels)){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(a);
		ScaleDraw::ScaleType type = sd->scaleType();
		if (type == ScaleDraw::Day)
			setLabelsDayFormat(axis, sd->nameFormat());
		else if (type == ScaleDraw::Month)
			setLabelsMonthFormat(axis, sd->nameFormat());
		else if (type == ScaleDraw::Time || type == ScaleDraw::Date)
			setLabelsDateTimeFormat(axis, type, sd->formatString());
		else
			setAxisScaleDraw(axis, new ScaleDraw(this, sd));
	}

	updateOppositeScaleDiv(axis);
}

int Graph::oppositeAxis(int axis)
{
	int a = -1;
	switch (axis){
		case QwtPlot::yLeft:
			a = QwtPlot::yRight;
		break;
		case QwtPlot::yRight:
			a = QwtPlot::yLeft;
		break;
		case QwtPlot::xBottom:
			a = QwtPlot::xTop;
		break;
		case QwtPlot::xTop:
			a = QwtPlot::xBottom;
		break;
	}
	return a;
}

void Graph::updateOppositeScaleDiv(int axis)
{
	int a = oppositeAxis(axis);
	if (!axisEnabled(a))
		return;

	ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(axis);
	sc_engine->clone((ScaleEngine *)axisScaleEngine(a));

	int minorTicks = axisMaxMinor(a);
	int max_min_intervals = minorTicks;
	if (minorTicks == 1)
		max_min_intervals = 3;
	if (minorTicks > 1)
		max_min_intervals = minorTicks + 1;

	setAxisMaxMajor(axis, axisMaxMajor(a));
	setAxisMaxMinor(axis, minorTicks);

	setAxisScaleDiv (axis, *axisScaleDiv(a));
	d_user_step[axis] = d_user_step[a];
}

void Graph::invertScale(int axis)
{
	QwtScaleDiv *scaleDiv = axisScaleDiv(axis);
	if (scaleDiv)
		scaleDiv->invert();
}

QwtDoubleInterval Graph::axisBoundingInterval(int axis)
{
	// Find bounding interval of the plot data

	QwtDoubleInterval intv;
	const QwtPlotItemList& itmList = itemList();
	QwtPlotItemIterator it;
	for ( it = itmList.begin(); it != itmList.end(); ++it ){
		const QwtPlotItem *item = *it;
		if (item->rtti() != QwtPlotItem::Rtti_PlotCurve)
			continue;

		if(axis != item->xAxis() && axis != item->yAxis())
			continue;

		const QwtDoubleRect rect = item->boundingRect();

		if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
			intv |= QwtDoubleInterval(rect.left(), rect.right());
		else
			intv |= QwtDoubleInterval(rect.top(), rect.bottom());
	}
	return intv;
}

void Graph::setScale(int axis, double start, double end, double step,
					int majorTicks, int minorTicks, int type, bool inverted,
					double left_break, double right_break, int breakPos,
					double stepBeforeBreak, double stepAfterBreak, int minTicksBeforeBreak,
					int minTicksAfterBreak, bool log10AfterBreak, int breakWidth, bool breakDecoration)
{
	ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(axis);
	sc_engine->setBreakRegion(left_break, right_break);
	sc_engine->setBreakPosition(breakPos);
	sc_engine->setBreakWidth(breakWidth);
	sc_engine->drawBreakDecoration(breakDecoration);
	sc_engine->setStepBeforeBreak(stepBeforeBreak);
	sc_engine->setStepAfterBreak(stepAfterBreak);
	sc_engine->setMinTicksBeforeBreak(minTicksBeforeBreak);
	sc_engine->setMinTicksAfterBreak(minTicksAfterBreak);
	sc_engine->setLog10ScaleAfterBreak(log10AfterBreak);
	sc_engine->setAttribute(QwtScaleEngine::Inverted, inverted);
	sc_engine->setType((ScaleTransformation::Type)type);

	bool limitInterval = false;
	switch(type){
		case ScaleTransformation::Log10:
		case ScaleTransformation::Ln:
		case ScaleTransformation::Log2:
			if (start <= 0 || end <= 0)
				limitInterval = true;
		break;
		case ScaleTransformation::Reciprocal:
			if (start == 0 || end == 0)
				limitInterval = true;
		break;
		default:
			break;
	}
	if (limitInterval){
		QwtDoubleInterval intv = axisBoundingInterval(axis);
		if (start < end)
			start = intv.minValue();
		else
			end = intv.minValue();

		if (start <= 0.0)
			start = 1e-4;
		if (end <= 0.0)
			end = 1e-3;
	}

	int max_min_intervals = minorTicks;
	if (minorTicks == 1)
		max_min_intervals = 3;
	if (minorTicks > 1)
		max_min_intervals = minorTicks + 1;

	QwtScaleDiv div = sc_engine->divideScale (QMIN(start, end), QMAX(start, end), majorTicks, max_min_intervals, step);
	setAxisMaxMajor(axis, majorTicks);
	setAxisMaxMinor(axis, minorTicks);

	if (inverted)
		div.invert();
	setAxisScaleDiv(axis, div);

	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();

	d_user_step[axis] = step;

	if (d_synchronize_scales){
		if (axis == QwtPlot::xBottom)
			updateSecondaryAxis(QwtPlot::xTop);
		else if (axis == QwtPlot::yLeft)
			updateSecondaryAxis(QwtPlot::yRight);
	}

	replot();

	updateMarkersBoundingRect();//keep markers on canvas area

	replot();
	axisWidget(axis)->repaint();
	emit axisDivChanged(this, axis);
}

void Graph::setCanvasCoordinates(const QRectF& r)
{
    setScale(QwtPlot::xBottom, r.left(), r.right());
    setScale(QwtPlot::yLeft, r.top(), r.top() - r.height());
}

QStringList Graph::analysableCurvesList()
{
	QStringList cList;
	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
			PlotCurve *c = (PlotCurve*)it;
			if (c->type() == ErrorBars)
				continue;
			if (c->type() != Function){
				DataCurve *dc = (DataCurve*)it;
				cList << dc->title().text() + " [" + QString::number(dc->startRow() + 1) + ":" + QString::number(dc->endRow() + 1) + "]";
			} else
				cList << c->title().text();
		}
	}
	return cList;
}

QString Graph::curveRange(QwtPlotCurve *c)
{
	if (!c)
		return QString();
	PlotCurve *pc = (PlotCurve *)c;
	if (pc->type() == ErrorBars)
		return QString();

	if (pc->type() != Function){
		DataCurve *dc = (DataCurve*)c;
		return dc->title().text() + " [" + QString::number(dc->startRow() + 1) + ":" + QString::number(dc->endRow() + 1) + "]";
	}
	return c->title().text();
}

QStringList Graph::curveNamesList()
{
	QStringList cList;
	foreach(QwtPlotItem *it, d_curves){
    	if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
			cList << it->title().text();
	}
	return cList;
}

QStringList Graph::plotItemsList()
{
	if (d_curves.isEmpty())
		return QStringList();

  	QStringList cList;
	foreach(QwtPlotItem *it, d_curves)
    	cList << it->title().text();
	return cList;
}

void Graph::copyImage()
{
#ifdef Q_OS_WIN
	if (OpenClipboard(0)){
		EmptyClipboard();

		QString path = QDir::tempPath();
		QString name = path + "/" + "qtiplot_clipboard.emf";
		name = QDir::cleanPath(name);
		exportEMF(name);
		HENHMETAFILE handle = GetEnhMetaFile(name.toStdWString().c_str());

		SetClipboardData(CF_ENHMETAFILE, handle);
		CloseClipboard();
		QFile::remove(name);
	}
#else
	QApplication::clipboard()->setPixmap(graphPixmap(), QClipboard::Clipboard);
#endif
}

QPixmap Graph::graphPixmap(const QSize& size, double scaleFontsFactor, bool transparent)
{
	QRect r = rect();
	QRect br = boundingRect();
	if (!size.isValid()){
		QPixmap pixmap(br.size());
		if (transparent)
			pixmap.fill(Qt::transparent);
		else
			pixmap.fill();
		QPainter p(&pixmap);
		print(&p, r, ScaledFontsPrintFilter(1.0));
		p.end();
		return pixmap;
	}

	if (br.width() != width() || br.height() != height()){
		double wfactor = (double)br.width()/(double)width();
		double hfactor = (double)br.height()/(double)height();
		r.setSize(QSize(qRound(size.width()/wfactor), qRound(size.height()/hfactor)));
	} else
		r.setSize(size);

	if (scaleFontsFactor == 0)
		scaleFontsFactor = (double)size.height()/(double)height();

	QPixmap pixmap(size);
	if (transparent)
		pixmap.fill(Qt::transparent);
	else
		pixmap.fill();
	QPainter p(&pixmap);
	print(&p, r, ScaledFontsPrintFilter(scaleFontsFactor));
	p.end();

	return pixmap;
}

void Graph::exportToFile(const QString& fileName)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

	if (fileName.contains(".eps") || fileName.contains(".pdf") || fileName.contains(".ps")){
		exportVector(fileName);
		return;
	} else if(fileName.contains(".svg")){
		exportSVG(fileName);
		return;
	}
#ifdef TEX_OUTPUT
	else if(fileName.contains(".tex")){
		exportTeX(fileName);
		return;
	}
#endif
	 else if(fileName.contains(".emf")){
		exportEMF(fileName);
		return;
	} else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
    	for(int i=0 ; i<list.count() ; i++){
			if (fileName.contains( "." + list[i].toLower())){
				exportImage(fileName);
				return;
			}
		}
    	QMessageBox::critical(this, tr("QtiPlot - Error"), tr("File format not handled, operation aborted!"));
	}
}

void Graph::exportImage(const QString& fileName, int quality, bool transparent, int dpi,
						const QSizeF& customSize, int unit, double fontsFactor, int compression)
{
	if (!dpi)
		dpi = logicalDpiX();

	QSize size = QSize();
	if (customSize.isValid())
		size = customPrintSize(customSize, unit, dpi);

	QPixmap pic = graphPixmap(size, fontsFactor, transparent);
	QImage image = pic.toImage();
	int dpm = (int)ceil(100.0/2.54*dpi);
	image.setDotsPerMeterX(dpm);
	image.setDotsPerMeterY(dpm);

#if QT_VERSION >= 0x040500
	if (fileName.endsWith(".odf")){
		QTextDocument *document = new QTextDocument();
		QTextCursor cursor = QTextCursor(document);
		cursor.movePosition(QTextCursor::End);
		MultiLayer *ml = multiLayer();
		if (ml)
			cursor.insertText(ml->objectName() + ", " + tr("layer") + " " + QString::number(ml->layerIndex(this) + 1));
		cursor.insertBlock();
		cursor.insertImage(image);

		QTextDocumentWriter writer(fileName);
		writer.write(document);
	} else
#endif
	{
		QImageWriter writer(fileName);
		if (compression > 0 && writer.supportsOption(QImageIOHandler::CompressionRatio)){
			writer.setQuality(quality);
			writer.setCompression(compression);
			writer.write(image);
		} else
			image.save(fileName, 0, quality);
	}
}

void Graph::exportVector(QPrinter *printer, int res, bool color,
						const QSizeF& customSize, int unit, double fontsFactor)
{
	if (!printer)
		return;
	if (!printer->resolution())
		printer->setResolution(logicalDpiX());//we set screen resolution as default

	printer->setDocName(multiLayer()->objectName());
	printer->setFontEmbeddingEnabled(true);
	printer->setCreator("QtiPlot");
	printer->setFullPage(true);
	QRect r = rect();
	QRect br = boundingRect();

	if (customSize.isValid()){
		QSize size = customPrintSize(customSize, unit, res);

		if (fontsFactor == 0.0)
			fontsFactor = customPrintSize(customSize, unit, logicalDpiX()).height()/(double)height();

		if (res && res != printer->resolution())
			printer->setResolution(res);
		printer->setPaperSize (QSizeF(size), QPrinter::DevicePixel);

		if (br.width() != width() || br.height() != height()){
			double wfactor = (double)br.width()/(double)width();
			double hfactor = (double)br.height()/(double)height();
			r.setSize(QSize(qRound(size.width()/wfactor), qRound(size.height()/hfactor)));
		} else
			r.setSize(size);
	} else if (res && res != printer->resolution()){
		double wfactor = (double)res/(double)logicalDpiX();
		double hfactor = (double)res/(double)logicalDpiY();
		printer->setResolution(res);

		// LegendWidget size doesn't increase linearly with resolution.
		// The extra width multiplication factor bellow accounts for this.
		// We could calculate it precisely, but it's quite complicated...
		printer->setPaperSize (QSizeF(br.width()*wfactor*1.05, br.height()*hfactor), QPrinter::DevicePixel);
		r.setSize(QSize(qRound(width()*wfactor), qRound(height()*hfactor)));
	} else
		printer->setPaperSize (QSizeF(br.size()), QPrinter::DevicePixel);

	if (color)
		printer->setColorMode(QPrinter::Color);
	else
		printer->setColorMode(QPrinter::GrayScale);

	printer->setOrientation(QPrinter::Portrait);

	QPainter paint(printer);
	print(&paint, r, ScaledFontsPrintFilter(fontsFactor));
	paint.end();
}

void Graph::exportVector(const QString& fileName, int res, bool color,
						const QSizeF& customSize, int unit, double fontsFactor)
{
	if (fileName.isEmpty()){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
		return;
	}

	QPrinter printer;
	printer.setOutputFileName(fileName);
	if (fileName.contains(".eps"))
		printer.setOutputFormat(QPrinter::PostScriptFormat);

	exportVector(&printer, res, color, customSize, unit, fontsFactor);
}

void Graph::print()
{
	QPrinter printer;
	printer.setColorMode (QPrinter::Color);
	printer.setFullPage(true);
#ifdef Q_OS_LINUX
	printer.setOutputFileName(multiLayer()->objectName());
#endif
	//printing should preserve plot aspect ratio, if possible
	double aspect = double(width())/double(height());
	if (aspect < 1)
		printer.setOrientation(QPrinter::Portrait);
	else
		printer.setOrientation(QPrinter::Landscape);

	QPrintDialog printDialog(&printer, multiLayer()->applicationWindow());
    if (printDialog.exec() == QDialog::Accepted){
	#ifdef Q_OS_LINUX
		if (printDialog.enabledOptions() & QAbstractPrintDialog::PrintToFile){
			QString fn = printer.outputFileName();
			if (printer.outputFormat() == QPrinter::PostScriptFormat && !fn.contains(".ps"))
				printer.setOutputFileName(fn + ".ps");
			else if (printer.outputFormat() == QPrinter::PdfFormat && !fn.contains(".pdf"))
				printer.setOutputFileName(fn + ".pdf");
		}
	#endif

		QRect plotRect = rect();
		QRect paperRect = printer.paperRect();
		double fontFactor = 1.0;
		if (multiLayer()->scaleLayersOnPrint()){
			int margin = (int)((1/2.54)*printer.logicalDpiY()); // 1 cm margins

			int width = qRound(aspect*printer.height()) - 2*margin;
			int x = qRound(abs(printer.width()- width)*0.5);

			plotRect = QRect(x, margin, width, printer.height() - 2*margin);
			if (x < margin){
				plotRect.setLeft(margin);
				plotRect.setWidth(printer.width() - 2*margin);
			}

			fontFactor = (double)plotRect.height()/(double)this->height();
		} else {
			int x_margin = (paperRect.width() - plotRect.width())/2;
			if (x_margin <= 0)
				x_margin = (int)((0.5/2.54)*printer.logicalDpiY()); // 0.5 cm margins
			int y_margin = (paperRect.height() - plotRect.height())/2;
			if (y_margin <= 0)
				y_margin = x_margin;
			plotRect.moveTo(x_margin, y_margin);
		}

        QPainter paint(&printer);
		if (multiLayer()->printCropmarksEnabled()){
			QRect cr = plotRect; // cropmarks rectangle
			cr.addCoords(-1, -1, 2, 2);
            paint.save();
            paint.setPen(QPen(QColor(Qt::black), 0.5, Qt::DashLine));
            paint.drawLine(paperRect.left(), cr.top(), paperRect.right(), cr.top());
            paint.drawLine(paperRect.left(), cr.bottom(), paperRect.right(), cr.bottom());
            paint.drawLine(cr.left(), paperRect.top(), cr.left(), paperRect.bottom());
            paint.drawLine(cr.right(), paperRect.top(), cr.right(), paperRect.bottom());
            paint.restore();
        }

		print(&paint, plotRect, ScaledFontsPrintFilter(fontFactor));
	}
}

void Graph::exportSVG(const QString& fname, const QSizeF& customSize, int unit, double fontsFactor)
{
	int res = 96;
#ifdef Q_OS_MAC
    res = 72;
#endif

	QSize size = boundingRect().size();
	if (customSize.isValid())
		size = Graph::customPrintSize(customSize, unit, res);

	QSvgGenerator svg;
	svg.setFileName(fname);
	svg.setSize(size);
	svg.setResolution(res);

	draw(&svg, size, fontsFactor);
}

void Graph::draw(QPaintDevice *device, const QSize& size, double fontsFactor)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (fontsFactor == 0.0)
		fontsFactor = size.height()/(double)height();

	QRect r = rect();
	QRect br = boundingRect();
	if (size != br.size()){
		if (br.width() != width() || br.height() != height()){
			double wfactor = (double)br.width()/(double)width();
			double hfactor = (double)br.height()/(double)height();
			r.setSize(QSize(qRound(size.width()/wfactor), qRound(size.height()/hfactor)));
		} else
			r.setSize(size);
	}

	QPainter p(device);
	print(&p, r, ScaledFontsPrintFilter(fontsFactor));
	p.end();

	QApplication::restoreOverrideCursor();
}

void Graph::exportEMF(const QString& fname, const QSizeF& customSize, int unit, double fontsFactor)
{
	if (!multiLayer()->applicationWindow())
		return;
	ImportExportPlugin *ep = multiLayer()->applicationWindow()->exportPlugin("emf");
	if (!ep)
		return;

	ep->exportGraph(this, fname, customSize, unit, fontsFactor);
}

void Graph::exportTeX(const QString& fname, bool color, bool escapeStrings, bool fontSizes, const QSizeF& customSize, int unit, double fontsFactor)
{
#ifdef TEX_OUTPUT
	int res = logicalDpiX();
	QSize size = boundingRect().size();
	if (customSize.isValid())
		size = Graph::customPrintSize(customSize, unit, res);

	d_is_exporting_tex = true;
	d_tex_escape_strings = escapeStrings;

	QTeXPaintDevice tex(fname, size);
	tex.setEscapeTextMode(false);
	tex.exportFontSizes(fontSizes);
	if (!color)
		tex.setColorMode(QPrinter::GrayScale);

	if (!fontSizes)
		fontsFactor = 1.0;

	draw(&tex, size, fontsFactor);

	d_is_exporting_tex = false;
#endif
}

bool Graph::markerSelected()
{
	if (d_selected_arrow)
		return true;
	if (d_active_enrichment)
		return true;
	return false;
}

void Graph::removeMarker()
{
	if (d_selected_arrow && d_lines.contains(d_selected_arrow))
		remove(d_selected_arrow);
	else if (d_markers_selector){
		QList <QWidget *> lst = d_markers_selector->widgetsList();
		foreach(QWidget *w, lst){
			FrameWidget *fw = qobject_cast<FrameWidget *>(w);
			if (fw)
				remove(fw);
		}
	}
}

void Graph::remove(ArrowMarker* arrow)
{
	if (!arrow)
		return;

	if (d_markers_selector && d_lines.contains(arrow))
		d_markers_selector->removeAll(arrow);

	if (d_lines.contains(arrow)){
		cp->disableEditing();

		int index = d_lines.indexOf(arrow);
		if (index >= 0 && index < d_lines.size())
			d_lines.removeAt(index);
	}

	if (arrow == d_selected_arrow)
		d_selected_arrow = NULL;

	arrow->detach();
	replot();

	emit modifiedGraph();
}

void Graph::remove(FrameWidget* f)
{
	if (!f)
		return;

    PieLabel *l = qobject_cast<PieLabel *>(f);
    if (!l){
        int index = d_enrichments.indexOf (f);
        if (index >= 0 && index < d_enrichments.size())
            d_enrichments.removeAt(index);

        if (f == d_active_enrichment)
            d_active_enrichment = NULL;
    }

	emit modifiedGraph();
	f->close();
}

bool Graph::arrowMarkerSelected()
{
	return (d_selected_arrow && d_lines.contains(d_selected_arrow));
}

bool Graph::imageMarkerSelected()
{
	ImageWidget *i = qobject_cast<ImageWidget *>(d_active_enrichment);
	if (i)
		return true;
	return false;
}

void Graph::deselect()
{
	deselectMarker();
	scalePicker->deselect();
	titlePicker->setSelected(false);
	deselectCurves();
	if (multiLayer())
		multiLayer()->deselect();
}

void Graph::deselectCurves()
{
	QList<QwtPlotItem *> curves = curvesList();
	foreach(QwtPlotItem *i, curves){
		if(i->rtti() == QwtPlotItem::Rtti_PlotSpectrogram &&
			((Spectrogram *)i)->hasSelectedLabels()){
			((Spectrogram *)i)->selectLabel(false);
		} else if (i->rtti() == QwtPlotItem::Rtti_PlotCurve && ((PlotCurve *)i)->type() != Graph::Function){
			((DataCurve *)i)->setLabelsSelected(false);
		}
	}
}

QwtPlotItem* Graph::selectedCurveLabels()
{
	QList<QwtPlotItem *> curves = curvesList();
	foreach(QwtPlotItem *i, curves){
		if(i->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && ((Spectrogram *)i)->hasSelectedLabels())
			return i;

		if(i->rtti() == QwtPlotItem::Rtti_PlotCurve && ((PlotCurve *)i)->type() != Graph::Function &&
		  ((DataCurve *)i)->hasSelectedLabels())
			return i;
	}
	return NULL;
}

bool Graph::titleSelected()
{
	return titlePicker->selected();
}

void Graph::selectTitle(bool select)
{
    titlePicker->setSelected(select);

    if (select){
        deselect();
        emit selectedGraph(this);
		currentFontChanged(title().font());
		currentColorChanged(title().color());
    }
}

void Graph::clearTitle()
{
	setTitle(" ");
	emit modifiedGraph();
}

void Graph::removeTitle()
{
	setTitle("");
	emit modifiedGraph();
}

void Graph::initTitle(bool on, const QFont& fnt)
{
	if (on){
		QwtText t = title();
		t.setFont(fnt);
		t.setText(tr("Title"));
		setTitle (t);
	}
}

QString Graph::legendText(bool layerSpec, int fromIndex)
{
	QString text = QString();

	if (layerSpec){
		int layerIndex = 1;
		MultiLayer *ml = multiLayer();
		if (ml)
			layerIndex = ml->layerIndex(this);

		for (int i = fromIndex; i < d_curves.size(); i++){
			PlotCurve* c = curve(i);
			if (!c)
				continue;

			if (c->type() != ErrorBars){
				text += "\\l(";
				text += QString::number(layerIndex + 1);
				text += ".";
				text += QString::number(i + 1);
				text += ")%(";
				text += QString::number(layerIndex + 1);
				text += ".";
				text += QString::number(i + 1);
				text += ")\n";
				i++;
			}
		}
	} else {
		for (int i = fromIndex; i < d_curves.size(); i++){
			PlotCurve* c = curve(i);
			if (!c)
				continue;

			int type = c->type();
			if (type == Function){
				text += "\\l(";
				text += QString::number(i + 1);
				text += ")%(";
				text += QString::number(i + 1);
				text += ")\n";
				continue;
			}

			if (type != ErrorBars){
				text += "\\l(";
				text += QString::number(i + 1);
				text += ")%(";
				text += QString::number(i + 1);

				LegendDisplayMode mode = ColumnName;
				ApplicationWindow *app = multiLayer()->applicationWindow();
				if (app)
					mode = app->d_graph_legend_display;

				switch(mode){
					case ColumnName:
						text += ",@C";
					break;
					case ColumnComment:
						text += ",@L";
					break;
					case TableName:
						text += ",@W";
					break;
					case TableLegend:
						text += ",@WL";
					break;
					case DataSetName:
						text += ",@D";
					break;
					default:
					break;
				}

				text += ")\n";
			}
		}
	}
	return text.trimmed();
}

QString Graph::pieLegendText()
{
	if (d_curves.isEmpty())
		return QString::null;

	QString text;
	const QwtPlotCurve *c = (QwtPlotCurve *)d_curves.first();
	if (c){
		for (int i = 0; i < int(c->dataSize()); i++){
			text += "\\l(1,";
			text += QString::number(i+1);
			text += ") %(1,@L,";
			text += QString::number(i+1);
			text += ")\n";
		}
	}
	return text.trimmed();
}

void Graph::updateCurvesData(Table* w, const QString& yColName)
{
	int updated_curves = 0;
	foreach(QwtPlotItem *it, d_curves){
    	if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
			PlotCurve *c = (PlotCurve*)it;
			if (c->type() == Function)
				continue;
			if(((DataCurve *)it)->updateData(w, yColName))
            	updated_curves++;
		}
	}

    if (updated_curves){
        for (int i = 0; i < QwtPlot::axisCnt; i++){
			QwtScaleWidget *scale = axisWidget(i);
			if (scale)
                connect(scale, SIGNAL(scaleDivChanged()), this, SLOT(updateMarkersBoundingRect()));
		}
        updatePlot();
    }
}

void Graph::reloadCurvesData()
{
	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
			PlotCurve *c = (PlotCurve*)it;
			if (c->type() == Function)
				continue;
			((DataCurve *)it)->loadData();
		}
	}
}

QString Graph::saveEnabledAxes()
{
	QString list="EnabledAxes\t";
	for (int i = 0;i<QwtPlot::axisCnt;i++)
		list+=QString::number(axisEnabled (i))+"\t";

	list+="\n";
	return list;
}

QColor Graph::canvasFrameColor()
{
	QPalette pal = canvas()->palette();
	return pal.color(QPalette::Active, QColorGroup::Foreground);
}

int Graph::canvasFrameWidth()
{
	return canvas()->lineWidth();
}

void Graph::setCanvasFrame(int width, const QColor& color)
{
	QwtPlotCanvas* canvas = (QwtPlotCanvas*)this->canvas();
	QPalette pal = canvas->palette();

	if (canvas->lineWidth() == width &&
		pal.color(QPalette::Active, QColorGroup::Foreground) == color)
		return;

	canvas->setLineWidth(width);
	pal.setColor(QColorGroup::Foreground,color);
	canvas->setPalette(pal);
	emit modifiedGraph();
}

void Graph::drawAxesBackbones(bool yes)
{
	if (drawAxesBackbone == yes)
		return;

	drawAxesBackbone = yes;

	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale=(QwtScaleWidget*) axisWidget(i);
		if (scale)
		{
			ScaleDraw *sclDraw = (ScaleDraw *)axisScaleDraw (i);
			sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, yes);
			scale->repaint();
		}
	}

	replot();
	emit modifiedGraph();
}

void Graph::loadAxesOptions(const QStringList& lst)
{
	if (lst.size() < 2)
		return;

	drawAxesBackbone = (lst[1] == "1");

	if (lst.size() == 6){
		for (int i = 0; i<QwtPlot::axisCnt; i++){
			QwtScaleWidget *scale = (QwtScaleWidget*) axisWidget(i);
			if (scale){
				ScaleDraw *sclDraw = (ScaleDraw *)axisScaleDraw (i);
				sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, lst[i + 2].toInt());
				scale->repaint();
			}
		}
	} else if (!drawAxesBackbone){
		for (int i=0; i<QwtPlot::axisCnt; i++){
			QwtScaleWidget *scale=(QwtScaleWidget*) axisWidget(i);
			if (scale){
				ScaleDraw *sclDraw = (ScaleDraw *)axisScaleDraw (i);
				sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, false);
				scale->repaint();
			}
		}
	}
}

void Graph::setAxesLinewidth(int width)
{
	if (axesLinewidth() == width)
		return;

	for (int i=0; i<QwtPlot::axisCnt; i++){
		QwtScaleWidget *scale=(QwtScaleWidget*) axisWidget(i);
		if (scale){
			scale->setPenWidth(width);
			scale->repaint();
		}
	}

	if (d_grid){
		if (d_grid->xZeroLineMarker()){
			QPen pen = d_grid->xZeroLineMarker()->linePen();
			pen.setWidth(width);
			d_grid->xZeroLineMarker()->setLinePen(pen);
		}
		if (d_grid->yZeroLineMarker()){
			QPen pen = d_grid->yZeroLineMarker()->linePen();
			pen.setWidth(width);
			d_grid->yZeroLineMarker()->setLinePen(pen);
		}
	}

	replot();
	emit modifiedGraph();
}

void Graph::loadAxesLinewidth(int width)
{
	setAxesLinewidth(width);
}

QString Graph::saveCanvas()
{
	QString s="";
	int w = canvas()->lineWidth();
	if (w>0)
	{
		s += "CanvasFrame\t" + QString::number(w)+"\t";
		s += canvasFrameColor().name()+"\n";
	}
	s += "CanvasBackground\t" + canvasBackground().name()+"\t";
	s += QString::number(canvasBackground().alpha())+"\n";
	return s;
}

QString Graph::saveFonts()
{
	int i;
	QString s;
	QStringList list,axesList;
	QFont f;
	list<<"TitleFont";
	f=title().font();
	list<<f.family();
	list<<QString::number(f.pointSize());
	list<<QString::number(f.weight());
	list<<QString::number(f.italic());
	list<<QString::number(f.underline());
	list<<QString::number(f.strikeOut());
	s=list.join ("\t")+"\n";

	for (i=0;i<axisCnt;i++)
	{
		f=axisTitle(i).font();
		list[0]="ScaleFont"+QString::number(i);
		list[1]=f.family();
		list[2]=QString::number(f.pointSize());
		list[3]=QString::number(f.weight());
		list[4]=QString::number(f.italic());
		list[5]=QString::number(f.underline());
		list[6]=QString::number(f.strikeOut());
		s+=list.join ("\t")+"\n";
	}

	for (i=0;i<axisCnt;i++)
	{
		f=axisFont(i);
		list[0]="AxisFont"+QString::number(i);
		list[1]=f.family();
		list[2]=QString::number(f.pointSize());
		list[3]=QString::number(f.weight());
		list[4]=QString::number(f.italic());
		list[5]=QString::number(f.underline());
		list[6]=QString::number(f.strikeOut());
		s+=list.join ("\t")+"\n";
	}
	return s;
}

QString Graph::saveAxesFormulas()
{
	QString s;
	for (int i=0; i<4; i++){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(i);
		if (!sd)
			continue;

		if (!sd->formula().isEmpty())
		{
			s += "<AxisFormula pos=\"" + QString::number(i) + "\">\n";
			s += sd->formula();
			s += "\n</AxisFormula>\n";
		}
	}
	return s;
}

QString Graph::saveScale()
{
	QString s;
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		s += "scale\t" + QString::number(i) + "\t";

		const QwtScaleDiv *scDiv = axisScaleDiv(i);

		s += QString::number(QMIN(scDiv->lowerBound(), scDiv->upperBound()), 'g', 15) + "\t";
		s += QString::number(QMAX(scDiv->lowerBound(), scDiv->upperBound()), 'g', 15) + "\t";
		s += QString::number(d_user_step[i], 'g', 15) + "\t";
		s += QString::number(scDiv->ticks(QwtScaleDiv::MajorTick).count()) + "\t";
		s += QString::number(axisMaxMinor(i)) + "\t";

		const ScaleEngine *sc_eng = (ScaleEngine *)axisScaleEngine(i);
		s += QString::number((int)sc_eng->type()) + "\t";
		s += QString::number(sc_eng->testAttribute(QwtScaleEngine::Inverted));

		ScaleEngine *se = (ScaleEngine *)axisScaleEngine(i);
		if (se->hasBreak()){
			s += "\t" + QString::number(se->axisBreakLeft(), 'g', 15);
			s += "\t" + QString::number(se->axisBreakRight(), 'g', 15);
			s += "\t" + QString::number(se->breakPosition());
			s += "\t" + QString::number(se->stepBeforeBreak(), 'g', 15);
			s += "\t" + QString::number(se->stepAfterBreak(), 'g', 15);
			s += "\t" + QString::number(se->minTicksBeforeBreak());
			s += "\t" + QString::number(se->minTicksAfterBreak());
			s += "\t" + QString::number(se->log10ScaleAfterBreak());
			s += "\t" + QString::number(se->breakWidth());
			s += "\t" + QString::number(se->hasBreakDecoration()) + "\n";
		} else
			 s += "\n";
	}
	return s;
}

void Graph::setAxisTitleColor(int axis, const QColor& c)
{
	QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(axis);
	if (scale){
		QwtText title = scale->title();
		title.setColor(c);
 		scale->setTitle(title);
	}
}

QString Graph::saveAxesTitleColors()
{
	QString s="AxesTitleColors\t";
	for (int i=0;i<4;i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
		QColor c;
		if (scale)
			c=scale->title().color();
		else
			c=QColor(Qt::black);

		s+=c.name()+"\t";
	}
	return s+"\n";
}

QString Graph::saveTitle()
{
	QString s="PlotTitle\t";
	s += title().text().replace("\n", "<br>")+"\t";
	s += title().color().name()+"\t";
	s += QString::number(title().renderFlags())+"\n";
	return s;
}

QString Graph::saveScaleTitles()
{
	int a = 0;
	QString s = "";
	for (int i = 0; i < 4; i++){
		switch (i){
			case 0:
				a = 2;
            break;
			case 1:
				a = 0;
            break;
			case 2:
				a = 3;
            break;
			case 3:
				a = 1;
            break;
		}
		QString title = axisTitleString(a);//axisTitle(a).text();
		if (!title.isEmpty())
			s += title.replace("\n", "<br>") + "\t";
        else
            s += "\t";
	}
	return s + "\n";
}

QString Graph::saveAxesTitleAlignement()
{
	QString s = "AxesTitleAlignment\t";

	QStringList axes;
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		axes << QString::number(Qt::AlignHCenter);
		if (axisEnabled(i))
			axes[i] = QString::number(axisTitle(i).renderFlags());
	}
	s += axes.join("\t") + "\n";

	QStringList invertedTitles;
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		axes[i] = "-1";
		invertedTitles << "0";
		if (axisEnabled(i)){
			const QwtScaleWidget *scale = axisWidget(i);
			axes[i] = QString::number(scale->spacing());
			if (scale->testLayoutFlag(QwtScaleWidget::TitleInverted))
				invertedTitles[i] = "1";
		}
	}

	s += "AxesTitleDistance\t" + axes.join("\t") + "\n";
	s += "InvertedTitle\t" + invertedTitles.join("\t") + "\n";
	return s;
}

QString Graph::savePieCurveLayout()
{
	QString s="PieCurve\t";

	PieCurve *pie = (PieCurve*)curve(0);
	s+= pie->title().text()+"\t";
	QPen pen = pie->pen();
	s+=QString::number(pen.widthF())+"\t";
	s+=pen.color().name()+"\t";
	s+=penStyleName(pen.style()) + "\t";
	s+=QString::number(PatternBox::patternIndex(pie->pattern()))+"\t";
	s+=QString::number(pie->radius())+"\t";
	s+=QString::number(pie->firstColor())+"\t";
	s+=QString::number(pie->startRow())+"\t"+QString::number(pie->endRow())+"\t";
	s+=QString::number(pie->isVisible())+"\t";

	//Starting with version 0.9.3-rc3
	s+=QString::number(pie->startAzimuth())+"\t";
	s+=QString::number(pie->viewAngle())+"\t";
	s+=QString::number(pie->thickness())+"\t";
	s+=QString::number(pie->horizontalOffset())+"\t";
	s+=QString::number(pie->labelsEdgeDistance())+"\t";
	s+=QString::number(pie->counterClockwise())+"\t";
	s+=QString::number(pie->labelsAutoFormat())+"\t";
	s+=QString::number(pie->labelsValuesFormat())+"\t";
	s+=QString::number(pie->labelsPercentagesFormat())+"\t";
	s+=QString::number(pie->labelCategories())+"\t";
	s+=QString::number(pie->fixedLabelsPosition())+"\n";
	return s;
}

QString Graph::rgbaName(const QColor& color)
{
	if (color.alpha() == 255)
		return color.name();

	return color.name() + "," + QString::number(color.alphaF());
}

QString Graph::saveCurveLayout(int index)
{
	QString s = QString::null;
	PlotCurve *c = (PlotCurve*)curve(index);
	int style = c->plotStyle();
	if (c){
		s += QString::number(style)+"\t";
		if (style == Spline)
			s+="5\t";
		else if (style == VerticalSteps)
			s+="6\t";
		else
			s+=QString::number(c->style())+"\t";
		s += rgbaName(c->pen().color()) + "\t";
		s += QString::number(c->pen().style()-1)+"\t";
		s += QString::number(c->pen().widthF())+"\t";

		const QwtSymbol symbol = c->symbol();
		s += QString::number(symbol.size().width()) + "\t";
		s += QString::number(SymbolBox::symbolIndex(symbol.style())) + "\t";
		s += rgbaName(symbol.pen().color()) + "\t";
		if (symbol.brush().style() != Qt::NoBrush)
			s += rgbaName(symbol.brush().color()) + "\t";
		else
			s += QString::number(-1) + "\t";

		bool filled = c->brush().style() == Qt::NoBrush ? false : true;
		if (filled)
			s += QString::number(c->brush().color().alphaF()) + "\t";
		else
			s += "0\t";
		s += c->brush().color().name() + "\t";
		s += QString::number(PatternBox::patternIndex(c->brush().style()))+"\t";
		if (style <= LineSymbols || style == Box)
			s += QString::number(symbol.pen().widthF())+"\t";
	}

	if(style == VerticalBars || style == HorizontalBars || style == Histogram){
		QwtBarCurve *b = (QwtBarCurve*)c;
		s+=QString::number(b->gap())+"\t";
		s+=QString::number(b->offset())+"\t";
	}

	if (style == Histogram){
		QwtHistogram *h = (QwtHistogram*)c;
		s+=QString::number(h->autoBinning())+"\t";
		s+=QString::number(h->binSize())+"\t";
		s+=QString::number(h->begin())+"\t";
		s+=QString::number(h->end())+"\t";
	} else if(style == VectXYXY || style == VectXYAM){
		VectorCurve *v = (VectorCurve*)c;
		s+=v->color().name()+"\t";
		s+=QString::number(v->width())+"\t";
		s+=QString::number(v->headLength())+"\t";
		s+=QString::number(v->headAngle())+"\t";
		s+=QString::number(v->filledArrowHead())+"\t";

		QStringList colsList = v->plotAssociation();
		s+=colsList[2].remove("(X)").remove("(A)")+"\t";
		s+=colsList[3].remove("(Y)").remove("(M)");
		if (style == VectXYAM)
			s+="\t"+QString::number(v->position());
		s+="\t";
	} else if(style == Box){
		BoxCurve *b = (BoxCurve*)c;
		s+=QString::number(SymbolBox::symbolIndex(b->maxStyle()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->p99Style()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->meanStyle()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->p1Style()))+"\t";
		s+=QString::number(SymbolBox::symbolIndex(b->minStyle()))+"\t";
		s+=QString::number(b->boxStyle())+"\t";
		s+=QString::number(b->boxWidth())+"\t";
		s+=QString::number(b->boxRangeType())+"\t";
		s+=QString::number(b->boxRange())+"\t";
		s+=QString::number(b->whiskersRangeType())+"\t";
		s+=QString::number(b->whiskersRange())+"\t";
	}
	return s;
}

void Graph::restoreSymbolImage(int index, const QStringList& lst)
{
	if (index < 0 || index >= d_curves.count())
		return;

	PlotCurve *c = this->curve(index);
	if (!c)
		return;

	QString path;
	QStringList::const_iterator line;
	for (line = lst.begin(); line != lst.end(); line++){
		QString s = *line;
		if (s.contains("<path>"))
			path = s.remove("<path>").remove("</path>");
		else if (s.contains("<xpm>")){
			QString xpm;
			while ( s != "</xpm>" ){
				s = *(++line);
				xpm += s + "\n";
			}
			QPixmap pix;
			pix.loadFromData(xpm.toAscii());
			c->setSymbol(ImageSymbol(pix, path));
		}
	}
}

QString Graph::saveCurves()
{
	QString s;
	if (isPiePlot())
		s += savePieCurveLayout();
	else {
		int i = -1;
		foreach (QwtPlotItem *it, d_curves){
			i++;
			if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
				s += ((Spectrogram *)it)->saveToString();
				continue;
			}

			DataCurve *c = (DataCurve *)it;
			if (c->type() != ErrorBars){
				if (c->type() == Function){
					s += ((FunctionCurve *)c)->saveToString();
					continue;
				} else if (c->type() == Box)
					s += "curve\t" + QString::number(c->x(0)) + "\t" + c->title().text() + "\t";
				else if (c->type() == Histogram)
					s += "curve\t-\t" + c->title().text() + "\t";//ugly hack to avoid having an empty string for c->xColumnName()
				else
					s += "curve\t" + c->xColumnName() + "\t" + c->title().text() + "\t";

				s += saveCurveLayout(i);
				s += QString::number(c->xAxis())+"\t"+QString::number(c->yAxis())+"\t";
				s += QString::number(c->startRow())+"\t"+QString::number(c->endRow())+"\t";
				s += QString::number(c->isVisible())+"\n";
				s += c->saveToString();
			} else if (c->type() == ErrorBars){
				ErrorBarsCurve *er = (ErrorBarsCurve *)it;
				s += "ErrorBars\t";
				s += QString::number(er->direction()) + "\t";
				s += er->masterCurve()->xColumnName() + "\t";
				s += er->masterCurve()->title().text() + "\t";
				s += er->title().text() + "\t";
				s += QString::number(er->width()) + "\t";
				s += QString::number(er->capLength()) + "\t";
				s += er->color().name() + "\t";
				s += QString::number(er->throughSymbol()) + "\t";
				s += QString::number(er->plusSide()) + "\t";
				s += QString::number(er->minusSide()) + "\t";
				s += QString::number(curveIndex(er->masterCurve())) + "\n";
				s += er->saveToString();
			}
		}
	}
	return s;
}

LegendWidget* Graph::legend()
{
	foreach (FrameWidget *w, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(w);
		if (l && l->isAutoUpdateEnabled())
			return l;
	}
	return NULL;
}

void Graph::setLegend(const QString& s)
{
	LegendWidget *l = legend();
	if (l){
		l->setText(s);
		l->repaint();
	}
}

void Graph::removeLegend()
{
	remove(legend());
}

LegendWidget* Graph::newLegend(const QString& text)
{
	LegendWidget* l = new LegendWidget(this);
	if (!l)
		return NULL;

	QString s = text;
	if (s.isEmpty()){
		if (isPiePlot())
			s = pieLegendText();
		else
			s = legendText();
	}
	l->setText(s);
	ApplicationWindow *app = multiLayer()->applicationWindow();
	if (app){
		l->setFrameStyle(app->legendFrameStyle);
		l->setFont(app->plotLegendFont);
		l->setTextColor(app->legendTextColor);
		l->setBackgroundColor(app->legendBackground);
	}

	l->setAutoUpdate(true);
	d_enrichments << l;
	emit modifiedGraph();
	return l;
}

LegendWidget* Graph::addTimeStamp()
{
	LegendWidget* l = newLegend(QDateTime::currentDateTime().toString(Qt::LocalDate));
	l->setAutoUpdate(false);

	QPoint p = canvas()->pos() + QPoint(canvas()->width()/2, 10);
	l->move(mapToParent(p));
	emit modifiedGraph();
	return l;
}

void Graph::insertLegend(const QStringList& lst, int fileVersion)
{
	LegendWidget *l = insertText(lst, fileVersion);
	l->setAutoUpdate(true);
}

LegendWidget* Graph::insertText(const QStringList& list, int fileVersion)
{
	QStringList fList = list;
	bool pieLabel = (list[0] == "<PieLabel>") ? true : false;
	LegendWidget* l = NULL;
	if (pieLabel)
		l = new PieLabel(this);
	else {
		l = new LegendWidget(this);
		d_enrichments << l;
		d_active_enrichment = l;
	}

	if (fileVersion < 86 || (fileVersion > 91 && fileVersion < 96))
		l->move(QPoint(fList[1].toInt(),fList[2].toInt()));
	else {
	    updateLayout();
		l->setOriginCoord(fList[1].toDouble(), fList[2].toDouble());
	}

	QFont fnt=QFont (fList[3],fList[4].toInt(),fList[5].toInt(),fList[6].toInt());
	fnt.setUnderline(fList[7].toInt());
	fnt.setStrikeOut(fList[8].toInt());
	l->setFont(fnt);

	l->setAngle(fList[11].toInt());

    QString text = QString();
	if (fileVersion < 71){
		int bkg=fList[10].toInt();
		if (bkg <= 2)
			l->setFrameStyle(bkg);
		else if (bkg == 3){
			l->setFrameStyle(0);
			l->setBackgroundColor(QColor(255, 255, 255));
		}
		else if (bkg == 4){
			l->setFrameStyle(0);
			l->setBackgroundColor(QColor(Qt::black));
		}

		int n =(int)fList.count();
		text += fList[12];
		for (int i=1; i<n-12; i++)
			text += "\n" + fList[12+i];
	} else if (fileVersion < 90) {
		l->setTextColor(QColor(fList[9]));
		l->setFrameStyle(fList[10].toInt());
		l->setBackgroundColor(QColor(fList[12]));

		int n=(int)fList.count();
		text += fList[13];
		for (int i=1; i<n-13; i++)
			text += "\n" + fList[13+i];
	} else {
		l->setTextColor(QColor(fList[9]));
		l->setFrameStyle(fList[10].toInt());
		QColor c = QColor(fList[12]);
		c.setAlpha(fList[13].toInt());
		l->setBackgroundColor(c);

		int n = (int)fList.count();
		if (n > 14)
			text += fList[14];

		for (int i=1; i<n-14; i++){
			int j = 14+i;
			if (n > j)
				text += "\n" + fList[j];
		}
	}

	if (fileVersion < 91)
		text = text.replace("\\c{", "\\l(").replace("}", ")");

    l->setText(text);
	if (pieLabel){
		PieCurve *pie = (PieCurve *)curve(0);
		if(pie)
			pie->addLabel((PieLabel *)l);
	}
    return l;
}

void Graph::addArrow(QStringList list, int fileVersion)
{
	ArrowMarker* mrk = new ArrowMarker();
	if(!mrk)
		return;

	insertMarker(mrk);
    d_lines.append(mrk);

	if (fileVersion < 86){
		mrk->setStartPoint(QPoint(list[1].toInt(), list[2].toInt()));
		mrk->setEndPoint(QPoint(list[3].toInt(), list[4].toInt()));
	} else
		mrk->setBoundingRect(list[1].toDouble(), list[2].toDouble(),
							list[3].toDouble(), list[4].toDouble());

	mrk->setWidth(list[5].toDouble());
	mrk->setColor(QColor(list[6]));
	mrk->setStyle(getPenStyle(list[7]));
	mrk->drawEndArrow(list[8]=="1");
	mrk->drawStartArrow(list[9]=="1");
	if (list.count()>10){
		mrk->setHeadLength(list[10].toInt());
		mrk->setHeadAngle(list[11].toInt());
		mrk->fillArrowHead(list[12]=="1");
	}

	if (list.count()>13)//introduced in file version 0.9.7.10
		mrk->setAttachPolicy((ArrowMarker::AttachPolicy)list[13].toInt());
}

ArrowMarker* Graph::addArrow(ArrowMarker* mrk)
{
	ArrowMarker* aux = new ArrowMarker();
	if (aux){
		d_lines.append(aux);
		insertMarker(aux);

		aux->setBoundingRect(mrk->startPointCoord().x(), mrk->startPointCoord().y(),
						 mrk->endPointCoord().x(), mrk->endPointCoord().y());
		aux->setWidth(mrk->width());
		aux->setColor(mrk->color());
		aux->setStyle(mrk->style());
		aux->drawEndArrow(mrk->hasEndArrow());
		aux->drawStartArrow(mrk->hasStartArrow());
		aux->setHeadLength(mrk->headLength());
		aux->setHeadAngle(mrk->headAngle());
		aux->fillArrowHead(mrk->filledArrowHead());
		aux->setAttachPolicy(mrk->attachPolicy());
	}
	return aux;
}

LegendWidget* Graph::addText(LegendWidget* t)
{
	LegendWidget* aux = new LegendWidget(this);
	aux->clone(t);
	d_active_enrichment = aux;
	d_enrichments << aux;
	t->raise();
	return aux;
}

QString Graph::saveMarkers()
{
	QString s;
	foreach (QwtPlotMarker *i, d_lines){
		ArrowMarker* mrkL = (ArrowMarker*)i;
		s+="<line>\t";

		QwtDoublePoint sp = mrkL->startPointCoord();
		s+=(QString::number(sp.x(), 'g', 15))+"\t";
		s+=(QString::number(sp.y(), 'g', 15))+"\t";

		QwtDoublePoint ep = mrkL->endPointCoord();
		s+=(QString::number(ep.x(), 'g', 15))+"\t";
		s+=(QString::number(ep.y(), 'g', 15))+"\t";

		s+=QString::number(mrkL->width())+"\t";
		s+=mrkL->color().name()+"\t";
		s+=penStyleName(mrkL->style())+"\t";
		s+=QString::number(mrkL->hasEndArrow())+"\t";
		s+=QString::number(mrkL->hasStartArrow())+"\t";
		s+=QString::number(mrkL->headLength())+"\t";
		s+=QString::number(mrkL->headAngle())+"\t";
		s+=QString::number(mrkL->filledArrowHead())+"\t";
		s+=QString::number(mrkL->attachPolicy())+"</line>\n";
	}

	foreach(FrameWidget *f, d_enrichments)
		s += f->saveToString();

	return s;
}

double Graph::selectedXStartValue()
{
	if (d_range_selector && d_range_selector->isVisible())
		return d_range_selector->minXValue();
	else
		return 0;
}

double Graph::selectedXEndValue()
{
	if (d_range_selector && d_range_selector->isVisible())
		return d_range_selector->maxXValue();
	else
		return 0;
}

QwtPlotItem* Graph::plotItem(int index)
{
	int curves = d_curves.size();
    if (!curves || index >= curves || index < 0)
		return 0;

	return d_curves.at(index);
}

PlotCurve *Graph::curve(int index)
{
	int curves = d_curves.size();
	if (!curves || index >= curves || index < 0)
		return 0;

	QwtPlotItem *it = d_curves.at(index);
	if (it && it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
		return (PlotCurve*)it;

	return 0;
}

DataCurve * Graph::dataCurve(int index)
{
	PlotCurve *c = curve(index);
	if (c && c->type() != Function)
		return (DataCurve*)c;

	return 0;
}

FunctionCurve * Graph::functionCurve(int index)
{
	PlotCurve *c = curve(index);
	if (c && c->type() == Function)
		return (FunctionCurve*)c;

	return 0;
}

BoxCurve * Graph::boxCurve(int index)
{
	DataCurve *c = dataCurve(index);
	if (c && c->type() == Box)
		return (BoxCurve*)c;

	return 0;
}

//! get curve title string by inde (convenience function for scripts)
QString Graph::curveTitle(int index)
{
	QwtPlotItem *item = plotItem(index);
	if (item)
		return item->title().text();

	return QString::null;
}

int Graph::range(const QString& curveTitle, double *start, double *end)
{
	QwtPlotCurve *c = curve(curveTitle);
	if (!c)
		return 0;

	return range(c, start, end);
}

int Graph::range(QwtPlotCurve *c, double *start, double *end)
{
	if (!c)
		return 0;

	if (c->curveType() == QwtPlotCurve::Yfx){
		if (d_range_selector && d_range_selector->isVisible() &&
			d_range_selector->selectedCurve() == c) {
			*start = d_range_selector->minXValue();
			*end = d_range_selector->maxXValue();
			return d_range_selector->dataSize();
		} else {
			const QwtData *data = &(c->data());
			*start = data->boundingRect().left();
			*end = data->boundingRect().right();
			return c->dataSize();
		}
	} else {
		if (d_range_selector && d_range_selector->isVisible() &&
			d_range_selector->selectedCurve() == c) {
			*start = d_range_selector->minYValue();
			*end = d_range_selector->maxYValue();
			return d_range_selector->dataSize();
		} else {
			const QwtData *data = &(c->data());
			*start = data->boundingRect().bottom();
			*end = data->boundingRect().top();
			return c->dataSize();
		}
	}
	return 0;
}

CurveLayout Graph::initCurveLayout()
{
	CurveLayout cl;
	cl.connectType = 1;
	cl.lStyle = 0;
	cl.lWidth = 1;
	cl.sSize = 3;
	cl.sType = 0;
	cl.filledArea = 0;
	cl.aCol = Qt::black;
	cl.aStyle = 0;
	cl.lCol = Qt::black;
	cl.penWidth = 1;
	cl.symCol = Qt::black;
	cl.fillCol = Qt::black;
	return cl;
}

CurveLayout Graph::initCurveLayout(int style, int curves, bool guessLayout)
{
	int i = d_curves.size() - 1;

	CurveLayout cl = initCurveLayout();
	int color = 0, sIndex = 0;
	if (guessLayout)
		guessUniqueCurveLayout(color, sIndex);

	QList<QColor> indexedColors = ColorBox::defaultColors();
	MultiLayer *ml = multiLayer();
	ApplicationWindow *app = 0;
	if (ml){
		app = ml->applicationWindow();
		if (app){
			indexedColors = app->indexedColors();
			if (app->d_indexed_symbols){
				QList<int> indexedSymbols = app->indexedSymbols();
				if (sIndex >= 0 && sIndex < indexedSymbols.size())
					cl.sType = indexedSymbols[sIndex] + 1;
			} else
				cl.sType = app->d_symbol_style;

			if (style == Area || style == VerticalBars || style == HorizontalBars || style == StackBar || style == StackColumn || style == Histogram){
				cl.aStyle = app->defaultCurveBrush;
				cl.filledArea = 0.01*app->defaultCurveAlpha;
			}

			cl.lStyle = app->d_curve_line_style;
		}
	}
	int colorsCount = indexedColors.size();

	if (color >= 0 && color < colorsCount)
		cl.lCol = indexedColors[color];
  	cl.symCol = cl.lCol;
	cl.fillCol = (app && app->d_fill_symbols) ? cl.lCol : QColor();
	if (app)
		cl.penWidth = app->defaultSymbolEdge;

	if (style == Line)
		cl.sType = 0;
	else if (style == Scatter)
		cl.connectType = 0;
	else if (style == VerticalDropLines)
		cl.connectType = 2;
	else if (style == HorizontalSteps || style == VerticalSteps){
		cl.connectType = 3;
		cl.sType = 0;
	} else if (style == Spline)
		cl.connectType = 5;
	else if (curves && (style == VerticalBars || style == HorizontalBars)){
		cl.lCol = Qt::black;
		if (i >= 0 && i < colorsCount)
			cl.aCol = indexedColors[i];
		cl.sType = 0;
		QwtBarCurve *b = (QwtBarCurve*)curve(i);
		if (b && (b->type() == VerticalBars || b->type() == HorizontalBars)){
			b->setGap(qRound(100*(1-1.0/(double)curves)));
			b->setOffset(-50*(curves-1) + i*100);
		}
	} else if (style == StackBar || style == StackColumn){
		cl.lCol = Qt::black;
		if (i >= 0 && i < colorsCount)
			cl.aCol = indexedColors[i];
		cl.sType = 0;
	} else if (style == Histogram){
		if (i >= 0 && i < colorsCount)
			cl.lCol = indexedColors[i];
		cl.aCol = cl.lCol;
		cl.sType = 0;
		cl.aStyle = 4;
	} else if (style == Area){
		cl.aCol = cl.lCol;
		cl.sType = 0;
		cl.connectType = 1;
	}
	return cl;
}

void Graph::updateCurveLayout(PlotCurve* c, const CurveLayout *cL)
{
	if (!c)
		return;

	int index = curveIndex(c);
	if (d_curves.size() < index)
		return;

	QPen pen = QPen(cL->symCol, cL->penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	pen.setCosmetic(true);
	if (cL->fillCol.isValid())
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(cL->fillCol), pen, QSize(cL->sSize, cL->sSize)));
	else
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(), pen, QSize(cL->sSize, cL->sSize)));

	pen = QPen(cL->lCol, cL->lWidth, getPenStyle(cL->lStyle), Qt::SquareCap, Qt::MiterJoin);
	pen.setCosmetic(true);
	c->setPen(pen);

	switch (c->plotStyle()){
		case Scatter:
			c->setStyle(QwtPlotCurve::NoCurve);
		break;
		case Spline:
			c->setStyle(QwtPlotCurve::Lines);
			c->setCurveAttribute(QwtPlotCurve::Fitted);
		break;
		case VerticalSteps:
			c->setStyle(QwtPlotCurve::Steps);
			c->setCurveAttribute(QwtPlotCurve::Inverted);
		break;
		default:
			c->setStyle((QwtPlotCurve::CurveStyle)cL->connectType);
		break;
	}

	QBrush brush = QBrush(cL->aCol);
	if (cL->filledArea){
		brush.setStyle(PatternBox::brushStyle(cL->aStyle));
		QColor color = brush.color();
		color.setAlphaF(cL->filledArea);
		brush.setColor(color);
	}else
		brush.setStyle(Qt::NoBrush);

	c->setBrush(brush);
}

void Graph::updateErrorBars(ErrorBarsCurve *er, bool xErr, double width, int cap, const QColor& c,
		bool plus, bool minus, bool through)
{
	if (!er)
		return;

	if (er->width() == width && er->capLength() == cap &&
		er->color() == c && er->plusSide() == plus &&
		er->minusSide() == minus && er->throughSymbol() == through &&
		er->xErrors() == xErr)
		return;

	er->setWidth(width);
	er->setCapLength(cap);
	er->setColor(c);
	er->setXErrors(xErr);
	er->drawThroughSymbol(through);
	er->drawPlusSide(plus);
	er->drawMinusSide(minus);

	replot();
	emit modifiedGraph();
}

ErrorBarsCurve* Graph::addErrorBars(const QString& yColName, Table *errTable, const QString& errColName,
		int type, double width, int cap, const QColor& color, bool through, bool minus, bool plus)
{
	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;
		if (((PlotCurve *)it)->type() == ErrorBars || ((PlotCurve *)it)->type() == Function)
			continue;

		if (it->title().text() == yColName)
			return addErrorBars((DataCurve*)it, errTable, errColName, type, width, cap, color, through, minus, plus);
	}
	return NULL;
}

ErrorBarsCurve* Graph::addErrorBars(const QString& xColName, const QString& yColName,
		Table *errTable, const QString& errColName, int type, double width, int cap,
		const QColor& color, bool through, bool minus, bool plus)
{
	return addErrorBars(masterCurve(xColName, yColName), errTable, errColName, type, width, cap, color, through, minus, plus);
}

ErrorBarsCurve* Graph::addErrorBars(DataCurve *c, Table *errTable, const QString& errColName,
				int type, double width, int cap, const QColor& color, bool through, bool minus, bool plus)
{
	ErrorBarsCurve *er = new ErrorBarsCurve(type, errTable, errColName);
	insertCurve(er);

	er->setMasterCurve(c);
	er->setCapLength(cap);
	er->setColor(color);
	er->setWidth(width);
	er->drawPlusSide(plus);
	er->drawMinusSide(minus);
	er->drawThroughSymbol(through);

	if (c)
		updatePlot();
	return er;
}

void Graph::loadErrorBars(QList<ErrorBarsCurve *> errBars, QList<int> mcIndexes)
{
	for (int i = 0; i < errBars.size() && i < mcIndexes.size(); i++){
		ErrorBarsCurve *er = errBars[i];
		if (er)
			er->setMasterCurve(dataCurve(mcIndexes[i]));
	}
}

bool Graph::isPiePlot()
{
	if (d_curves.size() != 1)
		return false;

	QwtPlotItem *it = d_curves.first();
	if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
		return false;

	if (((PlotCurve *)it)->type() == Pie)
		return true;

	return false;
}

PieCurve* Graph::plotPie(Table* w, const QString& name, const QPen& pen, int brush,
					int size, int firstColor, int startRow, int endRow, bool visible,
					double d_start_azimuth, double d_view_angle, double d_thickness,
					double d_horizontal_offset, double d_edge_dist, bool d_counter_clockwise,
					bool d_auto_labeling, bool d_values, bool d_percentages,
					bool d_categories, bool d_fixed_labels_pos)
{
	if (endRow < 0)
		endRow = w->numRows() - 1;

	PieCurve *pie = new PieCurve(w, name, startRow, endRow);
	insertCurve(pie);

	pie->loadData();
	pie->initLabels();

	pie->setPen(pen);
	pie->setRadius(size);
	pie->setFirstColor(firstColor);
	pie->setBrushStyle(PatternBox::brushStyle(brush));
	pie->setVisible(visible);

	pie->setStartAzimuth(d_start_azimuth);
	pie->setViewAngle(d_view_angle);
	pie->setThickness(d_thickness);
	pie->setHorizontalOffset(d_horizontal_offset);
	pie->setLabelsEdgeDistance(d_edge_dist);
	pie->setCounterClockwise(d_counter_clockwise);
	pie->setLabelsAutoFormat(d_auto_labeling);
	pie->setLabelValuesFormat(d_values);
	pie->setLabelPercentagesFormat(d_percentages);
	pie->setLabelCategories(d_categories);
	pie->setFixedLabelsPosition(d_fixed_labels_pos);
	return pie;
}

PieCurve* Graph::plotPie(Table* w, const QString& name, int startRow, int endRow)
{
	for (int i=0; i<QwtPlot::axisCnt; i++)
		enableAxis(i, false);
	scalePicker->refresh();

	setTitle(QString::null);

	QwtPlotCanvas* canvas = (QwtPlotCanvas*)this->canvas();
	canvas->setLineWidth(0);
	setFrame(1, Qt::black);

	PieCurve *pie = new PieCurve(w, name, startRow, endRow);
    insertCurve(pie);
	pie->loadData();
	pie->initLabels();

	replot();
	return pie;
}

void Graph::insertPlotItem(QwtPlotItem *i, int type)
{
	insertCurve(i);

	if (i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
		((PlotCurve *)i)->setPlotStyle(type);
  		addLegendItem();
	}
}

bool Graph::addCurves(Table* w, const QStringList& names, int style, double lWidth, int sSize, int startRow, int endRow)
{
	if (!w)
		return false;

	if (style == Pie)
		plotPie(w, names[0], startRow, endRow);
	else if (style == Box)
		plotBox(w, names, startRow, endRow);
	else if (style == VectXYXY || style == VectXYAM)
		plotVectors(w, names, style, startRow, endRow);
	else if (style == Histogram){
		int curves = names.count();
		for (int i=0; i<curves; i++){
			QwtHistogram *h = new QwtHistogram(w, names[i], startRow, endRow);
			if (h){
				insertCurve(h);
				h->loadData();
				CurveLayout cl = initCurveLayout(style, curves);
				cl.lWidth = lWidth;
				updateCurveLayout(h, &cl);
				addLegendItem();
			}
		}
	} else {
		int curves = names.count();
		int errCurves = 0;
		QStringList lst = QStringList(), masterCurvesLst = QStringList();
		for (int i=0; i<curves; i++){//We rearrange the list so that the error bars are placed at the end
			QString colName = names[i];
			int j = w->colIndex(colName);
			if (w->colPlotDesignation(j) == Table::xErr || w->colPlotDesignation(j) == Table::yErr ||
				w->colPlotDesignation(j) == Table::Label){
				errCurves++;
				lst << colName;
			} else {
				lst.prepend(colName);
				masterCurvesLst << colName;
			}
		}

		for (int i = 0; i < curves; i++){
			int j = w->colIndex(lst[i]);
			PlotCurve *c = NULL;
			if (w->colPlotDesignation(j) == Table::xErr || w->colPlotDesignation(j) == Table::yErr){
				int xcol = w->colX(j);
				int ycol = w->colY(j, xcol, masterCurvesLst);
				if (xcol < 0 || ycol < 0)
					return false;

				ErrorBarsCurve *er = NULL;
				if (w->colPlotDesignation(j) == Table::xErr)
					er = addErrorBars(w->colName(xcol), w->colName(ycol), w, lst[i], (int)ErrorBarsCurve::Horizontal);
				else
					er = addErrorBars(w->colName(xcol), w->colName(ycol), w, lst[i]);

				if (!er)
					continue;

				DataCurve *mc = er->masterCurve();
				if (mc)
					er->setColor(mc->pen().color());
			} else if (w->colPlotDesignation(j) == Table::Label){
				QString labelsCol = names[i];
				int xcol = w->colX(w->colIndex(labelsCol));
				int ycol = w->colY(w->colIndex(labelsCol), xcol);
				if (xcol < 0 || ycol < 0)
					return false;

				DataCurve* mc = masterCurve(w->colName(xcol), w->colName(ycol));
				if (mc){
					replot();
					mc->setLabelsColumnName(labelsCol);
				} else
					return false;
			} else
				c = (PlotCurve *)insertCurve(w, lst[i], style, startRow, endRow);

			if (c && c->type() != ErrorBars){
				CurveLayout cl = initCurveLayout(style, curves - errCurves);
				cl.sSize = sSize;
				cl.lWidth = lWidth;
				updateCurveLayout(c, &cl);
			}
		}
	}

	replot();

	updateSecondaryAxis(QwtPlot::xTop, true);
	updateSecondaryAxis(QwtPlot::yRight, true);
	updateAxesTitles();

	replot();

	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();

	if (!d_auto_scale){
		for (int i = 0; i < QwtPlot::axisCnt; i++)
			 setAxisScaleDiv(i, *axisScaleDiv(i));
	}

	return true;
}

DataCurve* Graph::insertCurve(Table* w, const QString& name, int style, int startRow, int endRow)
{//provided for convenience
	int ycol = w->colIndex(name);
	int xcol = w->colX(ycol);

	DataCurve* c = insertCurve(w, w->colName(xcol), w->colName(ycol), style, startRow, endRow);
	if (c)
		emit modifiedGraph();
	return c;
}

DataCurve* Graph::insertCurve(Table* w, int xcol, const QString& name, int style)
{
	return insertCurve(w, w->colName(xcol), w->colName(w->colIndex(name)), style);
}

DataCurve* Graph::insertCurve(Table* w, const QString& xColName, const QString& yColName, int style, int startRow, int endRow)
{
	if (style == Histogram){
		DataCurve *c = new QwtHistogram(w, yColName, startRow, endRow);
		insertCurve(c);
		return c;
	}

	int xcol = w->colIndex(xColName);
	int ycol = w->colIndex(yColName);
	if (xcol < 0 || ycol < 0)
		return NULL;

	if (endRow < 0)
		endRow = w->numRows() - 1;

	int xAxis = QwtPlot::xBottom;
	if (style == HorizontalBars)
		xAxis = QwtPlot::yLeft;

	int size = 0;
	for (int i = startRow; i <= endRow; i++ ){
		if (!w->text(i, xcol).isEmpty() && !w->text(i, ycol).isEmpty()){
			size++;
			break;
		}
	}
	if (!size)
		return NULL;

	DataCurve *c = 0;
	if (style == VerticalBars || style == StackColumn){
		c = new QwtBarCurve(QwtBarCurve::Vertical, w, xColName, yColName, startRow, endRow);
		if (style == StackColumn){
			style = VerticalBars;
			((QwtBarCurve*)c)->setStacked();
		}
	} else if (style == HorizontalBars || style == StackBar){
		c = new QwtBarCurve(QwtBarCurve::Horizontal, w, xColName, yColName, startRow, endRow);
		if (style == StackBar){
			style = HorizontalBars;
			((QwtBarCurve*)c)->setStacked();
		}
	} else
		c = new DataCurve(w, xColName, yColName, startRow, endRow);

	insertCurve(c);
	c->setPlotStyle(style);

	CurveLayout cl = initCurveLayout(style, 0, false);
	updateCurveLayout(c, &cl);

	c->loadData();
	c->enableSpeedMode();

	int xColType = w->columnType(xcol);
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(xAxis);
	if (xColType == Table::Time && sd && sd->scaleType() != ScaleDraw::Time)
		setLabelsDateTimeFormat(xAxis, ScaleDraw::Time, w->columnFormat(xcol));
	else if (xColType == Table::Date && sd && sd->scaleType() != ScaleDraw::Date)
		setLabelsDateTimeFormat(xAxis, ScaleDraw::Date, w->columnFormat(xcol));

	addLegendItem();
	return c;
}

DataCurve* Graph::insertCurve(Table* xt, const QString& xColName, Table* yt, const QString& yColName, int style, int startRow, int endRow)
{
	if (!xt || !yt)
		return NULL;

	int xcol = xt->colIndex(xColName);
	int ycol = yt->colIndex(yColName);
	if (xcol < 0 || ycol < 0)
		return NULL;

	if (endRow < 0)
		endRow = yt->numRows() - 1;

	int xAxis = QwtPlot::xBottom;
	if (style == HorizontalBars)
		xAxis = QwtPlot::yLeft;

	int size = 0;
	for (int i = startRow; i <= endRow; i++ ){
		if (!xt->text(i, xcol).isEmpty() && !yt->text(i, ycol).isEmpty()){
			size++;
			break;
		}
	}
	if (!size)
		return NULL;

	DataCurve *c = new DataCurve(xt, xColName, yt, yColName, startRow, endRow);
	insertCurve(c);
	c->setPlotStyle(style);

	CurveLayout cl = initCurveLayout(style, 0, false);
	updateCurveLayout(c, &cl);

	c->loadData();
	c->enableSpeedMode();

	int xColType = xt->columnType(xcol);
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(xAxis);
	if (xColType == Table::Time && sd && sd->scaleType() != ScaleDraw::Time)
		setLabelsDateTimeFormat(xAxis, ScaleDraw::Time, xt->columnFormat(xcol));
	else if (xColType == Table::Date && sd && sd->scaleType() != ScaleDraw::Date)
		setLabelsDateTimeFormat(xAxis, ScaleDraw::Date, xt->columnFormat(xcol));

	addLegendItem();
	return c;
}

QwtHistogram* Graph::addHistogram(Matrix *m)
{
	if (!m)
		return NULL;

	QwtHistogram *c = new QwtHistogram(m);
    c->setStyle(QwtPlotCurve::UserCurve);
	c->setPen(QPen(Qt::black, 1.0));
	c->setBrush(QBrush(Qt::black));
	c->loadData();
	insertCurve(c);

	addLegendItem();
	updatePlot();
	return c;
}

QwtHistogram* Graph::restoreHistogram(Matrix *m, const QStringList& l)
{
	if (!m)
		return NULL;

    QwtHistogram *h = new QwtHistogram(m);
    h->setBinning(l[17].toInt(), l[18].toDouble(), l[19].toDouble(), l[20].toDouble());
    h->setGap(l[15].toInt());
	h->setOffset(l[16].toInt());
    h->loadData();
    h->setAxis(l[l.count()-5].toInt(), l[l.count()-4].toInt());
    h->setVisible(l.last().toInt());

	insertCurve(h);
	return h;
}

VectorCurve* Graph::plotVectors(Table* w, const QStringList& colList, int style, int startRow, int endRow)
{
	if (colList.count() != 4)
		return NULL;

	if (endRow < 0)
		endRow = w->numRows() - 1;

	VectorCurve *v = 0;
	if (style == VectXYAM)
		v = new VectorCurve(VectorCurve::XYAM, w, colList[0], colList[1], colList[2], colList[3], startRow, endRow);
	else
		v = new VectorCurve(VectorCurve::XYXY, w, colList[0], colList[1], colList[2], colList[3], startRow, endRow);

	if (!v)
		return NULL;

	insertCurve(v);
	v->setPlotStyle(style);
	v->loadData();
	v->setStyle(QwtPlotCurve::NoCurve);

	addLegendItem();
	updatePlot();
	return v;
}

void Graph::updateVectorsLayout(int curve, const QColor& color, double width,
		int arrowLength, int arrowAngle, bool filled, int position,
		const QString& xEndColName, const QString& yEndColName)
{
	VectorCurve *vect = (VectorCurve *)this->curve(curve);
	if (!vect)
		return;

	vect->setColor(color);
	vect->setWidth(width);
	vect->setHeadLength(arrowLength);
	vect->setHeadAngle(arrowAngle);
	vect->fillArrowHead(filled);
	vect->setPosition(position);

	if (!xEndColName.isEmpty() && !yEndColName.isEmpty())
		vect->setVectorEnd(xEndColName, yEndColName);

	replot();
	emit modifiedGraph();
}

void Graph::setAutoScale()
{
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		setAxisAutoScale(i);
		d_user_step[i] = 0.0;
	}

	replot();

	updateScale();
	emit modifiedGraph();
}

void Graph::updateAxesTitles()
{
	for (int i = 0; i < QwtPlot::axisCnt; i++)
		((QwtPlot *)this)->setAxisTitle(i, parseAxisTitle(i));
}

void Graph::updateAxisTitle(int axis)
{
	if (axis < 0 || axis >= QwtPlot::axisCnt)
		return;

	((QwtPlot *)this)->setAxisTitle(axis, parseAxisTitle(axis));
}

void Graph::updatePlot()
{
	if (d_auto_scale && !zoomOn() && d_active_tool == NULL){
		for (int i = 0; i < QwtPlot::axisCnt; i++){
			setAxisAutoScale(i);
			d_user_step[i] = 0.0;
		}
	}
	replot();
	updateScale();
}

void Graph::updateScale()
{
	if (!d_auto_scale){
	//We need this hack due to the fact that in Qwt 5.0 we can't
	//disable autoscaling in an easier way, like for example: setAxisAutoScale(axisId, false)
		for (int i = 0; i < QwtPlot::axisCnt; i++)
			setAxisScaleDiv(i, *axisScaleDiv(i));
	}

	replot();

	updateMarkersBoundingRect();

	if (d_synchronize_scales){
		updateSecondaryAxis(QwtPlot::xTop);
		updateSecondaryAxis(QwtPlot::yRight);
	}

	replot();//TODO: avoid 2nd replot!
	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();
}

void Graph::setBarsGap(int curve, int gapPercent, int offset)
{
	QwtBarCurve *bars = (QwtBarCurve *)this->curve(curve);
	if (!bars)
		return;

    if (bars->gap() == gapPercent && bars->offset() == offset)
        return;

	bars->setGap(gapPercent);
	bars->setOffset(offset);
}

void Graph::removePie()
{
	QList <PieLabel *> labels = ((PieCurve *)curve(0))->labelsList();
	foreach(PieLabel *l, labels)
		l->setPieCurve(0);

	removeCurve(0);
	replot();

	emit modifiedGraph();
}

void Graph::removeCurves(const QString& s)
{
	foreach(QwtPlotItem *it, d_curves){
        if (it->title().text() == s){
            removeCurve(d_curves.indexOf(it));
            continue;
        }

        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

		if(((DataCurve *)it)->plotAssociation().join(",").contains(s))
            removeCurve(d_curves.indexOf(it));
	}
	replot();
}

void Graph::removeCurve(const QString& s)
{
	removeCurve(curveIndex(s));
}

void Graph::removeCurve(int index)
{
	if (index < 0 || index >= d_curves.size())
		return;

	removeCurve(d_curves.at(index));
}

void Graph::removeCurve(QwtPlotItem *it)
{
	if (!it)
		return;

	int index = d_curves.indexOf(it);
	removeLegendItem(index);

	if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
		int curveType = ((PlotCurve *)it)->type();
		if (curveType == ErrorBars)
			((ErrorBarsCurve *)it)->detachFromMasterCurve();
		else if (curveType != Function){
			((DataCurve *)it)->clearErrorBars();
			((DataCurve *)it)->clearLabels();
		}
		if (d_fit_curves.contains((QwtPlotCurve *)it)){
			int i = d_fit_curves.indexOf((QwtPlotCurve *)it);
			if (i >= 0 && i < d_fit_curves.size())
				d_fit_curves.removeAt(i);
		}
	} else {
		((Spectrogram *)it)->clearLabels();
		QwtScaleWidget *colorAxis = axisWidget(((Spectrogram *)it)->colorScaleAxis());
		if (colorAxis)
			colorAxis->setColorBarEnabled(false);
	}

	if (d_range_selector && d_range_selector->isVisible() &&
		curve(index) == d_range_selector->selectedCurve()){
		int curves = d_curves.size();
		if (curves > 1 && (index - 1) >= 0)
			d_range_selector->setSelectedCurve(curve(index - 1));
		else if (curves > 1 && index + 1 < curves)
			d_range_selector->setSelectedCurve(curve(index + 1));
		else
			disableTools();
	}

	it->detach();
	d_curves.removeAll(it);
	emit modifiedGraph();
}

void Graph::removeLegendItem(int index)
{
	if (index < 0 || index >= d_curves.size())
		return;

	QwtPlotItem *it = d_curves.at(index);
	if (!it || it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
		return;

	if (((PlotCurve *)it)->type() == ErrorBars)
		return;

	foreach(FrameWidget *fw, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l && l->isAutoUpdateEnabled()){
			QString text = l->text();
			QStringList items = text.split( "\n", QString::SkipEmptyParts);

			if (index >= (int) items.count())
				continue;

			QStringList lst = items.grep( "\\l(" + QString::number(index + 1) + ")" );
			if (lst.isEmpty())
				continue;

			items.remove(lst[0]);//remove the corresponding legend string

			for (int i=0; i<items.count(); i++){//set new curves indexes in legend text
				QString item = items[i];
				int pos1 = item.indexOf("\\l(");
				int pos2 = item.indexOf(")", pos1);
				int pos = pos1 + 3;
				int n = pos2 - pos;
				int cv = item.mid(pos, n).toInt();
				if (cv > index){
					int id = cv - 1;
					if (!id)
						id = 1;
					item.replace(pos, n, QString::number(id));
				}
				pos1 = item.indexOf("%(", pos2);
				if (item.contains(","))
					pos2 = item.indexOf(",", pos1);
				else
					pos2 = item.indexOf(")", pos1);
				pos = pos1 + 2;
				n = pos2 - pos;
				cv = item.mid(pos, n).toInt();
				if (cv > index){
					int id = cv - 1;
					if (!id)
						id = 1;
					item.replace(pos, n, QString::number(id));
				}
				items[i] = item;
			}
			text = items.join ( "\n" );
			l->setText(text);
		}
	}
}

void Graph::addLegendItem()
{
	foreach(FrameWidget *fw, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l && l->isAutoUpdateEnabled()){
			QString text = l->text();
			if (text.endsWith("\n") || text.isEmpty())
				text.append(legendText(false, d_curves.size() - 1));
			else
				text.append("\n" + legendText(false, d_curves.size() - 1));
			l->setText(text);
			l->repaint();
		}
	}
}

void Graph::contextMenuEvent(QContextMenuEvent *e)
{
	if (d_selected_arrow){
		emit showMarkerPopupMenu();
		return;
	}

	if (!canvas()->geometry().contains(mapFromGlobal(e->globalPos())))
		return;

	QPoint pos = canvas()->mapFromParent(e->pos());
	int dist, point;
	QwtPlotItem *item = closestCurve(pos.x(), pos.y(), dist, point);
	if (item && dist < 10)//10 pixels tolerance
		emit showCurveContextMenu(item);
	else
		emit showContextMenu();

	e->accept();
}

void Graph::closeEvent(QCloseEvent *e)
{
	emit closedGraph();
	e->accept();
}

bool Graph::zoomOn()
{
	return (d_zoomer[0]->isEnabled() || d_zoomer[1]->isEnabled());
}

void Graph::zoomed (const QwtDoubleRect &)
{
	updateMarkersBoundingRect();
	emit modifiedGraph();

	for (int i = 0; i < QwtPlot::axisCnt; i++)
		axisDivChanged(this, i);
}

void Graph::zoom(bool on)
{
	d_zoomer[0]->setEnabled(on);
	d_zoomer[1]->setEnabled(on);
	foreach(QwtPlotItem *it, d_curves){
  	    if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && ((Spectrogram *)it)->hasColorScale()){
  	     	if (((Spectrogram *)it)->colorScaleAxis() == QwtPlot::xBottom ||
				((Spectrogram *)it)->colorScaleAxis() == QwtPlot::yLeft)
  	        	d_zoomer[0]->setEnabled(false);
  	        else
  	        	d_zoomer[1]->setEnabled(false);
  	    }
  	}

	if (on)
		canvas()->setCursor(QCursor(QPixmap(":/lens.png"), -1, -1));
	else
		canvas()->setCursor(Qt::arrowCursor);
}

void Graph::zoomOut()
{
	d_zoomer[0]->zoom(-1);
	d_zoomer[1]->zoom(-1);

	if (d_synchronize_scales){
		updateSecondaryAxis(QwtPlot::xTop);
		updateSecondaryAxis(QwtPlot::yRight);
	}
}

void Graph::enablePanningMagnifier(bool on, int mode)
{
	if (d_magnifier)
		delete d_magnifier;
	if (d_panner)
		delete d_panner;

	QwtPlotCanvas *cnvs = canvas();
	if (on){
		cnvs->setCursor(Qt::pointingHandCursor);
		d_magnifier = new QwtPlotMagnifier(cnvs);
		d_magnifier->setZoomInKey(Qt::Key_Plus, Qt::ShiftModifier);

		for (int i = 0; i < QwtPlot::axisCnt; i++){
			QwtScaleWidget *scale = axisWidget(i);
			if (scale)
				connect(scale, SIGNAL(scaleDivChanged()), this, SLOT(updateMarkersBoundingRect()));
		}

		d_panner = new QwtPlotPanner(cnvs);
		connect(d_panner, SIGNAL(panned(int, int)), multiLayer(), SLOT(notifyChanges()));

		foreach (QwtPlotItem *it, d_curves){
			if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
				Spectrogram *sp = (Spectrogram *)it;
				int axis = sp->colorScaleAxis();
				d_magnifier->setAxisEnabled (axis, false);
				d_panner->setAxisEnabled (axis, false);
			}
		}

		if (mode == 1){
			cnvs->setCursor(Qt::SizeVerCursor);
			d_magnifier->setAxisEnabled (QwtPlot::xBottom, false);
			d_magnifier->setAxisEnabled (QwtPlot::xTop, false);
			d_panner->setAxisEnabled (QwtPlot::xBottom, false);
			d_panner->setAxisEnabled (QwtPlot::xTop, false);
		} else if (mode == 2){
			cnvs->setCursor(Qt::SizeHorCursor);
			d_magnifier->setAxisEnabled (QwtPlot::yLeft, false);
			d_magnifier->setAxisEnabled (QwtPlot::yRight, false);
			d_panner->setAxisEnabled (QwtPlot::yLeft, false);
			d_panner->setAxisEnabled (QwtPlot::yRight, false);
		}
	} else {
		for (int i = 0; i < QwtPlot::axisCnt; i++){
			QwtScaleWidget *scale = axisWidget(i);
			if (scale)
				disconnect(scale, SIGNAL(scaleDivChanged()), this, SLOT(updateMarkersBoundingRect()));
		}

		cnvs->setCursor(Qt::arrowCursor);
		d_magnifier = NULL;
		d_panner = NULL;
	}
}

ImageWidget* Graph::addImage(ImageWidget* i)
{
	if (!i)
		return 0;

	ImageWidget* i2 = new ImageWidget(this, i->fileName());
	if (i2){
		d_enrichments << i2;
		i2->setCoordinates(i->xValue(), i->yValue(), i->right(), i->bottom());
	}
	return i2;
}

ImageWidget* Graph::addImage(const QString& fileName)
{
	if (fileName.isEmpty() || !QFile::exists(fileName))
		return 0;

	ImageWidget* i = new ImageWidget(this, fileName);
	if (i){
		d_enrichments << i;
		emit modifiedGraph();
	}
	return i;
}

ImageWidget* Graph::addImage(const QImage& image)
{
	ImageWidget* i = new ImageWidget(this, image);
	if (i){
		d_enrichments << i;
		emit modifiedGraph();
	}
	return i;
}

void Graph::insertImageMarker(const QStringList& lst, int fileVersion)
{
	QString fn = lst[1];
	if (!QFile::exists(fn)){
		QMessageBox::warning(0, tr("QtiPlot - File open error"),
				tr("Image file: <p><b> %1 </b><p>does not exist anymore!").arg(fn));
	} else {
		ImageWidget* mrk = new ImageWidget(this, fn);
		if (!mrk)
			return;

        d_enrichments << mrk;

		if (fileVersion < 86){
			mrk->setOrigin(lst[2].toInt(), lst[3].toInt());
			mrk->resize(QSize(lst[4].toInt(), lst[5].toInt()));
		} else if (fileVersion < 90) {
		    double left = lst[2].toDouble();
		    double right = left + lst[4].toDouble();
		    double top = lst[3].toDouble();
		    double bottom = top - lst[5].toDouble();
			mrk->setCoordinates(left, top, right, bottom);
		} else
			mrk->setCoordinates(lst[2].toDouble(), lst[3].toDouble(), lst[4].toDouble(), lst[5].toDouble());
	}
}

void Graph::drawLine(bool on, bool arrow)
{
	drawLineOn = on;
	drawArrowOn = arrow;
	if (!on)
		emit drawLineEnded(true);
}

void Graph::modifyFunctionCurve(int curve, int type, const QStringList &formulas,
		const QString& var, double start, double end, int points, const QMap<QString, double>& constants)
{
	FunctionCurve *c = (FunctionCurve *)this->curve(curve);
	if (!c)
		return;

	QString oldVar = c->variable();
	QStringList oldFormulas = c->formulas();
	const QMap<QString, double> oldConstants = c->constants();
	double oldStartRange = c->startRange();
	double oldEndRange = c->endRange();
	int oldPoints = c->dataSize();
	int oldType = c->functionType();
	if (oldType == type &&
		oldVar == var &&
		oldFormulas == formulas &&
		oldStartRange == start &&
		oldEndRange == end &&
		oldPoints == points &&
		oldConstants == constants)
		return;

	QString oldLegend = c->legend();

	c->setFunctionType((FunctionCurve::FunctionType)type);
	c->setRange(start, end);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->setConstants(constants);
	if (!c->loadData(points)){
		c->setFunctionType((FunctionCurve::FunctionType)oldType);
		c->setRange(oldStartRange, oldEndRange);
		c->setFormulas(oldFormulas);
		c->setVariable(oldVar);
		c->setConstants(oldConstants);
		return;
	}

	foreach(FrameWidget *fw, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l && l->isAutoUpdateEnabled()){//update legends
			QString text = (l->text()).replace(oldLegend, c->legend());
        	l->setText(text);
		}
	}
	updatePlot();
	emit modifiedGraph();
}

QString Graph::generateFunctionName(const QString& name)
{
    int index = 1;
  	QString newName = name + QString::number(index);

  	QStringList lst;
  	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

  		PlotCurve *c = (PlotCurve*)it;
  	    if (c->type() == Function)
  	    	lst << c->title().text();
	}

  	while(lst.contains(newName))
  		newName = name + QString::number(++index);
  	return newName;
}

FunctionCurve* Graph::addFunction(const QStringList &formulas, double start, double end, int points, const QString &var, int type, const QString& title)
{
	QString name;
	if (!title.isEmpty())
		name = title;
	else
		name = generateFunctionName();

	FunctionCurve *c = new FunctionCurve((const FunctionCurve::FunctionType)type, name);
	c->setRange(start, end);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->loadData(points);

	insertCurve(c);

	int colorIndex = 0, symbolIndex;
	guessUniqueCurveLayout(colorIndex, symbolIndex);
	c->setPen(QPen(ColorBox::defaultColor(colorIndex), 1.0));

	addLegendItem();
	updatePlot();

	emit modifiedGraph();
	return c;
}

FunctionCurve* Graph::insertFunctionCurve(const QString& formula, int points, int fileVersion)
{
	int type = 0;
	QStringList formulas;
	QString var, name = QString::null;
	double start = 0.0, end = 0.0;

	QStringList curve = formula.split(",");
	if (fileVersion < 87) {
		if (curve[0][0]=='f') {
			type = FunctionCurve::Normal;
			formulas += curve[0].section('=',1,1);
			var = curve[1];
			start = curve[2].toDouble();
			end = curve[3].toDouble();
		} else if (curve[0][0]=='X') {
			type = FunctionCurve::Parametric;
			formulas += curve[0].section('=',1,1);
			formulas += curve[1].section('=',1,1);
			var = curve[2];
			start = curve[3].toDouble();
			end = curve[4].toDouble();
		} else if (curve[0][0]=='R') {
			type = FunctionCurve::Polar;
			formulas += curve[0].section('=',1,1);
			formulas += curve[1].section('=',1,1);
			var = curve[2];
			start = curve[3].toDouble();
			end = curve[4].toDouble();
		}
	} else {
		type = curve[0].toInt();
		name = curve[1];

		if (type == FunctionCurve::Normal) {
			formulas << curve[2];
			var = curve[3];
			start = curve[4].toDouble();
			end = curve[5].toDouble();
		} else if (type == FunctionCurve::Polar || type == FunctionCurve::Parametric) {
			formulas << curve[2];
			formulas << curve[3];
			var = curve[4];
			start = curve[5].toDouble();
			end = curve[6].toDouble();
		}
	}
	return addFunction(formulas, start, end, points,  var, type, name);
}

void Graph::createTable(const QString& curveName)
{
    if (curveName.isEmpty())
        return;

    const QwtPlotCurve* cv = curve(curveName);
    if (!cv)
        return;

    createTable(cv);
}

void Graph::createTable(const QwtPlotCurve* curve)
{
    if (!curve)
        return;

	MultiLayer *plot = multiLayer();
	if (!plot)
		return;

	ApplicationWindow *app = plot->applicationWindow();
	if (!app)
		return;

	int size = curve->dataSize();
	Table *t = app->newTable(size, 2, QString::null, tr("Data set generated from curve") + ": " + curve->title().text());
	for (int i = 0; i < size; i++){
		t->setCell(i, 0, curve->x(i));
		t->setCell(i, 1, curve->y(i));
	}
	t->showNormal();
}

QString Graph::saveToString(bool saveAsTemplate)
{
	QString s="<graph>\n";
	s+="ggeometry\t";
	s+=QString::number(pos().x())+"\t";
	s+=QString::number(pos().y())+"\t";
	s+=QString::number(geometry().width())+"\t";
	s+=QString::number(geometry().height())+"\n";

	MultiLayer *ml = multiLayer();
	if (ml){
		s += "<PageGeometry>";
		s += QString::number((double)pos().x()/(double)ml->canvas()->width()) + "\t";
		s += QString::number((double)pos().y()/(double)ml->canvas()->height()) + "\t";
		s += QString::number((double)geometry().width()/(double)ml->canvas()->width()) + "\t";
		s += QString::number((double)geometry().height()/(double)ml->canvas()->height());
		s += "</PageGeometry>\n";
	}

	s+=saveTitle();
	s+="<Antialiasing>" + QString::number(d_antialiasing) + "</Antialiasing>\n";
	s+="<Autoscaling>" + QString::number(d_auto_scale) + "</Autoscaling>\n";
	s+="<ScaleFonts>" + QString::number(autoScaleFonts) + "</ScaleFonts>\n";
	s+="<GridOnTop>" + QString::number(d_grid_on_top) + "</GridOnTop>\n";
	s+="<MissingDataGap>" + QString::number(d_missing_data_gap) + "</MissingDataGap>\n";
	s+="Background\t" + paletteBackgroundColor().name() + "\t";
	s+=QString::number(paletteBackgroundColor().alpha()) + "\n";
	s+=saveBackgroundImage();
	s+="Margin\t"+QString::number(margin())+"\n";
	s+="Border\t"+QString::number(lineWidth())+"\t"+frameColor().name()+"\n";
	s+=grid()->saveToString();
	s+=saveEnabledAxes();
	s+="AxesTitles\t"+saveScaleTitles();
	s+=saveAxesTitleColors();
	s+=saveAxesTitleAlignement();
	s+=saveFonts();
	s+=saveAxesColors();
	s+=saveAxesBaseline();
	s+=saveCanvas();

    if (!saveAsTemplate)
	   s+=saveCurves();

	s+=saveScale();
	s+=saveAxesFormulas();
	s+=saveLabelsFormat();
	s+=saveAxesLabelsType();
	s+=saveTicksType();
	s+="TicksLength\t"+QString::number(minorTickLength())+"\t"+QString::number(majorTickLength())+"\n";
	s+=saveAxesBackbones();
	s+="AxesLineWidth\t"+QString::number(axesLinewidth())+"\n";
	s+=saveLabelsRotation();
	s+=saveLabelsPrefixAndSuffix();
	s+=saveTickLabelsSpace();
	s+=saveEnabledTickLabels();
	s+=saveMarkers();
	if (d_Douglas_Peuker_tolerance > 0.0){
		s += "<SpeedMode>" + QString::number(d_Douglas_Peuker_tolerance) + "\t";
		s += QString::number(d_speed_mode_points) + "</SpeedMode>\n";
	}

	if (d_image_profiles_tool){
		s += "<ImageProfileTool>" +  d_image_profiles_tool->matrix()->objectName();
		if (!d_image_profiles_tool->horizontalTable().isNull())
			s += "\t" + d_image_profiles_tool->horizontalTable()->objectName();
		if (!d_image_profiles_tool->verticalTable().isNull())
			s += "\t" + d_image_profiles_tool->verticalTable()->objectName();
		s += "</ImageProfileTool>\n";
		s += "<ImageProfileValues>";
		s += QString::number(d_image_profiles_tool->xValue()) + "\t" + QString::number(d_image_profiles_tool->yValue());
		int pixels = d_image_profiles_tool->averagePixels();
		if (pixels > 1)
			s += "\t" + QString::number(pixels);
		s += "</ImageProfileValues>\n";
	}

	if (isWaterfallPlot()){
		s += "<waterfall>" + QString::number(d_waterfall_offset_x) + ",";
		s += QString::number(d_waterfall_offset_y) + ",";
		bool sideLines = d_curves.size() > 0 && curve(0)->sideLinesEnabled();
		s += QString::number(sideLines) + "</waterfall>\n";
	}

	s += "</graph>\n";
	return s;
}

QString Graph::saveBackgroundImage()
{
	if (d_canvas_bkg_path.isEmpty())
		return QString();

	QString s = "<BackgroundImage>\n";
	s += "<path>" + d_canvas_bkg_path + "</path>\n";
	s += "<xpm>\n";
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	d_canvas_bkg_pix.save(&buffer, "XPM");
	s += QString(bytes);
	s += "</xpm>\n";

	return s + "</BackgroundImage>\n";
}

void Graph::restoreBackgroundImage(const QStringList& lst)
{
	QStringList::const_iterator line;
	for (line = lst.begin(); line != lst.end(); line++){
		QString s = *line;
		if (s.contains("<path>"))
			d_canvas_bkg_path = s.remove("<path>").remove("</path>");
		else if (s.contains("<xpm>")){
			QString xpm;
			while ( s != "</xpm>" ){
				s = *(++line);
				xpm += s + "\n";
			}
			d_canvas_bkg_pix.loadFromData(xpm.toAscii());
		}
	}
}

void Graph::updateMarkersBoundingRect(bool rescaleEvent)
{
	foreach(FrameWidget *f, d_enrichments){
		if (!rescaleEvent && qobject_cast<LegendWidget *>(f)){
			f->resetOrigin();
			continue;
		}

		if (f->attachPolicy() == FrameWidget::Scales)
			f->resetCoordinates();
		else if (rescaleEvent)
			f->updateCoordinates();
	}
	replot();

	if (!d_lines.size())
		return;

	foreach (QwtPlotMarker *i, d_lines)
		((ArrowMarker*)i)->updateBoundingRect();

	replot();
}

void Graph::resizeEvent ( QResizeEvent *e )
{
	QSize size = e->size();
	if (size.height() <= 0)
		size.setHeight(e->oldSize().height());

	resize(size);
	updateLayout();
	updateCurveLabels();

	bool layerSelected = multiLayer()->hasSelectedLayers();
	foreach(FrameWidget *f, d_enrichments){
		if (layerSelected && f->attachPolicy() == FrameWidget::Page)
			f->updateCoordinates();
		else {
			TexWidget *tw = qobject_cast<TexWidget *>(f);
			LegendWidget *l = qobject_cast<LegendWidget *>(f);
			if (tw || l)
				f->resetOrigin();
			else
				f->resetCoordinates();
		}
	}
}

void Graph::scaleFonts(double factor)
{
	if (factor == 1.0 || factor <= 0.0)
		return;

	for (int i = 0; i<QwtPlot::axisCnt; i++){
		QFont font = axisFont(i);
		font.setPointSizeFloat(factor*font.pointSizeF());
		setAxisFont(i, font);

		QwtText title = axisTitle(i);
		font = title.font();
		font.setPointSizeF(factor*font.pointSizeF());
		title.setFont(font);
		((QwtPlot *)this)->setAxisTitle(i, title);
	}

	QwtText t = this->title();
	QFont font = t.font();
	font.setPointSizeF(factor*font.pointSizeF());
	t.setFont(font);
	setTitle(t);

	QList<QwtPlotItem *> curves = curvesList();
	foreach(QwtPlotItem *i, curves){
		if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
		  ((PlotCurve *)i)->type() != Graph::Function &&
		  ((DataCurve *)i)->hasLabels()){
			QFont font = ((DataCurve *)i)->labelsFont();
			font.setPointSizeF(factor*font.pointSizeF());
			((DataCurve *)i)->setLabelsFont(font);
			if (((DataCurve *)i)->hasSelectedLabels())
				notifyFontChange(font);
		}
	}

    foreach(FrameWidget *f, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(f);
		if (!l)
			continue;
		QFont font = l->font();
		font.setPointSizeF(factor*font.pointSizeF());
		l->setFont(font);
		l->resetOrigin();
	}
}

void Graph::setFrame (int width, const QColor& color)
{
	if (frameColor() == color && width == lineWidth())
		return;

	QPalette pal = palette();
	pal.setColor(QColorGroup::Foreground, color);
	setPalette(pal);

	setLineWidth(width);
}

void Graph::setBackgroundColor(const QColor& color)
{
    QColorGroup cg;
	QPalette p = palette();
	p.setColor(QColorGroup::Window, color);
    setPalette(p);

    setAutoFillBackground(true);
	emit modifiedGraph();
}

QString Graph::penStyleName(Qt::PenStyle style)
{
	if (style==Qt::SolidLine)
		return "SolidLine";
	else if (style==Qt::DashLine)
		return "DashLine";
	else if (style==Qt::DotLine)
		return "DotLine";
	else if (style==Qt::DashDotLine)
		return "DashDotLine";
	else if (style==Qt::DashDotDotLine)
		return "DashDotDotLine";
	else
		return "SolidLine";
}

Qt::PenStyle Graph::getPenStyle(int style)
{
	Qt::PenStyle linePen = Qt::SolidLine;
	switch (style)
	{
		case 0:
		break;
		case 1:
			linePen=Qt::DashLine;
		break;
		case 2:
			linePen=Qt::DotLine;
		break;
		case 3:
			linePen=Qt::DashDotLine;
		break;
		case 4:
			linePen=Qt::DashDotDotLine;
		break;
	}
	return linePen;
}

Qt::PenStyle Graph::getPenStyle(const QString& s)
{
	Qt::PenStyle style = Qt::SolidLine;
	if (s == "DashLine")
		style=Qt::DashLine;
	else if (s == "DotLine")
		style=Qt::DotLine;
	else if (s == "DashDotLine")
		style=Qt::DashDotLine;
	else if (s == "DashDotDotLine")
		style=Qt::DashDotDotLine;
	return style;
}

int Graph::obsoleteSymbolStyle(int type)
{
	if (type <= 4)
		return type + 1;
	else
		return type + 2;
}

void Graph::showPlotErrorMessage(QWidget *parent, const QStringList& emptyColumns)
{
	QApplication::restoreOverrideCursor();

	int n = (int)emptyColumns.count();
	if (n > 1)
	{
		QString columns;
		for (int i = 0; i < n; i++)
			columns += "<p><b>" + emptyColumns[i] + "</b></p>";

		QMessageBox::warning(parent, tr("QtiPlot - Warning"),
				tr("The columns") + ": " + columns + tr("are empty and will not be added to the plot!"));
	}
	else if (n == 1)
		QMessageBox::warning(parent, tr("QtiPlot - Warning"),
				tr("The column") + " <b>" + emptyColumns[0] + "</b> " + tr("is empty and will not be added to the plot!"));
}

void Graph::showTitleContextMenu()
{
	QMenu titleMenu(this);
	titleMenu.insertItem(QPixmap(":/cut.png"), tr("Cu&t"),this, SLOT(cutTitle()));
	titleMenu.insertItem(QPixmap(":/copy.png"), tr("&Copy"),this, SLOT(copyTitle()));
	titleMenu.insertItem(tr("C&lear"),this, SLOT(clearTitle()));
	titleMenu.insertItem(QPixmap(":/delete.png"), tr("&Delete"),this, SLOT(removeTitle()));
	titleMenu.insertSeparator();
	titleMenu.insertItem(tr("&Properties..."), this, SIGNAL(viewTitleDialog()));
	titleMenu.exec(QCursor::pos());
}

void Graph::cutTitle()
{
	QApplication::clipboard()->setText(title().text(), QClipboard::Clipboard);
	removeTitle();
}

void Graph::copyTitle()
{
	QApplication::clipboard()->setText(title().text(), QClipboard::Clipboard);
}

void Graph::removeAxisTitle()
{
	int selectedAxis = scalePicker->currentAxis()->alignment();
	int axis = (selectedAxis + 2)%4;//unconsistent notation in Qwt enumerations between
  	//QwtScaleDraw::alignment and QwtPlot::Axis
  	setAxisTitle(axis, "");//due to the plot layout updates, we must always have a non empty title
}

void Graph::clearAxisTitle()
{
	int selectedAxis = scalePicker->currentAxis()->alignment();
	int axis = (selectedAxis + 2)%4;//unconsistent notation in Qwt enumerations between
  	//QwtScaleDraw::alignment and QwtPlot::Axis
  	setAxisTitle(axis, " ");//due to the plot layout updates, we must always have a non empty title
}

void Graph::cutAxisTitle()
{
	copyAxisTitle();
	removeAxisTitle();
}

void Graph::copyAxisTitle()
{
	int selectedAxis = scalePicker->currentAxis()->alignment();
	int axis = (selectedAxis + 2)%4;//unconsistent notation in Qwt enumerations between
  	//QwtScaleDraw::alignment and QwtPlot::Axis
  	QApplication::clipboard()->setText(axisTitle(axis).text(), QClipboard::Clipboard);
	}

void Graph::showAxisTitleMenu()
{
	QMenu titleMenu(this);
	titleMenu.insertItem(QPixmap(":/cut.png"), tr("Cu&t"), this, SLOT(cutAxisTitle()));
	titleMenu.insertItem(QPixmap(":/copy.png"), tr("&Copy"), this, SLOT(copyAxisTitle()));
	titleMenu.insertItem(tr("C&lear"),this, SLOT(clearAxisTitle()));
	titleMenu.insertItem(QPixmap(":/delete.png"), tr("&Delete"),this, SLOT(removeAxisTitle()));
	titleMenu.insertSeparator();
	titleMenu.insertItem(tr("&Properties..."), this, SIGNAL(showAxisTitleDialog()));
	titleMenu.exec(QCursor::pos());
}

void Graph::showAxisContextMenu(int axis)
{
	QMenu menu(this);
	menu.setCheckable(true);
	menu.insertItem(QPixmap(":/unzoom.png"), tr("&Rescale to show all"), this, SLOT(setAutoScale()), tr("Ctrl+Shift+R"));
	menu.insertSeparator();
	menu.insertItem(tr("&Hide axis"), this, SLOT(hideSelectedAxis()));

	int gridsID = menu.insertItem(tr("&Show grids"), this, SLOT(showGrids()));
	if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale){
		if (grid()->yEnabled())
			menu.setItemChecked(gridsID, true);
	} else {
		if (grid()->xEnabled())
			menu.setItemChecked(gridsID, true);
	}

	menu.insertSeparator();
	menu.insertItem(tr("&Scale..."), this, SLOT(showScaleDialog()));
	menu.insertItem(tr("&Properties..."), this, SLOT(showAxisDialog()));
	menu.exec(QCursor::pos());
}

void Graph::showAxisDialog()
{
	QwtScaleWidget *scale = scalePicker->currentAxis();
	if (scale)
		emit showAxisDialog(scale->alignment());
}

void Graph::showScaleDialog()
{
	emit axisDblClicked(scalePicker->currentAxis()->alignment());
}

void Graph::hideSelectedAxis()
{
	int axis = -1;
	int selectedAxis = scalePicker->currentAxis()->alignment();
	if (selectedAxis == QwtScaleDraw::LeftScale || selectedAxis == QwtScaleDraw::RightScale)
		axis = selectedAxis - 2;
	else
		axis = selectedAxis + 2;

	enableAxis(axis, false);
	scalePicker->refresh();
	emit modifiedGraph();
}

void Graph::showGrids()
{
	showGrid (scalePicker->currentAxis()->alignment());
}

void Graph::showGrid()
{
	showGrid(QwtScaleDraw::LeftScale);
	showGrid(QwtScaleDraw::BottomScale);
}

void Graph::showGrid(int axis)
{
	if (!d_grid)
		return;

	if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale){
		d_grid->enableY(!d_grid->yEnabled());
		d_grid->enableYMin(!d_grid->yMinEnabled());
	} else if (axis == QwtScaleDraw::BottomScale || axis == QwtScaleDraw::TopScale){
		d_grid->enableX(!d_grid->xEnabled());
		d_grid->enableXMin(!d_grid->xMinEnabled());
	} else
		return;

	replot();
	emit modifiedGraph();
}

void Graph::copyScaleWidget(Graph* g, int i)
{
	if (g->axisEnabled (i)){
		enableAxis(i);
		QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
		if (scale){
			scale->setSpacing(g->axisWidget(i)->spacing());
			scale->setMargin(g->axisWidget(i)->margin());
			QPalette pal = scale->palette();
			pal.setColor(QColorGroup::Foreground, g->axisColor(i));
			pal.setColor(QColorGroup::Text, g->axisLabelsColor(i));
			scale->setPalette(pal);
			setAxisFont (i, g->axisFont(i));

			d_axis_titles[i] = g->axisTitleString(i);
			scale->setTitle(g->axisTitle(i));
			scale->setLayoutFlag(QwtScaleWidget::TitleInverted, g->axisWidget(i)->testLayoutFlag(QwtScaleWidget::TitleInverted));
		}
	}
}

void Graph::copyScaleDraw(Graph* g, int i)
{
	ScaleDraw *sdg = (ScaleDraw *)g->axisScaleDraw (i);
	if (sdg->hasComponent(QwtAbstractScaleDraw::Labels)){
		ScaleDraw::ScaleType type = sdg->scaleType();
		if (type == ScaleDraw::Numeric)
			setLabelsNumericFormat(i, g->axisLabelFormat(i), g->axisLabelPrecision(i), sdg->formula());
		else if (type == ScaleDraw::Day)
			setLabelsDayFormat(i, sdg->nameFormat());
		else if (type == ScaleDraw::Month)
			setLabelsMonthFormat(i, sdg->nameFormat());
		else if (type == ScaleDraw::Time || type == ScaleDraw::Date)
			setLabelsDateTimeFormat(i, type, sdg->formatString());
		else
			setAxisScaleDraw(i, new ScaleDraw(this, sdg->labelsList(), sdg->formatString(), sdg->scaleType()));
	} else
		axisScaleDraw (i)->enableComponent(QwtAbstractScaleDraw::Labels, false);

	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(i);
	sd->enableComponent (QwtAbstractScaleDraw::Backbone, sdg->hasComponent(QwtAbstractScaleDraw::Backbone));
	sd->setSpacing(sdg->spacing());
	sd->setShowTicksPolicy(sdg->showTicksPolicy());
	sd->setPrefix(sdg->prefix());
	sd->setSuffix(sdg->suffix());

	setAxisTicksLength(i, sdg->majorTicksStyle(), sdg->minorTicksStyle(), g->minorTickLength(), g->majorTickLength());

	const ScaleEngine *se = (ScaleEngine *)g->axisScaleEngine(i);
	if (!se)
		return;

	const QwtScaleDiv *div = g->axisScaleDiv(i);
	//set same scale
	setScale(i, div->lowerBound(), div->upperBound(), fabs(g->axisStep(i)), div->ticks(QwtScaleDiv::MajorTick).size(), g->axisMaxMinor(i),
			se->type(), se->testAttribute(QwtScaleEngine::Inverted), se->axisBreakLeft(), se->axisBreakRight(),
			se->breakPosition(), se->stepBeforeBreak(), se->stepAfterBreak(), se->minTicksBeforeBreak(),
			se->minTicksAfterBreak(), se->log10ScaleAfterBreak(), se->breakWidth(), se->hasBreakDecoration());
}

void Graph::copyEnrichments(Graph* g)
{
	if (!g)
		return;

	QList<FrameWidget *> enrichements = g->enrichmentsList();
	foreach (FrameWidget *e, enrichements){
		PieLabel *l = qobject_cast<PieLabel *>(e);
		if (l)
			continue;
		add(e);
	}

	QList<ArrowMarker *> lines = g->arrowsList();
	foreach (ArrowMarker *a, lines)
		addArrow(a);
}

void Graph::copy(Graph* g)
{
	d_waterfall_offset_x = g->waterfallXOffset();
	d_waterfall_offset_y = g->waterfallYOffset();

	setMargin(g->margin());
	setBackgroundColor(g->paletteBackgroundColor());
	setFrame(g->lineWidth(), g->frameColor());
	setCanvasBackground(g->canvasBackground());
	setCanvasBackgroundImage(g->canvasBackgroundFileName(), false);
	setAxisTitlePolicy(g->axisTitlePolicy());

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		enableAxis(i, g->axisEnabled(i));
		copyScaleWidget(g, i);
	}

	grid()->copy(g->grid());
	d_grid_on_top = g->hasGridOnTop();
	d_missing_data_gap = g->isMissingDataGapEnabled();
	setTitle(g->title());
	setCanvasFrame(g->canvasFrameWidth(), g->canvasFrameColor());
	setAxesLinewidth(g->axesLinewidth());

	d_Douglas_Peuker_tolerance = g->getDouglasPeukerTolerance();

	copyCurves(g);

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		copyScaleDraw(g, i);
		setAxisLabelRotation(i, g->labelsRotation(i));
	}

	updateAxesTitles();
    updateLayout();
	d_auto_scale = g->isAutoscalingEnabled();

	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();

	d_disable_curve_antialiasing = g->isCurveAntialiasingDisabled();
	d_max_antialising_size = g->maxAntialisingSize();
	setAntialiasing(g->antialiasing(), false);

	copyEnrichments(g);

	autoScaleFonts = g->autoscaleFonts();
	d_synchronize_scales = g->hasSynchronizedScaleDivisions();
}

void Graph::copyCurves(Graph* g)
{
	if (!g)
		return;
	
	QList<QwtPlotItem *> curvesList = g->curvesList();
	foreach (QwtPlotItem *it, curvesList){
		if (it->rtti() == QwtPlotItem::Rtti_PlotCurve){
			DataCurve *cv = (DataCurve *)it;
			int n = cv->dataSize();
			int style = ((PlotCurve *)it)->type();
			Table *t = cv->table();

			PlotCurve *c = 0;
			if (style == Pie){
				c = new PieCurve(t, cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
			} else if (style == Function) {
				c = new FunctionCurve(cv->title().text());
				insertCurve(c);
				((FunctionCurve*)c)->copy((FunctionCurve*)cv);
				((FunctionCurve*)c)->loadData(n);
			} else if (style == VerticalBars || style == HorizontalBars){
				c = new QwtBarCurve(((QwtBarCurve*)cv)->orientation(), t, cv->xColumnName(),
									cv->title().text(), cv->startRow(), cv->endRow());
                insertCurve(c);
				((QwtBarCurve*)c)->copy((QwtBarCurve*)cv);
			} else if (style == ErrorBars){
				ErrorBarsCurve *er = (ErrorBarsCurve*)cv;
				DataCurve *master_curve = masterCurve(er);
				if (master_curve){
					c = new ErrorBarsCurve(t, cv->title().text());
					insertCurve(c);
					((ErrorBarsCurve*)c)->copy(er);
					((ErrorBarsCurve*)c)->setMasterCurve(master_curve);
				}
			} else if (style == Histogram){
			    QwtHistogram *h = (QwtHistogram*)cv;
				if (h->matrix())
					c = new QwtHistogram(h->matrix());
				else
					c = new QwtHistogram(t, cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
				((QwtHistogram *)c)->copy(h);
			} else if (style == VectXYXY || style == VectXYAM){
				VectorCurve::VectorStyle vs = VectorCurve::XYXY;
				if (style == VectXYAM)
					vs = VectorCurve::XYAM;
				c = new VectorCurve(vs, t, cv->xColumnName(), cv->title().text(),
									((VectorCurve *)cv)->vectorEndXAColName(),
									((VectorCurve *)cv)->vectorEndYMColName(),
									cv->startRow(), cv->endRow());
                insertCurve(c);
				((VectorCurve *)c)->copy((const VectorCurve *)cv);
			} else if (style == Box){
				c = new BoxCurve(t, cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);

				QVector<double> y(n);
				for (int j = 0; j < n; j++)
					y[j] = cv->y(j);
				QwtSingleArrayData dat(cv->x(0), y, n);
				c->setData(dat);
				((BoxCurve*)c)->copy((BoxCurve *)cv);
			} else {
				if (t != cv->xTable())
					c = new DataCurve(cv->xTable(), cv->xColumnName(), t, cv->title().text(), cv->startRow(), cv->endRow());
				else
					c = new DataCurve(t, cv->xColumnName(), cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
			}
			if (c->type() != Box && c->type() != ErrorBars && c->type() != Function){
				((DataCurve *)c)->loadData();
				c->enableSideLines(cv->sideLinesEnabled());

				if (c->type() == Pie)
					((PieCurve*)c)->clone((PieCurve*)cv);
				else
					((DataCurve *)c)->clone(cv);
			}

			c->setPen(cv->pen());
			c->setBrush(cv->brush());
			c->setStyle(cv->style());
			c->setSymbol(cv->symbol());

			if (cv->testCurveAttribute (QwtPlotCurve::Fitted)){
				c->setCurveAttribute(QwtPlotCurve::Fitted, true);
				if (d_Douglas_Peuker_tolerance > 0.0 && c->dataSize() >= d_speed_mode_points){
					QwtWeedingCurveFitter *fitter = new QwtWeedingCurveFitter(d_Douglas_Peuker_tolerance);
					c->setCurveFitter(fitter);
				}
			} else if (cv->testCurveAttribute (QwtPlotCurve::Inverted))
				c->setCurveAttribute(QwtPlotCurve::Inverted, true);

			c->setRenderHint(QwtPlotItem::RenderAntialiased, cv->testRenderHint(QwtPlotItem::RenderAntialiased));
			c->setAxis(cv->xAxis(), cv->yAxis());
			c->setVisible(cv->isVisible());
			c->setPlotStyle(((PlotCurve *)it)->plotStyle());

			QList<QwtPlotCurve *>lst = g->fitCurvesList();
			if (lst.contains((QwtPlotCurve *)it))
				d_fit_curves << c;
		} else if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
  	     	Spectrogram *sp = ((Spectrogram *)it)->copy(this);
  	        insertCurve(sp);

			sp->setRenderHint(QwtPlotItem::RenderAntialiased, it->testRenderHint(QwtPlotItem::RenderAntialiased));
  	        sp->showColorScale(((Spectrogram *)it)->colorScaleAxis(), ((Spectrogram *)it)->hasColorScale());
  	        sp->setColorBarWidth(((Spectrogram *)it)->colorBarWidth());
			sp->setAxis(it->xAxis(), it->yAxis());
			sp->setVisible(it->isVisible());
        }
    }
}


void Graph::plotBox(Table *w, const QStringList& names, int startRow, int endRow)
{
	if (endRow < 0)
		endRow = w->numRows() - 1;

	QList<QColor> indexedColors = ColorBox::defaultColors();
	MultiLayer *ml = multiLayer();
	if (ml){
		ApplicationWindow *app = ml->applicationWindow();
		if (app)
			indexedColors = app->indexedColors();
	}

	QColor color = Qt::black;
	for (int j = 0; j < names.count(); j++){
        BoxCurve *c = new BoxCurve(w, names[j], startRow, endRow);
		insertCurve(c);

		if (j < indexedColors.size())
			color = indexedColors[j];
        c->setData(QwtSingleArrayData(double(j+1), QwtArray<double>(), 0));
        c->loadData();
		c->setPen(QPen(color, 1));
		c->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, QBrush(), QPen(color, 1), QSize(7, 7)));
	}

	foreach(FrameWidget *fw, d_enrichments){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l && l->isAutoUpdateEnabled())
			l->setText(legendText());
	}

	ScaleDraw *sd = new ScaleDraw(this, w->selectedYLabels(), w->objectName(), ScaleDraw::ColHeader);
	sd->setShowTicksPolicy(ScaleDraw::HideBeginEnd);
	setAxisScaleDraw (QwtPlot::xBottom, sd);
	setAxisMaxMajor(QwtPlot::xBottom, names.count() + 1);
	setAxisMaxMinor(QwtPlot::xBottom, 0);

	setAxisScaleDraw (QwtPlot::xTop, new ScaleDraw(this, w->selectedYLabels(), w->objectName(), ScaleDraw::ColHeader));
	setAxisMaxMajor(QwtPlot::xTop, names.count() + 1);
	setAxisMaxMinor(QwtPlot::xTop, 0);
}

void Graph::setCurveStyle(int index, int s)
{
	PlotCurve *c = (PlotCurve *)curve(index);
	if (!c)
		return;

    int curve_type = c->plotStyle();
    if (curve_type == VerticalBars || curve_type == HorizontalBars || curve_type == Histogram ||
        curve_type == Pie || curve_type == Box || curve_type == ErrorBars ||
        curve_type == VectXYXY || curve_type == VectXYAM)
        return;//these are not line styles, but distinct curve types and this function must not change the curve type

	c->setCurveAttribute(QwtPlotCurve::Fitted, false);
	c->setCurveAttribute(QwtPlotCurve::Inverted, false);

	if (s == 5){//ancient spline style in Qwt 4.2.0
		s = QwtPlotCurve::Lines;
		c->setCurveAttribute(QwtPlotCurve::Fitted, true);
		c->setPlotStyle(Spline);
	} else if (s == 6){// Vertical Steps
		s = QwtPlotCurve::Steps;
		c->setCurveAttribute(QwtPlotCurve::Inverted, false);
		c->setPlotStyle(VerticalSteps);
	} else if (s == QwtPlotCurve::Steps){// Horizontal Steps
		c->setPlotStyle(HorizontalSteps);
		c->setCurveAttribute(QwtPlotCurve::Inverted, true);
	} else if (s == QwtPlotCurve::Sticks)
		c->setPlotStyle(VerticalDropLines);
	else {//QwtPlotCurve::Lines || QwtPlotCurve::Dots
		if (c->symbol().style() == QwtSymbol::NoSymbol)
			c->setPlotStyle(Line);
		else if (c->symbol().style() != QwtSymbol::NoSymbol && (QwtPlotCurve::CurveStyle)s == QwtPlotCurve::NoCurve)
			c->setPlotStyle(Scatter);
		else
			c->setPlotStyle(LineSymbols);
	}

	c->setStyle((QwtPlotCurve::CurveStyle)s);
	if (curve_type != Function)
		((DataCurve *)c)->enableSpeedMode();
}

BoxCurve* Graph::openBoxDiagram(Table *w, const QStringList& l, int fileVersion)
{
    if (!w)
        return NULL;

    int startRow = 0;
    int endRow = w->numRows()-1;
    if (fileVersion >= 90) {
        startRow = l[l.count()-3].toInt();
        endRow = l[l.count()-2].toInt();
    }

	BoxCurve *c = new BoxCurve(w, l[2], startRow, endRow);
	insertCurve(c);

	c->setData(QwtSingleArrayData(l[1].toDouble(), QwtArray<double>(), 0));
	c->setData(QwtSingleArrayData(l[1].toDouble(), QwtArray<double>(), 0));
	c->loadData();

	c->setMaxStyle(SymbolBox::style(l[16].toInt()));
	c->setP99Style(SymbolBox::style(l[17].toInt()));
	c->setMeanStyle(SymbolBox::style(l[18].toInt()));
	c->setP1Style(SymbolBox::style(l[19].toInt()));
	c->setMinStyle(SymbolBox::style(l[20].toInt()));

	c->setBoxStyle(l[21].toInt());
	c->setBoxWidth(l[22].toInt());
	c->setBoxRange(l[23].toInt(), l[24].toDouble());
	c->setWhiskersRange(l[25].toInt(), l[26].toDouble());
	return c;
}

void Graph::setActiveTool(PlotToolInterface *tool)
{
	if (!tool && d_peak_fit_tool){
		delete d_peak_fit_tool;
		d_peak_fit_tool = NULL;
		return;
	}

	if (tool && tool->rtti() == PlotToolInterface::Rtti_MultiPeakFitTool){
		d_peak_fit_tool = tool;

		if (d_range_selector)
			d_range_selector->setEnabled(false);
		return;
	}

	if (tool && tool->rtti() == PlotToolInterface::Rtti_ImageProfilesTool){
		d_image_profiles_tool = (ImageProfilesTool*)tool;
		return;
	}

	if(d_active_tool)
		delete d_active_tool;

	d_active_tool = tool;
}

void Graph::disableTools()
{
	if (zoomOn())
		zoom(false);

	enablePanningMagnifier(false);

	if (drawLineActive())
		drawLine(false);

	if(d_active_tool)
		delete d_active_tool;
	d_active_tool = NULL;

	if (d_peak_fit_tool)
		delete d_peak_fit_tool;
	d_peak_fit_tool = NULL;

	if (d_range_selector)
		d_range_selector->setVisible(false);
}

void Graph::disableImageProfilesTool()
{
	if (d_image_profiles_tool)
		delete d_image_profiles_tool;
}

bool Graph::hasActiveTool()
{
	if (zoomOn() || drawLineActive() || d_active_tool || d_peak_fit_tool ||
		d_magnifier || d_panner || (d_range_selector && d_range_selector->isVisible()))
		return true;

	return false;
}

bool Graph::enableRangeSelectors(const QObject *status_target, const char *status_slot)
{
	if (!d_range_selector){
		d_range_selector = new RangeSelectorTool(this, status_target, status_slot);
		//setActiveTool(d_range_selector);
		connect(d_range_selector, SIGNAL(changed()), this, SIGNAL(dataRangeChanged()));
	}

	d_range_selector->setVisible(true);
	d_range_selector->setEnabled(true);
	return true;
}

void Graph::guessUniqueCurveLayout(int& colorIndex, int& symbolIndex)
{
	colorIndex = 0;
	symbolIndex = 0;

	QList<QColor> indexedColors = ColorBox::defaultColors();
	QList<int> indexedSymbols = SymbolBox::defaultSymbols();
	MultiLayer *ml = multiLayer();
	if (ml){
		ApplicationWindow *app = ml->applicationWindow();
		if (app){
			indexedColors = app->indexedColors();
			indexedSymbols = app->indexedSymbols();
		}
	}

	int curve_index = d_curves.size() - 1;
	if (curve_index >= 0){// find out the pen color of the master curve
		PlotCurve *c = (PlotCurve *)curve(curve_index);
		if (c && c->type() == ErrorBars){
			ErrorBarsCurve *er = (ErrorBarsCurve *)c;
			DataCurve *master_curve = er->masterCurve();
			if (master_curve){
				colorIndex = indexedColors.indexOf(master_curve->pen().color());
				return;
			}
		}
	}

	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotCurve){
			const QwtPlotCurve *c = (QwtPlotCurve *)it;
			int index = indexedColors.indexOf(c->pen().color());
			if (index > colorIndex)
				colorIndex = index;

			QwtSymbol symb = c->symbol();
			index = indexedSymbols.indexOf(int(symb.style()));
			if (index < 0)
				symbolIndex = 0;
			else if (index > symbolIndex)
				symbolIndex = index;
		}
	}
	if (d_curves.size() > 1){
		colorIndex = (++colorIndex)%16;
		symbolIndex = (++symbolIndex)%15;
	} else
		symbolIndex = 0;

	if (colorIndex == 13) //avoid white invisible curves
		colorIndex = 0;
}

void Graph::addFitCurve(QwtPlotCurve *c)
{
	if (c)
		d_fit_curves << c;
}

void Graph::deleteFitCurves()
{
	foreach(QwtPlotCurve *c, d_fit_curves)
		removeCurve(c);

	replot();
}

//! Returns a pointer to the spectrogram which data source is matrix m (the pointer can be NULL)
Spectrogram* Graph::spectrogram(Matrix *m)
{
	if (!m)
		return NULL;

	foreach (QwtPlotItem *item, d_curves){
		if(item && item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
			Spectrogram *s = (Spectrogram *)item;
			if (s && s->matrix() == m)
				return s;
		 }
	}
	return NULL;
}

Spectrogram* Graph::plotSpectrogram(Matrix *m, CurveType type)
{
	if (!m || (type != GrayScale && type != ColorMap && type != Contour))
  		return 0;

	if (plotItemsList().contains(m->objectName()))
		return spectrogram(m);

  	Spectrogram *d_spectrogram = new Spectrogram(this, m);
	insertCurve(d_spectrogram);

  	if (type == GrayScale){
  		d_spectrogram->setGrayScale();
  		d_spectrogram->clearLabels();
  	} else if (type == Contour){
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, false);
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    } else if (type == ColorMap){
  	    d_spectrogram->setDefaultColorMap();
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
	}

  	QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
  	rightAxis->setColorBarEnabled(type != Contour);
  	rightAxis->setColorMap(d_spectrogram->data().range(), d_spectrogram->colorMap());

	if (type != Contour)
		setAxisScale(QwtPlot::yRight,
		d_spectrogram->data().range().minValue(),
		d_spectrogram->data().range().maxValue());

  	replot();

	updateSecondaryAxis(QwtPlot::xTop);
	if (type == Contour){
		d_spectrogram->setColorScaleAxis(QwtPlot::xTop);
		updateSecondaryAxis(QwtPlot::yRight);
	}

    d_zoomer[0]->setZoomBase();
    d_zoomer[1]->setZoomBase();

	return d_spectrogram;
}

void Graph::restoreSpectrogram(ApplicationWindow *app, const QStringList& lst)
{
	QStringList::const_iterator line = lst.begin();
	QString s = (*line).stripWhiteSpace();
	QString matrixName = s.remove("<matrix>").remove("</matrix>");
	Matrix *m = app->matrix(matrixName);
	if (!m)
		return;

	Spectrogram *sp = new Spectrogram(this, m);
	insertCurve(sp);

	for (line++; line != lst.end(); line++){
		QString s = *line;
		if (s.contains("<useMatrixFormula>") && (s.trimmed().remove("<useMatrixFormula>").remove("</useMatrixFormula>")).toInt())
			sp->setUseMatrixFormula(true);
		else if (s.contains("<xAxis>")){
			sp->setXAxis(s.trimmed().remove("<xAxis>").remove("</xAxis>").toInt());
		} else if (s.contains("<yAxis>")){
			sp->setYAxis(s.trimmed().remove("<yAxis>").remove("</yAxis>").toInt());
		} else if (s.contains("<ColorPolicy>")){
			int color_policy = s.remove("<ColorPolicy>").remove("</ColorPolicy>").stripWhiteSpace().toInt();
			if (color_policy == Spectrogram::GrayScale)
				sp->setGrayScale();
			else if (color_policy == Spectrogram::Default)
				sp->setDefaultColorMap();
		} else if (s.contains("<ColorMap>")){
			QStringList lst;
			while ((*line).trimmed() != "</ColorMap>" )
				lst << *(++line);
			lst.pop_back();
			sp->setCustomColorMap(LinearColorMap::fromXmlStringList(lst));
		} else if (s.contains("<Image>")){
			int mode = s.remove("<Image>").remove("</Image>").stripWhiteSpace().toInt();
			sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, mode);
		} else if (s.contains("<ContourLines>")){
			int contours = s.remove("<ContourLines>").remove("</ContourLines>").stripWhiteSpace().toInt();
			sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, contours);
			if (contours){
				s = (*(++line)).stripWhiteSpace();
				int levels = s.remove("<Levels>").remove("</Levels>").toInt();
				QwtValueList levelsLst;
				for (int i = 0; i < levels; i++){
					s = (*(++line)).stripWhiteSpace();
					if (s.contains("</z>"))
						levelsLst += s.remove("<z>").remove("</z>").toDouble();
				}
				if (levelsLst.size() > 0)
					sp->setContourLevels(levelsLst);
				else
					sp->setLevelsNumber(levels);

				s = (*(++line)).stripWhiteSpace();
				int defaultPen = s.remove("<DefaultPen>").remove("</DefaultPen>").toInt();
				if (!defaultPen)
					sp->setColorMapPen();
				else {
					s = (*(++line)).stripWhiteSpace();
					QColor c = QColor(s.remove("<PenColor>").remove("</PenColor>"));
					s = (*(++line)).stripWhiteSpace();
					double width = s.remove("<PenWidth>").remove("</PenWidth>").toDouble();
					s = (*(++line)).stripWhiteSpace();
					int style = s.remove("<PenStyle>").remove("</PenStyle>").toInt();
					QPen pen = QPen(c, width, Graph::getPenStyle(style));
					pen.setCosmetic(true);
					sp->setDefaultContourPen(pen);
				}
				sp->showContourLineLabels(false);
			} else
				sp->clearLabels();
		} else if (s.contains("<PenList>")){
			int levels = sp->contourLevels().size();
			QList <QPen> penLst;
			for (int i = 0; i < levels; i++){
				s = (*(++line)).stripWhiteSpace();
				if (s.contains("</pen>")){
					QStringList lst = s.remove("<pen>").remove("</pen>").split(",");
					if (lst.size() == 3)
						penLst << QPen(QColor(lst[0]), lst[1].toDouble(), PenStyleBox::penStyle(lst[2].toInt()));
				}
			}
			if (!penLst.isEmpty())
				sp->setContourPenList(penLst);
		} else if (s.contains("<Labels>")){
			sp->showContourLineLabels(true);
			s = (*(++line)).stripWhiteSpace();
			sp->setLabelsColor(QColor(s.remove("<Color>").remove("</Color>")));
			s = (*(++line)).stripWhiteSpace();
			sp->setLabelsWhiteOut(s.remove("<WhiteOut>").remove("</WhiteOut>").toInt());
			s = (*(++line)).stripWhiteSpace();
			sp->setLabelsRotation(s.remove("<Angle>").remove("</Angle>").toDouble());
			s = (*(++line)).stripWhiteSpace();
			double xOffset = s.remove("<xOffset>").remove("</xOffset>").toDouble();
			s = (*(++line)).stripWhiteSpace();
			double yOffset = s.remove("<yOffset>").remove("</yOffset>").toDouble();
			sp->setLabelsOffset(xOffset, yOffset);
			s = (*(++line)).stripWhiteSpace().remove("<Font>").remove("</Font>");
			QFont fnt;
			fnt.fromString(s);
			sp->setLabelsFont(fnt);
		} else if (s.contains("<offset>")){
			QStringList lst = s.remove("<offset>").remove("</offset>").split(",");
			if (lst.size() == 3)
				sp->setLabelOffset(lst[0].toInt(), lst[1].toDouble(), lst[2].toDouble());
		} else if (s.contains("<ColorBar>")){
			s = *(++line);
			int color_axis = s.remove("<axis>").remove("</axis>").stripWhiteSpace().toInt();
			s = *(++line);
			int width = s.remove("<width>").remove("</width>").stripWhiteSpace().toInt();

			QwtScaleWidget *colorAxis = axisWidget(color_axis);
			if (colorAxis){
				colorAxis->setColorBarWidth(width);
				colorAxis->setColorBarEnabled(true);
			}
			line++;
		} else if (s.contains("<Visible>")){
			int on = s.remove("<Visible>").remove("</Visible>").stripWhiteSpace().toInt();
			sp->setVisible(on);
		}
	}
}

void Graph::restoreCurveLabels(int curveID, const QStringList& lst)
{
    DataCurve *c = (DataCurve *)curve(curveID);
	if (!c)
		return;

    c->restoreLabels(lst);
}

bool Graph::validCurvesDataSize()
{
	if (d_curves.size() == 0){
		QMessageBox::warning(this, tr("QtiPlot - Warning"), tr("There are no curves available on this plot!"));
		return false;
	} else {
		foreach (QwtPlotItem *item, d_curves){
  	         if(item && item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
  	             QwtPlotCurve *c = (QwtPlotCurve *)item;
  	             if (c->dataSize() >= 2)
                    return true;
  	         }
  	    }
		QMessageBox::warning(this, tr("QtiPlot - Error"),
		tr("There are no curves with more than two points on this plot. Operation aborted!"));
		return false;
	}
}

Graph::~Graph()
{
	if(d_markers_selector)
		delete d_markers_selector;
	if(d_peak_fit_tool)
		delete d_peak_fit_tool;
	if(d_active_tool)
		delete d_active_tool;
	if (d_range_selector)
		delete d_range_selector;
	delete titlePicker;
	delete scalePicker;
	delete cp;

	if (d_magnifier)
		delete d_magnifier;
	if (d_panner)
		delete d_panner;

	disableImageProfilesTool();

	foreach(FrameWidget *fw, d_enrichments)
		fw->close();
}

void Graph::setAntialiasing(bool on, bool update)
{
	if (d_antialiasing == on && !update)
		return;

	d_antialiasing = on;

	if (update){
		foreach(QwtPlotItem *it, d_curves)
			it->setRenderHint(QwtPlotItem::RenderAntialiased, isCurveAntialiasingEnabled(it));
		foreach (QwtPlotMarker *i, d_lines)
			i->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
		replot();
	}
}

bool Graph::isCurveAntialiasingEnabled(QwtPlotItem *it)
{
	if (!d_antialiasing || !it)
		return false;

	if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
		return true;

	if (d_disable_curve_antialiasing && ((PlotCurve *)it)->dataSize() > d_max_antialising_size)
		return false;

	return true;
}

void Graph::disableCurveAntialiasing(bool disable, int maxPoints)
{
	if (!d_antialiasing)
		return;

	if (d_disable_curve_antialiasing == disable && d_max_antialising_size == maxPoints)
		return;

	d_disable_curve_antialiasing = disable;
	d_max_antialising_size = maxPoints;

	if (d_curves.isEmpty())
		return;

	foreach(QwtPlotItem *it, d_curves)
		it->setRenderHint(QwtPlotItem::RenderAntialiased, isCurveAntialiasingEnabled(it));
	replot();
}

bool Graph::focusNextPrevChild ( bool )
{
	/*int markers = d_lines.size();
	int enrichments = d_enrichments.size();
	if (markers < 1 && enrichments < 1)
		return false;

	if (d_selected_arrow){
		int next = d_lines.indexOf(d_selected_arrow) + 1;
        if (next >= 0 && next < markers){//select next arrow
            cp->disableEditing();
            deselectMarker();
            setSelectedMarker(d_lines.at(next));
            return true;
        } else if (enrichments){ //select first enrichment
            cp->disableEditing();
            deselectMarker();
            select(d_enrichments[0]);
            return true;
        }
    }

    if (d_active_enrichment){
        int next = d_enrichments.indexOf(d_active_enrichment) + 1;
        if (next >= 0 && next < enrichments){//select next enrichment
            cp->disableEditing();
            deselectMarker();
            select(d_enrichments[next]);
            return true;
        } else if (markers){//select first arrow
            deselectMarker();
            setSelectedMarker(d_lines.at(0));
            return true;
        }
    }

    if (enrichments) { //select first enrichment
        select(d_enrichments[0]);
        return true;
    }

    if (markers){//select first arrow
        deselectMarker();
        setSelectedMarker(d_lines.at(0));
        return true;
    }*/

	return false;
}

QString Graph::axisFormatInfo(int axis)
{
	if (axis < 0 || axis > QwtPlot::axisCnt)
		return QString();

	return ((ScaleDraw *)axisScaleDraw(axis))->formatString();
}

void Graph::updateCurveNames(const QString& oldName, const QString& newName, bool updateTableName)
{
    //update plotted curves list
	foreach(QwtPlotItem *it, d_curves){
        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;

        DataCurve *c = (DataCurve *)it;
		if (c->type() != Function && c->plotAssociation().join(",").contains(oldName))
            c->updateColumnNames(oldName, newName, updateTableName);
	}

	updateAxesTitles();
	replot();
}

void Graph::setCurveFullRange(int curveIndex)
{
	DataCurve *c = (DataCurve *)curve(curveIndex);
	if (c){
		c->setFullRange();
		updatePlot();
		emit modifiedGraph();
	}
}

void Graph::setCurveLineColor(int curveIndex, int colorIndex)
{
	QwtPlotCurve *c = curve(curveIndex);
	if (c){
		QPen pen = c->pen();
		pen.setColor(ColorBox::defaultColor(colorIndex));
		c->setPen(pen);
		replot();
		emit modifiedGraph();
	}
}

void Graph::setCurveLineColor(int curveIndex, QColor qColor)
{
	QwtPlotCurve *c = curve(curveIndex);
	if (c){
		QPen pen = c->pen();
		pen.setColor(qColor);
		c->setPen(pen);
		replot();
		emit modifiedGraph();
	}
}

void Graph::setCurveLineStyle(int curveIndex, Qt::PenStyle style)
{
	QwtPlotCurve *c = curve(curveIndex);
	if (c){
		QPen pen = c->pen();
		pen.setStyle(style);
		c->setPen(pen);
		replot();
		emit modifiedGraph();
	}
}

void Graph::setCurveLineWidth(int curveIndex, double width)
{
	QwtPlotCurve *c = curve(curveIndex);
	if (c){
		QPen pen = c->pen();
		pen.setWidthF(width);
		c->setPen(pen);
		replot();
		emit modifiedGraph();
	}
}

void Graph::setGrayScale()
{
	if (isPiePlot())
		return;

	int curves = d_curves.size();
	int dv = int(255/double(curves));
	int i = 0;
	QColor color = Qt::black;
	int hue = color.hue();
	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
			((Spectrogram *)it)->setGrayScale();
			continue;
		}

		PlotCurve *c = (PlotCurve *)it;
		if (c->type() == ErrorBars)
			continue;

		QPen pen = c->pen();
		if (i){
			int v = i*dv;
			if (v > 255)
				v = 0;
			color = QColor::fromHsv(hue, 0, v);
		}
		pen.setColor(color);
		c->setPen(pen);

		QBrush brush = c->brush();
		if (brush.style() != Qt::NoBrush){
			brush.setColor(color);
			c->setBrush(brush);
		}

		QwtSymbol symbol = c->symbol();
		pen = symbol.pen();
		pen.setColor(color);
		symbol.setPen(pen);
		if (symbol.brush().style() != Qt::NoBrush)
			symbol.setBrush(QBrush(color));
		c->setSymbol(symbol);
		i++;
	}

	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)it;
		if (c->type() == ErrorBars){
			ErrorBarsCurve *er = (ErrorBarsCurve *)it;
			DataCurve* mc = er->masterCurve();
			if (mc)
				er->setColor(mc->pen().color());
		}
	}

	replot();
	emit modifiedGraph();
}

void Graph::setIndexedColors()
{
	QList<QColor> colors;
	MultiLayer *ml = multiLayer();
	if (ml && ml->applicationWindow())
		colors = ml->applicationWindow()->indexedColors();
	else
		colors = ColorBox::defaultColors();

	int i = 0;
	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)it;
		if (c->type() == ErrorBars)
			continue;

		QPen pen = c->pen();
		QColor color = colors[i];
		pen.setColor(color);
		c->setPen(pen);

		QBrush brush = c->brush();
		if (brush.style() != Qt::NoBrush){
			brush.setColor(color);
			c->setBrush(brush);
		}

		QwtSymbol symbol = c->symbol();
		pen = symbol.pen();
		pen.setColor(color);
		symbol.setPen(pen);
		if (symbol.brush().style() != Qt::NoBrush)
			symbol.setBrush(QBrush(color));
		c->setSymbol(symbol);
		i++;
	}

	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)it;
		if (c->type() == ErrorBars){
			ErrorBarsCurve *er = (ErrorBarsCurve *)it;
			DataCurve* mc = er->masterCurve();
			if (mc)
				er->setColor(mc->pen().color());
		}
	}

	replot();
	emit modifiedGraph();
}

DataCurve* Graph::masterCurve(ErrorBarsCurve *er)
{
	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;
		if (((PlotCurve *)it)->type() == Function)
			continue;

		DataCurve *c = (DataCurve *)it;
		DataCurve *mc = er->masterCurve();
		if (c && mc && c->plotAssociation() == mc->plotAssociation() &&
			c->startRow() == mc->startRow() && c->endRow() == mc->endRow())
			return c;
	}
	return 0;
}

DataCurve* Graph::masterCurve(const QString& xColName, const QString& yColName)
{
	QString master_curve = xColName + "(X)," + yColName + "(Y)";
	foreach(QwtPlotItem *it, d_curves){
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

		if (((DataCurve *)it)->xColumnName() == xColName && it->title().text() == yColName)
			return (DataCurve *)it;
	}
	return NULL;
}

void Graph::showCurve(int index, bool visible)
{
	QwtPlotItem *it = plotItem(index);
	if (it)
		it->setVisible(visible);

	emit modifiedGraph();
}

int Graph::visibleCurves()
{
    int c = 0;
	foreach(QwtPlotItem *it, d_curves){
    	if (it->isVisible())
            c++;
	}
	return c;
}

QwtScaleWidget* Graph::selectedScale()
{
	return scalePicker->selectedAxis();
}

bool Graph::axisTitleSelected()
{
	return scalePicker->titleSelected();
}

QwtScaleWidget* Graph::currentScale()
{
	return scalePicker->currentAxis();
}

QRect Graph::axisTitleRect(const QwtScaleWidget *scale)
{
	if (!scale)
		return QRect();

	return scalePicker->titleRect(scale);
}

void Graph::setCurrentFont(const QFont& f)
{
	QwtScaleWidget *axis = scalePicker->selectedAxis();
	if (axis){
		if (scalePicker->titleSelected()){
			QwtText title = axis->title();
			title.setFont(f);
			axis->setTitle(title);
		} else if (scalePicker->labelsSelected())
			axis->setFont(f);
		emit modifiedGraph();
	} else if (d_active_enrichment){
		LegendWidget *l = qobject_cast<LegendWidget *>(d_active_enrichment);
		if (l){
			l->setFont(f);
			l->repaint();
			emit modifiedGraph();
		}
	} else if (titlePicker->selected()){
		setTitleFont(f);
		updateMarkersBoundingRect();
	} else {
	    QList<QwtPlotItem *> curves = curvesList();
	    foreach(QwtPlotItem *i, curves){
	        if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
	          ((PlotCurve *)i)->type() != Graph::Function){
                if(((DataCurve *)i)->hasSelectedLabels()){
                   ((DataCurve *)i)->setLabelsFont(f);
                   replot();
                   emit modifiedGraph();
                   return;
                }
			}
	    }
	}
}

void Graph::setCurrentColor(const QColor& c)
{
	QwtScaleWidget *axis = scalePicker->selectedAxis();
	if (axis){
		if (scalePicker->titleSelected()){
			QwtText title = axis->title();
			title.setColor(c);
			axis->setTitle(title);
		} else if (scalePicker->labelsSelected()){
			QPalette pal = axis->palette();
			pal.setColor(QColorGroup::Text, c);
			axis->setPalette(pal);
		}
		emit modifiedGraph();
	} else if (d_active_enrichment){
		LegendWidget *l = qobject_cast<LegendWidget *>(d_active_enrichment);
		if (l){
			l->setTextColor(c);
			l->repaint();
			emit modifiedGraph();
		}
	} else if (titlePicker->selected()){
		QwtText t = title();
		t.setColor(c);
		setTitle(t);
		emit modifiedGraph();
	} else {
		QList<QwtPlotItem *> curves = curvesList();
		foreach(QwtPlotItem *i, curves){
			if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
			  ((PlotCurve *)i)->type() != Graph::Function){
				if(((DataCurve *)i)->hasSelectedLabels()){
				   ((DataCurve *)i)->setLabelsColor(c);
				   replot();
				   emit modifiedGraph();
				   return;
				}
			}
		}
	}
}

QString Graph::axisFormula(int axis)
{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(axis);
	if (sd)
		return sd->formula();

	return QString();
}

void Graph::setAxisFormula(int axis, const QString &formula)
{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw(axis);
	if (sd)
		sd->setFormula(formula);
}

QColor Graph::frameColor()
{
	return palette().color(QPalette::Active, QPalette::Foreground);
}

void Graph::printFrame(QPainter *painter, const QRect &rect) const
{
	painter->save();

	int lw = qRound((double)painter->device()->logicalDpiX()/(double)logicalDpiX()*lineWidth());
	if (lw){
		QColor color = palette().color(QPalette::Active, QPalette::Foreground);
		painter->setPen (QPen(color, lw, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
	} else
		painter->setPen(QPen(Qt::NoPen));

	int lw2 = lw/2;
	QRect r = rect;
	if (lw % 2)
		r.adjust(lw2, lw2, -(lw2 + 1), -(lw2 + 1));
	else
		r.adjust(lw2, lw2, -lw2, -lw2);

	QwtPainter::fillRect(painter, r, paletteBackgroundColor());
	painter->drawRect(r);
	painter->restore();
}

void Graph::setCanvasBackgroundImage(const QString & fn, bool update)
{
	if (fn == d_canvas_bkg_path)
		return;

	if (fn.isEmpty()){
		d_canvas_bkg_path = QString();
		d_canvas_bkg_pix = QPixmap();
		replot();
		return;
	}

	QFileInfo fi(fn);
	if (!fi.exists ()|| !fi.isReadable())
		return;

	QList<QByteArray> lst = QImageReader::supportedImageFormats() << "JPG";
	for (int i = 0; i<(int)lst.count(); i++){
		if (fn.contains("." + lst[i])){
			d_canvas_bkg_pix.load(fn, lst[i], QPixmap::Auto);
			d_canvas_bkg_path = fn;

			if (update)
				replot();

			return;
		}
	}
}

void Graph::printCanvas(QPainter *painter, const QRect &canvasRect,
   			 const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const
{
	painter->save();

	const QwtPlotCanvas* plotCanvas = canvas();
	int lw = qRound((double)painter->device()->logicalDpiX()/(double)logicalDpiX()*plotCanvas->lineWidth());

	QRect fillRect = canvasRect.adjusted(0, 0, -1, -1);
	QwtPainter::fillRect(painter, fillRect, canvasBackground());

	painter->setClipping(true);
	painter->setClipRect(fillRect);
	drawItems(painter, fillRect, map, pfilter);
    painter->restore();

	if(lw > 0){
		painter->save();
		QColor color = plotCanvas->palette().color(QPalette::Active, QColorGroup::Foreground);
		painter->setPen (QPen(color, lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		painter->drawRect(fillRect);
		painter->restore();
	}
}

void Graph::drawItems (QPainter *painter, const QRect &rect,
			const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const
{	
	for (int i = 0; i < QwtPlot::axisCnt; i++){
		if (!axisEnabled(i) || d_is_printing)
			continue;
		drawBreak(painter, rect, map[i], i);
	}

	for (int i = 0; i < QwtPlot::axisCnt; i++){
		ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(i);
		if (!sc_engine->hasBreak())
			continue;

		QwtScaleMap m = map[i];
		int lb = m.transform(sc_engine->axisBreakLeft());
		int rb = m.transform(sc_engine->axisBreakRight());

		if (d_is_printing && painter->paintEngine()->type() != QPaintEngine::Raster){
			if (i == QwtPlot::yLeft || i == QwtPlot::yRight){
				double yfactor = (double)painter->device()->logicalDpiY()/(double)this->logicalDpiY();
				int dy = qRound(abs(lb - rb)*yfactor*0.5);
				rb -= dy;
				lb += dy;
			} else {
				double xfactor = (double)painter->device()->logicalDpiX()/(double)this->logicalDpiX();
				int dx = qRound(abs(lb - rb)*xfactor*0.5);
				lb -= dx;
				rb += dx;
			}
		}

		int start = lb, end = rb;
		if (sc_engine->testAttribute(QwtScaleEngine::Inverted)){
			end = lb;
			start = rb;
		}
		QRegion cr(rect);
		if (i == QwtPlot::xBottom || i == QwtPlot::xTop)
			painter->setClipRegion(cr.subtracted(QRegion(start, rect.y(), abs(end - start + 1), rect.height())), Qt::IntersectClip);
		else if (i == QwtPlot::yLeft || i == QwtPlot::yRight)
			painter->setClipRegion(cr.subtracted(QRegion(rect.x(), end, rect.width(), abs(end - start + 1))), Qt::IntersectClip);
	}

	painter->setRenderHint(QPainter::TextAntialiasing);

	if (!d_canvas_bkg_pix.isNull())
		painter->drawPixmap(rect, d_canvas_bkg_pix);

	QwtPlot::drawItems(painter, rect, map, pfilter);

	if (d_is_printing)
		return;

	for (int i = 0; i<QwtPlot::axisCnt; i++){
		if (!axisEnabled(i))
			continue;

		drawBreak(painter, rect, map[i], i);

		ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (i);
		int majorTicksType = sd->majorTicksStyle();
		int minorTicksType = sd->minorTicksStyle();
		bool min = (minorTicksType == ScaleDraw::In || minorTicksType == ScaleDraw::Both);
		bool maj = (majorTicksType == ScaleDraw::In || majorTicksType == ScaleDraw::Both);
		if (min || maj)
			drawInwardTicks(painter, rect, map[i], i, min, maj);
	}
}

void Graph::drawInwardTicks(QPainter *painter, const QRect &rect,
		const QwtScaleMap &map, int axis, bool min, bool maj) const
{
	const QwtScaleWidget *scale = axisWidget(axis);
	if (!scale)
		return;

	int x1 = rect.left();
	int x2 = rect.right();
	int y1 = rect.top();
	int y2 = rect.bottom();

	QPalette pal = scale->palette();
	QColor color = pal.color(QPalette::Active, QColorGroup::Foreground);

	painter->save();
	painter->setPen(QPen(color, scale->penWidth(), Qt::SolidLine));

	QwtScaleDiv *scDiv = (QwtScaleDiv *)axisScaleDiv(axis);
	const QwtValueList minTickList = scDiv->ticks(QwtScaleDiv::MinorTick);
	int minTicks = (int)minTickList.count();

	const QwtValueList medTickList = scDiv->ticks(QwtScaleDiv::MediumTick);
	int medTicks = (int)medTickList.count();

	QwtValueList majTickList = scDiv->ticks(QwtScaleDiv::MajorTick);

	ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(axis);
	if (sc_engine->hasBreak()){
		majTickList.removeAll(sc_engine->axisBreakLeft());
		majTickList.removeAll(sc_engine->axisBreakRight());
	}
	int majTicks = (int)majTickList.count();

	int j, x, y, low,high;
	int clw = canvas()->lineWidth();
	switch (axis)
	{
		case QwtPlot::yLeft:
			x = x1;
			low = y1 + d_maj_tick_length;
			high = y2 - d_maj_tick_length;
			if (min && d_min_tick_length){
				for (j = 0; j < minTicks; j++){
					y = map.transform(minTickList[j]);
					if (y > low && y < high)
						QwtPainter::drawLine(painter, x, y, x + d_min_tick_length, y);
				}
				for (j = 0; j < medTicks; j++){
					y = map.transform(medTickList[j]);
					if (y > low && y < high)
						QwtPainter::drawLine(painter, x, y, x + d_min_tick_length, y);
				}
			}

			if (maj && d_maj_tick_length){
				for (j = 0; j < majTicks; j++){
					y = map.transform(majTickList[j]);
					if ((y > low && y < high) ||
						(y > high && !axisEnabled (QwtPlot::xBottom) && !clw) ||
						(y < low && !axisEnabled(QwtPlot::xTop) && !clw))
						QwtPainter::drawLine(painter, x, y, x + d_maj_tick_length, y);
				}
			}
			break;

		case QwtPlot::yRight:
			{
				x = x2;
				low = y1 + d_maj_tick_length;
				high = y2 - d_maj_tick_length;
				if (min && d_min_tick_length){
					for (j = 0; j < minTicks; j++){
						y = map.transform(minTickList[j]);
						if (y > low && y < high)
							QwtPainter::drawLine(painter, x, y, x - d_min_tick_length, y);
					}
					for (j = 0; j < medTicks; j++){
						y = map.transform(medTickList[j]);
						if (y > low && y < high)
							QwtPainter::drawLine(painter, x, y, x - d_min_tick_length, y);
					}
				}

				if (maj && d_maj_tick_length){
					for (j = 0; j <majTicks; j++){
						y = map.transform(majTickList[j]);
						if ((y > low && y < high) ||
						(y > high && !axisEnabled (QwtPlot::xBottom) && !clw) ||
						(y < low && !axisEnabled(QwtPlot::xTop) && !clw))
							QwtPainter::drawLine(painter, x, y, x - d_maj_tick_length, y);
					}
				}
			}
			break;

		case QwtPlot::xBottom:
			y = y2;
			low = x1 + d_maj_tick_length;
			high = x2 - d_maj_tick_length;
			if (min && d_min_tick_length){
				for (j = 0; j < minTicks; j++){
					x = map.transform(minTickList[j]);
					if (x > low && x < high)
						QwtPainter::drawLine(painter, x, y, x, y - d_min_tick_length);
				}
				for (j = 0; j < medTicks; j++){
					x = map.transform(medTickList[j]);
					if (x > low && x < high)
						QwtPainter::drawLine(painter, x, y, x, y - d_min_tick_length);
				}
			}

			if (maj && d_maj_tick_length){
				for (j = 0; j < majTicks; j++){
					x = map.transform(majTickList[j]);
					if ((x > low && x < high) ||
						(x > high && !axisEnabled(QwtPlot::yRight) && !clw) ||
						(x < low && !axisEnabled(QwtPlot::yLeft) && !clw))
						QwtPainter::drawLine(painter, x, y, x, y - d_maj_tick_length);
				}
			}
			break;

		case QwtPlot::xTop:
			y = y1;
			low = x1 + d_maj_tick_length;
			high = x2 - d_maj_tick_length;

			if (min && d_min_tick_length){
				for (j = 0; j < minTicks; j++){
					x = map.transform(minTickList[j]);
					if (x > low && x < high)
						QwtPainter::drawLine(painter, x, y, x, y + d_min_tick_length);
				}
				for (j = 0; j < medTicks; j++){
					x = map.transform(medTickList[j]);
					if (x > low && x < high)
						QwtPainter::drawLine(painter, x, y, x, y + d_min_tick_length);
				}
			}

			if (maj && d_maj_tick_length){
				for (j = 0; j < majTicks; j++){
					x = map.transform(majTickList[j]);
					if ((x > low && x < high) ||
						(x > high && !axisEnabled(QwtPlot::yRight) && !clw) ||
						(x < low && !axisEnabled(QwtPlot::yLeft) && !clw))
						QwtPainter::drawLine(painter, x, y, x, y + d_maj_tick_length);
				}
			}
			break;
	}
	painter->restore();
}

void Graph::drawBreak(QPainter *painter, const QRect &rect, const QwtScaleMap &map, int axis) const
{
    ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(axis);
    if (!sc_engine->hasBreak() || !sc_engine->hasBreakDecoration())
        return;

    painter->save();
	painter->setRenderHint(QPainter::Antialiasing);

	QColor color = axisWidget(axis)->palette().color(QPalette::Active, QColorGroup::Foreground);
	painter->setPen(QPen(color, axesLinewidth(), Qt::SolidLine));

	int left = map.transform(sc_engine->axisBreakLeft());
	int right = map.transform(sc_engine->axisBreakRight());
	int x, y;
	int len = d_maj_tick_length;
    switch (axis){
        case QwtPlot::yLeft:
			x = rect.left() - 1;
            QwtPainter::drawLine(painter, x, left, x + len, left - len);
            QwtPainter::drawLine(painter, x, right, x + len, right - len);
        break;

        case QwtPlot::yRight:
			x = rect.right() + 1;
            QwtPainter::drawLine(painter, x - len, left + len, x, left);
            QwtPainter::drawLine(painter, x - len, right + len, x, right);
        break;

        case QwtPlot::xBottom:
			y = rect.bottom() + 1;
			QwtPainter::drawLine(painter, left, y, left + len, y - len);
            QwtPainter::drawLine(painter, right, y, right + len, y - len);
        break;

        case QwtPlot::xTop:
			y = rect.top() - 1;
            QwtPainter::drawLine(painter, left - len, y + len, left, y);
            QwtPainter::drawLine(painter, right - len, y + len, right, y);
        break;
    }
	painter->restore();
}

int Graph::axesLinewidth() const
{
	for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ ) {
		const QwtScaleWidget *scale = this->axisWidget(axis);
		if (scale)
			return scale->penWidth();
	}
	return 0;
}

int Graph::minorTickLength() const
{
	return d_min_tick_length;
}

int Graph::majorTickLength() const
{
	return d_maj_tick_length;
}

void Graph::setTickLength (int minLength, int majLength)
{
	if (d_maj_tick_length == majLength && d_min_tick_length == minLength)
		return;

	d_maj_tick_length = majLength;
	d_min_tick_length = minLength;
}

QwtPlotItem* Graph::closestCurve(int xpos, int ypos, int &dist, int &point)
{
	double dmin = DBL_MAX;
	QPoint p = QPoint(xpos, ypos);
	QwtPlotItem *curve = NULL;
	foreach (QwtPlotItem *item, d_curves){
		if(item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)item;
		if (c->type() == Graph::ErrorBars)
			continue;

		if (c->type() != Graph::Function){
			DataCurve *dc = (DataCurve *)c;
			if (dc->hasLabels() && dc->selectedLabels(p)){
				dist = 0;
				return item;
			}
		}

		double d = 0;
		int pnt = c->closestPoint(p, &d);
		if (pnt >= 0 && d < dmin){
			dmin = d;
			curve = c;
			point = pnt;
		}
	}

	if (curve){
		if (dmin <= 10){
			dist = qRound(dmin);
			return curve;
		}
	}

	QwtScaleMap map[QwtPlot::axisCnt];
	for (int axis = 0; axis < QwtPlot::axisCnt; axis++)
		map[axis] = canvasMap(axis);

	foreach (QwtPlotItem *item, d_curves){
		if(item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		Spectrogram *c = (Spectrogram *)item;
		if (c->selectedLabels(p)){
			dist = 0;
			return item;
		} else {
			c->selectLabel(false);
			if (c->transform(map[c->xAxis()], map[c->yAxis()], c->boundingRect()).contains(p)){
				dist = 0;
				return item;
			}
		}
	}
	return NULL;
}

void Graph::insertMarker(QwtPlotMarker *m)
{
	m->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
	m->attach(((QwtPlot *)this));
}

void Graph::insertCurve(QwtPlotItem *c)
{
	if (!c)
		return;

	if (!d_curves.contains(c))
		d_curves.append(c);

	c->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
	c->attach(this);
}

QList<int> Graph::getMajorTicksType()
{
	QList<int> majorTicksType;
	for (int axis=0; axis<QwtPlot::axisCnt; axis++){
		if (axisEnabled(axis)){
			ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (axis);
			majorTicksType << sd->majorTicksStyle();
		} else
			majorTicksType << ScaleDraw::Out;
	}
	return majorTicksType;
}

void Graph::setMajorTicksType(int axis, int type)
{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
	if (sd)
		sd->setMajorTicksStyle ((ScaleDraw::TicksStyle)type);
}

QList<int> Graph::getMinorTicksType()
{
	QList<int> minorTicksType;
	for (int axis=0; axis<QwtPlot::axisCnt; axis++)
	{
		if (axisEnabled(axis))
		{
			ScaleDraw *sd = (ScaleDraw *) axisScaleDraw (axis);
			minorTicksType << sd->minorTicksStyle();
		}
		else
			minorTicksType << ScaleDraw::Out;
	}
	return minorTicksType;
}

void Graph::setMinorTicksType(int axis, int type)
{
	ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
	if (sd)
		sd->setMinorTicksStyle((ScaleDraw::TicksStyle)type);
}

int Graph::axisLabelFormat(int axis)
{
	if (axisValid(axis)){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
		return sd->labelNumericFormat();
	}
	return 0;
}

int Graph::axisLabelPrecision(int axis)
{
	if (axisValid(axis))
	{
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
		return sd->labelNumericPrecision();
	}

	//for a bad call we return the default values
	return 4;
}

/*!
  \return the number format for the major scale labels of a specified axis
  \param axis axis index
  \retval f format character
  \retval prec precision
  */
void Graph::axisLabelFormat(int axis, char &f, int &prec) const
{
	if (axisValid(axis)){
		ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (axis);
		sd->labelFormat(f, prec);
	} else {//for a bad call we return the default values
		f = 'g';
		prec = 4;
	}
}

/*!
  \brief Adjust plot content to its current size.
  Must be reimplemented because the base implementation adds a mask causing an ugly drawing artefact.
*/
void Graph::updateLayout()
{
	plotLayout()->activate(this, contentsRect());
	// resize and show the visible widgets

	if (!titleLabel()->text().isEmpty()){
		titleLabel()->setGeometry(plotLayout()->titleRect());
		if (!titleLabel()->isVisible())
			titleLabel()->show();
	} else
		titleLabel()->hide();

	for (int axisId = 0; axisId < axisCnt; axisId++){
		if (axisEnabled(axisId) ){
			axisWidget(axisId)->setGeometry(plotLayout()->scaleRect(axisId));
			if (!axisWidget(axisId)->isVisible())
				axisWidget(axisId)->show();
		} else
			axisWidget(axisId)->hide();
	}

	canvas()->setUpdatesEnabled(false);
	canvas()->setGeometry(plotLayout()->canvasRect());
	canvas()->setUpdatesEnabled(true);

	updatedLayout(this);
}

/*!
  \brief Adjust plot content to its canvas size.
  \param cr The new geometry of the canvas.
*/
void Graph::setCanvasGeometry(const QRect &cr)
{
	bool scaleFonts = autoScaleFonts;
	autoScaleFonts = false;

	QRect ocr = plotLayout()->canvasRect().translated(pos());//old canvas geometry

	QRect rect = geometry();
	rect.adjust(cr.x() - ocr.x(), cr.y() - ocr.y(), cr.right() - ocr.right(), cr.bottom() - ocr.bottom());
	setGeometry(rect);

	updateMarkersBoundingRect(false);
	autoScaleFonts = scaleFonts;
}

/*!
  \brief Adjust plot content to its canvas size.
  \param cr The new size of the canvas.
*/
void Graph::setCanvasSize(const QSize &size)
{
	bool scaleFonts = autoScaleFonts;
	autoScaleFonts = false;

	QRect ocr = plotLayout()->canvasRect().translated(pos());//old canvas geometry

	QRect rect = geometry();
	rect.adjust(0, 0, size.width() - ocr.width(), size.height() - ocr.height());
	setGeometry(rect);

	updateMarkersBoundingRect(false);
	autoScaleFonts = scaleFonts;
}

const QColor & Graph::paletteBackgroundColor() const
{
	return	palette().color(QPalette::Window);
}

void Graph::updateCurveLabels()
{
    QList<QwtPlotItem *> curves = curvesList();
    foreach(QwtPlotItem *i, curves){
        if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
        ((PlotCurve *)i)->type() != Graph::Function &&
        ((DataCurve *)i)->hasLabels())
            ((DataCurve *)i)->updateLabelsPosition();
    }
}

void Graph::showEvent (QShowEvent * event)
{
    event->accept();
    updateCurveLabels();
}

/*!
  \brief Paint the plot into a given rectangle.
  Paint the contents of a QwtPlot instance into a given rectangle (Qwt modified code).

  \param painter Painter
  \param plotRect Bounding rectangle
  \param pfilter Print filter
*/
void Graph::print(QPainter *painter, const QRect &plotRect, const QwtPlotPrintFilter &pfilter)
{
	if (painter == 0 || !painter->isActive() || !plotRect.isValid() || size().isNull())
		return;

	deselect();

	d_is_printing = true;

	printFrame(painter, plotRect);

	painter->save();

	// All paint operations need to be scaled according to
	// the paint device metrics.

	QwtPainter::setMetricsMap(this, painter->device());
	const QwtMetricsMap &metricsMap = QwtPainter::metricsMap();

	// It is almost impossible to integrate into the Qt layout
	// framework, when using different fonts for printing
	// and screen. To avoid writing different and Qt unconform
	// layout engines we change the widget attributes, print and
	// reset the widget attributes again. This way we produce a lot of
	// useless layout events ...

	pfilter.apply((QwtPlot *)this);

	int axisId;
	int baseLineDists[QwtPlot::axisCnt];
	if (pfilter.options() & QwtPlotPrintFilter::PrintFrameWithScales){
		// In case of no background we set the backbone of
		// the scale on the frame of the canvas.

		for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ ){
			QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
			if ( scaleWidget ){
				baseLineDists[axisId] = scaleWidget->margin();
				scaleWidget->setMargin(0);
			}
		}
	}

	// Calculate the layout for the print.

	int layoutOptions = QwtPlotLayout::IgnoreScrollbars;
	if ( !(pfilter.options() & QwtPlotPrintFilter::PrintMargin) )
		layoutOptions |= QwtPlotLayout::IgnoreMargin;
	if ( !(pfilter.options() & QwtPlotPrintFilter::PrintLegend) )
		layoutOptions |= QwtPlotLayout::IgnoreLegend;

	int bw = lineWidth();
	plotLayout()->activate(this,
		QwtPainter::metricsMap().deviceToLayout(plotRect.adjusted(bw, bw, -bw, -bw)), layoutOptions);

	QRect canvasRect = plotLayout()->canvasRect();

	// The border of the bounding rect needs to be scaled to
	// layout coordinates, so that it is aligned to the axes

	QRect boundingRect( canvasRect.left() - 1, canvasRect.top() - 1,
		canvasRect.width() + 2, canvasRect.height() + 2);
	boundingRect = metricsMap.layoutToDevice(boundingRect);
	boundingRect.setWidth(boundingRect.width() - 1);
	boundingRect.setHeight(boundingRect.height() - 1);

	canvasRect = metricsMap.layoutToDevice(canvasRect);

	// When using QwtPainter all sizes where computed in pixel
	// coordinates and scaled by QwtPainter later. This limits
	// the precision to screen resolution. A much better solution
	// is to scale the maps and print in unlimited resolution.

	QwtScaleMap map[axisCnt];
	for (axisId = 0; axisId < axisCnt; axisId++){
		map[axisId].setTransformation(axisScaleEngine(axisId)->transformation());

		const QwtScaleDiv &scaleDiv = *axisScaleDiv(axisId);
		map[axisId].setScaleInterval(scaleDiv.lowerBound(), scaleDiv.upperBound());

		double from, to;
		if ( axisEnabled(axisId) ){
			const int sDist = axisWidget(axisId)->startBorderDist();
			const int eDist = axisWidget(axisId)->endBorderDist();
			const QRect &scaleRect = plotLayout()->scaleRect(axisId);

			if ( axisId == xTop || axisId == xBottom ){
				from = metricsMap.layoutToDeviceX(scaleRect.left() + sDist);
				to = metricsMap.layoutToDeviceX(scaleRect.right() + 1 - eDist);
			} else {
				from = metricsMap.layoutToDeviceY(scaleRect.bottom() + 1 - eDist );
				to = metricsMap.layoutToDeviceY(scaleRect.top() + sDist);
			}
		} else {
			int margin = plotLayout()->canvasMargin(axisId);
			if ( axisId == yLeft || axisId == yRight ){
				margin = metricsMap.layoutToDeviceY(margin);
				from = canvasRect.bottom() - margin;
				to = canvasRect.top() + margin;
			} else {
				margin = metricsMap.layoutToDeviceX(margin);
				from = canvasRect.left() + margin;
				to = canvasRect.right() - margin;
			}
		}
		map[axisId].setPaintXInterval(from, to);
	}

	if (!metricsMap.isIdentity()){//we set non-cosmetic pens in order to scale pen width
		foreach (QwtPlotItem *item, d_curves){
			if(item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
				Spectrogram *sp = (Spectrogram *)item;
				QPen pen = sp->defaultContourPen();
				pen.setCosmetic(false);
				sp->setDefaultContourPen(pen);
			} else {
				PlotCurve *c = (PlotCurve *)item;
				QPen pen = c->pen();
				pen.setCosmetic(false);
				c->setPen(pen);
				if (c->type() == Graph::VectXYXY || c->type() == Graph::VectXYAM){
					VectorCurve *v = (VectorCurve *)item;
					pen = v->vectorPen();
					pen.setCosmetic(false);
					v->setVectorPen(pen);
				}
				QwtSymbol symbol = c->symbol();
				pen = symbol.pen();
				if (pen.style() != Qt::NoPen){
					pen.setCosmetic(false);
					symbol.setPen(pen);
					c->setSymbol(symbol);
				}
			}
		}
	}

	// The canvas maps are already scaled.
	QwtPainter::setMetricsMap(painter->device(), painter->device());

	double fontFactor = ((ScaledFontsPrintFilter *)(&pfilter))->scaleFontsFactor();
	QList<FrameWidget*> enrichments = stackingOrderEnrichmentsList();
	foreach(FrameWidget *f, enrichments){
		if (!f->isVisible() || f->isOnTop())
			continue;

		QFont fnt;
		LegendWidget *lw = qobject_cast<LegendWidget *>(f);
		if (lw){
			fnt = lw->font();
			QFont font(fnt);
			font.setPointSizeF(fontFactor*font.pointSizeF());
			lw->setFont(font);
		}

		f->print(painter, map);

		if (lw)//restore original font
			lw->setFont(fnt);
	}
	printCanvas(painter, canvasRect, map, pfilter);
	QwtPainter::resetMetricsMap();

	foreach (QwtPlotItem *item, d_curves){
		if(item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
			Spectrogram *sp = (Spectrogram *)item;
			QPen pen = sp->defaultContourPen();
			pen.setCosmetic(true);
			sp->setDefaultContourPen(pen);
		} else {
			PlotCurve *c = (PlotCurve *)item;
			QPen pen = c->pen();
			pen.setCosmetic(true);
			c->setPen(pen);
			if (c->type() == Graph::VectXYXY || c->type() == Graph::VectXYAM){
				VectorCurve *v = (VectorCurve *)item;
				pen = v->vectorPen();
				pen.setCosmetic(true);
				v->setVectorPen(pen);
			}

			QwtSymbol symbol = c->symbol();
			pen = symbol.pen();
			if (pen.style() != Qt::NoPen){
				pen.setCosmetic(true);
				symbol.setPen(pen);
				c->setSymbol(symbol);
			}
		}
	}

	QwtPainter::setMetricsMap(this, painter->device());

	if ((pfilter.options() & QwtPlotPrintFilter::PrintTitle) && (!titleLabel()->text().isEmpty())){
		QwtTextLabel *title = titleLabel();
		QString old_title = title->text().text();
#ifdef TEX_OUTPUT
		if (d_is_exporting_tex){
			QString s = old_title;
			if (d_tex_escape_strings)
				s = escapeTeXSpecialCharacters(s);
			s = texSuperscripts(s);
			title->setText(s);

			int flags = title->text().renderFlags();
			if (flags & Qt::AlignLeft)
				((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignLeft);
			else if (flags & Qt::AlignRight)
				((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignRight);
		}
#endif

		printTitle(painter, plotLayout()->titleRect());

#ifdef TEX_OUTPUT
		if (d_is_exporting_tex){
			title->setText(old_title);
			((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignHCenter);
		}
#endif
	}

	canvasRect = plotLayout()->canvasRect();

	for ( axisId = 0; axisId < QwtPlot::axisCnt; axisId++ ){
		QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
		if (scaleWidget){
			int baseDist = scaleWidget->margin();

			int startDist, endDist;
			scaleWidget->getBorderDistHint(startDist, endDist);

			QRect scaleRect = plotLayout()->scaleRect(axisId);
			if (!scaleWidget->margin()){
				switch(axisId){
					case xBottom:
						scaleRect.setTop(canvasRect.bottom());
					break;
					case xTop:
						scaleRect.setBottom(canvasRect.top());
					break;
					case yLeft:
						scaleRect.setRight(canvasRect.left());
					break;
					case yRight:
						scaleRect.setLeft(canvasRect.right());
					break;
				}
			}
			printScale(painter, axisId, startDist, endDist, baseDist, scaleRect);
		}
	}
	QwtPainter::resetMetricsMap();

	plotLayout()->invalidate();

	// reset all widgets with their original attributes.
	if ( pfilter.options() & QwtPlotPrintFilter::PrintFrameWithScales ){
		// restore the previous base line dists
		for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ ){
			QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
			if ( scaleWidget  )
				scaleWidget->setMargin(baseLineDists[axisId]);
		}
	}

	QwtPainter::setMetricsMap(painter->device(), painter->device());
	foreach(FrameWidget *f, enrichments){
		if (!f->isVisible() || !f->isOnTop())
			continue;
		QFont fnt;
		LegendWidget *lw = qobject_cast<LegendWidget *>(f);
		if (lw){
			fnt = lw->font();
			QFont font(fnt);
			font.setPointSizeF(fontFactor*font.pointSizeF());
			lw->setFont(font);
		}

		f->print(painter, map);

		if (lw)//restore original font
			lw->setFont(fnt);
	}
	QwtPainter::resetMetricsMap();

	pfilter.reset((QwtPlot *)this);

	plotLayout()->activate(this, contentsRect());

	painter->restore();

	d_is_printing = false;
}

TexWidget* Graph::addTexFormula(const QString& s, const QPixmap& pix)
{
	TexWidget *t = new TexWidget(this, s, pix);
	d_enrichments << t;
	emit modifiedGraph();
	return t;
}

FrameWidget* Graph::add(FrameWidget* fw, bool copy)
{
	if (!fw)
		return NULL;

	if (!copy){
		d_enrichments << fw;
		d_active_enrichment = fw;
		return fw;
	}

	FrameWidget *aux = NULL;
	LegendWidget *l = qobject_cast<LegendWidget *>(fw);
	if (l){
		aux = new LegendWidget(this);
		((LegendWidget *)aux)->clone(l);
	}

	TexWidget *t = qobject_cast<TexWidget *>(fw);
	if (t){
		aux = new TexWidget(this);
		((TexWidget *)aux)->clone(t);
	}

	ImageWidget *i = qobject_cast<ImageWidget *>(fw);
	if (i){
		aux = new ImageWidget(this);
		((ImageWidget *)aux)->clone(i);
	}

	RectangleWidget *r = qobject_cast<RectangleWidget *>(fw);
	if (r){
		aux = new RectangleWidget(this);
		((RectangleWidget *)aux)->clone(r);
	}

	EllipseWidget *e = qobject_cast<EllipseWidget *>(fw);
	if (e){
		aux = new EllipseWidget(this);
		((EllipseWidget *)aux)->clone(e);
	}

	aux->setAttachPolicy(fw->attachPolicy());
	aux->setOnTop(fw->isOnTop());
	aux->setVisible(fw->isVisible());

	d_enrichments << aux;
	d_active_enrichment = aux;
	return aux;
}

LegendWidget* Graph::activeText()
{
	return qobject_cast<LegendWidget *>(d_active_enrichment);
}

void Graph::raiseEnrichements()
{
	QList<Graph *> lst = multiLayer()->layersList();
	foreach(Graph *g, lst){
		if (g == this)
			continue;

		QList<FrameWidget *> eLst = g->enrichmentsList();
		foreach(FrameWidget *fw, eLst){
			if (fw->isOnTop())
				fw->raise();
			else
				fw->lower();
		}
	}

	foreach(FrameWidget *fw, d_enrichments){
		if (fw->isOnTop())
			fw->raise();
		else
			fw->lower();
	}
}

QRect Graph::boundingRect()
{
	QRect r = rect();

	foreach(FrameWidget *fw, d_enrichments){
		if (fw->isHidden() || fw->visibleRegion().isEmpty())//pie labels can be hidden
			continue;

		r = r.united(fw->geometry());
	}
	return r;
}

QSize Graph::customPrintSize(const QSizeF& customSize, int unit, int dpi)
{
	if (!customSize.isValid())
		return QSize();

	QSize size = QSize();
	switch(unit){
		case FrameWidget::Pixel:
			size = customSize.toSize();
		break;
		case FrameWidget::Inch:
			size = QSize(qRound(customSize.width()*dpi), qRound(customSize.height()*dpi));
		break;
		case FrameWidget::Millimeter:
			size = QSize(qRound(customSize.width()*dpi/25.4), qRound(customSize.height()*dpi/25.4));
		break;
		case FrameWidget::Centimeter:
			size = QSize(qRound(customSize.width()*dpi/2.54), qRound(customSize.height()*dpi/2.54));
		break;
		case FrameWidget::Point:
			size = QSize(qRound(customSize.width()*dpi/72.0), qRound(customSize.height()*dpi/72.0));
		break;
	}
	return size;
}

bool Graph::rangeSelectorsEnabled()
{
	if (d_range_selector && d_range_selector->isVisible())
		return true;
	return false;
}

/*!
  \brief Paint a scale into a given rectangle.
  Paint the scale into a given rectangle (modified code from Qwt).

  \param painter Painter
  \param axisId Axis
  \param startDist Start border distance
  \param endDist End border distance
  \param baseDist Base distance
  \param rect Bounding rectangle
*/

void Graph::printScale(QPainter *painter,
    int axisId, int startDist, int endDist, int baseDist,
    const QRect &rect) const
{
    if (!axisEnabled(axisId))
        return;

    QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
    if ( scaleWidget->isColorBarEnabled()
        && scaleWidget->colorBarWidth() > 0)
    {
        const QwtMetricsMap map = QwtPainter::metricsMap();

        QRect r = map.layoutToScreen(rect);
        r.setWidth(r.width() - 1);
        r.setHeight(r.height() - 1);

        scaleWidget->drawColorBar(painter, scaleWidget->colorBarRect(r));

        const int off = scaleWidget->colorBarWidth() + scaleWidget->spacing();
        if ( scaleWidget->scaleDraw()->orientation() == Qt::Horizontal )
            baseDist += map.screenToLayoutY(off);
        else
            baseDist += map.screenToLayoutX(off);
    }

    QwtScaleDraw::Alignment align;
    int x, y, w;

    switch(axisId)
    {
        case yLeft:
        {
            x = rect.right() - baseDist;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::LeftScale;
            break;
        }
        case yRight:
        {
			x = rect.left() + baseDist;
            y = rect.y() + startDist;
            w = rect.height() - startDist - endDist;
            align = QwtScaleDraw::RightScale;
            break;
        }
        case xTop:
        {
            x = rect.left() + startDist;
            y = rect.bottom() - baseDist;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::TopScale;
            break;
        }
        case xBottom:
        {
            x = rect.left() + startDist;
            y = rect.top() + baseDist;
            w = rect.width() - startDist - endDist;
            align = QwtScaleDraw::BottomScale;
            break;
        }
        default:
            return;
    }

	QwtText title = scaleWidget->title();
	QString old_title = title.text();
#ifdef TEX_OUTPUT
	if (d_is_exporting_tex){
		QString s = old_title;
		if (d_tex_escape_strings)
			s = escapeTeXSpecialCharacters(s);
		s = texSuperscripts(s);
		title.setText(s);

		int flags = title.renderFlags();
		if (flags & Qt::AlignLeft)
			((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignLeft);
		else if (flags & Qt::AlignRight)
			((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignRight);

		scaleWidget->setTitle(title);
	}
#endif

    scaleWidget->drawTitle(painter, align, rect);

#ifdef TEX_OUTPUT
	if (d_is_exporting_tex){
		title.setText(old_title);
		scaleWidget->setTitle(title);
		((QTeXPaintDevice *)painter->device())->setTextHorizontalAlignment(Qt::AlignHCenter);
	}
#endif

    painter->save();
    painter->setFont(scaleWidget->font());

    QPen pen = painter->pen();
	int lw = scaleWidget->penWidth();
	if (!lw)
		lw = 1;
	pen.setWidthF(lw*(double)painter->device()->logicalDpiX()/(double)logicalDpiX());
	painter->setPen(pen);

    QwtScaleDraw *sd = (QwtScaleDraw *)scaleWidget->scaleDraw();
    const QPoint sdPos = sd->pos();
    const int sdLength = sd->length();

    sd->move(x, y);
    sd->setLength(w);

    QPalette palette = scaleWidget->palette();
    palette.setCurrentColorGroup(QPalette::Active);

	sd->draw(painter, palette);

    // reset previous values
    sd->move(sdPos);
    sd->setLength(sdLength);

    painter->restore();
}

#ifdef TEX_OUTPUT
QString Graph::escapeTeXSpecialCharacters(const QString &s)
{
	QString text = s;
	text.replace("$", "\\$");
	text.replace("_", "\\_");
	text.replace("{", "\\{");
	text.replace("}", "\\}");
	text.replace("^", "\\^");
	text.replace("&", "\\&");
	text.replace("%", "\\%");
	text.replace("#", "\\#");
	return text;
}

QString Graph::texSuperscripts(const QString &text)
{
	QString s = text;
	s.replace("<sub>", "$_{");
	s.replace("<sup>", "$^{");
	s.replace("</sub>", "}$");
	s.replace("</sup>", "}$");
	return s;
}
#endif

void Graph::changeCurveIndex(int fromIndex, int toIndex)
{
	if (d_curves.isEmpty())
		return;

	if (fromIndex < 0 || fromIndex >= d_curves.size() ||
		toIndex < 0 || toIndex >= d_curves.size()) return;

	d_curves.move ( fromIndex, toIndex );

	int z = 0;
	foreach(QwtPlotItem *c, d_curves){
		c->setZ(z);
		z++;
	}
	replot();
	modifiedGraph();
}

void Graph::dragEnterEvent( QDragEnterEvent* e )
{
	Graph *g = qobject_cast<Graph*>(e->source());
	if (g && g->multiLayer() == this->multiLayer())
		return;

	if (e->mimeData()->hasFormat("text/plain"))
		e->acceptProposedAction();
}

void Graph::dropEvent(QDropEvent* event)
{
	MultiLayer *ml = multiLayer();
	if (!ml)
		return;

	ApplicationWindow *app = ml->applicationWindow();
	Table *t = qobject_cast<Table*>(event->source());
	if (t){
		QStringList columns = event->mimeData()->text().split("\n");
		if (columns.isEmpty())
			return;

		addCurves(t, columns, app->defaultCurveStyle, app->defaultCurveLineWidth, app->defaultSymbolSize);
		return;
	}

	Matrix *m = qobject_cast<Matrix*>(event->source());
	if (m){
		plotSpectrogram(m, ColorMap);
		return;
	}

	Graph *g = qobject_cast<Graph*>(event->source());
	if (!g || g->multiLayer() == this->multiLayer())
		return;

	QStringList lst = event->mimeData()->text().split(";");

	QPoint pos = multiLayer()->canvas()->mapFromGlobal(QCursor::pos());
	pos = QPoint(pos.x() - lst[0].toInt(), pos.y() - lst[1].toInt());
	Graph *clone = multiLayer()->addLayer(pos.x(), pos.y(), g->width(), g->height());
	if (clone)
		clone->copy(g);
}

void Graph::enableDouglasPeukerSpeedMode(double tolerance, int maxPoints)
{
	if (d_speed_mode_points == maxPoints &&
		d_Douglas_Peuker_tolerance == tolerance)
		return;

	d_speed_mode_points = maxPoints;
	d_Douglas_Peuker_tolerance = tolerance;

	foreach (QwtPlotItem *item, d_curves){
		if(item->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		PlotCurve *c = (PlotCurve *)item;
		if (!c || c->type() == Function)
			continue;

		if (tolerance == 0.0 || c->dataSize() < d_speed_mode_points){
			c->setCurveAttribute(QwtPlotCurve::Fitted, false);
			continue;
		}

		c->setCurveAttribute(QwtPlotCurve::Fitted);

		QwtWeedingCurveFitter *fitter = new QwtWeedingCurveFitter(tolerance);
		c->setCurveFitter(fitter);
	}
	replot();
}

QList<FrameWidget*> Graph::stackingOrderEnrichmentsList() const
{
	MultiLayer *ml = multiLayer();
	if (!ml)
		return d_enrichments;

	QList<FrameWidget*> enrichements;
	QObjectList lst = ml->canvas()->children();
	foreach(QObject *o, lst){
		FrameWidget *fw = qobject_cast<FrameWidget *>(o);
		if (fw && fw->plot() == this)
			enrichements << fw;
	}
	return enrichements;
}

bool Graph::mousePressed(QEvent *e)
{
	const QMouseEvent *me = (const QMouseEvent *)e;

	QList<FrameWidget*> lst = stackingOrderEnrichmentsList();
	foreach(FrameWidget *o, lst){
		QPoint pos = o->mapFromGlobal(me->globalPos());
		if (o->rect().contains(pos))
			return QCoreApplication::sendEvent(o, e);
	}

	QPoint pos = mapFromGlobal(me->globalPos());
	if (plotLayout()->titleRect().contains(pos))
		return QCoreApplication::sendEvent(titleLabel(), e);

	for (int i = 0; i < QwtPlot::axisCnt; i++ ){
		QwtScaleWidget *sw = axisWidget(i);
		if (!sw)
			continue;

		QRect r = plotLayout()->scaleRect(i);
		if (r.contains(pos)){
			if (scalePicker->scaleRect(sw).translated(r.topLeft()).contains(pos))
				scalePicker->selectLabels(sw);
			else
				scalePicker->selectTitle(sw);
			return true;
		}
	}

	return false;
}

void Graph::setWaterfallXOffset(int offset)
{
	if (offset == d_waterfall_offset_x)
		return;

	d_waterfall_offset_x = offset;
	updateDataCurves();
	emit modifiedGraph();
}

void Graph::setWaterfallYOffset(int offset)
{
	if (offset == d_waterfall_offset_y)
		return;

	d_waterfall_offset_y = offset;
	updateDataCurves();
	emit modifiedGraph();
}

void Graph::setWaterfallOffset(int x, int y, bool update)
{
	d_waterfall_offset_x = x;
	d_waterfall_offset_y = y;

	if (update){
		updateDataCurves();
		emit modifiedGraph();
	}
}

void Graph::updateWaterfallFill(bool on)
{
	int n = d_curves.size();
	if (!n)
		return;

	for (int i = 0; i < n; i++){
		PlotCurve *cv = (PlotCurve *)curve(i);
		if (!cv)
			continue;

		if (on && multiLayer())
			cv->setBrush(QBrush(multiLayer()->waterfallFillColor()));
		else
			cv->setBrush(QBrush());
	}
	replot();
	emit modifiedGraph();
}

void Graph::setWaterfallSideLines(bool on)
{
	int n = d_curves.size();
	if (!n)
		return;

	if (curve(0)->sideLinesEnabled() == on)
		return;

	for (int i = 0; i < n; i++){
		PlotCurve *cv = (PlotCurve *)curve(i);
		if (cv)
			cv->enableSideLines(on);
	}
	replot();
	emit modifiedGraph();
}

void Graph::setWaterfallFillColor(const QColor& c)
{
	int n = d_curves.size();
	if (!n)
		return;

	for (int i = 0; i < n; i++){
		PlotCurve *cv = (PlotCurve *)curve(i);
		if (cv)
			cv->setBrush(QBrush(c));
	}
	replot();
	emit modifiedGraph();
}

void Graph::reverseCurveOrder()
{
	if (d_curves.isEmpty())
		return;

	QList<QwtPlotItem *> lst;
	int n = d_curves.size();
	for (int i = 0; i < n; i++)
		lst << d_curves[n - i - 1];

	setCurvesList(lst);
	emit modifiedGraph();
}

void Graph::updateDataCurves()
{
	int n = d_curves.size();
	if (!n)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (int i = 0; i < n; i++){
		DataCurve *c = dataCurve(i);
		if (c)
			c->loadData();
	}
	replot();
	QApplication::restoreOverrideCursor();
}

void Graph::setGridOnTop(bool on, bool update)
{
	if (d_grid_on_top == on)
		return;

	d_grid_on_top = on;

	on ? d_grid->setZ(INT_MAX) : d_grid->setZ(INT_MIN);

	if (update){
		replot();
		modifiedGraph();
	}
}

void Graph::showMissingDataGap(bool on, bool update)
{
	if (d_missing_data_gap == on)
		return;

	d_missing_data_gap = on;

	if (update){
		replot();
		modifiedGraph();
	}
}
