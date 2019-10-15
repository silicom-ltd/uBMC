#include <time.h>
#include <sys/time.h>
#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi.h"
#include "ubmc_ipmi_check_st.h"
#include <time.h>
/*Checking driver status*/
void ubmc_ipmi_check_st_init(struct ubmc_ipmi_status_s *ubmc_ipmi_status,int dev_fd,device_type_t device_type)
{
	ubmc_ipmi_status->ipmi_dev_fd = dev_fd;
	ubmc_ipmi_status->device_type = device_type;
	ubmc_ipmi_status->update_flag = 0;
	ubmc_ipmi_status->status = 0;
	ubmc_ipmi_status->infor = 0;
	ubmc_ipmi_status->ipmi_time.tm_need_set = 0;
}

/*
int ubmc_ipmi_get_status_ioctl(struct ubmc_ipmi_status_s *ubmc_ipmi_status)
{
	int ret;
	int status;
	ret = ioctl(ubmc_ipmi_status->ipmi_dev_fd,IOCTL_LAN_INIT_CMD,&status);
	return ret;
}
*/

int ubmc_ipmi_get_time_ioctl(struct ubmc_ipmi_status_s *ubmc_ipmi_status)
{
	int ret;
	//time_t time_val = (time_t)stamp;
	ret = ioctl(ubmc_ipmi_status->ipmi_dev_fd,IOCTL_GET_SET_TIME_CMD,&ubmc_ipmi_status->ipmi_time);
	//printf("time is %d st is %d \n",ubmc_ipmi_status->ipmi_time.new_time,ubmc_ipmi_status->ipmi_time.tm_need_set);
	return ret;
}

#define NTP_VRF_PATH "/tmp/ntp.vrf"
int ubmc_ipmi_get_ntp_vrf(char *vrf_name)
{
	FILE* fp;
	char *find;
	char buf[64] = {0};
	if (ubmc_is_file_exist(NTP_VRF_PATH))
	{
		fp = fopen(NTP_VRF_PATH,"r");
		if(fp < 0)
		{
			//printf("ubmc_ipmi_get_ntp_vrf can not open file \n");
			return -1;
		}
		fgets(buf,64,fp);
		//find = strchr(buf, '\n');
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';
		strcpy(vrf_name,buf);
		fclose(fp);
		return 0;
	}
	else
	{
		//printf("ubmc_ipmi_get_ntp_vrf \n");
		vrf_name = NULL;
		return -1;
	}
}

int ubmc_ipmi_get_ntp_sync_st(char *vrf_name)
{
	int ret = 0;
	char cmd[128] = {0};
	char tmp[64] = {0};
	if(vrf_name != NULL)	//vrf exist
	{
		sprintf(cmd,"ip vrf exec %s ntpstat -p > /dev/null",vrf_name);
	}
	else
	{
		sprintf(cmd,"ntpstat > /dev/null");
	}
	ret = system(cmd);
	if(ret < 0)
	{
		ubmc_error("do ubmc_ipmi_get_ntp_sync_st fail:%s \n",cmd);
		return -1;
	}
    if (WIFEXITED(ret))
    {
    	//printf("subprocess exited, exit code: %d\n", WEXITSTATUS(rc));
    	if (0 == WEXITSTATUS(ret))
    	{

    		return 0;
    		//printf("command succeed \n");
    		//ntpd enable.dont do anything

    	}
    	else if(1 == WEXITSTATUS(ret))
		{
    		//ntp does not sync
    		return 1;

		}
    	else if(2 == WEXITSTATUS(ret))
    	{
    		//timeout maybe ntpd peer does not fit
    		return 2;
    	}
    	else
    	{
    		if(127 == WEXITSTATUS(ret))
    		{
    			//can not find this cmd
    			//printf("no this command \n");
    			return BMC_FALSE;
    		}
    	}
    }
    else
    {
    	//printf("subprocess exit failed\n");
    	return BMC_FALSE;
    }

}

int ubmc_ipmi_sync_date(struct ubmc_ipmi_status_s *ubmc_ipmi_status)
{
	struct timeval 	time_tv;
	int ret = 0;
	memcpy(&time_tv,&ubmc_ipmi_status->ipmi_time.new_time,sizeof(struct timeval));
	ret = settimeofday(&time_tv, NULL);
	if(ret != 0)
	{
		ubmc_error("settimeofday failed\n");
	    return -2;
	}
	if(ubmc_ipmi_status->device_type == LARGE || ubmc_ipmi_status->device_type == SMALL || ubmc_ipmi_status->device_type == MIDDLE)
	{
		ret = system("hwclock -w -f /dev/rtc1");
	}
	if(ret != 0)
	{
		ubmc_error("sync rtc1 failed\n");
		return -3;
	}
	return 0;
}
int ubmc_ipmi_check_status_handle(struct ubmc_ipmi_status_s *ubmc_ipmi_status)
{
	//char cmd[128] = {0};
	struct timeval 	time_tv;
	int ret = 0;
	char vrf_name[64] = {0};
	int ntp_sync = 0;
	if(ubmc_ipmi_get_time_ioctl(ubmc_ipmi_status) > 0)
	{
		if(ubmc_ipmi_status->ipmi_time.tm_need_set == 1)
		{
			//memset(cmd,'\0',128);
			//sprintf(cmd,"ps -ef|grep ntpd|grep -v grep > /dev/null");
			ret = system("ps -ef|grep ntpd|grep -v grep > /dev/null");
		    if(ret == -1)
		    {
		    	//do command fail
		    	//printf("command failed  \n");
		    	return BMC_FALSE;
		    }
		    if (WIFEXITED(ret))	//ntpd enable
		    {
		    	//printf("subprocess exited, exit code: %d\n", WEXITSTATUS(rc));
		    	//printf("ubmc_ipmi_check_status_handle enable \n");
		    	if (0 == WEXITSTATUS(ret))
		    	{
		    		if(ubmc_ipmi_get_ntp_vrf(vrf_name) == 0)
		    		{
		    			ntp_sync = ubmc_ipmi_get_ntp_sync_st(vrf_name);
		    			if(ntp_sync < 0)
		    			{
		    				ubmc_error("ubmc_ipmi_get_ntp_sync_st fail \n");
		    				return BMC_FALSE;
		    			}
		    		}
		    		else
		    		{
		    			ntp_sync = ubmc_ipmi_get_ntp_sync_st(NULL);
		    			if(ntp_sync < 0)
		    			{
		    				ubmc_error("ubmc_ipmi_get_ntp_sync_st fail \n");
		    				return BMC_FALSE;
		    			}

		    		}
		    		if(ntp_sync == 0)	//ntpd sync
		    		{

		    			//do not do anything
		    			;
		    		}
		    		else//ntpd do not sync
		    		{
		    			ubmc_ipmi_sync_date(ubmc_ipmi_status);
		    		}
		    		//printf("command succeed \n");
		    		//ntpd enable.dont do anything
		   			//printf("sync is %d \n",ntp_sync);
		    	}
		    	else if(1 == WEXITSTATUS(ret)) //ntpd disable
				{
		    		//printf("ubmc_ipmi_check_status_handle disable \n");
		    		//printf("command failed \n");
		    		//ntpd disable
		    		ret = ubmc_ipmi_sync_date(ubmc_ipmi_status);
		    		if(ret != 0)
		    		{
		    		    return -2;
		    		}

				}
		    	else
		    	{
		    		if(127 == WEXITSTATUS(ret))
		    		{
		    			//can not find this cmd
		    			//printf("no this command \n");
		    			return BMC_FALSE;
		    		}
		    	}
		    	return BMC_FALSE;
		    }
		    else
		    {
		    	//printf("subprocess exit failed\n");
		    	return BMC_FALSE;
		    }

		}
	}
}

void ubmc_ipmi_check_status_thread(void *data)
{
	struct timeval timeout = {0};
	int ret ;
	struct ubmc_ipmi_status_s* ipmi_status;
	ipmi_status = (struct ubmc_ipmi_lan_s*)data;

	if(ipmi_status == NULL)
	{
		UBMC_IPMI_ERRLOG("ubmc_ipmi_check_status_thread  is NULL \n");
		//return -1;
	}

	while(1)
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;		//check per 500 ms
		// just use the select funciton as a timer
	    switch( select(0,NULL,NULL,NULL,&timeout) )
		{
		case -1:
			/* Interrupted system call */
			//ubmc_debug(" Call select fail! errno = %d\n", errno);
			break;
		case 0:
			ubmc_ipmi_check_status_handle(ipmi_status);
			break;
		default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}

	  //  ubmc_ipmi_lan_netinf_update(&lan->eth0);
    }
	UBMC_IPMI_ERRLOG("ubmc_ipmi_lan_thread have finished  \n");
}
