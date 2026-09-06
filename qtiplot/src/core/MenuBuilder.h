/***************************************************************************
    File                 : MenuBuilder.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Menu and toolbar layout builder

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

#ifndef MENU_BUILDER_H
#define MENU_BUILDER_H

class ApplicationWindow;

class MenuBuilder
{
public:
    static void initPlot3DToolBar(ApplicationWindow *app);
    static void initToolBars(ApplicationWindow *app);
    static void initMainMenu(ApplicationWindow *app);
    static void tableMenuAboutToShow(ApplicationWindow *app);
    static void plotDataMenuAboutToShow(ApplicationWindow *app);
    static void plotMenuAboutToShow(ApplicationWindow *app);
    static void scriptingMenuAboutToShow(ApplicationWindow *app);
    static void analysisMenuAboutToShow(ApplicationWindow *app);
    static void matrixMenuAboutToShow(ApplicationWindow *app);
    static void fileMenuAboutToShow(ApplicationWindow *app);
    static void editMenuAboutToShow(ApplicationWindow *app);
    static void windowsMenuAboutToShow(ApplicationWindow *app);
};

#endif // MENU_BUILDER_H
