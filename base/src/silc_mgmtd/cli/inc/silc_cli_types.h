#ifndef SILC_CLI_TYPES_H_
#define SILC_CLI_TYPES_H_

#include "silc_common.h"
#include "silc_mgmtd_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -------------------------- TOKEN ------------------------------
 */
typedef enum
{
  TOKEN_TYPE_VARIABLE = 0,
  TOKEN_TYPE_STATIC,
  TOKEN_TYPE_DYNAMIC,
  TOKEN_TYPE_MODE,
  TOKEN_TYPE_COMMAND,
  TOKEN_TYPE_MAX,
}silc_cli_token_type;

typedef enum
{
  TOKEN_MODE_SINGLE = 0,
  TOKEN_MODE_OPTIONAL,
  TOKEN_MODE_MAX,
}silc_cli_token_mode;


typedef int (*silc_cli_cmd_func)(silc_list* p_token_list);
typedef int (*silc_cli_token_func)(void* data, char* output_val_buf, int* p_val_str_len);

typedef struct silc_cli_token_s
{
  silc_list_node		node;
  silc_list_node		rl_node;
  silc_cstr       name;
  silc_cstr       comment;
  silc_cstr				val_help;
  silc_cstr				val_str;
  silc_cstr				map_name;
  silc_cstr				map_val;
  silc_cli_token_type	type;
  silc_cli_token_mode	sub_mode;
  silc_cli_token_func	dync_func;
  silc_cli_cmd_func   cmd_func;
  void*				dync_func_data;
  int					dync_func_data_idx;	// data come from command line, prefix index
  char*				dync_val_str_buf;
  int					dync_val_str_buf_len;
  int					dync_val_str_len;
  int					dync_val_str_num;
  silc_list				sub_token_list;
  silc_list				err_list;
  struct silc_cli_token_s* p_parent;
  int					hidden;	// a bit-mask to map product
}silc_cli_token;

typedef struct silc_cli_token_info_s
{
    silc_cstr path;
    silc_cstr comment;
    silc_cstr val_help;
    silc_cstr map_name;
    silc_cstr map_val;
    silc_cli_token_mode sub_token_mode;
    silc_cli_token_type token_type;
    silc_cli_cmd_func do_func;
    silc_cli_token_func dync_func;
    int dync_buff_len;
    int dync_data_idx;
    int hidden;
}silc_cli_token_info;

typedef int (*silc_cli_get_token_func)(silc_cli_token_info** p_token_list, int* p_token_cnt);
typedef void (*silc_cli_show_snmp_trap_control)(silc_mgmtd_if_node* p_node);
typedef int (*silc_cli_get_snmp_v3_only)(void);
typedef int (*silc_cli_get_snmp_show_engine_id)(void);
typedef int (*silc_cli_get_snmp_threshold_enabled)(void);
typedef int (*silc_cli_get_permit_ip_support)(void);
typedef int (*silc_cli_get_dns_support)(void);
typedef int (*silc_cli_show_http_enabled)(void);
typedef silc_bool (*silc_cli_get_manufacture_mode)(void);

typedef struct silc_cli_product_info_s
{
    silc_cstr product_name;
    uint32_t  product_id;
	silc_cstr *vendor_list;
	int vendor_cnt;

	silc_cstr reboot_warn;

	int multi_eth_support;
	int whoami_support;
	int halt_support;

    silc_cli_get_token_func get_token_func;

    silc_cli_show_snmp_trap_control show_snmp_configure_trap_ctrl_func;
	silc_cli_get_snmp_v3_only snmp_v3_only_func;
	silc_cli_get_snmp_threshold_enabled snmp_threshold_enabled_func;
	silc_cli_get_snmp_show_engine_id snmp_show_engine_id_func;
    silc_cli_get_permit_ip_support permit_ip_support_func;		//default enabled
    silc_cli_get_dns_support dns_support_func;		//default enabled
    silc_cli_show_http_enabled show_http_func;		//default enabled

    silc_cli_get_manufacture_mode get_manufacture_mode_func;	//default false
}silc_cli_product_info;

silc_cli_product_info* silc_cli_get_product_info(void);

#define SILC_CLI_PATH_MAX 256
typedef struct silc_cli_token_oem_overlay_s
{
    char path[SILC_CLI_PATH_MAX];
    int  setup;
}silc_cli_token_overlay;

#define SILC_CLI_TOKEN_OVERLAY_MAX 1024


typedef silc_cli_token silc_cli_cmd;
typedef silc_cli_token silc_cli_mode;

void silc_cli_mode_list_init();

int silc_cli_mode_add(const silc_cstr name, const silc_cstr prompt);
silc_cli_mode* silc_cli_mode_curr_get();
int silc_cli_mode_curr_set(const silc_cstr name);
silc_cstr silc_cli_mode_curr_name_get();
silc_cstr silc_cli_mode_get_curr_prompt();

int silc_cli_token_add(silc_cli_token_info* p_info);
silc_cli_token* silc_cli_token_find(const silc_cstr token_path);
silc_cli_token* silc_cli_token_find_from_list(silc_list* p_token_list, const silc_cstr name);

void silc_cli_token_tree_show(silc_cli_token *p_token, int level);
void silc_cli_token_show_all();
int silc_cli_token_get_list_len(silc_list* p_token_list);

int silc_cli_mode_set_dev_name();
const silc_cstr silc_cli_mode_get_dev_name();

#ifdef __cplusplus
}
#endif

#endif /* SILC_CLI_TYPES_H_ */
