#ifndef INCLUDE_UBMC_STATE_H_
#define INCLUDE_UBMC_STATE_H_


#include <stdbool.h>
#include <inttypes.h>

#define UBMC_NAME_MAX			17//ipmi defined
//the maximum value could be of sensor
#define UBMC_LIMIT_SERSOR_TEMP       6
#define UBMC_LIMIT_SERSOR_FAN        5
#define UBMC_LIMIT_SERSOR_VOL        22
#define UBMC_LIMIT_SERSOR_PS		 3
typedef struct is_temp_state_s
{
	char		name[UBMC_NAME_MAX];
	bool		avail;
	bool        is_enable;
	uint32_t	temp;
	uint32_t	peak;
}is_temp_state;

typedef struct is_fan_state_s
{
	char		name[UBMC_NAME_MAX];
	bool		avail;
	bool		fault;
	bool		warning;
	bool        is_enable;
	uint8_t		status;
	uint32_t	speed;
	uint32_t	run_time;
}is_fan_state;

typedef struct is_voltage_state_s
{
	char		name[UBMC_NAME_MAX];
	uint32_t	voltage;    // millivolt
	bool        is_enable;
}is_voltage_state;

typedef struct is_power_supply_state_s
{
	char		name[UBMC_NAME_MAX];
	uint32_t	voltage_in;    // millivolt
	uint32_t	voltage_out;    // millivolt
	bool        is_enable;
	bool		fault;
	bool		warning;
	bool		status;
	unsigned short status_word;
	unsigned char status_vout;
	unsigned char status_iout;
	unsigned char status_input;
}is_power_supply_state;

typedef struct ubmc_ipmi_state_s
{
	uint32_t	device_type;
	char	hw_version[UBMC_NAME_MAX];
	char	hw_info[UBMC_NAME_MAX];
	char	fw_version[UBMC_NAME_MAX];
	bool	whoami;
	bool	test_running;

	is_temp_state    temp_state[UBMC_LIMIT_SERSOR_TEMP];
	is_fan_state	 fan_state[UBMC_LIMIT_SERSOR_FAN];
	is_voltage_state vol_state[UBMC_LIMIT_SERSOR_VOL];
	is_power_supply_state power_supply_state[UBMC_LIMIT_SERSOR_PS];
}ubmc_ipmi_state;


#endif /* INCLUDE_IS_STATE_H_ */
