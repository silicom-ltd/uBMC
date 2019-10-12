/*************************************************************************
 > File Name: is_console_mgmtd_client.c
 > Author: Jeff
 > Mail: jeff.zheng@silicom.co.il
 > Created Time: Mon 28 May 2018 10:07:36 AM HKT
 ************************************************************************/
#include "silc_mgmtd_variables.h"
#include "silc_mgmtd_interface.h"
#include "is_con_mgr_mgmtd.h"
#include "is_con_mgr_config.h"
#include "console_manager_cmdline.h"

static is_con_mgr_mgmtd_client g_is_console_mgmtd_client;
/*static*/ is_config_console g_is_config_console;

bool is_con_mgr_mgmtd_config_load(is_con_mgr_mgmtd_client* p_client,
		is_config_console * * p_cfg_console);

int is_mgmtd_client_parser_string(const silc_cstr val_str, char* buf,
		uint32_t buf_len)
{
	silc_mgmtd_var val;

	memset(&val, 0, sizeof(val));
	val.type = SILC_MGMTD_VAR_STRING;
	if (silc_mgmtd_var_set_by_str(&val, val_str) != 0)
		return -1;
	strncpy(buf, val.val.string_val, buf_len);
	buf[buf_len] = '\0';
	//printf("[%s] str:%s, val:%u\n", __func__, val_str, *p_uint);
	silc_mgmtd_var_clear(&val);
	return 0;
}

int is_mgmtd_client_parser_uint32(const silc_cstr val_str, uint32_t* p_uint)
{
	silc_mgmtd_var val;

	memset(&val, 0, sizeof(val));
	val.type = SILC_MGMTD_VAR_UINT32;
	if (silc_mgmtd_var_set_by_str(&val, val_str) != 0)
		return -1;
	*p_uint = val.val.uint32_val;
	//printf("[%s] str:%s, val:%u\n", __func__, val_str, *p_uint);
	silc_mgmtd_var_clear(&val);
	return 0;
}

int is_mgmtd_client_parser_bool(const silc_cstr val_str, bool* p_bool)
{
	silc_mgmtd_var val;

	memset(&val, 0, sizeof(val));
	val.type = SILC_MGMTD_VAR_BOOL;
	if (silc_mgmtd_var_set_by_str(&val, val_str) != 0)
		return -1;
	if (val.val.bool_val)
		*p_bool = true;
	else
		*p_bool = false;
	//printf("[%s] str:%s, val:%u\n", __func__, val_str, *p_bool);
	silc_mgmtd_var_clear(&val);
	return 0;
}


int is_con_mgr_mgmtd_notify_config_changed_handler(silc_mgmtd_if_req* p_notify,
		void* data)
{
	is_con_mgr_mgmtd_client* p_client = data;
	is_config_console *p_new_config;
	is_console *p_console = p_client->p_console;

	pthread_mutex_lock(&p_client->lock);

	if (!is_con_mgr_mgmtd_config_load(p_client, &p_new_config))
	{
		console_m_err("Failed to read new configuration from mgmtd");
	}
	else
	{
		p_console->is_status_changed = true;
		p_console->p_config = p_new_config;
	}

	pthread_mutex_unlock(&p_client->lock);
	return 0;
}

static int is_con_mgr_mgmtd_client_start(is_con_mgr_mgmtd_client* p_client,
		void* p_console)
{
	int ret = 0;
	p_client->p_console = p_console;

	is_call_init(silc_mgmtd_if_client_set_recv_notify,
			"/notify/bmc/config_changed",
			is_con_mgr_mgmtd_notify_config_changed_handler, p_client);
	is_call_init(silc_mgmtd_if_client_start_notify_daemon);

	return ret;

	ERR_RET: return 0;
}

int is_con_mgr_mgmtd_handle_response(silc_mgmtd_if_rsp *p_rsp,
		is_config_console* p_cfg_console)
{
	int ret = 0;
	silc_mgmtd_if_node *p_node;
	silc_mgmtd_if_node *p_mod_sub_node;
	is_config_console *p_console;

	p_console = p_cfg_console;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if (strcmp(p_node->name, "console") == 0)
		{
			silc_list_for_each_entry(p_mod_sub_node, &p_node->sub_node_list, node)
			{
				if (strcmp(p_mod_sub_node->name, "log-rotate-num") == 0)
				{
					if (is_mgmtd_client_parser_uint32(p_mod_sub_node->val_str,
							&p_console->log_rotate_num) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "log-rotate-size") == 0)
				{
					if (is_mgmtd_client_parser_uint32(p_mod_sub_node->val_str,
							&p_console->log_rotate_size) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "log-to-file") == 0)
				{
					if (is_mgmtd_client_parser_bool(p_mod_sub_node->val_str,
							&p_console->is_log_to_file) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-speed") == 0)
				{
					if (is_mgmtd_client_parser_uint32(p_mod_sub_node->val_str,
							&p_console->com_speed) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-data") == 0)
				{
					if (is_mgmtd_client_parser_uint32(p_mod_sub_node->val_str,
							&p_console->com_data) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-stopbits") == 0)
				{
					if (is_mgmtd_client_parser_uint32(p_mod_sub_node->val_str,
							&p_console->com_stopbits) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-hwflowctrl") == 0)
				{
					if (is_mgmtd_client_parser_bool(p_mod_sub_node->val_str,
							&p_console->com_hwflowctrl) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-swflowctrl") == 0)
				{
					if (is_mgmtd_client_parser_bool(p_mod_sub_node->val_str,
							&p_console->com_swflowctrl) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}
				else if (strcmp(p_mod_sub_node->name, "com-parity") == 0)
				{
					if (is_mgmtd_client_parser_string(p_mod_sub_node->val_str,
							p_console->com_parity, 32) != 0)
					{
						console_m_err("%s\n", p_mod_sub_node->name);
						return -1;
					}
				}

			}
		}

	}
	return ret;
}

int is_con_mgr_mgmtd_config_load_inner(is_config_console* p_cfg_console)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	p_req = silc_mgmtd_if_req_create("/config/bmc", SILC_MGMTD_IF_REQ_QUERY_SUB,
			"");
	if (!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if (!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if (!silc_mgmtd_if_rsp_check(p_rsp))
	{
		is_err("Query the console manager bmc configuration failed: %s.\n",
				p_rsp->return_str);
		ret = -1;
	}
	else
	{
		if (is_con_mgr_mgmtd_handle_response(p_rsp, p_cfg_console) != 0)
			ret = -1;
	}

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);
	return ret;
}

/*
 *return value :success - true, failure - false
 */
bool is_con_mgr_mgmtd_config_load(is_con_mgr_mgmtd_client* p_client,
		is_config_console * * p_cfg_console)
{
	*p_cfg_console = &g_is_config_console;

	if (is_con_mgr_mgmtd_config_load_inner(*p_cfg_console) != 0)
	{
		return false;
	}
//	is_mgmtd_client_print_config(p_cfg_console);

	return true;
}

int is_cons_mgr_mgmtd_config_lock(is_con_mgr_mgmtd_client* p_client)
{
	return pthread_mutex_lock(&p_client->lock);
}

int is_con_mgr_mgmtd_config_unlock(is_con_mgr_mgmtd_client* p_client)
{
	return pthread_mutex_unlock(&p_client->lock);
}

bool is_cons_mgr_mgmtd_get_param_changed_status(is_console * p_console)
{
	bool tmp_status = false;

	tmp_status = p_console->is_status_changed;
	p_console->is_status_changed = false;

	return tmp_status;
}

static is_con_mgr_mgmtd_client* is_con_mgr_mgmtd_client_init_inner(void)
{
	int ret = 0;
	is_con_mgr_mgmtd_client* p_client;
	p_client = &g_is_console_mgmtd_client;

	memset(p_client, 0, sizeof(*p_client));
	is_call_init(pthread_mutex_init, &p_client->lock, NULL);
	is_call_init(silc_mgmtd_if_client_init, "127.0.0.1", 2626, -1, 3); 
	is_call_init(silc_mgmtd_if_client_set_login_info,
			SILC_MGMTD_IF_CLIENT_CONSOLE, "root", NULL, NULL);
	return p_client;
	ERR_RET: return 0;
}

int is_cons_mgr_mgmtd_client_init(is_console * p_console)
{
	int ret = 0;
	bool res = false;
	is_config_console * p_config;
	is_con_mgr_mgmtd_client * p_client;
	if (p_console == NULL)
	{
		console_m_err("p_console is NULL \n");
		return -1;
	}
	is_call_create(p_client, is_con_mgr_mgmtd_client_init_inner);
	is_cons_mgr_mgmtd_config_lock(p_client);
	res = is_con_mgr_mgmtd_config_load(p_client, &p_config);
	if (!res)
	{
		is_con_mgr_mgmtd_config_unlock(p_client);
		console_m_err("Failed to load console configuration");
	}

	p_console->p_config = p_config;
	p_console->is_status_changed = false;
	is_con_mgr_mgmtd_config_unlock(p_client);
	is_call_init(is_con_mgr_mgmtd_client_start, p_client, p_console);
	if (!res)
		return -1;
	else
		return 0;

	ERR_RET: return -1;
}
