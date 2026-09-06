/***************************************************************************
    File                 : TablesConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Table preferences page

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
#ifndef TABLES_CONFIG_PAGE_H
#define TABLES_CONFIG_PAGE_H

#include "ConfigPage.h"
#include <QFont>

class QLabel;
class QComboBox;
class QGroupBox;
class ColorButton;
class QPushButton;
class QCheckBox;

class TablesConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit TablesConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("Tables"); }
    QIcon pageIcon() const override { return QIcon(":/configTable.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

    void setColumnSeparator(const QString& sep);

private slots:
    void pickTextFont();
    void pickHeaderFont();

private:
    QLabel *lblSeparator;
    QComboBox *boxSeparator;
    QGroupBox *groupBoxTableCol;
    QLabel *lblTableBackground;
    QLabel *lblTextColor;
    QLabel *lblHeaderColor;
    ColorButton *buttonBackground;
    ColorButton *buttonText;
    ColorButton *buttonHeader;
    QGroupBox *groupBoxTableFonts;
    QPushButton *buttonTextFont;
    QPushButton *buttonHeaderFont;
    QCheckBox *boxTableComments;
    QCheckBox *boxUpdateTableValues;
    QCheckBox *boxTablePasteDialog;

    QFont textFont;
    QFont headerFont;
};

#endif // TABLES_CONFIG_PAGE_H
