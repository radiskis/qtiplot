/***************************************************************************
    File                 : SelectionMoveResizer.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, 2008 by Ion Vasilief
    Email (use @ for *)  : knut.franke*gmx.de, ion_vasilief*yahoo.fr
    Description          : Selection of Widgets and QwtPlotMarkers

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

#include "SelectionMoveResizer.h"

#include <QPainter>
#include <QMouseEvent>
#include <QCoreApplication>

#include <qwt_scale_map.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include "FrameWidget.h"
#include "LegendWidget.h"
#include "ArrowMarker.h"

SelectionMoveResizer::SelectionMoveResizer(ArrowMarker *target)
	: QWidget(target->plot()->canvas())
{
	init();
	add(target);
}

SelectionMoveResizer::SelectionMoveResizer(QWidget *target)
	: QWidget(target->parentWidget())
{
	QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(target);
	if (canvas)
		setParent(canvas->plot()->parentWidget());

	init();
	add(target);
}

void SelectionMoveResizer::init()
{
	d_bounding_rect = QRect(0, 0, -1, -1);
	d_op = None;
	d_op_start = QPoint(0, 0);
	d_op_dp = QPoint(0, 0);

    setAttribute(Qt::WA_DeleteOnClose);
	setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
	setMouseTracking(true);
	parentWidget()->installEventFilter(this);
	show();
	setFocusPolicy (Qt::StrongFocus);
	setFocus();
}

SelectionMoveResizer::~SelectionMoveResizer()
{
	foreach(QWidget *w, d_widgets){
		QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(w);
		if (canvas){
			((Graph*)canvas->parent())->raiseEnrichements();
			break;
		}
	}

	if (parentWidget())
		parentWidget()->removeEventFilter(this);
}

void SelectionMoveResizer::add(ArrowMarker *target)
{
	if ((QWidget*)target->plot()->canvas() != parent())
		return;
	d_line_markers << target;

	if (d_bounding_rect.isValid())
		d_bounding_rect |= boundingRectOf(target);
	else
		d_bounding_rect = boundingRectOf(target);

	update();
}

void SelectionMoveResizer::add(QWidget *target)
{
	//if (target->parentWidget() != parent())
		//return;

	d_widgets << target;
	target->installEventFilter(this);
	connect(target, SIGNAL(destroyed(QObject*)), this, SLOT(removeWidget(QObject*)));

	QRect r = target->frameGeometry();
	QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(target);
	if (canvas)
		r = boundingRectOf(canvas);

	if (d_bounding_rect.isValid())
		d_bounding_rect |= r;
	else
		d_bounding_rect = r;

	update();
}

QRect SelectionMoveResizer::boundingRectOf(QwtPlotMarker *target) const
{
	return ((ArrowMarker *)target)->rect();
}

QRect SelectionMoveResizer::boundingRectOf(QwtPlotCanvas *canvas) const
{
	if (!canvas)
		return QRect();

	return canvas->frameGeometry().translated(canvas->plot()->pos());
}

int SelectionMoveResizer::removeAll(ArrowMarker *target)
{
	int result = d_line_markers.removeAll(target);
	if (d_line_markers.isEmpty() && d_widgets.isEmpty())
		delete this;
	else
		recalcBoundingRect();
	return result;
}

int SelectionMoveResizer::removeAll(QWidget *target)
{
	int result = d_widgets.removeAll(target);

	FrameWidget *fw = qobject_cast<FrameWidget *>(target);
	if (fw && !fw->isOnTop())
		fw->lower();

	if (d_line_markers.isEmpty() && d_widgets.isEmpty())
		delete this;
	else
		recalcBoundingRect();
	return result;
}

void SelectionMoveResizer::raiseTargets(bool on)
{
	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw)
			fw->setOnTop(on);
	}

	if (on)
		this->raise();
}

void SelectionMoveResizer::alignTargetsLeft()
{
	if (d_widgets.isEmpty())
		return;

	int left = d_widgets.first()->x();
	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && fw->x() < left)
			left = fw->x();
	}

	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && fw->x() != left)
			fw->move(QPoint(left, fw->y()));
	}

	FrameWidget *w = qobject_cast<FrameWidget *>(d_widgets.first());
	if (w)
		w->plot()->notifyChanges();
}

void SelectionMoveResizer::alignTargetsRight()
{
	if (d_widgets.isEmpty())
		return;

	int right = d_widgets.first()->x() + d_widgets.first()->width();
	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw){
			int r = fw->x() + fw->width();
			if (r > right)
				right = r;
		}
	}

	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && (fw->x() + fw->width()) != right)
			fw->move(QPoint(right - fw->width(), fw->y()));
	}

	FrameWidget *w = qobject_cast<FrameWidget *>(d_widgets.first());
	if (w)
		w->plot()->notifyChanges();
}

void SelectionMoveResizer::alignTargetsTop()
{
	if (d_widgets.isEmpty())
		return;

	int top = d_widgets.first()->y();
	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && fw->y() < top)
			top = fw->y();
	}

	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && fw->y() != top)
			fw->move(QPoint(fw->x(), top));
	}

	FrameWidget *w = qobject_cast<FrameWidget *>(d_widgets.first());
	if (w)
		w->plot()->notifyChanges();
}

void SelectionMoveResizer::alignTargetsBottom()
{
	if (d_widgets.isEmpty())
		return;

	int bottom = d_widgets.first()->y() + d_widgets.first()->height();
	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw){
			int b = fw->y() + fw->height();
			if (b > bottom)
				bottom = b;
		}
	}

	foreach(QWidget *w, d_widgets){
		FrameWidget *fw = qobject_cast<FrameWidget *>(w);
		if (fw && (fw->y() + fw->height()) != bottom)
			fw->move(QPoint(fw->x(), bottom - fw->height()));
	}

	FrameWidget *w = qobject_cast<FrameWidget *>(d_widgets.first());
	if (w)
		w->plot()->notifyChanges();
}

void SelectionMoveResizer::recalcBoundingRect()
{
	d_bounding_rect = QRect(0, 0, -1, -1);

	foreach(ArrowMarker *i, d_line_markers){
		if(d_bounding_rect.isValid())
			d_bounding_rect |= boundingRectOf(i);
		else
			d_bounding_rect = boundingRectOf(i);
	}

	foreach(QWidget *i, d_widgets){
		QRect r = i->frameGeometry();
		QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(i);
		if (canvas)
			r = boundingRectOf(canvas);

		if(d_bounding_rect.isValid())
			d_bounding_rect |= r;
		else
			d_bounding_rect = r;
	}

	update();
}

const QRect SelectionMoveResizer::handlerRect(QRect rect, Operation op)
{
	switch(op) {
		case Resize_N:
			return QRect( QPoint(rect.center().x()-handler_size/2, rect.top()),
					QSize(handler_size,handler_size));
		case Resize_NE:
			return QRect( QPoint(rect.right()-handler_size+1, rect.top()),
				QSize(handler_size,handler_size));
		case Resize_E:
			return QRect( QPoint(rect.right()-handler_size+1, rect.center().y()-handler_size/2),
					QSize(handler_size,handler_size));
		case Resize_SE:
			return QRect( QPoint(rect.right()-handler_size+1, rect.bottom()-handler_size+1),
					QSize(handler_size,handler_size));
		case Resize_S:
			return QRect( QPoint(rect.center().x()-handler_size/2, rect.bottom()-handler_size+1),
					QSize(handler_size,handler_size));
		case Resize_SW:
			return QRect( QPoint(rect.left(),rect.bottom()-handler_size+1),
					QSize(handler_size,handler_size));
		case Resize_W:
			return QRect( QPoint(rect.left(), rect.center().y()-handler_size/2),
					QSize(handler_size,handler_size));
		case Resize_NW:
			return QRect( rect.topLeft(),
					QSize(handler_size,handler_size));
		default:
			return QRect( rect.topLeft(), QSize(0,0));
	}
}

QRect SelectionMoveResizer::operateOn(const QRect in)
{
	QRect boundary_out = d_bounding_rect;
	switch (d_op) {
		case Move:
			boundary_out.translate(d_op_dp);
			break;
		case Resize_N:
			boundary_out.setTop(boundary_out.top()+d_op_dp.y());
			break;
		case Resize_E:
			boundary_out.setRight(boundary_out.right()+d_op_dp.x());
			break;
		case Resize_S:
			boundary_out.setBottom(boundary_out.bottom()+d_op_dp.y());
			break;
		case Resize_W:
			boundary_out.setLeft(boundary_out.left()+d_op_dp.x());
			break;
		case Resize_NE:
			boundary_out.setTop(boundary_out.top()+d_op_dp.y());
			boundary_out.setRight(boundary_out.right()+d_op_dp.x());
			break;
		case Resize_SE:
			boundary_out.setBottom(boundary_out.bottom()+d_op_dp.y());
			boundary_out.setRight(boundary_out.right()+d_op_dp.x());
			break;
		case Resize_SW:
			boundary_out.setBottom(boundary_out.bottom()+d_op_dp.y());
			boundary_out.setLeft(boundary_out.left()+d_op_dp.x());
			break;
		case Resize_NW:
			boundary_out.setTop(boundary_out.top()+d_op_dp.y());
			boundary_out.setLeft(boundary_out.left()+d_op_dp.x());
			break;
		default:
			return in;
	}
	boundary_out = boundary_out.normalized();
	if (in == d_bounding_rect)
		return boundary_out;

	double scale_x = ((double)boundary_out.width())/d_bounding_rect.width();
	double scale_y = ((double)boundary_out.height())/d_bounding_rect.height();
	int offset_x = qRound(boundary_out.left()-d_bounding_rect.left()*scale_x);
	int offset_y = qRound(boundary_out.top()-d_bounding_rect.top()*scale_y);
	return QRect(QPoint(qRound(in.left()*scale_x)+offset_x, qRound(in.top()*scale_y)+offset_y),
			QSize(qRound(in.width()*scale_x), qRound(in.height()*scale_y)));
}

void SelectionMoveResizer::operateOnTargets()
{
	foreach(ArrowMarker *i, d_line_markers){
		QPoint p1 = i->startPoint();
		QPoint p2 = i->endPoint();
		QRect new_rect = operateOn(i->rect());
		i->setStartPoint(QPoint(
					p1.x()<p2.x() ? new_rect.left() : new_rect.right(),
					p1.y()<p2.y() ? new_rect.top() : new_rect.bottom() ));
		i->setEndPoint(QPoint(
					p2.x()<p1.x() ? new_rect.left() : new_rect.right(),
					p2.y()<p1.y() ? new_rect.top() : new_rect.bottom() ));
	}

	foreach(QWidget *i, d_widgets){
		QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(i);
		if (canvas){
			((Graph *)canvas->plot())->setCanvasGeometry(operateOn(d_bounding_rect));
			continue;
		}

		LegendWidget *l = qobject_cast<LegendWidget *>(i);
		if (!l){
			QRect r = operateOn(i->geometry());
			i->setGeometry(r);

			FrameWidget *f = qobject_cast<FrameWidget *>(i);
			if (f)
				f->updateCoordinates();
			continue;
		}

		QRect new_rect = operateOn(l->geometry());
		l->move(new_rect.topLeft());
		if (!l->text().isEmpty()){
            QFont f = l->font();
            f.setPointSize(f.pointSize() * new_rect.width() * new_rect.height()/(l->rect().width() * l->rect().height()));
            l->setFont(f);
            l->plot()->notifyFontChange(f);
		}
	}

	recalcBoundingRect();

	d_op_start = d_op_dp = QPoint(0,0);

	update();
	emit targetsChanged();
}

void SelectionMoveResizer::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	QRect drawn_rect = operateOn(d_bounding_rect);

	QPen white_pen(Qt::white, 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	p.setPen(white_pen);
	p.drawRect(QRect(QPoint(drawn_rect.left(), drawn_rect.top()), drawn_rect.size()));
	white_pen.setWidth(2); p.setPen(white_pen);
	for (int i=0; i<8; i++)
		p.drawRect(handlerRect(drawn_rect, (Operation)i));
	p.setPen(QPen(Qt::black,1,Qt::SolidLine));
	p.drawRect(QRect(QPoint(drawn_rect.left(), drawn_rect.top()), drawn_rect.size()));
	for (int i=0; i<8; i++)
		p.fillRect(handlerRect(drawn_rect, (Operation)i), QBrush(Qt::black));

	e->accept();
}

void SelectionMoveResizer::mousePressEvent(QMouseEvent *me)
{
	if (me->button() == Qt::RightButton){
		// If one of the parents' event handlers deletes me, Qt crashes while trying to send the QContextMenuEvent.
		foreach(QWidget *w, d_widgets){
			FrameWidget *l = qobject_cast<FrameWidget *>(w);
			if (!l){
				QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(w);
				if (canvas){
					QContextMenuEvent e(QContextMenuEvent::Other, canvas->mapFromGlobal(me->globalPos()));
					QCoreApplication::sendEvent(canvas->plot(), &e);
					return;
				}
			}
			if(l->geometry().contains(me->pos()))
				return l->showContextMenu();
		}
		me->accept();
		return;
	}

	if (me->button() == Qt::LeftButton && !d_widgets.isEmpty()){
		FrameWidget *fw = qobject_cast<FrameWidget *>(d_widgets[0]);
		if (fw && fw->plot()){
			QList <FrameWidget *> lst = fw->plot()->increasingAreaEnrichmentsList();
			foreach(FrameWidget *f, lst){
				if(!d_widgets.contains(f) && f->geometry().contains(me->pos()) && !fw->geometry().contains(me->pos()))
					return fw->plot()->select(f, me->modifiers() & Qt::ShiftModifier);
			}
		}
	}

	if (me->button() != Qt::LeftButton || !d_bounding_rect.contains(me->pos())) {
		me->ignore();
		close();
		return;
	}
	d_op_start = me->pos();
	d_op = Move;
	for (int i=0; i<8; i++)
		if (handlerRect(d_bounding_rect, (Operation)i).contains(d_op_start)) {
			d_op = (Operation)i;
			break;
		}

	me->accept();
}

void SelectionMoveResizer::mouseMoveEvent(QMouseEvent *me)
{
	if (d_op == None && d_bounding_rect.contains(me->pos()) && me->button() == Qt::LeftButton) {
		d_op = Move;
		d_op_start = me->pos();
	}
	if (d_op == None || d_op_start == QPoint(-1,-1)) {
		if (d_bounding_rect.contains(me->pos())) {
			setCursor(Qt::SizeAllCursor);
			if (handlerRect(d_bounding_rect, Resize_N).contains(me->pos()) || handlerRect(d_bounding_rect, Resize_S).contains(me->pos()))
				setCursor(Qt::SizeVerCursor);
			else if (handlerRect(d_bounding_rect, Resize_E).contains(me->pos()) || handlerRect(d_bounding_rect, Resize_W).contains(me->pos()))
				setCursor(Qt::SizeHorCursor);
			else if (handlerRect(d_bounding_rect, Resize_NE).contains(me->pos()) || handlerRect(d_bounding_rect, Resize_SW).contains(me->pos()))
				setCursor(Qt::SizeBDiagCursor);
			else if (handlerRect(d_bounding_rect, Resize_NW).contains(me->pos()) || handlerRect(d_bounding_rect, Resize_SE).contains(me->pos()))
				setCursor(Qt::SizeFDiagCursor);
		} else
			unsetCursor();
		return QWidget::mouseMoveEvent(me);
	}
	d_op_dp = me->pos() - d_op_start;
	repaint();
	me->accept();
}

void SelectionMoveResizer::mouseDoubleClickEvent(QMouseEvent *e)
{
	foreach(QWidget *w, d_widgets){
		FrameWidget *l = qobject_cast<FrameWidget *>(w);
		if (!l){
			QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(w);
			if (canvas){
				QMouseEvent event(QEvent::MouseButtonDblClick, canvas->mapFromGlobal(e->globalPos()), Qt::LeftButton, 0, 0);
				QCoreApplication::sendEvent(canvas, &event);
				delete this;
				return;
			}
		}

		if(l && l->geometry().contains(e->pos())){
			LegendWidget *lw = qobject_cast<LegendWidget *>(w);
			if (lw)
				return lw->showTextEditor();
			else
				return l->showPropertiesDialog();
		}
	}

	e->ignore();
}

void SelectionMoveResizer::mouseReleaseEvent(QMouseEvent *me)
{
	if (me->button() != Qt::LeftButton || d_op == None || d_op_start == QPoint(-1,-1))
		return QWidget::mouseReleaseEvent(me);

	d_op_dp = me->pos() - d_op_start;
	operateOnTargets();
	d_op = None;
	me->accept();
}

void SelectionMoveResizer::keyPressEvent(QKeyEvent *ke)
{
	switch(ke->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if (d_op == None) {
			    foreach(QWidget *w, d_widgets){
                    FrameWidget *l = qobject_cast<FrameWidget *>(w);
                    if (l){
                        l->showPropertiesDialog();
                        ke->accept();
                        break;
                    }
                }
				ke->ignore();
				break;
			}
			operateOnTargets();
			d_op = None;
			ke->accept();
			break;
		case Qt::Key_Escape:
			foreach(QWidget *w, d_widgets){
				FrameWidget *l = qobject_cast<FrameWidget *>(w);
				if (l && !l->isOnTop())
					l->lower();
			}
			delete this;
			ke->accept();
			return;
		case Qt::Key_Left:
			if (d_op == None) {
				d_op = Move;
				d_op_start = QPoint(-1,-1);
			}
			d_op_dp += QPoint(-1,0);
			repaint();
			ke->accept();
			break;
		case Qt::Key_Right:
			if (d_op == None) {
				d_op = Move;
				d_op_start = QPoint(-1,-1);
			}
			d_op_dp += QPoint(1,0);
			repaint();
			ke->accept();
			break;
		case Qt::Key_Up:
			if (d_op == None) {
				d_op = Move;
				d_op_start = QPoint(-1,-1);
			}
			d_op_dp += QPoint(0,-1);
			repaint();
			ke->accept();
			break;
		case Qt::Key_Down:
			if (d_op == None) {
				d_op = Move;
				d_op_start = QPoint(-1,-1);
			}
			d_op_dp += QPoint(0,1);
			repaint();
			ke->accept();
			break;
		default:
			ke->ignore();
	}
}

bool SelectionMoveResizer::eventFilter(QObject *o, QEvent *e)
{
	switch (e->type()) {
		case QEvent::Resize:
			if((QWidget*)o == parentWidget())
				setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
			recalcBoundingRect();
			return false;
		case QEvent::Move:
			if((QWidget*)o != parentWidget())
				recalcBoundingRect();
			return false;
		default:
			return false;
	}
}
