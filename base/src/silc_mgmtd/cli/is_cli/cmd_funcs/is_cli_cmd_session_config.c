/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"

int silc_cli_session_timeout_config_update(void);

#define IS_CLI_PATH_CONFIG_MGMT	"/config/system/mgmt"

int is_cli_cmd_session_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;

		if(strcmp(p_token->name, "expired-time") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/session-exp-time");
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

int is_cli_cmd_session_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_session_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0 || req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

	silc_cli_session_timeout_config_update();
    return 0;
}


