/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"

//dynamic function token:local-auth 
int is_cli_dync_get_ipsec_auth_type(void* data, char* output_val_buf, int* p_val_str_len)
{
	int loop;
	int len = 20;
	silc_cstr names[] = {
		"pubkey",
		"psk",
		"eap",
		"eap-aka",
		"eap-gtc",
		"eap-md5",
		"eap-mschapv2",
		"eap-peap",
		"xauth",
		"xauth-generic",
		"xauth-eap",
	};
	int num = sizeof(names)/sizeof(silc_cstr);

	*p_val_str_len = len;
	for(loop=0; loop<num; loop++)
	{
		sprintf(output_val_buf+loop*len, "%s", names[loop]);
	}

	return num;
}

