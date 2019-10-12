/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_vnode.h"
#include "silc_mgmtd_config.h"
#include "ubmc_mgmtd_func_def.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "ubmc_shm_state.h"
#include "ubmc_state.h"

ubmc_shm_state	g_ubmc_shm_state = {-1, NULL};

ubmc_ipmi_state* ubmc_ipmi_state_handle_get(void)
{
	int ret = 0;
	struct stat st;
	ubmc_shm_state* p_ret = &g_ubmc_shm_state;

	if(p_ret->map_addr)
		return (ubmc_ipmi_state*)(p_ret->map_addr);

	if(stat(UBMC_SHM_FILE, &st)!=0)
	{
		SILC_ERR("File %s doesn't exist", UBMC_SHM_FILE);
		return NULL;
	}
	if(st.st_size != sizeof(ubmc_ipmi_state))
	{
		SILC_ERR("File %s exist but have different size %u %u", UBMC_SHM_FILE, (uint32_t)st.st_size, sizeof(ubmc_ipmi_state));
		return NULL;
	}
	ret = open(UBMC_SHM_FILE, O_RDWR);
	if(ret < 0)
	{
		SILC_ERR("Fail to open %s, ret %d", UBMC_SHM_FILE, ret);
		goto ERR_RET;
	}
	p_ret->fd = ret;
	p_ret->map_addr = mmap(NULL, sizeof(ubmc_ipmi_state), PROT_READ|PROT_WRITE, MAP_SHARED, p_ret->fd, 0);
	if(p_ret->map_addr == NULL)
	{
		SILC_ERR("Fail to mmap %s, ret %d", UBMC_SHM_FILE, ret);
		goto ERR_RET;
	}

	return (ubmc_ipmi_state*)(p_ret->map_addr);

ERR_RET:
	if(p_ret->fd > 0)
		close(p_ret->fd);
	return NULL;
}

int is_mgmtd_bmc_status_level_get_level_5(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	const silc_cstr grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 2);
	const silc_cstr great_grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 3);
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);

	ubmc_ipmi_state* p_shm_state = ubmc_ipmi_state_handle_get();
	if(p_shm_state == NULL)
	{
		SILC_ERR("ubmc_ipmi_state is not ready yet");
		return 0;
	}
	if(strcmp(great_grand_parent_name, "health") == 0)
	{
		if(strcmp(grand_parent_name, "fan") == 0)
		{
			long ui_id;
			is_fan_state* p_shm;
			if(0 != silc_mgmtd_if_cstr2l(parent_name, &ui_id))
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			p_shm = &p_shm_state->fan_state[ui_id-1];
			if(p_shm->is_enable == 1)
			{
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, name, str, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, speed, uint32, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, run_time, uint32, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, status, uint32, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, warning, boolean, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, fault, boolean, silc_false);
			}
		}
		else if(strcmp(grand_parent_name, "voltage") == 0)
		{
			long ui_id;
			is_voltage_state* p_shm;
			char vol_str[16];
			if(0 != silc_mgmtd_if_cstr2l(parent_name, &ui_id))
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			p_shm = &p_shm_state->vol_state[ui_id-1];
			if(p_shm->is_enable == 1)
			{
				//voltage = N*0.0001(V)
				if((int)p_shm->voltage < 0)
				{
					sprintf(vol_str, "N/A");
				}
				else
				{
					sprintf(vol_str, "%d.%04d", p_shm->voltage/10000, p_shm->voltage%10000);
				}
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, name, str, silc_false);
				//is_mgmtd_switch_vnode_add_by_field_maybe(p_parent_node, p_match_node, voltage, uint32, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "voltage", vol_str, str, silc_false);
			}
		}
		else if(strcmp(grand_parent_name, "temperature") == 0)
		{
			long ui_id;
			is_temp_state* p_shm;
			char temp_str[16], peak_str[16];
			if(0 != silc_mgmtd_if_cstr2l(parent_name, &ui_id))
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			p_shm = &p_shm_state->temp_state[ui_id-1];
			if(p_shm->is_enable == 1)
			{
				//temp = N*0.001(°C)
				if((int)p_shm->temp < 0 || (int)p_shm->peak < 0)
				{
					p_shm->peak = 0;
					sprintf(temp_str, "N/A");
					sprintf(peak_str, "%d.%03d", p_shm->peak/1000, p_shm->peak%1000);
				}
				else
				{
					sprintf(temp_str, "%d.%03d", p_shm->temp/1000, p_shm->temp%1000);
					sprintf(peak_str, "%d.%03d", p_shm->peak/1000, p_shm->peak%1000);

				}
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, name, str, silc_false);
				//silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, temp, uint32, silc_false);
				//silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, peak, uint32, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "temp", temp_str, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "peak", peak_str, str, silc_false);
			}

		}
		else if(strcmp(grand_parent_name, "power_supply") == 0)
		{
			long ui_id;
			is_power_supply_state* p_shm;
			char vout_str[16];
			if(0 != silc_mgmtd_if_cstr2l(parent_name, &ui_id))
				return IS_MGMTD_ERR_BASE_INVALID_PARAM;
			p_shm = &p_shm_state->power_supply_state[ui_id-1];
			if(p_shm->is_enable == 1)
			{
				//temp = N*0.001(°C)
				if((int)p_shm->voltage_out < 0)
				{
					p_shm->voltage_out = 0;
					sprintf(vout_str, "N/A");
				//	sprintf(peak_str, "%d.%03d", p_shm->voltage_out/1000, p_shm->voltage_out%1000);
				}
				else
				{
					sprintf(vout_str, "%d.%03d", p_shm->voltage_out/1000, p_shm->voltage_out%1000);

				}
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, name, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "vout_val", vout_str, str, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, status, boolean, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, warning, boolean, silc_false);
				silc_mgmtd_vnode_add_by_field_maybe(p_parent_node, p_match_node, fault, boolean, silc_false);

			}

		}
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}


int is_mgmtd_bmc_status_level_get_level_4(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	char tmp_name[128];
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);
	const silc_cstr grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 2);
	ubmc_ipmi_state* p_shm_state = ubmc_ipmi_state_handle_get();
	if(p_shm_state == NULL)
	{
		SILC_ERR("ubmc_ipmi_state is not ready yet");
		return 0;
	}
	if(strcmp(grand_parent_name, "health") == 0)
	{
		int loop;
		if(strcmp(parent_name, "fan") == 0)
		{
			for(loop=1; loop<=UBMC_LIMIT_SERSOR_FAN; loop++)
			{
				if(p_shm_state->fan_state[loop-1].is_enable == 1)
				{
					sprintf(tmp_name, "%u", loop);
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, tmp_name, loop, uint32, silc_true);
				}
			}
		}
		else if(strcmp(parent_name, "voltage") == 0)
		{
			for(loop=1; loop<=UBMC_LIMIT_SERSOR_VOL; loop++)
			{
				if(p_shm_state->vol_state[loop-1].is_enable == 1)
				{
					sprintf(tmp_name, "%u", loop);
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, tmp_name, loop, uint32, silc_true);
				}
			}
		}
		else if(strcmp(parent_name, "temperature") == 0)
		{
			for(loop=1; loop<=UBMC_LIMIT_SERSOR_TEMP; loop++)
			{
				if(p_shm_state->temp_state[loop-1].is_enable == 1)
				{
					sprintf(tmp_name, "%u", loop);
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, tmp_name, loop, uint32, silc_true);
				}

			}
		}
		else if(strcmp(parent_name, "power_supply") == 0)
		{
			for(loop=1; loop<=UBMC_LIMIT_SERSOR_PS; loop++)
			{
				if(p_shm_state->power_supply_state[loop-1].is_enable == 1)
				{
					sprintf(tmp_name, "%u", loop);
					silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, tmp_name, loop, uint32, silc_true);
				}
			}
		}
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_get_eeprom_var(char* name, char* value, int len)
{
	char cmd[256];
	FILE* fp;

	value[0] = 0;
	sprintf(cmd, "is_eeprom_op.sh %s", name);
	fp = popen(cmd, "r");
	fgets(value, len, fp);
	fclose(fp);
	silc_mgmtd_if_trim_trail_nl(value);
	if(strstr(value, "Error:"))
	{
		strcpy(value, "N/A");
	}
	return 0;
}

int is_mgmtd_bmc_status_level_get_level_3(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[256];
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);
	silc_cstr cmd;
	char out[256];
	int len;

	if(strcmp(parent_name, "info") == 0)
	{
		char value[256];

		is_mgmtd_get_eeprom_var("product_name", value, 256);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "device_name", value, str, silc_false);
		is_mgmtd_get_eeprom_var("serial_number", value, 256);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "device_sn", value, str, silc_false);
		is_mgmtd_get_eeprom_var("label_revision", value, 256);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "hw_version", value, str, silc_false);

		silc_mgmtd_get_ver_from_file("/etc/prod_version/software_version.txt", value);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "sw_version", value, str, silc_false);
		silc_mgmtd_get_ver_from_file("/etc/prod_version/uboot_version.txt", value);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "uboot_version", value, str, silc_false);
	}
	else if(strcmp(parent_name, "health") == 0)
	{
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "fan", "fan", str, silc_true);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "voltage", "voltage", str, silc_true);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "temperature", "temperature", str, silc_true);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "power_supply", "power_supply", str, silc_true);
	}
	else if(strcmp(parent_name, "console") == 0)
	{
		silc_bool connected = silc_false;
		silc_cstr connected_str = "serial is connected";
		len = 256;
		cmd = "ubmc_sys_ctrl.sh -g";
		if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
		{
			SILC_ERR("Error: fail to execute '%s'", cmd);
		}
		if(strncasecmp(out, connected_str, strlen(connected_str)) == 0)
			connected = silc_true;
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "cable-connected", connected, boolean, silc_true);
	}
	else if(strcmp(parent_name, "power") == 0)
	{
		if(!p_match_node || strcmp(p_match_node, "state") == 0)
		{
			len = 256;
			cmd = "ubmc_sys_ctrl.sh -q";
			if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
			{
				sprintf(out, "Error: fail to execute '%s'", cmd);
			}
			silc_mgmtd_if_trim_trail_nl(out);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "state", out, str, silc_true);
		}
		if(!p_match_node || strcmp(p_match_node, "action") == 0)
		{
			len = 256;
			cmd = "ubmc_sys_ctrl.sh -p";
			if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
			{
				sprintf(out, "Error: fail to execute '%s'", cmd);
			}
			silc_mgmtd_if_trim_trail_nl(out);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "action", out, str, silc_true);
		}
	}
	else if(strcmp(parent_name, "usb-cdrom") == 0)
	{
#define CDROM_OP_BIN_PATH       "/usr/sbin/ubmc_usb_cdrom_ctrl.sh"
		char cmd_buf[64];

		len = 256;
		snprintf(cmd_buf,64,"%s -s",CDROM_OP_BIN_PATH);
		if(silc_mgmtd_if_exec_system_cmd(cmd_buf, out, &len, 1000, silc_false) != 0)
		{
			sprintf(out, "Error: fail to execute '%s'", cmd_buf);
		}
		/*
		 * find if sub_str is in str
		 */
		if(strstr(out,"occupied") != NULL)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "enabled", silc_true, boolean, silc_true);
		}
		else
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "enabled", silc_false, boolean, silc_true);
		}

		len = 256;
		snprintf(cmd_buf,64,"%s -n",CDROM_OP_BIN_PATH);
		if(silc_mgmtd_if_exec_system_cmd(cmd_buf, out, &len, 1000, silc_false) != 0)
		{
			sprintf(out, "Error: fail to execute '%s'", cmd_buf);
		}
		silc_mgmtd_if_trim_trail_nl(out);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "image", out, str, silc_true);

		len = 256;
		snprintf(cmd_buf,64,"%s -t",CDROM_OP_BIN_PATH);
		if(silc_mgmtd_if_exec_system_cmd(cmd_buf, out, &len, 1000, silc_false) != 0)
		{
			sprintf(out, "Error: fail to execute '%s'", cmd_buf);
		}
		silc_mgmtd_if_trim_trail_nl(out);
		silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "location", out, str, silc_true);
	}
	else if(strcmp(parent_name, "bios") == 0)
	{
		if(!p_match_node || strcmp(p_match_node, "upgrade") == 0)
		{
			len = 256;
			cmd = "ubmc_sys_ctrl.sh -up";
			if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
			{
				sprintf(out, "Error: fail to execute '%s'", cmd);
			}
			silc_mgmtd_if_trim_trail_nl(out);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "upgrade", out, str, silc_true);
		}
	}
	else if(strcmp(parent_name, "phonehome") == 0)
	{
		if(!p_match_node || strcmp(p_match_node, "state") == 0)
		{
			len = 256;
			cmd = "ubmc_phonehome.sh status";
			if(silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 1000, silc_false) != 0)
			{
				sprintf(out, "Error: fail to execute '%s'", cmd);
			}
			silc_mgmtd_if_trim_trail_nl(out);
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "state", out, str, silc_true);
		}
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_bmc_status_level_get_level_2(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];

	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "info", "info", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "health", "health", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "console", "console", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "power", "power", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "usb-cdrom", "usb-cdrom", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "bios", "bios", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "phonehome", "phonehome", str, silc_true);

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}


int is_mgmtd_bmc_status_level_get_level_1(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "bmc", "bmc", str, silc_true);
	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_bmc_status_level_get_level(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	switch(level)
	{
	case 1:
		return is_mgmtd_bmc_status_level_get_level_1(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 2:
		return is_mgmtd_bmc_status_level_get_level_2(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 3:
		return is_mgmtd_bmc_status_level_get_level_3(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 4:
		return is_mgmtd_bmc_status_level_get_level_4(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 5:
		return is_mgmtd_bmc_status_level_get_level_5(p_parent_node, level, p_level_arr, p_match_node, add_node);
	default:
		SILC_ERR("Too many levels %u for switch status query", level);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	return 0;
}

int is_mgmtd_bmc_status(silc_mgmtd_if_req_type type, void* p_db_node, void* data)
{
	return silc_mgmtd_module_vnode_retrieve(type, p_db_node, data, is_mgmtd_bmc_status_level_get_level);
}
