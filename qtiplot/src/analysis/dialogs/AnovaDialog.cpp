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
#include <Anova.h>
#include <ApplicationWindow.h>
#include <Folder.h>
#include <DoubleSpinBox.h>

#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QLabel>

AnovaDialog::AnovaDialog(Table *t, bool twoWay, QWidget* parent, Qt::WFlags fl )
	: QDialog( parent, fl),
	d_two_way(twoWay)
{
	setObjectName( "AnovaDialog" );
	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

	availableSamples = new QListWidget();
	availableSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	availableSamples->addItems(((ApplicationWindow *)parent)->columnsList());

	selectedSamples = new QTreeWidget();
	selectedSamples->setRootIsDecorated(false);
	selectedSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	if (twoWay){
		setWindowTitle(tr("Two Way ANOVA"));
		selectedSamples->setHeaderLabels (QStringList() << tr("Sample") << tr("Factor A Level") << tr("Factor B Level"));
		selectedSamples->header()->setResizeMode(QHeaderView::ResizeToContents);
		selectedSamples->setUniformRowHeights(true);

		QString levelA = tr("A");
		QString levelB = tr("B");
		QStringList aLevels, bLevels;
		for (int i = 0; i < 2; i++){
			aLevels << levelA + QString::number(i + 1);
			bLevels << levelB + QString::number(i + 1);
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
		setWindowTitle(tr("One Way ANOVA"));
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

	QGridLayout *gl3 = new QGridLayout();
	gl3->addWidget(new QLabel(tr("Significance Level")), 0, 0);
	boxSignificance = new DoubleSpinBox();
	boxSignificance->setRange(0.0, 1.0);
	boxSignificance->setDecimals(2);
	boxSignificance->setSingleStep(0.01);
	boxSignificance->setValue(0.05);
	gl3->addWidget(boxSignificance, 0, 1);

	showStatisticsBox = new QCheckBox(tr("Show &Descriptive Statistics" ));
	showStatisticsBox->setChecked(!twoWay);
	gl3->addWidget(showStatisticsBox, 1, 0);
	gl3->setRowStretch(2, 1);

	gl1->addLayout(gl3, 2, 0);

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
	Anova anova((ApplicationWindow *)parent(), boxSignificance->value(), d_two_way);
	for(int i = 0; i < selectedSamples->topLevelItemCount(); i++){
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

	anova.showDescriptiveStatistics(showStatisticsBox->isChecked());

	if (d_two_way){
		anova.setAnovaTwoWayType(boxModel->currentIndex());
		anova.showAnovaTwoWayInteractions(showInteractionsBox->isChecked());
	}

	if (!anova.run())
		return;
}
