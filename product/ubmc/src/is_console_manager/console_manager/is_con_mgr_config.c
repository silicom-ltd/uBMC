/*************************************************************************
 > File Name: is_config.c
 > Author: Jeff
 > Mail: jeff.zheng@silicom.co.il
 > Created Time: Mon 28 May 2018 11:06:15 AM HKT
 ************************************************************************/
#include "is_con_mgr_config.h"
#include "silc_common.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void is_config_init(is_config_console* p_cfg_console)
{

	memset(p_cfg_console, 0, sizeof(*p_cfg_console));

	p_cfg_console->log_rotate_size = 5;
	p_cfg_console->log_rotate_num = 20;
	p_cfg_console->is_log_to_file = true;
}

is_config_console* is_config_alloc(void)
{
	int ret __attribute__ ((unused));
	is_config_console* p_cfg_console;

	is_call_alloc(p_cfg_console, malloc, sizeof(is_config_console));

	is_config_init(p_cfg_console);

	return p_cfg_console;

	ERR_RET: return NULL;
}

