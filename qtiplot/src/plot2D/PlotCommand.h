/***************************************************************************
    File                 : PlotCommand.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Description          : Plot and Graph undo/redo commands
 ***************************************************************************/

#ifndef PLOT_COMMAND_H
#define PLOT_COMMAND_H

#include <QUndoCommand>
#include <QPointer>
#include <QString>
#include <QColor>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QSize>
#include <qwt_symbol.h>
#include "Graph.h"

//! Undo command for modifying axis scales
class PlotSetScaleCommand : public QUndoCommand
{
public:
	PlotSetScaleCommand(Graph *g, int axis,
						double oldStart, double oldEnd, double oldStep,
						int oldMajTicks, int oldMinTicks, int oldType, bool oldInverted,
						double newStart, double newEnd, double newStep,
						int newMajTicks, int newMinTicks, int newType, bool newInverted,
						const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_axis;
	double d_old_start, d_old_end, d_old_step;
	int d_old_maj_ticks, d_old_min_ticks, d_old_type;
	bool d_old_inverted;

	double d_new_start, d_new_end, d_new_step;
	int d_new_maj_ticks, d_new_min_ticks, d_new_type;
	bool d_new_inverted;
};

//! Undo command for modifying axis titles
class PlotSetAxisTitleCommand : public QUndoCommand
{
public:
	PlotSetAxisTitleCommand(Graph *g, int axis, const QString &oldTitle, const QString &newTitle, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_axis;
	QString d_old_title, d_new_title;
};

//! Undo command for modifying axis title fonts
class PlotSetAxisFontCommand : public QUndoCommand
{
public:
	PlotSetAxisFontCommand(Graph *g, int axis, const QFont &oldFont, const QFont &newFont, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_axis;
	QFont d_old_font, d_new_font;
};

//! Undo command for modifying axis title colors
class PlotSetAxisColorCommand : public QUndoCommand
{
public:
	PlotSetAxisColorCommand(Graph *g, int axis, const QColor &oldColor, const QColor &newColor, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_axis;
	QColor d_old_color, d_new_color;
};

//! Undo command for modifying canvas background colors
class PlotSetCanvasColorCommand : public QUndoCommand
{
public:
	PlotSetCanvasColorCommand(Graph *g, const QColor &oldColor, const QColor &newColor, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	QColor d_old_color, d_new_color;
};

//! Undo command for showing/hiding grid
class PlotShowGridCommand : public QUndoCommand
{
public:
	PlotShowGridCommand(Graph *g, int axis, bool oldMajor, bool oldMinor, bool newMajor, bool newMinor, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_axis;
	bool d_old_major, d_old_minor;
	bool d_new_major, d_new_minor;
};

//! Undo command for modifying curve pen (color, width, style)
class PlotSetCurvePenCommand : public QUndoCommand
{
public:
	PlotSetCurvePenCommand(Graph *g, int curveIndex, const QPen &oldPen, const QPen &newPen, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_curve_index;
	QPen d_old_pen, d_new_pen;
};

//! Undo command for modifying curve brush (fill color, pattern)
class PlotSetCurveBrushCommand : public QUndoCommand
{
public:
	PlotSetCurveBrushCommand(Graph *g, int curveIndex, const QBrush &oldBrush, const QBrush &newBrush, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_curve_index;
	QBrush d_old_brush, d_new_brush;
};

//! Undo command for modifying curve symbol
class PlotSetCurveSymbolCommand : public QUndoCommand
{
public:
	PlotSetCurveSymbolCommand(Graph *g, int curveIndex,
							  QwtSymbol::Style oldStyle, const QBrush &oldBrush, const QPen &oldPen, const QSize &oldSize,
							  QwtSymbol::Style newStyle, const QBrush &newBrush, const QPen &newPen, const QSize &newSize,
							  const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	int d_curve_index;
	QwtSymbol::Style d_old_style, d_new_style;
	QBrush d_old_brush, d_new_brush;
	QPen d_old_pen, d_new_pen;
	QSize d_old_size, d_new_size;
};

//! Undo command for modifying plot title
class PlotSetPlotTitleCommand : public QUndoCommand
{
public:
	PlotSetPlotTitleCommand(Graph *g, const QwtText &oldTitle, const QwtText &newTitle, const QString &text = QString());

	void redo() override;
	void undo() override;

private:
	QPointer<Graph> d_graph;
	QwtText d_old_title, d_new_title;
};

#endif
