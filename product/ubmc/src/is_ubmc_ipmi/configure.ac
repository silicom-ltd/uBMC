dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ubmc_ipmi, 1.0)

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_SUBST([CFLAG_SILC_COMMON], ['-I$(top_srcdir)/include'])

AC_MSG_NOTICE(using CFLAGS as "$CFLAGS")
AC_MSG_NOTICE([  NetPerform Common Include = ${CFLAG_SILC_COMMON}])
SILC_LIBS="-lsilc"
AC_SUBST(SILC_LIBS)
AC_CONFIG_FILES([Makefile 
                 include/Makefile 
                 lib/Makefile 
				 ubmc_ipmi/Makefile
				 ubmc_ipmi_sel_prt/Makefile
				 ubmc_fan_control/Makefile
])


AC_OUTPUT
