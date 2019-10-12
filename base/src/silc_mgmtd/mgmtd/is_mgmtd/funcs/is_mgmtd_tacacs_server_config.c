/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"
#include "time.h"

int is_mgmtd_system_check_hostname(char* hostname);

int is_mgmtd_tacacs_static_enable_apply(silc_mgmtd_node* new_node, char* node_del, void* conn_entry);

int is_mgmtd_tacacs_server_add_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	USER_OP_LOG(conn_entry, "TACACS+ server %u is added", p_node->value.val.uint32_val);
	return is_mgmtd_tacacs_static_enable_apply(p_node, NULL, conn_entry);
}

int is_mgmtd_tacacs_server_modify_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	USER_OP_LOG(conn_entry, "TACACS+ server %u is modified", p_node->value.val.uint32_val);
	return is_mgmtd_tacacs_static_enable_apply(NULL, NULL, conn_entry);
}

int is_mgmtd_tacacs_server_delete_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	char node_path[256];
	silc_mgmtd_memdb_get_node_path(p_node, node_path);

	USER_OP_LOG(conn_entry, "TACACS+ server %u is deleted", p_node->value.val.uint32_val);
	return is_mgmtd_tacacs_static_enable_apply(NULL, node_path, conn_entry);
}

int is_mgmtd_tacacs_server_check_req(silc_mgmtd_if_req_type type, silc_mgmtd_node* server_node, void* conn_entry)
{
	silc_mgmtd_node* p_node;
	silc_cstr hostname;
	uint32_t port;
	silc_bool server_changed = silc_false;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-ip");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		hostname = p_node->tmp_value.val.string_val;
		int ret = is_mgmtd_system_check_hostname(hostname);
		if (ret)
			return ret;
		server_changed = silc_true;
	}
	else
		hostname = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-port");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		port = p_node->tmp_value.val.uint32_val;
		if (port == 0 || port > 65535)
		{
			SILC_ERR("Invalid TACACS+ server port: %u", port);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		server_changed = silc_true;
	}
	else
		port = p_node->value.val.uint32_val;

	if (SILC_MGMTD_IF_REQ_CHECK_ADD == type || server_changed)
	{
		silc_mgmtd_node* p_sub_node;
		p_node = silc_mgmtd_memdb_find_node("/config/tacacs/server");
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(p_sub_node->type != MEMDB_NODE_TYPE_DYNAMIC)
				continue;
			if (p_sub_node->value.val.uint32_val != server_node->value.val.uint32_val &&
					0 == strcmp(hostname, silc_mgmtd_memdb_find_sub_node(p_sub_node, "server-ip")->value.val.string_val) &&
					port == silc_mgmtd_memdb_find_sub_node(p_sub_node, "server-port")->value.val.uint32_val)
			{
				SILC_ERR("TACACS+ server [%s]:%u already exists", hostname, port);
				return IS_MGMTD_ERR_AAA_SERVER_DUPLICATED;
			}
		}
	}

	return 0;
}

int is_mgmtd_tacacs_server_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	int ret = 0;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
			ret = is_mgmtd_tacacs_server_check_req(type, p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_ADD:
			ret = is_mgmtd_tacacs_server_add_config(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = is_mgmtd_tacacs_server_modify_config(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_DELETE:
			ret = is_mgmtd_tacacs_server_delete_config(p_node, conn_entry);
			break;
		default:
			break;
	}

    return ret;
#else
    return 0;
#endif
}
