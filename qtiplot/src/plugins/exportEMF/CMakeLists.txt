project(exportEMF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport Svg Xml OpenGL WinExtras)

set(EXPORTEMF_SOURCES
    exportEMF.cpp
    3rdparty/EmfEngine/src/EmfEngine.cpp
    3rdparty/EmfEngine/src/EmfPaintDevice.cpp
)

add_library(exportEMF MODULE ${EXPORTEMF_SOURCES})

target_include_directories(exportEMF PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/EmfEngine/src
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
)

target_link_libraries(exportEMF PRIVATE
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Svg
    Qt5::Xml
    Qt5::OpenGL
    Qt5::WinExtras
    qtiplot
    qwt
    gdi32
)

target_compile_definitions(exportEMF PRIVATE 
)

set_target_properties(exportEMF PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qtiplot/plugins"
)
