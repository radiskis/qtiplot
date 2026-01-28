# qmake project file for building the EmfEngine libraries

include( ../config.pri )

TARGET       = EmfEngine
TEMPLATE     = lib

MOC_DIR      = ../tmp
OBJECTS_DIR  = ../tmp
DESTDIR      = ../

contains(CONFIG, EmfEngineDll) {
    CONFIG  += dll
    DEFINES += EMFENGINE_DLL EMFENGINE_DLL_BUILD
} else {
    CONFIG  += staticlib
}

HEADERS = EmfEngine.h
	   
contains(CONFIG, HAVE_GDI_PLUS){
	win32: {
		DEFINES += HAVE_GDIPLUS
		win32: LIBS += -lgdiplus
		SOURCES += EmfEnginePlus.cpp
		SOURCES += EmfPaintDevicePlus.cpp
	}
} else {
	win32: LIBS += -lgdi32
        unix:  LIBS += /usr/local/lib/libEMF.a
        unix:  INCLUDEPATH += /usr/local/include/libEMF
	SOURCES += EmfEngine.cpp 
	SOURCES += EmfPaintDevice.cpp
}

