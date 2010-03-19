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
#include <logging.hpp>

using namespace boost;

OriginFile::OriginFile(const string& fileName)
:	fileVersion(0)
{
	BOOST_LOG_INIT((logging::trace >> logging::eol)); // log format
	logging::sink s(new ofstream("./opjfile.log"), 1);
	s.attach_qualifier(logging::log);
	BOOST_LOG_ADD_OUTPUT_STREAM(s);

	ifstream file(fileName.c_str(), ios_base::binary);

	if(!file.is_open())
	{
		cerr << format("Could not open %s!") % fileName.c_str() << endl;
		return;
	}

	string vers(4, 0);
	file.seekg(0x7, ios_base::beg);
	file >> vers;
	fileVersion = lexical_cast<int>(vers);
	file.close();

	BOOST_LOG_(1, format("	[version = %d]") % fileVersion);

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
	else if(fileVersion == 2891)						// 8.0
		fileVersion = 800;
	else if(fileVersion == 2919)						// 8.1
		fileVersion = 810;
	else
	{
		BOOST_LOG_(1, format("Found unknown project version %d") % fileVersion);
		BOOST_LOG_(1, "Please contact the authors of liborigin2");
		BOOST_LOG_FINALIZE();
		throw std::logic_error("Unknown project version");
	}
	BOOST_LOG_(1, format("Found project version %.2f") % (fileVersion/100.0));

	switch(fileVersion)
	{
	case 800:
		parser.reset(createOrigin800Parser(fileName));
		break;
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

vector<Origin::SpreadSheet>::size_type OriginFile::spreadCount() const
{
	return parser->speadSheets.size();
}

Origin::SpreadSheet& OriginFile::spread(vector<Origin::SpreadSheet>::size_type s) const
{
	return parser->speadSheets[s];
}

vector<Origin::Matrix>::size_type OriginFile::matrixCount() const
{
	return parser->matrixes.size();
}

Origin::Matrix& OriginFile::matrix(vector<Origin::Matrix>::size_type m) const
{
	return parser->matrixes[m];
}

vector<Origin::Function>::size_type OriginFile::functionCount() const
{
	return parser->functions.size();
}

vector<Origin::Function>::size_type OriginFile::functionIndex(const string& name) const
{
	return parser->findFunctionByName(name);
}

Origin::Function& OriginFile::function(vector<Origin::Function>::size_type f) const
{
	return parser->functions[f];
}

vector<Origin::Graph>::size_type OriginFile::graphCount() const
{
	return parser->graphs.size();
}

Origin::Graph& OriginFile::graph(vector<Origin::Graph>::size_type g) const
{
	return parser->graphs[g];
}

vector<Origin::Note>::size_type OriginFile::noteCount() const
{
	return parser->notes.size();
}

Origin::Note& OriginFile::note(vector<Origin::Note>::size_type n) const
{
	return parser->notes[n];
}

string OriginFile::resultsLogString() const
{
	return parser->resultsLog;
}
