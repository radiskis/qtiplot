/***************************************************************************
	File                 : Origin800Parser.cpp
    --------------------------------------------------------------------
	Copyright            : (C) 2010 Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
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

#include "Origin800Parser.h"
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

Origin800Parser::Origin800Parser(const string& fileName)
:	file(fileName.c_str(), ios::binary)
{
	objectIndex = 0;
}

bool Origin800Parser::parse()
{
	unsigned int dataIndex = 0;

	// get length of file:
	file.seekg (0, ios::end);
	d_file_size = file.tellg();

	stringstream out;
	unsigned char c;
	/////////////////// find column ///////////////////////////////////////////////////////////
	file.seekg(0x10 + 1, ios_base::beg);
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

				pos = name.find_first_of("@");
				if(pos != string::npos){
					name.resize(pos);
					file.seekg(valuesize*size, ios_base::cur);
					//BOOST_LOG_(1, format("MATRIX %s is multisheet, only first shit will be imported!") % name.c_str());
					unsigned int sheets = matrixes.back().sheets;
					sheets++;
					matrixes.back().sheets = sheets;
					dataIndex++;
					break;
				}

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
				{
					file.seekg(2, ios_base::cur);
				}
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
	BOOST_LOG_(1, format("	nr_spreads = %d") % speadSheets.size());
	BOOST_LOG_(1, format("	[position @ 0x%X]") % POS);

	POS += 0xB;
	file.seekg(POS, ios_base::beg);
	while(POS < d_file_size){
		POS = file.tellg();

		file >> size;
		if(size == 0)
			break;

		file.seekg(POS + 0x7, ios_base::beg);
		string name(25, 0);
		file >> name;

		file.seekg(POS, ios_base::beg);

		if(findSpreadByName(name) != -1)
			readSpreadInfo();
		else if(findMatrixByName(name) != -1)
			readMatrixInfo();
		else if(findExcelByName(name) != -1)
			readExcelInfo();
		else
			readGraphInfo();

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

void Origin800Parser::readNotes()
{
	unsigned int pos = findStringPos("H");
	file.seekg(pos, ios_base::beg);
	while(pos < d_file_size){
		int size;
		file >> size;
		if(size != 0x48)
			break;

		file.seekg(1, ios_base::cur);

		Rect rect;
		unsigned int coord;
		file >> coord;
		rect.left = coord;
		file >> coord;
		rect.top = coord;
		file >> coord;
		rect.right = coord;
		file >> coord;
		rect.bottom = coord;

		if (!rect.bottom || !rect.right)
			break;

		unsigned char state;
		file.seekg(0x8, ios_base::cur);
		file >> state;

		double creationDate, modificationDate;
		file.seekg(0x7, ios_base::cur);
		file >> creationDate;
		file >> modificationDate;

		file.seekg(0x8, ios_base::cur);
		unsigned char c;
		file >> c;

		unsigned int labellen;
		file.seekg(0x3, ios_base::cur);
		file >> labellen;

		skipLine();

		file >> size;
		file.seekg(1, ios_base::cur);

		string name(size, 0);
		file >> name;

		notes.push_back(Note(name));
		notes.back().objectID = objectIndex;
		++objectIndex;

		notes.back().frameRect = rect;
		if (creationDate >= 1e10)
			return;
		notes.back().creationDate = doubleToPosixTime(creationDate);
		if (modificationDate >= 1e10)
			return;
		notes.back().modificationDate = doubleToPosixTime(modificationDate);

		if(c == 0x01)
			notes.back().title = Window::Label;
		else if(c == 0x02)
			notes.back().title = Window::Name;
		else
			notes.back().title = Window::Both;

		if(state == 0x07)
			notes.back().state = Window::Minimized;
		else if(state == 0x0b)
			notes.back().state = Window::Maximized;

		notes.back().hidden = (state & 0x40);

		file.seekg(1, ios_base::cur);
		file >> size;

		file.seekg(1, ios_base::cur);

		if(labellen > 1){
			file >> notes.back().label.assign(labellen - 1, 0);
			file.seekg(1, ios_base::cur);
		}

		file >> notes.back().text.assign(size - labellen, 0);

		BOOST_LOG_(1, format("NOTE %d NAME: %s") % notes.size() % notes.back().name);
		BOOST_LOG_(1, format("NOTE %d LABEL: %s") % notes.size() % notes.back().label);
		BOOST_LOG_(1, format("NOTE %d TEXT: %s") % notes.size() % notes.back().text);

		file.seekg(1, ios_base::cur);
		pos = file.tellg();
	}
}

void Origin800Parser::readResultsLog()
{
	int pos = findStringPos("ResultsLog");
	if (pos < 0)
		return;

	file.seekg(pos + 12, ios_base::beg);
	unsigned int size;
	file >> size;

	file.seekg(1, ios_base::cur);
	resultsLog.resize(size);
	file >> resultsLog;
	BOOST_LOG_(1, format("Results Log: %s") % resultsLog);
}

void Origin800Parser::readSpreadInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS += 5;

	BOOST_LOG_(1, format("			[Spreadsheet SECTION (@ 0x%X)]") % POS);

	// check spreadsheet name
	file.seekg(POS + 0x2, ios_base::beg);
	string name(25, 0);
	file >> name;
	BOOST_LOG_(1, format("Spreadsheet name: %s") % name);

	int spread = findSpreadByName(name);
	speadSheets[spread].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(speadSheets[spread], size);
	speadSheets[spread].loose = false;

	unsigned int maxSearchPos = findStringPos("__WIOTN");
	unsigned int stringSize = 47;
	unsigned int columns = speadSheets[spread].columns.size();
	for (unsigned int i = 0; i < columns; i++){
		unsigned int col = columns - i - 1;
		string colName = speadSheets[spread].columns[col].name;
		if (findSection(colName, stringSize, maxSearchPos)){
			file >> speadSheets[spread].columns[col].command.assign(32, 0);
			BOOST_LOG_(1, format("		Column %s has formula: %s cursor pos: 0x%X") % colName % speadSheets[spread].columns[col].command % file.tellg());
		}
	}

	file.seekg(findStringPos("__LayerInfoStorage"), ios_base::beg);
	for (unsigned int i = 0; i < 8; i++)
		skipLine();

	file >> size;

	const char* colTypes[] = {"X", "Y", "Z", "XErr", "YErr", "Label", "None"};
	POS = file.tellg();

	vector<SpreadColumn> header;

	unsigned int pos = POS + 1;
	for (unsigned int i = 0; i < columns; i++){
		if (speadSheets[spread].columns[i].sheet)//read only column info for first sheet
			break;

		pos += 18;
		file.seekg(pos, ios_base::beg);

		name.resize(12);
		file >> name;

		BOOST_LOG_(1, format("		Column %s @ 0x%X") % name % pos);
		int col_index = findColumnByName(spread, name);
		if(col_index == -1)
			continue;

		file.seekg(pos - 1, ios_base::beg);
		char c = 0;
		file >> c;

		SpreadColumn::ColumnType type;
		switch(c){
			case 3:
				type = SpreadColumn::X;
				break;
			case 0:
				type = SpreadColumn::Y;
				break;
			case 5:
				type = SpreadColumn::Z;
				break;
			case 6:
				type = SpreadColumn::XErr;
				break;
			case 2:
				type = SpreadColumn::YErr;
				break;
			case 4:
				type = SpreadColumn::Label;
				break;
			default:
				type = SpreadColumn::NONE;
				break;
		}
		speadSheets[spread].columns[col_index].type = type;
		BOOST_LOG_(1, format("			type: %s (@ 0x%X)") % colTypes[type] % (pos - 1));

		short width = 0;
		file.seekg(pos + 0x38, ios_base::beg);
		file >> width;
		width /= 0xA;
		if(width == 0)
			width = 8;
		if (width > 1000)
			break;

		speadSheets[spread].columns[col_index].width = width;
		BOOST_LOG_(1, format("			width: %d (@ 0x%X)") % width % (pos + 0x38));

		unsigned char c1, c2;
		file.seekg(pos + 0xC, ios_base::beg);
		file >> c1;
		file >> c2;

		switch(c1){
			case 0x00: // Numeric	   - Dec1000
			case 0x09: // Text&Numeric - Dec1000
			case 0x10: // Numeric	   - Scientific
			case 0x19: // Text&Numeric - Scientific
			case 0x20: // Numeric	   - Engeneering
			case 0x29: // Text&Numeric - Engeneering
			case 0x30: // Numeric	   - Dec1,000
			case 0x39: // Text&Numeric - Dec1,000
				speadSheets[spread].columns[col_index].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
				speadSheets[spread].columns[col_index].valueType = Numeric;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c1 / 0x10;
				if(c2 >= 0x80){
					speadSheets[spread].columns[col_index].significantDigits = c2 - 0x80;
					speadSheets[spread].columns[col_index].numericDisplayType = SignificantDigits;
				} else if(c2 > 0){
					speadSheets[spread].columns[col_index].decimalPlaces = c2 - 0x03;
					speadSheets[spread].columns[col_index].numericDisplayType = DecimalPlaces;
				}
				break;
			case 0x02: // Time
				speadSheets[spread].columns[col_index].valueType = Time;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2 - 0x80;
				break;
			case 0x03: // Date
				speadSheets[spread].columns[col_index].valueType = Date;
				speadSheets[spread].columns[col_index].valueTypeSpecification= c2 - 0x80;
				break;
			case 0x31: // Text
				speadSheets[spread].columns[col_index].valueType = Text;
				break;
			case 0x4: // Month
			case 0x34:
				speadSheets[spread].columns[col_index].valueType = Month;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2;
				break;
			case 0x5: // Day
			case 0x35:
				speadSheets[spread].columns[col_index].valueType = Day;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2;
				break;
			default: // Text
				speadSheets[spread].columns[col_index].valueType = Text;
				break;
		}

		file.seekg(pos + size - 17, ios_base::beg);
		unsigned int commentSize;
		file >> commentSize;

		pos = file.tellg();
		if(commentSize > 0){
			file.seekg(1, ios_base::cur);
			file >> speadSheets[spread].columns[col_index].comment.assign(size, 0);

			string comment = speadSheets[spread].columns[col_index].comment;
			string::size_type aux = comment.find_first_of("@");
			if(aux != string::npos){
				comment.resize(aux);
				speadSheets[spread].columns[col_index].comment = comment;
			}
			BOOST_LOG_(1, format("			comment: %s") % comment);

			pos += commentSize + 1;
		}
		pos += 1;
		file.seekg(pos, ios_base::beg);
		file >> size;
		pos += 0x5;
		header.push_back(speadSheets[spread].columns[col_index]);
	}

	for (unsigned int i = 0; i < header.size(); i++)
		speadSheets[spread].columns[i] = header[i];

	file.seekg(pos, ios_base::beg);

	for (unsigned int sheet = 1; sheet < speadSheets[spread].sheets; sheet++){
		for (unsigned int i = 0; i < columns; i++){
			if (speadSheets[spread].columns[i].sheet != sheet)
				continue;
			readColumnInfo(spread, i);
		}
	}

	skipObjectInfo();
	BOOST_LOG_(1, format("		Done with spreadsheet %d") % spread);
}

void Origin800Parser::readColumnInfo(int spread, int i)
{
	string colName = speadSheets[spread].columns[i].name;

	string::size_type sheetpos = colName.find_last_of("@");
	if(sheetpos != string::npos)
		colName.resize(sheetpos);

	if (colName.size() >= 11)
		colName.resize(11);
	BOOST_LOG_(1, format("		Column %s") % colName);

	unsigned int pos = findStringPos(colName);
	if (file.eof())
		return;

	file.seekg(pos - 1, ios_base::beg);
	char c = 0;
	file >> c;

	SpreadColumn::ColumnType type;
	switch(c){
		case 3:
			type = SpreadColumn::X;
			break;
		case 0:
			type = SpreadColumn::Y;
			break;
		case 5:
			type = SpreadColumn::Z;
			break;
		case 6:
			type = SpreadColumn::XErr;
			break;
		case 2:
			type = SpreadColumn::YErr;
			break;
		case 4:
			type = SpreadColumn::Label;
			break;
		default:
			type = SpreadColumn::NONE;
			break;
	}
	speadSheets[spread].columns[i].type = type;
	//BOOST_LOG_(1, format("			type: %s (@ 0x%X)") % colTypes[type] % (pos - 1));

	short width = 0;
	file.seekg(pos + 0x38, ios_base::beg);
	file >> width;
	width /= 0xA;
	if(width == 0)
		width = 8;
	if (width > 1000)
		return;

	speadSheets[spread].columns[i].width = width;
	BOOST_LOG_(1, format("			width: %d (@ 0x%X)") % width % (pos + 0x38));

	unsigned char c1, c2;
	file.seekg(pos + 0xC, ios_base::beg);
	file >> c1;
	file >> c2;

	switch(c1){
		case 0x00: // Numeric	   - Dec1000
		case 0x09: // Text&Numeric - Dec1000
		case 0x10: // Numeric	   - Scientific
		case 0x19: // Text&Numeric - Scientific
		case 0x20: // Numeric	   - Engeneering
		case 0x29: // Text&Numeric - Engeneering
		case 0x30: // Numeric	   - Dec1,000
		case 0x39: // Text&Numeric - Dec1,000
			speadSheets[spread].columns[i].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
			speadSheets[spread].columns[i].valueType = Numeric;
			speadSheets[spread].columns[i].valueTypeSpecification = c1 / 0x10;
			if(c2 >= 0x80){
				speadSheets[spread].columns[i].significantDigits = c2 - 0x80;
				speadSheets[spread].columns[i].numericDisplayType = SignificantDigits;
			} else if(c2 > 0){
				speadSheets[spread].columns[i].decimalPlaces = c2 - 0x03;
				speadSheets[spread].columns[i].numericDisplayType = DecimalPlaces;
			}
			break;
		case 0x02: // Time
			speadSheets[spread].columns[i].valueType = Time;
			speadSheets[spread].columns[i].valueTypeSpecification = c2 - 0x80;
			break;
		case 0x03: // Date
			speadSheets[spread].columns[i].valueType = Date;
			speadSheets[spread].columns[i].valueTypeSpecification= c2 - 0x80;
			break;
		case 0x31: // Text
			speadSheets[spread].columns[i].valueType = Text;
			break;
		case 0x4: // Month
		case 0x34:
			speadSheets[spread].columns[i].valueType = Month;
			speadSheets[spread].columns[i].valueTypeSpecification = c2;
			break;
		case 0x5: // Day
		case 0x35:
			speadSheets[spread].columns[i].valueType = Day;
			speadSheets[spread].columns[i].valueTypeSpecification = c2;
			break;
		default: // Text
			speadSheets[spread].columns[i].valueType = Text;
			break;
	}

	pos += 0x1D8;
	file.seekg(pos, ios_base::beg);

	unsigned int size;
	file >> size;
	if (size > 1000){
		pos += 0x4;
		file.seekg(pos, ios_base::beg);
		file >> size;
	}

	if(size > 0){
		file.seekg(pos + 0x5, ios_base::beg);

		unsigned char c;
		file >> c;
		unsigned int n = 1;
		while (n < size && c != '@'){
			file >> c;
			n++;
		}

		size = file.tellg();
		size -= pos + 0x6;

		file.seekg(pos + 0x5, ios_base::beg);
		string comment(size, 0);
		file >> comment;

		speadSheets[spread].columns[i].comment = comment;
		BOOST_LOG_(1, format("			comment: %s (@ 0x%X)") % comment % (pos + 0x5));
	}
}

void Origin800Parser::readExcelInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS += 5;

	BOOST_LOG_(1, format("			[EXCEL SECTION (@ 0x%X)]") % POS);

	// check spreadsheet name
	string name(25, 0);
	file.seekg(POS + 0x2, ios_base::beg);
	file >> name;

	int iexcel = findExcelByName(name);
	excels[iexcel].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(excels[iexcel], size);
	excels[iexcel].loose = false;
	char c = 0;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;
	int isheet = 0;
	while(LAYER < d_file_size)// multisheet loop
	{
		// LAYER section
		LAYER += 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(LAYER < d_file_size)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			string sec_name(41, 0);
			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> sec_name;

			BOOST_LOG_(1, format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46));

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			file.seekg(LAYER, ios_base::beg);
			//check if it is a formula
			int col_index = findExcelColumnByName(iexcel, isheet, sec_name);
			if(col_index!=-1)
			{
				file >> excels[iexcel].sheets[isheet].columns[col_index].command.assign(size, 0);
			}

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;

		/////////////// COLUMN Types ///////////////////////////////////////////
		BOOST_LOG_(1, format("			Excel sheet %d has %d columns") % isheet % excels[iexcel].sheets[isheet].columns.size());

		while(LAYER < d_file_size)
		{
			LAYER += 0x5;
			file.seekg(LAYER + 0x12, ios_base::beg);
			name.resize(12);
			file >> name;

			file.seekg(LAYER + 0x11, ios_base::beg);
			file >> c;

			short width=0;
			file.seekg(LAYER + 0x4A, ios_base::beg);
			file >> width;

			int col_index = findExcelColumnByName(iexcel, isheet, name);
			if(col_index != -1)
			{
				SpreadColumn::ColumnType type;
				switch(c)
				{
					case 3:
						type = SpreadColumn::X;
						break;
					case 0:
						type = SpreadColumn::Y;
						break;
					case 5:
						type = SpreadColumn::Z;
						break;
					case 6:
						type = SpreadColumn::XErr;
						break;
					case 2:
						type = SpreadColumn::YErr;
						break;
					case 4:
						type = SpreadColumn::Label;
						break;
					default:
						type = SpreadColumn::NONE;
						break;
				}
				excels[iexcel].sheets[isheet].columns[col_index].type = type;
				width/=0xA;
				if(width == 0)
					width = 8;
				excels[iexcel].sheets[isheet].columns[col_index].width = width;

				unsigned char c1,c2;
				file.seekg(LAYER + 0x1E, ios_base::beg);
				file >> c1;
				file >> c2;
				switch(c1)
				{
				case 0x00: // Numeric	   - Dec1000
				case 0x09: // Text&Numeric - Dec1000
				case 0x10: // Numeric	   - Scientific
				case 0x19: // Text&Numeric - Scientific
				case 0x20: // Numeric	   - Engeneering
				case 0x29: // Text&Numeric - Engeneering
				case 0x30: // Numeric	   - Dec1,000
				case 0x39: // Text&Numeric - Dec1,000
					excels[iexcel].sheets[isheet].columns[col_index].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c1 / 0x10;
					if(c2>=0x80)
					{
						excels[iexcel].sheets[isheet].columns[col_index].significantDigits = c2 - 0x80;
						excels[iexcel].sheets[isheet].columns[col_index].numericDisplayType = SignificantDigits;
					}
					else if(c2>0)
					{
						excels[iexcel].sheets[isheet].columns[col_index].decimalPlaces = c2 - 0x03;
						excels[iexcel].sheets[isheet].columns[col_index].numericDisplayType = DecimalPlaces;
					}
					break;
				case 0x02: // Time
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Time;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2 - 0x80;
					break;
				case 0x03: // Date
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Date;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2 - 0x80;
					break;
				case 0x31: // Text
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Text;
					break;
				case 0x4: // Month
				case 0x34:
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Month;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2;
					break;
				case 0x5: // Day
				case 0x35:
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Day;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2;
					break;
				default: // Text
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Text;
					break;
				}
				BOOST_LOG_(1, format("				COLUMN \"%s\" type = %s(%d) (@ 0x%X)") % excels[iexcel].sheets[isheet].columns[col_index].name.c_str() % type % (int)c % (LAYER + 0x11));
			}
			LAYER += 0x1E7 + 0x1;

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			LAYER += 0x5;
			if(size > 0)
			{
				if(col_index != -1)
				{
					file.seekg(LAYER, ios_base::beg);
					file >> excels[iexcel].sheets[isheet].columns[col_index].comment.assign(size, 0);
				}
				LAYER += size + 0x1;
			}

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			if(size != 0x1E7)
				break;
		}
		BOOST_LOG_(1, format("		Done with Excel %d") % iexcel);

		//POS = LAYER+0x5*0x6+0x1ED*0x12;
		LAYER += 0x5*0x5 + 0x1ED*0x12;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;

		++isheet;
	}

	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin800Parser::readMatrixInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS += 5;

	BOOST_LOG_(1, format("[Matrix SECTION (@ 0x%X)]") % POS);

	string name(25, 0);
	file.seekg(POS + 0x2, ios_base::beg);
	file >> name;

	int idx = findMatrixByName(name);
	matrixes[idx].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(matrixes[idx], size);
	BOOST_LOG_(1, format("	MATRIX %s has %d sheets") % name % matrixes[idx].sheets);

	unsigned int h;
	file.seekg(POS + 0x87, ios_base::beg);
	file >> h;
	BOOST_LOG_(1, format("		HeaderViewType: %d (@ 0x%X)") % h % (POS + 0x87));
	matrixes[idx].header = (h == 194) ? Matrix::XY : Matrix::ColumnRow;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;

	// LAYER section
	LAYER += 0x5;
	
	file.seekg(LAYER + 0x2B, ios_base::beg);
	file >> matrixes[idx].columnCount;
	BOOST_LOG_(1, format("		Columns: %d (@ 0x%X)") % matrixes[idx].columnCount % (LAYER + 0x2B));

	file.seekg(LAYER + 0x52, ios_base::beg);
	file >> matrixes[idx].rowCount;
	BOOST_LOG_(1, format("		Rows: %d (@ 0x%X)") % matrixes[idx].rowCount % (LAYER + 0x52));

	file.seekg(LAYER + 0x52 + 0x1F, ios_base::beg);
	unsigned short view;
	file >> view;
	if (view == 3)
		matrixes[idx].view = Matrix::ImageView;

	unsigned int maxSearchPos = findStringPos("__WIOTN");
	unsigned int stringSize = 47;

	const char* sectionNames[] = {"Y2", "X2", "Y1", "X1"};
	for (int i = 0; i < 4; i++){
		if (findSection(sectionNames[i], stringSize, maxSearchPos)){
			string s(32, 0);
			file >> s;
			BOOST_LOG_(1, format("		%s: %s cursor pos: 0x%X") % sectionNames[i] % s % file.tellg());
			matrixes[idx].coordinates[i] = QString(s.c_str()).replace(",", ".").toDouble();
		}
	}

	if (findSection("1", stringSize, maxSearchPos)){
		file >> matrixes[idx].command.assign(32, 0);
		BOOST_LOG_(1, format("		Formula: %s cursor pos: 0x%X") % matrixes[idx].command % file.tellg());
	}

	for (int i = 0; i < matrixes[idx].sheets; i++)
		findSection("__LayerInfoStorage", 20);

	for (int i = 0; i < 7; i++)
		skipLine();

	file.seekg(0x5, ios_base::cur);
	file >> size;
	POS = file.tellg();
	if (size){
		file.seekg(0x1, ios_base::cur);
		LAYER = file.tellg();

		unsigned short width;
		file.seekg(LAYER + 0x2B + 31, ios_base::beg);
		file >> width;

		width /= 0xA;
		if (width == 0)
			width = 8;
		matrixes[idx].width = width;

		unsigned char c1, c2;
		file.seekg(LAYER + 0x1E, ios_base::beg);
		file >> c1;
		file >> c2;
		matrixes[idx].valueTypeSpecification = c1/0x10;
		if(c2 >= 0x80){
			matrixes[idx].significantDigits = c2-0x80;
			matrixes[idx].numericDisplayType = SignificantDigits;
		} else if(c2 > 0){
			matrixes[idx].decimalPlaces = c2-0x03;
			matrixes[idx].numericDisplayType = DecimalPlaces;
		}
	}

	POS += size + 0x2;
	file.seekg(POS, ios_base::beg);
	//BOOST_LOG_(1, format("Cursor pos: 0x%X") % POS);

	file >> size;
	//BOOST_LOG_(1, format("		size: %d @ 0x%X") % size % file.tellg());
	POS += size + 0x2;

	file.seekg(size, ios_base::cur);
	//BOOST_LOG_(1, format("Cursor pos: 0x%X") % POS);

	skipObjectInfo();
}

void Origin800Parser::readGraphInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;
	POS += 5;

	BOOST_LOG_(1, format("			[Graph SECTION (@ 0x%X)]") % POS);

	string name(25, 0);
	file.seekg(POS + 0x02, ios_base::beg);
	file >> name;
	BOOST_LOG_(1, format("		GRAPH name: %s cursor pos: 0x%X") % name % file.tellg());

	graphs.push_back(Graph(name));
	file.seekg(POS, ios_base::beg);
	readWindowProperties(graphs.back(), size);

	file.seekg(POS + 0x23, ios_base::beg);
	file >> graphs.back().width;
	file >> graphs.back().height;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;

	while(LAYER < d_file_size)// multilayer loop
	{
		graphs.back().layers.push_back(GraphLayer());
		GraphLayer& layer(graphs.back().layers.back());
		// LAYER section
		LAYER += 0x05;

		file.seekg(LAYER + 0x0F, ios_base::beg);
		file >> layer.xAxis.min;
		file >> layer.xAxis.max;
		file >> layer.xAxis.step;

		file.seekg(LAYER + 0x2B, ios_base::beg);
		file >> layer.xAxis.majorTicks;

		file.seekg(LAYER + 0x37, ios_base::beg);
		file >> layer.xAxis.minorTicks;
		file >> layer.xAxis.scale;

		file.seekg(LAYER + 0x3A, ios_base::beg);
		file >> layer.yAxis.min;
		file >> layer.yAxis.max;
		file >> layer.yAxis.step;

		file.seekg(LAYER + 0x56, ios_base::beg);
		file >> layer.yAxis.majorTicks;

		file.seekg(LAYER + 0x62, ios_base::beg);
		file >> layer.yAxis.minorTicks;
		file >> layer.yAxis.scale;

		file.seekg(LAYER + 0x71, ios_base::beg);
		file.read(reinterpret_cast<char*>(&layer.clientRect), sizeof(Rect));

		unsigned char border;
		file.seekg(LAYER + 0x89, ios_base::beg);
		file >> border;
		layer.borderType = (BorderType)(border >= 0x80 ? border-0x80 : None);

		file.seekg(LAYER + 0x105, ios_base::beg);
		file >> layer.backgroundColor;

		LAYER += 0x12D + 0x1 + 40;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: axes, legend, __BC02, _202, _231, _232, __LayerInfoStorage etc
		//section name starts with 0x46 position
		while(LAYER < d_file_size)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			string sec_name(41, 0);
			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> sec_name;

			unsigned int sectionNamePos = LAYER + 0x46;
			BOOST_LOG_(1, format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46));

			Rect r;
			file.seekg(LAYER + 0x03, ios_base::beg);
			file.read(reinterpret_cast<char*>(&r), sizeof(Rect));

			unsigned char attach;
			file.seekg(LAYER + 0x28, ios_base::beg);
			file >> attach;

			unsigned char border;
			file >> border;

			Color color;
			file.seekg(LAYER + 0x33, ios_base::beg);
			file >> color;

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			unsigned int osize = size;

			unsigned char type;
			file.seekg(LAYER, ios_base::beg);
			file >> type;

			//text properties
			short rotation;
			file.seekg(LAYER + 0x02, ios_base::beg);
			file >> rotation;

			unsigned char fontSize;
			file >> fontSize;

			unsigned char tab;
			file.seekg(LAYER + 0x0A, ios_base::beg);
			file >> tab;

			//line properties
			unsigned char lineStyle = 0;
			double width = 0.0;
			LineVertex begin, end;
			unsigned int w = 0;

			file.seekg(LAYER + 0x12, ios_base::beg);
			file >> lineStyle;

			unsigned short w1;
			file >> w1;
			width = (double)w1/500.0;

			file.seekg(LAYER + 0x20, ios_base::beg);
			file >> begin.x;
			file >> end.x;

			file.seekg(LAYER + 0x40, ios_base::beg);
			file >> begin.y;
			file >> end.y;

			file.seekg(LAYER + 0x60, ios_base::beg);
			file >> begin.shapeType;

			file.seekg(LAYER + 0x64, ios_base::beg);
			file >> w;
			begin.shapeWidth = (double)w/500.0;

			file >> w;
			begin.shapeLength = (double)w/500.0;

			file.seekg(LAYER + 0x6C, ios_base::beg);
			file >> end.shapeType;

			file.seekg(LAYER + 0x70, ios_base::beg);
			file >> w;
			end.shapeWidth = (double)w/500.0;

			file >> w;
			end.shapeLength = (double)w/500.0;

			Figure figure;
			file.seekg(LAYER + 0x05, ios_base::beg);
			file >> w1;
			figure.width = (double)w1/500.0;

			file.seekg(LAYER + 0x08, ios_base::beg);
			file >> figure.style;

			file.seekg(LAYER + 0x42, ios_base::beg);
			file >> figure.fillAreaColor;
			file >> w1;
			figure.fillAreaPatternWidth = (double)w1/500.0;

			file.seekg(LAYER + 0x4A, ios_base::beg);
			file >> figure.fillAreaPatternColor;
			file >> figure.fillAreaPattern;

			unsigned char h;
			file.seekg(LAYER + 0x57, ios_base::beg);
			file >> h;
			figure.useBorderColor = (h == 0x10);

			//section_body_2_size
			LAYER += size + 0x1;

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;
			//check if it is a axis or legend

			file.seekg(1, ios_base::cur);
			if(sec_name == "XB")
			{
				string text(size, 0);
				file >> text;

				layer.xAxis.position = GraphAxis::Bottom;
				layer.xAxis.formatAxis[0].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "XT")
			{
				string text(size, 0);
				file >> text;

				layer.xAxis.position = GraphAxis::Top;
				layer.xAxis.formatAxis[1].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "YL")
			{
				string text(size, 0);
				file >> text;

				layer.yAxis.position = GraphAxis::Left;
				layer.yAxis.formatAxis[0].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "YR")
			{
				string text(size, 0);
				file >> text;

				layer.yAxis.position = GraphAxis::Right;
				layer.yAxis.formatAxis[1].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "ZF")
			{
				string text(size, 0);
				file >> text;

				layer.zAxis.position = GraphAxis::Front;
				layer.zAxis.formatAxis[0].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "ZB")
			{
				string text(size, 0);
				file >> text;

				layer.zAxis.position = GraphAxis::Back;
				layer.zAxis.formatAxis[1].label = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "3D")
			{
				file >> layer.zAxis.min;
				file >> layer.zAxis.max;
				file >> layer.zAxis.step;

				file.seekg(LAYER + 0x1C, ios_base::beg);
				file >> layer.zAxis.majorTicks;

				file.seekg(LAYER + 0x28, ios_base::beg);
				file >> layer.zAxis.minorTicks;
				file >> layer.zAxis.scale;

				file.seekg(LAYER + 0x218, ios_base::beg);
				file >> layer.xLength;
				file >> layer.yLength;
				file >> layer.zLength;

				layer.xLength /= 23.0;
				layer.yLength /= 23.0;
				layer.zLength /= 23.0;
			}
			else if(sec_name == "Legend")
			{
				string text(size, 0);
				file >> text;

				layer.legend = TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach);
			}
			else if(sec_name == "__BCO2") // histogram
			{
				file.seekg(LAYER + 0x10, ios_base::beg);
				file >> layer.histogramBin;

				file.seekg(LAYER + 0x20, ios_base::beg);
				file >> layer.histogramEnd;
				file >> layer.histogramBegin;

				unsigned int p = sectionNamePos + 93;
				file.seekg(p, ios_base::beg);

				file >> layer.percentile.p1SymbolType;
				file >> layer.percentile.p99SymbolType;
				file >> layer.percentile.meanSymbolType;
				file >> layer.percentile.maxSymbolType;
				file >> layer.percentile.minSymbolType;

				file.seekg(sectionNamePos + 106, ios_base::beg);
				file >> layer.percentile.whiskersRange;
				file >> layer.percentile.boxRange;

				file.seekg(sectionNamePos + 141, ios_base::beg);
				file >> layer.percentile.whiskersCoeff;
				file >> layer.percentile.boxCoeff;

				unsigned char h;
				file >> h;
				layer.percentile.diamondBox = (h == 0x82) ? true : false;

				p += 109;
				file.seekg(p, ios_base::beg);
				file >> layer.percentile.symbolSize;
				layer.percentile.symbolSize = layer.percentile.symbolSize/2 + 1;

				p += 163;
				file.seekg(p, ios_base::beg);
				file >> layer.percentile.symbolColor;
				file >> layer.percentile.symbolFillColor;
			}
			else if(sec_name == "vline") // Image profiles vertical cursor
			{
				file.seekg(sectionNamePos, ios_base::beg);
				for (int i = 0; i < 2; i++)
					skipLine();

				file.seekg(0x20, ios_base::cur);
				file >> layer.vLine;
				BOOST_LOG_(1, format("vLine: %g") % layer.vLine);

				layer.imageProfileTool = true;
			}
			else if(sec_name == "hline") // Image profiles horizontal cursor
			{
				file.seekg(sectionNamePos, ios_base::beg);
				for (int i = 0; i < 2; i++)
					skipLine();

				file.seekg(0x40, ios_base::cur);
				file >> layer.hLine;
				BOOST_LOG_(1, format("hLine: %g @ 0x%X") % layer.hLine % file.tellg());

				layer.imageProfileTool = true;
			}
			else if(sec_name == "ZCOLORS")
			{
				layer.isXYY3D = true;
			}
			else if(sec_name == "SPECTRUM1")
			{
				layer.isXYY3D = false;
			}
			else if(osize == 0x3E) // text
			{
				string text(size, 0);
				file >> text;

				layer.texts.push_back(TextBox(text, r, color, fontSize, rotation/10, tab, (BorderType)(border >= 0x80 ? border-0x80 : None), (Attach)attach));
			}
			else if(osize == 0x5E) // rectangle & circle
			{
				switch(type)
				{
				case 0:
				case 1:
					figure.type = Figure::Rectangle;
					break;
				case 2:
				case 3:
					figure.type = Figure::Circle;
					break;
				}
				figure.clientRect = r;
				figure.attach = (Attach)attach;
				figure.color = color;

				layer.figures.push_back(figure);
			}
			else if(osize == 0x78 && type == 2) // line
			{
				layer.lines.push_back(Line());
				Line& line(layer.lines.back());
				line.color = color;
				line.clientRect = r;
				line.attach = (Attach)attach;
				line.width = width;
				line.style = lineStyle;
				line.begin = begin;
				line.end = end;
			}
			else if(osize == 0x28 && type == 4) // bitmap
			{
				unsigned long filesize = size + 14;
				layer.bitmaps.push_back(Bitmap());
				Bitmap& bitmap(layer.bitmaps.back());
				bitmap.clientRect = r;
				bitmap.attach = (Attach)attach;
				bitmap.size = filesize;
				bitmap.data = new unsigned char[filesize];
				unsigned char* data = bitmap.data;
				//add Bitmap header
				memcpy(data, "BM", 2);
				data += 2;
				memcpy(data, &filesize, 4);
				data += 4;
				unsigned int d = 0;
				memcpy(data, &d, 4);
				data += 4;
				d = 0x36;
				memcpy(data, &d, 4);
				data += 4;
				file.read(reinterpret_cast<char*>(data), size);
			}

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			//section_body_3_size
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_3
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;
		unsigned char h;
		short w;

		file.seekg(LAYER, ios_base::beg);
		file >> size;
		if(size)//check layer is not empty
		{
			while(LAYER < d_file_size){
				LAYER += 0x5;

				layer.curves.push_back(GraphCurve());
				GraphCurve& curve(layer.curves.back());
				file.seekg(LAYER + 0x4C, ios_base::beg);
				file >> curve.type;
				BOOST_LOG_(1, format("			graph %d layer %d curve %d type : %d") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % (int)curve.type);

				file.seekg(LAYER + 0x04, ios_base::beg);
				file >> w;
				pair<string, string> column = findDataByIndex(w-1);
				short nColY = w;
				if(column.first.size() > 0){
					curve.dataName = column.first;
					if(layer.is3D()){
						BOOST_LOG_(1, format("			graph %d layer %d curve %d Z : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
						curve.zColumnName = column.second;
					} else {
						BOOST_LOG_(1, format("			graph %d layer %d curve %d Y : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
						curve.yColumnName = column.second;
					}
				}

				file.seekg(LAYER + 0x23, ios_base::beg);
				file >> w;
				column = findDataByIndex(w-1);
				if(column.first.size() > 0){
					if(curve.dataName != column.first)
						BOOST_LOG_(1, format("			graph %d X and Y from different tables") % graphs.size());

					if(layer.is3D()){
						BOOST_LOG_(1, format("			graph %d layer %d curve %d Y : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
						curve.yColumnName = column.second;
					} else if (layer.isXYY3D){
						BOOST_LOG_(1, format("			graph %d layer %d curve %d X : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
						curve.xColumnName = column.second;
					} else {
						BOOST_LOG_(1, format("			graph %d layer %d curve %d X : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
						curve.xColumnName = column.second;
					}
				}

				file.seekg(LAYER + 0x4D, ios_base::beg);
				file >> w;
				column = findDataByIndex(w-1);
				if(column.first.size() > 0 && layer.is3D()){
					BOOST_LOG_(1, format("			graph %d layer %d curve %d X : %s.%s") % graphs.size() % graphs.back().layers.size() % layer.curves.size() % column.first.c_str() % column.second.c_str());
					curve.xColumnName = column.second;
					if(curve.dataName != column.first)
						BOOST_LOG_(1, format("			graph %d X and Y from different tables") % graphs.size());
				}

				if(layer.is3D() || layer.isXYY3D)
					graphs.back().is3D = true;

				file.seekg(LAYER + 0x11, ios_base::beg);
				file >> curve.lineConnect;
				file >> curve.lineStyle;

				file.seekg(1, ios_base::cur);
				file >> curve.boxWidth;

				file >> w;
				curve.lineWidth=(double)w/500.0;

				file.seekg(LAYER + 0x19, ios_base::beg);
				file >> w;
				curve.symbolSize=(double)w/500.0;

				file.seekg(LAYER + 0x1C, ios_base::beg);
				file >> h;
				curve.fillArea = (h==2);

				file.seekg(LAYER + 0x1E, ios_base::beg);
				file >> curve.fillAreaType;

				//text
				if(curve.type == GraphCurve::TextPlot){
					file.seekg(LAYER + 0x13, ios_base::beg);
					file >> curve.text.rotation;

					curve.text.rotation /= 10;
					file >> curve.text.fontSize;

					file.seekg(LAYER + 0x19, ios_base::beg);
					file >> h;
					switch(h){
						case 26:
							curve.text.justify = TextProperties::Center;
							break;
						case 2:
							curve.text.justify = TextProperties::Right;
							break;
						default:
							curve.text.justify = TextProperties::Left;
							break;
					}

					file >> h;
					curve.text.fontUnderline = (h & 0x1);
					curve.text.fontItalic = (h & 0x2);
					curve.text.fontBold = (h & 0x8);
					curve.text.whiteOut = (h & 0x20);

					char offset;
					file.seekg(LAYER + 0x37, ios_base::beg);
					file >> offset;
					curve.text.xOffset = offset * 5;
					file >> offset;
					curve.text.yOffset = offset * 5;
				}

				//vector
				if(curve.type == GraphCurve::FlowVector || curve.type == GraphCurve::Vector){
					file.seekg(LAYER + 0x56, ios_base::beg);
					file >> curve.vector.multiplier;

					file.seekg(LAYER + 0x5E, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
						curve.vector.endXColumnName = column.second;

					file.seekg(LAYER + 0x62, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
						curve.vector.endYColumnName = column.second;

					file.seekg(LAYER + 0x18, ios_base::beg);
					file >> h;

					if(h >= 0x64){
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.angleColumnName = column.second;
					} else if(h <= 0x08)
						curve.vector.constAngle = 45*h;

					file >> h;

					if(h >= 0x64 && h < 0x1F4){
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.magnitudeColumnName = column.second;
					} else
						curve.vector.constMagnitude = (int)curve.symbolSize;

					file.seekg(LAYER + 0x66, ios_base::beg);
					file >> curve.vector.arrowLenght;
					file >> curve.vector.arrowAngle;

					file >> h;
					curve.vector.arrowClosed = !(h & 0x1);

					file >> w;
					curve.vector.width=(double)w/500.0;

					file.seekg(LAYER + 0x142, ios_base::beg);
					file >> h;
					switch(h){
						case 2:
							curve.vector.position = VectorProperties::Midpoint;
							break;
						case 4:
							curve.vector.position = VectorProperties::Head;
							break;
						default:
							curve.vector.position = VectorProperties::Tail;
							break;
					}
				}

				//pie
				if (curve.type == GraphCurve::Pie){
					file.seekg(LAYER + 0x92, ios_base::beg);
					file >> h;

					curve.pie.formatPercentages = (h & 0x01);
					curve.pie.formatValues		= (h & 0x02);
					curve.pie.positionAssociate = (h & 0x08);
					curve.pie.clockwiseRotation = (h & 0x20);
					curve.pie.formatCategories	= (h & 0x80);

					file >> curve.pie.formatAutomatic;
					file >> curve.pie.distance;
					file >> curve.pie.viewAngle;

					file.seekg(LAYER + 0x98, ios_base::beg);
					file >> curve.pie.thickness;

					file.seekg(LAYER + 0x9A, ios_base::beg);
					file >> curve.pie.rotation;

					file.seekg(LAYER + 0x9E, ios_base::beg);
					file >> curve.pie.displacement;

					file.seekg(LAYER + 0xA0, ios_base::beg);
					file >> curve.pie.radius;
					file >> curve.pie.horizontalOffset;

					file.seekg(LAYER + 0xA6, ios_base::beg);
					file >> curve.pie.displacedSectionCount;
				}
				//surface
				if (layer.isXYY3D || curve.type == GraphCurve::Mesh3D){
					file.seekg(LAYER + 0x17, ios_base::beg);
					file >> curve.surface.type;
					file.seekg(LAYER + 0x1C, ios_base::beg);
					file >> h;
					if(h & 0x60 == 0x60)
						curve.surface.grids = SurfaceProperties::X;
					else if(h & 0x20)
						curve.surface.grids = SurfaceProperties::Y;
					else if(h & 0x40)
						curve.surface.grids = SurfaceProperties::None;
					else
						curve.surface.grids = SurfaceProperties::XY;

					curve.surface.sideWallEnabled = (h & 0x10);
					file >> curve.surface.frontColor;

					file.seekg(LAYER + 0x14C, ios_base::beg);
					file >> w;
					curve.surface.gridLineWidth = (double)w/500.0;
					file >> curve.surface.gridColor;

					file.seekg(LAYER + 0x13, ios_base::beg);
					file >> h;
					curve.surface.backColorEnabled = (h & 0x08);				
					file.seekg(LAYER + 0x15A, ios_base::beg);
					file >> curve.surface.backColor;
					file >> curve.surface.xSideWallColor;
					file >> curve.surface.ySideWallColor;

					curve.surface.surface.fill = (h & 0x10);
					curve.surface.surface.contour = (h & 0x40);
					file.seekg(LAYER + 0x94, ios_base::beg);
					file >> w;
					curve.surface.surface.lineWidth = (double)w/500.0;
					file >> curve.surface.surface.lineColor;

					curve.surface.topContour.fill = (h & 0x02);
					curve.surface.topContour.contour = (h & 0x04);
					file.seekg(LAYER + 0xB4, ios_base::beg);
					file >> w;
					curve.surface.topContour.lineWidth = (double)w/500.0;
					file >> curve.surface.topContour.lineColor;

					curve.surface.bottomContour.fill = (h & 0x80);
					curve.surface.bottomContour.contour = (h & 0x01);
					file.seekg(LAYER + 0xA4, ios_base::beg);
					file >> w;
					curve.surface.bottomContour.lineWidth = (double)w/500.0;
					file >> curve.surface.bottomContour.lineColor;
				}

				if (curve.type == GraphCurve::Mesh3D || curve.type == GraphCurve::Contour){
					ColorMap& colorMap = (curve.type == GraphCurve::Mesh3D ? curve.surface.colorMap : curve.colorMap);
					file.seekg(LAYER + 0x13, ios_base::beg);
					file >> h;
					colorMap.fillEnabled = (h & 0x82);

					if (curve.type == GraphCurve::Contour){
						file.seekg(102, ios_base::cur);
						file >> curve.text.fontSize;

						file.seekg(7, ios_base::cur);
						file >> h;
						curve.text.fontUnderline = (h & 0x1);
						curve.text.fontItalic = (h & 0x2);
						curve.text.fontBold = (h & 0x8);
						curve.text.whiteOut = (h & 0x20);

						file.seekg(2, ios_base::cur);
						file >> curve.text.color;
					}

					file.seekg(LAYER + 0x259 + 0x2, ios_base::beg);
					readColorMap(colorMap);
				}

				file.seekg(LAYER + 0xC2, ios_base::beg);
				file >> curve.fillAreaColor;

				file >> w;
				curve.fillAreaPatternWidth=(double)w/500.0;

				file.seekg(LAYER + 0xCA, ios_base::beg);
				file >> curve.fillAreaPatternColor;

				file >> curve.fillAreaPattern;
				file >> curve.fillAreaPatternBorderStyle;
				file >> w;
				curve.fillAreaPatternBorderWidth=(double)w/500.0;
				file >> curve.fillAreaPatternBorderColor;

				file.seekg(LAYER + 0x16A, ios_base::beg);
				file >> curve.lineColor;
				if (curve.type != GraphCurve::Contour)
					curve.text.color = curve.lineColor;

				file.seekg(LAYER + 0x17, ios_base::beg);
				file >> curve.symbolType;

				file.seekg(LAYER + 0x12E, ios_base::beg);
				file >> curve.symbolFillColor;
				file >> curve.symbolColor;
				curve.vector.color = curve.symbolColor;

				file >> h;
				curve.symbolThickness = (h == 255 ? 1 : h);
				file >> curve.pointOffset;

				file.seekg(LAYER + 0x143, ios_base::beg);
				file >> h;
				curve.connectSymbols = (h&0x8);

				//LAYER += 0x1E7 + 0x1;
				LAYER += size + 0x1;

				int newSize;
				file.seekg(LAYER, ios_base::beg);
				file >> newSize;

				LAYER += newSize + (newSize > 0 ? 0x1 : 0) + 0x5;
	
				file.seekg(LAYER, ios_base::beg);
				file >> newSize;

				if(newSize != size)
					break;
			}
		}
		//LAYER+=0x5*0x5+0x1ED*0x12;
		//LAYER+=2*0x5;

		LAYER += 0x5;
		//read axis breaks
		while(LAYER < d_file_size)
		{
			file.seekg(LAYER, ios_base::beg);
			file >> size;
			if(size == 0x2D)
			{
				LAYER += 0x5;
				file.seekg(LAYER + 2, ios_base::beg);
				file >> h;

				if(h == 2)
				{
					layer.xAxisBreak.minorTicksBefore = layer.xAxis.minorTicks;
					layer.xAxisBreak.scaleIncrementBefore = layer.xAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(layer.xAxisBreak);
				}
				else if(h == 4)
				{
					layer.yAxisBreak.minorTicksBefore = layer.yAxis.minorTicks;
					layer.yAxisBreak.scaleIncrementBefore = layer.yAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(layer.yAxisBreak);
				}
				LAYER += 0x2D + 0x1;
			}
			else
				break;
		}

		LAYER += 0x5;

		file.seekg(LAYER, ios_base::beg);
		size = readGraphAxisInfo(layer.xAxis);
		LAYER += size*0x6;

		LAYER += 0x5;

		file.seekg(LAYER, ios_base::beg);
		readGraphAxisInfo(layer.yAxis);
		LAYER += size*0x6;

		LAYER += 0x5;

		file.seekg(LAYER, ios_base::beg);
		readGraphAxisInfo(layer.zAxis);
		LAYER += size*0x6;

		LAYER += 0x5;

		//LAYER += 0x2*0x5 + 0x1ED*0x6;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;
	}

	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin800Parser::skipObjectInfo()
{
	for (int i = 0; i < 3; i++)
		skipLine();

	unsigned int POS = file.tellg();
	unsigned int size;
	file >> size;
	while (POS < d_file_size && !size){
		skipLine();
		file >> size;
		POS = file.tellg();
	}
	
	unsigned int nextSize = size;
	//BOOST_LOG_(1, format("	skipObjectInfo() size: %d (0x%X) @ 0x%X") % size % size % POS);
	while (POS < d_file_size && nextSize == size){
		POS += nextSize + 0x2;
		file.seekg(POS, ios_base::beg);

		file >> nextSize;
		POS +=  0x4;
		//BOOST_LOG_(1, format("	next size: %d (0x%X) @ 0x%X") % nextSize % nextSize % POS);

		if (!nextSize){
			POS += 0x1;
			file.seekg(1, ios_base::cur);
			file >> nextSize;
			if (nextSize == size)
				POS += 0x4;
		} else if (nextSize > 1e6){
			file >> nextSize;
			if (nextSize == size)
				POS += 0x4;
		}
	}
	file.seekg(1, ios_base::cur);
	//BOOST_LOG_(1, format("	skipObjectInfo() pos:  0x%X") % file.tellg());
}

void Origin800Parser::skipLine()
{
	unsigned char c;
	file >> c;
	unsigned int POS = file.tellg();

	while(c != '\n'){
		file >> c;
		POS++;
		if (POS >= d_file_size)
			break;
	}
}

void Origin800Parser::readGraphGridInfo(GraphGrid& grid)
{
	unsigned int POS = file.tellg();
	
	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	grid.hidden = (h == 0);

	file.seekg(POS + 0x0F, ios_base::beg);
	file >> grid.color;

	file.seekg(POS + 0x12, ios_base::beg);
	file >> grid.style;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> w;
	grid.width = (double)w/500.0;
}

void Origin800Parser::readGraphAxisBreakInfo(GraphAxisBreak& axis_break)
{
	unsigned int POS = file.tellg();

	axis_break.show = true;

	file.seekg(POS + 0x0B, ios_base::beg);
	file >> axis_break.from;

	file >> axis_break.to;

	file >> axis_break.scaleIncrementAfter;
	
	file >> axis_break.position;

	unsigned char h;
	file >> h;
	axis_break.log10 = (h == 1);

	file >> axis_break.minorTicksAfter;
}

void Origin800Parser::readGraphAxisFormatInfo(GraphAxisFormat& format)
{
	unsigned int POS = file.tellg();
	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	format.hidden = (h == 0);

	file.seekg(POS + 0x0F, ios_base::beg);
	file >> format.color;

	file.seekg(POS + 0x4A, ios_base::beg);
	file >> w;
	format.majorTickLength = (double)w/10.0;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> w;
	format.thickness = (double)w/500.0;

	file.seekg(POS + 0x25, ios_base::beg);
	file >> h;

	format.minorTicksType = (h>>6);
	format.majorTicksType = ((h>>4) & 3);
	format.axisPosition = (h & 0x0F);

	switch(format.axisPosition) // need for testing
	{
	case 1:
		file.seekg(POS + 0x37, ios_base::beg);
		file >> h;
		format.axisPositionValue = (double)h;
		break;
	case 2:
		file.seekg(POS + 0x2F, ios_base::beg);
		file >> format.axisPositionValue;
		break;
	}
}

void Origin800Parser::readGraphAxisTickLabelsInfo(GraphAxisTick& tick)
{
	unsigned int POS = file.tellg();

	unsigned char h;
	unsigned char h1;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	tick.hidden = (h == 0);

	file.seekg(POS + 0x0F, ios_base::beg);
	file >> tick.color;

	file.seekg(POS + 0x13, ios_base::beg);
	file >> w;
	tick.rotation = w/10;

	file >> tick.fontSize;

	file.seekg(POS + 0x1A, ios_base::beg);
	file >> h;
	tick.fontBold = (h & 0x08);
	
	file.seekg(POS + 0x23, ios_base::beg);
	file >> w;
	file >> h;
	file >> h1;
	tick.valueType = (ValueType)(h & 0x0F);

	pair<string, string> column;
	switch(tick.valueType)
	{
	case Numeric:

		/*switch((h>>4))
		{
		case 0x9:
		tick.valueTypeSpecification=1;
		break;
		case 0xA:
		tick.valueTypeSpecification=2;
		break;
		case 0xB:
		tick.valueTypeSpecification=3;
		break;
		default:
		tick.valueTypeSpecification=0;
		}*/
		if((h>>4) > 7)
		{
			tick.valueTypeSpecification = (h>>4) - 8;
			tick.decimalPlaces = h1 - 0x40;
		}
		else
		{
			tick.valueTypeSpecification = (h>>4);
			tick.decimalPlaces = -1;
		}

		break;
	case Time:
	case Date:
	case Month:
	case Day:
	case ColumnHeading:
		tick.valueTypeSpecification = h1 - 0x40;
		break;
	case Text:
	case TickIndexedDataset:
	case Categorical:
		column = findDataByIndex(w-1);
		if(column.first.size() > 0)
		{
			tick.dataName = column.first;
			tick.columnName = column.second;
		}
		break;
	default: // Numeric Decimal 1.000
		tick.valueType = Numeric;
		tick.valueTypeSpecification = 0;
		break;
	}
}

unsigned int Origin800Parser::readGraphAxisInfo(GraphAxis& axis)
{
	unsigned int POS = file.tellg();
	unsigned int size;
	file >> size;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphGridInfo(axis.minorGrid);
	POS += size + 1;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphGridInfo(axis.majorGrid);
	POS += size + 1;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphAxisTickLabelsInfo(axis.tickAxis[0]);
	POS += size + 1;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphAxisFormatInfo(axis.formatAxis[0]);
	POS += size + 1;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphAxisTickLabelsInfo(axis.tickAxis[1]);
	POS += size + 1;

	POS += 0x5;
	file.seekg(POS, ios_base::beg);
	readGraphAxisFormatInfo(axis.formatAxis[1]);

	return (size + 1 + 0x5);
}

void Origin800Parser::readProjectTree()
{
	readProjectTreeFolder(projectTree.begin());

	BOOST_LOG_(1, "Origin project Tree");
	for(tree<ProjectNode>::iterator it = projectTree.begin(projectTree.begin()); it != projectTree.end(projectTree.begin()); ++it)
	{
		BOOST_LOG_(1, string(projectTree.depth(it) - 1, ' ') + (*it).name);
	}
}

void Origin800Parser::readProjectTreeFolder(tree<ProjectNode>::iterator parent)
{
	unsigned int POS = file.tellg();

	double creationDate, modificationDate;
	POS += 5;

	file.seekg(POS + 0x10, ios_base::beg);
	file >> creationDate;
	if (creationDate >= 1e10)
		return;

	file >> modificationDate;
	if (modificationDate >= 1e10)
		return;

	POS += 0x20 + 1 + 5;
	unsigned int size;
	file.seekg(POS, ios_base::beg);
	file >> size;

	POS += 5;

	// read folder name
	string name(size, 0);
	file.seekg(POS, ios_base::beg);
	file >> name;

	tree<ProjectNode>::iterator current_folder = projectTree.append_child(parent, ProjectNode(name, ProjectNode::Folder, doubleToPosixTime(creationDate), doubleToPosixTime(modificationDate)));
	POS += size + 1 + 5 + 5;

	unsigned int objectcount;
	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	POS += 5 + 5;

	for (unsigned int i = 0; i < objectcount; ++i){
		POS += 5;
		char c;
		file.seekg(POS + 0x2, ios_base::beg);
		file >> c;

		unsigned int objectID;
		file.seekg(POS + 0x4, ios_base::beg);
		file >> objectID;

		if(c == 0x10){
			projectTree.append_child(current_folder, ProjectNode(notes[objectID].name, ProjectNode::Note));
		} else {
			pair<ProjectNode::NodeType, string> object = findObjectByIndex(objectID);
			projectTree.append_child(current_folder, ProjectNode(object.second, object.first));
		}

		POS += 8 + 1 + 5 + 5;
	}

	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	file.seekg(1, ios_base::cur);
	for(unsigned int i = 0; i < objectcount; ++i)
		readProjectTreeFolder(current_folder);
}

void Origin800Parser::readWindowProperties(Window& window, unsigned int size)
{
	unsigned int POS = file.tellg();

	window.objectID = objectIndex;
	++objectIndex;

	file.seekg(POS + 0x1B, ios_base::beg);
	file.read(reinterpret_cast<char*>(&window.frameRect), sizeof(window.frameRect));

	char c;
	file.seekg(POS + 0x32, ios_base::beg);
	file >> c;

	if(c & 0x01)
		window.state = Window::Minimized;
	else if(c & 0x02)
		window.state = Window::Maximized;

	file.seekg(POS + 0x69, ios_base::beg);
	file >> c;

	if(c & 0x01)
		window.title = Window::Label;
	else if(c & 0x02)
		window.title = Window::Name;
	else
		window.title = Window::Both;

	window.hidden = (c & 0x08);
	if(window.hidden)
		BOOST_LOG_(1, format("			WINDOW : %s	is hidden") % window.name.c_str());
	
	double creationDate, modificationDate;
	file.seekg(POS + 0x73, ios_base::beg);
	file >> creationDate;
	if (creationDate > 1e4 && creationDate < 1e8)
		window.creationDate = doubleToPosixTime(creationDate);
	else
		return;

	file >> modificationDate;
	if (modificationDate > 1e4 && modificationDate < 1e8)
		window.modificationDate = doubleToPosixTime(modificationDate);
	else
		return;

	if(size > 0xC3)
	{
		unsigned int labellen = 0;
		file.seekg(POS + 0xC3, ios_base::beg);
		file >> c;
		while(c != '@')
		{
			file >> c;
			++labellen;
		}
		if(labellen > 0)
		{
			file.seekg(POS + 0xC3, ios_base::beg);
			file >> window.label.assign(labellen, 0);
		}

		BOOST_LOG_(1, format("			WINDOW LABEL: %s") % window.label);
	}
}

void Origin800Parser::readColorMap(ColorMap& colorMap)
{
	unsigned char h;
	short w;
	unsigned int colorMapSize;
	file >> colorMapSize;

	file.seekg(0x110, ios_base::cur);
	for(unsigned int i = 0; i < colorMapSize + 3; ++i){
		ColorMapLevel level;
		file >> level.fillPattern;

		file.seekg(0x03, ios_base::cur);
		file >> level.fillPatternColor;
		file >> w;
		level.fillPatternLineWidth = (double)w/500.0;

		file.seekg(0x06, ios_base::cur);
		file >> level.lineStyle;

		file.seekg(0x01, ios_base::cur);
		file >> w;
		level.lineWidth = (double)w/500.0;

		file >> level.lineColor;

		file.seekg(0x02, ios_base::cur);
		file >> h;
		level.labelVisible = (h & 0x1);
		level.lineVisible = !(h & 0x2);

		file.seekg(0x0D, ios_base::cur);
		file >> level.fillColor;
		file.seekg(0x04, ios_base::cur);
		double value;
		file >> value;

		colorMap.levels.push_back(make_pair(value, level));
	}
}

OriginParser* createOrigin800Parser(const string& fileName)
{
	return new Origin800Parser(fileName);
}

unsigned int Origin800Parser::findStringPos(const string& name)
{
	char c = 0;
	unsigned int startPos = file.tellg();
	unsigned int pos = startPos;
	while(pos < d_file_size){
		file >> c;

		if (c == name[0]){
			pos = file.tellg();

			file.seekg(pos - 0x3, ios_base::beg);
			file >> c;

			file.seekg(pos - 0x1, ios_base::beg);
			string s = string(name.size(), 0);
			file >> s;

			char end;
			file >> end;

			if (!c && !end && name == s){
				pos -= 0x1;
				file.seekg(startPos, ios_base::beg);
				//BOOST_LOG_(1, format("Found string: %s (@ 0x%X)") % name % pos);
				return pos;
			}
		}
		pos++;
	}
	return pos;
}

bool Origin800Parser::findSection(const string& name, int length, int maxLength)
{
	if (!maxLength)
		maxLength = d_file_size - 16;

	char c = 0;
	unsigned int pos = file.tellg();
	unsigned int startPos = pos;
	while(pos < maxLength){
		file >> c;
		if (c == name[0]){
			pos = file.tellg();
			file.seekg(pos - 0x2, ios_base::beg);
			file >> c;

			string s = string(name.size(), 0);
			file >> s;

			char end;
			file >> end;

			if (!c && !end && name == s){
				pos -= 0x1;
				file.seekg(pos + length, ios_base::beg);
				//BOOST_LOG_(1, format("Found section %s at: 0x%X") % name % pos);
				return true;
			}
		} else
			pos++;
	}
	file.seekg(startPos, ios_base::beg);
	return false;
}
