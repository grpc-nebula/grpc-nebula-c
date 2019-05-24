#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(liborientsec_common.a, 1.0)
AC_CONFIG_SRCDIR([orientsec_grpc_properties_tools.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.
AC_PROG_RANLIB                                    
# Checks for header files.
#AC_CHECK_HEADERS([arpa/inet.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname gethostname inet_ntoa memset socket strchr strrchr strstr])

AC_OUTPUT([Makefile])
