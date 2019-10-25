
#include "silc_mgmtd_error.h"
#include "ubmc_mgmtd_func_def.h"
#include "ubmc_mgmtd_common.h"

/* define memdb node info */ 
static silc_mgmtd_node_info s_ubmc_mgmtd_node_list[] = {
{"/config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/config/bmc", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_bmc_upgrade, 10},
{"/config/bmc/console", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/log-to-file", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/log-rotate-num", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "20", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/log-rotate-size", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "5", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-speed", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "115200", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-data", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "8", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-parity", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "none", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-stopbits", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-hwflowctrl", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_bmc_config, 10},
{"/config/bmc/console/com-swflowctrl", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_bmc_config, 10},
{"/status", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/status/bmc", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/device_type", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/device_name", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/device_sn", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/device_tn", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/hw_version", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/fw_version", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/sw_version", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/info/uboot_version", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/name", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/fault", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/warning", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/status", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/speed", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/fan/id/run_time", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/voltage", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/voltage/id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/voltage/id/name", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/voltage/id/voltage", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/temperature", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/temperature/id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/temperature/id/name", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/temperature/id/temp", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/temperature/id/peak", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id/name", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id/status", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id/vout_val", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id/fault", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/health/power_supply/id/warning", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/console", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/console/cable-connected", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/power", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/power/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/power/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/bios", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/bios/upgrade", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/usb-cdrom", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/usb-cdrom/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/usb-cdrom/image", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/usb-cdrom/location", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/phonehome", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_status, 5},
{"/status/bmc/phonehome/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_status, 5},
{"/notify", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/notify/bmc", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_notify, 5},
{"/notify/bmc/config_changed", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_notify, 5},
{"/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/action/bmc", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/power", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/power/control", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/sel", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/sel/clear", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/bios", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/bios/upgrade", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/bios/upgrade/image", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/bios/upgrade/all", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-local", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-local/image", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-remote", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-remote/address", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-remote/path", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-remote/user", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/attach-remote/password", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/detach", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/usb-cdrom/remove-local-isofile", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/phonehome", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/phonehome/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/images", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_bmc_action, 5},
{"/action/bmc/images/remove-local-image", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_bmc_action, 5},
};

static int ubmc_mgmtd_get_node(silc_mgmtd_node_info** p_node_list, int* p_node_cnt)
{
	*p_node_list = s_ubmc_mgmtd_node_list;
	*p_node_cnt = sizeof(s_ubmc_mgmtd_node_list)/sizeof(silc_mgmtd_node_info);
	return 0;
}

static silc_mgmtd_cberr_info s_ubmc_mgmtd_cberr_list[] = {
{"/", IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_NOT_FOUND, "Image file not found"},
{"/", IS_MGMTD_ERR_BMC_CDROM_ISO_FILE_IN_USE, "Image file in use"},
{"/", IS_MGMTD_ERR_BMC_CDROM_USERNAME_OR_PASSWD_ERR, "Remote user name or password error"},
{"/", IS_MGMTD_ERR_BMC_CDROM_REMOTE_HOST_UNAVAILABLE, "Remote host unavailable"},
{"/", IS_MGMTD_ERR_BMC_CDROM_ATTACHED, "CDROM is attached"},
{"/", IS_MGMTD_ERR_BMC_CDROM_NOT_ATTACHED, "CDROM is not attached"},
};

static int ubmc_mgmtd_get_cberr(silc_mgmtd_cberr_info** p_cberr_list, int* p_cberr_cnt)
{
	*p_cberr_list = s_ubmc_mgmtd_cberr_list;
	*p_cberr_cnt = sizeof(s_ubmc_mgmtd_cberr_list)/sizeof(silc_mgmtd_cberr_info);
	return 0;
}

static silc_mgmtd_config_cmd_map s_ubmc_mgmtd_config_2_cmds[] = {
		/* BMC */
		{"bmc console log ?",		{{"/config/bmc/console/log-to-file", "false,to-file"TAG_CMD_TRANS_FALSE2NO",true,to-file"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"bmc console log rotate-num ?",	{{"/config/bmc/console/log-rotate-num"}}},
		{"bmc console log rotate-size ?",	{{"/config/bmc/console/log-rotate-size"}}},
		{"bmc console speed ?",			{{"/config/bmc/console/com-speed"}}},
		{"bmc console data ?",			{{"/config/bmc/console/com-data"}}},
		{"bmc console parity ?",		{{"/config/bmc/console/com-parity"}}},
		{"bmc console stopbits ?",		{{"/config/bmc/console/com-stopbits"}}},
		{"bmc console ?",	{{"/config/bmc/console/com-hwflowctrl", "false,hw-flowctrl"TAG_CMD_TRANS_FALSE2NO",true,hw-flowctrl"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"bmc console ?",	{{"/config/bmc/console/com-swflowctrl", "false,sw-flowctrl"TAG_CMD_TRANS_FALSE2NO",true,sw-flowctrl"}}, FLAG_CMD_TRANS_FALSE2NO},
};

static int ubmc_mgmtd_get_config_cmd(silc_mgmtd_config_cmd_map** p_config_list, int* p_config_cnt)
{
	*p_config_list = s_ubmc_mgmtd_config_2_cmds;
	*p_config_cnt = sizeof(s_ubmc_mgmtd_config_2_cmds)/sizeof(silc_mgmtd_config_cmd_map);
	return 0;
}

static int ubmc_mgmtd_action_custom_sys_halt(void)
{
	//ubmc doesn't have halt, return 0 to indicate halt is successful.
	return 0;
}


#define OEMI_MODEL_FILE_PATH "/etc/product/UBMC/OEMI/model.txt"
#define DECODE_NAME_MAX 20

int is_mgmtd_system_get_machine_model(void)
{
	int ret;
	char buf[DECODE_NAME_MAX];
	FILE *fp;
	if(access(OEMI_MODEL_FILE_PATH,F_OK) != 0)
	{
		SILC_ERR("Can not find %s file :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return -1;
	}
	fp = fopen(OEMI_MODEL_FILE_PATH,"r+");
	if(fp == NULL)
	{
		SILC_ERR("open %s fail :%s",OEMI_MODEL_FILE_PATH,strerror(errno));
		return -1;
	}
	fseek(fp,0,SEEK_SET);
	fgets(buf,DECODE_NAME_MAX,fp);
	if(strcmp(buf,"ATT-V150") == 0)			//X-Small Model
	{
		ret = 0;
	}
	else if(strcmp(buf,"ATT-V250") == 0)	//Small Model
	{
		ret = 1;
	}
	else if(strcmp(buf,"ATT-V450") == 0)	//Medium Model
	{
		ret = 2;
	}
	else if(strcmp(buf,"ATT-V850") == 0)	//Large Model
	{
		ret = 3;
	}
	else
	{
		ret = 0;							//default is X-Small
	}
	fclose(fp);
	return ret;
}

static int ubmc_mgmtd_action_sync_hw_clock(void)
{
	int ret;
	ret = is_mgmtd_system_get_machine_model();
	if(ret < 0 || ret > 3)
	{
		ret = 0;
		SILC_LOG("Can not get machine model ,use default model \n");
	}
	if(ret == 0)
	{
		if((system("hwclock -w -u -f /dev/rtc0")) != 0)
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	else
	{
		if((system("hwclock -w -u -f /dev/rtc1")) != 0)
			return IS_MGMTD_ERR_BASE_EXEC_FAILED;
	}
	return 0;
}

static silc_cstr ubmc_mgmtd_vendor_list[] = {UBMC_MGMTD_VENDOR_LIST};
static int ubmc_mgmtd_vendor_cnt = sizeof(ubmc_mgmtd_vendor_list)/sizeof(silc_cstr);

static silc_mgmtd_storage_path ubmc_mgmtd_storage_list[] = { {"*var*images", "/var/images"} };

silc_mgmtd_product_info mgmtd_product_info = {
		UBMC_PRODUCT_NAME,
		UBMC_PRODUCT_ID,
		"UBMC",
		"is_admin",
		"$6$mns4d/vK1k0hTHhq$P7HGvlCMzNoKtLA/9Wmsn9AcryO.aMcX2Fvz6j/.D/UPDlustc6CPifjOWZPQyk6SHsReyzMsCyxp1u4l8iek1",
		//vendor
		ubmc_mgmtd_vendor_list,
		sizeof(ubmc_mgmtd_vendor_list)/sizeof(silc_cstr),
		//storage
		ubmc_mgmtd_storage_list,
		sizeof(ubmc_mgmtd_storage_list)/sizeof(silc_mgmtd_storage_path),
		1,	//multi_eth
		0,	//permit_ip
		0,	//com_baudrate
		1,  //vrf
		1,  //iptables
		1,  //ipsec
		NULL,	//default_add_node
		NULL,	//default_del_node
		ubmc_mgmtd_get_node,
		ubmc_mgmtd_get_cberr,
		ubmc_mgmtd_get_config_cmd,
		NULL,	//set_init_config
		ubmc_mgmtd_action_custom_sys_halt,
		ubmc_mgmtd_action_sync_hw_clock,
		NULL,  //password check
		NULL,  //snmp_get_sysoid
};

