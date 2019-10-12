/*************************************************************************
	> File Name: gpio_enum.h
	> Author: allen
	> Mail: allen.zhou@net-perf.com
	> Created Time: Wed 09 May 2018 04:51:10 PM HKT
 ************************************************************************/

#ifndef _GPIO_ENUM_H
#define _GPIO_ENUM_H
#include "ubmc_gpio.h"

#define  GPIO_0_BASE  (0 * 32)  
#define  GPIO_1_BASE  (1 * 32)
#define  GPIO_2_BASE  (2 * 32)
#define  GPIO_3_BASE  (3 * 32)

#define PIN_NO_SPI_HOST_MUX_EN         (GPIO_0_BASE + 27)
#define PIN_NO_BMC_PWRBTN_OUT_N        (GPIO_0_BASE + 29)
#define PIN_NO_HOST_FRU_EEPROM_MUX_SEL (GPIO_0_BASE + 31)
#define PIN_NO_HOST_PLTRST_N           (GPIO_1_BASE + 12)
#define PIN_NO_HOST_S45_N              (GPIO_1_BASE + 13)
#define PIN_NO_HOST_S3_N               (GPIO_1_BASE + 15)
#define PIN_NO_RS232_CABLE_DETECTED    (GPIO_1_BASE + 23)
#define PIN_NO_FP_RSTBTN_N             (GPIO_1_BASE + 24) 
#define PIN_NO_FP_PWRBTN_N             (GPIO_1_BASE + 25)
#define PIN_NO_BMC_FP_BUTTON_OVERRIDE  (GPIO_2_BASE + 23)
#define PIN_NO_BMC_RSTBTN_OUT_N        (GPIO_2_BASE + 25)

#define    GPIO_DIRECTION_SPI_HOST_MUX_EN          GPIO_OUT
#define    GPIO_DIRECTION_BMC_PWRBTN_OUT_N         GPIO_OUT
#define    GPIO_DIRECTION_HOST_FRU_EEPROM_MUX_SEL  GPIO_OUT
#define    GPIO_DIRECTION_HOST_PLTRST_N            GPIO_IN  
#define    GPIO_DIRECTION_HOST_S45_N               GPIO_IN
#define    GPIO_DIRECTION_HOST_S3_N                GPIO_IN
#define    GPIO_DIRECTION_RS232_CABLE_DETECTED     GPIO_IN
#define    GPIO_DIRECTION_FP_RSTBTN_N              GPIO_IN
#define    GPIO_DIRECTION_FP_PWRBTN_N              GPIO_IN
#define    GPIO_DIRECTION_BMC_FP_BUTTON_OVERRIDE   GPIO_OUT
#define    GPIO_DIRECTION_BMC_RSTBTN_OUT_N         GPIO_OUT

#define    SLEEP_TIME_MS_POWER_OFF       (3500) //3500 ms
#define    SLEEP_TIME_MS_RESET           (3500)
#define    SLEEP_TIME_MS_MAX             (9000)
#define    SLEEP_TIME_MS_LONG_MAX      (15000)

typedef enum _gpio_pin_pos{
    GPIO_POS_SPI_HOST_MUX_EN =0,
    GPIO_POS_BMC_PWRBTN_OUT_N,
    GPIO_POS_HOST_FRU_EEPROM_MUX_SEL,
    GPIO_POS_HOST_PLTRST_N,
    GPIO_POS_HOST_S45_N,
    GPIO_POS_HOST_S3_N,
    GPIO_POS_RS232_CABLE_DETECTED,
    GPIO_POS_FP_RSTBTN_N,
    GPIO_POS_FP_PWRBTN_N,
    GPIO_POS_BMC_FP_BUTTON_OVERRIDE,
    GPIO_POS_BMC_RSTBTN_OUT_N,
    GPIO_POS_MAX,
}gpio_pin_pos;

enum{
    GPIO_OP_FAIL = -1,
    GPIO_OP_SUCCESS = 0,
};

typedef enum _host_power_status{
    HOST_POWER_DOWN =0,
    HOST_POWER_UP ,
}host_power_status;

enum{
    HOST_SERIAL_DISCONNECTED =0,    
    HOST_SERIAL_CONNECTED,    
};

typedef enum{
    ACTION_OP_POWER_ON,
    ACTION_OP_POWER_OFF,
    ACTION_OP_FORCE_POWER_OFF,
    ACTION_OP_CYCLE,
    ACTION_OP_RESET,
    ACTION_OP_UPGRADE,
    ACTION_OP_PROCESS_STATUS,
    ACTION_OP_SERIAL_STATUS,
    ACTION_OP_POWER_STATUS,
    ACTION_OP_MAX,
}action_op_type;

#endif
