/***************************************************************************
    File                 : DoubleSpinBox.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A Customized Double Spin Box

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
#ifndef DoubleSpinBox_H
#define DoubleSpinBox_H

#include <QDoubleSpinBox>
#include <QCheckBox>

class DoubleSpinBox : public QDoubleSpinBox
{
public:
    DoubleSpinBox(const char format = 'g', QWidget * parent = 0);
	virtual QString textFromValue ( double value ) const {return locale().toString(value, d_format, decimals());};
	virtual double valueFromText ( const QString & text ) const {return locale().toDouble(text);};
	virtual QValidator::State validate ( QString & input, int & pos ) const;

private:
    const char d_format;
};

class RangeLimitBox : public QWidget
{
public:
	enum LimitType{LeftLimit, RightLimit};

    RangeLimitBox(LimitType type, QWidget * parent = 0);
	void setDecimals(int prec){d_spin_box->setDecimals(prec);};
	double value();
	bool isChecked(){return d_checkbox->isChecked();};

private:
    DoubleSpinBox *d_spin_box;
    QCheckBox *d_checkbox;
	LimitType d_type;
};
#endif // FITDIALOG_H

