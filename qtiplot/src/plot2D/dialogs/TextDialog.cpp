/***************************************************************************
    File                 : TextDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Title/axis label options dialog

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

#include "TextDialog.h"
#include "../../ApplicationWindow.h"
#include "../../ColorButton.h"
#include "../../TextFormatButtons.h"
#include "../LegendWidget.h"
#include "../FrameWidget.h"

#include <QFontDialog>
#include <QFont>
#include <QGroupBox>
#include <QTextEdit>
#include <QTextCursor>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>

#include <qwt_scale_widget.h>

TextDialog::TextDialog(TextType type, QWidget* parent, Qt::WFlags fl)
	: QDialog( parent, fl)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle( tr( "QtiPlot - Text options" ) );
	setSizeGripEnabled( true );

	d_graph = NULL;
	d_scale = NULL;

	textType = type;

	groupBox1 = new QGroupBox();
	QGridLayout * topLayout = new QGridLayout(groupBox1);
	topLayout->addWidget(new QLabel(tr("Text Color")), 0, 0);

	colorBtn = new ColorButton();
	topLayout->addWidget(colorBtn, 0, 1);

	buttonOk = new QPushButton(tr("&OK"));
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );

	topLayout->addWidget(buttonOk, 0, 3);
	topLayout->addWidget(new QLabel(tr("Font")), 1, 0);

	buttonFont = new QPushButton(tr( "&Font" ));
	topLayout->addWidget(buttonFont, 1, 1);

	buttonApply = new QPushButton(tr( "&Apply" ));
	buttonApply->setDefault( true );
	topLayout->addWidget( buttonApply, 1, 3 );

	topLayout->addWidget(new QLabel(tr("Alignment")), 2, 0);
	alignmentBox = new QComboBox();
	alignmentBox->addItem( tr( "Center" ) );
	alignmentBox->addItem( tr( "Left" ) );
	alignmentBox->addItem( tr( "Right" ) );
	topLayout->addWidget(alignmentBox, 2, 1);

	boxApplyToAll = new QCheckBox(tr("Apply format to all &labels in layer"));
	topLayout->addWidget(boxApplyToAll, 3, 0 );

	buttonCancel = new QPushButton( tr( "&Cancel" ) );
	topLayout->addWidget( buttonCancel, 2, 3 );

	topLayout->setColumnStretch(2, 1);

	textEditBox = new QTextEdit();
	textEditBox->setTextFormat(Qt::PlainText);
	textEditBox->setFont(QFont());

	formatButtons =  new TextFormatButtons(textEditBox);
	formatButtons->toggleCurveButton(false);

	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(textEditBox);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(groupBox1);
	mainLayout->addWidget(formatButtons);
	mainLayout->addWidget(textEditBox);
	setLayout( mainLayout );

	// signals and slots connections
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonFont, SIGNAL( clicked() ), this, SLOT(customFont() ) );
}

void TextDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	d_graph = g;
	QwtText l;
	if (textType == LayerTitle)
		l = d_graph->title();
	else if (textType == AxisTitle){
		d_scale = g->currentScale();
		if (!d_scale)
			return;

		l =	d_scale->title();
		switch(d_scale->alignment()){
			case QwtScaleDraw::BottomScale:
				setWindowTitle(tr("QtiPlot") + " - " + tr("X Axis Title"));
			break;
			case QwtScaleDraw::LeftScale:
				setWindowTitle(tr("QtiPlot") + " - " + tr("Y Axis Title"));
			break;
			case QwtScaleDraw::TopScale:
				setWindowTitle(tr("QtiPlot") + " - " + tr("Top Axis Title"));
			break;
			case QwtScaleDraw::RightScale:
				setWindowTitle(tr("QtiPlot") + " - " + tr("Right Axis Title"));
			break;
		}
	}

	setAlignment(l.renderFlags());
	setText(l.text());
	selectedFont = l.font();
	colorBtn->setColor(l.color());
}

void TextDialog::apply()
{
	if (textType == AxisTitle){
		if (!d_graph || !d_scale)
			return;

		QwtText t =	d_scale->title();
		t.setRenderFlags(alignment());
		t.setText(textEditBox->toPlainText());
		d_scale->setTitle(t);

		if (boxApplyToAll->isChecked())
			formatAllLabels();
		else {
			t.setFont(selectedFont);
			t.setColor(colorBtn->color());
			d_scale->setTitle(t);
			d_graph->replot();
		}
	} else if (textType == LayerTitle){
		if (!d_graph)
			return;

		QwtText t =	d_graph->title();
		t.setRenderFlags(alignment());
		t.setText(textEditBox->toPlainText());
		((QwtPlot *)d_graph)->setTitle(t);

		if (boxApplyToAll->isChecked())
			formatAllLabels();
		else {
			t.setFont(selectedFont);
			t.setColor(colorBtn->color());
			((QwtPlot *)d_graph)->setTitle(t);
			d_graph->replot();
		}
	}

	if (d_graph)
		d_graph->notifyChanges();
}

void TextDialog::formatAllLabels()
{
	if (!d_graph)
		return;

	QColor tc = colorBtn->color();
	
	QList<FrameWidget*> lst = d_graph->enrichmentsList();
	foreach(FrameWidget *fw, lst){
		LegendWidget *l = qobject_cast<LegendWidget *>(fw);
		if (l){
			l->setTextColor(tc);
			l->setFont(selectedFont);
		}
	}
		
	for (int i=0; i < QwtPlot::axisCnt; i++){
		QwtScaleWidget *scale = (QwtScaleWidget *)d_graph->axisWidget(i);
		if (scale){
			QwtText t = scale->title();
			t.setColor(tc);
			t.setFont(selectedFont);
			scale->setTitle(t);
		}
	}

	QwtText t = d_graph->title();
	t.setColor(tc);
	t.setFont(selectedFont);
	((QwtPlot *)d_graph)->setTitle (t);
	d_graph->replot();
}

void TextDialog::accept()
{
	apply();
	close();
}

int TextDialog::alignment()
{
	int align=-1;
	switch (alignmentBox->currentIndex())
	{
		case 0:
			align = Qt::AlignHCenter;
			break;

		case 1:
			align = Qt::AlignLeft;
			break;

		case 2:
			align = Qt::AlignRight;
			break;
	}
	return align;
}

void TextDialog::setAlignment(int align)
{
	switch(align)
	{
		case Qt::AlignHCenter:
			alignmentBox->setCurrentIndex(0);
			break;
		case Qt::AlignLeft:
			alignmentBox->setCurrentIndex(1);
			break;
		case Qt::AlignRight:
			alignmentBox->setCurrentIndex(2);
			break;
	}
}

void TextDialog::customFont()
{
	bool okF;
	QFont fnt = QFontDialog::getFont( &okF, selectedFont, this);
	if (okF && fnt != selectedFont)
		selectedFont = fnt;
}

void TextDialog::setText(const QString & t)
{
	QTextCursor cursor = textEditBox->textCursor();
	// select the whole (old) text
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// replace old text
	cursor.insertText(t);
	// select the whole (new) text
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// this line makes the selection visible to the user
	// (the 2 lines above only change the selection in the
	// underlying QTextDocument)
	textEditBox->setTextCursor(cursor);
	// give focus back to text edit
	textEditBox->setFocus();
}
