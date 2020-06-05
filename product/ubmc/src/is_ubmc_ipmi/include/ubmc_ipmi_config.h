/*
 * ubmc_config.h
 *
 *  Created on: Mar 19, 2019
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_CONFIG_H_
#define INCLUDE_UBMC_IPMI_CONFIG_H_

#include "ubmc_ipmi.h"
#include "ubmc_state.h"
#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi_sdr.h"
#include "ubmc_ipmi_hst_pwr_monitor.h"
#include "ubmc_ipmi_lan.h"
#include "ubmc_state.h"
#include <signal.h>
#include "ubmc_ipmi_sel_sensor.h"
#include <errno.h>
#include <stddef.h>
//#include <i2c/smbus.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <netinet/in.h>

static unsigned int g_debug_info_cnt = 0;



#define CPU_HOST_TEMP_DEV_NAME "peci_cputemp.cpu0"
#define CPU_HOST_TEMP_I2C_DEV "/dev/i2c-3"
#define POWER_SUPPLY_DEV "/dev/i2c-0"
#define VOL_DEV_NAME	"ads7830"
#define FAN_AND_TEMP_DEV_NAME	"silc_fan"//"emc2300"
#define FAN_AND_TEMP_DEV_L_S_M_NAME	"silc_fan"//"adt7475"
#define VOL_DEV_UCD9000_NAME "ucd90160"
#define VOL_DEV_PAC1014A_NAME "pac1014a"
/*For SKYD platform*/
#define SKYD_CPU_HOST_TEMP_DEV "test"
#define SKYD_FAN_DEV_NAME "max31785"
#define SKYD_ADC_NAME "ads7830"
#define SKYD_POWER_SUPPLY_DEV "/dev/i2c-1"
/*********************************************************config sensor number of XSMALL**************************************************************/
#define XSMALL_SERSOR_TEMP_NUM	3
#define XSMALL_SERSOR_VOL_NUM	8
#define XSMALL_SERSOR_FAN_NUM	1
#define XSMALL_SERSOR_PS_NUM	0
#define XSMALL_SENSOR_MAX_NUM (XSMALL_SERSOR_TEMP_NUM + XSMALL_SERSOR_VOL_NUM + XSMALL_SERSOR_FAN_NUM + XSMALL_SERSOR_PS_NUM)


/*********************************************************config sensor number of S/M**************************************************************/
#define S_M_SERSOR_TEMP_NUM	3
#define S_M_SERSOR_VOL_NUM	18
#define S_M_SERSOR_FAN_NUM	3
#define S_M_SERSOR_PS_NUM	0
#define S_M_SENSOR_MAX_NUM (S_M_SERSOR_TEMP_NUM + S_M_SERSOR_VOL_NUM + S_M_SERSOR_FAN_NUM + S_M_SERSOR_PS_NUM)


/*********************************************************config sensor number of LARGE**************************************************************/
#define L_SERSOR_TEMP_NUM	(3 + 3/*Power Supply Temp*/)
#define L_SERSOR_VOL_NUM	22
#define L_SERSOR_FAN_NUM	(3 + 2/*Power Supply Fan*/)
#define L_SERSOR_PS_NUM	3
#define L_SENSOR_MAX_NUM (L_SERSOR_TEMP_NUM + L_SERSOR_VOL_NUM + L_SERSOR_FAN_NUM + L_SERSOR_PS_NUM)


#define SENSOR_MAX_NUM (UBMC_LIMIT_SERSOR_TEMP + UBMC_LIMIT_SERSOR_VOL + UBMC_LIMIT_SERSOR_FAN + UBMC_LIMIT_SERSOR_PS)
/*********************************************************config sensor number of SKYD**************************************************************/
#define SKYD_SERSOR_TEMP_NUM	(2/*TEMP_HOST_PCB and TEMP_HOST_CPU*/ + 2 /*PSU*/)
#define SKYD_SERSOR_VOL_NUM	(8)
#define SKYD_SERSOR_FAN_NUM	(5 + 2)
#define SKYD_SERSOR_PS_NUM	2
#define SKYD_SENSOR_MAX_NUM (SKYD_SERSOR_TEMP_NUM + SKYD_SERSOR_VOL_NUM + SKYD_SERSOR_FAN_NUM + SKYD_SERSOR_PS_NUM)


#define SENSOR_MAX_NUM (UBMC_LIMIT_SERSOR_TEMP + UBMC_LIMIT_SERSOR_VOL + UBMC_LIMIT_SERSOR_FAN + UBMC_LIMIT_SERSOR_PS)
/******************************************************************************************************************************************************/
#define PS1_DEVICE_ADR 0x5a
#define PS2_DEVICE_ADR 0x59
#define PS_12_V_DEVICE_ADR 0x2a
#define PS_DEVICE_FAN_VAL_REG	0x90
#define PS_DEVICE_VO_VAL_REG	0x8b
#define PS_DEVICE_TEMP_VAL_REG	0x8d
#define CPU_TEMP_DEVICE_ADDR 0X48
#define CPU_TEMP_REG 0x40

/*If you want to add some sensor to ubmc_sensor_config_s :*/
/*1 ,need update kernel driver ubmc-ipmi.c ubmc_sensor_s correspond instance */
/*2 ,change the L_SENSOR_MAX_NUM  S_M_SENSOR_MAX_NUM  XSMALL_SENSOR_MAX_NUM to right value (need plus a number of sensor added)*/
/*3 ,change the UBMC_LIMIT_SERSOR_TEMP  UBMC_LIMIT_SERSOR_VOL  UBMC_LIMIT_SERSOR_FAN macro to right value*/
/*4 ,change the configure of board in init_board_configs(struct ubmc_ipmi_s *ubmc_ipmi)*/
/*5 ,need rebuild silc_mgmtd */
/*ubmc_sensor_config_s usage*/

/*
 * 				.dev_name = CPU_HOST_TEMP_DEV_NAME,
				.sensor_type = UBMC_SENSOR_TEMP,
				.sensor_sys_path_suffix = "",			//The suffix of sensor file path(the file path may be different in different sensor)
				.sensor_name = "TEMP_HOST_CPU",			//The name which show in gui
				.sensor_filename_flag = 0,				//indicate the sensor value file name format
				.sensor_channel_id = 1,					//indicate read value from which file ,like:temp1_input , the channel id is 1.Note:some sensor start with 1,
														//Note:some sensor start with 1,some sensor start with 0
				.index_of_gui_array = 0,				//the id of the gui display
				.sensor_id = 1,							//the id of the the same type sensor(FAN, VOL ,TEMP at present)
				.sensor_thresh = &temp_host_cpu_thresh,	//the sensor threshold (the value is not the same with kernel threshold)
				.sensor_value_multiple = 1,				//the sensor value multiple,some sensor maybe need.
				.sensor_read_value_mode = FILE_MODE,	//FILE_MODE:just read value buy /sys/class/hwmon... DEV_MODE:read the value by device
				.ipmitool_factor.m	= 1,				//The factor to support ipmitool.
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 1000
 *
 *
 */


extern struct ubmc_sensor_config_s ubmc_xsmall_sensor_cfg[XSMALL_SENSOR_MAX_NUM];
extern struct ubmc_sensor_config_s ubmc_s_m_sensor_cfg[S_M_SENSOR_MAX_NUM];
extern struct ubmc_sensor_config_s ubmc_large_sensor_cfg[L_SENSOR_MAX_NUM];
extern struct ubmc_sensor_config_s ubmc_skyd_sensor_cfg[SKYD_SENSOR_MAX_NUM];
extern struct ubmc_ipmi_s g_ubmc_ipmi;
extern ubmc_shm_state * ubmc_shm_base;
extern ubmc_sensor_event *sensor_events[SENSOR_MAX_NUM];
int ubmc_ipmi_large_init(void *data,uint32_t flag);
int ubmc_ipmi_small_init(void *data,uint32_t flag);
int ubmc_ipmi_medium_init(void *data,uint32_t flag);
int ubmc_ipmi_xsmall_init(void *data,uint32_t flag);
int ubmc_ipmi_skyd_init(void *data,uint32_t flag);
#endif /* INCLUDE_UBMC_IPMI_CONFIG_H_ */
