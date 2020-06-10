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
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#include <pthread.h>
#include <string.h>
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
#include "dying_gasp.h"
#include "silc_common/silc_gpio.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int is_dying_gasp;
static int gpio_fd;
static dying_gasp_pids g_dying_gasp_pids;
static trap_hosts_table g_trap_hosts_table;



int init_mgmtd_client()
{
	int ret;
	ret = silc_mgmtd_if_client_init("127.0.0.1", 2626, -1, 0);
	if(ret < 0)
	{
		DG_DEBUG_ERR("silc_mgmtd_if_client_init is fail \n");
	}
	ret = mgmtd_client_config_check("/config/snmp", SILC_MGMTD_IF_REQ_QUERY_SUB, "");
	if(ret < 0)
	{
		DG_DEBUG_ERR("Failed to load console configuration \n");
		return ret;
	}
	ret = init_config_change_callback("/notify/snmp/config_changed",mgmtd_client_notify_config_changed_handler,NULL);
	if(ret < 0)
	{
		DG_DEBUG_ERR("Failed to load console configuration \n");
		return ret;
	}
	return ret;
}

int mgmtd_client_notify_config_changed_handler(silc_mgmtd_if_notify* p_notify,void* data)
{
	int ret;
	DG_DEBUG_INFO("snmp host config changed  \n");
    ret = update_trap_table(&g_trap_hosts_table);
    update_snmp_session_table(&g_trap_hosts_table);
    if (ret < 0)
    {
    	DG_DEBUG_ERR("update_trap_host_table fail \n");
    }
    return ret;
}
int do_kill(struct pids_need_to_kill_s pids,int sig)
{
	int ret,status;
	int i;
	status = 0;
	for(i = 0 ; i< pids.pid_num;i++)
	{
		ret = kill(pids.pids[i],sig);
		DG_DEBUG_INFO("kill %d ret is %d \n",g_dying_gasp_pids.umount_varlog.pids[i],ret);
		if(ret < 0)
		{
			status ++;
			DG_DEBUG_ERR("kill %d process fail,status is %d",pids.pids[i],-status);
		}
	}
	ret = status;
	return ret;
}
/*execute the cmd of killing is_mgmtd */
void *shutdown_mgmtd_handle(void *arg)
{
	int status;
	// pthread_mutex_lock(&(g_dying_gasp_pids.kill_mgmtd.lock));
	//status = system(g_dying_gasp_pids.kill_mgmtd.kill_cmd);
	status = do_kill(g_dying_gasp_pids.kill_mgmtd,SIGKILL);
	//DG_DEBUG_INFO("shutdown_mgmtd cmd %s \n",g_dying_gasp_pids.kill_mgmtd.kill_cmd);
	if(status < 0)
	{
		DG_DEBUG_ERR("cmd: %s\t error: %d", g_dying_gasp_pids.umount_varlog.kill_cmd, status);
		//return -1;
	 }
	// pthread_mutex_unlock(&(g_dying_gasp_pids.kill_mgmtd.lock));
	// DG_DEBUG_INFO("shutdown_mgmtd \n");
	// DYING_GASP_LOG("shutdown_mgmtd \n");
	 sleep(1);
	 return NULL;
}

/*execute the cmd of umounting /var/log and kill the process which use it */
void *umount_varlog_handle(void *arg)
{
	int status;
	//pthread_mutex_lock(&(g_dying_gasp_pids.umount_varlog.lock));
	DYING_GASP_LOG("do dying gasp  \n");
	status = do_kill(g_dying_gasp_pids.umount_varlog,SIGKILL);
	//status = system(g_dying_gasp_pids.umount_varlog.kill_cmd);
	DG_DEBUG_INFO("umount carlog_cmd %s \n",g_dying_gasp_pids.umount_varlog.kill_cmd);
	//pthread_mutex_unlock(&(g_dying_gasp_pids.umount_varlog.lock));
	if(status < 0)
	{
		DG_DEBUG_ERR("cmd: %s\t error: %d", g_dying_gasp_pids.umount_varlog.kill_cmd, status);
	}
	status = system("umount /var/log");
	if(status < 0)
	{
		DG_DEBUG_ERR("umount /var/log fail !status: %d \n", status);
	}
	//DYING_GASP_LOG("umount /var/log status: %d",status);
	sleep(1);
	return NULL;
}

#define PRODUCT_SUB_PATH "/etc/product_sub.txt"
#define UBMC_SUB_NAME_MAX 20
#define UBMC_M_NAME "UBMC_M"
#define UBMC_DEFAULT 0
#define UBMC_M 1
int get_machine_prod_sub(void)
{
	int ret;
	int ubmc_sub_type = UBMC_DEFAULT;
#if 1
	char buf[UBMC_SUB_NAME_MAX];
	char prod_sub_name[UBMC_SUB_NAME_MAX];
	char len;
	FILE *fp;
	if(access(PRODUCT_SUB_PATH,F_OK) != 0)
	{
		DG_DEBUG_ERR("Can not find %s file :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fp = fopen(PRODUCT_SUB_PATH,"r+");
	if(fp == NULL)
	{
		DG_DEBUG_ERR("open %s fail :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fgets(buf,UBMC_SUB_NAME_MAX,fp);
	if (buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';
	strcpy(prod_sub_name,buf);
	if(strcmp(prod_sub_name,UBMC_M_NAME) == 0)
	{
		ubmc_sub_type = UBMC_M;
	}
	else
	{
		ubmc_sub_type = UBMC_DEFAULT;
	}
	//ubmc_debug("model is %s\n",buf);
	fclose(fp);
#endif
	return ubmc_sub_type;
}

/*poll the  dying_gasp gpio and do something before power off */
int poll_dying_gasp()
{
	struct pollfd fdset;
	struct timeval time;
	netsnmp_session *ss;
	int status,ret,count,cnt,i,rc;
	//char buf[1024] = {0};
	char value;
	char gpio_path[200] = {0};
	char cmdbuf[256] = {0};
	pthread_t tid1,tid2;
	//char cmd[100] = {0};
	int temp;
	int gpio_pin = 0;
	int ubmc_sub_type;
	char bank_num = 0,pin_num = 0;
	//memset(buf,0x00,sizeof(char)*1024);
	memset(gpio_path,0x00,sizeof(char)*200);
	memset(cmdbuf,0x00,sizeof(char)*256);
	//gpio_pin = silc_gpio_get_by_name(DYING_GASP_PIN);
	ret =  silc_gpio_get_by_name(DYING_GASP_PIN,&bank_num,&pin_num);
	if(ret < 0)
	{
		DG_DEBUG_ERR("silc_gpio_get_by_name fail %d \n",gpio_pin);
		return -1;
	}
	/*
	 * this formulation should be changed when platform be different
	 * */
	ubmc_sub_type = get_machine_prod_sub();
	//ubmc_sub_type = 0;
	if(ubmc_sub_type < 0)
	{
		DG_DEBUG_ERR("Can not get right Product Sub Name \n");
		return -1;
	}
	else if(UBMC_M == ubmc_sub_type)
	{
		if(bank_num == 0)
		{
			gpio_pin = pin_num + 476;
		}
		else if(bank_num == 1)
		{
			gpio_pin = pin_num + 446;
		}
	}
	else
	{
		gpio_pin = 32 * (bank_num) + (pin_num);
	}
	sprintf(cmdbuf,"echo %d > /sys/class/gpio/export",gpio_pin);
	status = system(cmdbuf);
	if(status < 0)
	{
		DG_DEBUG_ERR("cmd: %s\t error: %d", cmdbuf, status);
		return -1;
	}
	memset(cmdbuf,0x00,sizeof(char)*256);
	sprintf(cmdbuf,"echo falling > /sys/class/gpio/gpio%d/edge",gpio_pin);
	status = system(cmdbuf);
	if(status < 0)
	{
		DG_DEBUG_ERR("cmd: %s\t error: %d", cmdbuf, status);
		return -1;
	}
	sprintf(gpio_path,"/sys/class/gpio/gpio%d/value",gpio_pin);
	DYING_GASP_LOG("dying gasp gpio path is %s \n",gpio_path);
	gpio_fd = open(gpio_path,O_RDWR);
	if(gpio_fd < 0)
	{
		DG_DEBUG_ERR("open gpio file is fail \n");
		return -2;
	}
	while (1)
	{
		memset(&fdset,0x00,sizeof(struct pollfd));
	    fdset.fd = gpio_fd;
	    fdset.events = POLLPRI;
	    ret = poll(&fdset,1,5000);
	    if (ret > 0)
	    {
	    	count = read(fdset.fd,&value,1);
	    	lseek(fdset.fd,0,SEEK_SET);
	    	if(value == '0')
	    	{
				pthread_mutex_lock(&mutex);
				is_dying_gasp = 1 ;
				pthread_mutex_unlock(&mutex);
				while(is_dying_gasp)
				{

					rc = pthread_create(&tid1, NULL, umount_varlog_handle, NULL);
					if(rc != 0)
					{
						DG_DEBUG_ERR("%s: %s\n",__func__, strerror(rc));
					}
					/*rc = pthread_create(&tid2, NULL, shutdown_mgmtd_handle, NULL);
					if(rc != 0)
					{
						DG_DEBUG_ERR("%s: %d\n",__func__, strerror(rc));
					}*/
					for(temp = 0;temp < g_trap_hosts_table.trap_table_num;temp ++)
					{
						ss = slic_snmp_create_session(g_trap_hosts_table.session[temp]->peername,g_trap_hosts_table.session[temp]->community, SNMP_VERSION_2c);
						if(ss == NULL)
						{
							DG_DEBUG_ERR("Failed to create session \n");
							continue;
						}
						ret = silc_snmp_send_simple_trap("", "dyinggasp", 7,ss);
						//DG_DEBUG_INFO("sent trap %d \n",temp);
					}

					//status = system(g_dying_gasp_pids.kill_mgmtd.kill_cmd);
					status = do_kill(g_dying_gasp_pids.kill_mgmtd,SIGKILL);
					if(status < 0)
					{
						DYING_GASP_ERR("kill is_mgmtd fail \n");
					}
					DG_DEBUG_INFO("dying gasp done");
					sleep(1);
					pthread_join(tid1 , NULL);
					//pthread_join(tid2 , NULL);
					return 0;
				}
	    	}
	    	else
	    	{
	    		DG_DEBUG_INFO("gpio leve is %c \n",value);
	    	}

	    }
	    else
	    {
	    	//listen_pid_change(&g_dying_gasp_pids.kill_mgmtd,"ps -ef|grep is_mgmtd|grep -v grep|awk \'{print $1}\'");
	    	//listen_pid_change(&g_dying_gasp_pids.umount_varlog,"lsof |grep /var/log | awk \'!a[$1]++{print $1}\'");

	    }

	}
}


void main(int argc, char **argv)
{
	int     arg;
	char 	ret ;
	char	status;
	int 	i;
	init_snmp("dying_gasp_trap");
	ret = init_mgmtd_client();

	if (ret < 0)
	{
		DG_DEBUG_ERR("init_mgmtd_client is fail \n");
	}
	init_trap_host_table(&g_trap_hosts_table);
	if(ret < 0)
	{
		DG_DEBUG_ERR("init_trap_host_table is fail \n");
	}
	update_trap_table(&g_trap_hosts_table);
	update_snmp_session_table(&g_trap_hosts_table);
	listen_pid_change(&g_dying_gasp_pids.kill_mgmtd,"ps -ef|grep is_mgmtd|grep -v grep|awk \'{print $1}\'");
	listen_pid_change(&g_dying_gasp_pids.umount_varlog,"lsof |grep /var/log | awk \'!a[$1]++{print $1}\'");

	ret = poll_dying_gasp();
	//snmp_close(ss);
	snmp_shutdown("dying_gasp_trap");

}
