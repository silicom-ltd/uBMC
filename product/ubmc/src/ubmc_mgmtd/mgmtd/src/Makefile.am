# What to build: static and shared versions of the library.
AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES = libprod_mgmtd_inst.la

libprod_mgmtd_inst_la_CFLAGS = -I${top_builddir}/inc  

libprod_mgmtd_inst_la_SOURCES = \
                    ubmc_mgmtd_action.c \
                    ubmc_mgmtd_config.c \
                    ubmc_mgmtd_notify.c \
                    ubmc_mgmtd_status.c \
                    ubmc_mgmtd_upgrade.c \
                    ubmc_mgmtd_memdb_init.c


