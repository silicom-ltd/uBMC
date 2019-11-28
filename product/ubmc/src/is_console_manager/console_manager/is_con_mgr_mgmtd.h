/*************************************************************************
	> File Name: is_console_mgmtd_client.h
	> Author: Jeff
	> Mail: jeff.zheng@silicom.co.il
	> Created Time: Mon 28 May 2018 10:02:11 AM HKT
 ************************************************************************/

#ifndef _IS_CONSOLE_MGMTD_CLIENT_H
#define _IS_CONSOLE_MGMTD_CLIENT_H
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "is_con_mgr_config.h"

typedef struct is_console_s
{
    is_config_console *   p_config;
    host_console_match_str match_str;
    bool is_status_changed;
    bool is_initalized;
}is_console;

typedef struct is_console_mgmtd_client_s
{
	pthread_mutex_t lock;   //this lock is for locking notification for initial configuration change
	struct is_console_s* p_console;

}is_con_mgr_mgmtd_client;

int is_cons_mgr_mgmtd_client_init(is_console * p_console);
bool is_cons_mgr_mgmtd_get_param_changed_status(is_console * p_console);

#endif
