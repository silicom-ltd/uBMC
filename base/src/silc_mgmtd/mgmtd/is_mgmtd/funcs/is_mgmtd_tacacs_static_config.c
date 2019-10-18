/* this file is created by silc_mgmtd_inst_func_source_gen.py */

#include "is_mgmtd_func_def.h"

#define IS_MGMTD_PAM_CONFIG_FILE "/etc/pam.d/system-auth"

#define IS_MGMTD_TACACS_STATIC_PATH		"/config/tacacs/static"
#define IS_MGMTD_RADIUS_STATIC_PATH		"/config/radius/static"

#define MAX_SERVER_NAME_LEN		128
#define MAX_SERVER_KEY_LEN		128

void is_mgmtd_get_mdf_node_val(const silc_cstr node_path, silc_cstr buff, int buff_len, silc_bool* modified);

int is_mgmtd_unix_check_mapped_user(char* user);

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_aaa_tacacs_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("aaa_tacacs", conn_entry);
}

int is_mgmtd_tacacs_write_server(FILE* fp, silc_mgmtd_node* server_node)
{
	silc_mgmtd_node* p_node;
	char *ip, *key;
	uint32_t port;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-ip");
	ip = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "server-port");
	port = p_node->value.val.uint32_val;

	p_node = silc_mgmtd_memdb_find_sub_node(server_node, "secret");
	key = p_node->value.val.string_val;

	fprintf(fp, " secret=%s server=[%s]:%d",
			key, ip, port);
	return 0;
}

int is_mgmtd_tacacs_static_write_config(FILE* fp, silc_mgmtd_node* new_node, char* node_del)
{
	silc_mgmtd_node* p_node;
	silc_mgmtd_node* p_sub_node;
	char *template_user;
	char *login = "login";
	char* service = "silc-system";
	uint32_t timeout;
	silc_bool fallback;
	char path[256];

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/mapped-user");
	template_user = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/timeout");
	timeout = p_node->value.val.uint32_val;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/service");
	service = p_node->value.val.string_val;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/fallback");
	fallback = p_node->value.val.bool_val;

#ifdef IS_TACACS_LOGIN_CONFIG
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/login");
	login = p_node->value.val.string_val;
#endif

	fprintf(fp, "auth		sufficient	"
					"pam_tacplus.so "
					"template_user=%s timeout=%u login=%s try_first_pass", template_user, timeout, login);

	p_node = silc_mgmtd_memdb_find_node("/config/tacacs/server");
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if(p_sub_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		if (node_del)
		{
			silc_mgmtd_memdb_get_node_path(p_sub_node, path);
			if (strcmp(path, node_del) == 0)
				continue;
		}
		if (p_sub_node->type == MEMDB_NODE_TYPE_DYNAMIC)
		{
			is_mgmtd_tacacs_write_server(fp, p_sub_node);
		}
	}
	fprintf(fp, "\n");

	if (fallback)
		fprintf(fp, "auth		required	pam_unix.so shadow try_first_pass\n");

	fprintf(fp, "account	sufficient	pam_tacplus.so service=%s\n", service);

	if (fallback)
		fprintf(fp, "account	required	pam_unix.so\n");

	return 0;
}

int is_mgmtd_tacacs_static_enable_apply(silc_mgmtd_node* new_node, char* node_del, void* conn_entry)
{
	FILE *fp, *fp_bak;
	char temp[512];
	int flag = 0;

	silc_mgmtd_node* p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/enabled");
	silc_bool enabled = p_node->value.val.bool_val;

	is_mgmtd_aaa_tacacs_update_ts(conn_entry);
	if (!enabled)
		return 0;

	fp = fopen(IS_MGMTD_PAM_CONFIG_FILE, "r");
	fp_bak = fopen(IS_MGMTD_PAM_CONFIG_FILE".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (strncmp(temp, "auth", 4) == 0 || strncmp(temp, "account", 7) == 0)
		{
			// remove auth and account, which will be configured in is_mgmtd_tacacs_static_write_config
			continue;
		}
		if (!flag && strncmp(temp, "password", 8) == 0)
		{
			is_mgmtd_tacacs_static_write_config(fp_bak, new_node, node_del);
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

int is_mgmtd_tacacs_static_disable_apply(void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	FILE *fp, *fp_bak;
	char temp[512];

	fp = fopen(IS_MGMTD_PAM_CONFIG_FILE, "r");
	fp_bak = fopen(IS_MGMTD_PAM_CONFIG_FILE".bak", "w");
	while (fgets(temp, 512, fp))
	{
		if (strncmp(temp, "auth", 4) == 0 && strstr(temp, "pam_tacplus.so"))
		{
			fprintf(fp_bak, "auth		required	pam_unix.so shadow try_first_pass\n");
			continue;
		}
		if (strncmp(temp, "account", 7) == 0 && strstr(temp, "pam_tacplus.so"))
		{
			fprintf(fp_bak, "account	required	pam_unix.so\n");
			continue;
		}
		fprintf(fp_bak, "%s", temp);
	}
	fflush(fp_bak);
	fclose(fp_bak);
	fclose(fp);

	rename(IS_MGMTD_PAM_CONFIG_FILE".bak", IS_MGMTD_PAM_CONFIG_FILE);
#endif
	is_mgmtd_aaa_tacacs_update_ts(conn_entry);
	return 0;
}

int is_mgmtd_tacacs_static_modify(void* conn_entry)
{
	silc_bool modified = silc_false;
	char buff[512];
	silc_bool enabled, fallback;
	uint32_t timeout;

	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/enabled", buff, 512, &modified);
	enabled = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/fallback", buff, 512, &modified);
	fallback = (strcmp(buff, "true") == 0 ? silc_true:silc_false);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/timeout", buff, 512, &modified);
	timeout = atoi(buff);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/login", buff, 512, &modified);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/mapped-user", buff, 512, &modified);
	is_mgmtd_get_mdf_node_val(IS_MGMTD_TACACS_STATIC_PATH"/service", buff, 512, &modified);

	if (!modified)
	{
		return 0;
	}
	USER_OP_LOG(conn_entry, "TACACS+ setting is modified: enabled %d fallback %d timeout %u service-tag %s", enabled, fallback, timeout, buff);

	if (!enabled)
	{
		return is_mgmtd_tacacs_static_disable_apply(conn_entry);
	}

	return is_mgmtd_tacacs_static_enable_apply(NULL, NULL, conn_entry);
}

#define TACACS_LOGIN_NUM	3
int is_mgmtd_tacacs_check_login(char* login)
{
	char* logintype[TACACS_LOGIN_NUM] = {"pap", "chap", "login"};
	int i;
	for (i=0; i<TACACS_LOGIN_NUM; i++)
	{
		if (0 == strcmp(login, logintype[i]))
			break;
	}
	if (i == TACACS_LOGIN_NUM)
	{
		SILC_ERR("TACACS+ login type is invalid: %s", login);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	return 0;
}

#define TACACS_SERVICE_NUM	8
int is_mgmtd_tacacs_check_service(char* service)
{
	char* reserved_services[TACACS_SERVICE_NUM] = {"slip", "ppp", "arap", "shell", "tty-daemon", "connection", "system", "firewall"};
	int i;
	for (i=0; i<TACACS_SERVICE_NUM; i++)
	{
		if (0 == strcmp(service, reserved_services[i]))
		{
			SILC_ERR("TACACS+ service tag can't be one of the system reserved services");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	return 0;
}

int is_mgmtd_tacacs_check_req(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = NULL;
	char* val = NULL;
	int ret = 0;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/enabled");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL && p_node->tmp_value.val.bool_val)
	{
		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_RADIUS_STATIC_PATH"/enabled");
		if (p_node->value.val.bool_val)
		{
			SILC_ERR("Radius and TACACS+ can't be enabled together");
			return IS_MGMTD_ERR_AAA_RADIUS_TAC_CONFLICT;
		}
	}

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/timeout");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		uint32_t to = p_node->tmp_value.val.uint32_val;
		if (to < 1 || to > 60)
		{
			SILC_ERR("TACACS+ timeout should be 1-60");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/service");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		val = p_node->tmp_value.val.string_val;
	else
		val = p_node->value.val.string_val;
	ret = is_mgmtd_tacacs_check_service(val);
	if (ret)
		return ret;

	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/mapped-user");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		val = p_node->tmp_value.val.string_val;
	else
		val = p_node->value.val.string_val;
	ret = is_mgmtd_unix_check_mapped_user(val);
	if (ret)
		return ret;
#ifdef IS_TACACS_LOGIN_CONFIG
	p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_TACACS_STATIC_PATH"/login");
	if (p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		val = p_node->tmp_value.val.string_val;
	else
		val = p_node->value.val.string_val;
	ret = is_mgmtd_tacacs_check_login(val);
	if (ret)
		return ret;
#endif

	return 0;
}

int is_mgmtd_tacacs_static_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
#ifndef SILC_MGMTD_LOCAL_DEBUG
	int ret = 0;

	switch (type)
	{
		case SILC_MGMTD_IF_REQ_CHECK_ADD:
		case SILC_MGMTD_IF_REQ_CHECK_DELETE:
		case SILC_MGMTD_IF_REQ_ADD:
		case SILC_MGMTD_IF_REQ_DELETE:
			break;
		case SILC_MGMTD_IF_REQ_CHECK_MODIFY:
			ret = is_mgmtd_tacacs_check_req(type, p_db_node, conn_entry);
			break;
		case SILC_MGMTD_IF_REQ_MODIFY:
			ret = is_mgmtd_tacacs_static_modify(conn_entry);
			break;
		default:
			break;
	}

	return ret;
#else
	return 0;
#endif
}
