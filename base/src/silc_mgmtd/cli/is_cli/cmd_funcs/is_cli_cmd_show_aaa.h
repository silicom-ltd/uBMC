#ifndef _SILC_CLI_CMD_SHOW_AAA_H_
#define _SILC_CLI_CMD_SHOW_AAA_H_

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

static inline void is_cli_show_radius_configured_static(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-12s: %-8s";
	silc_cstr titles[] = { "Name", "Configured" };
	silc_cstr names[] = { "State", "LocalLogin", "Privilege", "Retry" };
	silc_cstr nodes[] = { "enabled", "fallback", "privilege", "retry" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr trans_privi[] = { "1", "readonly", "2", "normal", "3", "admin" };
	silc_cstr* trans[] = { trans_state, trans_state, trans_privi, NULL};
	int trans_num[] = { 2, 2, 3, 0 };

	silc_cli_l1tree_list_display(p_node,
			"RADIUS Basic Configuration:", titles,
			nodes, names, format, trans, trans_num, 4, NULL);
}

static inline void is_cli_show_radius_configured_servers(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-16s", "%-8s", "%-8s", "%-s" };
	silc_cstr names[] = { "ID", "IP", "Port", "Timeout", "Secret" };
	silc_cstr nodes[] = { NULL, "server-ip", "server-port", "timeout", "secret" };
	silc_cstr* trans[] = { NULL, NULL, NULL, NULL, NULL };
	int trans_num[] = { 0, 0, 0, 0, 0 };

	silc_cli_l2tree_display(p_node,
			"RADIUS Server Configuration:", nodes, names, formats, trans, trans_num, 5);
}

static inline void is_cli_show_tacacs_configured_static(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-12s: %-8s";
	silc_cstr titles[] = { "Name", "Configured" };
	silc_cstr names[] = { "State", "LocalLogin", "Timeout", "ServiceTag" };
	silc_cstr nodes[] = { "enabled", "fallback", "timeout", "service" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr* trans[] = { trans_state, trans_state, NULL, NULL};
	int trans_num[] = { 2, 2, 0, 0 };

	silc_cli_l1tree_list_display(p_node,
			"TACACS+ Basic Configuration:", titles,
			nodes, names, format, trans, trans_num, 4, NULL);
}

static inline void is_cli_show_tacacs_configured_servers(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-4s", "%-16s", "%-8s", "%-s" };
	silc_cstr names[] = { "ID", "IP", "Port", "Secret" };
	silc_cstr nodes[] = { NULL, "server-ip", "server-port", "secret" };
	silc_cstr* trans[] = { NULL, NULL, NULL, NULL };
	int trans_num[] = { 0, 0, 0, 0 };

	silc_cli_l2tree_display(p_node,
			"TACACS+ Server Configuration:", nodes, names, formats, trans, trans_num, 4);
}

static inline void is_cli_show_unix_configured_users(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-12s", "%-12s", "%-20s" };
	silc_cstr names[] = { "User", "Privilege", "FullName" };
	silc_cstr nodes[] = { NULL, "privilege", "full-name" };
	silc_cstr trans_privi[] = { "1", "readonly", "2", "normal", "3", "admin" };
	silc_cstr* trans[] = { NULL, trans_privi, NULL };
	int trans_num[] = { 0, 3, 0 };

	silc_cli_l2tree_display(p_node,
			"Local Users:", nodes, names, formats, trans, trans_num, 3);
}

static inline void is_cli_show_radius_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "static") == 0)
		{
			is_cli_show_radius_configured_static(p_node);
		}
		else if(strcmp(p_node->name, "server") == 0)
			is_cli_show_radius_configured_servers(p_node);
	}
}

static inline void is_cli_show_tacacs_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "static") == 0)
			is_cli_show_tacacs_configured_static(p_node);
		else if(strcmp(p_node->name, "server") == 0)
			is_cli_show_tacacs_configured_servers(p_node);
	}
}

static inline void is_cli_show_unix_user_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	is_cli_show_unix_configured_users(p_rsp->p_node_root);
}


#endif
