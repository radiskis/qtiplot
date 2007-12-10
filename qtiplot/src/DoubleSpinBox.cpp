/***************************************************************************
    File                 : DoubleSpinBox.cpp
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
#include "DoubleSpinBox.h"
#include <QHBoxLayout>
#include <float.h>

DoubleSpinBox::DoubleSpinBox(const char format, QWidget * parent)
:QDoubleSpinBox(parent),
d_format(format)
{
    setRange(-DBL_MAX, DBL_MAX);
}

QValidator::State DoubleSpinBox::validate(QString & input, int & ) const
{
	if (input.lower().contains("e"))
		return QValidator::Acceptable;

	bool ok = false;
	locale().toDouble (input, &ok);
	if (ok)
		return QValidator::Acceptable;
	else
		return QValidator::Invalid;
}

/*****************************************************************************
 *
 * Class RangeLimitBox
 *
 *****************************************************************************/

RangeLimitBox::RangeLimitBox(LimitType type, QWidget * parent)
:QWidget(parent),
d_type(type)
{
    d_checkbox = new QCheckBox();
	d_spin_box = new DoubleSpinBox('g');
	d_spin_box->setSpecialValueText(" ");
	d_spin_box->setValue(-DBL_MAX);
	d_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	d_spin_box->setEnabled(false);

	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(d_checkbox);
	l->addWidget(d_spin_box);

	setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d_spin_box);
	connect(d_checkbox, SIGNAL(toggled(bool)), d_spin_box, SLOT(setEnabled(bool)));
}

double RangeLimitBox::value()
{
	if (d_checkbox->isChecked())
		return d_spin_box->value();

	double val = -DBL_MAX;
	if (d_type == RightLimit)
		return DBL_MAX;
	return val;
}
