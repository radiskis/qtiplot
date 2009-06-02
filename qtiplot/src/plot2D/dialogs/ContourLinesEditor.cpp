/***************************************************************************
	File                 : ContourLinesEditor.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2009 by Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : A Contour Lines Editor Widget
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
#include "ContourLinesEditor.h"
#include "DoubleSpinBox.h"
#include "Spectrogram.h"
#include "PenStyleBox.h"
#include "ColorButton.h"

#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QGroupBox>

ContourLinesEditor::ContourLinesEditor(const QLocale& locale, int precision, QWidget* parent)
				: QWidget(parent),
				d_spectrogram(NULL),
				d_locale(locale),
				d_precision(precision)
{
	table = new QTableWidget();
	table->setColumnCount(2);
	table->hideColumn(1);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	table->horizontalHeader()->setClickable( false );
	table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	table->viewport()->setMouseTracking(true);
	table->viewport()->installEventFilter(this);
	table->setHorizontalHeaderLabels(QStringList() << tr("Level") << tr("Pen"));
	table->setMinimumHeight(6*table->horizontalHeader()->height() + 2);
	table->installEventFilter(this);

	connect(table, SIGNAL(cellClicked (int, int)), this, SLOT(showPenDialog(int, int)));

	insertBtn = new QPushButton(tr("&Insert"));
	insertBtn->setEnabled(false);
	connect(insertBtn, SIGNAL(clicked()), this, SLOT(insertLevel()));

	deleteBtn = new QPushButton(tr("&Delete"));
	deleteBtn->setEnabled(false);
	connect(deleteBtn, SIGNAL(clicked()), this, SLOT(deleteLevel()));

	QHBoxLayout* hb = new QHBoxLayout();
	hb->addWidget(insertBtn);
	hb->addWidget(deleteBtn);

	QVBoxLayout* vl = new QVBoxLayout(this);
	vl->setSpacing(0);
	vl->addWidget(table);
	vl->addLayout(hb);

	setFocusProxy(table);
	setMaximumWidth(200);

	penDialog = NULL;
}

void ContourLinesEditor::updateContourLevels()
{
	if (!d_spectrogram)
		return;

	int rows = table->rowCount();
	QwtValueList levels;
	for (int i = 0; i < rows; i++)
		levels << ((DoubleSpinBox*)table->cellWidget(i, 0))->value();

	d_spectrogram->setContourLevels(levels);
}

void ContourLinesEditor::updateContourPens()
{
	if (!d_spectrogram)
		return;

	if (!table->isColumnHidden(1))
		d_spectrogram->setContourPenList(d_pen_list);
}

void ContourLinesEditor::setSpectrogram(Spectrogram *sp)
{
	if (!sp || d_spectrogram == sp)
		return;

	d_spectrogram = sp;
	updateContents();
}

void ContourLinesEditor::updateContents()
{
	if (!d_spectrogram)
		return;

	QwtValueList levels = d_spectrogram->contourLevels ();

	int rows = (int)levels.size();
	table->setRowCount(rows);
	table->blockSignals(true);

	QwtDoubleInterval range = d_spectrogram->data().range();
	for (int i = 0; i < rows; i++){
		DoubleSpinBox *sb = new DoubleSpinBox();
		sb->setLocale(d_locale);
		sb->setDecimals(d_precision);
		sb->setValue(levels[i]);
		sb->setRange(range.minValue (), range.maxValue ());
		connect(sb, SIGNAL(activated(DoubleSpinBox *)), this, SLOT(spinBoxActivated(DoubleSpinBox *)));
    	table->setCellWidget(i, 0, sb);

		QPen pen = d_spectrogram->defaultContourPen();
		if (pen.style() == Qt::NoPen)
			pen = d_spectrogram->contourPen (levels[i]);

		int width = 80;
		int height = 20;
    	QPixmap pix(width, height);
    	pix.fill(Qt::white);
    	QPainter paint(&pix);
    	paint.setRenderHint(QPainter::Antialiasing);
    	paint.setPen(pen);
    	paint.drawLine(0, height/2, width, height/2);
    	paint.end();

    	QLabel *lbl = new QLabel();
    	lbl->setPixmap(pix);

    	table->setCellWidget(i, 1, lbl);

    	d_pen_list << pen;
	}
	table->blockSignals(false);
}

void ContourLinesEditor::insertLevel()
{
	if (!d_spectrogram)
		return;

	int row = table->currentRow();
	DoubleSpinBox *sb = (DoubleSpinBox*)table->cellWidget(row, 0);
	if (!sb)
		return;

	QwtDoubleInterval range = d_spectrogram->data().range();
	double current_value = sb->value();
	double previous_value = range.minValue ();
	sb = (DoubleSpinBox*)table->cellWidget(row - 1, 0);
	if (sb)
		previous_value = sb->value();

	double val = 0.5*(current_value + previous_value);

	table->blockSignals(true);
	table->insertRow(row);

	sb = new DoubleSpinBox();
	sb->setLocale(d_locale);
	sb->setDecimals(d_precision);
	sb->setValue(val);
	sb->setRange(range.minValue (), range.maxValue ());
	connect(sb, SIGNAL(activated(DoubleSpinBox *)), this, SLOT(spinBoxActivated(DoubleSpinBox *)));
    table->setCellWidget(row, 0, sb);

	QPen pen = d_spectrogram->defaultContourPen();
	if (pen.style() == Qt::NoPen)
		pen = d_spectrogram->contourPen (val);

	int width = 80;
	int height = 20;
	QPixmap pix(width, height);
	pix.fill(Qt::white);
	QPainter paint(&pix);
	paint.setRenderHint(QPainter::Antialiasing);
	paint.setPen(pen);
	paint.drawLine(0, height/2, width, height/2);
	paint.end();

	QLabel *lbl = new QLabel();
	lbl->setPixmap(pix);

	table->setCellWidget(row, 1, lbl);
	table->blockSignals(false);

	enableButtons(table->currentRow());
	d_pen_list.insert(row, pen);
}

void ContourLinesEditor::deleteLevel()
{
	int index = table->currentRow();
	table->removeRow (index);

	if (index >=0 && index < d_pen_list.size())
		d_pen_list.removeAt(index);
}

void ContourLinesEditor::showPenDialog(int row, int col)
{
	if (!d_spectrogram || col != 1)
		return;

	enableButtons(row);

	QPen pen = d_pen_list[row];

	if (!penDialog){
		penDialog = new QDialog(this);
		penDialog->setWindowTitle(tr("QtiPlot - Edit level pen"));

		QFormLayout *pdl = new QFormLayout(penDialog);

		penColorBox = new ColorButton();
		penColorBox->setColor(pen.color());
		pdl->addRow(tr("Color"), penColorBox);

		penStyleBox = new PenStyleBox;
		penStyleBox->setStyle(pen.style());
		pdl->addRow(tr("Style"), penStyleBox);

		penWidthBox = new DoubleSpinBox();
		penWidthBox->setValue(pen.widthF());
		pdl->addRow(tr("Width"), penWidthBox);

		QPushButton *acceptPenBtn = new QPushButton(tr("Ok"));
		pdl->addRow(acceptPenBtn);
		connect(acceptPenBtn, SIGNAL(clicked()), this, SLOT(updatePen()));
	} else {
		penColorBox->setColor(pen.color());
		penStyleBox->setStyle(pen.style());
		penWidthBox->setValue(pen.widthF());
	}

	d_pen_index = row;
	penDialog->exec();
}

void ContourLinesEditor::updatePen()
{
	QPen pen = QPen(penColorBox->color(), penWidthBox->value(), penStyleBox->style());

	int width = 80;
	int height = 20;
	QPixmap pix(width, height);
	pix.fill(Qt::white);
	QPainter paint(&pix);
	paint.setRenderHint(QPainter::Antialiasing);
	paint.setPen(pen);
	paint.drawLine(0, height/2, width, height/2);
	paint.end();

	QLabel *lbl = new QLabel();
	lbl->setPixmap(pix);

	table->setCellWidget(d_pen_index, 1, lbl);
	penDialog->close();

	d_pen_list[d_pen_index] = pen;
}

bool ContourLinesEditor::eventFilter(QObject *object, QEvent *e)
{
	if (e->type() == QEvent::MouseMove && object == table->viewport()){
        const QMouseEvent *me = (const QMouseEvent *)e;
        QPoint pos = table->viewport()->mapToParent(me->pos());
        int row = table->rowAt(pos.y() - table->horizontalHeader()->height());
        if (table->columnAt(pos.x()) == 1 && row >= 0 && row < table->rowCount())
            setCursor(QCursor(Qt::PointingHandCursor));
        else
            setCursor(QCursor(Qt::ArrowCursor));
        return true;
	} else if (e->type() == QEvent::Leave && object == table->viewport()){
		setCursor(QCursor(Qt::ArrowCursor));
		return true;
	} else if (e->type() == QEvent::KeyPress && object == table){
		QKeyEvent *ke = (QKeyEvent *)e;
		if (ke->key() == Qt::Key_Return && table->currentColumn() == 1){
			showPenDialog(table->currentRow(), 1);
			return true;
		}
	return false;
	}
	return QObject::eventFilter(object, e);
}

void ContourLinesEditor::enableButtons(int row)
{
	if (row < 0)
		return;

	deleteBtn->setEnabled(true);
	insertBtn->setEnabled(true);
}

void ContourLinesEditor::spinBoxActivated(DoubleSpinBox *sb)
{
	if (!sb)
		return;

	int rows = table->rowCount();
	for (int i = 0; i < rows; i++){
		DoubleSpinBox *box = (DoubleSpinBox*)table->cellWidget(i, 0);
		if (box && box == sb){
			table->setCurrentCell(i, 0);
			enableButtons(i);
			return;
		}
	}
}

void ContourLinesEditor::showPenColumn(bool on)
{
	if (on)
		table->showColumn(1);
	else
		table->hideColumn(1);
}

ContourLinesEditor::~ContourLinesEditor()
{
	if(penDialog)
        delete penDialog;
}
