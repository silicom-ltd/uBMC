/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

#define IS_CLI_PATH_CONFIG_SYSTEM_DATETIME	"/config/system/misc/datetime"
#define IS_CLI_PATH_ACTION_SYSTEM			"/action/system"

int is_cli_cmd_clock_config(silc_list* p_token_list)
{
	char path[256];
	silc_cli_token *p_token, *p_l1_token;
	char err_info[200];
	silc_bool ntp_enable;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);

		if(strcmp(p_token->name, "set") == 0)
		{
			if(!p_l1_token)
			{
				silc_cli_err_cmd_set_incomplete_cmd(p_token);
				return -1;
			}
			if(silc_cli_get_mgmtd_bool_node_value(IS_CLI_PATH_CONFIG_SYSTEM_DATETIME"/ntp-enabled", &ntp_enable) != 0)
			{
				silc_cli_err_cmd_set_err_info("%% Can not get NTP state");
				return -1;
			}

			if(ntp_enable)
			{
				silc_cli_err_cmd_set_err_info("%% NTP is enabled, please disable it first");
				return -1;
			}
			if(strcmp(p_l1_token->name, "date") == 0 || strcmp(p_l1_token->name, "time") == 0)
			{
				sprintf(path, IS_CLI_PATH_ACTION_SYSTEM"/%s", p_l1_token->map_name);
				if(silc_cli_cmd_do_simple_action(path, p_l1_token->val_str, err_info, 200) != 0)
				{
					silc_cli_print("%% Set %s failed! Error: %s.\n", p_l1_token->name, err_info);
					return -1;
				}
				// update session timestamp or it might cause idle timeout quit
				silc_time_sleep(0, 600000000); //wait silc_time update complete (500ms)
				silc_cli_session_timeout_update();
				return 0;
			}
			silc_cli_err_cmd_set_invalid_param(p_l1_token->name);
			return -1;
		}
		else if(strcmp(p_token->name, "timezone") == 0)
		{
			char tz_name[100];
			if(p_l1_token && strcmp(p_l1_token->name, "area") == 0)
			{
				sprintf(tz_name, "%s/%s", p_token->val_str, p_l1_token->val_str);
			}
			else if(strstr(p_token->val_str, "/"))
			{
				strcpy(tz_name, p_token->val_str);
			}
			else
			{
				silc_cli_err_cmd_set_invalid_param(p_token->name);
				return -1;
			}
			sprintf(path, IS_CLI_PATH_CONFIG_SYSTEM_DATETIME"/%s", p_token->map_name);
			if(silc_cli_cmd_do_simple_modify(path, tz_name, err_info, 200) != 0)
			{
				silc_cli_print("%% Set time zone %s failed! Error: %s.\n", tz_name, err_info);
				return -1;
			}
			return 0;
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;

}
