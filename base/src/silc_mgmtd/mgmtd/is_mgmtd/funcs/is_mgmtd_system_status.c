/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_vnode.h"
#include "silc_mgmtd_config.h"

silc_cstr_array* is_mgmtd_system_get_interface();

static silc_list s_is_mgmtd_system_session_info_list;

int is_mgmtd_system_status_get_current_clock(silc_cstr buf, int buf_len)
{
	int ret;
	ret = silc_mgmtd_if_exec_system_cmd("date", buf, &buf_len, 1000, silc_false);
	if(ret == 0)
		buf[strlen(buf)-1] = 0;
	return ret;
}

int is_mgmtd_system_status_get_current_uptime(silc_cstr buf, int buf_len)
{
	int len = 100;
	char out[len];
	int ret;
	char* start;
	ret = silc_mgmtd_if_exec_system_cmd("uptime", out, &len, 1000, silc_false);
	if(ret)
		return ret;
	out[strlen(out)-1] = 0;
	start = strstr(out, "up");
	strcpy(buf, start);
	return 0;
}

int is_mgmtd_system_status_get_cpu_number(long* cpus)
{
	int len = 100;
	char out[len];
	int ret;
	long num;
	char *num_str;

	ret = silc_mgmtd_if_exec_system_cmd("cat /proc/cpuinfo|grep processor|wc -l", out, &len, 1000, silc_false);
	if(ret)
		return ret;
	out[strlen(out)-1] = 0;
	num_str = &out[0];
	ret = silc_mgmtd_if_cstr2l(num_str, &num);
	if(ret)
		return ret;
	*cpus = num;
	return 0;
}

int is_mgmtd_system_status_get_cpu_single_info(char* name, int index, long* p_usage)
{
	int len = 100;
	char out[len];
	int ret;
	char cmd[200];
	long d1, d2;
	char *d1_str, *d2_str;

	sprintf(cmd, "tail /tmp/cpu_%s.log -n 2|grep -v Average|grep -v Linux|grep -v CPU|awk '{print $%d}'|tail -n 1", name, index);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	if(strlen(out) < 4)	//output should be format like x.xx
		return -1;
	out[strlen(out)-1] = 0;
	d1_str = &out[0];
	d2_str = strstr(out, ".");
	if(!d2_str)
		return -1;
	*d2_str = 0;
	d2_str++;
	ret = silc_mgmtd_if_cstr2l(d1_str, &d1);
	if(ret)
		return ret;
	if(d2_str[0] == '0')
		d2_str++;
	ret = silc_mgmtd_if_cstr2l(d2_str, &d2);
	if(ret)
		return ret;
//	*p_usage = d1*100 + d2;
	*p_usage = d1;
	return 0;
}

int is_mgmtd_system_status_get_cpu_info(char* name, char* p_user, char* p_sys, char* p_idle)
{
	int len = 200;
	char out[len];
	int ret;
	char cmd[100];
	silc_cstr_array* p_array = NULL;

	if(strcmp(name, "all") == 0)
		sprintf(cmd, "mpstat|tail -n 1");
	else
		sprintf(cmd, "mpstat -P %s|tail -n 1", name);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	p_array = silc_cstr_split(out, " ");
	if(!p_array)
		return -1;
	if(p_array->length >= 12)
	{
		strcpy(p_user, silc_cstr_array_get_quick(p_array, 2));
		strcpy(p_sys, silc_cstr_array_get_quick(p_array, 4));
		strcpy(p_idle, silc_cstr_array_get_quick(p_array, 11));
	}
	silc_cstr_array_destroy(p_array);
	return 0;
}


int is_mgmtd_system_status_get_memory_single_info(char* name, int index, long* p_size)
{
	int len = 100;
	char out[len];
	int ret;
	char cmd[100];

	sprintf(cmd, "free|grep '%s'|awk '{print $%d}'", name, index);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	out[strlen(out)-1] = 0;
	ret = silc_mgmtd_if_cstr2l(out, p_size);
	if(ret)
		return ret;
	return 0;
}

int is_mgmtd_system_status_get_memory_info(char* name, char* p_total, char* p_used, char* p_free)
{
	int len = 200;
	char out[len];
	int ret;
	char cmd[100];
	silc_cstr_array* p_array = NULL;

	sprintf(cmd, "free|grep '%s'", name);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	p_array = silc_cstr_split(out, " ");
	if(!p_array)
		return -1;
	if(p_array->length >= 4)
	{
		strcpy(p_total, silc_cstr_array_get_quick(p_array, 1));
		strcpy(p_used, silc_cstr_array_get_quick(p_array, 2));
		strcpy(p_free, silc_cstr_array_get_quick(p_array, 3));
	}
	silc_cstr_array_destroy(p_array);
	return 0;
}

int is_mgmtd_system_status_get_storage_single_info(char* path, int index, long* p_size)
{
	int len = 100;
	char out[len];
	int ret;
	char cmd[100];

	sprintf(cmd, "df '%s'| grep '%s' | awk '{print $%d}'", path, path, index);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	out[strlen(out)-1] = 0;
	ret = silc_mgmtd_if_cstr2l(out, p_size);
	if(ret)
		return ret;
	return 0;
}

int is_mgmtd_system_status_get_storage_info(char* name, char* p_total, char* p_used, char* p_avail)
{
	char path[100];
	int len = 200;
	char out[len];
	int ret, i;
	char cmd[100];
	silc_cstr_array* p_array = NULL;

	strcpy(path, name);
	for(i=0; i<strlen(path); i++)
	{
		if(path[i] == '*')
			path[i] = '/';
	}

	sprintf(cmd, "df '%s'| grep '%s'", path, path);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false);
	if(ret)
		return ret;
	p_array = silc_cstr_split(out, " ");
	if(!p_array)
		return -1;
	if(p_array->length >= 4)
	{
		strcpy(p_total, silc_cstr_array_get_quick(p_array, 1));
		strcpy(p_used, silc_cstr_array_get_quick(p_array, 2));
		strcpy(p_avail, silc_cstr_array_get_quick(p_array, 3));
	}
	silc_cstr_array_destroy(p_array);
	return ret;
}

#define IS_SYS_EVT_FILE	"/var/log/message-sys-event"

silc_cstr is_mgmtd_system_status_get_event()
{
	silc_cstr evt_str = NULL;
	FILE* fp;
	struct stat st;
	int len;

	if(access(IS_SYS_EVT_FILE, F_OK) != 0)
	{
		return evt_str;
	}
	stat(IS_SYS_EVT_FILE, &st);
	len = st.st_size;
	evt_str = (silc_cstr)malloc(len+1);
	if (!evt_str)
	{
		SILC_ERR("Malloc msg error!!!!\n");
		return evt_str;
	}
	fp = fopen(IS_SYS_EVT_FILE, "r");
	fread(evt_str, len, 1, fp);
	fclose(fp);
	evt_str[len] = 0;

	return evt_str;
}

int is_mgmtd_system_status_get_interface_info(char* name, char* info, int len)
{
	char cmd[64];
	sprintf(cmd, "ifconfig %s", name);
	silc_mgmtd_if_exec_system_cmd(cmd, info, &len, 1000, silc_false);
	return 0;
}

int is_mgmtd_system_status_level_get_level_5(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[1024];
	const silc_cstr grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 2);
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);

	if(strcmp(grand_parent_name, "interface-list") == 0)
	{
		static char info[1024];
		int len=1024;
		if(is_mgmtd_system_status_get_interface_info(parent_name, info, len) == 0)
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "info", info, str, silc_false);
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

#define CMD_OUTPUT_BUF_LEN	4096
int is_mgmtd_system_status_level_get_level_4(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[1024];
	const silc_cstr grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 2);
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);
	int len = CMD_OUTPUT_BUF_LEN;
	static char out[CMD_OUTPUT_BUF_LEN];

	if(strcmp(grand_parent_name, "session") == 0)
	{
		silc_mgmtd_if_session_info* p_sess;
		silc_list_for_each_entry(p_sess, &s_is_mgmtd_system_session_info_list, node)
		{
			if(strcmp(parent_name, p_sess->index) == 0)
			{
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "user", p_sess->login_user, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "type", p_sess->type, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "login-time", p_sess->login_time, str, silc_false);
				if(p_sess->login_ip[0])
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "login-ip", p_sess->login_ip, str, silc_false);
				if(p_sess->login_port)
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "login-port", p_sess->login_port, int32, silc_false);
			}
		}
	}
	else if(strcmp(grand_parent_name, "cpu") == 0)
	{
		char user[32], sys[32], idle[32];
		if(is_mgmtd_system_status_get_cpu_info(parent_name, user, sys, idle) == 0)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "user", user, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "sys", sys, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "idle", idle, str, silc_false);
		}
	}
	else if(strcmp(grand_parent_name, "memory") == 0)
	{
		char total[32], used[32], free[32];
		if(is_mgmtd_system_status_get_memory_info(parent_name, total, used, free) == 0)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "total", total, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "used", used, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "free", free, str, silc_false);
		}
	}
	else if(strcmp(grand_parent_name, "storage") == 0)
	{
		char total[32], used[32], free[32];
		if(is_mgmtd_system_status_get_storage_info(parent_name, total, used, free) == 0)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "total", total, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "used", used, str, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "free", free, str, silc_false);
		}
	}
	else if(strcmp(parent_name, "interface-list") == 0)
	{
		if(!silc_mgmtd_memdb_get_product_info()->multi_eth_support)
		{// single interface in system
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "eth0", "eth0", str, silc_true);
		}
		else
		{//multiple interfaces
			silc_mgmtd_node *p_list, *p_node;
			silc_cstr inf;
			p_list = silc_mgmtd_memdb_find_node("/config/system/mgmt/interface-list");
			silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
			{
				if (p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
					continue;
				inf = p_node->value.val.string_val;
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, inf, inf, str, silc_true);
			}
		}
	}
	else if(strcmp(parent_name, "dhcp") == 0)
	{
		if(0 == silc_mgmtd_if_exec_system_cmd("ps -ef|grep dhclient|grep -v grep", NULL, NULL, 1000, silc_false))
		{
			silc_cstr leases_file = "/tmp/dhclient.leases";
			silc_cstr buf = NULL;
			uint32_t buf_len=0;

			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "state", silc_true, bool, silc_false);
			if(0 == silc_file_read_all(leases_file, &buf, &buf_len))
			{
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "leases", buf, str, silc_false);
				free(buf);
			}
			else
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "leases", "N/A", str, silc_false);
		}
		else
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "state", silc_false, bool, silc_false);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "leases", "", str, silc_false);
		}
	}
	else if(strcmp(parent_name, "ipsec") == 0)
	{
		len = CMD_OUTPUT_BUF_LEN;
		if(0 == silc_mgmtd_if_exec_system_cmd("ipsec status", out, &len, 1000, silc_false))
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "connection", out, str, silc_false);
		else
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "connection", "IPSEC not started", str, silc_false);
	}
	else if(strcmp(parent_name, "iptables") == 0)
	{
		len = CMD_OUTPUT_BUF_LEN;
		silc_mgmtd_if_exec_system_cmd("iptables -S", out, &len, 1000, silc_false);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "iptables-s", out, str, silc_false);

		len = CMD_OUTPUT_BUF_LEN;
		silc_mgmtd_if_exec_system_cmd("iptables -L --line-numbers", out, &len, 1000, silc_false);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "iptables-l", out, str, silc_false);

		len = CMD_OUTPUT_BUF_LEN;
		silc_mgmtd_if_exec_system_cmd("ip6tables -S", out, &len, 1000, silc_false);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "ip6tables-s", out, str, silc_false);

		len = CMD_OUTPUT_BUF_LEN;
		silc_mgmtd_if_exec_system_cmd("ip6tables -L --line-numbers", out, &len, 1000, silc_false);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "ip6tables-l", out, str, silc_false);
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_system_status_level_get_level_3(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);

	if(strcmp(parent_name, "session") == 0)
	{
		int cnt = 0;
		silc_mgmtd_if_session_info* p_sess;
		if(silc_mgmtd_if_server_get_online_sessions_info(&s_is_mgmtd_system_session_info_list) != 0)
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		silc_list_for_each_entry(p_sess, &s_is_mgmtd_system_session_info_list, node)
		{
			cnt++;
			sprintf(p_sess->index, "%d", cnt);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, p_sess->index, p_sess->index, str, silc_true);
		}
	}

	if(strcmp(parent_name, "cpu") == 0)
	{
		int i;
		long cpu_num;

		if(is_mgmtd_system_status_get_cpu_number(&cpu_num) != 0)
			goto ERR_RET;
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "all", "all", str, silc_true);
		for(i=0; i<cpu_num; i++)
		{
			char name[8];
			sprintf(name, "%d", i);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, name, name, str, silc_true);
		}
	}

	if(strcmp(parent_name, "memory") == 0)
	{
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "Mem", "Mem", str, silc_true);
//		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "Swap", "Swap", str, silc_true);
	}

	if(strcmp(parent_name, "storage") == 0)
	{
		int i;
		silc_mgmtd_storage_path* p_path;

		//handle additional storage from product
		for (i = 0; i< silc_mgmtd_memdb_get_product_info()->storage_cnt; i++)
		{
			p_path = &silc_mgmtd_memdb_get_product_info()->storage_list[i];
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, p_path->name, p_path->path, str, silc_true);
		}

		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "*var*log", "/var/log", str, silc_true);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "*config", "/config", str, silc_true);
	}

	if(strcmp(parent_name, "mgmt") == 0)
	{
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "interface-list", "interface-list", str, silc_true);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "dhcp", "dhcp", str, silc_true);

		if(silc_mgmtd_memdb_get_product_info()->iptables_support)
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "iptables", "iptables", str, silc_true);
		if(silc_mgmtd_memdb_get_product_info()->ipsec_support)
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "ipsec", "ipsec", str, silc_true);
	}
	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_system_status_level_get_level_2(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	char clock_str[100];
	char uptime_str[100];
	int running_config_len = 1024*256;
	silc_cstr running_config = NULL;
	silc_cstr evt_str = NULL;

	if(p_match_node == NULL || strcmp(p_match_node, "config-saved") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "config-saved", silc_mgmtd_memdb_is_config_saved(), bool, silc_false);

	if(p_match_node == NULL || strcmp(p_match_node, "time") == 0)
	{
		if(is_mgmtd_system_status_get_current_clock(clock_str, 100) != 0)
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "time", clock_str, str, silc_false);
	}

	if(p_match_node == NULL || strcmp(p_match_node, "uptime") == 0)
	{
		if(is_mgmtd_system_status_get_current_uptime(uptime_str, 100) != 0)
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "uptime", uptime_str, str, silc_false);
	}

	if(p_match_node == NULL || strcmp(p_match_node, "running-config") == 0)
	{
		silc_mgmtd_if_node* __p_ret_node = NULL;
		running_config = malloc(running_config_len);
		if(!running_config)
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		if(silc_mgmtd_memdb_get_running_config(running_config, running_config_len) != 0)
		{
			free(running_config);
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		}
		silc_mgmtd_vnode_set_node_value(__p_ret_node, p_parent_node, "running-config", running_config);
		__p_ret_node->has_child = silc_false;
	}

	if(p_match_node == NULL || strcmp(p_match_node, "config-file-list") == 0)
	{
		silc_mgmtd_if_node* __p_ret_node = NULL;
		running_config = malloc(running_config_len);
		if(!running_config)
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		if(silc_mgmtd_cfg_get_config_file_list(running_config, &running_config_len) != 0)
		{
			free(running_config);
			return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
		}
		silc_mgmtd_vnode_set_node_value(__p_ret_node, p_parent_node, "config-file-list", running_config);
		__p_ret_node->has_child = silc_false;
	}

	if(p_match_node == NULL || strcmp(p_match_node, "event") == 0)
	{
		evt_str = is_mgmtd_system_status_get_event();
		if(evt_str)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "event", evt_str, str, silc_false);
			free(evt_str);
		}
		else
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "event", "", str, silc_false);
		}
	}

	//these four have children nodes
	if(p_match_node == NULL || strcmp(p_match_node, "session") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "session", "session", str, silc_true);
	if(p_match_node == NULL || strcmp(p_match_node, "cpu") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "cpu", "cpu", str, silc_true);
	if(p_match_node == NULL || strcmp(p_match_node, "memory") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "memory", "memory", str, silc_true);
	if(p_match_node == NULL || strcmp(p_match_node, "storage") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "storage", "storage", str, silc_true);
	if(p_match_node == NULL || strcmp(p_match_node, "mgmt") == 0)
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "mgmt", "mgmt", str, silc_true);

	if(running_config)
		free(running_config);
	return 0;
ERR_RET:
	if(running_config)
		free(running_config);
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}


int is_mgmtd_system_status_level_get_level_1(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "system", "system", str, silc_true);
	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_system_status_level_get_level(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	switch(level)
	{
	case 1:
		return is_mgmtd_system_status_level_get_level_1(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 2:
		return is_mgmtd_system_status_level_get_level_2(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 3:
		return is_mgmtd_system_status_level_get_level_3(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 4:
		return is_mgmtd_system_status_level_get_level_4(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 5:
		return is_mgmtd_system_status_level_get_level_5(p_parent_node, level, p_level_arr, p_match_node, add_node);
	default:
		SILC_ERR("Too many levels %u for system status query", level);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
}


int is_mgmtd_system_status(silc_mgmtd_if_req_type type, void* p_db_node, void* data)
{
	int ret = 0;

	silc_list_init(&s_is_mgmtd_system_session_info_list);
	ret = silc_mgmtd_module_vnode_retrieve(type, p_db_node, data, is_mgmtd_system_status_level_get_level);
	silc_mgmtd_if_server_free_online_sessions_info(&s_is_mgmtd_system_session_info_list);
	return ret;
}


