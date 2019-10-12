AC_INIT([console_manager], [0.1], [george@thoughtbot.com])
CXXFLAGS="-g -Wall -Werror "

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([stdio.h errno.h fcntl.h string.h termios.h sys/types.h unistd.h poll.h limits.h sys/socket.h sys/un.h])

AC_CONFIG_FILES([Makefile
                console_manager/Makefile
                serial_host/Makefile
                serial_socket_client/Makefile
                ubmc_system_ctrl/Makefile
                eeprom_op/Makefile
])

AC_OUTPUT
