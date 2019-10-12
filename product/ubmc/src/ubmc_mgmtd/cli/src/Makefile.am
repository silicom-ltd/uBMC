# What to build: static and shared versions of the library.
AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES = libprod_cli_inst.la

libprod_cli_inst_la_CFLAGS = -I${top_builddir}/inc  

libprod_cli_inst_la_SOURCES = \
                    ubmc_cli_cmd_bmc_config.c \
                    ubmc_cli_cmd_bmc_no.c \
                    ubmc_cli_cmd_bmc_show.c \
                    ubmc_cli_cmd_phonehome_config.c \
                    ubmc_cli_dync_get_cdrom_list.c \
                    ubmc_cli_init.c


