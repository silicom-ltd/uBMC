/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"
#include "is_cli_cmd_show_snmp.h"
#include "is_cli_cmd_show_system.h"
#include "is_cli_cmd_show_aaa.h"
#include "is_cli_cmd_show_running_config.h"

#define IS_CLI_PATH_QUERY_SNMP_CONFIG		"/config/snmp"
#define IS_CLI_PATH_QUERY_SNMP_STATUS		"/status/snmp"
#define IS_CLI_PATH_QUERY_SNMP_ENGINE_ID	"/status/snmp/engine-id"
#define IS_CLI_PATH_QUERY_SYSTEM_CONFIG		"/config/system"
#define IS_CLI_PATH_QUERY_SYSTEM_STATUS		"/status/system"
#define IS_CLI_PATH_QUERY_RADIUS_CONFIG		"/config/radius"
#define IS_CLI_PATH_QUERY_TACACS_CONFIG		"/config/tacacs"
#define IS_CLI_PATH_QUERY_UNIX_CONFIG		"/config/unix"
#define IS_CLI_PATH_QUERY_SESSION_EXP_CONFIG	"/config/system/mgmt/session-exp-time"
#define IS_CLI_PATH_QUERY_SESSION_STATUS	"/status/system/session"
#define IS_CLI_PATH_QUERY_SYSTEM_TIME		"/status/system/time"
#define IS_CLI_PATH_QUERY_SYSTEM_UPTIME		"/status/system/uptime"
#define IS_CLI_PATH_QUERY_SYSTEM_SERVICE	"/config/system/service"

int is_cli_cmd_show_file(silc_cstr filename)
{
#if 0
	struct stat fs;
	silc_cstr buff = NULL;
	int fd;

	if(stat(filename, &fs) < 0)
	{
		silc_cli_err_cmd_set_err_info("Invalid file %s", filename);
		return -1;
	}
	buff = malloc(fs.st_size);
	if(!buff)
	{
		silc_cli_err_cmd_set_err_info("Not enough memory");
		return -1;
	}
	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		silc_cli_err_cmd_set_err_info("Fail to open file %s", filename);
		free(buff);
		return -1;
	}

	if(read(fd, buff, fs.st_size) != fs.st_size)
	{
		silc_cli_err_cmd_set_err_info("Fail to read file %s (%d bytes)", filename, (int)fs.st_size);
		free(buff);
		return -1;
	}
	close(fd);
	silc_cli_print(buff);
	free(buff);
#else
	char cmd[256];
	if(!silc_mgmtd_if_file_exist(filename))
	{
		silc_cli_err_cmd_set_err_info("Invalid file %s", filename);
		return -1;
	}
	sprintf(cmd, "cat '%s'", filename);
	system(cmd);
#endif
	return 0;
}

int is_cli_cmd_show_snmp_get_req_info(silc_list* p_token_list,
		silc_cli_token *p_token, silc_cli_token *p_l1_token, is_cli_cmd_req_info* p_req_info)
{
	if(!p_l1_token)
	{
		sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SNMP_STATUS);
		p_req_info->type = SILC_MGMTD_IF_REQ_QUERY_SUB;
		p_req_info->rsp_cb = is_cli_show_snmp_status_cb;
	}
	else if(strcmp(p_l1_token->name, "configured") == 0)
	{
		sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SNMP_CONFIG);
		p_req_info->type = SILC_MGMTD_IF_REQ_QUERY_SUB;
		p_req_info->rsp_cb = is_cli_show_snmp_configured_cb;
	}
	else if(strcmp(p_l1_token->name, "engineID") == 0)
	{
		sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SNMP_ENGINE_ID);
		p_req_info->type = SILC_MGMTD_IF_REQ_QUERY;
		p_req_info->rsp_cb = is_cli_show_snmp_status_engine_id_cb;
	}
	else
	{
		silc_cli_err_cmd_set_invalid_param(p_l1_token->name);
		return -1;
	}
	p_req_info->root_val = silc_mgmtd_if_path_get_last_name(p_req_info->path);
	return 0;

}

#if 0
int is_cli_show_cpu()
{
	char str[128];
	FILE* fp;
	uint32_t sub_cnt[7], i, total;
	float usage[7];
	char name[8];

	silc_cli_print("CPU usage:\n");
	fp = popen("cat /proc/stat", "r");
	while (fgets(str, 128, fp))
	{
		if (strncmp(str, "cpu", 3) == 0)
		{
			sscanf(str, "%s %u %u %u %u %u %u %u",
					name, &sub_cnt[0], &sub_cnt[1], &sub_cnt[2], &sub_cnt[3], &sub_cnt[4], &sub_cnt[5], &sub_cnt[6]);
			total = 0;
			for (i=0; i<7; i++)
				total += sub_cnt[i];
			for (i=0; i<7; i++)
				usage[i] = ((float)sub_cnt[i]*100)/total;
			silc_cli_print("%-4s : %2.2f%% usr, %2.2f%% sys, %2.2f%% nice, %2.2f%% idle, %2.2f%% io, %2.2f%% irq, %2.2f%% sirq\n",
					name, usage[0], usage[1], usage[2], usage[3], usage[4], usage[5], usage[6]);
		}
		else
			break;
	}
	pclose(fp);
	silc_cli_print("\n");

	return 0;
}

int is_cli_show_mem()
{
	char output[256];
	int output_len= 256;

	silc_cli_print("Memory usage (KB):\n");
	output[0] = 0;
	if(silc_mgmtd_if_exec_system_cmd("free", output, &output_len, 1000, silc_false) != 0)
	{
		silc_cli_print("exec free error!\n");
		return 0;
	}
	silc_cli_print(output);
	return 0;
}
#endif

#define IS_LOG_FILE			"/var/log/messages"
#define LOCAL_IMAGE_PATH 	"/var/images"

int is_cli_show_images_storage()
{
	char cmd[256];

	silc_cli_print("\nStorage Usage(MByte):\n");
	silc_cli_print("%s\t%s\t%s\n","Total","Used","Free");

	snprintf(cmd,256,"df -m|grep %s|awk '{print $2,\"\t\",$3,\"\t\",$4}'",LOCAL_IMAGE_PATH);
	return silc_cli_show_cmd_output(cmd);
}

int is_cli_cmd_show_do_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);

		if(strcmp(p_token->name, "snmp") == 0)
		{
			return is_cli_cmd_show_snmp_get_req_info(p_token_list, p_token, p_l1_token, p_req_info);
		}
		else if(strcmp(p_token->name, "management") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/mgmt");
				p_req_info->rsp_cb = is_cli_show_mgmt_configured_cb;
				return 0;
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "state") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/mgmt");
				p_req_info->rsp_cb = is_cli_show_mgmt_status_cb;
				return 0;
			}
		}
		else if(strcmp(p_token->name, "radius") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_RADIUS_CONFIG);
				p_req_info->rsp_cb = is_cli_show_radius_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "tacacs") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_TACACS_CONFIG);
				p_req_info->rsp_cb = is_cli_show_tacacs_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "cli") == 0)
		{
			p_req_info->path[0] = 0;
			silc_cli_print("CLI Current auto logout time(s): %d\n", silc_cli_session_timeout_curr_get());
		}
		else if(strcmp(p_token->name, "clock") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_TIME);
			p_req_info->rsp_cb = is_cli_show_clock_status_cb;
		}
		else if(strcmp(p_token->name, "uptime") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_UPTIME);
			p_req_info->rsp_cb = is_cli_show_uptime_status_cb;
		}
		else if(strcmp(p_token->name, "web") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_SERVICE);
				p_req_info->rsp_cb = is_cli_show_web_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "telnet") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_SERVICE"/telnet");
				p_req_info->rsp_cb = is_cli_show_telnet_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "ssh") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_SERVICE"/ssh");
				p_req_info->rsp_cb = is_cli_show_ssh_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "session") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SESSION_EXP_CONFIG);
				p_req_info->rsp_cb = is_cli_show_session_configured_cb;
			}
			else
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SESSION_STATUS);
				p_req_info->rsp_cb = is_cli_show_session_status_cb;
			}
		}
		else if(strcmp(p_token->name, "users") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_UNIX_CONFIG"/user");
			p_req_info->rsp_cb = is_cli_show_unix_user_configured_cb;
		}
		else if(strcmp(p_token->name, "ntp") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/misc/datetime");
				p_req_info->rsp_cb = is_cli_show_ntp_configured_cb;
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "state") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/ntp-state");
				p_req_info->rsp_cb = is_cli_show_ntp_state_cb;
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "associations") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/ntp-associations");
				p_req_info->rsp_cb = is_cli_show_ntp_associations_cb;
			}
		}
		else if(strcmp(p_token->name, "log") == 0)
		{
			if(!p_l1_token)
			{
				return silc_cli_show_log(IS_LOG_FILE, NULL);
			}
			else if(strcmp(p_l1_token->name, "filter") == 0)
			{
				if (!silc_cli_check_log_filter(p_l1_token->val_str))
				{
					silc_cli_err_cmd_set_invalid_param("Invalid filter");
					return -1;
				}
				return silc_cli_show_log(IS_LOG_FILE, p_l1_token->val_str);
			}
			else if(strcmp(p_l1_token->name, "realtime") == 0)
			{
				system("tail -f "IS_LOG_FILE);
				return 0;
			}
			else if(strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/misc/log");
				p_req_info->rsp_cb = is_cli_show_log_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "com") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/service/com");
				p_req_info->rsp_cb = is_cli_show_com_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "name") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/misc/hostname");
				p_req_info->rsp_cb = is_cli_show_dev_name_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "login-banner") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_CONFIG"/misc/login-banner");
				p_req_info->rsp_cb = is_cli_show_dev_login_banner_configured_cb;
			}
		}
		else if(strcmp(p_token->name, "images") == 0)
		{
			char cmd[256];

			silc_cli_print("Image List:\n");
			snprintf(cmd,256,"ls -l --time-style=long-iso %s |grep -v lost+found |awk '{print($6,$7,\"\\t\",$8)}'",LOCAL_IMAGE_PATH);
			if(silc_cli_show_cmd_output(cmd) != 0)
			{
				return 0;
			}

			is_cli_show_images_storage();
			return 0;
		}
		else if(strcmp(p_token->name, "version") == 0)
		{
			system("/usr/sbin/is_upgrade -S");
			return 0;
		}
		else if(strcmp(p_token->name, "event") == 0)
		{
			if(access("/var/log/message-sys-event", F_OK) == 0)
			{
				system("cat /var/log/message-sys-event");
			}
			return 0;
		}
		else if(strcmp(p_token->name, "running-config") == 0)
		{
			if(silc_cli_cmd_do_simple_action("/action/system/check-is-admin", "true", NULL, 0) != 0)
			{
				silc_cli_err_cmd_set_err_info("no privilege to show running-config");
				return -1;
			}
			silc_cstr buff = malloc(1024*256);
			if(!buff)
			{
				silc_cli_err_set(CLI_ERR_SYSTEM);
				return -1;
			}
			if(silc_cli_cmd_get_simple_node("/status/system/running-config", buff) != 0)
			{
				free(buff);
				return -1;
			}
			is_cli_show_running_config(buff);
			free(buff);
			return 0;
		}
		else if(strcmp(p_token->name, "configurations") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "list") == 0)
			{
				char* cmd;
#if 0
				//busybox ls
				cmd = "ls -lt --full-time /config/extend/|awk '{print($6,$7,\"\\t\",$9)}'";
#else
				//coreutil ls
				cmd = "ls -l --time-style=long-iso /config/extend/|awk '{print($6,$7,\"\\t\",$8)}'";
#endif
				silc_cli_show_cmd_output(cmd);
				return 0;
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "detail") == 0)
			{
				char filename[256];
				if(!silc_cli_check_name(p_l1_token->val_str))
				{
					silc_cli_err_cmd_set_err_info("Invalid file name %s.", p_l1_token->val_str);
					return -1;
				}
				sprintf(filename, "/config/extend/%s", p_l1_token->val_str);
				return is_cli_cmd_show_file(filename);
			}
		}
		else if(strcmp(p_token->name, "dump") == 0)
		{
			static char output[4096];
			int output_len= 4096;
			output[0] = 0;
			if(silc_mgmtd_if_exec_system_cmd("ls -t /var/log/dump/|awk '{print $1}'",
				output, &output_len, 1000, silc_false) != 0)
			{
				return 0;
			}
			silc_cli_print(output);
			return 0;
		}
		else if(strcmp(p_token->name, "upgrade") == 0)
		{
			#define IS_UPGRADE_TOOL         "/usr/sbin/is_upgrade"
			if(p_l1_token && strcmp(p_l1_token->name, "state") == 0)
			{
				int ret = silc_mgmtd_if_system(IS_UPGRADE_TOOL" -a");
				if(ret == 1)
				{
					silc_cli_print("The upgraded is in progress, please don't power off the device.\n");
					silc_mgmtd_if_system(IS_UPGRADE_TOOL" -r");
				}
				else if(ret == 2)
				{
		       			silc_cli_print("The upgrade is done, please reload the device to complete the upgrade.\n");
				}
                                else if(ret == 0)
                                {
                                        silc_cli_print("The system is ready for upgrade.\n");
                                }
				return 0;
			}
		}
		else if(strcmp(p_token->name, "cpu") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/cpu");
			p_req_info->rsp_cb = is_cli_show_cpu_status_cb;
		}
		else if(strcmp(p_token->name, "memory") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/memory");
			p_req_info->rsp_cb = is_cli_show_memory_status_cb;
		}
		else if(strcmp(p_token->name, "storage") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_QUERY_SYSTEM_STATUS"/storage");
			p_req_info->rsp_cb = is_cli_show_storage_status_cb;
		}
		else if(strcmp(p_token->name, "show") != 0
				&& strcmp(p_token->name, "configured") != 0
				&& strcmp(p_token->name, "state") != 0
				&& strcmp(p_token->name, "associations") != 0)
		{
			silc_cli_err_cmd_set_invalid_param(p_token->name);
			return -1;
		}
		p_req_info->root_val = silc_mgmtd_if_path_get_last_name(p_req_info->path);
		p_req_info->type = SILC_MGMTD_IF_REQ_QUERY_SUB;
	}
	return 0;
}

int is_cli_cmd_show_do(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_show_do_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(req_info.path[0] == 0)
		return 0;

	if(req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}


