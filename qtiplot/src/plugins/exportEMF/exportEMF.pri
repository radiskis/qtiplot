###############################################################
######################### exportEMF ###########################
###############################################################
INCLUDEPATH +=  src/plugins/exportEMF/ \
				src/plugins/exportEMF/3rdparty/EmfEngine/src

HEADERS		+=  src/plugins/exportEMF/exportEMF.h \
				src/plugins/exportEMF/3rdparty/EmfEngine/src/EmfEngine.h
				
SOURCES		+=  src/plugins/exportEMF/exportEMF.cpp \
				src/plugins/exportEMF/3rdparty/EmfEngine/src/EmfEngine.cpp \
				src/plugins/exportEMF/3rdparty/EmfEngine/src/EmfPaintDevice.cpp
				
LIBS		+= -lgdi32
