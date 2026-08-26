TEMPLATE = lib
TARGET = png
CONFIG += staticlib
INCLUDEPATH += .

# Assuming zlib headers are in system path or provided by -lz
# If not, we might need ZLIB_INCLUDEPATH

HEADERS += png.h pngconf.h pnglibconf.h pngdebug.h pnginfo.h pngpriv.h pngstruct.h
SOURCES += png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c \
           pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c \
           pngwio.c pngwrite.c pngwtran.c pngwutil.c
