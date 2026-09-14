/***************************************************************************
	File                 : AnovaDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : ANOVA dialog

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
#ifndef AnovaDialog_H
#define AnovaDialog_H

#include <QDialog>
#include <StatisticTest.h>
#include "Ref.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QPushButton;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QTreeWidget;
class CollapsiveGroupBox;
class DoubleSpinBox;
class Table;
class Note;

//! Filter options dialog
class AnovaDialog : public QDialog
{
    Q_OBJECT

public:
	AnovaDialog(QWidget* parent, Table *t, const StatisticTest::TestType& type = StatisticTest::AnovaTest, bool twoWay = false);

private slots:
	void accept();
	void addData();
	void removeData();
	void showCurrentFolder(bool);
	void updateLevelBoxes();
	void enableDescriptiveStatistics();

private:
	void closeEvent(QCloseEvent*);
	void acceptNormalityTest();
#ifdef HAVE_TAMUANOVA
	void acceptAnova();
#endif
	void outputResults(StatisticTest* stats, const QString& s);
	StatisticTest::TestType d_test_type = StatisticTest::AnovaTest;
	bool d_two_way = false;

	QListWidget* availableSamples = nullptr;
	QTreeWidget *selectedSamples = nullptr;

	QCheckBox *currentFolderBox = nullptr;
	QCheckBox *showInteractionsBox = nullptr;
	QCheckBox *showStatisticsBox = nullptr;

	QPushButton* btnAdd = nullptr;
	QPushButton* btnRemove = nullptr;
	QPushButton* buttonOk = nullptr;

	QComboBox* boxModel = nullptr;

	QSpinBox *aLevelsBox = nullptr;
	QSpinBox *bLevelsBox = nullptr;
	DoubleSpinBox* boxSignificance = nullptr;

	CollapsiveGroupBox *outputSettingsBox = nullptr;
	QCheckBox *boxResultsTable = nullptr;
	QLineEdit *tableNameLineEdit = nullptr;
	QCheckBox *boxResultsLog = nullptr;
	QCheckBox *boxNoteWindow = nullptr;
	QLineEdit *noteNameLineEdit = nullptr;

	Ref<Table> d_table;
	Ref<Note> d_note;
};

#endif
