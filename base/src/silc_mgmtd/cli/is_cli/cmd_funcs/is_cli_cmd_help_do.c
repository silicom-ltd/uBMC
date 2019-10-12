/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"

int is_cli_cmd_help_do(silc_list* p_token_list)
{
	silc_cli_print("You may request context-sensitive help at any time by pressing '?'\n");
	silc_cli_print("on the command line.  This will show a list of choices for the\n");
	silc_cli_print("word you are on, or a list of top-level commands if you have not\n");
	silc_cli_print("typed anything yet.\n\n");
	silc_cli_print("If \"<cr>\" is shown, that means that what you have entered so far\n");
	silc_cli_print("is a complete command, and you may press Enter (carriage return)\n");
	silc_cli_print("to execute it.\n\n");
	silc_cli_print("Try the following to get started:\n");
	silc_cli_print("  ?\n");
	silc_cli_print("  show c?\n");
	silc_cli_print("  show clock?\n");
	silc_cli_print("  show clock ?\n");
    return 0;
}

