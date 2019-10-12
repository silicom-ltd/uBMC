/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

#define IS_CLI_PATH_CONFIG_MISC	"/config/system/misc"

int is_cli_cmd_misc_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token;
	static char none_str[10] = "";

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;

		if(strcmp(p_token->name, "name") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MISC"/hostname");
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		else if(strcmp(p_token->name, "login-banner") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MISC"/login-banner");
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		else if(strcmp(p_token->name, "log") == 0)
		{
			silc_cli_token* p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
			if(strcmp(p_l1_token->name, "reset") == 0)
			{
				sprintf(p_req_info->path, "/action/system/reset-log");
				p_req_info->root_val = none_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "remote") == 0)
			{
				silc_cli_token* p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(strcmp(p_l2_token->name, "server") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MISC"/log/remote-server-v2/%s", p_l2_token->val_str);
					p_req_info->root_val = p_l2_token->val_str;
					p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
					return 0;
				}
			}
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MISC"/log");
			p_req_info->root_val = none_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

int is_cli_cmd_misc_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_misc_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0 || req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}

