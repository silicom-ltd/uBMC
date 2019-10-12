/*
 * peci_monitor.c
 *
 *  Created on: Oct 11, 2018
 *      Author: jason_zhang
 */

#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi.h"
#include "ubmc_ipmi_hst_pwr_monitor.h"
static inline bool is_file_exist(const char *file,int type)
{
	 if ( !access(file,type) )
	 {
		 return true;
	 }
	 return false;

}


static int ipmi_peci_val_is_invalid(const char *path)
{
	unsigned int value;
	int value_temp;
	if( ubmc_get_sys_file_content(path,&value) == -1 )
	{
		//it could be that the host system down,can not read value from temp1_input
		UBMC_IPMI_ERRLOG(" ubmc_get_file_content Error ! \n");
		return 1;
	}
	value_temp = (int)value;
	if(value_temp < 0)
	{
		//it could be that the host system down,can not read value from temp1_input
		UBMC_IPMI_NOTE(" the cpu temp value is invalid ,the peci kmod will be reloaded ! \n");
		return 1;
	}
	return 0;
}

static int ipmi_peci_kmod_check(char *path)
{
	//just to check the path of /sys/class/hwmon/hwmon2 whether it exist
	//and read the value of /temp1_input,whether the value is valid
	unsigned int value;
	if(is_file_exist(path,F_OK|R_OK))
	{
		if( ipmi_peci_val_is_invalid(path) )
		{
			//it could be that the host system down,can not read value from temp1_input
			//or the value is not valid
			return -1;
		}
		return 0;
	}
	else
	{
		UBMC_IPMI_ERRLOG(" %s is not exist \n",path);
		return -1;
	}

}

static int ipmi_peci_kmod_reload()
{
	int ret ;
	ret = system(PECI_KMOD_RELOAD_CMD);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ipmi_peci_kmod_reload fail \n");
		return -1;
	}
	return 0;
}

static int ipmi_get_host_power_state(struct gpio_s *gpio)
{
	int ret = 0;
	char c;
	char value = -1;
	if (gpio->fd == -1)
		ret = ubmc_ipmi_open_gpio_value_file(gpio);

	if (ret < 0)
	{
		UBMC_IPMI_ERRLOG("can't access gpio name:%s,gpio pin num:%d\n",gpio->gpio_name,gpio->gpio_num);
		return ret;
	}

	ret = lseek(gpio->fd, 0x00, SEEK_SET);

	if (ret == -1)
	{
		ret = -errno;
		UBMC_IPMI_ERRLOG("rewind of gpio %s failed %s\n",gpio->gpio_name,strerror(errno));
		return ret;
	}

	ret = read(gpio->fd, &c, 1);

	if (ret != 1)
	{
		ret = -errno;
		UBMC_IPMI_ERRLOG("read value of gpio %s failed %s\n",gpio->gpio_name,strerror(errno));
		return ret;
	}
	switch (c)
	{
		case '0':
			value = 0;
			break;
		case '1':
			value = 1;
			break;
		default:
			UBMC_IPMI_ERRLOG("value %x of gpio %s unknown\n",c,gpio->gpio_name);
			return -EINVAL;
	}

	return value;
}


static int ipmi_host_power_monitor_init(struct gpio_s *gpio)
{
	int gpio_fd;
	/*note :this seting should be same as the gpio of ubmc_ipmi_poll_gpio_events function setting*/
	if(ubmc_ipmi_init_gpio_s(gpio,HOST_POWER_NAME,TRI_FALLING) < 0)
		return -1;
	gpio_fd = ubmc_ipmi_open_gpio_value_file(gpio);
	if(gpio_fd < 0)
	{
		UBMC_IPMI_ERRLOG("open gpio value file fail :%d \n",gpio_fd);
		return -1;
	}
	return 0;
}

extern ubmc_sensor_event *sensor_events[13];
int ipmi_host_power_monitor_thread( void )
{
	struct gpio_s host_pwr_pin;

	struct timeval timeout = {0};
	int ret;
	char value = -1;
	ret = ipmi_host_power_monitor_init(&host_pwr_pin);
	ubmc_sensor_event* sensor_p;
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ipmi_host_power_monitor_init fail ,cat not do ipmi_host_power_monitor_thread\n");
		return -1;
	}
	while(1)
	{
		timeout.tv_sec = HOST_POWER_MONITOR_INTERVAL;
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
			value = ipmi_get_host_power_state(&host_pwr_pin);
			if(value < 0)
			{
				UBMC_IPMI_ERRLOG("ipmi_get_host_power_state fail  \n");
				break;
			}
			//printf("value is %d \n",value);
			if(value == 0)
			{
				//system down
				//don't do anything

			}
			else if(value == 1)
			{
				//system up
				sensor_p = ubmc_ipmi_find_sesor_by_name(sensor_events,"TEMP_HOST_CPU");
				if(sensor_p == NULL)
				{
					UBMC_IPMI_ERRLOG("ubmc_ipmi_find_sesor_by_name fail TEMP_HOST_CPU  \n");
					break;
				}
				if(ipmi_peci_kmod_check(sensor_p->sensor_value_path) != 0)
				{
					ret = ipmi_peci_kmod_reload();
					if(ret < 0)
					{
						UBMC_IPMI_ERRLOG("ipmi_peci_kmod_reload fail  \n");
					}
				}
			}

			break;
		default:
			//ubmc_debug(" Begin to gather sensors' data \n");
			break;
		}
    }
	UBMC_IPMI_ERRLOG("ipmi_host_power_monitor_thread have finshed  \n");
}
