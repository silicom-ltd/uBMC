
#ifndef SILC_CLI_COMMON_H_
#define SILC_CLI_COMMON_H_

#include "silc_common.h"
#include "silc_mgmtd_interface.h"
#include "silc_cli_types.h"

#include <crypt.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct silc_cli_login_info_s
{
	silc_mgmtd_if_client_type type;
	char name[40];
	char ip[20];
	char port[10];
	char privil[10];
	char protocol[16];
}silc_cli_login_info;

typedef void (*is_cli_cmd_rsp_handle_cb)(silc_mgmtd_if_rsp* p_rsp);

typedef struct is_cli_cmd_req_info_s
{
	char 	path[SILC_MGMTD_IF_PATH_MAX_LEN];
	silc_cstr root_val;
	silc_mgmtd_if_req_type type;
	is_cli_cmd_rsp_handle_cb rsp_cb;
}is_cli_cmd_req_info;

int silc_cli_do_cmd(silc_cli_cmd* p_cmd, silc_list* p_token_list);

int silc_cli_get_mgmtd_dev_name(silc_cstr output_buff);
int silc_cli_get_mgmtd_session_expire_timeout(int* p_timeout);
int silc_cli_set_mgmtd_session_expire_timeout(silc_cstr timeout);
int silc_cli_check_mgmtd_node_exist(const silc_cstr path, silc_bool *p_exist);
int silc_cli_get_mgmtd_bool_node_value(const silc_cstr path, silc_bool *p_bool);
int silc_cli_get_mgmtd_int_node_value(const silc_cstr path, int *p_int);

int silc_cli_get_mgmtd_node_child_list(silc_cstr path, char* output_val_buf, int str_len);
int silc_cli_set_connect_login_info();
int silc_cli_session_timeout_status(void);
void silc_cli_session_timeout_disable(void);
void silc_cli_session_timeout_enable(void);
int silc_cli_session_timeout_init(void);
void silc_cli_session_timeout_update();
uint32_t silc_cli_session_timeout_curr_get();
silc_cstr silc_cli_get_connect_login_name();

int silc_cli_show_simple_string_query(const silc_cstr path, const silc_cstr format);
int silc_cli_show_simple_trans_query(const silc_cstr path, const silc_cstr format, const silc_cstr* trans, int trans_num);
int silc_cli_show_complex_string_query(const silc_cstr path,
		const silc_cstr info, const silc_cstr* item_names, const silc_cstr* item_formats, int item_num);

typedef void (*silc_cli_cmd_rsp_handle_cb)(silc_mgmtd_if_rsp* p_rsp);

int silc_cli_cmd_do_simple_request(silc_mgmtd_if_req_type type,
		const silc_cstr path, const silc_cstr root_val, silc_cli_cmd_rsp_handle_cb cb, silc_cstr err_info, int info_len);

int silc_cli_cmd_do_simple_notify(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len);
int silc_cli_cmd_do_simple_action(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len);
int silc_cli_cmd_do_simple_modify(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len);

int silc_cli_cmd_get_simple_node(const silc_cstr path, silc_cstr buff);

int silc_cli_cmd_split_mod_seg(silc_cstr data, silc_cstr mod, silc_cstr seg);
int silc_cli_cmd_split_mod_seg_port(silc_cstr data, silc_cstr mod, silc_cstr seg, silc_cstr port);

silc_cli_token* is_cli_cmd_get_next_rl_token(silc_list* p_token_list, silc_cli_token* p_token);

int is_cli_cmd_do_request_core(is_cli_cmd_req_info* p_req_info, silc_list* p_token_list, int timeout_sec);
int is_cli_cmd_do_request(is_cli_cmd_req_info* p_req_info, silc_list* p_token_list);

void silc_cli_simple_node_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, silc_cstr* trans_list, int trans_num);

typedef silc_cstr (*is_cli_display_cb)(silc_cstr node_name, silc_cstr node_val_str, silc_cstr buf, int len);

void silc_cli_l1tree_list_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num, is_cli_display_cb cb);

void silc_cli_l2tree_list_display(silc_mgmtd_if_node *p_node, const silc_cstr info_format,
		const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num);

void silc_cli_l2tree_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr* item_formats, silc_cstr** item_trans, int* item_trans_num, int item_num);

int silc_cli_upload_file(silc_cstr url, silc_cstr user, silc_cstr passwd, silc_cstr filename);
int silc_cli_cmd_confirm(silc_cstr prompt, silc_cstr y_msg, silc_cstr n_msg);
int silc_cli_show_cmd_output(silc_cstr cmd);
int silc_cli_show_log(silc_cstr log, silc_cstr filter);
silc_bool silc_cli_check_log_filter(silc_cstr filter);
silc_bool silc_cli_check_name(silc_cstr val);

#ifdef __cplusplus
}
#endif

#endif /* SILC_CLI_COMMON_H_ */
