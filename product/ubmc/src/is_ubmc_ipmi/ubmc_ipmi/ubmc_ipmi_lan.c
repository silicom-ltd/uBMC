/*
 * ubmc_ipmi_lan.c
 *
 *  Created on: Nov 8, 2018
 *      Author: jason_zhang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "ubmc_ipmi_lan.h"
#include "ubmc_ipmi.h"


void prt_netinf_info(struct ipmi_netinf* netinf)
{
	struct ipmi_netinf tmp;
	char ip[4];
	char mask[4];
	unsigned int net_ip;
	unsigned int net_mask;

	memcpy(&tmp,netinf,sizeof(struct ipmi_netinf));
	net_ip = htonl(tmp.ip_adr);
	net_mask= htonl(tmp.subnet_mask);
	memcpy(&ip,&net_ip,sizeof(unsigned int ));
	memcpy(&mask,&net_mask,sizeof(unsigned int));
	printf("prt_info : net_ip is %x ,net_mask is %x \n",net_ip,net_mask);
	printf("name is %s \n",tmp.eth_name);
	printf("ip is %d.%d.%d.%d ,ip state is %d \n",ip[0],ip[1],ip[2],ip[3],tmp.ip_adr_status);
	printf("mac is %x:%x:%x:%x:%x:%x ,mac state is %d \n",tmp.mac_adr[0],tmp.mac_adr[1],tmp.mac_adr[2],tmp.mac_adr[3],tmp.mac_adr[4],tmp.mac_adr[5],tmp.mac_adr_status);
	printf("mask is %d.%d.%d.%d ,mask state is %d \n",mask[0],mask[1],mask[2],mask[3],tmp.subnet_mask_status);

}


int ubmc_ipmi_get_ip_source(struct ipmi_netinf *netinf)
{
	char cmd[256];
	int ret;
	FILE *fp;
    struct ipmi_netinf new;
    memset(&new,0,sizeof(struct ipmi_netinf));
    memcpy(&new,netinf,sizeof(struct ipmi_netinf));
    new.ip_source = UNSPECIFIED;
	sprintf(cmd,"ps -ef|grep dhclient|grep %s |grep -v grep > /dev/null",new.eth_name);
	ret = system(cmd);
    if(ret == -1)
    {
    	//do command fail
    	return BMC_FALSE;
    }
    if (WIFEXITED(ret))
    {
    	//printf("subprocess exited, exit code: %d\n", WEXITSTATUS(rc));
    	if (0 == WEXITSTATUS(ret))
    	{
    		//printf("command succeed\n");
    		new.ip_source = DHCP;
    	}
    	else if(1 == WEXITSTATUS(ret))
		{
    		new.ip_source = STATIC;
		}
    	else
    	{
    		if(127 == WEXITSTATUS(ret))
    		{
    			//can not find this cmd
    			return BMC_FALSE;
    		}
    	}
    }
    else
    {
    	//printf("subprocess exit failed\n");
    	return BMC_FALSE;
    }
    if(memcmp(netinf,&new,sizeof(struct ipmi_netinf)) == 0 )
    {
    	ret = 0;
    }
    else
    {
    	memcpy(netinf,&new,sizeof(struct ipmi_netinf));
    	ret = 1;
    }
	return ret ;
}

int ubmc_ipmi_get_local_mac(struct ipmi_netinf *netinf)
{
    struct ifreq ifr;
    int sd;
    int ret;
    struct ipmi_netinf new;
    bzero(&ifr, sizeof(struct ifreq));
    memset(&new,0,sizeof(struct ipmi_netinf));
    memcpy(&new,netinf,sizeof(struct ipmi_netinf));

    if( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
    	ubmc_debug("get %s mac address socket creat error\n",new.eth_name);
        return -1;
    }

    strncpy(ifr.ifr_name, new.eth_name, sizeof(ifr.ifr_name) - 1);

    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
    	new.mac_adr_status = IPMI_LAN_UNACCESSBIL;
    	ubmc_debug("get %s mac address error\n", new.eth_name);
        close(sd);
        return -1;
    }
    memcpy(new.mac_adr,ifr.ifr_hwaddr.sa_data,MAC_SIZE);
    new.mac_adr_status = IPMI_LAN_ACCESSBIL;
    if(memcmp(netinf,&new,sizeof(struct ipmi_netinf)) == 0 )
    {
    	ret = 0;
    }
    else
    {
    	memcpy(netinf,&new,sizeof(struct ipmi_netinf));
    	ret = 1;
    }
    /*
    snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
*/
    close(sd);
    return ret;
}


//
int ubmc_ipmi_get_local_ip(struct ipmi_netinf *netinf)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
    struct ipmi_netinf new;
    int ret;
    bzero(&ifr, sizeof(struct ifreq));
    memset(&new,0,sizeof(struct ipmi_netinf));
    memcpy(&new,netinf,sizeof(struct ipmi_netinf));
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
    	ubmc_debug("socket error: %s\n", strerror(errno));
        return -1;
    }

    strcpy(ifr.ifr_name, new.eth_name);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        //ubmc_debug("ioctl error: %s\n", strerror(errno));
        new.ip_adr_status = IPMI_LAN_UNACCESSBIL;
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
   // snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
    new.ip_adr = htonl(sin.sin_addr.s_addr);
    new.ip_adr_status = IPMI_LAN_ACCESSBIL;
    if(memcmp(netinf,&new,sizeof(struct ipmi_netinf)) == 0 )
    {
    	ret = 0;
    }
    else
    {
    	memcpy(netinf,&new,sizeof(struct ipmi_netinf));
    	ret = 1;
    }
    close(sd);
    return ret;
}

int ubmc_ipmi_get_subnet_mask(struct ipmi_netinf *netinf)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
    struct ipmi_netinf new;
    int ret;
    bzero(&ifr, sizeof(struct ifreq));
    memset(&new,0,sizeof(struct ipmi_netinf));
    memcpy(&new,netinf,sizeof(struct ipmi_netinf));
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
    	ubmc_debug("socket error: %s\n", strerror(errno));
        return -1;
    }
    strcpy(ifr.ifr_name, new.eth_name);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFNETMASK, &ifr) < 0)
    {
       // printf("ioctl error: %s\n", strerror(errno));
        new.subnet_mask_status = IPMI_LAN_UNACCESSBIL;
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
   // snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
    new.subnet_mask = htonl(sin.sin_addr.s_addr);
    new.subnet_mask_status = IPMI_LAN_ACCESSBIL;
    if(memcmp(netinf,&new,sizeof(struct ipmi_netinf)) == 0 )
    {
    	ret = 0;
    }
    else
    {
    	memcpy(netinf,&new,sizeof(struct ipmi_netinf));
    	ret = 1;
    }
    close(sd);
    return ret;
}
void ubmc_ipmi_lan_init(struct ubmc_ipmi_lan_s *lan,int dev_fd)
{
	memset(lan,0,sizeof(struct ubmc_ipmi_lan_s));
	lan->ipmi_dev_fd = dev_fd;
	lan->eth0.ipmi_dev_fd = dev_fd;
	lan->eth1.ipmi_dev_fd = dev_fd;
}


int ubmc_ipmi_lan_init_ioctl(struct ubmc_ipmi_lan_s *lan)
{
	int ret;
	ret = ioctl(lan->ipmi_dev_fd,IOCTL_LAN_INIT_CMD,lan);
	return ret;
}

int ubmc_ipmi_netinf_update_ioctl(struct ipmi_netinf *netinf)
{
	int ret;
	ret = ioctl(netinf->ipmi_dev_fd,IOCTL_LAN_UPDATE_CMD,netinf);
	return ret;
}

int ubmc_ipmi_lan_register_netinf(struct ipmi_netinf *netinf,const char* eth_name,int id)
{
	int ret;
	strcpy(netinf->eth_name,eth_name);
	netinf->id = id;
	netinf->ip_adr_status = IPMI_LAN_UNACCESSBIL;
	netinf->mac_adr_status = IPMI_LAN_UNACCESSBIL;
	netinf->subnet_mask_status = IPMI_LAN_UNACCESSBIL;
}

int ubmc_ipmi_lan_netinf_update(struct ipmi_netinf *netinf)
{
	int update = 0;
	int ret;
	if(ubmc_ipmi_get_local_mac(netinf) == 1)
		update ++;
	if(ubmc_ipmi_get_local_ip(netinf) == 1)
		update ++;
	if(ubmc_ipmi_get_subnet_mask(netinf) == 1)
		update ++;
	if(ubmc_ipmi_get_ip_source(netinf) == 1)
		update ++;
	//prt_netinf_info(netinf);
	if(update > 0)
	{
		ret = ubmc_ipmi_netinf_update_ioctl(netinf);
	}
	return ret;
}

void ubmc_ipmi_lan_thread(void *data)
{
	struct timeval timeout = {0};
	int lan_is_ok = 1;
	int ret ;
	struct ubmc_ipmi_lan_s* lan;
	lan = (struct ubmc_ipmi_lan_s*)data;

	if(lan == NULL)
	{
		UBMC_IPMI_ERRLOG("ubmc_ipmi_lan_thread lan is NULL \n");
		//return -1;
	}
	ubmc_ipmi_lan_init_ioctl(lan);
	ubmc_ipmi_lan_register_netinf(&lan->eth0,"eth0",1);
	ubmc_ipmi_lan_register_netinf(&lan->eth1,"eth1",2);
	ret = ubmc_ipmi_lan_netinf_update(&lan->eth0);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ubmc_ipmi_lan_netinf_update eth0 fail \n");
		lan_is_ok = 0;
	}
	ret = ubmc_ipmi_lan_netinf_update(&lan->eth1);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ubmc_ipmi_lan_netinf_update eth1 fail \n");
		lan_is_ok = 0;
	}
	while(lan_is_ok)
	{
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		// just use the select funciton as a timer
	    switch( select(0,NULL,NULL,NULL,&timeout) )
		{
		case -1:
			/* Interrupted system call */
			//ubmc_debug(" Call select fail! errno = %d\n", errno);
			break;
		case 0:
			if(ubmc_ipmi_lan_netinf_update(&lan->eth0) < 0)
			{

			}
			if(ubmc_ipmi_lan_netinf_update(&lan->eth1) < 0)
			{

			}
			break;
		default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}

	  //  ubmc_ipmi_lan_netinf_update(&lan->eth0);
    }
	UBMC_IPMI_ERRLOG("ubmc_ipmi_lan_thread have finished  \n");
}
