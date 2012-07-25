#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Locke],,,,[http://www.humansoftware.com.br])
AC_CONFIG_SRCDIR([.metadata/.plugins/org.eclipse.cdt.make.core/specs.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-llockeapi':
AC_CHECK_LIB([lockeapi], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([strcasecmp])

AC_CONFIG_FILES([Makefile
                 dist/documentation/Makefile
                 documentation/Makefile
                 sample_apps/Makefile
                 sample_apps/demo_http/Makefile
                 sample_apps/demo_socket/Makefile
                 server/Makefile
                 server/api/Makefile
                 server/src/Makefile])
AC_OUTPUT
