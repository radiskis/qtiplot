TEMPLATE = lib
CONFIG += staticlib
TARGET = gslcblas
DESTDIR = lib
INCLUDEPATH += . ..
SOURCES += $$files(cblas/*.c)
SOURCES -= $$files(cblas/test*.c)
