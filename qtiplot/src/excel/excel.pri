###############################################################
################# Excel Import ################################
###############################################################
INCLUDEPATH += src/excel/

win32 {
	HEADERS += src/excel/adodb.h

	SOURCES += src/excel/adodb.cpp
	SOURCES += src/excel/ExcelADO.cpp
	contains(CONFIG, Excel) {
		SOURCES += src/excel/ExcelImport.cpp
	}
}

!isEmpty(QUAZIP_LIBS) {
	HEADERS += src/excel/ExcelFileConverter.h
	SOURCES += src/excel/ExcelFileConverter.cpp
}
