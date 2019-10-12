/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

int is_cli_get_txt_file(const silc_cstr filename, silc_cstr* p_val);

#define IS_CLI_PATH_CONFIG_MGMT_IF		"/config/system/mgmt/interface"
#define IS_CLI_PATH_CONFIG_MGMT			"/config/system/mgmt"

int is_cli_cmd_mgmt_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token, *p_l2_token;
	static char none_str[10] = "";

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
		{
			if(strcmp(p_token->name, "default-gw") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/%s", p_token->map_name);
				p_req_info->root_val = p_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_token->name, "file-url") == 0)
			{
				char filename[256] = {0};
				silc_cstr content;
				if(silc_cli_upload_file(p_token->val_str, NULL, NULL, filename) != 0)
					return -1;
				if(is_cli_get_txt_file(filename, &content) != 0)
					return -1;
				free(p_token->val_str);
				p_token->val_str = content;
				return 0;
			}
			else if(strcmp(p_token->name, "raw") == 0)
			{
				return 0;
			}
			continue;
		}

		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "eth-if") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT_IF);
			p_req_info->root_val = none_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		else if(strcmp(p_token->name, "vrf") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/vrf-list/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			return 0;
		}
		else if(strcmp(p_token->name, "vrf-process") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/vrf-process-list/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			return 0;
		}
		else if(strcmp(p_token->name, "route") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/route-list/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			return 0;
		}
		else if(strcmp(p_token->name, "interface") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/interface-list/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			if(strcmp(p_l1_token->name, "device") == 0)
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			else
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		else if(strcmp(p_token->name, "secondary-address") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/address-list/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			return 0;
		}
		else if(strcmp(p_token->name, "ipsec") == 0)
		{
			if(strcmp(p_l1_token->name, "enable") == 0 || strcmp(p_l1_token->name, "disable") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/%s", p_l1_token->map_name);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "connection") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-conn/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(!p_l2_token)
					p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				else
					p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "ca") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-ca/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "secret") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-secret/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "key") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/key-list/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			}
			else if(strcmp(p_l1_token->name, "cert") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/cert-list/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			}
		}
		else if(strcmp(p_token->name, "iptables") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/iptables-rule/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
			return 0;
		}
		else if(strcmp(p_token->name, "dns") == 0)
		{
			if(strcmp(p_l1_token->name, "server") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/dns/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				return 0;
			}
		}
		else if(strcmp(p_token->name, "permitted") == 0)
		{
			int permit_ip_support = 1;
			if(silc_cli_get_product_info()->permit_ip_support_func)
			{
				permit_ip_support = silc_cli_get_product_info()->permit_ip_support_func();
			}
			if(permit_ip_support)
			{
				if(strcmp(p_l1_token->name, "disable") == 0 ||
						strcmp(p_l1_token->name, "enable") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/%s", p_l1_token->map_name);
					p_req_info->root_val = p_l1_token->map_val;
					p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
					return 0;
				}
				else if(strcmp(p_l1_token->name, "ip") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/permit-list/%s", p_l1_token->val_str);
					p_req_info->root_val = p_l1_token->val_str;
					p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
					return 0;
				}
			}
		}
		else if(strcmp(p_token->name, "whoami") == 0)
		{
			if(silc_cli_get_product_info()->whoami_support)
			{
				sprintf(p_req_info->path, "/notify/switch/who_am_i");
				p_req_info->root_val = none_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_NOTIFY;
				return 0;
			}
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

int is_cli_cmd_mgmt_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_mgmt_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0 || req_info.root_val == NULL || req_info.type == SILC_MGMTD_IF_REQ_NULL)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}

