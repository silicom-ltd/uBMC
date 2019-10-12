#ifndef __USER_IPMID_H_
#define __USER_IPMID_H_



#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi_lan.h"
#include "ubmc_ipmi_sel_sensor.h"
#include "ubmc_ipmi_check_st.h"
#include "ubmc_ipmi_board.h"
#define BMC_DEVICE_NAME                   "/dev/bmc_drv"
#define BMC_INFO_MAX_LEN                  64




#define ubmc_debug(fmt, ...) \
            syslog(LOG_NOTICE, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);
#define ubmc_error(fmt, ...) \
            syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__);



typedef int  (*update_sensor_state_private_handler)(void* sensor_date ,void* private_date,uint32_t flag,void *output);
typedef int  (*ubmc_ipmi_board_init_handler)(void* data ,uint32_t flag);

struct ubmc_sensor_config_s
{
	ipmi_sensor_type sensor_type;
	uint8_t	sub_type;
	char dev_name[64];
	char sensor_name[64];
	char sensor_sys_path_suffix[32];
	char sensor_fault_path[128];
	char sensor_channel_id;
	char sensor_value_multiple;
	char sensor_filename_flag;
	uint8_t index_of_gui_array;
	ubmc_ipmi_sensor_thresh_s *sensor_thresh;
	uint8_t sensor_read_value_mode;
	uint8_t sensor_read_value_reg;
	uint8_t dev_adr;				//device addr,like i2c slave addr
	uint8_t sensor_id;
	ubmc_ipmitool_factor ipmitool_factor;
	update_sensor_state_private_handler update_sensor_state;
	update_sensor_state_private_handler  sensor_read_dev_value_handler;		//for DEV Mode
	update_sensor_state_private_handler  sensor_read_file_value_handler;	//for File Mode
	update_sensor_state_private_handler sync_sensor_kernel_shm;
	update_sensor_state_private_handler sync_sensor_gui_shm;
	update_sensor_state_private_handler sensor_event_handler;
	//thresh
};


struct ubmc_board_info_s
{
	device_type_t device_type;
	char temp_sensor_max_num;
	char fan_sensor_max_num;
	char volt_sensor_max_num;
	char power_supply_sensor_max_num;
	//add new sensor type if need
	char sensor_max_num;
	struct ubmc_sensor_config_s* sensor_config;
	ubmc_ipmi_board_init_handler init_handler;

};
//ubmc_sensor_event* ubmc_add_sensor(char *dev_name,char channel_id,char *sensor_name,ipmi_sensor_type type,
//					uint8_t index,update_sensor_state_handler cb,uint8_t id,ubmc_ipmi_sensor_thresh_s *thresh,uint8_t read_value_mode)

//the general structure
struct ubmc_ipmi_s
{
	struct ubmc_board_info_s ubmc_board_info;
	struct ubmc_ipmi_sel ubmc_ipmi_sel;
	struct ubmc_ipmi_lan_s ubmc_ipmi_lan;
	struct ubmc_ipmi_status_s ubmc_ipmi_status;
	void *ubmc_sensor_base;

};




//ipmi sel
#define IOCTL_ADD_SEL_NODE_CMD 0X100
#define IOCTL_CLR_SEL_LIST_CMD 0X101
#define IOCTL_SET_SEL_ENTRY_MAXNUM_CMD 0X102

//ipmi lan
#define IOCTL_LAN_INIT_CMD	0X201
#define IOCTL_LAN_UPDATE_CMD	0X202

//check status command
#define IOCTL_GET_STATUS_CMD	0X301
#define IOCTL_GET_TIME_STATUS_CMD	0X302
#define IOCTL_GET_SET_TIME_CMD	0X303

#endif
