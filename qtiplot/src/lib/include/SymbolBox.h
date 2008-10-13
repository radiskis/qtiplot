/***************************************************************************
    File                 : SymbolBox.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Plot symbol combo box
                           
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
#ifndef SYMBOLBOX_H
#define SYMBOLBOX_H

#include <QComboBox>
#include <qwt_symbol.h>

//! A modified QComboBox allowing to choose a QwtSmbol style.
/**
 * This is a simple hack on top of the QComboBox class.
 */
class SymbolBox : public QComboBox
{
  Q_OBJECT
public:
		//! Constructor.
		/**
		 * \param parent parent widget (only affects placement of the widget)
		 */
  		SymbolBox(QWidget *parent);

  		void setStyle(const QwtSymbol::Style& c);
 		QwtSymbol::Style selectedSymbol() const;

  		static QwtSymbol::Style style(int index);
  		static int symbolIndex(const QwtSymbol::Style& style);

protected:
  		void init();

private:
		static const QwtSymbol::Style symbols[];
};

#endif
