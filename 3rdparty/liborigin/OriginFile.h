/***************************************************************************
    File                 : OriginFile.h
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2007 Stefan Gerlach
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

#ifndef ORIGIN_FILE_H
#define ORIGIN_FILE_H

/* version 0.0 2007-09-26 */
#define LIBORIGIN_VERSION 0x00070926
#define LIBORIGIN_VERSION_STRING "2007-09-26"

#include "OriginObj.h"
#include "OriginParser.h"
#include <memory>

using namespace std;

class OriginFile
{
public:
	OriginFile(const string& fileName);

	bool parse();												//!< parse Origin file
	double version() const;										//!< get version of Origin file

	const tree<Origin::ProjectNode>* project() const;			//!< get project tree

	int spreadCount() const;									//!< get number of spreadsheets
	Origin::SpreadSheet& spread(int s) const;					//!< get spreadsheet s

	int matrixCount() const;									//!< get number of matrices
	Origin::Matrix& matrix(int m) const;						//!< get matrix m

	int functionCount() const;									//!< get number of functions
	int functionIndex(const char* s) const;						//!< get name of function s
	Origin::Function& function(int f) const;					//!< get function f

	int graphCount() const;										//!< get number of graphs
	Origin::Graph& graph(int g) const;							//!< get graph g
	
	int noteCount() const;										//!< get number of notes
	Origin::Note& note(int n) const;							//!< get note n

	string resultsLogString() const;						//!< get Results Log

private:
	int fileVersion;
	auto_ptr<OriginParser> parser;
};

#endif // ORIGIN_FILE_H
