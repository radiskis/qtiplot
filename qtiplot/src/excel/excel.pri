###############################################################
################# Excel Import ################################
###############################################################
INCLUDEPATH += src/excel/

win32 {
	contains(CONFIG, Excel) {
		SOURCES += src/excel/ExcelImport.cpp
	}
}

!isEmpty(QUAZIP_LIBS) {
	HEADERS += src/excel/ExcelFileConverter.h
	HEADERS += src/excel/OdsFileHandler.h

	SOURCES += src/excel/ExcelFileConverter.cpp
	SOURCES += src/excel/OdsFileHandler.cpp
}
