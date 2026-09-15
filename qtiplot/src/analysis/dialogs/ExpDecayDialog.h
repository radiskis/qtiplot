/***************************************************************************
    File                 : ExpDecayDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
	Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Fit exponential decay dialog

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
#ifndef EXPDECAYDIALOG_H
#define EXPDECAYDIALOG_H

#include <QDialog>

class QPushButton;
class QComboBox;
class QLabel;
class Graph;
class ColorButton;
class Fit;
class DoubleSpinBox;

//! Fit exponential decay dialog
class ExpDecayDialog : public QDialog
{
    Q_OBJECT

public:
    ExpDecayDialog( int type, QWidget* parent = nullptr, Qt::WindowFlags fl = {} );

public slots:
	void fit();
	void setGraph(Graph *g);

private slots:
	void activateCurve(const QString&);
	void changeDataRange();

private:
    void closeEvent (QCloseEvent *) override;

    Fit *fitter = nullptr;
	Graph *graph = nullptr;
	int slopes = 1;

	QPushButton* buttonFit = nullptr;
	QPushButton* buttonCancel = nullptr;
	QComboBox* boxName = nullptr;
	DoubleSpinBox* boxAmplitude = nullptr;
	DoubleSpinBox* boxFirst = nullptr;
	DoubleSpinBox* boxSecond = nullptr;
	DoubleSpinBox* boxThird = nullptr;
	DoubleSpinBox* boxStart = nullptr;
	DoubleSpinBox* boxYOffset = nullptr;
	QLabel* thirdLabel = nullptr, *dampingLabel = nullptr;
	ColorButton* boxColor = nullptr;
};

#endif
