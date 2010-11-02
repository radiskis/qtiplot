/***************************************************************************
	File                 : StudentTestDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Student's t-Test dialog

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
#include "StudentTestDialog.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <DoubleSpinBox.h>
#include <tTest.h>

#include <QGroupBox>
#include <QComboBox>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QDateTime>
#include <QSpinBox>

StudentTestDialog::StudentTestDialog(Table *t, bool twoSamples, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
	d_table(t),
	d_two_samples(twoSamples)
{
	setObjectName( "StudentTestDialog" );
	if (twoSamples)
		setWindowTitle(tr("Two sample t-Test"));
	else
		setWindowTitle(tr("One sample t-Test"));

	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

	QGridLayout *gl1 = new QGridLayout();
	gl1->setColumnStretch(1, 1);
	gl1->addWidget(new QLabel(tr("Sample") + ":"), 0, 0);

	boxSample1 = new QComboBox();
	boxSample1->addItems(t->columnsList());
	gl1->addWidget(boxSample1, 0, 1);

	int col = t->selectedColumn();
	if (col >= 0)
		boxSample1->setCurrentIndex(col);

	if (twoSamples){
		gl1->addWidget(new QLabel(tr("Sample") + "2:"), 1, 0);

		boxSample2 = new QComboBox();
		boxSample2->addItems(t->columnsList());
		gl1->addWidget(boxSample2, 1, 1);
		gl1->setRowStretch(2, 1);
	}

	QGroupBox *gb2 = new QGroupBox(tr("Hypotheses"));
	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->setColumnStretch(2, 1);

	gl2->addWidget(new QLabel(tr("Null") + ":"), 0, 0);

	meanLabel = new QLabel(tr("Mean") + " = ");
	gl2->addWidget(meanLabel, 0, 1);

	boxMean = new DoubleSpinBox();
	gl2->addWidget(boxMean, 0, 2);

	gl2->addWidget(new QLabel(tr("Alternate") + ":"), 1, 0);

	QString mean = tr("Mean");
	bothTailButton = new QRadioButton(mean + " <>");
	bothTailButton->setChecked(true);
	gl2->addWidget(bothTailButton, 1, 1);
	bothTailLabel = new QLabel();
	bothTailLabel->setText("0");
	gl2->addWidget(bothTailLabel, 1, 2);

	rightTailButton = new QRadioButton(mean + " >");
	gl2->addWidget(rightTailButton, 2, 1);
	rightTailLabel = new QLabel();
	rightTailLabel->setText("0");
	gl2->addWidget(rightTailLabel, 2, 2);

	leftTailButton = new QRadioButton(mean + " <");
	gl2->addWidget(leftTailButton, 3, 1);
	leftTailLabel = new QLabel();
	leftTailLabel->setText("0");
	gl2->addWidget(leftTailLabel, 3, 2);

	gl2->addWidget(new QLabel(tr("Significance Level")), 4, 0);
	boxSignificance = new DoubleSpinBox();
	boxSignificance->setRange(0.0, 1.0);
	boxSignificance->setDecimals(2);
	boxSignificance->setSingleStep(0.01);
	boxSignificance->setValue(0.05);
	gl2->addWidget(boxSignificance, 4, 1);

	boxConfidenceInterval = new QGroupBox(tr("Confidence &Interval(s)"));
	boxConfidenceInterval->setCheckable(true);

	DoubleSpinBox *sbox = new DoubleSpinBox();
	sbox->setRange(0.01, 99.99);
	sbox->setSingleStep(1.0);
	sbox->setValue(90);

	QGridLayout *gl3 = new QGridLayout(boxConfidenceInterval);
	gl3->setColumnStretch(1, 1);
	gl3->addWidget(new QLabel(tr("Level(s) in %")), 0, 0);
	gl3->addWidget(sbox, 0, 1);

	buttonAddLevel = new QPushButton(tr("&Add Level"));
	gl3->addWidget(buttonAddLevel, 0, 2);

	boxPowerAnalysis = new QGroupBox(tr("&Power Analysis"));
	boxPowerAnalysis->setCheckable(true);

	QGridLayout *gl4 = new QGridLayout(boxPowerAnalysis);

	boxPowerLevel = new DoubleSpinBox();
	boxPowerLevel->setRange(0.01, 0.99);
	boxPowerLevel->setSingleStep(0.01);
	boxPowerLevel->setValue(0.5);

	gl4->addWidget(new QLabel(tr("Alpha")), 0, 0);
	gl4->addWidget(boxPowerLevel, 0, 1);

	boxOtherSampleSize = new QCheckBox(tr("Sample &Size"));
	boxOtherSampleSize->setChecked(false);
	gl4->addWidget(boxOtherSampleSize, 1, 0);

	boxSampleSize = new QSpinBox();
	boxSampleSize->setRange(1, INT_MAX);
	boxSampleSize->setValue(50);
	boxSampleSize->setEnabled(false);
	gl4->addWidget(boxSampleSize, 1, 1);

	buttonOk = new QPushButton(tr( "&Compute" ));

	QHBoxLayout *hl2 = new QHBoxLayout();
	hl2->addStretch();
	hl2->addWidget(buttonOk);
	hl2->addStretch();

	QVBoxLayout *vl = new QVBoxLayout(this);
	vl->addLayout(gl1);
	vl->addWidget(gb2);
	vl->addWidget(boxConfidenceInterval);
	vl->addWidget(boxPowerAnalysis);
	vl->addStretch();
	vl->addLayout(hl2);

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(buttonAddLevel, SIGNAL(clicked()), this, SLOT(addConfidenceLevel()));
	connect(boxOtherSampleSize, SIGNAL(toggled(bool)), boxSampleSize, SLOT(setEnabled(bool)));
	connect(leftTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(rightTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(bothTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(boxMean, SIGNAL(valueChanged(double)), this, SLOT(updateMeanLabels(double)));
}

void StudentTestDialog::addConfidenceLevel()
{
	QGridLayout *gl = (QGridLayout *)boxConfidenceInterval->layout();
	if (!gl)
		return;

	int rows = gl->rowCount();

	DoubleSpinBox *sbox = new DoubleSpinBox();
	sbox->setRange(0.01, 99.999);
	sbox->setSingleStep(1.0);

	DoubleSpinBox *sb = (DoubleSpinBox *)gl->itemAtPosition (rows - 1, 1)->widget();
	sbox->setValue(0.5*(100 + sb->value()));

	gl->addWidget(sbox, rows, 1);
}

void StudentTestDialog::updateMeanLabel()
{
	QString s = tr("Mean");
	if (bothTailButton->isChecked())
		s += " = ";
	else if (rightTailButton->isChecked())
		s += " <= ";
	else if (leftTailButton->isChecked())
		s += " >= ";

	meanLabel->setText(s);
}

void StudentTestDialog::updateMeanLabels(double val)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	QString s = app->locale().toString(val, 'g', app->d_decimal_digits);
	leftTailLabel->setText(s);
	rightTailLabel->setText(s);
	bothTailLabel->setText(s);
}

void StudentTestDialog::accept()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	double testMean = boxMean->value();

	tTest stats(app, testMean, boxSignificance->value());
	if (!stats.setData(boxSample1->currentText()))
		return;

	tTest::Tail tail = tTest::Left;
	if (bothTailButton->isChecked())
		tail = tTest::Both;
	else if (rightTailButton->isChecked())
		tail = tTest::Right;
	stats.setTail(tail);

	int p = app->d_decimal_digits;
	QLocale l = app->locale();

	QString sep = "\t\t";
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";
	QString s = stats.logInfo();

	if (boxConfidenceInterval->isChecked()){
		QGridLayout *gl = (QGridLayout *)boxConfidenceInterval->layout();
		int rows = gl->rowCount();

		s += "\n" + tr("Confidence Interval for Mean") + "\n\n";
		s += tr("Level") + sep + tr("Lower Limit") + sep + tr("Upper Limit") + "\n";
		s += sep1;
		for (int i = 0; i < rows; i++){
			DoubleSpinBox *sb = (DoubleSpinBox *)gl->itemAtPosition (i, 1)->widget();
			double level = sb->value();
			s += l.toString(level) + sep + l.toString(stats.lcl(level), 'g', p);
			s += "\t" + l.toString(stats.ucl(level),'g', p) + "\n";
		}
		s += sep1;
	}

	if (boxPowerAnalysis->isChecked()){
		double power = stats.power(boxPowerLevel->value());
		s += "\n" + tr("Power Analysis") + "\n\n";
		s += tr("Alpha") + sep + tr("Sample Size") + sep + tr("Power") + "\n";
		s += sep1;
		s += l.toString(boxPowerLevel->value(), 'g', 6) + sep + QString::number(stats.dataSize()) + sep + l.toString(power, 'g', p) + "   (" + tr("actual") + ")\n";
		if (boxOtherSampleSize->isChecked()){
			int size = boxSampleSize->value();
			power = stats.power(boxPowerLevel->value(), size);
			s += l.toString(boxPowerLevel->value(), 'g', 6) + sep + QString::number(size) + sep + l.toString(power, 'g', p) + "\n";
		}
		s += sep1;
	}

	app->updateLog(s);
}
