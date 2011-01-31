###############################################################
################# Excel Import ################################
###############################################################
INCLUDEPATH += src/excel/

win32 {
	contains(CONFIG, Excel) {
		SOURCES += src/excel/ExcelImport.cpp
	}
}

HEADERS += src/excel/ExcelFileConverter.h
SOURCES += src/excel/ExcelFileConverter.cpp

!isEmpty(QUAZIP_LIBS) {
	HEADERS += src/excel/OdsFileHandler.h
	SOURCES += src/excel/OdsFileHandler.cpp
}
