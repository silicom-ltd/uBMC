/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"

//dynamic function token:configuration 
int is_cli_dync_get_config_list(void* data, char* output_val_buf, int* p_val_str_len)
{
	int len = 100;
	*p_val_str_len = len;
	int loop, count = 0, output_len= 4000;
	silc_cstr output, name;
	silc_cstr_array* p_arr;

	output = malloc(output_len);
	if(!output)
		return 0;
#ifndef SILC_MGMTD_LOCAL_DEBUG
	if(silc_mgmtd_if_exec_system_cmd("ls -l /config/extend|awk '{print $9}'", output, &output_len, 1000, silc_false) != 0)
#else
	if(silc_mgmtd_if_exec_system_cmd("ls -l ./extend|awk '{print $9}'", output, &output_len, 1000, silc_false) != 0)
#endif
	{
		free(output);
		return 0;
	}

	p_arr = silc_cstr_split(output, "\n");
	for(loop=0; loop<p_arr->length; loop++)
	{
		name = silc_cstr_array_get_quick(p_arr, loop);
		if(strlen(name) > 0)
		{
			sprintf(output_val_buf+count*len, "%s", name);
			count++;
		}
	}
	silc_cstr_array_destroy(p_arr);
	free(output);
    return count;
}

