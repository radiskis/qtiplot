/***************************************************************************
    File                 : ConfigDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Preferences dialog

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
#include "ConfigDialog.h"
#include "ApplicationWindow.h"
#include "scripting/Note.h"
#include "plot2D/MultiLayer.h"
#include "plot2D/Graph.h"
#include "matrix/Matrix.h"
#include "ColorButton.h"
#include "ColorBox.h"
#include "pixmaps.h"
#include "DoubleSpinBox.h"

#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QFontDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStyleFactory>
#include <QRegExp>
#include <QMessageBox>
#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QFontMetrics>
#include <QFileDialog>
#include <QFontComboBox>

static const char * notes_32_xpm[] = {
"32 32 343 2",
"  	c None",
". 	c #3D3939",
"+ 	c #191414",
"@ 	c #000000",
"# 	c #191111",
"$ 	c #403838",
"% 	c #474141",
"& 	c #171111",
"* 	c #1F1414",
"= 	c #433838",
"- 	c #221B1B",
"; 	c #474747",
"> 	c #3F3F3F",
", 	c #1E1E1E",
"' 	c #1C1C1C",
") 	c #202020",
"! 	c #212121",
"~ 	c #302020",
"{ 	c #464F4F",
"] 	c #506B6B",
"^ 	c #416F6F",
"/ 	c #307070",
"( 	c #2F7373",
"_ 	c #347474",
": 	c #447070",
"< 	c #416C6C",
"[ 	c #436C6C",
"} 	c #257979",
"| 	c #1C7979",
"1 	c #165353",
"2 	c #122C2C",
"3 	c #142121",
"4 	c #303030",
"5 	c #476060",
"6 	c #609F9F",
"7 	c #66C9C9",
"8 	c #60DFDF",
"9 	c #45CCCC",
"0 	c #30AFAF",
"a 	c #45A0A0",
"b 	c #66C1C1",
"c 	c #69C1C1",
"d 	c #2DD0D0",
"e 	c #00AFAF",
"f 	c #138888",
"g 	c #305F5F",
"h 	c #2B3A3A",
"i 	c #173C3C",
"j 	c #2D6969",
"k 	c #479595",
"l 	c #62C5C5",
"m 	c #6BE5E5",
"n 	c #69F4F4",
"o 	c #58E8E8",
"p 	c #4AD5D5",
"q 	c #55CCCC",
"r 	c #65CCCC",
"s 	c #6BE1E1",
"t 	c #54C9C9",
"u 	c #6CDCDC",
"v 	c #69EDED",
"w 	c #3AD0D0",
"x 	c #14A9A9",
"y 	c #389393",
"z 	c #607A7A",
"A 	c #4B5151",
"B 	c #2F2F2F",
"C 	c #145050",
"D 	c #4BD2D2",
"E 	c #63E2E2",
"F 	c #63E4E4",
"G 	c #42B7B7",
"H 	c #308F8F",
"I 	c #629090",
"J 	c #909090",
"K 	c #6E6868",
"L 	c #404040",
"M 	c #170F0F",
"N 	c #2C6565",
"O 	c #48CFCF",
"P 	c #59ECEC",
"Q 	c #60E7E7",
"R 	c #55C8C8",
"S 	c #48A7A7",
"T 	c #48A4A4",
"U 	c #4BAAAA",
"V 	c #4AABAB",
"W 	c #48ABAB",
"X 	c #48A9A9",
"Y 	c #48A2A2",
"Z 	c #4BA6A6",
"` 	c #57C4C4",
" .	c #60E3E3",
"..	c #57EEEE",
"+.	c #48E4E4",
"@.	c #31A7A7",
"#.	c #2D7272",
"$.	c #738E8E",
"%.	c #AEABAB",
"&.	c #807A7A",
"*.	c #424242",
"=.	c #2A1A1A",
"-.	c #478282",
";.	c #66F1F1",
">.	c #306F6F",
",.	c #2D6868",
"'.	c #4BA7A7",
").	c #4BEEEE",
"!.	c #30DFDF",
"~.	c #249999",
"{.	c #809090",
"].	c #C0C0C0",
"^.	c #898686",
"/.	c #183C3C",
"(.	c #4BADAD",
"_.	c #68DBDB",
":.	c #60BBBB",
"<.	c #458C8C",
"[.	c #2A6161",
"}.	c #275B5B",
"|.	c #2A5F5F",
"1.	c #266D6D",
"2.	c #2A8585",
"3.	c #4AB7B7",
"4.	c #63DFDF",
"5.	c #48D7D7",
"6.	c #2DBABA",
"7.	c #3C8F8F",
"8.	c #5F7474",
"9.	c #9C9F9F",
"0.	c #C3C3C3",
"a.	c #888787",
"b.	c #4BD6D6",
"c.	c #66C5C5",
"d.	c #488383",
"e.	c #2D6C6C",
"f.	c #306B6B",
"g.	c #2D8C8C",
"h.	c #4BD0D0",
"i.	c #45BABA",
"j.	c #5A8888",
"k.	c #B6B0B0",
"l.	c #838383",
"m.	c #57E6E6",
"n.	c #63CFCF",
"o.	c #54AFAF",
"p.	c #46A6A6",
"q.	c #48A5A5",
"r.	c #48B7B7",
"s.	c #48CBCB",
"t.	c #4AE1E1",
"u.	c #33A9A9",
"v.	c #2C7171",
"w.	c #617D7D",
"x.	c #9E9B9B",
"y.	c #BEB8B8",
"z.	c #828282",
"A.	c #63EAEA",
"B.	c #48E7E7",
"C.	c #269B9B",
"D.	c #667676",
"E.	c #A0A0A0",
"F.	c #BEBCBC",
"G.	c #BFBFBF",
"H.	c #818181",
"I.	c #4BABAB",
"J.	c #65E7E7",
"K.	c #60E1E1",
"L.	c #62E5E5",
"M.	c #63E6E6",
"N.	c #68E6E6",
"O.	c #66E2E2",
"P.	c #48D3D3",
"Q.	c #338686",
"R.	c #4B6060",
"S.	c #747777",
"T.	c #9D9D9D",
"U.	c #BAB9B9",
"V.	c #BDBDBD",
"W.	c #808080",
"X.	c #44B9B9",
"Y.	c #4A7878",
"Z.	c #707070",
"`.	c #8A8484",
" +	c #BABABA",
".+	c #57E4E4",
"++	c #62E0E0",
"@+	c #62DFDF",
"#+	c #56E9E9",
"$+	c #31A8A8",
"%+	c #2B7070",
"&+	c #607C7C",
"*+	c #C2C2C2",
"=+	c #D5D5D5",
"-+	c #D1D1D1",
";+	c #898989",
">+	c #768686",
",+	c #D8D6D6",
"'+	c #DFDFDF",
")+	c #EBEBEB",
"!+	c #1E4141",
"~+	c #337070",
"{+	c #4DACAC",
"]+	c #66E3E3",
"^+	c #68E9E9",
"/+	c #63E0E0",
"(+	c #66E5E5",
"_+	c #46D5D5",
":+	c #2EBBBB",
"<+	c #398C8C",
"[+	c #566A6A",
"}+	c #868989",
"|+	c #B5B5B5",
"1+	c #CAC9C9",
"2+	c #D3D3D3",
"3+	c #DDDDDD",
"4+	c #888888",
"5+	c #155454",
"6+	c #43B8B8",
"7+	c #538080",
"8+	c #938D8D",
"9+	c #B2B2B2",
"0+	c #C9C9C9",
"a+	c #176363",
"b+	c #33D4D4",
"c+	c #43EBEB",
"d+	c #4BE4E4",
"e+	c #4CE5E5",
"f+	c #4AE3E3",
"g+	c #48E1E1",
"h+	c #46DFDF",
"i+	c #45DEDE",
"j+	c #45E0E0",
"k+	c #46D0D0",
"l+	c #288888",
"m+	c #1E6363",
"n+	c #567474",
"o+	c #939090",
"p+	c #9C9696",
"q+	c #9A9A9A",
"r+	c #AAAAAA",
"s+	c #B9B9B9",
"t+	c #C4C4C4",
"u+	c #BBBBBB",
"v+	c #7E7E7E",
"w+	c #186868",
"x+	c #30CFCF",
"y+	c #33DCDC",
"z+	c #30D3D3",
"A+	c #31B3B3",
"B+	c #206666",
"C+	c #204F4F",
"D+	c #A6A3A3",
"E+	c #B0B0B0",
"F+	c #104444",
"G+	c #166565",
"H+	c #146060",
"I+	c #125F5F",
"J+	c #1E6D6D",
"K+	c #2B7C7C",
"L+	c #388989",
"M+	c #419292",
"N+	c #429494",
"O+	c #3F9393",
"P+	c #4D8E8E",
"Q+	c #5C8A8A",
"R+	c #648484",
"S+	c #718686",
"T+	c #939C9C",
"U+	c #B4B4B4",
"V+	c #BDBCBC",
"W+	c #D4D4D4",
"X+	c #E3E3E3",
"Y+	c #E5E5E5",
"Z+	c #878787",
"`+	c #151515",
" @	c #4B4B4B",
".@	c #606060",
"+@	c #636363",
"@@	c #5D5D5D",
"#@	c #757575",
"$@	c #ABABAB",
"%@	c #C1C1C1",
"&@	c #CECECE",
"*@	c #F3F3F3",
"=@	c #FFFFFF",
"-@	c #FBFBFB",
";@	c #8E8E8E",
">@	c #403636",
",@	c #635959",
"'@	c #837979",
")@	c #877D7D",
"!@	c #817777",
"~@	c #8F8787",
"{@	c #9E9898",
"]@	c #AEAAAA",
"^@	c #BAB7B7",
"/@	c #B6B4B4",
"(@	c #DEDEDE",
"_@	c #E4E4E4",
":@	c #505050",
"<@	c #767676",
"[@	c #A5A5A5",
"}@	c #9E9E9E",
"|@	c #C7C7C7",
"1@	c #9F9F9F",
"2@	c #9C9C9C",
"3@	c #979797",
"4@	c #989898",
"5@	c #A9A9A9",
"6@	c #B7B7B7",
"7@	c #BCBCBC",
"8@	c #666666",
"9@	c #939393",
"0@	c #C5C5C5",
"a@	c #CACACA",
"b@	c #CBCBCB",
"c@	c #E8E8E8",
"d@	c #E0E0E0",
"e@	c #858585",
"f@	c #6F6F6F",
"g@	c #A4A4A4",
"h@	c #E6E6E6",
"i@	c #EFEFEF",
"j@	c #F0F0F0",
"k@	c #CFCFCF",
"l@	c #848484",
"m@	c #363636",
"n@	c #727272",
"o@	c #CDCDCD",
"p@	c #CCCCCC",
"q@	c #D7D7D7",
"r@	c #3A3A3A",
"s@	c #A2A2A2",
"t@	c #A1A1A1",
"u@	c #575757",
"v@	c #1F1F1F",
"w@	c #454545",
"x@	c #4C4C4C",
"y@	c #4D4D4D",
"z@	c #4A4A4A",
"A@	c #484848",
"B@	c #444444",
"                      . + @ # $ % . & @ # $ % . & @ * = - @     ",
"                    ; > , @ ' > ; > ' @ ' > ; > ' @ ) > ! @ @   ",
"                  ~ { ] ^ / ( _ : ] < / ( _ : ] [ / } | 1 2 3 ' ",
"                  4 5 6 7 8 9 0 a 6 b 8 9 0 a 6 c 8 d e f g h ) ",
"                i j k l m n o p q r s n o p t l u v w x y z A B ",
"                C 0 D 8 E 8 8 8 8 8 8 8 8 8 8 8 F 8 G H I J K L ",
"              M N O P Q R S T U V W X S Y Z `  ...+.@.#.$.%.&.*.",
"            =.4 -.8 ;.8 S >.,.>.>.>.>.>.,.>.'.8 ).!.~.g {.].^.L ",
"            /./ (.E _.:.<.[.}.[.[.[.|.[.1.2.3.4.5.6.7.8.9.0.a.L ",
"          @ C 0 b.8 c.6 d.>.e.>.>.>.f.>.g.0 h.8 i.H j.J k.].l.L ",
"          M N O m.8 n.:.o.S p.S S S q.S r.s.t.+.u.v.w.x.y.].z.L ",
"        =.4 -.8 A.8 8 8 8 8 8 8 8 8 8 8 4.8 B.!.C.g D.E.F.G.H.L ",
"        /./ I.E J.8 K. .L.M.M.M.M.M.M.M.N.O.P.6.Q.R.S.T.U.V.W.L ",
"      @ C 0 D 8 E 8 8 8 8 8 8 8 8 8 8 8 F 8 X.H Y.Z.`.E. +G.H.L ",
"      M N O .+8 ++8 8 8 8 8 8 8 8 8 @+8 #++.$+%+&+x.k.*+=+-+;+L ",
"    =.4 -.8 A.8 8 8 8 8 8 8 8 8 8 8 4.8 B.!.~.g >+].,+'+)+'+J L ",
"    !+~+{+]+^+/+/+/+/+/+/+/+/+/+/+/+(+F _+:+<+[+}+|+1+2+3+-+4+L ",
"  @ 5+0 D 8 E 8 8 8 8 8 8 8 8 8 8 8 F 8 6+H 7+W.8+E.9+G.0+G.W.L ",
"  @ a+b+c+d+e+d+d+d+f+g+h+i+i+i+j+i+k+G l+m+n+o+p+q+r+s+t+u+v+L ",
"  @ w+x+y+x+x+x+x+x+x+x+x+x+x+x+z+x+A+H B+C+8.E.D+E.E+G.0+G.W.L ",
"  @ F+a+G+H+H+H+I+H+J+K+L+M+N+M+O+M+P+Q+R+S+T+U+V+*+W+X+Y+-+Z+L ",
"    @ @ @ @ @ @ @ @ `+4  @.@+@.@@@.@#@J $@].%@].&@'+*@=@-@'+;@L ",
"                      >@,@'@)@'@!@'@~@{@]@^@/@9+%@2+(@X+_@-+Z+L ",
"                      :@<@E.[@E.E.E.E.E.E.E.}@E.E+G.%@G.|@G.W.L ",
"                      :@#@E.[@E.E.E.1@T.2@q+3@4@5@s+ +6@%@7@v+L ",
"                      :@<@E.[@E.E.E.E.E.E.E.}@E.E+G.%@G.|@G.W.L ",
"                      8@9@0@a@0.0.0.0.0.*+0.b@=+(@Y+c@Y+d@b@e@*.",
"                      f@g@'+h@'+'+'+'+'+3+'+i@=@=@=@=@=@j@k@l@L ",
"                      m@n@u+o@o@k@o@o@o@p@o@q@'+d@(@d@(@=+ +Z.B ",
"                      @ r@W.4@E.s@E.E.E.E.E.E.E.E.E.t@E.1@J u@) ",
"                      @ v@r@w@x@y@x@x@x@x@x@ @z@z@z@z@z@A@B@m@' ",
"                        @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @   "};

static const char* choose_folder_xpm[]={
    "16 16 11 1",
    "# c #000000",
    "g c #c0c0c0",
    "e c #303030",
    "a c #ffa858",
    "b c #808080",
    "d c #a0a0a4",
    "f c #585858",
    "c c #ffdca8",
    "h c #dcdcdc",
    "i c #ffffff",
    ". c None",
    "....###.........",
    "....#ab##.......",
    "....#acab####...",
    "###.#acccccca#..",
    "#ddefaaaccccca#.",
    "#bdddbaaaacccab#",
    ".eddddbbaaaacab#",
    ".#bddggdbbaaaab#",
    "..edgdggggbbaab#",
    "..#bgggghghdaab#",
    "...ebhggghicfab#",
    "....#edhhiiidab#",
    "......#egiiicfb#",
    "........#egiibb#",
    "..........#egib#",
    "............#ee#"};

ConfigDialog::ConfigDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	// get current values from app window
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	plot3DTitleFont = app->plot3DTitleFont;
	plot3DNumbersFont = app->plot3DNumbersFont;
	plot3DAxesFont = app->plot3DAxesFont;
	textFont = app->tableTextFont;
	headerFont = app->tableHeaderFont;
	appFont = app->appFont;
	axesFont = app->plotAxesFont;
	numbersFont = app->plotNumbersFont;
	legendFont = app->plotLegendFont;
	titleFont = app->plotTitleFont;

	// create the GUI
	generalDialog = new QStackedWidget();
	itemsList = new QListWidget();
	itemsList->setSpacing(10);
	itemsList->setAlternatingRowColors( true );

	initAppPage();
	initTablesPage();
	initPlotsPage();
	initPlots3DPage();
	initNotesPage();
	initFittingPage();

	generalDialog->addWidget(appTabWidget);
	generalDialog->addWidget(tables);
	generalDialog->addWidget(plotsTabWidget);
	generalDialog->addWidget(plots3D);
	generalDialog->addWidget(notesPage);
	generalDialog->addWidget(fitPage);

	QVBoxLayout * rightLayout = new QVBoxLayout();
	lblPageHeader = new QLabel();
	QFont fnt = this->font();
	fnt.setPointSize(fnt.pointSize() + 3);
	fnt.setBold(true);
	lblPageHeader->setFont(fnt);
	lblPageHeader->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QPalette pal = lblPageHeader->palette();
	pal.setColor( QPalette::Window, app->panelsColor );
	lblPageHeader->setPalette(pal);
	lblPageHeader->setAutoFillBackground( true );

	rightLayout->setSpacing(10);
	rightLayout->addWidget( lblPageHeader );
	rightLayout->addWidget( generalDialog );

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);
	topLayout->setMargin(5);
	topLayout->addWidget( itemsList );
	topLayout->addLayout( rightLayout );

	QHBoxLayout * bottomButtons = new QHBoxLayout();
	bottomButtons->addStretch();
	buttonApply = new QPushButton();
	buttonApply->setAutoDefault( true );
	bottomButtons->addWidget( buttonApply );

	buttonOk = new QPushButton();
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );
	bottomButtons->addWidget( buttonOk );

	buttonCancel = new QPushButton();
	buttonCancel->setAutoDefault( true );
	bottomButtons->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout( this );
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomButtons);

	languageChange();

	// signals and slots connections
	connect( itemsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentPage(int)));
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonTextFont, SIGNAL( clicked() ), this, SLOT( pickTextFont() ) );
	connect( buttonHeaderFont, SIGNAL( clicked() ), this, SLOT( pickHeaderFont() ) );

	setCurrentPage(0);
}

void ConfigDialog::setCurrentPage(int index)
{
	generalDialog->setCurrentIndex(index);
	if(itemsList->currentItem())
		lblPageHeader->setText(itemsList->currentItem()->text());
}

void ConfigDialog::initTablesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	tables = new QWidget();

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);

	lblSeparator = new QLabel();
	topLayout->addWidget( lblSeparator );
	boxSeparator = new QComboBox();
	boxSeparator->setEditable( true );
	topLayout->addWidget( boxSeparator );

	QString help = tr("The column separator can be customized. \nThe following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: \n0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	lblSeparator->setWhatsThis(help);
	lblSeparator->setToolTip(help);

	groupBoxTableCol = new QGroupBox();
	QGridLayout * colorsLayout = new QGridLayout(groupBoxTableCol);

	lblTableBackground = new QLabel();
	colorsLayout->addWidget( lblTableBackground, 0, 0 );
	buttonBackground= new ColorButton();
	buttonBackground->setColor(app->tableBkgdColor);
	colorsLayout->addWidget( buttonBackground, 0, 1 );

	lblTextColor = new QLabel();
	colorsLayout->addWidget( lblTextColor, 1, 0 );
	buttonText = new ColorButton();
	buttonText->setColor(app->tableTextColor);
	colorsLayout->addWidget( buttonText, 1, 1 );

	lblHeaderColor = new QLabel();
	colorsLayout->addWidget( lblHeaderColor, 2, 0 );
	buttonHeader= new ColorButton();
	buttonHeader->setColor(app->tableHeaderColor);
	colorsLayout->addWidget( buttonHeader, 2, 1 );

	groupBoxTableFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBoxTableFonts );

	buttonTextFont= new QPushButton();
	bottomLayout->addWidget( buttonTextFont );
	buttonHeaderFont= new QPushButton();
	bottomLayout->addWidget( buttonHeaderFont );

	boxTableComments = new QCheckBox();
	boxTableComments->setChecked(app->d_show_table_comments);

	boxUpdateTableValues = new QCheckBox();
	boxUpdateTableValues->setChecked(app->autoUpdateTableValues());

	QVBoxLayout * tablesPageLayout = new QVBoxLayout( tables );
	tablesPageLayout->addLayout(topLayout,1);
	tablesPageLayout->addWidget(groupBoxTableCol);
	tablesPageLayout->addWidget(groupBoxTableFonts);
    tablesPageLayout->addWidget(boxTableComments);
	tablesPageLayout->addWidget(boxUpdateTableValues);
	tablesPageLayout->addStretch();
}

void ConfigDialog::initPlotsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	plotsTabWidget = new QTabWidget();
	plotOptions = new QWidget();

	QVBoxLayout * optionsTabLayout = new QVBoxLayout( plotOptions );
	optionsTabLayout->setSpacing(5);

	QGroupBox * groupBoxOptions = new QGroupBox();
	optionsTabLayout->addWidget( groupBoxOptions );

	QGridLayout * optionsLayout = new QGridLayout( groupBoxOptions );

	boxAutoscaling = new QCheckBox();
	boxAutoscaling->setChecked(app->autoscale2DPlots);
	optionsLayout->addWidget( boxAutoscaling, 0, 0);

	boxScaleFonts = new QCheckBox();
	boxScaleFonts->setChecked(app->autoScaleFonts);
	optionsLayout->addWidget( boxScaleFonts, 0, 1);

	boxAntialiasing = new QCheckBox();
	boxAntialiasing->setChecked(app->antialiasing2DPlots);
	optionsLayout->addWidget( boxAntialiasing, 0, 2);

	boxTitle = new QCheckBox();
	boxTitle->setChecked(app->titleOn);
	optionsLayout->addWidget(boxTitle, 1, 0);

	boxAllAxes = new QCheckBox();
	boxAllAxes->setChecked (app->allAxesOn);
	optionsLayout->addWidget(boxAllAxes, 1, 1);

	boxBackbones= new QCheckBox();
	boxBackbones->setChecked(app->drawBackbones);
	optionsLayout->addWidget(boxBackbones, 1, 2);

	boxFrame = new QCheckBox();
	boxFrame->setChecked(app->canvasFrameWidth > 0);
	optionsLayout->addWidget(boxFrame, 3, 0 );

	labelFrameWidth = new QLabel();
	optionsLayout->addWidget(labelFrameWidth, 4, 0);
	boxFrameWidth= new QSpinBox();
	optionsLayout->addWidget(boxFrameWidth, 4, 1 );
	boxFrameWidth->setRange(1, 100);
	boxFrameWidth->setValue(app->canvasFrameWidth);
	if (!app->canvasFrameWidth){
		labelFrameWidth->hide();
		boxFrameWidth->hide();
	}

	lblAxesLineWidth = new QLabel();
	optionsLayout->addWidget(lblAxesLineWidth, 5, 0);
	boxLineWidth= new QSpinBox();
	boxLineWidth->setRange(0, 100);
	boxLineWidth->setValue(app->axesLineWidth);
	optionsLayout->addWidget(boxLineWidth, 5, 1);

	labelGraphAxesLabelsDist = new QLabel();
	optionsLayout->addWidget(labelGraphAxesLabelsDist, 6, 0);
	boxAxesLabelsDist = new QSpinBox();
	boxAxesLabelsDist->setRange(0, 1000);
	boxAxesLabelsDist->setValue(app->d_graph_axes_labels_dist);
	optionsLayout->addWidget(boxAxesLabelsDist, 6, 1);
	
	lblMargin = new QLabel();
	optionsLayout->addWidget(lblMargin, 7, 0);
	boxMargin = new QSpinBox();
	boxMargin->setRange(0, 1000);
	boxMargin->setSingleStep(5);
	boxMargin->setValue(app->defaultPlotMargin);
	optionsLayout->addWidget(boxMargin, 7, 1);
	optionsLayout->setRowStretch(8, 1);

	groupBackgroundOptions = new QGroupBox(tr("Background"));
	optionsTabLayout->addWidget( groupBackgroundOptions );
	QGridLayout *graphBackgroundLayout = new QGridLayout( groupBackgroundOptions );

	labelGraphBkgColor = new QLabel(tr("Background Color"));
    graphBackgroundLayout->addWidget(labelGraphBkgColor, 0, 0 );
    boxBackgroundColor = new ColorButton();
	boxBackgroundColor->setColor(app->d_graph_background_color);
    graphBackgroundLayout->addWidget(boxBackgroundColor, 0, 1 );

	labelGraphBkgOpacity = new QLabel(tr( "Opacity" ));
    graphBackgroundLayout->addWidget(labelGraphBkgOpacity, 0, 2 );
    boxBackgroundTransparency = new QSpinBox();
    boxBackgroundTransparency->setRange(0, 255);
    boxBackgroundTransparency->setSingleStep(5);
    boxBackgroundTransparency->setWrapping(true);
	boxBackgroundTransparency->setValue(app->d_graph_background_opacity);
    graphBackgroundLayout->addWidget(boxBackgroundTransparency, 0, 3 );

	labelGraphCanvasColor = new QLabel(tr("Canvas Color" ));
    graphBackgroundLayout->addWidget(labelGraphCanvasColor, 1, 0);
    boxCanvasColor = new ColorButton();
	boxCanvasColor->setColor(app->d_graph_canvas_color);
    graphBackgroundLayout->addWidget( boxCanvasColor, 1, 1 );

	labelGraphCanvasOpacity = new QLabel(tr("Opacity"));
    graphBackgroundLayout->addWidget(labelGraphCanvasOpacity, 1, 2 );
    boxCanvasTransparency = new QSpinBox();
    boxCanvasTransparency->setRange(0, 255);
    boxCanvasTransparency->setSingleStep(5);
    boxCanvasTransparency->setWrapping(true);
	boxCanvasTransparency->setValue(app->d_graph_canvas_opacity);
    graphBackgroundLayout->addWidget(boxCanvasTransparency, 1, 3 );

	labelGraphFrameColor = new QLabel(tr("Border Color"));
    graphBackgroundLayout->addWidget(labelGraphFrameColor, 2, 0);
    boxBorderColor = new ColorButton();
	boxBorderColor->setColor(app->d_graph_border_color);
    graphBackgroundLayout->addWidget(boxBorderColor, 2, 1);

	labelGraphFrameWidth = new QLabel(tr( "Width" ));
    graphBackgroundLayout->addWidget(labelGraphFrameWidth, 2, 2);
    boxBorderWidth = new QSpinBox();
	boxBorderWidth->setValue(app->d_graph_border_width);
    graphBackgroundLayout->addWidget(boxBorderWidth, 2, 3);

	graphBackgroundLayout->setRowStretch(4, 1);

	boxResize = new QCheckBox();
	boxResize->setChecked(!app->autoResizeLayers);
	optionsTabLayout->addWidget( boxResize );

    boxLabelsEditing = new QCheckBox();
    boxLabelsEditing->setChecked(!app->d_in_place_editing);
    optionsTabLayout->addWidget(boxLabelsEditing);

	plotsTabWidget->addTab( plotOptions, QString() );

	initCurvesPage();

	plotsTabWidget->addTab( curves, QString() );

	plotTicks = new QWidget();
	QVBoxLayout * plotTicksLayout = new QVBoxLayout( plotTicks );

	QGroupBox * ticksGroupBox = new QGroupBox();
	QGridLayout * ticksLayout = new QGridLayout( ticksGroupBox );
	plotTicksLayout->addWidget( ticksGroupBox );

	lblMajTicks = new QLabel();
	ticksLayout->addWidget( lblMajTicks, 0, 0 );
	boxMajTicks = new QComboBox();
	ticksLayout->addWidget( boxMajTicks, 0, 1 );

	lblMajTicksLength = new QLabel();
	ticksLayout->addWidget( lblMajTicksLength, 0, 2 );
	boxMajTicksLength = new QSpinBox();
	boxMajTicksLength->setRange(0, 100);
	boxMajTicksLength->setValue(app->majTicksLength);
	ticksLayout->addWidget( boxMajTicksLength, 0, 3 );

	lblMinTicks = new QLabel();
	ticksLayout->addWidget( lblMinTicks, 1, 0 );
	boxMinTicks = new QComboBox();
	ticksLayout->addWidget( boxMinTicks, 1, 1 );

	lblMinTicksLength = new QLabel();
	ticksLayout->addWidget( lblMinTicksLength, 1, 2 );
	boxMinTicksLength = new QSpinBox();
	boxMinTicksLength->setRange(0, 100);
	boxMinTicksLength->setValue(app->minTicksLength);
	ticksLayout->addWidget( boxMinTicksLength, 1, 3 );

	ticksLayout->setRowStretch( 4, 1 );

	plotsTabWidget->addTab( plotTicks, QString() );

	plotFonts = new QWidget();
	QVBoxLayout * plotFontsLayout = new QVBoxLayout( plotFonts );

	QGroupBox * groupBox2DFonts = new QGroupBox();
	plotFontsLayout->addWidget( groupBox2DFonts );
	QVBoxLayout * fontsLayout = new QVBoxLayout( groupBox2DFonts );
	buttonTitleFont= new QPushButton();
	fontsLayout->addWidget( buttonTitleFont );
	buttonLegendFont= new QPushButton();
	fontsLayout->addWidget( buttonLegendFont );
	buttonAxesFont= new QPushButton();
	fontsLayout->addWidget( buttonAxesFont );
	buttonNumbersFont= new QPushButton();
	fontsLayout->addWidget( buttonNumbersFont );
	fontsLayout->addStretch();

	plotsTabWidget->addTab( plotFonts, QString() );

	plotPrint = new QWidget();
	QVBoxLayout *printLayout = new QVBoxLayout( plotPrint );

	boxScaleLayersOnPrint = new QCheckBox();
	boxScaleLayersOnPrint->setChecked(app->d_scale_plots_on_print);
	printLayout->addWidget( boxScaleLayersOnPrint );

	boxPrintCropmarks = new QCheckBox();
	boxPrintCropmarks->setChecked(app->d_print_cropmarks);
	printLayout->addWidget( boxPrintCropmarks );
	printLayout->addStretch();
	plotsTabWidget->addTab(plotPrint, QString());

	connect( boxFrame, SIGNAL( toggled(bool) ), this, SLOT( showFrameWidth(bool) ) );
	connect( buttonAxesFont, SIGNAL( clicked() ), this, SLOT( pickAxesFont() ) );
	connect( buttonNumbersFont, SIGNAL( clicked() ), this, SLOT( pickNumbersFont() ) );
	connect( buttonLegendFont, SIGNAL( clicked() ), this, SLOT( pickLegendFont() ) );
	connect( buttonTitleFont, SIGNAL( clicked() ), this, SLOT( pickTitleFont() ) );
}

void ConfigDialog::showFrameWidth(bool ok)
{
	if (!ok)
	{
		boxFrameWidth->hide();
		labelFrameWidth->hide();
	}
	else
	{
		boxFrameWidth->show();
		labelFrameWidth->show();
	}
}

void ConfigDialog::initPlots3DPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	plots3D = new QWidget();

	QGroupBox * topBox = new QGroupBox();
	QGridLayout * topLayout = new QGridLayout( topBox );
	topLayout->setSpacing(5);

	lblResolution = new QLabel();
	topLayout->addWidget( lblResolution, 0, 0 );
	boxResolution = new QSpinBox();
	boxResolution->setRange(1, 100);
	boxResolution->setValue(app->plot3DResolution);
	topLayout->addWidget( boxResolution, 0, 1 );

	boxShowLegend = new QCheckBox();
	boxShowLegend->setChecked(app->showPlot3DLegend);
	topLayout->addWidget( boxShowLegend, 1, 0 );

	boxShowProjection = new QCheckBox();
	boxShowProjection->setChecked(app->showPlot3DProjection);
	topLayout->addWidget( boxShowProjection, 1, 1 );

	boxSmoothMesh = new QCheckBox();
	boxSmoothMesh->setChecked(app->smooth3DMesh);
	topLayout->addWidget( boxSmoothMesh, 2, 0 );

	boxOrthogonal = new QCheckBox();
	boxOrthogonal->setChecked(app->orthogonal3DPlots);
	topLayout->addWidget( boxOrthogonal, 2, 1 );

	boxAutoscale3DPlots = new QCheckBox();
	boxAutoscale3DPlots->setChecked(app->autoscale3DPlots);
	topLayout->addWidget( boxAutoscale3DPlots, 3, 0 );

	groupBox3DCol = new QGroupBox();
	QGridLayout * middleLayout = new QGridLayout( groupBox3DCol );

	QStringList plot3DColors = app->plot3DColors;

	btnFromColor = new ColorButton();
	btnFromColor->setColor(QColor(plot3DColors[4]));
	middleLayout->addWidget( btnFromColor, 0, 0 );
	btnLabels = new ColorButton();
	btnLabels->setColor(QColor(plot3DColors[1]));
	middleLayout->addWidget( btnLabels, 0, 1 );
	btnMesh = new ColorButton();
	btnMesh->setColor(QColor(plot3DColors[2]));
	middleLayout->addWidget( btnMesh, 0, 2 );
	btnGrid = new ColorButton();
	btnGrid->setColor(QColor(plot3DColors[3]));
	middleLayout->addWidget( btnGrid, 0, 3 );
	btnToColor = new ColorButton();
	btnToColor->setColor(QColor(plot3DColors[0]));
	middleLayout->addWidget( btnToColor, 1, 0 );
	btnNumbers = new ColorButton();
	btnNumbers->setColor(QColor(plot3DColors[5]));
	middleLayout->addWidget( btnNumbers, 1, 1 );
	btnAxes = new ColorButton();
	btnAxes->setColor(QColor(plot3DColors[6]));
	middleLayout->addWidget( btnAxes, 1, 2 );
	btnBackground3D = new ColorButton();
	btnBackground3D->setColor(QColor(plot3DColors[7]));
	middleLayout->addWidget( btnBackground3D, 1, 3 );

	groupBox3DFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBox3DFonts );
	btnTitleFnt = new QPushButton();
	bottomLayout->addWidget( btnTitleFnt );
	btnLabelsFnt = new QPushButton();
	bottomLayout->addWidget( btnLabelsFnt );
	btnNumFnt = new QPushButton();
	bottomLayout->addWidget( btnNumFnt );

	QVBoxLayout * plots3DPageLayout = new QVBoxLayout( plots3D );
	plots3DPageLayout->addWidget(topBox);
	plots3DPageLayout->addWidget(groupBox3DCol);
	plots3DPageLayout->addWidget(groupBox3DFonts);
	plots3DPageLayout->addStretch();

	connect( btnNumFnt, SIGNAL( clicked() ), this, SLOT(pick3DNumbersFont() ) );
	connect( btnTitleFnt, SIGNAL( clicked() ), this, SLOT(pick3DTitleFont() ) );
	connect( btnLabelsFnt, SIGNAL( clicked() ), this, SLOT(pick3DAxesFont() ) );
}

void ConfigDialog::initAppPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	appTabWidget = new QTabWidget(generalDialog);
	appTabWidget->setUsesScrollButtons(false);

	application = new QWidget();
	QVBoxLayout * applicationLayout = new QVBoxLayout( application );
	QGroupBox * groupBoxApp = new QGroupBox();
	applicationLayout->addWidget(groupBoxApp);
	QGridLayout * topBoxLayout = new QGridLayout( groupBoxApp );

	lblLanguage = new QLabel();
	topBoxLayout->addWidget( lblLanguage, 0, 0 );
	boxLanguage = new QComboBox();
	insertLanguagesList();
	topBoxLayout->addWidget( boxLanguage, 0, 1 );

	lblStyle = new QLabel();
	topBoxLayout->addWidget( lblStyle, 1, 0 );
	boxStyle = new QComboBox();
	topBoxLayout->addWidget( boxStyle, 1, 1 );
	QStringList styles = QStyleFactory::keys();
	styles.sort();
	boxStyle->addItems(styles);
	boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle,Qt::MatchWildcard));

	lblFonts = new QLabel();
	topBoxLayout->addWidget( lblFonts, 2, 0 );
	fontsBtn= new QPushButton();
	topBoxLayout->addWidget( fontsBtn, 2, 1 );

	lblScriptingLanguage = new QLabel();
	topBoxLayout->addWidget( lblScriptingLanguage, 3, 0 );
	boxScriptingLanguage = new QComboBox();
	QStringList llist = ScriptingLangManager::languages();
	boxScriptingLanguage->insertStringList(llist);
	boxScriptingLanguage->setCurrentItem(llist.findIndex(app->defaultScriptingLang));
	topBoxLayout->addWidget( boxScriptingLanguage, 3, 1 );

    lblUndoStackSize = new QLabel();
	topBoxLayout->addWidget( lblUndoStackSize, 4, 0 );
    undoStackSizeBox = new QSpinBox();
    undoStackSizeBox->setValue(app->matrixUndoStackSize());
    topBoxLayout->addWidget( undoStackSizeBox, 4, 1 );

	lblEndOfLine = new QLabel();
	topBoxLayout->addWidget(lblEndOfLine, 5, 0 );
	boxEndLine = new QComboBox();
	boxEndLine->addItem(tr("LF (Unix)"));
	boxEndLine->addItem(tr("CRLF (Windows)"));
	boxEndLine->addItem(tr("CR (Mac)"));
	boxEndLine->setCurrentIndex((int)app->d_eol);
	topBoxLayout->addWidget(boxEndLine, 5, 1);

	lblInitWindow = new QLabel();
	topBoxLayout->addWidget( lblInitWindow, 6, 0 );
	boxInitWindow = new QComboBox();
	topBoxLayout->addWidget( boxInitWindow, 6, 1 );

    boxSave= new QCheckBox();
	boxSave->setChecked(app->autoSave);
	topBoxLayout->addWidget( boxSave, 7, 0 );

	boxMinutes = new QSpinBox();
	boxMinutes->setRange(1, 100);
	boxMinutes->setValue(app->autoSaveTime);
	boxMinutes->setEnabled(app->autoSave);
	topBoxLayout->addWidget( boxMinutes, 7, 1 );

    boxBackupProject = new QCheckBox();
	boxBackupProject->setChecked(app->d_backup_files);
	topBoxLayout->addWidget( boxBackupProject, 8, 0, 1, 2 );

	boxSearchUpdates = new QCheckBox();
	boxSearchUpdates->setChecked(app->autoSearchUpdates);
	topBoxLayout->addWidget( boxSearchUpdates, 9, 0, 1, 2 );

#ifdef SCRIPTING_PYTHON
    completionBox = new QCheckBox();
	completionBox->setChecked(app->d_completion);
	topBoxLayout->addWidget(completionBox, 10, 0);
#endif

	topBoxLayout->setRowStretch(11, 1);

	appTabWidget->addTab(application, QString());

	initConfirmationsPage();

	appTabWidget->addTab( confirm, QString() );

	appColors = new QWidget();
	QVBoxLayout * appColorsLayout = new QVBoxLayout( appColors );
	QGroupBox * groupBoxAppCol = new QGroupBox();
	appColorsLayout->addWidget( groupBoxAppCol );
	QGridLayout * colorsBoxLayout = new QGridLayout( groupBoxAppCol );

	lblWorkspace = new QLabel();
	colorsBoxLayout->addWidget( lblWorkspace, 0, 0 );
	btnWorkspace = new ColorButton();
	btnWorkspace->setColor(app->workspaceColor);
	colorsBoxLayout->addWidget( btnWorkspace, 0, 1 );

	lblPanels = new QLabel();
	colorsBoxLayout->addWidget( lblPanels, 1, 0 );
	btnPanels = new ColorButton();
	colorsBoxLayout->addWidget( btnPanels, 1, 1 );
	btnPanels->setColor(app->panelsColor);

	lblPanelsText = new QLabel();
	colorsBoxLayout->addWidget( lblPanelsText, 2, 0 );
	btnPanelsText = new ColorButton();
	colorsBoxLayout->addWidget( btnPanelsText, 2, 1 );
	btnPanelsText->setColor(app->panelsTextColor);

	colorsBoxLayout->setRowStretch( 3, 1 );

	appTabWidget->addTab( appColors, QString() );

	numericFormatPage = new QWidget();
	QVBoxLayout *numLayout = new QVBoxLayout( numericFormatPage );
	QGroupBox *numericFormatBox = new QGroupBox();
	numLayout->addWidget( numericFormatBox );
	QGridLayout *numericFormatLayout = new QGridLayout( numericFormatBox );

	lblAppPrecision = new QLabel();
	numericFormatLayout->addWidget(lblAppPrecision, 0, 0);
	boxAppPrecision = new QSpinBox();
	boxAppPrecision->setRange(0, 14);
	boxAppPrecision->setValue(app->d_decimal_digits);
	numericFormatLayout->addWidget(boxAppPrecision, 0, 1);

    lblDecimalSeparator = new QLabel();
    numericFormatLayout->addWidget(lblDecimalSeparator, 1, 0 );
	boxDecimalSeparator = new QComboBox();
	boxDecimalSeparator->addItem(tr("System Locale Setting"));
	boxDecimalSeparator->addItem("1,000.0");
	boxDecimalSeparator->addItem("1.000,0");
	boxDecimalSeparator->addItem("1 000,0");

	numericFormatLayout->addWidget(boxDecimalSeparator, 1, 1);

    boxThousandsSeparator = new QCheckBox();
    boxThousandsSeparator->setChecked(app->locale().numberOptions() & QLocale::OmitGroupSeparator);
    numericFormatLayout->addWidget(boxThousandsSeparator, 1, 2);

	lblClipboardSeparator = new QLabel();
    numericFormatLayout->addWidget(lblClipboardSeparator, 2, 0 );
	boxClipboardLocale = new QComboBox();
	boxClipboardLocale->addItem(tr("System Locale Setting"));
	boxClipboardLocale->addItem("1,000.0");
	boxClipboardLocale->addItem("1.000,0");
	boxClipboardLocale->addItem("1 000,0");
	numericFormatLayout->addWidget(boxClipboardLocale, 2, 1);

	numericFormatLayout->setRowStretch(3, 1);
	appTabWidget->addTab( numericFormatPage, QString() );

	initFileLocationsPage();

	connect( boxLanguage, SIGNAL( activated(int) ), this, SLOT( switchToLanguage(int) ) );
	connect( fontsBtn, SIGNAL( clicked() ), this, SLOT( pickApplicationFont() ) );
	connect( boxSave, SIGNAL( toggled(bool) ), boxMinutes, SLOT( setEnabled(bool) ) );
}

void ConfigDialog::initNotesPage()
{
    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	notesPage = new QWidget();

	QGroupBox *gb1 = new QGroupBox();
	QGridLayout * gl1 = new QGridLayout(gb1);
	gl1->setSpacing(5);

	labelTabLength = new QLabel();
	gl1->addWidget(labelTabLength, 0, 0);

	boxTabLength = new QSpinBox();
	boxTabLength->setRange(0, 1000);
	boxTabLength->setSingleStep(5);
	boxTabLength->setValue(app->d_notes_tab_length);
    connect(boxTabLength, SIGNAL(valueChanged(int)), this, SLOT(customizeNotes()));
	gl1->addWidget(boxTabLength, 0, 1);

	labelNotesFont = new QLabel();
	gl1->addWidget(labelNotesFont, 1, 0);

    boxFontFamily = new QFontComboBox();
    boxFontFamily->setCurrentFont(app->d_notes_font);
    connect(boxFontFamily, SIGNAL(activated(int)), this, SLOT(customizeNotes()));
    gl1->addWidget(boxFontFamily, 1, 1);

    boxFontSize = new QSpinBox();
	boxFontSize->setRange(0, 1000);
	boxFontSize->setValue(app->d_notes_font.pointSize());
    connect(boxFontSize, SIGNAL(valueChanged(int)), this, SLOT(customizeNotes()));
	gl1->addWidget(boxFontSize, 1, 2);

    buttonBoldFont = new QPushButton(tr("&B"));
    QFont font = QFont();
    font.setBold(true);
    buttonBoldFont->setFont(font);
    buttonBoldFont->setCheckable(true);
    buttonBoldFont->setChecked(app->d_notes_font.bold());
    connect(buttonBoldFont, SIGNAL(clicked()), this, SLOT(customizeNotes()));
	gl1->addWidget(buttonBoldFont, 1, 3);

    buttonItalicFont = new QPushButton(tr("&It"));
    font = QFont();
    font.setItalic(true);
    buttonItalicFont->setFont(font);
    buttonItalicFont->setCheckable(true);
    buttonItalicFont->setChecked(app->d_notes_font.italic());
    connect(buttonItalicFont, SIGNAL(clicked()), this, SLOT(customizeNotes()));
	gl1->addWidget(buttonItalicFont, 1, 4);

    lineNumbersBox = new QCheckBox();
	lineNumbersBox->setChecked(app->d_note_line_numbers);
    connect(lineNumbersBox, SIGNAL(toggled(bool)), this, SLOT(customizeNotes()));
	gl1->addWidget(lineNumbersBox, 2, 0);
	gl1->setColumnStretch(5, 1);

	QVBoxLayout* vl = new QVBoxLayout(notesPage);
	vl->addWidget(gb1);

#ifdef SCRIPTING_PYTHON
	groupSyntaxHighlighter = new QGroupBox();
	QGridLayout *gl = new QGridLayout(groupSyntaxHighlighter);

	buttonCommentColor = new ColorButton();
	buttonCommentColor->setColor(app->d_comment_highlight_color);
	connect(buttonCommentColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonCommentColor, 0, 0);
	
	buttonKeywordColor = new ColorButton();
	buttonKeywordColor->setColor(app->d_keyword_highlight_color);
	connect(buttonKeywordColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonKeywordColor, 0, 1);
	
	buttonQuotationColor = new ColorButton();
	buttonQuotationColor->setColor(app->d_quotation_highlight_color);
	connect(buttonQuotationColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonQuotationColor, 1, 0);
	
	buttonNumericColor = new ColorButton();
	buttonNumericColor->setColor(app->d_numeric_highlight_color);
	connect(buttonNumericColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonNumericColor, 1, 1);
	
	buttonFunctionColor = new ColorButton();
	buttonFunctionColor->setColor(app->d_function_highlight_color);
	connect(buttonFunctionColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonFunctionColor, 2, 0);
	
	buttonClassColor = new ColorButton();
	buttonClassColor->setColor(app->d_class_highlight_color);
	connect(buttonClassColor, SIGNAL(colorChanged()), this, SLOT(rehighlight()));
	gl->addWidget(buttonClassColor, 2, 1);
	
	vl->addWidget(groupSyntaxHighlighter);
#endif

	vl->addStretch();
}

void ConfigDialog::initFittingPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	fitPage = new QWidget();

	groupBoxFittingCurve = new QGroupBox();
	QGridLayout * fittingCurveLayout = new QGridLayout(groupBoxFittingCurve);
	fittingCurveLayout->setSpacing(5);

	generatePointsBtn = new QRadioButton();
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(generatePointsBtn, 0, 0);

	lblPoints = new QLabel();
	fittingCurveLayout->addWidget(lblPoints, 0, 1);
	generatePointsBox = new QSpinBox();
	generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(10);
	generatePointsBox->setValue(app->fitPoints);
	fittingCurveLayout->addWidget(generatePointsBox, 0, 2);

    linearFit2PointsBox = new QCheckBox();
    linearFit2PointsBox->setChecked(app->d_2_linear_fit_points);
    fittingCurveLayout->addWidget(linearFit2PointsBox, 0, 3);

	showPointsBox(!app->generateUniformFitPoints);

	samePointsBtn = new QRadioButton();
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(samePointsBtn, 1, 0);

	groupBoxMultiPeak = new QGroupBox();
	groupBoxMultiPeak->setCheckable(true);
	groupBoxMultiPeak->setChecked(app->generatePeakCurves);

	QHBoxLayout * multiPeakLayout = new QHBoxLayout(groupBoxMultiPeak);

	lblPeaksColor = new QLabel();
	multiPeakLayout->addWidget(lblPeaksColor);
	boxPeaksColor = new ColorBox(0);
	boxPeaksColor->setCurrentItem(app->peakCurvesColor);
	multiPeakLayout->addWidget(boxPeaksColor);

	groupBoxFitParameters = new QGroupBox();
	QGridLayout * fitParamsLayout = new QGridLayout(groupBoxFitParameters);

	lblPrecision = new QLabel();
	fitParamsLayout->addWidget(lblPrecision, 0, 0);
	boxPrecision = new QSpinBox();
	fitParamsLayout->addWidget(boxPrecision, 0, 1);
	boxPrecision->setValue(app->fit_output_precision);

	logBox = new QCheckBox();
	logBox->setChecked(app->writeFitResultsToLog);
	fitParamsLayout->addWidget(logBox, 1, 0);

	plotLabelBox = new QCheckBox();
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	fitParamsLayout->addWidget(plotLabelBox, 2, 0);

	scaleErrorsBox = new QCheckBox();
	fitParamsLayout->addWidget(scaleErrorsBox, 3, 0);
	scaleErrorsBox->setChecked(app->fit_scale_errors);

	QVBoxLayout* fitPageLayout = new QVBoxLayout(fitPage);
	fitPageLayout->addWidget(groupBoxFittingCurve);
	fitPageLayout->addWidget(groupBoxMultiPeak);
	fitPageLayout->addWidget(groupBoxFitParameters);
	fitPageLayout->addStretch();

	connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
}


void ConfigDialog::initCurvesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	curves = new QWidget();

	QGroupBox * curvesGroupBox = new QGroupBox();
	QGridLayout * curvesBoxLayout = new QGridLayout( curvesGroupBox );

	lblCurveStyle = new QLabel();
	curvesBoxLayout->addWidget( lblCurveStyle, 0, 0 );
	boxCurveStyle = new QComboBox();
	curvesBoxLayout->addWidget( boxCurveStyle, 0, 1 );

	lblLineWidth = new QLabel();
	curvesBoxLayout->addWidget( lblLineWidth, 1, 0 );
	boxCurveLineWidth = new DoubleSpinBox('f');
	boxCurveLineWidth->setLocale(app->locale());
	boxCurveLineWidth->setSingleStep(0.1);
	boxCurveLineWidth->setRange(0.1, 100);
	boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
	curvesBoxLayout->addWidget( boxCurveLineWidth, 1, 1 );

	lblSymbSize = new QLabel();
	curvesBoxLayout->addWidget( lblSymbSize, 2, 0 );
	boxSymbolSize = new QSpinBox();
	boxSymbolSize->setRange(1,100);
	boxSymbolSize->setValue(app->defaultSymbolSize/2);
	curvesBoxLayout->addWidget( boxSymbolSize, 2, 1 );

	curvesBoxLayout->setRowStretch( 3, 1 );

	QHBoxLayout * curvesPageLayout = new QHBoxLayout( curves );
	curvesPageLayout->addWidget( curvesGroupBox );
}

void ConfigDialog::initConfirmationsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	confirm = new QWidget();

	groupBoxConfirm = new QGroupBox();
	QVBoxLayout * layout = new QVBoxLayout( groupBoxConfirm );

	boxFolders = new QCheckBox();
	boxFolders->setChecked(app->confirmCloseFolder);
	layout->addWidget( boxFolders );

	boxTables = new QCheckBox();
	boxTables->setChecked(app->confirmCloseTable);
	layout->addWidget( boxTables );

	boxMatrices = new QCheckBox();
	boxMatrices->setChecked(app->confirmCloseMatrix);
	layout->addWidget( boxMatrices );

	boxPlots2D = new QCheckBox();
	boxPlots2D->setChecked(app->confirmClosePlot2D);
	layout->addWidget( boxPlots2D );

	boxPlots3D = new QCheckBox();
	boxPlots3D->setChecked(app->confirmClosePlot3D);
	layout->addWidget( boxPlots3D );

	boxNotes = new QCheckBox();
	boxNotes->setChecked(app->confirmCloseNotes);
	layout->addWidget( boxNotes );
	layout->addStretch();

	boxPromptRenameTables = new QCheckBox();
	boxPromptRenameTables->setChecked(app->d_inform_rename_table);

	QVBoxLayout * confirmPageLayout = new QVBoxLayout( confirm );
	confirmPageLayout->addWidget(groupBoxConfirm);
	confirmPageLayout->addWidget(boxPromptRenameTables);
	confirmPageLayout->addStretch();
}

void ConfigDialog::initFileLocationsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	fileLocationsPage = new QWidget();

	QGroupBox *gb = new QGroupBox();
	QGridLayout *gl = new QGridLayout(gb);

	lblTranslationsPath = new QLabel(tr("Translations"));
	gl->addWidget(lblTranslationsPath , 0, 0);

	translationsPathLine = new QLineEdit();
	translationsPathLine->setText(app->d_translations_folder);
	gl->addWidget(translationsPathLine, 0, 1);

	QPushButton *browseTranslationsBtn = new QPushButton();
	browseTranslationsBtn->setIcon(QIcon(QPixmap(choose_folder_xpm)));
	gl->addWidget(browseTranslationsBtn, 0, 2);

	lblHelpPath = new QLabel(tr("Help"));
	gl->addWidget(lblHelpPath, 1, 0 );

	QFileInfo hfi(app->helpFilePath);
	helpPathLine = new QLineEdit(hfi.dir().absolutePath());
	gl->addWidget( helpPathLine, 1, 1);

	QPushButton *browseHelpBtn = new QPushButton();
	browseHelpBtn->setIcon(QIcon(QPixmap(choose_folder_xpm)));
	gl->addWidget(browseHelpBtn, 1, 2);
	gl->setRowStretch(2, 1);

#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir = new QLabel(tr("Python Configuration Files"));
	gl->addWidget(lblPythonConfigDir, 2, 0);

	pythonConfigDirLine = new QLineEdit(app->d_python_config_folder);
	gl->addWidget(pythonConfigDirLine, 2, 1);

	QPushButton *browsePythonConfigBtn = new QPushButton();
	browsePythonConfigBtn->setIcon(QIcon(QPixmap(choose_folder_xpm)));
	connect(browsePythonConfigBtn, SIGNAL(clicked()), this, SLOT(choosePythonConfigFolder()));
	gl->addWidget(browsePythonConfigBtn, 2, 2);
	gl->setRowStretch(3, 1);
#endif

	QVBoxLayout *vl = new QVBoxLayout(fileLocationsPage);
	vl->addWidget(gb);

	appTabWidget->addTab(fileLocationsPage, QString());

	connect(browseTranslationsBtn, SIGNAL(clicked()), this, SLOT(chooseTranslationsFolder()));
	connect(browseHelpBtn, SIGNAL(clicked()), this, SLOT(chooseHelpFolder()));
}

void ConfigDialog::languageChange()
{
	setWindowTitle( tr( "QtiPlot - Choose default settings" ) );
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	// pages list
	itemsList->clear();
	itemsList->addItem( tr( "General" ) );
	itemsList->addItem( tr( "Tables" ) );
	itemsList->addItem( tr( "2D Plots" ) );
	itemsList->addItem( tr( "3D Plots" ) );
	itemsList->addItem( tr( "Notes" ) );
	itemsList->addItem( tr( "Fitting" ) );
	itemsList->setCurrentRow(0);
	itemsList->item(0)->setIcon(QIcon(QPixmap(general_xpm)));
	itemsList->item(1)->setIcon(QIcon(QPixmap(configTable_xpm)));
	itemsList->item(2)->setIcon(QIcon(QPixmap(config_curves_xpm)));
	itemsList->item(3)->setIcon(QIcon(QPixmap(logo_xpm)));
	itemsList->item(4)->setIcon(QIcon(QPixmap(notes_32_xpm)));
	itemsList->item(5)->setIcon(QIcon(QPixmap(fit_xpm)));
	itemsList->setIconSize(QSize(32,32));
	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(itemsList->font());
	int width = 32,i;
	for(i=0 ; i<itemsList->count() ; i++)
		if( fm.width(itemsList->item(i)->text()) > width)
			width = fm.width(itemsList->item(i)->text());
	itemsList->setMaximumWidth( itemsList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	itemsList->resize(itemsList->maximumWidth(),itemsList->height());

	//plots 2D page
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotOptions), tr("Options"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(curves), tr("Curves"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotTicks), tr("Ticks"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotFonts), tr("Fonts"));

	boxResize->setText(tr("Do not &resize layers when window size changes"));
    boxLabelsEditing->setText(tr("&Disable in-place editing"));
	lblMinTicksLength->setText(tr("Length"));

	lblAxesLineWidth->setText(tr("Axes linewidth" ));
	lblMajTicksLength->setText(tr("Length" ));
	lblMajTicks->setText(tr("Major Ticks" ));
	lblMinTicks->setText(tr("Minor Ticks" ));

	lblMargin->setText(tr("Margin" ));
	labelGraphAxesLabelsDist->setText(tr("Axes title space" ));
	labelFrameWidth->setText(tr("Frame width" ));
	boxBackbones->setText(tr("Axes &backbones"));
	boxFrame->setText(tr("Canvas Fra&me"));
	boxAllAxes->setText(tr("Sho&w all axes"));
	boxTitle->setText(tr("Show &Title"));
	boxScaleFonts->setText(tr("Scale &Fonts"));
	boxAutoscaling->setText(tr("Auto&scaling"));
	boxAntialiasing->setText(tr("Antia&liasing"));

	groupBackgroundOptions->setTitle(tr("Background"));
	labelGraphBkgColor->setText(tr("Background Color"));
	labelGraphBkgOpacity->setText(tr( "Opacity" ));
	labelGraphCanvasColor->setText(tr("Canvas Color" ));
	labelGraphCanvasOpacity->setText(tr("Opacity"));
	labelGraphFrameColor->setText(tr("Border Color"));
	labelGraphFrameWidth->setText(tr( "Width" ));
	boxBackgroundTransparency->setSpecialValueText(tr("Transparent"));
	boxCanvasTransparency->setSpecialValueText(tr("Transparent"));

	boxMajTicks->clear();
	boxMajTicks->addItem(tr("None"));
	boxMajTicks->addItem(tr("Out"));
	boxMajTicks->addItem(tr("In & Out"));
	boxMajTicks->addItem(tr("In"));

	boxMinTicks->clear();
	boxMinTicks->addItem(tr("None"));
	boxMinTicks->addItem(tr("Out"));
	boxMinTicks->addItem(tr("In & Out"));
	boxMinTicks->addItem(tr("In"));

	boxMajTicks->setCurrentIndex(app->majTicksStyle);
	boxMinTicks->setCurrentIndex(app->minTicksStyle);

	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotPrint), tr("Print"));
	boxPrintCropmarks->setText(tr("Print Crop&marks"));
	boxScaleLayersOnPrint->setText(tr("&Scale layers to paper size"));

	//confirmations page
	groupBoxConfirm->setTitle(tr("Prompt on closing"));
	boxFolders->setText(tr("Folders"));
	boxTables->setText(tr("Tables"));
	boxPlots3D->setText(tr("3D Plots"));
	boxPlots2D->setText(tr("2D Plots"));
	boxMatrices->setText(tr("Matrices"));
	boxNotes->setText(tr("&Notes"));

	buttonOk->setText( tr( "&OK" ) );
	buttonCancel->setText( tr( "&Cancel" ) );
	buttonApply->setText( tr( "&Apply" ) );
	buttonTextFont->setText( tr( "&Text Font" ) );
	buttonHeaderFont->setText( tr( "&Labels Font" ) );
	buttonAxesFont->setText( tr( "A&xes Labels" ) );
	buttonNumbersFont->setText( tr( "Axes &Numbers" ) );
	buttonLegendFont->setText( tr( "&Legend" ) );
	buttonTitleFont->setText( tr( "T&itle" ) );
	boxPromptRenameTables->setText( tr( "Prompt on &renaming tables when appending projects" ) );

	//application page
	appTabWidget->setTabText(appTabWidget->indexOf(application), tr("Application"));
	appTabWidget->setTabText(appTabWidget->indexOf(confirm), tr("Confirmations"));
	appTabWidget->setTabText(appTabWidget->indexOf(appColors), tr("Colors"));
	appTabWidget->setTabText(appTabWidget->indexOf(numericFormatPage), tr("Numeric Format"));
	appTabWidget->setTabText(appTabWidget->indexOf(fileLocationsPage), tr("File Locations"));

	lblLanguage->setText(tr("Language"));
	lblStyle->setText(tr("Style"));
	lblFonts->setText(tr("Main Font"));
	fontsBtn->setText(tr("Choose &font"));
	lblWorkspace->setText(tr("Workspace"));
	lblPanelsText->setText(tr("Panels text"));
	lblPanels->setText(tr("Panels"));
	boxSave->setText(tr("Save every"));
	boxBackupProject->setText(tr("&Backup project before saving"));
	boxSearchUpdates->setText(tr("Check for new versions at startup"));
	boxMinutes->setSuffix(tr(" minutes"));
	lblScriptingLanguage->setText(tr("Default scripting language"));
	lblUndoStackSize->setText(tr("Matrix Undo Stack Size"));
	lblEndOfLine->setText(tr("Endline character"));
	lblInitWindow->setText(tr("Start New Project"));
	boxInitWindow->clear();
	boxInitWindow->addItem(tr("Empty"));
	boxInitWindow->addItem(tr("Table"));
	boxInitWindow->addItem(tr("Matrix"));
	boxInitWindow->addItem(tr("Empty Graph"));
	boxInitWindow->addItem(tr("Note"));
	boxInitWindow->setCurrentIndex((int)app->d_init_window_type);
#ifdef SCRIPTING_PYTHON
    completionBox->setText(tr("&Enable autocompletion (Ctrl+U)"));
#endif

	lblAppPrecision->setText(tr("Number of Decimal Digits"));
	lblDecimalSeparator->setText(tr("Decimal Separators"));
	boxDecimalSeparator->clear();
	boxDecimalSeparator->addItem(tr("System Locale Setting"));
	boxDecimalSeparator->addItem("1,000.0");
	boxDecimalSeparator->addItem("1.000,0");
	boxDecimalSeparator->addItem("1 000,0");
	boxThousandsSeparator->setText(tr("Omit &Thousands Separator"));

    QLocale locale = app->locale();
    if (locale.name() == QLocale::c().name())
        boxDecimalSeparator->setCurrentIndex(1);
    else if (locale.name() == QLocale(QLocale::German).name())
        boxDecimalSeparator->setCurrentIndex(2);
    else if (locale.name() == QLocale(QLocale::French).name())
        boxDecimalSeparator->setCurrentIndex(3);

	lblClipboardSeparator->setText(tr("Clipboard Decimal Separators"));
	boxClipboardLocale->clear();
	boxClipboardLocale->addItem(tr("System Locale Setting"));
	boxClipboardLocale->addItem("1,000.0");
	boxClipboardLocale->addItem("1.000,0");
	boxClipboardLocale->addItem("1 000,0");

    if (app->clipboardLocale().name() == QLocale::c().name())
        boxClipboardLocale->setCurrentIndex(1);
    else if (app->clipboardLocale().name() == QLocale(QLocale::German).name())
        boxClipboardLocale->setCurrentIndex(2);
    else if (app->clipboardLocale().name() == QLocale(QLocale::French).name())
        boxClipboardLocale->setCurrentIndex(3);

	lblTranslationsPath->setText(tr("Translations"));
	lblHelpPath->setText(tr("Help"));
#ifdef SCRIPTING_PYTHON
	lblPythonConfigDir->setText(tr("Python Configuration Files"));
#endif

	//tables page
	boxUpdateTableValues->setText(tr("Automatically &Recalculate Column Values"));
	boxTableComments->setText(tr("&Display Comments in Header"));
	groupBoxTableCol->setTitle(tr("Colors"));
	lblSeparator->setText(tr("Default Column Separator"));
	boxSeparator->clear();
	boxSeparator->addItem(tr("TAB"));
	boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem(";" + tr("SPACE"));
	boxSeparator->addItem("," + tr("SPACE"));
	boxSeparator->addItem(";");
	boxSeparator->addItem(",");
	setColumnSeparator(app->columnSeparator);

	lblTableBackground->setText(tr( "Background" ));
	lblTextColor->setText(tr( "Text" ));
	lblHeaderColor->setText(tr("Labels"));
	groupBoxTableFonts->setTitle(tr("Fonts"));

	//curves page
	lblCurveStyle->setText(tr( "Default curve style" ));
	lblLineWidth->setText(tr( "Line width" ));
	lblSymbSize->setText(tr( "Symbol size" ));

	boxCurveStyle->clear();
	boxCurveStyle->addItem( QPixmap(lPlot_xpm), tr( " Line" ) );
	boxCurveStyle->addItem( QPixmap(pPlot_xpm), tr( " Scatter" ) );
	boxCurveStyle->addItem( QPixmap(lpPlot_xpm), tr( " Line + Symbol" ) );
	boxCurveStyle->addItem( QPixmap(dropLines_xpm), tr( " Vertical drop lines" ) );
	boxCurveStyle->addItem( QPixmap(spline_xpm), tr( " Spline" ) );
	boxCurveStyle->addItem( QPixmap(vert_steps_xpm), tr( " Vertical steps" ) );
	boxCurveStyle->addItem( QPixmap(hor_steps_xpm), tr( " Horizontal steps" ) );
	boxCurveStyle->addItem( QPixmap(area_xpm), tr( " Area" ) );
	boxCurveStyle->addItem( QPixmap(vertBars_xpm), tr( " Vertical Bars" ) );
	boxCurveStyle->addItem( QPixmap(hBars_xpm), tr( " Horizontal Bars" ) );

	int style = app->defaultCurveStyle;
	if (style == Graph::Line)
		boxCurveStyle->setCurrentItem(0);
	else if (style == Graph::Scatter)
		boxCurveStyle->setCurrentItem(1);
	else if (style == Graph::LineSymbols)
		boxCurveStyle->setCurrentItem(2);
	else if (style == Graph::VerticalDropLines)
		boxCurveStyle->setCurrentItem(3);
	else if (style == Graph::Spline)
		boxCurveStyle->setCurrentItem(4);
	else if (style == Graph::VerticalSteps)
		boxCurveStyle->setCurrentItem(5);
	else if (style == Graph::HorizontalSteps)
		boxCurveStyle->setCurrentItem(6);
	else if (style == Graph::Area)
		boxCurveStyle->setCurrentItem(7);
	else if (style == Graph::VerticalBars)
		boxCurveStyle->setCurrentItem(8);
	else if (style == Graph::HorizontalBars)
		boxCurveStyle->setCurrentItem(9);

	//plots 3D
	lblResolution->setText(tr("Resolution"));
	boxResolution->setSpecialValueText( "1 " + tr("(all data shown)") );
	boxShowLegend->setText(tr( "&Show Legend" ));
	boxShowProjection->setText(tr( "Show &Projection" ));
	btnFromColor->setText( tr( "&Data Max" ) );
	boxSmoothMesh->setText(tr( "Smoot&h Line" ));
	boxOrthogonal->setText(tr( "O&rthogonal" ));
	btnLabels->setText( tr( "Lab&els" ) );
	btnMesh->setText( tr( "Mesh &Line" ) );
	btnGrid->setText( tr( "&Grid" ) );
	btnToColor->setText( tr( "Data &Min" ) );
	btnNumbers->setText( tr( "&Numbers" ) );
	btnAxes->setText( tr( "A&xes" ) );
	btnBackground3D->setText( tr( "&Background" ) );
	groupBox3DCol->setTitle(tr("Colors" ));
	groupBox3DFonts->setTitle(tr("Fonts" ));
	btnTitleFnt->setText( tr( "&Title" ) );
	btnLabelsFnt->setText( tr( "&Axes Labels" ) );
	btnNumFnt->setText( tr( "&Numbers" ) );
	boxAutoscale3DPlots->setText( tr( "Autosca&ling" ) );

    //Notes page
    labelTabLength->setText(tr("Tab length (pixels)"));
    labelNotesFont->setText(tr("Font"));
    lineNumbersBox->setText(tr("&Display line numbers"));
#ifdef SCRIPTING_PYTHON
	groupSyntaxHighlighter->setTitle(tr("Syntax Highlighting"));
	buttonCommentColor->setText(tr("Co&mments"));
	buttonKeywordColor->setText(tr("&Keywords"));
	buttonNumericColor->setText(tr("&Numbers"));
	buttonQuotationColor->setText(tr("&Quotations"));
	buttonFunctionColor->setText(tr("&Functions"));
	buttonClassColor->setText(tr("Q&t Classes"));
#endif
	
	//Fitting page
	groupBoxFittingCurve->setTitle(tr("Generated Fit Curve"));
	generatePointsBtn->setText(tr("Uniform X Function"));
	lblPoints->setText( tr("Points") );
	samePointsBtn->setText( tr( "Same X as Fitting Data" ) );
	linearFit2PointsBox->setText( tr( "2 points for linear fits" ) );

	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));

	groupBoxFitParameters->setTitle(tr("Parameters Output"));
	lblPrecision->setText(tr("Significant Digits"));
	logBox->setText(tr("Write Parameters to Result Log"));
	plotLabelBox->setText(tr("Paste Parameters to Plot"));
	scaleErrorsBox->setText(tr("Scale Errors with sqrt(Chi^2/doF)"));
	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));
	lblPeaksColor->setText(tr("Peaks Color"));
}

void ConfigDialog::accept()
{
	apply();
	close();
}

void ConfigDialog::apply()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	// tables page
	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", false);
	sep.replace("\\t", "\t");
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");

	if (sep.contains(QRegExp("[0-9.eE+-]"))!=0){
		QMessageBox::warning(0, tr("QtiPlot - Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	app->columnSeparator = sep;
	app->setAutoUpdateTableValues(boxUpdateTableValues->isChecked());

	app->tableBkgdColor = buttonBackground->color();
	app->tableTextColor = buttonText->color();
	app->tableHeaderColor = buttonHeader->color();
	app->tableTextFont = textFont;
	app->tableHeaderFont = headerFont;
	app->d_show_table_comments = boxTableComments->isChecked();

    QColorGroup cg;
	cg.setColor(QColorGroup::Base, buttonBackground->color());
	cg.setColor(QColorGroup::Text, buttonText->color());
	QPalette palette(cg, cg, cg);

	QList<MdiSubWindow *> windows = app->windowsList();
	foreach(MdiSubWindow *w, windows){
		if (w->inherits("Table")){
			Table *t = (Table*)w;
            w->setPalette(palette);
			t->setHeaderColor(buttonHeader->color());
            t->setTextFont(textFont);
            t->setHeaderFont(headerFont);
            t->showComments(boxTableComments->isChecked());
		}
	}

	app->d_graph_background_color = boxBackgroundColor->color();
	app->d_graph_background_opacity = boxBackgroundTransparency->value();
	app->d_graph_canvas_color = boxCanvasColor->color();
	app->d_graph_canvas_opacity = boxCanvasTransparency->value();
	app->d_graph_border_color = boxBorderColor->color();
	app->d_graph_border_width = boxBorderWidth->value();

	// 2D plots page: options tab
	app->d_in_place_editing = !boxLabelsEditing->isChecked();
	app->titleOn=boxTitle->isChecked();
	app->allAxesOn = boxAllAxes->isChecked();

	if (boxFrame->isChecked())
		app->canvasFrameWidth = boxFrameWidth->value();
	else
		app->canvasFrameWidth = 0;

	app->drawBackbones = boxBackbones->isChecked();
	app->axesLineWidth = boxLineWidth->value();
	app->defaultPlotMargin = boxMargin->value();
	app->d_graph_axes_labels_dist = boxAxesLabelsDist->value();
	app->setGraphDefaultSettings(boxAutoscaling->isChecked(),boxScaleFonts->isChecked(),
								boxResize->isChecked(), boxAntialiasing->isChecked());
	// 2D plots page: curves tab
	app->defaultCurveStyle = curveStyle();
	app->defaultCurveLineWidth = boxCurveLineWidth->value();
	app->defaultSymbolSize = 2*boxSymbolSize->value() + 1;
	// 2D plots page: ticks tab
	app->majTicksLength = boxMajTicksLength->value();
	app->minTicksLength = boxMinTicksLength->value();
	app->majTicksStyle = boxMajTicks->currentItem();
	app->minTicksStyle = boxMinTicks->currentItem();
	// 2D plots page: fonts tab
	app->plotAxesFont=axesFont;
	app->plotNumbersFont=numbersFont;
	app->plotLegendFont=legendFont;
	app->plotTitleFont=titleFont;
	// 2D plots page: print tab
	app->d_print_cropmarks = boxPrintCropmarks->isChecked();
	app->d_scale_plots_on_print = boxScaleLayersOnPrint->isChecked();
	foreach(MdiSubWindow *w, windows){
		MultiLayer *ml = qobject_cast<MultiLayer *>(w);
		if (ml){
			ml->setScaleLayersOnPrint(boxScaleLayersOnPrint->isChecked());
			ml->printCropmarks(boxPrintCropmarks->isChecked());
		}
	}
	// general page: application tab
	app->changeAppFont(appFont);
	setFont(appFont);
	app->changeAppStyle(boxStyle->currentText());
	app->autoSearchUpdates = boxSearchUpdates->isChecked();
	app->setSaveSettings(boxSave->isChecked(), boxMinutes->value());
	app->d_backup_files = boxBackupProject->isChecked();
	app->defaultScriptingLang = boxScriptingLanguage->currentText();
	app->d_init_window_type = (ApplicationWindow::WindowType)boxInitWindow->currentIndex();
	app->setMatrixUndoStackSize(undoStackSizeBox->value());
	app->d_eol = (ApplicationWindow::EndLineChar)boxEndLine->currentIndex();
#ifdef SCRIPTING_PYTHON
    app->enableCompletion(completionBox->isChecked());
#endif

	// general page: numeric format tab
	app->d_decimal_digits = boxAppPrecision->value();
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex()){
        case 0:
            locale = QLocale::system();
        break;
        case 1:
            locale = QLocale::c();
        break;
        case 2:
            locale = QLocale(QLocale::German);
        break;
        case 3:
            locale = QLocale(QLocale::French);
        break;
    }
    if (boxThousandsSeparator->isChecked())
        locale.setNumberOptions(QLocale::OmitGroupSeparator);

    QLocale oldLocale = app->locale();
    app->setLocale(locale);

	if (generalDialog->currentWidget() == appTabWidget &&
		appTabWidget->currentWidget() == numericFormatPage){
    	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QList<MdiSubWindow *> windows = app->windowsList();
        foreach(MdiSubWindow *w, windows){
            w->setLocale(locale);

            if(w->isA("Table"))
                ((Table *)w)->updateDecimalSeparators(oldLocale);
            else if(w->isA("Matrix"))
                ((Matrix *)w)->resetView();
        }

		switch (boxClipboardLocale->currentIndex()){
        	case 0:
            	app->setClipboardLocale(QLocale::system());
        	break;
        	case 1:
            	app->setClipboardLocale(QLocale::c());
        	break;
        	case 2:
            	app->setClipboardLocale(QLocale(QLocale::German));
        	break;
        	case 3:
            	app->setClipboardLocale(QLocale(QLocale::French));
        	break;
    	}

        app->modifiedProject();
    	QApplication::restoreOverrideCursor();
	}

	// general page: file locations tab
	if (generalDialog->currentWidget() == appTabWidget &&
		appTabWidget->currentWidget() == fileLocationsPage){
		QString path = translationsPathLine->text();
		if (path != app->d_translations_folder){
			QFileInfo fi(path);
			if (fi.exists() && fi.isDir()){
				app->d_translations_folder = fi.absoluteFilePath();
				app->createLanguagesList();
				insertLanguagesList();
			}
		}

		path = helpPathLine->text() + "/index.html";
		if (path != app->helpFilePath){
			QFileInfo fi(path);
			if (fi.exists() && fi.isFile())
				app->helpFilePath = fi.absoluteFilePath();
		}

#ifdef SCRIPTING_PYTHON
		path = pythonConfigDirLine->text();
		if (path != app->d_python_config_folder){
			QFileInfo fi(path);
			if (fi.exists() && fi.isDir())
				app->d_python_config_folder = fi.absoluteFilePath();
		}
#endif
	}


	// general page: confirmations tab
	app->d_inform_rename_table = boxPromptRenameTables->isChecked();
	app->confirmCloseFolder = boxFolders->isChecked();
	app->updateConfirmOptions(boxTables->isChecked(), boxMatrices->isChecked(),
			boxPlots2D->isChecked(), boxPlots3D->isChecked(),
			boxNotes->isChecked());
	// general page: colors tab
	app->setAppColors(btnWorkspace->color(), btnPanels->color(), btnPanelsText->color());
	// 3D plots page
	QStringList plot3DColors = QStringList() << btnToColor->color().name() << btnLabels->color().name();
	plot3DColors << btnMesh->color().name() << btnGrid->color().name() << btnFromColor->color().name();
	plot3DColors << btnNumbers->color().name() << btnAxes->color().name() << btnBackground3D->color().name();
	app->plot3DColors = plot3DColors;

	app->showPlot3DLegend = boxShowLegend->isChecked();
	app->showPlot3DProjection = boxShowProjection->isChecked();
	app->plot3DResolution = boxResolution->value();
	app->plot3DTitleFont = plot3DTitleFont;
	app->plot3DNumbersFont = plot3DNumbersFont;
	app->plot3DAxesFont = plot3DAxesFont;
	app->orthogonal3DPlots = boxOrthogonal->isChecked();
	app->smooth3DMesh = boxSmoothMesh->isChecked();
	app->autoscale3DPlots = boxAutoscale3DPlots->isChecked();
	app->setPlot3DOptions();

	// fitting page
	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->generatePeakCurves = groupBoxMultiPeak->isChecked();
	app->peakCurvesColor = boxPeaksColor->currentIndex();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->d_2_linear_fit_points = linearFit2PointsBox->isChecked();
	app->saveSettings();

	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(itemsList->font());
	int width = 32;
	for(int i=0; i<itemsList->count(); i++)
		if( fm.width(itemsList->item(i)->text()) > width)
			width = fm.width(itemsList->item(i)->text());
	itemsList->setMaximumWidth( itemsList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	itemsList->resize(itemsList->maximumWidth(),itemsList->height());
}

int ConfigDialog::curveStyle()
{
	int style = 0;
	switch (boxCurveStyle->currentItem())
	{
		case 0:
			style = Graph::Line;
			break;
		case 1:
			style = Graph::Scatter;
			break;
		case 2:
			style = Graph::LineSymbols;
			break;
		case 3:
			style = Graph::VerticalDropLines;
			break;
		case 4:
			style = Graph::Spline;
			break;
		case 5:
			style = Graph::VerticalSteps;
			break;
		case 6:
			style = Graph::HorizontalSteps;
			break;
		case 7:
			style = Graph::Area;
			break;
		case 8:
			style = Graph::VerticalBars;
			break;
		case 9:
			style = Graph::HorizontalBars;
			break;
	}
	return style;
}

void ConfigDialog::pickTextFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,textFont,this);
	if ( ok ) {
		textFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickHeaderFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,headerFont,this);
	if ( ok ) {
		headerFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickLegendFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,legendFont,this);
	if ( ok ) {
		legendFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,axesFont,this);
	if ( ok ) {
		axesFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,numbersFont,this);
	if ( ok ) {
		numbersFont = font;
	} else {
		return;
	}
}

void ConfigDialog::pickTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,titleFont,this);
	if ( ok )
		titleFont = font;
	else
		return;
}

void ConfigDialog::pickApplicationFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,appFont,this);
	if ( ok )
		appFont = font;
	else
		return;
	fontsBtn->setFont(appFont);
}

void ConfigDialog::pick3DTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DTitleFont,this);
	if ( ok )
		plot3DTitleFont = font;
	else
		return;
}

void ConfigDialog::pick3DNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DNumbersFont,this);
	if ( ok )
		plot3DNumbersFont = font;
	else
		return;
}

void ConfigDialog::pick3DAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DAxesFont,this);
	if ( ok )
		plot3DAxesFont = font;
	else
		return;
}

void ConfigDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep==" ")
		boxSeparator->setCurrentIndex(1);
	else if (sep==";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep==",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep=="; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep==", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep==";")
		boxSeparator->setCurrentIndex(6);
	else if (sep==",")
		boxSeparator->setCurrentIndex(7);
	else
	{
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

void ConfigDialog::switchToLanguage(int param)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	app->switchToLanguage(param);
	languageChange();
}

void ConfigDialog::insertLanguagesList()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if(!app)
		return;

	boxLanguage->clear();
	QString qmPath = app->d_translations_folder;
	QDir dir(qmPath);
	QStringList locales = app->locales;
	QStringList languages;
	int lang = 0;
	for (int i=0; i < (int)locales.size(); i++)
	{
		if (locales[i] == "en")
			languages.push_back("English");
		else
		{
			QTranslator translator;
			translator.load("qtiplot_"+locales[i], qmPath);

			QString language = translator.translate("ApplicationWindow", "English");
			if (!language.isEmpty())
				languages.push_back(language);
			else
				languages.push_back(locales[i]);
		}

		if (locales[i] == app->appLanguage)
			lang = i;
	}
	boxLanguage->addItems(languages);
	boxLanguage->setCurrentIndex(lang);
}


void ConfigDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked())
	{
		lblPoints->show();
		generatePointsBox->show();
		linearFit2PointsBox->show();
	}
	else
	{
		lblPoints->hide();
		generatePointsBox->hide();
		linearFit2PointsBox->hide();
	}
}

void ConfigDialog::chooseTranslationsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo tfi(app->d_translations_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the QtiPlot translations folder!"),
#ifdef Q_CC_MSVC
			tfi.dir().absolutePath(), 0);
#else
			tfi.dir().absolutePath(), !QFileDialog::ShowDirsOnly);
#endif

	if (!dir.isEmpty()){
		app->d_translations_folder = dir;
		translationsPathLine->setText(dir);
		app->createLanguagesList();
		insertLanguagesList();
	}
}

void ConfigDialog::chooseHelpFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	app->chooseHelpFolder();

	QFileInfo hfi(app->helpFilePath);
	helpPathLine->setText(hfi.dir().absolutePath());
}

#ifdef SCRIPTING_PYTHON
void ConfigDialog::choosePythonConfigFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	QFileInfo tfi(app->d_python_config_folder);
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the Python configuration files!"),
#ifdef Q_CC_MSVC
			tfi.dir().absolutePath(), 0);
#else
			tfi.dir().absolutePath(), !QFileDialog::ShowDirsOnly);
#endif

	if (!dir.isEmpty()){
		app->d_python_config_folder = dir;
		pythonConfigDirLine->setText(dir);

		if (app->scriptingEnv()->name() == QString("Python"))
			app->setScriptingLanguage(QString("Python"), true);
	}
}

void ConfigDialog::rehighlight()
{
    if (generalDialog->currentWidget() != notesPage)
        return;

    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

    app->d_comment_highlight_color = buttonCommentColor->color();
	app->d_keyword_highlight_color = buttonKeywordColor->color();
	app->d_quotation_highlight_color = buttonQuotationColor->color();
	app->d_numeric_highlight_color = buttonNumericColor->color();
	app->d_function_highlight_color = buttonFunctionColor->color();
	app->d_class_highlight_color = buttonClassColor->color();
	
    QList<MdiSubWindow *> windows = app->windowsList();
    foreach(MdiSubWindow *w, windows){
        Note *n = qobject_cast<Note *>(w);
        if (n)
			n->editor()->rehighlight();
    }
}
#endif

void ConfigDialog::customizeNotes()
{
    if (generalDialog->currentWidget() != notesPage)
        return;

    ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

    app->d_note_line_numbers = lineNumbersBox->isChecked();
    app->d_notes_tab_length = boxTabLength->value();
    QFont f = QFont(boxFontFamily->currentFont().family(), boxFontSize->value());
    f.setBold(buttonBoldFont->isChecked());
    f.setItalic(buttonItalicFont->isChecked());
    app->d_notes_font = f;
    QList<MdiSubWindow *> windows = app->windowsList();
    foreach(MdiSubWindow *w, windows){
        Note *n = qobject_cast<Note *>(w);
        if (n){
            n->showLineNumbers(app->d_note_line_numbers);
            n->editor()->setTabStopWidth(app->d_notes_tab_length);
            n->setFont(f);
        }
    }
}
