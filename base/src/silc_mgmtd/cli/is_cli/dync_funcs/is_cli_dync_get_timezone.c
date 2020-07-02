/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

#ifndef SILC_MGMTD_LOCAL_DEBUG
#define IS_CLI_ZONEINFO_DIR		"/usr/share/zoneinfo/posix"
#else
#define IS_CLI_ZONEINFO_DIR		"/usr/share/zoneinfo"
#endif
//dynamic function
int is_cli_dync_get_timezone(void* data, char* output_val_buf, int* p_val_str_len)
{
	int len = 20;
	char cmd[200], path[200];
	int loop, count = 0, output_len= 4000;
	silc_cstr output;
	silc_cstr continent = data;
	silc_cstr area;
	silc_cstr_array* p_arr;

	if(!data)
		return 0;

	*p_val_str_len = len;
	if(strcmp(continent, "Etc") == 0)
	{
		sprintf(output_val_buf+count*len, "UTC");
		return 1;
	}
	else
	{
		int slen = strlen(continent);
		char c;
		for(loop=0; loop<slen; loop++)
		{
			c = continent[loop];
			if ((c < '0' || c > '9') &&
					(c < 'a' || c > 'z') &&
					(c < 'A' || c > 'Z'))
				return 0;
		}
	}
	sprintf(path, "%s/%s", IS_CLI_ZONEINFO_DIR, continent);
	if(!silc_mgmtd_if_dir_exist(path))
		return 0;

	output = malloc(output_len);
	if(!output)
		return 0;
	sprintf(cmd, "ls -l %s|grep -v drw|awk '{print $9}'", path);
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &output_len, 1000, silc_false) != 0)
	{
		free(output);
		return 0;
	}

	p_arr = silc_cstr_split(output, "\n");
	for(loop=0; loop<p_arr->length; loop++)
	{
		area = silc_cstr_array_get_quick(p_arr, loop);
		if(strlen(area) > 0)
		{
			sprintf(output_val_buf+count*len, "%s", area);
			count++;
		}
	}
	silc_cstr_array_destroy(p_arr);
	free(output);
    return count;
}

