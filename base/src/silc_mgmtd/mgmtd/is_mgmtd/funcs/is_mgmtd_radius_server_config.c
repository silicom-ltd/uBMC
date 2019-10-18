/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"
#include "time.h"

#define IS_MGMTD_RADIUS_SERVER_CONFIG "/etc/pam_radius_auth.conf"

int is_mgmtd_system_check_hostname(char* hostname);

int is_mgmtd_radius_write_server(FILE* fp, silc_mgmtd_node* server_node)
{
	silc_mgmtd_node* p_node;
	char *ip, *key;
	uint32_t port, timeout;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-ip");
	ip = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-port");
	port = p_node->value.val.uint32_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "secret");
	key = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "timeout");
	timeout = p_node->value.val.uint32_val;

	fprintf(fp, "%s:%d\t%s\t%d\n\n",
				ip, port, key, timeout);
	return 0;
}

int is_mgmtd_radius_server_apply(silc_mgmtd_node* new_node, char* path_del)
{
	silc_mgmtd_node *p_db_sub_node;
	silc_mgmtd_node *p_node;
	FILE* fp;
	time_t t;

	char path[256];

	fp = fopen(IS_MGMTD_RADIUS_SERVER_CONFIG".bak", "w");

	time(&t);
	fprintf(fp, "##\n"
				"## This file was AUTOMATICALLY GENERATED.  DO NOT MODIFY.\n"
				"## Any changes will be lost.\n"
				"##\n"
				"## Generated by %s at %s"
				"##\n", __func__, ctime(&t));

	fprintf(fp, "# server[:port]\tshared_secret\ttimeout (sec)\n");

	p_node = silc_mgmtd_memdb_find_node("/config/radius/server");
	silc_list_for_each_entry(p_db_sub_node, &p_node->sub_node_list, node)
	{
		if (path_del)
		{
			silc_mgmtd_memdb_get_node_path(p_db_sub_node, path);
			if (strcmp(path, path_del) == 0)
				continue;
		}

		if (p_db_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		{
			is_mgmtd_radius_write_server(fp, p_db_sub_node);
		}
	}
	/*
	if (new_node)
	{
		is_mgmtd_radius_write_server(fp, new_node);
	}
	*/

	fflush(fp);
	fclose(fp);

	rename(IS_MGMTD_RADIUS_SERVER_CONFIG".bak", IS_MGMTD_RADIUS_SERVER_CONFIG);

	return 0;
}

int is_mgmtd_radius_server_add_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	USER_OP_LOG(conn_entry, "Radius server %u is added", p_node->value.val.uint32_val);
	is_mgmtd_radius_server_apply(p_node, NULL);
	return 0;
}

int is_mgmtd_radius_server_modify_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	USER_OP_LOG(conn_entry, "Radius server %u is modified", p_node->value.val.uint32_val);
	is_mgmtd_radius_server_apply(NULL, NULL);
	return 0;
}

int is_mgmtd_radius_server_delete_config(silc_mgmtd_node* p_node, void* conn_entry)
{
	char node_path[256];
	silc_mgmtd_memdb_get_node_path(p_node, node_path);

	USER_OP_LOG(conn_entry, "Radius server %u is deleted", p_node->value.val.uint32_val);
	is_mgmtd_radius_server_apply(NULL, node_path);
	return 0;
}

int is_mgmtd_radius_server_check_req(silc_mgmtd_if_req_type type, silc_mgmtd_node* server_node, void* conn_entry)
{
	silc_mgmtd_node* p_node;
	silc_cstr hostname;
	uint32_t port;
	silc_bool server_changed = silc_false;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "timeout");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t to = p_node->tmp_value.val.uint32_val;
		if (to < 1 || to > 60)
		{
			SILC_ERR("RADIUS server timeout should be 1-60");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

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
			SILC_ERR("Invalid Radius server port: %u", port);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		server_changed = silc_true;
	}
	else
		port = p_node->value.val.uint32_val;

	if (SILC_MGMTD_IF_REQ_CHECK_ADD == type || server_changed)
	{
		silc_mgmtd_node* p_sub_node;
		p_node = silc_mgmtd_memdb_find_node("/config/radius/server");
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(p_sub_node->type != MEMDB_NODE_TYPE_DYNAMIC)
				continue;
			if (p_sub_node->value.val.uint32_val != server_node->value.val.uint32_val &&
					0 == strcmp(hostname, silc_mgmtd_memdb_find_sub_node(p_sub_node, "server-ip")->value.val.string_val) &&
					port == silc_mgmtd_memdb_find_sub_node(p_sub_node, "server-port")->value.val.uint32_val)
			{
				SILC_ERR("Radius server %s:%u already exists", hostname, port);
				return IS_MGMTD_ERR_AAA_SERVER_DUPLICATED;
			}
		}
	}

	return 0;
}

int is_mgmtd_radius_server_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	int ret = 0;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
			ret = is_mgmtd_radius_server_check_req(type, p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_ADD:
			ret = is_mgmtd_radius_server_add_config(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = is_mgmtd_radius_server_modify_config(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_DELETE:
			ret = is_mgmtd_radius_server_delete_config(p_node, conn_entry);
			break;
		default:
			break;
	}

    return ret;
#else
    return 0;
#endif
}
