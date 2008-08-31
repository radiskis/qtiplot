/***************************************************************************
    File                 : Origin750Parser.cpp
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

#include "Origin750Parser.h"
#include <cstring>
#include <sstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace boost;

const char* colTypeNames[] = {"X", "Y", "Z", "XErr", "YErr", "Label", "None"};

void Debug(const string& s)
{
	ofstream out("opjfile.log", ios_base::app);
	out << s << endl;
	out.close();
}

inline boost::posix_time::ptime doubleToPosixTime(double jdt)
{
	return boost::posix_time::ptime(boost::gregorian::date(boost::gregorian::gregorian_calendar::from_julian_day_number(jdt+1)), boost::posix_time::seconds((jdt-(int)jdt)*86400));
}

Origin750Parser::Origin750Parser(const string& fileName)
:	file(fileName.c_str(), ios::binary)
{
	objectIndex = 0;
}

bool Origin750Parser::parse()
{
	int dataIndex = 0;

	////////////////////////////// check version from header ///////////////////////////////
	char vers[5];
	vers[4]=0;

	// get version
	file.seekg(0x7, ios_base::beg);
	file >> vers;

	Debug("HEADER :");
	stringstream out;
	unsigned char c;
	for(int i = 0; i < 0x16; ++i)
	{	// skip header + 5 Bytes ("27")
		file >> c;
		out << format("%02X ") % (unsigned int)c;
		if(!((i+1)%16))
		{
			out << endl;
		}
	}
	Debug(out.str());

	do
	{
		file >> c;
	}
	while (c != '\n');
	Debug(str(format("	[file header @ 0x%X]") % (unsigned int) file.tellg()));

	/////////////////// find column ///////////////////////////////////////////////////////////

	file.seekg(5, ios_base::cur);

	int col_found;
	file >> col_found;

	file.seekg(1, ios_base::cur);
	Debug(str(format("	[column found = %d/0x%X @ 0x%X]") % col_found % col_found % (unsigned int) file.tellg()));

	unsigned int colpos = file.tellg();

	int current_col = 1, nr = 0, nbytes = 0;
	double a;
	char name[25], valuesize;
	while(col_found > 0 && col_found < 0x84) {	// should be 0x72, 0x73 or 0x83
		//////////////////////////////// COLUMN HEADER /////////////////////////////////////////////
		short data_type;
		char data_type_u;
		unsigned int oldpos = file.tellg();

		file.seekg(oldpos+0x16, ios_base::beg);
		file >> data_type;

		file.seekg(oldpos+0x3F, ios_base::beg);
		file >> data_type_u;
		
		file.seekg(oldpos, ios_base::beg);

		Debug("COLUMN HEADER :");
		out.str(string());
		for(int i = 0;i < 0x3D; ++i)
		{	// skip 0x3C chars to value size
			file >> c;
			out << format("%02X ") % (unsigned int)c;
			if(!((i+1)%16))
			{
				out << endl;
			}
		}
		Debug(out.str());

		file >> valuesize;
		Debug(str(format("	[valuesize = %d @ 0x%X]") % (int)valuesize % ((unsigned int) file.tellg()-1)));
		if(valuesize <= 0)
		{
			Debug(str(format("	WARNING : found strange valuesize of %d") % (int)valuesize));
			valuesize=10;
		}

		Debug("SKIP :");
		out.str(string());
		for(int i = 0; i < 0x1A; ++i)
		{	// skip to name
			file >> c;
			out << format("%02X ") % (unsigned int)c;
			if(!((i+1)%16))
			{
				out << endl;
			}
		}
		Debug(out.str());

		// read name
		Debug(str(format("	[Spreadsheet @ 0x%X]") % (unsigned int) file.tellg()));

		file.read(name, 25);

		char sname[26];
		sprintf(sname,"%s",strtok(name,"_"));	// spreadsheet name
		char* cname = strtok(NULL,"_");	// column name
		while(char* tmpstr = strtok(NULL,"_")) {	// get multiple-"_" title correct
			strcat(sname,"_");
			strcat(sname,cname);
			strcpy(cname,tmpstr);
		}
		int spread=0;
		if(cname == 0)
		{
			Debug("NO COLUMN NAME FOUND! Must be a Matrix or Function.");
			////////////////////////////// READ matrixes or functions ////////////////////////////////////

			Debug(str(format("	[position @ 0x%X]") % (unsigned int) file.tellg()));
			// TODO
			short signature;
			file >> signature;
			Debug(str(format("	SIGNATURE : %02X ") % signature));

			do{	// skip until '\n'
				file >> c;
				// fprintf(debug,"%.2X ",c);
			} while (c != '\n');

			// read size
			int size;
			file >> size;

			file.seekg(1, ios_base::cur);
			// TODO : use entry size : double, float, ...
			size /= valuesize;
			Debug(str(format("	SIZE = %d") % size));

			// catch exception
			/*if(size>10000)
			size=1000;*/

			switch(signature)
			{
			case 0x50CA:
			case 0x70CA:
			case 0x50F2:
			case 0x50E2:
				Debug("NEW matrixes");
				matrixes.push_back(Matrix(sname, dataIndex));
				++dataIndex;

				Debug("VALUES :");
				out.str(string());
				switch(data_type)
				{
				case 0x6001://double
					for(int i = 0; i < size; ++i)
					{
						double value;
						//fread(&value,valuesize,1,f);
						
						file >> value;
						matrixes.back().data.push_back((double)value);
						out << format("%g ") % matrixes.back().data.back();
					}
					Debug(out.str());
					break;
				case 0x6003://float
					for(int i = 0; i < size; ++i)
					{
						float value;
						//fread(&value,valuesize,1,f);
						
						file >> value;
						matrixes.back().data.push_back((double)value);
						out << format("%g ") % matrixes.back().data.back();
					}
					Debug(out.str());
					break;
				case 0x6801://int
					if(data_type_u==8)//unsigned
					{
						for(int i = 0; i < size; ++i)
						{
							unsigned int value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					else
					{
						for(int i = 0; i < size; ++i)
						{
							int value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					break;
				case 0x6803://short
					if(data_type_u==8)//unsigned
					{
						for(int i = 0; i < size; ++i)
						{
							unsigned short value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					else
					{
						for(int i = 0; i < size; ++i)
						{
							short value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					break;
				case 0x6821://char
					if(data_type_u==8)//unsigned
					{
						for(int i = 0; i < size; ++i)
						{
							unsigned char value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					else
					{
						for(int i = 0; i < size; ++i)
						{
							char value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						Debug(out.str());
					}
					break;
				default:
					Debug(str(format("UNKNOWN matrixes DATATYPE: %02X SKIP DATA") % data_type));
					file.seekg(valuesize*size, ios_base::cur);
					matrixes.pop_back();
				}
				break;
			case 0x10C8:
				Debug("NEW functions");
				functions.push_back(Function(sname, dataIndex));
				++dataIndex;

				file.read(&(functions.back().formula.assign(valuesize, 0))[0], valuesize);
				oldpos=file.tellg();
				short t;

				file.seekg(colpos + 0xA, ios_base::beg);
				file >> t;

				if(t==0x1194)
					functions.back().type = Function::Polar;

				file.seekg(colpos+0x21, ios_base::beg);
				file >> functions.back().totalPoints;

				double d;
				file >> functions.back().begin;

				file >> d;

				functions.back().end = functions.back().begin + d*(functions.back().totalPoints-1);

				Debug(str(format("functions %s : %s") % functions.back().name.c_str() % functions.back().formula.c_str()));
				Debug(str(format(" interval %g : %g, number of points %d") % functions.back().begin % functions.back().end % functions.back().totalPoints));

				file.seekg(oldpos, ios_base::beg);
				break;
			default:
				Debug(str(format("UNKNOWN SIGNATURE: %.2X SKIP DATA") % signature));
				file.seekg(valuesize*size, ios_base::cur);

				if(valuesize != 8 && valuesize <= 16)
				{
					file.seekg(2, ios_base::cur);
				}
			}
		}
		else
		{	// worksheet
			if(speadSheets.size() == 0 || findSpreadByName(sname) == -1)
			{
				Debug("NEW speadSheets");
				current_col=1;
				speadSheets.push_back(SpreadSheet(sname));
				spread=speadSheets.size()-1;
				speadSheets.back().maxRows=0;
			}
			else
			{
				spread = findSpreadByName(sname);

				current_col=speadSheets[spread].columns.size();

				if(!current_col)
					current_col=1;
				++current_col;
			}
			Debug(str(format("speadSheets = %s COLUMN NAME = %s (%d) (@0x%X)") % sname % cname % current_col % (unsigned int)file.tellg()));
			speadSheets[spread].columns.push_back(SpreadColumn(cname, dataIndex));
			int sheetpos=speadSheets[spread].columns.back().name.find_last_of("@");
			if(!speadSheets[spread].multisheet && sheetpos!=-1)
				if(lexical_cast<int>(string(cname).substr(sheetpos+1).c_str())>1)
				{
					speadSheets[spread].multisheet=true;
					Debug(str(format("speadSheets \"%s\" IS MULTISHEET") % sname));
				}
				++dataIndex;

				////////////////////////////// SIZE of column /////////////////////////////////////////////
				do{	// skip until '\n'
					file >> c;
				} while (c != '\n');

				file >> nbytes;
				if(fmod(nbytes,(double)valuesize)>0)
				{
					Debug("WARNING: data section could not be read correct");
				}
				nr = nbytes / valuesize;
				Debug(str(format("	[number of rows = %d (%d Bytes) @ 0x%X]") % nr % nbytes % (unsigned int)file.tellg()));

				speadSheets[spread].maxRows<nr ? speadSheets[spread].maxRows=nr : 0;

				////////////////////////////////////// DATA ////////////////////////////////////////////////
				file.seekg(1, ios_base::cur);
				/*if(valuesize != 8 && valuesize <= 16 && nbytes>0) {	// skip 0 0
				fread(&c,1,1,f);
				fread(&c,1,1,f);
				}*/
				Debug(str(format("	[data @ 0x%X]") % (unsigned int)file.tellg()));
				out.str(string());
				for(int i = 0; i < nr; ++i)
				{
					if(valuesize <= 8)	// Numeric, Time, Date, Month, Day
					{
						file >> a;
						out << format("%g ") % a;
						speadSheets[spread].columns[(current_col-1)].data.push_back(/*Data(a)*/a);
					}
					else if((data_type & 0x100) == 0x100) // Text&Numeric
					{
						file >> c;
						file.seekg(1, ios_base::cur);
						if(c==0) //value
						{
							file >> a;
							out << format("%g ") % a;
							speadSheets[spread].columns[(current_col-1)].data.push_back(/*Data(a)*/a);
							file.seekg(valuesize-10, ios_base::cur);
						}
						else //text
						{
							char *stmp = new char[valuesize-1];
							file.read(stmp, valuesize-2);
							if(strchr(stmp,0x0E)) // try find non-printable symbol - garbage test
								stmp[0]='\0';
							speadSheets[spread].columns[(current_col-1)].data.push_back(/*Data(stmp)*/string(stmp));
							out << format("%s ") % stmp;
							delete stmp;
						}
					}
					else //Text
					{
						char *stmp = new char[valuesize+1];

						file.read(stmp, valuesize);
						if(strchr(stmp,0x0E)) // try find non-printable symbol - garbage test
							stmp[0]='\0';
						speadSheets[spread].columns[(current_col-1)].data.push_back(/*Data(stmp)*/string(stmp));
						out << format("%s ") % stmp;
						delete stmp;
					}
				}
				Debug(out.str());

		}	// else

		if(nbytes>0||cname==0)
		{
			file.seekg(1, ios_base::cur);
		}

		int tailsize;
		file >> tailsize;
		file.seekg(1+tailsize+(tailsize>0?1:0), ios_base::cur);

		
		file >> col_found;

		file.seekg(1, ios_base::cur);
		Debug(str(format("	[column found = %d/0x%X (@ 0x%X)]") % col_found % col_found %((unsigned int) file.tellg()-5)));
		colpos = file.tellg();
	}

	////////////////////////////////////////////////////////////////////////////
	for(unsigned int i = 0; i < speadSheets.size(); ++i)
		if(speadSheets[i].multisheet)
		{
			Debug(str(format("		CONVERT speadSheets \"%s\" to excels") % speadSheets[i].name.c_str()));
			convertSpreadToExcel(i);
			--i;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////// HEADER SECTION //////////////////////////////////////

		unsigned int POS = (unsigned int)file.tellg()-11;
		Debug("\nHEADER SECTION");
		Debug(str(format("	nr_spreads = %d") % speadSheets.size()));
		Debug(str(format("	[position @ 0x%X]") % POS));

		//////////////////////// OBJECT INFOS //////////////////////////////////////
		POS+=0xB;
		file.seekg(POS, ios_base::beg);
		while(1)
		{
			Debug("			reading	Header");
			// HEADER
			// check header
			POS=file.tellg();
			int headersize;
			file >> headersize;
			if(headersize==0)
				break;
			char object_type[10];
			char object_name[25];
			file.seekg(POS + 0x7, ios_base::beg);
			file.read(object_name, 25);

			file.seekg(POS + 0x4A, ios_base::beg);
			file.read(object_type, 10);

			file.seekg(POS, ios_base::beg);

			if(findSpreadByName(object_name) != -1)
				readSpreadInfo();
			else if(findMatrixByName(object_name) != -1)
				readMatrixInfo();
			else if(findExcelByName(object_name) != -1)
				readExcelInfo();
			else
				readGraphInfo();
		}


		file.seekg(1, ios_base::cur);
		Debug(str(format("Some Origin params @ 0x%X:") % (unsigned int)file.tellg()));

		file >> c;
		while(c!=0)
		{
			out.str(string());
			out << "		";
			while(c!='\n')
			{
				out << c;
				file >> c;
			}
			double parvalue;
			file >> parvalue;
			out << format(": %g") % parvalue;
			Debug(out.str());

			file.seekg(1, ios_base::cur);
			file >> c;
		}
		file.seekg(1+5, ios_base::cur);
		while(1)
		{
			//fseek(f,5+0x40+1,SEEK_CUR);
			int size;
			file >> size;
			if(size!=0x40)
				break;

			double creationDate, modificationDate;
			file.seekg(1+0x20, ios_base::cur);
			file >> creationDate;

			file >> modificationDate;


			unsigned char labellen;
			file.seekg(0x10-4, ios_base::cur);
			file >> labellen;

			file.seekg(4, ios_base::cur);
			file >> size;

			file.seekg(1, ios_base::cur);

			char *stmp = new char[size];
			file.read(stmp, size);

			if(0==strcmp(stmp,"ResultsLog"))
			{
				delete stmp;

				file.seekg(1, ios_base::cur);
				file >> size;

				stmp = new char[size];
				file.seekg(1, ios_base::cur);
				file.read(stmp, size);

				resultsLog=stmp;
				Debug(str(format("Results Log: %s") % resultsLog.c_str()));
				delete stmp;
				break;
			}
			else
			{
				notes.push_back(Note(stmp));
				notes.back().objectID = objectIndex;
				notes.back().creationDate = doubleToPosixTime(creationDate);
				notes.back().modificationDate = doubleToPosixTime(modificationDate);
				++objectIndex;
				delete stmp;

				file.seekg(1, ios_base::cur);
				file >> size;

				file.seekg(1, ios_base::cur);

				if(labellen>1)
				{
					file.read(&(notes.back().label.assign(labellen-1, 0))[0], labellen-1);
					file.seekg(1, ios_base::cur);
				}

				file.read(&(notes.back().text.assign(size-labellen, 0))[0], size-labellen);

				Debug(str(format("notes %d NAME: %s") % notes.size() % notes.back().name.c_str()));
				Debug(str(format("notes %d LABEL: %s") % notes.size() % notes.back().label.c_str()));
				Debug(str(format("notes %d TEXT: %s") % notes.size() % notes.back().text.c_str()));

				file.seekg(1, ios_base::cur);
			}
		}

		file.seekg(1+4*5+0x10+1, ios_base::cur);
		try
		{
			readProjectTree();
		}
		catch(...)
		{}
		Debug("Done parsing");

		return true;
}

void Origin750Parser::readSpreadInfo()
{
	unsigned int POS = file.tellg();

	int size;
	file >> size;

	POS+=5;

	Debug(str(format("			[Spreadsheet SECTION (@ 0x%X)]") % POS));

	// check spreadsheet name
	char name[25];
	file.seekg(POS + 0x2, ios_base::beg);
	file.read(name, 25);

	int spread = findSpreadByName(name);
	speadSheets[spread].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(speadSheets[spread], size);
	speadSheets[spread].loose = false;
	char c = 0;

	int LAYER = POS;
	{
		// LAYER section
		LAYER += size + 0x1 + 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header
			file.seekg(LAYER + 0x46, ios_base::beg);
			char sec_name[41];
			file.read(sec_name, 41);

			Debug(str(format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46)));

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			file.seekg(LAYER, ios_base::beg);
			//check if it is a formula
			int col_index = findSpreadColumnByName(spread, sec_name);
			if(col_index != -1)
			{
				file.read(&(speadSheets[spread].columns[col_index].command.assign(size, 0))[0], size);
			}

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

			if(0==strcmp(sec_name, "__LayerInfoStorage"))
				break;

		}
		LAYER += 0x5;
	}

	/////////////// COLUMN Types ///////////////////////////////////////////
	Debug(str(format("			Spreadsheet has %d columns") % speadSheets[spread].columns.size()));

	while(1)
	{
		LAYER += 0x5;
		file.seekg(LAYER + 0x12, ios_base::beg);
		file.read(name, 12);

		file.seekg(LAYER + 0x11, ios_base::beg);
		file >> c;

		short width=0;
		file.seekg(LAYER + 0x4A, ios_base::beg);
		file >> width;
		int col_index = findSpreadColumnByName(spread, name);
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
			speadSheets[spread].columns[col_index].type = type;
			width/=0xA;
			if(width == 0)
				width = 8;
			speadSheets[spread].columns[col_index].width = width;

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
				speadSheets[spread].columns[col_index].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c1 / 0x10;
				if(c2 >= 0x80)
				{
					speadSheets[spread].columns[col_index].significantDigits = c2 - 0x80;
					speadSheets[spread].columns[col_index].numericDisplayType = SignificantDigits;
				}
				else if(c2 > 0)
				{
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
			Debug(str(format("				COLUMN \"%s\" type = %s(%d) (@ 0x%X)") % speadSheets[spread].columns[col_index].name.c_str() % colTypeNames[type] % (int)c % (LAYER + 0x11)));
		}
		LAYER += 0x1E7 + 0x1;

		int size;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		LAYER += 0x5;
		if(size > 0)
		{
			if(col_index != -1)
			{
				file.seekg(LAYER, ios_base::beg);
				file.read(&(speadSheets[spread].columns[col_index].comment.assign(size, 0))[0], size);
			}
			LAYER += size + 0x1;
		}

		file.seekg(LAYER, ios_base::beg);
		file >> size;
		if(size != 0x1E7)
			break;
	}
	Debug(str(format("		Done with spreadsheet %d") % spread));

	file.seekg(LAYER + 0x5*0x6 + 0x1ED*0x12, ios_base::beg);
}

void Origin750Parser::readExcelInfo()
{
	unsigned int POS = file.tellg();

	int size;
	file >> size;

	POS += 5;

	Debug(str(format("			[excels SECTION (@ 0x%X)]") % POS));

	// check spreadsheet name
	char name[25];
	file.seekg(POS + 0x2, ios_base::beg);
	file.read(name, 25);

	int iexcel = findExcelByName(name);
	excels[iexcel].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(excels[iexcel], size);
	excels[iexcel].loose = false;
	char c = 0;

	int LAYER = POS;
	LAYER += size + 0x1;
	int isheet = 0;
	while(1)// multisheet loop
	{
		// LAYER section
		LAYER += 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			char sec_name[41];
			file.seekg(LAYER + 0x46, ios_base::beg);
			file.read(sec_name, 41);

			Debug(str(format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46)));

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
				file.read(&(excels[iexcel].sheets[isheet].columns[col_index].command.assign(size, 0))[0], size);
			}

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

			if(0==strcmp(sec_name, "__LayerInfoStorage"))
				break;

		}
		LAYER += 0x5;

		/////////////// COLUMN Types ///////////////////////////////////////////
		Debug(str(format("			Excel sheet %d has %d columns") % isheet % excels[iexcel].sheets[isheet].columns.size()));

		while(1)
		{
			LAYER += 0x5;
			file.seekg(LAYER + 0x12, ios_base::beg);
			file.read(name, 12);

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
				Debug(str(format("				COLUMN \"%s\" type = %s(%d) (@ 0x%X)") % excels[iexcel].sheets[isheet].columns[col_index].name.c_str() % type % (int)c % (LAYER + 0x11)));
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
					file.read(&(excels[iexcel].sheets[isheet].columns[col_index].comment.assign(size, 0))[0], size);
				}
				LAYER += size + 0x1;
			}

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			if(size != 0x1E7)
				break;
		}
		Debug(str(format("		Done with Excel %d") % iexcel));

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

void Origin750Parser::readMatrixInfo()
{
	unsigned int POS = file.tellg();

	int size;
	file >> size;

	POS+=5;

	Debug(str(format("			[Matrix SECTION (@ 0x%X)]") % POS));

	// check spreadsheet name
	char name[25];
	file.seekg(POS + 0x2, ios_base::beg);
	file.read(name, 25);

	int idx = findMatrixByName(name);
	matrixes[idx].name=name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(matrixes[idx], size);

	unsigned char h;
	file.seekg(POS + 0x87, ios_base::beg);
	file >> h;

	switch(h)
	{
	case 1:
		matrixes[idx].view = Matrix::ImageView;
		break;
	case 2:
		matrixes[idx].header = Matrix::XY;
		break;
	}

	int LAYER = POS;
	LAYER += size + 0x1;

	// LAYER section
	LAYER += 0x5;
	
	file.seekg(LAYER + 0x2B, ios_base::beg);
	file >> matrixes[idx].columnCount;

	file.seekg(LAYER + 0x52, ios_base::beg);
	file >> matrixes[idx].rowCount;

	LAYER += 0x12D + 0x1;
	//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
	//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
	//section name(column name in formula case) starts with 0x46 position
	while(1)
	{
		//section_header_size=0x6F(4 bytes) + '\n'
		LAYER += 0x5;

		//section_header
		char sec_name[41];
		file.seekg(LAYER + 0x46, ios_base::beg);
		file.read(sec_name, 41);

		//section_body_1_size
		LAYER += 0x6F+0x1;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		//section_body_1
		LAYER += 0x5;
		//check if it is a formula
		if(0==strcmp(sec_name,"MV"))
		{
			file.seekg(LAYER, ios_base::beg);
			file.read(&(matrixes[idx].command.assign(size, 0))[0], size);
		}

		//section_body_2_size
		LAYER += size+0x1;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		//section_body_2
		LAYER += 0x5;

		//close section 00 00 00 00 0A
		LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

		if(0==strcmp(sec_name, "__LayerInfoStorage"))
			break;

	}
	LAYER += 0x5;

	while(1)
	{
		LAYER+=0x5;

		unsigned short width;
		file.seekg(LAYER + 0x2B, ios_base::beg);
		file >> width;

		width = (width-55)/0xA;
		if(width == 0)
			width = 8;
		matrixes[idx].width = width;

		unsigned char c1,c2;
		file.seekg(LAYER + 0x1E, ios_base::beg);
		file >> c1;
		file >> c2;

		matrixes[idx].valueTypeSpecification = c1/0x10;
		if(c2 >= 0x80)
		{
			matrixes[idx].significantDigits = c2-0x80;
			matrixes[idx].numericDisplayType = SignificantDigits;
		}
		else if(c2 > 0)
		{
			matrixes[idx].decimalPlaces = c2-0x03;
			matrixes[idx].numericDisplayType = DecimalPlaces;
		}

		LAYER += 0x1E7+0x1;
		
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size != 0x1E7)
			break;
	}

	file.seekg(LAYER + 0x5*0x5 + 0x1ED*0x12 + 0x5, ios_base::beg);
}


void Origin750Parser::readGraphInfo()
{
	unsigned int POS = file.tellg();

	int size;
	file >> size;
	POS += 5;

	Debug(str(format("			[Graph SECTION (@ 0x%X)]") % POS));

	char name[25];
	file.seekg(POS+0x2, ios_base::beg);
	file.read(name, 25);

	graphs.push_back(Graph(name));
	file.seekg(POS, ios_base::beg);
	readWindowProperties(graphs.back(), size);

	file.seekg(POS + 0x23, ios_base::beg);
	file >> graphs.back().width;
	file >> graphs.back().height;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;

	while(1)// multilayer loop
	{
		graphs.back().layers.push_back(GraphLayer());
		// LAYER section
		LAYER += 0x5;

		file.seekg(LAYER+0xF, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.min;
		file >> graphs.back().layers.back().xAxis.max;
		file >> graphs.back().layers.back().xAxis.step;

		file.seekg(LAYER+0x2B, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.majorTicks;

		file.seekg(LAYER+0x37, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.minorTicks;
		file >> graphs.back().layers.back().xAxis.scale;

		file.seekg(LAYER+0x3A, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.min;
		file >> graphs.back().layers.back().yAxis.max;
		file >> graphs.back().layers.back().yAxis.step;

		file.seekg(LAYER+0x56, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.majorTicks;

		file.seekg(LAYER+0x62, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.minorTicks;
		file >> graphs.back().layers.back().yAxis.scale;

		file.seekg(LAYER+0x71, ios_base::beg);
		file.read(reinterpret_cast<char*>(&graphs.back().layers.back().clientRect), sizeof(Rect));

		LAYER += 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: axes, legend, __BC02, _202, _231, _232, __LayerInfoStorage etc
		//section name starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			char sec_name[41];
			file.seekg(LAYER+0x46, ios_base::beg);
			file.read(sec_name, 41);

			Debug(str(format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46)));

			Rect r;
			file.seekg(LAYER+0x3, ios_base::beg);
			file.read(reinterpret_cast<char*>(&r), sizeof(Rect));

			unsigned char attach;
			file.seekg(LAYER+0x28, ios_base::beg);
			file >> attach;

			unsigned char border;
			file >> border;

			unsigned char color;
			file.seekg(LAYER+0x33, ios_base::beg);
			file >> color;

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			int osize = size;

			unsigned char type;
			file.seekg(LAYER, ios_base::beg);
			file >> type;

			//text properties
			short rotation;
			file.seekg(LAYER + 0x2, ios_base::beg);
			file >> rotation;

			unsigned char fontSize;
			file >> fontSize;

			unsigned char tab;
			file.seekg(LAYER + 0xA, ios_base::beg);
			file >> tab;

			//line properties
			unsigned char lineStyle = 0;
			double width = 0.0;
			LineVertex begin, end;
			unsigned int w = 0;

			file.seekg(LAYER + 0x12, ios_base::beg);
			file >> lineStyle;

			file >> w;
			width = (double)w/500.0;

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

			// bitmap properties
			short bitmap_width;
			file.seekg(LAYER + 0x1, ios_base::beg);
			file >> bitmap_width;

			short bitmap_height;
			file >> bitmap_height;

			double bitmap_left = 0.0;
			file.seekg(LAYER + 0x13, ios_base::beg);
			file >> bitmap_left;

			double bitmap_top = 0.0;
			file >> bitmap_top;

			//section_body_2_size
			LAYER += size + 0x1;

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;
			//check if it is a axis or legend

			file.seekg(1, ios_base::cur);
			char stmp[255];
			if(0==strcmp(sec_name,"XB"))
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().xAxis.position = GraphAxis::Bottom;
				graphs.back().layers.back().xAxis.label = TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(0==strcmp(sec_name,"XT"))
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().xAxis.position = GraphAxis::Top;
				graphs.back().layers.back().xAxis.label = TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(0==strcmp(sec_name,"YL"))
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().yAxis.position = GraphAxis::Left;
				graphs.back().layers.back().yAxis.label = TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(0==strcmp(sec_name,"YR"))
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().yAxis.position = GraphAxis::Right;
				graphs.back().layers.back().yAxis.label = TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(0==strcmp(sec_name,"Legend"))
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().legend = TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(0==strcmp(sec_name,"__BCO2")) // histogram
			{
				file.seekg(LAYER + 0x10, ios_base::beg);
				file >> graphs.back().layers.back().histogramBin;

				file.seekg(LAYER + 0x20, ios_base::beg);
				file >> graphs.back().layers.back().histogramEnd;

				file.seekg(LAYER + 0x28, ios_base::beg);
				file >> graphs.back().layers.back().histogramBegin;
			}
			else if(osize==0x3E) // text
			{
				stmp[size]='\0';
				file.read(stmp, size);

				graphs.back().layers.back().texts.push_back(
								TextBox(stmp, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach));
			}
			else if(osize==0x78 && type==2) // line
			{
				graphs.back().layers.back().lines.push_back(Line());
				graphs.back().layers.back().lines.back().color=color;
				graphs.back().layers.back().lines.back().clientRect=r;
				graphs.back().layers.back().lines.back().attach=(Attach)attach;
				graphs.back().layers.back().lines.back().width=width;
				graphs.back().layers.back().lines.back().style=lineStyle;
				graphs.back().layers.back().lines.back().begin=begin;
				graphs.back().layers.back().lines.back().end=end;
			}
			else if(osize==0x28 && type==4) // bitmap
			{
				unsigned long filesize = size + 14;
				graphs.back().layers.back().bitmaps.push_back(Bitmap());
				graphs.back().layers.back().bitmaps.back().left=bitmap_left;
				graphs.back().layers.back().bitmaps.back().top=bitmap_top;
				graphs.back().layers.back().bitmaps.back().width=
					(graphs.back().layers.back().xAxis.max - graphs.back().layers.back().xAxis.min)*bitmap_width/10000;
				graphs.back().layers.back().bitmaps.back().height=
					(graphs.back().layers.back().yAxis.max - graphs.back().layers.back().yAxis.min)*bitmap_height/10000;
				graphs.back().layers.back().bitmaps.back().attach=(Attach)attach;
				graphs.back().layers.back().bitmaps.back().size=filesize;
				graphs.back().layers.back().bitmaps.back().data=new unsigned char[filesize];
				unsigned char *data=graphs.back().layers.back().bitmaps.back().data;
				//add Bitmap header
				memcpy(data, "BM", 2);
				data+=2;
				memcpy(data, &filesize, 4);
				data+=4;
				unsigned int d=0;
				memcpy(data, &d, 4);
				data+=4;
				d=0x36;
				memcpy(data, &d, 4);
				data+=4;
				//fread(data,sec_size,1,f);
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

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER += 0x5;
		unsigned char h;
		short w;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size==0x1E7)//check layer is not empty
		{
			while(1)
			{
				LAYER += 0x5;

				GraphCurve curve;

				file.seekg(LAYER + 0x4, ios_base::beg);
				file >> w;

				pair<string, string> column = findDataByIndex(w-1);
				short nColY = w;
				if(column.first.size() > 0)
				{
					Debug(str(format("			graphs %d layer %d curve %d Y : %s.%s") % graphs.size() % graphs.back().layers.size() % graphs.back().layers.back().curves.size() % column.first.c_str() % column.second.c_str()));
					curve.dataName = column.first;
					curve.yColumnName = column.second;
				}

				file.seekg(LAYER + 0x23, ios_base::beg);
				file >> w;

				column = findDataByIndex(w-1);
				if(column.first.size() > 0)
				{
					Debug(str(format("			graphs %d layer %d curve %d X : %s.%s") % graphs.size() % graphs.back().layers.size() % graphs.back().layers.back().curves.size() % column.first.c_str() % column.second.c_str()));
					curve.xColumnName = column.second;
					if(curve.dataName != column.first)
					{
						Debug(str(format("			graphs %d X and Y from different tables") % graphs.size()));
					}
				}

				file.seekg(LAYER + 0x4C, ios_base::beg);
				file >> curve.type;

				file.seekg(LAYER + 0x11, ios_base::beg);
				file >> curve.lineConnect;
				file >> curve.lineStyle;

				file.seekg(LAYER + 0x15, ios_base::beg);
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

				//vector
				if(curve.type == GraphCurve::FlowVector || curve.type == GraphCurve::Vector)
				{
					file.seekg(LAYER + 0x56, ios_base::beg);
					file >> curve.vector.multiplier;

					file.seekg(LAYER + 0x5E, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
					{
						curve.vector.endXColumnName = column.second;
					}

					file.seekg(LAYER + 0x62, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
					{
						curve.vector.endYColumnName = column.second;
					}

					file.seekg(LAYER + 0x18, ios_base::beg);
					file >> h;

					if(h >= 0x64)
					{
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.angleColumnName = column.second;
					}
					else if(h <= 0x08)
					{
						curve.vector.constAngle = 45*h;
					}

					file >> h;

					if(h >= 0x64 && h < 0x1F4)
					{
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.magnitudeColumnName = column.second;
					}
					else
					{
						curve.vector.constMagnitude = (int)curve.symbolSize;
					}

					file.seekg(LAYER + 0x66, ios_base::beg);
					file >> curve.vector.arrowLenght;
					file >> curve.vector.arrowAngle;

					file >> h;
					curve.vector.arrowClosed = !(h&0x1);

					file >> w;
					curve.vector.width=(double)w/500.0;

					file.seekg(LAYER + 0x142, ios_base::beg);
					file >> h;
					switch(h)
					{
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
				if(curve.type == GraphCurve::Pie)
				{
					file.seekg(LAYER + 0x92, ios_base::beg);
					file >> h;

					curve.pie.formatPercentages = (h&0x01);
					curve.pie.formatValues = (h&0x02);
					curve.pie.positionAssociate = (h&0x08);
					curve.pie.clockwiseRotation = (h&0x20);
					curve.pie.formatCategories = (h&0x80);

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

				file.seekg(LAYER + 0xC2, ios_base::beg);
				file >> curve.fillAreaColor;
				file >> curve.fillAreaFirstColor;

				file.seekg(LAYER + 0xC6, ios_base::beg);
				file >> w;
				curve.fillAreaPatternWidth=(double)w/500.0;

				file.seekg(LAYER + 0xCA, ios_base::beg);
				file >> curve.fillAreaPatternColor;

				file.seekg(LAYER + 0xCE, ios_base::beg);
				file >> curve.fillAreaPattern;
				file >> curve.fillAreaPatternBorderStyle;
				file >> w;
				curve.fillAreaPatternBorderWidth=(double)w/500.0;
				file >> curve.fillAreaPatternBorderColor;

				file.seekg(LAYER + 0x16A, ios_base::beg);
				file >> curve.lineColor;

				file.seekg(LAYER + 0x17, ios_base::beg);
				file >> curve.symbolType;

				file.seekg(LAYER + 0x12E, ios_base::beg);
				file >> curve.symbolFillColor;

				file.seekg(LAYER + 0x132, ios_base::beg);
				file >> curve.symbolColor;
				curve.vector.color = curve.symbolColor;

				file.seekg(LAYER + 0x136, ios_base::beg);
				file >> h;
				curve.symbolThickness = (h == 255 ? 1 : h);
				file >> curve.pointOffset;

				graphs.back().layers.back().curves.push_back(curve);

				LAYER += 0x1E7 + 0x1;

				int size;
				file.seekg(LAYER, ios_base::beg);
				file >> size;

				LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;
	
				file.seekg(LAYER, ios_base::beg);
				file >> size;

				if(size != 0x1E7)
					break;
			}

		}
		//LAYER+=0x5*0x5+0x1ED*0x12;
		//LAYER+=2*0x5;

		LAYER += 0x5;
		//read axis breaks
		while(1)
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
					graphs.back().layers.back().xAxisBreak.minorTicksBefore = graphs.back().layers.back().xAxis.minorTicks;
					graphs.back().layers.back().xAxisBreak.scaleIncrementBefore = graphs.back().layers.back().xAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(graphs.back().layers.back().xAxisBreak);
				}
				else if(h == 4)
				{
					graphs.back().layers.back().yAxisBreak.minorTicksBefore = graphs.back().layers.back().yAxis.minorTicks;
					graphs.back().layers.back().yAxisBreak.scaleIncrementBefore = graphs.back().layers.back().yAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(graphs.back().layers.back().yAxisBreak);
				}
				LAYER += 0x2D + 0x1;
			}
			else
				break;
		}
		LAYER += 0x5;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().xAxis.minorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().xAxis.majorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().xAxis.tickAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().xAxis.formatAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().xAxis.tickAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().xAxis.formatAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;


		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().yAxis.minorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().yAxis.majorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().yAxis.tickAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().yAxis.formatAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().yAxis.tickAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().yAxis.formatAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x2*0x5 + 0x1ED*0x6;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;
	}

	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin750Parser::skipObjectInfo()
{
	unsigned int POS = file.tellg();

	int headersize;
	file >> headersize;
	
	POS+=5;

	unsigned int LAYER = POS;
	LAYER += headersize + 0x1;
	int sec_size;
	while(1)// multilayer loop
	{
		// LAYER section
		LAYER +=0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER+=0x5;

			//section_header
			char sec_name[42];
			sec_name[41]='\0';
			file.seekg(LAYER + 0x46, ios_base::beg);
			file.read(sec_name, 41);

			//section_body_1_size
			LAYER += 0x6F+0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> sec_size;

			//section_body_1
			LAYER += 0x5;

			//section_body_2_size
			LAYER += sec_size+0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> sec_size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += sec_size+(sec_size>0?0x1:0);

			//section_body_3_size
			file.seekg(LAYER, ios_base::beg);
			file >> sec_size;

			//section_body_3
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += sec_size + (sec_size>0 ? 0x1 : 0);

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER += 0x5;

		while(1)
		{
			LAYER += 0x5;

			LAYER += 0x1E7+0x1;
			int comm_size;
			file.seekg(LAYER, ios_base::beg);
			file >> comm_size;

			LAYER += 0x5;
			if(comm_size>0)
			{
				LAYER += comm_size+0x1;
			}

			int ntmp;
			file.seekg(LAYER, ios_base::beg);
			file >> ntmp;

			if(ntmp != 0x1E7)
				break;
		}

		LAYER += 0x5*0x5+0x1ED*0x12;
		file.seekg(LAYER, ios_base::beg);
		file >> sec_size;

		if(sec_size == 0)
			break;
	}
	POS = LAYER+0x5;
	file.seekg(POS, ios_base::beg);
}

void Origin750Parser::readGraphGridInfo(GraphGrid &grid)
{
	unsigned int POS = file.tellg();
	
	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	grid.hidden = (h==0);

	file.seekg(POS + 0xF, ios_base::beg);
	file >> grid.color;

	file.seekg(POS + 0x12, ios_base::beg);
	file >> grid.style;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> w;

	grid.width = (double)w/500.0;
}

void Origin750Parser::readGraphAxisBreakInfo(GraphAxisBreak& axis_break)
{
	unsigned int POS = file.tellg();

	axis_break.show = true;

	file.seekg(POS + 0xB, ios_base::beg);
	file >> axis_break.from;

	file >> axis_break.to;

	file >> axis_break.scaleIncrementAfter;
	
	file >> axis_break.position;

	unsigned char h;
	file >> h;
	axis_break.log10 = (h==1);

	file >> axis_break.minorTicksAfter;
}

void Origin750Parser::readGraphAxisFormatInfo(GraphAxisFormat& format)
{
	unsigned int POS = file.tellg();

	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	format.hidden = (h==0);

	file.seekg(POS + 0xF, ios_base::beg);
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
	format.majorTicksType = ((h>>4)&3);
	format.axisPosition = (h&0xF);
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

void Origin750Parser::readGraphAxisTickLabelsInfo(GraphAxisTick& tick)
{
	unsigned int POS = file.tellg();

	unsigned char h;
	unsigned char h1;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	tick.hidden = (h==0);

	file.seekg(POS + 0xF, ios_base::beg);
	file >> tick.color;

	file.seekg(POS + 0x13, ios_base::beg);
	file >> w;
	tick.rotation = w/10;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> tick.fontSize;

	file.seekg(POS + 0x1A, ios_base::beg);
	file >> h;
	tick.fontBold = (h&0x8);
	
	file.seekg(POS + 0x23, ios_base::beg);
	file >> w;
	file >> h;
	file >> h1;
	tick.valueType = (ValueType)(h&0xF);

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

void Origin750Parser::readProjectTree()
{
	readProjectTreeFolder(projectTree.begin());

	Debug("Origin project Tree");
	for(tree<ProjectNode>::iterator it = projectTree.begin(projectTree.begin()); it != projectTree.end(projectTree.begin()); ++it)
	{
		Debug(string(projectTree.depth(it) - 1, ' ') + (*it).name);
	}
}

void Origin750Parser::readProjectTreeFolder(tree<ProjectNode>::iterator parent)
{
	unsigned int POS = file.tellg();

	double creationDate, modificationDate;
	POS += 5;

	file.seekg(POS + 0x10, ios_base::beg);
	file >> creationDate;

	file >> modificationDate;

	POS += 0x20 + 1 + 5;
	int size;
	file.seekg(POS, ios_base::beg);
	file >> size;

	POS += 5;

	// read folder name
	char* name = new char[size+1];
	name[size]='\0';

	file.seekg(POS, ios_base::beg);
	file.read(name, size);

	tree<ProjectNode>::iterator current_folder = projectTree.append_child(parent, ProjectNode(name, ProjectNode::Folder, doubleToPosixTime(creationDate), doubleToPosixTime(modificationDate)));
	POS += size + 1 + 5 + 5;

	int objectcount;
	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	POS += 5 + 5;

	for(int i = 0; i < objectcount; ++i)
	{
		POS += 5;
		char c;
		file.seekg(POS + 0x2, ios_base::beg);
		file >> c;

		int objectID;
		file.seekg(POS + 0x4, ios_base::beg);
		file >> objectID;

		if(c == 0x10)
			projectTree.append_child(current_folder, ProjectNode(notes[objectID].name));
		else
			projectTree.append_child(current_folder, ProjectNode(findObjectByIndex(objectID)));

		POS += 8 + 1 + 5 + 5;
	}

	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	file.seekg(1, ios_base::cur);
	for(int i = 0; i < objectcount; ++i)
		readProjectTreeFolder(current_folder);
}

void Origin750Parser::readWindowProperties(Window& window, int size)
{
	unsigned int POS = file.tellg();

	window.objectID = objectIndex;
	++objectIndex;

	file.seekg(POS + 0x1B, ios_base::beg);
	file.read(reinterpret_cast<char*>(&window.clientRect), sizeof(window.clientRect));

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
	{
		Debug(str(format("			WINDOW %d NAME : %s	is hidden") % objectIndex % window.name.c_str()));
	}
	
	double creationDate, modificationDate;
	file.seekg(POS + 0x73, ios_base::beg);
	file >> creationDate;
	file >> modificationDate;
	window.creationDate = doubleToPosixTime(creationDate);
	window.modificationDate = doubleToPosixTime(modificationDate);

	if(size > 0xC3)
	{
		int labellen = 0;
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
			file.read(&(window.label.assign(labellen, 0))[0], labellen);
		}
		else
			window.label = "";

		Debug(str(format("			WINDOW %d LABEL: %s") % objectIndex % window.label.c_str()));
	}
}

OriginParser* createOrigin750Parser(const string& fileName)
{
	return new Origin750Parser(fileName);
}