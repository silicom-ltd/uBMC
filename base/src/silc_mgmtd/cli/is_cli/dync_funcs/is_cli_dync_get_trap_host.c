/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

//dynamic function token:host 
int is_cli_dync_get_trap_host(void* data, char* output_val_buf, int* p_val_str_len)
{
	int len = 30;

	*p_val_str_len = len;
	return silc_cli_get_mgmtd_node_child_list("/config/snmp/agent/trap-hosts", output_val_buf, len);
}

