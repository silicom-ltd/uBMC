
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <security/pam_appl.h>

void silc_mgmtd_if_trans_privil(char* privil);

static int luci_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
	lua_State *L = appdata_ptr;
	char* pswd = luaL_checkstring(L,2);

	*resp = (struct pam_response *)malloc(sizeof(struct pam_response));
	if (*resp == NULL)
		return PAM_CONV_ERR;
	(*resp)[0].resp = strdup(pswd);
	(*resp)[0].resp_retcode = 0;

	return PAM_SUCCESS;
}

static int luci_pam_auth(lua_State* L)
{
	char* user = luaL_checkstring(L,1);
	struct pam_conv pam_conversation = {luci_conversation, L};
	pam_handle_t *pamh = NULL;
	char retcode[32], retuser[64], privil[16], proto[16];
	char *pam_user, *pam_env;
	int err;

	err = pam_start("silc-web", user, &pam_conversation, &pamh);
	if (err != PAM_SUCCESS)
	{
		return luaL_error(L, "pam_start error %d", err);
	}

	err = pam_authenticate(pamh, 0);
	if (err != PAM_SUCCESS)
	{
		strcpy(retcode, "Error");
	}
	else
	{
		strcpy(retcode, "OK");
	}
	pam_get_item(pamh, PAM_USER, (const void**)&pam_user);
	strcpy(retuser, pam_user);

	pam_env = pam_getenv(pamh, "AUTH_PROTOCOL");
	if (pam_env)
		strcpy(proto, pam_env);
	else
		strcpy(proto, "local");

	if (0 == strcmp(proto, "tacacs"))
	{
		pam_acct_mgmt(pamh, 0);
	}

	pam_env = pam_getenv(pamh, "USER_PRIVILEGE");
	if (pam_env)
	{
		strcpy(privil, pam_env);
		silc_mgmtd_if_trans_privil(privil);
	}
	else
		strcpy(privil, "null");

	err = pam_end(pamh, err);
	if (err != PAM_SUCCESS)
	{
		return luaL_error(L, "pam_end error %d", err);
	}

	lua_pushstring(L, retcode);
	lua_pushstring(L, retuser);
	lua_pushstring(L, privil);
	lua_pushstring(L, proto);
	return 4;
}

static luaL_Reg R[] = {
    {"authenticate", luci_pam_auth},
    {NULL, NULL}
};

int luaopen_pam(lua_State* L)
{
    const char* libName = "pam";
    luaL_register(L, libName, R);
    return 1;
}
