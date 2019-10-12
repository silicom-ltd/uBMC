dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(is_mgmtd, 1.0)

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

if test "x" == "x$CFLAGS"
then
CFLAGS=" -O0 -g -Wall -Werror -D_GNU_SOURCE -D_XOPEN_SOURCE=500 -fPIC"
else
CFLAGS+=" -g -Wall -Werror -D_GNU_SOURCE -D_XOPEN_SOURCE=500 -fPIC"
fi

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_SUBST([CFLAG_SILC_COMMON], ['-I$(top_srcdir)/include'])

AC_MSG_NOTICE(using CFLAGS as "$CFLAGS")
AC_MSG_NOTICE([  Silicom Common Include = ${CFLAG_SILC_COMMON}])

AC_CONFIG_FILES([Makefile 
                 inc/Makefile 
                 lib/Makefile 
                 mgmtd/Makefile 
                 mgmtd/inc/Makefile 
                 mgmtd/src/Makefile
                 mgmtd/is_mgmtd/Makefile
                 cli/Makefile 
                 cli/inc/Makefile 
                 cli/src/Makefile
                 cli/is_cli/Makefile
                 cli/is_upg/Makefile
                 cli/is_action/Makefile
])


AC_OUTPUT

