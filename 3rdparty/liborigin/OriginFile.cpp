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

#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "OriginFile.h"

using namespace boost;

OriginFile::OriginFile(const char* fileName)
:	version(0)
{
	ifstream file(fileName, ios_base::binary);

	if(!file.is_open())
	{
		printf("Could not open %s!\n", fileName);
		return;
	}
	
	char vers[4];
	file.seekg(0x7, ios_base::beg);
	file >> vers;
	version = lexical_cast<int>(vers);
	file.close();


	ofstream debug("opjfile.log", ios_base::trunc);

	if(!debug.is_open())
	{
		printf("Could not open log file!\n", fileName);
		return;
	}

	debug << format("	[version = %d]") % version << endl;

	// translate version
	if(version >= 130 && version <= 140)			// 4.1
		version = 410;
	else if(version == 210)							// 5.0
		version = 500;
	else if(version == 2625)						// 6.0
		version = 600;
	else if(version == 2627)						// 6.0 SR1
		version = 601;
	else if(version == 2630)						// 6.0 SR4
		version = 604;
	else if(version == 2635)						// 6.1
		version = 610;
	else if(version == 2656)						// 7.0
		version = 700;
	else if(version == 2672)						// 7.0 SR3
		version = 703;
	else if(version >= 2766 && version <= 2769)		// 7.5
		version = 750;
	else
	{
		debug << format("Found unknown project version %d") % version << endl;
		debug << "Please contact the author of opj2dat" << endl;
	}
	debug << format("Found project version %.2f") % (version/100.0) << endl;
	debug.close();

	switch(version)
	{
	case 750:
		parser.reset(createOrigin750Parser(fileName));
		break;
	default:
		parser.reset(createOriginDefaultParser(fileName));
	    break;
	}
}

int OriginFile::parse()
{
	return parser->parse();
}