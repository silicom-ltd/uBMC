/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_common.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_readline.h"
#include "silc_cli_error.h"

int is_cli_cmd_cli_config(silc_list* p_token_list)
{
	silc_cli_token* p_token;

	if(silc_cli_token_find_from_list(p_token_list, "clear-history"))
	{
		silc_cli_rl_clear_history();
	}
	else if((p_token=silc_cli_token_find_from_list(p_token_list, "auto-logout")))
	{
		long timeout;
		if(silc_mgmtd_if_cstr2l(p_token->val_str, &timeout) != 0)
		{
			silc_cli_err_cmd_set_invalid_param(p_token->val_str);
			return -1;
		}
		if(timeout<0)
		{
			silc_cli_err_cmd_set_invalid_param(p_token->val_str);
			return -1;
		}
//		silc_mgmtd_if_client_set_local_session_timeout((int)timeout);
		if(silc_cli_set_mgmtd_session_expire_timeout(p_token->val_str) !=0)
		{
			return -1;
		}
	}
    return 0;
}

