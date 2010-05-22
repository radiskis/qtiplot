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

class OriginParser
{
public:
	virtual ~OriginParser() {};
	virtual bool parse() = 0;
	void setFileVersion(unsigned int version){fileVersion = version;};

	vector<Origin::SpreadSheet>::size_type findSpreadByName(const string& name) const;
	vector<Origin::Matrix>::size_type findMatrixByName(const string& name) const;
	vector<Origin::Function>::size_type findFunctionByName(const string& name) const;
	vector<Origin::Excel>::size_type findExcelByName(const string& name) const;

protected:
	vector<Origin::SpreadColumn>::size_type findSpreadColumnByName(vector<Origin::SpreadSheet>::size_type spread, const string& name) const;
	vector<Origin::SpreadColumn>::size_type findExcelColumnByName(vector<Origin::Excel>::size_type excel, vector<Origin::SpreadSheet>::size_type sheet, const string& name) const;
	pair<string, string> findDataByIndex(unsigned int index) const;
	pair<Origin::ProjectNode::NodeType, string> findObjectByIndex(unsigned int index) const;
	void convertSpreadToExcel(vector<Origin::SpreadSheet>::size_type spread);

	int findColumnByName(int spread, const string& name);

public:
	vector<Origin::SpreadSheet> speadSheets;
	vector<Origin::Matrix> matrixes;
	vector<Origin::Excel> excels;
	vector<Origin::Function> functions;
	vector<Origin::Graph> graphs;
	vector<Origin::Note> notes;
	tree<Origin::ProjectNode> projectTree;
	string resultsLog;
	unsigned int windowsCount;
	unsigned int fileVersion;
};

OriginParser* createOriginDefaultParser(const string& fileName);
OriginParser* createOrigin600Parser(const string& fileName);
OriginParser* createOrigin610Parser(const string& fileName);
OriginParser* createOrigin700Parser(const string& fileName);
OriginParser* createOrigin750Parser(const string& fileName);
OriginParser* createOrigin800Parser(const string& fileName);
OriginParser* createOrigin810Parser(const string& fileName);

#endif // ORIGIN_PARSER_H
