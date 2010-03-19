/***************************************************************************
    File                 : OriginParser.cpp
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

#include "OriginParser.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/if.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace boost::algorithm;
using namespace Origin;

vector<Origin::SpreadSheet>::size_type OriginParser::findSpreadByName(const string& name) const
{
	vector<SpreadSheet>::const_iterator it = find_if(speadSheets.begin(), 
													 speadSheets.end(),
													 bind(iequals<string, string>,
														  bind(&SpreadSheet::name, _1),
														  name,
														  locale()));
	return it != speadSheets.end() ? it - speadSheets.begin() : -1;
}

vector<Origin::Excel>::size_type OriginParser::findExcelByName(const string& name) const
{
	vector<Excel>::const_iterator it = find_if(excels.begin(),
											   excels.end(),
											   bind(iequals<string, string>,
													bind(&Excel::name, _1),
													name,
													locale()));
	return it != excels.end() ? it - excels.begin() : -1;
}

vector<Origin::SpreadColumn>::size_type OriginParser::findSpreadColumnByName(vector<Origin::SpreadSheet>::size_type spread, const string& name) const
{
	vector<SpreadColumn>::const_iterator it = find_if(speadSheets[spread].columns.begin(),
													  speadSheets[spread].columns.end(),
													  bind(&SpreadColumn::name, _1) == name);
	return it != speadSheets[spread].columns.end() ? it - speadSheets[spread].columns.begin() : -1;
}

vector<Origin::SpreadColumn>::size_type OriginParser::findExcelColumnByName(vector<Origin::Excel>::size_type excel, vector<Origin::SpreadSheet>::size_type sheet, const string& name) const
{
	vector<SpreadColumn>::const_iterator it = find_if(excels[excel].sheets[sheet].columns.begin(),
													  excels[excel].sheets[sheet].columns.end(),
													  bind(&SpreadColumn::name, _1) == name);
	return it != excels[excel].sheets[sheet].columns.end() ? it - excels[excel].sheets[sheet].columns.begin() : -1;
}

vector<Origin::Matrix>::size_type OriginParser::findMatrixByName(const string& name) const
{
	vector<Matrix>::const_iterator it = find_if(matrixes.begin(),
												matrixes.end(),
												bind(iequals<string, string>,
													 bind(&Matrix::name, _1),
													 name,
													 locale()));
	return it != matrixes.end() ? it - matrixes.begin() : -1;
}

vector<Origin::Function>::size_type OriginParser::findFunctionByName(const string& name) const
{
	vector<Function>::const_iterator it = find_if(functions.begin(),
												  functions.end(),
												  bind(iequals<string, string>,
													   bind(&Function::name, _1),
													   name,
													   locale()));
	return it != functions.end() ? it - functions.begin() : -1;
}

pair<string, string> OriginParser::findDataByIndex(unsigned int index) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		for(vector<SpreadColumn>::const_iterator it1 = it->columns.begin(); it1 != it->columns.end(); ++it1)
		{
			if(it1->index == index)
				return make_pair("T_" + it->name, it1->name);
		}
	}

	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if(it->index == index)
			return make_pair("M_" + it->name, it->name);
	}

	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		for(vector<SpreadSheet>::const_iterator it1 = it->sheets.begin(); it1 != it->sheets.end(); ++it1)
		{
			for(vector<SpreadColumn>::const_iterator it2 = it1->columns.begin(); it2 != it1->columns.end(); ++it2)
			{
				if(it2->index == index)
					return make_pair("E_" + it->name, it2->name);
			}
		}
	}

	for(vector<Function>::const_iterator it = functions.begin(); it != functions.end(); ++it)
	{
		if(it->index == index)
			return make_pair("F_" + it->name, it->name);
	}

	return pair<string, string>();
}

pair<ProjectNode::NodeType, string> OriginParser::findObjectByIndex(unsigned int index) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		if(it->objectID == index)
			return make_pair(ProjectNode::SpreadSheet, it->name);
	}

	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if(it->objectID == index)
			return make_pair(ProjectNode::Matrix, it->name);
	}

	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		if(it->objectID == index)
			return make_pair(ProjectNode::Excel, it->name);
	}

	for(vector<Graph>::const_iterator it = graphs.begin(); it != graphs.end(); ++it)
	{
		if(it->objectID == index)
			return make_pair(ProjectNode::Graph, it->name);
	}

	return pair<ProjectNode::NodeType, string>();
}

void OriginParser::convertSpreadToExcel(vector<Origin::SpreadSheet>::size_type spread)
{
	//add new Excel sheet
	excels.push_back(Excel(speadSheets[spread].name, speadSheets[spread].label, speadSheets[spread].maxRows, speadSheets[spread].hidden, speadSheets[spread].loose));

	for(vector<SpreadColumn>::iterator it = speadSheets[spread].columns.begin(); it != speadSheets[spread].columns.end(); ++it)
	{
		unsigned int index = 0;
		int pos = it->name.find_last_of("@");
		if(pos != -1)
		{
			index = lexical_cast<int>(it->name.substr(pos + 1).c_str()) - 1;
			it->name = it->name.substr(0, pos);
		}

		if(excels.back().sheets.size() <= index)
			excels.back().sheets.resize(index + 1);

		excels.back().sheets[index].columns.push_back(*it);
	}

	speadSheets.erase(speadSheets.begin() + spread);
}
