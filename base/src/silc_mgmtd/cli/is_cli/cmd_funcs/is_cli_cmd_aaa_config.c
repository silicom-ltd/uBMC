/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"
#include "silc_cli_types.h"

void is_split_base64_lines(silc_cstr str);

#define IS_CLI_PATH_CONFIG_WILD_STATIC_FMT	"/config/%s/static"
#define IS_CLI_PATH_CONFIG_WILD_SERVER_FMT	"/config/%s/server"
#define IS_CLI_PATH_CONFIG_UNIX_USER		"/config/unix/user"


int is_cli_cmd_aaa_config_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token, *p_l1_token;
	static char none_str[10] = "";

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "radius") == 0 || strcmp(p_token->name, "tacacs") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "server") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_WILD_SERVER_FMT"/%s", p_token->name, p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				return 0;
			}
			else
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_WILD_STATIC_FMT, p_token->name);
				p_req_info->root_val = none_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
				return 0;
			}
		}
		else if(strcmp(p_token->name, "user") == 0)
		{
			if(p_l1_token && strcmp(p_l1_token->name, "name") == 0)
			{
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_UNIX_USER"/%s", p_l1_token->val_str);
				p_req_info->root_val = p_l1_token->val_str;
				p_req_info->type = SILC_MGMTD_IF_REQ_ADD;
				break;
			}
			if(p_l1_token && strcmp(p_l1_token->name, "change-password") == 0)
			{
				silc_cstr user_name = silc_cli_get_connect_login_name();
				sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_UNIX_USER"/%s", user_name);
				p_req_info->root_val = user_name;
				p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
			}
		}
		else if(strcmp(p_token->name, "user-name") == 0)
		{
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_UNIX_USER"/%s", p_token->val_str);
			p_req_info->root_val = p_token->val_str;
		}
	}

	return 0;
}

int is_cli_cmd_aaa_change_passwd_req(silc_cstr user, silc_cstr old_pass, silc_cstr new_pass)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create("/action/aaa/change-passwd", SILC_MGMTD_IF_REQ_ACTION, "");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
		return -1;
	}

	if(!silc_mgmtd_if_req_add_node_ext(p_req->p_node_root, "user", user))
	{
		silc_cli_err_cmd_set_req_add_node_failed("user", user);
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	if(!silc_mgmtd_if_req_add_node_ext(p_req->p_node_root, "curr-passwd", old_pass))
	{
		silc_cli_err_cmd_set_req_add_node_failed("curr-passwd", old_pass);
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	if(!silc_mgmtd_if_req_add_node_ext(p_req->p_node_root, "new-passwd", new_pass))
	{
		silc_cli_err_cmd_set_req_add_node_failed("new-passwd", new_pass);
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(p_msg==NULL)
	{
		silc_cli_err_cmd_set_send_req_failed();
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_cli_err_cmd_set_rsp_error(p_rsp->return_str);
		silc_mgmtd_if_client_free_msg(p_msg);
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	silc_mgmtd_if_client_free_msg(p_msg);
	silc_mgmtd_if_req_delete(p_req);
    return 0;
}

#define MAX_PASSWD_LEN	32
int is_cli_cmd_aaa_change_passwd(silc_cli_token *p_token, silc_list* p_token_list)
{
	silc_cli_token *p_l1_token, *p_l2_token;
	silc_cstr new_pass, user, login_user;
	silc_cstr old_pass = "";

	login_user = silc_cli_get_connect_login_name();
	p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
	if(!p_l1_token)
	{
		silc_cstr input;
		char old[MAX_PASSWD_LEN]={0}, new[MAX_PASSWD_LEN]={0}, cfm[MAX_PASSWD_LEN]={0};
		input = getpass("Old Password: ");
		strncpy(old, input, MAX_PASSWD_LEN-1);
		input = getpass("New Password: ");
		strncpy(new, input, MAX_PASSWD_LEN-1);
		input = getpass("Confirm New Password: ");
		strncpy(cfm, input, MAX_PASSWD_LEN-1);
		if(strcmp(new, cfm))
		{
			silc_cli_err_cmd_set_err_info("New password and confirm password mismatch");
			return -1;
		}
		return is_cli_cmd_aaa_change_passwd_req(login_user, old, new);
	}
	else if(strcmp(p_l1_token->name, "new-password") != 0)
	{
		return -1;
	}
	new_pass = p_l1_token->val_str;
	p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
	if (p_l2_token && strcmp(p_l2_token->name, "user-name") != 0)
	{
		return -1;
	}
	user = login_user;
	if (p_l2_token)
	{
		user = p_l2_token->val_str;
	}
	if (strcmp(user, login_user) == 0)
	{
		char* passwd = getpass("Old Password: ");
		if (passwd)
			old_pass = passwd;
	}
	return is_cli_cmd_aaa_change_passwd_req(user, old_pass, new_pass);
}

silc_bool is_cli_cmd_validate_base64(silc_cstr val)
{
	silc_cstr start, end;
	char c;

	start = val;
	end = val+strlen(val)-1;
	if(*start == '\'' || *start == '\"')
	{
		if(*start != *end)
			return silc_false;
		start++;
		end--;
	}

	do
	{
		c = *start;
		if ((c < '0' || c > '9') &&
				(c < 'a' || c > 'z') &&
				(c < 'A' || c > 'Z') &&
				(c != '+') && (c != '/') && (c != '='))
			return silc_false;
		start++;
	} while (start <= end);

	return silc_true;
}

int is_cli_cmd_aaa_decrypt_shadow(silc_cstr enc_shadow)
{
	int len = strlen(enc_shadow);
	char cmd[320];

	if(strlen(enc_shadow) > 256)
	{
		silc_cli_err_cmd_set_err_info("Encrypted password is too long");
		return -1;
	}
	if(!is_cli_cmd_validate_base64(enc_shadow))
	{
		silc_cli_err_cmd_set_err_info("Invalid encrypted password");
		return -1;
	}

	sprintf(cmd, "echo %s| openssl enc -base64 -d", enc_shadow);
	if(silc_mgmtd_if_exec_system_cmd(cmd, enc_shadow, &len, 1000, silc_false) != 0)
	{
		silc_cli_err_cmd_set_err_info("Fail to exec cmd '%s'", cmd);
		return -1;
	}
	if(enc_shadow[strlen(enc_shadow)-1] == '\n')
		enc_shadow[strlen(enc_shadow)-1] = 0;
	return 0;
}

int is_cli_cmd_aaa_config(silc_list* p_token_list)
{
	silc_cli_token *p_token;
	is_cli_cmd_req_info req_info;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "change-password") == 0)
		{
			silc_cli_token *p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
			if(!p_l1_token || strcmp(p_l1_token->name, "new-password") == 0)
				return is_cli_cmd_aaa_change_passwd(p_token, p_token_list);
			else if(strcmp(p_l1_token->name, "new-encrypt-password") == 0)
			{
				if(is_cli_cmd_aaa_decrypt_shadow(p_l1_token->val_str))
					return -1;
			}
		}
		if(strcmp(p_token->name, "encrypt-password") == 0)
		{
			// get the decrypt shadow
			if(is_cli_cmd_aaa_decrypt_shadow(p_token->val_str))
				return -1;
		}
	}

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_aaa_config_get_req_info(p_token_list, &req_info) != 0)
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
