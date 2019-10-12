/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_vnode.h"

#ifdef SILC_MGMTD_LOCAL_DEBUG
#define SILC_SNMP_CONF_FULL_NAME    "/etc/snmp/snmpd.conf"
#define SILC_SNMP_CONF2_FULL_NAME 	"/var/lib/net-snmp/snmpd.conf"
#else
#define SILC_SNMP_CONF_FULL_NAME    "/etc/snmp/snmpd.conf"
#define SILC_SNMP_CONF2_FULL_NAME 	"/var/lib/snmp/snmpd.conf"
#endif

typedef struct is_mgmtd_snmp_info_s
{
	silc_bool state;
	char 	engine_id[200];
	silc_list community_list;
	silc_list user_list;
	silc_list traphost_list;
}is_mgmtd_snmp_info;

typedef struct is_mgmtd_snmp_community_s
{
	silc_list_node node;
	silc_cstr name;
	silc_bool full_access;
}is_mgmtd_snmp_community;

typedef struct is_mgmtd_snmp_user_s
{
	silc_list_node node;
	silc_cstr name;
	silc_bool full_access;
}is_mgmtd_snmp_user;

typedef struct is_mgmtd_snmp_traphost_s
{
	silc_list_node node;
	silc_cstr ip;
	silc_cstr version;
	silc_cstr community;
	silc_cstr auth;
	silc_cstr passwd;
}is_mgmtd_snmp_traphost;

#define is_mgmtd_snmp_status_free_list(list, type)	\
	do{\
		type *ptr, *tmp; \
		silc_list_for_each_entry_safe(ptr, tmp, list, node) \
		{ \
			free(ptr); \
		} \
	}while(0)

silc_bool is_mgmtd_snmp_status_get_agent_state()
{
#ifdef SILC_MGMTD_LOCAL_DEBUG
	silc_cstr cmd = "sudo service snmpd status|grep running";
#else
	silc_cstr cmd = "ps|grep -v grep|grep -w snmpd";
#endif
	if(silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 2000, silc_false) != 0)
		return silc_false;
	return silc_true;
}

int is_mgmtd_snmp_status_get_engine_id(silc_cstr buf, int buf_len)
{
	int len = 200;
	char cmd[100];
	char output[len];
	silc_cstr cur;
	silc_cstr key = "oldEngineID ";

#ifdef SILC_MGMTD_LOCAL_DEBUG
	sprintf(cmd, "sudo cat %s|grep oldEngineID", SILC_SNMP_CONF2_FULL_NAME);
#else
	sprintf(cmd, "cat %s|grep oldEngineID", SILC_SNMP_CONF2_FULL_NAME);
#endif

	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 2000, silc_false) != 0)
		return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;

	cur = strchr(output, '\n');
	if(cur)
		*cur = 0;
	cur = strstr(output, key);
	if(!cur)
		return IS_MGMTD_ERR_BASE_GET_INFO_FAILED;
	cur += strlen(key);
	if(strlen(cur) >= buf_len)
		return IS_MGMTD_ERR_BASE_NOT_ENOUGH_BUF;
	strcpy(buf, cur);
	return 0;
}

int is_mgmtd_snmp_status_get_communites(silc_list *p_list)
{
	int loop;
	int len = 1000;
	char cmd[100];
	char output[len];
	silc_cstr line;
	silc_cstr_array* p_arr;
	is_mgmtd_snmp_community* p_community;

#ifdef SILC_MGMTD_LOCAL_DEBUG
	sprintf(cmd, "sudo cat %s|egrep \"rocommunity|rwcommunity\"", SILC_SNMP_CONF_FULL_NAME);
#else
	sprintf(cmd, "cat %s|egrep \"rocommunity|rwcommunity\"", SILC_SNMP_CONF_FULL_NAME);
#endif
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 2000, silc_false) != 0)
		return 0;	// no community

	p_arr = silc_cstr_split(output, "\n");
	if(!p_arr)
		return IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED;

	silc_list_init(p_list);
	for(loop=0; loop<p_arr->length; loop++)
	{
		silc_cstr val_str;
		silc_cstr ro_key = "rocommunity ";
		silc_cstr rw_key = "rwcommunity ";
		silc_bool access;
		line = silc_cstr_array_get_quick(p_arr, loop);
		if((val_str = strstr(line, ro_key)))
		{
			val_str += strlen(ro_key);
			access = silc_false;
		}
		else if((val_str = strstr(line, rw_key)))
		{
			val_str += strlen(rw_key);
			access = silc_true;
		}
		else
			continue;
		p_community = malloc(sizeof(is_mgmtd_snmp_community) + strlen(val_str) + 1);
		if(!p_community)
			goto ERROR;
		p_community->name = (silc_cstr)(p_community+1);
		p_community->full_access = access;
		strcpy(p_community->name, val_str);
		silc_list_add_tail(&p_community->node, p_list);
	}
	silc_cstr_array_destroy(p_arr);
	return 0;
ERROR:
	silc_cstr_array_destroy(p_arr);
	is_mgmtd_snmp_status_free_list(p_list, is_mgmtd_snmp_community);
	return IS_MGMTD_ERR_BASE_MALLOC_FAILED;
}

int is_mgmtd_snmp_status_get_users(silc_list* p_list)
{
	int loop;
	int len = 1000;
	char cmd[100];
	char output[len];
	silc_cstr line;
	silc_cstr_array* p_arr;
	is_mgmtd_snmp_user* p_user;

#ifdef SILC_MGMTD_LOCAL_DEBUG
	sprintf(cmd, "sudo cat %s|egrep \"rouser|rwuser\"", SILC_SNMP_CONF_FULL_NAME);
#else
	sprintf(cmd, "cat %s|egrep \"rouser|rwuser\"", SILC_SNMP_CONF_FULL_NAME);
#endif
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 2000, silc_false) != 0)
		return 0;

	p_arr = silc_cstr_split(output, "\n");
	if(!p_arr)
		return IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED;

	silc_list_init(p_list);
	for(loop=0; loop<p_arr->length; loop++)
	{
		silc_cstr val_str;
		silc_cstr ro_key = "rouser ";
		silc_cstr rw_key = "rwuser ";
		silc_bool access;
		line = silc_cstr_array_get_quick(p_arr, loop);
		if((val_str = strstr(line, ro_key)))
		{
			val_str += strlen(ro_key);
			access = silc_false;
		}
		else if((val_str = strstr(line, rw_key)))
		{
			val_str += strlen(rw_key);
			access = silc_true;
		}
		else
			continue;
		p_user = malloc(sizeof(is_mgmtd_snmp_user) + strlen(val_str) + 1);
		if(!p_user)
			goto ERROR;
		p_user->name = (silc_cstr)(p_user+1);
		p_user->full_access = access;
		strcpy(p_user->name, val_str);
		silc_list_add_tail(&p_user->node, p_list);
	}
	silc_cstr_array_destroy(p_arr);
	return 0;
ERROR:
	silc_cstr_array_destroy(p_arr);
	is_mgmtd_snmp_status_free_list(p_list, is_mgmtd_snmp_user);
	return IS_MGMTD_ERR_BASE_MALLOC_FAILED;
}

int is_mgmtd_snmp_status_get_trap_hosts(silc_list* p_list)
{
	int i;
	int len = 1000;
	char cmd[100];
	char output[len];
	silc_cstr line;
	silc_cstr_array* p_arr;
	is_mgmtd_snmp_traphost* p_traphost;

#ifdef SILC_MGMTD_LOCAL_DEBUG
	sprintf(cmd, "sudo cat %s|egrep \"trapsink|trap2sink|trapsess\"", SILC_SNMP_CONF_FULL_NAME);
#else
	sprintf(cmd, "cat %s|egrep \"trapsink|trap2sink|trapsess\"", SILC_SNMP_CONF_FULL_NAME);
#endif
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 2000, silc_false) != 0)
		return 0;

	p_arr = silc_cstr_split(output, "\n");
	if(!p_arr)
		return IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED;

	silc_list_init(p_list);
	for(i=0; i<p_arr->length; i++)
	{
		silc_cstr val_str, ip, ver, community, auth, passwd;
		silc_cstr_array* p_sub_arr;

		line = silc_cstr_array_get_quick(p_arr, i);
		p_sub_arr = silc_cstr_split(line, " ");
		if(!p_sub_arr)
			continue;

		if((val_str = strstr(line, "trapsink")))
		{
			ver = "v1";
			ip = silc_cstr_array_get_quick(p_sub_arr, 1);
			community = silc_cstr_array_get_quick(p_sub_arr, 2);
			auth = "";
			passwd = "";
		}
		else if((val_str = strstr(line, "trap2sink")))
		{
			ver = "v2c";
			ip = silc_cstr_array_get_quick(p_sub_arr, 1);
			community = silc_cstr_array_get_quick(p_sub_arr, 2);
			auth = "";
			passwd = "";
		}
		else if((val_str = strstr(line, "trapsess")))
		{
			ver = "v3";
			ip = silc_cstr_array_get_quick(p_sub_arr, 16);
			community = silc_cstr_array_get_quick(p_sub_arr, 15);
			auth = silc_cstr_array_get_quick(p_sub_arr, 5);
			passwd = silc_cstr_array_get_quick(p_sub_arr, 7);
		}
		else
		{
			silc_cstr_array_destroy(p_sub_arr);
			continue;
		}
		if (!ip || !community)
		{
			silc_cstr_array_destroy(p_sub_arr);
			goto ERROR;
		}
		p_traphost = malloc(sizeof(is_mgmtd_snmp_traphost) +
				strlen(ip) + strlen(ver) + strlen(community) + strlen(auth) + strlen(passwd) + 5);
		if(!p_traphost)
		{
			silc_cstr_array_destroy(p_sub_arr);
			goto ERROR;
		}
		p_traphost->ip = (silc_cstr)(p_traphost+1);
		p_traphost->version = p_traphost->ip + strlen(ip) + 1;
		p_traphost->community = p_traphost->version + strlen(ver) + 1;
		p_traphost->auth = p_traphost->community + strlen(community) + 1;
		p_traphost->passwd = p_traphost->auth + strlen(auth) + 1;
		strcpy(p_traphost->ip, ip);
		strcpy(p_traphost->version, ver);
		strcpy(p_traphost->community, community);
		strcpy(p_traphost->auth, auth);
		strcpy(p_traphost->passwd, passwd);
		silc_list_add_tail(&p_traphost->node, p_list);
		silc_cstr_array_destroy(p_sub_arr);
	}
	silc_cstr_array_destroy(p_arr);
	return 0;
ERROR:
	silc_cstr_array_destroy(p_arr);
	is_mgmtd_snmp_status_free_list(p_list, is_mgmtd_snmp_user);
	return IS_MGMTD_ERR_BASE_MALLOC_FAILED;
}

static is_mgmtd_snmp_info s_is_mgmtd_snmp_info;

void is_mgmtd_snmp_status_free_info()
{
	is_mgmtd_snmp_status_free_list(&s_is_mgmtd_snmp_info.community_list, is_mgmtd_snmp_community);
	is_mgmtd_snmp_status_free_list(&s_is_mgmtd_snmp_info.user_list, is_mgmtd_snmp_user);
	is_mgmtd_snmp_status_free_list(&s_is_mgmtd_snmp_info.traphost_list, is_mgmtd_snmp_traphost);
}

int is_mgmtd_snmp_status_fill_info()
{
	int ret;
	memset(&s_is_mgmtd_snmp_info, 0, sizeof(s_is_mgmtd_snmp_info));
	silc_list_init(&s_is_mgmtd_snmp_info.community_list);
	silc_list_init(&s_is_mgmtd_snmp_info.user_list);
	silc_list_init(&s_is_mgmtd_snmp_info.traphost_list);

	s_is_mgmtd_snmp_info.state = is_mgmtd_snmp_status_get_agent_state();
	if(!s_is_mgmtd_snmp_info.state)
		return 0;

	if((ret=is_mgmtd_snmp_status_get_engine_id(s_is_mgmtd_snmp_info.engine_id, 200)) != 0)
		return ret;
	if((ret=is_mgmtd_snmp_status_get_communites(&s_is_mgmtd_snmp_info.community_list)) != 0)
		return ret;
	if((ret=is_mgmtd_snmp_status_get_users(&s_is_mgmtd_snmp_info.user_list)) != 0)
		return ret;
	if((ret=is_mgmtd_snmp_status_get_trap_hosts(&s_is_mgmtd_snmp_info.traphost_list)) != 0)
		return ret;
	return 0;
}


int is_mgmtd_snmp_status_level_get_level_4(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	const silc_cstr grand_parent_name = silc_cstr_array_get_quick(p_level_arr, level - 2);
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);

	if(strcmp(grand_parent_name, "communities") == 0)
	{
		is_mgmtd_snmp_community* p_comm_info;
		silc_list_for_each_entry(p_comm_info, &s_is_mgmtd_snmp_info.community_list, node)
		{
			if(strcmp(parent_name, p_comm_info->name) == 0)
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "full-access", p_comm_info->full_access, bool, silc_false);
		}
	}
	else if(strcmp(grand_parent_name, "users") == 0)
	{
		is_mgmtd_snmp_user* p_user_info;
		silc_list_for_each_entry(p_user_info, &s_is_mgmtd_snmp_info.user_list, node)
		{
			if(strcmp(parent_name, p_user_info->name) == 0)
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "full-access", p_user_info->full_access, bool, silc_false);
		}
	}
	else if(strcmp(grand_parent_name, "trap-hosts") == 0)
	{
		is_mgmtd_snmp_traphost* p_host_info;
		silc_list_for_each_entry(p_host_info, &s_is_mgmtd_snmp_info.traphost_list, node)
		{
			if(strcmp(parent_name, p_host_info->ip) == 0)
			{
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "version", p_host_info->version, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "community", p_host_info->community, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "auth", p_host_info->auth, str, silc_false);
				silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "passwd", p_host_info->passwd, str, silc_false);
			}
		}
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_snmp_status_level_get_level_3(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	const silc_cstr parent_name = silc_cstr_array_get_quick(p_level_arr, level - 1);

	if(strcmp(parent_name, "communities") == 0)
	{
		is_mgmtd_snmp_community* p_comm_info;
		silc_list_for_each_entry(p_comm_info, &s_is_mgmtd_snmp_info.community_list, node)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, p_comm_info->name, p_comm_info->name, str, silc_true);
		}
	}
	else if(strcmp(parent_name, "users") == 0)
	{
		is_mgmtd_snmp_user* p_user_info;
		silc_list_for_each_entry(p_user_info, &s_is_mgmtd_snmp_info.user_list, node)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, p_user_info->name, p_user_info->name, str, silc_true);
		}
	}
	else if(strcmp(parent_name, "trap-hosts") == 0)
	{
		is_mgmtd_snmp_traphost* p_host_info;
		silc_list_for_each_entry(p_host_info, &s_is_mgmtd_snmp_info.traphost_list, node)
		{
			silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, p_host_info->ip, p_host_info->ip, str, silc_true);
		}
	}

	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_snmp_status_level_get_level_2(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];

	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "state", s_is_mgmtd_snmp_info.state, bool, silc_false);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "engine-id", s_is_mgmtd_snmp_info.engine_id, str, silc_false);

	//these three have children nodes
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "communities", "communities", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "users", "users", str, silc_true);
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "trap-hosts", "trap-hosts", str, silc_true);
	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}


int is_mgmtd_snmp_status_level_get_level_1(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	char add_node_tmp_str[128];
	silc_mgmtd_vnode_add_maybe(p_parent_node, p_match_node, "snmp", "snmp", str, silc_true);
	return 0;
ERR_RET:
	return IS_MGMTD_ERR_BASE_ADD_RSP_NODE;
}

int is_mgmtd_snmp_status_level_get_level(silc_mgmtd_if_node* p_parent_node, uint32_t level, silc_cstr_array* p_level_arr, const silc_cstr p_match_node, silc_bool add_node)
{
	switch(level)
	{
	case 1:
		return is_mgmtd_snmp_status_level_get_level_1(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 2:
		return is_mgmtd_snmp_status_level_get_level_2(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 3:
		return is_mgmtd_snmp_status_level_get_level_3(p_parent_node, level, p_level_arr, p_match_node, add_node);
	case 4:
		return is_mgmtd_snmp_status_level_get_level_4(p_parent_node, level, p_level_arr, p_match_node, add_node);
	default:
		SILC_ERR("Too many levels %u for snmp status query", level);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
}


int is_mgmtd_snmp_status(silc_mgmtd_if_req_type type, void* p_db_node, void* data)
{
	int ret;
	if((ret=is_mgmtd_snmp_status_fill_info()) != 0)
		return ret;
	ret = silc_mgmtd_module_vnode_retrieve(type, p_db_node, data, is_mgmtd_snmp_status_level_get_level);
	is_mgmtd_snmp_status_free_info();
	return ret;
}
