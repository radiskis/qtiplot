/***************************************************************************
File                 : globals.h
Project              : QtiPlot
--------------------------------------------------------------------
Copyright            : (C) 2006 - 2012 by Ion Vasilief
					   (C) 2006 by Tilman Hoener zu Siederdissen
Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Definition of global constants
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
//  Don't forget to change the Doxyfile when changing these!

#ifndef GLOBALS_H
#define GLOBALS_H

#if defined(_WIN32) && defined(_MSC_VER)
  #ifdef QTI_ROOT
    #define QTI_EXPORT __declspec(dllexport)
  #else
    #define QTI_EXPORT __declspec(dllimport)
  #endif
#else
  #define QTI_EXPORT
#endif
//! Major version number
const int maj_version = 0;
//! Minor version number (0..9)
const int min_version = 9;
//! Patch version number (0..9)
const int patch_version = 8;
//! Extra version information string (like "alpha", "-2", etc...)
static const char * extra_version = ".10";
static const char * svn_revision = " svn 2394";//SVN_REVISION;  //SRB: SVN_REVISION set by compiler from QTIPLOT_SVN_REVISION environment variable. (10/1/2010 )

//! Copyright string containing the author names
static const char * copyright_string = "Copyright (C) 2004-2012 Ion Vasilief";
//! Release date as a string
static const char * release_date = " 2012/05/08";

#endif
