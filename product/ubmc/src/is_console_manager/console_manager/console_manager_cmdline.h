/*************************************************************************
 > File Name: console_manager_cmdline.h
 > Author: allen
 > Mail: allen.zhou@net-perf.com
 > Created Time: Fri 20 Apr 2018 06:10:04 PM HKT
 ************************************************************************/

#ifndef _CONSOLE_MANAGER_CMDLINE_H
#define _CONSOLE_MANAGER_CMDLINE_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <syslog.h>

//#define DEBUG

#define get_bit_status(x,y)  (x & 1 << y)

#define set_bit_status(x,y)  (x |= 1 << y)

#define clear_bit_status(x,y)  (x &= ~(1 << y))

#define min(x,y) (x < y ? x : y)
/*
 *
 * !!!note: when priority of syslog is LOG_DEBUG ,the content of log is redirected to /var/log/host_serial.log
 * if priority of syslog is others ,then redirect to /var/log/messages 
 *
 */
#ifdef CONSOLE_M_DEBUG

#define console_m_debug(fmt, ...) \
            syslog(LOG_DEBUG, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);
#define console_m_info(fmt, ...)                           syslog(LOG_INFO, fmt, ## __VA_ARGS__)
#define console_m_notice(fmt, ...)                         syslog(LOG_NOTICE, fmt, ## __VA_ARGS__)
#define console_m_warn(fmt, ...)                           syslog(LOG_WARNING, fmt, ## __VA_ARGS__)

#else

#define console_m_debug(fmt, ...)

#define console_m_info(fmt, ...)
#define console_m_notice(fmt, ...)
#define console_m_warn(fmt, ...)

#endif
#define console_m_err(fmt, ...) \
            syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);

/*
 *serial source 
 */
enum
{
	serial_s_uart_sim = 0, serial_s_tty_n_1, serial_s_max,
};

typedef enum
{
	shm_op_st_null = 0, shm_op_st_processing = 0, shm_op_st_confirm, shm_op_max,
} shm_op_status;

typedef enum
{
	shm_pos_serial_source = 0, shm_pos_status,
} shm_positon;

#define serial_source_1_msg_queue_key 120180420
#define serial_source_2_msg_queue_key 220180420

#define shm_key  20180629

#define SHMSZ (2 * sizeof(int))

#define MSG_QUEUE_BUFSIZE 128

enum
{
	CLI_CMD_TYPE_KICK_OUT_SOCKET = 1, CLI_CMD_TYPE_MAX,
};

// structure for message queue
struct mesg_buffer
{
	long mesg_type;
	char mesg_text[MSG_QUEUE_BUFSIZE];
};
typedef enum
{
	CONSOLE_CONFIG_TYPE_STATUS_LOG_ROTATION = 0,
	CONSOLE_CONFIG_TYPE_STATUS_HOST_SERIAL,
	CONSOLE_CONFIG_TYPE_STATUS_MAX,
} console_config_type_status_t;

#endif
