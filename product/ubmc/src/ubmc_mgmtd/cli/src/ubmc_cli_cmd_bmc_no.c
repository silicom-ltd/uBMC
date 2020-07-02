
#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"
#include "ubmc_mgmtd_common.h"

#define PATH_CONFIG_BMC		"/config/bmc"

int ubmc_cli_cmd_no_do_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token, *p_l2_token;
	static char none_str[10] = "";

	p_req_info->root_val = none_str;
	p_req_info->type = SILC_MGMTD_IF_REQ_DELETE;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
		{
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "bmc") == 0)
		{
			if(!p_l1_token)
				break;
			if(strcmp(p_l1_token->name, "console") == 0)
			{
				strcpy(p_req_info->path, PATH_CONFIG_BMC"/console");
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "usb-cdrom") == 0)
			{
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(p_l2_token && strcmp(p_l2_token->name, "local-iso") == 0)
				{
					sprintf(p_req_info->path, "/action/bmc/usb-cdrom/remove-local-isofile");
					p_req_info->root_val = p_l2_token->val_str;
					p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
					return 0;
				}
			}
		}
		else if(strcmp(p_token->name, "images") == 0)
		{
			sprintf(p_req_info->path, "/action/bmc/images/remove-local-image");
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
			return 0;
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

int ubmc_cli_cmd_no_do(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(ubmc_cli_cmd_no_do_get_req_info(p_token_list, &req_info) != 0)
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
