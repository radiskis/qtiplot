QTI_ROOT = ../..
!include( $$QTI_ROOT/build.conf ) {
  message( "You need a build.conf with local settings!" )
}

QMAKE_PROJECT_DEPTH = 0
linux-g++-64: libsuff=64

TARGET = planck_wavelength
TEMPLATE = lib
CONFIG += dll
CONFIG -= qt
CONFIG += release
DESTDIR = ../

INSTALLS += target
 
INCLUDEPATH += $$GSL_INCLUDEPATH
LIBS        += $$GSL_LIBS

target.path=/usr/lib$${libsuff}/qtiplot/plugins

SOURCES = planck_wavelength.c

# At the very end: add global include- and lib path
INCLUDEPATH += $$SYS_INCLUDEPATH
LIBS += $$SYS_LIBS
