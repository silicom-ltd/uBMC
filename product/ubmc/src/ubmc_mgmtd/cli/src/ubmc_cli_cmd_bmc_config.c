/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

#include <silc_common.h>
#include <libgen.h>
#include <string.h>

#include "ubmc_mgmtd_common.h"

#define PATH_CONFIG_BMC	"/config/bmc"
#define LOCAL_BIOS_IMAGE_FILE	"/tmp/bmc_cli_upload_bios.img"
#define LOCAL_IMAGE_PATH		"/var/images"
#define CMD_BUFFER_SIZE			512

extern int ubmc_cli_show_cdrom_storage();

int ubmc_cli_upload_cdrom(silc_list* p_token_list, silc_cli_token* p_l1_token)
{
	silc_cli_token* p_l2_token;
	char distfile[CMD_BUFFER_SIZE];
	silc_cstr url = p_l1_token->val_str;

	p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
	if(p_l2_token && strcmp(p_l2_token->name, "as") == 0)
	{
		snprintf(distfile, CMD_BUFFER_SIZE, "%s/%s", LOCAL_IMAGE_PATH, p_l2_token->val_str);
	}
	else if(strncmp(url, "scp://", strlen("scp://")) == 0)
	{
		//sprintf(distfile, "%s/cdrom_%u.iso", LOCAL_IMAGE_PATH , (uint32_t)time(NULL));
		sprintf(distfile, "%s/", LOCAL_IMAGE_PATH);
	}
	else
	{
		snprintf(distfile, CMD_BUFFER_SIZE, "%s/%s", LOCAL_IMAGE_PATH, basename(url));
	}

	if(0 != silc_cli_upload_file(url, NULL, NULL, distfile))
		return -1;

	ubmc_cli_show_cdrom_storage();
	return 0;
}

static int show_progress = 0;
static inline void ubmc_cli_show_bmc_op_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr state_str = p_rsp->p_node_root->val_str;
	silc_cstr prog_tag_list[] = {
			"...",
			"Processing",
	};
	int i, size=sizeof(prog_tag_list)/sizeof(silc_cstr);

	if(!state_str[0])
		return;
	silc_cli_print("%s\n", state_str);
	for(i=0; i<size; i++)
	{
		if(strcasestr(state_str, prog_tag_list[i]) != NULL)
		{
			return;
		}
	}
	// progress tag not found, stop showing progress
	show_progress = 0;
}

static inline silc_bool ubmc_cli_cmd_bmc_qurey_param(silc_cli_token *p_token, silc_list* p_token_list, silc_cstr token_name)
{
	silc_bool res = silc_false;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name,token_name) == 0)
		{
			res = silc_true;
			break;
		}
	}
	return res;
}

int ubmc_cli_cmd_bmc_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;
	silc_cli_token *p_token, *p_l1_token, *p_l2_token;

	show_progress = 0;
	memset(&req_info, 0, sizeof(req_info));
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
			continue;

		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "console") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "connect") == 0)
			{
				silc_cli_session_timeout_disable();
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(p_l2_token && strcmp(p_l2_token->name, "force") == 0)
					system("serial_socket_client -f");
				else
					system("serial_socket_client");
				silc_cli_session_timeout_update();
				silc_cli_session_timeout_enable();
				return 0;
			}
			else
			{
				strcpy(req_info.path, PATH_CONFIG_BMC"/console");
				req_info.root_val = "";
				req_info.type = SILC_MGMTD_IF_REQ_MODIFY;
				break;
			}
		}
		else if(strcmp(p_token->name, "power") == 0)
		{
			if(p_l1_token && (
					strcmp(p_l1_token->name, "on") == 0 ||
					strcmp(p_l1_token->name, "off") == 0 ||
					strcmp(p_l1_token->name, "forceoff") == 0 ||
					strcmp(p_l1_token->name, "reset") == 0 ||
					strcmp(p_l1_token->name, "cycle") == 0))
			{
				strcpy(req_info.path, "/action/bmc/power/control");
				req_info.root_val = p_l1_token->name;
				req_info.type = SILC_MGMTD_IF_REQ_ACTION;
				show_progress = 1;
				break;
			}
			else
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
		}
		else if(strcmp(p_token->name, "bios") == 0)
		{
			if(!p_l1_token)
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
			if(strcmp(p_l1_token->name, "upgrade") == 0)
			{
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(!p_l2_token)
				{
					silc_cli_err_cmd_set_invalid_cmd();
					return -1;
				}
				if(strcmp(p_l2_token->name, "file-url") == 0)
				{
					silc_cli_token *p_l3_token;
					silc_cstr url = p_l2_token->val_str;
					int ret;
					p_l3_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l2_token);
					if(p_l3_token && strcmp(p_l3_token->name, "all") == 0)
					{
						char buf[16] = {0};
						silc_cli_print("%% Warning: Overwrite all sections of the BIOS may result in network switch no working, "
								"proceed only if you know what you are doing.\n"
								"Do you really want to continue? Type \"YES\" in capital letters to confirm: ");
						if(NULL == fgets(buf, 16, stdin) || strncmp(buf, "YES", 3) != 0 || buf[3] != '\n')
						{
							silc_cli_print("%% Cancelled");
							return 0;
						}
					}
					ret = silc_cli_upload_file(url, NULL, NULL, LOCAL_BIOS_IMAGE_FILE);
					if(ret != 0)
						return -1;
					silc_cli_print("%% Warning: Make sure the host is powered on and not in reboot. "
							"Also make sure the host will not reboot during the BIOS upgrade process.\n");
					if (0 != silc_cli_cmd_confirm("Continue (y|n) ?", NULL, "%% Cancelled"))
						return 0;

					free(p_l2_token->val_str);
					p_l2_token->val_str = malloc(strlen(LOCAL_BIOS_IMAGE_FILE)+1);
					strcpy(p_l2_token->val_str, LOCAL_BIOS_IMAGE_FILE);
					req_info.type = SILC_MGMTD_IF_REQ_ACTION;
					strcpy(req_info.path, "/action/bmc/bios/upgrade");
					req_info.root_val = "";
					show_progress = 2;
					break;
				}
			}
		}
		else if(strcmp(p_token->name, "usb-cdrom") == 0)
		{
			if(!p_l1_token)
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
			if(strcmp(p_l1_token->name, "attach-local") == 0)
			{
				req_info.root_val = "";
				strcpy(req_info.path, "/action/bmc/usb-cdrom/attach-local");
			}
			else if(strcmp(p_l1_token->name, "attach-remote") == 0)
			{
				if(!ubmc_cli_cmd_bmc_qurey_param(p_l1_token,p_token_list,"password"))
				{
					silc_cli_token *p_pass;
					silc_cstr passwd = NULL;
					passwd = getpass("Please input windows user password: ");
					if(!passwd)
					{
						silc_cli_err_cmd_set_err_info("Invalid password");
						return -1;
					}
					p_pass = malloc(sizeof(silc_cli_token));
					memset(p_pass, 0x0, sizeof(silc_cli_token));
					p_pass->name = "password";
					p_pass->map_name = "password";
					p_pass->val_str = malloc(strlen(passwd)+1);
					strcpy(p_pass->val_str, passwd);
					silc_list_add_tail(&p_pass->rl_node, p_token_list);
				}
				req_info.root_val = "";
				strcpy(req_info.path, "/action/bmc/usb-cdrom/attach-remote");
			}
			else if(strcmp(p_l1_token->name, "detach") == 0)
			{
				req_info.root_val = "";
				strcpy(req_info.path, "/action/bmc/usb-cdrom/detach");
			}
			else if(strcmp(p_l1_token->name, "upload") == 0)
			{
				return ubmc_cli_upload_cdrom(p_token_list, p_l1_token);
			}
			else
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}

			req_info.type = SILC_MGMTD_IF_REQ_ACTION;
			break;
		}
		else if(strcmp(p_l1_token->name, "sel") == 0)
		{
			p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
			if(strcmp(p_l2_token->name, "clear") == 0)
			{
				if (0 != silc_cli_cmd_confirm("%% Warning: Current sel record will be clear ! Continue (y|n) ?", NULL, "%% Cancelled"))
					return 0;
				req_info.root_val = "";
				strcpy(req_info.path, "/action/bmc/sel/clear");
				req_info.type = SILC_MGMTD_IF_REQ_ACTION;
				break;
			}
			else
			{
				silc_cli_err_cmd_set_invalid_cmd();
				return -1;
			}
		}
	}

	if(strlen(req_info.path) == 0 || req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

	if(show_progress)
	{
		silc_cli_print("This may take about 2 minutes, please wait.\nOperation progress: \n");
		memset(&req_info, 0, sizeof(req_info));
		req_info.type = SILC_MGMTD_IF_REQ_QUERY_SUB;
		if(show_progress == 1)
			sprintf(req_info.path, "/status/bmc/power/action");
		else if(show_progress == 2)
			sprintf(req_info.path, "/status/bmc/bios/upgrade");
		req_info.root_val = "";
		req_info.rsp_cb = ubmc_cli_show_bmc_op_status_cb;
		while(show_progress)
		{
			sleep(1);
			if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
				return -1;
		}
	}
	return 0;
}

