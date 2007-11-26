/***************************************************************************
    File                 : CustomActionDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Custom Action dialog

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
#ifndef CustomActionDialog_H
#define CustomActionDialog_H

#include <QDialog>
#include <QXmlDefaultHandler>

class QGroupBox;
class QPushButton;
class QRadioButton;
class QComboBox;
class QListWidget;
class QLineEdit;

//! Preferences dialog
class CustomActionDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget (must be the application window!=
	 * \param fl window flags
	 */
    CustomActionDialog( QWidget* parent, Qt::WFlags fl = 0 );

private slots:
	void chooseIcon();
	void chooseFile();
	void chooseFolder();
	QAction* addAction();
	void removeAction();
	void saveAction();
	void saveAction(QAction *action);
	void setCurrentAction(int);

private:
    void loadCustomActions();
    QList<QAction *> d_actions;

    QListWidget *itemsList;
    QPushButton *buttonCancel, *buttonAdd, *buttonRemove, *buttonSave;
    QPushButton *folderBtn, *fileBtn, *iconBtn;
    QLineEdit *folderBox, *fileBox, *iconBox, *textBox, *toolTipBox, *shortcutBox;
    QRadioButton *menuBtn, *toolBarBtn;
    QComboBox *menuBox, *toolBarBox;
};

class CustomActionHandler : public QXmlDefaultHandler
{
public:
    CustomActionHandler(QAction *action);

    bool startElement(const QString &namespaceURI, const QString &localName,
                       const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                     const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

private:
    bool metFitTag;
    bool toolBar;
    QString currentText;
    QString errorStr;
    QString filePath;
    QAction *d_action;
};
#endif

