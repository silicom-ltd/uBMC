/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "ubmc_mgmtd_common.h"

#define LOCAL_IMAGE_PATH		"/var/images"

//dynamic function token:as 
int ubmc_cli_dync_get_dir_list(silc_cstr_array* lscmd, char* output_val_buf, int* p_val_str_len)
{
	int len = 100;
	int loop, count = 0, output_len= 4000;
	silc_cstr output, name;
	silc_cstr_array* p_arr;

	*p_val_str_len = len;

	output = malloc(output_len);
	if(!output)
		return 0;

	if(silc_mgmtd_if_exec_system_cmd(lscmd, output, &output_len, 1000, silc_false) != 0)
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


int ubmc_cli_dync_get_cdrom_list(void* data, char* output_val_buf, int* p_val_str_len)
{
	char cmd[64];
	snprintf(cmd,64,"ls %s|grep '\\.iso'",LOCAL_IMAGE_PATH);
	return ubmc_cli_dync_get_dir_list(cmd, output_val_buf, p_val_str_len);
}

int ubmc_cli_dync_get_image_list(void* data, char* output_val_buf, int* p_val_str_len)
{
	char cmd[64];
	snprintf(cmd,64,"ls %s|grep -v lost+found",LOCAL_IMAGE_PATH);
	return ubmc_cli_dync_get_dir_list(cmd, output_val_buf, p_val_str_len);
}

