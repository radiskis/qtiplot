/***************************************************************************
    File                 : muParserScript.h
	Project              : QtiPlot
    --------------------------------------------------------------------

    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Evaluate mathematical expressions using muParser

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
#ifndef MUPARSER_SCRIPT_H
#define MUPARSER_SCRIPT_H

#include "ScriptingEnv.h"
#include "Script.h"

#include <MyParser.h>
#include "math.h"
#include <gsl/gsl_sf.h>
//#include <q3asciidict.h>
#include <QMap>

//! TODO
class muParserScript: public Script
{
  Q_OBJECT

  public:
    muParserScript(ScriptingEnv *env, const QString &code, QObject *context=0, const QString &name="<input>");
    ~muParserScript();

  public slots:
    bool compile(bool asFunction=true);
    QVariant eval();
    double evalSingleLine();
    QString evalSingleLineToString(const QLocale& locale, char f, int prec);
    bool exec();
    bool setQObject(QObject *val, const char *name);
    bool setInt(int val, const char* name);
    bool setDouble(double val, const char* name);
    double* defineVariable(const char *name, double val = 0.0);
    int codeLines(){return muCode.size();};

  private:
	double avg(const QString &arg, int start = 0, int end = -1);
	double sum(const QString &arg, int start = 0, int end = -1);
	double min(const QString &arg, int start = 0, int end = -1);
	double max(const QString &arg, int start = 0, int end = -1);
	double col(const QString &arg);
    double tablecol(const QString &arg);
    double cell(int row, int col);
    double tableCell(int col, int row);
    double *addVariable(const char *name);
    double *addVariableR(const char *name);
	static double mu_avg(void *pUserData, const mu::char_type *arg, double start, double end) {
		return static_cast<muParserScript*>(pUserData)->avg(QString::fromWCharArray(arg), qRound(start - 1), qRound(end - 1));
	}
	static double mu_sum(void *pUserData, const mu::char_type *arg, double start, double end) {
		return static_cast<muParserScript*>(pUserData)->sum(QString::fromWCharArray(arg), qRound(start - 1), qRound(end - 1));
	}
	static double mu_min(void *pUserData, const mu::char_type *arg, double start, double end) {
		return static_cast<muParserScript*>(pUserData)->min(QString::fromWCharArray(arg), qRound(start - 1), qRound(end - 1));
	}
	static double mu_max(void *pUserData, const mu::char_type *arg, double start, double end) {
		return static_cast<muParserScript*>(pUserData)->max(QString::fromWCharArray(arg), qRound(start - 1), qRound(end - 1));
	}
	static double mu_col(void *pUserData, const mu::char_type *arg) {
		return static_cast<muParserScript*>(pUserData)->col(QString::fromWCharArray(arg));
	}
	static double mu_cell(void *pUserData, double row, double col) {
		return static_cast<muParserScript*>(pUserData)->cell(qRound(row), qRound(col));
	}
	static double mu_tableCell(void *pUserData, double col, double row) {
		return static_cast<muParserScript*>(pUserData)->tableCell(qRound(col), qRound(row));
	}
	static double mu_tablecol(void *pUserData, const mu::char_type *arg) {
		return static_cast<muParserScript*>(pUserData)->tablecol(QString::fromWCharArray(arg));
	}
	static double *mu_addVariable(const mu::char_type *name, void *pUserData) {
		return static_cast<muParserScript*>(pUserData)->addVariable(QString::fromWCharArray(name).toLocal8Bit().constData());
	}
	static double *mu_addVariableR(const mu::char_type *name, void *pUserData) {
		return static_cast<muParserScript*>(pUserData)->addVariableR(QString::fromWCharArray(name).toLocal8Bit().constData());
	}
	static QString compileColArg(const QString& in);

	MyParser parser, rparser;
	QMap<QString, double*> variables, rvariables;
	QStringList muCode;
};

#endif
