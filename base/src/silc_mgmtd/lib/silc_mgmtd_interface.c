
#include "silc_common.h"
#include "silc_common/silc_thread.h"

#include "silc_mgmtd_variables.h"
#include "silc_mgmtd_connect.h"
#include "silc_mgmtd_interface.h"

/*
 * --------------------------------- NODE ---------------------------------
 */

void silc_mgmtd_if_trim_trail_nl(char* str)
{
	int l = strlen(str);
	if (l > 0 && (str[l-1] == '\n' || str[l-1] == '\r'))
		str[l-1] = 0;
}

void silc_mgmtd_if_splitter_convert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);

	for(i=0; i<len; i++)
	{
		if(cstr[i] == ',')
			cstr[i] = 28;
		else if(cstr[i] == ';')
			cstr[i] = 29;
	}
}

void silc_mgmtd_if_splitter_revert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);

	for(i=0; i<len; i++)
	{
		if(cstr[i] == 28)
			cstr[i] = ',';
		else if(cstr[i] == 29)
			cstr[i] = ';';
	}
}

void silc_mgmtd_if_base64_escape(silc_cstr str)
{
	char* p = str;
	int i, len = strlen(str);

	for (i=0; i<len; i++)
	{
		if(*p == '\n')
			*p = '#';
		p++;
	}
}

void silc_mgmtd_if_base64_unescape(silc_cstr str)
{
	char* p = str;
	int i, len = strlen(str);

	for (i=0; i<len; i++)
	{
		if(*p == '#')
			*p = '\n';
		p++;
	}
}

int silc_mgmtd_if_str_escape_len(char* str)
{
	int len = 0;
	char*r_pos = str;

	if(!str)
		return len;

	while(*r_pos)
	{
		if(*r_pos == '\n' || *r_pos == '\r' || *r_pos == '\\')
		{
			len++;
		}
		len++;
		r_pos++;
	}
	len++;
	return len;
}

void silc_mgmtd_if_str_escape(char* str, char* newstr)
{
	char *r_pos=str, *w_pos=newstr;

	while(*r_pos)
	{
		if(*r_pos == '\n')
		{
			*w_pos = '\\';
			w_pos++;
			*w_pos = 'n';
		}
		else if(*r_pos == '\r')
		{
			*w_pos = '\\';
			w_pos++;
			*w_pos = 'r';
		}
		else if(*r_pos == '\\')
		{
			*w_pos = '\\';
			w_pos++;
			*w_pos = '\\';
		}
		else if(*r_pos == ',')
		{
			*w_pos = 28;
		}
		else if(*r_pos == ';')
		{
			*w_pos = 29;
		}
		else
		{
			*w_pos = *r_pos;
		}
		r_pos++;
		w_pos++;
	}
	*w_pos = 0;
}

void silc_mgmtd_if_str_unescape(char* str, char* newstr)
{
	char *r_pos=str, *w_pos=newstr;

	while(*r_pos)
	{
		if(*r_pos == '\\' && *(r_pos+1) == 'n')
		{
			*w_pos = '\n';
			r_pos++;
		}
		else if(*r_pos == '\\' && *(r_pos+1) == 'r')
		{
			*w_pos = '\r';
			r_pos++;
		}
		else if(*r_pos == '\\' && *(r_pos+1) == '\\')
		{
			*w_pos = '\\';
			r_pos++;
		}
		else if(*r_pos == 28)
		{
			*w_pos = ',';
		}
		else if(*r_pos == 29)
		{
			*w_pos = ';';
		}
		else
		{
			*w_pos = *r_pos;
		}
		r_pos++;
		w_pos++;
	}
	*w_pos = 0;
}

void silc_mgmtd_if_server_node_delete(silc_mgmtd_if_node* p_node)
{
	silc_mgmtd_if_node *p_sub_node, *p_tmp_node;

	silc_list_for_each_entry_safe(p_sub_node, p_tmp_node, &p_node->sub_node_list, node)
	{
		silc_mgmtd_if_server_node_delete(p_sub_node);
	}
	if(p_node->val_str_dync)
		free(p_node->val_str);
	free(p_node);
}


silc_mgmtd_if_node* silc_mgmtd_if_node_create(const silc_cstr node_name, const silc_cstr val_str)
{
	silc_mgmtd_if_node* p_node;
	silc_cstr val_str_cur = val_str;

	if(!val_str_cur)
		val_str_cur = node_name;
	p_node = malloc(sizeof(silc_mgmtd_if_node) +
				strlen(node_name) + strlen(val_str_cur) + 2);
	if(!p_node)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return NULL;
	}
	p_node->add_dync = silc_false;
	p_node->val_str_dync = silc_false;
	p_node->name = (silc_cstr)(p_node+1);
	p_node->val_str = p_node->name + strlen(node_name) + 1;
	strcpy(p_node->name, node_name);
	strcpy(p_node->val_str, val_str);
	silc_mgmtd_if_splitter_revert(p_node->val_str);
//	SILC_DEBUG("[%s] p_node:%p, name:%s, val:%s\n", __func__, p_node, p_node->name, p_node->val_str);
	silc_list_init(&p_node->sub_node_list);
	return p_node;
}

silc_mgmtd_if_node* silc_mgmtd_if_server_node_add(const silc_cstr node_name, const silc_cstr val_str, silc_mgmtd_if_node* p_parent_node)
{
	silc_mgmtd_if_node* p_node = silc_mgmtd_if_node_create(node_name, val_str);
	if(!p_node)
		return NULL;
	p_node->add_dync = silc_true;
	silc_list_add_tail(&p_node->node, &p_parent_node->sub_node_list);
	return p_node;
}

int silc_mgmtd_if_server_node_set_val(silc_mgmtd_if_node* p_node, const silc_cstr new_val_str)
{
	silc_cstr new_str = "null";

	if(new_val_str)
		new_str = new_val_str;

	if(p_node->val_str_dync)
		free(p_node->val_str);

	p_node->val_str = malloc(strlen(new_str) + 1);
	if(!p_node->val_str)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}
	p_node->val_str_dync = silc_true;
	strcpy(p_node->val_str, new_str);
	return 0;
}


silc_cstr silc_mgmtd_if_path_get_last_name(const silc_cstr path)
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

silc_mgmtd_if_node* silc_mgmtd_if_node_find_parent(const silc_cstr node_path, silc_mgmtd_if_node* p_root)
{
	int name_len;
	char name[SILC_MGMTD_IF_NAME_MAX_LEN];
	silc_mgmtd_if_node* p_node;
	silc_cstr sub_path;

	sub_path = strstr(node_path, "/");
	if(!sub_path || strlen(node_path) == 0)
		return p_root;
	name_len = sub_path - node_path;
	strncpy(name, node_path, name_len);
	name[name_len] = '\0';

	silc_list_for_each_entry(p_node, &p_root->sub_node_list, node)
	{
		if(strcmp(name, p_node->name) == 0)
			return silc_mgmtd_if_node_find_parent(sub_path+1, p_node);
	}
	silc_mgmtd_lib_err_set(LIB_ERR_IF_NODE_NOTEXIST);
	return NULL;
}


/*
 *
 */
int silc_mgmtd_if_node_tree_traversal(const silc_cstr parent_path, silc_mgmtd_if_node* p_node,
		silc_mgmtd_if_node_traversal_cb cb_func, void* cb_data)
{
	int ret = 0;
	char path[SILC_MGMTD_IF_PATH_MAX_LEN] = "";
	silc_mgmtd_if_node* p_sub_node;

	if(cb_func)
		ret = cb_func(parent_path, p_node, cb_data);

	if(ret < 0)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_NODE_TRAVERSAL_CB_FAILED);
		return ret;
	}

	if(strlen(parent_path))
		sprintf(path, "%s", parent_path);
	if(strlen(p_node->name))
	{
		if(strlen(path))
			strcat(path, "/");
		strcat(path, p_node->name);
	}

	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		ret = silc_mgmtd_if_node_tree_traversal(path, p_sub_node, cb_func, cb_data);
	}
	return ret;
}

/*
 * -------------------------------- MESSAGE STRUCTURE ----------------------------
 *
 */
silc_mgmtd_if_item* silc_mgmtd_if_item_create(const silc_cstr path, silc_mgmtd_if_req_type type,
		const silc_cstr ret_str, const silc_cstr node_root_val_str)
{
	silc_mgmtd_if_item* p_item;
	int len = sizeof(silc_mgmtd_if_item) + strlen(path) + strlen(ret_str) + 2;

	p_item = malloc(len);
	if(!p_item)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return NULL;
	}
	p_item->type = type;
	p_item->path_str = (silc_cstr)(p_item+1);
	strcpy(p_item->path_str, path);
	p_item->return_str = p_item->path_str + strlen(path) + 1;
	strcpy(p_item->return_str, ret_str);
	silc_mgmtd_if_splitter_revert(p_item->path_str);
	silc_mgmtd_if_splitter_revert(p_item->return_str);
	p_item->p_node_root = silc_mgmtd_if_node_create(silc_mgmtd_if_path_get_last_name(path), node_root_val_str);
	if(!p_item->p_node_root)
	{
		free(p_item);
		return NULL;
	}

	return p_item;
}

void silc_mgmtd_if_item_delete(silc_mgmtd_if_item* p_item)
{
	silc_mgmtd_if_server_node_delete(p_item->p_node_root);
	free(p_item);
}

void silc_mgmtd_if_item_list_delete(silc_list* p_item_list)
{

}

int silc_mgmtd_if_item_add_node(silc_mgmtd_if_item* p_item, const silc_cstr node_path, const silc_cstr node_val_str)
{
	silc_mgmtd_if_node *p_node, *p_parent;
	silc_cstr node_name;

	p_parent = silc_mgmtd_if_node_find_parent(node_path, p_item->p_node_root);
	if(!p_parent)
		return -1;

	node_name = silc_mgmtd_if_path_get_last_name(node_path);
	p_node = silc_mgmtd_if_node_create(node_name, node_val_str);
	if(!p_node)
		return -1;
	silc_list_add_tail(&p_node->node, &p_parent->sub_node_list);
	return 0;
}

silc_mgmtd_if_node* silc_mgmtd_if_item_add_node_ext(silc_mgmtd_if_node* p_parent,
		const silc_cstr name, const silc_cstr node_val_str)
{
	silc_mgmtd_if_node *p_node;

	p_node = silc_mgmtd_if_node_create(name, node_val_str);
	if(!p_node)
		return NULL;
	silc_list_add_tail(&p_node->node, &p_parent->sub_node_list);
	return p_node;
}

int silc_mgmtd_if_item_node_traversal(silc_mgmtd_if_item* p_item, silc_mgmtd_if_node_traversal_cb cb_func, void* cb_data)
{
	return silc_mgmtd_if_node_tree_traversal(p_item->path_str, p_item->p_node_root, cb_func, cb_data);
}

int silc_mgmtd_if_item_node_sub_traversal(silc_mgmtd_if_item* p_item, silc_mgmtd_if_node_traversal_cb cb_func, void* cb_data)
{
	silc_mgmtd_if_node* p_node;

	silc_list_for_each_entry(p_node, &p_item->p_node_root->sub_node_list, node)
	{
		if(silc_mgmtd_if_node_tree_traversal("", p_node, cb_func, cb_data) < 0)
			return -1;
	}
	return 0;
}

int silc_mgmtd_if_item_clone_traversal_cb(const silc_cstr parent_path, silc_mgmtd_if_node* p_node, void* data)
{
	silc_mgmtd_if_item* p_item = (silc_mgmtd_if_item*)data;
	return silc_mgmtd_if_item_add_node(p_item, parent_path, p_node->val_str);
}

silc_mgmtd_if_item* silc_mgmtd_if_item_clone(const silc_cstr new_path, silc_mgmtd_if_item* p_item)
{
	silc_mgmtd_if_item* p_clone_item = silc_mgmtd_if_item_create(new_path, p_item->type,
			p_item->return_str, p_item->p_node_root->val_str);

	if(!p_clone_item)
		return NULL;

	p_clone_item->conn_entry = p_item->conn_entry;

	if(silc_mgmtd_if_item_node_sub_traversal(p_item, silc_mgmtd_if_item_clone_traversal_cb, p_clone_item) != 0)
	{
		silc_mgmtd_if_item_delete(p_clone_item);
		return NULL;
	}

	return p_clone_item;
}
/*
 * -------------------------------- MESSAGE FORMAT -------------------------------
 *
 * <message start label>,<sequence>;
 * <item start label>,<request type>,<return string>,<root node path>,<root node value string>;<node path>,<node value string>;...;<item end label>;
 * <message end label>;
 */
#define SILC_MGMTD_IF_MSG_START_LABEL			"MSG_START,"
#define SILC_MGMTD_IF_MSG_END_LABEL				"MSG_END;"
#define SILC_MGMTD_IF_MSG_ITEM_START_LABEL 		"ITEM_START,"
#define SILC_MGMTD_IF_MSG_ITEM_END_LABEL 		"ITEM_END;"
#define SILC_MGMTD_IF_MSG_HEAD_ELEM_NUM 		1
#define SILC_MGMTD_IF_MSG_ITEM_HEAD_ELEM_NUM	4
#define SILC_MGMTD_IF_MSG_ITEM_NODE_ELEM_NUM	2

// do not change the splitter
#define SILC_MGMTD_IF_MSG_SPLITTER 			";"
#define SILC_MGMTD_IF_MSG_SUB_SPLiTTER 		","

static silc_cstr s_silc_mgmtd_if_req_type_str[SILC_MGMTD_IF_REQ_MAX] = {
		"NULL",
		"ADD",
		"MODIFY",
		"DELETE",
		"QUERY",
		"QUERY_CHILD",
		"QUERY_SUB",
		"QUERY_WILD",
		"ACTION",
		"NOTIFY",
		"CHECK_ADD",
		"CHECK_MODIFY",
		"CHECK_DELETE",
		"UPGRADE"
};

silc_cstr silc_mgmtd_if_get_req_type_str(silc_mgmtd_if_req_type type)
{
	return s_silc_mgmtd_if_req_type_str[type];
}

silc_mgmtd_if_req_type silc_mgmtd_if_get_req_type_by_str(const silc_cstr type_str)
{
	int loop;

	for(loop=0; loop<SILC_MGMTD_IF_REQ_MAX; loop++)
	{
		if(strcmp(type_str, s_silc_mgmtd_if_req_type_str[loop]) == 0)
			return loop;
	}
	silc_mgmtd_lib_err_set(LIB_ERR_IF_REQ_TYPE_UNKNOWN);
	return SILC_MGMTD_IF_REQ_MAX;
}

/*
 * ---- CREATE MESSAGE STRING ----
 */
#define SILC_MGMTD_IF_CHECK_LEN(buff, inc_len)		\
	do{\
		if((strlen(buff)+inc_len) >= SILC_MGMTD_IF_BUFFER_MAX_LEN) \
		{ \
			silc_mgmtd_lib_err_set(LIB_ERR_IF_REQ_BUFF_NOTENOUGH); \
			return -1; \
		} \
	}while(0)


void silc_mgmtd_if_msg_insert_head(silc_cstr buff, uint16_t seq_num)
{
	buff[0] = '\0';
	sprintf(buff, "%s%d;", SILC_MGMTD_IF_MSG_START_LABEL, seq_num);
}

int silc_mgmtd_if_msg_insert_item_head(silc_cstr buff, silc_mgmtd_if_req_type req_type,
		const silc_cstr ret_str, const silc_cstr root_path, const silc_cstr root_val)
{
	silc_cstr type_str = silc_mgmtd_if_get_req_type_str(req_type);
	int len = strlen(SILC_MGMTD_IF_MSG_ITEM_START_LABEL) + strlen(type_str) +
			strlen(ret_str) + strlen(root_path) + strlen(root_val) + 4;

	SILC_MGMTD_IF_CHECK_LEN(buff, len);
	sprintf(buff+strlen(buff), "%s%s,", SILC_MGMTD_IF_MSG_ITEM_START_LABEL, type_str);

	// ret string
	len = strlen(buff);
	sprintf(buff+len, "%s", ret_str);
	silc_mgmtd_if_splitter_convert(buff+len);
	strcat(buff, ",");

	// path
	len = strlen(buff);
	sprintf(buff+len, "%s", root_path);
	silc_mgmtd_if_splitter_convert(buff+len);
	strcat(buff, ",");

	// root value string
	len = strlen(buff);
	sprintf(buff+len, "%s", root_val);
	silc_mgmtd_if_splitter_convert(buff+len);
	strcat(buff, ";");

	return 0;
}

int silc_mgmtd_if_msg_insert_node(silc_cstr buff, const silc_cstr parent_path, const silc_cstr node_name, const silc_cstr node_val_str)
{
	int len = strlen(parent_path) + strlen(node_name) + strlen(node_val_str) + 3;

	SILC_MGMTD_IF_CHECK_LEN(buff, len);
	if(strlen(parent_path) == 0)
		sprintf(buff+strlen(buff), "%s,", node_name);
	else
		sprintf(buff+strlen(buff), "%s/%s,", parent_path, node_name);
	len = strlen(buff);
	sprintf(buff+len, "%s", node_val_str);
	silc_mgmtd_if_splitter_convert(buff+len);
	strcat(buff, ";");
	return 0;
}

int silc_mgmtd_if_msg_insert_item_tail(silc_cstr buff)
{
	int len = strlen(SILC_MGMTD_IF_MSG_ITEM_END_LABEL) + 1;

	SILC_MGMTD_IF_CHECK_LEN(buff, len);
	sprintf(buff+strlen(buff), "%s", SILC_MGMTD_IF_MSG_ITEM_END_LABEL);
	return 0;
}

int silc_mgmtd_if_msg_insert_tail(silc_cstr buff)
{
	int len = strlen(SILC_MGMTD_IF_MSG_END_LABEL) + 1;

	SILC_MGMTD_IF_CHECK_LEN(buff, len);
	sprintf(buff+strlen(buff), "%s", SILC_MGMTD_IF_MSG_END_LABEL);
	return 0;
}

/*
 * ---- PARSER MESSAGE STRING ----
 * input buffer will be kept till the interface message was been handled.
 */
int silc_mgmtd_if_msg_parser_elem(silc_cstr buff, int* p_parser_len, silc_cstr* elem_array, int elem_num)
{
	int cur_pos = 0;
	int cur_elem_num = 0;
	silc_cstr cur_str = buff;
	silc_cstr elem_start = buff;

	while(1)
	{
		if(cur_str[cur_pos] == ',')
		{
			cur_str[cur_pos] = '\0';
			elem_array[cur_elem_num] = elem_start;
			elem_start = cur_str + cur_pos + 1;
			cur_elem_num ++;
		}
		else if(cur_str[cur_pos] == ';')
		{
			cur_str[cur_pos] = '\0';
			elem_array[cur_elem_num] = elem_start;
			cur_elem_num ++;
			break;
		}
		cur_pos++;
	}

	*p_parser_len = cur_pos + 1;
	if(cur_elem_num != elem_num)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_STR_INVALID);
		return -1;
	}
	return 0;
}

int silc_mgmtd_if_cstr2l(const silc_cstr val_str, long *p_val)
{
	char *endptr = NULL;

	*p_val = strtol(val_str, &endptr, 0);
	if(*endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_SEQ_STR_INVALID);
		return -1;
	}
	return 0;
}

int silc_mgmtd_if_cstr2ll(const silc_cstr val_str, long long *p_val)
{
	char *endptr = NULL;

	*p_val = strtoll(val_str, &endptr, 0);
	if(*endptr != '\0')
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_SEQ_STR_INVALID);
		return -1;
	}
	return 0;
}

int silc_mgmtd_if_cstr2ipv4addr(const silc_cstr val_str, in_addr_t *p_val)
{
	int result;

	if(strcmp(val_str, "") == 0)
	{
		*p_val = 0;
		return 0;
	}

	result = inet_pton(AF_INET, val_str, p_val);
	if(result != 1)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_SEQ_STR_INVALID);
		return -1;
	}
	return 0;
}

int silc_mgmtd_if_msg_get_head(silc_cstr buff, int* p_start_pos, uint32_t* p_seq_num)
{
	int parser_len = 0;
	int head_len = strlen(SILC_MGMTD_IF_MSG_START_LABEL);
	silc_cstr elem_array[SILC_MGMTD_IF_MSG_HEAD_ELEM_NUM];
	silc_cstr cur_start = buff + *p_start_pos;
	long seq_num;

	// check head label
	if(strncmp(cur_start, SILC_MGMTD_IF_MSG_START_LABEL, head_len) != 0)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_HEAD_NOTEXIST);
		return -1;
	}

	*p_start_pos += head_len;
	if(silc_mgmtd_if_msg_parser_elem(cur_start+head_len, &parser_len, elem_array, SILC_MGMTD_IF_MSG_HEAD_ELEM_NUM) < 0)
	{
		*p_start_pos += parser_len;
		return -1;
	}
	*p_start_pos += parser_len;
	if(silc_mgmtd_if_cstr2l(elem_array[0], &seq_num) != 0)
		return -1;
	*p_seq_num = seq_num;
	return 0;
}

//<item start label>,<request type>,<return string>,<root node path>,<root node value string>;
silc_mgmtd_if_item* silc_mgmtd_if_msg_get_item_head(silc_cstr buff, int* p_start_pos)
{
	int parser_len = 0;
	int head_len = strlen(SILC_MGMTD_IF_MSG_ITEM_START_LABEL);
	silc_cstr elem_array[SILC_MGMTD_IF_MSG_ITEM_HEAD_ELEM_NUM];
	silc_cstr cur_start = buff + *p_start_pos;
	silc_mgmtd_if_item* p_item;

	// check item head label
	if(strncmp(cur_start, SILC_MGMTD_IF_MSG_ITEM_START_LABEL, head_len) != 0)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_ITEM_HEAD_NOTEXIST);
		return NULL;
	}

	*p_start_pos += head_len;
	if(silc_mgmtd_if_msg_parser_elem(cur_start+head_len, &parser_len, elem_array, SILC_MGMTD_IF_MSG_ITEM_HEAD_ELEM_NUM) < 0)
	{
		*p_start_pos += parser_len;
		return NULL;
	}
	*p_start_pos += parser_len;
	p_item = silc_mgmtd_if_item_create(elem_array[2],
			silc_mgmtd_if_get_req_type_by_str(elem_array[0]), elem_array[1], elem_array[3]);
	if(!p_item)
		return NULL;

	return p_item;
}

silc_bool silc_mgmtd_if_msg_check_item_end(silc_cstr buff, int* p_start_pos)
{
	int check_len = strlen(SILC_MGMTD_IF_MSG_ITEM_END_LABEL);
	silc_cstr cur_start = buff + *p_start_pos;

	// check item end label
	if(strncmp(cur_start, SILC_MGMTD_IF_MSG_ITEM_END_LABEL, check_len) != 0)
		return silc_false;
	*p_start_pos += check_len;
	return silc_true;
}

silc_bool silc_mgmtd_if_msg_check_end(silc_cstr buff, int* p_start_pos)
{
	int check_len = strlen(SILC_MGMTD_IF_MSG_END_LABEL);
	silc_cstr cur_start = buff + *p_start_pos;

	// check message end label
	if(strncmp(cur_start, SILC_MGMTD_IF_MSG_END_LABEL, check_len) != 0)
		return silc_false;
	*p_start_pos += check_len;
	return silc_true;
}

int silc_mgmtd_if_msg_get_item_node(silc_cstr buff, int* p_start_pos, silc_mgmtd_if_item* p_item)
{
	int parser_len = 0;
	silc_cstr elem_array[SILC_MGMTD_IF_MSG_ITEM_NODE_ELEM_NUM];
	silc_cstr cur_start = buff + *p_start_pos;
	silc_cstr node_path;

	if(silc_mgmtd_if_msg_parser_elem(cur_start, &parser_len, elem_array, SILC_MGMTD_IF_MSG_ITEM_NODE_ELEM_NUM) < 0)
	{
		*p_start_pos += parser_len;
		return -1;
	}

	*p_start_pos += parser_len;
	node_path = elem_array[0];

	if(strlen(node_path) == 0) // must have a path
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_NODE_PATH_NOTEXIST);
		return -1;
	}
	if(silc_mgmtd_if_item_add_node(p_item, node_path, elem_array[1]) < 0)
		return -1;
	return 0;
}


/*
 * MESSAGE
 */
int silc_mgmtd_if_msg_insert_node_cb(const silc_cstr parent_path, silc_mgmtd_if_node* p_node, void* data)
{
	return silc_mgmtd_if_msg_insert_node(data, parent_path, p_node->name, p_node->val_str);
}


int silc_mgmtd_if_msg_to_str(void* buff, silc_mgmtd_if_item* p_item, uint16_t seq_num)
{
	silc_mgmtd_if_msg_insert_head(buff, seq_num);

	if(silc_mgmtd_if_msg_insert_item_head(buff, p_item->type, p_item->return_str,
			p_item->path_str, p_item->p_node_root->val_str) < 0)
		return -1;

	if(silc_mgmtd_if_item_node_sub_traversal(p_item, silc_mgmtd_if_msg_insert_node_cb, buff) < 0)
		return -1;

	if(silc_mgmtd_if_msg_insert_item_tail(buff) < 0)
		return -1;

	if(silc_mgmtd_if_msg_insert_tail(buff) < 0)
		return -1;
	return 0;
}

int silc_mgmtd_if_msg_list_to_str(silc_list* p_item_list, silc_cstr buff, uint32_t seq_num)
{
	silc_mgmtd_if_item* p_item;

	silc_mgmtd_if_msg_insert_head(buff, seq_num);

	silc_list_for_each_entry(p_item, p_item_list, node)
	{
		if(silc_mgmtd_if_msg_insert_item_head(buff, p_item->type, p_item->return_str,
				p_item->path_str, p_item->p_node_root->val_str) < 0)
			return -1;

		if(silc_mgmtd_if_item_node_sub_traversal(p_item, silc_mgmtd_if_msg_insert_node_cb, buff) < 0)
			return -1;

		if(silc_mgmtd_if_msg_insert_item_tail(buff) < 0)
			return -1;
	}

	if(silc_mgmtd_if_msg_insert_tail(buff) < 0)
		return -1;

	return 0;
}

int silc_mgmtd_if_msg_send_items_to_str(silc_mgmtd_msg* p_msg, silc_cstr buff)
{
	return silc_mgmtd_if_msg_list_to_str(&p_msg->if_send_items, buff, p_msg->seq_num);
}

int silc_mgmtd_if_msg_recv_items_from_str(silc_mgmtd_msg* p_msg)
{
	int start_pos = 0;
	void* buff = p_msg->data;
	silc_mgmtd_if_item* p_item;
	int item_num = 0;

	if(silc_mgmtd_if_msg_get_head(buff, &start_pos, & p_msg->seq_num) < 0)
		return -1;

	if(silc_mgmtd_if_msg_check_end(buff, &start_pos))
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_IS_EMPTY);
		return -1;
	}

	while(1)
	{
		if(silc_mgmtd_if_msg_check_end(buff, &start_pos))
			break;
		p_item = silc_mgmtd_if_msg_get_item_head(buff, &start_pos);
		if(!p_item)
			return -1;
		p_item->conn_entry = p_msg->conn_entry;
		silc_list_add_tail(&p_item->node, &p_msg->if_recv_items);
		//printf("add node:%p, list->next:%p, list->prev:%p\n", &p_item->node, p_item_list->next, p_item_list->prev);
		while(1)
		{
			if(silc_mgmtd_if_msg_check_item_end(buff, &start_pos))
				break;
			if(silc_mgmtd_if_msg_get_item_node(buff, &start_pos, p_item) < 0)
				return -1;
		}
		item_num++;
	}
	return item_num;
}

/*
 * --------------------------- CONNECT CALLBACK -----------------------------
 */
int silc_mgmtd_if_get_msg_head()
{
	return strlen(SILC_MGMTD_IF_MSG_START_LABEL)+1;
}

/*
 * if check OK, return 0.
 */
int silc_mgmtd_if_check_msg_head_cb(void* data, int data_len)
{
	int head_len = strlen(SILC_MGMTD_IF_MSG_START_LABEL);

	if(strncmp(data, SILC_MGMTD_IF_MSG_START_LABEL, head_len) == 0)
		return 0;
	return -1;
}

/*
 * if find start, return the head position. if not, return -1
 */
int silc_mgmtd_if_seek_msg_head_cb(void* data, int data_len)
{
	int head_len = strlen(SILC_MGMTD_IF_MSG_START_LABEL);
	int loop;

	for(loop=0;loop<(data_len-head_len);loop++)
	{
		if(strncmp(data+loop, SILC_MGMTD_IF_MSG_START_LABEL, head_len) == 0)
			return loop;
	}
	return -1;
}


/*
 * if find start, return the end last position. if not, return -1
 */
int silc_mgmtd_if_seek_msg_end_cb(void* data, int data_len)
{
	void* end_start;

	end_start = strstr(data, SILC_MGMTD_IF_MSG_END_LABEL);
	if(!end_start)
		return -1;
	else
		return (end_start - data) + strlen(SILC_MGMTD_IF_MSG_END_LABEL);
}


/*
 * ============================== API Interface ==============================
 */
static silc_mgmtd_net s_silc_mgmtd_conn_handler;
static silc_mgmtd_net s_silc_mgmtd_conn_notify_handler;
static silc_list s_silc_mgmtd_if_client_notify_list;

/*
 * =============== Common API =================
 */
silc_mgmtd_msg* silc_mgmtd_if_pop_msg(silc_mgmtd_net* p_handler, int timeout_us)
{
	silc_mgmtd_msg* p_msg;

	p_msg = silc_mgmtd_net_msg_receive(p_handler, timeout_us);
	if(p_msg == NULL)
	{
		return NULL;
	}

	silc_mgmtd_if_msg_recv_items_from_str(p_msg);
	return p_msg;
}


/*
 * Create a request.
 */
silc_mgmtd_if_req* silc_mgmtd_if_req_create(const silc_cstr path,
		silc_mgmtd_if_req_type type, const silc_cstr node_root_val_str)
{
	return silc_mgmtd_if_item_create(path, type, "OK", node_root_val_str);
}


/*
 * Delete a request.
 */
void silc_mgmtd_if_req_delete(silc_mgmtd_if_req* p_req)
{
	return silc_mgmtd_if_item_delete(p_req);
}


/*
 * Clone a request
 */
silc_mgmtd_if_req* silc_mgmtd_if_req_clone(const silc_cstr new_path, silc_mgmtd_if_req* p_req)
{
	return silc_mgmtd_if_item_clone(new_path, p_req);
}


/*
 * Add a node into the request.
 */
int silc_mgmtd_if_req_add_node(silc_mgmtd_if_req* p_req, const silc_cstr node_path, const silc_cstr node_val_str)
{
	return silc_mgmtd_if_item_add_node(p_req, node_path, node_val_str);
}

int silc_mgmtd_if_req_reset_node(silc_mgmtd_if_node* p_node, const silc_cstr node_val_str)
{
	if(p_node->val_str_dync && p_node->val_str)
	{
		free(p_node->val_str);
		p_node->val_str_dync = silc_false;
	}
	p_node->val_str = malloc(strlen(node_val_str)+1);
	if(!p_node->val_str)
		return -1;
	strcpy(p_node->val_str, node_val_str);
	p_node->val_str_dync = silc_true;
	return 0;
}

silc_mgmtd_if_node* silc_mgmtd_if_req_find_node(silc_mgmtd_if_node* p_parent, const silc_cstr name)
{
	silc_mgmtd_if_node* p_node;

	silc_list_for_each_entry(p_node, &p_parent->sub_node_list, node)
	{
		if(strcmp(p_node->name, name) == 0)
			return p_node;
	}
	return NULL;
}


silc_mgmtd_if_node* silc_mgmtd_if_req_add_node_ext(silc_mgmtd_if_node* p_parent,
		const silc_cstr name, const silc_cstr node_val_str)
{
	return silc_mgmtd_if_item_add_node_ext(p_parent, name, node_val_str);
}

/*
 * Send a request and wait the response.
 */
static silc_mgmtd_msg* silc_mgmtd_if_req_send_sync_inner(silc_mgmtd_net *p_handler, silc_mgmtd_if_req* p_req, int timeout_sec)
{

	uint32_t seq_num;
	silc_cstr buff = p_handler->send_buff;
	silc_mgmtd_msg* p_response;

	seq_num = p_handler->curr_sequence;
	p_handler->curr_sequence++;
	if(silc_mgmtd_if_msg_to_str(buff, p_req, seq_num) < 0)
		return NULL;

	if(silc_mgmtd_net_conn_send(p_handler, NULL, buff, strlen(buff)) < 0)
		return NULL;

	p_response = silc_mgmtd_if_pop_msg(p_handler, timeout_sec * 1000000);
	if(p_response == NULL)
	{
		SILC_ERR("[%s] p_handler:%p, receive NULL message.\n", __func__, p_handler);
		return NULL;
	}
	else if(seq_num != p_response->seq_num)
	{
		SILC_ERR("[%s] wrong sequence req[%d] rsp[%d].\n", __func__, seq_num, p_response->seq_num);
		silc_mgmtd_lib_err_set(LIB_ERR_IF_MSG_WRONG_SEQNUM);
		return NULL;
	}

	return p_response;
}

silc_mgmtd_msg* silc_mgmtd_if_client_send_req_sync(silc_mgmtd_if_req* p_req, int timeout_sec)
{
	return silc_mgmtd_if_req_send_sync_inner(&s_silc_mgmtd_conn_handler, p_req, timeout_sec);
}


/*
 * Do a traversal for all the nodes in response.
 */
int silc_mgmtd_if_rsp_node_traversal(silc_mgmtd_if_rsp* p_rsp, silc_mgmtd_if_node_traversal_cb cb_func, void* cb_data)
{
	char path[200];
	silc_cstr cur, start = &path[0];

	strcpy(path, p_rsp->path_str);
	cur = start + strlen(path);
	while(*cur != '/')
	{
		cur--;
		continue;
	}
	*cur = 0;
	return silc_mgmtd_if_node_tree_traversal(path, p_rsp->p_node_root, cb_func, cb_data);
}

/*
 * Delete a response.
 */
void silc_mgmtd_if_rsp_delete(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_item_delete(p_rsp);
}

silc_mgmtd_if_rsp* silc_mgmtd_if_rsp_create(const silc_cstr path,
		const silc_cstr return_str, const silc_cstr node_root_val_str)
{
	return silc_mgmtd_if_item_create(path, SILC_MGMTD_IF_REQ_NULL, return_str, node_root_val_str);
}


int silc_mgmtd_if_rsp_add_node(silc_mgmtd_if_rsp* p_rsp, const silc_cstr node_path, const silc_cstr node_val_str)
{
	return silc_mgmtd_if_item_add_node(p_rsp, node_path, node_val_str);
}

silc_bool silc_mgmtd_if_rsp_check(silc_mgmtd_if_rsp* p_rsp)
{
	if(strcmp(p_rsp->return_str, "OK") == 0)
		return silc_true;
	return silc_false;
}


/*
 * notify
 */
silc_mgmtd_if_notify* silc_mgmtd_if_notify_create(const silc_cstr path, const silc_cstr node_root_val_str)
{
	if(!node_root_val_str)
		return NULL;
	return silc_mgmtd_if_item_create(path, SILC_MGMTD_IF_REQ_NOTIFY, "OK", node_root_val_str);
}

void silc_mgmtd_if_notify_delete(silc_mgmtd_if_notify* p_notify)
{
	return silc_mgmtd_if_item_delete(p_notify);
}

int silc_mgmtd_if_notify_add_node(silc_mgmtd_if_notify* p_notify, const silc_cstr node_path, const silc_cstr node_val_str)
{
	return silc_mgmtd_if_item_add_node(p_notify, node_path, node_val_str);
}

#include <sys/types.h>
#include <sys/wait.h>
int silc_mgmtd_if_system(const silc_cstr cmd)
{
	int ret = system(cmd);
	ret = WEXITSTATUS(ret);
	return ret;
}

int silc_mgmtd_if_exec_system_cmd(const silc_cstr cmd, silc_cstr output_buf, int *p_buf_len, int timeout_msec, silc_bool retry)
{
	int ret = 0, cur_read = 0;
	int pipe_fd[2];
	pid_t pid, ret_pid;
	int bk;
	char temp[16];

	if(output_buf && p_buf_len)
		memset(output_buf, 0, *p_buf_len);
	if(pipe2(pipe_fd, O_NONBLOCK) < 0)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}

	pid = fork();
	if(pid == 0)
	{
		close(pipe_fd[0]);
		bk = dup(1);
		dup2(pipe_fd[1], 1); // redirect stdout to pipe
		while(1)
		{
			lseek(pipe_fd[1], 0, SEEK_SET);
			ret = system(cmd);
			if(ret == 0 || !retry)
				break;
			usleep(100000);
		}
		dup2(bk, 1);
		SILC_TRACE("[%s](child %d) system run:%s, return:%d\n", __func__, getpid(), cmd, ret);
		if(sizeof(int) != write(pipe_fd[1], &ret, sizeof(int)))
		{
			SILC_ERR("[%s](child %d) write pipe error!\n", __func__, getpid());
		}
		close(pipe_fd[1]);
		exit(0);
	}
	else if(pid > 0)
	{
		close(pipe_fd[1]);
		while(timeout_msec-- > 0)
		{
			ret_pid = waitpid(pid, NULL, WNOHANG);
			if(ret_pid == pid)
			{
				if(!output_buf)
				{
					// check retcode
					if(sizeof(int) == read(pipe_fd[0], temp, 16))
					{
						memcpy(&ret, temp, sizeof(int));
						if (0 != WEXITSTATUS(ret))
							ret = -1;
					}
					goto NORMAL_OUT;
				}
				cur_read = read(pipe_fd[0], output_buf, *p_buf_len);
				if(cur_read >= sizeof(int) && cur_read < *p_buf_len)
				{
					memcpy(&ret, &output_buf[cur_read-sizeof(int)], sizeof(int));
					cur_read = cur_read - sizeof(int);
					output_buf[cur_read] = 0;
					if (0 != WEXITSTATUS(ret))
						ret = -1;
					goto NORMAL_OUT;
				}
				else
				{
					silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
					goto ERROR_OUT;
				}
			}
			usleep(1000);
		}
		silc_mgmtd_lib_err_set(LIB_ERR_IF_EXEC_SYSCMD_TIMEOUT);
		goto ERROR_OUT;
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
	return -1;
NORMAL_OUT:
	close(pipe_fd[0]);
	if(p_buf_len)
		*p_buf_len = cur_read;
	if(output_buf)
		SILC_TRACE("[%s] system run output:%s\n", __func__, output_buf);

	return ret;
ERROR_OUT:
	close(pipe_fd[0]);
	kill(pid, SIGTERM);
	return -1;
}

typedef struct sys_cmd_arg_s {
	char cmd[256];
	int timeout;
} sys_cmd_arg;


void* silc_mgmtd_if_system_cmd_once(void* thread_arg)
{
	sys_cmd_arg* arg = (sys_cmd_arg*)thread_arg;

	sleep(arg->timeout);
	silc_mgmtd_if_system(arg->cmd);
	free(arg);
	return NULL;
}

void silc_mgmtd_if_system_cmd_later(char* cmd, int timeout)
{
	sys_cmd_arg* arg = malloc(sizeof(sys_cmd_arg));
	strcpy(arg->cmd, cmd);
	arg->timeout = timeout;
	silc_thread_create_detached(silc_mgmtd_if_system_cmd_once, arg);
}


silc_cstr silc_mgmtd_if_get_client_type_str(silc_mgmtd_if_client_type type)
{
	static silc_cstr type_names[] = {
			"NULL",
			"SNMP",
			"SWITCH",
			"LOCAL",
			"SSH",
			"TELNET",
			"WEB",
			"NETCONF",
			"MAX"
	};
	return type_names[type];
}

silc_mgmtd_if_client_type silc_mgmtd_if_get_client_type(const silc_cstr type_name)
{
	if(strcmp(type_name, "NULL") == 0)
		return SILC_MGMTD_IF_CLIENT_NULL;
	else if(strcmp(type_name, "LOCAL") == 0)
		return SILC_MGMTD_IF_CLIENT_LOCAL;
	else if(strcmp(type_name, "SNMP") == 0)
		return SILC_MGMTD_IF_CLIENT_SNMP;
	else if(strcmp(type_name, "SWITCH") == 0)
		return SILC_MGMTD_IF_CLIENT_SWITCH;
	else if(strcmp(type_name, "SSH") == 0)
		return SILC_MGMTD_IF_CLIENT_SSH;
	else if(strcmp(type_name, "TELNET") == 0)
		return SILC_MGMTD_IF_CLIENT_TELNET;
	else if(strcmp(type_name, "WEB") == 0)
		return SILC_MGMTD_IF_CLIENT_WEB;
	else if(strcmp(type_name, "NETCONF") == 0)
		return SILC_MGMTD_IF_CLIENT_NETCONF;
	else if(strcmp(type_name, "MAX") == 0)
		return SILC_MGMTD_IF_CLIENT_MAX;

	return SILC_MGMTD_IF_CLIENT_MAX;
}

/*
 * =================== Client(CLI/GUI/Internal) API ===================
 */
void silc_mgmtd_if_client_free_msg(silc_mgmtd_msg* p_msg)
{
	silc_mgmtd_if_item *p_item, *p_tmp_item;
	silc_list_for_each_entry_safe(p_item, p_tmp_item, &p_msg->if_recv_items, node)
	{
		silc_list_del(&p_item->node);
		silc_mgmtd_if_item_delete(p_item);
	}
	silc_list_for_each_entry_safe(p_item, p_tmp_item, &p_msg->if_send_items, node)
	{
		silc_list_del(&p_item->node);
		silc_mgmtd_if_item_delete(p_item);
	}

	silc_mgmtd_net_msg_free(p_msg);
}

void silc_mgmtd_if_client_clear_msg(silc_mgmtd_msg* p_msg)
{
	silc_mgmtd_if_item *p_item, *p_tmp_item;
	silc_list_for_each_entry_safe(p_item, p_tmp_item, &p_msg->if_recv_items, node)
	{
		silc_list_del(&p_item->node);
		silc_mgmtd_if_item_delete(p_item);
	}
	silc_list_for_each_entry_safe(p_item, p_tmp_item, &p_msg->if_send_items, node)
	{
		silc_list_del(&p_item->node);
		silc_mgmtd_if_item_delete(p_item);
	}
}

int silc_mgmtd_if_client_init(const silc_cstr server_addr, int listen_port, int idle_timeout_sec, int conn_timeout_sec)
{
	silc_time_lib_init();

	memset(&s_silc_mgmtd_conn_handler, 0, sizeof(s_silc_mgmtd_conn_handler));

	silc_sem_init(&s_silc_mgmtd_conn_handler.sleep_sem);

	if(silc_mgmtd_net_init(&s_silc_mgmtd_conn_handler, NULL, listen_port, idle_timeout_sec,
			server_addr, silc_mgmtd_if_get_msg_head(), silc_mgmtd_if_check_msg_head_cb,
			silc_mgmtd_if_seek_msg_head_cb, silc_mgmtd_if_seek_msg_end_cb) < 0)
		return -1;

	if(silc_mgmtd_net_init(&s_silc_mgmtd_conn_notify_handler, NULL, listen_port, -1,	// receive notify
			server_addr, silc_mgmtd_if_get_msg_head(), silc_mgmtd_if_check_msg_head_cb,
			silc_mgmtd_if_seek_msg_head_cb, silc_mgmtd_if_seek_msg_end_cb) < 0)
		return -1;

	if(silc_mgmtd_net_client_start(&s_silc_mgmtd_conn_handler, conn_timeout_sec) < 0)
		return -1;

	silc_list_init(&s_silc_mgmtd_if_client_notify_list);
	if(silc_mgmtd_net_client_start(&s_silc_mgmtd_conn_notify_handler, conn_timeout_sec) < 0)
		return -1;

	return 0;
}

void silc_mgmtd_if_client_shutdown(void )
{
	silc_mgmtd_net_stop(&s_silc_mgmtd_conn_handler);
	silc_mgmtd_net_deinit(&s_silc_mgmtd_conn_handler);

	silc_time_lib_deinit();
}

int __silc_mgmtd_if_client_set_login_info(silc_mgmtd_if_client_type type,
		const silc_cstr user_name, const silc_cstr login_ip, const silc_cstr login_port,
		const silc_cstr privil, const silc_cstr proto)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_SET_LGOIN_INFO, SILC_MGMTD_IF_REQ_ACTION, "");
	if(!p_req)
		return -1;

	if(silc_mgmtd_if_req_add_node(p_req, "type", silc_mgmtd_if_get_client_type_str(type)) < 0 ||
			(user_name && silc_mgmtd_if_req_add_node(p_req, "username", user_name) < 0) ||
			(login_ip && silc_mgmtd_if_req_add_node(p_req, "ip", login_ip) < 0) ||
			(login_port && silc_mgmtd_if_req_add_node(p_req, "port", login_port) < 0) ||
			(privil && silc_mgmtd_if_req_add_node(p_req, "privilege", privil) < 0) ||
			(proto && silc_mgmtd_if_req_add_node(p_req, "protocol", proto) < 0))
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_SET_LOGIN_INFO_FAILED);
		ret = -1;
	}

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);
	return ret;

}

int silc_mgmtd_if_client_set_login_info(silc_mgmtd_if_client_type type,
		const silc_cstr user_name, const silc_cstr login_ip, const silc_cstr login_port)
{
	return __silc_mgmtd_if_client_set_login_info(type, user_name, login_ip, login_port, NULL, NULL);
}

int silc_mgmtd_if_client_set_login_info_ex(silc_mgmtd_if_client_type type,
		const silc_cstr user_name, const silc_cstr login_ip, const silc_cstr login_port,
		const silc_cstr privil, const silc_cstr proto)
{
	return __silc_mgmtd_if_client_set_login_info(type, user_name, login_ip, login_port, privil, proto);
}

int silc_mgmtd_if_client_get_mgmtd_session_timeout(int* p_timeout)
{
	int ret = 0;
	long timeout_sec;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_CFG_SESS_EXP_TIME, SILC_MGMTD_IF_REQ_QUERY, "");
	if(!p_req)
		return -1;

	p_msg = silc_mgmtd_if_client_send_req_sync(p_req, 10);
	if(!p_msg)
	{
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	p_rsp = silc_list_entry(p_msg->if_recv_items.next, silc_mgmtd_if_rsp, node);
	if(!silc_mgmtd_if_rsp_check(p_rsp))
	{
		silc_mgmtd_lib_err_set(LIB_ERR_IF_GET_IDLE_TIMEOUT_FAILED);
		ret = -1;
	}
	else if(silc_mgmtd_if_cstr2l(p_rsp->p_node_root->val_str, &timeout_sec) != 0)
		ret = -1;

	*p_timeout = timeout_sec;
	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);
    return ret;
}

void silc_mgmtd_if_client_set_local_session_timeout(int timeout_sec)
{
	silc_sock_conn_set_idle_timeout(s_silc_mgmtd_conn_handler.sock, timeout_sec);
}

int silc_mgmtd_if_client_get_local_session_timeout()
{
	return silc_sock_conn_get_idle_timeout(s_silc_mgmtd_conn_handler.sock);
}




typedef struct silc_mgmtd_if_notify_args_s
{
	silc_list_node node;
	char notify_path[SILC_MGMTD_IF_PATH_MAX_LEN];
	silc_mgmtd_if_notify_cb daemon_entry;
	void* data;
}silc_mgmtd_if_notify_args;

int silc_mgmtd_if_client_handle_notify(silc_mgmtd_if_notify* p_notify)
{
	silc_mgmtd_if_notify_args* p_args;

	silc_list_for_each_entry(p_args, &s_silc_mgmtd_if_client_notify_list, node)
	{
		SILC_TRACE("compare receive:%s, current:%s\n", p_notify->path_str, p_args->notify_path);
		if(strcmp(p_args->notify_path, p_notify->path_str) == 0)
		{
			if(p_args->daemon_entry)
				return p_args->daemon_entry(p_notify, p_args->data);
			return 0;
		}
	}
	return 0;
}

int silc_mgmtd_if_client_set_recv_notify(const silc_cstr path,
		silc_mgmtd_if_notify_cb daemon_entry, void* data)
{
	int ret = 0;
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_req *p_req;
	silc_mgmtd_if_rsp *p_rsp;
	silc_mgmtd_if_notify_args* p_args = malloc(sizeof(silc_mgmtd_if_notify_args));

	if(!p_args)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}

	p_req = silc_mgmtd_if_req_create(SILC_MGMTD_IF_PATH_SET_NOTIFY_PATH, SILC_MGMTD_IF_REQ_ACTION, path);
	if(!p_req)
		return -1;

	SILC_TRACE("Setup receive notify for %s", path);
	p_msg = silc_mgmtd_if_req_send_sync_inner(&s_silc_mgmtd_conn_notify_handler, p_req, 100);
	if(!p_msg)
	{
		SILC_ERR("Failed to setup notify receive function for %s", path);
		silc_mgmtd_if_req_delete(p_req);
		return -1;
	}

	silc_list_for_each_entry(p_rsp, &p_msg->if_recv_items, node)
	{
		if(!silc_mgmtd_if_rsp_check(p_rsp))
		{
			silc_mgmtd_lib_err_set(LIB_ERR_IF_SET_NOTIFY_PATH_FAILED);
			ret = -1;
			break;
		}
	}

	silc_mgmtd_if_req_delete(p_req);
	silc_mgmtd_if_client_free_msg(p_msg);

	p_args->daemon_entry = daemon_entry;
	p_args->data = data;
	strcpy(p_args->notify_path, path);
	silc_list_add_tail(&p_args->node, &s_silc_mgmtd_if_client_notify_list);
    return ret;
}

void* silc_mgmtd_if_client_notify_thread(void* data)
{
	silc_mgmtd_msg* p_msg;
	silc_mgmtd_if_notify *p_notify, *p_tmp;

	while(1)
	{
		p_msg = silc_mgmtd_if_pop_msg(&s_silc_mgmtd_conn_notify_handler, 10000);
		if(p_msg == NULL)
			continue;
		silc_list_for_each_entry_safe(p_notify, p_tmp, &p_msg->if_recv_items, node)
		{
			SILC_TRACE("[%s] Receive notify, path:%s\n", __func__, p_notify->path_str);
			silc_mgmtd_if_client_handle_notify(p_notify);
			silc_list_del(&p_notify->node);
			silc_mgmtd_if_notify_delete(p_notify);
		}
	}

	return NULL;

}

int silc_mgmtd_if_client_start_notify_daemon(void)
{
	return silc_thread_create_detached(silc_mgmtd_if_client_notify_thread, NULL);
}


/*
 * ================= Server(mgmtd) API ================
 */
int silc_mgmtd_if_server_init(const char* bind_addr, int listen_port, int timeout_sec)
{
	if(silc_mgmtd_net_init(&s_silc_mgmtd_conn_handler, bind_addr, listen_port, timeout_sec,
			NULL, silc_mgmtd_if_get_msg_head(), silc_mgmtd_if_check_msg_head_cb,
			silc_mgmtd_if_seek_msg_head_cb, silc_mgmtd_if_seek_msg_end_cb) < 0)
		return -1;

	if(silc_mgmtd_net_server_start(&s_silc_mgmtd_conn_handler) < 0)
		return -1;
	return 0;
}


silc_mgmtd_msg* silc_mgmtd_if_server_pop_msg(int timeout_us)
{
	return silc_mgmtd_if_pop_msg(&s_silc_mgmtd_conn_handler, timeout_us);
}

void silc_mgmtd_if_server_simplify_rsp_list(silc_list* p_rsp_list)
{
	silc_mgmtd_if_rsp* p_rsp;
	silc_mgmtd_if_node *p_node, *p_tmp_node;

	silc_list_for_each_entry(p_rsp, p_rsp_list, node)
	{
		if(p_rsp->type == SILC_MGMTD_IF_REQ_ADD ||
				p_rsp->type == SILC_MGMTD_IF_REQ_MODIFY ||
				p_rsp->type == SILC_MGMTD_IF_REQ_DELETE ||
				p_rsp->type == SILC_MGMTD_IF_REQ_ACTION)
		{
			silc_list_for_each_entry_safe(p_node, p_tmp_node, &p_rsp->p_node_root->sub_node_list, node)
			{
				silc_list_del(&p_node->node);
				silc_mgmtd_if_server_node_delete(p_node);
			}
		}
	}
}

int silc_mgmtd_if_server_send_msg(silc_mgmtd_msg *p_request_msg)
{
	silc_cstr buff = s_silc_mgmtd_conn_handler.send_buff;

	silc_mgmtd_if_server_simplify_rsp_list(&p_request_msg->if_send_items);
	if(silc_mgmtd_if_msg_send_items_to_str(p_request_msg, buff) < 0)
	{
		silc_mgmtd_req_dbg(p_request_msg, "ToStrFail", "Resp: %s", p_request_msg->data);
		return -1;
	}

	silc_mgmtd_req_dbg(p_request_msg, "SendResp", "Resp: %s", buff);

	//printf("[%s]send: %s\n", __func__, buff);
	if(silc_mgmtd_net_conn_send(&s_silc_mgmtd_conn_handler, p_request_msg->conn_entry, buff, strlen(buff)) < 0)
		return -1;

	return 0;
}

int silc_mgmtd_if_server_send_notify(silc_mgmtd_if_notify* p_notify)
{
	silc_list notify_list;
	silc_cstr buff = s_silc_mgmtd_conn_notify_handler.send_buff;

	silc_list_init(&notify_list);
	silc_list_add(&p_notify->node, &notify_list);

	if(silc_mgmtd_if_msg_list_to_str(&notify_list, buff, 0) < 0)
		return -1;

	//printf("[%s]send notify: %s\n", __func__, buff);
	if(silc_mgmtd_net_server_notify(p_notify->path_str, &s_silc_mgmtd_conn_handler, buff, strlen(buff)) < 0)
		return -1;

	return 0;
}


int silc_mgmtd_if_server_get_ssh_sessions_info(silc_list* p_list)
{
	silc_mgmtd_if_session_info* p_info;
	silc_mgmtd_conn* p_conn;
	struct tm tm;

	silc_mutex_lock(s_silc_mgmtd_conn_handler.conn_lock);
	silc_list_for_each_entry(p_conn, &s_silc_mgmtd_conn_handler.conn_list, node)
	{
		if(p_conn->type != SILC_MGMTD_IF_CLIENT_LOCAL &&
				p_conn->type != SILC_MGMTD_IF_CLIENT_SSH &&
				p_conn->type != SILC_MGMTD_IF_CLIENT_TELNET)
			continue;
		p_info = malloc(sizeof(*p_info));
		if(!p_info)
		{
			silc_mutex_unlock(s_silc_mgmtd_conn_handler.conn_lock);
			silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
			return -1;
		}
		memset(p_info, 0, sizeof(*p_info));
//		printf("[%s] user:%s\n", __func__, p_conn->login_user);
		if(localtime_r(&p_conn->login_time, &tm) == NULL ||
				strftime(p_info->login_time, 40, "%F %T", &tm) < 0)
			strcpy(p_info->login_time, "unknown");
		sprintf(p_info->index, "%d", p_conn->index);
		strcpy(p_info->login_user, p_conn->login_user);
		strcpy(p_info->login_ip, p_conn->login_ip);
		p_info->login_port = p_conn->login_port;
		strcpy(p_info->type, silc_mgmtd_if_get_client_type_str(p_conn->type));
		silc_list_add_tail(&p_info->node, p_list);
	}
	silc_mutex_unlock(s_silc_mgmtd_conn_handler.conn_lock);
	return 0;
}

int silc_mgmtd_if_server_get_web_sessions_info(silc_list* p_list)
{
	FILE* fp;
	char line[256];
	silc_mgmtd_if_session_info* p_info;
	silc_cstr_array* arg_list;

	fp = popen("/opt/silc-web/silc-web-cmd.sh showsess", "r");
	while (fgets(line, 256, fp))
	{
		arg_list = silc_cstr_split(line, " ");
		if(!arg_list)
		{
			SILC_ERR("silc_cstr_split error");
			continue;
		}
		if(arg_list->length < 6)
		{
			SILC_ERR("Invalid line %s", line);
			silc_cstr_array_destroy(arg_list);
			continue;
		}
		p_info = malloc(sizeof(*p_info));
		if(!p_info)
		{
			SILC_ERR("malloc error");
			silc_cstr_array_destroy(arg_list);
			continue;
		}
		memset(p_info, 0, sizeof(*p_info));
		strcpy(p_info->login_user, silc_cstr_array_get_quick(arg_list, 2));
		sprintf(p_info->login_time, "%s %s", silc_cstr_array_get_quick(arg_list, 3), silc_cstr_array_get_quick(arg_list, 4));
		strcpy(p_info->login_ip, silc_cstr_array_get_quick(arg_list, 5));
		p_info->login_port = 0;
		strcpy(p_info->type,"WEB");
		silc_list_add_tail(&p_info->node, p_list);
		silc_cstr_array_destroy(arg_list);
	}
	pclose(fp);
	return 0;
}

int silc_mgmtd_if_server_get_online_sessions_info(silc_list* p_list)
{
	silc_list_init(p_list);
	silc_mgmtd_if_server_get_ssh_sessions_info(p_list);
	silc_mgmtd_if_server_get_web_sessions_info(p_list);
	return 0;
}

void silc_mgmtd_if_server_free_online_sessions_info(silc_list* p_list)
{
	silc_mgmtd_if_session_info *p_info, *p_safe;
	silc_list_for_each_entry_safe(p_info, p_safe, p_list, node)
	{
		silc_list_del(&p_info->node);
		free(p_info);
	}
}

silc_mgmtd_if_client_type silc_mgmtd_if_server_get_conn_type(void* conn_entry)
{
	return ((silc_mgmtd_conn*)conn_entry)->type;
}

silc_mgmtd_if_privilege silc_mgmtd_if_server_get_conn_privilege(void* conn_entry)
{
	return ((silc_mgmtd_conn*)conn_entry)->if_level;
}

void silc_mgmtd_if_server_set_conn_privilege(void* conn_entry, silc_mgmtd_if_privilege level)
{
	((silc_mgmtd_conn*)conn_entry)->if_level = level;
}

uint16_t silc_mgmtd_if_server_get_conn_id(void* conn_entry)
{
	return ((silc_mgmtd_conn*)conn_entry)->index;
}

const silc_cstr silc_mgmtd_if_server_get_conn_index_user(uint16_t index)
{
	silc_cstr none = "Notfind";
	silc_mgmtd_conn* p_conn;

	silc_mutex_lock(s_silc_mgmtd_conn_handler.conn_lock);
	silc_list_for_each_entry(p_conn, &s_silc_mgmtd_conn_handler.conn_list, node)
	{
		if(p_conn->index == index)
		{
			silc_mutex_unlock(s_silc_mgmtd_conn_handler.conn_lock);
			return p_conn->login_user;
		}
	}
	silc_mutex_unlock(s_silc_mgmtd_conn_handler.conn_lock);
	return none;
}

const silc_cstr silc_mgmtd_if_server_get_conn_entry_user(void* conn_entry)
{
	silc_mgmtd_conn* p_conn = (silc_mgmtd_conn*)conn_entry;
	return p_conn->login_user;
}

int silc_mgmtd_if_server_conn_login_set(void* conn_entry,
		silc_mgmtd_if_client_type type, const silc_cstr user_name, const silc_cstr ip, uint16_t port)
{
	silc_mgmtd_conn* p_entry = (silc_mgmtd_conn*)conn_entry;
	int len;

//	printf("entry: %p\n", p_entry);
	p_entry->type = type;

	if(!user_name)
		return 0;

	if(ip)
		len = strlen(user_name)+strlen(ip)+2;
	else
		len = strlen(user_name)+1;
	p_entry->login_user = malloc(len);
	if(!p_entry->login_user)
	{
		silc_mgmtd_lib_err_set(LIB_ERR_SYS_CLIB);
		return -1;
	}
	strcpy(p_entry->login_user, user_name);
	if(ip)
	{
		p_entry->login_ip = p_entry->login_user + strlen(user_name) + 1;
		strcpy(p_entry->login_ip, ip);
		p_entry->login_port = port;
	}
	return 0;
}

int silc_mgmtd_if_server_notify_path_set(void* conn_entry, const silc_cstr notify_path)
{
	int loop;
	silc_mgmtd_conn* p_entry = (silc_mgmtd_conn*)conn_entry;

	for(loop=0; loop<SILC_MGMTD_CONN_RECV_NOTIFY_MAX; loop++)
	{
		if(!p_entry->recv_notify_path_list[loop])	// find a null pointer
		{
			p_entry->recv_notify_path_list[loop] = malloc(strlen(notify_path) + 1);
			strcpy(p_entry->recv_notify_path_list[loop], notify_path);
			return 0;
		}
	}
	silc_mgmtd_lib_err_set(LIB_ERR_IF_NOT_ENOUGH_NOTIFY_PATH);
	return -1;
}

int silc_mgmtd_if_server_notify(silc_cstr path)
{
	silc_mgmtd_if_notify* p_notify;

	p_notify = silc_mgmtd_if_notify_create(path, "");
	if(!p_notify)
	{
		SILC_ERR("Send notify %s failed!\n", path);
		return -1;
	}

	if(silc_mgmtd_if_server_send_notify(p_notify) != 0)
	{
		SILC_ERR("Send notify %s failed!\n", path);
		silc_mgmtd_if_notify_delete(p_notify);
		return -1;
	}

	silc_mgmtd_if_notify_delete(p_notify);
    return 0;
}

void silc_mgmtd_if_trans_privil(silc_cstr privil)
{
	int outer_privil = atoi(privil);
	if (outer_privil >= 10)
		strcpy(privil, "3");
	else if (outer_privil >= 5)
		strcpy(privil, "2");
	else
		strcpy(privil, "1");
}


//vendor list is defined in product.
//these two variables are initialized in produt_info, either from cli or mgmtd
silc_cstr g_silc_mgmtd_product_name = NULL;
uint32_t g_silc_mgmtd_product_id = 0xFFFFFFFFUL;
silc_cstr* silc_mgmtd_if_vendor_list = NULL;
int	silc_mgmtd_if_vendor_cnt = 0;

void silc_mgmtd_if_product_info_set(silc_cstr product_name, int product_id, silc_cstr* vendor_list, int vendor_cnt)
{
	g_silc_mgmtd_product_name = product_name;
	g_silc_mgmtd_product_id = product_id;
	silc_mgmtd_if_vendor_list = vendor_list;
	silc_mgmtd_if_vendor_cnt = vendor_cnt;
}

static char silc_mgmtd_if_vendor_name[64] = {0xFF};
static int silc_mgmtd_if_vendor_id = -1;

static int silc_mgmtd_if_read_vendor()
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_cstr vendor_file = "/etc/product/vendor";
	silc_cstr vendor_name = NULL;
	uint32_t buf_len=0;
	int i;

	if(!silc_mgmtd_if_vendor_list)
	{
		SILC_ERR("Vendor list is empty!");
		return -1;
	}
	silc_mgmtd_if_vendor_name[0] = 0;
	silc_mgmtd_if_vendor_id = VENDOR_DEFAULT;
	//is_oem.sh will write this file /etc/product/vendor, we don't need to call is_upgrade.sh to get the vendor name
	if(0!=silc_file_read_all(vendor_file, &vendor_name, &buf_len))
	{
		SILC_ERR("Fail to read %s", vendor_file);
		return -1;
	}

	for(i=0; i<silc_mgmtd_if_vendor_cnt; i++)
	{
		if(strncmp(vendor_name, silc_mgmtd_if_vendor_list[i], strlen(silc_mgmtd_if_vendor_list[i])) == 0)
		{
			strcpy(silc_mgmtd_if_vendor_name, silc_mgmtd_if_vendor_list[i]);
			silc_mgmtd_if_vendor_id = SILC_MGMTD_PRODUCT_ID<<4 | (i+1);
			break;
		}
	}
	free(vendor_name);
#else
	strcpy(silc_mgmtd_vendor_name, "oem_test");
#endif
	return 0;
}

silc_cstr silc_mgmtd_if_get_vendor_name()
{
	if(silc_mgmtd_if_vendor_name[0] == 0xFF)
	{
		silc_mgmtd_if_read_vendor();
	}
	if(silc_mgmtd_if_vendor_name[0])
		return silc_mgmtd_if_vendor_name;
	return NULL;
}

silc_bool silc_mgmtd_if_cmp_vendor_name(silc_cstr name)
{
	silc_cstr vendor = silc_mgmtd_if_get_vendor_name();
	if(vendor)
		return (strcmp(vendor, name) == 0);
	return silc_false;
}

int silc_mgmtd_if_get_vendor_id()
{
	if(silc_mgmtd_if_vendor_id == -1)
	{
		silc_mgmtd_if_read_vendor();
	}
	return silc_mgmtd_if_vendor_id;
}

silc_bool silc_mgmtd_if_cmp_vendor_id(silc_mgmtd_vendor_id id)
{
	return (silc_mgmtd_if_get_vendor_id() == id);
}
