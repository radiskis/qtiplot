###############################################################
######################## OriginPlugin #########################
###############################################################
INCLUDEPATH	+=	src/plugins/OriginPlugin

HEADERS		+=  src/plugins/OriginPlugin/OriginPlugin.h

SOURCES		+=  src/plugins/OriginPlugin/OriginPlugin.cpp

include(src/plugins/OriginPlugin/liborigin1/liborigin1.pri)
