/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_snmp_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("snmp", conn_entry);
}

void is_mgmtd_snmp_log_usr_op(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	if (strcmp(p_node->name, "agent") == 0 || strcmp(p_node->p_parent->name, "agent") == 0)
	{
		silc_mgmtd_node* p_state = silc_mgmtd_memdb_find_node("/config/snmp/agent/state");
		if (SILC_MGMTD_IF_REQ_MODIFY == type && silc_mgmtd_node_configured(p_state))
		{
			SILC_LOG("SNMP state is %s", p_state->value.val.bool_val? "enabled":"disabled");
		}
	}
	else if (strcmp(p_node->name, "name") == 0 && strcmp(p_node->p_parent->name, "communities") == 0)
	{
		if (SILC_MGMTD_IF_REQ_ADD == type)
		{
			SILC_LOG("SNMP v1/v2c community %s is added", p_node->value.val.string_val);
		}
		else if (SILC_MGMTD_IF_REQ_MODIFY == type)
		{
			SILC_LOG("SNMP v1/v2c community %s is modified", p_node->value.val.string_val);
		}
		else
			SILC_LOG("SNMP v1/v2c community %s is deleted", p_node->value.val.string_val);
	}
	else if (strcmp(p_node->name, "name") == 0 && strcmp(p_node->p_parent->name, "users") == 0)
	{
		if (SILC_MGMTD_IF_REQ_ADD == type)
		{
			SILC_LOG("SNMP v3 user %s is added", p_node->value.val.string_val);
		}
		else if (SILC_MGMTD_IF_REQ_MODIFY == type)
		{
			SILC_LOG("SNMP v3 user %s is modified", p_node->value.val.string_val);
		}
		else
			SILC_LOG("SNMP v3 user %s is deleted", p_node->value.val.string_val);
	}
	else if (strcmp(p_node->name, "ip") == 0 && strcmp(p_node->p_parent->name, "trap-hosts") == 0)
	{
		char buff[64];
		silc_mgmtd_var_to_str(&p_node->value, buff, 64);
		if (SILC_MGMTD_IF_REQ_ADD == type)
		{
			SILC_LOG("SNMP trap host %s is added", buff);
		}
		else if (SILC_MGMTD_IF_REQ_MODIFY == type)
		{
			SILC_LOG("SNMP trap host %s is modified", buff);
		}
		else
			SILC_LOG("SNMP trap host %s is deleted", buff);
	}
}

int is_mgmtd_snmp_check_name(char* p_name)
{
	int len = strlen(p_name);
	char* p;
	if(len == 0 || len > 31)
	{
		return -1;
	}
	p = p_name;
	while (*p)
	{
		if ((*p < 'a' || *p > 'z') && (*p < 'A' || *p > 'Z') && (*p < '0' || *p > '9') && (*p != '.') && (*p != '_'))
		{
			return -1;
		}
		p++;
	}
	return 0;
}

int is_mgmtd_snmp_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	int ret = 0;
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	if (silc_list_empty(&p_node->sub_node_list))
	{
		p_node = p_node->p_parent;
	}
	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
			if(strcmp(p_node->p_parent->name, "communities") == 0)
			{
				if(is_mgmtd_snmp_check_name(p_node->value.val.string_val))
				{
					SILC_ERR("SNMP community %s is invalid", p_node->value.val.string_val);
					return IS_MGMTD_ERR_SNMP_COMMUNITY_NAME_INVALID;
				}
			}
			else if(strcmp(p_node->p_parent->name, "users") == 0)
			{
				if(is_mgmtd_snmp_check_name(p_node->value.val.string_val))
				{
					SILC_ERR("SNMP user %s is invalid", p_node->value.val.string_val);
					return IS_MGMTD_ERR_SNMP_USER_NAME_INVALID;
				}
			}
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			if(strcmp(p_node->p_parent->name, "users") == 0 || strcmp(p_node->p_parent->name, "trap-hosts") == 0)
			{
				silc_mgmtd_node* p_auth = NULL;
				silc_mgmtd_node* p_passwd = NULL;
				p_auth = silc_mgmtd_memdb_find_sub_node(p_node, "auth");
				if(p_auth && p_auth->tmp_value.type != SILC_MGMTD_VAR_NULL && p_auth->tmp_value.val.string_val[0] &&
						(strcmp(p_auth->tmp_value.val.string_val, "md5") != 0 && strcmp(p_auth->tmp_value.val.string_val, "sha") != 0))
				{
					SILC_ERR("SNMP auth should be md5 or sha");
					return IS_MGMTD_ERR_SNMP_AUTH_INVALID;
				}
				p_passwd = silc_mgmtd_memdb_find_sub_node(p_node, "password-tmp");
				if(p_passwd && p_passwd->tmp_value.type != SILC_MGMTD_VAR_NULL &&
						strlen(p_passwd->tmp_value.val.string_val) > 0 &&
						strlen(p_passwd->tmp_value.val.string_val) < 8)
				{
					silc_mgmtd_node* p_ver = silc_mgmtd_memdb_find_sub_node(p_node, "version");
					if(p_ver)
					{
						silc_cstr ver = p_ver->value.val.string_val;
						if(p_ver->tmp_value.type != SILC_MGMTD_VAR_NULL)
						{
							ver = p_ver->tmp_value.val.string_val;
						}
						if(strcmp(ver, "v3") != 0)
						{
							//v1 and v2c ignore password
							break;
						}
					}
					SILC_ERR("SNMP password is too short");
					return IS_MGMTD_ERR_SNMP_PASSWD_INVALID;
				}
			}
			break;
		case SILC_MGMTD_IF_REQ_ADD:
		case SILC_MGMTD_IF_REQ_MODIFY:
		case SILC_MGMTD_IF_REQ_DELETE:
			if(conn_entry)
			{
				is_mgmtd_snmp_log_usr_op(type, p_node);
			}
			if(type == SILC_MGMTD_IF_REQ_ADD || type == SILC_MGMTD_IF_REQ_MODIFY)
			{
				if(strcmp(p_node->p_parent->name, "users") == 0 || strcmp(p_node->p_parent->name, "trap-hosts") == 0)
				{
					silc_mgmtd_node* p_passwd_tmp = silc_mgmtd_memdb_find_sub_node(p_node, "password-tmp");
					if(strlen(p_passwd_tmp->value.val.string_val) > 0)
					{
						//password is changed, set it to password node and clear password-tmp node
						silc_mgmtd_node* p_passwd = silc_mgmtd_memdb_find_sub_node(p_node, "password");
						silc_cstr new_pass = p_passwd_tmp->value.val.string_val;
						silc_mgmtd_var_set_by_str(&p_passwd->value, new_pass);
						silc_mgmtd_var_set_by_str(&p_passwd_tmp->value, "");	//clear password-tmp so it can't be seen when query
					}
				}
			}
			is_mgmtd_snmp_update_ts(conn_entry);
			silc_mgmtd_if_server_notify("/notify/snmp/config_changed");
			break;
		default:
			break;
	}

    return ret;
}

