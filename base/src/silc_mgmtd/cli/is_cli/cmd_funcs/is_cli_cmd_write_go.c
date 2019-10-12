/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

int is_cli_cmd_write_go(silc_list* p_token_list)
{
	char err_info[200];
#if 0
	silc_cli_token *p_token;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "as") == 0)
		{
			if(is_cli_cmd_check_config_file(p_token->val_str) != 0)
				return 0;
			if(silc_cli_cmd_do_simple_action("/action/system/save-config-as", p_token->val_str, err_info, 200) != 0)
			{
				silc_cli_print("%% Write configuration failed, error:%s\n", err_info);
				return -1;
			}
			return 0;
		}
	}
#endif
	if(silc_cli_cmd_do_simple_action("/action/system/save-config", "", err_info, 200) != 0)
	{
		silc_cli_print("%% Write configuration failed, error:%s\n", err_info);
		return -1;
	}
    return 0;
}

