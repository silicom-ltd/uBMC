/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "ubmc_mgmtd_func_def.h"

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_bmc_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("bmc", conn_entry);
}

#define IS_MGMTD_BMC_PATH	"/config/bmc/"

#define LOG_ROTATE_NUM_MAX	50
#define LOG_ROTATE_SIZE_MAX	10

int is_mgmtd_bmc_config_console_check(silc_mgmtd_node* p_node)
{
	silc_mgmtd_node* p_sub_node;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "log-rotate-num");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL &&
			(p_sub_node->tmp_value.val.uint32_val > LOG_ROTATE_NUM_MAX ||
					p_sub_node->tmp_value.val.uint32_val < 1))
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "log-rotate-size");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL &&
			(p_sub_node->tmp_value.val.uint32_val > LOG_ROTATE_SIZE_MAX ||
					p_sub_node->tmp_value.val.uint32_val < 1))
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;

	return 0;
}

int is_mgmtd_bmc_config_check(silc_mgmtd_node* p_req_node)
{
	silc_mgmtd_node* p_node = NULL;
	char* node_name = "console";

	if(strcmp(p_req_node->name, node_name) == 0)
		p_node = p_req_node;
	else if(strcmp(p_req_node->p_parent->name, node_name) == 0)
		p_node = p_req_node->p_parent;

	if(p_node)
		return is_mgmtd_bmc_config_console_check(p_node);
	return 0;
}

int is_mgmtd_bmc_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			return is_mgmtd_bmc_config_check(p_node);
		case SILC_MGMTD_IF_REQ_ADD:
		case SILC_MGMTD_IF_REQ_DELETE:
			return 0;
		case SILC_MGMTD_IF_REQ_MODIFY:
			silc_mgmtd_if_server_notify("/notify/bmc/config_changed");
			is_mgmtd_bmc_update_ts(conn_entry);
			break;
		default:
			return 0;
	}

    return 0; 
}

