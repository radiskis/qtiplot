/***************************************************************************
    File                 : PlotCommand.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Plot and Graph undo/redo commands
 ***************************************************************************/

#include "PlotCommand.h"
#include "PlotCurve.h"
#include "MultiLayer.h"

/*************************************************************************/
/*           Class PlotSetScaleCommand                                   */
/*************************************************************************/
PlotSetScaleCommand::PlotSetScaleCommand(Graph *g, int axis,
										double oldStart, double oldEnd, double oldStep,
										int oldMajTicks, int oldMinTicks, int oldType, bool oldInverted,
										double newStart, double newEnd, double newStep,
										int newMajTicks, int newMinTicks, int newType, bool newInverted,
										const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Scale") : text),
  d_graph(g), d_axis(axis),
  d_old_start(oldStart), d_old_end(oldEnd), d_old_step(oldStep),
  d_old_maj_ticks(oldMajTicks), d_old_min_ticks(oldMinTicks), d_old_type(oldType), d_old_inverted(oldInverted),
  d_new_start(newStart), d_new_end(newEnd), d_new_step(newStep),
  d_new_maj_ticks(newMajTicks), d_new_min_ticks(newMinTicks), d_new_type(newType), d_new_inverted(newInverted)
{
}

void PlotSetScaleCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setScale(d_axis, d_new_start, d_new_end, d_new_step, d_new_maj_ticks, d_new_min_ticks, d_new_type, d_new_inverted);
	d_graph->replot();
}

void PlotSetScaleCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setScale(d_axis, d_old_start, d_old_end, d_old_step, d_old_maj_ticks, d_old_min_ticks, d_old_type, d_old_inverted);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotSetAxisTitleCommand                               */
/*************************************************************************/
PlotSetAxisTitleCommand::PlotSetAxisTitleCommand(Graph *g, int axis, const QString &oldTitle, const QString &newTitle, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Axis Title") : text),
  d_graph(g), d_axis(axis), d_old_title(oldTitle), d_new_title(newTitle)
{
}

void PlotSetAxisTitleCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitle(d_axis, d_new_title);
	d_graph->replot();
}

void PlotSetAxisTitleCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitle(d_axis, d_old_title);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotSetAxisFontCommand                                */
/*************************************************************************/
PlotSetAxisFontCommand::PlotSetAxisFontCommand(Graph *g, int axis, const QFont &oldFont, const QFont &newFont, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Axis Font") : text),
  d_graph(g), d_axis(axis), d_old_font(oldFont), d_new_font(newFont)
{
}

void PlotSetAxisFontCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitleFont(d_axis, d_new_font);
	d_graph->replot();
}

void PlotSetAxisFontCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitleFont(d_axis, d_old_font);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotSetAxisColorCommand                               */
/*************************************************************************/
PlotSetAxisColorCommand::PlotSetAxisColorCommand(Graph *g, int axis, const QColor &oldColor, const QColor &newColor, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Axis Color") : text),
  d_graph(g), d_axis(axis), d_old_color(oldColor), d_new_color(newColor)
{
}

void PlotSetAxisColorCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitleColor(d_axis, d_new_color);
	d_graph->replot();
}

void PlotSetAxisColorCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setAxisTitleColor(d_axis, d_old_color);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotSetCanvasColorCommand                             */
/*************************************************************************/
PlotSetCanvasColorCommand::PlotSetCanvasColorCommand(Graph *g, const QColor &oldColor, const QColor &newColor, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Canvas Color") : text),
  d_graph(g), d_old_color(oldColor), d_new_color(newColor)
{
}

void PlotSetCanvasColorCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setCanvasBackground(d_new_color);
	d_graph->replot();
}

void PlotSetCanvasColorCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setCanvasBackground(d_old_color);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotShowGridCommand                                   */
/*************************************************************************/
PlotShowGridCommand::PlotShowGridCommand(Graph *g, int axis, bool oldMajor, bool oldMinor, bool newMajor, bool newMinor, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Toggle Grid") : text),
  d_graph(g), d_axis(axis),
  d_old_major(oldMajor), d_old_minor(oldMinor),
  d_new_major(newMajor), d_new_minor(newMinor)
{
}

void PlotShowGridCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->showGrid(d_axis, d_new_major, d_new_minor);
	d_graph->replot();
}

void PlotShowGridCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->showGrid(d_axis, d_old_major, d_old_minor);
	d_graph->replot();
}

/*************************************************************************/
/*           Class PlotSetCurvePenCommand                                */
/*************************************************************************/
PlotSetCurvePenCommand::PlotSetCurvePenCommand(Graph *g, int curveIndex, const QPen &oldPen, const QPen &newPen, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Curve Pen") : text),
  d_graph(g), d_curve_index(curveIndex), d_old_pen(oldPen), d_new_pen(newPen)
{
}

void PlotSetCurvePenCommand::redo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setPen(d_new_pen);
		d_graph->replot();
	}
}

void PlotSetCurvePenCommand::undo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setPen(d_old_pen);
		d_graph->replot();
	}
}

/*************************************************************************/
/*           Class PlotSetCurveBrushCommand                              */
/*************************************************************************/
PlotSetCurveBrushCommand::PlotSetCurveBrushCommand(Graph *g, int curveIndex, const QBrush &oldBrush, const QBrush &newBrush, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Curve Brush") : text),
  d_graph(g), d_curve_index(curveIndex), d_old_brush(oldBrush), d_new_brush(newBrush)
{
}

void PlotSetCurveBrushCommand::redo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setBrush(d_new_brush);
		d_graph->replot();
	}
}

void PlotSetCurveBrushCommand::undo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setBrush(d_old_brush);
		d_graph->replot();
	}
}

/*************************************************************************/
/*           Class PlotSetCurveSymbolCommand                             */
/*************************************************************************/
PlotSetCurveSymbolCommand::PlotSetCurveSymbolCommand(Graph *g, int curveIndex,
													QwtSymbol::Style oldStyle, const QBrush &oldBrush, const QPen &oldPen, const QSize &oldSize,
													QwtSymbol::Style newStyle, const QBrush &newBrush, const QPen &newPen, const QSize &newSize,
													const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Curve Symbol") : text),
  d_graph(g), d_curve_index(curveIndex),
  d_old_style(oldStyle), d_old_brush(oldBrush), d_old_pen(oldPen), d_old_size(oldSize),
  d_new_style(newStyle), d_new_brush(newBrush), d_new_pen(newPen), d_new_size(newSize)
{
}

void PlotSetCurveSymbolCommand::redo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setSymbol(new QwtSymbol(d_new_style, d_new_brush, d_new_pen, d_new_size));
		d_graph->replot();
	}
}

void PlotSetCurveSymbolCommand::undo()
{
	if (!d_graph)
		return;
	PlotCurve *c = d_graph->curve(d_curve_index);
	if (c) {
		c->setSymbol(new QwtSymbol(d_old_style, d_old_brush, d_old_pen, d_old_size));
		d_graph->replot();
	}
}

/*************************************************************************/
/*           Class PlotSetPlotTitleCommand                               */
/*************************************************************************/
PlotSetPlotTitleCommand::PlotSetPlotTitleCommand(Graph *g, const QwtText &oldTitle, const QwtText &newTitle, const QString &text)
: QUndoCommand(text.isEmpty() ? QObject::tr("Set Plot Title") : text),
  d_graph(g), d_old_title(oldTitle), d_new_title(newTitle)
{
}

void PlotSetPlotTitleCommand::redo()
{
	if (!d_graph)
		return;
	d_graph->setTitle(d_new_title);
	d_graph->replot();
	d_graph->updateMarkersBoundingRect();
}

void PlotSetPlotTitleCommand::undo()
{
	if (!d_graph)
		return;
	d_graph->setTitle(d_old_title);
	d_graph->replot();
	d_graph->updateMarkersBoundingRect();
}
