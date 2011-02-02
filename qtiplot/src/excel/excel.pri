###############################################################
################# Excel Import ################################
###############################################################
INCLUDEPATH += src/excel/

HEADERS += src/excel/ExcelFileConverter.h
SOURCES += src/excel/ExcelFileConverter.cpp

!isEmpty(QUAZIP_LIBS) {
	HEADERS += src/excel/OdsFileHandler.h
	SOURCES += src/excel/OdsFileHandler.cpp
}
