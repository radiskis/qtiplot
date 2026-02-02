cmake_minimum_required(VERSION 3.16)

project(qtiplot LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
endif()

# Find Qt6
find_package(Qt6 COMPONENTS Widgets Svg Xml OpenGL PrintSupport Concurrent Network REQUIRED)



# Find Python
option(ENABLE_PYTHON "Enable Python scripting" ON)
if(ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

# Find ZLIB (system or bundled)
# Find ZLIB (system or bundled)
# find_package(ZLIB REQUIRED) 
# We will handle ZLIB via Qt or system libs manually in qtiplot/CMakeLists.txt


# Add 3rdparty components
add_subdirectory(3rdparty)

# Add Main Application
add_subdirectory(qtiplot)
