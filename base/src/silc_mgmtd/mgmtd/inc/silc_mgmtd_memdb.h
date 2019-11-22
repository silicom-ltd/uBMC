
#ifndef SILC_MGMTD_MEMDB_H_
#define SILC_MGMTD_MEMDB_H_

#include "silc_mgmtd_variables.h"
#include "silc_mgmtd_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SILC_MGMTD_MEMDB_NAME_MAX_LEN		SILC_MGMTD_IF_NAME_MAX_LEN

typedef enum
{
	MEMDB_NODE_TYPE_NULL = 0,
	MEMDB_NODE_TYPE_STATIC,
	MEMDB_NODE_TYPE_TEMPLATE,
	MEMDB_NODE_TYPE_TEMPLATE_SUB,
	MEMDB_NODE_TYPE_DYNAMIC,
	MEMDB_NODE_TYPE_DYNAMIC_SUB,
	MEMDB_NODE_TYPE_BOOT,
	MEMDB_NODE_TYPE_UNKNOWN,
	MEMDB_NODE_TYPE_MAX,
}silc_mgmtd_node_type;

typedef struct silc_mgmtd_cb_err_s
{
	silc_list_node node;
	int err_num;
	silc_cstr  err_str;
}silc_mgmtd_cb_err;

typedef int (*silc_mgmtd_node_ref_callback)(const silc_cstr path, void* p_db_node);
typedef struct silc_mgmtd_node_ref_s
{
	silc_list_node node;
	silc_cstr path;
	silc_mgmtd_node_ref_callback do_ref;
}silc_mgmtd_node_ref;

/*
 * ------------- node definition ---------------
 */
typedef int (*silc_mgmtd_node_callback)(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry);

typedef struct silc_mgmtd_node_s
{
	silc_list_node 	node;
	struct silc_mgmtd_node_s *p_parent;

	char name[SILC_MGMTD_MEMDB_NAME_MAX_LEN];
	silc_mgmtd_node_type type;

	silc_mgmtd_node_callback do_ops;
	int do_ops_timeout;
	silc_list	do_ops_err_list;

	silc_mgmtd_var value;
	silc_mgmtd_var tmp_value;

	silc_list	sub_node_list;
	silc_list ref_node_list;

	silc_mgmtd_if_privilege if_level;
	pthread_mutex_t*	p_lock;
	int	rlocked_cnt;
//	silc_bool	wlocked;
	uint16_t wlock_owner; // connection index
}silc_mgmtd_node;

typedef struct silc_mgmtd_node_info_s
{
    silc_cstr path;
    silc_mgmtd_node_type type;
    silc_mgmtd_if_privilege level;
    silc_mgmtd_var_type val_type;
    silc_cstr val_str;
    silc_mgmtd_node_callback cb;
    int cb_timeout;
}silc_mgmtd_node_info;

/* define memdb node err info struct */
typedef struct silc_mgmtd_cberr_info_s
{
    silc_cstr path;
    int num;
    silc_cstr info;
}silc_mgmtd_cberr_info;

/* define memdb node ref info struct */
typedef struct silc_mgmtd_ref_info_s
{
    silc_cstr path;
    silc_cstr ref_path;
    silc_mgmtd_node_ref_callback ref_cb;
}silc_mgmtd_ref_info;

/* define memdb running config struct */
#define SILC_MGMTD_CONFIG_PATH_PARAM_MIX	4
#define SILC_MGMTD_CONFIG_CMD_PARAM_MAX		8
#define SILC_MGMTD_CONFIG_CMD_LEN_MAX		8192

typedef silc_cstr (*silc_mgmtd_config_get_val_param_str)(silc_cstr val_str, int param_idx);
typedef silc_cstr (*silc_mgmtd_config_set_val_param)(silc_cstr val_str);

typedef struct silc_mgmtd_config_param_s
{
	silc_cstr path_str;
	silc_cstr val_param_map;
	int param_path_idx[SILC_MGMTD_CONFIG_PATH_PARAM_MIX];	// parameter position in path, -1 means the last one
	silc_mgmtd_config_get_val_param_str val_param_func;
	silc_mgmtd_config_set_val_param set_val_param;
}silc_mgmtd_config_param;

#define FLAG_CMD_TRANS_FALSE2NO	1

#define TAG_CMD_TRANS_FALSE2NO	"#*NEGATIVE*#"

typedef silc_cstr (*silc_mgmtd_config_get_cmd_str)(void *p_cmd);

typedef struct silc_mgmtd_config_cmd_map_s
{
	silc_cstr command_str;
	silc_mgmtd_config_param config_params[SILC_MGMTD_CONFIG_CMD_PARAM_MAX];
	int flag;
	silc_mgmtd_config_get_cmd_str get_cmd_func;
}silc_mgmtd_config_cmd_map;

typedef struct silc_mgmtd_config_cmd_s
{
	silc_list_node node;
	silc_cstr cmd_key_str;
	silc_cstr_array* cmd_params[SILC_MGMTD_CONFIG_CMD_PARAM_MAX];
	silc_mgmtd_config_cmd_map* p_map;
	silc_cstr cmd_params_dft[SILC_MGMTD_CONFIG_CMD_PARAM_MAX];
}silc_mgmtd_config_cmd;


typedef int (*silc_mgmtd_default_node_add_func)(void);
typedef void (*silc_mgmtd_default_node_del_func)(void);
typedef int (*silc_mgmtd_get_node_func)(silc_mgmtd_node_info** p_node_list, int* p_node_cnt);
typedef int (*silc_mgmtd_get_cberr_func)(silc_mgmtd_cberr_info** p_cberr_list, int* p_cberr_cnt);
typedef int (*silc_mgmtd_get_config_cmd_func)(silc_mgmtd_config_cmd_map** p_config_list, int* p_config_cnt);
typedef int (*silc_mgmtd_set_init_config_func)(void);
typedef int (*silc_mgmtd_custom_sys_halt_func)(void);
typedef int (*silc_mgmtd_custom_sync_hw_clock)(void);
typedef int (*silc_mgmtd_custom_user_password_chk)(silc_cstr pass, int len);
typedef silc_cstr (*silc_mgmtd_snmp_get_sysoid)(void);
typedef silc_cstr (*silc_mgmtd_get_ttyd_cmd)(void);

typedef struct silc_mgmtd_storage_path_s
{
	silc_cstr name;
	silc_cstr path;
}silc_mgmtd_storage_path;


typedef struct silc_mgmtd_product_info_s
{
	silc_cstr product_name;
	uint32_t product_id;
	silc_cstr device_name;
	silc_cstr admin_name;
	silc_cstr admin_shadow;
	silc_cstr *vendor_list;
	int vendor_cnt;
	silc_mgmtd_storage_path * storage_list;
	int	storage_cnt;

	int multi_eth_support;	//management feature enable/disable
	int permit_ip_support;	//managemnet feature enable/disable. permit ip is a simple ip blocking mechanism
	int com_baudrate_support; //management feature enable/disable com port baudrate settings.
	int vrf_support;
	int iptables_support;
	int ipsec_support;

	silc_mgmtd_default_node_add_func	default_node_add_func;
	
	silc_mgmtd_default_node_del_func	default_node_del_func;

	silc_mgmtd_get_node_func get_node_func;

	silc_mgmtd_get_cberr_func get_cberr_func;

	silc_mgmtd_get_config_cmd_func get_config_cmd_func;

	silc_mgmtd_set_init_config_func set_init_config_func;

	silc_mgmtd_custom_sys_halt_func custom_sys_halt_func;

	silc_mgmtd_custom_sync_hw_clock custom_sync_hw_clock_func;

	silc_mgmtd_custom_user_password_chk custom_user_password_chk_func;

	silc_mgmtd_snmp_get_sysoid get_snmp_sysoid_func;//can return NULL

	silc_mgmtd_get_ttyd_cmd get_ttyd_cmd_func;//can return NULL

}silc_mgmtd_product_info;

#define silc_mgmtd_node_changed(p_node)		((p_node)->tmp_value.type == (p_node)->value.type && !silc_mgmtd_var_equal(&(p_node)->tmp_value, &(p_node)->value))
#define silc_mgmtd_node_configured(p_node)	((p_node)->tmp_value.type == (p_node)->value.type)

#define USER_OP_LOG(conn, fmt, ...)		do { if(conn) { SILC_LOG(fmt, ## __VA_ARGS__); } } while (0);

int silc_mgmtd_memdb_init();

void silc_mgmtd_memdb_deinit();

void silc_mgmtd_memdb_set_config_saved();

void silc_mgmtd_memdb_set_config_unsave();

silc_bool silc_mgmtd_memdb_is_config_saved();

silc_mgmtd_node* silc_mgmtd_memdb_find_parent(const silc_cstr node_path);

void silc_mgmtd_memdb_insert_node(silc_mgmtd_node* p_node, silc_mgmtd_node* p_parent);

int silc_mgmtd_memdb_add_node(const silc_cstr node_path, silc_mgmtd_if_privilege level, silc_mgmtd_node_type node_type,
		silc_mgmtd_var_type val_type, const silc_cstr val_str, silc_mgmtd_node_callback do_ops, int timeout);
int silc_mgmtd_memdb_add_node_ref(const silc_cstr node_path, const silc_cstr path, silc_mgmtd_node_ref_callback do_ref);

silc_mgmtd_node* silc_mgmtd_memdb_create_node(const silc_cstr node_name, silc_mgmtd_if_privilege level, silc_mgmtd_node_type node_type,
		silc_mgmtd_var_type val_type, const silc_cstr val_str, silc_mgmtd_node_callback do_ops, int timeout);

void silc_mgmtd_memdb_delete_node(silc_mgmtd_node* p_node);

silc_mgmtd_node* silc_mgmtd_memdb_clone_node(silc_mgmtd_node* p_src_node);

silc_mgmtd_node* silc_mgmtd_memdb_find_init_node(const silc_cstr node_path);
silc_mgmtd_node* silc_mgmtd_memdb_find_node(const silc_cstr node_path);
silc_mgmtd_node* silc_mgmtd_memdb_find_node_top(const silc_cstr node_path);

silc_mgmtd_node* silc_mgmtd_memdb_find_sub_node(silc_mgmtd_node* p_node, const silc_cstr name);
silc_mgmtd_node* silc_mgmtd_memdb_find_sub_value(silc_mgmtd_node* p_node, const silc_cstr val_str);

int silc_mgmtd_memdb_modify_node(silc_mgmtd_node* p_node, const silc_cstr new_val_str);

int silc_mgmtd_memdb_modify_node_confirm(silc_mgmtd_node* p_node);

int silc_mgmtd_memdb_modify_node_confirm_tree(silc_mgmtd_node* p_node);

silc_cstr silc_mgmtd_memdb_path_get_last_name(const silc_cstr path);

silc_cstr silc_mgmtd_memdb_get_node_path(silc_mgmtd_node* p_node, silc_cstr path);

int silc_mgmtd_memdb_add_cb_err(const silc_cstr node_path, int err_num, const silc_cstr err_str);

silc_cstr silc_mgmtd_memdb_get_cb_err(silc_mgmtd_node* p_node, int err_num);

typedef int (*silc_mgmtd_memdb_dnode_traversal_cb)(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* cb_data);
int silc_mgmtd_memdb_dnode_tree_traversal(silc_mgmtd_node* p_db_node,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_memdb_dnode_traversal_cb cb_func, void* cb_data);

typedef int (*silc_mgmtd_memdb_node_traversal_cb)(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data);
int silc_mgmtd_memdb_node_tree_traversal(const silc_cstr parent_path,
		silc_mgmtd_node* p_db_node, silc_mgmtd_memdb_node_traversal_cb cb_func, void* cb_data);

void silc_mgmtd_memdb_node_show(silc_mgmtd_node* p_db_node);
typedef int (*silc_mgmtd_memdb_if_node_traversal_cb)(const silc_cstr path, silc_mgmtd_if_node* p_if_node, void* cb_data);
void silc_mgmtd_memdb_if_node_show(silc_mgmtd_if_node* p_if_node);
void silc_mgmtd_memdb_show_all();

silc_mgmtd_node* silc_mgmtd_memdb_find_config_module_node(silc_mgmtd_node* p_node);

int silc_mgmtd_memdb_node_r_lock(silc_mgmtd_node* p_node);

void silc_mgmtd_memdb_node_r_unlock(silc_mgmtd_node* p_node);

int silc_mgmtd_memdb_node_w_lock(silc_mgmtd_node* p_node, uint16_t wlock_owner, silc_bool is_wait);

void silc_mgmtd_memdb_node_w_unlock(silc_mgmtd_node* p_node);

int silc_mgmtd_memdb_get_running_config(silc_cstr buff, int len);
int silc_mgmtd_memdb_restore_ext_conf_file(silc_cstr filename, silc_cstr output_filename);
int silc_mgmtd_memdb_save_ext_conf_file(silc_cstr filename, silc_cstr output_filename);
void silc_mgmtd_memdb_remove_ext_conf_file(silc_cstr filename);

silc_mgmtd_product_info* silc_mgmtd_memdb_get_product_info();

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_MEMDB_H_ */
