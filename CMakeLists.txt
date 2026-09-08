cmake_minimum_required(VERSION 3.16)

project(qtiplot LANGUAGES C CXX)

option(QTIPLOT_SANITIZE "Build with AddressSanitizer (+UBSan where available)" OFF)

if(QTIPLOT_SANITIZE)
    if(MSVC)
        # /RTC1 is incompatible with ASan and is in CMake's default Debug flags.
        string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
        set(QTIPLOT_SANITIZE_COMPILE_OPTIONS /fsanitize=address /Zi /D_DISABLE_VECTOR_ANNOTATION /D_DISABLE_STRING_ANNOTATION)
        # ASan cannot be combined with incremental linking or LTCG.
        set(QTIPLOT_SANITIZE_LINK_OPTIONS /INCREMENTAL:NO /DEBUG)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    else()
        set(QTIPLOT_SANITIZE_COMPILE_OPTIONS -fsanitize=address,undefined -fno-omit-frame-pointer -g)
        set(QTIPLOT_SANITIZE_LINK_OPTIONS -fsanitize=address,undefined)
    endif()
    add_compile_options(${QTIPLOT_SANITIZE_COMPILE_OPTIONS})
    add_link_options(${QTIPLOT_SANITIZE_LINK_OPTIONS})
endif()

option(QTIPLOT_LEDGER "Track object lifetimes and assert a clean ledger at project close" ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

add_executable(test_object_registry
    tests/test_object_registry.cpp
    qtiplot/src/core/ObjectRegistry.cpp
)
set_target_properties(test_object_registry PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qtiplot"
)
target_include_directories(test_object_registry PRIVATE
    qtiplot/src/core
)
target_link_libraries(test_object_registry PRIVATE
    Qt6::Core
)
add_test(NAME test_object_registry COMMAND test_object_registry)

add_test(NAME python_suite
         COMMAND qtiplot -X "${CMAKE_SOURCE_DIR}/tests/pytest_runner.py")

add_test(NAME gate_curve_ledger
         COMMAND qtiplot -X "${CMAKE_SOURCE_DIR}/tests/gate_curve_ledger.py")
if(NOT MSVC)
    set_tests_properties(python_suite PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:abort_on_error=1;LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tests/lsan.supp")
    set_tests_properties(gate_curve_ledger PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:abort_on_error=1;LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tests/lsan.supp")
endif()
