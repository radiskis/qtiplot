TARGET   = qtiplot
QTI_ROOT = ..
!include( $$QTI_ROOT/build.conf ) {
  message( "You need a build.conf file with local settings!" )
}

##################### 3rd PARTY HEADER FILES SECTION ########################
#!!! Warning: You must set this up in $$QTIROOT/build.conf
#############################################################################

# configurable
INCLUDEPATH       += $$ZLIB_INCLUDEPATH
INCLUDEPATH       += $$MUPARSER_INCLUDEPATH
INCLUDEPATH       += $$QWT_INCLUDEPATH
INCLUDEPATH       += $$QWT3D_INCLUDEPATH
INCLUDEPATH       += $$GSL_INCLUDEPATH

# configurable libs
LIBS         += $$MUPARSER_LIBS
LIBS         += $$QWT_LIBS
LIBS         += $$QWT3D_LIBS
LIBS         += $$GSL_LIBS

#############################################################################
###################### BASIC PROJECT PROPERTIES #############################
#############################################################################

DEFINES       += SVN_REVISION="\"\\\"$$(QTIPLOT_SVN_REVISION)\\\"\""

QMAKE_PROJECT_DEPTH = 0
	
!contains(CONFIG, BrowserPlugin){
	TEMPLATE       = app
}

CONFIG        += qt warn_on exceptions opengl thread
CONFIG        += assistant
win32:CONFIG  += qaxcontainer

DEFINES       += QT_PLUGIN
contains(CONFIG, CustomInstall){
	INSTALLS        += target
	INSTALLS        += translations
	INSTALLS        += manual
	INSTALLS        += documentation
	unix:INSTALLS        += man

	unix: INSTALLBASE = /usr
	win32: INSTALLBASE = C:/QtiPlot

	unix: target.path = $$INSTALLBASE/bin
	unix: translations.path = $$INSTALLBASE/share/qtiplot/translations
	unix: manual.path = $$INSTALLBASE/share/doc/qtiplot/manual
	unix: documentation.path = $$INSTALLBASE/share/doc/qtiplot
	unix: man.path = $$INSTALLBASE/share/man/man1/

	win32: target.path = $$INSTALLBASE
	win32: translations.path = $$INSTALLBASE/translations
	win32: manual.path = $$INSTALLBASE/manual
	win32: documentation.path = $$INSTALLBASE/doc

	DEFINES       += TRANSLATIONS_PATH="\\\"$$replace(translations.path," ","\ ")\\\"
	DEFINES       += MANUAL_PATH="\\\"$$replace(manual.path," ","\ ")\\\"
	}

QT            += opengl qt3support network svg xml
contains(CONFIG, StaticBuild){
	QTPLUGIN += qjpeg qgif qtiff qmng qsvg
	DEFINES += QTIPLOT_STATIC_BUILD
} else {
	win32:DEFINES += QT_DLL QT_THREAD_SUPPORT
}

MOC_DIR        = ../tmp/qtiplot
OBJECTS_DIR    = ../tmp/qtiplot
SIP_DIR        = ../tmp/qtiplot
DESTDIR        = ./

#############################################################################
###################### PROJECT FILES SECTION ################################
#############################################################################

###################### ICONS ################################################
INCLUDEPATH  += icons/
win32:RC_FILE = icons/qtiplot.rc
mac:RC_FILE   = icons/qtiplot.icns
RESOURCES     = ../manual/html/icons/icons.qrc

###################### TRANSLATIONS #########################################

TRANSLATIONS    = translations/qtiplot_cn.ts \
		  translations/qtiplot_cz.ts \
		  translations/qtiplot_de.ts \
		  translations/qtiplot_el.ts \
		  translations/qtiplot_es.ts \
		  translations/qtiplot_fr.ts \
		  translations/qtiplot_it.ts \
		  translations/qtiplot_pt.ts \
		  translations/qtiplot_ro.ts \
		  translations/qtiplot_ru.ts \
		  translations/qtiplot_ja.ts \
		  translations/qtiplot_sv.ts

translations.files += translations/qtiplot_cn.qm \
		translations/qtiplot_cz.qm \
		translations/qtiplot_de.qm \
		translations/qtiplot_es.qm \
		translations/qtiplot_el.qm \
		translations/qtiplot_fr.qm \
		translations/qtiplot_pt.qm \
		translations/qtiplot_ro.qm \
		translations/qtiplot_ru.qm \
		translations/qtiplot_ja.qm \
		translations/qtiplot_sv.qm

isEmpty(LUPDATE): LUPDATE = lupdate
#system($$LUPDATE -verbose qtiplot.pro)
isEmpty(LRELEASE): LRELEASE = lrelease
#system($$LRELEASE -verbose qtiplot.pro)

###################### DOCUMENTATION ########################################

manual.files += ../manual/html \
                ../manual/qtiplot-manual-en.pdf

documentation.files += ../README.html \
                       ../gpl_licence.txt

unix: man.files += ../qtiplot.1

###############################################################
##################### Compression (zlib-1.2.3) ################
###############################################################

SOURCES += ../3rdparty/zlib/minigzip.c

###############################################################
################# Default Modules #############################
###############################################################

include(src/analysis/analysis.pri)
include(src/core/core.pri)
include(src/excel/excel.pri)
include(src/lib/libqti.pri)
include(src/plot2D/plot2D.pri)
include(src/plot3D/plot3D.pri)
include(src/matrix/matrix.pri)
include(src/table/table.pri)
include(src/scripting/scripting.pri)

###############################################################
##################### Scripting: PYTHON + SIP + PyQT ##########
###############################################################

contains(SCRIPTING_LANGS, Python) {
  contains(CONFIG, CustomInstall){
  	INSTALLS += pythonconfig
  	pythonconfig.files += qtiplotrc.py \
  				    qtiUtil.py \
  				    qti_wordlist.txt \

  	unix: pythonconfig.path = /usr/local/qtiplot
  	win32: pythonconfig.path = $$INSTALLBASE
  	DEFINES += PYTHON_CONFIG_PATH="\\\"$$replace(pythonconfig.path," ","\ ")\\\"
  }

  unix {
    isEmpty(PYTHON): PYTHON = python
    INCLUDEPATH += $$system($$PYTHON python-includepath.py)
    LIBS        += $$system($$PYTHON -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS        += -lm
    system(mkdir -p $${SIP_DIR})
    system($$system($$PYTHON python-sipcmd.py) -c $${SIP_DIR} src/scripting/qti.sip)
  }

  win32 {
    INCLUDEPATH += $$system(call python-includepath.py)
    LIBS        += $$system(call python-libs-win.py)
    system($$system(call python-sipcmd.py) -c $${SIP_DIR} src/scripting/qti.sip)
  }
}

###############################################################

# check if we have libpng
!isEmpty(LIBPNG_LIBS) {
	DEFINES += GL2PS_HAVE_LIBPNG
	INCLUDEPATH += $$LIBPNG_INCLUDEPATH
	LIBS        += $$LIBPNG_LIBS
}

###############################################################

# check if we have QTeXEngine
!isEmpty(TEX_ENGINE_LIBS) {
	DEFINES += TEX_OUTPUT
	INCLUDEPATH += $$TEX_ENGINE_INCLUDEPATH
	LIBS        += $$TEX_ENGINE_LIBS
}

###############################################################

# check if we have ALGLIB
!isEmpty(ALGLIB_LIBS) {
	DEFINES += HAVE_ALGLIB
	INCLUDEPATH += $$ALGLIB_INCLUDEPATH
	LIBS        += $$ALGLIB_LIBS
}

###############################################################

# check if we have TAMUANOVA
!isEmpty(TAMUANOVA_LIBS) {
	DEFINES += HAVE_TAMUANOVA
	INCLUDEPATH += $$TAMUANOVA_INCLUDEPATH
	LIBS        += $$TAMUANOVA_LIBS
}

###############################################################

#At the very end: add global include- and lib path
unix:INCLUDEPATH += $$SYS_INCLUDEPATH
unix:LIBS += $$SYS_LIBS

###############################################################
############### Building QtiPlot as a browser plugin ##########
###############################################################

contains(CONFIG, BrowserPlugin){
	DEFINES += BROWSER_PLUGIN
	win32: CONFIG  += qaxserver
	RC_FILE	= qtiplot.rc
	include(../3rdparty/QtSolutions/qtbrowserplugin/src/qtbrowserplugin.pri)
}
