###############################################################
################# Origin Import (liborigin) ###################
###############################################################
INCLUDEPATH += src/origin/

HEADERS += src/origin/importOPJ.h
HEADERS += ../3rdparty/liborigin/OriginObj.h
HEADERS += ../3rdparty/liborigin/OriginFile.h
HEADERS += ../3rdparty/liborigin/OriginParser.h
HEADERS += ../3rdparty/liborigin/OriginDefaultParser.h
HEADERS += ../3rdparty/liborigin/Origin750Parser.h

SOURCES += src/origin/importOPJ.cpp
SOURCES += ../3rdparty/liborigin/OriginFile.cpp
SOURCES += ../3rdparty/liborigin/OriginParser.cpp
SOURCES += ../3rdparty/liborigin/OriginDefaultParser.cpp
SOURCES += ../3rdparty/liborigin/Origin750Parser.cpp
