/*
 * ubmc_ipmi_hst_pwr_monitor.h
 *
 *  Created on: Oct 11, 2018
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_HST_PWR_MONITOR_H_
#define INCLUDE_UBMC_IPMI_HST_PWR_MONITOR_H_

#define PECI_CPU_TEMP_SYS_PATH "/sys/class/hwmon/hwmon2"
#define PECI_CPU_TEMP_VALUE_PATH "/sys/class/hwmon/hwmon2/temp1_input"

#define HOST_POWER_MONITOR_INTERVAL 20
#define HOST_POWER_NAME "HOST_PLTRST_N"
#define PECI_KMOD_RELOAD_CMD "/etc/init.d/S35is_ipmi reload_peci &"

int ipmi_host_power_monitor_thread( void );


#endif /* INCLUDE_UBMC_IPMI_HST_PWR_MONITOR_H_ */
