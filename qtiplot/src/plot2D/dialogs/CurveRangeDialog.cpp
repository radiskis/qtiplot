/***************************************************************************
    File                 : CurveRangeDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Curve range dialog

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
#include "CurveRangeDialog.h"
#include "PlotCurve.h"
#include "Graph.h"
#include "Table.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>

CurveRangeDialog::CurveRangeDialog(QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle(tr("QtiPlot - Plot range"));
	setName( "CurveRangeDialog" );
	setAttribute(Qt::WA_DeleteOnClose);

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Data set: ")), 0, 0);

	boxName = new QLabel();
	gl1->addWidget(boxName, 0, 1);

	gl1->addWidget(new QLabel(tr("From row number")), 1, 0);
	boxStart = new QSpinBox();
	boxStart->setMinValue(1);
	gl1->addWidget(boxStart, 1, 1);

	gl1->addWidget(new QLabel(tr("To row number")), 2, 0);
	boxEnd = new QSpinBox();
	boxEnd->setMinValue(1);
	gl1->addWidget(boxEnd, 2, 1);

	boxApplyToAll = new QCheckBox(tr("Apply to &all layer curves"));
	gl1->addWidget(boxApplyToAll, 3, 0);
	gl1->setRowStretch(4, 1);

	buttonOK = new QPushButton(tr( "&OK" ));
	buttonOK->setDefault( true );
	buttonCancel = new QPushButton(tr( "&Close" ));

	QHBoxLayout *hl = new QHBoxLayout();
	hl->addStretch();
	hl->addWidget(buttonOK);
	hl->addWidget(buttonCancel);

	QVBoxLayout *vb = new QVBoxLayout(this);
	vb->addWidget(gb1);
	vb->addLayout(hl);

	connect( buttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void CurveRangeDialog::accept()
{
	if (!d_graph)
		return;

	int start = boxStart->value() - 1;
	int end = boxEnd->value() - 1;

	if (boxApplyToAll->isChecked()){
		for (int i = 0; i < d_graph->curveCount(); i++){
			DataCurve *c = d_graph->dataCurve(i);
			if (c)
				c->setRowRange(QMIN(start, end), QMAX(start, end));
		}
	} else if (!d_curves.isEmpty()){
		foreach(DataCurve *c, d_curves)
			c->setRowRange(QMIN(start, end), QMAX(start, end));
	}

	d_graph->updatePlot();
	d_graph->notifyChanges();
	close();
}

void CurveRangeDialog::setCurveToModify(Graph *g, int curve)
{
	if (!g)
		return;

	d_graph = g;
	DataCurve *d_curve = (DataCurve *)d_graph->curve(curve);
	if (!d_curve)
		return;

	d_curves << d_curve;

	Table *t = d_curve->table();
	if (t){
		boxStart->setMaxValue(t->numRows());
		boxEnd->setMaxValue(t->numRows());
	}

	boxName->setText(d_curve->title().text());
	boxStart->setValue(d_curve->startRow() + 1);
	boxEnd->setValue(d_curve->endRow() + 1);
}

void CurveRangeDialog::setCurvesToModify(Graph *g, const QList<int>& indexes)
{
	if (!g || indexes.isEmpty())
		return;

	d_graph = g;

	QStringList curveNames;
	foreach(int i, indexes){
		DataCurve *c = (DataCurve *)d_graph->curve(i);
		if (c){
			d_curves << c;
			curveNames << c->title().text();
		}
	}

	if (d_curves.isEmpty())
		return;

	DataCurve *d_curve = d_curves[0];
	Table *t = d_curve->table();
	if (t){
		boxStart->setMaxValue(t->numRows());
		boxEnd->setMaxValue(t->numRows());
	}

	boxName->setText(curveNames.join("\n"));
	boxStart->setValue(d_curve->startRow() + 1);
	boxEnd->setValue(d_curve->endRow() + 1);
}
