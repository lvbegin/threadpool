#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([threadpool], [1.0])
AC_CONFIG_SRCDIR([test/threadpoolTest.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_INIT_AUTOMAKE
# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create)
# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_LANG([C++])
CXXFLAGS='-Wall -std=c++14'
# Checks for library functions.

AC_OUTPUT
