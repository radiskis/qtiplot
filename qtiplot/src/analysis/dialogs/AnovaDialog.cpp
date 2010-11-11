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

#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

AnovaDialog::AnovaDialog(Table *t, bool twoWay, QWidget* parent, Qt::WFlags fl )
	: QDialog( parent, fl),
	d_two_way(twoWay)
{
	setObjectName( "AnovaDialog" );
	if (twoWay)
		setWindowTitle(tr("Two Way ANOVA"));
	else
		setWindowTitle(tr("One Way ANOVA"));

	setSizeGripEnabled( true );
	setAttribute(Qt::WA_DeleteOnClose);

	availableSamples = new QListWidget();
	availableSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	availableSamples->addItems(((ApplicationWindow *)parent)->columnsList());

	selectedSamples = new QListWidget();
	selectedSamples->setSelectionMode (QAbstractItemView::ExtendedSelection);
	selectedSamples->addItems(t->selectedColumns());

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
	gl1->addWidget(new QLabel(tr("Available Data")), 0, 0);
	gl1->addWidget(new QLabel(tr("Selected Data")), 0, 2);

	gl1->addWidget(availableSamples, 1, 0);
	gl1->addLayout(vl1, 1, 1);
	gl1->addWidget(selectedSamples, 1, 2);

	currentFolderBox = new QCheckBox(tr("Show current &folder only" ));
	gl1->addWidget(currentFolderBox, 2, 0);

	QHBoxLayout *hl1 = new QHBoxLayout();
	hl1->addWidget(new QLabel(tr("Significance Level")));
	boxSignificance = new DoubleSpinBox();
	boxSignificance->setRange(0.0, 1.0);
	boxSignificance->setDecimals(2);
	boxSignificance->setSingleStep(0.01);
	boxSignificance->setValue(0.05);
	hl1->addWidget(boxSignificance, 1);

	gl1->addLayout(hl1, 2, 2);

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

void AnovaDialog::addData()
{
	QList<QListWidgetItem *> items = availableSamples->selectedItems();
	foreach(QListWidgetItem *item, items){
		QString s = item->text();
		if (selectedSamples->findItems(s, Qt::MatchExactly).isEmpty())
			selectedSamples->addItem(s);
	}
}

void AnovaDialog::removeData()
{
	QList<QListWidgetItem *> items = selectedSamples->selectedItems();
	foreach(QListWidgetItem *item, items)
		selectedSamples->takeItem(selectedSamples->row(item));
}

void AnovaDialog::accept()
{
	Anova anova((ApplicationWindow *)parent(), boxSignificance->value());
	for(int i = 0; i < selectedSamples->count(); i++){
		QListWidgetItem *item = selectedSamples->item(i);
		if (item && !anova.addSample(item->text()))
			return;
	}

	if (!anova.run())
		return;
}
