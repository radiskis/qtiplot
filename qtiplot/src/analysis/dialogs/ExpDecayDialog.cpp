/***************************************************************************
    File                 : ExpDecayDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Fit exponential decay dialog

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
#include "ExpDecayDialog.h"
#include <Fit.h>
#include <ExponentialFit.h>
#include <Graph.h>
#include <ApplicationWindow.h>
#include <DoubleSpinBox.h>
#include <ColorButton.h>
#include <PlotCurve.h>
#include <RangeSelectorTool.h>

#include <QMessageBox>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

ExpDecayDialog::ExpDecayDialog(int type, QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl ), slopes(type)
{
    setName( "ExpDecayDialog" );
	setWindowTitle(tr("QtiPlot - Verify initial guesses"));
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	int precision = app->fit_output_precision;
	QLocale l = app->locale();

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout *gl1 = new QGridLayout();
	gl1->addWidget(new QLabel(tr("Exponential Fit of")), 0, 0);

	boxName = new QComboBox();
	connect(boxName, SIGNAL(activated(const QString&)), this, SLOT(activateCurve(const QString&)));
	gl1->addWidget(boxName, 0, 1);

	if (type < 0)
		dampingLabel = new QLabel( tr("Growth time"));
	else if (type == 1)
		dampingLabel = new QLabel( tr("Decay time"));
	else
		dampingLabel = new QLabel( tr("First decay time (t1)"));
	gl1->addWidget(dampingLabel, 1, 0);

	boxFirst = new DoubleSpinBox();
	boxFirst->setValue(1.0);
	boxFirst->setDecimals(precision);
	boxFirst->setLocale(l);
	gl1->addWidget(boxFirst, 1, 1);

	if (type > 1){
		gl1->addWidget(new QLabel( tr("Second decay time (t2)")), 2, 0);

		boxSecond = new DoubleSpinBox();
		boxSecond->setValue(1.0);
		boxSecond->setDecimals(precision);
		boxSecond->setLocale(l);
		gl1->addWidget(boxSecond, 2, 1);

		thirdLabel = new QLabel( tr("Third decay time (t3)"));
		gl1->addWidget(thirdLabel, 3, 0);

		boxThird = new DoubleSpinBox();
		boxThird->setValue(1.0);
		boxThird->setDecimals(precision);
		boxThird->setLocale(l);
		gl1->addWidget(boxThird, 3, 1);

		if (type < 3){
			thirdLabel->hide();
			boxThird->hide();
		}
	}

	if (type <= 1){
		gl1->addWidget(new QLabel(tr("Amplitude")), 2, 0);
		boxAmplitude = new DoubleSpinBox();
		boxAmplitude->setValue(1.0);
		boxAmplitude->setDecimals(precision);
		boxAmplitude->setLocale(l);
		gl1->addWidget(boxAmplitude, 2, 1);
	}

	gl1->addWidget(new QLabel(tr("Y Offset")), 4, 0 );
	boxYOffset = new DoubleSpinBox();
	boxYOffset->setValue(0.0);
	boxYOffset->setDecimals(precision);
	boxYOffset->setLocale(l);
	gl1->addWidget(boxYOffset, 4, 1);

    gl1->addWidget(new QLabel(tr("Initial time")), 5, 0);

	boxStart = new DoubleSpinBox();
	boxStart->setValue(0.0);
	boxStart->setDecimals(precision);
	boxStart->setLocale(l);
	gl1->addWidget(boxStart, 5, 1);

	gl1->addWidget(new QLabel(tr("Color")), 6, 0 );
	boxColor = new ColorButton();
	boxColor->setColor(Qt::red);
	gl1->addWidget(boxColor, 6, 1);

	gl1->setRowStretch(7, 1);
	gl1->setColumnStretch(1, 1);
	gb1->setLayout(gl1);

	buttonFit = new QPushButton(tr("&Fit"));
	buttonFit->setDefault(true);

	buttonCancel = new QPushButton(tr("&Close"));

	QBoxLayout *bl1 = new QBoxLayout (QBoxLayout::TopToBottom);
	bl1->addWidget(buttonFit);
	bl1->addWidget(buttonCancel);
	bl1->addStretch();

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addWidget(gb1, 1);
	hlayout->addLayout(bl1);
	setLayout(hlayout);

	// signals and slots connections
	connect( buttonFit, SIGNAL( clicked() ), this, SLOT(fit()));
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT(close()));
}

void ExpDecayDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	fitter = 0;
	graph = g;

	boxName->addItems(graph->analysableCurvesList());

	if (g->rangeSelectorsEnabled())
		boxName->setCurrentIndex(boxName->findText(g->curveRange(g->rangeSelectorTool()->selectedCurve())));

	activateCurve(boxName->currentText());

	connect (graph, SIGNAL(destroyed()), this, SLOT(close()));
	connect (graph, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
}

void ExpDecayDialog::activateCurve(const QString& s)
{
	PlotCurve *c = graph->curve(s.left(s.indexOf(" [")));
	if (!c)
		return;

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
        return;

	double start, end;
	graph->range(c, &start, &end);
	boxStart->setValue(QMIN(start, end));
	boxYOffset->setValue(c->minYValue());
	if (slopes < 2)
        boxAmplitude->setValue(c->maxYValue() - c->minYValue());

}

void ExpDecayDialog::changeDataRange()
{
	double start = graph->selectedXStartValue();
	double end = graph->selectedXEndValue();
	boxStart->setValue(QMIN(start, end));
}

void ExpDecayDialog::fit()
{
	QString curve = boxName->currentText();
	PlotCurve *c = graph->curve(curve.left(curve.indexOf(" [")));
	QStringList curvesList = graph->analysableCurvesList();
	if (!c || !curvesList.contains(curve)){
		QMessageBox::critical(this,tr("QtiPlot - Warning"),
				tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
		boxName->clear();
		boxName->addItems(curvesList);
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (!app)
        return;

	if (fitter)
        delete fitter;

	if (slopes == 3){
		double x_init[7] = {1.0, boxFirst->value(), 1.0, boxSecond->value(), 1.0, boxThird->value(), boxYOffset->value()};
		fitter = new ThreeExpFit(app, graph);
		fitter->setInitialGuesses(x_init);
	} else if (slopes == 2) {
		double x_init[5] = {1.0, boxFirst->value(), 1.0, boxSecond->value(), boxYOffset->value()};
		fitter = new TwoExpFit(app, graph);
		fitter->setInitialGuesses(x_init);
	} else if (slopes == 1 || slopes == -1){
		double x_init[3] = {boxAmplitude->value(), slopes*boxFirst->value(), boxYOffset->value()};
		fitter = new ExponentialFit(app, graph, slopes == -1);
		fitter->setInitialGuesses(x_init);
	}

	if (fitter->setDataFromCurve(c, boxStart->value(), c->maxXValue())){
		fitter->setColor(boxColor->color());
		fitter->scaleErrors(app->fit_scale_errors);
        fitter->setOutputPrecision(app->fit_output_precision);
		fitter->generateFunction(app->generateUniformFitPoints, app->fitPoints);
		fitter->fit();

		double *results = fitter->results();
		boxFirst->setValue(results[1]);
		if (slopes < 2){
            boxAmplitude->setValue(results[0]);
            boxYOffset->setValue(results[2]);
        } else if (slopes == 2){
            boxSecond->setValue(results[3]);
            boxYOffset->setValue(results[4]);
        } else if (slopes == 3){
            boxSecond->setValue(results[3]);
            boxThird->setValue(results[5]);
            boxYOffset->setValue(results[6]);
        }
	}
}

void ExpDecayDialog::closeEvent (QCloseEvent * e )
{
	if(fitter)
	{
        ApplicationWindow *app = (ApplicationWindow *)this->parent();
        if (app && app->pasteFitResultsToPlot)
            fitter->showLegend();

        delete fitter;
	}

	e->accept();
}
