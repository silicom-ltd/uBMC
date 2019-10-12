/*
 * ubmc_ipmi_lan.h
 *
 *  Created on: Nov 8, 2018
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_LAN_H_
#define INCLUDE_UBMC_IPMI_LAN_H_


#define MAC_SIZE    16
#define IP_SIZE     16
#define SUBNET_MASK_SIZE     16
#define ETH_NAME_SIZE 16

#define IPMI_LAN_ACCESSBIL 1
#define IPMI_LAN_UNACCESSBIL 0

#define UNSPECIFIED 0
#define STATIC 1
#define DHCP 2

struct ipmi_netinf
{
	int id;
	char eth_name[ETH_NAME_SIZE];
	char mac_adr[MAC_SIZE];
	char mac_adr_status;
	unsigned int ip_adr;
	char ip_adr_status;
	unsigned int subnet_mask;
	char subnet_mask_status;
	char ip_source;
	int socket_ctr_fd;
	char staus;
	int ipmi_dev_fd;

};


struct ubmc_ipmi_lan_s
{
    int ipmi_dev_fd;
	struct ipmi_netinf eth0;
	struct ipmi_netinf eth1;
};


void ubmc_ipmi_lan_thread(void *data);
void ubmc_ipmi_lan_init(struct ubmc_ipmi_lan_s *lan,int dev_fd);

#endif /* INCLUDE_UBMC_IPMI_LAN_H_ */


