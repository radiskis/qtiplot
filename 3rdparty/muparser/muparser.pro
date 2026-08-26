TEMPLATE = lib
TARGET = muparser
CONFIG += staticlib

INCLUDEPATH += include
DEFINES += MUPARSER_STATIC

HEADERS += include/muParser.h \
           include/muParserBase.h \
           include/muParserBytecode.h \
           include/muParserCallback.h \
           include/muParserDLL.h \
           include/muParserDef.h \
           include/muParserError.h \
           include/muParserFixes.h \
           include/muParserInt.h \
           include/muParserTemplateMagic.h \
           include/muParserTest.h \
           include/muParserToken.h \
           include/muParserTokenReader.h

SOURCES += src/muParser.cpp \
           src/muParserBase.cpp \
           src/muParserBytecode.cpp \
           src/muParserCallback.cpp \
           src/muParserDLL.cpp \
           src/muParserError.cpp \
           src/muParserInt.cpp \
           src/muParserTest.cpp \
           src/muParserTokenReader.cpp
