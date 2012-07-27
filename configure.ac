#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Locke],[0.1],,,[http://www.humansoftware.com.br])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([.metadata/.plugins/org.eclipse.cdt.make.core/specs.c])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
AM_PROG_AR

AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CHECK_PROG([PKGCONFIG],[pkg-config], yes)
if test -z "$PKGCONFIG"; then
   AC_MSG_ERROR([pkg-config required])
fi

AC_CHECK_PROG([DOXYGEN], [doxygen], yes)
if test -z "$DOXYGEN"; then
   AC_MSG_WARN([[Doxygen not found - continuing without Doxygen support]])
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-llockeapi':
AC_CHECK_LIB([lockeapi], [main])
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0])
PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.0.0])
PKG_CHECK_MODULES([GMODULE], [gmodule-2.0 >= 2.0.0])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([strcasecmp])

AC_CONFIG_FILES([Makefile
		 server/Makefile
		 server/api/Makefile
		 server/src/Makefile])

# AC_CONFIG_FILES([Makefile
#                  sample_apps/Makefile
#                  sample_apps/demo_http/Makefile
#                  sample_apps/demo_socket/Makefile
#                  server/Makefile
#                  server/api/Makefile
#                  server/src/Makefile])
#
# Only Generate Makefile for Doxygen if the user has it on their system
# AM_CONDITIONAL([HAVE_DOXYGEN],
#                [test -n "$DOXYGEN"])
# AM_COND_IF([HAVE_DOXYGEN],
#         [AC_CONFIG_FILES([documentation/Doxyfile
#                           documentation/Makefile])])

AC_OUTPUT
