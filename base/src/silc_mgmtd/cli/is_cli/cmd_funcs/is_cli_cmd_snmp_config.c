/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

#define IS_CLI_PATH_CONFIG_SNMP_TRAP_HOST	"/config/snmp/agent/trap-hosts"
#define IS_CLI_PATH_CONFIG_SNMP_COMMUNITIES	"/config/snmp/agent/communities"
#define IS_CLI_PATH_CONFIG_SNMP_USERS		"/config/snmp/agent/users"
#define IS_CLI_PATH_CONFIG_SNMP_STATE		"/config/snmp/agent"
#define IS_CLI_PATH_CONFIG_SNMP_TRAP		"/config/snmp/trap"
#define IS_CLI_PATH_CONFIG_SNMP_FAN			"/config/snmp/threshold/fan"
#define IS_CLI_PATH_CONFIG_SNMP_SENSOR		"/config/snmp/threshold/sensor"
#define IS_CLI_PATH_ACTION_SNMP_APPLY		"/action/snmp/apply"

static int s_snmp_need_apply = 1;

int is_cli_cmd_snmp_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	int token_idx = 0;
	silc_cli_token* p_token;
	int ret = -1;

	s_snmp_need_apply = 1;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "disable") == 0 ||
				strcmp(p_token->name, "enable") == 0)
		{
			if(token_idx == 1)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_STATE"/%s", p_token->map_name);
				p_req_info->root_val = p_token->map_val;
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else
			{
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
		}
		if(strcmp(p_token->name, "full-access") == 0 || strcmp(p_token->name, "read-only") == 0)
		{
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}

		token_idx++;
		if(p_token->map_name)
			continue;
		if(strcmp(p_token->name, "apply") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_ACTION_SNMP_APPLY);
			p_req_info->root_val = silc_mgmtd_if_path_get_last_name(p_req_info->path);
			p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
			return 0;
		}
		else if(strcmp(p_token->name, "community") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_COMMUNITIES"/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			ret = 0;
		}
		else if(strcmp(p_token->name, "host") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_TRAP_HOST"/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			ret = 0;
		}
		else if(strcmp(p_token->name, "user") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_USERS"/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			ret = 0;
		}
		else if(strcmp(p_token->name, "trap") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_TRAP);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			ret = 0;
		}
		else if(strcmp(p_token->name, "fan") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_FAN);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			s_snmp_need_apply = 0;
			ret = 0;
		}
		else if(strcmp(p_token->name, "sensor") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_SENSOR);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			s_snmp_need_apply = 0;
			ret = 0;
		}

	}
	if(ret)
		silc_cli_err_cmd_set_invalid_cmd();
	return ret;
}

int is_cli_cmd_snmp_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_snmp_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0 || req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

	if(s_snmp_need_apply && req_info.type != SILC_MGMTD_IF_REQ_ACTION)
		silc_cli_print("New SNMP setting will take effect after 'snmp apply'.\n");

    return 0;
}


