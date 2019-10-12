#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
 #include <arpa/inet.h>
#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi.h"
#define FAN_SYS_PATH_MAX_NUM 128

#define FAN_MAXNUM 3
#define FAN_CHIP_SYS_PATH "/sys/class/hwmon/hwmon0/device"

int fan_freq[]={11, 14, 22, 29, 35, 44, 58, 88, 22500};
typedef int  (*fan_handle)(void *data,int *val);

typedef enum
{
	SLOW,
	FAST,
}fan_speed_state;

typedef struct fan_ctr
{
	char		fan_sys_path[FAN_SYS_PATH_MAX_NUM];
	char		fan_pwm_path[FAN_SYS_PATH_MAX_NUM];
	char		fan_pwm_mode_path[FAN_SYS_PATH_MAX_NUM];
	char		fan_pwm_range_path[FAN_SYS_PATH_MAX_NUM];
	char		fan_speed_path[FAN_SYS_PATH_MAX_NUM];
	char		fan_freq_path[FAN_SYS_PATH_MAX_NUM];
	char 		index;
	fan_handle			setpwm;
	fan_handle			getpwm;
	fan_handle			set_fan_speed;
	fan_handle			get_fan_speed;
	fan_handle			set_pwm_freq;
	fan_handle			get_pwm_freq;
	int			init_cfg;
	char        is_enable;
	int			fan_speed;
	int 		pwm_fd;
	int 		pwm_mode_fd;
	int 		speed_fd;
	int 		fan_pwm_range_fd;
	int 		pwm_freq_fd;

}ubmc_fan_ctr;

typedef struct fan_set_cfg
{
	long int interv ;
	long int temp_case ;
	unsigned char fan_pwm_max_range;
	unsigned char fan_pwm_min_range;
	unsigned int fan_pwm_freq;
	char mode;
	char prt;

}ubmc_fan_set_cfg;

static ubmc_fan_set_cfg fan_setting ={
								 .interv = 1,
								 .temp_case = 65,
								 .fan_pwm_max_range = 80,
								 .fan_pwm_min_range = 0,
								 .fan_pwm_freq = 22500,
								 .mode = 0,		//default is parameter mode
								 .prt = 0
								};

ubmc_fan_ctr ubmc_fans[FAN_MAXNUM];


int get_cpu_temp(unsigned int *cpu_temp);
int init_fan(ubmc_fan_ctr *ubmc_fans);
int config_fan_mode(ubmc_fan_ctr *ubmc_fans);

static void fan_speed_ctr_auto_thread( void* data )
{
	unsigned int cpu_temp = 0;
	unsigned int set_pwm = 0;
	unsigned int fan_speed = 0;
	unsigned char host_pw_st = 0;
	int i = 0;
	int ret = 0;
	while(1)
	{

		if(get_cpu_temp(&cpu_temp) == 0)
		{
			host_pw_st = 0;
			if(cpu_temp < 78)
			{
				set_pwm = 0;
			}
			else if(cpu_temp < 79)
			{
				set_pwm = 50;
			}
			else if(cpu_temp < 80)
			{
				set_pwm = 76;
			}
			else if(cpu_temp < 81)
			{
				set_pwm = 103;
			}
			else if(cpu_temp < 82)
			{
				set_pwm = 128;
			}
			else if(cpu_temp < 83)
			{
				set_pwm = 154;
			}
			else if(cpu_temp < 84)
			{
				set_pwm = 180;
			}
			else if(cpu_temp < 85)
			{
				set_pwm = 205;
			}
			else if(cpu_temp < 86)
			{
				set_pwm = 230;
			}
			else
			{
				set_pwm = 255;
				ret = system("/usr/bin/ubmc_sys_ctrl -f");
				host_pw_st = 1;
				if(ret < 0)
				{
					ubmc_error("Power off host fail \n");
				}
			}
			for(i = 0;i < FAN_MAXNUM ;i ++)
			{
				ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
			}
			if(fan_setting.prt) printf("Current temperature is %d Cel\n \n",cpu_temp);
			if(fan_setting.prt) printf("Setting pwm to %d \n",set_pwm);
			for(i = 0;i < FAN_MAXNUM ;i ++)
			{
				ubmc_fans[i].get_fan_speed(&ubmc_fans[i],&fan_speed);
			}
			if(host_pw_st != 0)
			{
				sleep(30);
				set_pwm = 0;
				for(i = 0;i < FAN_MAXNUM ;i ++)
				{
					ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
				}
				if(fan_setting.prt) printf("Setting pwm to %d \n",set_pwm);
			}
			sleep(fan_setting.interv);
		}
		else
		{
			//
		}
	}
}
static void fan_speed_ctr_with_temp_thread( void* data )
{
	struct timeval timeout = {0};
	int ret ;
	char i;
	char is_start = 0;
	char keep = 0;
	unsigned int cpu_temp;
	unsigned int set_pwm;
	unsigned int faster;
	unsigned int slower;
	fan_speed_state cur_state;
	unsigned int fan_speed;
	//first loop just to matain the fan speed as minimum speed
	set_pwm = fan_setting.fan_pwm_min_range; 	//init the set_pwm
	while(1)
	{
		if(get_cpu_temp(&cpu_temp) == 0)
		{
			if(cpu_temp < fan_setting.temp_case - 5)
			{
				if(set_pwm == fan_setting.fan_pwm_max_range)
				{
					is_start = 1;
					set_pwm = (fan_setting.fan_pwm_min_range + fan_setting.fan_pwm_max_range)/2; //init set_pwm for next loop to use
					for(i = 0;i < FAN_MAXNUM ;i ++)
					{
						ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
					}
					for(i = 0;i < FAN_MAXNUM ;i ++)
					{
						ubmc_fans[i].get_fan_speed(&ubmc_fans[i],&fan_speed);
					}
					if(fan_setting.prt) printf("current pwm is %d \n \n",set_pwm);
					cur_state = SLOW;
					sleep(fan_setting.interv);
					break;
				}
				for(i = 0;i < FAN_MAXNUM ;i ++)
				{
					ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
				}
			}
			else
			{
				set_pwm = fan_setting.fan_pwm_max_range;
				cur_state = FAST;
				for(i = 0;i < FAN_MAXNUM;i ++)
				{
					ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
				}
			}
			for(i = 0;i < FAN_MAXNUM ;i ++)
			{
				ubmc_fans[i].get_fan_speed(&ubmc_fans[i],&fan_speed);
			}
			if(fan_setting.prt) printf("current pwm is %d \n \n",set_pwm);
		}
		else
		{
			if(fan_setting.prt) printf("Can not get the CPU temp \n");
			//if can not get cpu temp ,what speed we should set?
		}
		sleep(fan_setting.interv);
	}
	//main fan control loop when the cpu_temp more than value of expectation
	while(is_start)
	{
		timeout.tv_sec = fan_setting.interv;
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
			if(get_cpu_temp(&cpu_temp) == 0)
			{
				if(cpu_temp < fan_setting.temp_case - 5)
				{
					if(cur_state == SLOW)
					{
						;
					}
					else if(cur_state == FAST)
					{
						for(i = 0;i < FAN_MAXNUM ;i ++)
						{
							set_pwm = (set_pwm + fan_setting.fan_pwm_max_range)/2;
							ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
						}

					}
					cur_state = SLOW;
				}
				else
				{
					if(cur_state == SLOW)
					{
						set_pwm = (set_pwm + fan_setting.fan_pwm_max_range)/2;
					}
					else if(cur_state == FAST)
					{
						set_pwm = (set_pwm + fan_setting.fan_pwm_max_range)/2;
					}
					cur_state = FAST;
					for(i = 0;i < FAN_MAXNUM ;i ++)
					{
						ubmc_fans[i].setpwm(&ubmc_fans[i],&set_pwm);
					}

				}
				for(i = 0;i < FAN_MAXNUM ;i ++)
				{
					ubmc_fans[i].get_fan_speed(&ubmc_fans[i],&fan_speed);
				}
				if(fan_setting.prt) printf("current pwm is %d \n\n",set_pwm);
			}
			//update_all_sensor_states(sensor_num);
			break;
			default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
    }
}

void usage(char * prog)
{
	fprintf(stderr,
			"usage: %s [-t <interval>] [-T <temperature>] [-f <pwm frequence>] [-V <maximum pwm value>] [-v <minimum pwm valu>]\n \
            -h : help \n \
            -t : set interval,default is 1 second \n \
			-f : set pwm frequence,the value only can be one of [11, 14, 22, 29, 35, 44, 58, 88, 22500],default is 22500 \n \
            -T : set T-case (default is 65 Cel) \n \
			-V : set maximum pwm value from 0~255 (default is 80) \n \
			-v : set minimum pwm value from 0~255 (default is 0) \n \
			-D : display the debug info on the output \n \
			-A : auto mode ,using the default temp-speed table \n \
			For example: if you want to set interval as 1 sec and T_case as 45 Cel \n \
			%s -t 1 -T 45 -v 0 -V 255 -f 22500 -D \n",\
			prog,prog);
}


int main(int argc, char *argv[])
{

	char * prog;
	int opt;
	int ret;
	/*
	 *record if there is arguments or not
	 */
	int arg_flag = 0;
	pthread_t fan_control_thread_id;
	prog = argv[0];
	while ((opt = getopt(argc, argv, "t:T:V:v:f:ADh?")) != -1)
	{
		arg_flag++;
		switch (opt)
		{
		case 't':
			if(optarg != NULL)
			{
				fan_setting.interv = atoi(optarg);
			}
			//printf("t case optarg is %s interv is %d \n",optarg,fan_setting.interv);
			break;
		case 'T':
			if(optarg != NULL)
			{
				fan_setting.temp_case = atoi(optarg);
			}
			//printf("T case optarg is %s temp_case: %d \n",optarg,fan_setting.temp_case);
			break;
		case 'V':
			if(optarg != NULL)
			{
				fan_setting.fan_pwm_max_range = atoi(optarg);
			}
			break;
		case 'v':
			if(optarg != NULL)
			{
				fan_setting.fan_pwm_min_range = atoi(optarg);
			}
			break;
		case 'f':
			if(optarg != NULL)
			{
				fan_setting.fan_pwm_freq = atoi(optarg);
				//printf("fan_setting.fan_pwm_freq is %d \n",fan_setting.fan_pwm_freq);
			}
			break;
		case 'A':
				fan_setting.mode = 1;	//1 means auto mode .0 means parameter mode.
			break;
		case 'D':
				fan_setting.prt = 1;
			break;
		case '?':
		case 'h':
		default:
			usage(prog);
			arg_flag = 0;
			break;
		}

	}
	/*
	 *if there is arguments ,output usage
	 */
	if (fan_setting.fan_pwm_max_range <= fan_setting.fan_pwm_min_range)
	{
		ubmc_error("Please set the valid value ,make sure maximum pwm value > minimum pwm value \n");
		exit(0);
	}
	if (!arg_flag)
	{
		usage(prog);
		exit(0);
	}
	ret = init_fan(ubmc_fans);

	if(ret < 0)
	{
		ubmc_error("init_fan fail \n");
		exit(1);
	}
	ret = config_fan_mode(ubmc_fans);
	if(ret < 0)
	{
		ubmc_error("config_fan_mode fail \n");
		exit(1);
	}
	if(fan_setting.mode == 0)
	{
		ret = pthread_create(&fan_control_thread_id, NULL, (void *)&fan_speed_ctr_with_temp_thread, NULL);
	}
	else if(fan_setting.mode == 1)
	{
		ret = pthread_create(&fan_control_thread_id, NULL, (void *)&fan_speed_ctr_auto_thread, NULL);
	}
	while(ret == 0)
	{
		sleep(1);
	}

}




#define CPU_TEMP_SLAVE_ADDR 0X48	//sync fromubmc_ipmi.c
#define CPU_TEMP_REG 0x40
#define I2C_DEV "/dev/i2c-3"
int get_cpu_temp(unsigned int *cpu_temp)
{
	int ret;
	unsigned int temp;
	int fd;
	fd = open_i2c_dev(I2C_DEV,CPU_TEMP_SLAVE_ADDR);
	if(fd < 0)
	{
		ubmc_error("open_i2c_dev fail");
		return -1;
	}
	ret = i2c_smbus_read_word_data(fd,CPU_TEMP_REG);
	if(ret < 0)
	{
		ubmc_error("i2c_smbus_read_word_data fail:%s \n",strerror(errno));
		return -1;
	}
	ret = htons(ret);
	ret = ret & 0x0ff;
	//printf("temp1 is 0x%x \n",ret);
	//temp = ret*1000;
	//printf("temp2 is %d \n",temp);
	*cpu_temp = ret;
	if(fan_setting.prt) printf("current CPU temp is %d \n",ret);
	close(fd);
	return BMC_SUCCESS;
}


#ifdef TEST
#define CPU_TEMP_FILE_PATH "/var/log/temperature.txt"
int get_cpu_temp(int *var)
{
	int fd;
	char buf[8] = {0};
	fd = open(CPU_TEMP_FILE_PATH,O_RDWR);
	if(fd < 0)
	{
		printf("cat read cpu temp \n");
		return -1;
	}
	read(fd,buf,8);
	*var = atoi(buf);
	printf("get_cpu_temp :%d \n",*var);
	return 0;
}

#endif
int fan_set_pwm(void *data,int *val)
{
	char buf[8] = {0};
	int ret;
	ubmc_fan_ctr *ubmc_fans;
	ubmc_fans = (ubmc_fan_ctr *)data;
	if(ubmc_fans->is_enable)
	{
		sprintf(buf,"%d",*val);
		ret = write(ubmc_fans->pwm_fd,buf,10);	//set the fan speed by seting pwm
		if(ret < 0)
			return -1;
	}
	return 0;
}

int  fan_get_pwm(void *data,int *val)
{
	char buf[8] = {0};
	int ret;
	ubmc_fan_ctr *ubmc_fans;
	ubmc_fans = (ubmc_fan_ctr *)data;
	ret = read(ubmc_fans->pwm_fd,buf,10);	//set the fan speed by seting pwm
	if(ret < 0)
		return -1;
	*val =atoi(buf);
	return 0;
}

int  fan_get_speed(void *data,int *val)
{
	char buf[8] = {0};
	int ret;
	ubmc_fan_ctr *ubmc_fans;
	ubmc_fans = (ubmc_fan_ctr *)data;
	lseek(ubmc_fans->speed_fd, 0x00, SEEK_SET);
	ret = read(ubmc_fans->speed_fd,buf,10);	//set the fan speed by seting pwm
	if(ret < 0)
	{
		ubmc_error("can not read fan speed \n");
		return -1;
	}
	//*val =strtol(buf);
	if(fan_setting.prt) printf("fan_%d speed is %s ",ubmc_fans->index,buf);
	return 0;
}

int  fan_get_pwm_freq(void *data,int *val)
{
	char buf[8] = {0};
	int ret;
	ubmc_fan_ctr *ubmc_fans;
	ubmc_fans = (ubmc_fan_ctr *)data;
	lseek(ubmc_fans->pwm_freq_fd, 0x00, SEEK_SET);
	ret = read(ubmc_fans->pwm_freq_fd,buf,10);	//set the fan speed by seting pwm
	if(ret < 0)
	{
		ubmc_error("can not read fan pwm frequence \n");
		return -1;
	}
	//*val =strtol(buf);
	if(fan_setting.prt) printf("fan_%d pwm frequence is %s \n",ubmc_fans->index,buf);
	return 0;
}

int  fan_set_pwm_freq(void *data,int *val)
{
	char buf[8] = {0};
	int ret;
	ubmc_fan_ctr *ubmc_fans;
	ubmc_fans = (ubmc_fan_ctr *)data;
	if(ubmc_fans->is_enable)
	{
		sprintf(buf,"%d",*val);
		ret = write(ubmc_fans->pwm_freq_fd,buf,10);	//set the fan speed by seting pwm
		if(ret < 0)
			return -1;
	}
	return 0;
}

int init_fan(ubmc_fan_ctr *ubmc_fans)
{
	int i=0;
	int fd;
	int ret;
	char buf[FAN_SYS_PATH_MAX_NUM] = {0};
	for(i = 0;i < FAN_MAXNUM; i ++)
	{
		ubmc_fans[i].is_enable = 1;

		ubmc_fans[i].index = i+1;		//the index start from 1
		ret = access(FAN_CHIP_SYS_PATH,F_OK|R_OK);
		if(ret < 0)
		{
			ubmc_error("can not access %s err:%s\n",FAN_CHIP_SYS_PATH,strerror(errno));
			return -1;
		}
		strcpy(ubmc_fans[i].fan_sys_path,FAN_CHIP_SYS_PATH);
		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%s/pwm%d",FAN_CHIP_SYS_PATH,ubmc_fans[i].index);
		strcpy(ubmc_fans[i].fan_pwm_path,buf);
		//printf("pwm path:%s \n",buf);
		fd = open(ubmc_fans[i].fan_pwm_path,O_RDWR);
		if(fd < 0)
		{
			ubmc_error("open %s fail. err:%s\n",ubmc_fans[i].fan_pwm_path,strerror(errno));
			return -1;
		}
		ubmc_fans[i].pwm_fd = fd;

		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%s/pwm%d_enable",FAN_CHIP_SYS_PATH,ubmc_fans[i].index);
		strcpy(ubmc_fans[i].fan_pwm_mode_path,buf);
		fd = open(ubmc_fans[i].fan_pwm_mode_path,O_RDWR);
		if(fd < 0)
		{
			ubmc_error("open %s fail. err:%s\n",ubmc_fans[i].fan_pwm_mode_path,strerror(errno));
			return -1;
		}
		ubmc_fans[i].pwm_mode_fd = fd; 	//set pwm mode as manual ,we can read and write pwm directly.
		//printf("fan_pwm_mode_path : %s \n",ubmc_fans[i].fan_pwm_mode_path);

		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%s/pwm%d_auto_point2_pwm",FAN_CHIP_SYS_PATH,ubmc_fans[i].index);
		strcpy(ubmc_fans[i].fan_pwm_range_path,buf);
		fd = open(ubmc_fans[i].fan_pwm_range_path,O_RDWR);
		if(fd < 0)
		{
			ubmc_error("open %s fail. err:%s\n",ubmc_fans[i].fan_pwm_range_path,strerror(errno));
			return -1;
		}
		ubmc_fans[i].fan_pwm_range_fd = fd;
		//printf("fan_pwm_range : %s \n",ubmc_fans[i].fan_pwm_range_path);

		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%s/fan%d_input",FAN_CHIP_SYS_PATH,ubmc_fans[i].index);
		strcpy(ubmc_fans[i].fan_speed_path,buf);
		fd = open(ubmc_fans[i].fan_speed_path,O_RDONLY);
		if(fd < 0)
		{
			ubmc_error("open %s fail. err:%s\n",ubmc_fans[i].fan_speed_path,strerror(errno));
			return -1;
		}
		ubmc_fans[i].speed_fd = fd;

		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%s/pwm%d_freq",FAN_CHIP_SYS_PATH,ubmc_fans[i].index);
		strcpy(ubmc_fans[i].fan_freq_path,buf);
		fd = open(ubmc_fans[i].fan_freq_path,O_RDWR);
		if(fd < 0)
		{
			ubmc_error("open %s fail. err:%s\n",ubmc_fans[i].fan_freq_path,strerror(errno));
			return -1;
		}
		ubmc_fans[i].pwm_freq_fd = fd;


		//register the fan handle
		ubmc_fans[i].setpwm = fan_set_pwm;
		ubmc_fans[i].getpwm = fan_get_pwm;
		ubmc_fans[i].get_fan_speed = fan_get_speed;
		ubmc_fans[i].set_pwm_freq = fan_set_pwm_freq;
		ubmc_fans[i].get_pwm_freq = fan_get_pwm_freq;
	}
	return 0;
}

int config_fan_mode(ubmc_fan_ctr *ubmc_fans)
{
	int ret;
	int i;
	char buf[10]={0};
	for(i = 0;i < FAN_MAXNUM ; i ++)
	{
		if(ubmc_fans[i].is_enable)
		{
			ret = write(ubmc_fans[i].pwm_mode_fd,"1",10);	//set to manual mode
			if(ret < 0)
				return -1;
			ret = write(ubmc_fans[i].fan_pwm_range_fd,"255",10);	//set to minimum fan speed,after this fan speed will up to fan_pwm_range
			if(ret < 0)
				return -1;
			ret = write(ubmc_fans[i].pwm_fd,"0",10);	//set the fan init-speed to 0
			if(ret < 0)
				return -1;
			sprintf(buf,"%d",fan_setting.fan_pwm_freq);
			ret = write(ubmc_fans[i].pwm_freq_fd,buf,10);	//set the fan init-speed to 0
			if(ret < 0)
				return -1;
		}
	}
	return 0;
}


int delete_fan(ubmc_fan_ctr *ubmc_fans)
{
	int i;
	for(i = 0;i < FAN_MAXNUM; i ++)
	{
		close(ubmc_fans[i].pwm_mode_fd);
		close(ubmc_fans[i].pwm_fd);
		close(ubmc_fans[i].fan_pwm_range_fd);
		close(ubmc_fans[i].speed_fd);
	}
}

