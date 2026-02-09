
# 3rdparty CMakeLists.txt

# MuParser (Has valid CMakeLists.txt)
set(ENABLE_WIDE_CHAR ON CACHE BOOL "Enable wide char support" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared/static libs" FORCE)
set(ENABLE_OPENMP OFF CACHE BOOL "Enable OpenMP" FORCE)
add_subdirectory(muparser)


# QwtPlot3D (Has valid CMakeLists.txt)
# QwtPlot3D (Has valid CMakeLists.txt)
add_subdirectory(qwtplot3d)


# Qwt (Building from source for Qt 6 migration)
add_subdirectory(qwt)
# add_library(qwt SHARED IMPORTED GLOBAL)
# set_target_properties(qwt PROPERTIES
#     IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/qwt/lib/libqwt.a"
#     IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/qwt/lib/qwt.dll"
#     INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/qwt/src"
#     INTERFACE_COMPILE_DEFINITIONS "QWT_DLL"
# )


# GSL (Building from source)
add_subdirectory(gsl)

# ALGLIB (Building from source)
add_subdirectory(alglib)

# TAMUANOVA (Building from source)
# add_subdirectory(tamuanova)

# ZLIB (Building from source)
# add_subdirectory(zlib)

