###################### HEADERS ##############################################
INCLUDEPATH += src/core/

HEADERS  += src/core/ApplicationWindow.h \
			src/core/ConfigDialog.h \
			src/core/CreateBinMatrixDialog.h \
			src/core/CustomActionDialog.h \
			src/core/Folder.h\
			src/core/FindDialog.h\
			src/core/ImportASCIIDialog.h \
			src/core/ImportExportPlugin.h \
			src/core/MdiSubWindow.h \
			src/core/OpenProjectDialog.h\
			src/core/PlotWizard.h \
			src/core/QtiPlotApplication.h \
			src/core/RenameWindowDialog.h \
			src/core/globals.h\


###################### SOURCES ##############################################

!contains(CONFIG, BrowserPlugin){
	SOURCES  += src/core/main.cpp
}

SOURCES  += src/core/ApplicationWindow.cpp \
			src/core/ConfigDialog.cpp \
			src/core/CreateBinMatrixDialog.cpp \
			src/core/CustomActionDialog.cpp \
			src/core/Folder.cpp\
			src/core/FindDialog.cpp\
			src/core/ImportASCIIDialog.cpp \
			src/core/MdiSubWindow.cpp \
			src/core/OpenProjectDialog.cpp\
			src/core/PlotWizard.cpp \
			src/core/QtiPlotApplication.cpp \
			src/core/RenameWindowDialog.cpp \
