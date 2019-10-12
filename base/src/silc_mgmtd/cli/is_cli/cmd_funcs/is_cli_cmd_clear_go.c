/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_error.h"
#include "silc_cli_common.h"

int is_cli_cmd_do_sys_evt_clear()
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create("/action/system/reset-sys-evt", SILC_MGMTD_IF_REQ_ACTION, "reset-sys-evt");
	if(!p_req)
	{
		silc_cli_err_cmd_set_create_req_failed();
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

int is_cli_cmd_clear_go(silc_list* p_token_list)
{
	silc_cli_token *p_token;

	silc_list_for_each_entry(p_token, p_token_list, rl_node)
	{
		if(strcmp(p_token->name, "event") == 0)
		{
			return is_cli_cmd_do_sys_evt_clear();
		}
	}
	silc_cli_err_cmd_set_invalid_cmd();
	return -1;
}

