#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([GCalendar], [0.1], [holger.langenau@s2004.tu-chemnitz.de])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.24])
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.24])

LIBS="$LIBS $GLIB_LIBS $GTK_LIBS"
CFLAGS="$CFLAGS $GLIB_CFLAGS $GTK_CFLAGS"

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 data/Makefile])
AC_OUTPUT
