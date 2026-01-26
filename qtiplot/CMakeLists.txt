
# qtiplot/CMakeLists.txt

# --- Sources ---
# Glob all sources in src/
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES "${PROJECT_SOURCE_DIR}/3rdparty/zlib/minigzip.c")
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE UI_FILES "src/*.ui")

# Exclude redundant/duplicate files
list(FILTER SOURCES EXCLUDE REGEX "src/core/MultiPeakFitTool")
list(FILTER HEADERS EXCLUDE REGEX "src/core/MultiPeakFitTool")


# Resources
set(RESOURCES "../manual/html/icons/icons.qrc")

# --- SIP generated sources ---
file(GLOB SIP_SOURCES "${PROJECT_SOURCE_DIR}/build/qti/*.cpp" "${PROJECT_SOURCE_DIR}/build/qti/*.c")

# --- Executable ---
add_executable(qtiplot WIN32 
    ${SOURCES} 
    ${HEADERS} 
    ${UI_FILES} 
    ${RESOURCES}
    ${SIP_SOURCES}
)

# --- Include Directories ---
target_include_directories(qtiplot PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plot2D"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plot3D"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/matrix"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/table"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/dialogs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scripting"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plot2D/dialogs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/excel"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/3rdparty/qtcolorpicker/src"
    "${PROJECT_SOURCE_DIR}/build/qti"

    # 3rdparty Includes


    "${PROJECT_SOURCE_DIR}/3rdparty/qwt/src"
    "${PROJECT_SOURCE_DIR}/3rdparty/qwtplot3d-old/include"
    "${PROJECT_SOURCE_DIR}/3rdparty/muparser/include"
    "${PROJECT_SOURCE_DIR}/3rdparty/gsl" 
    "${PROJECT_SOURCE_DIR}/3rdparty/alglib/src"
    "${PROJECT_SOURCE_DIR}/3rdparty/tamuanova"
)


# --- Definitions ---
target_compile_definitions(qtiplot PRIVATE 
    NOMINMAX
    UNICODE
    _UNICODE
    SCRIPTING_CONSOLE 


    SCRIPTING_PYTHON 
    MUPARSER_STATIC
    QTIPLOT_STATIC_BUILD # If we want static? Or maybe NOT.
    QWT_DLL # Assuming we are linking against Qwt DLL
    # Legacy defines from .pro

    QTI_ROOT
)

# --- Linking ---
target_link_libraries(qtiplot PRIVATE
    Qt5::Widgets Qt5::Svg Qt5::Xml Qt5::OpenGL Qt5::PrintSupport Qt5::Concurrent Qt5::Network Qt5::AxContainer

    Python3::Python # Or Python3::Module

    # ZLIB::ZLIB
    # Link ZLIB manually (using Qt's bundled zlib implicitly or system)
    -lz

    
    # 3rdparty Targets
    muparser
    qwtplot3d
    qwt
    gsl
    gslcblas
    alglib
    tamuanova
)


# --- Installation ---
install(TARGETS qtiplot RUNTIME DESTINATION bin)
