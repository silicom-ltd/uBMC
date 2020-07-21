/*
 * ubmc_ipmi_pm_xs.c
 *
 *  Created on: Mar 19, 2019
 *      Author: jason_zhang
 */

#include "ubmc_ipmi_config.h"

struct ubmc_ipmi_sensor_thresh skyd_temp_host_cpu_thresh =
{
	.lnr = TEMP_HOST_CPU_LNR,
	.lcr = TEMP_HOST_CPU_LCR,
	.lnc = TEMP_HOST_CPU_LNC,
	.unc = TEMP_HOST_CPU_UNC,
	.ucr = TEMP_HOST_CPU_UCR,
	.unr = TEMP_HOST_CPU_UNR,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_temp_host_pcb_thresh =
{
	.lnr = TEMP_HOST_PCB_LNR,
	.lcr = TEMP_HOST_PCB_LCR,
	.lnc = TEMP_HOST_PCB_LNC,
	.unc = TEMP_HOST_PCB_UNC,
	.ucr = TEMP_HOST_PCB_UCR,
	.unr = TEMP_HOST_PCB_UNR,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_temp_inlet_amb_thresh =
{
	.lnr = TEMP_INLET_AMB_LNR,
	.lcr = TEMP_INLET_AMB_LCR,
	.lnc = TEMP_INLET_AMB_LNC,
	.unc = TEMP_INLET_AMB_UNC,
	.ucr = TEMP_INLET_AMB_UCR,
	.unr = TEMP_INLET_AMB_UNR,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_fan1_thresh =
{
	.lnr = 2400,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 16000,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_volt_vccin_thresh =
{
	.lnr = 15000,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 20000,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_volt_vnn_thresh =
{
	.lnr = 8500,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 10000,
	.status = UBMC_IPMI_NORMAL,
};
struct ubmc_ipmi_sensor_thresh skyd_volt_vccio_thresh =
{
	.lnr = 8000,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 11000,
	.status = UBMC_IPMI_NORMAL,
};
struct ubmc_ipmi_sensor_thresh skyd_volt_vccsa_thresh =
{
	.lnr = 5000,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 11000,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_volt_1_0_5_thresh =
{
	.lnr = 10100,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 10900,
	.status = UBMC_IPMI_NORMAL,
};
struct ubmc_ipmi_sensor_thresh skyd_mem_vddq_thresh =
{
	.lnr = MEM_VDDQ_LNR,
	.lcr = MEM_VDDQ_LCR,
	.lnc = MEM_VDDQ_LNC,
	.unc = MEM_VDDQ_UNC,
	.ucr = MEM_VDDQ_UCR,
	.unr = MEM_VDDQ_UNR,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_volt_1_8_thresh =
{
	.lnr = 17100,
	.lcr = VOLT_1_8_LCR,
	.lnc = VOLT_1_8_LNC,
	.unc = VOLT_1_8_UNC,
	.ucr = VOLT_1_8_UCR,
	.unr = 18900,
	.status = UBMC_IPMI_NORMAL,
};

struct ubmc_ipmi_sensor_thresh skyd_volt_12v_thresh =
{
	.lnr = 10800,
	.lcr = UBMC_IPMI_N_A,
	.lnc = UBMC_IPMI_N_A,
	.unc = UBMC_IPMI_N_A,
	.ucr = UBMC_IPMI_N_A,
	.unr = 12700,
	.status = UBMC_IPMI_NORMAL,
};

struct gpio_s skyd_ps1_used_state = {.gpio_num = 471,.tri = TRI_NULL};
struct gpio_s skyd_ps2_used_state = {.gpio_num = 474,.tri = TRI_NULL};
int ubmc_ipmi_skyd_init(void *data,uint32_t flag)
{
	int fd;
	fd = ubmc_ipmi_open_gpio_value_file(&skyd_ps1_used_state);
	if(fd < 0)
	{
		ubmc_error("open gpio value file fail :%d \n",fd);
		return -1;
	}
	fd = ubmc_ipmi_open_gpio_value_file(&skyd_ps2_used_state);
	if(fd < 0)
	{
		ubmc_error("open gpio value file fail :%d \n",fd);
		return -1;
	}
	return 0;
}

static int skyd_sync_sensor_state_to_kernel(void* sensor_data ,void* ipmi_p,uint32_t value,void *output)
{
	uint8_t val = 0;          // the ipmitool just need one byte to representative the last value
							  // so we need to calculte before sync it
	int m = 0, b = 0, k1 = 0, k2 = 0;
	double result;
	ubmc_sensor_event *ubmc_sensor_arry;
	ubmc_sensor_arry = sensor_data;
	struct ubmc_ipmi_s *ubmc_ipmi;
	ubmc_ipmi = ipmi_p;
	ipmi_sensor_type type = ubmc_sensor_arry->type;
	uint8_t offset = ubmc_sensor_arry->sensor_offset;
	ubmc_sensor_s *pbase = (ubmc_sensor_s *)ubmc_ipmi->ubmc_sensor_base;
	uint32_t value_tpm;
	if( offset < 0 || offset >= ubmc_sensor_arry->sensor_max_num /*|| (int)value < 0 */){
		return BMC_FALSE;
	}
	pbase = pbase + offset;
	if( update_calculation_factor_value(pbase,(uint16_t)ubmc_sensor_arry->ipmitool_factor->m,
						__REV_TO_BACC(ubmc_sensor_arry->ipmitool_factor->b,
						 ubmc_sensor_arry->ipmitool_factor->k1,
						 ubmc_sensor_arry->ipmitool_factor->k2)) == BMC_FALSE )
	{
		ubmc_error("update_calculation_factor_value Error !\n");
		return BMC_FALSE;
	}
	// sync the data

	value_tpm = value/ubmc_sensor_arry->ipmitool_factor->minification;
	if(value_tpm > 0x7f)
	{
		value_tpm = 0x7f;
	}
	pbase->value = value_tpm;
/*	printf("b is %d , m is %d , k1 is %d ,k2 is %d ,minification is %d val is %d \n",
			ubmc_sensor_arry->ipmitool_factor->b,
			ubmc_sensor_arry->ipmitool_factor->m,ubmc_sensor_arry->ipmitool_factor->k1,
			ubmc_sensor_arry->ipmitool_factor->k2,ubmc_sensor_arry->ipmitool_factor->minification,pbase->value);
*/
	return BMC_SUCCESS;
}
#if 0
static int skyd_sync_sensor_state_to_gui(void* sensor_data ,void* ipmi_p,uint32_t value,void *output)
{
	//before set the value ,should check the value
	ubmc_sensor_event *ubmc_sensor_arry;
	unsigned int interval_hour;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	ipmi_sensor_type type = ubmc_sensor_arry->type;
	uint8_t index = ubmc_sensor_arry->index_of_gui_array;
	uint8_t sensornum = ubmc_sensor_arry->sensor_offset;
	uint32_t is_fault = 0;
	uint32_t is_warning = 0;
	if(sensor_data == NULL)
	{
		ubmc_error(" sync_sensor_state_to_gui: (ubmc_sensor_arry == NULL)\n");
		return BMC_FALSE;
	}
	if(!ubmc_ipmi_check_sensor_value_is_valid(value))
	{
		value = -1;
	}
	switch( type )
	{
		case UBMC_SENSOR_TEMP:
			UBMC_SENSOR_SET_GUI_INT(temp_state,temp,index,value);
			// set the peak value when the value is smaller the value
			//the value maybe is a negative number
			if( UBMC_SENSOR_GET_GUI_INT(temp_state,peak,index) < value ){
				UBMC_SENSOR_SET_GUI_INT(temp_state,peak,index,value);
			}
			if( UBMC_SENSOR_GET_GUI_STR(temp_state,name,index)[0] == '\0' ){
				UBMC_SENSOR_SET_GUI_STR(temp_state,name,index,sensor_events[sensornum]->sensor_name);
			}
			break;
		case UBMC_SENSOR_VOL:
			//printf("sensor num is %d ,index is %d \n",sensornum,index);
			UBMC_SENSOR_SET_GUI_INT(vol_state,voltage,index,value);
			if( UBMC_SENSOR_GET_GUI_STR(vol_state,name,index)[0] == '\0' ){
				UBMC_SENSOR_SET_GUI_STR(vol_state,name,index,sensor_events[sensornum]->sensor_name);
			}
			break;
		case UBMC_SENSOR_FAN:

			UBMC_SENSOR_SET_GUI_INT(fan_state,speed,index,value);
			if( !UBMC_SENSOR_GET_GUI_INT(fan_state,status,index) ){
				UBMC_SENSOR_SET_GUI_INT(fan_state,status,index,1);
			}
			//to set runtime of fan
			//disable this function
			//interval_hour = ubmc_ipmi_get_rt_interval(&g_runtime)/3600;
			//UBMC_SENSOR_SET_GUI_INT(fan_state,run_time,index,interval_hour);

			if( UBMC_SENSOR_GET_GUI_STR(fan_state,name,index)[0] == '\0' ){
				UBMC_SENSOR_SET_GUI_STR(fan_state,name,index,sensor_events[sensornum]->sensor_name);
			}
			//set fan fault status
			if(ubmc_sensor_arry->sensor_fault_path[0] != '\0')
			{
				if( ubmc_get_sys_file_content(ubmc_sensor_arry->sensor_fault_path,&is_fault) == BMC_FALSE )
				{
					ubmc_debug(" ubmc_get_sys_file_content  get sensor fault state Error ! \n");
				}
				UBMC_SENSOR_SET_GUI_INT(fan_state,fault,index,is_fault);
				//set fan warning status
				if(ubmc_sensor_arry->thresh.status > NO_EVENT)
				{
					is_warning = 1;
				}
				UBMC_SENSOR_SET_GUI_INT(fan_state,warning,index,is_warning);
			}
			break;
		default:
			ubmc_error("Error type : %d \n",type);
			return BMC_FALSE;
			break;
	}
	return BMC_SUCCESS;
}
#endif


static int skyd_sync_sensor_state_to_gui(void* sensor_data ,void* ipmi_p,uint32_t value,void *output)
{
	//before set the value ,should check the value
	ubmc_sensor_event *ubmc_sensor_arry;
	unsigned int interval_hour;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	ipmi_sensor_type type = ubmc_sensor_arry->type;
	uint8_t sub_type = ubmc_sensor_arry->sub_type;
	uint8_t index = ubmc_sensor_arry->index_of_gui_array;
	uint8_t sensornum = ubmc_sensor_arry->sensor_offset;
	uint32_t is_fault = 0;
	uint32_t is_warning = 0;
	int ret;
	int power_supply_present = 0;
	uint8_t status = 0;
	if(sensor_data == NULL)
	{
		ubmc_error(" sync_sensor_state_to_gui: (ubmc_sensor_arry == NULL)\n");
		return BMC_FALSE;
	}
	if(!ubmc_ipmi_check_sensor_value_is_valid(value))
	{
		value = -1;
	}
	switch( type )
	{
		case UBMC_SENSOR_TEMP:
			UBMC_SENSOR_SET_GUI_INT(temp_state,temp,index,value);
			// set the peak value when the value is smaller the value
			//the value maybe is a negative number
			if( UBMC_SENSOR_GET_GUI_INT(temp_state,peak,index) < value ){
				UBMC_SENSOR_SET_GUI_INT(temp_state,peak,index,value);
			}
			if( UBMC_SENSOR_GET_GUI_STR(temp_state,name,index)[0] == '\0' ){
				UBMC_SENSOR_SET_GUI_STR(temp_state,name,index,sensor_events[sensornum]->sensor_name);
			}
			break;
		case UBMC_SENSOR_VOL:
			//printf("sensor num is %d ,index is %d \n",sensornum,index);
			if(sub_type == 0)	//for normal type
			{
				UBMC_SENSOR_SET_GUI_INT(vol_state,voltage,index,value);
				if( UBMC_SENSOR_GET_GUI_STR(vol_state,name,index)[0] == '\0' )
				{
					UBMC_SENSOR_SET_GUI_STR(vol_state,name,index,sensor_events[sensornum]->sensor_name);
				}
			}
			else if(sub_type == 1)	//for power supply
			{

				UBMC_SENSOR_SET_GUI_STR(power_supply_state,name,index,sensor_events[sensornum]->sensor_name);
				UBMC_SENSOR_SET_GUI_INT(power_supply_state,voltage_out,index,value);
				if(index == 0)
				{
					power_supply_present = ubmc_ipmi_read_gpio_value(&skyd_ps1_used_state);
					UBMC_SENSOR_SET_GUI_INT(power_supply_state,status,index,power_supply_present);
				}
				else if(index == 1)
				{
					power_supply_present = ubmc_ipmi_read_gpio_value(&skyd_ps2_used_state);
					UBMC_SENSOR_SET_GUI_INT(power_supply_state,status,index,power_supply_present);
				}
				else if(index == 2)
				{
					if(value > 0)
					{
						UBMC_SENSOR_SET_GUI_INT(power_supply_state,status,index,1);
					}
					else
					{
						UBMC_SENSOR_SET_GUI_INT(power_supply_state,status,index,0);
					}

				}
				if( UBMC_SENSOR_GET_GUI_STR(power_supply_state,name,index)[0] == '\0')
				{
					UBMC_SENSOR_SET_GUI_STR(power_supply_state,name,index,sensor_events[sensornum]->sensor_name);
				}
			//	ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&status,0);
				if(status > 0)
				{
					if((status & 0x95) > 0)	//refer to PMbus spec
					{
						UBMC_SENSOR_SET_GUI_INT(power_supply_state,fault,index,1);
					}
					else
					{
						UBMC_SENSOR_SET_GUI_INT(power_supply_state,warning,index,1);
					}
				}
				else
				{
					UBMC_SENSOR_SET_GUI_INT(power_supply_state,fault,index,0);
					UBMC_SENSOR_SET_GUI_INT(power_supply_state,warning,index,0);

				}

			}
			else
			{
				;
			}

			break;
		case UBMC_SENSOR_FAN:

			UBMC_SENSOR_SET_GUI_INT(fan_state,speed,index,value);
			if( !UBMC_SENSOR_GET_GUI_INT(fan_state,status,index) ){
				UBMC_SENSOR_SET_GUI_INT(fan_state,status,index,1);
			}
			//to set runtime of fan
			//disable this function
			//interval_hour = ubmc_ipmi_get_rt_interval(&g_runtime)/3600;
			//UBMC_SENSOR_SET_GUI_INT(fan_state,run_time,index,interval_hour);

			if( UBMC_SENSOR_GET_GUI_STR(fan_state,name,index)[0] == '\0' ){
				UBMC_SENSOR_SET_GUI_STR(fan_state,name,index,sensor_events[sensornum]->sensor_name);
			}
			//set fan fault status
			if(ubmc_sensor_arry->read_value_mode == FILE_MODE)
			{
				if(ubmc_sensor_arry->sensor_fault_path[0] != '\0')
				{
					if( ubmc_get_sys_file_content(ubmc_sensor_arry->sensor_fault_path,&is_fault) == BMC_FALSE )
					{
						ubmc_debug(" ubmc_get_sys_file_content  get sensor fault state Error ! \n");
					}
					UBMC_SENSOR_SET_GUI_INT(fan_state,fault,index,is_fault);
					//set fan warning status
					if(ubmc_sensor_arry->thresh.status > NO_EVENT)
					{
						is_warning = 1;
					}
					UBMC_SENSOR_SET_GUI_INT(fan_state,warning,index,is_warning);
				}
			}
			else if(ubmc_sensor_arry->read_value_mode == DEV_MODE)
			{
				;
			}

			break;
		default:
			ubmc_error("Error type : %d \n",type);
			return BMC_FALSE;
			break;
	}
	return BMC_SUCCESS;
}
static int skyd_event_handler(void* sensor_data ,void* ipmi_p,uint32_t evt_flag,void *output)
{
	ubmc_sensor_event *ubmc_sensor_arry;
	int ret;
	struct sel_event_record  sensor_event;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	struct ubmc_ipmi_s *ubmc_ipmi;
	ubmc_ipmi = ipmi_p;
	if(sensor_data == NULL || ipmi_p == NULL)
	{
		ubmc_error(" event_handler: (ubmc_sensor_arry == NULL || ipmi_p == NULL)\n");
		return BMC_FALSE;
	}
	//seting event info by some parameters
	ret = ubmc_ipmi_set_sensor_evt(evt_flag,ubmc_sensor_arry->type,ubmc_sensor_arry->sensor_id,&sensor_event);
	if(ret < 0)
	{
		ubmc_error(" ubmc_ipmi_set_sensor_evt Error ! \n");
		return BMC_FALSE;
	}
	//record the event to sel
	ret = ubmc_ipmi_event_handle(&ubmc_ipmi->ubmc_ipmi_sel,&sensor_event);
	if(ret < 0)
	{
		ubmc_error(" ubmc_ipmi_event_handle Error ! \n");
		return BMC_FALSE;
	}
	return BMC_SUCCESS;
}

static int skyd_update_sensor_state_callback(void* sensor_data ,void* private_date,uint32_t flag,void *output)
{
	uint32_t value = 0;
	int ret = 0;
	unsigned int evt_flag = 0;
	//const char *path ,ipmi_sensor_type type ,uint8_t sensornum,uint8_t index_of_gui_array,uint8_t sensor_id,ubmc_ipmi_sensor_thresh_s *thresh
	struct sel_event_record  sensor_event;
	struct ubmc_ipmi_s *ubmc_ipmi;
	ubmc_ipmi = private_date;
	ubmc_sensor_event *ubmc_sensor_arry;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	//check parameters
	if( ubmc_sensor_arry == NULL )
	{
		return BMC_FALSE;
	}
	if(ubmc_sensor_arry->sync_sensor_gui_shm == NULL || ubmc_sensor_arry->sync_sensor_kernel_shm == NULL || ubmc_sensor_arry->sensor_event_handler == NULL )
	{
		ubmc_error(" Did not register valid callback for sensor !\n");
		return BMC_FALSE;
	}
	//check whether file exist
	if( !ubmc_is_file_exist(ubmc_sensor_arry->sensor_value_path) )
	{
		//just log this do not return;
		//if(g_debug_info_cnt ++ <= 10) ubmc_debug(" Path : %s is not exist ! \n",ubmc_sensor_arry->sensor_value_path);
		//return BMC_FALSE;
	}
	//get the value from sys, the way to get value depend on the device form.you can add  read_value_mode if needed
	if(ubmc_sensor_arry->read_value_mode == FILE_MODE)
	{
		if( ubmc_get_sys_file_content(ubmc_sensor_arry->sensor_value_path,&value) == BMC_FALSE )
		{
			//if(g_debug_info_cnt ++ <= 10) ubmc_debug(" ubmc_get_sys_file_content Error ! \n");
			//return BMC_FALSE;
			//do not return ,just set value to -1,means can not get the value ,N/A
			value = -1;
		}
		else
		{
			if(strcmp(ubmc_sensor_arry->sensor_name,"TEMP_HOST_CPU") == 0)
			{
				value = value + 15000 ; //The CPU temp is measured by PCB temp on SKY-D system.
			}
		}

	}
	else if(ubmc_sensor_arry->read_value_mode == DEV_MODE)
	{
		if( ubmc_sensor_arry->sensor_read_dev_value_handler(ubmc_sensor_arry ,NULL,0,&value) < 0)
		{
			//if(g_debug_info_cnt ++ <= 10) ubmc_debug(" ubmc_get_sys_dev_content Error ! \n");
			//do not return ,just set value to -1,means can not get the value ,N/A
			value = -1;
		}
	}
	//do value modify ,Some data read from sys is not a normal form, need to modify it.
	if(ubmc_sensor_arry->sensor_value_multiple != 0)
	{
		//printf("value is %d sensor_value_multiple is %d \n",value,ubmc_sensor_arry->sensor_value_multiple);
		value = value*ubmc_sensor_arry->sensor_value_multiple;	//some sensor has diffirent index value
	}
	//update sensor state to kernel driver.
	if( ubmc_sensor_arry->sync_sensor_gui_shm(ubmc_sensor_arry ,ubmc_ipmi,value,NULL) )
	{
		ubmc_error(" sync_sensor_state_to_gui Error ! \n");
		return BMC_FALSE;
	}
	//update sensor state to gui space.
	if( ubmc_sensor_arry->sync_sensor_kernel_shm(ubmc_sensor_arry ,ubmc_ipmi,value,NULL) )
	{
		ubmc_error(" sync_sensor_state_to_kernel Error ! \n");
		return BMC_FALSE;
	}
	// check whether sensor event happen
	evt_flag = check_sensor_event(value,&ubmc_sensor_arry->thresh);
	//printf("value is %ld evt_flag is %d\n",value,evt_flag);
	if(evt_flag < 0)
	{
		ubmc_error(" sensor thresh seting is not valid ! \n");
	}
	//do event handler if there is a event
	if(evt_flag > 0)
	{
		ret = ubmc_sensor_arry->sensor_event_handler(ubmc_sensor_arry ,ubmc_ipmi,evt_flag,NULL);
		if(ret < 0)
		{
			ubmc_error(" ubmc_ipmi_event_happan Error ! \n");
			return BMC_FALSE;
		}
	}
	//Do something else:

	//printf("value is %ld path %s \n",value,ubmc_sensor_arry->sensor_value_path);
	return BMC_SUCCESS;
}
static int skyd_sensor_read_dev_value(void* sensor_data ,void* private_date,uint32_t flag,void *output)
{
	ubmc_sensor_event *ubmc_sensor_arry;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	unsigned int value = 0;
	unsigned short val = 0;
	unsigned int result = 0;
	int ret = 0;
	float fvalue;
	if(ubmc_sensor_arry->dev_adr == CPU_TEMP_DEVICE_ADDR)
	{
		switch (ubmc_sensor_arry->sensor_value_reg)
		{
			case CPU_TEMP_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				val = htons(val);
				//value = val & 0x0ff;
				//printf("temp1 is 0x%x \n",ret);
				value = value*1000;
			break;
			default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
	}
	else if(ubmc_sensor_arry->dev_adr == PS1_DEVICE_ADR ||  ubmc_sensor_arry->dev_adr == PS2_DEVICE_ADR)
	{
		switch (ubmc_sensor_arry->sensor_value_reg)
		{
			case PS_DEVICE_FAN_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				if((signed short)val < 0)
				{
					value = 0;
				}
				else
				{
					value = val;
				}
				//printf("get the fan speed %d ret is %d \n",value,ret);
			break;
			case PS_DEVICE_VO_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				if(ret < 0)
				{
					break;
				}
				//printf("get the val %d ret is %d \n",val,ret);
				//ubmc_pmbus_get_linear_value(val,&fvalue);
				//To see the PSU datesheet
				value = 2*9*val/10; //converted to mv

			break;
			case PS_DEVICE_TEMP_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				//printf("get the temp %d ret is %d \n",val,ret);
				value = val;
			break;
			default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
	}
	else if(ubmc_sensor_arry->dev_adr == PS_12_V_DEVICE_ADR)
	{
		switch (ubmc_sensor_arry->sensor_value_reg)
		{
			case PS_DEVICE_FAN_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);

			break;
			case PS_DEVICE_VO_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				if(ret < 0)
				{
					break;
				}
				//ubmc_pmbus_get_linear_value(value,&fvalue);
				//printf("vo is %f %d \n",fvalue,value);
				fvalue = val/4096.0;
				value = fvalue*1000; //converted to mv
			break;
			case PS_DEVICE_TEMP_VAL_REG:
				ret = ubmc_smbus_read_value(ubmc_sensor_arry->sensor_dev_name,ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg,&val,1);
				value = val /2;
			break;
			default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
	}
	else
	{
		ubmc_debug("Unknow device 0x%x:0x%x \n",ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg);
		return -1;
	}
	if(ret < 0)
	{
		//ubmc_debug("Read value from 0x%x:0x%x fail \n",ubmc_sensor_arry->dev_adr,ubmc_sensor_arry->sensor_value_reg);
		return -1;
	}
	*(unsigned int*)output = value;
	return BMC_SUCCESS;
}

static int skyd_sensor_read_file_value(void* sensor_data ,void* private_date,uint32_t flag,void *value)
{
	ubmc_sensor_event *ubmc_sensor_arry;
	ubmc_sensor_arry = (ubmc_sensor_event *)sensor_data;
	int ret;
	int temp;
	ret = ubmc_get_sys_file_content(ubmc_sensor_arry->sensor_value_path,&temp);
	if(ret < 0)
	{
		//if(g_debug_info_cnt ++ <= 10) ubmc_debug(" ubmc_get_sys_file_content Error ! file path:%s \n",ubmc_sensor_arry->sensor_value_path);
		return -1;
	}
	*(int *)value = temp;
	return BMC_SUCCESS;
}
struct ubmc_sensor_config_s ubmc_skyd_sensor_cfg[SKYD_SENSOR_MAX_NUM] =
{
		{
				.dev_name = SKYD_FAN_DEV_NAME,		// for testting,need chang when we can read cpu temp from SML
				.sensor_type = UBMC_SENSOR_TEMP,
				.sensor_sys_path_suffix = "",
				.sensor_name = "TEMP_HOST_CPU",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 1,
				.index_of_gui_array = 0,				//the id of the same type
				.sensor_id = 1,
				.sensor_thresh = &skyd_temp_host_cpu_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 1,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 1000


		},
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_TEMP,
				.sensor_sys_path_suffix = "",
				.sensor_name = "TEMP_HOST_PCB",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 1,
				.index_of_gui_array = 1,
				.sensor_id = 2,
				.sensor_thresh = &skyd_temp_host_pcb_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 1,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 1000

		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_TEMP,
				.sensor_name = "PS1_TEMP",
				.sensor_filename_flag = 0,
				.sensor_sys_path_suffix = "device",
				//.sensor_channel_id = 3,
				.index_of_gui_array = 2,
				.sensor_id = 3,
				.sensor_thresh = &skyd_temp_host_cpu_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS1_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_TEMP_VAL_REG,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 1,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 1000
		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_TEMP,
				.sensor_name = "PS2_TEMP",
				.sensor_filename_flag = 0,
				.sensor_sys_path_suffix = "device",
				//.sensor_channel_id = 4,
				.index_of_gui_array = 3,
				.sensor_id = 4,
				.sensor_thresh = &skyd_temp_host_cpu_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS2_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_TEMP_VAL_REG,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 1,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 1000
		},
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_sys_path_suffix = "",
				.sensor_name = "FAN1_TACH",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 1,
				.index_of_gui_array = 0,
				.sensor_id = 1,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
#if 1
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_sys_path_suffix = "",
				.sensor_name = "FAN2_TACH",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 2,
				.index_of_gui_array = 1,
				.sensor_id = 2,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_sys_path_suffix = "",
				.sensor_name = "FAN3_TACH",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 3,
				.index_of_gui_array = 2,
				.sensor_id = 3,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_sys_path_suffix = "",
				.sensor_name = "FAN4_TACH",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 4,
				.index_of_gui_array = 3,
				.sensor_id = 4,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
		{
				.dev_name = SKYD_FAN_DEV_NAME,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_sys_path_suffix = "",
				.sensor_name = "FAN5_TACH",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 5,
				.index_of_gui_array = 4,
				.sensor_id = 5,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_name = "PS1_FAN_TACH",
				.sensor_filename_flag = 0,
				.sensor_sys_path_suffix = "device",
				.sensor_channel_id = 6,
				.index_of_gui_array = 5,
				.sensor_id = 6,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS1_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_FAN_VAL_REG,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_FAN,
				.sensor_name = "PS2_FAN_TACH",
				.sensor_filename_flag = 0,
				.sensor_sys_path_suffix = "device",
				.sensor_channel_id = 7,
				.index_of_gui_array = 6,
				.sensor_id = 7,
				.sensor_thresh = &skyd_fan1_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS2_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_FAN_VAL_REG,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 200,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = 0,
				.ipmitool_factor.minification = 200
		},
#endif
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD 1.8V",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 0,
				.index_of_gui_array = 0,
				.sensor_id = 1,
				.sensor_thresh = &skyd_volt_1_8_thresh,	/*1.71 1.89*/
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU MEMVDDQ 01",			/*1.16  1.26*/
				.sensor_filename_flag = 0,
				.sensor_channel_id = 1,
				.index_of_gui_array = 1,
				.sensor_id = 2,
				.sensor_thresh = &skyd_mem_vddq_thresh,
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU MEMVDDQ 34",			/*1.16  1.26*/
				.sensor_filename_flag = 0,
				.sensor_channel_id = 2,
				.index_of_gui_array = 2,
				.sensor_id = 3,
				.sensor_thresh = &skyd_mem_vddq_thresh,
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = VOL_DEV_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD VCCSA",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 3,
				.index_of_gui_array = 3,
				.sensor_id = 4,
				.sensor_thresh = &skyd_volt_vccsa_thresh,
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = VOL_DEV_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD VCCIN",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 4,
				.index_of_gui_array = 4,
				.sensor_id = 5,
				.sensor_thresh = &skyd_volt_vccin_thresh, /*1.5 2.0*/
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD VNN",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 5,
				.index_of_gui_array = 5,
				.sensor_id = 6,
				.sensor_thresh = &skyd_volt_vnn_thresh,	/*0.85 1.0*/
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD VCCIO",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 6,
				.index_of_gui_array = 6,
				.sensor_id = 7,
				.sensor_thresh = &skyd_volt_vccio_thresh,	/*0.8 1.1*/
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_ADC_NAME,
				.sensor_type = UBMC_SENSOR_VOL,
				.sensor_sys_path_suffix = "",
				.sensor_name = "CPU_BRD 1.05V",
				.sensor_filename_flag = 0,
				.sensor_channel_id = 7,
				.index_of_gui_array = 7,
				.sensor_id = 8,
				.sensor_thresh = &skyd_volt_1_0_5_thresh,
				.sensor_value_multiple = 10,
				.sensor_read_value_mode = FILE_MODE,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 15,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 150
		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_VOL,
				.sub_type = 1,
				.sensor_filename_flag = 0,
				.sensor_name = "PS1_V12",
				.sensor_sys_path_suffix = "",
				//.sensor_channel_id = 15,	//not used
				.index_of_gui_array = 0,
				.sensor_id = 1,
				.sensor_thresh = &skyd_volt_12v_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS1_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_VO_VAL_REG,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 100,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 100
		},
		{
				.dev_name = SKYD_POWER_SUPPLY_DEV,
				.sensor_type = UBMC_SENSOR_VOL,
				.sub_type = 1,
				.sensor_filename_flag = 0,
				.sensor_name = "PS2_V12",
				.sensor_sys_path_suffix = "",
				//.sensor_channel_id = 14,		//not used
				.index_of_gui_array = 1,
				.sensor_id = 2,
				.sensor_thresh = &skyd_volt_12v_thresh,
				.sensor_value_multiple = 1,
				.sensor_read_value_mode = DEV_MODE,
				.dev_adr = PS2_DEVICE_ADR,
				.sensor_read_value_reg = PS_DEVICE_VO_VAL_REG,
				.update_sensor_state = skyd_update_sensor_state_callback,
				.sensor_read_dev_value_handler = skyd_sensor_read_dev_value,
				.sensor_read_file_value_handler = skyd_sensor_read_file_value,
				.sync_sensor_kernel_shm = skyd_sync_sensor_state_to_kernel,
				.sync_sensor_gui_shm = skyd_sync_sensor_state_to_gui,
				.sensor_event_handler = skyd_event_handler,
				.ipmitool_factor.m	= 100,
				.ipmitool_factor.b	= 0,
				.ipmitool_factor.k1 = 0,
				.ipmitool_factor.k2 = -3,
				.ipmitool_factor.minification = 100
		},

};
