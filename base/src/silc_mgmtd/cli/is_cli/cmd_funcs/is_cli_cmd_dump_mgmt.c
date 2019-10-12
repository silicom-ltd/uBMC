/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

int is_cli_cmd_dump_mgmt_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token;

	p_req_info->root_val = "";
	p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "create") == 0)
		{
			sprintf(p_req_info->path, "/action/system/create-log-dump");
		}
		else if(strcmp(p_token->name, "delete") == 0)
		{
#if 0
			char filename[128];
			char cmd[512];
			sprintf(filename, "/var/log/dump/%s", p_token->val_str);
			if(access(filename, F_OK) != 0)
				return -1;
			sprintf(cmd, "rm -rf /%s", filename);
			system(cmd);
			return 0;
#else
			sprintf(p_req_info->path, "/action/system/remove-log-dump");
			p_req_info->root_val = p_token->val_str;
#endif
		}
	}

	return 0;
}

int is_cli_cmd_dump_mgmt(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_dump_mgmt_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0)
		return -1;

	if(is_cli_cmd_do_request_core(&req_info, p_token_list, 30) != 0)
		return -1;

    return 0;
}

