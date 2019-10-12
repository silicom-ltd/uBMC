
#ifndef SILC_MGMTD_OPERATION_H_
#define SILC_MGMTD_OPERATION_H_

#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_memdb.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct silc_mgmtd_op_s
{
	silc_list_node node;
	silc_list if_req_list;
	silc_list if_rsp_list;
	silc_mgmtd_op_state state;
	void* conn_entry;
	uint32_t seq_num;
	int timeout;
	int ret;
}silc_mgmtd_op;
#endif

int silc_mgmtd_op_do(silc_mgmtd_msg* p_op, int timeout_sec);
silc_bool silc_mgmtd_op_timedout(silc_mgmtd_msg* p_op);

int silc_mgmtd_op_do_callback(silc_mgmtd_if_req_type type, void* p_db_node, int timeout, void* conn_entry);
int silc_mgmtd_op_do_callback_thread(silc_mgmtd_if_req_type type, void* p_db_node, int timeout, void* conn_entry);
int silc_mgmtd_op_do_add_single(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry, silc_bool call_add_cb);
int silc_mgmtd_op_external_run(const silc_cstr cmd);
int silc_mgmtd_op_add_node(silc_mgmtd_node* p_node, silc_cstr path, silc_cstr val_str);

void silc_mgmtd_enable_dryrun();
void silc_mgmtd_disable_dryrun();
silc_bool silc_mgmtd_is_dryrun();

#ifdef __cplusplus
}
#endif

#endif /* SILC_MGMTD_OPERATION_H_ */
