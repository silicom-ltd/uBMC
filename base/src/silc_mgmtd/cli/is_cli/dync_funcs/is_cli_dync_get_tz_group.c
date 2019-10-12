/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"

//dynamic function token:timezone 
int is_cli_dync_get_tz_group(void* data, char* output_val_buf, int* p_val_str_len)
{
	int loop;
	int len = 20;
	*p_val_str_len = len;
	silc_cstr continents[] = {
			"America",
			"Asia",
			"Europe",
			"Africa",
			"Australia",
			"Pacific",
			"Antarctica",
			"Atlantic",
			"Arctic",
			"Etc",
	};

	for(loop=0; loop<sizeof(continents)/sizeof(silc_cstr); loop++)
	{
		sprintf(output_val_buf+loop*len, "%s", continents[loop]);
	}
    return loop;
}

