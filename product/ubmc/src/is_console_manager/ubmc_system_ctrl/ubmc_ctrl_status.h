/*************************************************************************
	> File Name: operate_status.h
	> Author: allen
	> Mail: allen.zhou@net-perf.com
	> Created Time: Wed 16 May 2018 10:26:36 AM HKT
 ************************************************************************/

#ifndef _UBMC_CTRL_STATUS_H
#define _UBMC_CTRL_STATUS_H

//#define  ubmcctrl_status_file "./gpio_operate.status"
#define  param_attr_size (64)
typedef struct {
    char attr_name[32];
    char attr[param_attr_size];
}ubmc_ctrl_status;

typedef enum{
    PARAM_ATTR_POS_POWER_ON =0,
    PARAM_ATTR_POS_POWER_OFF,
    PARAM_ATTR_POS_FORCE_POWER_OFF,
    PARAM_ATTR_POS_RESET,
    PARAM_ATTR_POS_UPGRADE,
    PARAM_ATTR_POS_CYCLE,
    PARAM_ATTR_POS_TYPE,
    PARAM_ATTR_POS_MAX,
} param_attr_pos;

typedef enum{
    PARAM_STATUS_POS_IN_PROCESS,
    PARAM_STATUS_POS_OK,
    PARAM_STATUS_POS_ERROR,
    PARAM_STATUS_POS_POWER_ON,
    PARAM_STATUS_POS_POWER_OFF,
    PARAM_STATUS_POS_FORCE_POWER_OFF,
    PARAM_STATUS_POS_RESET,
    PARAM_STATUS_POS_UPGRADE,
    PARAM_STATUS_POS_CYCLE,
    PARAM_STATUS_POS_NULL,
    PARAM_STATUS_POS_MAX,
}param_status_pos;

void ubmc_ctrl_param_init(FILE * fp);
void ubmc_ctrl_set_operation_type(param_attr_pos pos);
void ubmc_ctrl_set_param_name(param_attr_pos pos);
void ubmc_ctrl_set_param_value(param_attr_pos pos, param_status_pos value_pos);
void ubmc_ctrl_get_param_value(char * buf ,int len ,param_attr_pos pos);
int ubmc_ctrl_param_sync_file(FILE *fp);
int ubmc_ctrl_file_sync_param(FILE * fp);
param_attr_pos ubmc_ctrl_get_operation_type_pos(void);
int ubmc_ctrl_test(void);

#endif
