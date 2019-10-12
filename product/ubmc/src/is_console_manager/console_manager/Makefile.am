AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = is_console_manager 

ACLOCAL_AMFLAGS = -I m4
is_console_manager_CFLAGS = \
					-I${top_builddir}/../../../silicom/src/is_switchd/include

is_console_manager_SOURCES = \
					is_con_mgr.c \
					is_con_mgr_mgmtd.c \
					is_con_mgr_config.c

is_console_manager_LDADD = \
				-lsilc_mgmtd \
                -lsilc \
                -lnsl \
                -lpthread -lrt -lm

