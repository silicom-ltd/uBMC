/*
 * ubmc_ipmi_sel.c
 *
 *  Created on: Jul 27, 2018
 *      Author: jason_zhang
 */
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ubmc_ipmi_sel.h"
#include "ubmc_ipmi_sel_sensor.h"
#include "ubmc_common.h"
#include "silc_common/silc_gpio.h"


bool ubmc_is_file_exist(const char *file){
	 if ( !access(file,F_OK|R_OK) ){
		 return true;
	 }
	 return false;

}
/**
 * pbase->sensor.mtol and pbase->sensor.bacc is used to calculate the sensor value in 'ipmitool sdr'
 * Update this value to show more accuracy in 'ipmitool sdr' command .
 * */
int update_calculation_factor_value(ubmc_sensor_s *pbase,uint16_t mtol,uint32_t bacc){
	if( !pbase ){
		return BMC_FALSE;
	}
	if( pbase->sensor.mtol != mtol ){
		pbase->sensor.mtol = mtol;
	}
	if( pbase->sensor.bacc != bacc ){
		pbase->sensor.bacc = bacc;
	}
	return BMC_SUCCESS;
}

static uint8_t* ubmc_trim(uint8_t *str)
{
	int len = 0;
    if (!str)
    {
        return NULL;
    }
    while(IS_SPACE(*str)) str++;
    len = strlen(str);
    if (!len)
    {
        return str;
    }
    char *end = str + len - 1;
    while(IS_SPACE(*end)) end--;
    *(++end) = '\0';

    return str;
}

int i2c_smbus_access(int file, char read_write, unsigned char  command,
		       int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	unsigned int err;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	err = ioctl(file, I2C_SMBUS, &args);
	if (err == -1)
		err = -errno;
	return err;
}

int i2c_smbus_write_block_data(int file, unsigned char command, unsigned char length,
				 const unsigned char  *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
				I2C_SMBUS_BLOCK_DATA, &data);
}

unsigned int i2c_smbus_write_byte_data(int file, unsigned char command, unsigned char value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
				I2C_SMBUS_BYTE_DATA, &data);
}

unsigned int i2c_smbus_write_word_data(int file, unsigned char command, unsigned short value)
{
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
				I2C_SMBUS_WORD_DATA, &data);
}

unsigned int i2c_smbus_read_block_data(int file, unsigned char command, unsigned char *values)
{
	union i2c_smbus_data data;
	 int i, err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			       I2C_SMBUS_BLOCK_DATA, &data);
	if (err < 0)
		return err;

	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
	return data.block[0];
}

unsigned int i2c_smbus_read_word_data(int file, unsigned char command)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			       I2C_SMBUS_WORD_DATA, &data);
	if (err < 0)
	{
		close(file);
		return err;
	}
	return 0x0FFFF & data.word;
}

unsigned int i2c_smbus_read_byte_data(int file, unsigned char command)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			       I2C_SMBUS_BYTE_DATA, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

int open_i2c_dev(const char *dev,unsigned char slave_adr)
{

	int iic_fd;
	//open device
	iic_fd = open(dev, O_RDWR);
	if (iic_fd < 0)
	{
		UBMC_IPMI_ERRLOG("ERROR: Unable to open %s for i2cBus access: %d\n",dev,iic_fd);
		return -1;
	}
	//set device address
	if (ioctl(iic_fd,I2C_SLAVE, slave_adr) < 0)
	{
		UBMC_IPMI_ERRLOG("ERROR: Unable to set I2C slave address 0x%x\n",slave_adr);
		close(iic_fd);
		return -1;
	}
	return iic_fd;
}
int ubmc_pmbus_get_linear_value(unsigned short in_value,float* out_value)
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
	N = ((~N) & 0x01f)+1;
	exp = 1 << N;
	*out_value = (float)Y/exp;
	return 0;

}
int ubmc_smbus_write_value(const char *dev,unsigned char device_adr,unsigned char reg_adr,unsigned short value,char mode)
{
	int ret;
	unsigned int temp;
	int fd;
	fd = open_i2c_dev(dev,device_adr);
	if(fd < 0)
	{
		UBMC_IPMI_ERRLOG("open_i2c_dev fail");
		return -1;
	}
	if(mode == 0)
	{
		ret = i2c_smbus_write_byte_data(fd,reg_adr,value);
	}
	else if(mode == 1)
	{
		ret = i2c_smbus_write_word_data(fd,reg_adr,value);
	}
	else
	{
		UBMC_IPMI_ERRLOG("ubmc_smbus_write_value fail:Read Mode is invalid \n");
		return -1;
	}
	close(fd);
	return BMC_SUCCESS;
}

int ubmc_smbus_read_value(const char *dev,unsigned char device_adr,unsigned char reg_adr,uint16_t *p_value,char mode)
{
	int ret;
	unsigned int temp;
	int fd;
	fd = open_i2c_dev(dev,device_adr);
	if(fd < 0)
	{
		UBMC_IPMI_ERRLOG("open_i2c_dev fail");
		return -1;
	}
	if(mode == 0)
	{
		ret = i2c_smbus_read_byte_data(fd,reg_adr);
	}
	else if(mode == 1)
	{
		ret = i2c_smbus_read_word_data(fd,reg_adr);
	}
	else
	{
		UBMC_IPMI_ERRLOG("ubmc_smbus_read_value fail:Read Mode is invalid \n");
		close(fd);
		return -1;
	}
	if(ret < 0)
	{
//		UBMC_IPMI_ERRLOG("i2c_smbus_read_word_data fail:%s \n",strerror(errno));
		close(fd);
		return -1;
	}
	//ret = htons(ret);
	*p_value = (uint16_t)ret;
	//printf("device_adr:%x reg_adr: %x p_value is %d 0x%x ret is 0x%x\n",device_adr,reg_adr,*p_value,*p_value,ret);

	close(fd);
	return BMC_SUCCESS;
}

#define CPU_TEMP_SLAVE_ADDR 0X48
#define CPU_TEMP_REG 0x40
int ubmc_get_dev_file_content(const char *dev,uint32_t *p_value)
{
	int ret;
	unsigned int temp;
	int fd;
	fd = open_i2c_dev(dev,CPU_TEMP_SLAVE_ADDR);
	if(fd < 0)
	{
		UBMC_IPMI_ERRLOG("open_i2c_dev fail");
		return -1;
	}
	ret = i2c_smbus_read_word_data(fd,CPU_TEMP_REG);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("i2c_smbus_read_word_data fail:%s \n",strerror(errno));
		return -1;
	}
	ret = htons(ret);
	ret = ret & 0x0ff;
	//printf("temp1 is 0x%x \n",ret);
	temp = ret*1000;
	//printf("temp2 is %d \n",temp);
	*p_value = temp;
	//printf("p_value is %d 0x%x ret is 0x%x\n",*p_value,*p_value,ret);
	close(fd);
	return BMC_SUCCESS;
}


int ubmc_get_sys_file_content(const char *p_path,uint32_t *p_value)
{
	int ret = BMC_FALSE ;
	int rc;
    FILE *fp = NULL;
    uint8_t buffer[128] = { 0 };
    uint8_t cmd[256] = { 0 };
    uint8_t *tmp = NULL;

	if( !p_path || !p_value )
	{
		return ret;
	}
	sprintf(cmd,"cat %s 2>/dev/null ",p_path);
	fp=popen(cmd,"r");
	if( fp == NULL )
	{
		UBMC_IPMI_ERRLOG("popen Error! :%s\n",strerror(errno));
		return ret;
	}
	memset(buffer,0,128);
    tmp = fgets(buffer,sizeof(buffer),fp);
	// the result of the shell command is null
	if( !tmp )
	{
		ret = BMC_SUCCESS;
		goto exit;
	}
	*p_value = atoi(ubmc_trim(buffer));

	ret = BMC_SUCCESS ;
exit:
    rc = pclose(fp);
    //in below check the cmd is done successfully
    if(rc == -1)
    {
    	//do command fail
    	return BMC_FALSE;
    }
    if (WIFEXITED(rc))
    {
    	//printf("subprocess exited, exit code: %d\n", WEXITSTATUS(rc));
    	if (0 == WEXITSTATUS(rc))
    	{
    		//printf("command succeed\n");
    	}
    	else
    	{
    		if(127 == WEXITSTATUS(rc))
    		{
    			//can not find this cmd
    			return BMC_FALSE;
    		}
    		else
    		{
    			//printf("command:%s failed: %s\n",cmd,strerror(WEXITSTATUS(rc)));
    			return BMC_FALSE;
    		}
    	}
    }
    else
    {
    	//printf("subprocess exit failed\n");
    	return BMC_FALSE;
    }
	return ret ;
}

static inline bool is_gpio_file_exist(const char *file,int type)
{
	 if ( !access(file,type) )
	 {
		 return true;
	 }
	 return false;
}

static int ubmc_ipmi_mutex_lock(pthread_mutex_t *mutex)
{
    if (pthread_mutex_lock(mutex) != 0)
    {
            UBMC_IPMI_ERRLOG("lock error is %s !\n",strerror(errno));
            return -1;
    }
    return 1;
}

static int ubmc_ipmi_mutex_unlock(pthread_mutex_t *mutex)
{
    if (pthread_mutex_unlock(mutex) != 0)
    {
            UBMC_IPMI_ERRLOG("lock error is %s !\n",strerror(errno));
            return -1;
    }
    return 1;
}

#define CMD_BUF_SIZE 256
#define DIR_BUF_SIZE 128

/**
 *create gpio file on /sys/class/gpio directory
 * @param gpio_bank	the number of gpio bank
 * @param gpio_pin the number of gpio pin
 * @return success :gpio pin number fail -1
 */
static int ubmc_ipmi_create_gpio_file(int num)
{
	bool res;
	int ret = 0;
	int i = 0;
	int gpio_num;
	char cmd_buf[CMD_BUF_SIZE] = {0};
	char dir_name_buf[DIR_BUF_SIZE] = {0};
	res = is_gpio_file_exist(SYS_GPIO_PATH,F_OK|W_OK|R_OK);
	if(!res)
	{
		UBMC_IPMI_ERRLOG("can not find /sys/class/gpio dir \n");
		return -1;
	}
	res = is_gpio_file_exist(SYS_GPIO_EXPORT_PATH,F_OK|W_OK|R_OK);
	if(!res)
	{
		UBMC_IPMI_ERRLOG("can not find /sys/class/gpio/export dir \n");
		return -1;
	}
	//init the gpio array
	gpio_num = num;
	sprintf(dir_name_buf,"/sys/class/gpio/gpio%d",gpio_num);
	res = is_gpio_file_exist(dir_name_buf,F_OK|W_OK|R_OK);
	if(res)
	{
		//UBMC_IPMI_DEBUG_INFO("this file %s has been exist\n",dir_name_buf);
		return gpio_num;
	}
	sprintf(cmd_buf,"echo %d > /sys/class/gpio/export",gpio_num);
	ret = system(cmd_buf);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("do cmd %s fail\n",cmd_buf);
		return -1;
	}
	res = is_gpio_file_exist(dir_name_buf,F_OK|W_OK|R_OK);
	if(!res)
	{
		UBMC_IPMI_ERRLOG("create %s file fail \n",dir_name_buf);
		return -1;
	}
	return gpio_num;
}
#define SEL_MSG_SIZE 30
// the number of sel member is 12 , add one as checksum
#define SEL_RECORD_MEMBER_NUM 13

/**
 * set the member of sel_event_record by *val
 * @param val
 * @param event
 * @return record id
 */
int ubmc_ipmi_parse_val_to_msg(long unsigned int *val,struct sel_event_record* event)
{
	unsigned int checksum;
	unsigned short int record_id;
	event->record_id = (unsigned short int)val[0];
	event->record_type = (unsigned char)val[1];
	event->sel_type.standard_type.timestamp = val[2];
	event->sel_type.standard_type.gen_id = (unsigned short int)val[3];
	event->sel_type.standard_type.evm_rev = (unsigned char)val[4];
	event->sel_type.standard_type.sensor_type = (unsigned char)val[5];
	event->sel_type.standard_type.sensor_num = (unsigned char)val[6];
	event->sel_type.standard_type.event_type = (unsigned char)val[7];
	event->sel_type.standard_type.event_dir = (unsigned char)val[8];
	event->sel_type.standard_type.event_data[0] = (unsigned char)val[9];
	event->sel_type.standard_type.event_data[1] = (unsigned char)val[10];
	event->sel_type.standard_type.event_data[2] = (unsigned char)val[11];
	checksum = (unsigned char)val[12];
	record_id = event->record_id;
	//printf("record id is %d \n",event->record_id);
	return record_id ;
}
/*
void prt_event(char* name,struct sel_event_record* event)
{
	printf("%s:record_id is 0x%x\n",name,htons(event->record_id));
	printf("%s:record_type is 0lx%x\n",name,event->record_type);
	printf("%s:timestamp is 0x%lx\n",name,htonl(event->sel_type.standard_type.timestamp));
	printf("%s:gen_id is 0x%lx\n",name,event->sel_type.standard_type.gen_id);
	printf("%s:sensor type is 0x%lx\n",name,event->sel_type.standard_type.sensor_type);
}*/

/***
 * change msg endian
 * @param event the instance of sel_event_record point
 * @param type 0 means big-endian,1 means little-endian
 * @return 1 when success,-1 when fail
 */
int change_msg_endian(struct sel_event_record* event,unsigned char type)
{
	if(event == NULL || type > 1)
	{
		UBMC_IPMI_ERRLOG("change_msg_endian fail ");
		return -1;
	}
	else if(type == 0 )		//change to big-endian
	{
		event->record_id = htons(event->record_id);
		event->sel_type.standard_type.gen_id = htons(event->sel_type.standard_type.gen_id);
		event->sel_type.standard_type.timestamp = htonl(event->sel_type.standard_type.timestamp);
	}
	else if(type == 1)
	{
		event->record_id = ntohs(event->record_id);
		event->sel_type.standard_type.gen_id = ntohs(event->sel_type.standard_type.gen_id);
		event->sel_type.standard_type.timestamp = ntohl(event->sel_type.standard_type.timestamp);
	}
	return 1;
}

/***
 * parse the record msg from file
 * @param s the source msg from record
 * @param event which used for
 * @return nonzero when success,-1 when fail
 */
int ubmc_ipmi_parse_sel_msg(char *s , struct sel_event_record* event,char endian)
{
	char *endptr = NULL;
	char *tmp;
	struct sel_event_record* pevt;
	int base = 0,cnt = 0 ,record_id = 0;
	long unsigned int ret;
	if(s == NULL || event == NULL)
	{
		UBMC_IPMI_ERRLOG("record msg is NULL");
		return -1;
	}
	//event_var = (long unsigned int*)malloc(sizeof(long unsigned int) * SEL_RECORD_MEMBER_NUM);
	pevt = (struct sel_event_record *)malloc(sizeof(struct sel_event_record));
	if(pevt == NULL)
	{
		UBMC_IPMI_ERRLOG("malloc fail \n");
		return -1;
	}
	memcpy(pevt,s,sizeof(struct sel_event_record));
	ret = change_msg_endian(pevt,endian);
	memcpy(event,pevt,sizeof(struct sel_event_record));
//	UBMC_IPMI_DEBUG_INFO("event->record_id NO HTONS is 0x%lx \n",pevt->record_id);
	record_id = event->record_id ;
	//prt_event("parse",event);
	free(pevt);
	return record_id;
}


/***
 * when the record id larger than maxnum we set,the funtion will be called
 * @param from
 * @param to
 * @param endian
 * @return 1 when success,-1 when fail
 */
int change_sel_record_id(char *from,struct sel_event_record *to,char endian)
{
	unsigned short int record_id = 0;
	record_id = ubmc_ipmi_parse_sel_msg(from,to,endian);
//	UBMC_IPMI_DEBUG_INFO("record_id %d\n",record_id);
	//to keep the record id less than SEL_ENTRY_MAXNUM
	to->record_id = to->record_id - 1;
	if(to->record_id >= SEL_ENTRY_MAXNUM - 1)
	{
		//UBMC_IPMI_DEBUG_INFO("change_sel_record_id 0x%x \n",to->record_id);
		return 0;
	}
	return 1;

}

/***
 *add a new record msg to record file
 * @param fp the file point of record file
 * @param event
 * @return 1 when success,-1 when fail
 */
int ubmc_ipmi_add_msg(FILE* fp,struct sel_event_record* event)
{
	struct sel_event_record msg;
	int ret = 0;
	memcpy(&msg,event,sizeof(struct sel_event_record));
	ret = change_msg_endian(&msg,0);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("change_msg_endian fail");
		return -1;
	}
	ret = fwrite(&msg,sizeof(char),sizeof(struct sel_event_record)/sizeof(char),fp);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("write msg fail ret %d",ret);
		return -1;
	}
	fflush(fp);
	return 1;
}

/***
 * if the number of record more than SEL_ENTRY_MAXNUM,delete the oldest entry,
 * add the new entry.
 * @param pfile
 * @param event	:the new record
 * @return nonzero when success,-1 when fail
 */
int upgrade_sel_record(FILE* pfile)
{
	unsigned char is_last = 1;
	struct sel_event_record to;
	char ret = 0;
	char from[SEL_MSG_SIZE] = {0};
    long int record_ofset =0,last_record_p = 0,cur_record_p = 0,next_record_p = 0;
    unsigned int cnt = 0 ,checksum =0xffff;
	fseek(pfile,0,SEEK_SET);
	last_record_p = ftell(pfile);
	ret = fread(from,sizeof(struct sel_event_record),1,pfile);
	//UBMC_IPMI_DEBUG_INFO("fgets 1 :%s \n",from);
	record_ofset = ftell(pfile) ;
	cur_record_p = ftell(pfile) ;
	//the current item covered by the next item,and their record id decrease
	while(is_last)
	{
		fseek(pfile,cur_record_p,SEEK_SET);
		ret = fread(from,sizeof(struct sel_event_record),1,pfile);
		if(ret < 0)
		{
			UBMC_IPMI_DEBUG_INFO("fread is fail: %d \n",ret);
			return -1;
		}
		else if(ret == 0)
		{
			break;
		}
		//UBMC_IPMI_DEBUG_INFO("fgets 2:%s \n",from);
		next_record_p = ftell(pfile) ;
	    is_last = change_sel_record_id(from,&to,0);
	    fseek(pfile,last_record_p ,SEEK_SET);
	    ret = change_msg_endian(&to,0);
		if(ret < 0)
		{
			UBMC_IPMI_ERRLOG("change_msg_endian fail");
			return -1;
		}
	    ret = fwrite(&to,sizeof(struct sel_event_record),1,pfile);
	    fflush(pfile);
	    last_record_p = ftell(pfile) ;
	    //UBMC_IPMI_DEBUG_INFO("fputs 2:%s\n",to);
	    cur_record_p = next_record_p;
	    cnt ++;
	    if(cnt > SEL_ENTRY_MAXNUM -1)
	    {
	    	UBMC_IPMI_DEBUG_INFO("cnt is larger than sel_entry_max_num :0x%x \n",cnt);
	    	break;
	    }
	}
	return ret;
}

/***
 * to clear driver list of record
 * @param ubmc_ipmi_sel
 * @return nonzero when success,-1 when fail
 */
int ubmc_ipmi_clr_sel_list(struct ubmc_ipmi_sel* ubmc_ipmi_sel)
{
	int ret = 0;
	if(ubmc_ipmi_sel->ipmi_dev_fd < 0)
	{
		UBMC_IPMI_ERRLOG("can not access ubmc ipmi dev ");
		return -1;
	}
	ret = ioctl(ubmc_ipmi_sel->ipmi_dev_fd,IOCTL_CLR_SEL_LIST_CMD,NULL);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ioctl add sel node fail ret is %ld",ret);
		return ret;
	}
}
int ubmc_ipmi_check_sensor_value_is_valid(unsigned int value)
{
	int value_temp;
	value_temp = (int)value;
	if(value_temp < 0)
	{
		return 0;
	}
	return 1;

}

/**
 * check the value ,if the value is not in limit ,verify the event type
 * @param value
 * @param thresh
 * @return nonzero when success,-1 when fail
 */
int check_sensor_event(unsigned int value,ubmc_ipmi_sensor_thresh_s *thresh)
{
/*	if(!check_thresh_is_valid(thresh))
		return -1;*/
	//first check the value,if the value is not valid ,pass it
	if(!ubmc_ipmi_check_sensor_value_is_valid(value))
	{
		return NO_EVENT;
	}
	if(value >= thresh->unr && thresh->unr != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_UNR)
		{
			thresh->status = UBMC_IPMI_ON_UNR;
			return UNR_EVENT;
		}
		else
		{
			return NO_EVENT;
		}
	}
	else if(value >= thresh->ucr && thresh->ucr != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_UCR)
		{
			thresh->status = UBMC_IPMI_ON_UCR;
			return UCR_EVENT;
		}
		else
		{
			return NO_EVENT;
		}
	}
	else if(value >= thresh->unc && thresh->unc != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_UNC)
		{
			thresh->status = UBMC_IPMI_ON_UNC;
			return UNC_EVENT;
		}
		else
		{
			return NO_EVENT;
		}

	}
	else if(value <= thresh->lnr && thresh->lnr != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_LNR)
		{
			thresh->status = UBMC_IPMI_ON_LNR;
			return LNR_EVENT;
		}
		else
		{

			return NO_EVENT;
		}
	}
	else if(value <= thresh->lcr && thresh->lcr != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_LCR)
		{
			thresh->status = UBMC_IPMI_ON_LCR;
			return LCR_EVENT;
		}
		else
		{
			return NO_EVENT;
		}
	}
	else if(value <= thresh->lnc && thresh->lnc != UBMC_IPMI_N_A)
	{
		if(thresh->status < UBMC_IPMI_ON_LNC)
		{
			thresh->status = UBMC_IPMI_ON_LNC;
			return LNC_EVENT;
		}
		else
		{
			return NO_EVENT;
		}
	}
	thresh->status = UBMC_IPMI_NORMAL;
	return NO_EVENT;
}

/***
 * set the sensor event type of threshold event
 * @param flag event type code
 * @param event
 * @return
 */
static int ubmc_ipmi_set_evt_type(unsigned int flag,struct sel_event_record  *event)
{
	int ret = 0;
	switch(flag)
	{
		case UNR_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x0b;	//Upper Non-recoverable - going high
			ret = 1;
			break;
		}
		case UCR_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x09;	//Upper Critical - going high
			ret = 1;
			break;
		}
		case UNC_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x07;	//Upper Non-critical - going high
			ret = 0;											//N/A this event
			break;
		}
		case LNC_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x00;	//Upper Non-critical - going high
			ret = 0;											//N/A this event
			break;
		}
		case LCR_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x02;	//Lower Critical - going low
			ret = 1;
			break;
		}
		case LNR_EVENT:
		{
			event->sel_type.standard_type.event_type = 0x01;
			event->sel_type.standard_type.event_data[0] = 0x04;	//Lower Non-recoverable - going low
			ret = 1;
			break;
		}
		default:
			UBMC_IPMI_ERRLOG("Error event type : %d \n",flag);
			return BMC_FALSE;
			break;
	}
	return ret;
}


static int ubmc_ipmi_set_sensor_type_num(unsigned int sensor_type,unsigned char num,struct sel_event_record  *event)
{
	int ret = 0;
	switch(sensor_type)
	{
		case UBMC_SENSOR_TEMP:
		{
			event->sel_type.standard_type.sensor_type = SANSOR_TYPE_TEMP;
			event->sel_type.standard_type.sensor_num = num;
			ret = 1;
			break;
		}
		case UBMC_SENSOR_VOL:
		{
			event->sel_type.standard_type.sensor_type  = SANSOR_TYPE_VOL;
			event->sel_type.standard_type.sensor_num = num;
			ret = 1;
			break;
		}
		case UBMC_SENSOR_FAN:
		{
			event->sel_type.standard_type.sensor_type  = SANSOR_TYPE_FAN;
			event->sel_type.standard_type.sensor_num = num;
			ret = 1;
			break;
		}
		default:
			UBMC_IPMI_ERRLOG("Error sensor type : %d \n",sensor_type);
			return BMC_FALSE;
			break;
	}
	return ret;
}


int ubmc_ipmi_set_sensor_evt(unsigned int evt_flag, ipmi_sensor_type type,unsigned char sensor_id,struct sel_event_record  *event)
{
	int ret = 0;
	event->record_type = 0x02;
	event->sel_type.standard_type.event_dir = 0x00;
	ret = ubmc_ipmi_set_sensor_type_num(type,sensor_id,event);
	if(ret < 0)
	{
		return BMC_FALSE;
	}
	ret = ubmc_ipmi_set_evt_type(evt_flag,event);
	if(ret < 0)
	{
		return BMC_FALSE;
	}
	return 1;
}

int ubmc_ipmi_set_gpio_evt(unsigned int evt_flag, ipmi_sensor_type type,unsigned char sensor_id,struct sel_event_record  *event)
{
	int ret = 0;
	event->record_type = 0x01;
	event->sel_type.standard_type.event_dir = 0x00;
	ret = ubmc_ipmi_set_sensor_type_num(type,sensor_id,event);
	if(ret < 0)
	{
		return BMC_FALSE;
	}
	ret = ubmc_ipmi_set_evt_type(evt_flag,event);
	if(ret < 0)
	{
		return BMC_FALSE;
	}
	return 1;
}

int ubmc_ipmi_set_sel_maxnum(struct ubmc_ipmi_sel* ubmc_ipmi_sel,unsigned int num)
{
	int ret = 0;
	if(ubmc_ipmi_sel->ipmi_dev_fd < 0)
	{
		UBMC_IPMI_ERRLOG("can not access ubmc ipmi dev ");
		return -1;
	}
	ret = ioctl(ubmc_ipmi_sel->ipmi_dev_fd,IOCTL_SET_SEL_ENTRY_MAXNUM_CMD,&num);
	ubmc_ipmi_sel->sel_entry_max_num = num;
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ioctl set sel num fail ret is %ld",ret);
		return ret;
	}
}

/***
 * add a new record node to driver record list
 * @param ubmc_ipmi_sel
 * @param event
 * @return
 */
int ubmc_ipmi_add_sel_node(struct ubmc_ipmi_sel* ubmc_ipmi_sel,struct sel_event_record* event)
{
	int ret = 0;
	if(ubmc_ipmi_sel->ipmi_dev_fd < 0)
	{
		UBMC_IPMI_ERRLOG("can not access ubmc ipmi dev ");
		return -1;
	}
	ret = ioctl(ubmc_ipmi_sel->ipmi_dev_fd,IOCTL_ADD_SEL_NODE_CMD,event);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ioctl add sel node fail ret is %ld",ret);
		return ret;
	}
}

/***
 * read record msg from emmc,and sync driver record list
 * @param ubmc_ipmi_sel the instance of ubmc_ipmi_sel
 * @param event
 * @return
 */
int ubmc_ipmi_read_msg_from_emmc(struct ubmc_ipmi_sel *ubmc_ipmi_sel,struct sel_event_record* event)
{
	char msg_buf[SEL_MSG_SIZE] = {0};
	int ret = 0,cnt = 0,cur_record_id = 0,record_id = 0;
	unsigned int cur_offset = 0 ,last_offset = 0;
	fseek(ubmc_ipmi_sel->sel_msg_fd,0,SEEK_SET);
	while(fread(msg_buf,sizeof(struct sel_event_record),1,ubmc_ipmi_sel->sel_msg_fd) != 0)
	{
		//UBMC_IPMI_DEBUG_INFO("%s ",msg_buf);
		cur_offset = ftell(ubmc_ipmi_sel->sel_msg_fd);
		cur_record_id = ubmc_ipmi_parse_sel_msg(msg_buf,event,1);
		cnt ++;
		//if the record id is 0,it means record id is invalid,
		if(cur_record_id <= 0)
		{
			UBMC_IPMI_ERRLOG("ubmc_ipmi_parse_sel_msg fail record_id is %d is 0x%lx offset is 0x%lx",record_id,cnt,last_offset);
			//to avoid the sel file be corrupted when the system power off.
			//sometimes the file will add some '\0' that do not write when power off the system
			ret = truncate("/var/log/sel",last_offset);
			if(ret < 0)
			{
				UBMC_IPMI_ERRLOG("truncate file fail error : %s\n",strerror(errno));
				return ret;
			}
			fseek(ubmc_ipmi_sel->sel_msg_fd,last_offset,SEEK_SET);

			break;
		}
		//add record node to driver record list by record file on emmc
		ret = ubmc_ipmi_add_sel_node(ubmc_ipmi_sel,event);
		if(ret < 0)
		{
			UBMC_IPMI_ERRLOG("add node fail cnt is %ld ",cnt);
			fseek(ubmc_ipmi_sel->sel_msg_fd,cur_offset,SEEK_SET);
			break;
		}
		record_id = cur_record_id;
		last_offset = cur_offset;
	}
	//set record_id after read the record
	//lock
	ubmc_ipmi_mutex_lock(&ubmc_ipmi_sel->record_mutex);
	ubmc_ipmi_sel->record_id = record_id + 1;
	ubmc_ipmi_mutex_unlock(&ubmc_ipmi_sel->record_mutex);
	//unclock
	return 1;
}

/**
 * this function do that add a event message to a record file which is full
 * @param fp the file point of sel record file
 * @param event	event which add
 * @return the record id
 */
int ubmc_ipmi_add_msg_if_full(FILE* fp,struct sel_event_record* event)
{
	struct sel_event_record record_event;
	int ret;
	upgrade_sel_record(fp);
	memcpy(&record_event,event,sizeof(struct sel_event_record));
	record_event.record_id = record_event.record_id - 1;
	//	UBMC_IPMI_DEBUG_INFO("event->record_id is 0x%lx \n",event->record_id);
	ret = ubmc_ipmi_add_msg(fp,&record_event);
	return ret;
}

/**
 * add a event record to file
 * @param fp
 * @param event
 * @return
 */
int ubmc_ipmi_record_msg_to_emmc(FILE* fp,struct sel_event_record* event)
{
	char msg_buf[SEL_MSG_SIZE] = {0};
	unsigned int checksum;
	int ret = 0;
	if(event == NULL)
	{
		UBMC_IPMI_ERRLOG("event is NULL ");
		return -1;
	}
	if(event->record_id <= SEL_ENTRY_MAXNUM)
	{
		ret = ubmc_ipmi_add_msg(fp,event);
	}
	else
	{
		ret = ubmc_ipmi_add_msg_if_full(fp ,event);
		//upgrade_sel_record(fp ,event);
	}

	return ret;
}

/***
 * create sel record file if there is no exist file on emmc and change the file permission
 * @param path of the record file
 * @return
 */
FILE* create_sel_file_on_emmc(const char *path)
{
	int res;
	FILE* fp;
	res = is_gpio_file_exist(path,F_OK|W_OK|R_OK);
	if(res)
	{
		//UBMC_IPMI_DEBUG_INFO("this file %s has been exist \n",path);
		fp = fopen(path,"r+");
		if(fp == NULL)
		{
			UBMC_IPMI_ERRLOG("open %s fail  error %s\n",path,strerror(errno));
		}
	}
	else
	{
		fp = fopen(path,"w+");
		if(fp == NULL)
		{
			UBMC_IPMI_ERRLOG("create %s file fail  error %s \n",path,strerror(errno));
			return NULL;
		}
		if(0 != chmod(path,S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH))
		{
			UBMC_IPMI_ERRLOG("chmod %s fail error : %s\n",path,strerror(errno));
			return NULL;
		}
		//UBMC_IPMI_DEBUG_INFO("create %s \n",path);
	}
	res = fseek(fp,0,SEEK_SET);
	if(res < 0)
	{
		UBMC_IPMI_ERRLOG(" %s fseek fail  error %s \n",path,strerror(errno));
		return NULL;
	}

	return fp;

}

void ubmc_ipmi_init_record_id(struct ubmc_ipmi_sel *ubmc_ipmi_sel)
{
	ubmc_ipmi_mutex_lock(&ubmc_ipmi_sel->record_mutex);
	ubmc_ipmi_sel->record_id = 1;
	ubmc_ipmi_mutex_unlock(&ubmc_ipmi_sel->record_mutex);
}
/*
void ubmc_ipmi_record_id_increase(struct ubmc_ipmi_sel *ubmc_ipmi_sel, unsigned short int count)
{
	ubmc_ipmi_sel->record_id = ubmc_ipmi_sel->record_id + count;
}

void ubmc_ipmi_record_id_decrease(struct ubmc_ipmi_sel *ubmc_ipmi_sel, unsigned short int count)
{
	ubmc_ipmi_sel->record_id = ubmc_ipmi_sel->record_id - count;
}
*/
int ubmc_ipmi_clear_record_file(struct ubmc_ipmi_sel *ubmc_ipmi_sel)
{
	FILE* fp;
	int ret = 0;
	fp = ubmc_ipmi_sel->sel_msg_fd;
	ret = fseek(fp,0,SEEK_SET);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("fseek fail error : %s\n",strerror(errno));
		return ret;
	}
	ret = truncate("/var/log/sel",0);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("truncate file fail error : %s\n",strerror(errno));
		return ret;
	}
	return ret;
}

int ubmc_ipmi_init_sel(struct ubmc_ipmi_sel *ubmc_ipmi_sel,int ipmi_dev_fd,device_type_t device_type)
{
	FILE* sel_msg_record_fd;
	struct sel_event_record event;
	sel_msg_record_fd = create_sel_file_on_emmc("/var/log/sel");
	if(sel_msg_record_fd == NULL)
	{
		UBMC_IPMI_ERRLOG("create_sel_file_on_emmc fail \n");
		return -1;
	}
	if(pthread_mutex_init(&ubmc_ipmi_sel->record_mutex, NULL) != 0)
	{
		UBMC_IPMI_ERRLOG(" record_mutex init fail \n");
		return -1;
	}
	ubmc_ipmi_init_record_id(ubmc_ipmi_sel);
	ubmc_ipmi_sel->ipmi_dev_fd = ipmi_dev_fd;
	ubmc_ipmi_sel->sel_msg_fd = sel_msg_record_fd;
	ubmc_ipmi_sel->device_type = device_type;
	ubmc_ipmi_clr_sel_list(ubmc_ipmi_sel);
	ubmc_ipmi_set_sel_maxnum(ubmc_ipmi_sel,SEL_ENTRY_MAXNUM);
	ubmc_ipmi_sel->state = 1;
	//read the record from file and init the driver record list
	ubmc_ipmi_read_msg_from_emmc(ubmc_ipmi_sel,&event);
}

static const char *ubmc_ipmi_get_event_type(uint8_t code)
{
        if (code == 0)
                return "Unspecified";
        if (code == 1)
                return "Threshold";
        if (code >= 0x02 && code <= 0x0b)
                return "Generic Discrete";
        if (code == 0x6f)
                return "Sensor-specific Discrete";
        if (code >= 0x70 && code <= 0x7f)
                return "OEM";
        return "Reserved";
}


//the sensor type ,copy from ipmitool
const char *ubmc_ipmi_generic_sensor_type_vals[] = {
    "reserved",
    "Temperature", "Voltage", "Current", "Fan",
    "Physical Security", "Platform Security", "Processor",
    "Power Supply", "Power Unit", "Cooling Device", "Other",
    "Memory", "Drive Slot / Bay", "POST Memory Resize",
    "System Firmwares", "Event Logging Disabled", "Watchdog1",
    "System Event", "Critical Interrupt", "Button",
    "Module / Board", "Microcontroller", "Add-in Card",
    "Chassis", "Chip Set", "Other FRU", "Cable / Interconnect",
    "Terminator", "System Boot Initiated", "Boot Error",
    "OS Boot", "OS Critical Stop", "Slot / Connector",
    "System ACPI Power State", "Watchdog2", "Platform Alert",
    "Entity Presence", "Monitor ASIC", "LAN",
    "Management Subsys Health", "Battery", "Session Audit",
    "Version Change", "FRU State",
    NULL
};


const char* ubmc_ipmi_get_generic_sensor_type(uint8_t code)
{
	if (code <= SENSOR_TYPE_MAX) {
		return ubmc_ipmi_generic_sensor_type_vals[code];
	}

	return NULL;
}

const char *ubmc_ipmi_get_sensor_type(uint8_t code)
{
	const char *type;

	if (code >= 0xC0) {
		//do not support this type
		//type = ubmc_ipmi_get_oem_sensor_type(code);
	} else {
		type = ubmc_ipmi_get_generic_sensor_type(code);
	}

	if (type == NULL) {
		type = "Unknown";
	}
	return type;
}

static char *ubmc_ipmi_sel_timestamp(uint32_t stamp)
{
	static char tbuf[40];
	time_t s = (time_t)stamp;
	memset(tbuf, 0, 40);
	strftime(tbuf, sizeof(tbuf), "%m/%d/%Y %H:%M:%S", gmtime(&s));
	return tbuf;
}

static char *ubmc_ipmi_sel_timestamp_date(uint32_t stamp)
{
	static char tbuf[11];
	time_t s = (time_t)stamp;
	strftime(tbuf, sizeof(tbuf), "%m/%d/%Y", gmtime(&s));
	return tbuf;
}

static char *ubmc_ipmi_sel_timestamp_time(uint32_t stamp)
{
	static char tbuf[9];
	time_t s = (time_t)stamp;
	strftime(tbuf, sizeof(tbuf), "%H:%M:%S", gmtime(&s));
	return tbuf;
}

static int ubmc_ipmi_get_event_desc(struct sel_event_record *event, char* des)
{
	const struct ipmi_event_sensor_types *evt, *start, *next = NULL;
	unsigned char code = 0,offset = 0,sensor_type_code = 0;
	unsigned char event_type = 0;
	event_type = event->sel_type.standard_type.event_type;
	if (event_type == 0x6f)
	{
		start = sensor_specific_event_types;
		code = event->sel_type.standard_type.sensor_type;
		offset = event->sel_type.standard_type.event_data[0] & 0x0f;
	}
	else if(event_type  < 0x0c)
	{
		start = generic_event_types;
		code = event_type;
		offset = event->sel_type.standard_type.event_data[0] & 0x0f;
	}
	else
	{
		strcpy(des,"UNKNOWN");
		return 1;

	}
	for (evt = start; evt->desc != NULL || next != NULL; evt++)
	{
		if (evt->desc == NULL)
		{
			/* proceed with next table */
			evt = next;
			next = NULL;
		}
		if (code == evt->code)
		{
			if((evt + offset)->desc != NULL)
			{
				strcpy(des,(evt + offset)->desc);
				//UBMC_IPMI_DEBUG_INFO("%s ",des);
			}
			else
			{
				strcpy(des,"NULL");
			}
			return 1;
		}
	}

	return 1;
}

#define ID_MAXSIZE	        20
#define TIME_DATE_MAXSIZE	20
#define TIME_TIME_MAXSIZE	20
#define SENSOR_TYPE_MAXSIZE	32
#define SENSOR_NUM_MAXSIZE	8
#define DIR_MAXSIZE			32
#define DES_MAXSIZE			128
#define LOG_MAXSIZE 		256

/***
 * generate the event log by the sel record
 * @param evt
 * @param log
 * @return return 1 when success return -1 when fail
 */
static int ubmc_ipmi_gen_event_log(struct sel_event_record *evt,char *log)
{

	char id_domain[ID_MAXSIZE] = {0};
	char time_date_domain[TIME_DATE_MAXSIZE] = {0};

	char time_time_domain[TIME_TIME_MAXSIZE] = {0};
	char sensor_type_domain[SENSOR_TYPE_MAXSIZE] = {0};
	char sensor_num_domain[SENSOR_NUM_MAXSIZE] = {0};
	char des_domain[DES_MAXSIZE] = {0};
	char dir_domain[DIR_MAXSIZE] = {0};
	int ret = 0;
	sprintf(id_domain,"%4x",evt->record_id);
	if(evt->record_type >= 0xc0)
	{
		UBMC_IPMI_ERRLOG("do not support this type \n");
		return -1;
	}
	if ((evt->sel_type.standard_type.timestamp < 0x20000000))
	{
		sprintf(time_date_domain," Pre-Init ");
		sprintf(time_time_domain,"%010d", evt->sel_type.standard_type.timestamp );
	}
	else
	{
		sprintf(time_date_domain,"%s", ubmc_ipmi_sel_timestamp_date(evt->sel_type.standard_type.timestamp));
		sprintf(time_time_domain,"%s", ubmc_ipmi_sel_timestamp_time(evt->sel_type.standard_type.timestamp));
	}
	sprintf(sensor_type_domain,"%s ", ubmc_ipmi_get_sensor_type(evt->sel_type.standard_type.sensor_type));
	if (evt->sel_type.standard_type.sensor_num != 0)
	{
		sprintf(sensor_num_domain," #0x%02x", evt->sel_type.standard_type.sensor_num);
	}
	ret = ubmc_ipmi_get_event_desc(evt, des_domain);
	if (evt->sel_type.standard_type.event_dir)
	{
		sprintf(dir_domain,"Deasserted");
	}
	else
	{
		sprintf(dir_domain,"Asserted");
	}

	//sprintf(des_domain,"%s",des_domain);
	sprintf(log,"%s | %s | %s | %s  %s| %s | %s |",id_domain,time_date_domain,time_time_domain,sensor_type_domain,sensor_num_domain,des_domain,dir_domain);
	return 1;
}

/**
 * just print the sel record by decode the message record file
 * @param ubmc_ipmi_sel
 * @return
 */
int ubmc_ipmi_prt_sel(struct ubmc_ipmi_sel *ubmc_ipmi_sel)
{
	struct sel_event_record event;
	char msg_buf[SEL_MSG_SIZE] = {0};
	char prt_buf[LOG_MAXSIZE] = {0};
	int offset = 0,record_id = 0;
	int cnt;
	int ret;
	ret = fseek(ubmc_ipmi_sel->sel_msg_fd,0,SEEK_SET);
	if(ret != 0)
	{
		UBMC_IPMI_ERRLOG("fseek failed  %s",strerror(errno));
		return -1;
	}
	while(fread(msg_buf,sizeof(struct sel_event_record),1,ubmc_ipmi_sel->sel_msg_fd) != 0)
	{
		offset = ftell(ubmc_ipmi_sel->sel_msg_fd);

		record_id = ubmc_ipmi_parse_sel_msg(msg_buf,&event,1);
		if(record_id < 0)
		{
			UBMC_IPMI_ERRLOG("ubmc_ipmi_parse_sel_msg fail cnt is %d",cnt);
			fseek(ubmc_ipmi_sel->sel_msg_fd,offset,SEEK_SET);
			break;
		}
		ubmc_ipmi_gen_event_log(&event,prt_buf);
		printf("%s \n",prt_buf);
	}
	return 1;
}

/***
 * generate a sel event record by current event
 * @param ubmc_ipmi_sel
 * @param event
 * @return
 */
static struct sel_event_record * ubmc_ipmi_gen_event_msg(struct ubmc_ipmi_sel *ubmc_ipmi_sel,struct sel_event_record * event)
{
	struct sel_event_record * event_msg;
	unsigned int offset = 0;
	time_t tt,t_ret;
	unsigned int ret;
	unsigned short int record_id_lsb = 0,record_id_msb = 0;
	event_msg = malloc(sizeof(struct sel_event_record));
	if(event_msg == NULL)
	{
		UBMC_IPMI_ERRLOG("alloc event_msg fail \n");
		return NULL;
	}
	event_msg->record_id = ubmc_ipmi_sel->record_id;
	//lock
	ubmc_ipmi_mutex_lock(&ubmc_ipmi_sel->record_mutex);
	ubmc_ipmi_sel->record_id ++;
	if(ubmc_ipmi_sel->record_id > SEL_ENTRY_MAXNUM)
	{
		ubmc_ipmi_sel->record_id = SEL_ENTRY_MAXNUM + 1;
	}
	ubmc_ipmi_mutex_unlock(&ubmc_ipmi_sel->record_mutex);
	//unclock
	event_msg->record_type = event->record_type;
	t_ret = time(&tt);
	if(t_ret < 0)
	{
		UBMC_IPMI_ERRLOG("get time fail ");
	}
	if(event_msg->record_type < 0xc0)
	{
		//system record event
		event_msg->sel_type.standard_type.timestamp = t_ret;
		event_msg->sel_type.standard_type.gen_id = event->sel_type.standard_type.gen_id;
		event_msg->sel_type.standard_type.evm_rev = event->sel_type.standard_type.evm_rev;
		event_msg->sel_type.standard_type.sensor_type = event->sel_type.standard_type.sensor_type;
		event_msg->sel_type.standard_type.sensor_num = event->sel_type.standard_type.sensor_num;
		event_msg->sel_type.standard_type.event_dir = event->sel_type.standard_type.event_dir;
		event_msg->sel_type.standard_type.event_type = event->sel_type.standard_type.event_type;
		event_msg->sel_type.standard_type.event_data[0] = event->sel_type.standard_type.event_data[0];
		event_msg->sel_type.standard_type.event_data[1] = event->sel_type.standard_type.event_data[1];
		event_msg->sel_type.standard_type.event_data[2] = event->sel_type.standard_type.event_data[2];
	}
	else if(event_msg->record_type>=0xc0 && event_msg->record_type <=0xdf)
	{
		//OEM timestamped ,byte 8-16 OEM defined
	}
	else if(event_msg->record_type>=0xe0 && event_msg->record_type <=0xff)
	{
		//OEM non-timestamped ,byte 4-16 OEM defined
	}
	else
	{
		;
	}
	return event_msg;

}

static struct sel_event_record  do_test_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x01,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x0a,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x56,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  rst_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x1d,		//
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,	    //
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x07,	//
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};


static struct sel_event_record  thermtrip_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x19,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x01,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  prochot_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x01,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x0a,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x56,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  err_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x13,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x03,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  s3_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x22,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x03,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  s45_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x22,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x06,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  psu_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x08,
		.sel_type.standard_type.sensor_num = 0x01,
		.sel_type.standard_type.event_type = 0x6f,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x03,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};

static struct sel_event_record  sensor_event =
{
		.record_id = 0x00,
		.record_type = 0x01,
		.sel_type.standard_type.timestamp = 0x00,
		.sel_type.standard_type.gen_id	= 0x00,
		.sel_type.standard_type.evm_rev = 0x04,
		.sel_type.standard_type.sensor_type = 0x08,
		.sel_type.standard_type.sensor_num = 0x00,
		.sel_type.standard_type.event_type = 0x01,
		.sel_type.standard_type.event_dir = 0x00,
		.sel_type.standard_type.event_data[0] = 0x03,
	    .sel_type.standard_type.event_data[1] = 0x00,
	    .sel_type.standard_type.event_data[2] = 0x00,
};


int ubmc_ipmi_read_gpio_value(struct gpio_s *gpio)
{
	int ret;
	int value;
	char c;
	if (gpio->fd < 0)
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

int ubmc_ipmi_read_gpio_value_by_name(struct ubmc_ipmi_sel* ubmc_ipmi_sel,const char*name)
{
	int ret;
	int i;
	int value;
	struct poll_gpio_fd *gpio_fds;
	gpio_fds = &ubmc_ipmi_sel->gpio_fds;
	for(i = 0; i < UBMC_IPMI_GPIO_MAXNUM; i++)
	{
		if(strcmp(gpio_fds->gpio[i].gpio_name,name) == 0)
		{
			value = ubmc_ipmi_read_gpio_value(&gpio_fds->gpio[i]);
			if(ret < 0)
			{
				UBMC_IPMI_ERRLOG("read gpio %s value fail  \n",name);
				return -1;
			}
			return value;
		}
	}
	UBMC_IPMI_ERRLOG("did not monitor this gpio %s \n",name);

	return -1;;

}
int ubmc_ipmi_check_gpio_value_of_event(struct ubmc_ipmi_sel* ubmc_ipmi_sel,struct sel_event_record *event)
{
	//
	struct gpio_s gpio;
	int gpio_num = 0;
	int fd;
	int value;
	int ret = 0;
	value = ubmc_ipmi_read_gpio_value_by_name(ubmc_ipmi_sel,"HOST_S45_N");
	if(value == 0)
	{
		memcpy(event,&s45_event,sizeof(struct sel_event_record ));
		return 1;
	}
	else if(value < 0)
	{
		UBMC_IPMI_ERRLOG("read HOST_S45_N pin value fail \n");
		ret = ret + value;
	}
	value = ubmc_ipmi_read_gpio_value_by_name(ubmc_ipmi_sel,"HOST_S3_N");
	if(value == 0)
	{
		memcpy(event,&s3_event,sizeof(struct sel_event_record ));
		return 1;
	}
	else if(value < 0)
	{
		UBMC_IPMI_ERRLOG("read HOST_S3_N pin value fail \n");
		ret = ret + value;
	}
	value = ubmc_ipmi_read_gpio_value_by_name(ubmc_ipmi_sel,"HOST_PLTRST_N");
	if(value == 0)
	{
		memcpy(event,&rst_event,sizeof(struct sel_event_record ));
		//do a delay to wait the system reboot.To avoid some event happen in this interval
		ubmc_ipmi_sel->state = 0;
		sleep(10);
		return 1;
	}
	else if(value < 0)
	{
		UBMC_IPMI_ERRLOG("read HOST_PLTRST_N pin value fail \n");
		ret = ret + value;
	}
	return ret;
}
//sometimes a event maybe generate multiple event.So we only record the most fundamental event.
//just check are there multiple events
int ubmc_ipmi_event_check(struct ubmc_ipmi_sel* ubmc_ipmi_sel,struct sel_event_record * event)
{
	int ret = 0;
	ret = ubmc_ipmi_check_gpio_value_of_event(ubmc_ipmi_sel,event);
	if(ret == 1)
	{
		ubmc_ipmi_sel->state = 0; //the last one sel when system down
		//UBMC_IPMI_NOTE("SEL is disable \n");
	}
	return ret;
}
int ubmc_ipmi_event_handle(struct ubmc_ipmi_sel* ubmc_ipmi_sel,struct sel_event_record * event)
{
	struct sel_event_record * event_msg;
	struct sel_event_record  new_event;
	int ret;
	if(ubmc_ipmi_sel->state > 0)
	{
		ret = ubmc_ipmi_event_check(ubmc_ipmi_sel,&new_event);
		if(ret < 0)
		{
			UBMC_IPMI_ERRLOG("ubmc_ipmi_event_check is fail \n");
		}
		//ret == 0 means do not happen multiple events
		else if(ret == 0)
		{
			memcpy(&new_event,event,sizeof(struct sel_event_record ));
		}
		event_msg = ubmc_ipmi_gen_event_msg(ubmc_ipmi_sel,&new_event);
		if(event_msg == NULL)
		{
			UBMC_IPMI_ERRLOG("generate event_msg is fail \n");
			free(event_msg);
			return -1;
		}
		// add new sel record to emmc
	    ret = ubmc_ipmi_record_msg_to_emmc(ubmc_ipmi_sel->sel_msg_fd,event_msg);
	    if(ret < 0)
	    {
	    	UBMC_IPMI_ERRLOG("ubmc_ipmi_record_the_event fail \n");
	    	free(event_msg);
	    	return -1;
	    }
	    // add new sel record to driver sel list
		ret = ubmc_ipmi_add_sel_node(ubmc_ipmi_sel,event_msg);
		if(ret < 0)
		{
			UBMC_IPMI_DEBUG_INFO("ubmc_ipmi add node is fail \n");
			free(event_msg);
			return -1;
		}
		free(event_msg);
		return 1;
	}
	else
	{
		return 1;
	}

}


int ubmc_ipmi_open_gpio_value_file(struct gpio_s *gpio)
{
	int fd = 0,status = 0;
	int gpio_num = 0;
	bool res;
	char cmd_buf[CMD_BUF_SIZE] = {0};
	char path_buf[DIR_BUF_SIZE] = {0};
	gpio_num = ubmc_ipmi_create_gpio_file(gpio->gpio_num);
	if(gpio_num < 0)
	{
		UBMC_IPMI_ERRLOG("can not create this file of gpio :%d \n",gpio_num);
		return -1;
	}
	sprintf(path_buf,"/sys/class/gpio/gpio%d/value",gpio_num);
	res = is_gpio_file_exist(path_buf,F_OK|W_OK|R_OK);
	if(!res)
	{
		UBMC_IPMI_ERRLOG("can not find this file %s \n",path_buf);
		return -1;
	}
	if(gpio->tri == TRI_FALLING)
	{
		sprintf(cmd_buf,"echo falling > /sys/class/gpio/gpio%d/edge",gpio_num);
	}
	else if(gpio->tri == TRI_RISING)
	{
		sprintf(cmd_buf,"echo rising > /sys/class/gpio/gpio%d/edge",gpio_num);
	}
	else if(gpio->tri == TRI_HIGH_LEVE || gpio->tri == TRI_LOW_LEVE)
	{
		sprintf(cmd_buf,"echo none > /sys/class/gpio/gpio%d/edge",gpio_num);
	}
	else if(gpio->tri == TRI_NULL)
	{
		fd = open(path_buf,O_RDWR);
		gpio->fd = fd;
		return fd;
	}
	else
	{
		UBMC_IPMI_ERRLOG("no this edg type \n");
	}
	status = system(cmd_buf);
	if(status < 0)
	{
		UBMC_IPMI_ERRLOG("do cmd %s fail \n",cmd_buf);
	}
	fd = open(path_buf,O_RDWR);
	gpio->fd = fd;
	return fd;
}


/**
 *set gpio attribute ,will init a struct gpio_s instance
 * @param gpio
 * @param gpio_bank
 * @param gpio_pin
 * @param tri	the edge of gpio when the event happen
 * @param flag indicate which platform was in
 */
int ubmc_ipmi_init_gpio_s(struct gpio_s *gpio,const char *name,char tri,device_type_t device_type)
{
	int gpio_num = 0;
	int ret = 0;
	char bank_num;
	char pin_num;
	/*gpio->gpio_bank = gpio_bank;
	gpio->gpio_pin = gpio_pin;*/
	if(gpio == NULL || name == NULL)
	{
		UBMC_IPMI_ERR("can not get gpio or name \n");
		return -1;
	}
	ret = silc_gpio_get_by_name(name,&bank_num,&pin_num);
	if (ret < 0)
	{
		UBMC_IPMI_ERR("can not get_gpio_by_name %s\n",name);
		return -1;
	}
	if(device_type == XSMALL || device_type == SMALL || device_type == LARGE || device_type == MIDDLE)	//in TI-AM335X
	{
		gpio_num = AM335X_GPIO_TO_PIN(bank_num,pin_num);
	}
	else if(device_type == SKYD)
	{
		//Use Marvell A3700 pin map
		if(bank_num == 0)
		{
			gpio_num = pin_num + 476;
		}
		else if(bank_num == 1)
		{
			gpio_num = pin_num + 446;
		}
	}
	else
	{
		UBMC_IPMI_ERR("Invalid device type %d \n",device_type);
		return -1;
	}
	strcpy(gpio->gpio_name,name);
	gpio->gpio_num = gpio_num;
	gpio->tri = tri;
	if(gpio->tri == TRI_FALLING || gpio->tri == TRI_LOW_LEVE)
	{
		//the gpio status when no events
		gpio->init_value = HIGH;
	}
	else
	{
		gpio->init_value = LOW;
	}
	return 1;
}
/*
int get_gpios_by_name(struct poll_gpio_fd *gpios,const char* name)
{
	int num = 0;
	char i;
	num = UBMC_IPMI_GPIO_MAXNUM;
	for(i = 0; i < num; i++)
	{
		if(strcmp(gpios->gpio.gpio_name,name) == 0)
		{
			return gpios->gpio.gpio_num;
		}
	}
	UBMC_IPMI_ERRLOG("did not monitor this gpio %s \n",name);
}
*/
int ubmc_ipmi_sel_clear(struct ubmc_ipmi_sel *ubmc_ipmi_sel)
{
	int ret = 0;
	ret = ubmc_ipmi_clear_record_file(ubmc_ipmi_sel);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("truncate sel file fail %s \n",strerror(errno));
		return -1;
	}
	ret = ubmc_ipmi_clr_sel_list(ubmc_ipmi_sel);
	if(ret < 0)
	{
		UBMC_IPMI_ERRLOG("ubmc_ipmi_clr_sel_list fail\n");
	}
	ubmc_ipmi_init_record_id(ubmc_ipmi_sel);

}

/**
 *poll the gpio which bind to the event
 * @param gpio_bank
 * @param gpio_pin
 */
int ubmc_ipmi_poll_gpio_events(struct ubmc_ipmi_sel *ubmc_ipmi_sel)
{
	struct poll_gpio_fd *gpio_fds;
	device_type_t device_type;
	char fd_fail_num = 0,gpio_fds_num = 0,count = 0;
	int gpio_fd = 0, i = 0,value = 0;
	int ret;
	int gpio_num = 0;
	gpio_fds = &ubmc_ipmi_sel->gpio_fds;
	device_type = ubmc_ipmi_sel->device_type;
	//to init the gpio pin which we neet to poll
	if(device_type == SKYD)
	{
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[0],"PSU_LEFT_PWRGD",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[1],"PSU_RIGHT_PWRGD",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[2],"HOST_S45_N",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[3],"HOST_S3_N",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[4],"HOST_ERROR_N",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[5],"HOST_PROCHOT_N",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[6],"HOST_THERMTRIP_N",TRI_NULL,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[7],"HOST_PLTRST_N",TRI_NULL,device_type) < 0)
			return -1;
	}
	else
	{
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[0],"PSU_LEFT_PWRGD",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[1],"PSU_RIGHT_PWRGD",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[2],"HOST_S45_N",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[3],"HOST_S3_N",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[4],"HOST_ERROR_N",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[5],"HOST_PROCHOT_N",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[6],"HOST_THERMTRIP_N",TRI_FALLING,device_type) < 0)
			return -1;
		if(ubmc_ipmi_init_gpio_s(&gpio_fds->gpio[7],"HOST_PLTRST_N",TRI_FALLING,device_type) < 0)
			return -1;
	}

//for test
#ifdef UBMC_IPMI_DEBUG_TEST
	if(ubmc_ipmi_set_poll_gpio(&gpio_fds.gpio[UBMC_IPMI_GPIO_MAXNUM-1],TEST_GPIO,TRI_FALLING) < 0)
		return -1;
#endif

	for(i = 0; i < UBMC_IPMI_GPIO_MAXNUM; i ++)
	{
		memset(&gpio_fds->fdset[i],0x00,sizeof(struct pollfd));
		gpio_fd = ubmc_ipmi_open_gpio_value_file(&gpio_fds->gpio[i]);
		if(gpio_fd < 0)
		{
			UBMC_IPMI_ERRLOG("open gpio value file fail :%d \n",gpio_fd);
			fd_fail_num ++;
			continue;
		}
		gpio_fds->fdset[i].fd =gpio_fd;
		gpio_fds->fdset[i].events = POLLPRI;

	}
	gpio_fds_num =UBMC_IPMI_GPIO_MAXNUM - fd_fail_num;
	while(1)
	{
		ret = poll(gpio_fds->fdset,gpio_fds_num,2000);
		if(ret < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			UBMC_IPMI_ERR("poll error :%s \n",strerror(errno));
		}
		else if (ret > 0)
		{
			for(i = 0; i < gpio_fds_num;i ++)
			{
				if((gpio_fds->fdset[i].revents & (POLLPRI|POLLERR)) == (POLLPRI | POLLERR))
				{
					count = read(gpio_fds->fdset[i].fd,&value,1);
					lseek(gpio_fds->fdset[i].fd,0,SEEK_SET);
					if(value == gpio_fds->gpio[i].init_value)
					{
						continue;
					}
					if(strcmp(gpio_fds->gpio[i].gpio_name,"PSU_LEFT_PWRGD") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&psu_event);
						UBMC_IPMI_DEBUG_INFO("psu_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"PSU_RIGHT_PWRGD") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&psu_event);
						UBMC_IPMI_DEBUG_INFO("psu_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_S45_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&s45_event);
						UBMC_IPMI_DEBUG_INFO("s45_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_S3_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&s3_event);
						UBMC_IPMI_DEBUG_INFO("s3_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_ERROR_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&err_event);
						UBMC_IPMI_DEBUG_INFO("err_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_PROCHOT_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&prochot_event);
						UBMC_IPMI_DEBUG_INFO("prochot_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_THERMTRIP_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&thermtrip_event);
						UBMC_IPMI_DEBUG_INFO("thermtrip_event");
					}
					else if(strcmp(gpio_fds->gpio[i].gpio_name,"HOST_PLTRST_N") == 0)
					{
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&rst_event);
						UBMC_IPMI_DEBUG_INFO("rst_event");
					}
#ifdef UBMC_IPMI_DEBUG_TEST
					else if(strcmp(gpio_fds.gpio[i].gpio_name,TEST_GPIO) == 0)
					{
						UBMC_IPMI_DEBUG_INFO("test");
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&do_test_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&rst_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&err_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&thermtrip_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&prochot_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&s3_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&s45_event);
						ubmc_ipmi_event_handle(ubmc_ipmi_sel,&psu_event);
					}
#endif
				}
			}
		}
		else
		{
			if(ubmc_ipmi_sel->state == 0)
			{
				if(ubmc_ipmi_read_gpio_value_by_name(ubmc_ipmi_sel,"HOST_S45_N") == 1 && ubmc_ipmi_read_gpio_value_by_name(ubmc_ipmi_sel,"HOST_PLTRST_N") == 1)
				{
					ubmc_ipmi_sel->state = 1;
					UBMC_IPMI_NOTE("SEL is reable \n");
				}
			}
			//UBMC_IPMI_NOTE("POLL TIME OUT \n");
		}
	}

}

