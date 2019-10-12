/* this file is created by silc_cli_inst_code_gen.py */ 

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

//dynamic function token:user 
int is_cli_dync_get_user_account(void* data, char* output_val_buf, int* p_val_str_len)
{
	int len = 50;
	*p_val_str_len = len;
	int count = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	silc_mgmtd_if_node *p_node;

	p_req = silc_mgmtd_if_req_create("/config/unix/user", SILC_MGMTD_IF_REQ_QUERY_CHILD, "");
	if(!p_req)
		return 0;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return 0;
	}

	silc_list_for_each_entry(p_rsp, &p_msg->if_recv_items, node)
	{
		if(!silc_mgmtd_if_rsp_check(p_rsp))
			continue;

		silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
		{
			sprintf(output_val_buf+count*len, "%s", p_node->name);
			count++;
		}

	}
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);
	*p_val_str_len = len;
    return count;
}

