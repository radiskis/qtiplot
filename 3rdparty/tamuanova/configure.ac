dnl Process this file with autoconf to produce a configure script.

AC_INIT([tamu_anova],[0.2])
AC_CONFIG_SRCDIR(tamu_anova.h)

AM_INIT_AUTOMAKE([gnu no-dependencies])
AC_CONFIG_HEADER([config.h])

AC_PROG_MAKE_SET

dnl Check for which system.
AC_CANONICAL_HOST

dnl Checks for programs.
AC_LANG(C)
AC_PROG_CC
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
