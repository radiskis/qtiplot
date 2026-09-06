/***************************************************************************
    File                 : ConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Preferences page base class

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
#ifndef CONFIG_PAGE_H
#define CONFIG_PAGE_H

#include <QWidget>
#include <QString>
#include <QIcon>

class ApplicationWindow;
class ApplicationSettings;

class ConfigPage : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigPage(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~ConfigPage() override = default;

    virtual QString pageTitle() const = 0;
    virtual QIcon pageIcon() const = 0;

    //! Initialize UI controls from current application and settings
    virtual void init(ApplicationWindow *app, ApplicationSettings *settings) = 0;

    //! Apply changes from UI back to ApplicationSettings and ApplicationWindow
    virtual void apply(ApplicationWindow *app, ApplicationSettings *settings) = 0;

    //! Dynamic UI string retranslation on language change
    virtual void retranslateUi() = 0;
};

#endif // CONFIG_PAGE_H
