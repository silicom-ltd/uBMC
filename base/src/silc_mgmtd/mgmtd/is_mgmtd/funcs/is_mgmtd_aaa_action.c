/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_operation.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_config.h"

#define IS_ROOT_USER	"root"

int is_mgmtd_unix_check_pass(silc_cstr pass);
int is_mgmtd_unix_user_passwd_set(silc_mgmtd_node* p_node, silc_bool check_modified);
silc_bool is_mgmtd_unix_user_passwd_auth(silc_mgmtd_node* p_node, silc_cstr passwd);

int is_mgmtd_unix_update_passwd_file(silc_mgmtd_if_req_type type, silc_cstr user, silc_cstr full_name, uint32_t uid,uint32_t gid);
int is_mgmtd_unix_update_shadow_file(silc_mgmtd_if_req_type type, silc_cstr user, silc_cstr shadow);

int is_mgmtd_tacacs_static_modify();
int is_mgmtd_radius_static_modify();

int is_mgmtd_add_dft_admin()
{
	silc_mgmtd_node *p_tacacs_mu, *p_radius_mu, *p_unix_user, *p_admin_node;
	silc_mgmtd_if_req* p_sim_req = NULL;
	silc_cstr admin, shadow;
	char val_str[64], path_str[256];

	// add is_default_tacacs for tacacs login with a hidden passwd
	is_mgmtd_unix_update_passwd_file(SILC_MGMTD_IF_REQ_ADD, "is_default_tacacs", "is_default_tacacs", 9998,100);
	is_mgmtd_unix_update_shadow_file(SILC_MGMTD_IF_REQ_ADD, "is_default_tacacs",
			"$6$BntPH8fRITjITarC$YjjfIXjPw32RMQOHgmVd0Rqs6KH9bc8BvMK9OusuKQuEeQuWFReM99HB99FHl8e0d/kCvpIM9sY/kikEs65HS/");
	p_tacacs_mu = silc_mgmtd_memdb_find_node("/config/tacacs/static/mapped-user");
	if(p_tacacs_mu)
		silc_mgmtd_var_set_by_str(&p_tacacs_mu->value, "is_default_tacacs");
	// add is_default_radius for radius login with a hidden passwd
	is_mgmtd_unix_update_passwd_file(SILC_MGMTD_IF_REQ_ADD, "is_default_radius", "is_default_radius", 9999,100);
	is_mgmtd_unix_update_shadow_file(SILC_MGMTD_IF_REQ_ADD, "is_default_radius",
			"$6$mv6ciEtLpBe8Cq1u$aqr0eJLr6JsXpPT0FwoA9TUTcvaldXCoMYUmz1YaFSMrpGNRyK5Nu5XxyU1akLN0LhmP4cXYkGeJXBzkXWKu80");
	p_radius_mu = silc_mgmtd_memdb_find_node("/config/radius/static/mapped-user");
	if(p_radius_mu)
		silc_mgmtd_var_set_by_str(&p_radius_mu->value, "is_default_radius");

	// add default admin if necessary
	admin = silc_mgmtd_get_default_admin();
	if(!admin)
	{
		SILC_ERR("Default admin not set");
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
	}
	p_unix_user = silc_mgmtd_memdb_find_node("/config/unix/user");
	if(!p_unix_user)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
	if(silc_mgmtd_memdb_find_sub_value(p_unix_user, admin))
	{
		return 0;
	}

	sprintf(path_str, "/config/unix/user/%s", admin);
	p_sim_req = silc_mgmtd_if_req_create(path_str, SILC_MGMTD_IF_REQ_ADD, admin);
	if(!p_sim_req)
		goto ERROR;
	shadow = silc_mgmtd_get_default_admin_shadow();
	if(!shadow)
		goto ERROR;
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "shadow", shadow))
		goto ERROR;
	sprintf(val_str, "%d", SILC_MGMTD_IF_LEVEL_ADMIN);
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "privilege", val_str))
		goto ERROR;
	if(!silc_mgmtd_if_req_add_node_ext(p_sim_req->p_node_root, "full-name", "Default administrator"))
		goto ERROR;
	if(silc_mgmtd_op_do_add_single(p_unix_user, p_sim_req->p_node_root, NULL, silc_true) != 0)
		goto ERROR;
	p_admin_node = silc_mgmtd_memdb_find_node(path_str);
	if(!p_admin_node)
		goto ERROR;
#if 0
	if(silc_mgmtd_cfg_save_config_to_file() != 0)
		goto ERROR;
#endif
	silc_mgmtd_if_req_delete(p_sim_req);
	return 0;
ERROR:
	if(p_sim_req)
		silc_mgmtd_if_req_delete(p_sim_req);
	return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
}

int is_mgmtd_reset_admin_passwd()
{
	silc_mgmtd_node *p_admin_node;
	silc_mgmtd_node* p_shadow_node;
	silc_mgmtd_node* p_fallback_node;
	char* admin = silc_mgmtd_get_default_admin();
	char path[128];
	int ret = 0;

	SILC_LOG("Reset %s password and Tacacs/Radius fallback option", admin);

	p_fallback_node = silc_mgmtd_memdb_find_node("/config/tacacs/static/fallback");
	if (p_fallback_node)
	{
		silc_mgmtd_var_set_by_str(&p_fallback_node->value, "true");
		silc_mgmtd_var_set_by_str(&p_fallback_node->tmp_value, "true");
		is_mgmtd_tacacs_static_modify();
	}
	p_fallback_node = silc_mgmtd_memdb_find_node("/config/radius/static/fallback");
	if (p_fallback_node)
	{
		silc_mgmtd_var_set_by_str(&p_fallback_node->value, "true");
		silc_mgmtd_var_set_by_str(&p_fallback_node->tmp_value, "true");
		is_mgmtd_radius_static_modify();
	}

	sprintf(path, "/config/unix/user/%s", admin);
	p_admin_node = silc_mgmtd_memdb_find_node(path);
	if(!p_admin_node)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
	p_shadow_node = silc_mgmtd_memdb_find_sub_node(p_admin_node, "shadow");
	if(!p_shadow_node)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

	silc_mgmtd_var_set_by_str(&p_shadow_node->value, silc_mgmtd_get_default_admin_shadow());
	ret = is_mgmtd_unix_user_passwd_set(p_admin_node, silc_false);
	if (ret)
		return ret;
	silc_mgmtd_memdb_set_config_unsave();
	return 0;
}

int is_mgmtd_change_passwd(silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node *p_user_node, *p_curr_pass_node, *p_new_pass_node;
	silc_mgmtd_node *p_target_node, *p_target_pass_node;
	silc_cstr user=NULL, curr_pass=NULL, new_pass=NULL;
	char path[256];
	int ret = 0;

	silc_cstr login_user = silc_mgmtd_if_server_get_conn_entry_user(conn_entry);

	p_user_node = silc_mgmtd_memdb_find_sub_node(p_node, "user");
	p_curr_pass_node = silc_mgmtd_memdb_find_sub_node(p_node, "curr-passwd");
	p_new_pass_node = silc_mgmtd_memdb_find_sub_node(p_node, "new-passwd");
	if (!p_user_node || !p_curr_pass_node || !p_new_pass_node)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

	if(p_user_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		user = p_user_node->tmp_value.val.string_val;
	if(p_new_pass_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		new_pass = p_new_pass_node->tmp_value.val.string_val;
	if (!user || !user[0] || !new_pass || !new_pass[0])
		return IS_MGMTD_ERR_BASE_MISS_PARAM;

	if (strcmp(user, IS_ROOT_USER) == 0)
	{
		SILC_ERR("root's password is forbidden to change");
		return IS_MGMTD_ERR_AAA_USER_FORBIDDEN;
	}
	sprintf(path, "/config/unix/user/%s", user);
	p_target_node = silc_mgmtd_memdb_find_node(path);
	if (NULL == p_target_node)
	{
		SILC_ERR("User %s is not found", user);
		return IS_MGMTD_ERR_BASE_NODE_NOT_EXIST;
	}
	p_target_pass_node = silc_mgmtd_memdb_find_sub_node(p_target_node, "passwd");
	if(!p_target_pass_node)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

	if (strcmp(user, login_user) == 0)
	{
		// change his own passwd will auth old passwd
		if(p_curr_pass_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
			curr_pass = p_curr_pass_node->tmp_value.val.string_val;
		if (!curr_pass || !curr_pass[0])
			return IS_MGMTD_ERR_BASE_MISS_PARAM;
		if (!is_mgmtd_unix_user_passwd_auth(p_target_node, curr_pass))
		{
			SILC_ERR("%s's old password auth failed", user);
			return IS_MGMTD_ERR_AAA_AUTH_FAIL;
		}
	}
	else if (silc_mgmtd_if_server_get_conn_privilege(conn_entry) < SILC_MGMTD_IF_LEVEL_ADMIN)
	{
		SILC_ERR("User %s is not allowed to change %s's password", login_user, user);
		return IS_MGMTD_ERR_AAA_USER_NO_PRIVIL;
	}
	ret = is_mgmtd_unix_check_pass(new_pass);
	if (ret)
		return ret;
	silc_mgmtd_var_set_by_str(&p_target_pass_node->value, new_pass);
	ret = is_mgmtd_unix_user_passwd_set(p_target_node, silc_false);
	if (ret)
		return ret;
	SILC_LOG("%s's password is changed by %s", user, login_user);
	silc_mgmtd_memdb_set_config_unsave();
	return 0;
}

int is_mgmtd_aaa_action(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	if(type != SILC_MGMTD_IF_REQ_ACTION)
		return 0;

	if(strcmp(p_node->name, "add-default-admin") == 0)
	{
		return is_mgmtd_add_dft_admin();
	}
	else if(strcmp(p_node->name, "reset-admin-passwd") == 0)
	{
		return is_mgmtd_reset_admin_passwd();
	}
	else if(strcmp(p_node->name, "change-passwd") == 0)
	{
		return is_mgmtd_change_passwd(p_node, conn_entry);
	}
    return 0; 
}

