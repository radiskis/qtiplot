
# 3rdparty CMakeLists.txt

# MuParser (Has valid CMakeLists.txt)
set(ENABLE_WIDE_CHAR ON CACHE BOOL "Enable wide char support" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared/static libs" FORCE)
set(ENABLE_OPENMP OFF CACHE BOOL "Enable OpenMP" FORCE)
add_subdirectory(muparser)


# QwtPlot3D (Has valid CMakeLists.txt)
# QwtPlot3D (Has valid CMakeLists.txt)
add_subdirectory(qwtplot3d-old)


# Qwt (Building from source for Qt 6 migration)
add_subdirectory(qwt)
# add_library(qwt SHARED IMPORTED GLOBAL)
# set_target_properties(qwt PROPERTIES
#     IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/qwt/lib/libqwt.a"
#     IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/qwt/lib/qwt.dll"
#     INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/qwt/src"
#     INTERFACE_COMPILE_DEFINITIONS "QWT_DLL"
# )


# GSL (Imported)
add_library(gsl STATIC IMPORTED GLOBAL)
set_target_properties(gsl PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/gsl/lib/libgsl.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/gsl"
)

add_library(gslcblas STATIC IMPORTED GLOBAL)
set_target_properties(gslcblas PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/gsl/lib/libgslcblas.a"
)

# ALGLIB (Imported)
add_library(alglib STATIC IMPORTED GLOBAL)
set_target_properties(alglib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/alglib/libalglib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/alglib/src"
)

# TAMUANOVA (Imported)
add_library(tamuanova STATIC IMPORTED GLOBAL)
set_target_properties(tamuanova PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/tamuanova/libtamuanova.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/tamuanova"
)
