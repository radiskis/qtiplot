QTI_ROOT = ../..
!include( $$QTI_ROOT/build.conf ) {
  message( "You need a build.conf with local settings!" )
}

QMAKE_PROJECT_DEPTH = 0
linux-g++-64: libsuff=64

TARGET            = explin
TEMPLATE          = lib

CONFIG           += thread
CONFIG           += warn_on 
CONFIG           += release
CONFIG           += dll 

DESTDIR           = ../
 
INCLUDEPATH += $$GSL_INCLUDEPATH
LIBS        += $$GSL_LIBS

target.path=/usr/lib$${libsuff}/qtiplot/plugins
INSTALLS += target

SOURCES += explin.c

# At the very end: add global include- and lib path
INCLUDEPATH += $$SYS_INCLUDEPATH
LIBS += $$SYS_LIBS
