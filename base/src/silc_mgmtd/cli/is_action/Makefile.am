AUTOMAKE_OPTIONS = foreign 1.9

bin_PROGRAMS=is_action

ACLOCAL_AMFLAGS = -I m4

is_action_CFLAGS = -I${top_builddir}/inc -I${top_builddir}/cli/inc 
is_action_LDADD = ${top_builddir}/cli/src/libsilc_cli_inst.la \
                   ${top_builddir}/lib/libsilc_mgmtd.la \
                   -lsilc \
                   -ledit -lncurses -lpthread

is_action_SOURCES = is_action_main.c
