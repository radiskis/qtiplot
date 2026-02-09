
project(alglib CXX)

file(GLOB ALGLIB_SOURCES src/*.cpp)
file(GLOB ALGLIB_HEADERS src/*.h)

add_library(alglib STATIC ${ALGLIB_SOURCES} ${ALGLIB_HEADERS})

target_include_directories(alglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
