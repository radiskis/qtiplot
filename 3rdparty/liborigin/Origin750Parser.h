/***************************************************************************
    File                 : Origin750Parser.h
    --------------------------------------------------------------------
	Copyright            : (C) 2007-2008 Alex Kargovsky, Stefan Gerlach, 
						   Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin 7.5 file parser class

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


#ifndef ORIGIN_750_PARSER_H
#define ORIGIN_750_PARSER_H

#include "OriginParser.h"
#include "endianfstream.hh"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
using namespace std;
using namespace Origin;

class Origin750Parser : public OriginParser
{
public:
	Origin750Parser(const string& fileName);
	bool parse();

protected:
	void readSpreadInfo();
	void readExcelInfo();
	void readMatrixInfo();
	void readGraphInfo();
	unsigned int readGraphAxisInfo(GraphAxis& axis);
	void readGraphGridInfo(GraphGrid& grid);
	void readGraphAxisBreakInfo(GraphAxisBreak& axis_break);
	void readGraphAxisFormatInfo(GraphAxisFormat& format);
	void readGraphAxisTickLabelsInfo(GraphAxisTick& tick);
	void readGraphAxisPrefixSuffixInfo(const string& sec_name, unsigned int size, GraphLayer& layer);
	void readProjectTree();
	virtual void readProjectTreeFolder(tree<ProjectNode>::iterator parent);
	void readWindowProperties(Window& window, unsigned int size);
	virtual void readColorMap(ColorMap& colorMap);
	void skipLine();
	inline double stringToDouble(const string& s)
	{
		string s1 = s;
		size_t pos = s.find(",");
		if (pos != string::npos)
			s1.replace(pos, 1, ".");
		return strtod(s1.c_str(), NULL);
	}

	inline boost::posix_time::ptime doubleToPosixTime(double jdt)
	{
		return boost::posix_time::ptime(boost::gregorian::date(boost::gregorian::gregorian_calendar::from_julian_day_number(jdt+1)), boost::posix_time::seconds((jdt-(int)jdt)*86400));
	}

	unsigned int objectIndex;
	iendianfstream file;

	unsigned int d_file_size;
	unsigned int d_colormap_offset;
	unsigned int d_start_offset;
};

#endif // ORIGIN_750_PARSER_H
