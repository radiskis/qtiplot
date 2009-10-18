/***************************************************************************
    File                 : TextDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Title/axis label options dialog

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

#ifndef TEXTDLG_H
#define TEXTDLG_H

#include <QDialog>
class QGroupBox;
class QTextEdit;
class QTextCursor;
class QComboBox;
class QCheckBox;
class QLabel;
class QSpinBox;
class Graph;
class QwtScaleWidget;
class ColorButton;
class TextFormatButtons;

//! Options dialog for text labels/axes labels
class TextDialog : public QDialog
{
	Q_OBJECT

public:
	//! Label types
	enum TextType{
		AxisTitle,   /*!< axis label */
		LayerTitle
	};

	//! Constructor
	/**
	 * \param type text type (TextMarker | AxisTitle)
	 * \param parent parent widget
	 * \param fl window flags
	 */
	TextDialog(TextType type, QWidget* parent = 0, Qt::WFlags fl = 0 );

	//! Return axis label alignment
	/**
	 * \sa setAlignment()
	 */
	int alignment();

public slots:
	//! Set the contents of the text editor box
	void setText(const QString & t);
	//! Set axis label alignment
	/**
	 * \param align alignment (can be -1 for invalid,
	 *  Qt::AlignHCenter, Qt::AlignLeft, or Qt::AlignRight)
	 */
	void setAlignment(int align);
	void setGraph(Graph *g);

private slots:
	//! Let the user select another font
	void customFont();
	//! Apply changes
	void apply();

private:
	void formatLayerLabels(Graph *g);

	//! current font
	QFont selectedFont;
	TextType d_text_type;

	ColorButton *colorBtn, *backgroundBtn;
	QPushButton *buttonFont;
	QComboBox *backgroundBox;
	QPushButton *buttonCancel;
	QPushButton *buttonApply;
	QTextEdit *textEditBox;
	QGroupBox *groupBox1, *groupBox2;
	QComboBox *alignmentBox;
	TextFormatButtons *formatButtons;
	QComboBox *formatApplyToBox;
	QSpinBox *distanceBox;
	QCheckBox *invertTitleBox;

	Graph *d_graph;
	QwtScaleWidget *d_scale;
};

#endif // TEXTDLG_H
