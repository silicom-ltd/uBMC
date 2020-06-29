
#include "silc_common.h"
#include "silc_common/silc_thread.h"

#include "silc_mgmtd_error.h"
#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_config.h"
#include "silc_mgmtd_operation.h"
#include "silc_mgmtd_interface.h"

/*
 * ----------------------------------- HANDLE OPERATION -------------------------------------
 */
/*
 * ADD
 */
int silc_mgmtd_op_add_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	int ret = 0;
	silc_cstr val_str = p_if_node->val_str;

	if(p_db_node->type == MEMDB_NODE_TYPE_DYNAMIC)
	{
		p_db_node->tmp_value.type = SILC_MGMTD_VAR_NULL;
		ret = silc_mgmtd_var_set_by_str(&p_db_node->value, val_str);
	}
	else
	{
		p_db_node->tmp_value.type = p_db_node->value.type;
		ret = silc_mgmtd_var_set_by_str(&p_db_node->tmp_value, val_str);
	}
	if(ret != 0)
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
	return ret;
}

int silc_mgmtd_op_do_add_single_inner(silc_mgmtd_node** pp_added_node, silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry, silc_bool call_add_cb)
{
	int ret = 0;
	silc_mgmtd_node* p_src_node;
	silc_mgmtd_node* p_dst_node;
	char info[200];

//	printf("%s p_db_node:%s, p_if_node:%s\n", __func__, p_db_node->name, p_if_node->name);

	if(silc_list_empty(&p_db_node->sub_node_list))
	{
		silc_mgmtd_memdb_get_node_path(p_db_node, info);
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_PARENT_NO_CHILD, info);
		return -1;
	}

	p_src_node = silc_list_entry(p_db_node->sub_node_list.next, silc_mgmtd_node, node);
	if(p_src_node->type != MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_mgmtd_memdb_get_node_path(p_src_node, info);
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, info);
		return -1;
	}

	p_dst_node = silc_mgmtd_memdb_find_sub_value(p_db_node, p_if_node->val_str);
	if(p_dst_node)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_EXIST, p_if_node->val_str);
		return -1;
	}

	p_dst_node = silc_mgmtd_memdb_clone_node(p_src_node);
	if(!p_dst_node)
		return -1;

	// change the default configuration
	if(silc_mgmtd_memdb_dnode_tree_traversal(p_dst_node, p_if_node, silc_mgmtd_op_add_traversal_cb, NULL) < 0)
		return -1;

	if(call_add_cb)
	{
		if((ret=silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_CHECK_ADD, p_dst_node, p_dst_node->do_ops_timeout, conn_entry)) != 0)
		{
			silc_mgmtd_memdb_delete_node(p_dst_node);
			return ret;
		}
	}
	silc_mgmtd_memdb_insert_node(p_dst_node, p_db_node);
	silc_mgmtd_memdb_modify_node_confirm_tree(p_dst_node);
	if(pp_added_node)
		*pp_added_node = p_dst_node;
	return ret;
}

int silc_mgmtd_op_do_add_single(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry, silc_bool call_add_cb)
{
	int ret = 0;
	silc_mgmtd_node* p_added_node;

	if((ret=silc_mgmtd_op_do_add_single_inner(&p_added_node, p_db_node, p_if_node, conn_entry, call_add_cb)) != 0)
		return ret;

	if(call_add_cb)
		return silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_ADD, p_added_node, p_added_node->do_ops_timeout, conn_entry);
	else
		return 0;
}

int silc_mgmtd_op_do_add_sub(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	int ret = 0;
	silc_mgmtd_if_node* p_if_sub_node;
	silc_mgmtd_node* p_db_sub_node, *p_src_node;

//	printf("%s p_db_node:%s, p_if_node:%s\n", __func__, p_db_node->name, p_if_node->name);

	if(silc_list_empty(&p_db_node->sub_node_list))
		return 0;
	p_src_node = silc_list_entry(p_db_node->sub_node_list.next, silc_mgmtd_node, node);
	if(p_src_node->type == MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
		{
			if((ret=silc_mgmtd_op_do_add_single_inner(NULL, p_db_node, p_if_sub_node, conn_entry, silc_false)) != 0)
				return ret;
			p_db_sub_node = silc_mgmtd_memdb_find_sub_value(p_db_node, p_if_sub_node->name);
			if(p_db_sub_node)
			{
				if((ret=silc_mgmtd_op_do_add_sub(p_db_sub_node, p_if_sub_node, conn_entry)) != 0)
					return ret;
			}
		}
	}
	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
//		printf("[%s]compare, db:%s\n", __func__, p_db_sub_node->name);
		if(p_db_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
		{
//			printf("[%s]compare, db:%s, if:%s\n", __func__, p_db_sub_node->name, p_if_sub_node->name);
			if(strcmp(p_db_sub_node->name, p_if_sub_node->name) == 0)
			{
				if((ret=silc_mgmtd_op_do_add_sub(p_db_sub_node, p_if_sub_node, conn_entry)) != 0)
					return ret;
			}
		}
	}
	return 0;
}

int silc_mgmtd_op_do_add_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	int ret=0;
	silc_mgmtd_node* p_added_node;
	silc_mgmtd_node* p_node;

	if((ret=silc_mgmtd_op_do_add_single_inner(&p_node, p_db_node, p_if_node, conn_entry, silc_true)) !=0 )
		return ret;
	p_added_node = silc_mgmtd_memdb_find_sub_value(p_db_node, p_if_node->val_str);
	if(p_added_node)
	{
		if((ret=silc_mgmtd_op_do_add_sub(p_added_node, p_if_node, conn_entry)) != 0)
			return ret;
	}

	if((ret=silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_ADD, p_node, p_node->do_ops_timeout, conn_entry)) != 0)
		return ret;

	silc_mgmtd_memdb_set_config_unsave();
	return 0;
}

/*
 * MODIFY
 */
int silc_mgmtd_op_modify_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	return silc_mgmtd_memdb_modify_node(p_db_node, p_if_node->val_str);
}

int silc_mgmtd_op_modify_clean_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	silc_mgmtd_var_clear(&p_db_node->tmp_value);
	return 0;
}

int silc_mgmtd_op_modify_confirm_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	return silc_mgmtd_memdb_modify_node_confirm(p_db_node);
}

int silc_mgmtd_op_do_modify_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	int ret = 0;

	// change the default configuration
	if(silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_modify_traversal_cb, NULL) < 0)
		return -1;

	ret = silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_CHECK_MODIFY, p_db_node, p_db_node->do_ops_timeout, conn_entry);
	if(ret == 0)
	{
		if(silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_modify_confirm_traversal_cb, NULL) == 0)
		{
			ret = silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_MODIFY, p_db_node, p_db_node->do_ops_timeout, conn_entry);
		}
	}

	silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_modify_clean_traversal_cb, NULL);
	silc_mgmtd_memdb_set_config_unsave();
	return ret;
}

/*
 * DELETE
 */
int silc_mgmtd_op_do_delete_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	int ret = 0;
	silc_mgmtd_node* p_node;
	silc_mgmtd_node_ref* p_ref;

	if(p_db_node->type != MEMDB_NODE_TYPE_DYNAMIC)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, __func__);
		return -1;
	}

	// find template
	p_node = silc_list_entry(p_db_node->p_parent->sub_node_list.next, silc_mgmtd_node, node);
	if(p_node->type != MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, __func__);
		return -1;
	}

	if((ret=silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_CHECK_DELETE, p_db_node, p_db_node->do_ops_timeout, conn_entry)) != 0)
		return ret;

	if(p_node != p_db_node)
	{
		silc_list_for_each_entry(p_ref, &p_node->ref_node_list, node)
		{
			if(p_ref->do_ref)
			{
				ret = p_ref->do_ref(p_ref->path, p_db_node);
				if(ret != 0)
				{
					silc_mgmtd_err_set(MGMTD_ERR_CALLBACK, p_ref->path);
					return ret;
				}
			}
		}
	}

	ret = silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_DELETE, p_db_node, p_db_node->do_ops_timeout, conn_entry);
	if(ret == 0)
	{
		silc_list_del(&p_db_node->node);
		silc_mgmtd_memdb_delete_node(p_db_node);
		silc_mgmtd_memdb_set_config_unsave();
		return 0;
	}
	silc_mgmtd_err_set(MGMTD_ERR_CALLBACK, __func__);
	return ret;
}

/*
 * ACTION
 */
int silc_mgmtd_op_do_action_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	silc_mgmtd_node* p_db_sub_node;
	silc_mgmtd_if_node* p_if_sub_node;

	// clear
	silc_mgmtd_var_clear(&p_db_node->tmp_value);
	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
		silc_mgmtd_var_clear(&p_db_sub_node->tmp_value);
	}

	// set action parameter
	p_db_node->tmp_value.type = p_db_node->value.type;
	if(silc_mgmtd_var_set_by_str(&p_db_node->tmp_value, p_if_node->val_str) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, p_if_node->val_str);
		return -1;
	}
	silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
	{
		p_db_sub_node = silc_mgmtd_memdb_find_sub_node(p_db_node, p_if_sub_node->name);
		if(!p_db_sub_node)
			return -1;
		p_db_sub_node->tmp_value.type = p_db_sub_node->value.type;
		if(silc_mgmtd_var_set_by_str(&p_db_sub_node->tmp_value, p_if_sub_node->val_str) < 0)
		{
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, p_if_node->val_str);
			return -1;
		}
	}
	return silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_ACTION, p_db_node, p_db_node->do_ops_timeout, conn_entry);
}

/*
 * NOTIFY
 */
int silc_mgmtd_op_do_notify_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	silc_mgmtd_node* p_db_sub_node;
	silc_mgmtd_if_node* p_if_sub_node;

	// clear
	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
		p_db_sub_node->tmp_value.type = SILC_MGMTD_VAR_NULL;
	}

	// set notify parameter
	silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
	{
		p_db_sub_node = silc_mgmtd_memdb_find_sub_node(p_db_node, p_if_sub_node->name);
		if(!p_db_sub_node)
			return -1;
		p_db_sub_node->tmp_value.type = p_db_sub_node->value.type;
		if(silc_mgmtd_var_set_by_str(&p_db_sub_node->tmp_value, p_if_sub_node->val_str) < 0)
		{
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, p_if_sub_node->val_str);
			return -1;
		}
	}
	return silc_mgmtd_op_do_callback(SILC_MGMTD_IF_REQ_NOTIFY, p_db_node, p_db_node->do_ops_timeout, conn_entry);
}

/*
 * QUERY
 */
int silc_mgmtd_op_query_node_do(silc_mgmtd_if_req_type type,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, int timeout, void* conn_entry)
{
	int ret = 0;
	char val_str[SILC_MGMTD_VAR_VAL_STR_MAX_LEN];

	if((ret=silc_mgmtd_op_do_callback(type, p_db_node, p_db_node->do_ops_timeout, conn_entry)) != 0)
		return ret;

	if(!silc_mgmtd_var_to_str(&p_db_node->value, val_str, SILC_MGMTD_VAR_VAL_STR_MAX_LEN))
	{
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
		return -1;
	}
	if(silc_mgmtd_if_server_node_set_val(p_if_node, val_str) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, val_str);
		return -1;
	}
	return 0;
}

int silc_mgmtd_op_query_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	return silc_mgmtd_op_query_node_do(SILC_MGMTD_IF_REQ_QUERY,
			p_if_node, p_db_node, p_db_node->do_ops_timeout, data);
}

int silc_mgmtd_op_do_query_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	return silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_query_traversal_cb, conn_entry);
}

/*
 * QUERY CHILD
 */
int silc_mgmtd_op_query_child_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	int ret = 0;
	silc_mgmtd_node* p_sub_node;
	silc_mgmtd_if_node* p_if_sub_node;
	char val_str[SILC_MGMTD_VAR_VAL_STR_MAX_LEN];

	if((ret=silc_mgmtd_op_query_node_do(SILC_MGMTD_IF_REQ_QUERY_CHILD,
			p_if_node, p_db_node, p_db_node->do_ops_timeout, data)) != 0)
		return ret;

	silc_list_for_each_entry(p_sub_node, &p_db_node->sub_node_list, node)
	{
		if(p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE ||
				p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB ||
				p_sub_node->type == MEMDB_NODE_TYPE_UNKNOWN)
			continue;
		if(p_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
			p_if_sub_node = silc_mgmtd_if_server_node_add(
					silc_mgmtd_var_to_str(&p_sub_node->value, val_str, SILC_MGMTD_VAR_VAL_STR_MAX_LEN), "", p_if_node);
		else
			p_if_sub_node = silc_mgmtd_if_server_node_add(p_sub_node->name, "", p_if_node);
		if(!p_if_sub_node)
		{
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
			return -1;
		}
		if((ret=silc_mgmtd_op_query_node_do(SILC_MGMTD_IF_REQ_QUERY_CHILD,
						p_if_sub_node, p_sub_node, p_sub_node->do_ops_timeout, data)) != 0)
			return ret;
	}
	return 0;
}

int silc_mgmtd_op_do_query_child_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	return silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_query_child_traversal_cb, conn_entry);
}


/*
 * QUERY SUB
 */
int silc_mgmtd_op_query_sub_traversal_cb(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* data)
{
	int ret = 0;
	silc_mgmtd_node* p_sub_node;
	silc_mgmtd_if_node* p_if_sub_node;
	char cur_path[SILC_MGMTD_IF_PATH_MAX_LEN] = "";
	char val_str[SILC_MGMTD_VAR_VAL_STR_MAX_LEN];

	if((ret=silc_mgmtd_op_query_node_do(SILC_MGMTD_IF_REQ_QUERY_SUB,
			p_if_node, p_db_node, p_db_node->do_ops_timeout, data)) != 0)
		return ret;

	if(strlen(path))
		strcat(cur_path, path);
	if(strlen(p_if_node->name))
	{
		if(strlen(cur_path))
			strcat(cur_path, "/");
		strcat(cur_path, p_if_node->name);
	}

	silc_list_for_each_entry(p_sub_node, &p_db_node->sub_node_list, node)
	{
		if(p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE ||
				p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB ||
				p_sub_node->type == MEMDB_NODE_TYPE_UNKNOWN)
			continue;
		if(p_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
			p_if_sub_node = silc_mgmtd_if_server_node_add(
					silc_mgmtd_var_to_str(&p_sub_node->value, val_str, SILC_MGMTD_VAR_VAL_STR_MAX_LEN), "", p_if_node);
		else
			p_if_sub_node = silc_mgmtd_if_server_node_add(p_sub_node->name, "", p_if_node);
		if(!p_if_sub_node)
		{
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
			return -1;
		}
		if((ret=silc_mgmtd_op_query_sub_traversal_cb(cur_path, p_if_sub_node, p_sub_node, data)) != 0)
			return ret;
	}
	return 0;
}

int silc_mgmtd_op_do_query_sub_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_node* p_if_node, void* conn_entry)
{
	return silc_mgmtd_memdb_dnode_tree_traversal(p_db_node, p_if_node, silc_mgmtd_op_query_sub_traversal_cb, conn_entry);
}


/*
 * QUERY vnode
 */
int silc_mgmtd_op_do_query_vnode_request(silc_mgmtd_node* p_db_node, silc_mgmtd_if_req* p_req)
{
	return silc_mgmtd_op_do_callback(p_req->type, p_db_node, p_db_node->do_ops_timeout, (void*)p_req);
}


/*
 * -------------------------------- GENERATE RESPONSE -----------------------------------
 */
/*
 * ERROR RESPONSE
 */
silc_mgmtd_if_rsp* silc_mgmtd_op_gen_op_err_rsp(const silc_cstr path, const silc_cstr err_str)
{
	silc_mgmtd_if_rsp* p_rsp;
	silc_cstr cur_err_str;

	if(!err_str)
		cur_err_str = silc_mgmtd_err_str();
	else
		cur_err_str = err_str;

	if(!(p_rsp=silc_mgmtd_if_rsp_create(path, cur_err_str, "")))
	{
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
	}
	return p_rsp;
}

/*
 * EMPTY RESPONSE
 */
silc_mgmtd_if_rsp* silc_mgmtd_op_gen_op_empty_rsp()
{
	silc_mgmtd_if_rsp* p_rsp = silc_mgmtd_if_rsp_create("", "OK", "");
	if(!p_rsp)
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
	return p_rsp;
}

/*
 * --------------------------------- HANDLE REQUEST ------------------------------------
 */
silc_mgmtd_if_rsp* silc_mgmtd_op_req_handle(silc_mgmtd_if_req* p_req)
{
	int ret;
	silc_cstr err_str = NULL;
	silc_bool is_vnode_query = silc_false;
	silc_mgmtd_node* p_db_node;
	silc_mgmtd_node* p_cfg_module;

	if(p_req->type == SILC_MGMTD_IF_REQ_ADD)
		p_db_node = silc_mgmtd_memdb_find_parent(p_req->path_str);
	else if(strncmp(p_req->path_str, "/status/", strlen("/status/")) == 0 ||
			strncmp(p_req->path_str, "/stats/", strlen("/stats/")) == 0 )
	{
		is_vnode_query = silc_true;
		p_db_node = silc_mgmtd_memdb_find_node_top(p_req->path_str);
	}
	else
		p_db_node = silc_mgmtd_memdb_find_node(p_req->path_str);

	if(!p_db_node)
		goto ERROR_OUT;

	p_cfg_module = silc_mgmtd_memdb_find_config_module_node(p_db_node);

	if(p_cfg_module)
	{
		if(p_req->type == SILC_MGMTD_IF_REQ_ADD ||
				p_req->type == SILC_MGMTD_IF_REQ_MODIFY ||
				p_req->type == SILC_MGMTD_IF_REQ_DELETE)
		{
			silc_mgmtd_memdb_node_w_lock(p_db_node, silc_mgmtd_if_server_get_conn_id(p_req->conn_entry), silc_true);
		}
		else
		{
			if(silc_mgmtd_memdb_node_r_lock(p_db_node) != 0)
				goto ERROR_OUT;
		}
	}

	if(is_vnode_query)
	{
		ret = silc_mgmtd_op_do_query_vnode_request(p_db_node, p_req);
	}
	else
	{
		switch(p_req->type)
		{
		case SILC_MGMTD_IF_REQ_ADD:
			ret = silc_mgmtd_op_do_add_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = silc_mgmtd_op_do_modify_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_DELETE:
			ret = silc_mgmtd_op_do_delete_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_QUERY:
			ret = silc_mgmtd_op_do_query_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_QUERY_CHILD:
			ret = silc_mgmtd_op_do_query_child_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_QUERY_SUB:
			ret = silc_mgmtd_op_do_query_sub_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_QUERY_WILD:
			ret = silc_mgmtd_op_do_query_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_ACTION:
			ret = silc_mgmtd_op_do_action_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_NOTIFY:
			ret = silc_mgmtd_op_do_notify_request(p_db_node, p_req->p_node_root, p_req->conn_entry);
			break;

		default:
			ret = -1;
		}
	}
	if(p_cfg_module)
	{
		if(p_req->type == SILC_MGMTD_IF_REQ_ADD ||
				p_req->type == SILC_MGMTD_IF_REQ_MODIFY ||
				p_req->type == SILC_MGMTD_IF_REQ_DELETE)
		{
			//p_db_node will be freed in silc_mgmtd_op_do_delete_request() if OK
			if(p_req->type != SILC_MGMTD_IF_REQ_DELETE || ret != 0)
				silc_mgmtd_memdb_node_w_unlock(p_db_node);
		}
		else
		{
			silc_mgmtd_memdb_node_r_unlock(p_db_node);
		}
	}

	if(ret != 0)
	{
		if(ret != -1)
		{
			silc_mgmtd_err_set(ret, p_req->path_str);
			err_str = silc_mgmtd_memdb_get_cb_err(p_db_node, ret);
			if(!err_str)
				silc_mgmtd_err_set(MGMTD_ERR_MEMDB_CB_ERR_NOTFIND, p_req->path_str);
		}
		goto ERROR_OUT;
	}

	silc_list_del(&p_req->node);
	return p_req;

ERROR_OUT:
	return silc_mgmtd_op_gen_op_err_rsp(p_req->path_str, err_str);
}


void* silc_mgmtd_op_handle_entry(void* thread_arg)
{
	silc_mgmtd_msg* p_op = (silc_mgmtd_msg*)thread_arg;
	silc_mgmtd_if_req* p_req, *p_tmp_req;
	silc_mgmtd_if_rsp* p_rsp;

	p_op->state = OP_STATE_RUNNING;

	silc_mgmtd_req_dbg(p_op, "process", "");

	if(silc_list_empty(&p_op->if_recv_items))
	{
		p_rsp = silc_mgmtd_op_gen_op_empty_rsp();
		if(p_rsp)
			silc_list_add_tail(&p_rsp->node, &p_op->if_send_items);
	}

	silc_list_for_each_entry_safe(p_req, p_tmp_req, &p_op->if_recv_items, node)
	{
		p_rsp = silc_mgmtd_op_req_handle(p_req);
		if(!p_rsp)
		{
			p_op->ret = -1;
			p_op->state = OP_STATE_FINISHED;
			silc_mgmtd_req_dbg(p_op, "Fail", "");
			return NULL;
		}
		silc_list_add_tail(&p_rsp->node, &p_op->if_send_items);
	}

	p_op->ret = 0;
	p_op->state = OP_STATE_FINISHED;
	silc_mgmtd_req_dbg(p_op, "Finish", "");
	return NULL;
}

int silc_mgmtd_op_handle_wild_path(silc_list* p_req_list)
{
	char new_path[SILC_MGMTD_IF_PATH_MAX_LEN];
	silc_mgmtd_if_req* p_req;
	silc_mgmtd_if_req* p_tmp_req;
	silc_mgmtd_if_req* p_clone_req;
	silc_mgmtd_node* p_node;
	silc_mgmtd_node* p_sub_node;
	silc_bool is_wild = silc_true;
	char val_str[200];

	while(is_wild)
	{
		is_wild = silc_false;
		silc_list_for_each_entry_safe(p_req, p_tmp_req, p_req_list, node)
		{
			silc_cstr wild_head;
			if(p_req->type != SILC_MGMTD_IF_REQ_QUERY_WILD ||
					strncmp(p_req->path_str, "/config", strlen("/config")) != 0)
				continue;

			wild_head = strstr(p_req->path_str, "/*");
			if(wild_head != NULL)
			{
				is_wild = silc_true;
				*wild_head = '\0';
				p_node = silc_mgmtd_memdb_find_node(p_req->path_str);
				if(!p_node)
					return -1;
				silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
				{
					if(p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE)
						continue;
					else if(p_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
					{
						if(strlen(wild_head+1) == 1)
							sprintf(new_path, "%s/%s", p_req->path_str,	silc_mgmtd_var_to_str(&p_sub_node->value, val_str, 200));
						else
							sprintf(new_path, "%s/%s/%s", p_req->path_str,
								silc_mgmtd_var_to_str(&p_sub_node->value, val_str, 200), wild_head+3);
					}
					else
					{
						if(strlen(wild_head+1) == 1)
							sprintf(new_path, "%s/%s", p_req->path_str, p_sub_node->name);
						else
							sprintf(new_path, "%s/%s/%s", p_req->path_str, p_sub_node->name, wild_head+3);
					}
					p_clone_req = silc_mgmtd_if_req_clone(new_path, p_req);
					if(!p_clone_req)
					{
						silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
						return -1;
					}
					silc_list_add(&p_clone_req->node, &p_req->node);
				}
				silc_list_del(&p_req->node);
				silc_mgmtd_if_req_delete(p_req);
			}
		}

	}
	return 0;
}

int silc_mgmtd_op_check_node_privilege(const silc_cstr path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_node* p_db_node, void* cb_data)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_if_privilege *p_level = cb_data;

	if(p_db_node->if_level > *p_level)
		return -1;
#endif
	return 0;
}

int silc_mgmtd_op_check_privilege(silc_mgmtd_msg* p_op, silc_mgmtd_if_privilege level)
{
	silc_mgmtd_node* p_db_node;
	silc_mgmtd_if_req *p_req, *p_tmp_req;
	silc_mgmtd_if_rsp *p_rsp;
	char err_str[255];

	silc_list_for_each_entry_safe(p_req, p_tmp_req, &p_op->if_recv_items, node)
	{
		if(p_req->type == SILC_MGMTD_IF_REQ_QUERY ||
				p_req->type == SILC_MGMTD_IF_REQ_QUERY_CHILD ||
				p_req->type == SILC_MGMTD_IF_REQ_QUERY_SUB ||
				p_req->type == SILC_MGMTD_IF_REQ_QUERY_WILD)
			continue;

		p_db_node = silc_mgmtd_memdb_find_node(p_req->path_str);
		if(!p_db_node)
		{
			if(p_req->type == SILC_MGMTD_IF_REQ_ADD)
			{
				p_db_node = silc_mgmtd_memdb_find_parent(p_req->path_str);
				if(!p_db_node)
				{
					sprintf(err_str, "Can not find the node %s or its parent", p_req->path_str);
					p_rsp = silc_mgmtd_op_gen_op_err_rsp(p_req->path_str, err_str);
					if(!p_rsp)
						return -1;
					silc_list_add_tail(&p_rsp->node, &p_op->if_send_items);
					silc_list_del(&p_req->node);
					silc_mgmtd_if_req_delete(p_req);
					return -1;
				}
			}
		}

		if(silc_mgmtd_memdb_dnode_tree_traversal(p_db_node,
				p_req->p_node_root, silc_mgmtd_op_check_node_privilege, &level) != 0)
		{
			sprintf(err_str, "Privilege %d has no right for %s or its parent", level, p_req->path_str);
			p_rsp = silc_mgmtd_op_gen_op_err_rsp(p_req->path_str, err_str);
			if(!p_rsp)
				return -1;
			silc_list_add_tail(&p_rsp->node, &p_op->if_send_items);
			silc_list_del(&p_req->node);
			silc_mgmtd_if_req_delete(p_req);
			return -1;
		}
	}
	return 0;
}

silc_bool silc_mgmtd_op_timedout(silc_mgmtd_msg* p_op)
{
	return p_op->timeout_abs_ms < silc_time_get_ms();
}

int silc_mgmtd_op_do(silc_mgmtd_msg* p_op, int timeout_sec)
{
	p_op->timeout_abs_ms = silc_time_get_ms() + (timeout_sec*1000);
	if(silc_mgmtd_op_handle_wild_path(&p_op->if_recv_items) < 0)
	{
		return -1;
	}

	if(silc_mgmtd_op_check_privilege(p_op, silc_mgmtd_if_server_get_conn_privilege(p_op->conn_entry)) < 0)
	{
		p_op->state = OP_STATE_FINISHED;
		return 0;
	}

	if(0!=silc_thread_create_detached(silc_mgmtd_op_handle_entry, p_op))
	{
		return -1;
	}

	return 0;
}

typedef struct silc_mgmtd_op_cb_s
{
	silc_mgmtd_if_req_type type;
	silc_mgmtd_if_privilege level;
	silc_mgmtd_node* p_node;
	silc_mgmtd_op_state state;
	silc_thread_id		thread;
	void* data;
	int ret;
}silc_mgmtd_op_cb;

void* silc_mgmtd_op_do_callback_entry(void* thread_arg)
{
	silc_mgmtd_op_cb* p_cb = (silc_mgmtd_op_cb*)thread_arg;
	p_cb->state = OP_STATE_RUNNING;
	p_cb->ret = p_cb->p_node->do_ops(p_cb->type, p_cb->p_node, p_cb->data);
#if 0
	if(p_cb->state == OP_STATE_TIMEOUT)
		free(p_cb);
	else
		p_cb->state = OP_STATE_FINISHED;
#endif
	SILC_TRACE("Callback op done %p", p_cb);
	return NULL;
}

static silc_bool s_silc_mgmtd_dryrun = silc_false;

void silc_mgmtd_enable_dryrun()
{
	s_silc_mgmtd_dryrun = silc_true;
}

void silc_mgmtd_disable_dryrun()
{
	s_silc_mgmtd_dryrun = silc_false;
}

silc_bool silc_mgmtd_is_dryrun()
{
	return s_silc_mgmtd_dryrun;
}

int silc_mgmtd_op_do_callback(silc_mgmtd_if_req_type type, void* p_db_node, int timeout, void* data)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	if(p_node->do_ops == NULL)
	{
		silc_mgmtd_err_set(MGMTD_ERR_OP_NOT_SUPPORTED, NULL);
		return -1;
	}
	if(silc_mgmtd_is_dryrun() &&
			(SILC_MGMTD_IF_REQ_ADD == type ||
			SILC_MGMTD_IF_REQ_MODIFY == type ||
			SILC_MGMTD_IF_REQ_DELETE == type))
		//don't apply config when dryrun
		return 0;
	return p_node->do_ops(type, p_node, data);
}

int silc_mgmtd_op_do_callback_thread(silc_mgmtd_if_req_type type, void* p_db_node, int timeout, void* data)
{
	int ret = 0;
//	int count = timeout*1000;
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	silc_mgmtd_op_cb* p_cb;

	if(!p_node->do_ops)
		return 0;

	p_cb = malloc(sizeof(silc_mgmtd_op_cb));
	if(!p_cb)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}
	SILC_TRACE("Callback op started %p", p_cb);
	p_cb->type = type;
	p_cb->state = OP_STATE_NULL;
	p_cb->ret = 0;
	p_cb->data = data;
	p_cb->p_node = (silc_mgmtd_node*)p_db_node;

	SILC_TRACE("Callback op starting %p", p_cb);
	p_cb->thread = silc_thread_create(silc_mgmtd_op_do_callback_entry, p_cb);
	if(p_cb->thread ==NULL)
	{
		SILC_ERR("Callback op failed to start %p", p_cb);
	}

#if 0
	while(count--)
	{
		if(p_cb->state == OP_STATE_FINISHED)
		{
			ret = p_cb->ret;
			free(p_cb);
			if(ret != 0)
			{
				char info[200];
				silc_mgmtd_memdb_get_node_path(p_node, info);
				silc_mgmtd_err_set(MGMTD_ERR_CALLBACK, info);
			}
			return ret;
		}
		usleep(1000);
	}
	p_cb->state = OP_STATE_TIMEOUT;
	free(p_cb);
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_CB_TIMEOUT, __func__);
	return -1;
#else
	silc_thread_wait_destroy(p_cb->thread);
	SILC_TRACE("Callback op result %p ret %d state %u", p_cb, p_cb->ret, p_cb->state);
	ret = p_cb->ret;
	free(p_cb);
	if(ret != 0)
	{
		char info[200];
		silc_mgmtd_memdb_get_node_path(p_node, info);
		silc_mgmtd_err_set(MGMTD_ERR_CALLBACK, info);
	}
	return ret;
#endif
}

int silc_mgmtd_op_external_run(const silc_cstr cmd)
{
	int ret = 0;
	ret = silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 2000, silc_false);
	usleep(10000);
	return ret;
}

int silc_mgmtd_op_add_node(silc_mgmtd_node* p_node, silc_cstr path, silc_cstr val_str)
{
	silc_mgmtd_if_req* p_sim_req;

	p_sim_req = silc_mgmtd_if_req_create(path, SILC_MGMTD_IF_REQ_ADD, val_str);
	if(!p_sim_req)
		return -1;

	if(silc_mgmtd_op_do_add_single(p_node, p_sim_req->p_node_root, NULL, silc_false) != 0)
		return -1;
	return 0;
}
