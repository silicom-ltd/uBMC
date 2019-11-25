/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

#include <curses.h>

#define IS_CLI_PATH_STATUS_CONFIG_SAVED		"/status/system/config-saved"
#define IS_CLI_PATH_ACTION_CONFIG_SAVE		"/action/system/save-config"
#define IS_CLI_PATH_ACTION_SYSTEM_REBOOT	"/action/system/reboot"
#define IS_CLI_PATH_ACTION_SYSTEM_REBOOT_F	"/action/system/reboot-force"
#define IS_CLI_PATH_ACTION_SYSTEM_HALT		"/action/system/halt"
#define IS_CLI_PATH_ACTION_SYSTEM_RESET		"/action/system/reset"
#define IS_CLI_PATH_ACTION_SWITCH_SOFTWARE	"/action/system/switch-software"

int is_cli_cmd_reboot()
{
	char err_info[200];
	if(silc_cli_cmd_do_simple_action(IS_CLI_PATH_ACTION_SYSTEM_REBOOT, "", err_info, 200) != 0)
	{
		silc_cli_print("%% Reboot system failed! Error: %s.\n", err_info);
		return -1;
	}
	silc_cli_print("Reboot.\n");
	return 0;
}

int is_cli_cmd_reload_go_confirm()
{
	silc_bool saved;
	silc_cstr reboot_warn_msg = "%% The system will reboot, Continue (y|n) ?";

	if(silc_cli_get_mgmtd_bool_node_value(IS_CLI_PATH_STATUS_CONFIG_SAVED, &saved) == 0)
	{
		if(!saved)
		{
			if (silc_cli_cmd_confirm("%% Some configurations are not saved, Save (y|n) ?", NULL, "%% Some configurations will be lost.\n") == 0)
			{
				char err_info[200];
				if(silc_cli_cmd_do_simple_action(IS_CLI_PATH_ACTION_CONFIG_SAVE, "", err_info, 200) != 0)
				{
					silc_cli_print("%% Save configuration failed! Error: %s.\n", err_info);
				}
				else
				{
					silc_cli_print("Configuration is saved.\n");
				}
			}
		}
	}
	else
		silc_cli_print("%% Can not query the configuration state, maybe lose unsaved configuration!");

	if(silc_cli_get_product_info()->reboot_warn)
		reboot_warn_msg = silc_cli_get_product_info()->reboot_warn;
	return silc_cli_cmd_confirm(reboot_warn_msg, NULL, "%% Cancelled");
}

int is_cli_cmd_reset_confirm()
{
	return silc_cli_cmd_confirm("%% Warning! The system will reboot and all configurations and logs will be lost! Continue (y|n) ?",
			"%% It will take about 15 seconds to format the flash and reboot, please wait and don't power off the device!", "%% Cancelled");
}

int is_cli_cmd_switch_software()
{
	char err_info[200];
	if(silc_cli_cmd_do_simple_action(IS_CLI_PATH_ACTION_SWITCH_SOFTWARE, "", err_info, 200) != 0)
	{
		silc_cli_print("%% Switch software failed! Error: %s.\n", err_info);
		return -1;
	}
	silc_cli_print("Switch OK!\n");
	return 0;
}

int is_cli_cmd_switch_software_do()
{
	int len = 500;
	char buf[len];
	int ch;

	if(silc_mgmtd_if_exec_system_cmd("/usr/sbin/is_upgrade -S|grep Software", buf, &len, 10000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Can not get the system version");
		return -1;
	}
SELECT_SWITCH:
	silc_cli_print("%% Warning: \n");
	silc_cli_print("   The backup software maybe can not work with current configuration!!\n");
	silc_cli_print("   If the system can not boot up after reboot, please do a manual reset, \n");
	silc_cli_print("   then the system will revert the switch.\n");
	silc_cli_print("%s", buf);
	silc_cli_print("%% Do you want switch current and backup software(y|n) ?");
	ch = getchar();
	while(getchar() != '\n');
	if(ch == 'y')
	{
		if(is_cli_cmd_switch_software())
			return -1;
		if(is_cli_cmd_reboot())
		{
			silc_cli_print("%% Auto reboot failed, please do a manual reboot");
			return 0;;
		}
		return 0;
	}
	if(ch == 'n')
	{
		silc_cli_print("%% Cancelled");
		return 0;
	}
	else
		goto SELECT_SWITCH;

	return 0;
}

int is_cli_cmd_reload_go(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;
	silc_cli_token *p_token;
	static char none_str[10] = "";
	silc_bool confirm = silc_true;
	int timeout=10;

	memset(&req_info, 0, sizeof(req_info));

	req_info.root_val = none_str;
	req_info.type = SILC_MGMTD_IF_REQ_ACTION;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "reload") == 0)
		{
			sprintf(req_info.path, IS_CLI_PATH_ACTION_SYSTEM_REBOOT);
		}
		else if(strcmp(p_token->name, "halt") == 0)
		{
			if(silc_cli_get_product_info()->halt_support)
				sprintf(req_info.path, IS_CLI_PATH_ACTION_SYSTEM_HALT);

		}
		else if(strcmp(p_token->name, "force") == 0)
		{
			sprintf(req_info.path, IS_CLI_PATH_ACTION_SYSTEM_REBOOT_F);
			break;
		}
		else if(strcmp(p_token->name, "noconfirm") == 0)
		{
			confirm = silc_false;
			break;
		}
		else if(strcmp(p_token->name, "switch-software") == 0)
		{
			return is_cli_cmd_switch_software_do();
		}
		else if(strcmp(p_token->name, "reset") == 0)
		{
			sprintf(req_info.path, IS_CLI_PATH_ACTION_SYSTEM_RESET);
			if (is_cli_cmd_reset_confirm() != 0)
				return 0;
			if(is_cli_cmd_do_request_core(&req_info, p_token_list, timeout) != 0)
				return -1;
			sleep(30);
			return 0;
		}
	}

	if(confirm)
	{
		if (0 != is_cli_cmd_reload_go_confirm())
			return 0;
	}

	if(is_cli_cmd_do_request_core(&req_info, p_token_list, timeout) != 0)
		return -1;

    return 0;
}


