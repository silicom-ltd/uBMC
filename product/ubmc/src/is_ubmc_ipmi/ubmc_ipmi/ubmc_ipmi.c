#include "ubmc_ipmi.h"
#include "ubmc_state.h"
#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi_sdr.h"
#include "ubmc_ipmi_hst_pwr_monitor.h"
#include "ubmc_ipmi_lan.h"
#include "ubmc_state.h"
#include "ubmc_ipmi_board.h"
#include <signal.h>
#include "ubmc_ipmi_sel_sensor.h"
#include <errno.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <netinet/in.h>
#include "ubmc_ipmi_config.h"
void* ubmc_sensor_base = NULL;
ubmc_shm_state * ubmc_shm_base = NULL;
ubmc_sensor_event *sensor_events[SENSOR_MAX_NUM] = {0};
//struct ubmc_ipmi_sel ubmc_ipmi_g_sel;
static runtime_s g_runtime;

//static int update_sensor_state_callback(void *ubmc_sensor_arry,uint8_t flag,void *output);
struct ubmc_ipmi_s g_ubmc_ipmi;

int pmbus_get_linear_value(unsigned short in_value,float* out_value)
{

	unsigned short N,Y;
	unsigned char exp,i;
	Y = in_value & 0x07ff;
	if(Y == 0)
	{
		*out_value = 0;
		return 0;
	}
	N = (in_value >> 11) & 0x001f;
	printf("N1 is 0x%x \n",N);
	N = ((~N) & 0x01f)+1;
	printf("N2 is 0x%x \n",N);
	exp = 1 << N;
	*out_value = (float)Y/exp;
	printf("pmbus_get_linear_value:in_valueis 0x%x Y is 0x%x,N is 0x%x exp is %d out_value is %f \n",in_value,Y,N,exp,*out_value);
	return 0;

}

int do_i2c_read_test(const char *dev,unsigned char slave_adr)
{
	uint16_t value;
	uint16_t *p_value;
	int i ;
	int ret;
	unsigned char reg;
	float out;
	p_value = &value;

	reg = 0x20;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	printf("VOUT MODE :0x%x is 0x%x \n",reg,value);

	reg = 0x79;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	printf("summry state :0x%x is 0x%x \n",reg,value);
	reg = 0x7a;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	printf("states vout :0x%x is 0x%x \n",reg,value);
	reg = 0x7b;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	printf("states iout :0x%x is 0x%x \n",reg,value);
	reg = 0x7c;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	printf("states input :0x%x is 0x%x \n",reg,value);
	reg = 0x7d;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	printf("states temp :0x%x is 0x%x \n",reg,value);
	reg = 0x88;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	printf("read vin :0x%x is 0x%x ,value %f\n",reg,value,value/8.0);
	reg = 0x89;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,0);
	pmbus_get_linear_value(value,&out);
	printf("read iin :0x%x is 0x%x Y is 0x%x N is 0x%x value is %f \n",reg,value,value&0x7ff,(value >> 11)&0x1f,out);

	reg = 0x8b;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	pmbus_get_linear_value(value,&out);
	printf("read vout :0x%x is 0x%x Y is 0x%x N is 0x%x value is %f \n",reg,value,value&0x7ff,(value >> 11)&0x1f,out);
	//printf("read vout :0x%x is 0x%x ,value is %f\n",reg,value,value/4096.0);
	reg = 0x8c;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	printf("read iout :0x%x is 0x%x ,value is %f\n",reg,value,value/16.0);
	reg = 0x8d;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	printf("read temprature 0x%x is 0x%x value is %.3f \n",reg,value,value/4.0);
	reg = 0x90;
	ubmc_smbus_read_value("/dev/i2c-0",slave_adr,reg,p_value,1);
	printf("read fan speed 0x%x is 0x%x \n",reg,value);

}


int ubmc_get_path_of_sensor(char* senor_dev_name,char *senor_path,const char* sys_path_fuffix)
{
#define SYS_HWMON_PATH "/sys/class/hwmon/"
#define SYS_HWMON_FILE_NUM 3
	char path[IPMI_SENSOR_PATH_MAX] = {0};
	char buf[IPMI_SENSOR_DEV_NAME_MAX] = {0};
	char name[IPMI_SENSOR_DEV_NAME_MAX] = {0};
	char cmd[256] = {0};
	char *tmp;
	FILE* fp;
	int ret;
	int i = 0;
	for(i = 0; i < SYS_HWMON_FILE_NUM; i++)
	{
		memset(path,0,sizeof(path));
		memset(cmd,0,sizeof(cmd));
		memset(name,'\0',sizeof(name));
		memset(buf,'\0',sizeof(buf));
		sprintf(path,"%s""hwmon%d/%s",SYS_HWMON_PATH,i,sys_path_fuffix);
		if(!ubmc_is_file_exist(path))
		{
			ubmc_error("No such file  %s can nott find %s \n",path,senor_dev_name);
			return -1;
		}
		sprintf(cmd,"cat %s/name 2>/dev/null",path);
		fp = popen(cmd,"r");
		if( fp == NULL )
		{
			ubmc_error("popen Error!\n");
			return -1;
		}
	    tmp = fgets(buf,sizeof(buf),fp);
	    if(buf[strlen(buf)-1] == '\n')
	    	buf[strlen(buf)-1] = '\0';
	    strcpy(name,buf);
		if( strcmp(senor_dev_name,name) == 0 )
		{
			strcpy(senor_path,path);
			ret = pclose(fp);
			if(ret < 0)
			{
				ubmc_error("pclose  %s fail :%s!\n",cmd,strerror(errno));
				return ret;
			}
			return 1;
		}
		ret = pclose(fp);
		if(ret < 0)
		{
			ubmc_error("pclose  %s fail :%s!\n",cmd,strerror(errno));
			return ret;
		}
	}
	ubmc_error("can not find %s \n",senor_dev_name);
	return -1;
}


ubmc_sensor_event* ubmc_add_sensor( struct ubmc_sensor_config_s *config)
{
	ubmc_sensor_event *sensor;
	int ret;
	char buffer[128] = {0};
	char fault_path[128] = {0};
	sensor = malloc(sizeof(struct ubmc_sensor_event));
	if(sensor == NULL)
	{
		ubmc_error("Malloc fail : %s \n",strerror(errno));
		return NULL;
	}
	memset(sensor,0,sizeof(struct ubmc_sensor_event));
	strcpy(sensor->sensor_dev_name,config->dev_name);
	strcpy(sensor->sensor_name,config->sensor_name);
	if(config->sensor_fault_path != NULL)
	{
		strcpy(sensor->sensor_fault_path,config->sensor_fault_path);
	}
	else
	{
		sensor->sensor_fault_path == NULL;
	}
	sensor->type = config->sensor_type;
	sensor->sensor_value_multiple = config->sensor_value_multiple;
	//call back function
	sensor->index_of_gui_array = config->index_of_gui_array;
	sensor->update_sensor_state = config->update_sensor_state;
	sensor->sync_sensor_gui_shm = config->sync_sensor_gui_shm;
	sensor->sync_sensor_kernel_shm = config->sync_sensor_kernel_shm;
	sensor->sensor_event_handler = config->sensor_event_handler;
	sensor->sensor_read_dev_value_handler = config->sensor_read_dev_value_handler;
	sensor->sensor_read_file_value_handler = config->sensor_read_file_value_handler;
	//adding anther callback function
	sensor->sensor_id = config->sensor_id;
	sensor->read_value_mode = config->sensor_read_value_mode;
	sensor->ipmitool_factor = &config->ipmitool_factor;
	sensor->sub_type = config->sub_type;
	memcpy(&sensor->thresh,config->sensor_thresh,sizeof(ubmc_ipmi_sensor_thresh_s));
	sensor->channel_id = config->sensor_channel_id;
	memset(sensor->sensor_fault_path,'\0',sizeof(sensor->sensor_fault_path));
	if(sensor->read_value_mode == FILE_MODE)
	{
		ret = ubmc_get_path_of_sensor(sensor->sensor_dev_name,sensor->sensor_path,config->sensor_sys_path_suffix);
		if(ret < 0)
		{
			ubmc_error("ubmc_get_path_of_sensor fail:%s sensor_path %s \n",sensor->sensor_dev_name,sensor->sensor_path);
			goto err;
		}
		if(sensor->type == UBMC_SENSOR_TEMP)
		{
			sprintf(buffer,"%s/temp%d_input",sensor->sensor_path,sensor->channel_id);
		}
		else if(sensor->type == UBMC_SENSOR_FAN )
		{
			sprintf(buffer,"%s/fan%d_input",sensor->sensor_path,sensor->channel_id);
			//only Fan has fault state
			sprintf(fault_path,"%s/fan%d_fault",sensor->sensor_path,sensor->channel_id);
		}
		else if(sensor->type == UBMC_SENSOR_VOL)
		{
			if(config->sensor_filename_flag == 0)
			{
				sprintf(buffer,"%s/in%d_input",sensor->sensor_path,sensor->channel_id);
			}
			else if(config->sensor_filename_flag == 1)
			{
				sprintf(buffer,"%s/vmon%d",sensor->sensor_path,sensor->channel_id);
			}
		}
		else
		{
			ubmc_error("Do not support this sensor type \n");
			goto err;
		}
	}
	else if(sensor->read_value_mode == DEV_MODE)
	{
		if(sensor->type == UBMC_SENSOR_TEMP)
		{
			sprintf(buffer,"%s",sensor->sensor_dev_name);
		}
		else if(sensor->type == UBMC_SENSOR_VOL)
		{
			sprintf(buffer,"%s",sensor->sensor_dev_name);
		}
		sensor->sensor_value_reg = config->sensor_read_value_reg;
		sensor->dev_adr = config->dev_adr;
	}

	strcpy(sensor->sensor_value_path,buffer);
	if(ubmc_is_file_exist(fault_path))
	{
		strcpy(sensor->sensor_fault_path,fault_path);
	}
	return sensor;
err:
	free(sensor);
	return NULL;
}
//register sensor date by different sensor configures
int ubmc_register_sensor(struct ubmc_board_info_s *ubmc_board_info ,ubmc_sensor_event **ubmc_sensor_arry)
{
	int i = 0;
	struct ubmc_sensor_config_s *cur_cfg;
	cur_cfg = ubmc_board_info->sensor_config;
	for(i = 0;i < ubmc_board_info->sensor_max_num;i ++)
	{
		ubmc_sensor_arry[i] = ubmc_add_sensor(&cur_cfg[i]);
		if(ubmc_sensor_arry[i] == NULL)
			return -1;
		ubmc_sensor_arry[i]->sensor_offset = i;
		ubmc_sensor_arry[i]->sensor_max_num = ubmc_board_info->sensor_max_num;
	}
	return BMC_SUCCESS;
}

ubmc_sensor_event* ubmc_ipmi_find_sesor_by_name(ubmc_sensor_event **ubmc_sensor_arry,char *sensor_name)
{
	int i = 0;
	if(sensor_name == NULL)
		return NULL;
	for(i = 0;i < SENSOR_MAX_NUM;i ++)
	{
		if(strcmp(ubmc_sensor_arry[i]->sensor_name,sensor_name) == 0)
			return ubmc_sensor_arry[i];
	}
	return NULL;
}

static bool equal(double elem1, double elem2)
{
    if ((elem1 - elem2 < 0.0000001) && (elem1 - elem2 > -0.0000001))
        return true;
    else
        return false;
}

static double power_unsigned_exp(double base, unsigned int exponent)
{
    double result = 1, last_pow = base;
       while (exponent > 0) {
           if ((exponent & 1) != 0)
        	   result = result * last_pow ;
           exponent = exponent >> 1;
           last_pow = last_pow * last_pow;
       }
       return result;
}

static double power(double base, int exponent)
{
	bool isnegative = false;
    unsigned int abs_exponent = exponent;
    double result = 0.0;

    if (equal(base, 0.0) && exponent < 0)
    {
        return 0.0;
    }

    if (exponent < 0){
    	isnegative = true;
        abs_exponent = (unsigned int)(-exponent);
    }
    result = power_unsigned_exp(base, abs_exponent >> 1);

    return isnegative ? 1 / result : result;
}

static void ubmc_ipmi_init_runtime(runtime_s *runtime)
{
	runtime->start = 1;
	runtime->interval_sec = 0;
	gettimeofday(&runtime->start_time,NULL);
	gettimeofday(&runtime->curr_time,NULL);
	gettimeofday(&runtime->last_time,NULL);

}

#define MAX_TIME_INTERVAL_SEC 60
static int ubmc_ipmi_get_rt_interval(runtime_s *runtime)
{
    int interval_sec;
    unsigned int time;
	struct timeval curr_time;
	gettimeofday(&curr_time,NULL);
	interval_sec = curr_time.tv_sec - runtime->last_time.tv_sec;
	if(runtime->start == 1)
	{
		//maybe the date has been changed
		if(interval_sec < 0 || interval_sec > MAX_TIME_INTERVAL_SEC)
		{
			//rewrite start_time by interval_sec,we should make sure that
			//start_time.tv_sec = curr_time.tv_sec - interval_sec
			runtime->start_time.tv_sec =curr_time.tv_sec - runtime->interval_sec;
			time = curr_time.tv_sec - runtime->start_time.tv_sec;
		}
		else
		{
			time = curr_time.tv_sec - runtime->start_time.tv_sec;
		}
		runtime->interval_sec = time;
		gettimeofday(&runtime->last_time,NULL);
		return runtime->interval_sec;
	}
	else
	{
		return 0;
	}
}

static void update_all_sensor_states(struct ubmc_ipmi_s* ubmc_ipmi)
{
	uint8_t i = 0;
	char sensor_num;
	struct ubmc_board_info_s* board_info;
	sensor_num = ubmc_ipmi->ubmc_board_info.sensor_max_num;
	ubmc_sensor_event *pevent = NULL;
	for(i=0;i<sensor_num;++i){
		pevent = sensor_events[i];
		if( pevent->update_sensor_state(pevent,ubmc_ipmi,i,NULL) == BMC_FALSE ){
			ubmc_error(" Update %s sensor data Error \n",pevent->sensor_name);
		}
	}
}

static int ubmc_open_device(const char *devname , int *pfd)
{
	int fd = 0;
	if( !devname || !pfd ){
		return BMC_FALSE;
	}
	fd = open(devname,O_RDWR);
	if (fd < 0)
	{
		ubmc_error(" open dev %s Error \n",BMC_DEVICE_NAME);
		return BMC_FALSE;
	}
	*pfd = fd;

	return BMC_SUCCESS;
}

static int ubmc_mmap_sdr_base(struct ubmc_ipmi_s *ubmc_ipmi,int fd,void **pbase)
{
	void *base = NULL;
	int ret;
	if( !pbase || fd == -1 ){
		return BMC_FALSE;
	}
	ret = ioctl(fd,IOCTL_INIT_SHM_DEVICE_TYPE_CMD,&ubmc_ipmi->ubmc_board_info.device_type);
	if(ret < 0)
	{
		ubmc_error(" IOCTL fail: can not get device type \n");
		return BMC_FALSE;
	}

	base = mmap(NULL, PAGE_SIZE*2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(base == (void *)-1) {
		ubmc_error(" mmap fail Error \n");
		return BMC_FALSE;
	}
	*pbase = base;
	ubmc_ipmi->ubmc_sensor_base = base;
	return BMC_SUCCESS;
}

/**
 * the thread use to gather all sensors' data interval
 * */
static void gather_sensors_data_thread( void* data )
{
	struct timeval timeout = {0};
	int ret ;
	char ubmc_ipmi_is_ok;
	struct ubmc_ipmi_s* ipmi;
	ipmi = (struct ubmc_ipmi_s*)data;
	ubmc_ipmi_is_ok = 1;
	while(ubmc_ipmi_is_ok) {
		timeout.tv_sec = BMC_SENORS_DATA_GATHER_INTERVAL;
		timeout.tv_usec=0;

		// just use the select funciton as a timer
	    switch( select(0,NULL,NULL,NULL,&timeout) )
		{
		case -1:
			/* Interrupted system call */
			//ubmc_debug(" Call select fail! errno = %d\n", errno);
			break;
		case 0:
			//ubmc_debug(" Timer expire when call select! \n");
			update_all_sensor_states(ipmi);
			break;
		default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
    }
}

// read default ubmc sensors state from the shared memory
static int read_default_ubmc_sensor_states_from_shm(struct ubmc_board_info_s *board_info)
{
	uint8_t count = 0;
	uint8_t i = 0;
	ubmc_sensor_s *pbase = (ubmc_sensor_s *)ubmc_sensor_base;
	ubmc_ipmi_state *ubmc_state;
	ubmc_state = (ubmc_ipmi_state *)ubmc_shm_base->map_addr;
	while( pbase && pbase->name[0] != '\0' ){
		count++;
		pbase++;
	}
	if( count != board_info->sensor_max_num ){
		ubmc_error("#### Read all sensor from shm Error! count:%d != num:%d .Maybe ubmc device type error ####\n",count,board_info->sensor_max_num);
		return BMC_FALSE;
	}
	if(board_info->temp_sensor_max_num > 0)
	{
		for(i=0 ; i < board_info->temp_sensor_max_num; i++)
		{
			ubmc_state->temp_state[i].is_enable = 1;
		}

	}
	if(board_info->volt_sensor_max_num > 0)
	{
		for(i=0 ; i < board_info->volt_sensor_max_num; i++)
		{
			ubmc_state->vol_state[i].is_enable = 1;
		}
	}

	if(board_info->fan_sensor_max_num > 0)
	{
		for(i=0 ; i < board_info->fan_sensor_max_num; i++)
		{
			ubmc_state->fan_state[i].is_enable = 1;
		}
	}
	if(board_info->power_supply_sensor_max_num > 0)
	{
		for(i=0 ; i < board_info->power_supply_sensor_max_num; i++)
		{
			ubmc_state->power_supply_state[i].is_enable = 1;
		}
	}

	return BMC_SUCCESS;
}

#define OEMI_MODEL_FILE_PATH "/etc/product/UBMC/OEMI/model.txt"
#define DECODE_NAME_MAX     20
#define UBMC_XSMALL_MODEL "ATT-V150"
#define UBMC_SMALL_MODEL "ATT-V250"
#define UBMC_MIDDLE_MODEL "ATT-V450"
#define UBMC_LARGE_MODEL "ATT-V850"
int get_machine_model(void)
{
	int ret;
	device_type_t device_type;
	char buf[DECODE_NAME_MAX];
	char model[DECODE_NAME_MAX];
	FILE *fp;
	if(access(OEMI_MODEL_FILE_PATH,F_OK) != 0)
	{
		ubmc_error("Can not find %s file :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return -1;
	}
	fp = fopen(OEMI_MODEL_FILE_PATH,"r+");
	if(fp == NULL)
	{
		ubmc_error("open %s fail :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return -1;
	}
	fgets(buf,DECODE_NAME_MAX,fp);
	if(strcmp(buf,UBMC_XSMALL_MODEL) == 0)
	{
		device_type = XSMALL;
	}
	else if(strcmp(buf,UBMC_SMALL_MODEL) == 0)
	{
		device_type = SMALL;
	}
	else if(strcmp(buf,UBMC_MIDDLE_MODEL) == 0)
	{
		device_type = MIDDLE;
	}
	else if(strcmp(buf,UBMC_LARGE_MODEL) == 0)
	{
		device_type = LARGE;
	}
	else
	{
		device_type = UNKNOW;

	}
	//ubmc_debug("model is %s\n",buf);
	fclose(fp);
	return device_type;
}
int get_device_type(struct ubmc_ipmi_s *ubmc_ipmi)
{
	int ret = 0;
	device_type_t device_type;
	if((ret == 1) || (ubmc_ipmi == NULL))
	{
		return -1;
	}
	ret = get_machine_model();
	if(ret < 0 || ret >= UNKNOW)
	{
		ubmc_ipmi->ubmc_board_info.device_type = XSMALL;
		ubmc_error("can not find right model info,using XSMALL as default configuration! \n")
	}
	else
	{
		ubmc_ipmi->ubmc_board_info.device_type = ret;
	}
	//ubmc_ipmi->ubmc_board_info.device_type = SMALL;

	return ubmc_ipmi->ubmc_board_info.device_type;
}


int init_board_configs(struct ubmc_ipmi_s *ubmc_ipmi)
{
	int ret = 0;
	device_type_t device_type;
	if(get_device_type(ubmc_ipmi) < 0 )
	{
		ubmc_error("get_device_type fail \n");
		return BMC_FALSE;
	}
	device_type = ubmc_ipmi->ubmc_board_info.device_type;
	if(device_type == XSMALL)
	{
		ubmc_ipmi->ubmc_board_info.sensor_config = ubmc_xsmall_sensor_cfg;
		ubmc_ipmi->ubmc_board_info.sensor_max_num = XSMALL_SENSOR_MAX_NUM;
		ubmc_ipmi->ubmc_board_info.fan_sensor_max_num = XSMALL_SERSOR_FAN_NUM;
		ubmc_ipmi->ubmc_board_info.volt_sensor_max_num = XSMALL_SERSOR_VOL_NUM;
		ubmc_ipmi->ubmc_board_info.temp_sensor_max_num = XSMALL_SERSOR_TEMP_NUM;
		ubmc_ipmi->ubmc_board_info.power_supply_sensor_max_num = XSMALL_SERSOR_PS_NUM;
		ubmc_ipmi->ubmc_board_info.init_handler = ubmc_ipmi_xsmall_init;
	}
	else if(device_type == SMALL || device_type == MIDDLE)
	{
		ubmc_ipmi->ubmc_board_info.sensor_config = ubmc_s_m_sensor_cfg;
		ubmc_ipmi->ubmc_board_info.sensor_max_num = S_M_SENSOR_MAX_NUM;
		ubmc_ipmi->ubmc_board_info.fan_sensor_max_num = S_M_SERSOR_FAN_NUM;
		ubmc_ipmi->ubmc_board_info.volt_sensor_max_num = S_M_SERSOR_VOL_NUM;
		ubmc_ipmi->ubmc_board_info.temp_sensor_max_num = S_M_SERSOR_TEMP_NUM;
		ubmc_ipmi->ubmc_board_info.power_supply_sensor_max_num = S_M_SERSOR_PS_NUM;
		ubmc_ipmi->ubmc_board_info.init_handler = ubmc_ipmi_small_init;
	}
	else if(device_type == LARGE)
	{
		ubmc_ipmi->ubmc_board_info.sensor_config = ubmc_large_sensor_cfg;
		ubmc_ipmi->ubmc_board_info.sensor_max_num = L_SENSOR_MAX_NUM;
		ubmc_ipmi->ubmc_board_info.fan_sensor_max_num = L_SERSOR_FAN_NUM;
		ubmc_ipmi->ubmc_board_info.volt_sensor_max_num = L_SERSOR_VOL_NUM;
		ubmc_ipmi->ubmc_board_info.temp_sensor_max_num = L_SERSOR_TEMP_NUM;
		ubmc_ipmi->ubmc_board_info.power_supply_sensor_max_num = L_SERSOR_PS_NUM;
		ubmc_ipmi->ubmc_board_info.init_handler = ubmc_ipmi_large_init;
	}
	else
	{
		return BMC_FALSE;
	}
	return BMC_SUCCESS;
}



/***
 * the signal handler ,to clear the sel when received the USR1 signal
 * @param signo
 */
void sig_handler(int signo)
{
	int ret = 0;
	if (signo == SIGUSR1)
    ret = ubmc_ipmi_sel_clear(&g_ubmc_ipmi.ubmc_ipmi_sel);
	if(ret < 0)
	{
		ubmc_error("\ndo clear fail\n");
	}
}


int main(int argc, char* argv[])
{
	int ret = BMC_FALSE;
	int res = -1,fd = -1;
	pthread_t sensor_thread_id;              // sensor thread
	pthread_t host_pwr_thread_id;
	pthread_t lan_thread_id;
	pthread_t check_status_thread_id;
#ifdef UBMC_IPMI_SENSOR_TEST
	create_test_file();
#endif
	//do_i2c_read_test(CPU_HOST_TEMP_I2C_DEV,CPU_TEMP_SLAVE_ADDR);
#if 0
	do_i2c_read_test("/dev/i2c-0",0x59);
	printf("\n\n");
	do_i2c_read_test("/dev/i2c-0",0x5a);
	printf("\n\n");
	do_i2c_read_test("/dev/i2c-0",0x2a);
#endif
#if 1
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
	{
		ubmc_error("\ncan't catch SIGUSR1\n");
	}

	if(init_board_configs(&g_ubmc_ipmi) < 0)		//get board info
	{
		return BMC_FALSE ;
	}

	if( ubmc_open_device(BMC_DEVICE_NAME,&fd) == BMC_FALSE )
	{
		return BMC_FALSE ;
	}

	/*for kernel*/
	if( ubmc_mmap_sdr_base(&g_ubmc_ipmi,fd,&ubmc_sensor_base) == BMC_FALSE )
	{
		goto exit;
	}
	/*for GUI*/
	if( !(ubmc_shm_base = ubmc_shm_state_create()) )
	{
		goto exit;
	}
	//ubmc_ipmi_init_runtime(&g_runtime);	//disable this funtion
	// read and simple verify the shared memory operation
	if( read_default_ubmc_sensor_states_from_shm(&g_ubmc_ipmi.ubmc_board_info) == BMC_FALSE ){
		ubmc_error("#### read_default_ubmc_sensor_states_from_shm Error ####\n");
		goto exit;
	}
	if(g_ubmc_ipmi.ubmc_board_info.init_handler != NULL)
	{
		res = g_ubmc_ipmi.ubmc_board_info.init_handler(NULL,1);
		if( res )
		{
			ubmc_error("ubmc board info init fail !\n");
			goto exit;
		}
	}

	res = ubmc_register_sensor(&g_ubmc_ipmi.ubmc_board_info,sensor_events);
	if( res )
	{
		ubmc_error("ubmc_register_sensor fail ,can not do gather_sensors_data_thread !\n");
		goto exit;
	}
	res = pthread_create(&sensor_thread_id, NULL, (void *)&gather_sensors_data_thread, &g_ubmc_ipmi);
	if ( res )
	{
		ubmc_error(" Create sensors data gather thread error !\n");
		goto exit;
	}
	//Just need to do power monitor thread when device type is X-small
	if(g_ubmc_ipmi.ubmc_board_info.device_type == XSMALL)
	{
		res = pthread_create(&host_pwr_thread_id, NULL, (void *)&ipmi_host_power_monitor_thread, NULL);
		if ( res )
		{
			ubmc_error(" Create ipmi_host_power_monitor_thread thread error !\n");
			goto exit;
		}
	}
	ubmc_ipmi_lan_init(&g_ubmc_ipmi.ubmc_ipmi_lan,fd);
	res = pthread_create(&lan_thread_id, NULL, (void *)&ubmc_ipmi_lan_thread, &g_ubmc_ipmi.ubmc_ipmi_lan);
	if ( res )
	{
		ubmc_error(" Create ubmc_ipmi_lan_thread  error !\n");
		goto exit;
	}
	ret = ubmc_ipmi_init_sel(&g_ubmc_ipmi.ubmc_ipmi_sel,fd);
	if(ret < 0)
	{
		ubmc_error(" ubmc_ipmi_init_sel fail ret = %d!\n");
		goto exit;
	}
	ubmc_ipmi_check_st_init(&g_ubmc_ipmi.ubmc_ipmi_status,fd,g_ubmc_ipmi.ubmc_board_info.device_type);
	res = pthread_create(&check_status_thread_id, NULL, (void *)&ubmc_ipmi_check_status_thread, &g_ubmc_ipmi.ubmc_ipmi_status);
	if ( res )
	{
		ubmc_error(" Create ubmc_ipmi_lan_thread  error !\n");
		goto exit;
	}

	ret = ubmc_ipmi_poll_gpio_events(&g_ubmc_ipmi.ubmc_ipmi_sel);
	if(ret < 0)
	{
		ubmc_error(" ubmc_ipmi_poll_gpio_events fail ret = %d!\n");
		goto exit;
	}
	ret = BMC_SUCCESS;
	pthread_join(sensor_thread_id,NULL);

exit:
	if( ubmc_shm_base && ubmc_shm_base->map_addr )
	{
		munmap(ubmc_shm_base->map_addr, sizeof(ubmc_shm_state));
		if(ubmc_shm_base->fd >=0 )
		{
			close(ubmc_shm_base->fd);
		}
		free(ubmc_shm_base);
	}
	if( ubmc_sensor_base != NULL )
	{
		munmap(ubmc_sensor_base, PAGE_SIZE);
	}
	if( fd != -1 )
		close(fd);
	if( g_ubmc_ipmi.ubmc_ipmi_sel.sel_msg_fd != NULL)
		fclose(g_ubmc_ipmi.ubmc_ipmi_sel.sel_msg_fd);
#endif
    return ret;
}
