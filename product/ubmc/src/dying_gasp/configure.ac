AC_INIT(dying_gasp, 1.0)
CXXFLAGS="-g -Wall -Werror "

AM_INIT_AUTOMAKE

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CC


# Checks for header files.
AC_CHECK_HEADERS([stdio.h errno.h fcntl.h string.h sys/types.h poll.h limits.h sys/socket.h pthread.h sys/time.h syslog.h ])
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_RANLIB
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_FUNC_MMAP

common_cflags='-I${top_srcdir}/inc'
AC_SUBST(common_cflags)


AC_CONFIG_FILES([Makefile
                src/Makefile
                inc/Makefile
])

AC_CHECK_FUNCS([memset select munmap])

AC_OUTPUT
