/*************************************************************************
    > File Name: gpio_main.c
    > Author: allen
    > Mail: allen.zhou@net-perf.com
    > Created Time: Wed 09 May 2018 04:59:32 PM HKT
 ************************************************************************/
#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <syslog.h>
#include "ubmc_sys_control_enum.h"
#include "ubmc_ctrl_status.h"
#include "silc_common.h"

#define host_sys_ctrl_lock_file   "/var/run/ubmc_sys_ctrl.lock"/*only a single instance application*/
#define host_sys_ctrl_status   "/var/run/ubmc_sys_ctrl_status"/*cache processing status*/

#define FAIL_RETRY_MAX_TIMES (5)

typedef enum {
    RESOURCE_GPIO_NO_OCCUPY =0,
    RESOURCE_GPIO_OCCUPY ,
}resource_gpio_status;

static ubmc_gpio_pin pin[GPIO_POS_MAX];
static unsigned int gpio_pin_no[GPIO_POS_MAX] = {0};

static const char *gpio_pin_name[GPIO_POS_MAX] = {
                        "SPI_HOST_MUX_EN",
                        "BMC_PWRBTN_OUT_N",
                        "HOST_FRU_EEPROM_MUX_SEL",
                        "HOST_PLTRST_N",
                        "HOST_S45_N",
                        "HOST_S3_N",
                        "RS232_CABLE_DETECTED",
                        "FP_RSTBTN_N",
                        "FP_PWRBTN_N",
                        "BMC_FP_BUTTON_OVERRIDE",
                        "BMC_RSTBTN_OUT_N"
};

static ubmc_gpio_direction gpio_pin_dir[GPIO_POS_MAX] = {
                        GPIO_DIRECTION_SPI_HOST_MUX_EN,
                        GPIO_DIRECTION_BMC_PWRBTN_OUT_N,
                        GPIO_DIRECTION_HOST_FRU_EEPROM_MUX_SEL,
                        GPIO_DIRECTION_HOST_PLTRST_N,
                        GPIO_DIRECTION_HOST_S45_N,
                        GPIO_DIRECTION_HOST_S3_N,
                        GPIO_DIRECTION_RS232_CABLE_DETECTED,
                        GPIO_DIRECTION_FP_RSTBTN_N,
                        GPIO_DIRECTION_FP_PWRBTN_N,
                        GPIO_DIRECTION_BMC_FP_BUTTON_OVERRIDE,
                        GPIO_DIRECTION_BMC_RSTBTN_OUT_N,      
};

static FILE *fp = NULL;
static int lock_file;

#define PRODUCT_SUB_PATH "/etc/product_sub.txt"
#define UBMC_SUB_NAME_MAX 20
#define UBMC_ESP_NAME "UBMC_ESP"
#define UBMC_DEFAULT 0
#define UBMC_ESP 1
int get_machine_prod_sub(void)
{
	int ret;
	int ubmc_sub_type;
	char buf[UBMC_SUB_NAME_MAX];
	char prod_sub_name[UBMC_SUB_NAME_MAX];
	char len;
	FILE *fp;
	if(access(PRODUCT_SUB_PATH,F_OK) != 0)
	{
		//eeprom_err("Can not find %s file :%s",PRODUCT_SUB_PATH,strerror(errno));
		syslog(LOG_WARNING,"Can not find %s file :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fp = fopen(PRODUCT_SUB_PATH,"r+");
	if(fp == NULL)
	{
		//eeprom_err("open %s fail :%s",PRODUCT_SUB_PATH,strerror(errno));
		syslog(LOG_WARNING,"open %s fail :%s",PRODUCT_SUB_PATH,strerror(errno));
		return -1;
	}
	fgets(buf,UBMC_SUB_NAME_MAX,fp);
	if (buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';
	strcpy(prod_sub_name,buf);
	if(strcmp(prod_sub_name,UBMC_ESP_NAME) == 0)
	{
		ubmc_sub_type = UBMC_ESP;
	}
	else
	{
		ubmc_sub_type = UBMC_DEFAULT;
	}
	//ubmc_debug("model is %s\n",buf);
	fclose(fp);
	return ubmc_sub_type;
}

int ubmc_sys_ctrl_gpio_init_by_name(const char **gpio_pin_name,unsigned int *pin_no)
{
	int i = 0;
	int pin;
	int ret;
	char bank_num = 0,pin_num = 0;
	int ubmc_sub_type;
	ubmc_sub_type = get_machine_prod_sub();
	if( ubmc_sub_type < 0)
		return -1;
	for(i = 0;i < GPIO_POS_MAX;i ++)
	{
		ret =  silc_gpio_get_by_name(gpio_pin_name[i],&bank_num,&pin_num);
		if(pin < 0)
		{
			return -1;
		}
		/*
		 * The formulation is different between platforms
		 * */
		if( ubmc_sub_type == UBMC_ESP)
		{
			if(bank_num == 0)
			{
				pin = pin_num + 476;
			}
			else if(bank_num == 1)
			{
				pin = pin_num + 446;
			}
		}
		else
		{
			pin = 32 * (bank_num) + (pin_num);
		}
		pin_no[i] = pin;
	}
	return 0;
}

int ubmc_sys_ctrl_gpio_pin_set_direction(ubmc_gpio_pin *pin,int pos)
{
    int ret = GPIO_OP_SUCCESS;
    if(pin->valid == GPIO_INVALID
       || pin->direction != gpio_pin_dir[pos])
    {
        switch (gpio_pin_dir[pos])
        {
            case GPIO_OUT:
                ret = ubmc_gpio_out(pin);
                //printf("gpio %d out\n",gpio_pin_no[pos]);
                break;
            case GPIO_IN:
                ret = ubmc_gpio_in(pin);
                //printf("gpio %d in\n",gpio_pin_no[pos]);
                break;
            default:
                ret = GPIO_OP_FAIL;
                break;
        }
    }


    if(ret)
    {
        return GPIO_OP_FAIL;
    }

    return ret;
}

/*
 *if read,gpio resource can be shared
 */
int ubmc_sys_ctrl_gpio_resource_read_mode_init(gpio_pin_pos pin_pos,resource_gpio_status * flag)
{
    int ret,res;
    int i;
    char path[128];

    i = pin_pos;

    snprintf(path,128,"/sys/class/gpio/gpio%d",gpio_pin_no[i]);
    ret = access(path,R_OK);
    if(!ret)
    {
        /*
         *gpio may be used by this program, and share it
         */
       pin[i].no =gpio_pin_no[i];
       pin[i].fd = -1;
       pin[i].valid = GPIO_VALID;
       *flag = RESOURCE_GPIO_OCCUPY;
       ret =0;
#ifdef DEBUG
       printf("gpio%d RESOURCE_GPIO_OCCUPY\n", gpio_pin_no[i]);
#endif
    }else{
        ret += ubmc_gpio_open(&pin[i],gpio_pin_no[i]);
        ret += ubmc_sys_ctrl_gpio_pin_set_direction(&pin[i],i);        
        if(ret){
            syslog(LOG_ERR, "set gpio%d direction failed\n",gpio_pin_no[i]);
            ret = ubmc_gpio_close(&pin[i]);
        }
#ifdef DEBUG
       printf("gpio%d RESOURCE_GPIO_NO_OCCUPY\n", gpio_pin_no[i]);
#endif
       *flag = RESOURCE_GPIO_NO_OCCUPY;
    }

    return ret;

}

/*
 *if write,gpio resource are exclusive
 */
int ubmc_sys_ctrl_gpio_resource_write_mode_init(gpio_pin_pos pin_pos)
{
    int ret,res;
    int i;

    i = pin_pos;

    ret = ubmc_gpio_open(&pin[i],gpio_pin_no[i]);

    if(ret){
        /*
         *gpio may be used by others,so firstly close it and open it
         */
        ubmc_gpio_close(&pin[i]);

        ret = ubmc_gpio_open(&pin[i],gpio_pin_no[i]);
    }

    if(ret){
        syslog(LOG_ERR, "open gpio%d failed\n",gpio_pin_no[i]);
        goto quit;
    }

    ret = ubmc_sys_ctrl_gpio_pin_set_direction(&pin[i],i);        
    if(ret){
        syslog(LOG_ERR, "set gpio%d direction failed\n",gpio_pin_no[i]);
        goto quit;
    }

    return ret;

quit:
    res = ubmc_gpio_close(&pin[i]);
    
    return res;
}

int ubmc_sys_ctrl_gpio_pin_init(action_op_type op_type)
{
    int ret = 0;

    switch(op_type)
    {
        case ACTION_OP_POWER_ON :
        case ACTION_OP_POWER_OFF :
        case ACTION_OP_FORCE_POWER_OFF :
        case ACTION_OP_CYCLE :
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_PWRBTN_OUT_N);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_HOST_S45_N);
            break;
        case ACTION_OP_RESET :
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_PWRBTN_OUT_N);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_RSTBTN_OUT_N);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_HOST_S45_N);
            break;
        case ACTION_OP_UPGRADE :
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_SPI_HOST_MUX_EN);
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            break;
        case ACTION_OP_PROCESS_STATUS :
            break;
        case ACTION_OP_SERIAL_STATUS :
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_RS232_CABLE_DETECTED);
            break;
        case ACTION_OP_POWER_STATUS :
            ret += ubmc_sys_ctrl_gpio_resource_write_mode_init(GPIO_POS_HOST_S45_N);
            break;
        default:
            break;
    }

    return ret;
}

int ubmc_sys_ctrl_gpio_resource_free(gpio_pin_pos pin_pos)
{
    int i;
    int ret;

    i = pin_pos;
    
    ret = ubmc_gpio_close(&pin[i]);
    //printf("ubmc_gpio_close %d\n",pin[i].no);
    if(ret){
        syslog(LOG_ERR, "close gpio%d failed\n",gpio_pin_no[i]);
    }

    return ret;
    
}

int ubmc_sys_ctrl_gpio_pin_un_init(action_op_type op_type)
{
    
    int ret = 0;

    switch(op_type)
    {
        case ACTION_OP_POWER_ON :
        case ACTION_OP_POWER_OFF :
        case ACTION_OP_FORCE_POWER_OFF :
        case ACTION_OP_CYCLE :
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_BMC_PWRBTN_OUT_N);
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_HOST_S45_N);
            break;
        case ACTION_OP_RESET :
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_BMC_RSTBTN_OUT_N);
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_HOST_S45_N);
            break;
        case ACTION_OP_UPGRADE :
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_SPI_HOST_MUX_EN);
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_BMC_FP_BUTTON_OVERRIDE);
            break;
        case ACTION_OP_PROCESS_STATUS :
            break;
        case ACTION_OP_SERIAL_STATUS :
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_RS232_CABLE_DETECTED);
            break;
        case ACTION_OP_POWER_STATUS :
            ret += ubmc_sys_ctrl_gpio_resource_free(GPIO_POS_HOST_S45_N);
            break;
        default:
            break;
    }

    return ret;
}

int ubmc_sys_ctrl_get_gpio_value(int gpio_pos,int map_gpio_low,int map_gpio_high)
{
    int ret = 0,status = GPIO_OP_FAIL;
    ubmc_gpio_value value;
    ret = ubmc_gpio_get_value(&pin[gpio_pos],&value);
    if(ret)
    {
        return GPIO_OP_FAIL;
    }

    switch(value)
    {
        case GPIO_LOW:
            status = map_gpio_low;
            break;
        case GPIO_HIGH:
            status = map_gpio_high;
            break;
        default:
            status = GPIO_OP_FAIL;
    }

    return status;

}

int ubmc_sys_ctrl_get_host_power_status()
{
    return ubmc_sys_ctrl_get_gpio_value(GPIO_POS_HOST_S45_N,HOST_POWER_DOWN,HOST_POWER_UP);
}

int ubmc_sys_ctrl_get_host_serial_connection_status()
{
   return ubmc_sys_ctrl_get_gpio_value(GPIO_POS_RS232_CABLE_DETECTED,HOST_SERIAL_DISCONNECTED,HOST_SERIAL_CONNECTED); 
}

int ubmc_sys_ctrl_clear_button_override()
{
    int ret = GPIO_OP_SUCCESS;
    ubmc_gpio_value value;

    ret = ubmc_gpio_get_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],&value);
    if(ret)
        return GPIO_OP_FAIL;

    if(value)
    {
        ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    }

    return GPIO_OP_SUCCESS;
}

int ubmc_sys_ctrl_operate_host_action(gpio_pin_pos target_pos,int wait_time_ms,host_power_status power_st)
{
    int ret;
    int count =0;
    int status;
    int loop_count;

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[target_pos],1);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],1);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[target_pos],0);
    if(ret)
        goto quit;

    loop_count = wait_time_ms / 10;
    while(count < loop_count) 
    {
        count ++;
        usleep(10000);//10ms
        if(ubmc_sys_ctrl_get_host_power_status() == power_st)
        {
            break;
        }
    }

    if(count >= loop_count)
    {
        syslog(LOG_WARNING,"Power operation timed out\n");
    }else{
#ifdef DEBUG
        printf("count %d \n",count);
#endif
    }

    ret = ubmc_gpio_set_value(&pin[target_pos],1);
    if(ret)
    {
        goto quit;
    }

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    if(ret)
    {
        goto quit;
    }

    return GPIO_OP_SUCCESS;

quit:
    /*
     *restore privous status
     */
    ubmc_gpio_set_value(&pin[target_pos],1);
    ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    return GPIO_OP_FAIL;
}

int ubmc_sys_ctrl_power_on_host()
{
    host_power_status status;
    status = ubmc_sys_ctrl_get_host_power_status();
    if(status == HOST_POWER_UP)
    {
        syslog(LOG_NOTICE,"Host already powered on\n");
        return GPIO_OP_FAIL;
    }
    return ubmc_sys_ctrl_operate_host_action(GPIO_POS_BMC_PWRBTN_OUT_N, SLEEP_TIME_MS_MAX, HOST_POWER_UP);
}

int ubmc_sys_ctrl_force_power_off_host()
{
    host_power_status status;
    status = ubmc_sys_ctrl_get_host_power_status();
    if(status == HOST_POWER_DOWN)
    {
        syslog(LOG_NOTICE,"Host already powered off\n");
        return GPIO_OP_FAIL;
    }
    return ubmc_sys_ctrl_operate_host_action(GPIO_POS_BMC_PWRBTN_OUT_N, SLEEP_TIME_MS_LONG_MAX, HOST_POWER_DOWN);
}

int ubmc_sys_ctrl_power_cycle_host()
{
    int ret;

    ret = ubmc_sys_ctrl_force_power_off_host();
    if(ret != GPIO_OP_SUCCESS)
    {
        return ret;
    }

    sleep(2);

    ret = ubmc_sys_ctrl_power_on_host();
    if(ret != GPIO_OP_SUCCESS)
    {
        return ret;
    }

    return ret;


}

int ubmc_sys_ctrl_power_action_no_check(gpio_pin_pos pin_ctrl, int wait_time_ms)
{
    int ret;

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[pin_ctrl],1);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],1);
    if(ret)
        goto quit;
    ret = ubmc_gpio_set_value(&pin[pin_ctrl],0);
    if(ret)
        goto quit;

    usleep(wait_time_ms * 1000);

    ret = ubmc_gpio_set_value(&pin[pin_ctrl],1);
    if(ret)
        goto quit;

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    if(ret)
        goto quit;

    return GPIO_OP_SUCCESS;

quit:
    /*
     *restore privous status
     */
    ubmc_gpio_set_value(&pin[pin_ctrl],1);
    ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    return GPIO_OP_FAIL;
}

int ubmc_sys_ctrl_force_reset_host()
{
    host_power_status status;
    status = ubmc_sys_ctrl_get_host_power_status();
    if(status == HOST_POWER_DOWN)
    {
        syslog(LOG_WARNING,"Host not powered on\n");
        return GPIO_OP_FAIL;
    }
    return ubmc_sys_ctrl_power_action_no_check(GPIO_POS_BMC_RSTBTN_OUT_N, SLEEP_TIME_MS_RESET);
}

int ubmc_sys_ctrl_power_off_host()
{
    host_power_status status;
    status = ubmc_sys_ctrl_get_host_power_status();
    if(status == HOST_POWER_DOWN)
    {
        syslog(LOG_WARNING,"Host not powered on\n");
        return GPIO_OP_FAIL;
    }
    return ubmc_sys_ctrl_power_action_no_check(GPIO_POS_BMC_PWRBTN_OUT_N, SLEEP_TIME_MS_POWER_OFF);
}

int ubmc_sys_ctrl_update_host_bios()
{
    int ret;
    ret =ubmc_sys_ctrl_clear_button_override();
    if(ret)
    {
        goto quit;
    }

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],1);
    if(ret)
    {
        goto quit;
    }

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_SPI_HOST_MUX_EN],0);
    if(ret)
    {
        goto quit;
    }

    /*
     *BIOS update program will be called
     */

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_SPI_HOST_MUX_EN],1);
    if(ret)
    {
        goto quit;
    }

    ret = ubmc_gpio_set_value(&pin[GPIO_POS_BMC_FP_BUTTON_OVERRIDE],0);
    if(ret)
    {
        goto quit;
    }

    return ret;

quit:
    return GPIO_OP_FAIL;
}

void usage(char * prog) {
  fprintf(stderr,"usage: %s [-h] [-o] [-s] [-f] [-c] [-r] [-g] [-q] [-u] [-p]\n \
             -h : help \n \
             -c : cycle \n \
             -o : power on \n \
             -s : power off \n \
             -f : power forceoff \n \
             -g : query serial connection status \n \
             -q : query power status \n \
             -p : operiton progress \n \
             -u : upgrade BIOS \n \
             -r : reset \n", prog);
}

int ubmc_sys_ctrl_instance_is_exsit()
{

    int lock_res;

    lock_file = open(host_sys_ctrl_lock_file, O_CREAT | O_RDWR, 0666);
    lock_res = flock(lock_file, LOCK_EX | LOCK_NB);

    if(lock_res && EWOULDBLOCK == errno) {

        return 1;
    } 

    return 0;

}

int ubmc_sys_ctrl_write_status(char* str)
{
	FILE* status_fp;

	status_fp = fopen(host_sys_ctrl_status, "w+");
	if(status_fp == NULL)
		return -1;
    fwrite(str, strlen(str)+1, 1, status_fp);
    fclose(status_fp);

	return 0;
}

int ubmc_sys_ctrl_read_status(char* buf, int len)
{
	FILE* status_fp;

	status_fp = fopen(host_sys_ctrl_status, "r");
	if(status_fp == NULL)
		return -1;
    fgets(buf, len, status_fp);
    fclose(status_fp);

	return 0;
}

static char * acton_tag_value[ACTION_OP_MAX] = {
		"Powering on the host",
		"Powering off the host",
		"Powering off the host",
		"Powering cycle the host",
		"Resetting the host",
		"N/A",
		"N/A",
		"N/A",
		"N/A",
};
int ubmc_sys_ctrl_init_status(action_op_type op)
{
	char str[128];
	char* prog_tag = "...";

	sprintf(str, "%s%s", acton_tag_value[op], prog_tag);
	return ubmc_sys_ctrl_write_status(str);
}

int ubmc_sys_ctrl_sync_status(action_op_type op, int ret)
{
	char str[128];
	char* ret_str = (ret==GPIO_OP_SUCCESS) ? "OK" : "Error";

	printf("%s\n", ret_str);
	sprintf(str, "%s: %s", acton_tag_value[op], ret_str);
	return ubmc_sys_ctrl_write_status(str);
}

static inline int ubmc_sys_ctrl_get_power_status()
{
    int ret;
    resource_gpio_status flag;

    ret = GPIO_OP_FAIL;
    flag = RESOURCE_GPIO_OCCUPY;

    if(ubmc_sys_ctrl_instance_is_exsit())
    {
        ubmc_sys_ctrl_gpio_resource_read_mode_init(GPIO_POS_HOST_S45_N,&flag);
        ret = ubmc_sys_ctrl_get_host_power_status();
        if(flag == RESOURCE_GPIO_NO_OCCUPY)
            ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_STATUS);
    }
    else
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_STATUS);
        ret = ubmc_sys_ctrl_get_host_power_status();
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_STATUS);
    }

    return ret;
}
#if 0
int ubmc_sys_ctrl_status_read_mode_init(FILE *fp)
{
    return ubmc_ctrl_file_sync_param(fp); 
}

int ubmc_sys_ctrl_status_write_mode_init(param_attr_pos pos)
{
    if(ubmc_sys_ctrl_instance_is_exsit())
    {
#ifdef DEBUG
        printf("host_sys_control already running\n"); // another instance is running
#endif
        return -1;
    }

    fp = fopen(host_sys_ctrl_status,"w+");
    if(fp == NULL)
        return -1;

    ubmc_ctrl_param_init(fp); 
    ubmc_ctrl_set_operation_type(pos);
    ubmc_ctrl_set_param_value(pos, PARAM_STATUS_POS_IN_PROCESS);
    ubmc_ctrl_param_sync_file(fp);

    return 0;
}

int ubmc_sys_ctrl_sync_status_to_file(param_attr_pos pos, int ret)
{
    
    if(ret == GPIO_OP_SUCCESS)
    {
        ubmc_ctrl_set_param_value(pos, PARAM_STATUS_POS_OK);
        printf("OK\n");
    }else{
        ubmc_ctrl_set_param_value(pos, PARAM_STATUS_POS_ERROR);
        printf("Error\n");
    }

    ubmc_ctrl_param_sync_file(fp);
    return 0;
}
#endif
static inline int ubmc_sys_ctrl_do_upgrade_bios()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_UPGRADE);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_UPGRADE);
        ret = ubmc_sys_ctrl_update_host_bios();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_UPGRADE, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_UPGRADE);
        fclose(fp);

    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_UP)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is not powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_UPGRADE);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_UPGRADE);
    ret = ubmc_sys_ctrl_update_host_bios();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_UPGRADE);
    ubmc_sys_ctrl_sync_status(ACTION_OP_UPGRADE, ret);
#endif
    return ret;
}

static inline int ubmc_sys_ctrl_do_reset()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_RESET);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_RESET);
        ret = ubmc_sys_ctrl_force_reset_host();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_RESET, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_RESET);
        fclose(fp);

    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_UP)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is not powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_RESET);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_RESET);
    ret = ubmc_sys_ctrl_force_reset_host();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_RESET);
    ubmc_sys_ctrl_sync_status(ACTION_OP_RESET, ret);
#endif
    return ret;
}

static inline int ubmc_sys_ctrl_do_cycle_boot()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_CYCLE);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_CYCLE);
        ret = ubmc_sys_ctrl_power_cycle_host();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_CYCLE, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_CYCLE);
        fclose(fp);
    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_UP)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is not powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_CYCLE);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_CYCLE);
    ret = ubmc_sys_ctrl_power_cycle_host();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_CYCLE);
    ubmc_sys_ctrl_sync_status(ACTION_OP_CYCLE, ret);
#endif
    return ret;
}

static inline int ubmc_sys_ctrl_do_process_status()
{
#if 0
    int ret;
    int pos;
    char buf[param_attr_size];
    char op_type_buf[param_attr_size];

    fp = fopen(host_sys_ctrl_status,"r");
    if(fp == NULL)
    {
        printf("no current action\n");
        return -1;
    }
    ret = ubmc_sys_ctrl_status_read_mode_init(fp);
    if(!ret)
    {
        pos = ubmc_ctrl_get_operation_type_pos();
        if(pos != PARAM_ATTR_POS_MAX)
        {
            ubmc_ctrl_get_param_value(buf,param_attr_size,pos);
            ubmc_ctrl_get_param_value(op_type_buf, param_attr_size,PARAM_ATTR_POS_TYPE);
            printf("%s : %s\n",op_type_buf,buf);
        }else{
            syslog(LOG_ERR, "Fail to get operation status from %s\n",host_sys_ctrl_status);
            printf("no current action\n");
        }
    }
    fclose(fp);
    return ret;
#else
    char buf[128] = {0};

    if(access(host_sys_ctrl_status, F_OK) != 0 ||
    		ubmc_sys_ctrl_read_status(buf, 128) != 0)
    {
        printf("N/A\n");
        return -1;
    }
    printf("%s\n", buf);
    return 0;
#endif
}

static inline int ubmc_sys_ctrl_do_power_status()
{
    int ret;
#if 0
    resource_gpio_status flag;

    ret = GPIO_OP_FAIL;
    flag = RESOURCE_GPIO_OCCUPY;

    instance_is_exsit = ubmc_sys_ctrl_instance_is_exsit();
    if(instance_is_exsit == INSTANCE_EXSIT)
    {
        ubmc_sys_ctrl_gpio_resource_read_mode_init(GPIO_POS_HOST_S45_N,&flag);
        ret = ubmc_sys_ctrl_get_host_power_status();
        if(flag == RESOURCE_GPIO_NO_OCCUPY)
            ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_STATUS);
            
    }else{
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_STATUS);
        ret = ubmc_sys_ctrl_get_host_power_status();
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_STATUS);
    }
#else
    ret = ubmc_sys_ctrl_get_power_status();
#endif
    if(ret == HOST_POWER_UP)
    {
        printf("Host is powered up\n");
    }else if(ret == HOST_POWER_DOWN)
    {
        printf("Host is powered down\n");
    }else{
        printf("Get host power status error\n");
    }

    return ret;
}

static inline int ubmc_sys_ctrl_do_serial_connection_status()
{
    int ret;

    ret = GPIO_OP_FAIL;
    resource_gpio_status flag;

    if(ubmc_sys_ctrl_instance_is_exsit())
    {
        ubmc_sys_ctrl_gpio_resource_read_mode_init(GPIO_POS_RS232_CABLE_DETECTED,&flag);
        ret = ubmc_sys_ctrl_get_host_serial_connection_status();
        if(flag == RESOURCE_GPIO_NO_OCCUPY)
            ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_SERIAL_STATUS);
    }else{
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_SERIAL_STATUS);
        ret = ubmc_sys_ctrl_get_host_serial_connection_status();
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_SERIAL_STATUS);
    }
    if(ret == HOST_SERIAL_CONNECTED)
    {
        printf("Serial is connected\n");
    }else if(ret == HOST_SERIAL_DISCONNECTED)
    {
        printf("Serial is disconnected\n");
    }else{
        printf("Get serial status error\n");
    }

    
    return ret;
}

static inline int ubmc_sys_ctrl_do_force_power_off()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_FORCE_POWER_OFF);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_FORCE_POWER_OFF);
        ret = ubmc_sys_ctrl_force_power_off_host();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_FORCE_POWER_OFF, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_FORCE_POWER_OFF);
        fclose(fp);
    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_UP)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is not powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_FORCE_POWER_OFF);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_FORCE_POWER_OFF);
    ret = ubmc_sys_ctrl_force_power_off_host();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_FORCE_POWER_OFF);
    ubmc_sys_ctrl_sync_status(ACTION_OP_FORCE_POWER_OFF, ret);
#endif
    return ret;
}

static inline int ubmc_sys_ctrl_do_power_off()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_POWER_OFF);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_OFF);
        ret = ubmc_sys_ctrl_power_off_host();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_POWER_OFF, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_OFF);
        fclose(fp);

    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_UP)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is not powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_POWER_OFF);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_OFF);
    ret = ubmc_sys_ctrl_power_off_host();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_OFF);
    ubmc_sys_ctrl_sync_status(ACTION_OP_POWER_OFF, ret);
#endif
    return ret;
}

static inline int ubmc_sys_ctrl_do_power_on()
{
    int ret;
#if 0
    ret = ubmc_sys_ctrl_status_write_mode_init(PARAM_ATTR_POS_POWER_ON);
    if(!ret)
    {
        ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_ON);
        ret = ubmc_sys_ctrl_power_on_host();
        ubmc_sys_ctrl_sync_status_to_file(PARAM_ATTR_POS_POWER_ON, ret);
        ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_ON);
        fclose(fp);
    }
#else
    if(ubmc_sys_ctrl_instance_is_exsit())
    	return GPIO_OP_FAIL;
    if(ubmc_sys_ctrl_get_power_status() != HOST_POWER_DOWN)
    {
    	ubmc_sys_ctrl_write_status("Error: the host is already powered up!");
    	return GPIO_OP_FAIL;
    }
    ubmc_sys_ctrl_init_status(ACTION_OP_POWER_ON);
    ubmc_sys_ctrl_gpio_pin_init(ACTION_OP_POWER_ON);
    ret = ubmc_sys_ctrl_power_on_host();
    ubmc_sys_ctrl_gpio_pin_un_init(ACTION_OP_POWER_ON);
    ubmc_sys_ctrl_sync_status(ACTION_OP_POWER_ON, ret);
#endif
    return ret;
}

int main(int argc ,char *argv[])
{
    int ret;

    char * prog;
    int opt;

    /*
     *record if there is arguments or not
     */
    int arg_flag = 0;

    prog = argv[0];
    ret = ubmc_sys_ctrl_gpio_init_by_name(gpio_pin_name,gpio_pin_no);
    if(ret < 0)
    {
    	syslog(LOG_ERR, "Fail to init gpio _by_name \n");
    }
    while ( (opt = getopt(argc,argv,"osfgqpcrhu")) != -1) {

        arg_flag ++;

        switch(opt) {
          case 'o':
            ret = ubmc_sys_ctrl_do_power_on();
            break;
          case 's':
            ret = ubmc_sys_ctrl_do_power_off();
            break;
          case 'f':
            ret = ubmc_sys_ctrl_do_force_power_off();
            break;
          case 'g':
            ret = ubmc_sys_ctrl_do_serial_connection_status();
            break;
          case 'q':
            ret = ubmc_sys_ctrl_do_power_status();
            break;
          case 'p':
            ret = ubmc_sys_ctrl_do_process_status();
            break;
          case 'c':
            ret = ubmc_sys_ctrl_do_cycle_boot();
            break;
          case 'r':
            ret = ubmc_sys_ctrl_do_reset();
            break;
          case 'u':
            ret = ubmc_sys_ctrl_do_upgrade_bios();
            break;
          case '?':
          case 'h':
          default:
            usage(prog);
            break;
        }
    }

    /*
     *if there is arguments ,output usage
     */
    if(!arg_flag)
        usage(prog);
        
    
    /*
     *for every case,error code are not from return code
     */
    return 0;
}
