/***************************************************************************
    File                 : InterpolationDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Interpolation options dialog

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
#include "InterpolationDialog.h"
#include <Interpolation.h>
#include <Graph.h>
#include <ColorButton.h>
#include <DoubleSpinBox.h>
#include <RangeSelectorTool.h>

#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>

InterpolationDialog::InterpolationDialog( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setObjectName( "InterpolationDialog" );
	setWindowTitle(tr("QtiPlot - Interpolation Options"));
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled( true );

    QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Make curve from")), 0, 0);

	boxName = new QComboBox();
	gl1->addWidget(boxName, 0, 1);

	gl1->addWidget(new QLabel(tr("Spline")), 1, 0);
	boxMethod = new QComboBox();
	boxMethod->addItem(tr("Linear"));
    boxMethod->addItem(tr("Cubic"));
    boxMethod->addItem(tr("Non-rounded Akima"));
	gl1->addWidget(boxMethod, 1, 1);

	gl1->addWidget(new QLabel(tr("Points")), 2, 0);
	boxPoints = new QSpinBox();
	boxPoints->setRange(3, 100000);
	boxPoints->setSingleStep(10);
	boxPoints->setValue(1000);
	gl1->addWidget(boxPoints, 2, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent;

	gl1->addWidget(new QLabel(tr("From Xmin")), 3, 0);
	boxStart = new DoubleSpinBox();
	boxStart->setDecimals(app->d_decimal_digits);
	boxStart->setLocale(app->locale());
	gl1->addWidget(boxStart, 3, 1);

	gl1->addWidget(new QLabel(tr("To Xmax")), 4, 0);
	boxEnd = new DoubleSpinBox();
	boxEnd->setDecimals(app->d_decimal_digits);
	boxEnd->setLocale(app->locale());
	gl1->addWidget(boxEnd, 4, 1);

	gl1->addWidget(new QLabel(tr("Color")), 5, 0);

	boxColor = new ColorButton();
	boxColor->setColor(Qt::red);
	gl1->addWidget(boxColor, 5, 1);
	gl1->setRowStretch(6, 1);
	gl1->setColumnStretch(1, 1);

	buttonFit = new QPushButton(tr( "&Make" ));
    buttonFit->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(buttonFit);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1, 1);
    hb->addLayout(vl);

	connect(boxName, QOverload<const QString&>::of(&QComboBox::activated), this, &InterpolationDialog::activateCurve);
	connect(buttonFit, &QAbstractButton::clicked, this, &InterpolationDialog::interpolate);
	connect(buttonCancel, &QAbstractButton::clicked, this, &InterpolationDialog::close);
}

void InterpolationDialog::interpolate()
{
	QString curveName = boxName->currentText();
	QStringList curvesList = graph->analysableCurvesList();
	if (!curvesList.contains(curveName)){
		QMessageBox::critical(this,tr("QtiPlot - Warning"),
		tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curveName));
		boxName->clear();
		boxName->addItems(curvesList);
		return;
	}

	double from = boxStart->value();
	double to = boxEnd->value();
	if (from >= to){
		QMessageBox::critical(this, tr("QtiPlot - Input error"), tr("Please enter x limits that satisfy: from < to!"));
		boxEnd->setFocus();
		return;
	}

	Interpolation *i = new Interpolation((ApplicationWindow *)parent(), (PlotCurve *)graph->curve(curveName), from, to, boxMethod->currentIndex());
	i->setOutputPoints(boxPoints->value());
	i->setColor(boxColor->color());
	i->run();
	delete i;
}

void InterpolationDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	graph = g;
	boxName->addItems(g->analysableCurvesList());

	if (g->rangeSelectorsEnabled())
		boxName->setCurrentIndex(boxName->findText(g->curveRange(g->rangeSelectorTool()->selectedCurve())));

	activateCurve(boxName->currentText());

	connect(graph, &QObject::destroyed, this, &InterpolationDialog::close);
	connect(graph, &Graph::dataRangeChanged, this, &InterpolationDialog::changeDataRange);
}

void InterpolationDialog::activateCurve(const QString& s)
{
	PlotCurve *c = (PlotCurve *)graph->curve(s);
	if (!c)
		return;

    ApplicationWindow *app = (ApplicationWindow *)parent();
    if(!app)
        return;

	double start, end;
	graph->range(c, &start, &end);
	boxStart->setValue(qMin(start, end));
	boxEnd->setValue(qMax(start, end));
}

void InterpolationDialog::changeDataRange()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if(!app)
		return;

	double start = graph->selectedXStartValue();
	double end = graph->selectedXEndValue();
	boxStart->setValue(qMin(start, end));
	boxEnd->setValue(qMax(start, end));
}
