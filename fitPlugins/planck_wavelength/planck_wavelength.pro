QMAKE_PROJECT_DEPTH = 0
linux-g++-64: libsuff=64

TARGET = planck_wavelength
TEMPLATE = lib
CONFIG += dll
CONFIG -= qt
CONFIG += release
DESTDIR = ../

INSTALLS += target
 
# statically link against GSL in 3rdparty
INCLUDEPATH += ../../3rdparty/gsl/include/
LIBS        += ../../3rdparty/gsl/lib/libgsl.a
LIBS        += ../../3rdparty/gsl/lib/libgslcblas.a

#dynamically link against GSL installed system-wide
#unix:LIBS += -L /usr/lib$${libsuff} -lgsl -lgslcblas

target.path=/usr/lib$${libsuff}/qtiplot/plugins

SOURCES = planck_wavelength.c

