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

class QGroupBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QRadioButton;
class DoubleSpinBox;
class Table;

//! Filter options dialog
class StudentTestDialog : public QDialog
{
    Q_OBJECT

public:
	StudentTestDialog(Table *t, bool twoSamples = false, QWidget* parent = 0, Qt::WFlags fl = 0);

private slots:
    void accept();
	void updateMeanLabels(double);
	void updateMeanLabel();
	void addConfidenceLevel();

private:
	Table *d_table;
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
	QGroupBox *boxConfidenceInterval;
	QPushButton* buttonAddLevel;
	QPushButton* buttonOk;
	QCheckBox *boxPowerAnalysis;
	DoubleSpinBox* boxMean;
	DoubleSpinBox* boxSignificance;
	DoubleSpinBox* boxPowerLevel;
};

#endif
