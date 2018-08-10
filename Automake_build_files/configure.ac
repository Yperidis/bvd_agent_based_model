#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([include/AdvancedOutput.h])
AC_CONFIG_HEADERS([config.h])

dnl Most of these things are boiler plate (from autoscan);
dnl but here's some of my stuff
dnl BEGIN Pascal's BLOCK OF STUFF

AM_INIT_AUTOMAKE([1.9 foreign])


dnl END Pascal's BLOCK OF STUFF


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lgsl':
AC_CHECK_LIB([gsl], [main])
# FIXME: Replace `main' with a function in `-lgslcblas':
AC_CHECK_LIB([gslcblas], [main])
# FIXME: Replace `main' with a function in `-lhdf5':
AC_CHECK_LIB([hdf5], [main])
# FIXME: Replace `main' with a function in `-lhdf5_hl':
AC_CHECK_LIB([hdf5_hl], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([projectImports/inih])
AC_OUTPUT
