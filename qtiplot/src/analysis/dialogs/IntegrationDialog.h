/***************************************************************************
	File                 : IntegrationDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Integration options dialog

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
#ifndef INTEGRATIONDIALOG_H
#define INTEGRATIONDIALOG_H

#include <QDialog>

class QPushButton;
class QComboBox;
class QCheckBox;
class QSpinBox;
class Graph;
class Table;
class DoubleSpinBox;

//! Integration options dialog
class IntegrationDialog : public QDialog
{
    Q_OBJECT

public:
	IntegrationDialog(Graph *g, QWidget* parent = 0, Qt::WFlags fl = 0 );
	IntegrationDialog(Table *t, QWidget* parent = 0, Qt::WFlags fl = 0 );

public slots:
	void activateCurve(const QString&);
	void integrate();
	void changeDataRange();

private:
	void closeEvent (QCloseEvent * e );

	void setGraph(Graph *g);
	void setTable(Table *t);
	void integrateCurve();
	void integrateTable();

	Graph *d_graph;
	Table *d_table;
	QPushButton* buttonFit;
	QPushButton* buttonCancel;
	QComboBox* boxName;
	QCheckBox *boxSortData;
	QCheckBox *boxShowPlot;
	QCheckBox *boxShowTable;
	QSpinBox *boxStartRow;
	QSpinBox *boxEndRow;
	DoubleSpinBox* boxStart;
	DoubleSpinBox* boxEnd;
};

#endif
