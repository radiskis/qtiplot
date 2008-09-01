/***************************************************************************
    File                 : OriginFile.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2008 Stefan Gerlach
						   (C) 2007-2008 Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin file import class

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

#include "OriginFile.h"
#include <stdio.h>
#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;

OriginFile::OriginFile(const string& fileName)
:	fileVersion(0)
{
	ifstream file(fileName.c_str(), ios_base::binary);

	if(!file.is_open())
	{
		cerr << format("Could not open %s!") % fileName.c_str() << endl;
		return;
	}
	
	char vers[4];
	file.seekg(0x7, ios_base::beg);
	file >> vers;
	fileVersion = lexical_cast<int>(vers);
	file.close();


	ofstream debug("opjfile.log", ios_base::trunc);

	if(!debug.is_open())
	{
		cerr << "Could not open log file!" << endl;
		return;
	}

	debug << format("	[version = %d]") % fileVersion << endl;

	// translate version
	if(fileVersion >= 130 && fileVersion <= 140)		// 4.1
		fileVersion = 410;
	else if(fileVersion == 210)							// 5.0
		fileVersion = 500;
	else if(fileVersion == 2625)						// 6.0
		fileVersion = 600;
	else if(fileVersion == 2627)						// 6.0 SR1
		fileVersion = 601;
	else if(fileVersion == 2630)						// 6.0 SR4
		fileVersion = 604;
	else if(fileVersion == 2635)						// 6.1
		fileVersion = 610;
	else if(fileVersion == 2656)						// 7.0
		fileVersion = 700;
	else if(fileVersion == 2672)						// 7.0 SR3
		fileVersion = 703;
	else if(fileVersion >= 2766 && fileVersion <= 2769)	// 7.5
		fileVersion = 750;
	else
	{
		debug << format("Found unknown project version %d") % fileVersion << endl;
		debug << "Please contact the author of opj2dat" << endl;
	}
	debug << format("Found project version %.2f") % (fileVersion/100.0) << endl;
	debug.close();

	switch(fileVersion)
	{
	case 750:
		parser.reset(createOrigin750Parser(fileName));
		break;
	default:
		parser.reset(createOriginDefaultParser(fileName));
	    break;
	}
}

bool OriginFile::parse()
{
	return parser->parse();
}

double OriginFile::version() const
{
	return fileVersion/100.0;
}

const tree<Origin::ProjectNode>* OriginFile::project() const
{
	return &parser->projectTree;
}

int OriginFile::spreadCount() const
{
	return parser->speadSheets.size();
}

Origin::SpreadSheet& OriginFile::spread(int s) const
{
	return parser->speadSheets[s];
}

int OriginFile::matrixCount() const
{
	return parser->matrixes.size();
}	

Origin::Matrix& OriginFile::matrix(int m) const
{
	return parser->matrixes[m];
}

int OriginFile::functionCount() const
{
	return parser->functions.size();
}

int OriginFile::functionIndex(const char* s) const
{
	return parser->findFunctionByName(s);
}

Origin::Function& OriginFile::function(int f) const
{
	return parser->functions[f];
}

int OriginFile::graphCount() const
{
	return parser->graphs.size();
}

Origin::Graph& OriginFile::graph(int g) const
{
	return parser->graphs[g];
}

int OriginFile::noteCount() const
{
	return parser->notes.size();
}

Origin::Note& OriginFile::note(int n) const
{
	return parser->notes[n];
}

string OriginFile::resultsLogString() const
{
	return parser->resultsLog;
}