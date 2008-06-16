/***************************************************************************
    File                 : Graph.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2008 by Ion Vasilief
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

static const char *cut_xpm[]={
"18 18 3 1",
". c None",
"# c #000000",
"a c #00007f",
"..................",
"..................",
"..................",
".......#...#......",
".......#...#......",
".......#...#......",
".......##.##......",
"........#.#.......",
"........###.......",
".........#........",
"........a#a.......",
"........a.aaa.....",
"......aaa.a..a....",
".....a..a.a..a....",
".....a..a.a..a....",
".....a..a..aa.....",
"......aa..........",
".................."};

static const char *copy_xpm[]={
"15 13 4 1",
"# c None",
". c #000000",
"b c #00007f",
"a c #ffffff",
"......#########",
".aaaa..########",
".aaaa.a.#######",
".a..a.bbbbbb###",
".aaaaabaaaabb##",
".a....baaaabab#",
".aaaaaba..abbbb",
".a....baaaaaaab",
".aaaaaba.....ab",
"......baaaaaaab",
"######ba.....ab",
"######baaaaaaab",
"######bbbbbbbbb"};

static const char *unzoom_xpm[]={
"18 17 2 1",
". c None",
"# c #000000",
"..................",
"...#..............",
"..###.............",
".#.#.#.......##...",
"...#.....##..##...",
"...#..##.##.......",
"...#..##....##....",
"...#........##....",
"...#...##.........",
"...#...##.##.##...",
".#.#.#....##.##...",
"..###.............",
"...#...#......#...",
"...#..#........#..",
"...##############.",
"......#........#..",
".......#......#..."};

#include "Graph.h"
#include "Grid.h"
#include "CanvasPicker.h"
#include "QwtErrorPlotCurve.h"
#include "TexWidget.h"
#include "LegendWidget.h"
#include "FrameWidget.h"
#include "ArrowMarker.h"
#include "../cursors.h"
#include "ScalePicker.h"
#include "TitlePicker.h"
#include "QwtPieCurve.h"
#include "ImageWidget.h"
#include "QwtBarCurve.h"
#include "BoxCurve.h"
#include "QwtHistogram.h"
#include "VectorCurve.h"
#include "ScaleDraw.h"
#include "../ColorBox.h"
#include "../PatternBox.h"
#include "../SymbolBox.h"
#include "FunctionCurve.h"
#include "Spectrogram.h"
#include "SelectionMoveResizer.h"
#include "RangeSelectorTool.h"
#include "PlotCurve.h"
#include "../ApplicationWindow.h"
#include "ScaleEngine.h"

#ifdef EMF_OUTPUT
#include "EmfEngine.h"
#endif

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QTextStream>
#include <QLocale>
#include <QPrintDialog>
#include <QImageWriter>
#include <QFileInfo>

#if QT_VERSION >= 0x040300
	#include <QSvgGenerator>
#endif

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_text_label.h>
#include <qwt_color_map.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

Graph::Graph(int x, int y, int width, int height, QWidget* parent, Qt::WFlags f)
: QwtPlot(parent)
{
	setWindowFlags(f);

	d_active_tool = NULL;
	d_peak_fit_tool = NULL;
	d_active_enrichement = NULL;
	d_legend = NULL; // no legend for an empty graph
	d_selected_marker = NULL;
	drawTextOn = false;
	drawLineOn = false;
	drawArrowOn = false;
	drawAxesBackbone = true;
	d_auto_scale = true;
	autoScaleFonts = false;
	d_antialiasing = false;
	d_scale_on_print = true;
	d_print_cropmarks = false;

	d_user_step = QVector<double>(QwtPlot::axisCnt);
	for (int i=0; i<QwtPlot::axisCnt; i++)
		d_user_step[i] = 0.0;

	setGeometry(x, y, width, height);
	setAttribute(Qt::WA_DeleteOnClose);
	setAutoReplot (false);

	d_min_tick_length = 5;
	d_maj_tick_length = 9;

	setAxisTitle(QwtPlot::yLeft, tr("Y Axis Title"));
	setAxisTitle(QwtPlot::xBottom, tr("X Axis Title"));
	//due to the plot layout updates, we must always have a non empty title
	setAxisTitle(QwtPlot::yRight, tr(" "));
	setAxisTitle(QwtPlot::xTop, tr(" "));

	// grid
	d_grid = new Grid();
	d_grid->attach(this);

	//custom scale
	for (int i= 0; i<QwtPlot::axisCnt; i++) {
		QwtScaleWidget *scale = (QwtScaleWidget *) axisWidget(i);
		if (scale) {
			scale->setMargin(0);

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
	setMouseTracking(true );

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

	connect (cp,SIGNAL(selectPlot()),this,SLOT(activateGraph()));
	connect (cp,SIGNAL(drawTextOff()),this,SIGNAL(drawTextOff()));
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
	connect (scalePicker, SIGNAL(axisTitleDblClicked()), this, SLOT(enableTextEditor()));
	connect (scalePicker,SIGNAL(axisTitleRightClicked()),this,SLOT(showAxisTitleMenu()));
	connect (scalePicker,SIGNAL(axisRightClicked(int)),this,SLOT(showAxisContextMenu(int)));

	connect (d_zoomer[0],SIGNAL(zoomed (const QwtDoubleRect &)),this,SLOT(zoomed (const QwtDoubleRect &)));
}

MultiLayer* Graph::multiLayer()
{
	if (!parent())
		return NULL;

	return (MultiLayer *)(this->parent()->parent()->parent());
}

void Graph::notifyChanges()
{
	emit modifiedGraph();
}

void Graph::activateGraph()
{
	emit selectedGraph(this);
	setFocus();
}

void Graph::deselectMarker()
{
	if (d_selected_marker && d_lines.contains(d_selected_marker)){
		ArrowMarker *a = (ArrowMarker *)d_selected_marker;
		a->setEditable(false);
	}

	d_selected_marker = NULL;
	if (d_markers_selector)
		delete d_markers_selector;

    emit enableTextEditor(NULL);

	cp->disableEditing();

	deselect(d_active_enrichement);
	d_active_enrichement = NULL;
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
	foreach(FrameWidget *f, d_enrichements){
		LegendWidget *l = qobject_cast<LegendWidget *>(f);
		if (l)
			texts << l;
	}
	return texts;
}

void Graph::select(QWidget *l, bool add)
{
    if (!l){
        d_active_enrichement = NULL;
        return;
    }

    selectTitle(false);
    scalePicker->deselect();
    deselectCurves();

    d_active_enrichement = qobject_cast<LegendWidget *>(l);
    if (d_active_enrichement)
        emit currentFontChanged(((LegendWidget *)l)->font());
    else
        d_active_enrichement = qobject_cast<FrameWidget *>(l);

    if (add){
        if (d_markers_selector && d_markers_selector->contains(l))
            return;
        else if (d_markers_selector)
            d_markers_selector->add(l);
        else {
            d_markers_selector = new SelectionMoveResizer(l);
            connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
        }
    } else {
        if (d_markers_selector)
            delete d_markers_selector;

        d_markers_selector = new SelectionMoveResizer(l);
        connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
    }
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

void Graph::setSelectedMarker(QwtPlotMarker *mrk, bool add)
{
	if (!mrk)
		return;

    selectTitle(false);
	scalePicker->deselect();

	d_selected_marker = mrk;
	if (add){
	    if (d_markers_selector){
            if (d_lines.contains(mrk))
                d_markers_selector->add((ArrowMarker*)mrk);
            else
                return;
        } else {
            if (d_lines.contains(mrk))
                d_markers_selector = new SelectionMoveResizer((ArrowMarker*)mrk);
            else
                return;
            connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
        }
	} else {
	    if (d_lines.contains(mrk)){
	        if (((ArrowMarker*)mrk)->editable()){
	            if (d_markers_selector){
	                delete d_markers_selector;
	            }
                return;
	        }

			if (d_markers_selector && d_markers_selector->contains((ArrowMarker*)mrk))
                return;
            else
                d_markers_selector = new SelectionMoveResizer((ArrowMarker*)mrk);
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
	for (int i=0; i<4; i++){
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
	QList<int> ticksTypeList=getMajorTicksType();
	QString s="MajorTicks\t";
	for (int i=0; i<4; i++)
		s+=QString::number(ticksTypeList[i])+"\t";
	s += "\n";

	ticksTypeList=getMinorTicksType();
	s += "MinorTicks\t";
	for (int i=0; i<4; i++)
		s+=QString::number(ticksTypeList[i])+"\t";

	return s+"\n";
}

QString Graph::saveEnabledTickLabels()
{
	QString s="EnabledTickLabels\t";
	for (int axis=0; axis<QwtPlot::axisCnt; axis++){
		const QwtScaleDraw *sd = axisScaleDraw (axis);
		s += QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels))+"\t";
	}
	return s+"\n";
}

QString Graph::saveLabelsFormat()
{
	QString s="LabelsFormat\t";
	for (int axis=0; axis<QwtPlot::axisCnt; axis++){
		s += QString::number(axisLabelFormat(axis))+"\t";
		s += QString::number(axisLabelPrecision(axis))+"\t";
	}
	return s+"\n";
}

QString Graph::saveAxesBaseline()
{
	QString s="AxesBaseline\t";
	for (int i = 0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
		if (scale)
			s+= QString::number(scale->margin()) + "\t";
		else
			s+= "0\t";
	}
	return s+"\n";
}

QString Graph::saveLabelsRotation()
{
	QString s="LabelsRotation\t";
	s+=QString::number(labelsRotation(QwtPlot::xBottom))+"\t";
	s+=QString::number(labelsRotation(QwtPlot::xTop))+"\n";
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
		majLength = minLength;
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
	if (minorTickLength() == minLength &&
			majorTickLength() == majLength)
		return;

	setTicksLength(minLength, majLength);

	hide();
	for (int i=0; i<4; i++)
	{
		if (axisEnabled(i))
		{
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
		const QString& formula, const QColor& labelsColor)
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
			sd->hasComponent (QwtAbstractScaleDraw::Labels) == labelsOn)
		return;

	scale->setMargin(baselineDist);
	QPalette pal = scale->palette();
	if (pal.color(QPalette::Active, QColorGroup::Foreground) != c)
		pal.setColor(QColorGroup::Foreground, c);
    if (pal.color(QPalette::Active, QColorGroup::Text) != labelsColor)
		pal.setColor(QColorGroup::Text, labelsColor);
    scale->setPalette(pal);

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
	sd->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);

	setAxisTicksLength(axis, majTicksType, minTicksType,
			minorTickLength(), majorTickLength());

	if (axisOn && (axis == QwtPlot::xTop || axis == QwtPlot::yRight))
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

	QStringList list = formatInfo.split(";", QString::KeepEmptyParts);
	if ((int)list.count() < 2)
	{
        QMessageBox::critical(this, tr("QtiPlot - Error"),
		tr("Couldn't change the axis type to the requested format!"));
        return;
    }
    if (list[0].isEmpty() || list[1].isEmpty())
    {
        QMessageBox::critical(this, tr("QtiPlot - Error"),
		tr("Couldn't change the axis type to the requested format!"));
        return;
    }

	if (type == ScaleDraw::Time)
	{
		ScaleDraw *sd = new ScaleDraw(this);
		sd->setTimeFormat(QTime::fromString (list[0]), list[1]);
		sd->enableComponent (QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
		setAxisScaleDraw (axis, sd);
	}
	else if (type == ScaleDraw::Date)
	{
		ScaleDraw *sd = new ScaleDraw(this);
		sd->setDateFormat(QDateTime::fromString (list[0], Qt::ISODate), list[1]);
		sd->enableComponent (QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
		setAxisScaleDraw (axis, sd);
	}
}

void Graph::setAxisLabelRotation(int axis, int rotation)
{
	if (axis==QwtPlot::xBottom)
	{
		if (rotation > 0)
			setAxisLabelAlignment(axis, Qt::AlignRight|Qt::AlignVCenter);
		else if (rotation < 0)
			setAxisLabelAlignment(axis, Qt::AlignLeft|Qt::AlignVCenter);
		else if (rotation == 0)
			setAxisLabelAlignment(axis, Qt::AlignHCenter|Qt::AlignBottom);
	}
	else if (axis==QwtPlot::xTop)
	{
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
	if (scale){
		QPalette pal = scale->palette();
		pal.setColor(QColorGroup::Foreground, color);
		scale->setPalette(pal);
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
	((QwtPlot *)this)->setTitle(t);
	replot();
	emit modifiedGraph();
}

void Graph::setTitleAlignment(int align)
{
	QwtText t = title();
	t.setRenderFlags(align);
	((QwtPlot *)this)->setTitle(t);
	replot();
	emit modifiedGraph();
}

void Graph::setTitleFont(const QFont &fnt)
{
	QwtText t = title();
	t.setFont(fnt);
	((QwtPlot *)this)->setTitle(t);
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

void Graph::setScaleTitle(int axis, const QString& text)
{
	int a = 0;
	switch (axis)
	{
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

void Graph::setAxisTitle(int axis, const QString& text)
{
	if (text.isEmpty())//avoid empty titles due to plot layout behavior
		((QwtPlot *)this)->setAxisTitle(axis, " ");
	else
		((QwtPlot *)this)->setAxisTitle(axis, text);

	replot();
	emit modifiedGraph();
}

void Graph::updateSecondaryAxis(int axis)
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

	int a = QwtPlot::xBottom;
	if (axis == QwtPlot::yRight)
		a = QwtPlot::yLeft;

	if (!axisEnabled(a))
		return;

	ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(axis);
	sc_engine->clone((ScaleEngine *)axisScaleEngine(a));

	setAxisScaleDiv (axis, *axisScaleDiv(a));
	d_user_step[axis] = d_user_step[a];
}

void Graph::initScaleLimits()
{//We call this function the first time we add curves to a plot in order to avoid curves with cut symbols.
	replot();

	QwtDoubleInterval intv[QwtPlot::axisCnt];
    const QwtPlotItemList& itmList = itemList();
    QwtPlotItemIterator it;
	double maxSymbolSize = 0;
    for ( it = itmList.begin(); it != itmList.end(); ++it ){
        const QwtPlotItem *item = *it;
        if (item->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;

		const QwtPlotCurve *c = (QwtPlotCurve *)item;
		const QwtSymbol s = c->symbol();
		if (s.style() != QwtSymbol::NoSymbol && s.size().width() >= maxSymbolSize)
			maxSymbolSize = s.size().width();

        const QwtDoubleRect rect = item->boundingRect();
        intv[item->xAxis()] |= QwtDoubleInterval(rect.left(), rect.right());
        intv[item->yAxis()] |= QwtDoubleInterval(rect.top(), rect.bottom());
    }

	if (maxSymbolSize == 0.0)
		return;

	maxSymbolSize *= 0.5;

	QwtScaleDiv *div = axisScaleDiv(QwtPlot::xBottom);
	double start = div->lBound();
	double end = div->hBound();
	QwtValueList majTicksLst = div->ticks(QwtScaleDiv::MajorTick);
	int ticks = majTicksLst.size();
	double step = fabs(end - start)/(double)(ticks - 1.0);
    d_user_step[QwtPlot::xBottom] = step;
    d_user_step[QwtPlot::xTop] = step;

	const QwtScaleMap &xMap = canvasMap(QwtPlot::xBottom);
    double x_left = xMap.xTransform(intv[QwtPlot::xBottom].minValue());

	if (start >= xMap.invTransform(x_left - maxSymbolSize))
		start = div->lBound() - step;

	double x_right = xMap.xTransform(intv[QwtPlot::xBottom].maxValue());
	if (end <= xMap.invTransform(x_right + maxSymbolSize))
		end = div->hBound() + step;

	setAxisScale(QwtPlot::xBottom, start, end, step);
	setAxisScale(QwtPlot::xTop, start, end, step);

	div = axisScaleDiv(QwtPlot::yLeft);
	start = div->lBound();
	end = div->hBound();
	majTicksLst = div->ticks(QwtScaleDiv::MajorTick);
	ticks = majTicksLst.size();
	step = fabs(end - start)/(double)(ticks - 1.0);
    d_user_step[QwtPlot::yLeft] = step;
    d_user_step[QwtPlot::yRight] = step;

	const QwtScaleMap &yMap = canvasMap(QwtPlot::yLeft);
    double y_bottom = yMap.xTransform(intv[QwtPlot::yLeft].minValue());
	if (start >= yMap.invTransform(y_bottom + maxSymbolSize))
		start = div->lBound() - step;

	double y_top = yMap.xTransform(intv[QwtPlot::yLeft].maxValue());
	if (end <= yMap.invTransform(y_top - maxSymbolSize))
		end = div->hBound() + step;

	setAxisScale(QwtPlot::yLeft, start, end, step);
	setAxisScale(QwtPlot::yRight, start, end, step);
	
	replot();

	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();
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

	if (type == 1){
		sc_engine->setType(QwtScaleTransformation::Log10);
		if (start <= 0 || end <= 0){
            QwtDoubleInterval intv = axisBoundingInterval(axis);
            if (start < end)
                start = intv.minValue();
            else
                end = intv.minValue();
		}
	} else
		sc_engine->setType(QwtScaleTransformation::Linear);

	int max_min_intervals = minorTicks;
	if (minorTicks == 1)
		max_min_intervals = 3;
	if (minorTicks > 1)
		max_min_intervals = minorTicks + 1;

	QwtScaleDiv div = sc_engine->divideScale (QMIN(start, end), QMAX(start, end), majorTicks, max_min_intervals, step);
	setAxisMaxMajor (axis, majorTicks);
	setAxisMaxMinor (axis, minorTicks);

	if (inverted)
		div.invert();
	setAxisScaleDiv (axis, div);

	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();

	d_user_step[axis] = step;

	if (axis == QwtPlot::xBottom || axis == QwtPlot::yLeft){
  		updateSecondaryAxis(QwtPlot::xTop);
  	    updateSecondaryAxis(QwtPlot::yRight);
  	}

	replot();
	//keep markers on canvas area
	updateMarkersBoundingRect();
	replot();
	axisWidget(axis)->repaint();
}

QStringList Graph::analysableCurvesList()
{
	QStringList cList;
	foreach(QwtPlotItem *it, d_curves){
    	if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
			PlotCurve *c = (PlotCurve*)it;
        	if (c->type() != ErrorBars)
				cList << c->title().text();
		}
	}

	return cList;
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
	QApplication::clipboard()->setPixmap(graphPixmap(), QClipboard::Clipboard);
}

QPixmap Graph::graphPixmap()
{
	QPixmap pixmap(size());
    QPainter p(&pixmap);
    print(&p, rect());
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

void Graph::exportImage(const QString& fileName, int quality, bool transparent)
{
    QPixmap pic(size());
    QPainter p(&pic);
    print(&p, rect());
    p.end();

	if (transparent){
		QBitmap mask(pic.size());
		mask.fill(Qt::color1);
		QPainter p(&mask);
		p.setPen(Qt::color0);

        QColor background = QColor (Qt::white);
		QRgb backgroundPixel = background.rgb ();
		QImage image = pic.convertToImage();
		for (int y=0; y<image.height(); y++){
			for ( int x=0; x<image.width(); x++ ){
				QRgb rgb = image.pixel(x, y);
				if (rgb == backgroundPixel) // we want the frame transparent
					p.drawPoint(x, y);
			}
		}
		p.end();
		pic.setMask(mask);
	}
	pic.save(fileName, 0, quality);
}

void Graph::exportVector(const QString& fileName, int res, bool color, bool keepAspect,
					QPrinter::PageSize pageSize)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

	QPrinter printer;
    printer.setCreator("QtiPlot");
	printer.setFullPage(true);
	//if (res) //only printing with screen resolution works correctly for the moment
		//printer.setResolution(res);

    printer.setOutputFileName(fileName);
    if (fileName.contains(".eps"))
    	printer.setOutputFormat(QPrinter::PostScriptFormat);

#ifdef Q_OS_MAC
    if (fileName.contains(".pdf"))
    	printer.setOutputFormat(QPrinter::NativeFormat);
#endif

    if (color)
		printer.setColorMode(QPrinter::Color);
	else
		printer.setColorMode(QPrinter::GrayScale);

	printer.setOrientation(QPrinter::Portrait);

	QRect plotRect = rect();
	if (pageSize == QPrinter::Custom)
        printer.setPaperSize (QSizeF(width(), height()), QPrinter::DevicePixel);
    else {
        printer.setPageSize(pageSize);
		double plot_aspect = double(frameGeometry().width())/double(frameGeometry().height());
		if (keepAspect){// export should preserve plot aspect ratio
        double page_aspect = double(printer.width())/double(printer.height());
        	if (page_aspect > plot_aspect){
            	int margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
            	int height = printer.height() - 2*margin;
            	int width = int(height*plot_aspect);
            	int x = (printer.width()- width)/2;
            	plotRect = QRect(x, margin, width, height);
        	} else if (plot_aspect >= page_aspect){
           		int margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
            	int width = printer.width() - 2*margin;
            	int height = int(width/plot_aspect);
            	int y = (printer.height()- height)/2;
            	plotRect = QRect(margin, y, width, height);
        	}
		} else {
	    	int x_margin = (int) ((0.1/2.54)*printer.logicalDpiX()); // 1 mm margins
        	int y_margin = (int) ((0.1/2.54)*printer.logicalDpiY()); // 1 mm margins
        	int width = printer.width() - 2*x_margin;
        	int height = printer.height() - 2*y_margin;
        	plotRect = QRect(x_margin, y_margin, width, height);
		}
	}

    QPainter paint(&printer);
	print(&paint, plotRect);
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
		if (d_scale_on_print){
			int dpiy = printer.logicalDpiY();
			int margin = (int) ((2/2.54)*dpiy ); // 2 cm margins

			int width = qRound(aspect*printer.height()) - 2*margin;
			int x=qRound(abs(printer.width()- width)*0.5);

			plotRect = QRect(x, margin, width, printer.height() - 2*margin);
			if (x < margin){
				plotRect.setLeft(margin);
				plotRect.setWidth(printer.width() - 2*margin);
			}
		} else {
    		int x_margin = (paperRect.width() - plotRect.width())/2;
    		int y_margin = (paperRect.height() - plotRect.height())/2;
    		plotRect.moveTo(x_margin, y_margin);
		}

        QPainter paint(&printer);
        if (d_print_cropmarks){
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

		print(&paint, plotRect);
	}
}

void Graph::exportSVG(const QString& fname)
{
	QSvgGenerator svg;
	svg.setFileName(fname);
	svg.setSize(size());

	QPainter p(&svg);
	print(&p, rect());
	p.end();
}

#ifdef EMF_OUTPUT
void Graph::exportEMF(const QString& fname)
{
	EmfPaintDevice *emf = new EmfPaintDevice(size(), fname);
	QPainter paint;
	paint.begin(emf);
	print(&paint, rect());
	paint.end();
	delete emf;
}
#endif

QString Graph::selectedCurveTitle()
{
	if (d_range_selector)
		return d_range_selector->selectedCurve()->title().text();
	else
		return QString::null;
}

bool Graph::markerSelected()
{
	if (d_selected_marker)
		return true;
	if (d_active_enrichement)
		return true;
	return false;
}

void Graph::removeMarker()
{
	if (d_selected_marker && d_lines.contains(d_selected_marker))
			remove((ArrowMarker*)d_selected_marker);
	else if (d_active_enrichement)
		remove(d_active_enrichement);
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

	if (arrow == d_selected_marker)
		d_selected_marker = NULL;

	arrow->detach();
	replot();

	emit modifiedGraph();
}

void Graph::remove(FrameWidget* f)
{
	if (!f)
		return;

	int index = d_enrichements.indexOf (f);
	if (index >= 0 && index < d_enrichements.size())
		d_enrichements.removeAt(index);

	if (f == d_legend)
		d_legend = NULL;
	if (f == d_active_enrichement)
		d_active_enrichement = NULL;

	f->close();
	emit modifiedGraph();
}

bool Graph::arrowMarkerSelected()
{
	return (d_selected_marker && d_lines.contains(d_selected_marker));
}

bool Graph::imageMarkerSelected()
{
	ImageWidget *i = qobject_cast<ImageWidget *>(d_active_enrichement);
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
}

void Graph::deselectCurves()
{
	QList<QwtPlotItem *> curves = curvesList();
    foreach(QwtPlotItem *i, curves){
        if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
          ((PlotCurve *)i)->type() != Graph::Function &&
          ((DataCurve *)i)->hasSelectedLabels()){
            ((DataCurve *)i)->setLabelsSelected(false);
            return;
        }
    }
}

DataCurve* Graph::selectedCurveLabels()
{
	QList<QwtPlotItem *> curves = curvesList();
    foreach(QwtPlotItem *i, curves){
        if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
          ((PlotCurve *)i)->type() != Graph::Function &&
          ((DataCurve *)i)->hasSelectedLabels())
            return (DataCurve *)i;
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
		emit currentFontChanged(title().font());
    }
}

void Graph::removeTitle()
{
	if (titleLabel()->hasFocus()){
		setTitle(" ");
		emit modifiedGraph();
	}
}

void Graph::initTitle(bool on, const QFont& fnt)
{
	if (on){
		QwtText t = title();
		t.setFont(fnt);
		t.setText(tr("Title"));
		((QwtPlot *)this)->setTitle (t);
	}
}

void Graph::removeLegend()
{
	if (d_legend){
	    delete d_legend;
	    d_legend = NULL;
	}
}

QString Graph::legendText()
{
	QString text;
	int i = 0;
	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;

		if (((PlotCurve *)it)->type() != ErrorBars ){
			text += "\\l(";
			text += QString::number(i + 1);
			text += ")%(";
			text += QString::number(i + 1);
			text += ")\n";
			i++;
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
		for (int i=0;i<int(c->dataSize());i++){
			text += "\\p{";
			text += QString::number(i+1);
			text += "} ";
			text += QString::number(i+1);
			text += "\n";
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
        	if (c->type() == ErrorBars || c->type() == Function)
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

void Graph::loadAxesOptions(const QString& s)
{
	if (s == "1")
		return;

	drawAxesBackbone = false;

	for (int i=0; i<QwtPlot::axisCnt; i++)
	{
		QwtScaleWidget *scale=(QwtScaleWidget*) axisWidget(i);
		if (scale)
		{
			ScaleDraw *sclDraw = (ScaleDraw *)axisScaleDraw (i);
			sclDraw->enableComponent (QwtAbstractScaleDraw::Backbone, false);
			scale->repaint();
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
	for (int i=0; i < QwtPlot::axisCnt; i++)
	{
		s += "scale\t" + QString::number(i)+"\t";

		const QwtScaleDiv *scDiv = axisScaleDiv(i);
		QwtValueList lst = scDiv->ticks (QwtScaleDiv::MajorTick);

		s += QString::number(QMIN(scDiv->lBound(), scDiv->hBound()), 'g', 15)+"\t";
		s += QString::number(QMAX(scDiv->lBound(), scDiv->hBound()), 'g', 15)+"\t";
		s += QString::number(d_user_step[i], 'g', 15)+"\t";
		s += QString::number(axisMaxMajor(i))+"\t";
		s += QString::number(axisMaxMinor(i))+"\t";

		const ScaleEngine *sc_eng = (ScaleEngine *)axisScaleEngine(i);
		s += QString::number((int)sc_eng->type())+"\t";
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
	QString s="";
	for (int i=0; i<4; i++)
	{
		switch (i)
		{
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
		QString title = axisTitle(a).text();
		if (!title.isEmpty())
            s += title.replace("\n", "<br>")+"\t";
        else
            s += "\t";
	}
	return s+"\n";
}

QString Graph::saveAxesTitleAlignement()
{
	QString s="AxesTitleAlignment\t";
	QStringList axes;
	int i;
	for (i=0;i<4;i++)
		axes<<QString::number(Qt::AlignHCenter);

	for (i=0;i<4;i++)
	{

		if (axisEnabled(i))
			axes[i]=QString::number(axisTitle(i).renderFlags());
	}

	s+=axes.join("\t")+"\n";
	return s;
}

QString Graph::savePieCurveLayout()
{
	QString s="PieCurve\t";

	QwtPieCurve *pie = (QwtPieCurve*)curve(0);
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
		s+=QString::number(ColorBox::colorIndex(c->pen().color()))+"\t";
		s+=QString::number(c->pen().style()-1)+"\t";
		s+=QString::number(c->pen().widthF())+"\t";

		const QwtSymbol symbol = c->symbol();
		s+=QString::number(symbol.size().width())+"\t";
		s+=QString::number(SymbolBox::symbolIndex(symbol.style()))+"\t";
		s+=QString::number(ColorBox::colorIndex(symbol.pen().color()))+"\t";
		if (symbol.brush().style() != Qt::NoBrush)
			s+=QString::number(ColorBox::colorIndex(symbol.brush().color()))+"\t";
		else
			s+=QString::number(-1)+"\t";

		bool filled = c->brush().style() == Qt::NoBrush ? false : true;
		s+=QString::number(filled)+"\t";

		s+=QString::number(ColorBox::colorIndex(c->brush().color()))+"\t";
		s+=QString::number(PatternBox::patternIndex(c->brush().style()))+"\t";
		if (style <= LineSymbols || style == Box)
			s+=QString::number(symbol.pen().widthF())+"\t";
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

		QStringList colsList = v->plotAssociation().split(",", QString::SkipEmptyParts);
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
				else
					s += "curve\t" + c->xColumnName() + "\t" + c->title().text() + "\t";

				s += saveCurveLayout(i);
				s += QString::number(c->xAxis())+"\t"+QString::number(c->yAxis())+"\t";
				s += QString::number(c->startRow())+"\t"+QString::number(c->endRow())+"\t";
				s += QString::number(c->isVisible())+"\n";
				s += c->saveToString();
			} else if (c->type() == ErrorBars){
  	        	QwtErrorPlotCurve *er = (QwtErrorPlotCurve *)it;
  	            s += "ErrorBars\t";
  	            s += QString::number(er->direction())+"\t";
  	            s += er->masterCurve()->xColumnName() + "\t";
  	            s += er->masterCurve()->title().text() + "\t";
  	            s += er->title().text() + "\t";
  	            s += QString::number(er->width())+"\t";
  	            s += QString::number(er->capLength())+"\t";
  	            s += er->color().name()+"\t";
  	            s += QString::number(er->throughSymbol())+"\t";
  	            s += QString::number(er->plusSide())+"\t";
  	            s += QString::number(er->minusSide())+"\n";
  	       }
		}
	}
	return s;
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

    d_legend = l;
	d_enrichements << l;
	emit modifiedGraph();
	return l;
}

LegendWidget* Graph::addTimeStamp()
{
	LegendWidget* l = newLegend(QDateTime::currentDateTime().toString(Qt::LocalDate));

	QPoint p = canvas()->pos();
	l->move(QPoint(p.x() + canvas()->width()/2, p.y() + 10));
	emit modifiedGraph();
	return l;
}

void Graph::insertLegend(const QStringList& lst, int fileVersion)
{
	d_legend = insertText(lst, fileVersion);
}

LegendWidget* Graph::insertText(const QStringList& list, int fileVersion)
{
	QStringList fList = list;
	bool pieLabel = (list[0] == "<PieLabel>") ? true : false;
	LegendWidget* l = NULL;
	if (pieLabel)
		l = new PieLabel(this);
	else
		l = new LegendWidget(this);

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
		QwtPieCurve *pie = (QwtPieCurve *)curve(0);
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
	}
	return aux;
}

LegendWidget* Graph::addText(LegendWidget* t)
{
	LegendWidget* aux = new LegendWidget(this);
	aux->clone(t);
	d_active_enrichement = aux;
	d_enrichements << aux;
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
		s+=QString::number(mrkL->filledArrowHead())+"</line>\n";
	}

	QObjectList lst = children();
	foreach(QObject *o, lst){
		if (o->inherits("LegendWidget")){
			LegendWidget *l = (LegendWidget *)o;
			if (l == d_legend)
				s += "<legend>\t";
			else if (l->isA("PieLabel")){
				if (l->text().isEmpty())
					continue;
				else
					s += "<PieLabel>\t";
			} else
				s += "<text>\t";

			s += QString::number(l->xValue(), 'g', 15) + "\t";
			s += QString::number(l->yValue(), 'g', 15) + "\t";

			QFont f = l->font();
			s += f.family() + "\t";
			s += QString::number(f.pointSize())+"\t";
			s += QString::number(f.weight())+"\t";
			s += QString::number(f.italic())+"\t";
			s += QString::number(f.underline())+"\t";
			s += QString::number(f.strikeOut())+"\t";
			s += l->textColor().name()+"\t";
			s += QString::number(l->frameStyle())+"\t";
			s += QString::number(l->angle())+"\t";
			s += l->backgroundColor().name()+"\t";
			s += QString::number(l->backgroundColor().alpha())+"\t";

			QStringList textList=l->text().split("\n", QString::KeepEmptyParts);
			s += textList.join ("\t");
			if (l == d_legend)
				s += "</legend>\n";
			else if (l->isA("PieLabel"))
				s += "</PieLabel>\n";
			else
				s += "</text>\n";
		}
	}

	foreach(FrameWidget *f, d_enrichements)
		s += f->saveToString();

	return s;
}

double Graph::selectedXStartValue()
{
	if (d_range_selector)
		return d_range_selector->minXValue();
	else
		return 0;
}

double Graph::selectedXEndValue()
{
	if (d_range_selector)
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

QwtPlotCurve *Graph::curve(int index)
{
	int curves = d_curves.size();
    if (!curves || index >= curves || index < 0)
		return 0;

	QwtPlotItem *it = d_curves.at(index);
    if (it && it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
        return (QwtPlotCurve*)it;

	return 0;
}

int Graph::range(const QString& curveTitle, double *start, double *end)
{
	if (d_range_selector && d_range_selector->selectedCurve() == curve(curveTitle)) {
		*start = d_range_selector->minXValue();
		*end = d_range_selector->maxXValue();
		return d_range_selector->dataSize();
	} else {
		QwtPlotCurve *c = curve(curveTitle);
		if (!c)
			return 0;

		*start = c->minXValue();
		*end = c->maxXValue();
		return c->dataSize();
	}
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
	cl.aCol = 0;
	cl.aStyle = 0;
	cl.lCol = 0;
	cl.penWidth = 1;
	cl.symCol = 0;
	cl.fillCol = 0;
	return cl;
}

CurveLayout Graph::initCurveLayout(int style, int curves)
{
    int i = d_curves.size() - 1;

	CurveLayout cl = initCurveLayout();	
	int color;
	guessUniqueCurveLayout(color, cl.sType);

  	cl.lCol = color;
  	cl.symCol = color;
  	cl.fillCol = color;

	if (style == Graph::Line)
		cl.sType = 0;
	else if (style == Graph::Scatter)
		cl.connectType = 0;
	else if (style == Graph::VerticalDropLines)
		cl.connectType = 2;
	else if (style == Graph::HorizontalSteps || style == Graph::VerticalSteps){
		cl.connectType = 3;
		cl.sType = 0;
	} else if (style == Graph::Spline)
		cl.connectType = 5;
	else if (curves && (style == Graph::VerticalBars || style == Graph::HorizontalBars)){
		cl.filledArea = 1;
		cl.lCol = 0;//black color pen
		cl.aCol = i + 1;
		cl.sType = 0;
		QwtBarCurve *b = (QwtBarCurve*)curve(i);
		if (b && (b->type() == VerticalBars || b->type() == HorizontalBars)){
			b->setGap(qRound(100*(1-1.0/(double)curves)));
			b->setOffset(-50*(curves-1) + i*100);
		}
	} else if (style == Graph::Histogram){
		cl.filledArea = 1;
		cl.lCol = i + 1;//start with red color pen
		cl.aCol = i + 1; //start with red fill color
		cl.aStyle = 4;
		cl.sType = 0;
	} else if (style == Graph::Area){
		cl.filledArea = 1;
		cl.aCol = color;
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

	QPen pen = QPen(ColorBox::color(cL->symCol), cL->penWidth, Qt::SolidLine);
	if (cL->fillCol != -1)
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(ColorBox::color(cL->fillCol)), pen, QSize(cL->sSize, cL->sSize)));
	else
		c->setSymbol(QwtSymbol(SymbolBox::style(cL->sType), QBrush(), pen, QSize(cL->sSize, cL->sSize)));

	c->setPen(QPen(ColorBox::color(cL->lCol), cL->lWidth, getPenStyle(cL->lStyle)));

	switch (c->plotStyle()){
		case Scatter:
			c->setStyle(QwtPlotCurve::NoCurve);
		break;
		case Spline:
			c->setStyle(QwtPlotCurve::Lines);
			c->setCurveAttribute(QwtPlotCurve::Fitted, true);
		break;
		case VerticalSteps:
			c->setStyle(QwtPlotCurve::Steps);
			c->setCurveAttribute(QwtPlotCurve::Inverted, true);
		break;
		default:
			c->setStyle((QwtPlotCurve::CurveStyle)cL->connectType);
		break;
	}

	QBrush brush = QBrush(ColorBox::color(cL->aCol));
	if (cL->filledArea)
		brush.setStyle(PatternBox::brushStyle(cL->aStyle));
	else
		brush.setStyle(Qt::NoBrush);
	c->setBrush(brush);
}

void Graph::updateErrorBars(QwtErrorPlotCurve *er, bool xErr, double width, int cap, const QColor& c,
		bool plus, bool minus, bool through)
{
	if (!er)
		return;

	if (er->width() == width && er->capLength() == cap &&
		er->color() == c && er->plusSide() == plus &&
		er->minusSide() == minus && er->throughSymbol() == through && er->xErrors() == xErr)
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

QwtErrorPlotCurve* Graph::addErrorBars(const QString& yColName, Table *errTable, const QString& errColName,
		int type, double width, int cap, const QColor& color, bool through, bool minus, bool plus)
{
	foreach(QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
			continue;
		if (((PlotCurve *)it)->type() == ErrorBars || ((PlotCurve *)it)->type() == Function)
			continue;

		if (it->title().text() == yColName){
			return addErrorBars(((DataCurve*)it)->xColumnName(), yColName, errTable, errColName,
					type, width, cap, color, through, minus, plus);
		}
	}
	return NULL;
}

QwtErrorPlotCurve* Graph::addErrorBars(const QString& xColName, const QString& yColName,
		Table *errTable, const QString& errColName, int type, double width, int cap,
		const QColor& color, bool through, bool minus, bool plus)
{
	DataCurve *master_curve = masterCurve(xColName, yColName);
	if (!master_curve)
		return NULL;

	QwtErrorPlotCurve *er = new QwtErrorPlotCurve(type, errTable, errColName);
	insertCurve(er);

	er->setMasterCurve(master_curve);
	er->setCapLength(cap);
	er->setColor(color);
	er->setWidth(width);
	er->drawPlusSide(plus);
	er->drawMinusSide(minus);
	er->drawThroughSymbol(through);

	updatePlot();
	return er;
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

QwtPieCurve* Graph::plotPie(Table* w, const QString& name, const QPen& pen, int brush,
					int size, int firstColor, int startRow, int endRow, bool visible,
					double d_start_azimuth, double d_view_angle, double d_thickness,
					double d_horizontal_offset, double d_edge_dist, bool d_counter_clockwise,
					bool d_auto_labeling, bool d_values, bool d_percentages,
					bool d_categories, bool d_fixed_labels_pos)
{
	if (endRow < 0)
		endRow = w->numRows() - 1;

	QwtPieCurve *pie = new QwtPieCurve(w, name, startRow, endRow);
	insertCurve(pie);

	pie->loadData();
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

QwtPieCurve* Graph::plotPie(Table* w, const QString& name, int startRow, int endRow)
{
	for (int i=0; i<QwtPlot::axisCnt; i++)
		enableAxis(i, false);
	scalePicker->refresh();

	setTitle(QString::null);

	QwtPlotCanvas* canvas =(QwtPlotCanvas*)this->canvas();
	canvas->setLineWidth(1);

	QwtPieCurve *pie = new QwtPieCurve(w, name, startRow, endRow);
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

bool Graph::addCurves(Table* w, const QStringList& names, int style, double lWidth,
							int sSize, int startRow, int endRow)
{
	if (style == Pie)
		plotPie(w, names[0], startRow, endRow);
	else if (style == Box)
		plotBoxDiagram(w, names, startRow, endRow);
	else if (style == VectXYXY || style == VectXYAM)
		plotVectorCurve(w, names, style, startRow, endRow);
	else {
		int curves = (int)names.count();
        int errCurves = 0;
		QStringList lst = QStringList();
        for (int i=0; i<curves; i++)
        {//We rearrange the list so that the error bars are placed at the end
        	int j = w->colIndex(names[i]);
  	        if (w->colPlotDesignation(j) == Table::xErr || w->colPlotDesignation(j) == Table::yErr ||
				w->colPlotDesignation(j) == Table::Label){
				errCurves++;
				lst << names[i];
			} else
				lst.prepend(names[i]);
        }

		for (int i=0; i<curves; i++){
            int j = w->colIndex(names[i]);
            PlotCurve *c = NULL;
            if (w->colPlotDesignation(j) == Table::xErr || w->colPlotDesignation(j) == Table::yErr){
				int ycol = w->colY(w->colIndex(names[i]));
                if (ycol < 0)
                    return false;

                if (w->colPlotDesignation(j) == Table::xErr)
                    c = (PlotCurve *)addErrorBars(w->colName(ycol), w, names[i], (int)QwtErrorPlotCurve::Horizontal);
                else
                    c = (PlotCurve *)addErrorBars(w->colName(ycol), w, names[i]);
			} else if (w->colPlotDesignation(j) == Table::Label){
				QString labelsCol = names[i];
				int xcol = w->colX(w->colIndex(labelsCol));
				int ycol = w->colY(w->colIndex(labelsCol));
				if (xcol < 0 || ycol < 0)
                    return false;

				DataCurve* mc = masterCurve(w->colName(xcol), w->colName(ycol));
				if (mc){
				    replot();
					mc->setLabelsColumnName(labelsCol);
				} else
					return false;
			} else
                c = (PlotCurve *)insertCurve(w, names[i], style, startRow, endRow);

            if (c && c->type() != ErrorBars){
				CurveLayout cl = initCurveLayout(style, curves - errCurves);
				cl.sSize = sSize;
				cl.lWidth = lWidth;
				updateCurveLayout(c, &cl);
			}
		}
	}
	initScaleLimits();
	return true;
}

PlotCurve* Graph::insertCurve(Table* w, const QString& name, int style, int startRow, int endRow)
{//provided for convenience
	int ycol = w->colIndex(name);
	int xcol = w->colX(ycol);

	PlotCurve* c = insertCurve(w, w->colName(xcol), w->colName(ycol), style, startRow, endRow);
	if (c)
		emit modifiedGraph();
	return c;
}

PlotCurve* Graph::insertCurve(Table* w, int xcol, const QString& name, int style)
{
	return insertCurve(w, w->colName(xcol), w->colName(w->colIndex(name)), style);
}

PlotCurve* Graph::insertCurve(Table* w, const QString& xColName, const QString& yColName, int style, int startRow, int endRow)
{
	int xcol=w->colIndex(xColName);
	int ycol=w->colIndex(yColName);
	if (xcol < 0 || ycol < 0)
		return NULL;

	int xColType = w->columnType(xcol);
	int yColType = w->columnType(ycol);
	int size=0;
	QString date_time_fmt = w->columnFormat(xcol);
	QStringList xLabels, yLabels;// store text labels
	QTime time0;
	QDateTime date0;

	if (endRow < 0)
		endRow = w->numRows() - 1;

	int r = abs(endRow - startRow) + 1;
    QVector<double> X(r), Y(r);
	if (xColType == Table::Time){
		for (int i = startRow; i<=endRow; i++ ){
			QString xval=w->text(i,xcol);
			if (!xval.isEmpty()){
				time0 = QTime::fromString (xval, date_time_fmt);
				if (time0.isValid())
					break;
			}
		}
	} else if (xColType == Table::Date){
		for (int i = startRow; i<=endRow; i++ ){
			QString xval=w->text(i,xcol);
			if (!xval.isEmpty()){
				date0 = QDateTime::fromString(xval, date_time_fmt);
				if (date0.isValid())
					break;
			}
		}
	}

	for (int i = startRow; i<=endRow; i++ ){
		QString xval=w->text(i,xcol);
		QString yval=w->text(i,ycol);
		if (!xval.isEmpty() && !yval.isEmpty()){
		    bool valid_data = true;
			if (xColType == Table::Text){
				if (xLabels.contains(xval) == 0)
					xLabels << xval;
				X[size] = (double)(xLabels.findIndex(xval)+1);
			} else if (xColType == Table::Time){
				QTime time = QTime::fromString (xval, date_time_fmt);
				if (time.isValid())
					X[size] = time0.msecsTo (time);
				else
					X[size] = 0;
			} else if (xColType == Table::Date){
				QDateTime d = QDateTime::fromString (xval, date_time_fmt);
				if (d.isValid())
					X[size] = (double) date0.secsTo(d);
			} else
                X[size] = locale().toDouble(xval, &valid_data);

			if (yColType == Table::Text){
				yLabels << yval;
				Y[size] = (double) (size + 1);
			} else
                Y[size] = locale().toDouble(yval, &valid_data);

            if (valid_data)
                size++;
		}
	}

	if (!size)
		return NULL;

	X.resize(size);
	Y.resize(size);

	DataCurve *c = 0;
	if (style == VerticalBars){
		c = new QwtBarCurve(QwtBarCurve::Vertical, w, xColName, yColName, startRow, endRow);
	} else if (style == HorizontalBars){
		c = new QwtBarCurve(QwtBarCurve::Horizontal, w, xColName, yColName, startRow, endRow);
	} else if (style == Histogram){
		c = new QwtHistogram(w, xColName, yColName, startRow, endRow);
		((QwtHistogram *)c)->initData(Y.data(), size);
	} else
		c = new DataCurve(w, xColName, yColName, startRow, endRow);

	insertCurve(c);
	c->setPlotStyle(style);
	c->setPen(QPen(Qt::black, 1.0));

	if (style == HorizontalBars)
		c->setData(Y.data(), X.data(), size);
	else if (style != Histogram)
		c->setData(X.data(), Y.data(), size);

	if (xColType == Table::Text ){
		if (style == HorizontalBars)
			setAxisScaleDraw(QwtPlot::yLeft, new ScaleDraw(this, xLabels, xColName));
		else
			setAxisScaleDraw (QwtPlot::xBottom, new ScaleDraw(this, xLabels, xColName));
	} else if (xColType == Table::Time){
		QString fmtInfo = time0.toString() + ";" + date_time_fmt;
		if (style == HorizontalBars)
			setLabelsDateTimeFormat(QwtPlot::yLeft, ScaleDraw::Time, fmtInfo);
		else
			setLabelsDateTimeFormat(QwtPlot::xBottom, ScaleDraw::Time, fmtInfo);
	} else if (xColType == Table::Date ){
		QString fmtInfo = date0.toString(Qt::ISODate) + ";" + date_time_fmt;
		if (style == HorizontalBars)
			setLabelsDateTimeFormat(QwtPlot::yLeft, ScaleDraw::Date, fmtInfo);
		else
			setLabelsDateTimeFormat(QwtPlot::xBottom, ScaleDraw::Date, fmtInfo);
	}

	if (yColType == Table::Text)
		setAxisScaleDraw (QwtPlot::yLeft, new ScaleDraw(this, yLabels, yColName));

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

VectorCurve* Graph::plotVectorCurve(Table* w, const QStringList& colList, int style, int startRow, int endRow)
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
	updateSecondaryAxis(QwtPlot::xTop);
	updateSecondaryAxis(QwtPlot::yRight);

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
	if (d_legend)
        d_legend->setText(QString::null);

	QList <PieLabel *> labels = ((QwtPieCurve *)curve(0))->labelsList();
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

        if(((DataCurve *)it)->plotAssociation().contains(s))
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
        if (((PlotCurve *)it)->type() == ErrorBars)
            ((QwtErrorPlotCurve *)it)->detachFromMasterCurve();
		else if (((PlotCurve *)it)->type() != Function){
			((DataCurve *)it)->clearErrorBars();
			((DataCurve *)it)->clearLabels();
		}

		if (d_fit_curves.contains((QwtPlotCurve *)it)){
			int i = d_fit_curves.indexOf((QwtPlotCurve *)it);
			if (i >= 0 && i < d_fit_curves.size())
				d_fit_curves.removeAt(i);
		}
	} else {
  	    QwtScaleWidget *colorAxis = axisWidget(((Spectrogram *)it)->colorScaleAxis());
  	    if (colorAxis)
  	    	colorAxis->setColorBarEnabled(false);
  	}

    if (d_range_selector && curve(index) == d_range_selector->selectedCurve()){
		int curves = d_curves.size();
        if (curves > 1 && (index - 1) >= 0)
            d_range_selector->setSelectedCurve(curve(index - 1));
        else if (curves > 1 && index + 1 < curves)
            d_range_selector->setSelectedCurve(curve(index + 1));
        else
            disableTools();
    }

	it->detach();
	d_curves.removeAt(index);
	emit modifiedGraph();
}

void Graph::removeLegendItem(int index)
{
	if (!d_legend || index < 0 || index >= d_curves.size())
		return;

	QwtPlotItem *it = d_curves.at(index);
	if (!it || it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
		return;

	if (((PlotCurve *)it)->type() == ErrorBars)
		return;

	if (isPiePlot()){
		d_legend->setText(QString::null);
		return;
	}

	QString text = d_legend->text();
	QStringList items = text.split( "\n", QString::SkipEmptyParts);

	if (index >= (int) items.count())
		return;

	QStringList l = items.grep( "\\l(" + QString::number(index+1) + ")" );
	if (l.isEmpty())
		return;

	items.remove(l[0]);//remove the corresponding legend string

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
	d_legend->setText(text);
}

void Graph::addLegendItem()
{
	if (d_legend){
		QString text = d_legend->text();
		int curves = d_curves.size();
        if (text.endsWith ("\n") || text.isEmpty())
            text.append("\\l("+QString::number(curves)+")"+"%("+QString::number(curves)+")");
        else
            text.append("\n\\l("+QString::number(curves)+")"+"%("+QString::number(curves)+")");

        d_legend->setText(text);
        d_legend->repaint();
	}
}

void Graph::contextMenuEvent(QContextMenuEvent *e)
{
	if (d_selected_marker) {
		emit showMarkerPopupMenu();
		return;
	}

	QPoint pos = canvas()->mapFrom(this, e->pos());
	int dist, point;
	QwtPlotCurve *c = closestCurve(pos.x(), pos.y(), dist, point);
	if (c && dist < 10)//10 pixels tolerance
		emit showCurveContextMenu(c);
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
	emit modifiedGraph();
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
		canvas()->setCursor(QCursor(QPixmap(lens_xpm), -1, -1));
	else
		canvas()->setCursor(Qt::arrowCursor);
}

void Graph::zoomOut()
{
	d_zoomer[0]->zoom(-1);
	d_zoomer[1]->zoom(-1);

	updateSecondaryAxis(QwtPlot::xTop);
  	updateSecondaryAxis(QwtPlot::yRight);
}

void Graph::drawText(bool on)
{
	deselectMarker();

	QCursor c = QCursor(Qt::IBeamCursor);
	if (on){
		canvas()->setCursor(c);
		//setCursor(c);
	} else {
		canvas()->setCursor(Qt::arrowCursor);
		//setCursor(Qt::arrowCursor);
	}
	drawTextOn = on;
}

ImageWidget* Graph::addImage(ImageWidget* i)
{
	if (!i)
		return 0;

	ImageWidget* i2 = new ImageWidget(this, i->fileName());
	if (i2){
		d_enrichements << i2;
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
		d_enrichements << i;
		emit modifiedGraph();
	}
	return i;
}

ImageWidget* Graph::addImage(const QImage& image)
{
	ImageWidget* i = new ImageWidget(this, image);
	if (i){
		d_enrichements << i;
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

        d_enrichements << mrk;

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
	drawLineOn=on;
	drawArrowOn=arrow;
	if (!on)
		emit drawLineEnded(true);
}

void Graph::modifyFunctionCurve(int curve, int type, const QStringList &formulas,
		const QString& var, double start, double end, int points)
{
	FunctionCurve *c = (FunctionCurve *)this->curve(curve);
	if (!c)
		return;

	if (c->functionType() == type &&
		c->variable() == var &&
		c->formulas() == formulas &&
		c->startRange() == start &&
		c->endRange() == end &&
		c->dataSize() == points)
		return;

	QString oldLegend = c->legend();

	c->setFunctionType((FunctionCurve::FunctionType)type);
	c->setRange(start, end);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->loadData(points);

	if (d_legend){//update the legend marker
		QString text = (d_legend->text()).replace(oldLegend, c->legend());
        d_legend->setText(text);
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
	c->setPen(QPen(ColorBox::color(colorIndex), 1.0));

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

void Graph::restoreFunction(const QStringList& lst)
{
	int type = 0;
	int points = 0, style = 0;
	QStringList formulas;
	QString var, title = QString::null;
	double start = 0.0, end = 0.0;

	QStringList::const_iterator line;
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Type>"))
			type = s.remove("<Type>").remove("</Type>").stripWhiteSpace().toInt();
		else if (s.contains("<Title>"))
			title = s.remove("<Title>").remove("</Title>").stripWhiteSpace();
		else if (s.contains("<Expression>"))
			formulas = s.remove("<Expression>").remove("</Expression>").split("\t");
		else if (s.contains("<Variable>"))
			var = s.remove("<Variable>").remove("</Variable>").stripWhiteSpace();
		else if (s.contains("<Range>")){
			QStringList l = s.remove("<Range>").remove("</Range>").split("\t");
			if (l.size() == 2){
				start = l[0].toDouble();
				end = l[1].toDouble();
			}
		} else if (s.contains("<Points>"))
			points = s.remove("<Points>").remove("</Points>").stripWhiteSpace().toInt();
		else if (s.contains("<Style>")){
			style = s.remove("<Style>").remove("</Style>").stripWhiteSpace().toInt();
			break;
		}
	}

	FunctionCurve *c = new FunctionCurve((FunctionCurve::FunctionType)type, title);
	c->setRange(start, end);
	c->setFormulas(formulas);
	c->setVariable(var);
	c->loadData(points);
	c->setPlotStyle(style);

	insertCurve(c);

	QStringList l;
	for (line++; line != lst.end(); line++)
        l << *line;
	c->restoreCurveLayout(l);

	addLegendItem();
	updatePlot();
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

    int size = curve->dataSize();
    QString text = "1\t2\n";
    for (int i=0; i<size; i++)
	{
        text += QString::number(curve->x(i))+"\t";
        text += QString::number(curve->y(i))+"\n";
    }
    QString legend = tr("Data set generated from curve") + ": " + curve->title().text();
    emit createTable(tr("Table") + "1" + "\t" + legend, size, 2, text);
}

QString Graph::saveToString(bool saveAsTemplate)
{
	QString s="<graph>\n";
	s+="ggeometry\t";
	s+=QString::number(this->pos().x())+"\t";
	s+=QString::number(this->pos().y())+"\t";
	s+=QString::number(this->frameGeometry().width())+"\t";
	s+=QString::number(this->frameGeometry().height())+"\n";
	s+=saveTitle();
	s+="<Antialiasing>" + QString::number(d_antialiasing) + "</Antialiasing>\n";
	s+="Background\t" + paletteBackgroundColor().name() + "\t";
	s+=QString::number(paletteBackgroundColor().alpha()) + "\n";
	s+="Margin\t"+QString::number(margin())+"\n";
	s+="Border\t"+QString::number(lineWidth())+"\t"+frameColor().name()+"\n";
	s+=grid()->saveToString();
	s+=saveEnabledAxes();
	s+="AxesTitles\t"+saveScaleTitles();
	s+=saveAxesTitleColors();
	s+=saveAxesTitleAlignement();
	s+=saveFonts();
	s+=saveEnabledTickLabels();
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
	s+="DrawAxesBackbone\t"+QString::number(drawAxesBackbone)+"\n";
	s+="AxesLineWidth\t"+QString::number(axesLinewidth())+"\n";
	s+=saveLabelsRotation();
	s+=saveMarkers();
	s+="</graph>\n";
	return s;
}

void Graph::updateMarkersBoundingRect()
{
    foreach(FrameWidget *f, d_enrichements)
		f->updateCoordinates();

	if (!d_lines.size())
		return;

	foreach (QwtPlotMarker *i, d_lines)
		((ArrowMarker*)i)->updateBoundingRect();

    replot();
}

void Graph::resizeEvent ( QResizeEvent *e )
{
	if (autoScaleFonts){
		QSize oldSize = e->oldSize();
		QSize size = e->size();
		resize(e->size());
		updateLayout();
		if(oldSize.isValid() && size.isValid())
			scaleFonts((double)size.height()/(double)oldSize.height());
	} else {
        resize(e->size());
		updateLayout();
        updateCurveLabels();
	}

	foreach(FrameWidget *f, d_enrichements){
		ImageWidget *i = qobject_cast<ImageWidget *>(f);
		if (i)
			i->resetCoordinates();
		else
			f->resetOrigin();
	}
}

void Graph::scaleFonts(double factor)
{
	QObjectList lst = children();
	foreach(QObject *o, lst){
		if (o->inherits("LegendWidget")){
			QFont font = ((LegendWidget *)o)->font();
			font.setPointSizeFloat(factor*font.pointSizeFloat());
			((LegendWidget *)o)->setFont(font);
		}
	}
			
	for (int i = 0; i<QwtPlot::axisCnt; i++){
		QFont font = axisFont(i);
		font.setPointSizeFloat(factor*font.pointSizeFloat());
		setAxisFont(i, font);

		QwtText title = axisTitle(i);
		font = title.font();
		font.setPointSizeFloat(factor*font.pointSizeFloat());
		title.setFont(font);
		((QwtPlot *)this)->setAxisTitle(i, title);
	}

	QwtText t = this->title();
	QFont font = t.font();
	font.setPointSizeFloat(factor*font.pointSizeFloat());
	t.setFont(font);
	((QwtPlot *)this)->setTitle(t);

	QList<QwtPlotItem *> curves = curvesList();
    foreach(QwtPlotItem *i, curves){
        if(i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram &&
          ((PlotCurve *)i)->type() != Graph::Function &&
          ((DataCurve *)i)->hasLabels()){
            QFont font = ((DataCurve *)i)->labelsFont();
            font.setPointSizeFloat(factor*font.pointSizeFloat());
            ((DataCurve *)i)->setLabelsFont(font);
            if (((DataCurve *)i)->hasSelectedLabels())
                notifyFontChange(font);
        }
    }
    replot();
}

void Graph::setMargin (int d)
{
	if (margin() == d)
		return;

	setMargin(d);
	emit modifiedGraph();
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
	titleMenu.insertItem(QPixmap(cut_xpm), tr("&Cut"),this, SLOT(cutTitle()));
	titleMenu.insertItem(QPixmap(copy_xpm), tr("&Copy"),this, SLOT(copyTitle()));
	titleMenu.insertItem(tr("&Delete"),this, SLOT(removeTitle()));
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
  	setAxisTitle(axis, " ");//due to the plot layout updates, we must always have a non empty title
	replot();
	emit modifiedGraph();
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
	titleMenu.insertItem(QPixmap(cut_xpm), tr("&Cut"), this, SLOT(cutAxisTitle()));
	titleMenu.insertItem(QPixmap(copy_xpm), tr("&Copy"), this, SLOT(copyAxisTitle()));
	titleMenu.insertItem(tr("&Delete"),this, SLOT(removeAxisTitle()));
	titleMenu.insertSeparator();
	titleMenu.insertItem(tr("&Properties..."), this, SIGNAL(showAxisTitleDialog()));
	titleMenu.exec(QCursor::pos());
}

void Graph::showAxisContextMenu(int axis)
{
	QMenu menu(this);
	menu.setCheckable(true);
	menu.insertItem(QPixmap(unzoom_xpm), tr("&Rescale to show all"), this, SLOT(setAutoScale()), tr("Ctrl+Shift+R"));
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

void Graph::copy(Graph* g)
{
	setMargin(g->margin());
	setBackgroundColor(g->paletteBackgroundColor());
	setFrame(g->lineWidth(), g->frameColor());
	setCanvasBackground(g->canvasBackground());

	for (int i = 0; i<QwtPlot::axisCnt; i++){
		if (g->axisEnabled (i)){
			enableAxis(i);
			QwtScaleWidget *scale = (QwtScaleWidget *)axisWidget(i);
			if (scale){
				scale->setMargin(g->axisWidget(i)->margin());
				QPalette pal = scale->palette();
				pal.setColor(QColorGroup::Foreground, g->axisColor(i));
				pal.setColor(QColorGroup::Text, g->axisLabelsColor(i));
				scale->setPalette(pal);
				setAxisFont (i, g->axisFont(i));

				QwtText src_axis_title = g->axisTitle(i);
				QwtText title = scale->title();
				title.setText(src_axis_title.text());
				title.setColor(src_axis_title.color());
				title.setFont (src_axis_title.font());
				title.setRenderFlags(src_axis_title.renderFlags());
 				scale->setTitle(title);
			}
		} else
			enableAxis(i, false);
	}

	grid()->copy(g->grid());
	((QwtPlot *)this)->setTitle (g->title());
	setCanvasFrame(g->canvasFrameWidth(), g->canvasFrameColor());
	setAxesLinewidth(g->axesLinewidth());
	removeLegend();

	QList<QwtPlotItem *> curvesList = g->curvesList();
    foreach (QwtPlotItem *it, curvesList){
        if (it->rtti() == QwtPlotItem::Rtti_PlotCurve){
  	        DataCurve *cv = (DataCurve *)it;
			int n = cv->dataSize();
			int style = ((PlotCurve *)it)->type();
			QVector<double> x(n);
			QVector<double> y(n);
			for (int j=0; j<n; j++){
				x[j]=cv->x(j);
				y[j]=cv->y(j);
			}

			PlotCurve *c = 0;
			if (style == Pie){
				c = new QwtPieCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
			} else if (style == Function) {
				c = new FunctionCurve(cv->title().text());
				insertCurve(c);
				((FunctionCurve*)c)->copy((FunctionCurve*)cv);
			} else if (style == VerticalBars || style == HorizontalBars) {
				c = new QwtBarCurve(((QwtBarCurve*)cv)->orientation(), cv->table(), cv->xColumnName(),
									cv->title().text(), cv->startRow(), cv->endRow());
                insertCurve(c);
				((QwtBarCurve*)c)->copy((const QwtBarCurve*)cv);
			} else if (style == ErrorBars) {
				QwtErrorPlotCurve *er = (QwtErrorPlotCurve*)cv;
				DataCurve *master_curve = masterCurve(er);
				if (master_curve) {
					c = new QwtErrorPlotCurve(cv->table(), cv->title().text());
					insertCurve(c);
					((QwtErrorPlotCurve*)c)->copy(er);
					((QwtErrorPlotCurve*)c)->setMasterCurve(master_curve);
				}
			} else if (style == Histogram) {
			    QwtHistogram *h = (QwtHistogram*)cv;
				if (h->matrix())
					c = new QwtHistogram(h->matrix());
				else
					c = new QwtHistogram(cv->table(), cv->xColumnName(), cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
				((QwtHistogram *)c)->copy(h);
			} else if (style == VectXYXY || style == VectXYAM) {
				VectorCurve::VectorStyle vs = VectorCurve::XYXY;
				if (style == VectXYAM)
					vs = VectorCurve::XYAM;
				c = new VectorCurve(vs, cv->table(), cv->xColumnName(), cv->title().text(),
									((VectorCurve *)cv)->vectorEndXAColName(),
									((VectorCurve *)cv)->vectorEndYMColName(),
									cv->startRow(), cv->endRow());
                insertCurve(c);
				((VectorCurve *)c)->copy((const VectorCurve *)cv);
			} else if (style == Box) {
				c = new BoxCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
				insertCurve(c);
				((BoxCurve*)c)->copy((const BoxCurve *)cv);
				QwtSingleArrayData dat(x[0], y, n);
				c->setData(dat);
			} else {
				c = new DataCurve(cv->table(), cv->xColumnName(), cv->title().text(), cv->startRow(), cv->endRow());
                insertCurve(c);
			}
			if (c->type() != Box && c->type() != ErrorBars){
				c->setData(x.data(), y.data(), n);
				if (c->type() != Function && c->type() != Pie)
                    ((DataCurve *)c)->clone(cv);
                else if (c->type() == Pie)
                    ((QwtPieCurve*)c)->clone((QwtPieCurve*)cv);
			}

			c->setPen(cv->pen());
			c->setBrush(cv->brush());
			c->setStyle(cv->style());
			c->setSymbol(cv->symbol());

			if (cv->testCurveAttribute (QwtPlotCurve::Fitted))
				c->setCurveAttribute(QwtPlotCurve::Fitted, true);
			else if (cv->testCurveAttribute (QwtPlotCurve::Inverted))
				c->setCurveAttribute(QwtPlotCurve::Inverted, true);

			c->setAxis(cv->xAxis(), cv->yAxis());
			c->setVisible(cv->isVisible());
			c->setPlotStyle(((PlotCurve *)it)->plotStyle());

			QList<QwtPlotCurve *>lst = g->fitCurvesList();
			if (lst.contains((QwtPlotCurve *)it))
				d_fit_curves << c;
		} else if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram){
  	     	Spectrogram *sp = ((Spectrogram *)it)->copy();
  	        insertCurve(sp);

  	        sp->showColorScale(((Spectrogram *)it)->colorScaleAxis(), ((Spectrogram *)it)->hasColorScale());
  	        sp->setColorBarWidth(((Spectrogram *)it)->colorBarWidth());
			sp->setVisible(it->isVisible());
  	        }
  	    }

	for (int i=0; i<QwtPlot::axisCnt; i++){
		QwtScaleWidget *sc = g->axisWidget(i);
		if (!sc)
			continue;

		ScaleDraw *sdg = (ScaleDraw *)g->axisScaleDraw (i);
		if (sdg->hasComponent(QwtAbstractScaleDraw::Labels))
		{
			ScaleDraw::ScaleType type = sdg->scaleType();
			if (type == ScaleDraw::Numeric)
				setLabelsNumericFormat(i, g->axisLabelFormat(i), g->axisLabelPrecision(i), sdg->formula());
			else if (type == ScaleDraw::Day)
				setLabelsDayFormat(i, sdg->nameFormat());
			else if (type == ScaleDraw::Month)
				setLabelsMonthFormat(i, sdg->nameFormat());
			else if (type == ScaleDraw::Time || type == ScaleDraw::Date)
				setLabelsDateTimeFormat(i, type, sdg->formatString());
			else{
				ScaleDraw *sd = (ScaleDraw *)g->axisScaleDraw(i);
				setAxisScaleDraw(i, new ScaleDraw(this, sd->labelsList(), sd->formatString(), sd->scaleType()));
			}
		} else {
			ScaleDraw *sd = (ScaleDraw *)axisScaleDraw (i);
			sd->enableComponent (QwtAbstractScaleDraw::Labels, false);
		}
	}
	for (int i=0; i<QwtPlot::axisCnt; i++){//set same scales
		const ScaleEngine *se = (ScaleEngine *)g->axisScaleEngine(i);
		if (!se)
			continue;

        ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(i);
        sc_engine->clone(se);

		int majorTicks = g->axisMaxMajor(i);
  	    int minorTicks = g->axisMaxMinor(i);
  	    setAxisMaxMajor (i, majorTicks);
  	    setAxisMaxMinor (i, minorTicks);

        double step = g->axisStep(i);
		d_user_step[i] = step;
		const QwtScaleDiv *sd = g->axisScaleDiv(i);
		QwtScaleDiv div = sc_engine->divideScale (QMIN(sd->lBound(), sd->hBound()),
				QMAX(sd->lBound(), sd->hBound()), majorTicks, minorTicks, step);

		if (se->testAttribute(QwtScaleEngine::Inverted))
			div.invert();
		setAxisScaleDiv (i, div);
	}

	drawAxesBackbones(g->drawAxesBackbone);
	setMajorTicksType(g->getMajorTicksType());
	setMinorTicksType(g->getMinorTicksType());
	setTicksLength(g->minorTickLength(), g->majorTickLength());

	setAxisLabelRotation(QwtPlot::xBottom, g->labelsRotation(QwtPlot::xBottom));
  	setAxisLabelRotation(QwtPlot::xTop, g->labelsRotation(QwtPlot::xTop));

    updateLayout();
	d_auto_scale = g->isAutoscalingEnabled();
	
	d_zoomer[0]->setZoomBase();
	d_zoomer[1]->setZoomBase();

	QList<FrameWidget *> enrichements = g->enrichementsList();
	foreach (FrameWidget *e, enrichements){
		LegendWidget *l = qobject_cast<LegendWidget *>(e);
		if (l && l == g->legend())
			d_legend = addText(l);
		else
			add(e);
	}

	QList<QwtPlotMarker *> lines = g->linesList();
	foreach (QwtPlotMarker *i, lines)
		addArrow((ArrowMarker*)i);

	setAntialiasing(g->antialiasing(), true);
}

void Graph::plotBoxDiagram(Table *w, const QStringList& names, int startRow, int endRow)
{
	if (endRow < 0)
		endRow = w->numRows() - 1;

	for (int j = 0; j <(int)names.count(); j++){
        BoxCurve *c = new BoxCurve(w, names[j], startRow, endRow);
		insertCurve(c);

        c->setData(QwtSingleArrayData(double(j+1), QwtArray<double>(), 0));
        c->loadData();
        c->setPen(QPen(ColorBox::color(j), 1));
        c->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, QBrush(), QPen(ColorBox::color(j), 1), QSize(7,7)));
	}

	if (d_legend)
		d_legend->setText(legendText());

	setAxisScaleDraw (QwtPlot::xBottom, new ScaleDraw(this, w->selectedYLabels(), w->objectName(), ScaleDraw::ColHeader));
	setAxisMaxMajor(QwtPlot::xBottom, names.count()+1);
	setAxisMaxMinor(QwtPlot::xBottom, 0);

	setAxisScaleDraw (QwtPlot::xTop, new ScaleDraw(this, w->selectedYLabels(), w->objectName(), ScaleDraw::ColHeader));
	setAxisMaxMajor(QwtPlot::xTop, names.count()+1);
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

    if(d_active_tool)
        delete d_active_tool;

    d_active_tool = tool;
}

void Graph::disableTools()
{
	if (zoomOn())
		zoom(false);
	if (drawLineActive())
		drawLine(false);

    if(d_active_tool)
        delete d_active_tool;
    d_active_tool = NULL;

	if (d_peak_fit_tool)
		delete d_peak_fit_tool;
	d_peak_fit_tool = NULL;

	if (d_range_selector)
		delete d_range_selector;
	d_range_selector = NULL;
}

bool Graph::enableRangeSelectors(const QObject *status_target, const char *status_slot)
{
	if (d_range_selector){
		delete d_range_selector;
		d_range_selector = NULL;
	}
	d_range_selector = new RangeSelectorTool(this, status_target, status_slot);
	setActiveTool(d_range_selector);
	connect(d_range_selector, SIGNAL(changed()), this, SIGNAL(dataRangeChanged()));
	return true;
}

void Graph::guessUniqueCurveLayout(int& colorIndex, int& symbolIndex)
{
	colorIndex = 0;
	symbolIndex = 0;

	int curve_index = d_curves.size() - 1;
	if (curve_index >= 0){// find out the pen color of the master curve
		PlotCurve *c = (PlotCurve *)curve(curve_index);
		if (c && c->type() == ErrorBars){
			QwtErrorPlotCurve *er = (QwtErrorPlotCurve *)c;
			DataCurve *master_curve = er->masterCurve();
			if (master_curve){
				colorIndex = ColorBox::colorIndex(master_curve->pen().color());
				return;
			}
		}
	}

	foreach (QwtPlotItem *it, d_curves){
		if (it->rtti() == QwtPlotItem::Rtti_PlotCurve){
			const QwtPlotCurve *c = (QwtPlotCurve *)it;
			int index = ColorBox::colorIndex(c->pen().color());
			if (index > colorIndex)
				colorIndex = index;

			QwtSymbol symb = c->symbol();
			index = SymbolBox::symbolIndex(symb.style());
			if (index > symbolIndex)
				symbolIndex = index;
		}
	}
	if (d_curves.size() > 1)
		colorIndex = (++colorIndex)%16;
	if (colorIndex == 13) //avoid white invisible curves
		colorIndex = 0;

	symbolIndex = (++symbolIndex)%15;
	if (!symbolIndex)
		symbolIndex = 1;
}

void Graph::addFitCurve(QwtPlotCurve *c)
{
	if (c)
		d_fit_curves << c;
}

void Graph::deleteFitCurves()
{
	foreach(QwtPlotCurve *c, d_fit_curves)
		removeCurve(curveIndex(c));

	replot();
}

Spectrogram* Graph::plotSpectrogram(Matrix *m, CurveType type)
{
	if (type != GrayScale && type != ColorMap && type != Contour)
  		return 0;

  	Spectrogram *d_spectrogram = new Spectrogram(m);
  	if (type == GrayScale)
  		d_spectrogram->setGrayScale();
  	else if (type == Contour){
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, false);
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    } else if (type == ColorMap){
  	    d_spectrogram->setDefaultColorMap();
  	    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
	}

  	insertCurve(d_spectrogram);

  	QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
  	rightAxis->setColorBarEnabled(type != Contour);
  	rightAxis->setColorMap(d_spectrogram->data().range(), d_spectrogram->colorMap());

  	setAxisScale(QwtPlot::yRight,
  	d_spectrogram->data().range().minValue(),
  	d_spectrogram->data().range().maxValue());
  	enableAxis(QwtPlot::yRight, type != Contour);

  	replot();
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

  	Spectrogram *sp = new Spectrogram(m);
	insertCurve(sp);

  	for (line++; line != lst.end(); line++)
    {
        QString s = *line;
        if (s.contains("<ColorPolicy>"))
        {
            int color_policy = s.remove("<ColorPolicy>").remove("</ColorPolicy>").stripWhiteSpace().toInt();
            if (color_policy == Spectrogram::GrayScale)
                sp->setGrayScale();
            else if (color_policy == Spectrogram::Default)
                sp->setDefaultColorMap();
        }
        else if (s.contains("<ColorMap>"))
        {
            s = *(++line);
            int mode = s.remove("<Mode>").remove("</Mode>").stripWhiteSpace().toInt();
            s = *(++line);
            QColor color1 = QColor(s.remove("<MinColor>").remove("</MinColor>").stripWhiteSpace());
            s = *(++line);
            QColor color2 = QColor(s.remove("<MaxColor>").remove("</MaxColor>").stripWhiteSpace());

            QwtLinearColorMap colorMap = QwtLinearColorMap(color1, color2);
            colorMap.setMode((QwtLinearColorMap::Mode)mode);

            s = *(++line);
            int stops = s.remove("<ColorStops>").remove("</ColorStops>").stripWhiteSpace().toInt();
            for (int i = 0; i < stops; i++)
            {
                s = (*(++line)).stripWhiteSpace();
                QStringList l = QStringList::split("\t", s.remove("<Stop>").remove("</Stop>"));
                colorMap.addColorStop(l[0].toDouble(), QColor(l[1]));
            }
            sp->setCustomColorMap(colorMap);
            line++;
        }
        else if (s.contains("<Image>"))
        {
            int mode = s.remove("<Image>").remove("</Image>").stripWhiteSpace().toInt();
            sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, mode);
        }
        else if (s.contains("<ContourLines>"))
        {
            int contours = s.remove("<ContourLines>").remove("</ContourLines>").stripWhiteSpace().toInt();
            sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, contours);
            if (contours)
            {
                s = (*(++line)).stripWhiteSpace();
                int levels = s.remove("<Levels>").remove("</Levels>").toInt();
                sp->setLevelsNumber(levels);

                s = (*(++line)).stripWhiteSpace();
                int defaultPen = s.remove("<DefaultPen>").remove("</DefaultPen>").toInt();
                if (!defaultPen)
                    sp->setDefaultContourPen(Qt::NoPen);
                else
                {
                    s = (*(++line)).stripWhiteSpace();
                    QColor c = QColor(s.remove("<PenColor>").remove("</PenColor>"));
                    s = (*(++line)).stripWhiteSpace();
                    double width = s.remove("<PenWidth>").remove("</PenWidth>").toDouble();
                    s = (*(++line)).stripWhiteSpace();
                    int style = s.remove("<PenStyle>").remove("</PenStyle>").toInt();
                    sp->setDefaultContourPen(QPen(c, width, Graph::getPenStyle(style)));
                }
            }
        }
        else if (s.contains("<ColorBar>"))
        {
            s = *(++line);
            int color_axis = s.remove("<axis>").remove("</axis>").stripWhiteSpace().toInt();
            s = *(++line);
            int width = s.remove("<width>").remove("</width>").stripWhiteSpace().toInt();

            QwtScaleWidget *colorAxis = axisWidget(color_axis);
            if (colorAxis)
            {
                colorAxis->setColorBarWidth(width);
                colorAxis->setColorBarEnabled(true);
            }
            line++;
        }
		else if (s.contains("<Visible>"))
        {
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

    QString labelsColumn = QString();
    int xoffset = 0, yoffset = 0;
  	QStringList::const_iterator line = lst.begin();
  	QString s = *line;
    if (s.contains("<column>"))
        labelsColumn = s.remove("<column>").remove("</column>").trimmed();

  	for (line++; line != lst.end(); line++){
        s = *line;
        if (s.contains("<color>"))
            c->setLabelsColor(QColor(s.remove("<color>").remove("</color>").trimmed()));
        else if (s.contains("<whiteOut>"))
            c->setLabelsWhiteOut(s.remove("<whiteOut>").remove("</whiteOut>").toInt());
        else if (s.contains("<font>")){
            QStringList fontList = s.remove("<font>").remove("</font>").trimmed().split("\t");
            QFont font = QFont(fontList[0], fontList[1].toInt());
            if (fontList.count() >= 3)
                font.setBold(fontList[2].toInt());
            if (fontList.count() >= 4)
                font.setItalic(fontList[3].toInt());
            if (fontList.count() >= 5)
                font.setUnderline(fontList[4].toInt());
            c->setLabelsFont(font);
        } else if (s.contains("<angle>"))
            c->setLabelsRotation(s.remove("<angle>").remove("</angle>").toDouble());
        else if (s.contains("<justify>"))
            c->setLabelsAlignment(s.remove("<justify>").remove("</justify>").toInt());
        else if (s.contains("<xoffset>"))
            xoffset = s.remove("<xoffset>").remove("</xoffset>").toInt();
        else if (s.contains("<yoffset>"))
            yoffset = s.remove("<yoffset>").remove("</yoffset>").toInt();
    }
    c->setLabelsOffset(xoffset, yoffset);
    c->setLabelsColumnName(labelsColumn);
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
	if(d_peak_fit_tool)
        delete d_peak_fit_tool;
	if(d_active_tool)
        delete d_active_tool;
	if (d_range_selector)
		delete d_range_selector;
	delete titlePicker;
	delete scalePicker;
	delete cp;
}

void Graph::setAntialiasing(bool on, bool update)
{
	if (d_antialiasing == on)
		return;

	d_antialiasing = on;

	if (update){
		foreach(QwtPlotItem *it, d_curves)
			it->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
		foreach (QwtPlotMarker *i, d_lines)
			i->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
		replot();
	}
}

bool Graph::focusNextPrevChild ( bool )
{
	QList<QwtPlotMarker *> lst;
	foreach(QwtPlotMarker *l, d_lines)
		lst << l;

	int markers = lst.size();
	if (markers < 1)
		return false;

	int next = 0;
	if (d_selected_marker)
		next = lst.indexOf(d_selected_marker) + 1;
	if (next >= markers)
		next = 0;

	cp->disableEditing();
	deselectMarker();
	setSelectedMarker(lst.at(next));
	return true;
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
        if (c->type() != Function && c->plotAssociation().contains(oldName))
            c->updateColumnNames(oldName, newName, updateTableName);
	}

	replot();
}

void Graph::setCurveFullRange(int curveIndex)
{
	DataCurve *c = (DataCurve *)curve(curveIndex);
	if (c)
	{
		c->setFullRange();
		updatePlot();
		emit modifiedGraph();
	}
}

DataCurve* Graph::masterCurve(QwtErrorPlotCurve *er)
{
	foreach(QwtPlotItem *it, d_curves){
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (((PlotCurve *)it)->type() == Function)
            continue;

        if (((DataCurve *)it)->plotAssociation() == er->masterCurve()->plotAssociation())
			return (DataCurve *)it;
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
	} else if (d_active_enrichement){
		LegendWidget *l = qobject_cast<LegendWidget *>(d_active_enrichement);
		if (l){
			l->setFont(f);
			l->repaint();
			emit modifiedGraph();
		}
	} else if (titlePicker->selected()){
		QwtText t = title();
		t.setFont(f);
		((QwtPlot *)this)->setTitle(t);
		emit modifiedGraph();
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

	int lw = lineWidth();
	if (lw){
		QColor color = palette().color(QPalette::Active, QPalette::Foreground);
		painter->setPen (QPen(color, lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
	} else
		painter->setPen(QPen(Qt::NoPen));

    painter->setBrush(paletteBackgroundColor());
    QwtPainter::drawRect(painter, rect.adjusted(0, 0, -1, -1));
	painter->restore();
}

void Graph::printCanvas(QPainter *painter, const QRect &canvasRect,
   			 const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const
{
	painter->save();

	const QwtPlotCanvas* plotCanvas = canvas();
	QRect rect = canvasRect.adjusted(1, 1, -2, -2);//FIXME: change this to canvasRect.adjusted(1, 1, -1, -1) as soon as Qt 4.4 is released!!!
    QwtPainter::fillRect(painter, rect, canvasBackground());

	painter->setClipping(true);
	QwtPainter::setClipRect(painter, rect);

    drawItems(painter, canvasRect, map, pfilter);
    painter->restore();

    painter->save();
	int lw = plotCanvas->lineWidth();
	if(lw > 0){
		QColor color = plotCanvas->palette().color(QPalette::Active, QColorGroup::Foreground);
		painter->setPen (QPen(color, lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		QwtPainter::drawRect(painter, canvasRect.adjusted(0, 0, -1, -1));
	}
    painter->restore();

	foreach(FrameWidget *f, d_enrichements){
		if (f->isVisible())
			f->print(painter, map);
	}
}

void Graph::drawItems (QPainter *painter, const QRect &rect,
			const QwtScaleMap map[axisCnt], const QwtPlotPrintFilter &pfilter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    for (int i=0; i<QwtPlot::axisCnt; i++){
		if (!axisEnabled(i))
			continue;
        drawBreak(painter, rect, map[i], i);
    }
    painter->restore();

    for (int i=0; i<QwtPlot::axisCnt; i++){
		if (!axisEnabled(i))
			continue;

		ScaleEngine *sc_engine = (ScaleEngine *)axisScaleEngine(i);
		if (!sc_engine->hasBreak())
			continue;

		QwtScaleMap m = map[i];
		int lb = m.transform(sc_engine->axisBreakLeft());
		int rb = m.transform(sc_engine->axisBreakRight());
		int start = lb, end = rb;
		if (sc_engine->testAttribute(QwtScaleEngine::Inverted)){
			end = lb;
			start = rb;
		}
		QRegion cr(rect);
		if (i == QwtPlot::xBottom || i == QwtPlot::xTop)
			painter->setClipRegion(cr.subtracted(QRegion(start, rect.y(), abs(end - start), rect.height())), Qt::IntersectClip);
		else if (i == QwtPlot::yLeft || i == QwtPlot::yRight)
			painter->setClipRegion(cr.subtracted(QRegion(rect.x(), end, rect.width(), abs(end - start))), Qt::IntersectClip);
	}

	QwtPlot::drawItems(painter, rect, map, pfilter);

	for (int i=0; i<QwtPlot::axisCnt; i++){
		if (!axisEnabled(i))
			continue;

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
	int x1=rect.left();
	int x2=rect.right();
	int y1=rect.top();
	int y2=rect.bottom();

	QPalette pal=axisWidget(axis)->palette();
	QColor color=pal.color(QPalette::Active, QColorGroup::Foreground);

	painter->save();
	painter->setPen(QPen(color, axesLinewidth(), Qt::SolidLine));

	QwtScaleDiv *scDiv=(QwtScaleDiv *)axisScaleDiv(axis);
	const QwtValueList minTickList = scDiv->ticks(QwtScaleDiv::MinorTick);
	int minTicks = (int)minTickList.count();

	const QwtValueList medTickList = scDiv->ticks(QwtScaleDiv::MediumTick);
	int medTicks = (int)medTickList.count();

	const QwtValueList majTickList = scDiv->ticks(QwtScaleDiv::MajorTick);
	int majTicks = (int)majTickList.count();

	int j, x, y, low,high;
	switch (axis)
	{
		case QwtPlot::yLeft:
			x=x1;
			low=y1+d_maj_tick_length;
			high=y2-d_maj_tick_length;
			if (min){
				for (j = 0; j < minTicks; j++){
					y = map.transform(minTickList[j]);
					if (y>low && y< high)
						QwtPainter::drawLine(painter, x, y, x+d_min_tick_length, y);
				}
				for (j = 0; j < medTicks; j++){
					y = map.transform(medTickList[j]);
					if (y>low && y< high)
						QwtPainter::drawLine(painter, x, y, x+d_min_tick_length, y);
				}
			}

			if (maj){
				for (j = 0; j < majTicks; j++){
					y = map.transform(majTickList[j]);
					if (y>low && y< high)
						QwtPainter::drawLine(painter, x, y, x+d_maj_tick_length, y);
				}
			}
			break;

		case QwtPlot::yRight:
			{
				x=x2;
				low=y1+d_maj_tick_length;
				high=y2-d_maj_tick_length;
				if (min){
					for (j = 0; j < minTicks; j++){
						y = map.transform(minTickList[j]);
						if (y>low && y< high)
							QwtPainter::drawLine(painter, x+1, y, x-d_min_tick_length, y);
					}
					for (j = 0; j < medTicks; j++){
						y = map.transform(medTickList[j]);
						if (y>low && y< high)
							QwtPainter::drawLine(painter, x+1, y, x-d_min_tick_length, y);
					}
				}

				if (maj){
					for (j = 0; j <majTicks; j++){
						y = map.transform(majTickList[j]);
						if (y>low && y< high)
							QwtPainter::drawLine(painter, x+1, y, x-d_maj_tick_length, y);
					}
				}
			}
			break;

		case QwtPlot::xBottom:
			y=y2;
			low=x1+d_maj_tick_length;
			high=x2-d_maj_tick_length;
			if (min){
				for (j = 0; j < minTicks; j++){
					x = map.transform(minTickList[j]);
					if (x>low && x<high)
						QwtPainter::drawLine(painter, x, y+1, x, y-d_min_tick_length);
				}
				for (j = 0; j < medTicks; j++){
					x = map.transform(medTickList[j]);
					if (x>low && x<high)
						QwtPainter::drawLine(painter, x, y+1, x, y-d_min_tick_length);
				}
			}

			if (maj){
				for (j = 0; j < majTicks; j++){
					x = map.transform(majTickList[j]);
					if (x>low && x<high)
						QwtPainter::drawLine(painter, x, y+1, x, y-d_maj_tick_length);
				}
			}
			break;

		case QwtPlot::xTop:
			y=y1;
			low=x1+d_maj_tick_length;
			high=x2-d_maj_tick_length;

			if (min){
				for (j = 0; j < minTicks; j++){
					x = map.transform(minTickList[j]);
					if (x>low && x<high)
						QwtPainter::drawLine(painter, x, y, x, y + d_min_tick_length);
				}
				for (j = 0; j < medTicks; j++){
					x = map.transform(medTickList[j]);
					if (x>low && x<high)
						QwtPainter::drawLine(painter, x, y, x, y + d_min_tick_length);
				}
			}

			if (maj){
				for (j = 0; j <majTicks; j++){
					x = map.transform(majTickList[j]);
					if (x>low && x<high)
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
	if (d_maj_tick_length == majLength &&
			d_min_tick_length == minLength)
		return;

	d_maj_tick_length = majLength;
	d_min_tick_length = minLength;
}

PlotCurve* Graph::closestCurve(int xpos, int ypos, int &dist, int &point)
{
	QwtScaleMap map[QwtPlot::axisCnt];
	for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
		map[axis] = canvasMap(axis);

	double dmin = 1.0e10;
	PlotCurve *curve = NULL;
	foreach (QwtPlotItem *item, d_curves){
		if(item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram){
			PlotCurve *c = (PlotCurve *)item;
			if (c->type() != Graph::Function && ((DataCurve *)c)->hasLabels() &&
                ((DataCurve *)c)->selectedLabels(QPoint(xpos, ypos))){
                dist = 0;
			    return c;
            } else
                ((DataCurve *)c)->setLabelsSelected(false);

			for (int i=0; i<c->dataSize(); i++){
				double cx = map[c->xAxis()].xTransform(c->x(i)) - double(xpos);
				double cy = map[c->yAxis()].xTransform(c->y(i)) - double(ypos);
				double f = qwtSqr(cx) + qwtSqr(cy);
				if (f < dmin && c->type() != Graph::ErrorBars){
					dmin = f;
					curve = c;
					point = i;
				}
			}
		}
	}
	dist = qRound(sqrt(dmin));
	return curve;
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

	if (c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
		((QwtPlotCurve *)c)->setPaintAttribute(QwtPlotCurve::PaintFiltered);

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

    for (int axisId = 0; axisId < axisCnt; axisId++ ){
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
void Graph::print(QPainter *painter, const QRect &plotRect,
        const QwtPlotPrintFilter &pfilter)
{
    int axisId;

    if ( painter == 0 || !painter->isActive() ||
            !plotRect.isValid() || size().isNull() )
       return;

    QwtText t = title();
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

    int layoutOptions = QwtPlotLayout::IgnoreScrollbars
        | QwtPlotLayout::IgnoreFrames;
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintMargin) )
        layoutOptions |= QwtPlotLayout::IgnoreMargin;
    if ( !(pfilter.options() & QwtPlotPrintFilter::PrintLegend) )
        layoutOptions |= QwtPlotLayout::IgnoreLegend;

    ((QwtPlot *)this)->plotLayout()->activate(this,
        QwtPainter::metricsMap().deviceToLayout(plotRect),
        layoutOptions);

    if ((pfilter.options() & QwtPlotPrintFilter::PrintTitle)
        && (!titleLabel()->text().isEmpty())){
        printTitle(painter, plotLayout()->titleRect());
    }

    QRect canvasRect = plotLayout()->canvasRect();

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
                        scaleRect.translate(0, canvasRect.bottom() - scaleRect.top());
                    break;
                    case xTop:
                        scaleRect.translate(0, canvasRect.top() - scaleRect.bottom());
                    break;
                    case yLeft:
                        scaleRect.translate(canvasRect.left() - scaleRect.right(), 0);
                    break;
                    case yRight:
                        scaleRect.translate(canvasRect.right() - scaleRect.left(), 0);
                    break;
                }
            }
            printScale(painter, axisId, startDist, endDist, baseDist, scaleRect);
        }
    }

	/*
       The border of the bounding rect needs to ba scaled to
       layout coordinates, so that it is aligned to the axes
     */
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
        map[axisId].setScaleInterval(scaleDiv.lBound(), scaleDiv.hBound());

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
            const int margin = plotLayout()->canvasMargin(axisId);
            if ( axisId == yLeft || axisId == yRight ){
                from = metricsMap.layoutToDeviceX(canvasRect.bottom() - margin);
                to = metricsMap.layoutToDeviceX(canvasRect.top() + margin);
            } else {
                from = metricsMap.layoutToDeviceY(canvasRect.left() + margin);
                to = metricsMap.layoutToDeviceY(canvasRect.right() - margin);
            }
        }
        map[axisId].setPaintXInterval(from, to);
    }

    // The canvas maps are already scaled.
    QwtPainter::setMetricsMap(painter->device(), painter->device());
    printCanvas(painter, canvasRect, map, pfilter);
    QwtPainter::resetMetricsMap();

    ((QwtPlot *)this)->plotLayout()->invalidate();

    // reset all widgets with their original attributes.
    if ( pfilter.options() & QwtPlotPrintFilter::PrintFrameWithScales ){
        // restore the previous base line dists
        for (axisId = 0; axisId < QwtPlot::axisCnt; axisId++ ){
            QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(axisId);
            if ( scaleWidget  )
                scaleWidget->setMargin(baseLineDists[axisId]);
        }
    }

    pfilter.reset((QwtPlot *)this);
    painter->restore();
    ((QwtPlot *)this)->setTitle(t);//hack used to avoid bug in Qwt::printTitle(): the title attributes are overwritten
}

TexWidget* Graph::addTexFormula(const QString& s, const QPixmap& pix)
{
	TexWidget *t = new TexWidget(this, s, pix);
	d_enrichements << t;
	emit modifiedGraph();
	return t;
}

FrameWidget* Graph::add(FrameWidget* fw)
{
	if (!fw)
		return NULL;

	LegendWidget *l = qobject_cast<LegendWidget *>(fw);
	if (l){
		LegendWidget* aux = new LegendWidget(this);
		aux->clone(l);
	}

	TexWidget *t = qobject_cast<TexWidget *>(fw);
	if (t){
		TexWidget* aux = new TexWidget(this);
		aux->clone(t);
	}

	ImageWidget *i = qobject_cast<ImageWidget *>(fw);
	if (i){
		ImageWidget* aux = new ImageWidget(this);
		aux->clone(i);
	}
	
	d_enrichements << fw;
	d_active_enrichement = fw;
	return d_active_enrichement;
}

LegendWidget* Graph::activeText()
{
	return qobject_cast<LegendWidget *>(d_active_enrichement);
}
