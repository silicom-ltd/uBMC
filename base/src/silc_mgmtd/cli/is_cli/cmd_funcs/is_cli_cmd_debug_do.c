/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

#define IS_CLI_PATH_ACTION_SYSTEM			"/action/system"

int is_cli_traversal_cb(const silc_cstr parent_path, silc_mgmtd_if_node* p_node, void* data)
{
	if(strlen(parent_path))
		silc_cli_print("%s", parent_path);
	if(strlen(p_node->name))
		silc_cli_print("/%s", p_node->name);
	silc_cli_print(", value:%s\n", p_node->val_str);
	return 0;
}

void is_cli_debug_handle_query_rsp(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_rsp_node_traversal(p_rsp, is_cli_traversal_cb, NULL);
}


int is_cli_cmd_debug_shell()
{
	silc_cstr passwd, shadow;
	silc_cstr shadow_shell = "$6$F7qyA2MPBRkjXcLr$7s90tPApMXm04VAqeFYorQn6bZQs9tJskCxyYNkqXQgaB.l7ToqYo16TniuRAi9Q0gdYsafTFOQyxhpZXbBYu0";
	passwd = getpass("Password: ");
	if(passwd)
	{
		shadow = crypt(passwd, shadow_shell);
		if(shadow && strcmp(shadow, shadow_shell) == 0)
		{
			silc_cli_session_timeout_disable();
#ifndef SILC_MGMTD_LOCAL_DEBUG
			system(". /etc/profile;/bin/ash;");
#else
			system("/bin/sh");
#endif
			silc_cli_session_timeout_update();
			silc_cli_session_timeout_enable();
			return 0;
		}
	}
	silc_cli_err_cmd_set_err_info("Input wrong password");
	return -1;
}

int is_cli_cmd_debug_do(silc_list* p_token_list)
{
	char err_info[200];
	silc_cli_token *p_token, *p_l1_token, *p_l2_token;

	if(silc_cli_cmd_do_simple_action("/action/system/check-is-admin", "true", NULL, 0) != 0)
	{
		silc_cli_err_cmd_set_err_info("no privilege to debug");
		return -1;
	}

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		p_l1_token = is_cli_cmd_get_next_rl_token(p_token_list, p_token);
		if(strcmp(p_token->name, "shell") == 0)
		{
			return is_cli_cmd_debug_shell();
		}
		else
		{
			silc_mgmtd_if_req_type type;
			silc_cstr root_val;
			if(!p_l1_token)
			{
				silc_cli_err_cmd_set_incomplete_cmd(p_token);
				return -1;
			}
			root_val = silc_mgmtd_if_path_get_last_name(p_l1_token->val_str);
			if(strcmp(p_token->name, "add") == 0)
			{
				if(silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_ADD,
						p_l1_token->val_str, root_val, NULL, err_info, 200) != 0)
				{
					silc_cli_print("%% Add %s failed! Error: %s.\n", p_l1_token->val_str, err_info);
					return -1;
				}
				return 0;
			}
			else if(strcmp(p_token->name, "delete") == 0)
			{
				if(silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_DELETE,
						p_l1_token->val_str, root_val, NULL, err_info, 200) != 0)
				{
					silc_cli_print("%% Delete %s failed! Error: %s.\n", p_l1_token->val_str, err_info);
					return -1;
				}
				return 0;
			}
			else
			{
				p_l2_token = is_cli_cmd_get_next_rl_token(p_token_list, p_l1_token);
				if(strcmp(p_token->name, "modify") == 0)
				{
					if(!p_l2_token)
					{
						silc_cli_err_cmd_set_incomplete_cmd(p_l1_token);
						return -1;
					}
					if(silc_cli_cmd_do_simple_request(SILC_MGMTD_IF_REQ_MODIFY,
							p_l1_token->val_str, p_l2_token->val_str, NULL, err_info, 200) != 0)
					{
						silc_cli_print("%% Modify %s to %s failed! Error: %s.\n", p_l1_token->val_str, p_l2_token->val_str, err_info);
						return -1;
					}
					return 0;
				}
				else if(strcmp(p_token->name, "query") == 0)
				{
					type = SILC_MGMTD_IF_REQ_QUERY;
					if(p_l2_token)
					{
						if(strcmp(p_l2_token->name, "sub") == 0)
							type = SILC_MGMTD_IF_REQ_QUERY_SUB;
						else if(strcmp(p_l2_token->name, "child") == 0)
							type = SILC_MGMTD_IF_REQ_QUERY_CHILD;
						else if(strcmp(p_l2_token->name, "wild") == 0)
							type = SILC_MGMTD_IF_REQ_QUERY_WILD;
					}
					if(silc_cli_cmd_do_simple_request(type,
							p_l1_token->val_str, "", is_cli_debug_handle_query_rsp, err_info, 200) != 0)
					{
						silc_cli_print("%% Query %s failed! Error: %s.\n", p_l1_token->val_str, err_info);
						return -1;
					}
					return 0;
				}
			}
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}
