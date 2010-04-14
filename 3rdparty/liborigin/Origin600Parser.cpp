/***************************************************************************
	File                 : Origin600Parser.cpp
    --------------------------------------------------------------------
	Copyright            : (C) 2010 Ion Vasilief
	Email (use @ for *)  : ion_vasilief*yahoo.fr
	Description          : Origin 6.0 file parser class (uses code from file
							Origin750Parser.cpp written by Alex Kargovsky)
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

#include "Origin600Parser.h"
#include <cstring>
#include <sstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <logging.hpp>
#include <QString>

using namespace boost;

inline boost::posix_time::ptime doubleToPosixTime(double jdt)
{
	return boost::posix_time::ptime(boost::gregorian::date(boost::gregorian::gregorian_calendar::from_julian_day_number(jdt+1)), boost::posix_time::seconds((jdt-(int)jdt)*86400));
}

Origin600Parser::Origin600Parser(const string& fileName)
:	Origin610Parser(fileName)
{}

bool Origin600Parser::parse()
{
	unsigned int dataIndex = 0;

	// get length of file:
	file.seekg (0, ios::end);
	d_file_size = file.tellg();

	stringstream out;
	unsigned char c;
	/////////////////// find column ///////////////////////////////////////////////////////////
	file.seekg(0xE, ios_base::beg);
	unsigned int size;
	file >> size;

	file.seekg(1 + size + 1 + 5, ios_base::cur);
	file >> size;

	file.seekg(1, ios_base::cur);
	BOOST_LOG_(1, format("	[column found = %d/0x%X @ 0x%X]") % size % size % (unsigned int) file.tellg());

	unsigned int colpos = file.tellg();
	unsigned int current_col = 1, nr = 0, nbytes = 0;
	
	while(size > 0 && size <= 0x8C){// should be 0x72, 0x73 or 0x83 ?
		//////////////////////////////// COLUMN HEADER /////////////////////////////////////////////

		short data_type;
		char data_type_u;
		unsigned int oldpos = file.tellg();

		file.seekg(oldpos + 0x16, ios_base::beg);
		file >> data_type;

		file.seekg(oldpos + 0x3F, ios_base::beg);
		file >> data_type_u;
		
		char valuesize;
		file.seekg(oldpos + 0x3D, ios_base::beg);
		file >> valuesize;

		BOOST_LOG_(1, format("	[valuesize = %d @ 0x%X]") % (int)valuesize % ((unsigned int) file.tellg()-1));
		if(valuesize <= 0)
		{
			BOOST_LOG_(1, format("	WARNING : found strange valuesize of %d") % (int)valuesize);
			valuesize = 10;
		}

		file.seekg(oldpos + 0x58, ios_base::beg);
		BOOST_LOG_(1, format("	[Spreadsheet @ 0x%X]") % (unsigned int) file.tellg());

		string name(25, 0);
		file >> name;

		string::size_type pos = name.find_last_of("_");
		string columnname;
		if(pos != string::npos){
			columnname = name.substr(pos + 1);
			name.resize(pos);
		}

		BOOST_LOG_(1, format("	NAME: %s") % name.c_str());

		unsigned int spread = 0;
		if(columnname.empty()){
			BOOST_LOG_(1, "NO COLUMN NAME FOUND! Must be a Matrix or Function.");
			////////////////////////////// READ matrixes or functions ////////////////////////////////////

			BOOST_LOG_(1, format("	[position @ 0x%X]") % (unsigned int) file.tellg());
			// TODO
			short signature;
			file >> signature;
			BOOST_LOG_(1, format("	SIGNATURE : %02X ") % signature);


			file.seekg(oldpos + size + 1, ios_base::beg);
			file >> size;
			file.seekg(1, ios_base::cur);
			size /= valuesize;
			BOOST_LOG_(1, format("	SIZE = %d") % size);

			switch(signature)
			{
			case 0x50CA:
			case 0x70CA:
			case 0x50F2:
			case 0x50E2:
			case 0x50C8:
			case 0x50E7:
			case 0x50DB:
			case 0x50DC:
			case 0xAE2:
			case 0xAF2:
			case 0xACA:

				BOOST_LOG_(1, "NEW MATRIX");
				matrixes.push_back(Matrix(name, dataIndex));
				//BOOST_LOG_(1, format("MATRIX %s has dataIndex: %d") % name.c_str() % dataIndex);

				++dataIndex;

				BOOST_LOG_(1, "VALUES :");
				out.str(size > 100 ? "matrix too big..." : string());

				switch(data_type)
				{
				case 0x6001://double
					for(unsigned int i = 0; i < size; ++i)
					{
						double value;				
						file >> value;
						matrixes.back().data.push_back((double)value);
						//if(size < 100)
							//out << format("%g ") % matrixes.back().data.back();
					}
					//BOOST_LOG_(1, out.str());
					break;
				case 0x6003://float
					for(unsigned int i = 0; i < size; ++i)
					{
						float value;						
						file >> value;
						matrixes.back().data.push_back((double)value);
						if(size < 100)
							out << format("%g ") % matrixes.back().data.back();
					}
					BOOST_LOG_(1, out.str());
					break;
				case 0x6801://int
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned int value;						
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							int value;							
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				case 0x6803://short
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned short value;						
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							short value;							
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				case 0x6821://char
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned char value;						
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							char value;							
							file >> value;
							matrixes.back().data.push_back((double)value);
							if(size < 100)
								out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				default:
					BOOST_LOG_(1, format("UNKNOWN MATRIX DATATYPE: %02X SKIP DATA") % data_type);
					file.seekg(valuesize*size, ios_base::cur);
					matrixes.pop_back();
				}
				break;

			case 0x10C8:
				BOOST_LOG_(1, "NEW FUNCTION");
				functions.push_back(Function(name, dataIndex));
				++dataIndex;

				file >> functions.back().formula.assign(valuesize, 0);
				oldpos = file.tellg();
				short t;

				file.seekg(colpos + 0xA, ios_base::beg);
				file >> t;

				if(t == 0x1194)
					functions.back().type = Function::Polar;

				file.seekg(colpos + 0x21, ios_base::beg);
				file >> functions.back().totalPoints;

				file >> functions.back().begin;
				double d;
				file >> d;
				functions.back().end = functions.back().begin + d*(functions.back().totalPoints - 1);

				BOOST_LOG_(1, format("FUNCTION %s : %s") % functions.back().name.c_str() % functions.back().formula.c_str());
				BOOST_LOG_(1, format(" interval %g : %g, number of points %d") % functions.back().begin % functions.back().end % functions.back().totalPoints);

				file.seekg(oldpos, ios_base::beg);
				break;

			default:
				BOOST_LOG_(1, format("UNKNOWN SIGNATURE: %.2X SKIP DATA") % signature);
				file.seekg(valuesize*size, ios_base::cur);
				++dataIndex;

				if(valuesize != 8 && valuesize <= 16)
					file.seekg(2, ios_base::cur);
			}
		}
		else
		{	// worksheet
			if(speadSheets.size() == 0 || findSpreadByName(name) == -1)
			{
				BOOST_LOG_(1, "NEW SPREADSHEET");
				current_col = 1;
				speadSheets.push_back(SpreadSheet(name));
				spread = speadSheets.size() - 1;
				speadSheets.back().maxRows = 0;
			}
			else
			{
				spread = findSpreadByName(name);

				current_col = speadSheets[spread].columns.size();

				if(!current_col)
					current_col = 1;
				++current_col;
			}
			speadSheets[spread].columns.push_back(SpreadColumn(columnname, dataIndex));
			string::size_type sheetpos = speadSheets[spread].columns.back().name.find_last_of("@");
			if(sheetpos != string::npos){
				unsigned int sheet = lexical_cast<int>(columnname.substr(sheetpos + 1).c_str());
				if( sheet > 1){
					speadSheets[spread].multisheet = true;

					speadSheets[spread].columns.back().name = columnname;
					speadSheets[spread].columns.back().sheet = sheet - 1;

					if (speadSheets[spread].sheets < sheet)
						speadSheets[spread].sheets = sheet;
				}
			}
			BOOST_LOG_(1, format("SPREADSHEET = %s SHEET = %d COLUMN NAME = %s (%d) (@0x%X)") % name % speadSheets[spread].columns.back().sheet % columnname % current_col % (unsigned int)file.tellg());

			++dataIndex;

			////////////////////////////// SIZE of column /////////////////////////////////////////////
			file.seekg(oldpos + size + 1, ios_base::beg);

			file >> nbytes;
			if(fmod(nbytes, (double)valuesize)>0)
			{
				BOOST_LOG_(1, "WARNING: data section could not be read correct");
			}
			nr = nbytes / valuesize;
			BOOST_LOG_(1, format("	[number of rows = %d (%d Bytes) @ 0x%X]") % nr % nbytes % (unsigned int)file.tellg());

			speadSheets[spread].maxRows<nr ? speadSheets[spread].maxRows=nr : 0;

			////////////////////////////////////// DATA ////////////////////////////////////////////////
			file.seekg(1, ios_base::cur);

			BOOST_LOG_(1, format("	[data @ 0x%X]") % (unsigned int)file.tellg());
			out.str(string());
			for(unsigned int i = 0; i < nr; ++i)
			{
				double value;
				if(valuesize <= 8)	// Numeric, Time, Date, Month, Day
				{
					file >> value;
					out << format("%g ") % value;
					speadSheets[spread].columns[(current_col-1)].data.push_back(value);
				}
				else if((data_type & 0x100) == 0x100) // Text&Numeric
				{
					file >> c;
					file.seekg(1, ios_base::cur);
					if(c == 0) //value
					{
						file >> value;
						out << format("%g ") % value;
						speadSheets[spread].columns[(current_col-1)].data.push_back(value);
						file.seekg(valuesize - 10, ios_base::cur);
					}
					else //text
					{
						string stmp(valuesize - 2, 0);
						file >> stmp;
						if(stmp.find(0x0E) != string::npos) // try find non-printable symbol - garbage test
							stmp = string();
						speadSheets[spread].columns[(current_col-1)].data.push_back(stmp);
						out << format("%s ") % stmp;
					}
				}
				else //text
				{
					string stmp(valuesize, 0);
					file >> stmp;
					if(stmp.find(0x0E) != string::npos) // try find non-printable symbol - garbage test
						stmp = string();
					speadSheets[spread].columns[(current_col-1)].data.push_back(stmp);
					out << format("%s ") % stmp;
				}
			}
			BOOST_LOG_(1, out.str());
		}

		if(nbytes > 0 || columnname.empty())
		{
			file.seekg(1, ios_base::cur);
		}

		file >> size;
		file.seekg(1 + size + (size > 0 ? 1 : 0), ios_base::cur);

		file >> size;

		file.seekg(1, ios_base::cur);
		BOOST_LOG_(1, format("	[column found = %d/0x%X (@ 0x%X)]") % size % size %((unsigned int) file.tellg()-5));
		colpos = file.tellg();
	}

	////////////////////////////////////////////////////////////////////////////
	////////////////////// HEADER SECTION //////////////////////////////////////

	unsigned int POS = (unsigned int)file.tellg()-11;
	BOOST_LOG_(1, "\nHEADER SECTION");
	BOOST_LOG_(1, format("	[position @ 0x%X]") % POS);

	POS += 0xB;
	file.seekg(POS, ios_base::beg);
	unsigned int tableId = 0;
	while(POS < d_file_size){
		POS = file.tellg();

		file >> size;
		if(size == 0)
			break;

		file.seekg(POS + 0x7, ios_base::beg);
		string name(25, 0);
		file >> name;

		file.seekg(POS, ios_base::beg);

		if(findSpreadByName(name) != -1){
			readSpreadInfo();
			tableId++;
		} else if(findMatrixByName(name) != -1)
			readMatrixInfo();
		else if(findExcelByName(name) != -1)
			readExcelInfo();
		else if (!readGraphInfo()){
			BOOST_LOG_(1, format("		%s is NOT A GRAPH, trying to read next SPREADSHEET...") % name);
			findObjectInfoSectionByName(POS, speadSheets[tableId].name);
			readSpreadInfo();
			tableId++;
		}
		POS = file.tellg();
	}

	readNotes();
	readResultsLog();

	file.seekg(1 + 4*5 + 0x10 + 1, ios_base::cur);
	try {
		readProjectTree();
	} catch(...) {}

	BOOST_LOG_(1, "Done parsing");
	BOOST_LOG_FINALIZE();

	return true;
}

void Origin600Parser::readProjectTree()
{
	readProjectTreeFolder(projectTree.begin());

	BOOST_LOG_(1, "Origin project Tree");
	for(tree<ProjectNode>::iterator it = projectTree.begin(projectTree.begin()); it != projectTree.end(projectTree.begin()); ++it)
	{
		BOOST_LOG_(1, string(projectTree.depth(it) - 1, ' ') + (*it).name);
	}
}

OriginParser* createOrigin600Parser(const string& fileName)
{
	return new Origin600Parser(fileName);
}
