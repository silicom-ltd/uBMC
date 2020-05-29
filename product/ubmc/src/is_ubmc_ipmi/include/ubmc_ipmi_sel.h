/*
 * ubmc_ipmi_sel.h
 *
 *  Created on: Jul 27, 2018
 *      Author: jason_zhang
 */

#ifndef INCLUDE_UBMC_IPMI_SEL_H_
#define INCLUDE_UBMC_IPMI_SEL_H_

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/poll.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ubmc_ipmi_sel_sensor.h"
#include "ubmc_ipmi_board.h"
#define AM335X_GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))
//#define UBMC_IPMI_DEBUG_TEST
//#define UBMC_IPMI_SENSOR_TEST
//For monitor PSU for rack mounted
#define PSU_LEFT_PWRGD_GPIO_BANK 	0        //power off
#define PSU_LEFT_PWRGD_GPIO_PIN 	23
#define PSU_RIGHT_PWRGD_GPIO_BANK 	0        //power off
#define PSU_RIGHT_PWRGD_GPIO_PIN 	26
//Host PowerUP 1, Host Power Down 0
#define HOST_S45_GPIO_BANK		    1
#define HOST_S45_GPIO_PIN			13
//Detected suspend, but host ubuntu rebooted, didn't resume
#define HOST_S3_GPIO_BANK			1
#define HOST_S3_GPIO_PIN			15
//NMI? Log this event
#define HOST_ERROR_GPIO_BANK		1
#define HOST_ERROR_GPIO_PIN			29
//Warning, throttle
#define HOST_PROCHOT_GPIO_BANK		2
#define HOST_PROCHOT_GPIO_PIN		1
//CPU will shutdown. Log this event
#define HOST_THERMTRIP_GPIO_BANK	2
#define HOST_THERMTRIP_GPIO_PIN		2
//Detected host reset
#define HOST_PLTRST_GPIO_BANK		1
#define HOST_PLTRST_GPIO_PIN		12

#ifdef UBMC_IPMI_DEBUG_TEST
//for test
#define TEST_GPIO "FP_PWRBTN_N"
#define TEST_GPIO_BANK 1	  //button down
#define TEST_GPIO_PIN 25
#endif
#define SYS_GPIO_PATH "/sys/class/gpio"
#define SYS_GPIO_EXPORT_PATH "/sys/class/gpio/export"

//#define UBMC_IPMI_DEBUG 1
#ifdef  UBMC_IPMI_DEBUG
#define UBMC_IPMI_DEBUG_LINE() printf("[%s:%s] line=%d\r\n",__FILE__, __func__, __LINE__)
//#define DEBUG_ERR(fmt, args...) printf("\033[46;31m[%s:%d]\033[0m "#fmt" errno=%d, %m\r\n", __func__, __LINE__, ##args, errno, errno)
#define UBMC_IPMI_DEBUG_INFO(fmt, args...) syslog(LOG_NOTICE, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##args)
#define UBMC_IPMI_DEBUG_ERR(fmt, args...) printf("[ERROR][%s:%s] line=%d "#fmt"\r\n",__FILE__, __func__, __LINE__)
#else
#define UBMC_IPMI_DEBUG_LINE()
#define UBMC_IPMI_DEBUG_ERR(fmt, args...)
#define UBMC_IPMI_DEBUG_INFO(fmt, ...)
#endif

#define UBMC_IPMI_ERR(fmt, args...) printf("[ERROR][%s:%s] line=%d "#fmt"\r\n",__FILE__, __func__, __LINE__)
#define UBMC_IPMI_ERRLOG(fmt, args...) syslog(LOG_ERR, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##args)
#define UBMC_IPMI_LOG(fmt, args...) printf("\033[33m[%s:%d]\033[0m "#fmt"\r\n", __func__, __LINE__, ##args)
#define UBMC_IPMI_NOTE(fmt, args...) syslog(LOG_NOTICE, "[File: %s,Line: %05d, FUNCTION: %s]"fmt"\n",__FILE__, __LINE__,__FUNCTION__, ##args)

//copy from ipmitool ipmi_sel.h
#define IPMI_CMD_GET_SEL_INFO		0x40
#define IPMI_CMD_GET_SEL_ALLOC_INFO	0x41
#define IPMI_CMD_RESERVE_SEL		0x42
#define IPMI_CMD_GET_SEL_ENTRY		0x43
#define IPMI_CMD_ADD_SEL_ENTRY		0x44
#define IPMI_CMD_PARTIAL_ADD_SEL_ENTRY	0x45
#define IPMI_CMD_DELETE_SEL_ENTRY	0x46
#define IPMI_CMD_CLEAR_SEL		0x47
#define IPMI_CMD_GET_SEL_TIME		0x48
#define IPMI_CMD_SET_SEL_TIME		0x49
#define IPMI_CMD_GET_AUX_LOG_STATUS	0x5A
#define IPMI_CMD_SET_AUX_LOG_STATUS	0x5B

enum {
	IPMI_EVENT_CLASS_DISCRETE,
	IPMI_EVENT_CLASS_DIGITAL,
	IPMI_EVENT_CLASS_THRESHOLD,
	IPMI_EVENT_CLASS_OEM,
};

#ifdef HAVE_PRAGMA_PACK
#pragma pack(1)
#endif
struct sel_get_rq {
	uint16_t	reserve_id;
	uint16_t	record_id;
	uint8_t	offset;
	uint8_t	length;
} ATTRIBUTE_PACKING;
#ifdef HAVE_PRAGMA_PACK
#pragma pack(0)
#endif

struct standard_spec_sel_rec{
	uint32_t	timestamp;
	uint16_t	gen_id;
	uint8_t	evm_rev;
	uint8_t	sensor_type;
	uint8_t	sensor_num;
#if WORDS_BIGENDIAN
	uint8_t	event_dir  : 1;
	uint8_t	event_type : 7;
#else
	uint8_t	event_type : 7;
	uint8_t	event_dir  : 1;
#endif
#define DATA_BYTE2_SPECIFIED_MASK	0xc0    /* event_data[0] bit mask */
#define DATA_BYTE3_SPECIFIED_MASK	0x30    /* event_data[0] bit mask */
#define EVENT_OFFSET_MASK		0x0f    /* event_data[0] bit mask */
	uint8_t	event_data[3];
};
/* Dell Specific MACRO's */
#define	OEM_CODE_IN_BYTE2		0x80	  /* Dell specific OEM Byte in Byte 2 Mask */
#define	OEM_CODE_IN_BYTE3		0x20	  /* Dell specific OEM Byte in Byte 3 Mask */
/* MASK MACROS */
#define	MASK_LOWER_NIBBLE		0x0F
#define	MASK_HIGHER_NIBBLE		0xF0
/*Senosr type Macro's */
#define	SENSOR_TYPE_MEMORY		0x0C
#define	SENSOR_TYPE_CRIT_INTR		0x13
#define	SENSOR_TYPE_EVT_LOG		0x10
#define	SENSOR_TYPE_SYS_EVENT		0x12
#define	SENSOR_TYPE_PROCESSOR		0x07
#define	SENSOR_TYPE_OEM_SEC_EVENT	0xC1
#define SENSOR_TYPE_VER_CHANGE		0x2B
#define	SENSOR_TYPE_FRM_PROG		0x0F
#define	SENSOR_TYPE_WTDOG		0x23
#define	SENSOR_TYPE_OEM_NFATAL_ERROR	0xC2
#define	SENSOR_TYPE_OEM_FATAL_ERROR	0xC3
#define SENSOR_TYPE_TXT_CMD_ERROR	0x20
#define SENSOR_TYPE_SUPERMICRO_OEM 0xD0
/* End of Macro for DELL Specific */
#define SEL_OEM_TS_DATA_LEN		6
#define SEL_OEM_NOTS_DATA_LEN		13
struct oem_ts_spec_sel_rec{
	uint32_t timestamp;
	uint8_t manf_id[3];
	uint8_t	oem_defined[SEL_OEM_TS_DATA_LEN];
};

struct oem_nots_spec_sel_rec{
	uint8_t oem_defined[SEL_OEM_NOTS_DATA_LEN];
};


struct sel_event_record {
	uint16_t	record_id;
	uint8_t	record_type;
	union{
		struct standard_spec_sel_rec standard_type;
		struct oem_ts_spec_sel_rec oem_ts_type;
		struct oem_nots_spec_sel_rec oem_nots_type;
	} sel_type;
};

struct ipmi_event_sensor_types {
	uint8_t	code;
	uint8_t	offset;
#define ALL_OFFSETS_SPECIFIED  0xff
	uint8_t   data;
	const char	* desc;
};

static const struct ipmi_event_sensor_types generic_event_types[] = {
    /* Threshold Based States */
    { 0x01, 0x00, 0xff, "Lower Non-critical going low " },
    { 0x01, 0x01, 0xff, "Lower Non-critical going high" },
    { 0x01, 0x02, 0xff, "Lower Critical going low " },
    { 0x01, 0x03, 0xff, "Lower Critical going high" },
    { 0x01, 0x04, 0xff, "Lower Non-recoverable going low " },
    { 0x01, 0x05, 0xff, "Lower Non-recoverable going high" },
    { 0x01, 0x06, 0xff, "Upper Non-critical going low " },
    { 0x01, 0x07, 0xff, "Upper Non-critical going high" },
    { 0x01, 0x08, 0xff, "Upper Critical going low " },
    { 0x01, 0x09, 0xff, "Upper Critical going high" },
    { 0x01, 0x0a, 0xff, "Upper Non-recoverable going low " },
    { 0x01, 0x0b, 0xff, "Upper Non-recoverable going high" },
    /* DMI-based "usage state" States */
    { 0x02, 0x00, 0xff, "Transition to Idle" },
    { 0x02, 0x01, 0xff, "Transition to Active" },
    { 0x02, 0x02, 0xff, "Transition to Busy" },
    /* Digital-Discrete Event States */
    { 0x03, 0x00, 0xff, "State Deasserted" },
    { 0x03, 0x01, 0xff, "State Asserted" },
    { 0x04, 0x00, 0xff, "Predictive Failure Deasserted" },
    { 0x04, 0x01, 0xff, "Predictive Failure Asserted" },
    { 0x05, 0x00, 0xff, "Limit Not Exceeded" },
    { 0x05, 0x01, 0xff, "Limit Exceeded" },
    { 0x06, 0x00, 0xff, "Performance Met" },
    { 0x06, 0x01, 0xff, "Performance Lags" },
    /* Severity Event States */
    { 0x07, 0x00, 0xff, "Transition to OK" },
    { 0x07, 0x01, 0xff, "Transition to Non-critical from OK" },
    { 0x07, 0x02, 0xff, "Transition to Critical from less severe" },
    { 0x07, 0x03, 0xff, "Transition to Non-recoverable from less severe" },
    { 0x07, 0x04, 0xff, "Transition to Non-critical from more severe" },
    { 0x07, 0x05, 0xff, "Transition to Critical from Non-recoverable" },
    { 0x07, 0x06, 0xff, "Transition to Non-recoverable" },
    { 0x07, 0x07, 0xff, "Monitor" },
    { 0x07, 0x08, 0xff, "Informational" },
    /* Availability Status States */
    { 0x08, 0x00, 0xff, "Device Absent" },
    { 0x08, 0x01, 0xff, "Device Present" },
    { 0x09, 0x00, 0xff, "Device Disabled" },
    { 0x09, 0x01, 0xff, "Device Enabled" },
    { 0x0a, 0x00, 0xff, "Transition to Running" },
    { 0x0a, 0x01, 0xff, "Transition to In Test" },
    { 0x0a, 0x02, 0xff, "Transition to Power Off" },
    { 0x0a, 0x03, 0xff, "Transition to On Line" },
    { 0x0a, 0x04, 0xff, "Transition to Off Line" },
    { 0x0a, 0x05, 0xff, "Transition to Off Duty" },
    { 0x0a, 0x06, 0xff, "Transition to Degraded" },
    { 0x0a, 0x07, 0xff, "Transition to Power Save" },
    { 0x0a, 0x08, 0xff, "Install Error" },
    /* Redundancy States */
    { 0x0b, 0x00, 0xff, "Fully Redundant" },
    { 0x0b, 0x01, 0xff, "Redundancy Lost" },
    { 0x0b, 0x02, 0xff, "Redundancy Degraded" },
    { 0x0b, 0x03, 0xff, "Non-Redundant: Sufficient from Redundant" },
    { 0x0b, 0x04, 0xff, "Non-Redundant: Sufficient from Insufficient" },
    { 0x0b, 0x05, 0xff, "Non-Redundant: Insufficient Resources" },
    { 0x0b, 0x06, 0xff, "Redundancy Degraded from Fully Redundant" },
    { 0x0b, 0x07, 0xff, "Redundancy Degraded from Non-Redundant" },
    /* ACPI Device Power States */
    { 0x0c, 0x00, 0xff, "D0 Power State" },
    { 0x0c, 0x01, 0xff, "D1 Power State" },
    { 0x0c, 0x02, 0xff, "D2 Power State" },
    { 0x0c, 0x03, 0xff, "D3 Power State" },
    /* END */
    { 0x00, 0x00, 0xff, NULL },
};

static const struct ipmi_event_sensor_types sensor_specific_event_types[] = {
    /* Physical Security */
    { 0x05, 0x00, 0xff, "General Chassis intrusion" },
    { 0x05, 0x01, 0xff, "Drive Bay intrusion" },
    { 0x05, 0x02, 0xff, "I/O Card area intrusion" },
    { 0x05, 0x03, 0xff, "Processor area intrusion" },
    { 0x05, 0x04, 0xff, "System unplugged from LAN" },
    { 0x05, 0x05, 0xff, "Unauthorized dock" },
    { 0x05, 0x06, 0xff, "FAN area intrusion" },
    /* Platform Security */
    { 0x06, 0x00, 0xff, "Front Panel Lockout violation attempted" },
    { 0x06, 0x01, 0xff, "Pre-boot password violation - user password" },
    { 0x06, 0x02, 0xff, "Pre-boot password violation - setup password" },
    { 0x06, 0x03, 0xff, "Pre-boot password violation - network boot password" },
    { 0x06, 0x04, 0xff, "Other pre-boot password violation" },
    { 0x06, 0x05, 0xff, "Out-of-band access password violation" },
    /* Processor */
    { 0x07, 0x00, 0xff, "IERR" },
    { 0x07, 0x01, 0xff, "Thermal Trip" },
    { 0x07, 0x02, 0xff, "FRB1/BIST failure" },
    { 0x07, 0x03, 0xff, "FRB2/Hang in POST failure" },
    { 0x07, 0x04, 0xff, "FRB3/Processor startup/init failure" },
    { 0x07, 0x05, 0xff, "Configuration Error" },
    { 0x07, 0x06, 0xff, "SM BIOS Uncorrectable CPU-complex Error" },
    { 0x07, 0x07, 0xff, "Presence detected" },
    { 0x07, 0x08, 0xff, "Disabled" },
    { 0x07, 0x09, 0xff, "Terminator presence detected" },
    { 0x07, 0x0a, 0xff, "Throttled" },
    { 0x07, 0x0b, 0xff, "Uncorrectable machine check exception" },
    { 0x07, 0x0c, 0xff, "Correctable machine check error" },
    /* Power Supply */
    { 0x08, 0x00, 0xff, "Presence detected" },
    { 0x08, 0x01, 0xff, "Failure detected" },
    { 0x08, 0x02, 0xff, "Predictive failure" },
    { 0x08, 0x03, 0xff, "Power Supply AC lost" },
    { 0x08, 0x04, 0xff, "AC lost or out-of-range" },
    { 0x08, 0x05, 0xff, "AC out-of-range, but present" },
    { 0x08, 0x06, 0x00, "Config Error: Vendor Mismatch" },
    { 0x08, 0x06, 0x01, "Config Error: Revision Mismatch" },
    { 0x08, 0x06, 0x02, "Config Error: Processor Missing" },
    { 0x08, 0x06, 0x03, "Config Error: Power Supply Rating Mismatch" },
    { 0x08, 0x06, 0x04, "Config Error: Voltage Rating Mismatch" },
    { 0x08, 0x06, 0xff, "Config Error" },
    { 0x08, 0x06, 0xff, "Power Supply Inactive" },
    /* Power Unit */
    { 0x09, 0x00, 0xff, "Power off/down" },
    { 0x09, 0x01, 0xff, "Power cycle" },
    { 0x09, 0x02, 0xff, "240VA power down" },
    { 0x09, 0x03, 0xff, "Interlock power down" },
    { 0x09, 0x04, 0xff, "AC lost" },
    { 0x09, 0x05, 0xff, "Soft-power control failure" },
    { 0x09, 0x06, 0xff, "Failure detected" },
    { 0x09, 0x07, 0xff, "Predictive failure" },
    /* Memory */
    { 0x0c, 0x00, 0xff, "Correctable ECC" },
    { 0x0c, 0x01, 0xff, "Uncorrectable ECC" },
    { 0x0c, 0x02, 0xff, "Parity" },
    { 0x0c, 0x03, 0xff, "Memory Scrub Failed" },
    { 0x0c, 0x04, 0xff, "Memory Device Disabled" },
    { 0x0c, 0x05, 0xff, "Correctable ECC logging limit reached" },
    { 0x0c, 0x06, 0xff, "Presence Detected" },
    { 0x0c, 0x07, 0xff, "Configuration Error" },
    { 0x0c, 0x08, 0xff, "Spare" },
    { 0x0c, 0x09, 0xff, "Throttled" },
    { 0x0c, 0x0a, 0xff, "Critical Overtemperature" },
    /* Drive Slot */
    { 0x0d, 0x00, 0xff, "Drive Present" },
    { 0x0d, 0x01, 0xff, "Drive Fault" },
    { 0x0d, 0x02, 0xff, "Predictive Failure" },
    { 0x0d, 0x03, 0xff, "Hot Spare" },
    { 0x0d, 0x04, 0xff, "Parity Check In Progress" },
    { 0x0d, 0x05, 0xff, "In Critical Array" },
    { 0x0d, 0x06, 0xff, "In Failed Array" },
    { 0x0d, 0x07, 0xff, "Rebuild In Progress" },
    { 0x0d, 0x08, 0xff, "Rebuild Aborted" },
    /* System Firmware Error */
    { 0x0f, 0x00, 0x00, "Unspecified" },
    { 0x0f, 0x00, 0x01, "No system memory installed" },
    { 0x0f, 0x00, 0x02, "No usable system memory" },
    { 0x0f, 0x00, 0x03, "Unrecoverable IDE device failure" },
    { 0x0f, 0x00, 0x04, "Unrecoverable system-board failure" },
    { 0x0f, 0x00, 0x05, "Unrecoverable diskette failure" },
    { 0x0f, 0x00, 0x06, "Unrecoverable hard-disk controller failure" },
    { 0x0f, 0x00, 0x07, "Unrecoverable PS/2 or USB keyboard failure" },
    { 0x0f, 0x00, 0x08, "Removable boot media not found" },
    { 0x0f, 0x00, 0x09, "Unrecoverable video controller failure" },
    { 0x0f, 0x00, 0x0a, "No video device selected" },
    { 0x0f, 0x00, 0x0b, "BIOS corruption detected" },
    { 0x0f, 0x00, 0x0c, "CPU voltage mismatch" },
    { 0x0f, 0x00, 0x0d, "CPU speed mismatch failure" },
    { 0x0f, 0x00, 0xff, "Unknown Error" },
    /* System Firmware Hang */
    { 0x0f, 0x01, 0x00, "Unspecified" },
    { 0x0f, 0x01, 0x01, "Memory initialization" },
    { 0x0f, 0x01, 0x02, "Hard-disk initialization" },
    { 0x0f, 0x01, 0x03, "Secondary CPU Initialization" },
    { 0x0f, 0x01, 0x04, "User authentication" },
    { 0x0f, 0x01, 0x05, "User-initiated system setup" },
    { 0x0f, 0x01, 0x06, "USB resource configuration" },
    { 0x0f, 0x01, 0x07, "PCI resource configuration" },
    { 0x0f, 0x01, 0x08, "Option ROM initialization" },
    { 0x0f, 0x01, 0x09, "Video initialization" },
    { 0x0f, 0x01, 0x0a, "Cache initialization" },
    { 0x0f, 0x01, 0x0b, "SMBus initialization" },
    { 0x0f, 0x01, 0x0c, "Keyboard controller initialization" },
    { 0x0f, 0x01, 0x0d, "Management controller initialization" },
    { 0x0f, 0x01, 0x0e, "Docking station attachment" },
    { 0x0f, 0x01, 0x0f, "Enabling docking station" },
    { 0x0f, 0x01, 0x10, "Docking station ejection" },
    { 0x0f, 0x01, 0x11, "Disabling docking station" },
    { 0x0f, 0x01, 0x12, "Calling operating system wake-up vector" },
    { 0x0f, 0x01, 0x13, "System boot initiated" },
    { 0x0f, 0x01, 0x14, "Motherboard initialization" },
    { 0x0f, 0x01, 0x15, "reserved" },
    { 0x0f, 0x01, 0x16, "Floppy initialization" },
    { 0x0f, 0x01, 0x17, "Keyboard test" },
    { 0x0f, 0x01, 0x18, "Pointing device test" },
    { 0x0f, 0x01, 0x19, "Primary CPU initialization" },
    { 0x0f, 0x01, 0xff, "Unknown Hang" },
    /* System Firmware Progress */
    { 0x0f, 0x02, 0x00, "Unspecified" },
    { 0x0f, 0x02, 0x01, "Memory initialization" },
    { 0x0f, 0x02, 0x02, "Hard-disk initialization" },
    { 0x0f, 0x02, 0x03, "Secondary CPU Initialization" },
    { 0x0f, 0x02, 0x04, "User authentication" },
    { 0x0f, 0x02, 0x05, "User-initiated system setup" },
    { 0x0f, 0x02, 0x06, "USB resource configuration" },
    { 0x0f, 0x02, 0x07, "PCI resource configuration" },
    { 0x0f, 0x02, 0x08, "Option ROM initialization" },
    { 0x0f, 0x02, 0x09, "Video initialization" },
    { 0x0f, 0x02, 0x0a, "Cache initialization" },
    { 0x0f, 0x02, 0x0b, "SMBus initialization" },
    { 0x0f, 0x02, 0x0c, "Keyboard controller initialization" },
    { 0x0f, 0x02, 0x0d, "Management controller initialization" },
    { 0x0f, 0x02, 0x0e, "Docking station attachment" },
    { 0x0f, 0x02, 0x0f, "Enabling docking station" },
    { 0x0f, 0x02, 0x10, "Docking station ejection" },
    { 0x0f, 0x02, 0x11, "Disabling docking station" },
    { 0x0f, 0x02, 0x12, "Calling operating system wake-up vector" },
    { 0x0f, 0x02, 0x13, "System boot initiated" },
    { 0x0f, 0x02, 0x14, "Motherboard initialization" },
    { 0x0f, 0x02, 0x15, "reserved" },
    { 0x0f, 0x02, 0x16, "Floppy initialization" },
    { 0x0f, 0x02, 0x17, "Keyboard test" },
    { 0x0f, 0x02, 0x18, "Pointing device test" },
    { 0x0f, 0x02, 0x19, "Primary CPU initialization" },
    { 0x0f, 0x02, 0xff, "Unknown Progress" },
    /* Event Logging Disabled */
    { 0x10, 0x00, 0xff, "Correctable memory error logging disabled" },
    { 0x10, 0x01, 0xff, "Event logging disabled" },
    { 0x10, 0x02, 0xff, "Log area reset/cleared" },
    { 0x10, 0x03, 0xff, "All event logging disabled" },
    { 0x10, 0x04, 0xff, "Log full" },
    { 0x10, 0x05, 0xff, "Log almost full" },
    /* Watchdog 1 */
    { 0x11, 0x00, 0xff, "BIOS Reset" },
    { 0x11, 0x01, 0xff, "OS Reset" },
    { 0x11, 0x02, 0xff, "OS Shut Down" },
    { 0x11, 0x03, 0xff, "OS Power Down" },
    { 0x11, 0x04, 0xff, "OS Power Cycle" },
    { 0x11, 0x05, 0xff, "OS NMI/Diag Interrupt" },
    { 0x11, 0x06, 0xff, "OS Expired" },
    { 0x11, 0x07, 0xff, "OS pre-timeout Interrupt" },
    /* System Event */
    { 0x12, 0x00, 0xff, "System Reconfigured" },
    { 0x12, 0x01, 0xff, "OEM System boot event" },
    { 0x12, 0x02, 0xff, "Undetermined system hardware failure" },
    { 0x12, 0x03, 0xff, "Entry added to auxiliary log" },
    { 0x12, 0x04, 0xff, "PEF Action" },
    { 0x12, 0x05, 0xff, "Timestamp Clock Sync" },
    /* Critical Interrupt */
    { 0x13, 0x00, 0xff, "NMI/Diag Interrupt" },
    { 0x13, 0x01, 0xff, "Bus Timeout" },
    { 0x13, 0x02, 0xff, "I/O Channel check NMI" },
    { 0x13, 0x03, 0xff, "Software NMI" },
    { 0x13, 0x04, 0xff, "PCI PERR" },
    { 0x13, 0x05, 0xff, "PCI SERR" },
    { 0x13, 0x06, 0xff, "EISA failsafe timeout" },
    { 0x13, 0x07, 0xff, "Bus Correctable error" },
    { 0x13, 0x08, 0xff, "Bus Uncorrectable error" },
    { 0x13, 0x09, 0xff, "Fatal NMI" },
    { 0x13, 0x0a, 0xff, "Bus Fatal Error" },
    { 0x13, 0x0b, 0xff, "Bus Degraded" },
    /* Button */
    { 0x14, 0x00, 0xff, "Power Button pressed" },
    { 0x14, 0x01, 0xff, "Sleep Button pressed" },
    { 0x14, 0x02, 0xff, "Reset Button pressed" },
    { 0x14, 0x03, 0xff, "FRU Latch" },
    { 0x14, 0x04, 0xff, "FRU Service" },
    /* Chip Set */
    { 0x19, 0x00, 0xff, "Soft Power Control Failure" },
    { 0x19, 0x01, 0xff, "Thermal Trip" },
    /* Cable/Interconnect */
    { 0x1b, 0x00, 0xff, "Connected" },
    { 0x1b, 0x01, 0xff, "Config Error" },
    /* System Boot Initiated */
    { 0x1d, 0x00, 0xff, "Initiated by power up" },
    { 0x1d, 0x01, 0xff, "Initiated by hard reset" },
    { 0x1d, 0x02, 0xff, "Initiated by warm reset" },
    { 0x1d, 0x03, 0xff, "User requested PXE boot" },
    { 0x1d, 0x04, 0xff, "Automatic boot to diagnostic" },
    { 0x1d, 0x05, 0xff, "OS initiated hard reset" },
    { 0x1d, 0x06, 0xff, "OS initiated warm reset" },
    { 0x1d, 0x07, 0xff, "System Restart" },
    /* Boot Error */
    { 0x1e, 0x00, 0xff, "No bootable media" },
    { 0x1e, 0x01, 0xff, "Non-bootable disk in drive" },
    { 0x1e, 0x02, 0xff, "PXE server not found" },
    { 0x1e, 0x03, 0xff, "Invalid boot sector" },
    { 0x1e, 0x04, 0xff, "Timeout waiting for selection" },
    /* OS Boot */
    { 0x1f, 0x00, 0xff, "A: boot completed" },
    { 0x1f, 0x01, 0xff, "C: boot completed" },
    { 0x1f, 0x02, 0xff, "PXE boot completed" },
    { 0x1f, 0x03, 0xff, "Diagnostic boot completed" },
    { 0x1f, 0x04, 0xff, "CD-ROM boot completed" },
    { 0x1f, 0x05, 0xff, "ROM boot completed" },
    { 0x1f, 0x06, 0xff, "boot completed - device not specified" },
    { 0x1f, 0x07, 0xff, "Installation started" },
    { 0x1f, 0x08, 0xff, "Installation completed" },
    { 0x1f, 0x09, 0xff, "Installation aborted" },
    { 0x1f, 0x0a, 0xff, "Installation failed" },
    /* OS Stop/Shutdown */
    { 0x20, 0x00, 0xff, "Error during system startup" },
    { 0x20, 0x01, 0xff, "Run-time critical stop" },
    { 0x20, 0x02, 0xff, "OS graceful stop" },
    { 0x20, 0x03, 0xff, "OS graceful shutdown" },
    { 0x20, 0x04, 0xff, "PEF initiated soft shutdown" },
    { 0x20, 0x05, 0xff, "Agent not responding" },
    /* Slot/Connector */
    { 0x21, 0x00, 0xff, "Fault Status" },
    { 0x21, 0x01, 0xff, "Identify Status" },
    { 0x21, 0x02, 0xff, "Device Installed" },
    { 0x21, 0x03, 0xff, "Ready for Device Installation" },
    { 0x21, 0x04, 0xff, "Ready for Device Removal" },
    { 0x21, 0x05, 0xff, "Slot Power is Off" },
    { 0x21, 0x06, 0xff, "Device Removal Request" },
    { 0x21, 0x07, 0xff, "Interlock" },
    { 0x21, 0x08, 0xff, "Slot is Disabled" },
    { 0x21, 0x09, 0xff, "Spare Device" },
    /* System ACPI Power State */
    { 0x22, 0x00, 0xff, "S0/G0: working" },
    { 0x22, 0x01, 0xff, "S1: sleeping with system hw & processor context maintained" },
    { 0x22, 0x02, 0xff, "S2: sleeping, processor context lost" },
    { 0x22, 0x03, 0xff, "S3: sleeping, processor & hw context lost, memory retained" },
    { 0x22, 0x04, 0xff, "S4: non-volatile sleep/suspend-to-disk" },
    { 0x22, 0x05, 0xff, "S5/G2: soft-off" },
    { 0x22, 0x06, 0xff, "S4/S5: soft-off" },
    { 0x22, 0x07, 0xff, "G3: mechanical off" },
    { 0x22, 0x08, 0xff, "Sleeping in S1/S2/S3 state" },
    { 0x22, 0x09, 0xff, "G1: sleeping" },
    { 0x22, 0x0a, 0xff, "S5: entered by override" },
    { 0x22, 0x0b, 0xff, "Legacy ON state" },
    { 0x22, 0x0c, 0xff, "Legacy OFF state" },
    { 0x22, 0x0e, 0xff, "Unknown" },
    /* Watchdog 2 */
    { 0x23, 0x00, 0xff, "Timer expired" },
    { 0x23, 0x01, 0xff, "Hard reset" },
    { 0x23, 0x02, 0xff, "Power down" },
    { 0x23, 0x03, 0xff, "Power cycle" },
    { 0x23, 0x04, 0xff, "reserved" },
    { 0x23, 0x05, 0xff, "reserved" },
    { 0x23, 0x06, 0xff, "reserved" },
    { 0x23, 0x07, 0xff, "reserved" },
    { 0x23, 0x08, 0xff, "Timer interrupt" },
    /* Platform Alert */
    { 0x24, 0x00, 0xff, "Platform generated page" },
    { 0x24, 0x01, 0xff, "Platform generated LAN alert" },
    { 0x24, 0x02, 0xff, "Platform Event Trap generated" },
    { 0x24, 0x03, 0xff, "Platform generated SNMP trap, OEM format" },
    /* Entity Presence */
    { 0x25, 0x00, 0xff, "Present" },
    { 0x25, 0x01, 0xff, "Absent" },
    { 0x25, 0x02, 0xff, "Disabled" },
    /* LAN */
    { 0x27, 0x00, 0xff, "Heartbeat Lost" },
    { 0x27, 0x01, 0xff, "Heartbeat" },
    /* Management Subsystem Health */
    { 0x28, 0x00, 0xff, "Sensor access degraded or unavailable" },
    { 0x28, 0x01, 0xff, "Controller access degraded or unavailable" },
    { 0x28, 0x02, 0xff, "Management controller off-line" },
    { 0x28, 0x03, 0xff, "Management controller unavailable" },
    { 0x28, 0x04, 0xff, "Sensor failure" },
    { 0x28, 0x05, 0xff, "FRU failure" },
    /* Battery */
    { 0x29, 0x00, 0xff, "Low" },
    { 0x29, 0x01, 0xff, "Failed" },
    { 0x29, 0x02, 0xff, "Presence Detected" },
    /* Version Change */
    { 0x2b, 0x00, 0xff, "Hardware change detected" },
    { 0x2b, 0x01, 0x00, "Firmware or software change detected" },
    { 0x2b, 0x01, 0x01, "Firmware or software change detected, Mngmt Ctrl Dev Id" },
    { 0x2b, 0x01, 0x02, "Firmware or software change detected, Mngmt Ctrl Firm Rev" },
    { 0x2b, 0x01, 0x03, "Firmware or software change detected, Mngmt Ctrl Dev Rev" },
    { 0x2b, 0x01, 0x04, "Firmware or software change detected, Mngmt Ctrl Manuf Id" },
    { 0x2b, 0x01, 0x05, "Firmware or software change detected, Mngmt Ctrl IPMI Vers" },
    { 0x2b, 0x01, 0x06, "Firmware or software change detected, Mngmt Ctrl Aux Firm Id" },
    { 0x2b, 0x01, 0x07, "Firmware or software change detected, Mngmt Ctrl Firm Boot Block" },
    { 0x2b, 0x01, 0x08, "Firmware or software change detected, Mngmt Ctrl Other" },
    { 0x2b, 0x01, 0x09, "Firmware or software change detected, BIOS/EFI change" },
    { 0x2b, 0x01, 0x0A, "Firmware or software change detected, SMBIOS change" },
    { 0x2b, 0x01, 0x0B, "Firmware or software change detected, O/S change" },
    { 0x2b, 0x01, 0x0C, "Firmware or software change detected, O/S loader change" },
    { 0x2b, 0x01, 0x0D, "Firmware or software change detected, Service Diag change" },
    { 0x2b, 0x01, 0x0E, "Firmware or software change detected, Mngmt SW agent change" },
    { 0x2b, 0x01, 0x0F, "Firmware or software change detected, Mngmt SW App change" },
    { 0x2b, 0x01, 0x10, "Firmware or software change detected, Mngmt SW Middle" },
    { 0x2b, 0x01, 0x11, "Firmware or software change detected, Prog HW Change (FPGA)" },
    { 0x2b, 0x01, 0x12, "Firmware or software change detected, board/FRU module change" },
    { 0x2b, 0x01, 0x13, "Firmware or software change detected, board/FRU component change" },
    { 0x2b, 0x01, 0x14, "Firmware or software change detected, board/FRU replace equ ver" },
    { 0x2b, 0x01, 0x15, "Firmware or software change detected, board/FRU replace new ver" },
    { 0x2b, 0x01, 0x16, "Firmware or software change detected, board/FRU replace old ver" },
    { 0x2b, 0x01, 0x17, "Firmware or software change detected, board/FRU HW conf change" },
    { 0x2b, 0x02, 0xff, "Hardware incompatibility detected" },
    { 0x2b, 0x03, 0xff, "Firmware or software incompatibility detected" },
    { 0x2b, 0x04, 0xff, "Invalid or unsupported hardware version" },
    { 0x2b, 0x05, 0xff, "Invalid or unsupported firmware or software version" },
    { 0x2b, 0x06, 0xff, "Hardware change success" },
    { 0x2b, 0x07, 0x00, "Firmware or software change success" },
    { 0x2b, 0x07, 0x01, "Firmware or software change success, Mngmt Ctrl Dev Id" },
    { 0x2b, 0x07, 0x02, "Firmware or software change success, Mngmt Ctrl Firm Rev" },
    { 0x2b, 0x07, 0x03, "Firmware or software change success, Mngmt Ctrl Dev Rev" },
    { 0x2b, 0x07, 0x04, "Firmware or software change success, Mngmt Ctrl Manuf Id" },
    { 0x2b, 0x07, 0x05, "Firmware or software change success, Mngmt Ctrl IPMI Vers" },
    { 0x2b, 0x07, 0x06, "Firmware or software change success, Mngmt Ctrl Aux Firm Id" },
    { 0x2b, 0x07, 0x07, "Firmware or software change success, Mngmt Ctrl Firm Boot Block" },
    { 0x2b, 0x07, 0x08, "Firmware or software change success, Mngmt Ctrl Other" },
    { 0x2b, 0x07, 0x09, "Firmware or software change success, BIOS/EFI change" },
    { 0x2b, 0x07, 0x0A, "Firmware or software change success, SMBIOS change" },
    { 0x2b, 0x07, 0x0B, "Firmware or software change success, O/S change" },
    { 0x2b, 0x07, 0x0C, "Firmware or software change success, O/S loader change" },
    { 0x2b, 0x07, 0x0D, "Firmware or software change success, Service Diag change" },
    { 0x2b, 0x07, 0x0E, "Firmware or software change success, Mngmt SW agent change" },
    { 0x2b, 0x07, 0x0F, "Firmware or software change success, Mngmt SW App change" },
    { 0x2b, 0x07, 0x10, "Firmware or software change success, Mngmt SW Middle" },
    { 0x2b, 0x07, 0x11, "Firmware or software change success, Prog HW Change (FPGA)" },
    { 0x2b, 0x07, 0x12, "Firmware or software change success, board/FRU module change" },
    { 0x2b, 0x07, 0x13, "Firmware or software change success, board/FRU component change" },
    { 0x2b, 0x07, 0x14, "Firmware or software change success, board/FRU replace equ ver" },
    { 0x2b, 0x07, 0x15, "Firmware or software change success, board/FRU replace new ver" },
    { 0x2b, 0x07, 0x16, "Firmware or software change success, board/FRU replace old ver" },
    { 0x2b, 0x07, 0x17, "Firmware or software change success, board/FRU HW conf change" },
    /* FRU State */
    { 0x2c, 0x00, 0xff, "Not Installed" },
    { 0x2c, 0x01, 0xff, "Inactive" },
    { 0x2c, 0x02, 0xff, "Activation Requested" },
    { 0x2c, 0x03, 0xff, "Activation in Progress" },
    { 0x2c, 0x04, 0xff, "Active" },
    { 0x2c, 0x05, 0xff, "Deactivation Requested" },
    { 0x2c, 0x06, 0xff, "Deactivation in Progress" },
    { 0x2c, 0x07, 0xff, "Communication lost" },
    /* PICMG FRU Hot Swap */
    { 0xF0, 0x00, 0xFF, "Transition to M0" },
    { 0xF0, 0x01, 0xFF, "Transition to M1" },
    { 0xF0, 0x02, 0xFF, "Transition to M2" },
    { 0xF0, 0x03, 0xFF, "Transition to M3" },
    { 0xF0, 0x04, 0xFF, "Transition to M4" },
    { 0xF0, 0x05, 0xFF, "Transition to M5" },
    { 0xF0, 0x06, 0xFF, "Transition to M6" },
    { 0xF0, 0x07, 0xFF, "Transition to M7" },
    /* PICMG IPMB Physical Link */
    { 0xF1, 0x00, 0xff, "IPMB-A disabled, IPMB-B disabled" },
    { 0xF1, 0x01, 0xff, "IPMB-A enabled, IPMB-B disabled" },
    { 0xF1, 0x02, 0xff, "IPMB-A disabled, IPMB-B enabled" },
    { 0xF1, 0x03, 0xff, "IPMB-A enabled, IPMP-B enabled" },
    /* PICNG Moduke Hot Swap */
    { 0xF2, 0x00, 0xff, "Module Handle Closed" },
    { 0xF2, 0x01, 0xff, "Module Handle Opened" },
    { 0xF2, 0x02, 0xff, "Quiesced" },
    { 0x00, 0x00, 0xff, NULL },
};

static const struct ipmi_event_sensor_types vita_sensor_event_types[] = {
    /* VITA FRU State */
    { 0xF0, 0x00, 0xFF, "Transition to M0" },
    { 0xF0, 0x01, 0xFF, "Transition to M1" },
    { 0xF0, 0x04, 0xFF, "Transition to M4" },
    { 0xF0, 0x05, 0xFF, "Transition to M5" },
    { 0xF0, 0x06, 0xFF, "Transition to M6" },
    { 0xF0, 0x07, 0xFF, "Transition to M7" },
    /* VITA System IPMB Link */
    { 0xF1, 0x00, 0xFF, "IPMB-A disabled, IPMB-B disabled" },
    { 0xF1, 0x01, 0xFF, "IPMB-A enabled, IPMB-B disabled" },
    { 0xF1, 0x02, 0xFF, "IPMB-A disabled, IPMB-B enabled" },
    { 0xF1, 0x03, 0xFF, "IPMB-A enabled, IPMP-B enabled" },
    /* VITA FRU Temperature */
    { 0xF3, 0x00, 0xff, "At or below Lower Non-critical" },
    { 0xF3, 0x01, 0xff, "At or below Lower Critical" },
    { 0xF3, 0x02, 0xff, "At or below Lower Non-recoverable" },
    { 0xF3, 0x03, 0xff, "At or above Upper Non-critical" },
    { 0xF3, 0x04, 0xff, "At or above Upper Critical" },
    { 0xF3, 0x05, 0xff, "At or above Upper Non-recoverable" },
    { 0x00, 0x00, 0xff, NULL }
};

static const struct ipmi_event_sensor_types oem_kontron_event_types[] = {
    /* Board Reset(cPCI) */
    { 0xC1, 0x00, 0xff, "Push Button" },
    { 0xC1, 0x01, 0xff, "Bridge Reset" },
    { 0xC1, 0x02, 0xff, "Backplane" },
    { 0xC1, 0x03, 0xff, "Hotswap Fault" },
    { 0xC1, 0x04, 0xff, "Hotswap Healty" },
    { 0xC1, 0x05, 0xff, "Unknown" },
    { 0xC1, 0x06, 0xff, "ITP" },
    { 0xC1, 0x07, 0xff, "Hardware Watchdog" },
    { 0xC1, 0x08, 0xff, "Software Reset" },
    /* IPMB-L Link State, based on PICMG IPMB-0 Link state sensor */
    { 0xC3, 0x02, 0xff, "IPMB L Disabled" },
    { 0xC3, 0x03, 0xff, "IPMB L Enabled" },
    /* Board Reset */
    { 0xC4, 0x00, 0xff, "Push Button" },
    { 0xC4, 0x01, 0xff, "Hardware Power Failure" },
    { 0xC4, 0x02, 0xff, "Unknown" },
    { 0xC4, 0x03, 0xff, "Hardware Watchdog" },
    { 0xC4, 0x04, 0xff, "Soft Reset" },
    { 0xC4, 0x05, 0xff, "Warm Reset" },
    { 0xC4, 0x06, 0xff, "Cold Reset" },
    { 0xC4, 0x07, 0xff, "IPMI Command" },
    { 0xC4, 0x08, 0xff, "Setup Reset (Save CMOS)" },
    { 0xC4, 0x09, 0xff, "Power Up Reset" },
    /* POST Value */
    { 0xC6, 0x0E, 0xff, "Post Error (see data2)" },
    /* FWUM Status */
    { 0xC7, 0x00, 0xff, "First Boot After Upgrade" },
    { 0xC7, 0x01, 0xff, "First Boot After Rollback(error)" },
    { 0xC7, 0x02, 0xff, "First Boot After Errors (watchdog)" },
    { 0xC7, 0x03, 0xff, "First Boot After Manual Rollback" },
    { 0xC7, 0x08, 0xff, "Firmware Watchdog Bite, reset occurred" },
    /* Switch Mngt Software Status */
    { 0xC8, 0x00, 0xff, "Not Loaded" },
    { 0xC8, 0x01, 0xff, "Initializing" },
    { 0xC8, 0x02, 0xff, "Ready" },
    { 0xC8, 0x03, 0xff, "Failure (see data2)" },
    /* Diagnostic Status */
    { 0xC9, 0x00, 0xff, "Started" },
    { 0xC9, 0x01, 0xff, "Pass" },
    { 0xC9, 0x02, 0xff, "Fail" },
    { 0xCA, 0x00, 0xff, "In progress"},
    { 0xCA, 0x01, 0xff, "Success"},
    { 0xCA, 0x02, 0xff, "Failure"},
    /* FRU Over Current */
    { 0xCB, 0x00, 0xff, "Asserted"},
    { 0xCB, 0x01, 0xff, "Deasserted"},
    /* FRU Sensor Error */
    { 0xCC, 0x00, 0xff, "Asserted"},
    { 0xCC, 0x01, 0xff, "Deasserted"},
    /* FRU Power Denied */
    { 0xCD, 0x00, 0xff, "Asserted"},
    { 0xCD, 0x01, 0xff, "Deasserted"},
    /* Reset */
    { 0xCF, 0x00, 0xff, "Asserted"},
    { 0xCF, 0x01, 0xff, "Deasserted"},
    /* END */
    { 0x00, 0x00, 0xff, NULL },
};
#define SENSOR_TYPE_MAX 0x2c

#define IOCTL_INIT_SHM_DEVICE_TYPE_CMD 0X001

#define IOCTL_ADD_SEL_NODE_CMD 0X100
#define IOCTL_CLR_SEL_LIST_CMD 0X101
#define IOCTL_SET_SEL_ENTRY_MAXNUM_CMD 0X102

#define SANSOR_TYPE_TEMP 0X01
#define SANSOR_TYPE_VOL 0x02
#define SANSOR_TYPE_FAN 0x04

#define SEL_ENTRY_MAXNUM 200


#define PATH_NAME_SIZE 128

#ifdef UBMC_IPMI_DEBUG_TEST
#define UBMC_IPMI_GPIO_MAXNUM 9
#else
#define UBMC_IPMI_GPIO_MAXNUM 8
#endif

struct file_head
{
	unsigned char is_full;
	unsigned char flag;
	unsigned short int num_of_sel;

};
struct ubmc_ipmi_sel_record_file
{
	const char f_path[PATH_NAME_SIZE];
	struct file_head f_head;
	unsigned int offset_of_head;
	unsigned int offset_of_sel;
};
//event flag
typedef enum
{
	TRI_FALLING = 1,
	TRI_RISING = 2,
	TRI_HIGH_LEVE = 3,
	TRI_LOW_LEVE = 4,
	TRI_NULL = 5,
}trigger_type;

typedef enum
{
	LOW = '0',
	HIGH = '1',
}init_value;

struct gpio_s
{
	device_type_t device_type;
	char gpio_bank;
	char gpio_pin;
	int gpio_num;
	char gpio_name[128];
	init_value init_value;
	trigger_type tri;
	int fd;

};
struct poll_gpio_fd
{
	struct gpio_s gpio[UBMC_IPMI_GPIO_MAXNUM];
	struct pollfd fdset[UBMC_IPMI_GPIO_MAXNUM];

};

struct ubmc_ipmi_sel
{
	FILE* sel_msg_fd;
	pthread_mutex_t sel_add_node_mutex;
	int ipmi_dev_fd;
	unsigned short int record_id;
	unsigned int sel_entry_max_num;
	pthread_mutex_t record_mutex;
	unsigned char state;
	struct poll_gpio_fd gpio_fds;
	device_type_t device_type;

};
//extern struct ubmc_ipmi_sel ubmc_ipmi_g_sel;
//int ubmc_ipmi_init_gpio_file(void);
int ubmc_ipmi_poll_gpio_events(struct ubmc_ipmi_sel *ubmc_ipmi_sel);
int ubmc_ipmi_event_handle(struct ubmc_ipmi_sel* ubmc_ipmi_sel,struct sel_event_record * event);
extern int ubmc_ipmi_prt_sel(struct ubmc_ipmi_sel *ubmc_ipmi_sel);
int ubmc_ipmi_sel_clear(struct ubmc_ipmi_sel *ubmc_ipmi_sel);
int ubmc_ipmi_init_record_file(struct ubmc_ipmi_sel *ubmc_ipmi_sel);
void ubmc_ipmi_init_record_id(struct ubmc_ipmi_sel *ubmc_ipmi_sel);
int ubmc_ipmi_clr_sel_list(struct ubmc_ipmi_sel* ubmc_ipmi_sel);
int check_sensor_event(uint32_t value,ubmc_ipmi_sensor_thresh_s *thresh);
int ubmc_ipmi_set_sensor_evt(unsigned int evt_flag, ipmi_sensor_type type,unsigned char sensor_id,struct sel_event_record  *event);
extern int ubmc_ipmi_init_sel(struct ubmc_ipmi_sel *ubmc_ipmi_sel,int ipmi_dev_fd,device_type_t device_type);
extern FILE* create_sel_file_on_emmc(const char *path);
int ubmc_ipmi_open_gpio_value_file(struct gpio_s *gpio);
int ubmc_ipmi_init_gpio_s(struct gpio_s *gpio,const char *name,char tri,device_type_t device_type);
int ubmc_ipmi_check_sensor_value_is_valid(unsigned int value);

int i2c_smbus_access(int file, char read_write, unsigned char  command,int size, union i2c_smbus_data *data);
unsigned int i2c_smbus_read_block_data(int file, unsigned char command, unsigned char *values);
unsigned int i2c_smbus_read_word_data(int file, unsigned char command);
int open_i2c_dev(const char *dev,unsigned char slave_adr);
int ubmc_smbus_read_value(const char *dev,unsigned char device_adr,unsigned char reg_adr,unsigned short *p_value,char mode);
int ubmc_smbus_write_value(const char *dev,unsigned char device_adr,unsigned char reg_adr,unsigned short value,char mode);
int ubmc_get_sys_file_content(const char *p_path,uint32_t *p_value);
int ubmc_get_dev_file_content(const char *dev,uint32_t *p_value);
int update_calculation_factor_value(ubmc_sensor_s *pbase,uint16_t mtol,uint32_t bacc);
bool ubmc_is_file_exist(const char *file);
int ubmc_pmbus_get_linear_value(unsigned short in_value,float* out_value);
int ubmc_ipmi_read_gpio_value(struct gpio_s *gpio);
#endif /* INCLUDE_UBMC_IPMI_SEL_H_ */
