#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <pthread.h>
#include <silc_mgmtd_interface.h>

static int g_md_cli_init_flag = 0;
static void __attribute__((constructor)) __md_cli_init(void)
{
	g_md_cli_init_flag = silc_mgmtd_if_client_init("127.0.0.1", 2626, 600, 0);
}

static int md_cli_init(void)
{
	return g_md_cli_init_flag;
}

static void __attribute__((destructor)) md_cli_shutdown(void)
{
    silc_mgmtd_if_client_shutdown();
}

static void md_iter_if_node(silc_mgmtd_if_node *p_if_node, lua_State* L)
{
	lua_pushstring(L, p_if_node->name);
	if (silc_list_empty(&p_if_node->sub_node_list))
	{
		// a parent node without child node, map it to "null"
		// or a child node, map it to a string
		lua_pushstring(L, p_if_node->val_str);
	}
	else
	{
		// a parent node with child node, map it to a table
		silc_mgmtd_if_node *p_node;
		lua_newtable(L);
		silc_list_for_each_entry(p_node, &p_if_node->sub_node_list, node)
		{
			md_iter_if_node(p_node, L);
		}
	}
	// insert this k-v pair to table
	lua_settable(L, -3);
}

static int md_cli_exec_core(lua_State* L, silc_mgmtd_if_req_type req_type)
{
	silc_mgmtd_msg* p_msg = NULL;
	silc_mgmtd_if_req* req = NULL;
	silc_mgmtd_if_rsp* rsp = NULL;
	const char* path;
	const char* rootval;
	char retcode[256];
	//int ret = 1;

	if (0 != md_cli_init())
	{
		strcpy(retcode, "Init mgmtd cli error");
		goto RET_ERROR;
	}

	path = luaL_checkstring(L,1);
	if (lua_isstring(L, 2))
	{
		rootval = luaL_checkstring(L,2);
	}
	else
	{
		rootval = silc_mgmtd_if_path_get_last_name(path);
	}
	req = silc_mgmtd_if_req_create(path, req_type, rootval);
	if (!req)
	{
		strcpy(retcode, "Create req error");
		goto RET_ERROR;
	}

	// req arg = {xx=aa, yy=bb, ...}
	if (lua_istable(L, 2))
	{
		const char* name;
		const char* val;
		lua_pushnil(L);
		while (lua_next(L, 2) != 0)
		{
			if (!lua_istable(L, -1))	// ignore table type
			{
				name = lua_tostring(L, -2);
				val = lua_tostring(L, -1);
				silc_mgmtd_if_req_add_node(req, name, val);
			}
			lua_pop(L, 1);
		}
	}

	p_msg = silc_mgmtd_if_client_send_req_sync(req, 30);
	if (!p_msg)
	{
		strcpy(retcode, "Failed to send request");
		goto RET_ERROR;
	}

	silc_list_for_each_entry(rsp, &p_msg->if_recv_items, node)
	{
		strcpy(retcode, rsp->return_str);
		if (strcmp(retcode, "OK"))
		{
			goto RET_ERROR;
		}
		lua_pushstring(L, retcode);
		if (silc_list_empty(&rsp->p_node_root->sub_node_list))
		{
			// no sub data for current operation, just return node value
			lua_pushstring(L, rsp->p_node_root->val_str);
		}
		else
		{
			silc_mgmtd_if_node *p_node;
			lua_newtable(L);
			silc_list_for_each_entry(p_node, &rsp->p_node_root->sub_node_list, node)
			{
				md_iter_if_node(p_node, L);
			}
			// now we have 2 ret: retcode, rectbl
			// ret = 2;
		}
		// only support one rsp at present
		break;
	}
	silc_mgmtd_if_client_free_msg(p_msg);
	return 2;

RET_ERROR:
	lua_pushstring(L, retcode);
	return 1;
}

static int md_cli_add(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_ADD);
}

static int md_cli_modify(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_MODIFY);
}

static int md_cli_delete(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_DELETE);
}

static int md_cli_query(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_QUERY);
}

static int md_cli_query_sub(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_QUERY_SUB);
}

static int md_cli_query_child(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_QUERY_CHILD);
}

static int md_cli_action(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_ACTION);
}

static int md_cli_notify(lua_State* L)
{
    return md_cli_exec_core(L, SILC_MGMTD_IF_REQ_NOTIFY);
}

static int md_cli_set_login(lua_State* L)
{
	const char *user=NULL, *proto=NULL;
	int privil;
	char privil_str[8];
	if (0 != md_cli_init())
	{
		return 0;
	}
	user = luaL_checkstring(L,1);
	privil = luaL_checkint(L,2);
	sprintf(privil_str, "%d", privil);
	proto = luaL_checkstring(L,3);
	silc_mgmtd_if_client_set_login_info_ex(SILC_MGMTD_IF_CLIENT_WEB, user, NULL, NULL, privil_str, proto);
	return 0;
}

static luaL_Reg R[] = {
    {"add", md_cli_add},
    {"modify", md_cli_modify},
    {"delete", md_cli_delete},
    {"query", md_cli_query},
    {"query_child", md_cli_query_child},
    {"query_sub", md_cli_query_sub},
    {"action", md_cli_action},
    {"notify", md_cli_notify},
    {"set_login", md_cli_set_login},
    {NULL, NULL}
};

int luaopen_mgmtdclient(lua_State* L)
{
    const char* libName = "mgmtdclient";
    luaL_register(L, libName, R);
    return 1;
}

