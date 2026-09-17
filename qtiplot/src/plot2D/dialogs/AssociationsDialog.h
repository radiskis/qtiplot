/***************************************************************************
File                 : AssociationsDialog.h
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2006 - 2012 by Ion Vasilief
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Plot associations dialog
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
#ifndef ASSOCIATIONSDIALOG_H
#define ASSOCIATIONSDIALOG_H

#include <QDialog>

class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;
#include <QStringList>
class Table;
class Graph;
class MdiSubWindow;

//! Plot associations dialog
class AssociationsDialog : public QDialog
{
	Q_OBJECT

public:
	AssociationsDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

	void initTablesList(QList<MdiSubWindow *> lst, int curve);
	void setGraph(Graph *g);

private slots:
	void updateTable(int index);
	void updateCurves();
	void accept() override;

private:
	void changePlotAssociation(int curve, const QStringList& lst);
	QStringList plotAssociation(const QString& text);
	void updateColumnTypes();
	void uncheckCol(int col);
	void updatePlotAssociation(int row, int col);
	Table *findTable(int index) const;
	bool eventFilter(QObject *object, QEvent *e) override;

	QList <MdiSubWindow*> tables;
	Table *active_table = nullptr;
	Graph *graph = nullptr;
	QStringList plotAssociationsList;
	QList<int> curvesIndicesList;

	QLabel* tableCaptionLabel = nullptr;
	QTableWidget *table = nullptr;
	QPushButton *btnOK = nullptr, *btnCancel = nullptr, *btnApply = nullptr;
	QListWidget* associations = nullptr;
};
#endif // ASSOCIATIONSDIALOG_H
