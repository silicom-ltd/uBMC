/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_operation.h"
#include "is_mgmtd_func_def.h"

int is_mgmtd_system_upgrade_1_to_2()
{
	silc_mgmtd_node* p_old_svr;
	silc_mgmtd_node* p_svr;
	silc_cstr host;
	char path[256];

	p_old_svr = silc_mgmtd_memdb_find_node("/config/system/misc/log/remote-server");
	host = p_old_svr->value.val.string_val;
	if (strlen(host) > 0 && strcmp(host, "0.0.0.0") != 0)
	{
		sprintf(path, "/config/system/misc/log/remote-server-v2/%s", host);
		p_svr = silc_mgmtd_memdb_find_node("/config/system/misc/log/remote-server-v2");
		if(silc_mgmtd_op_add_node(p_svr, path, host) != 0)
			return -1;
	}

	p_old_svr = silc_mgmtd_memdb_find_node("/config/system/misc/datetime/ntp-server");
	host = p_old_svr->value.val.string_val;
	if (strlen(host) > 0)
	{
		sprintf(path, "/config/system/misc/datetime/ntp-server-v2/%s", host);
		p_svr = silc_mgmtd_memdb_find_node("/config/system/misc/datetime/ntp-server-v2");
		if(silc_mgmtd_op_add_node(p_svr, path, host) != 0)
			return -1;
	}

	return 0;
}

int is_mgmtd_system_upgrade(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
    int ret = 0;
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	uint32_t old_ver, cur_ver, new_ver;

	if(type != SILC_MGMTD_IF_REQ_UPGRADE)
		return 0;

	old_ver = p_node->tmp_value.val.uint32_val;
	new_ver = p_node->value.val.uint32_val;

	if(new_ver > old_ver)
	{
		cur_ver = old_ver + 1;
		while(cur_ver <= new_ver)
		{
			switch(cur_ver)
			{
			case 2:
				ret = is_mgmtd_system_upgrade_1_to_2();
				break;
			default:
				return -1;
			}
			cur_ver++;
		}
	}

    return ret;
}

