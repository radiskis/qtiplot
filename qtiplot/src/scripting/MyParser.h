/***************************************************************************
    File                 : MyParser.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Parser class based on muParser

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
#ifndef MYPARSER_H
#define MYPARSER_H

#include <muParser.h>
#include <qstringlist.h>

using namespace mu;

class QLocale;

/*!\brief Mathematical parser class based on muParser.
 *
 * \section future Future Plans
 * Eliminate in favour of Script/ScriptingEnv.
 * This will allow you to use e.g. Python's global variables and functions everywhere.
 * Before this happens, a cleaner and more generic solution for accessing the current ScriptingEnv
 * should be implemented (maybe by making it a property of Project; see ApplicationWindow).
 */
class MyParser : public Parser
{
public:
	MyParser();
	void addGSLConstants();
	void setLocale(const QLocale& locale);
	static QLocale getLocale();

	const static QStringList functionsList();
	const static QStringList functionNamesList();
	static QString explainFunction(int index);

	double EvalRemoveSingularity(double *xvar, bool noisy = true) const;
	double DiffRemoveSingularity(double *xvar, double *a_Var,double a_fPos) const;
	static void SingularityErrorMessage(double xvar);

	class Singularity {};
	class Pole {};
};

#endif
