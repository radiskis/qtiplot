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
//#include <stdio.h>
#include <cstring>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace Origin;

//compare two strings ignoring case
int strcmp_i(const char* str1, const char* str2)
{
#ifdef _WINDOWS
	return stricmp(str1, str2);
#else
	return strcasecmp(str1, str2);
#endif
}

OriginParser::~OriginParser()
{
	for(vector<Graph>::const_iterator g = graphs.begin(); g != graphs.end(); ++g)
	{
		for(vector<GraphLayer>::const_iterator l = g->layers.begin(); l != g->layers.end(); ++l)
		{
			for(vector<Bitmap>::const_iterator b = l->bitmaps.begin(); b != l->bitmaps.end(); ++b)
			{
				if(b->size > 0)
					delete b->data;
			}
		}
	}
}

int OriginParser::findSpreadByName(const char* name) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		if(0 == strcmp_i(it->name.c_str(), name))
			return it - speadSheets.begin();
	}

	return -1;
}

int OriginParser::findExcelByName(const char* name) const
{
	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		if(0 == strcmp_i(it->name.c_str(), name))
			return it - excels.begin();
	}

	return -1;
}

int OriginParser::findSpreadColumnByName(int spread, const char* name) const
{
	for(vector<SpreadColumn>::const_iterator it = speadSheets[spread].columns.begin(); it != speadSheets[spread].columns.end(); ++it)
	{
		if(it->name == name)
			return it - speadSheets[spread].columns.begin();
	}

	return -1;
}

int OriginParser::findExcelColumnByName(int iexcel, int isheet, const char* name) const
{
	for(vector<SpreadColumn>::const_iterator it = excels[iexcel].sheets[isheet].columns.begin(); it != excels[iexcel].sheets[isheet].columns.end(); ++it)
	{
		if(it->name == name)
			return it - excels[iexcel].sheets[isheet].columns.begin();
	}

	return -1;
}

int OriginParser::findMatrixByName(const char* name) const
{
	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if(0 == strcmp_i(it->name.c_str(), name))
			return it - matrixes.begin();
	}

	return -1;
}

int OriginParser::findFunctionByName(const char* name) const
{
	for(vector<Function>::const_iterator it = functions.begin(); it != functions.end(); ++it)
	{
		if(0 == strcmp_i(it->name.c_str(), name))
			return it - functions.begin();
	}

	return -1;
}

pair<string, string> OriginParser::findDataByIndex(int index) const
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

string OriginParser::findObjectByIndex(int index) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		if(it->objectID == index)
			return it->name;
	}

	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if(it->objectID == index)
			return it->name;
	}

	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		if(it->objectID == index)
			return it->name;
	}

	for(vector<Graph>::const_iterator it = graphs.begin(); it != graphs.end(); ++it)
	{
		if(it->objectID == index)
			return it->name;
	}

	return "";
}

void OriginParser::convertSpreadToExcel(int spread)
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