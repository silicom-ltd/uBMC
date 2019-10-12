#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "ubmc_mgmtd_common.h"

#define PATH_CONFIG_BMC		"/config/bmc"
#define PATH_STATUS_BMC		"/status/bmc"

#define PATH_DEVICE_INFO	PATH_STATUS_BMC"/info"
#define PATH_DEVICE_HEALTH	PATH_STATUS_BMC"/health"

#define UBMC_HOSTLOG_FILE			"/var/log/host_serial.log"
#define UBMC_IPMI_SEL_TMP_FILE		"/tmp/ubmc_ipmi_sel_tmp.log"

#define LOCAL_IMAGE_PATH 			"/var/images"

int ubmc_cli_show_cdrom_storage()
{
	char cmd[256];

	silc_cli_print("\nStorage Usage(MByte):\n");
	silc_cli_print("%s\t%s\t%s\n","Total","Used","Free");

	snprintf(cmd,256,"df -m|grep %s|awk '{print $2,\"\t\",$3,\"\t\",$4}'",LOCAL_IMAGE_PATH);
	return silc_cli_show_cmd_output(cmd);
}

static int ubmc_ipmi_sel_show_log(void)
{
	char cmd[256];
	int ret = 0;
	sprintf(cmd, "/usr/bin/ubmc_ipmi_sel_prt | less");
	ret = system(cmd);
	if(ret < 0)
	{
		silc_cli_err_cmd_set_err_info("do cmd %s fail error :%s \n",cmd,strerror(errno));
		return -1;
	}
	return 0;
}

static void ubmc_cli_show_device(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-22s: %-s";
	silc_cstr titles[] = { "Title", "State" };
	silc_cstr names[] = {
			"DeviceType", "DeviceSerialNumber",
			"HardwareVersion", "FirmwareVersion", "SwitchVersion", "UbootVersion",
	};
	silc_cstr nodes[] = {
			"device_name", "device_sn",
			"hw_version", "fw_version", "sw_version", "uboot_version",
	};

	silc_cli_l1tree_list_display(p_node, "Device Information:", titles,
			nodes, names, format, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr), NULL);
}

static void ubmc_cli_show_device_cb(silc_mgmtd_if_rsp *p_rsp)
{
	ubmc_cli_show_device(p_rsp->p_node_root);
}

static void ubmc_cli_show_fan_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-20s", "%-12s", "%-8s", "%-12s", "%-8s", "%-8s" };
	silc_cstr names[] = { "ID", "Name", "Speed(RPM)", "Status",
			"Fault", "Warning" };
	silc_cstr nodes[] = { NULL, "name", "speed", "status", "fault", "warning" };
	silc_cstr trans_state[] = { "false", "no", "true", "yes"};
	silc_cstr trans_status[] = { "0", "green", "1", "green", "2", "yellow", "4", "orange", "8", "red"};
	silc_cstr* trans[] = { NULL, NULL, NULL, trans_status, trans_state, trans_state };
	int trans_num[] = { 0, 0, 0, 4, 2, 2 };
	silc_cstr title = "Fan State:";
	silc_cli_l2tree_display(p_node,
			title, nodes, names, formats, trans, trans_num, sizeof(nodes)/sizeof(silc_cstr));
}

static void ubmc_cli_show_vol_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-20s", "%-20s" };
	silc_cstr names[] = { "ID", "Name", "Voltage(V)" };
	silc_cstr nodes[] = { NULL, "name", "voltage" };

	silc_cli_l2tree_display(p_node,
			"Voltage State:", nodes, names, formats, NULL, 0, 3);
}

static void ubmc_cli_show_temp_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-20s", "%-20s", "%-20s" };
	silc_cstr names[] = { "ID", "Name", "Temperature(°C)", "Peak(°C)" };
	silc_cstr nodes[] = { NULL, "name", "temp", "peak" };

	silc_cli_l2tree_display(p_node,
			"Temperature State:", nodes, names, formats, NULL, 0, 4);
}

static void ubmc_cli_show_power_supply_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-20s", "%-20s", "%-20s", "%-20s", "%-20s"};
	silc_cstr names[] = { "ID", "Name", "Voltage out(V)","Status","Fault","Warning"};
	silc_cstr trans_state[] = { "false", "no", "true", "yes"};
	silc_cstr trans_states[] = { "false", "Not Present", "true", "Present"};
	silc_cstr nodes[] = { NULL, "name", "vout_val","status","fault","warning" };
	silc_cstr* trans[] = { NULL, NULL, NULL, trans_states,trans_state, trans_state};
	int trans_num[] = { 0, 0, 0, 2,2, 2 };
	silc_cli_l2tree_display(p_node,
			"Power Supply State:", nodes, names, formats, trans, trans_num, 6);
}

static void ubmc_cli_show_health_cb(silc_mgmtd_if_rsp *p_rsp)
{
	silc_mgmtd_if_node *p_node, *p_sensor;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "fan") == 0)
		{
			ubmc_cli_show_fan_status(p_node);
		}
		else if(strcmp(p_node->name, "voltage") == 0)
		{
			ubmc_cli_show_vol_status(p_node);
		}
		else if(strcmp(p_node->name, "temperature") == 0)
		{
			ubmc_cli_show_temp_status(p_node);
		}
		else if(strcmp(p_node->name, "power_supply") == 0)
		{
			ubmc_cli_show_power_supply_status(p_node);
		}
	}
}

static void ubmc_cli_show_bmc_console_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Log File", "Log Rotate Num", "Log Rotate Size(M)",
	"COM Speed", "COM Data", "COM Parity", "COM Stopbits", "COM HW flowctrl", "COM SW flowctrl"};
	silc_cstr nodes[] = { "log-to-file", "log-rotate-num", "log-rotate-size",
			"com-speed", "com-data", "com-parity", "com-stopbits", "com-hwflowctrl", "com-swflowctrl" };

	silc_cli_l1tree_list_display(p_node, "BMC Console Configuration:", titles,
			nodes, names, format, NULL, NULL, 9, NULL);
}

static void ubmc_cli_show_bmc_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "console") == 0)
			ubmc_cli_show_bmc_console_configured(p_node);
	}
}

static void ubmc_cli_show_bmc_console_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-20s: %-20s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Cable" };
	silc_cstr nodes[] = { "cable-connected"};
	silc_cstr trans_state[] = { "false", "disconnected", "true", "connected"};
	silc_cstr* trans[] = { trans_state };
	int trans_num[] = { 2 };

	silc_cli_l1tree_list_display(p_node, "BMC Console Status:", titles,
			nodes, names, format, trans, trans_num, 1, NULL);
}

static void ubmc_cli_show_bmc_power_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-20s: %-20s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Power Status", "Power Action" };
	silc_cstr nodes[] = { "state", "action" };

	silc_cli_l1tree_list_display(p_node, "BMC Power Status:", titles,
			nodes, names, format, NULL, NULL, 2, NULL);
}

static void ubmc_cli_show_bmc_usbcdrom_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-20s: %-20s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Mounted", "Image" ,"Location"};
	silc_cstr nodes[] = { "enabled", "image" , "location"};
	silc_cstr trans_state[] = { "false", "No", "true", "Yes"};
	silc_cstr* trans[] = { trans_state, NULL, NULL };
	int trans_num[] = { 2, 0, 0 };

	silc_cli_l1tree_list_display(p_node, "BMC USB CD-ROM Status:", titles,
			nodes, names, format, trans, trans_num, 3, NULL);
}

static void ubmc_cli_show_bmc_phonehome_status(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "state") == 0)
			silc_cli_simple_node_display(p_node, "Phonehome Status:", NULL, 0);
	}
}

static void ubmc_cli_show_bmc_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "console") == 0)
			ubmc_cli_show_bmc_console_status(p_node);
		else if(strcmp(p_node->name, "power") == 0)
			ubmc_cli_show_bmc_power_status(p_node);
		else if(strcmp(p_node->name, "usb-cdrom") == 0)
			ubmc_cli_show_bmc_usbcdrom_status(p_node);
	}
}

int ubmc_cli_cmd_show_do_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "bmc") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "configured") == 0)
			{
				sprintf(p_req_info->path, PATH_CONFIG_BMC);
				p_req_info->rsp_cb = ubmc_cli_show_bmc_configured_cb;
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "state") == 0)
			{
				sprintf(p_req_info->path, PATH_STATUS_BMC);
				p_req_info->rsp_cb = ubmc_cli_show_bmc_status_cb;
			}
			else if(strcmp(p_l1_token->name, "console-log") == 0)
			{
				silc_cli_token* p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(!p_l2_token)
				{
					return silc_cli_show_log(UBMC_HOSTLOG_FILE, NULL);
				}
				else if(strcmp(p_l2_token->name, "filter") == 0)
				{
					if (strlen(p_l2_token->val_str) > 128 )
					{
						silc_cli_err_cmd_set_invalid_param("filter is too long, please <128");
						return -1;
					}
					return silc_cli_show_log(UBMC_HOSTLOG_FILE, p_l2_token->val_str);
				}
				else if(strcmp(p_l2_token->name, "realtime") == 0)
				{
					system("tail -f "UBMC_HOSTLOG_FILE);
					return 0;
				}
			}
			else if(strcmp(p_l1_token->name, "local-iso") == 0)
			{
				silc_cli_token* p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(!p_l2_token)
				{
					return -1;
				}
				else if(strcmp(p_l2_token->name, "list") == 0)
				{
					char cmd[256];

					silc_cli_print("ISO List:\n");
					snprintf(cmd,256,"ls -l --time-style=long-iso %s |grep '\\.iso$' |awk '{print($6,$7,\"\\t\",$8)}'",LOCAL_IMAGE_PATH);
					if(silc_cli_show_cmd_output(cmd) != 0)
					{
						return 0;
					}

					ubmc_cli_show_cdrom_storage();
					return 0;
				}
			}
			else if(p_l1_token && strcmp(p_l1_token->name, "sel") == 0)
			{
				return ubmc_ipmi_sel_show_log();
			}
		}
		else if(strcmp(p_token->name, "health") == 0)
		{
			sprintf(p_req_info->path, PATH_DEVICE_HEALTH);
			p_req_info->rsp_cb = ubmc_cli_show_health_cb;
		}
		else if(strcmp(p_token->name, "device") == 0)
		{
			sprintf(p_req_info->path, PATH_DEVICE_INFO);
			p_req_info->rsp_cb = ubmc_cli_show_device_cb;
		}
		else if(strcmp(p_token->name, "phonehome") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "state") == 0)
			{
				sprintf(p_req_info->path, PATH_STATUS_BMC"/phonehome");
				p_req_info->rsp_cb = ubmc_cli_show_bmc_phonehome_status;
			}
		}
		else if(strcmp(p_token->name, "show") != 0
				&& strcmp(p_token->name, "configured") != 0
				&& strcmp(p_token->name, "state") != 0)
		{
			silc_cli_err_cmd_set_invalid_param(p_token->name);
			return -1;
		}
		p_req_info->root_val = silc_mgmtd_if_path_get_last_name(p_req_info->path);
		p_req_info->type = SILC_MGMTD_IF_REQ_QUERY_SUB;
	}
	return 0;
}

int ubmc_cli_cmd_show(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(ubmc_cli_cmd_show_do_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0)
		return 0;

	if(req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}
