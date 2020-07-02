
#include "silc_common.h"
#include "silc_cli_types.h"
#include "ubmc_mgmtd_common.h"

extern int ubmc_cli_cmd_bmc_config(silc_list* p_token_list);

extern int ubmc_cli_cmd_no_do(silc_list* p_token_list);

extern int ubmc_cli_cmd_phonehome_config(silc_list* p_token_list);

extern int ubmc_cli_cmd_show(silc_list* p_token_list);

extern int ubmc_cli_dync_get_cdrom_list(void* data, char* output_val_buf, int* p_val_str_len);

extern int ubmc_cli_dync_get_image_list(void* data, char* output_val_buf, int* p_val_str_len);

// token
static silc_cli_token_info s_ubmc_cli_token_info_list[] = {
		{"user_exec show device", "Display device information", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},
		{"user_exec show health", "Display device health state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},

		{"privileged_exec show device", "Display device information", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},
		{"privileged_exec show health", "Display device health state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},

		{"configure bmc", "Configure BMC", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_bmc_config, NULL, 0, 0, 1},
		{"configure bmc console", "Configure BMC host console", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console connect", "Connect to host console", "", NULL, NULL, TOKEN_MODE_OPTIONAL, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console connect force", "Force connecting even if someone is already connected", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console log", "Configure host console logging options", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console log to-file", "Enable writing log to file", "", "log-to-file", "true", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console log rotate-num", "Configure the log rotate number", "1-50", "log-rotate-num", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc console log rotate-size", "Configure the log rotate size", "1-10(MB)", "log-rotate-size", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc console speed", "Configure host console baudrate", "", "com-speed", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console speed 9600", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console speed 38400", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console speed 115200", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console data", "Configure host console data bits", "", "com-data", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console data 5", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console data 6", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console data 7", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console data 8", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity", "Configure host console parity", "", "com-parity", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity none", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity even", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity odd", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity mark", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console parity space", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console stopbits", "Configure host console stopbits", "", "com-stopbits", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console stopbits 1", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console stopbits 2", "", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console hw-flowctrl", "Enable hardware flow control", "", "com-hwflowctrl", "true", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc console sw-flowctrl", "Enable software flow control", "", "com-swflowctrl", "true", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power", "Configure BMC host power", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power on", "Power on host", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power off", "Power off host", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power forceoff", "Force power off host", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power cycle", "Power cycle host", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc power reset", "Power reset host", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc bios", "Configure BMC host BIOS", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc bios backup", "Back up host BIOS", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc bios restore", "Restore host BIOS", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc bios upgrade", "Upgrade host BIOS", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc bios upgrade file-url", "Upload a BIOS image from an HTTP/HTTPS/SCP URL", "url such as http://xxx/file or scp://xxx@x.x.x.x:/path/file", "image", NULL, TOKEN_MODE_OPTIONAL, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc bios upgrade file-url all", "Upgrade all flash regions", "", "all", "true", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom", "Configure BMC host USB CD-ROM", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom upload", "Upload an ISO file", "url such as http://xxx/file or scp://xxx@x.x.x.x:/path/file", NULL, NULL, TOKEN_MODE_OPTIONAL, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom upload as", "Save an uploaded ISO file as the given name", "filename", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_DYNAMIC, NULL, ubmc_cli_dync_get_cdrom_list, 2048, 0, 0},
		{"configure bmc usb-cdrom attach-local", "Attach a local ISO image as CDROM to the host USB", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom attach-local image", "The local image file", "filename", "image", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_DYNAMIC, NULL, ubmc_cli_dync_get_cdrom_list, 2048, 0, 0},
		{"configure bmc usb-cdrom attach-remote", "Attach a remote ISO image as CDROM to the host USB", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom attach-remote host", "The remote host address", "hostname", "address", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom attach-remote host path", "The full shared path for the image file", "path as /path/file", "path", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom attach-remote host path user", "The login user name", "username", "user", NULL, TOKEN_MODE_OPTIONAL, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom attach-remote host path user password", "The login user password", "password", "password", NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure bmc usb-cdrom detach", "Detach a previously attached ISO image", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc sel", "Configure BMC sel", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure bmc sel clear", "Clear all SEL log entries", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},

		{"configure no bmc", "Remove BMC configuration options", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_no_do, NULL, 0, 0, 1},
		{"configure no bmc console", "Configure BMC host console", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc console log", "Configure host console logging options", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc console log to-file", "Disable writing log to file", "", "log-to-file", "false", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc console hw-flowctrl", "Disable hardware flow control", "", "com-hwflowctrl", "false", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc console sw-flowctrl", "Disable software flow control", "", "com-swflowctrl", "false", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc usb-cdrom", "Configure BMC usb cdrom", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure no bmc usb-cdrom local-iso", "Remove the local ISO files", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_DYNAMIC, NULL, ubmc_cli_dync_get_cdrom_list, 2048, 0, 0},

		{"configure no images", "Remove the local image files", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_DYNAMIC, ubmc_cli_cmd_no_do, ubmc_cli_dync_get_image_list, 2048, 0, 1},

		{"configure phonehome", "Configure PhoneHome service", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_phonehome_config, NULL, 0, 0, 1},
		{"configure phonehome start", "Start PhoneHome service", "", NULL, "true", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure phonehome stop", "Stop PhoneHome service", "", NULL, "false", TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},

		{"configure show device", "Display device information", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},
		{"configure show health", "Display device health state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 0},

		{"configure show bmc", "Display BMC configuration", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 1},
		{"configure show bmc configured", "Display BMC configuration", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc state", "Display BMC runtime state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc console-log", "Display the host console log", "", NULL, NULL, TOKEN_MODE_OPTIONAL, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc console-log filter", "Display the host console log with filter", "filter keyword", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_VARIABLE, NULL, NULL, 0, 0, 0},
		{"configure show bmc console-log realtime", "Display realtime log", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc local-iso", "Display local ISO files", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc local-iso list", "Display local ISO files", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
		{"configure show bmc sel", "Display host SEL log entries", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},

		{"configure show phonehome", "Display PhoneHome runtime state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, ubmc_cli_cmd_show, NULL, 0, 0, 1},
		{"configure show phonehome state", "Display PhoneHome runtime state", "", NULL, NULL, TOKEN_MODE_SINGLE, TOKEN_TYPE_STATIC, NULL, NULL, 0, 0, 0},
};

static silc_cstr ubmc_cli_vendor_list[] = {UBMC_MGMTD_VENDOR_LIST};

static silc_cli_product_info cli_product_info = {
		.product_name = UBMC_PRODUCT_NAME,
		.product_id = UBMC_PRODUCT_ID,
		.vendor_list = ubmc_cli_vendor_list,
		.vendor_cnt = sizeof(ubmc_cli_vendor_list)/sizeof(silc_cstr),

		.token_list = s_ubmc_cli_token_info_list,
		.token_cnt = sizeof(s_ubmc_cli_token_info_list)/sizeof(silc_cli_token_info),

		.reboot_warn = "%% The uBMC will reboot, Continue (y|n) ?",  //reboot_warn

		.multi_eth_support = 1,
		.whoami_support = 0,
		.halt_support = 0,	//halt support, ubmc doesn't do halt

		.snmp_v3_only = 0,
		.snmp_threshold_enabled = 0,
		.snmp_show_engine_id = 0,
		.permit_ip_support = 0,
		.dns_support = 1,
		.show_http = 1,
		.manufacture_support = 0,

		.show_snmp_configure_trap_ctrl_func = NULL,
};

int ubmc_cli_get_manufacture_support(void)
{
	if (silc_mgmtd_if_cmp_vendor_id(VENDOR_UBMC_MANUFACTURE))
		return 1;
	return 0;
}

silc_cli_product_info * cli_get_product_info(void)
{
	silc_mgmtd_if_product_info_set(cli_product_info.product_name,
			cli_product_info.product_id,
			cli_product_info.vendor_list,
			cli_product_info.vendor_cnt);

	cli_product_info.manufacture_support = ubmc_cli_get_manufacture_support();

	return &cli_product_info;
}
