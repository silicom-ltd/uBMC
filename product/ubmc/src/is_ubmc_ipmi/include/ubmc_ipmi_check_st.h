/*
 * ubmc_ipmi_check_st.h
 *
 *  Created on: Jun 25, 2019
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_CHECK_ST_H_
#define INCLUDE_UBMC_IPMI_CHECK_ST_H_
#include "ubmc_ipmi.h"
#include "ubmc_ipmi_board.h"
//need to sync with ipmi driver :ubmc-i2c-ipmi.h
#define STATUS_SET_TIME	0x01
struct ubmc_ipmi_time
{
	bool tm_need_set;	//if it is 0,indicate that there are something need to do by checking io_status
	struct timeval new_time;	//ipmitool sel time set time which want to set
};
struct ubmc_ipmi_status_s
{
    int ipmi_dev_fd;
	bool update_flag;
	device_type_t device_type;
	int status;
	struct ubmc_ipmi_time ipmi_time;
	int infor;
};

void ubmc_ipmi_check_st_init(struct ubmc_ipmi_status_s *ubmc_ipmi_status,int dev_fd,device_type_t device_type);
void ubmc_ipmi_check_status_thread(void *data);
#endif /* INCLUDE_UBMC_IPMI_CHECK_ST_H_ */
