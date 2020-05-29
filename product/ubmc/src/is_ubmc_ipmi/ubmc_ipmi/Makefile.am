AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ubmc_ipmi 

ubmc_ipmi_CFLAGS = ${common_cflags} -I../include

ubmc_ipmi_SOURCES = \
	ubmc_ipmi.c \
	ubmc_ipmi_hst_pwr_monitor.c\
	ubmc_shm_state.c \
	ubmc_ipmi_pm_l.c \
	ubmc_ipmi_pm_sm.c \
	ubmc_ipmi_pm_xs.c \
	ubmc_ipmi_pm_skyd.c \
	ubmc_ipmi_lan.c \
	ubmc_ipmi_check_st.c 
#	ubmc_ipmi_sel.c

ubmc_ipmi_LDADD = \
	   		-lpthread\
    	   ../lib/libubmc_ipmi_sel.la
			

