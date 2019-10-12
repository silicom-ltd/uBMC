# What to build: static and shared versions of the library.
lib_LTLIBRARIES = libsilc_mgmtd_inst.la


# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
libsilc_mgmtd_inst_la_CFLAGS = -I${top_builddir}/inc -I${top_builddir}/mgmtd/inc  

libsilc_mgmtd_inst_la_SOURCES = \
                    silc_mgmtd_config.c \
                    silc_mgmtd_memdb.c \
		            silc_mgmtd_error.c \
		            silc_mgmtd_operation.c \
                            silc_mgmtd_vnode.c \
		            silc_mgmtd_daemon.c


