###############################################################
################# Module: Matrix ##############################
###############################################################

INCLUDEPATH += src/matrix/

HEADERS  += src/matrix/ColorMapDialog.h \
            src/matrix/Matrix.h \
            src/matrix/MatrixCommand.h \
            src/matrix/MatrixDialog.h \
            src/matrix/MatrixModel.h \
            src/matrix/MatrixSizeDialog.h \
            src/matrix/MatrixValuesDialog.h \

SOURCES += 	src/matrix/ColorMapDialog.cpp \
            src/matrix/Matrix.cpp \
            src/matrix/MatrixCommand.cpp \
            src/matrix/MatrixDialog.cpp \
            src/matrix/MatrixModel.cpp \
            src/matrix/MatrixSizeDialog.cpp \
            src/matrix/MatrixValuesDialog.cpp \

!isEmpty(ALGLIB_LIBS) {
	HEADERS += src/matrix/MatrixResamplingDialog.h
	SOURCES += src/matrix/MatrixResamplingDialog.cpp
}
