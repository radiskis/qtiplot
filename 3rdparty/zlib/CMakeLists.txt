
project(zlib C)

file(GLOB ZLIB_SOURCES *.c)
file(GLOB ZLIB_HEADERS *.h)

# Exclude minigzip.c and example.c as they are executables/tests
list(REMOVE_ITEM ZLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/minigzip.c")
list(REMOVE_ITEM ZLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/example.c")

add_library(zlib STATIC ${ZLIB_SOURCES} ${ZLIB_HEADERS})
target_compile_definitions(zlib PUBLIC ZLIB_STATIC Z_PREFIX)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
