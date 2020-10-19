/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"

#define IS_MGMTD_PAM_CONFIG_FILE	"/etc/pam.d/system-auth"

#define IS_MGMTD_RADIUS_STATIC_PATH		"/config/radius/static"
#define IS_MGMTD_TACACS_STATIC_PATH		"/config/tacacs/static"

void is_mgmtd_get_mdf_node_val(const silc_cstr node_path, silc_cstr buff, int buff_len, silc_bool* modified);

int is_mgmtd_unix_check_mapped_user(char* user);
int is_mgmtd_unix_check_privil(char* user, uint32_t privil);

int is_mgmtd_radius_static_enable_apply(char* template_user, uint32_t retry, silc_bool fallback)
{
	FILE* fp;
	FILE* fp_bak;
	char temp[512];
	int flag = 0;

	fp = fopen(IS_MGMTD_PAM_CONFIG_FILE, "r");
	fp_bak = fopen(IS_MGMTD_PAM_CONFIG_FILE".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (strncmp(temp, "auth", 4) == 0)
		{
			continue;
		}
		if (!flag && strncmp(temp, "account", 7) == 0)
		{
			fprintf(fp_bak, "auth		sufficient	"
							"pam_radius_auth.so "
							"conf=%s template_user=%s retry=%d\n", "/etc/pam_radius_auth.conf", template_user, retry);
			if (fallback)
				fprintf(fp_bak, "auth		required	pam_unix.so shadow try_first_pass\n");
			flag = 1;
		}
		fprintf(fp_bak, "%s", temp);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_MGMTD_PAM_CONFIG_FILE".bak", IS_MGMTD_PAM_CONFIG_FILE);

	return 0;
}

int is_mgmtd_radius_static_disable_apply()
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	FILE* fp;
	FILE* fp_bak;
	char temp[512];

	fp = fopen(IS_MGMTD_PAM_CONFIG_FILE, "r");
	fp_bak = fopen(IS_MGMTD_PAM_CONFIG_FILE".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (strncmp(temp, "auth", 4) == 0 && strstr(temp, "pam_radius_auth.so"))
		{
			fprintf(fp_bak, "auth		required	pam_unix.so shadow try_first_pass\n");
			continue;
		}
		fprintf(fp_bak, "%s", temp);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_MGMTD_PAM_CONFIG_FILE".bak", IS_MGMTD_PAM_CONFIG_FILE);
#endif
	return 0;
}

int is_mgmtd_radius_static_modify(void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[512];
	silc_bool enabled, fallback;
	char template_user[64];
	uint32_t retry;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_RADIUS_STATIC_PATH"/enabled", buff, 512, &modified);
	enabled = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_RADIUS_STATIC_PATH"/fallback", buff, 512, &modified);
	fallback = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_RADIUS_STATIC_PATH"/mapped-user", buff, 512, &modified);
	strcpy(template_user, buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_RADIUS_STATIC_PATH"/retry", buff, 512, &modified);
	retry = atoi(buff);

	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "Radius setting is modified: enabled %d fallback %d retry %u", enabled, fallback, retry);

	if (!enabled)
	{
		return is_mgmtd_radius_static_disable_apply();
	}

	return is_mgmtd_radius_static_enable_apply(template_user, retry, fallback);
}

int is_mgmtd_radius_check_req(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = NULL;
	int ret = 0;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/enabled");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL && p_node->tmp_value.val.bool_val)
	{
		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/enabled");
		if (p_node->value.val.bool_val)
		{
			SILC_ERR("Radius and TACACS+ can't be enabled together");
			return IS_MGMTD_ERR_AAA_RADIUS_TAC_CONFLICT;
		}
	}

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/privilege");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		ret = is_mgmtd_unix_check_privil("", p_node->tmp_value.val.uint32_val);
		if (ret)
			return ret;
	}
#if 0
	char* val = NULL;
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/mapped-user");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		val = p_node->tmp_value.val.string_val;
	else
		val = p_node->value.val.string_val;
	ret = is_mgmtd_unix_check_mapped_user(val);
	if (ret)
		return ret;
#endif
	return 0;
}

int is_mgmtd_radius_static_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	int ret = 0;

	switch(type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
		case SILC_MGMTD_IF_REQ_ADD:
		case SILC_MGMTD_IF_REQ_DELETE:
			break;
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			ret = is_mgmtd_radius_check_req(type, p_db_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = is_mgmtd_radius_static_modify(conn_entry);
			break;
		default:
			break;
	}

    return ret;
#else
    return 0;
#endif
}

