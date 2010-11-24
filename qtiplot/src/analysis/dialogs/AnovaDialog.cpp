/***************************************************************************
	File                 : AnovaDialog.cpp
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
#include "AnovaDialog.h"
#ifdef HAVE_TAMUANOVA
	#include <Anova.h>
#endif
#include <ShapiroWilkTest.h>
#include <ApplicationWindow.h>
#include <Folder.h>
#include <CollapsiveGroupBox.h>
#include <DoubleSpinBox.h>
#include <Note.h>

#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QLabel>

AnovaDialog::AnovaDialog(QWidget* parent, Table *t, const StatisticTest::TestType& type, bool twoWay)
	: QDialog(parent),
	d_test_type(type),
	d_two_way(twoWay)
{
	ApplicationWindow *app = (ApplicationWindow *)parent;
	d_table = 0;
	d_note = 0;

	setObjectName( "AnovaDialog" );
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

#ifdef Q_OS_WIN
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
#endif

	availableSamples = new QListWidget();
	availableSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	availableSamples->addItems(((ApplicationWindow *)parent)->columnsList());

	selectedSamples = new QTreeWidget();
	selectedSamples->setRootIsDecorated(false);
	selectedSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	if (type == StatisticTest::AnovaTest && twoWay){
		setWindowTitle(tr("Two Way ANOVA"));
		selectedSamples->setHeaderLabels (QStringList() << tr("Sample") << tr("Factor A Level") << tr("Factor B Level"));
		selectedSamples->header()->setResizeMode(QHeaderView::ResizeToContents);
		selectedSamples->setUniformRowHeights(true);

		QString levelA = tr("A");
		QString levelB = tr("B");
		QStringList aLevels, bLevels;
		for (int i = 0; i < 2; i++){
			QString l = QString::number(i + 1);
			aLevels << levelA + l;
			bLevels << levelB + l;
		}

		QStringList lst = t->selectedColumns();
		foreach(QString text, lst){
			QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(text));
			selectedSamples->addTopLevelItem(item);
			QComboBox *box = new QComboBox();
			box->addItems(aLevels);
			selectedSamples->setItemWidget(item, 1, box);

			box = new QComboBox();
			box->addItems(bLevels);
			selectedSamples->setItemWidget(item, 2, box);
		}
	} else {
		if (type == StatisticTest::AnovaTest)
			setWindowTitle(tr("One Way ANOVA"));
		else
			setWindowTitle(tr("Normality Test (Shapiro-Wilk)"));

		selectedSamples->setHeaderHidden(true);
		QStringList lst = t->selectedColumns();
		foreach(QString text, lst)
			selectedSamples->addTopLevelItem(new QTreeWidgetItem(QStringList(text)));
	}

	QVBoxLayout* vl1 = new QVBoxLayout();
	btnAdd = new QPushButton();
	btnAdd->setPixmap( QPixmap(":/next.png") );
	btnAdd->setFixedWidth (35);
	btnAdd->setFixedHeight (30);
	vl1->addWidget(btnAdd);

	btnRemove = new QPushButton();
	btnRemove->setPixmap( QPixmap(":/prev.png") );
	btnRemove->setFixedWidth (35);
	btnRemove->setFixedHeight(30);
	vl1->addWidget(btnRemove);
	vl1->addStretch();

	QGridLayout *gl1 = new QGridLayout();
	QHBoxLayout *hl1 = new QHBoxLayout();
	hl1->addWidget(new QLabel(tr("Available Data")));
	hl1->addStretch();
	currentFolderBox = new QCheckBox(tr("Show current &folder only" ));
	hl1->addWidget(currentFolderBox);
	gl1->addLayout(hl1, 0, 0);
	gl1->addWidget(new QLabel(tr("Selected Data")), 0, 2);

	gl1->addWidget(availableSamples, 1, 0);
	gl1->addLayout(vl1, 1, 1);
	gl1->addWidget(selectedSamples, 1, 2);

	QHBoxLayout *hl3 = new QHBoxLayout();
	hl3->addWidget(new QLabel(tr("Significance Level")));
	boxSignificance = new DoubleSpinBox();
	boxSignificance->setRange(0.0, 1.0);
	boxSignificance->setDecimals(2);
	boxSignificance->setSingleStep(0.01);
	boxSignificance->setValue(app->d_stats_significance_level);

	hl3->addWidget(boxSignificance);

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

	if (d_test_type == StatisticTest::AnovaTest){
		showStatisticsBox = new QCheckBox(tr("&Descriptive Statistics" ));
		showStatisticsBox->setChecked(app->d_descriptive_stats);
		showStatisticsBox->setEnabled(boxNoteWindow->isChecked() || boxResultsLog->isChecked());
		gl4->addWidget(showStatisticsBox, 2, 1);
	}
	outputSettingsBox->setChecked(app->d_stats_output);

	QVBoxLayout *vl0 = new QVBoxLayout();
	vl0->addLayout(hl3);
	vl0->addWidget(outputSettingsBox);

	gl1->addLayout(vl0, 2, 0);

	QGridLayout *gl2 = new QGridLayout();

	if (twoWay){
		aLevelsBox = new QSpinBox();
		aLevelsBox->setMinimum(2);
		aLevelsBox->setValue(2);
		connect(aLevelsBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevelBoxes()));

		QLabel *lblA = new QLabel(tr("Factor &A Levels"));
		lblA->setBuddy(aLevelsBox);
		gl2->addWidget(lblA, 0, 0);
		gl2->addWidget(aLevelsBox, 0, 1);

		bLevelsBox = new QSpinBox();
		bLevelsBox->setMinimum(2);
		bLevelsBox->setValue(2);
		connect(bLevelsBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevelBoxes()));

		QLabel *lblB = new QLabel(tr("Factor &B Levels"));
		lblB->setBuddy(bLevelsBox);
		gl2->addWidget(lblB, 1, 0);
		gl2->addWidget(bLevelsBox, 1, 1);

		QLabel *lbl = new QLabel(tr("ANOVA &Type"));
		gl2->addWidget(lbl, 2, 0);
		boxModel = new QComboBox();
		boxModel->addItems(QStringList() << tr("Fixed") << tr("Random") << tr("Mixed"));
		lbl->setBuddy(boxModel);
		gl2->addWidget(boxModel, 2, 1);

		showInteractionsBox = new QCheckBox(tr("&Interactions" ));
		showInteractionsBox->setChecked(true);
		gl2->addWidget(showInteractionsBox, 3, 1);
		gl2->setRowStretch(4, 1);
	}

	gl1->addLayout(gl2, 2, 2);

	buttonOk = new QPushButton(tr( "&Compute" ));

	QHBoxLayout *hl2 = new QHBoxLayout();
	hl2->addStretch();
	hl2->addWidget(buttonOk);
	hl2->addStretch();

	QVBoxLayout *vl = new QVBoxLayout(this);
	vl->addLayout(gl1);
	vl->addStretch();
	vl->addLayout(hl2);

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(btnAdd, SIGNAL(clicked()),this, SLOT(addData()));
	connect(btnRemove, SIGNAL(clicked()),this, SLOT(removeData()));
	connect(currentFolderBox, SIGNAL(toggled(bool)), this, SLOT(showCurrentFolder(bool)));
	connect(boxResultsTable, SIGNAL(toggled(bool)), tableNameLineEdit, SLOT(setEnabled(bool)));
	connect(boxNoteWindow, SIGNAL(toggled(bool)), noteNameLineEdit, SLOT(setEnabled(bool)));
	connect(boxNoteWindow, SIGNAL(toggled(bool)), this, SLOT(enableDescriptiveStatistics()));
	connect(boxResultsLog, SIGNAL(toggled(bool)), this, SLOT(enableDescriptiveStatistics()));
}

void AnovaDialog::enableDescriptiveStatistics()
{
	if (d_test_type != StatisticTest::AnovaTest)
		return;

	showStatisticsBox->setEnabled(boxNoteWindow->isChecked() || boxResultsLog->isChecked());
}

void AnovaDialog::showCurrentFolder(bool currentFolder)
{
	availableSamples->clear();
	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (currentFolder){
		QStringList list;
		QList<MdiSubWindow *> windows = app->currentFolder()->windowsList();
		foreach(MdiSubWindow *w, windows){
			if (!w->inherits("Table"))
				continue;

			Table *t = (Table *)w;
			for (int i = 0; i < t->numCols(); i++)
					list << t->colName(i);
		}
		availableSamples->addItems(list);
	} else
		availableSamples->addItems(app->columnsList());
}

void AnovaDialog::updateLevelBoxes()
{
	QString levelA = tr("A");
	QString levelB = tr("B");
	QStringList aLevels, bLevels;
	for (int i = 0; i < aLevelsBox->value(); i++)
		aLevels << levelA + QString::number(i + 1);
	for (int i = 0; i < bLevelsBox->value(); i++)
		bLevels << levelB + QString::number(i + 1);

	int count = selectedSamples->topLevelItemCount();
	for(int i = 0; i < count; i++){
		QTreeWidgetItem *item = selectedSamples->topLevelItem(i);
		if (!item)
			continue;

		QComboBox *box = (QComboBox *)selectedSamples->itemWidget(item, 1);
		box->clear();
		box->addItems(aLevels);

		box = (QComboBox *)selectedSamples->itemWidget(item, 2);
		box->clear();
		box->addItems(bLevels);
	}
}

void AnovaDialog::addData()
{
	QList<QListWidgetItem *> items = availableSamples->selectedItems();
	if (items.isEmpty())
		return;

	if (d_two_way){
		foreach(QListWidgetItem *item, items){
			QString s = item->text();
			if (selectedSamples->findItems(s, Qt::MatchExactly).isEmpty()){
				QTreeWidgetItem *it = new QTreeWidgetItem(QStringList(s));
				selectedSamples->addTopLevelItem(it);

				QComboBox *box = new QComboBox();
				selectedSamples->setItemWidget(it, 1, box);

				box = new QComboBox();
				selectedSamples->setItemWidget(it, 2, box);
			}
		}
		updateLevelBoxes();
	} else {
		foreach(QListWidgetItem *item, items){
			QString s = item->text();
			if (selectedSamples->findItems(s, Qt::MatchExactly).isEmpty())
				selectedSamples->addTopLevelItem(new QTreeWidgetItem(QStringList(s)));
		}
	}
}

void AnovaDialog::removeData()
{
	QList<QTreeWidgetItem *> items = selectedSamples->selectedItems();
	if (items.isEmpty())
		return;

	foreach(QTreeWidgetItem *item, items)
		selectedSamples->takeTopLevelItem(selectedSamples->indexOfTopLevelItem(item));

	if (d_two_way)
		updateLevelBoxes();
}

void AnovaDialog::accept()
{
#ifdef HAVE_TAMUANOVA
	if (d_test_type == StatisticTest::AnovaTest)
		acceptAnova();
	else
#endif
	if (d_test_type == StatisticTest::NormalityTest)
		acceptNormalityTest();
}

void AnovaDialog::acceptNormalityTest()
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	for (int i = 0; i < selectedSamples->topLevelItemCount(); i++){
		QTreeWidgetItem *item = selectedSamples->topLevelItem(i);
		if (!item)
			continue;

		QString s = QString::null;
		ShapiroWilkTest *sw = new ShapiroWilkTest(app, item->text(0));
		unsigned int n = sw->dataSize();
		if (n >= 3 && n <= 5000){
			sw->setSignificanceLevel(boxSignificance->value());
			if (i)
				s = sw->shortLogInfo();
			else
				s = sw->logInfo();
		}
		outputResults(sw, s);
		delete sw;
	}
}

#ifdef HAVE_TAMUANOVA
void AnovaDialog::acceptAnova()
{
	Anova anova((ApplicationWindow *)parent(), d_two_way, boxSignificance->value());
	for (int i = 0; i < selectedSamples->topLevelItemCount(); i++){
		QTreeWidgetItem *item = selectedSamples->topLevelItem(i);
		if (!item)
			continue;

		if (d_two_way){
			QComboBox *box1 = (QComboBox *)selectedSamples->itemWidget(item, 1);
			QComboBox *box2 = (QComboBox *)selectedSamples->itemWidget(item, 2);
			if (!anova.addSample(item->text(0), box1->currentIndex() + 1, box2->currentIndex() + 1))
				return;
		} else if (!anova.addSample(item->text(0)))
			return;
	}

	anova.showDescriptiveStatistics(showStatisticsBox->isEnabled() && showStatisticsBox->isChecked());

	if (d_two_way){
		anova.setAnovaTwoWayModel(boxModel->currentIndex());
		anova.showAnovaTwoWayInteractions(showInteractionsBox->isChecked());
	}

	if (!anova.run())
		return;

	outputResults(&anova, anova.logInfo());
}
#endif

void AnovaDialog::outputResults(StatisticTest* stats, const QString& s)
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

void AnovaDialog::closeEvent(QCloseEvent* e)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (app){
		app->d_stats_significance_level = boxSignificance->value();
		app->d_stats_result_table = boxResultsTable->isChecked();
		app->d_stats_result_log = boxResultsLog->isChecked();
		app->d_stats_result_notes = boxNoteWindow->isChecked();
		app->d_stats_output = outputSettingsBox->isChecked();
		if (d_test_type == StatisticTest::AnovaTest)
			app->d_descriptive_stats = showStatisticsBox->isChecked();
	}

	e->accept();
}
