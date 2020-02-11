/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"
#include "ubmc_mgmtd_common.h"

int ubmc_cli_cmd_phonehome_config(silc_list* p_token_list)
{
	silc_cli_token *p_token;
	char path[200], err_info[200];

	if(silc_cli_cmd_do_simple_action("/action/system/check-is-admin", "true", NULL, 0) != 0)
        {
                silc_cli_err_cmd_set_err_info("no privilege to control PhoneHome service");
                return -1;
        }
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "start") == 0 || strcmp(p_token->name, "stop") == 0)
		{
			strcpy(path, "/action/bmc/phonehome/enabled");
			if(silc_cli_cmd_do_simple_action(path, p_token->map_val, err_info, 200) != 0)
			{
				silc_cli_print("%% Set %s failed! Error: %s.\n", path, err_info);
				return -1;
			}
			return 0;
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

