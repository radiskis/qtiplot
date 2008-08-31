/***************************************************************************
    File                 : OriginParser.h
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Alex Kargovsky
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su
    Description          : Origin file parser base class

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

#ifndef ORIGIN_PARSER_H
#define ORIGIN_PARSER_H

#include "OriginObj.h"
#include "tree.hh"
#include <utility>

using namespace std;

class OriginParser
{
public:
	virtual ~OriginParser();
	virtual bool parse() = 0;

	int findSpreadByName(const char* name) const;
	int findMatrixByName(const char* name) const;
	int findFunctionByName(const char* name) const;
	int findExcelByName(const char* name) const;

protected:
	int findSpreadColumnByName(int spread, const char* name) const;
	int findExcelColumnByName(int excel, int sheet, const char* name) const;
	pair<string, string> findDataByIndex(int index) const;
	string findObjectByIndex(int index) const;
	void convertSpreadToExcel(int spread);

public:
	vector<Origin::SpreadSheet> speadSheets;
	vector<Origin::Matrix> matrixes;
	vector<Origin::Excel> excels;
	vector<Origin::Function> functions;
	vector<Origin::Graph> graphs;
	vector<Origin::Note> notes;
	tree<Origin::ProjectNode> projectTree;
	string resultsLog;
};

OriginParser* createOriginDefaultParser(const string& fileName);
OriginParser* createOrigin750Parser(const string& fileName);

#endif // ORIGIN_PARSER_H
