###############################################################
################# Origin Import (liborigin) ###################
###############################################################
INCLUDEPATH	+=	src/plugins/OriginPlugin/liborigin1 \
				src/plugins/OriginPlugin/liborigin2 \
				src/plugins/OriginPlugin/tree.phi-sci.com \
				../3rdparty/boost

HEADERS		+=	src/plugins/OriginPlugin/liborigin1/importOPJ.h \
				src/plugins/OriginPlugin/liborigin2/OriginObj.h \
				src/plugins/OriginPlugin/liborigin2/OriginFile.h \
				src/plugins/OriginPlugin/liborigin2/OriginParser.h \
				src/plugins/OriginPlugin/liborigin2/Origin410Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin500Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin610Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin700Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin750Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin800Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin810Parser.h \
				src/plugins/OriginPlugin/liborigin2/Origin850Parser.h

SOURCES		+=	src/plugins/OriginPlugin/liborigin1/importOPJ.cpp \
				src/plugins/OriginPlugin/liborigin2/OriginFile.cpp \
				src/plugins/OriginPlugin/liborigin2/OriginParser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin410Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin500Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin610Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin700Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin750Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin800Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin810Parser.cpp \
				src/plugins/OriginPlugin/liborigin2/Origin850Parser.cpp

DEFINES		+=	NO_LOG_FILE
