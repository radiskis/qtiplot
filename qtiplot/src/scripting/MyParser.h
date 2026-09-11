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
	static void setCLocale(bool cLocale);

	void DefineVar(const string_type& name, double* var) { Parser::DefineVar(name, var); }
	void DefineVar(const char* name, double* var) { Parser::DefineVar(QString(name).toStdWString(), var); }

	void DefineConst(const string_type& name, double val) { Parser::DefineConst(name, val); }
	void DefineConst(const char* name, double val) { Parser::DefineConst(QString(name).toStdWString(), val); }

	void SetExpr(const string_type& expr) { Parser::SetExpr(expr); }
	void SetExpr(const char* expr) { Parser::SetExpr(QString(expr).toStdWString()); }

	void DefineFun(const char* name, double (*fun)(double)) { Parser::DefineFun(QString(name).toStdWString(), fun); }
	void DefineFun(const char* name, double (*fun)(double, double)) { Parser::DefineFun(QString(name).toStdWString(), fun); }
	void DefineFun(const char* name, double (*fun)(double, double, double)) { Parser::DefineFun(QString(name).toStdWString(), fun); }
	void DefineFun(const char* name, double (*fun)(const mu::char_type*), bool allowOpt = true) { Parser::DefineFun(QString(name).toStdWString(), fun, allowOpt); }
	void DefineFun(const char* name, double (*fun)(const mu::char_type*, double), bool allowOpt = true) { Parser::DefineFun(QString(name).toStdWString(), fun, allowOpt); }
	void DefineFun(const char* name, double (*fun)(const mu::char_type*, double, double), bool allowOpt = true) { Parser::DefineFun(QString(name).toStdWString(), fun, allowOpt); }

	template<typename T>
	void DefineFunUserData(const char* name, T fun, void* pUserData, bool allowOpt = true) {
		Parser::DefineFunUserData(QString(name).toStdWString(), fun, pUserData, allowOpt);
	}
	template<typename T>
	void DefineFunUserData(const string_type& name, T fun, void* pUserData, bool allowOpt = true) {
		Parser::DefineFunUserData(name, fun, pUserData, allowOpt);
	}

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
