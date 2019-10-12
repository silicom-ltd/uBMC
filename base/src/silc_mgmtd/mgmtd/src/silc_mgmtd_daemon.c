
#include "silc_common.h"
#include "silc_common/silc_thread.h"

#include "silc_mgmtd_error.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_operation.h"

//static char s_silc_mgmtd_daemon_rsp_send_buff[SILC_MGMTD_IF_BUFFER_MAX_LEN];

void* silc_mgmtd_daemon_request_loop(void* thread_arg)
{
	silc_list request_list;
	silc_list req_tmo_list;
	silc_mgmtd_msg *p_request_msg, *p_tmp_op;
	silc_list_init(&request_list);
	silc_list_init(&req_tmo_list);

	while(1)
	{
		silc_list_for_each_entry_safe(p_request_msg, p_tmp_op, &request_list, node)
		{
			if(p_request_msg->state == OP_STATE_FINISHED)
			{
				silc_mgmtd_req_dbg(p_request_msg, "Done", "");

				silc_list_del(&p_request_msg->node);
				if(silc_mgmtd_if_server_send_msg(p_request_msg) < 0)
				{
					SILC_ERR("[%s] Send response failed!!\n", __func__);
				}
				silc_mgmtd_if_client_free_msg(p_request_msg);
			}
			else if(silc_mgmtd_op_timedout(p_request_msg))
			{
				SILC_ERR("Request timed out");
				silc_list_del(&p_request_msg->node);
				silc_list_add_tail(&p_request_msg->node, &req_tmo_list);
			}
		}

		//read a message, we wake every ms if there's no message, to do the above cleanup.
		p_request_msg = silc_mgmtd_if_server_pop_msg(1000);
		if(p_request_msg)
		{
			p_request_msg->state = OP_STATE_NULL;
			if(silc_mgmtd_op_do(p_request_msg, 60) < 0)
			{//TODO: need to retrieve mgmtd errno and send fail message to client!!!!
				silc_mgmtd_if_client_free_msg(p_request_msg);
			}
			else
			{
				silc_list_add_tail(&p_request_msg->node, &request_list);
			}
		}
	}
	return NULL;
}

int silc_mgmtd_action_boot_node(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	int ret;

	if(p_db_node->type == MEMDB_NODE_TYPE_BOOT) // enable add new node tree
	{
		SILC_TRACE("[%s] do action %s!", __func__, path);
		ret = silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_ACTION, p_db_node, p_db_node->do_ops_timeout, NULL);
		if(ret != 0)
			return ret;
	}

	return 0;
}

int silc_mgmtd_daemon_action_boot()
{
	silc_mgmtd_node* p_node;

	p_node = silc_mgmtd_memdb_find_node("/action");
	if(!p_node)
		return -1;

	return silc_mgmtd_memdb_node_tree_traversal("/", p_node, silc_mgmtd_action_boot_node, NULL);
}


int silc_mgmtd_daemon_start(const char* bind_addr, int listen_port)
{
	if(silc_mgmtd_daemon_action_boot() < 0)
		return -1;

	if(silc_mgmtd_if_server_init(bind_addr, listen_port, -1) < 0)
		return -1;

	return silc_thread_create_detached(silc_mgmtd_daemon_request_loop, NULL);
}
