#include "silc_common.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_vnode.h"



typedef struct silc_mgmtd_vnode_retrieve_arg_static_s
{
	silc_bool type;
	silc_bool allow_sub;
	uint32_t query_level_src_max;
	uint32_t query_level_max;
	silc_cstr_array* p_level_arr;
	silc_mgmtd_vnode_retrieve_level_get_cbf get_level_arr;
}silc_mgmtd_vnode_retrieve_arg_static;

int silc_mgmtd_module_vnode_retrieve_recursive(silc_mgmtd_if_node* p_parent_node, uint32_t curr_level, silc_mgmtd_vnode_retrieve_arg_static* p_static_arg, silc_bool add_node)
{
	int ret;
	silc_mgmtd_if_node* p_child_node;
	silc_cstr curr_level_name = NULL;
	uint32_t next_level = curr_level + 1;

	if(curr_level < p_static_arg->query_level_src_max)
		curr_level_name = silc_cstr_array_get_quick(p_static_arg->p_level_arr, curr_level);
	//get child, curr_level_name could be NULL, which means all child
	if(curr_level_name && strcmp(curr_level_name, "*") == 0)
		curr_level_name = NULL;//wild matching for this level
	ret = p_static_arg->get_level_arr(p_parent_node, curr_level, p_static_arg->p_level_arr, curr_level_name, add_node);
	if(ret != 0)
		return ret;

	if(p_static_arg->allow_sub || next_level < p_static_arg->query_level_max)
	{
		if(!add_node)
		{
			ret = silc_mgmtd_module_vnode_retrieve_recursive(p_parent_node, next_level, p_static_arg, silc_true);
			if(ret != 0)
				return ret;
		}
		else
		{
			silc_list_for_each_entry(p_child_node, &p_parent_node->sub_node_list, node)
			{
				if(p_child_node->has_child)
				{
					silc_cstr_array_set_no_copy(p_static_arg->p_level_arr, curr_level, p_child_node->name);
					ret = silc_mgmtd_module_vnode_retrieve_recursive(p_child_node, next_level, p_static_arg, silc_true);
					if(ret != 0)
						return ret;
				}
			}
			silc_cstr_array_set_no_copy(p_static_arg->p_level_arr, next_level, NULL);
		}
	}
	return 0;
}




int silc_mgmtd_module_vnode_retrieve(silc_mgmtd_if_req_type type, void* p_db_node, void* data,
		silc_mgmtd_vnode_retrieve_level_get_cbf p_query_level_get)
{
	int ret;
	uint32_t loop, start_level;
	silc_mgmtd_if_req* p_req = (silc_mgmtd_if_req*)data;
	silc_cstr_array* p_orig_arr = NULL;
	silc_cstr_array* p_query_arr = NULL;
	silc_cstr path_rewrite = p_req->path_str;
	silc_cstr curr_name;

	silc_mgmtd_vnode_retrieve_arg_static	query_arg;

	if(type != SILC_MGMTD_IF_REQ_QUERY &&
			type != SILC_MGMTD_IF_REQ_QUERY_CHILD &&
			type != SILC_MGMTD_IF_REQ_QUERY_SUB &&
			type != SILC_MGMTD_IF_REQ_QUERY_WILD)
		return 0;

	if(strstr(p_req->path_str, "/*") && type != SILC_MGMTD_IF_REQ_QUERY_WILD)
		return 0;

	query_arg.allow_sub = silc_false;
	query_arg.get_level_arr = p_query_level_get;
	p_orig_arr = silc_cstr_split(p_req->path_str, "/");
	if(!p_orig_arr)
	{
		return 1;
	}
	query_arg.query_level_max = p_orig_arr->length;
	query_arg.query_level_src_max = p_orig_arr->length;
//	fprintf(stderr, "Query root was %s\n", p_req->path_str);
	switch(type)
	{
	case SILC_MGMTD_IF_REQ_QUERY:		// query the node itself
		start_level = p_orig_arr->length-1;
		break;
	case SILC_MGMTD_IF_REQ_QUERY_CHILD: 	// query the son and the daughter
		query_arg.query_level_max++;
		start_level = p_orig_arr->length-1;
		break;
	case SILC_MGMTD_IF_REQ_QUERY_SUB:	// query all the offspring.
		start_level = p_orig_arr->length-1;
		query_arg.allow_sub = silc_true;
		break;
	case SILC_MGMTD_IF_REQ_QUERY_WILD:
		//use the level before the first * as start_level
		//also need to rewrite path_str,
		for(loop = 0; loop < p_orig_arr->length; loop++)
		{
			curr_name = silc_cstr_array_get_quick(p_orig_arr, loop);
			if(strcmp(curr_name, "*")==0)
				break;
			path_rewrite += sprintf(path_rewrite, "/%s", curr_name);
		}
		fprintf(stderr, "Query root rewrite to %s\n", p_req->path_str);
		if(loop<2)
			return 1;
		start_level = loop - 1;
		break;
	default:
		return 1;
	}
	if(p_orig_arr->length < 2 || p_orig_arr->length > SILC_MGMTD_QUERY_MAX_LEVEL)
		goto ERR_RET;

	//we start from root_node, which is /stats/module,
	//first curr level is 2,
	//we copy from orig array to query arrary, its legnth will be set to max
	//the query array will be modified internally
	p_query_arr = silc_cstr_array_create(SILC_MGMTD_QUERY_MAX_LEVEL, SILC_MGMTD_IF_PATH_MAX_LEN);
	if(p_query_arr == NULL)
		goto ERR_RET;
	p_query_arr->length = SILC_MGMTD_QUERY_MAX_LEVEL;
	for(loop = 0; loop < p_orig_arr->length; loop++)
		silc_cstr_array_set_no_copy(p_query_arr, loop, silc_cstr_array_get_quick(p_orig_arr, loop));
	query_arg.p_level_arr = p_query_arr;
	ret = silc_mgmtd_module_vnode_retrieve_recursive(p_req->p_node_root, start_level, &query_arg, silc_false);
	if(ret != 0)
		goto ERR_RET;

	silc_cstr_array_destroy(p_query_arr);
	silc_cstr_array_destroy(p_orig_arr);
	return ret;
ERR_RET:
	if(p_orig_arr)
		silc_cstr_array_destroy(p_orig_arr);
	if(p_query_arr)
		silc_cstr_array_destroy(p_query_arr);
	return ret;
}
