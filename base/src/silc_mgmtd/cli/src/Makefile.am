# What to build: static and shared versions of the library.
lib_LTLIBRARIES = libsilc_cli_inst.la


# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
libsilc_cli_inst_la_CFLAGS = -I${top_builddir}/inc -I${top_builddir}/cli/inc  

libsilc_cli_inst_la_SOURCES = \
                    silc_cli_terminal.c \
                    silc_cli_types.c \
                    silc_cli_common.c \
		            silc_cli_readline.c \
		            silc_cli_error.c


