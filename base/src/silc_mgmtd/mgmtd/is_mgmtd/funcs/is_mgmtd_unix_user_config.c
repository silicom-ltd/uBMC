/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"

#include <crypt.h>

#define UNIX_UID_BASE	10000
#define UNIX_GID		100
#define UNIX_IS_ADMIN_GID		2000

#define PASSWD_FILE_NAME		"/etc/passwd"
#define SHADOW_FILE_NAME		"/etc/shadow"

#define UNIX_UID_BASE_PATH		"/config/unix/static/uid-base"

#define IS_ROOT_USER			"root"

#define IS_MGMTD_TACACS_STATIC_PATH		"/config/tacacs/static"
#define IS_MGMTD_RADIUS_STATIC_PATH		"/config/radius/static"

char* silc_mgmtd_get_default_admin();

void is_mgmtd_common_file_name_exchange(char* config, char* config_bak)
{
	char tmp_file[256];
	sprintf(tmp_file, "%s.tmp", config);
	rename(config, tmp_file);
	rename(config_bak, config);
	rename(tmp_file, config_bak);
}

int is_mgmtd_unix_check_user(char* user)
{
	int len = strlen(user);
	char* p;
	if (len < 2 || len > 31)
	{
		SILC_ERR("User name %s length %d invalid", user, len);
		return IS_MGMTD_ERR_AAA_USER_NAME_LEN;
	}
	if (0 == strcmp(user, IS_ROOT_USER))
	{
		SILC_ERR("User name can't be %s", user);
		return IS_MGMTD_ERR_AAA_USER_FORBIDDEN;
	}
	p = user;
	while (*p)
	{
		if ((*p < 'a' || *p > 'z') && (*p < '0' || *p > '9') && (*p != '-') && (*p != '_'))
		{
			SILC_ERR("User name %s has invalid character", user);
			return IS_MGMTD_ERR_AAA_USER_NAME_CHAR;
		}
		p++;
	}
	return 0;
}

int is_mgmtd_unix_check_pass(silc_cstr pass)
{
	int len = strlen(pass);
	int flag = 0;
	char* p;
	int ret;

	if(silc_mgmtd_memdb_get_product_info()->custom_user_password_chk_func)
	{
		ret = silc_mgmtd_memdb_get_product_info()->custom_user_password_chk_func(pass, len);
		if(ret)
			return ret;
	}
	if (len < 6 || len > 40)
	{
		SILC_ERR("Password length %d invalid", len);
		return IS_MGMTD_ERR_AAA_PASSWD_LEN_6_40;
	}
	p = pass;
	while (*p)
	{
		if ((*p >= 'a' && *p <= 'z'))
			flag |= 0x1;
		else if ((*p >= 'A' && *p <= 'Z'))
			flag |= 0x2;
		else if ((*p >= '0' && *p <= '9'))
			flag |= 0x4;
		else
			flag |= 0x8;
		p++;
	}
	if (((flag&0x1)+((flag&0x2)>>1)+((flag&0x4)>>2)+((flag&0x8)>>3)) < 3)
	{
		SILC_ERR("Password is too weak");
		return IS_MGMTD_ERR_AAA_PASSWD_TOO_WEAK;
	}
	return 0;
}

int is_mgmtd_unix_check_privil(char* user, uint32_t privil)
{
	if (privil < SILC_MGMTD_IF_LEVEL_READONLY || privil > SILC_MGMTD_IF_LEVEL_ADMIN)
	{
		SILC_ERR("%s's privil %u is invalid", user, privil);
		return IS_MGMTD_ERR_AAA_PRIVIL_INVALID;
	}
	if (0 == strcmp(user, silc_mgmtd_get_default_admin()))
	{
		if (privil < SILC_MGMTD_IF_LEVEL_ADMIN)
		{
			SILC_ERR("%s's privil is forbidden to change", user);
			return IS_MGMTD_ERR_AAA_PRIVIL_FORBIDDEN;
		}
	}
	else
	{
		/* allow to create multiple admin users
		if (privil == SILC_MGMTD_IF_LEVEL_ADMIN)
			return IS_MGMTD_ERR_AAA_PRIVIL_FORBIDDEN;
		*/
	}
	return 0;
}

int is_mgmtd_unix_check_mapped_user(char* user)
{
	char path[256];
	int ret = is_mgmtd_unix_check_user(user);
	if (ret)
		return ret;
	if (0 == strcmp(user, IS_ROOT_USER))
		return IS_MGMTD_ERR_AAA_USER_FORBIDDEN;
	// the user can be a hidden user not in mgmtd
	return 0;

	sprintf(path, "/config/unix/user/%s", user);
	if (NULL == silc_mgmtd_memdb_find_node(path))
		return IS_MGMTD_ERR_BASE_NODE_NOT_EXIST;
	return 0;
}

int is_mgmtd_unix_is_mapped_user(char* user)
{
	silc_mgmtd_node* p_node;
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/enabled");
	if (p_node->value.val.bool_val)
	{
		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/mapped-user");
		if (0 == strcmp(user, p_node->value.val.string_val))
		{
			return 1;
		}
	}
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/enabled");
	if (p_node->value.val.bool_val)
	{
		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/mapped-user");
		if (0 == strcmp(user, p_node->value.val.string_val))
		{
			return 1;
		}
	}
	return 0;
}

int is_mgmtd_unix_update_passwd_file(silc_mgmtd_if_req_type type, silc_cstr user, silc_cstr full_name, uint32_t uid, uint32_t gid)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	FILE* fp;
	FILE* fp_bak;
	char temp[512];

	fp = fopen(PASSWD_FILE_NAME, "r");
	fp_bak = fopen(PASSWD_FILE_NAME".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (0 == strncmp(user, temp, strlen(user)) && temp[strlen(user)] == ':')
		{
			// remove it if the user is in passwd file
			continue;
		}
		fprintf(fp_bak, "%s", temp);
	}
	if (SILC_MGMTD_IF_REQ_ADD == type)
	{
		fprintf(fp_bak, "%s:x:%u:%u:%s:/home/%s:/usr/bin/is_cli\n", user, uid, gid, full_name, user);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(PASSWD_FILE_NAME".bak", PASSWD_FILE_NAME);
	if (SILC_MGMTD_IF_REQ_ADD == type)
	{
		char cmd[256];
		sprintf(cmd, "mkdir -p /home/%s; chmod 770 /home/%s; chown %s /home/%s", user, user, user, user);
		system(cmd);
	}
#endif
	return 0;
}

int is_mgmtd_unix_generate_shadow(char* pass, char* salt, char* shadow)
{
	char salt_str[32];

	if (salt == NULL)
	{
		char rand_str[24];
		FILE* fp;

		fp = popen("openssl rand -base64 12", "r");
		fread(rand_str, 20, 1, fp);
		rand_str[16] = 0;
		pclose(fp);

		sprintf(salt_str, "$6$%s$", rand_str);	// SHA-512
		salt = salt_str;
	}
	strcpy(shadow, crypt(pass, salt));
	return 0;
}

int is_mgmtd_unix_update_shadow_file(silc_mgmtd_if_req_type type, silc_cstr user, silc_cstr shadow)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	FILE* fp;
	FILE* fp_bak;
	char temp[512];

	fp = fopen(SHADOW_FILE_NAME, "r");
	fp_bak = fopen(SHADOW_FILE_NAME".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (0 == strncmp(user, temp, strlen(user)) && temp[strlen(user)] == ':')
		{
			// remove it if the user is in passwd file
			continue;
		}
		fprintf(fp_bak, "%s", temp);
	}
	if (SILC_MGMTD_IF_REQ_DELETE != type)
	{
		time_t t = time(NULL);
		uint32_t d = t/86400;
		fprintf(fp_bak, "%s:%s:%u:0:99999:7:::\n", user, shadow, d>0?d:10000);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(SHADOW_FILE_NAME".bak", SHADOW_FILE_NAME);
#endif
	return 0;
}

int is_mgmtd_unix_check_req(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node* p_pass_node = NULL;
	silc_mgmtd_node* p_privil_node = NULL;
	silc_cstr user = NULL;
	silc_cstr login_user = NULL;
	int is_change_self = silc_false;
	int ret = 0;

	if (NULL == conn_entry)
	{
		// internal action, needn't check
		return 0;
	}

	if (silc_list_empty(&p_node->sub_node_list))
	{
		// always point to user node
		p_node = p_node->p_parent;
	}
	user = p_node->value.val.string_val;
	login_user = silc_mgmtd_if_server_get_conn_entry_user(conn_entry);
	if (strcmp(login_user, user) == 0)
		is_change_self = silc_true;

	if (type == SILC_MGMTD_IF_REQ_CHECK_ADD)
	{
		ret = is_mgmtd_unix_check_user(user);
		if (ret)
			return ret;
	}
	else if (type == SILC_MGMTD_IF_REQ_CHECK_DELETE)
	{
		if (is_change_self)
		{
			SILC_ERR("User %s can't delete himself", user);
			return IS_MGMTD_ERR_AAA_USER_NO_PRIVIL;
		}
		if (0 == strcmp(user, silc_mgmtd_get_default_admin()) || 0 == strcmp(user, IS_ROOT_USER))
		{
			SILC_ERR("User %s is not allowed to be deleted", user);
			return IS_MGMTD_ERR_AAA_USER_FORBIDDEN;
		}
		if (is_mgmtd_unix_is_mapped_user(user))
		{
			SILC_ERR("User %s is not allowed to be deleted", user);
			return IS_MGMTD_ERR_AAA_MAPPED_USER_INUSE;
		}
		return 0;
	}

	p_pass_node = silc_mgmtd_memdb_find_sub_node(p_node, "passwd");
	if (p_pass_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		silc_cstr passwd = p_pass_node->tmp_value.val.string_val;
		if (strlen(passwd) > 0)
		{
			if (!is_change_self)
			{
				SILC_TRACE("User %s try to modify user %s passwd!", login_user, user);
				if (silc_mgmtd_if_server_get_conn_privilege(conn_entry) < SILC_MGMTD_IF_LEVEL_ADMIN)
				{
					SILC_ERR("User %s is not allowed to change %s's password", login_user, user);
					return IS_MGMTD_ERR_AAA_USER_NO_PRIVIL;
				}
			}
			ret = is_mgmtd_unix_check_pass(passwd);
			if (ret)
				return ret;
		}
	}

	p_privil_node = silc_mgmtd_memdb_find_sub_node(p_node, "privilege");
	if (p_privil_node->tmp_value.type != SILC_MGMTD_VAR_NULL &&
			!silc_mgmtd_var_equal(&p_privil_node->value, &p_privil_node->tmp_value))
	{
		uint32_t privil;
		if (is_change_self)
		{
			SILC_ERR("User %s can't change his own privilege", user);
			return IS_MGMTD_ERR_AAA_USER_NO_PRIVIL;
		}
		privil = p_privil_node->tmp_value.val.uint32_val;
		ret = is_mgmtd_unix_check_privil(user, privil);
		if (ret)
			return ret;
	}

	return 0;
}

int is_mgmtd_unix_user_passwd_set(silc_mgmtd_node* p_node, silc_bool check_modified)
{
	silc_mgmtd_node* p_pass_node;
	silc_mgmtd_node* p_shadow_node;
	silc_cstr user = p_node->value.val.string_val;
	silc_cstr passwd = NULL;
	silc_cstr shadow = NULL;

	p_pass_node = silc_mgmtd_memdb_find_sub_node(p_node, "passwd");
	p_shadow_node = silc_mgmtd_memdb_find_sub_node(p_node, "shadow");
	if (check_modified)
	{
		if (p_pass_node->tmp_value.type == SILC_MGMTD_VAR_NULL && p_shadow_node->tmp_value.type == SILC_MGMTD_VAR_NULL)
		{
			return 0;
		}
	}

	passwd = p_pass_node->value.val.string_val;
	if (strlen(passwd) > 0)
	{
		char shadow_str[256];
		is_mgmtd_unix_generate_shadow(passwd, NULL, shadow_str);
		silc_mgmtd_var_set_by_str(&p_shadow_node->value, shadow_str);
		silc_mgmtd_var_set_by_str(&p_pass_node->value, "");	// clear pass
	}
	shadow = p_shadow_node->value.val.string_val;
	if (strlen(shadow) == 0)
	{
		// both passwd and shadow are ""
		SILC_LOG("User %s passwd is not set.\n", user);
		return 0;
	}

	return is_mgmtd_unix_update_shadow_file(SILC_MGMTD_IF_REQ_ADD, user, shadow);
}

silc_bool is_mgmtd_unix_user_passwd_auth(silc_mgmtd_node* p_node, silc_cstr passwd)
{
	silc_mgmtd_node* p_shadow_node = silc_mgmtd_memdb_find_sub_node(p_node, "shadow");
	silc_cstr curr_shadow = p_shadow_node->value.val.string_val;
	char shadow_str[256];

	is_mgmtd_unix_generate_shadow(passwd, curr_shadow, shadow_str); // use the same salt
	if (strcmp(shadow_str, curr_shadow) == 0)
		return silc_true;
	return silc_false;
}

int is_mgmtd_unix_user_add(silc_mgmtd_node* p_node, void* conn_entry)
{
	static uint32_t unix_uid_base = UNIX_UID_BASE;
	silc_mgmtd_node* p_sub_node;
	silc_cstr user = p_node->value.val.string_val;
	silc_cstr full_name = NULL;
	uint32_t uid,gid = UNIX_GID;
	silc_mgmtd_if_privilege privilege;
	int ret = 0;

	USER_OP_LOG(conn_entry, "Local user %s is added", p_node->value.val.string_val);
#if 0
	p_sub_node = silc_mgmtd_memdb_find_node(UNIX_UID_BASE_PATH);
	uid = p_sub_node->value.val.uint32_val;
	p_sub_node->value.val.uint32_val = uid + 1; // increase uid-base everytime when adding user
#else
	uid = unix_uid_base++;
#endif
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "uid");
	p_sub_node->value.val.uint32_val = uid;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "full-name");
	full_name = p_sub_node->value.val.string_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "privilege");
	privilege = p_sub_node->value.val.int32_val;
	if(privilege > SILC_MGMTD_IF_LEVEL_READONLY)
	{
		gid = UNIX_IS_ADMIN_GID;
	}

	ret = is_mgmtd_unix_update_passwd_file(SILC_MGMTD_IF_REQ_ADD, user, full_name, uid,gid);
	if (ret)
		return ret;

	ret = is_mgmtd_unix_user_passwd_set(p_node, silc_false);
	return ret;
}

int is_mgmtd_unix_user_modify(silc_mgmtd_node* p_node, void* conn_entry)
{
	if (silc_list_empty(&p_node->sub_node_list))
	{
		// always point to user node
		p_node = p_node->p_parent;
	}

	USER_OP_LOG(conn_entry, "Local user %s is modified", p_node->value.val.string_val);

	return is_mgmtd_unix_user_passwd_set(p_node, silc_true);
}

int is_mgmtd_unix_user_delete(silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_cstr user = p_node->value.val.string_val;
	int ret = 0;

	USER_OP_LOG(conn_entry, "Local user %s is deleted", user);

	ret = is_mgmtd_unix_update_passwd_file(SILC_MGMTD_IF_REQ_DELETE, user, NULL, 0,0);
	if (ret)
		return ret;

	ret = is_mgmtd_unix_update_shadow_file(SILC_MGMTD_IF_REQ_DELETE, user, NULL);
	return ret;
}

int is_mgmtd_unix_user_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;
	int ret = 0;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
			ret = is_mgmtd_unix_check_req(type, p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_ADD:
			ret = is_mgmtd_unix_user_add(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = is_mgmtd_unix_user_modify(p_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_DELETE:
			ret = is_mgmtd_unix_user_delete(p_node, conn_entry);
			break;
		default:
			break;
	}

	return ret;
#endif
    return 0;
}

