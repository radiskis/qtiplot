###############################################################
################# Excel Import ################################
###############################################################
INCLUDEPATH += src/excel/

HEADERS += src/excel/adodb.h

SOURCES += src/excel/adodb.cpp
SOURCES += src/excel/ExcelADO.cpp
contains(CONFIG, Excel) {
	SOURCES += src/excel/ExcelImport.cpp
}
