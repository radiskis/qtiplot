project(OriginPlugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Svg Xml OpenGL)

set(ORIGINPLUGIN_SOURCES
    OriginPlugin.cpp
    liborigin1/importOPJ.cpp
    liborigin2/OriginFile.cpp
    liborigin2/OriginParser.cpp
    liborigin2/Origin410Parser.cpp
    liborigin2/Origin500Parser.cpp
    liborigin2/Origin610Parser.cpp
    liborigin2/Origin700Parser.cpp
    liborigin2/Origin750Parser.cpp
    liborigin2/Origin800Parser.cpp
    liborigin2/Origin810Parser.cpp
    liborigin2/Origin850Parser.cpp
)

add_library(OriginPlugin MODULE ${ORIGINPLUGIN_SOURCES})

target_include_directories(OriginPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/liborigin1
    ${CMAKE_CURRENT_SOURCE_DIR}/liborigin2
    ${CMAKE_CURRENT_SOURCE_DIR}/tree.phi-sci.com
    ${CMAKE_SOURCE_DIR}/qtiplot/src
    ${CMAKE_SOURCE_DIR}/qtiplot/src/core
    ${CMAKE_SOURCE_DIR}/qtiplot/src/table
    ${CMAKE_SOURCE_DIR}/qtiplot/src/matrix
    ${CMAKE_SOURCE_DIR}/qtiplot/src/plot2D
    ${CMAKE_SOURCE_DIR}/qtiplot/src/plot2D/dialogs
    ${CMAKE_SOURCE_DIR}/qtiplot/src/plot3D
    ${CMAKE_SOURCE_DIR}/qtiplot/src/analysis
    ${CMAKE_SOURCE_DIR}/qtiplot/src/analysis/dialogs
    ${CMAKE_SOURCE_DIR}/qtiplot/src/scripting
    ${CMAKE_SOURCE_DIR}/qtiplot/src/lib/include
    ${CMAKE_SOURCE_DIR}/3rdparty/qwt/src
    ${CMAKE_SOURCE_DIR}/3rdparty/qwtplot3d-old/include
    ${CMAKE_SOURCE_DIR}/3rdparty/gsl
    ${CMAKE_SOURCE_DIR}/3rdparty/muparser/include
    ${CMAKE_SOURCE_DIR}/3rdparty
)

target_compile_definitions(OriginPlugin PRIVATE 
    NO_LOG_FILE
    UNICODE
    _UNICODE
)

target_link_libraries(OriginPlugin PRIVATE
    Qt5::Widgets
    Qt5::Svg
    Qt5::Xml
    Qt5::OpenGL
    qtiplot
    qwt
)

set_target_properties(OriginPlugin PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
