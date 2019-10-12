
#ifndef SILC_MGMTD_VNODE_H_
#define SILC_MGMTD_VNODE_H_

#include "silc_mgmtd_interface.h"

#define silc_mgmtd_vnode_ret_errno(_err)	\
	do { errno = _err; return -1; }while(0)


typedef int (*silc_mgmtd_vnode_retrieve_level_get_cbf)(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node);

int silc_mgmtd_module_vnode_retrieve(silc_mgmtd_if_req_type type, void* p_db_node, void* data,
		silc_mgmtd_vnode_retrieve_level_get_cbf p_query_level_get);

#define silc_mgmtd_vnode_set_node_value(p_ret_node, p_node, node_name, node_value_str) \
		do { \
			if(add_node) \
				p_ret_node = silc_mgmtd_if_req_add_node_ext(p_node, node_name, node_value_str); \
			else { \
				if(0==silc_mgmtd_if_req_reset_node(p_node, node_value_str)) \
					p_ret_node = p_node; \
				else p_ret_node = NULL; \
			} \
			if(p_ret_node == NULL) { errno = ENOMEM; goto ERR_RET; } \
		} while(0)


#define silc_mgmtd_vnode_add_maybe(p_parent, query_node_name, field_name, value, field_type, child)	({\
		silc_mgmtd_if_node* __p_ret_node = NULL; \
		if(query_node_name == NULL || strcmp(query_node_name, field_name)==0) {\
			silc_mgmtd_var_##field_type##_to_str(value , add_node_tmp_str, sizeof(add_node_tmp_str) - 1); \
			silc_mgmtd_vnode_set_node_value(__p_ret_node, p_parent, field_name, add_node_tmp_str); \
			__p_ret_node->has_child = child; \
		} __p_ret_node;})

#define silc_mgmtd_vnode_add_by_field_maybe(p_parent, query_node_name, field_name, field_type, child)	\
		silc_mgmtd_vnode_add_maybe(p_parent, query_node_name, #field_name, p_shm->field_name, field_type, child)

#endif /* SILC_MGMTD_VNODE_H_ */
