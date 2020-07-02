/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

#define IS_CLI_PATH_CONFIG_MGMT		"/config/system/mgmt"
#define IS_CLI_PATH_CONFIG_LOG		"/config/system/misc/log"
#define IS_CLI_PATH_CONFIG_NTP		"/config/system/misc/datetime"
#define IS_CLI_PATH_CONFIG_SSH		"/config/system/service/ssh"
#define IS_CLI_PATH_CONFIG_HTTP		"/config/system/service/http"
#define IS_CLI_PATH_CONFIG_HTTPS	"/config/system/service/https"
#define IS_CLI_PATH_CONFIG_RADIUS	"/config/radius"
#define IS_CLI_PATH_CONFIG_TACACS	"/config/tacacs"
#define IS_CLI_PATH_CONFIG_UNIX_USER	"/config/unix/user"
#define IS_CLI_PATH_CONFIG_SNMP_AGENT	"/config/snmp/agent"
#define IS_CLI_PATH_CONFIG_SNMP_TRAP	"/config/snmp/trap"

int is_cli_cmd_no_do_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token, *p_l2_token;
	static char none_str[10] = "";

	p_req_info->root_val = none_str;
	p_req_info->type = SILC_MGMTD_IF_REQ_DELETE;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(p_token->map_name)
		{
			p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			return 0;
		}
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "management") == 0)
		{
			// management cmd should have at least 2 sub-tokens
			if(!p_l1_token)
				break;
			p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT);
			if(strcmp(p_l1_token->name, "eth-if") == 0)
			{
				strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/interface");
			}
			else if(strcmp(p_l1_token->name, "permitted") == 0)
			{
				if(p_l2_token && strcmp(p_l2_token->name, "ip") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/permit-list/%s", p_l2_token->val_str);
					return 0;
				}
			}
			else if(strcmp(p_l1_token->name, "dns") == 0)
			{
				if(p_l2_token && strcmp(p_l2_token->name, "server") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/dns/%s", p_l2_token->val_str);
					return 0;
				}
			}
			else if(strcmp(p_l1_token->name, "interface") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/interface-list/%s", p_l1_token->val_str);
				if(!p_l2_token)
					return 0;
			}
			else if(strcmp(p_l1_token->name, "secondary-address") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/address-list/%s", p_l1_token->val_str);
				return 0;
			}
			else if(strcmp(p_l1_token->name, "vrf") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/vrf-list/%s", p_l1_token->val_str);
				return 0;
			}
			else if(strcmp(p_l1_token->name, "vrf-process") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/vrf-process-list/%s", p_l1_token->val_str);
				return 0;
			}
			else if(strcmp(p_l1_token->name, "route") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/route-list/%s", p_l1_token->val_str);
				return 0;
			}
			else if(strcmp(p_l1_token->name, "ipsec") == 0)
			{
				if(!p_l2_token)
					break;
				if(strcmp(p_l2_token->name, "key") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/key-list/%s", p_l2_token->val_str);
					return 0;
				}
				else if(strcmp(p_l2_token->name, "cert") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/cert-list/%s", p_l2_token->val_str);
					return 0;
				}
				else if(strcmp(p_l2_token->name, "connection") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-conn/%s", p_l2_token->val_str);
					return 0;
				}
				else if(strcmp(p_l2_token->name, "secret") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-secret/%s", p_l2_token->val_str);
					return 0;
				}
				else if(strcmp(p_l2_token->name, "ca") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/ipsec-ca/%s", p_l2_token->val_str);
					return 0;
				}
			}
			else if(strcmp(p_l1_token->name, "iptables") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_MGMT"/iptables-rule/%s", p_l1_token->val_str);
				return 0;
			}
		}
		else if(strcmp(p_token->name, "radius") == 0)
		{
			if(!p_l1_token)
				break;
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_RADIUS"/static");
			if(strcmp(p_l1_token->name, "server") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_RADIUS"/server/%s", p_l1_token->val_str);
				return 0;
			}
		}
		else if(strcmp(p_token->name, "tacacs") == 0)
		{
			if(!p_l1_token)
				break;
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_TACACS"/static");
			if(strcmp(p_l1_token->name, "server") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_TACACS"/server/%s", p_l1_token->val_str);
				return 0;
			}
		}
		else if(strcmp(p_token->name, "user") == 0)
		{
			if(!p_l1_token)
				break;
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_UNIX_USER"/%s", p_l1_token->val_str);
			return 0;
		}
		else if(strcmp(p_token->name, "configuration") == 0)
		{
			sprintf(p_req_info->path, "/action/system/remove-config-file");
			p_req_info->root_val = p_token->val_str;
			p_req_info->type = SILC_MGMTD_IF_REQ_ACTION;
			return 0;
		}
		else if(strcmp(p_token->name, "snmp") == 0)
		{
			if(!p_l1_token)
				break;
			silc_cli_print("New SNMP setting will take effect after 'snmp apply'.\n");
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_AGENT);
			p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
			if(strcmp(p_l1_token->name, "community") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_AGENT"/communities/%s", p_l1_token->val_str);
				if(p_l2_token)
					p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "host") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_AGENT"/trap-hosts/%s", p_l1_token->val_str);
				if(p_l2_token)
					p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "user") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_AGENT"/users/%s", p_l1_token->val_str);
				if(p_l2_token)
					p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
			else if(strcmp(p_l1_token->name, "trap") == 0)
			{
				strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_SNMP_TRAP);
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
		}
		else if(strcmp(p_token->name, "log") == 0)
		{
			if(!p_l1_token)
				break;
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_LOG);
			if(strcmp(p_l1_token->name, "remote") == 0)
			{
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(!p_l2_token)
					break;
				if(strcmp(p_l2_token->name, "server") == 0)
				{
					sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_LOG"/remote-server-v2/%s", p_l2_token->val_str);
					return 0;
				}
			}
		}
		else if(strcmp(p_token->name, "ntp") == 0)
		{
			if(!p_l1_token)
				break;
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_NTP);
			if(strcmp(p_l1_token->name, "server") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_NTP"/ntp-server-v2/%s", p_l1_token->val_str);
				return 0;
			}
		}
		else if(strcmp(p_token->name, "ssh") == 0)
		{
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_SSH);
		}
		else if(strcmp(p_token->name, "http") == 0)
		{
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_HTTP);
		}
		else if(strcmp(p_token->name, "https") == 0)
		{
			strcpy(p_req_info->path, IS_CLI_PATH_CONFIG_HTTPS);
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

int is_cli_cmd_no_do(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_no_do_get_req_info(p_token_list, &req_info) != 0)
		return -1;

	if(strlen(req_info.path) == 0 || req_info.root_val == 0 || req_info.type == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}


