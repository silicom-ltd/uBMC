AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ubmc_fan_control

ubmc_fan_control_CFLAGS = ${common_cflags} -I../include

ubmc_fan_control_SOURCES = \
		ubmc_fan_control.c 


ubmc_fan_control_LDADD = \
                         -lpthread\
                         ../lib/libubmc_ipmi_sel.la
						 


