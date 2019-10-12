AUTOMAKE_OPTIONS = foreign 1.9

bin_PROGRAMS=is_cli

ACLOCAL_AMFLAGS = -I m4

is_cli_CFLAGS = -I${top_builddir}/inc -I${top_builddir}/cli/inc 
is_cli_LDADD = ${top_builddir}/cli/src/libsilc_cli_inst.la \
           ${top_builddir}/lib/libsilc_mgmtd.la \
           -lsilc \
           -lcrypt \
           -ledit \
           -lncurses -ldl -lpthread

is_cli_SOURCES = is_cli_init.c is_cli_main.c \
             cmd_funcs/is_cli_cmd_cli_config.c \
             cmd_funcs/is_cli_cmd_enable_go.c \
             cmd_funcs/is_cli_cmd_exit_go.c \
             cmd_funcs/is_cli_cmd_help_do.c \
             cmd_funcs/is_cli_cmd_show_do.c \
             cmd_funcs/is_cli_cmd_clear_go.c \
             cmd_funcs/is_cli_cmd_config_go.c \
             cmd_funcs/is_cli_cmd_clock_config.c \
             cmd_funcs/is_cli_cmd_disable_go.c \
             cmd_funcs/is_cli_cmd_reload_go.c \
             cmd_funcs/is_cli_cmd_write_go.c \
             cmd_funcs/is_cli_cmd_image_config.c \
             cmd_funcs/is_cli_cmd_service_config.c \
             cmd_funcs/is_cli_cmd_config_mgmt.c \
             cmd_funcs/is_cli_cmd_debug_do.c \
             cmd_funcs/is_cli_cmd_dump_mgmt.c \
             cmd_funcs/is_cli_cmd_config_exit.c \
             cmd_funcs/is_cli_cmd_misc_config.c \
             cmd_funcs/is_cli_cmd_mgmt_config.c \
             cmd_funcs/is_cli_cmd_no_do.c \
             cmd_funcs/is_cli_cmd_ntp_config.c \
             cmd_funcs/is_cli_cmd_aaa_config.c \
             cmd_funcs/is_cli_cmd_session_config.c \
             cmd_funcs/is_cli_cmd_snmp_config.c \
             cmd_funcs/is_cli_cmd_upgrade_do.c \
             cmd_funcs/is_cli_cmd_shellcmd_do.c \
             dync_funcs/is_cli_dync_get_image_list.c \
             dync_funcs/is_cli_dync_get_tz_group.c \
             dync_funcs/is_cli_dync_get_timezone.c \
             dync_funcs/is_cli_dync_get_config_list.c \
             dync_funcs/is_cli_dync_get_dump_list.c \
             dync_funcs/is_cli_dync_get_inf_list.c \
             dync_funcs/is_cli_dync_get_netdev_list.c \
             dync_funcs/is_cli_dync_get_vrf_list.c \
             dync_funcs/is_cli_dync_get_net_process_list.c \
             dync_funcs/is_cli_dync_get_ipsec_conn.c \
             dync_funcs/is_cli_dync_get_ipsec_auth_type.c \
             dync_funcs/is_cli_dync_get_cert_list.c \
             dync_funcs/is_cli_dync_get_key_list.c \
             dync_funcs/is_cli_dync_get_mgmt_dns.c \
             dync_funcs/is_cli_dync_get_addr_list.c \
             dync_funcs/is_cli_dync_get_ipsec_secret.c \
             dync_funcs/is_cli_dync_get_ipsec_ca.c \
             dync_funcs/is_cli_dync_get_mgmt_permitted.c \
             dync_funcs/is_cli_dync_get_vrf_process_list.c \
             dync_funcs/is_cli_dync_get_route_list.c \
             dync_funcs/is_cli_dync_get_iptables_rule_list.c \
             dync_funcs/is_cli_dync_get_log_server.c \
             dync_funcs/is_cli_dync_get_ntp_server.c \
             dync_funcs/is_cli_dync_get_agent_commu.c \
             dync_funcs/is_cli_dync_get_trap_host.c \
             dync_funcs/is_cli_dync_get_agent_user.c \
             dync_funcs/is_cli_dync_get_radius_srv.c \
             dync_funcs/is_cli_dync_get_tacacs_srv.c \
             dync_funcs/is_cli_dync_get_user_account.c \
             is_cli_user_init.c
