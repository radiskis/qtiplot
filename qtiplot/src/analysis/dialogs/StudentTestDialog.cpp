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
#include <DoubleSpinBox.h>
#include <CollapsiveGroupBox.h>
#include <tTest.h>
#include <ChiSquareTest.h>
#include <Note.h>

#include <QGroupBox>
#include <QComboBox>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

StudentTestDialog::StudentTestDialog(const StatisticTest::TestType& type, Table *t, bool twoSamples, QWidget* parent)
	: QDialog( parent),
	d_test_type(type),
	d_two_samples(twoSamples)
{
	ApplicationWindow *app = (ApplicationWindow *)parent;
	d_table = 0;
	d_note = 0;

	setObjectName( "StudentTestDialog" );
	QHBoxLayout *hl = 0;

	if (type == StatisticTest::ChiSquareTest)
		setWindowTitle(tr("Chi-square Test for Variance"));
	else {
		if (twoSamples){
			setWindowTitle(tr("Two sample t-Test"));

			independentTestBtn = new QRadioButton(tr("In&dependent Test"));
			independentTestBtn->setChecked(true);
			pairedTestBtn = new QRadioButton(tr("Pai&red Test"));
			hl = new QHBoxLayout();
			hl->addWidget(independentTestBtn);
			hl->addWidget(pairedTestBtn);
			hl->addStretch();
		} else
			setWindowTitle(tr("One sample t-Test"));
	}

	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_OS_WIN
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
#endif

	QGridLayout *gl1 = new QGridLayout();
	gl1->setColumnStretch(1, 1);

	QLabel *sample1Label = new QLabel(tr("Sample") + ":");
	gl1->addWidget(sample1Label, 0, 0);

	QStringList columnsList = t->applicationWindow()->columnsList();
	boxSample1 = new QComboBox();
	boxSample1->addItems(columnsList);
	gl1->addWidget(boxSample1, 0, 1);

	QStringList lst = t->selectedColumns();
	int selectedColumns = lst.size();
	if (selectedColumns)
		boxSample1->setCurrentIndex(boxSample1->findText(lst[0]));
	else
		boxSample1->setCurrentIndex(boxSample1->findText(t->colName(0)));

	QString mean = tr("Mean");
	if (type == StatisticTest::ChiSquareTest)
		mean = tr("Variance");
	else {
		if (twoSamples){
			mean = mean + "1 - " + mean + "2";
			sample1Label->setText(tr("Sample") + "1:");
			gl1->addWidget(new QLabel(tr("Sample") + "2:"), 1, 0);

			boxSample2 = new QComboBox();
			boxSample2->addItems(columnsList);
			gl1->addWidget(boxSample2, 1, 1);

			if (selectedColumns > 1)
				boxSample2->setCurrentIndex(boxSample2->findText(lst[1]));
		}
	}

	QGroupBox *gb2 = new QGroupBox(tr("Hypotheses"));
	QGridLayout *gl2 = new QGridLayout(gb2);
	gl2->setColumnStretch(2, 1);

	gl2->addWidget(new QLabel(tr("Null") + ":"), 0, 0);

	meanLabel = new QLabel(mean + "  = ");
	gl2->addWidget(meanLabel, 0, 1);

	boxMean = new DoubleSpinBox();
	gl2->addWidget(boxMean, 0, 2);

	gl2->addWidget(new QLabel(tr("Alternate") + ":"), 1, 0);

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
	boxSignificance->setValue(app->d_stats_significance_level);
	gl2->addWidget(boxSignificance, 4, 1);

	boxConfidenceInterval = new CollapsiveGroupBox(tr("Confidence &Interval(s)"));

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
	boxConfidenceInterval->setChecked(app->d_stats_confidence);

	if (type == StatisticTest::StudentTest){
		boxPowerAnalysis = new CollapsiveGroupBox(tr("&Power Analysis"));
		QGridLayout *gl4 = new QGridLayout(boxPowerAnalysis);

		boxPowerLevel = new DoubleSpinBox();
		boxPowerLevel->setRange(0.01, 0.99);
		boxPowerLevel->setSingleStep(0.01);
		boxPowerLevel->setValue(0.05);

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

		connect(boxOtherSampleSize, SIGNAL(toggled(bool)), boxSampleSize, SLOT(setEnabled(bool)));
		boxPowerAnalysis->setChecked(app->d_stats_power);
	}

	outputSettingsBox = new CollapsiveGroupBox("&" + tr("Output Settings"));
	QGridLayout *gl4 = new QGridLayout(outputSettingsBox);

	boxResultsTable = new QCheckBox(tr("&Table"));
	boxResultsTable->setChecked(app->d_stats_result_table);
	gl4->addWidget(boxResultsTable, 0, 0);

	tableNameLineEdit = new QLineEdit();
	tableNameLineEdit->setEnabled(false);
	gl4->addWidget(tableNameLineEdit, 0, 1);

	boxNoteWindow = new QCheckBox(tr("&Notes Window"));
	boxNoteWindow->setChecked(app->d_stats_result_notes);
	gl4->addWidget(boxNoteWindow, 1, 0);

	noteNameLineEdit = new QLineEdit();
	noteNameLineEdit->setEnabled(false);
	gl4->addWidget(noteNameLineEdit, 1, 1);

	boxResultsLog = new QCheckBox(tr("Results &Log"));
	boxResultsLog->setChecked(app->d_stats_result_log);
	gl4->addWidget(boxResultsLog, 2, 0);

	showStatisticsBox = new QCheckBox(tr("&Descriptive Statistics" ));
	showStatisticsBox->setChecked(app->d_descriptive_stats);
	enableDescriptiveStatistics();
	gl4->addWidget(showStatisticsBox, 2, 1);
	outputSettingsBox->setChecked(app->d_stats_output);

	buttonOk = new QPushButton(tr( "&Compute" ));

	QHBoxLayout *hl2 = new QHBoxLayout();
	hl2->addStretch();
	hl2->addWidget(buttonOk);
	hl2->addStretch();

	QVBoxLayout *vl = new QVBoxLayout(this);
	if (twoSamples){
		boxOtherSampleSize->hide();
		boxSampleSize->hide();
		vl->addLayout(hl);
	}
	vl->addLayout(gl1);
	vl->addWidget(gb2);
	vl->addWidget(boxConfidenceInterval);
	if (type == StatisticTest::StudentTest)
		vl->addWidget(boxPowerAnalysis);
	vl->addWidget(outputSettingsBox);
	vl->addStretch();
	vl->addLayout(hl2);

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(buttonAddLevel, SIGNAL(clicked()), this, SLOT(addConfidenceLevel()));
	connect(leftTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(rightTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(bothTailButton, SIGNAL(toggled(bool)), this, SLOT(updateMeanLabel()));
	connect(boxMean, SIGNAL(valueChanged(double)), this, SLOT(updateMeanLabels(double)));
	connect(boxResultsTable, SIGNAL(toggled(bool)), tableNameLineEdit, SLOT(setEnabled(bool)));
	connect(boxNoteWindow, SIGNAL(toggled(bool)), noteNameLineEdit, SLOT(setEnabled(bool)));
	connect(boxNoteWindow, SIGNAL(toggled(bool)), this, SLOT(enableDescriptiveStatistics()));
	connect(boxResultsLog, SIGNAL(toggled(bool)), this, SLOT(enableDescriptiveStatistics()));
}

void StudentTestDialog::enableDescriptiveStatistics()
{
	showStatisticsBox->setEnabled(boxNoteWindow->isChecked() || boxResultsLog->isChecked());
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
	sbox->setValue(floor(0.5*(100 + sb->value())));

	gl->addWidget(sbox, rows, 1);
}

void StudentTestDialog::updateMeanLabel()
{
	QString s = tr("Mean");
	if (d_test_type == StatisticTest::ChiSquareTest)
		s = tr("Variance");
	else if (d_two_samples)
		s = s + "1 - " + s + "2";

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
	if (d_test_type == StatisticTest::ChiSquareTest)
		acceptChiSquareTest();
	else if (d_test_type == StatisticTest::StudentTest)
		acceptStudentTest();
}

void StudentTestDialog::acceptStudentTest()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();

	tTest stats(app, boxMean->value(), boxSignificance->value());
	if (!stats.setData(boxSample1->currentText()))
		return;

	if (d_two_samples){
		if (!stats.setSample2(boxSample2->currentText(), pairedTestBtn->isChecked()))
			return;
	}

	StatisticTest::Tail tail = StatisticTest::Left;
	if (bothTailButton->isChecked())
		tail = StatisticTest::Both;
	else if (rightTailButton->isChecked())
		tail = StatisticTest::Right;
	stats.setTail(tail);

	stats.showDescriptiveStatistics(showStatisticsBox->isEnabled() && showStatisticsBox->isChecked());

	int p = app->d_decimal_digits;
	QLocale l = app->locale();

	QString sep = "\t\t";
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";
	QString s = stats.logInfo();

	if (boxConfidenceInterval->isChecked()){
		QGridLayout *gl = (QGridLayout *)boxConfidenceInterval->layout();
		int rows = gl->rowCount();

		s += "\n";
		if (d_two_samples)
			s += tr("Confidence Interval for Difference of Means");
		else
			s += tr("Confidence Interval for Mean");
		s += "\n\n";
		s += tr("Level") + sep + tr("Lower Limit") + sep + tr("Upper Limit") + "\n";
		s += sep1;
		for (int i = 0; i < rows; i++){
			DoubleSpinBox *sb = (DoubleSpinBox *)gl->itemAtPosition (i, 1)->widget();
			double level = sb->value();
			s += l.toString(level) + sep + l.toString(stats.lcl(level), 'g', p);
			s += sep + l.toString(stats.ucl(level),'g', p) + "\n";
		}
		s += sep1;
	}

	if (boxPowerAnalysis->isChecked()){
		double power = stats.power(boxPowerLevel->value());
		s += "\n" + tr("Power Analysis") + "\n\n";
		s += tr("Alpha") + sep + tr("Sample Size") + sep + tr("Power") + "\n";
		s += sep1;
		s += l.toString(boxPowerLevel->value(), 'g', 6) + sep + QString::number(stats.dataSize()) + sep + l.toString(power, 'g', p) + "   (" + tr("actual") + ")\n";
		if (!d_two_samples && boxOtherSampleSize->isChecked()){
			int size = boxSampleSize->value();
			power = stats.power(boxPowerLevel->value(), size);
			s += l.toString(boxPowerLevel->value(), 'g', 6) + sep + QString::number(size) + sep + l.toString(power, 'g', p) + "\n";
		}
		s += sep1;
	}

	outputResults(&stats, s);
}

void StudentTestDialog::acceptChiSquareTest()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();

	ChiSquareTest stats(app, boxMean->value(), boxSignificance->value());
	if (!stats.setData(boxSample1->currentText()))
		return;

	StatisticTest::Tail tail = StatisticTest::Left;
	if (bothTailButton->isChecked())
		tail = StatisticTest::Both;
	else if (rightTailButton->isChecked())
		tail = StatisticTest::Right;

	stats.setTail(tail);
	stats.showDescriptiveStatistics(showStatisticsBox->isEnabled() && showStatisticsBox->isChecked());

	int p = app->d_decimal_digits;
	QLocale l = app->locale();

	QString sep = "\t\t";
	QString sep1 = "-----------------------------------------------------------------------------------------------------------------------------\n";
	QString s = stats.logInfo();

	if (boxConfidenceInterval->isChecked()){
		QGridLayout *gl = (QGridLayout *)boxConfidenceInterval->layout();
		int rows = gl->rowCount();

		s += "\n";
		s += tr("Confidence Intervals for Variance");
		s += "\n\n";
		s += tr("Level") + sep + tr("Lower Limit") + sep + tr("Upper Limit") + "\n";
		s += sep1;
		for (int i = 0; i < rows; i++){
			DoubleSpinBox *sb = (DoubleSpinBox *)gl->itemAtPosition (i, 1)->widget();
			double level = sb->value();
			s += l.toString(level) + sep + l.toString(stats.lcl(level), 'g', p);
			s += sep + l.toString(stats.ucl(level),'g', p) + "\n";
		}
		s += sep1;
	}

	outputResults(&stats, s);
}

void StudentTestDialog::outputResults(StatisticTest* stats, const QString& s)
{
	if (!stats)
		return;

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (boxResultsLog->isChecked())
		app->updateLog(s);

	if (boxResultsTable->isChecked()){
		QString name = tableNameLineEdit->text();
		if (!d_table)
			d_table = stats->resultTable(name);
		else {
			if (d_table->objectName() != name){
				Table *t = app->table(name);
				if (t){
					d_table = t;
					stats->outputResultsTo(d_table);
				} else
					d_table = stats->resultTable(name);
			} else
				stats->outputResultsTo(d_table);
		}

		if (d_table && name.isEmpty())
			tableNameLineEdit->setText(d_table->objectName());
	}

	if (boxNoteWindow->isChecked()){
		QString name = noteNameLineEdit->text();
		if (!d_note){
			d_note = app->newNote(name);
			d_note->setText(s);
		} else {
			if (d_note->objectName() != name){
				Note *n = qobject_cast<Note *>(app->window(name));
				if (n){
					d_note = n;
					d_note->currentEditor()->append(s);
				} else {
					d_note = app->newNote(name);
					d_note->setText(s);
				}
			} else
				d_note->currentEditor()->append(s);
		}

		if (d_note && name.isEmpty())
			noteNameLineEdit->setText(d_note->objectName());
	}
}

void StudentTestDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (app){
		app->d_stats_significance_level = boxSignificance->value();
		app->d_stats_result_table = boxResultsTable->isChecked();
		app->d_stats_result_log = boxResultsLog->isChecked();
		app->d_stats_result_notes = boxNoteWindow->isChecked();
		app->d_descriptive_stats = showStatisticsBox->isChecked();
		app->d_stats_output = outputSettingsBox->isChecked();
		app->d_stats_confidence = boxConfidenceInterval->isChecked();
		if (d_test_type == StatisticTest::StudentTest)
			app->d_stats_power = boxPowerAnalysis->isChecked();
	}

	e->accept();
}
