/*
 * ubmc_ipmi_sel_sensor.h
 *
 *  Created on: Nov 8, 2018
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_SEL_SENSOR_H_
#define INCLUDE_UBMC_IPMI_SEL_SENSOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <limits.h>
#include <sys/mman.h>
#include <pthread.h>
#include <syslog.h>

#include "ubmc_shm_state.h"
#include "ubmc_ipmi_sdr.h"
#define BMC_DEVICE_NAME                   "/dev/bmc_drv"
#define BMC_INFO_MAX_LEN                  64


#ifndef BMC_SUCCESS
#define BMC_SUCCESS                       0
#endif

#ifndef BMC_FALSE
#define BMC_FALSE                         -1
#endif

#define PAGE_SIZE 4096
// the default unit is second
#define BMC_SENORS_DATA_GATHER_INTERVAL   3

// the maximum number of the sensor
#define IPMI_SDR_SERSOR_MAX(a)   (sizeof(a)/sizeof(*(a)))

//the event of thresh
#define NO_EVENT 0X00
#define UNR_EVENT 0x01
#define UCR_EVENT 0x02
#define UNC_EVENT 0x03	//	N/A
#define LNC_EVENT 0x04	//	N/A
#define LCR_EVENT 0x05
#define LNR_EVENT 0x06
/*
(Name)             (Reading)    (Units)     (Status)(LNR)       (LCR)       (UCR)      (UNR)
TEMP_HOST_CPU    | 41.000     | degrees C  | ok    | 0         | 4         | 87       | 91
TEMP_HOST_PCB    | 37.000     | degrees C  | ok    | 0         | 4         | 87       | 91
TEMP_INLET_AMB   | 28.000     | degrees C  | ok    | 0         | 4         | 40       | 44
FAN1_TACH        | 5800.000   | RPM        | ok    | 800       | 1000      | 9000     | 10000
FAN2_TACH        | 5400.000   | RPM        | ok    | 800       | 1000      | 9000     | 10000
5V               | 4.960      | Volts      | ok    | 4.500     | na        | na       | 5.500
3.3V             | 3.160      | Volts      | ok    | 2.756     | na        | na       | 3.845
CPU_VCCSRAM      | 1.150      | Volts      | ok    | 0.750     | na        | na       | 1.200
CPU_VCCP         | 1.150      | Volts      | ok    | 0.520     | na        | na       | 1.240
1.05V            | 1.051      | Volts      | ok    | 1.024     | na        | na       | 1.076
MEM_VDDQ         | 1.200      | Volts      | ok    | 1.162     | na        | na       | 1.260
CPU_VNN          | 1.050      | Volts      | ok    | 0.650     | na        | na       | 1.240
1.8V             | 1.800      | Volts      | ok    | 0.504     | na        | na       | 1.93
{L,U}NR lower/upper non-recoverable
{L,U}CR  lower/upper critical
{L,U}NCR  lower/upper non-critical (currently n/a)
*/
#define UBMC_IPMI_N_A 0xffffffff
//TEMP_HOST_CPU limit
#define TEMP_HOST_CPU_LNR 0
#define TEMP_HOST_CPU_LCR 4000
#define TEMP_HOST_CPU_LNC UBMC_IPMI_N_A
#define TEMP_HOST_CPU_UNC UBMC_IPMI_N_A
#define TEMP_HOST_CPU_UCR 87000
#define TEMP_HOST_CPU_UNR 91000

//TEMP_HOST_PCB limit
#define TEMP_HOST_PCB_LNR 0
#define TEMP_HOST_PCB_LCR 4000
#define TEMP_HOST_PCB_LNC UBMC_IPMI_N_A
#define TEMP_HOST_PCB_UNC UBMC_IPMI_N_A
#define TEMP_HOST_PCB_UCR 87000
#define TEMP_HOST_PCB_UNR 91000

//TEMP_INLET_AMB limit
#define TEMP_INLET_AMB_LNR 0
#define TEMP_INLET_AMB_LCR 4000
#define TEMP_INLET_AMB_LNC UBMC_IPMI_N_A
#define TEMP_INLET_AMB_UNC UBMC_IPMI_N_A
#define TEMP_INLET_AMB_UCR 40000
#define TEMP_INLET_AMB_UNR 44000

//FAN1_TACH limit
#define FAN1_TACH_LNR 800
#define FAN1_TACH_LCR 1000
#define FAN1_TACH_LNC UBMC_IPMI_N_A
#define FAN1_TACH_UNC UBMC_IPMI_N_A
#define FAN1_TACH_UCR 9000
#define FAN1_TACH_UNR 10000

//FAN2_TACH limit
#define FAN2_TACH_LNR 800
#define FAN2_TACH_LCR 1000
#define FAN2_TACH_LNC UBMC_IPMI_N_A
#define FAN2_TACH_UNC UBMC_IPMI_N_A
#define FAN2_TACH_UCR 9000
#define FAN2_TACH_UNR 10000

//5V limit
#define VOLT_5_0_LNR 45000
#define VOLT_5_0_LCR UBMC_IPMI_N_A
#define VOLT_5_0_LNC UBMC_IPMI_N_A
#define VOLT_5_0_UNC UBMC_IPMI_N_A
#define VOLT_5_0_UCR UBMC_IPMI_N_A
#define VOLT_5_0_UNR 55000

//3.3V limit
#define VOLT_3_3_LNR 27560
#define VOLT_3_3_LCR UBMC_IPMI_N_A
#define VOLT_3_3_LNC UBMC_IPMI_N_A
#define VOLT_3_3_UNC UBMC_IPMI_N_A
#define VOLT_3_3_UCR UBMC_IPMI_N_A
#define VOLT_3_3_UNR 38450

//CPUVCCSRAM limit
#define VCCSRAM_LNR 7500
#define VCCSRAM_LCR UBMC_IPMI_N_A
#define VCCSRAM_LNC UBMC_IPMI_N_A
#define VCCSRAM_UNC UBMC_IPMI_N_A
#define VCCSRAM_UCR UBMC_IPMI_N_A
#define VCCSRAM_UNR 12000

//CPUVCCP limit
#define VCCP_LNR 5200
#define VCCP_LCR UBMC_IPMI_N_A
#define VCCP_LNC UBMC_IPMI_N_A
#define VCCP_UNC UBMC_IPMI_N_A
#define VCCP_UCR UBMC_IPMI_N_A
#define VCCP_UNR 12400

//1.05V limit
#define VOLT_1_0_5_LNR 10240
#define VOLT_1_0_5_LCR UBMC_IPMI_N_A
#define VOLT_1_0_5_LNC UBMC_IPMI_N_A
#define VOLT_1_0_5_UNC UBMC_IPMI_N_A
#define VOLT_1_0_5_UCR UBMC_IPMI_N_A
#define VOLT_1_0_5_UNR 10760

//1.8V limit
#define VOLT_1_8_LNR 5040
#define VOLT_1_8_LCR UBMC_IPMI_N_A
#define VOLT_1_8_LNC UBMC_IPMI_N_A
#define VOLT_1_8_UNC UBMC_IPMI_N_A
#define VOLT_1_8_UCR UBMC_IPMI_N_A
#define VOLT_1_8_UNR 19300

//MEM_VDDQ limit
#define MEM_VDDQ_LNR 11620
#define MEM_VDDQ_LCR UBMC_IPMI_N_A
#define MEM_VDDQ_LNC UBMC_IPMI_N_A
#define MEM_VDDQ_UNC UBMC_IPMI_N_A
#define MEM_VDDQ_UCR UBMC_IPMI_N_A
#define MEM_VDDQ_UNR 12620

//CPU VNN limit
#define CPU_VNN_LNR 6500
#define CPU_VNN_LCR UBMC_IPMI_N_A
#define CPU_VNN_LNC UBMC_IPMI_N_A
#define CPU_VNN_UNC UBMC_IPMI_N_A
#define CPU_VNN_UCR UBMC_IPMI_N_A
#define CPU_VNN_UNR 12400


//CPU VNN limit
#define CPU_VNN_LNR 6500
#define CPU_VNN_LCR UBMC_IPMI_N_A
#define CPU_VNN_LNC UBMC_IPMI_N_A
#define CPU_VNN_UNC UBMC_IPMI_N_A
#define CPU_VNN_UCR UBMC_IPMI_N_A
#define CPU_VNN_UNR 12400

/*For large system*/
//Main 12V IO board
#define V12_LNR 108000
#define V12_LCR UBMC_IPMI_N_A
#define V12_LNC UBMC_IPMI_N_A
#define V12_UNC UBMC_IPMI_N_A
#define V12_UCR UBMC_IPMI_N_A
#define V12_UNR 130000

//PS 54V
#define PS_54V_LNR 517800
#define PS_54V_LCR UBMC_IPMI_N_A
#define PS_54V_LNC UBMC_IPMI_N_A
#define PS_54V_UNC UBMC_IPMI_N_A
#define PS_54V_UCR UBMC_IPMI_N_A
#define PS_54V_UNR 572300

//1.5v eth switch DDR_VDD_1V5
#define DDR_VDD_1V5_LNR 14520
#define DDR_VDD_1V5_LCR UBMC_IPMI_N_A
#define DDR_VDD_1V5_LNC UBMC_IPMI_N_A
#define DDR_VDD_1V5_UNC UBMC_IPMI_N_A
#define DDR_VDD_1V5_UCR UBMC_IPMI_N_A
#define DDR_VDD_1V5_UNR 15650

//main 5V IO board
#define V5_LNR 47000
#define V5_LCR UBMC_IPMI_N_A
#define V5_LNC UBMC_IPMI_N_A
#define V5_UNC UBMC_IPMI_N_A
#define V5_UCR UBMC_IPMI_N_A
#define V5_UNR 53800

//main 3.3V IO board
#define VDD_3V3_LNR 29500
#define VDD_3V3_LCR UBMC_IPMI_N_A
#define VDD_3V3_LNC UBMC_IPMI_N_A
#define VDD_3V3_UNC UBMC_IPMI_N_A
#define VDD_3V3_UCR UBMC_IPMI_N_A
#define VDD_3V3_UNR 36300

//1V
#define V1_LNR 9650
#define V1_LCR UBMC_IPMI_N_A
#define V1_LNC UBMC_IPMI_N_A
#define V1_UNC UBMC_IPMI_N_A
#define V1_UCR UBMC_IPMI_N_A
#define V1_UNR 10350

//1.8V VDD_1V8
#define VDD_1V8_LNR 17350
#define VDD_1V8_LCR UBMC_IPMI_N_A
#define VDD_1V8_LNC UBMC_IPMI_N_A
#define VDD_1V8_UNC UBMC_IPMI_N_A
#define VDD_1V8_UCR UBMC_IPMI_N_A
#define VDD_1V8_UNR 18650

//1.35V V1P35
#define V1P35_LNR 12850
#define V1P35_LCR UBMC_IPMI_N_A
#define V1P35_LNC UBMC_IPMI_N_A
#define V1P35_UNC UBMC_IPMI_N_A
#define V1P35_UCR UBMC_IPMI_N_A
#define V1P35_UNR 13950

//VTT DDR3 DDR_VTT
#define DDR_VTT_LNR 6300
#define DDR_VTT_LCR UBMC_IPMI_N_A
#define DDR_VTT_LNC UBMC_IPMI_N_A
#define DDR_VTT_UNC UBMC_IPMI_N_A
#define DDR_VTT_UCR UBMC_IPMI_N_A
#define DDR_VTT_UNR 7250

//1.1V UBMC V1P1
#define V1P1_LNR 10550
#define V1P1_LCR UBMC_IPMI_N_A
#define V1P1_LNC UBMC_IPMI_N_A
#define V1P1_UNC UBMC_IPMI_N_A
#define V1P1_UCR UBMC_IPMI_N_A
#define V1P1_UNR 11450

#define UBMC_SENSOR_EVNET_SEG(sensor_name,sensor_type,filepath,index,callback,sensorid,pthresh)  \
	{ \
		.name = sensor_name, \
		.type = sensor_type, \
		.path = filepath, \
		.index_of_gui_array = index, \
		.cb = callback, \
		.sensor_id = sensorid,\
		.thresh = pthresh, \
	}


#define IS_SPACE(c)  ((c) == ' ' || (c) == '\t' || (c) == '\n' )


// for int type value
#define UBMC_SENSOR_GET_GUI_INT(array,var,index)      ((ubmc_ipmi_state *)(ubmc_shm_base->map_addr))->array[index].var

#define UBMC_SENSOR_SET_GUI_INT(array,var,index,val)  ((ubmc_ipmi_state *)(ubmc_shm_base->map_addr))->array[index].var = val

// for string type value
#define UBMC_SENSOR_GET_GUI_STR(array,var,index)      ((ubmc_ipmi_state *)(ubmc_shm_base->map_addr))->array[index].var

#define UBMC_SENSOR_SET_GUI_STR(array,var,index,val)    \
	strncpy(((ubmc_ipmi_state *)(ubmc_shm_base->map_addr))->array[index].var,val,strlen(val))



// define the current three primary sensor type
typedef enum {
	UBMC_SENSOR_UNKNOWN ,   //  unknown
	UBMC_SENSOR_TEMP ,      //  temperature
	UBMC_SENSOR_VOL  ,      //  voltage
	UBMC_SENSOR_FAN  ,      //  fan
} ipmi_sensor_type;

typedef enum {
	UBMC_IPMI_NORMAL = 0 ,
	UBMC_IPMI_ON_LNC ,
	UBMC_IPMI_ON_LCR ,
	UBMC_IPMI_ON_LNR ,
	UBMC_IPMI_ON_UNC ,
	UBMC_IPMI_ON_UCR ,
	UBMC_IPMI_ON_UNR ,
} ubmc_ipmi_sensor_status;

typedef struct ubmc_ipmi_sensor_thresh
{
	int32_t	lnr;	//lower non-recoverable
	int32_t	lcr;	//lower critical
	int32_t	lnc;	//lower non-critical
	int32_t	unc;	//upper non-critical
	int32_t	ucr;	//upper critical
	int32_t	unr;	//upper non-recoverable
	ubmc_ipmi_sensor_status status;	//sensor current status
}ubmc_ipmi_sensor_thresh_s;

typedef struct runtime
{
	bool start;
	char *name;
	struct timeval start_time;
	struct timeval last_time;
	struct timeval curr_time;
	unsigned int interval_sec;
}runtime_s;

typedef struct sdr_get_rs sdr_get_rs;

typedef struct sdr_record_full_sensor sdr_record_full_sensor;

/**
 * This is the function pointer to update all sensor state
 * path ---> the file path to get the sensor data
 * type ---> the sensor type
 * sensornum  ----> the index of the kernel shared memory array
 * index_of_gui_array ---> the index of the userspace shared memeory array ( which must be display to the gui )
 * */
typedef int  (*update_sensor_state_handler)(void* sensor_date ,void* private_date,uint32_t flag,void *output);


#pragma pack(1)

// Merge the previous two structures : 'ubmc_sensor_state' and 'ipmi_sensor_s' into one ,
// in order to change the 'mtol' and 'bacc' in user space .
// ensure that the structure size is 128 bytes ,
// In order to fit into the kernel shared memory align by 128 bytes .


typedef struct _ubmc_sensor_s_ {
#define IPMI_SENSOR_NAME_MAX    16
	// this is use to fill sdr_record_full_sensor->id_string segment
	// so the length must be not larger than the sdr_record_full_sensor->id_string segment
	uint8_t                      name[IPMI_SENSOR_NAME_MAX];
	ipmi_sensor_type             type;
	uint8_t                      sensornum;   // it will be used in the userspace
	uint8_t                      value;       // the sensor value
#define UBMC_SENSOR_THRESHOLD_MAX  6
	// there will be 6 threshold value as the ipmitool source code
	uint8_t                      thresholds[UBMC_SENSOR_THRESHOLD_MAX];
	sdr_get_rs                   sdr_header;   // the header of the sdr
	sdr_record_full_sensor       sensor;       // the real content of the sdr
	// to ensure that the structure size is 128 bytes.
	uint8_t 					threshold_mask;
	uint8_t                      __reserved[36];
}ubmc_sensor_s;

typedef struct ubmc_ipmitool_factor_s
{
	int m ; //multiple
	int b ; //remainder
	int k1; //exponet
	int k2 ;//No decimal
	int minification;
}ubmc_ipmitool_factor;


typedef struct ubmc_sensor_event{
#define  IPMI_SENSOR_PATH_MAX		 128
#define IPMI_SENSOR_DEV_NAME_MAX 	 40
#define FILE_MODE		0	//read value from the sys file interface
#define DEV_MODE 		1	//read value from the device file interface
	uint8_t 					 read_value_mode;
	uint8_t                      sensor_value_reg;
	uint8_t 					 dev_adr;
	uint32_t					 fd;
	uint8_t                      sensor_dev_name[IPMI_SENSOR_DEV_NAME_MAX];
	uint8_t                      sensor_name[IPMI_SENSOR_DEV_NAME_MAX];
	uint8_t                      sensor_path[IPMI_SENSOR_PATH_MAX];
	uint8_t                      sensor_value_path[IPMI_SENSOR_PATH_MAX];
	uint8_t                      sensor_fault_path[IPMI_SENSOR_PATH_MAX];
	uint8_t						 channel_id;
	uint8_t						 sensor_value_multiple;
	ipmi_sensor_type             type;
	uint8_t					     sub_type;
	uint8_t                      index_of_gui_array;
	update_sensor_state_handler  update_sensor_state;
	update_sensor_state_handler  sensor_read_dev_value_handler;		//for DEV Mode
	update_sensor_state_handler  sensor_read_file_value_handler;	//for File Mode
	update_sensor_state_handler  sync_sensor_kernel_shm;
	update_sensor_state_handler  sync_sensor_gui_shm;
	update_sensor_state_handler  sensor_event_handler;
	uint8_t                      sensor_id;
	uint8_t                      sensor_max_num;
	uint8_t                      sensor_offset;
	ubmc_ipmi_sensor_thresh_s 	 thresh;
	ubmc_ipmitool_factor		 *ipmitool_factor;
}ubmc_sensor_event;




int ubmc_get_sys_file_content(const char *p_path,uint32_t *p_value);
ubmc_sensor_event* ubmc_ipmi_find_sesor_by_name(ubmc_sensor_event **ubmc_sensor_arry,char *sensor_name);
#pragma pack(0)



#endif /* INCLUDE_UBMC_IPMI_SEL_SENSOR_H_ */
