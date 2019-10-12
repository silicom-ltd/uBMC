/*
 * silc_snmp_trap_fast.h
 *
 *  Created on: Jul 12, 2018
 *      Author: jason_zhang
 */

#ifndef INC_SILC_SNMP_TRAP_FAST_H_
#define INC_SILC_SNMP_TRAP_FAST_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <syslog.h>
#include <stdbool.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_module_config.h>
#include <net-snmp/agent/mib_module_config.h>
#include "silc_mgmtd_variables.h"
#include "silc_mgmtd_connect.h"
#include "silc_mgmtd_interface.h"

//#define DYING_GASP_DEBUG

#ifdef  DYING_GASP_DEBUG
#define DG_DEBUG_LINE() printf("[%s:%s] line=%d\r\n",__FILE__, __func__, __LINE__)
//#define DEBUG_ERR(fmt, args...) printf("\033[46;31m[%s:%d]\033[0m "#fmt" errno=%d, %m\r\n", __func__, __LINE__, ##args, errno, errno)
#define DG_DEBUG_INFO(fmt, args...) printf("\033[33m[%s:%d]\033[0m "#fmt"\r\n", __func__, __LINE__, ##args)
#define DG_DEBUG_ERR(fmt, args...) printf("[ERROR][%s:%s] line=%d "#fmt"\r\n",__FILE__, __func__, __LINE__)
#else
#define DG_DEBUG_LINE()
#define DG_DEBUG_ERR(fmt, ...) syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define DG_DEBUG_INFO(fmt, ...)
#endif

#define DYING_GASP_ERR(fmt, args...) printf("[ERROR][%s:%s] line=%d "#fmt"\r\n",__FILE__, __func__, __LINE__)
#define DYING_GASP_LOG(fmt, ...) syslog(LOG_NOTICE	, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
//#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

//#define GPIO_BANK 2        //power off
//#define GPIO_PIN 4
//#define GPIO_BANK 1	  //button down
//#define GPIO_PIN 25


#ifdef DYING_GASP_SYSLOG
#define dying_gasp_debug(fmt, ...) \
            syslog(LOG_DEBUG, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define dying_gasp_info(fmt, ...)                           syslog(LOG_INFO, fmt, ## __VA_ARGS__)
#define dying_gasp_notice(fmt, ...)                         syslog(LOG_NOTICE, fmt, ## __VA_ARGS__)
#define dying_gasp_warn(fmt, ...)                           syslog(LOG_WARNING, fmt, ## __VA_ARGS__)
#define dying_gasp_err(fmt, ...) \
            syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#else
#define dying_gasp_debug(fmt, ...)
#define dying_gasp_info(fmt, ...)
#define dying_gasp_notice(fmt, ...)
#define dying_gasp_warn(fmt, ...)
#define dying_gasp_err(fmt, ...)
#endif

#define ERR_EXIT(m) \
    do \
{ \
    perror(m); \
    exit(EXIT_FAILURE); \
} while(0)

#define is_err(fmt, ...)                fprintf(stderr, "[ERR ]"fmt"\n", ## __VA_ARGS__)

#define PID_MAXSIZE 10
#define CMD_MAXSIZE 256
struct pids_need_to_kill_s
{
	int pids[PID_MAXSIZE];
	int pid_num;
	char kill_cmd[CMD_MAXSIZE];
	pthread_mutex_t lock;

};
typedef struct dying_gasp_pids_s
{
	struct pids_need_to_kill_s kill_mgmtd;
	struct pids_need_to_kill_s umount_varlog;
}dying_gasp_pids;

#define PEERNAME_SIZE 30
#define COMMUNITY_SIZE 50
#define AUTH_SIZE 50
#define IP_SIZE 20
#define VERSION_SIZE 10
#define PASSWORD_SIZE 50
#define CMD_BUF_SIZE 256
typedef struct trap_table_s
{
	char ip[IP_SIZE];
	bool state;
	unsigned int port ;
	char version[VERSION_SIZE];
	char auth[AUTH_SIZE];
	char community[COMMUNITY_SIZE];
	char password[PASSWORD_SIZE];
	char peername[PEERNAME_SIZE];
	char table_sum;

}trap_table;
#define TRAP_TABLE_MAXSIZE 25
typedef struct trap_hosts_table_s
{
	trap_table *trap_table[TRAP_TABLE_MAXSIZE];
	netsnmp_session *session[TRAP_TABLE_MAXSIZE];
	unsigned int trap_table_num;

}trap_hosts_table;



netsnmp_session* slic_snmp_create_session(char *peername,char *com, int version);
#define DYING_GASP_PIN "DYING_GASP_N"
//#define DYING_GASP_PIN "FP_PWRBTN_N"
int silc_snmp_send_simple_trap(char* trap_name, char* module, int event,netsnmp_session* session);
int listen_pid_change(struct pids_need_to_kill_s* pid,const char* cmd);
int mgmtd_client_config_check(const silc_cstr path ,silc_mgmtd_if_req_type type, const silc_cstr node_root_val_str);
int init_config_change_callback(const silc_cstr cfg_changed_path,silc_mgmtd_if_notify_cb daemon_entry,void *data);
int update_trap_table(trap_hosts_table *table);

int update_snmp_session_table(trap_hosts_table *table);
int init_trap_host_table(trap_hosts_table *table);
int mgmtd_client_notify_config_changed_handler(silc_mgmtd_if_notify* p_notify,void* data);
int init_mgmtd_client(void);

#endif /* INC_SILC_SNMP_TRAP_FAST_H_ */
