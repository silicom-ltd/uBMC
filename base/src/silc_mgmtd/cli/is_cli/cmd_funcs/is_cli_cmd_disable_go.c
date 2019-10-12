/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_error.h"

int is_cli_cmd_disable_go(silc_list* p_token_list)
{
	if(silc_cli_mode_curr_set("user_exec") != 0)
	{
		silc_cli_err_cmd_set_err_info("change to user command mode failed");
		return -1;
	}
	return 0;
}

