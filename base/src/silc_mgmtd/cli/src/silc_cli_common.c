#include "silc_common.h"
#include "silc_common/silc_thread.h"
#include "silc_cli_types.h"
#include "silc_cli_error.h"
#include "silc_cli_terminal.h"
#include "silc_mgmtd_interface.h"
#include "silc_cli_common.h"

int silc_cli_get_mgmtd_dev_name(silc_cstr output_buff)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_CFG_DEV_NAME, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		ret = -1;
		goto OUT;
	}

	if(p_rsp->p_node_root->val_str)
		strcpy(output_buff, p_rsp->p_node_root->val_str);
	else
		ret = -1;

OUT:
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_get_mgmtd_session_expire_timeout(int* p_timeout)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	long timeout;

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_CFG_SESS_EXP_TIME, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		ret = -1;
		goto OUT;
	}

	if(silc_mgmtd_if_cstr2l(p_rsp->p_node_root->val_str, &timeout) != 0)
	{
		ret = -1;
		goto OUT;
	}
	*p_timeout = timeout;
OUT:
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_set_mgmtd_session_expire_timeout(silc_cstr timeout)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_CFG_SESS_EXP_TIME, SILC_MGMTD_IF_REQ_MODIFY, timeout);
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		ret = -1;
		goto OUT;
	}
OUT:
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_check_mgmtd_node_exist(const silc_cstr path, silc_bool *p_exist)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		*p_exist = silc_false;
		goto OUT;
	}
	else
		*p_exist = silc_true;

OUT:
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_cmd_do_simple_request(silc_mgmtd_if_req_type type,
		const silc_cstr path, const silc_cstr root_val, silc_cli_cmd_rsp_handle_cb cb, silc_cstr err_info, int info_len)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, type, root_val);
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		if(err_info)
			strncpy(err_info, p_rsp->return_str, info_len);
		ret = -1;
	}
	if(cb)
		cb(p_rsp);
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_cmd_do_simple_notify(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len)
{
	return silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_NOTIFY, path, root_val, NULL, err_info, info_len);
}

int silc_cli_cmd_do_simple_action(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len)
{
	return silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_ACTION, path, root_val, NULL, err_info, info_len);
}

int silc_cli_cmd_do_simple_modify(const silc_cstr path, const silc_cstr root_val, silc_cstr err_info, int info_len)
{
	return silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_MODIFY, path, root_val, NULL, err_info, info_len);
}

int silc_cli_cmd_get_simple_node(const silc_cstr path, silc_cstr buff)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		ret = -1;
		silc_cli_err_cmd_set_err_info("%s", p_rsp->return_str);
		goto OUT;
	}

	if(p_rsp->p_node_root->val_str)
		strcpy(buff, p_rsp->p_node_root->val_str);
	else
		ret = -1;

OUT:
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_get_mgmtd_bool_node_value(const silc_cstr path, silc_bool *p_bool)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
		ret = -1;
	else
	{
		if(strcmp(p_rsp->p_node_root->val_str, "false") == 0)
			*p_bool = silc_false;
		else
			*p_bool = silc_true;
	}

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_get_mgmtd_int_node_value(const silc_cstr path, int *p_int)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
		ret = -1;
	else
	{
		long val;
		ret = silc_mgmtd_if_cstr2l(p_rsp->p_node_root->val_str, &val);
		if(ret == 0)
			*p_int = val;
	}

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return ret;
}

int silc_cli_get_mgmtd_node_child_list(silc_cstr path, char* output_val_buf, int str_len)
{
	int count = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	silc_mgmtd_if_node *p_node;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY_CHILD, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return 0;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return 0;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
	}
	else
	{
		silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
		{
			sprintf(output_val_buf+count*str_len, "%s", p_node->name);
			count++;
		}
	}
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return count;
}

#define	IS_NETCONF_PORT		830

static silc_cli_login_info s_silc_cli_login_info;

int silc_cli_get_current_login_info(silc_cli_login_info* p_info)
{
	int i;
	int buf_len = 4000;
	char buf[4000];
	silc_cstr line, val_str;
	silc_cstr cmd = "env";
	silc_cstr_array* p_arr;

	if(silc_mgmtd_if_exec_system_cmd(cmd, buf, &buf_len, 2000, silc_false) != 0)
		return -1;

	p_arr = silc_cstr_split(buf, "\n");
	if(!p_arr)
		return -1;

	memset(p_info, 0, sizeof(*p_info));
	p_info->type = SILC_MGMTD_IF_CLIENT_LOCAL;
	for(i=0; i<p_arr->length; i++)
	{
		line = silc_cstr_array_get_quick(p_arr, i);
		if((0 == strncmp(line, "SSH_CLIENT=", strlen("SSH_CLIENT="))))
		{
			silc_cstr ip = NULL, port = NULL, listen_port = NULL;
			p_info->type = SILC_MGMTD_IF_CLIENT_SSH;
			ip = line + strlen("SSH_CLIENT=");
			val_str = strchr(ip, ' ');
			if(!val_str)
				goto ERROR_OUT;
			*val_str = 0;
			port = val_str + 1;
			val_str = strchr(port, ' ');
			if(!val_str)
				goto ERROR_OUT;
			*val_str = 0;
			listen_port = val_str + 1;
			if(atoi(listen_port) == IS_NETCONF_PORT)
				p_info->type = SILC_MGMTD_IF_CLIENT_NETCONF;
			strcpy(p_info->ip, ip);
			strcpy(p_info->port, port);
		}
		else if((0 == strncmp(line, "USER=", strlen("USER="))))
		{
			if (strlen(p_info->name) == 0)
			{
				// if the name is not set by LOGIN_USER
				val_str = line + strlen("USER=");
				strcpy(p_info->name, val_str);
			}
		}
		else if((0 == strncmp(line, "LOGIN_USER=", strlen("LOGIN_USER="))))
		{
			val_str = line + strlen("LOGIN_USER=");
			strcpy(p_info->name, val_str);
		}
		else if((0 == strncmp(line, "USER_PRIVILEGE=", strlen("USER_PRIVILEGE="))))
		{
			val_str = line + strlen("USER_PRIVILEGE=");
			strcpy(p_info->privil, val_str);
			silc_mgmtd_if_trans_privil(p_info->privil);
		}
		else if((0 == strncmp(line, "AUTH_PROTOCOL=", strlen("AUTH_PROTOCOL="))))
		{
			val_str = line + strlen("AUTH_PROTOCOL=");
			strcpy(p_info->protocol, val_str);
		}
	}
	silc_cstr_array_destroy(p_arr);
	return 0;
ERROR_OUT:
	silc_cstr_array_destroy(p_arr);
	return -1;
}

int silc_cli_set_connect_login_info()
{
	silc_cli_login_info* p_info = &s_silc_cli_login_info;
	silc_cstr name = NULL, ip = NULL, port = NULL, privil=NULL, protocol=NULL;

	memset(p_info, 0x0, sizeof(silc_cli_login_info));
	if(silc_cli_get_current_login_info(p_info) != 0)
		return -1;

	if(strlen(p_info->name))
		name = p_info->name;
	if(strlen(p_info->ip))
		ip = p_info->ip;
	if(strlen(p_info->port))
		port = p_info->port;
	if(strlen(p_info->privil))
		privil = p_info->privil;
	if(strlen(p_info->protocol))
		protocol = p_info->protocol;

	return silc_mgmtd_if_client_set_login_info_ex(p_info->type, name, ip, port, privil, protocol);
}

silc_cstr silc_cli_get_connect_login_name()
{
	return s_silc_cli_login_info.name;
}

volatile uint64_t g_last_exec_time_ms = 0;
volatile uint64_t g_silc_cli_timeout_ms = 0;
volatile uint64_t g_silc_cli_timeout_enable = 0;

uint32_t silc_cli_session_timeout_curr_get()
{
	return g_silc_cli_timeout_ms/1000;
}

void silc_cli_session_timeout_set( uint32_t timeout_sec)
{
	g_silc_cli_timeout_ms = timeout_sec * 1000;
}

void silc_cli_session_timeout_update()
{
	g_last_exec_time_ms = silc_time_get_ms();
}

int silc_cli_session_timeout_config_update(void)
{
	int mgmtd_timeout;

	if(silc_mgmtd_if_client_get_mgmtd_session_timeout(&mgmtd_timeout) !=0)
		return -1;

	if(mgmtd_timeout <0)
		mgmtd_timeout = 0;
	silc_cli_session_timeout_set(mgmtd_timeout);
	return 0;
}
void silc_cli_session_timeout_enable(void)
{
	g_silc_cli_timeout_enable = 1;
}
void silc_cli_session_timeout_disable(void)
{
	g_silc_cli_timeout_enable = 0;
}
int silc_cli_session_timeout_status(void)
{
	return g_silc_cli_timeout_enable;;
}

void* silc_cli_session_timeout_thread(void* p_arg)
{
	uint64_t curr_ms;

	while(1)
	{
		//sleep for 10s;
		if(g_silc_cli_timeout_ms && silc_cli_session_timeout_status())
		{
			curr_ms = silc_time_get_ms();
			if(g_last_exec_time_ms + g_silc_cli_timeout_ms < curr_ms)
			{
				printf("Connection timed out, exiting...\n");
				exit(0);
			}
		}
		silc_time_sleep(10, 0);
	}
}



int silc_cli_session_timeout_init(void)
{
	silc_cli_session_timeout_enable();
	if(0!=silc_cli_session_timeout_config_update())
		return -1;
	silc_cli_session_timeout_update();
	if(0!=silc_thread_create_detached(silc_cli_session_timeout_thread, NULL))
		return -1;
	return 0;
}

/*
 * format is x,x,...,x;...
 */
void silc_cli_complex_string_display(silc_cstr string,
		const silc_cstr info, const silc_cstr* item_names, const silc_cstr* item_formats, int item_num)
{
	int i, j;
	silc_cstr_array* p_arr;
	silc_bool print_head = silc_true;

	silc_cli_print("%s\n", info);

	p_arr = silc_cstr_split(string, ";");

	for(i=0; i<p_arr->length; i++)
	{
		silc_cstr_array* p_sub_arr;
		silc_cstr line = silc_cstr_array_get_quick(p_arr, i);

		p_sub_arr = silc_cstr_split(line, ",");
		if(!p_sub_arr)
			continue;

		if(print_head)
		{
			for(j=0; j<item_num; j++)
				silc_cli_print(item_formats[j], item_names[j]);
			silc_cli_print("\n");
			print_head = silc_false;
		}

		for(j=0; j<item_num; j++)
		{
			if(j < p_sub_arr->length)
				silc_cli_print(item_formats[j], silc_cstr_array_get_quick(p_sub_arr, j));
			else
				silc_cli_print(item_formats[j], "");
		}
		silc_cli_print("\n");
		silc_cstr_array_destroy(p_sub_arr);
	}
	silc_cli_print("\n");

	if(print_head)
		silc_cli_print("None\n\n");
	silc_cstr_array_destroy(p_arr);
}

int silc_cli_show_complex_string_query(const silc_cstr path,
		const silc_cstr info, const silc_cstr* item_names, const silc_cstr* item_formats, int item_num)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return 4;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return 3;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
		silc_cli_print("Remote error: %s.\n", p_rsp->return_str);
	    return 5;
	}

	silc_cli_complex_string_display(p_rsp->p_node_root->val_str,
			info, item_names, item_formats, item_num);

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}

int silc_cli_show_simple_string_query(const silc_cstr path, const silc_cstr format)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return 4;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return 3;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
		silc_cli_print("Remote error: %s.\n", p_rsp->return_str);
	    return 5;
	}

	silc_cli_print(format, p_rsp->p_node_root->val_str);
	silc_cli_print("\n");

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}

void silc_cli_simple_node_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, silc_cstr* trans_list, int trans_num)
{
	int i;
	silc_cstr val_str = p_node->val_str;

	for(i=0; i<trans_num; i++)
	{
		if(strcmp(trans_list[i*2], val_str) == 0)
		{
			val_str = trans_list[i*2+1];
		}
	}
	silc_cli_print("%s\n%s\n\n", info, val_str);
}

int silc_cli_show_simple_trans_query(const silc_cstr path, const silc_cstr format, const silc_cstr* trans, int trans_num)
{
	int i;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	silc_cstr val_str;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
	    return -1;
	}

	val_str = p_rsp->p_node_root->val_str;
	for(i=0; i<trans_num*2; i++)
	{
		if(strcmp(val_str, trans[i]) == 0)
		{
			val_str = trans[i+1];
			break;
		}
		i++;
	}
	silc_cli_print(format, val_str);
	silc_cli_print("\n\n");

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}


void silc_cli_l2tree_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr* item_formats, silc_cstr** item_trans, int* item_trans_num, int item_num)
{
	int i, j, num = 20;
	silc_mgmtd_if_node *p_sub_node, *p_ss_node;
	silc_cstr val_arr[item_num];

	memset(val_arr, 0, sizeof(val_arr));

	silc_cli_print("%s\n", info);
	if(silc_list_empty(&p_node->sub_node_list))
	{
		silc_cli_print("None\n\n");
		return;
	}

	// print the title
	for(i=0; i<item_num; i++)
	{
		if(!item_names[i])
			continue;
		silc_cli_print(item_formats[i], item_names[i]);
		if(i && i%num == 0 && (i+1) != item_num)
		{
			silc_cli_print("\n");
			silc_cli_print(item_formats[0], "");
		}
	}
	silc_cli_print("\n");

	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		memset(val_arr, 0x0, sizeof(silc_cstr)*item_num);
		if(!item_nodes[0])
			val_arr[0] = p_sub_node->name;
		silc_list_for_each_entry(p_ss_node, &p_sub_node->sub_node_list, node)
		{
			for(i=0; i<item_num; i++)
			{
				if(item_nodes[i] && strcmp(p_ss_node->name, item_nodes[i]) == 0)
				{
					val_arr[i] = p_ss_node->val_str;
					break;
				}
			}
		}
		for(i=0; i<item_num; i++)
		{
			if(!val_arr[i])
				val_arr[i] = "";
			if(item_trans)
			{
				silc_cstr* trans = item_trans[i];
				if(trans)
				{
					for(j=0; j<item_trans_num[i]*2; j++)
					{
						if(strcmp(val_arr[i], trans[j]) == 0)
						{
							val_arr[i] = trans[j+1];
							break;
						}
						j++; // skip the translated value
					}
				}
			}
		}
		for(i=0; i<item_num; i++)
		{
			if(!item_names[i])
				continue;
			silc_cli_print(item_formats[i], val_arr[i]);
			if(i && i%num == 0 && (i+1) != item_num)
			{
				silc_cli_print("\n");
				silc_cli_print(item_formats[0], "");
			}
		}
		silc_cli_print("\n");
	}
	silc_cli_print("\n");
}

int silc_cli_show_l2tree_query(const silc_cstr path,
		const silc_cstr info, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr* item_formats, silc_cstr** item_trans, int* item_trans_num, int item_num)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY_SUB, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
	    return -1;
	}

	silc_cli_l2tree_display(p_rsp->p_node_root,
			info, item_nodes, item_names, item_formats, item_trans, item_trans_num, item_num);

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}

void silc_cli_l1tree_list_display(silc_mgmtd_if_node *p_node,
		const silc_cstr info, const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num, is_cli_display_cb cb)
{
	int i, j;
	silc_mgmtd_if_node *p_sub_node;
	silc_cstr val_str, trans_str;
	char buf[256];

	silc_cli_print("%s\n", info);
	if(silc_list_empty(&p_node->sub_node_list))
	{
		silc_cli_print("None\n\n");
		return;
	}

	// print the title
	//silc_cli_print(item_format, titles[0], titles[1]);
	//silc_cli_print("\n");

	for(i=0; i<item_num; i++)
	{
		if(!item_names[i])
			continue;
		silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
		{
			if(strcmp(p_sub_node->name, item_nodes[i]) == 0)
			{
				val_str = p_sub_node->val_str;
				if(item_trans)
				{
					silc_cstr* trans = item_trans[i];
					if(trans)
					{
						for(j=0; j<item_trans_num[i]*2; j++)
						{
							if(strcmp(val_str, trans[j]) == 0)
							{
								val_str = trans[j+1];
								break;
							}
							j++; // skip the translated value
						}
					}
				}
				if(cb)
				{
					trans_str = cb(p_sub_node->name, val_str, buf, 100);
					if(trans_str)
						val_str = trans_str;
				}
				silc_cli_print(item_format, item_names[i], val_str);
				silc_cli_print("\n");
				break;
			}
		}
	}
	silc_cli_print("\n");
}

int silc_cli_show_l1tree_list_query(const silc_cstr path,
		const silc_cstr info, const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY_CHILD, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
	    return -1;
	}

	silc_cli_l1tree_list_display(p_rsp->p_node_root,
			info, titles, item_nodes, item_names, item_format, item_trans, item_trans_num, item_num, NULL);

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}

void silc_cli_l2tree_list_display(silc_mgmtd_if_node *p_node, const silc_cstr info_format,
		const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num)
{
	int i, j;
	silc_mgmtd_if_node *p_sub_node, *p_ss_node;
	silc_cstr val_str;

	if(silc_list_empty(&p_node->sub_node_list))
	{
		silc_cli_print("\n\n");
		return;
	}

	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		silc_cli_print(info_format, p_sub_node->name);
		// print the title
		silc_cli_print(item_format, titles[0], titles[1]);
		silc_cli_print("\n");
		silc_list_for_each_entry(p_ss_node, &p_sub_node->sub_node_list, node)
		{
			for(i=0; i<item_num; i++)
			{
				if(strcmp(p_ss_node->name, item_nodes[i]) == 0)
				{
					val_str = p_ss_node->val_str;
					if(item_trans)
					{
						silc_cstr* trans = item_trans[i];
						if(trans)
						{
							for(j=0; j<item_trans_num[i]*2; j++)
							{
								if(strcmp(val_str, trans[j]) == 0)
								{
									val_str = trans[j+1];
									break;
								}
								j++; // skip the translated value
							}
						}
					}
					silc_cli_print(item_format, item_names[i], val_str);
					silc_cli_print("\n");
					break;
				}
			}
		}
		silc_cli_print("\n");
	}
	silc_cli_print("\n");
}

int silc_cli_show_l2tree_list_query(const silc_cstr path, const silc_cstr info_format,
		const silc_cstr* titles, const silc_cstr* item_nodes, const silc_cstr* item_names,
		const silc_cstr item_format, silc_cstr** item_trans, int* item_trans_num, int item_num)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_QUERY_SUB, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}
	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
	    return -1;
	}

	silc_cli_l2tree_list_display(p_rsp->p_node_root,
			info_format, titles, item_nodes, item_names, item_format, item_trans, item_trans_num, item_num);

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);;
    return 0;
}

int silc_cli_modify_static_node(silc_mgmtd_if_req **pp_req, const silc_cstr path, const silc_cstr val_str)
{
	silc_mgmtd_if_req *p_req;

	p_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_MODIFY, val_str);
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	*pp_req = p_req;
    return 0;
}

int silc_cli_cmd_split_mod_seg(silc_cstr data, silc_cstr mod, silc_cstr seg)
{
	int cur = 0;
	char buff[20];

	strcpy(buff, data);

	for(cur=0; cur<strlen(buff); cur++)
	{
		if(buff[cur] == '.')
		{
			buff[cur] = 0;
			strcpy(mod, buff);
			strcpy(seg, buff+cur+1);
			return 0;
		}
	}
	silc_cli_err_cmd_set_err_info("invalid segment ID(%s)", data);
	return -1;
}

int silc_cli_cmd_split_mod_seg_port(silc_cstr data, silc_cstr mod, silc_cstr seg, silc_cstr port)
{
	int cur = 0, idx = 0;
	int len;
	char buff[20];

	strcpy(buff, data);
	len = strlen(buff);

	for(cur=0; cur<len; cur++)
	{
		if(buff[cur] == '.')
		{
			buff[cur] = 0;
			if(idx == 0)
			{
				strcpy(mod, buff);
				idx = cur + 1;
			}
			else if(idx != 0)
			{
				strcpy(seg, buff+idx);
				strcpy(port, buff+cur+1);
				return 0;
			}
		}
	}
	silc_cli_err_cmd_set_err_info("invalid port ID(%s)", data);
	return -1;
}

silc_cli_token* is_cli_cmd_get_next_rl_token(silc_list* p_token_list, silc_cli_token* p_token)
{
	if(p_token->rl_node.next == p_token_list) // hit the end
		return NULL;
	return silc_list_entry(p_token->rl_node.next, silc_cli_token, rl_node);
}


int is_cli_cmd_do_request_core(is_cli_cmd_req_info* p_req_info, silc_list* p_token_list, int timeout_sec)
{
	int loop;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	silc_cli_token* p_token;
	silc_cli_token* p_tmp_token;
	silc_cstr node_val, node_name;
	silc_cstr_array* p_arr;

	p_req = silc_mgmtd_if_req_create(p_req_info->path, p_req_info->type, p_req_info->root_val);
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(!p_token->map_name)
			continue;

		node_val = p_token->val_str;
		if(p_token->type == TOKEN_TYPE_STATIC)
		{
			silc_bool find_val = silc_false;
			if(p_token->map_val)
			{
				node_val = p_token->map_val;
				find_val = silc_true;
			}
			else
			{
				p_tmp_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
				if(p_tmp_token && p_tmp_token->type == TOKEN_TYPE_STATIC && !p_tmp_token->map_name)
				{
					if(p_tmp_token->map_val)
						node_val = p_tmp_token->map_val;
					else
						node_val = p_tmp_token->name;
					find_val = silc_true;
				}
			}
			if(!find_val)
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
		}

		p_arr = silc_cstr_split(p_token->map_name, ",");
		if(!p_arr)
		{
			silc_cli_err_cmd_set_err_info("Split mapped node name failed");
			return -1;
		}
		for(loop=0; loop<p_arr->length; loop++)
		{
			node_name = silc_cstr_array_get_quick(p_arr, loop);
			if(!silc_mgmtd_if_req_add_node_ext(p_req->p_node_root, node_name, node_val))
			{
				silc_cli_err_cmd_set_req_add_node_failed(node_name, node_val);
				silc_mgmtd_if_req_delete(p_req);
				silc_cstr_array_destroy(p_arr);
				return -1;
			}
		}
		silc_cstr_array_destroy(p_arr);
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, timeout_sec);
	if(!p_msg)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_req_delete(p_req);
		silc_mgmtd_if_client_free_msg(p_msg);;
		return -1;
	}

	if(p_req_info->rsp_cb)
		p_req_info->rsp_cb(p_rsp);

	silc_mgmtd_if_client_free_msg(p_msg);;
	silc_mgmtd_if_req_delete(p_req);
    return 0;
}

int is_cli_cmd_do_request(is_cli_cmd_req_info* p_req_info, silc_list* p_token_list)
{
	return is_cli_cmd_do_request_core(p_req_info, p_token_list, 10);
}

int silc_cli_do_cmd(silc_cli_cmd* p_cmd, silc_list* p_token_list)
{
	if(p_cmd->cmd_func(p_token_list) != 0)
	{
		silc_cli_print("%% Return error: %s!\n", silc_cli_err_cmd_get_err_info());
		return -1;
	}
	return 0;
}

int silc_cli_upload_file(silc_cstr url, silc_cstr user, silc_cstr passwd, silc_cstr filename)
{
	char cmd[512];

	srand(time(NULL));
	if(!filename[0])
		sprintf(filename, "/tmp/is_upload_%u_%d", (uint32_t)time(NULL), rand());
	// upload first
	if(strncmp(url, "ftp://", strlen("ftp://")) == 0)
	{
		if(!user)
		{
			silc_cli_err_cmd_set_err_info("need user name");
			return -1;
		}
		if(!passwd)
		{
			silc_cli_err_cmd_set_err_info("need user password");
			return -1;
		}
		silc_cli_print("Upload file from FTP %s.\n", url);
		sprintf(cmd, "curl -# -u %s:%s -o %s %s", user, passwd, filename, url);
	}
	else if(strncmp(url, "http://", strlen("http://")) == 0)
	{
		silc_cli_print("Upload file from HTTP %s.\n", url);
		sprintf(cmd, "ret=$(curl --write-out %%{http_code} -f -# -o %s %s); [ ${ret} == 200 ]", filename, url);
	}
	else if(strncmp(url, "https://", strlen("https://")) == 0)
	{
		silc_cli_print("Upload file from HTTPS %s.\n", url);
		sprintf(cmd, "ret=$(curl --write-out %%{http_code} -f -# -o %s -k %s); [ ${ret} == 200 ]", filename, url);
	}
	else if(strncmp(url, "scp://", strlen("scp://")) == 0)
	{
		silc_cli_print("Upload file from SCP %s.\n", url);
		sprintf(cmd, "scp %s %s", url+strlen("scp://"), filename);
	}
	else
	{
		silc_cli_err_cmd_set_err_info("Invalid url");
		return -1;
	}
	if(silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 1000000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Fail to get %s", url);
		return -1;
	}
	return 0;
}

int silc_cli_cmd_confirm(silc_cstr prompt, silc_cstr y_msg, silc_cstr n_msg)
{
	int ch;

SELECT:
	if (prompt)
		silc_cli_print(prompt);
	ch = getchar();
	while(getchar() != '\n');
	if(ch == 'y')
	{
		if (y_msg)
			silc_cli_print(y_msg);
		return 0;
	}
	if(ch == 'n')
	{
		if (n_msg)
			silc_cli_print(n_msg);
		return -1;
	}
	else
		goto SELECT;

	return -1;
}

int silc_cli_show_cmd_output(silc_cstr cmd)
{
	static char output[4096];
	int output_len= 4096;

	output[0] = 0;
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &output_len, 1000, silc_false) != 0)
	{
		silc_cli_print("exec cmd '%s' error!\n", cmd);
		return -1;
	}
	silc_cli_print("%s", output);
	return 0;
}

static void gen_random(silc_cstr s, const int len)
{
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.";
	int i;

	for (i = 0; i < len; ++i)
	{
		s[i] = alphanum[rand() % 63];
	}
	s[len] = 0;
}

#define IS_TMP_LOG_FILE		"/tmp/silc_tmp.log"

int silc_cli_show_log(silc_cstr log, silc_cstr filter)
{
	char filename[32];
	char tmp[16];
	char cmd[256];

	gen_random(tmp, 8);
	sprintf(filename, IS_TMP_LOG_FILE".%s", tmp);
	sprintf(cmd, "for f in $(ls -rt %s*); do cat $f >> %s; done", log, filename);
	system(cmd);

	if (filter)
		sprintf(cmd, "grep '%s' %s|less", filter, filename);
	else
		sprintf(cmd, "less %s", filename);
	system(cmd);

	unlink(filename);
	system("rm -rf "IS_TMP_LOG_FILE"*");

	return 0;
}
