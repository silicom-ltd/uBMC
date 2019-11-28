/*************************************************************************
 > File Name: is_console_mgmtd_config.h
 > Author: Jeff
 > Mail: jeff.zheng@silicom.co.il
 > Created Time: Mon 28 May 2018 11:39:47 AM HKT
 ************************************************************************/

#ifndef _IS_CONSOLE_MGMTD_CONFIG_H
#define _IS_CONSOLE_MGMTD_CONFIG_H
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define is_err(fmt, ...)                fprintf(stderr, "[ERR ]"fmt"\n", ## __VA_ARGS__)

#define is_call_init(func, ...) \
	do { \
			ret = func(__VA_ARGS__);        \
			if(ret<0){ \
					is_err("Failed to initialize %s", #func); \
					goto ERR_RET; \
			} \
	}while(0)

#define is_call_create(ret_obj, func, ...) \
	do { \
			ret_obj = func(__VA_ARGS__);    \
			if(ret_obj==NULL){ \
					ret = -1; \
					is_err("Failed to create %s", #func); \
					if(ret!=0) goto ERR_RET; \
			} \
	}while(0)

#define is_call_alloc(ret_obj, func, ...) \
	do { \
			ret_obj = func(__VA_ARGS__);    \
			if(ret_obj==NULL){ \
					ret = -1; \
					is_err("Failed to allocate %s", #func); \
					goto ERR_RET; \
			} \
	}while(0)

typedef struct is_config_console_s
{
	bool com_hwflowctrl;
	bool com_swflowctrl;
	bool is_log_to_file;
	uint32_t log_rotate_num;
	uint32_t log_rotate_size;
	uint32_t com_speed;
	uint32_t com_data;
	uint32_t com_stopbits;
	char com_parity[32];


} is_config_console;

typedef enum
{
	BIOS_TYPE_UNKNOW = 0,
	BIOS_TYPE_UBMC,
} bios_boot_type;
typedef enum
{
	BIOS_BOOT_START = 0,
	BIOS_BOOT_STATE_1,
	BIOS_BOOT_STATE_2,
	BIOS_BOOT_STATE_3,
	BIOS_BOOT_STATE_4,
	BIOS_BOOT_END,
} bios_boot_state;

typedef struct host_console_match_str_s
{
	uint32_t match_str_pos; //record the match string position
	bios_boot_type bios_type;
	bios_boot_state bios_state;
	bool str_match_enable;
}host_console_match_str;

is_config_console* is_config_alloc(void);
void is_config_init(is_config_console* p_cfg_console);

#endif
