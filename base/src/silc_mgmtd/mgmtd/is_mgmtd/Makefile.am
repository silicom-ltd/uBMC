AUTOMAKE_OPTIONS = foreign 1.9

bin_PROGRAMS=is_mgmtd

ACLOCAL_AMFLAGS = -I m4

is_mgmtd_CFLAGS = -I${top_builddir}/inc -I${top_builddir}/mgmtd/inc -I${top_builddir}/../../../silicom/src/is_switchd/include
is_mgmtd_LDADD = ${top_builddir}/mgmtd/src/libsilc_mgmtd_inst.la \
           ${top_builddir}/lib/libsilc_mgmtd.la \
           -lsilc \
           -lcrypt \
           -ldl \
           -lpthread

is_mgmtd_SOURCES=is_mgmtd_memdb_init.c \
         funcs/is_mgmtd_system_upgrade.c \
         funcs/is_mgmtd_system_mgmt_config.c \
         funcs/is_mgmtd_system_service_config.c \
         funcs/is_mgmtd_system_misc_config.c \
         funcs/is_mgmtd_system_status.c \
         funcs/is_mgmtd_system_action.c \
         funcs/is_mgmtd_radius_upgrade.c \
         funcs/is_mgmtd_radius_static_config.c \
         funcs/is_mgmtd_radius_server_config.c \
         funcs/is_mgmtd_tacacs_upgrade.c \
         funcs/is_mgmtd_tacacs_static_config.c \
         funcs/is_mgmtd_tacacs_server_config.c \
         funcs/is_mgmtd_unix_upgrade.c \
         funcs/is_mgmtd_unix_static_config.c \
         funcs/is_mgmtd_unix_user_config.c \
         funcs/is_mgmtd_aaa_action.c \
         funcs/is_mgmtd_snmp_upgrade.c \
         funcs/is_mgmtd_snmp_config.c \
         funcs/is_mgmtd_threshold_config.c \
         funcs/is_mgmtd_snmp_status.c \
         funcs/is_mgmtd_snmp_action.c \
         funcs/is_mgmtd_running_config.c \
         is_mgmtd_main.c
