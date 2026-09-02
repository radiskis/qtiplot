# qtiplot/CMakeLists.txt

# --- Dependencies ---
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Xml OpenGL OpenGLWidgets PrintSupport Concurrent Network)
find_package(ZLIB)
if(TARGET ZLIB::ZLIB)
    set(ZLIB_LIBRARIES ZLIB::ZLIB)
else()
    find_package(Qt6 REQUIRED COMPONENTS ZlibPrivate)
    set(ZLIB_LIBRARIES Qt6::ZlibPrivate)
endif()

# --- Sources ---
# Glob all sources in src/
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE UI_FILES "src/*.ui")

# Exclude redundant/duplicate files and plugins
list(FILTER SOURCES EXCLUDE REGEX "src/core/MultiPeakFitTool")
list(FILTER HEADERS EXCLUDE REGEX "src/core/MultiPeakFitTool")
list(FILTER SOURCES EXCLUDE REGEX "src/plugins")
list(FILTER HEADERS EXCLUDE REGEX "src/plugins")

# ANOVA files are now enabled with tamuanova
# list(FILTER SOURCES EXCLUDE REGEX "src/analysis/Anova.*")
# list(FILTER HEADERS EXCLUDE REGEX "src/analysis/Anova.*")

if(NOT ENABLE_PYTHON)
    list(FILTER SOURCES EXCLUDE REGEX "src/scripting/Python")
    list(FILTER HEADERS EXCLUDE REGEX "src/scripting/Python")
endif()


# Resources
set(RESOURCES "../manual/html/icons/icons.qrc")

# --- SIP generated sources ---
set(SIP_SOURCES "")
if(ENABLE_PYTHON)
    file(GLOB SIP_SOURCES "${PROJECT_BINARY_DIR}/sip_temp/qti/*.cpp" "${PROJECT_BINARY_DIR}/sip_temp/qti/*.c")
endif()

set(PLATFORM_SOURCES "")
if(WIN32)
    list(APPEND PLATFORM_SOURCES "icons/qtiplot.rc")
endif()

# --- Executable ---
add_executable(qtiplot
    ${SOURCES} 
    ${HEADERS} 
    ${UI_FILES} 
    ${RESOURCES}
    ${SIP_SOURCES}
    ${PLATFORM_SOURCES}
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
)
if(ENABLE_PYTHON)
    target_include_directories(qtiplot PRIVATE "${PROJECT_BINARY_DIR}/sip_temp/qti" "${PROJECT_BINARY_DIR}/sip_temp")
endif()
target_include_directories(qtiplot PRIVATE
    # 3rdparty Includes
    "${PROJECT_SOURCE_DIR}/3rdparty/qwt/src"
    "${PROJECT_SOURCE_DIR}/3rdparty/qwtplot3d/include"
    "${PROJECT_SOURCE_DIR}/3rdparty/muparser/include"
    "${PROJECT_SOURCE_DIR}/3rdparty/gsl" 
    "${PROJECT_SOURCE_DIR}/3rdparty/alglib/src"
    # "${PROJECT_SOURCE_DIR}/3rdparty/tamuanova"
    "${PROJECT_SOURCE_DIR}/3rdparty/zlib"
)


# --- Definitions ---
target_compile_definitions(qtiplot PRIVATE 
    NOMINMAX
    UNICODE
    _UNICODE
    SCRIPTING_CONSOLE 
    SCRIPTING_MUPARSER 
    ZLIB_STATIC
)
if(ENABLE_PYTHON)
    target_compile_definitions(qtiplot PRIVATE SCRIPTING_PYTHON)
endif()
target_compile_definitions(qtiplot PRIVATE
    MUPARSER_STATIC
    # QTIPLOT_STATIC_BUILD # Disabled to allow plugin linkage
    # QWT_DLL # Removed: Linking against static Qwt
    # Legacy defines from .pro

    QTI_ROOT
)
if(QTIPLOT_LEDGER OR QTIPLOT_SANITIZE)
    target_compile_definitions(qtiplot PRIVATE QTIPLOT_LEDGER)
endif()

# --- Linking ---
target_link_libraries(qtiplot PRIVATE
    Qt6::Widgets Qt6::Svg Qt6::Xml Qt6::OpenGL Qt6::OpenGLWidgets Qt6::PrintSupport Qt6::Concurrent Qt6::Network
)
if(ENABLE_PYTHON)
    target_link_libraries(qtiplot PRIVATE Python3::Python)
    target_sources(qtiplot PRIVATE 
        src/scripting/PythonScript.cpp
        src/scripting/PythonScripting.cpp
        src/scripting/PythonSyntaxHighlighter.cpp
        src/scripting/PythonScript.h
        src/scripting/PythonScripting.h
        src/scripting/PythonSyntaxHighlighter.h
    )
endif()
target_link_libraries(qtiplot PRIVATE
    ${ZLIB_LIBRARIES}
    
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

# --- Definitions ---
add_definitions(-DHAVE_TAMUANOVA)

# --- Python Config Files Deployment ---
if(ENABLE_PYTHON)
    file(COPY 
        "${CMAKE_CURRENT_SOURCE_DIR}/qtiplotrc.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/qti_wordlist.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/qtiUtil.py"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
    )
    install(FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/qtiplotrc.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/qti_wordlist.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/qtiUtil.py"
        DESTINATION bin
    )
endif()

# Install deployed runtime dependencies for standalone distribution
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" 
    DESTINATION bin 
    FILES_MATCHING 
    PATTERN "*.dll" 
    PATTERN "CMakeFiles" EXCLUDE 
    PATTERN "src" EXCLUDE 
    PATTERN "qtiplot_autogen" EXCLUDE
)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/platforms" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imageformats" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/iconengines" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/styles" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tls" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generic" DESTINATION bin OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/networkinformation" DESTINATION bin OPTIONAL)



set_target_properties(qtiplot PROPERTIES
    ENABLE_EXPORTS ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    OUTPUT_NAME "qtiplot"
)
if(MINGW)
    target_link_options(qtiplot PRIVATE "-Wl,--export-all-symbols")
endif()

# --- Plugins ---
if(WIN32)
    add_subdirectory(src/plugins/exportEMF)
endif()
add_subdirectory(src/plugins/OriginPlugin)
