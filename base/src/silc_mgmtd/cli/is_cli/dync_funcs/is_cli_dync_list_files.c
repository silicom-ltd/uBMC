/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"

//dynamic function token:packet 
int is_cli_dync_list_files(void* data, char* output_val_buf, int* p_val_str_len)
{
	int i;
	int len = 50;
	int count = 0;
	int buff_len = len*1000;
	silc_cstr buff;
	silc_cstr last_head, cur;

	buff = malloc(buff_len);
	if(!buff)
		return 0;

	if(silc_mgmtd_if_exec_system_cmd("ls -1 -F | grep -v [/$]", buff, &buff_len, 2000, silc_false) < 0)
	{
		free(buff);
		return 0;
	}

	last_head = cur = buff;
	for(i=0; i < buff_len; i++)
	{
		cur = buff + i;
		if(*cur == '\n')
		{
			if(last_head)
			{
				*cur = 0;
				strcpy(output_val_buf+count*len, last_head);
				count++;
				last_head = NULL;
			}
		}
		else
		{
			if(!last_head)
				last_head = cur;
		}
	}
	*p_val_str_len = len;
	free(buff);
	return count;
}

