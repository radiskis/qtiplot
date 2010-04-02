/***************************************************************************
	File                 : Origin800Parser.h
    --------------------------------------------------------------------
	Copyright            : (C) 2010 Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Origin 8.0 file parser class

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


#ifndef ORIGIN_800_PARSER_H
#define ORIGIN_800_PARSER_H

#include "OriginParser.h"
#include "endianfstream.hh"

using namespace Origin;

class Origin800Parser : public OriginParser
{
public:
	Origin800Parser(const string& fileName);
	bool parse();

private:
	void readSpreadInfo();
	void readExcelInfo();
	void readMatrixInfo();
	void readGraphInfo();
	void skipObjectInfo();
	void skipLine();
	unsigned int readGraphAxisInfo(GraphAxis& axis);
	void readGraphGridInfo(GraphGrid& grid);
	void readGraphAxisBreakInfo(GraphAxisBreak& axis_break);
	void readGraphAxisFormatInfo(GraphAxisFormat& format);
	void readGraphAxisTickLabelsInfo(GraphAxisTick& tick);
	void readProjectTree();
	void readProjectTreeFolder(tree<ProjectNode>::iterator parent);
	void readWindowProperties(Window& window, unsigned int size);
	void readColorMap(ColorMap& colorMap);
	void readResultsLog();
	void readNotes();

	bool findSection(const string& name, int length, int maxLength = 0);
	int findObjectInfoSectionByName(int start, const string& name);
	int findStringPos(const string& name);
	pair<ProjectNode::NodeType, string> findObjectByInfoPosition(unsigned int index) const;

	unsigned int objectIndex;
	iendianfstream file;

	int d_file_size;
	vector<unsigned int> object_info_positions;
};

#endif // ORIGIN_800_PARSER_H
