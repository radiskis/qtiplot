
project(qwt CXX)

file(GLOB QWT_SOURCES src/*.cpp)
list(REMOVE_ITEM QWT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/qwt_plot_glcanvas.cpp")
file(GLOB QWT_HEADERS src/*.h)

add_library(qwt STATIC ${QWT_SOURCES} ${QWT_HEADERS})

target_include_directories(qwt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Qwt depends on Qt Widgets and PrintSupport
find_package(Qt6 COMPONENTS Widgets PrintSupport Concurrent OpenGLWidgets Svg REQUIRED)
target_link_libraries(qwt PRIVATE Qt6::Widgets Qt6::PrintSupport Qt6::Concurrent Qt6::OpenGLWidgets Qt6::Svg)

target_compile_definitions(qwt PRIVATE QWT_DLL QWT_MAKEDLL) 
