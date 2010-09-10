/***************************************************************************
    File                 : CurvesDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Add/remove curves dialog

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
#ifndef CURVESDIALOG_H
#define CURVESDIALOG_H

#include <QDialog>
#include <QTreeWidget>

class QComboBox;
class QListWidget;
class QPushButton;
class QCheckBox;
class QTreeWidget;
class TreeWidgetFolderItem;
class Graph;
class Folder;
class Matrix;
class Table;
class ApplicationWindow;

//! Add/remove curves dialog
class CurvesDialog : public QDialog
{
    Q_OBJECT

public:
	enum TreeItemType{FolderItem, ColumnItem, TableItem, MatrixItem};
    CurvesDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );

	void setGraph(Graph *graph);

private slots:
	void addCurves();
	void removeCurves();
	int curveStyle();
	void showCurveRangeDialog();
	void showPlotAssociations();
	void showFunctionDialog();
	void showCurveBtn(int);
    void enableAddBtn();
	void enableContentsBtns();
	void showCurveRange(bool);
	void updateCurveRange();
	void showCurrentFolder(bool);
	void raiseCurve();
	void shiftCurveBy(int offset = 1);

private:
	void closeEvent(QCloseEvent*);
	void addFolderItems(Folder *f, QTreeWidgetItem* parent = 0);

    void init();

	bool addCurveFromMatrix(Matrix *m);
	bool addCurveFromTable(ApplicationWindow *app, Table *t, const QString& name);

    QSize sizeHint() const;
    void contextMenuEvent(QContextMenuEvent *);

	Graph *d_graph;

    QPushButton* btnAdd;
    QPushButton* btnRemove;
    QPushButton* btnOK;
    QPushButton* btnCancel;
	QPushButton* btnAssociations;
	QPushButton* btnEditFunction;
	QPushButton* btnRange;
    QTreeWidget* available;
    QListWidget* contents;
	QComboBox* boxStyle;
	QComboBox* boxMatrixStyle;
	QCheckBox* boxShowRange;
	QCheckBox* boxShowCurrentFolder;
	QPushButton* btnUp;
	QPushButton* btnDown;
	QComboBox* boxXAxis;
	QComboBox* boxYAxis;
};

/*****************************************************************************
 *
 * Class TreeWidgetFolderItem
 *
 *****************************************************************************/
//! Folders list item class
class TreeWidgetFolderItem : public QTreeWidgetItem
{
public:
	TreeWidgetFolderItem( QTreeWidget *parent, Folder *f );
	TreeWidgetFolderItem( QTreeWidgetItem *parent, Folder *f );

	Folder *folder() { return myFolder; };

protected:
	Folder *myFolder;
};

#endif // CurvesDialog_H
