/***************************************************************************
	File                 : BaselineDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Subtract baseline dialog

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
#include <BaselineDialog.h>
#include <ApplicationWindow.h>
#include <Folder.h>
#include <Graph.h>
#include <PlotCurve.h>

#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>

BaselineDialog::BaselineDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setObjectName( "BaselineDialog" );
	setWindowTitle(tr("QtiPlot") + " - " + tr("Baseline"));
	setAttribute(Qt::WA_DeleteOnClose);
	setSizeGripEnabled( true );

	QGroupBox *gb1 = new QGroupBox(tr("Create Baseline"));
	QGridLayout *gl1 = new QGridLayout(gb1);

	btnAutomatic = new QRadioButton(tr("&Automatic"));
	btnAutomatic->setChecked(true);
	gl1->addWidget(btnAutomatic, 0, 0);

	boxPoints = new QSpinBox();

	QHBoxLayout *hb1 = new QHBoxLayout();
	hb1->addStretch();
	hb1->addWidget(new QLabel(tr("Points")));
	hb1->addWidget(boxPoints);
	gl1->addLayout(hb1, 0, 1);

	btnEquation = new QRadioButton(tr("User Defined &Equation Y ="));
	btnEquation->setChecked(false);
	gl1->addWidget(btnEquation, 1, 0);

	boxEquation = new QLineEdit();
	gl1->addWidget(boxEquation, 1, 1);

	btnDataset = new QRadioButton(tr("Existing &Dataset"));
	btnDataset->setChecked(false);
	gl1->addWidget(btnDataset, 2, 0);

	boxTableName = new QComboBox();
	boxColumnName = new QComboBox();

	QHBoxLayout *hb0 = new QHBoxLayout();
	hb0->addWidget(boxTableName);
	hb0->addWidget(boxColumnName);
	gl1->addLayout(hb0, 2, 1);

	gl1->setRowStretch(3, 1);

	ApplicationWindow *app = (ApplicationWindow *)parent;
	boxTableName->addItems(app->tableNames());
	updateTableColumns(0);

	buttonCreate = new QPushButton(tr( "Create &Baseline" ));
	buttonCreate->setDefault( true );
	buttonSubtract = new QPushButton(tr( "&Subtract" ));
	buttonUndo = new QPushButton(tr( "&Undo Subtraction" ));
	buttonModify = new QPushButton(tr( "&Modify" ));
	buttonCancel = new QPushButton(tr( "&Close" ));

	QVBoxLayout *vl = new QVBoxLayout();
	vl->addWidget(buttonCreate);
	vl->addWidget(buttonSubtract);
	vl->addWidget(buttonUndo);
	vl->addWidget(buttonModify);
	vl->addWidget(buttonCancel);
	vl->addStretch();

	QHBoxLayout *hb = new QHBoxLayout(this);
	hb->addWidget(gb1);
	hb->addLayout(vl);

	connect(boxTableName, SIGNAL(activated(int)), this, SLOT(updateTableColumns(int)));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close()));

	/*connect( btnDataset, SIGNAL( toggled(bool) ), boxOffset, SLOT( setDisabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxReferenceName, SLOT( setEnabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxColumnName, SLOT( setEnabled(bool) ) );
	connect( btnDataset, SIGNAL( toggled(bool) ), boxCurrentFolder, SLOT( setEnabled(bool) ) );

	connect( btnValue, SIGNAL( toggled(bool) ), boxOffset, SLOT( setEnabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxReferenceName, SLOT( setDisabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxColumnName, SLOT( setDisabled(bool) ) );
	connect( btnValue, SIGNAL( toggled(bool) ), boxCurrentFolder, SLOT( setDisabled(bool) ) );

	connect( boxCurrentFolder, SIGNAL( toggled(bool) ), this, SLOT( setCurrentFolder(bool) ) );

	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( interpolate() ) );
	*/
}


void BaselineDialog::updateTableColumns(int tabnr)
{
	boxColumnName->clear();

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (!app)
		return;

	Table *t = app->table(boxTableName->itemText(tabnr));
	if (!t)
		return;

	QStringList columns;
	for (int i = 0; i < t->numCols(); i++)
		columns << t->colLabel(i);

	boxColumnName->addItems(columns);
}


void BaselineDialog::setGraph(Graph *g)
{
	if (!g)
		return;

	graph = g;
	DataCurve *c = g->dataCurve(0);
	if (c)
		boxPoints->setValue(c->dataSize());

	/*boxInputName->addItems(g->analysableCurvesList());

	QString selectedCurve = g->selectedCurveTitle();
	if (!selectedCurve.isEmpty())
		boxInputName->setCurrentIndex(boxInputName->findText(selectedCurve));*/

	connect (graph, SIGNAL(destroyed()), this, SLOT(close()));
}
