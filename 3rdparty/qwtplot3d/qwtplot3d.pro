# pro file for building the makefile for qwtplot3d
#

include (qwtplot3d.pri)

CONFIG          += qt warn_on opengl thread release
MOC_DIR          = tmp
OBJECTS_DIR      = tmp
INCLUDEPATH      = include
DEPENDPATH       = include src
QT              += opengl

#win32:CONFIG    += static
win32:CONFIG    += dll
unix:CONFIG     += staticlib
win32:CONFIG    += exceptions

win32:dll:DEFINES    += QT_DLL QWT3D_DLL QWT3D_MAKEDLL
win32:QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL

# Comment the next line, if you do not want debug message output
#DEFINES -= QT_NO_DEBUG_OUTPUT

linux-g++:TMAKE_CXXFLAGS += -fno-exceptions
unix:VERSION     = 0.3.0

DESTDIR  = lib

OBJECTS_DIR  = $$DESTDIR/tmp
RCC_DIR      = $$DESTDIR/tmp
MOC_DIR      = $$DESTDIR/tmp

DEFINES     += GL2PS_HAVE_LIBPNG
INCLUDEPATH += ../zlib/
INCLUDEPATH += ../libpng/
LIBS        += ../libpng/libpng.a

# install
target.path = lib
INSTALLS += target
