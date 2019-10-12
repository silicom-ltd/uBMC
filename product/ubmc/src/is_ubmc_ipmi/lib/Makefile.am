# What to build: static and shared versions of the library.
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES = libubmc_ipmi_sel.la
libubmc_ipmi_sel_la_LIBADD = \
           			@SILC_LIBS@ 
libubmc_ipmi_sel_la_CFLAGS = -I${top_builddir}/include
libubmc_ipmi_sel_la_SOURCES = \
                    ubmc_ipmi_sel.c 



