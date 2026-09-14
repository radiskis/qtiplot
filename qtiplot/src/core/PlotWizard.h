/***************************************************************************
    File                 : PlotWizard.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2004 - 2010 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A wizard type dialog to create new plots

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


#ifndef PLOTWIZARD_H
#define PLOTWIZARD_H

#include <QDialog>
class QGroupBox;
class QPushButton;
class QListWidget;
class QComboBox;

//! A wizard type dialog class to create new plots
class PlotWizard : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl Qt window flags
	 */
    PlotWizard( QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

private:
				//! Button "Plot"
    QPushButton* buttonOk = nullptr;
				//! Button "<->xErr"
	QPushButton* buttonXErr = nullptr;
				//! Button "<->yErr"
	QPushButton* buttonYErr = nullptr;
				//! Button "Delete curve"
	QPushButton* buttonDelete = nullptr;
				//! Button "cancel"
	QPushButton* buttonCancel = nullptr;
				//! Button "<->X"
	QPushButton* buttonX = nullptr;
				//! Button "<->Y"
	QPushButton* buttonY = nullptr;
				//! Button "New curve"
	QPushButton* buttonNew = nullptr;
				//! Button "<->Z"
	QPushButton* buttonZ = nullptr;
				//! Button group defining the layout
    QGroupBox* groupBox1 = nullptr;
				//! Button group defining the layout
	QGroupBox* groupBox2 = nullptr;
				//! Button group defining the layout
	QGroupBox* groupBox3 = nullptr;
				//! Combo box to select the table
    QComboBox* boxTables = nullptr;
				//! List of the columns in the selected table
	QListWidget *columnsList = nullptr;
				//! List of the plots to generate
	QListWidget *plotAssociations = nullptr;

	//! Internal list of columns (contains all columns in all tables)
	QStringList columns;

public slots:
	//! Update the columns list box to contain the columns of 'table'
	void changeColumnsList(const QString &table);
	//! Insert a list of tables into the tables combo box
	void insertTablesList(const QStringList& tables);
	//! Set the contents of the columns list box to the strings in 'cols'
	void setColumnsListBoxContents(const QStringList& cols);
	//! Set the internal columns list to 'cols'
	void setColumnsList(const QStringList& cols);
	//! Add new curve
	void addCurve();
	//! Delete selected curve
	void removeCurve();
	//! Add column as X
	void addXCol();
	//! Add column as Y
	void addYCol();
	//! Add column as X error
	void addXErrCol();
	//! Add column as Y error
	void addYErrCol();
	//! Accept settings, close the dialog
	void accept() override;
	//! Add column as Z
	void addZCol();

private:
	void plot2D(const QStringList&);
	void plot3D(const QStringList&);
	void plot3DRibbon(const QStringList&);
	//! Display a warning that a new curve must be added first before the selection of the columns
	bool noCurves();
    QSize sizeHint() const override;
};

#endif //  PLOTWIZARD_H
