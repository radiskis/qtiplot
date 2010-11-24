/***************************************************************************
	File                 : StudentTestDialog.h
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
#ifndef StudentTestDialog_H
#define StudentTestDialog_H

#include <QDialog>
#include <StatisticTest.h>

class QGroupBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class DoubleSpinBox;
class Table;
class Note;
class CollapsiveGroupBox;

//! Filter options dialog
class StudentTestDialog : public QDialog
{
    Q_OBJECT

public:
	StudentTestDialog(const StatisticTest::TestType& type, Table *t, bool twoSamples = false, QWidget* parent = 0);

private slots:
    void accept();
	void updateMeanLabels(double);
	void updateMeanLabel();
	void addConfidenceLevel();
	void enableDescriptiveStatistics();

private:
	void closeEvent(QCloseEvent*);
	void acceptStudentTest();
	void acceptChiSquareTest();
	void outputResults(StatisticTest* stats, const QString& s);

	StatisticTest::TestType d_test_type;
	bool d_two_samples;

	QComboBox *boxSample1;
	QComboBox *boxSample2;
	QRadioButton *bothTailButton;
	QRadioButton *leftTailButton;
	QRadioButton *rightTailButton;
	QLabel *meanLabel;
	QLabel *bothTailLabel;
	QLabel *leftTailLabel;
	QLabel *rightTailLabel;
	CollapsiveGroupBox *boxConfidenceInterval;
	QPushButton* buttonAddLevel;
	QPushButton* buttonOk;
	CollapsiveGroupBox *boxPowerAnalysis;
	QSpinBox *boxSampleSize;
	QCheckBox *boxOtherSampleSize;
	DoubleSpinBox* boxMean;
	DoubleSpinBox* boxSignificance;
	DoubleSpinBox* boxPowerLevel;

	QRadioButton *independentTestBtn;
	QRadioButton *pairedTestBtn;

	CollapsiveGroupBox *outputSettingsBox;
	QCheckBox *boxResultsTable;
	QLineEdit *tableNameLineEdit;
	QCheckBox *boxResultsLog;
	QCheckBox *boxNoteWindow;
	QLineEdit *noteNameLineEdit;
	QCheckBox *showStatisticsBox;

	Table *d_table;
	Note *d_note;
};

#endif
