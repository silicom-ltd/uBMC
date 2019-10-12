/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_cli_types.h"


extern int is_cli_init(); 
extern int is_cli_user_init();

/* declare functions */ 

extern int is_cli_cmd_cli_config(silc_list* p_token_list); 

extern int is_cli_cmd_enable_go(silc_list* p_token_list); 

extern int is_cli_cmd_exit_go(silc_list* p_token_list); 

extern int is_cli_cmd_help_do(silc_list* p_token_list); 

extern int is_cli_cmd_show_do(silc_list* p_token_list); 

extern int is_cli_cmd_clear_go(silc_list* p_token_list); 

extern int is_cli_cmd_config_go(silc_list* p_token_list); 

extern int is_cli_cmd_clock_config(silc_list* p_token_list); 

extern int is_cli_cmd_disable_go(silc_list* p_token_list); 

extern int is_cli_cmd_reload_go(silc_list* p_token_list); 

extern int is_cli_cmd_write_go(silc_list* p_token_list); 

extern int is_cli_cmd_bypass_config(silc_list* p_token_list); 

extern int is_cli_cmd_service_config(silc_list* p_token_list); 

extern int is_cli_cmd_image_config(silc_list* p_token_list);

extern int is_cli_cmd_config_mgmt(silc_list* p_token_list); 

extern int is_cli_cmd_debug_do(silc_list* p_token_list); 

extern int is_cli_cmd_dump_mgmt(silc_list* p_token_list); 

extern int is_cli_cmd_config_exit(silc_list* p_token_list); 

extern int is_cli_cmd_misc_config(silc_list* p_token_list); 

extern int is_cli_cmd_mgmt_config(silc_list* p_token_list); 

extern int is_cli_cmd_manufacture(silc_list* p_token_list); 

extern int is_cli_cmd_no_do(silc_list* p_token_list); 

extern int is_cli_cmd_ntp_config(silc_list* p_token_list); 

extern int is_cli_cmd_aaa_config(silc_list* p_token_list); 

extern int is_cli_cmd_session_config(silc_list* p_token_list); 

extern int is_cli_cmd_snmp_config(silc_list* p_token_list); 

extern int is_cli_cmd_test_go(silc_list* p_token_list); 

extern int is_cli_cmd_upgrade_do(silc_list* p_token_list); 

extern int is_cli_cmd_shellcmd_do(silc_list* p_token_list); 


extern int is_cli_dync_get_bypass_modules(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_bypass_segments(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_bypass_ports(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_hb_pkt_type(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_image_list(void* data, char* output_val_buf, int* p_val_str_len);

extern int is_cli_dync_get_tz_group(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_timezone(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_config_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_bsl_source(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_bsl_severity(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_trap_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_dump_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_inf_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_netdev_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_vrf_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_net_process_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_ipsec_conn(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_ipsec_auth_type(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_cert_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_key_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_mf_segment(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_mgmt_dns(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_addr_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_ipsec_secret(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_ipsec_ca(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_mgmt_permitted(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_vrf_process_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_route_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_iptables_rule_list(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_log_server(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_ntp_server(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_agent_commu(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_trap_host(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_agent_user(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_radius_srv(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_tacacs_srv(void* data, char* output_val_buf, int* p_val_str_len); 

extern int is_cli_dync_get_user_account(void* data, char* output_val_buf, int* p_val_str_len); 

