# What to build: static and shared versions of the library.
lib_LTLIBRARIES = libsilc_mgmtd.la


# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
libsilc_mgmtd_la_CFLAGS = -I${top_builddir}/inc  

libsilc_mgmtd_la_SOURCES = \
                    silc_mgmtd_lib_err.c \
                    silc_mgmtd_variables.c \
		    silc_mgmtd_connect.c \
		    silc_mgmtd_interface.c


