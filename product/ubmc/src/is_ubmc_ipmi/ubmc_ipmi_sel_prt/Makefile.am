AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ubmc_ipmi_sel_prt

ubmc_ipmi_sel_prt_CFLAGS = ${common_cflags} -I../include

ubmc_ipmi_sel_prt_SOURCES = \
		ubmc_ipmi_sel_prt.c 


ubmc_ipmi_sel_prt_LDADD = \
                         -lpthread\
                         ../lib/libubmc_ipmi_sel.la
						 


