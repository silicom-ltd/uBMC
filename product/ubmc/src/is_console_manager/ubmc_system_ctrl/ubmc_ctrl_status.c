/*************************************************************************
	> File Name: file_test.c
	> Author: allen
	> Mail: allen.zhou@net-perf.com
	> Created Time: Fri 11 May 2018 04:11:32 PM HKT
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include "ubmc_ctrl_status.h"


static char * attr_name[PARAM_ATTR_POS_MAX] = {
    "POWER ON :",
    "POWER OFF :",
    "FORE POWER OFF :",
    "RESET :",
    "UPGRADE :",
    "CYCLE BOOT :",
    "OP_TYPE :",
};

static ubmc_ctrl_status param[PARAM_ATTR_POS_MAX];

static char * attr_value[PARAM_STATUS_POS_MAX] = {
    "processing",
    "ok",
    "error",
    "power_on",
    "power_off",
    "force_power_off",
    "reset",
    "upgrade",
    "cycle_boot",
    "null",
}; 

void ubmc_ctrl_set_param_name(param_attr_pos pos)
{
    strncpy(param[pos].attr_name,attr_name[pos],param_attr_size);
}

void ubmc_ctrl_set_param_value(param_attr_pos pos, param_status_pos value_pos)
{
    strncpy(param[pos].attr,attr_value[value_pos],param_attr_size);
}

void ubmc_ctrl_get_param_value(char * buf ,int len ,param_attr_pos pos)
{
    strncpy(buf, param[pos].attr, len > param_attr_size ? param_attr_size : len);
}

static int ubmc_ctrl_param_write_file(FILE *fp,param_attr_pos pos)
{
#define tmp_str_size (128)
    char str[tmp_str_size];    

    memset(str,0,tmp_str_size);
    snprintf(str,tmp_str_size,"%s%s\n",param[pos].attr_name, param[pos].attr);

#ifdef DEBUG
    printf("%s\n",str);
#endif

    fwrite(str,strlen(str),sizeof(char),fp);
}

int ubmc_ctrl_param_sync_file(FILE *fp)
{

    int fd;
    int count = 0;

    fd = fileno(fp);
    if(-1 == fd)
    {
        //printf("fp to fd error \n");    
        return -1;
    }else{
        flock(fd, LOCK_EX);
    }

    fseek(fp, 0, SEEK_SET);
    for(count =0; count < PARAM_ATTR_POS_MAX; count++)
    {

        ubmc_ctrl_param_write_file(fp,count);

    }

    fflush(fp);

    flock(fd, LOCK_UN);

    return 0;

}

int ubmc_ctrl_file_sync_param(FILE * fp)
{

    size_t len,name_len;
    int read;
    char *line = NULL,*ptr = NULL;
    char c = ':';
    int count =0;
    int fd;

    if(fp == NULL)
    {
        printf("fp null\n");
        return -1;
    }

    fd = fileno(fp);
    if(-1 == fd)
    {
        printf("fp to fd error \n");    
    }else{
        flock(fd, LOCK_EX);
    }

    while((read = getline(&line, &len, fp)) != -1)
    {
        ptr = strchr(line,c);
        if(ptr == NULL)
        {

#ifdef DEBUG
            printf("status file format unvalid\n");
#endif
            continue;
        }

        ubmc_ctrl_set_param_name(count); 

        len = strlen(ptr+1);

        //delete char space
        while(*(ptr + 1 ) == 0x20)
        {
            ptr = ptr + 1;
            len --;
        }

        strncpy(param[count].attr,ptr+1,len);
        //delete \n
        param[count].attr[len-1] = '\0';

        
#ifdef DEBUG
        printf("[%s %s]\n",param[count].attr_name, param[count].attr);
#endif

        count ++;
    }

    flock(fd, LOCK_UN);

    if(line)
        free(line);

    return 0;
}

/*
 *ubmc_ctrl_param_init and  are mutually exclusive
 */

void ubmc_ctrl_param_init(FILE * fp)
{
    int count = 0;
#ifdef DEBUG
    printf("ubmc_ctrl_param_init start\n");
#endif

    for(count =0; count < PARAM_ATTR_POS_MAX; count++)
    {
        ubmc_ctrl_set_param_name(count); 
        ubmc_ctrl_set_param_value(count,PARAM_STATUS_POS_NULL); 
    }

    ubmc_ctrl_param_sync_file(fp);

#ifdef DEBUG
    printf("ubmc_ctrl_param_init end\n");
#endif
    
}

void ubmc_ctrl_set_operation_type(param_attr_pos pos)
{
    param_status_pos status_pos;

    switch(pos)
    {
        case PARAM_ATTR_POS_POWER_ON :
            status_pos = PARAM_STATUS_POS_POWER_ON;
            break;
        case PARAM_ATTR_POS_POWER_OFF :
            status_pos = PARAM_STATUS_POS_POWER_OFF;
            break;
        case PARAM_ATTR_POS_FORCE_POWER_OFF :
            status_pos = PARAM_STATUS_POS_FORCE_POWER_OFF;
            break;
        case PARAM_ATTR_POS_RESET :
            status_pos = PARAM_STATUS_POS_RESET;
            break;
        case PARAM_ATTR_POS_UPGRADE :
            status_pos = PARAM_STATUS_POS_UPGRADE;
            break;
        case PARAM_ATTR_POS_CYCLE :
            status_pos = PARAM_STATUS_POS_CYCLE;
            break;
        default:
            status_pos = PARAM_STATUS_POS_NULL;
            break;
    }

    ubmc_ctrl_set_param_value(PARAM_ATTR_POS_TYPE, status_pos);
}

param_attr_pos ubmc_ctrl_get_operation_type_pos()
{
    int count ,pos,len;
    char tmp_buf[param_attr_size];
    char *str;

    ubmc_ctrl_get_param_value(tmp_buf, param_attr_size, PARAM_ATTR_POS_TYPE);

    /*
     *skip char space
     */
    len = strlen(tmp_buf) >= param_attr_size ? param_attr_size : strlen(tmp_buf);
    for(count =0 ;count < len;count++)
    {
        if(tmp_buf[count] != 0x20)
        {
            str = tmp_buf + count;
            break;
        }
    }

    for(count = PARAM_STATUS_POS_POWER_ON; count < PARAM_STATUS_POS_NULL; count++)
    {
        if(!strncmp(str, attr_value[count], param_attr_size))
            break;
    }

    switch(count)
    {
        case PARAM_STATUS_POS_POWER_ON :
            pos = PARAM_ATTR_POS_POWER_ON;
            break;
        case PARAM_STATUS_POS_POWER_OFF :
            pos = PARAM_ATTR_POS_POWER_OFF;
            break;
        case PARAM_STATUS_POS_FORCE_POWER_OFF :
            pos = PARAM_ATTR_POS_FORCE_POWER_OFF;
            break;
        case PARAM_STATUS_POS_RESET :
            pos = PARAM_ATTR_POS_RESET;
            break;
        case PARAM_STATUS_POS_UPGRADE :
            pos = PARAM_ATTR_POS_UPGRADE;
            break;
        case PARAM_STATUS_POS_CYCLE :
            pos = PARAM_ATTR_POS_CYCLE;
            break;
        default:
            //this mean unvalid value
            pos = PARAM_ATTR_POS_MAX;
            break;
    }

    return pos;
}

#if 0
int ubmc_ctrl_test()
{
    FILE *fp;

#if 1
    fp = fopen(ubmc_ctrl_status_file,"w+");
    ubmc_ctrl_param_init(fp);
    ubmc_ctrl_set_param_value(PARAM_ATTR_POS_TYPE,PARAM_STATUS_POS_FORCE_POWER_OFF);
    ubmc_ctrl_set_param_value(PARAM_ATTR_POS_FORCE_POWER_OFF,PARAM_STATUS_POS_ERROR);
    ubmc_ctrl_set_param_value(PARAM_ATTR_POS_UPGRADE,PARAM_STATUS_POS_IN_PROCESS);
    ubmc_ctrl_param_sync_file(fp);
    int pos =ubmc_ctrl_get_operation_type_pos();
    printf("pos %d\n",pos);
#else
    fp = fopen(ubmc_ctrl_status_file,"r");
    ubmc_ctrl_file_sync_param(fp);
#endif

    return 0;
}
#endif
