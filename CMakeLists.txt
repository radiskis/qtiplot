cmake_minimum_required(VERSION 3.16)

project(qtiplot LANGUAGES C CXX)

option(QTIPLOT_SANITIZE "Build with AddressSanitizer (+UBSan where available)" OFF)

if(QTIPLOT_SANITIZE)
    if(MSVC)
        # /RTC1 is incompatible with ASan and is in CMake's default Debug flags.
        string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
        add_compile_options(/fsanitize=address /Zi)
        # ASan cannot be combined with incremental linking or LTCG.
        add_link_options(/INCREMENTAL:NO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

option(QTIPLOT_LEDGER "Track object lifetimes and assert a clean ledger at project close" ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS /W3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# Find Qt6
find_package(Qt6 COMPONENTS Widgets Svg Xml OpenGL OpenGLWidgets PrintSupport Concurrent Network OPTIONAL_COMPONENTS LinguistTools REQUIRED)



# Find Python
option(ENABLE_PYTHON "Enable Python scripting" ON)
if(ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

# Find ZLIB (system or bundled)
# Find ZLIB (system or bundled)
# find_package(ZLIB REQUIRED) 
# We will handle ZLIB via Qt or system libs manually in qtiplot/CMakeLists.txt


# --- Definitions ---
add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

# Add 3rdparty components
add_subdirectory(3rdparty)

# Add Main Application
add_subdirectory(qtiplot)

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "qtiplot")
set(CPACK_PACKAGE_VENDOR "QtiPlot Project")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "9")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QtiPlot - Data Analysis and Scientific Plotting")
set(CPACK_GENERATOR "ZIP")
include(CPack)

# --- Testing (CTest) ---
enable_testing()
add_test(NAME python_suite
         COMMAND qtiplot -X "${CMAKE_SOURCE_DIR}/tests/pytest_runner.py")
set_tests_properties(python_suite PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:abort_on_error=1;LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tests/lsan.supp")

add_test(NAME gate_curve_ledger
         COMMAND qtiplot -X "${CMAKE_SOURCE_DIR}/tests/gate_curve_ledger.py")
