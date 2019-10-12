
#include "silc_common.h"
#include "silc_mgmtd_error.h"
#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_interface.h"

static silc_mgmtd_node s_silc_mgmtd_memdb_root;
static silc_bool s_silc_mgmtd_memdb_config_unsave = silc_false;

int silc_mgmtd_memdb_init()
{
#ifdef SILC_MGMTD_LOCAL_DEBUG
	silc_log_level_set(SILC_LOG_LEVEL_DEBUG);
#endif
	silc_list_init(&s_silc_mgmtd_memdb_root.sub_node_list);
	silc_list_init(&s_silc_mgmtd_memdb_root.do_ops_err_list);
	silc_list_init(&s_silc_mgmtd_memdb_root.ref_node_list);
    return 0;
}

void silc_mgmtd_memdb_deinit()
{
	silc_mgmtd_node *p_sub_node, *p_tmp_node;

	silc_list_for_each_entry_safe(p_sub_node, p_tmp_node, &s_silc_mgmtd_memdb_root.sub_node_list, node)
	{
		silc_mgmtd_memdb_delete_node(p_sub_node);
	}
	silc_list_init(&s_silc_mgmtd_memdb_root.sub_node_list);
}

void silc_mgmtd_memdb_set_config_saved()
{
	s_silc_mgmtd_memdb_config_unsave = silc_false;
}

void silc_mgmtd_memdb_set_config_unsave()
{
	s_silc_mgmtd_memdb_config_unsave = silc_true;
}

silc_bool silc_mgmtd_memdb_is_config_saved()
{
	return !s_silc_mgmtd_memdb_config_unsave;
}

/*
 * node lock
 */
pthread_mutex_t* silc_mgmtd_memdb_node_create_lock()
{
	pthread_mutex_t* p_lock = malloc(sizeof(pthread_mutex_t));
	if(!p_lock)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return NULL;
	}
	pthread_mutex_init(p_lock, NULL);
	return p_lock;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_config_module_node(silc_mgmtd_node* p_node)
{
	silc_mgmtd_node* p_curr = p_node;
	silc_mgmtd_node* p_parent = p_node->p_parent;

	while(p_parent)
	{
		if(strcmp(p_parent->name, "config") == 0)
		{
			if(!p_node->p_lock)	// module node need a lock
			{
				p_node->p_lock = silc_mgmtd_memdb_node_create_lock();
				if(!p_node->p_lock)
					return NULL;
			}
			return p_node;
		}
		p_curr = p_parent;
		p_parent = p_curr->p_parent;

	}
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_CONFIG_NODE, __func__);
	return NULL;
}


int silc_mgmtd_memdb_node_r_lock(silc_mgmtd_node* p_node)
{
	if(pthread_mutex_trylock(p_node->p_lock) == 0)
	{
		p_node->rlocked_cnt++;
		pthread_mutex_unlock(p_node->p_lock);
	}
	else
	{
		char info[200];
		sprintf(info, "Locked by user %s", silc_mgmtd_if_server_get_conn_index_user(p_node->wlock_owner));
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_LOCKED, info);
		return -1;
	}
	return 0;
}

void silc_mgmtd_memdb_node_r_unlock(silc_mgmtd_node* p_node)
{
	pthread_mutex_lock(p_node->p_lock);
	p_node->rlocked_cnt--;
	pthread_mutex_unlock(p_node->p_lock);
}

int silc_mgmtd_memdb_node_w_lock(silc_mgmtd_node* p_node, uint16_t wlock_owner, silc_bool is_wait)
{
	pthread_mutex_lock(p_node->p_lock);
	if(p_node->rlocked_cnt)
	{
		pthread_mutex_unlock(p_node->p_lock);
		if(!is_wait)
		{
			char info[200];
			sprintf(info, "Locked by user %s", silc_mgmtd_if_server_get_conn_index_user(p_node->wlock_owner));
			silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_LOCKED, info);
			return -1;
		}
		while(1)
		{
			pthread_mutex_lock(p_node->p_lock);
			if(p_node->rlocked_cnt == 0)
			{
				break;
			}
			else
			{
				pthread_mutex_unlock(p_node->p_lock);
				usleep(10000);
			}
		}
	}
	p_node->wlock_owner = wlock_owner;
	return 0;
}

void silc_mgmtd_memdb_node_w_unlock(silc_mgmtd_node* p_node)
{
	p_node->wlock_owner = 0;
	pthread_mutex_unlock(p_node->p_lock);
}


silc_cstr silc_mgmtd_memdb_path_get_last_name(const silc_cstr path)
{
	int loop;
	int path_len = strlen(path);

	for(loop=path_len; loop>=0; loop--)
	{
		if(path[loop] == '/')
			return path+loop+1;
	}
	return path;
}

/*
 * not safe
 */
silc_cstr silc_mgmtd_memdb_get_node_path(silc_mgmtd_node* p_node, silc_cstr path)
{
	char tmp[SILC_MGMTD_IF_PATH_MAX_LEN];

	strcpy(path, "");
	while(p_node)
	{
		strcpy(tmp, path);
		if(p_node->type == MEMDB_NODE_TYPE_DYNAMIC)
			silc_mgmtd_var_to_str(&p_node->value, path, 200);
		else
			sprintf(path, "%s", p_node->name);
		if(strlen(tmp) != 0)
		{
			strcat(path, "/");
			strcat(path, tmp);
		}
		p_node = p_node->p_parent;
	}
	if(strlen(path) == 0)
		strcpy(path, "/");
	return path;
}

silc_mgmtd_node* silc_mgmtd_memdb_create_node(const silc_cstr node_name, silc_mgmtd_if_privilege level, silc_mgmtd_node_type node_type,
		silc_mgmtd_var_type val_type, const silc_cstr val_str, silc_mgmtd_node_callback do_ops, int timeout)
{
	silc_mgmtd_node* p_node;
	p_node = malloc(sizeof(silc_mgmtd_node));
	if(!p_node)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return NULL;
	}
	memset(p_node, 0, sizeof(silc_mgmtd_node));
	strcpy(p_node->name, node_name);
	p_node->type = node_type;
	p_node->if_level = level;
	p_node->do_ops = do_ops;
	p_node->do_ops_timeout = timeout;
	p_node->value.type = val_type;
	if(p_node->value.type != SILC_MGMTD_VAR_NULL && val_str &&
			strcmp(val_str, "None") != 0 && strcmp(val_str, "null") != 0)
	{
		if(silc_mgmtd_var_set_by_str(&p_node->value, val_str) < 0)
		{
			free(p_node);
			silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, val_str);
			return NULL;
		}
	}
	p_node->tmp_value.type = SILC_MGMTD_VAR_NULL;
	silc_list_init(&p_node->sub_node_list);
	silc_list_init(&p_node->ref_node_list);
	silc_list_init(&p_node->do_ops_err_list);
	return p_node;
}

void silc_mgmtd_memdb_insert_node(silc_mgmtd_node* p_node, silc_mgmtd_node* p_parent)
{
	p_node->p_parent = p_parent;
	silc_list_add_tail(&p_node->node, &p_parent->sub_node_list);
}

int silc_mgmtd_memdb_add_node(const silc_cstr node_path, silc_mgmtd_if_privilege level, silc_mgmtd_node_type node_type,
		silc_mgmtd_var_type val_type, const silc_cstr val_str, silc_mgmtd_node_callback do_ops, int timeout)
{
	silc_mgmtd_node *p_node, *p_parent;
	silc_cstr node_name;

	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(p_node)
		return 0;

	p_parent = silc_mgmtd_memdb_find_parent(node_path);
	if(!p_parent)
		return -1;

	node_name = silc_mgmtd_memdb_path_get_last_name(node_path);
	p_node = silc_mgmtd_memdb_create_node(node_name, level, node_type, val_type, val_str, do_ops, timeout);
	if(!p_node)
		return -1;
	// for init, set the tmp value to enable the configuration
	silc_mgmtd_var_copy(&p_node->value, &p_node->tmp_value);
	silc_mgmtd_memdb_insert_node(p_node, p_parent);
	return 0;
}

int silc_mgmtd_memdb_add_node_ref(const silc_cstr node_path, const silc_cstr path, silc_mgmtd_node_ref_callback do_ref)
{
	silc_mgmtd_node_ref* p_ref;
	silc_mgmtd_node *p_node, *p_parent;

	p_parent = silc_mgmtd_memdb_find_parent(node_path);
	if(!p_parent)
		return -1;

	if(silc_list_empty(&p_parent->sub_node_list))
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_PARENT_NO_CHILD, __func__);
		return -1;
	}
	p_node = silc_list_entry(p_parent->sub_node_list.next, silc_mgmtd_node, node);
	if(p_node->type != MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, __func__);
		return -1;
	}

	p_ref = malloc(sizeof(silc_mgmtd_node_ref) + strlen(path) + 1);
	if(!p_ref)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}

	p_ref->do_ref = do_ref;
	p_ref->path = (silc_cstr)(p_ref+1);
	strcpy(p_ref->path, path);
	silc_list_add_tail(&p_ref->node, &p_node->ref_node_list);
	return 0;
}


int silc_mgmtd_memdb_clone_node_inner(silc_mgmtd_node* p_src_node, silc_mgmtd_node* p_dst_parent)
{
	silc_mgmtd_node* p_node;
	silc_mgmtd_node* p_src_sub_node;
	char val_str[200];

	if(p_src_node->type != MEMDB_NODE_TYPE_TEMPLATE_SUB &&
			p_src_node->type != MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, __func__);
		return -1;
	}

	p_node = silc_mgmtd_memdb_create_node(p_src_node->name, p_src_node->if_level, p_src_node->type,
			p_src_node->value.type, silc_mgmtd_var_to_str(&p_src_node->value, val_str, 200),
			p_src_node->do_ops, p_src_node->do_ops_timeout);
	if(!p_node)
		return -1;

	if(p_dst_parent->type == MEMDB_NODE_TYPE_TEMPLATE)
		p_node->type = MEMDB_NODE_TYPE_TEMPLATE_SUB;
	else if(p_src_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB)
		p_node->type = MEMDB_NODE_TYPE_DYNAMIC_SUB;
	else
		p_node->type = MEMDB_NODE_TYPE_TEMPLATE;
	silc_mgmtd_memdb_insert_node(p_node, p_dst_parent);
//	printf("clone node:%s\n", p_node->name);
	silc_list_for_each_entry(p_src_sub_node, &p_src_node->sub_node_list, node)
	{
		if(silc_mgmtd_memdb_clone_node_inner(p_src_sub_node, p_node) < 0)
			return -1;
	}
	return 0;
}

silc_mgmtd_node* silc_mgmtd_memdb_clone_node(silc_mgmtd_node* p_src_node)
{
	silc_mgmtd_node* p_node;
	silc_mgmtd_node* p_src_sub_node;
	char val_str[200];

	if(p_src_node->type != MEMDB_NODE_TYPE_TEMPLATE)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NOT_DYNC_NODE, __func__);
		return NULL;
	}

	p_node = silc_mgmtd_memdb_create_node(p_src_node->name, p_src_node->if_level, p_src_node->type,
			p_src_node->value.type, silc_mgmtd_var_to_str(&p_src_node->value, val_str, 200),
			p_src_node->do_ops, p_src_node->do_ops_timeout);
	if(!p_node)
		return NULL;

	p_node->type = MEMDB_NODE_TYPE_DYNAMIC;
	p_node->p_parent = p_src_node->p_parent;
	silc_list_for_each_entry(p_src_sub_node, &p_src_node->sub_node_list, node)
	{
		if(silc_mgmtd_memdb_clone_node_inner(p_src_sub_node, p_node) < 0)
		{
			silc_mgmtd_memdb_delete_node(p_node);
			return NULL;
		}
	}
	return p_node;
}


void silc_mgmtd_memdb_delete_node(silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node, *p_tmp_node;

	silc_list_for_each_entry_safe(p_sub_node, p_tmp_node, &p_node->sub_node_list, node)
	{
		silc_mgmtd_memdb_delete_node(p_sub_node);
	}
	silc_mgmtd_var_clear(&p_node->value);
	if(!silc_list_empty(&p_node->do_ops_err_list))
	{
		silc_mgmtd_cb_err *p_err, *p_tmp_err;
		silc_list_for_each_entry_safe(p_err, p_tmp_err, &p_node->do_ops_err_list, node)
		{
			free(p_err);
		}
	}
	if(p_node->p_lock)
		free(p_node->p_lock);
	free(p_node);
}


silc_mgmtd_node* silc_mgmtd_memdb_find_node_comm(const silc_cstr node_path, silc_mgmtd_node* p_root)
{
	int name_len;
	char name[SILC_MGMTD_MEMDB_NAME_MAX_LEN];
	silc_mgmtd_node* p_node;
	silc_cstr sub_path, path;
	char val_str[200];

	if(strcmp(node_path, "/") == 0)
		return p_root;

	if(node_path[0] == '/')
		path = node_path + 1;
	else
		path = node_path;

	if(strlen(path) == 0)
		return p_root;

	sub_path = strstr(path, "/");
	if(!sub_path)
		sub_path = path + strlen(path);
	name_len = sub_path - path;
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_node, &p_root->sub_node_list, node)
	{
		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE ||
				p_node->type == MEMDB_NODE_TYPE_TEMPLATE_SUB)
			continue;

		if(p_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		{
			if(strcmp(name, silc_mgmtd_var_to_str(&p_node->value, val_str, 200)) == 0)
				return silc_mgmtd_memdb_find_node_comm(sub_path, p_node);
		}
		else
		{
			if(strcmp(name, p_node->name) == 0)
				return silc_mgmtd_memdb_find_node_comm(sub_path, p_node);
		}
	}
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_NOTEXIST, node_path);
	return NULL;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_node(const silc_cstr node_path)
{
	return silc_mgmtd_memdb_find_node_comm(node_path, &s_silc_mgmtd_memdb_root);
}

silc_mgmtd_node* silc_mgmtd_memdb_find_init_node_comm(const silc_cstr node_path, silc_mgmtd_node* p_root)
{
	int name_len;
	char name[SILC_MGMTD_MEMDB_NAME_MAX_LEN];
	silc_mgmtd_node* p_node;
	silc_cstr sub_path, path;

	if(strcmp(node_path, "/") == 0)
		return p_root;

	if(node_path[0] == '/')
		path = node_path + 1;
	else
		path = node_path;

	if(strlen(path) == 0)
		return p_root;

	sub_path = strstr(path, "/");
	if(!sub_path)
		sub_path = path + strlen(path);
	name_len = sub_path - path;
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_node, &p_root->sub_node_list, node)
	{
		if(p_node->type == MEMDB_NODE_TYPE_TEMPLATE)
		{
			if(strcmp(name, "*") == 0)
				return silc_mgmtd_memdb_find_init_node_comm(sub_path, p_node);
			continue;
		}
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
		{
			if(strcmp(name, p_node->name) == 0)
				return silc_mgmtd_memdb_find_init_node_comm(sub_path, p_node);
		}
	}
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_NOTEXIST, node_path);
	return NULL;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_init_node(const silc_cstr node_path)
{
	return silc_mgmtd_memdb_find_init_node_comm(node_path, &s_silc_mgmtd_memdb_root);
}

silc_mgmtd_node* silc_mgmtd_memdb_find_node_top_inner(const silc_cstr node_path, silc_mgmtd_node* p_root)
{
	int name_len;
	char name[SILC_MGMTD_MEMDB_NAME_MAX_LEN];
	silc_mgmtd_node* p_node;
	silc_cstr sub_path, path;
//	char val_str[200];

	if(strcmp(node_path, "/") == 0)
		return p_root;

	if(node_path[0] == '/')
		path = node_path + 1;
	else
		path = node_path;

	if(strlen(path) == 0)
		return p_root;

	sub_path = strstr(path, "/");
	if(!sub_path)
		sub_path = path + strlen(path);
	name_len = sub_path - path;
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_node, &p_root->sub_node_list, node)
	{
		if(strcmp(name, p_node->name) == 0)
		{
			if(p_node->do_ops)
				return p_node;
			return silc_mgmtd_memdb_find_node_top_inner(sub_path, p_node);
		}
	}
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_NOTEXIST, node_path);
	return NULL;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_node_top(const silc_cstr node_path)
{
	return silc_mgmtd_memdb_find_node_top_inner(node_path, &s_silc_mgmtd_memdb_root);
}


silc_mgmtd_node* silc_mgmtd_memdb_find_parent_inner(const silc_cstr node_path, silc_mgmtd_node* p_root)
{
	int name_len;
	char name[SILC_MGMTD_MEMDB_NAME_MAX_LEN];
	silc_mgmtd_node* p_node;
	silc_cstr sub_path, path;
	char val_str[200];

	if(node_path[0] == '/')
		path = node_path + 1;
	else
		path = node_path;

	if(strlen(path) == 0)
		return p_root;

	sub_path = strstr(path, "/");
	if(!sub_path)
		return p_root;
	name_len = sub_path - path;
	strncpy(name, path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_node, &p_root->sub_node_list, node)
	{
		if(p_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		{
			if(strcmp(name, silc_mgmtd_var_to_str(&p_node->value, val_str, 200)) == 0)
				return silc_mgmtd_memdb_find_parent_inner(sub_path, p_node);
		}
		else
		{
			if(strcmp(name, p_node->name) == 0)
				return silc_mgmtd_memdb_find_parent_inner(sub_path, p_node);
		}
	}
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_PARENT_NOTEXIST, node_path);
	return NULL;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_parent(const silc_cstr node_path)
{
	return silc_mgmtd_memdb_find_parent_inner(node_path, &s_silc_mgmtd_memdb_root);
}


silc_mgmtd_node* silc_mgmtd_memdb_find_sub_node(silc_mgmtd_node* p_node, const silc_cstr name)
{
	char info[200];
	silc_mgmtd_node* p_sub_node;
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, name) == 0)
			return p_sub_node;
	}
	silc_mgmtd_memdb_get_node_path(p_node, info);
	sprintf(info+strlen(info), "/%s", name);
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_NOTEXIST, info);
	return NULL;
}

silc_mgmtd_node* silc_mgmtd_memdb_find_sub_value(silc_mgmtd_node* p_node, const silc_cstr val_str)
{
	char info[200];
	silc_mgmtd_node* p_sub_node;
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		char val[200];
		if(p_sub_node->type == MEMDB_NODE_TYPE_TEMPLATE)
			continue;
		silc_mgmtd_var_to_str(&p_sub_node->value, val, 200);
		if(strcmp(val, val_str) == 0)
			return p_sub_node;
	}
	silc_mgmtd_memdb_get_node_path(p_node, info);
	sprintf(info+strlen(info), " has not sub value %s", val_str);
	silc_mgmtd_err_set(MGMTD_ERR_MEMDB_NODE_NOTEXIST, info);
	return NULL;
}

int silc_mgmtd_memdb_modify_node(silc_mgmtd_node* p_node, const silc_cstr new_val_str)
{
	if(p_node->value.type == SILC_MGMTD_VAR_NULL)
		return 0;
	//if(strlen(new_val_str) == 0)
		//return 0;

	if(p_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		return 0;

	silc_mgmtd_var_clear(&p_node->tmp_value);
	p_node->tmp_value.type = p_node->value.type;

	if(silc_mgmtd_var_set_by_str(&p_node->tmp_value, new_val_str) < 0)
	{
		p_node->tmp_value.type = SILC_MGMTD_VAR_NULL;
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
		return -1;
	}
	return 0;
}

int silc_mgmtd_memdb_modify_node_confirm(silc_mgmtd_node* p_node)
{
	if(p_node->tmp_value.type == SILC_MGMTD_VAR_NULL)
		return 0;
	silc_mgmtd_var_clear(&p_node->value);
	if(silc_mgmtd_var_copy(&p_node->tmp_value, &p_node->value) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MGMTD_LIB, __func__);
		return -1;
	}
	return 0;
}

int silc_mgmtd_memdb_add_cb_err(const silc_cstr node_path, int err_num, const silc_cstr err_str)
{
	silc_mgmtd_cb_err* p_err;
	silc_mgmtd_node *p_node;

	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_node)
		return -1;

	p_err = malloc(sizeof(silc_mgmtd_cb_err) + strlen(err_str) + 1);
	if(!p_err)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, __func__);
		return -1;
	}

	p_err->err_num = err_num;
	p_err->err_str = (silc_cstr)(p_err+1);
	strcpy(p_err->err_str, err_str);
	silc_list_add_tail(&p_err->node, &p_node->do_ops_err_list);
	return 0;
}


silc_cstr silc_mgmtd_memdb_get_cb_err(silc_mgmtd_node* p_node, int err_num)
{
	silc_mgmtd_cb_err* p_err;

	if(!p_node)
		return NULL;

	silc_list_for_each_entry(p_err, &p_node->do_ops_err_list, node)
	{
		if(p_err->err_num == err_num)
			return p_err->err_str;
	}

	return silc_mgmtd_memdb_get_cb_err(p_node->p_parent, err_num);
}

int silc_mgmtd_memdb_dnode_tree_traversal_inner(const silc_cstr parent_path, silc_mgmtd_node* p_db_node,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_memdb_dnode_traversal_cb cb_func, void* data)
{
	int ret = 0;
	char path[SILC_MGMTD_IF_PATH_MAX_LEN];
	silc_mgmtd_if_node* p_if_sub_node;
	silc_mgmtd_node* p_db_sub_node;

	if(!p_db_node)
		return 0;

	if(strlen(p_if_node->name) == 0) // node root
		strcpy(path, "");
	else if(strlen(parent_path))
		sprintf(path, "%s/%s", parent_path, p_db_node->name);
	else
		strcpy(path, p_db_node->name);

	if(cb_func)
		ret = cb_func(path, p_if_node, p_db_node, data);

	if(ret != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_TRAVERSAL_ERR, path);
		return ret;
	}

	silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
	{
		if(p_if_sub_node->add_dync)
			continue;
		silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
		{
			if(p_db_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
			{
				char val_str[100];
				silc_mgmtd_var_to_str(&p_db_sub_node->value, val_str, 100);
				if(strcmp(val_str, p_if_sub_node->name) != 0)
					continue;
			}
			if(strcmp(p_db_sub_node->name, p_if_sub_node->name) != 0)
				continue;

			ret = silc_mgmtd_memdb_dnode_tree_traversal_inner(path, p_db_sub_node, p_if_sub_node, cb_func, data);
			if(ret != 0)
				return ret;
		}
	}
	return 0;
}


int silc_mgmtd_memdb_dnode_tree_traversal(silc_mgmtd_node* p_db_node,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_memdb_dnode_traversal_cb cb_func, void* cb_data)
{
	return silc_mgmtd_memdb_dnode_tree_traversal_inner("", p_db_node, p_if_node, cb_func, cb_data);
}

int silc_mgmtd_memdb_node_tree_traversal(const silc_cstr parent_path,
		silc_mgmtd_node* p_db_node, silc_mgmtd_memdb_node_traversal_cb cb_func, void* cb_data)
{
	int ret = 0;
	silc_mgmtd_node* p_db_sub_node;
	char path[SILC_MGMTD_IF_PATH_MAX_LEN];
	char val_str[200];

	if(strlen(parent_path) == 0)
		strcpy(path, p_db_node->name);
	else if(strcmp(parent_path, "/") == 0)
		sprintf(path, "/%s", p_db_node->name);
	else if(p_db_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		sprintf(path, "%s/%s", parent_path, silc_mgmtd_var_to_str(&p_db_node->value, val_str, 200));
	else
		sprintf(path, "%s/%s", parent_path, p_db_node->name);

	if(cb_func)
		ret = cb_func(path, p_db_node, cb_data);

	if(ret != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_TRAVERSAL_ERR, path);
		return ret;
	}

	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
		ret = silc_mgmtd_memdb_node_tree_traversal(path, p_db_sub_node, cb_func, cb_data);
	}
	return ret;
}

int silc_mgmtd_memdb_if_node_tree_traversal(const silc_cstr parent_path,
		silc_mgmtd_if_node* p_if_node, silc_mgmtd_memdb_if_node_traversal_cb cb_func, void* cb_data)
{
	int ret = 0;
	silc_mgmtd_if_node* p_if_sub_node;
	char path[SILC_MGMTD_IF_PATH_MAX_LEN];

	if(strlen(parent_path) == 0)
		strcpy(path, p_if_node->name);
	else if(strcmp(parent_path, "/") == 0)
		sprintf(path, "/%s", p_if_node->name);
	sprintf(path, "%s/%s", parent_path, p_if_node->name);

	if(cb_func)
		ret = cb_func(path, p_if_node, cb_data);

	if(ret != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_MEMDB_TRAVERSAL_ERR, path);
		return ret;
	}

	silc_list_for_each_entry(p_if_sub_node, &p_if_node->sub_node_list, node)
	{
		ret = silc_mgmtd_memdb_if_node_tree_traversal(path, p_if_sub_node, cb_func, cb_data);
	}
	return ret;
}


silc_cstr silc_mgmtd_memdb_get_print_val_str(silc_mgmtd_var* p_var)
{
	static char val_buf[SILC_MGMTD_VAR_VAL_STR_MAX_LEN];
	silc_cstr val_str = silc_mgmtd_var_to_str(p_var, val_buf, SILC_MGMTD_VAR_VAL_STR_MAX_LEN);

	if(!val_str)
		return "unknown";
	return val_str;
}

int silc_mgmtd_memdb_node_show_cb(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	char* type_strs[] = {
							"NULL",
							"STATIC",
							"TEMPLATE",
							"TEMPLATE_SUB",
							"DYNAMIC",
							"DYNAMIC_SUB",
							"MAX"};
	char* val_type_strs[] = {
								"NULL",
								"UINT32",
								"INT32",
								"UINT64",
								"INT64",
								"FLOAT",
								"DOUBLE",
								"BOOL",
								"STRING",
								"MACADDR",
								"IPV4ADDR",
								"DATETIME",
								"DATE",
								"TIME",
								"HEX",
								"MAX"
	};

	printf("%s, %s, %s, %s", path, type_strs[p_db_node->type], val_type_strs[p_db_node->value.type], val_type_strs[p_db_node->tmp_value.type]);
	if(strstr(path, "config") != 0)
	{
		printf(", %s", silc_mgmtd_memdb_get_print_val_str(&p_db_node->value));
		printf(", %s\n", silc_mgmtd_memdb_get_print_val_str(&p_db_node->tmp_value));
	}
	else
		printf("\n");
	return 0;
}

void silc_mgmtd_memdb_node_show(silc_mgmtd_node* p_db_node)
{
	silc_mgmtd_memdb_node_tree_traversal("", p_db_node, silc_mgmtd_memdb_node_show_cb, NULL);
}

int silc_mgmtd_memdb_if_node_show_cb(const silc_cstr path, silc_mgmtd_if_node* p_if_node, void* cb_data)
{
	printf("%s, %s\n", path, p_if_node->val_str);
	return 0;
}
void silc_mgmtd_memdb_if_node_show(silc_mgmtd_if_node* p_if_node)
{
	silc_mgmtd_memdb_if_node_tree_traversal("", p_if_node, silc_mgmtd_memdb_if_node_show_cb, NULL);
}

void silc_mgmtd_memdb_show_all()
{
	silc_mgmtd_memdb_node_tree_traversal("/", &s_silc_mgmtd_memdb_root, silc_mgmtd_memdb_node_show_cb, NULL);
}

int silc_mgmtd_memdb_node_confirm_clean_cb(const silc_cstr path, silc_mgmtd_node* p_db_node, void* cb_data)
{
	int ret = 0;
	ret = silc_mgmtd_memdb_modify_node_confirm(p_db_node);
	if(ret != 0)
		return ret;
	silc_mgmtd_var_clear(&p_db_node->tmp_value);
	return 0;
}

int silc_mgmtd_memdb_modify_node_confirm_tree(silc_mgmtd_node* p_node)
{
	return silc_mgmtd_memdb_node_tree_traversal("", p_node, silc_mgmtd_memdb_node_confirm_clean_cb, NULL);
}


