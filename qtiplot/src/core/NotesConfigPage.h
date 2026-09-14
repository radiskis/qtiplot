/***************************************************************************
    File                 : NotesConfigPage.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 - 2011 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Notes preferences page

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
#ifndef NOTES_CONFIG_PAGE_H
#define NOTES_CONFIG_PAGE_H

#include "ConfigPage.h"
#include <QFont>

class QLabel;
class QSpinBox;
class QFontComboBox;
class QPushButton;
class QCheckBox;
class QGroupBox;
class ColorButton;

class NotesConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit NotesConfigPage(QWidget *parent = nullptr);

    QString pageTitle() const override { return tr("Notes"); }
    QIcon pageIcon() const override { return QIcon(":/notes_32.png"); }

    void init(ApplicationWindow *app, ApplicationSettings *settings) override;
    void apply(ApplicationWindow *app, ApplicationSettings *settings) override;
    void retranslateUi() override;

private slots:
    void customizeNotes();
    void rehighlight();

private:
    ApplicationWindow *d_app = nullptr;
    QLabel *labelTabLength = nullptr;
    QSpinBox *boxTabLength = nullptr;
    QLabel *labelNotesFont = nullptr;
    QFontComboBox *boxFontFamily = nullptr;
    QSpinBox *boxFontSize = nullptr;
    QPushButton *buttonBoldFont = nullptr;
    QPushButton *buttonItalicFont = nullptr;
    QCheckBox *lineNumbersBox = nullptr;

    QGroupBox *groupSyntaxHighlighter = nullptr;
    QLabel *buttonCommentLabel = nullptr;
    ColorButton *buttonCommentColor = nullptr;
    QLabel *buttonKeywordLabel = nullptr;
    ColorButton *buttonKeywordColor = nullptr;
    QLabel *buttonQuotationLabel = nullptr;
    ColorButton *buttonQuotationColor = nullptr;
    QLabel *buttonNumericLabel = nullptr;
    ColorButton *buttonNumericColor = nullptr;
    QLabel *buttonFunctionLabel = nullptr;
    ColorButton *buttonFunctionColor = nullptr;
    QLabel *buttonClassLabel = nullptr;
    ColorButton *buttonClassColor = nullptr;
};

#endif // NOTES_CONFIG_PAGE_H
