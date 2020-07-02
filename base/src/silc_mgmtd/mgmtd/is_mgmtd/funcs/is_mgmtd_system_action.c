/* this file is created by silc_mgmtd_inst_func_source_gen.py */
#include "silc_common.h"
#include "silc_common/silc_thread.h"

#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_operation.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_config.h"

int is_mgmtd_system_service_ssh_sync_key2db();

silc_cstr_array* is_mgmtd_system_get_interface()
{
	char inf_list_str[64];
	int len = 64;
	silc_cstr_array* ret = NULL;

	if(silc_mgmtd_if_exec_system_cmd("ip link |grep eth|grep -v 'link/ether'|awk '{gsub(\":\",\"\",$2);printf $2\",\"}'", inf_list_str, &len, 1000, silc_false))
		return NULL;
	ret = silc_cstr_split(inf_list_str, ",");
	return ret;
}

int is_mgmtd_system_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	if(type != SILC_MGMTD_IF_REQ_ACTION)
		return 0;
	if(p_node->tmp_value.type == SILC_MGMTD_VAR_STRING)
	{
		//validate input string
		silc_cstr value = p_node->tmp_value.val.string_val;
		if(value && value[0])
		{
			if(silc_mgmtd_if_multi_cmd(value) ||
					strstr(value, " ") || strstr(value, "\t"))
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	if(strcmp(p_node->name, "reboot") == 0)
	{
		SILC_LOG("System reboot");
#ifndef SILC_MGMTD_LOCAL_DEBUG
		silc_mgmtd_if_system_cmd_later("reboot", 1);
#else
		printf("[%s] reboot\n", __func__);
#endif
		return 0;
	}
	else if(strcmp(p_node->name, "reboot-force") == 0)
	{
		SILC_LOG("System reboot force");
#ifndef SILC_MGMTD_LOCAL_DEBUG
		silc_mgmtd_if_system_cmd_later("reboot -n -f", 1);
#else
		printf("[%s] reboot force\n", __func__);
#endif
		return 0;
	}
	else if(strcmp(p_node->name, "reset") == 0)
	{
		SILC_LOG("System reset to factory setting");
#ifndef SILC_MGMTD_LOCAL_DEBUG
		silc_mgmtd_if_system_cmd_later("is_reset.sh &", 1);
#else
		printf("[%s] reset\n", __func__);
#endif
		return 0;
	}
	else if(strcmp(p_node->name, "halt") == 0)
	{
		int ret = -1;
		SILC_LOG("System halt");
#ifndef SILC_MGMTD_LOCAL_DEBUG
		if(silc_mgmtd_memdb_get_product_info()->custom_sys_halt_func)
		{
			ret = (silc_mgmtd_memdb_get_product_info()->custom_sys_halt_func());
		}
		if(ret != 0) //custom halt didn't execute, let's force halt
		{
			silc_mgmtd_if_system_cmd_later("halt", 1);
		}
#else
		printf("[%s] halt \n", __func__);
#endif
		return 0;
	}
	else if(strcmp(p_node->name, "save-config") == 0)
	{
		SILC_LOG("System save config");
		if(silc_mgmtd_cfg_save_config_to_file() != 0)
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		return 0;
	}
	else if(strcmp(p_node->name, "save-config-as") == 0)
	{
		silc_cstr filename = p_node->tmp_value.val.string_val;
		SILC_LOG("System save config as %s", filename);
		if(silc_mgmtd_cfg_save_as_config_to_file(filename) != 0)
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		if(silc_mgmtd_memdb_save_ext_conf_file(filename, filename) != 0)
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		return 0;
	}
	else if(strcmp(p_node->name, "load-default-config") == 0)
	{
		SILC_LOG("System load default config");
		if(silc_mgmtd_cfg_load_default_config_file() != 0)
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		return 0;
	}
	else if(strcmp(p_node->name, "load-config-from") == 0)
	{
		SILC_LOG("System load config from %s", p_node->tmp_value.val.string_val);
		if(silc_mgmtd_cfg_load_config_file(p_node->tmp_value.val.string_val) != 0)
		{
			if(silc_mgmtd_memdb_restore_ext_conf_file(p_node->tmp_value.val.string_val, p_node->tmp_value.val.string_val) != 0)
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			if(silc_mgmtd_cfg_load_config_file(p_node->tmp_value.val.string_val) != 0)
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		return 0;
	}
	else if(strcmp(p_node->name, "remove-config-file") == 0)
	{
		SILC_LOG("System remove config %s", p_node->tmp_value.val.string_val);
		if(silc_mgmtd_cfg_remove_config_file(p_node->tmp_value.val.string_val) != 0)
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		silc_mgmtd_memdb_remove_ext_conf_file(p_node->tmp_value.val.string_val);
		return 0;
	}
	else if(strcmp(p_node->name, "check-is-admin") == 0)
	{
		if(silc_mgmtd_if_server_get_conn_privilege(conn_entry) != SILC_MGMTD_IF_LEVEL_ADMIN)
			return IS_MGMTD_ERR_AAA_PRIVIL_FORBIDDEN;
		return 0;
	}
	else if(strcmp(p_node->name, "set-connect-login") == 0)
	{
		silc_mgmtd_node *p_type_node, *p_user_node, *p_ip_node, *p_port_node, *p_priv_node, *p_proto_node;
		silc_cstr type=NULL, user=NULL, ip="", protol="local";
		uint16_t port=0;
		char path[255];

		p_type_node = silc_mgmtd_memdb_find_sub_node(p_node, "type");
		p_user_node = silc_mgmtd_memdb_find_sub_node(p_node, "username");
		p_ip_node = silc_mgmtd_memdb_find_sub_node(p_node, "ip");
		p_port_node = silc_mgmtd_memdb_find_sub_node(p_node, "port");
		p_priv_node = silc_mgmtd_memdb_find_sub_node(p_node, "privilege");
		p_proto_node = silc_mgmtd_memdb_find_sub_node(p_node, "protocol");
		if(!p_type_node || !p_user_node || !p_ip_node || !p_port_node || !p_priv_node || !p_proto_node)
			return -1;
		if (p_type_node->tmp_value.type == SILC_MGMTD_VAR_NULL || p_user_node->tmp_value.type == SILC_MGMTD_VAR_NULL)
			return -1;

		type = p_type_node->tmp_value.val.string_val;
		user = p_user_node->tmp_value.val.string_val;
		if (p_ip_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
			ip = p_ip_node->tmp_value.val.string_val;
		if (p_port_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
			port = p_port_node->tmp_value.val.uint32_val;
		if(silc_mgmtd_if_server_conn_login_set(conn_entry, silc_mgmtd_if_get_client_type(type), user, ip, port) != 0)
			return -1;

		if (p_priv_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		{
			silc_mgmtd_if_server_set_conn_privilege(conn_entry, p_priv_node->tmp_value.val.uint32_val);
			SILC_INFO("[%s]p_conn:%p, User:%s, get privilege from req: %d\n", __func__, conn_entry, user, silc_mgmtd_if_server_get_conn_privilege(conn_entry));
			return 0;
		}

		if (p_proto_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
			protol = p_proto_node->tmp_value.val.string_val;

		if(strcmp(protol, "local") == 0)
		{
			if (strcmp(user, "root") == 0)
				silc_mgmtd_if_server_set_conn_privilege(conn_entry, SILC_MGMTD_IF_LEVEL_ADMIN);
			else
			{
				sprintf(path, "/config/unix/user/%s/privilege", user);
				p_priv_node = silc_mgmtd_memdb_find_node(path);
				if(p_priv_node)
					silc_mgmtd_if_server_set_conn_privilege(conn_entry, p_priv_node->value.val.uint32_val);
				else
					SILC_LOG("[%s] Can not find the local user:%s.\n", __func__, user);
			}
		}
		else if (strcmp(protol, "radius") == 0)
		{
			sprintf(path, "/config/radius/static/privilege");
			p_priv_node = silc_mgmtd_memdb_find_node(path);
			if(p_priv_node)
				silc_mgmtd_if_server_set_conn_privilege(conn_entry, p_priv_node->value.val.uint32_val);
		}
		SILC_INFO("[%s]p_conn:%p, User:%s, Protocol:%s, get privilege: %d\n", __func__, conn_entry, user, protol, silc_mgmtd_if_server_get_conn_privilege(conn_entry));
		return 0;
	}
	else if(strcmp(p_node->name, "set-notify-path") == 0)
	{
		SILC_TRACE("[%s]enable receive notify, path:%s\n", __func__, p_node->tmp_value.val.string_val);
		silc_mgmtd_if_server_notify_path_set(conn_entry, p_node->tmp_value.val.string_val);
		return 0;
	}
	else if(strcmp(p_node->name, "generate-ssh-key") == 0)
	{
		int ret;
		SILC_LOG("System generate ssh key");
		system("rm -rf /etc/ssh/*key*; ssh-keygen -A");
		ret = is_mgmtd_system_service_ssh_sync_key2db();
		return ret;
	}
	else if(strcmp(p_node->name, "set-time") == 0 ||
			strcmp(p_node->name, "set-date") == 0 ||
			strcmp(p_node->name, "set-datetime") == 0)
	{
		int ret = 0;
		char cmd[256];
		char dt_str[128];

		silc_mgmtd_var_to_str(&p_node->tmp_value, dt_str, 100);
		if(strcmp(p_node->name, "set-time") == 0)
		{
			sprintf(cmd, "date -s \"%s\"", dt_str);
			SILC_LOG("System set time %s", dt_str);
		}
		else if(strcmp(p_node->name, "set-date") == 0)
		{
			char cur_time[128];
			int len = 128;
			if((ret=silc_mgmtd_if_exec_system_cmd("date +%T", cur_time, &len, 1000, silc_false)) != 0)
				return ret;
			cur_time[strlen(cur_time)-1] = 0;
			sprintf(cmd, "date -s \"%s %s\"", dt_str, cur_time);
			SILC_LOG("System set date %s", dt_str);
		}
		else
		{
			silc_cstr_array* p_arr = silc_cstr_split(dt_str, ",");
			if(!p_arr)
			{
				SILC_ERR("silc_cstr_split error");
				return IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED;
			}
			if(p_arr->length != 2)
			{
				silc_cstr_array_destroy(p_arr);
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			}
			sprintf(cmd, "date -s \"%s %s\"", silc_cstr_array_get_quick(p_arr, 0), silc_cstr_array_get_quick(p_arr, 1));
			silc_cstr_array_destroy(p_arr);
			SILC_LOG("System set datetime %s", dt_str);
		}
#ifndef SILC_MGMTD_LOCAL_DEBUG
		if((ret=silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 1000, silc_false)) != 0)
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
		//sync with hw clock
		//deal with special hardware if needed
		if(silc_mgmtd_memdb_get_product_info()->custom_sync_hw_clock_func)
			return silc_mgmtd_memdb_get_product_info()->custom_sync_hw_clock_func();
		//no special hardware, do hwclock directly
		if((ret=silc_mgmtd_if_exec_system_cmd("hwclock -w -u", NULL, NULL, 1000, silc_false)) != 0)
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
#else
		printf("set-datetime: %s\n", cmd);
#endif
		return 0;
	}
#if 0
	else if(strcmp(p_node->name, "set-date") == 0)
	{
		int ret = 0, len = 100;
		int model;
		char cmd[100];
		char cur_time[100];
		char set_date[100];
		model = is_mgmtd_system_get_machine_model();
		if(model < 0 || model > 3)
		{
			model = 0;
			SILC_LOG("Can not get machine model ,use default model \n");
		}
		silc_mgmtd_var_to_str(&p_node->tmp_value, set_date, 100);
		if((ret=silc_mgmtd_if_exec_system_cmd("date +%T", cur_time, &len, 1000, silc_false)) != 0)
			return ret;
		cur_time[strlen(cur_time)-1] = 0;
		sprintf(cmd, "date -s \"%s %s\"", set_date, cur_time);
		SILC_LOG("System set date %s", set_date);
#ifndef SILC_MGMTD_LOCAL_DEBUG
		if((ret=silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 1000, silc_false)) != 0)
			return ret;
		if(model == 0)
		{
			if((ret=system("hwclock -w -u -f /dev/rtc0")) != 0)
				return ret;
		}
		else if(model == 1 || model == 2 || model == 3)
		{
			if((ret=system("hwclock -w -u -f /dev/rtc1")) != 0)
				return ret;
		}
#else
		printf("set-date %s\n", cmd);
#endif
		return 0;
	}
	else if(strcmp(p_node->name, "set-datetime") == 0)
	{
		int ret = 0;
		int model;
		char cmd[100];
		char datetime[100];
		silc_cstr_array* p_arr;
		model = is_mgmtd_system_get_machine_model();
		if(model < 0 || model > 3)
		{
			model = 0;
			SILC_LOG("Can not get machine model ,use default model \n");
		}
		silc_mgmtd_var_to_str(&p_node->tmp_value, datetime, 100);
		p_arr = silc_cstr_split(datetime, ",");
		if(!p_arr || p_arr->length != 2)
		{
			if(p_arr)
				silc_cstr_array_destroy(p_arr);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		sprintf(cmd, "date -s \"%s %s\"", silc_cstr_array_get_quick(p_arr, 0), silc_cstr_array_get_quick(p_arr, 1));
		silc_cstr_array_destroy(p_arr);
		SILC_LOG("System set datetime %s", datetime);
#ifndef SILC_MGMTD_LOCAL_DEBUG
		if((ret=silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 1000, silc_false)) != 0)
			return ret;
		//sync with hw clock
		if(model == 0)
		{
			if((ret=system("hwclock -w -u -f /dev/rtc0")) != 0)
				return ret;
		}
		else if(model == 1 || model == 2 || model == 3)
		{
			if((ret=system("hwclock -w -u -f /dev/rtc1")) != 0)
				return ret;
		}
#else
		printf("set-datetime %s\n", cmd);
#endif
		return 0;
	}
#endif
	else if(strcmp(p_node->name, "reset-log") == 0)
	{
		SILC_LOG("System reset log");
		system("rm -rf /var/log/messages* ; touch /var/log/messages; /etc/init.d/S01logging restart");
		return 0;
	}
	else if(strcmp(p_node->name, "create-log-dump") == 0)
	{
		char output[32];
		int len=32, ret, usage;

		ret = silc_mgmtd_if_exec_system_cmd("df -k /var/log/|grep -v Filesystem|awk '{print $5}'", output, &len, 1000, silc_false);
		if (ret)
			return ret;
		usage = atoi(output);
		if (usage > 85)
		{
			SILC_ERR("System fail to create log dump for disk full");
			return IS_MGMTD_ERR_SYSTEM_DISK_FULL;
		}
		SILC_LOG("System create log dump");
		system("is_logdump.sh");
		return 0;
	}
	else if(strcmp(p_node->name, "remove-log-dump") == 0)
	{
		char cmd[256], *filename=p_node->tmp_value.val.string_val;
		SILC_LOG("System remove config %s", filename);
		sprintf(cmd, "rm -rf /var/log/dump/%s", filename);
		system(cmd);
		return 0;
	}
	else if(strcmp(p_node->name, "init-config") == 0)
	{
		if(silc_mgmtd_memdb_get_product_info()->set_init_config_func)
			return silc_mgmtd_memdb_get_product_info()->set_init_config_func();
		return 0;
	}
	else if(strcmp(p_node->name, "check-upgrade") == 0)
	{
		silc_mgmtd_if_exec_system_cmd("/usr/sbin/is_upgrade.sh -C", NULL, NULL, 10000, silc_false);
		return 0;
	}
	else if(strcmp(p_node->name, "switch-software") == 0)
	{
		int ret;
		SILC_LOG("System switch software");
		ret = silc_mgmtd_if_exec_system_cmd("/usr/sbin/is_upgrade.sh -w", NULL, NULL, 10000, silc_false);
		if(0 != ret)
		{
			SILC_ERR("Execute switch software error: %d", ret);
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
		}
		return 0;
	}
	else if(strcmp(p_node->name, "reset-sys-evt") == 0)
	{
		SILC_LOG("System reset sys-event");
		system("rm -rf /var/log/message-sys-event");
		return 0;
	}

    return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
}

