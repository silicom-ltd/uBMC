/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

#define IS_CLI_PATH_CONFIG_SYSTEM			"/config/system/service"
#define IS_CLI_PATH_CONFIG_SYSTEM_COM		IS_CLI_PATH_CONFIG_SYSTEM"/com"
#define IS_CLI_PATH_CONFIG_SYSTEM_SSH		IS_CLI_PATH_CONFIG_SYSTEM"/ssh"
#define IS_CLI_PATH_CONFIG_SYSTEM_WEB		IS_CLI_PATH_CONFIG_SYSTEM"/web"
#define IS_CLI_PATH_CONFIG_SYSTEM_HTTP		IS_CLI_PATH_CONFIG_SYSTEM"/http"
#define IS_CLI_PATH_CONFIG_SYSTEM_HTTPS		IS_CLI_PATH_CONFIG_SYSTEM"/https"
#define IS_CLI_PATH_CONFIG_SYSTEM_TELNET	IS_CLI_PATH_CONFIG_SYSTEM"/telnet"

int is_cli_get_txt_file(const silc_cstr filename, silc_cstr* p_val)
{
	int fd;
	struct stat fs;
	silc_cstr buff = NULL;

	if(stat(filename, &fs) < 0)
		return -1;

	if(!(buff=malloc(fs.st_size+1)))
	{
		silc_cli_err_cmd_set_err_info("malloc buffer for file(%s)(%d bytes) failed", filename, (int)fs.st_size);
		goto OUT;
	}

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		silc_cli_err_cmd_set_err_info("open file(%s) failed", filename);
		goto OUT;
	}
	read(fd, buff, fs.st_size);
	buff[fs.st_size] = 0;
	close(fd);
	unlink(filename);

	*p_val = buff;
	return 0;
OUT:
	close(fd);
	unlink(filename);
	if(buff)
		free(buff);
	return -1;
}

int is_cli_cmd_service_config_get_req_info(silc_list* p_token_list, is_cli_cmd_req_info* p_req_info)
{
	silc_cli_token *p_token;
	static char none_str[10] = "";

	p_req_info->root_val = none_str;
	p_req_info->type = SILC_MGMTD_IF_REQ_MODIFY;
	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "session-expired-time") == 0)
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_WEB);
		if(p_token->map_name)
			continue;
		if(strcmp(p_token->name, "com") == 0)
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_COM);
		else if(strcmp(p_token->name, "ssh") == 0)
		{
			silc_cli_token* p_l1_token;
			p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
			if(p_l1_token && strcmp(p_l1_token->name, "host-key") == 0)
			{
				silc_cli_token* p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(p_l2_token && strcmp(p_l2_token->name, "remove") == 0)
				{
					char cmd[256];
					silc_cstr host = p_l2_token->val_str;
					if(!silc_cli_check_name(host))
					{
						silc_cli_err_cmd_set_err_info("Invalid host %s", host);
						return -1;
					}
					sprintf(cmd, "ssh-keygen -R %s", host);
					system(cmd);
					p_req_info->type = 0;
					return 0;
				}
				return -1;
			}
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_SSH);
		}
		else if(strcmp(p_token->name, "http") == 0)
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_HTTP);
		else if(strcmp(p_token->name, "https") == 0)
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_HTTPS);
		else if(strcmp(p_token->name, "telnet") == 0)
			sprintf(p_req_info->path, IS_CLI_PATH_CONFIG_SYSTEM_TELNET);
		else if(strcmp(p_token->name, "cert") == 0)
			strcat(p_req_info->path, "/certificate");
		else if(strcmp(p_token->name, "key") == 0)
			strcat(p_req_info->path, "/private-key");
		else if(strcmp(p_token->name, "file-url") == 0)
		{
			char filename[256] = {0};
			if(silc_cli_upload_file(p_token->val_str, NULL, NULL, filename) != 0)
				return -1;
			if(is_cli_get_txt_file(filename, &p_req_info->root_val) != 0)
				return -1;
			return 0;
		}
		else if(strcmp(p_token->name, "raw") == 0)
		{
			p_req_info->root_val = p_token->val_str;
			return 0;
		}
	}

	return 0;
}

int is_cli_cmd_service_config(silc_list* p_token_list)
{
	is_cli_cmd_req_info req_info;

	memset(&req_info, 0, sizeof(req_info));
	if(is_cli_cmd_service_config_get_req_info(p_token_list, &req_info) != 0)
		return -1;
	if(req_info.type == 0)
		return 0;
	if(strlen(req_info.path) == 0 || req_info.root_val == 0)
	{
		silc_cli_err_cmd_set_invalid_cmd();
		return -1;
	}

	if(is_cli_cmd_do_request(&req_info, p_token_list) != 0)
		return -1;

    return 0;
}
