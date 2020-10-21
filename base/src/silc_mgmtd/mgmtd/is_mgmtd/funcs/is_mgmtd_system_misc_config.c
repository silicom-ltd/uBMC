/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"

int is_write_file(silc_cstr path, silc_cstr buf, int len);

#define IS_MGMTD_SYSTEM_MISC_PATH	"/config/system/misc"

void is_mgmtd_get_mdf_node_val(const silc_cstr node_path, silc_cstr buff, int buff_len, silc_bool* modified);
int is_mgmtd_check_port(uint32_t port);

void is_mgmtd_system_mgmt_restart_interface();

int is_mgmtd_get_vrf_process_config(silc_cstr proc, silc_cstr* vrf_list, int len);
int is_mgmtd_run_vrf_process_cmd(silc_cstr proc, silc_cstr cmd);

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_system_misc_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("sys_misc", conn_entry);
}

#define IS_HOSTNAME_FILE		"/etc/hostname"
#define MAX_HOST_NAME_LEN		256

int is_mgmtd_system_misc_config_hostname(void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[MAX_HOST_NAME_LEN];
	char cmd[MAX_HOST_NAME_LEN+64];
	//FILE* fp;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/hostname", buff, MAX_HOST_NAME_LEN, &modified);
	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "Device name is configured: %s", buff);
#if 0
	fp = fopen(IS_HOSTNAME_FILE".bak", "w");
	fprintf(fp, "%s", buff);
	fflush(fp);
	fclose(fp);
	rename(IS_HOSTNAME_FILE".bak", IS_HOSTNAME_FILE);
	sethostname(buff, MAX_HOST_NAME_LEN);
#endif
	//execl("hostname", "hostname", buff, NULL);
	sprintf(cmd, "hostname '%s'", buff);
	system(cmd);

	//interface DHCP may use sending hostname
	is_mgmtd_system_mgmt_restart_interface();
	return 0;
}

#define IS_ISSUE_FILE		"/etc/issue"
int is_mgmtd_system_misc_config_banner(void* conn_entry)
{
	silc_mgmtd_node* p_node;
	silc_cstr content;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/login-banner");
	if (!p_node)
	{
		SILC_ERR("Can't fine node for login-banner!\n");
		return -1;
	}
	if (p_node->tmp_value.type == SILC_MGMTD_VAR_NULL)
	{
		return 0;
	}
	content = p_node->tmp_value.val.string_val;
	USER_OP_LOG(conn_entry, "Login-banner is configured: %s", content);
	is_write_file(IS_ISSUE_FILE, content, strlen(content));
	silc_mgmtd_var_clear(&p_node->tmp_value);

	return 0;
}

#define IS_SYSLOG_CONF			"/etc/rsyslog.conf"
#define IS_SYSLOG_MAX_ROT_NUM	8
#define IS_SYSLOG_MAX_SIZE		10
#define IS_SYSLOG_TEMPLATE		"FileFormatWithPriText"

int is_mgmtd_system_misc_config_log(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_bool modified = silc_false;
	char buff[512];
	int level, max_size;
	static char* log_level_name[8] = {"emerg", "alert", "crit", "error", "warning", "notice", "info", "debug"};
	char* level_name;
	silc_bool remote_enabled;
	FILE* fp;
	FILE* fp_bak;
	int rotate_size;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/log/level", buff, 512, &modified);
	level = atoi(buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/log/max-size", buff, 512, &modified);
	max_size = atoi(buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/log/remote-enabled", buff, 512, &modified);
	remote_enabled = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	if (modified)
	{
		USER_OP_LOG(conn_entry, "System log is configured: level %d, maxsize %d, remote-enable %d",
				level, max_size, remote_enabled);
	}
	else if (p_node)
	{
		if (SILC_MGMTD_IF_REQ_ADD == type)
		{
			USER_OP_LOG(conn_entry, "System log remote server %s is added", p_node->value.val.string_val);
		}
		else if (SILC_MGMTD_IF_REQ_DELETE == type)
		{
			USER_OP_LOG(conn_entry, "System log remote server %s is deleted", p_node->value.val.string_val);
		}
		else
		{
			USER_OP_LOG(conn_entry, "System log remote server %s is modified", p_node->value.val.string_val);
		}
	}
	else
	{
		// no modify on global and no operation on remote-server
		return 0;
	}

	level_name = log_level_name[level];
	rotate_size = (1024*1024)*max_size/IS_SYSLOG_MAX_ROT_NUM;

	fp = fopen(IS_SYSLOG_CONF, "r");
	fp_bak = fopen(IS_SYSLOG_CONF".bak", "w");
	while (fgets(buff, 512, fp))
	{
		if (strncmp(buff, "$outchannel", 11) == 0 && strstr(buff, "rsyslog_rotation.sh"))
		{
			fprintf(fp_bak, "$outchannel log_rotation,/var/log/messages, %d,/etc/rsyslog_rotation.sh \n", rotate_size);
			continue;
		}
		if (strncmp(buff, "user.", 5) == 0)
		{
			// skip user log
			continue;
		}
		fprintf(fp_bak, "%s", buff);
	}
	// write local log
	fprintf(fp_bak, "user.%s;auth.notice;authpriv.*		:omfile:$log_rotation;"IS_SYSLOG_TEMPLATE"\n", level_name);
	// write remote log
	if (remote_enabled)
	{
		silc_mgmtd_node* p_host_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/log/remote-server-v2");
		silc_mgmtd_node* p_host;
		silc_list_for_each_entry(p_host, &p_host_list->sub_node_list, node)
		{
			if (SILC_MGMTD_IF_REQ_DELETE == type && strcmp(p_host->value.val.string_val, p_node->value.val.string_val) == 0)
			{
				continue;
			}
			if (p_host->type == MEMDB_NODE_TYPE_DYNAMIC)
			{
				fprintf(fp_bak, "user.%s;auth.notice;authpriv.*		@%s:%u;"IS_SYSLOG_TEMPLATE"\n",
					level_name, p_host->value.val.string_val, silc_mgmtd_memdb_find_sub_node(p_host, "port")->value.val.uint32_val);
			}
		}
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_SYSLOG_CONF".bak", IS_SYSLOG_CONF);

	is_mgmtd_run_vrf_process_cmd("syslog", "/etc/init.d/S01logging restart");
#endif
	return 0;
}

#define IS_ZONEINFO_DIR		"/usr/share/zoneinfo"
#define IS_NTP_CONFIG_FILE	"/etc/ntp.conf"
#define IS_NTP_LISTEN_PORT	123

int is_mgmtd_system_misc_config_dt(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[512];
	silc_bool ntp_enabled;
	char tz[64];
	char cmd[256];
	FILE* fp;
	FILE* fp_bak;
	silc_mgmtd_node* p_host_list;
	silc_mgmtd_node* p_host;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/datetime/timezone", buff, 512, &modified);
	strncpy(tz, buff, 64);
	if (modified)
	{
		USER_OP_LOG(conn_entry, "System timezone is configured: %s", tz);
		sprintf(cmd, "ln -fs %s/%s /etc/localtime", IS_ZONEINFO_DIR, tz);
		system(cmd);
		modified = silc_false;
	}

	is_mgmtd_get_mdf_node_val(IS_MGMTD_SYSTEM_MISC_PATH"/datetime/ntp-enabled", buff, 512, &modified);
	ntp_enabled = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	if (modified)
	{
		USER_OP_LOG(conn_entry, "System ntp is configured: enable %d", ntp_enabled);
	}
	else if (p_node)
	{
		if (SILC_MGMTD_IF_REQ_ADD == type)
		{
			USER_OP_LOG(conn_entry, "System ntp server %s is added", p_node->value.val.string_val);
		}
		else if (SILC_MGMTD_IF_REQ_DELETE == type)
		{
			USER_OP_LOG(conn_entry, "System ntp server %s is deleted", p_node->value.val.string_val);
		}
		else
		{
			USER_OP_LOG(conn_entry, "System ntp server %s is modified", p_node->value.val.string_val);
		}
	}
	else
	{
		// no modify on global and no operation on remote-server
		return 0;
	}

	if (0 == access("/etc/init.d/S49ntp", F_OK))
	{
		// rename it so rcS won't start it later
		system("mv /etc/init.d/S49ntp /etc/init.d/IS_S49ntp");
	}
	if (!ntp_enabled)
	{
		is_mgmtd_run_vrf_process_cmd("ntp", "/etc/init.d/IS_S49ntp stop");
		return 0;
	}

	fp = fopen(IS_NTP_CONFIG_FILE, "r");
	fp_bak = fopen(IS_NTP_CONFIG_FILE".bak", "w");
	while (fgets(buff, 512, fp))
	{
		if (strncmp(buff, "server", strlen("server")) == 0)
		{
			continue;
		}

		if(silc_mgmtd_memdb_get_product_info()->vrf_support)
		{
			if (strncmp(buff, "restrict default", strlen("restrict default")) == 0)
			{
				// allow local query
				fprintf(fp_bak, "restrict default nomodify nopeer limited kod\n");
				continue;
			}
			else if (strncmp(buff, "restrict 127.0.0.1", strlen("restrict 127.0.0.1")) == 0)
			{
				// listen on 0.0.0.0 because 127.0.0.1 won't work in vrf
				fprintf(fp_bak, "restrict 0.0.0.0\n");
				continue;
			}
		}
		fprintf(fp_bak, "%s", buff);
	}
	p_host_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/datetime/ntp-server-v2");
	silc_list_for_each_entry(p_host, &p_host_list->sub_node_list, node)
	{
		if (SILC_MGMTD_IF_REQ_DELETE == type && strcmp(p_host->value.val.string_val, p_node->value.val.string_val) == 0)
		{
			continue;
		}
		if (p_host->type == MEMDB_NODE_TYPE_DYNAMIC)
		{
			fprintf(fp_bak, "server %s iburst\n", p_host->value.val.string_val);
		}
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_NTP_CONFIG_FILE".bak", IS_NTP_CONFIG_FILE);

#if 0	// this will block the normal NTP sync
	static int ntp_iptables_rule_init = 0;
	if(!ntp_iptables_rule_init)
	{
		silc_cstr vrf_list[4] = {NULL};
		int num, i;
		char cmd[256];

		num = is_mgmtd_get_vrf_process_config("ntp", vrf_list, 4);
		if(num > 0)
		{
			for(i=0; i<num; i++)
			{
				// drop all incoming packets not from local address
				sprintf(cmd, "iptables -I INPUT -p udp --dport %d -i %s -m mac ! --mac-source $(cat /sys/class/net/%s/address) -j DROP",
						IS_NTP_LISTEN_PORT, vrf_list[i], vrf_list[i]);
				system(cmd);
			}
		}
		else
		{
			sprintf(cmd, "iptables -I INPUT -p udp --dport %d ! -s 127.0.0.1 -j DROP", IS_NTP_LISTEN_PORT);
			system(cmd);
		}
		ntp_iptables_rule_init = 1;
	}
#endif
	is_mgmtd_run_vrf_process_cmd("ntp", "/etc/init.d/IS_S49ntp restart");

	return 0;
}

int is_mgmtd_system_check_hostname(char* hostname)
{
	int len = strlen(hostname);
	char* p;
	if (len < 1 || len > MAX_HOST_NAME_LEN)
	{
		SILC_ERR("Invalid hostname %s", hostname);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	p = hostname;
	while (*p)
	{
		if ((*p < 'a' || *p > 'z')
			&& (*p < 'A' || *p > 'Z')
			&& (*p < '0' || *p > '9')
			&& (*p != '-')
			&& (*p != '_')
			&& (*p != '.')
			&& (*p != ':'))
		{
			SILC_ERR("Invalid hostname %s", hostname);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		p++;
	}
	return 0;
}

int is_mgmtd_system_misc_check_add(void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	silc_cstr host = p_node->value.val.string_val;
	int ret = 0;

	ret = is_mgmtd_system_check_hostname(host);
	if (ret)
		return ret;

	if (strcmp(p_node->p_parent->name, "remote-server-v2") == 0)
	{
		uint32_t port = silc_mgmtd_memdb_find_sub_node(p_node, "port")->tmp_value.val.uint32_val;
		ret = is_mgmtd_check_port(port);
		if (ret)
			return ret;
	}
	return 0;
}

int is_mgmtd_system_misc_check_modify(void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node;
	int ret = 0;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/hostname");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		char* hostname = p_node->tmp_value.val.string_val;
		ret = is_mgmtd_system_check_hostname(hostname);
		if (ret)
			return ret;
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/datetime/timezone");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		char tz[64] = {0};
		char pathname[128];
		int len;
		silc_mgmtd_var_to_str(&p_node->tmp_value, tz, 64);
		len = strlen(tz);
		if (len == 0)
		{
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		if(strstr(tz, ".") || silc_mgmtd_if_multi_cmd(tz))
		{
			SILC_ERR("Invalid zoneinfo %s", tz);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		sprintf(pathname, "%s/%s", IS_ZONEINFO_DIR, tz);
		if (0 != access(pathname, F_OK))
		{
			SILC_ERR("Can't access the zoneinfo file %s", pathname);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/log/level");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t level = p_node->tmp_value.val.uint32_val;
		if (level > 7)
		{
			SILC_ERR("Invalid log level %u", level);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MISC_PATH"/log/max-size");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t max_size = p_node->tmp_value.val.uint32_val;
		if (max_size < 1 || max_size > IS_SYSLOG_MAX_SIZE)
		{
			SILC_ERR("Invalid log maxsize %u", max_size);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	p_node = (silc_mgmtd_node*)p_db_node;
	if (silc_list_empty(&p_node->sub_node_list))
	{
		p_node = p_node->p_parent;
	}
	if (strcmp(p_node->p_parent->name, "remote-server-v2") == 0)
	{
		uint32_t port = silc_mgmtd_memdb_find_sub_node(p_node, "port")->tmp_value.val.uint32_val;
		ret = is_mgmtd_check_port(port);
		if (ret)
			return ret;
	}

	return 0;
}

int is_mgmtd_system_misc_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	int ret;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
			return is_mgmtd_system_misc_check_add(p_db_node, conn_entry);
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			return is_mgmtd_system_misc_check_modify(p_db_node, conn_entry);
		case SILC_MGMTD_IF_REQ_ADD:
		case SILC_MGMTD_IF_REQ_DELETE:
		case SILC_MGMTD_IF_REQ_MODIFY:
			is_mgmtd_system_misc_update_ts(conn_entry);
			if (silc_list_empty(&p_node->sub_node_list))
			{
				p_node = p_node->p_parent;
			}
			if (strcmp(p_node->p_parent->name, "remote-server-v2") == 0)
			{
				return is_mgmtd_system_misc_config_log(type, p_node, conn_entry);
			}
			else if (strcmp(p_node->p_parent->name, "ntp-server-v2") == 0)
			{
				return is_mgmtd_system_misc_config_dt(type, p_node, conn_entry);
			}
			break;
		default:
			return 0;
	}

	ret = is_mgmtd_system_misc_config_hostname(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_misc_config_banner(conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_misc_config_dt(type, NULL, conn_entry);
	if (ret)
		return ret;

	ret = is_mgmtd_system_misc_config_log(type, NULL, conn_entry);
	if (ret)
		return ret;

	return 0;
#endif
    return 0;
}
