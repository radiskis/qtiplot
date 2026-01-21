###############################################################
##################### SCRIPTING LANGUAGES SECTION #############
###############################################################

INCLUDEPATH += src/scripting/ \
               src/core/ \
               src/plot2D/ \
               src/analysis/ \
               src/table/ \
               src/matrix/

HEADERS  += src/scripting/customevents.h\
            src/scripting/FindReplaceDialog.h\
            src/scripting/MyParser.h\
            src/scripting/Note.h\
            src/scripting/PythonSyntaxHighlighter.h\
            src/scripting/ScriptingEnv.h\
            src/scripting/Script.h\
            src/scripting/ScriptEdit.h\
            src/scripting/ScriptingLangDialog.h\
            src/scripting/ScriptWindow.h\

SOURCES  += src/scripting/FindReplaceDialog.cpp\
            src/scripting/MyParser.cpp\
            src/scripting/Note.cpp\
            src/scripting/PythonSyntaxHighlighter.cpp\
            src/scripting/ScriptingEnv.cpp\
            src/scripting/Script.cpp\
            src/scripting/ScriptEdit.cpp\
            src/scripting/ScriptingLangDialog.cpp\
            src/scripting/ScriptWindow.cpp\

##################### Default: muParser v1.28 #################

contains(SCRIPTING_LANGS, muParser) {
  DEFINES += SCRIPTING_MUPARSER

  HEADERS += src/scripting/muParserScript.h \
             src/scripting/muParserScripting.h \

  SOURCES += src/scripting/muParserScript.cpp \
             src/scripting/muParserScripting.cpp
}

##################### PYTHON + SIP + PyQT #####################

contains(SCRIPTING_LANGS, Python) {

  DEFINES += SCRIPTING_PYTHON

  HEADERS += src/scripting/PythonScript.h\
             src/scripting/PythonScripting.h\

  SOURCES += src/scripting/PythonScript.cpp\
  			 src/scripting/PythonScripting.cpp\

##################### SIP generated files #####################
  
  INCLUDEPATH += $${SIP_DIR}
  include(sip_sources.pri)
}
