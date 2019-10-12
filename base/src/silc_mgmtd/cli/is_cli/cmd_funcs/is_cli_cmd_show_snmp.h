#ifndef _SILC_CLI_CMD_SHOW_SNMP_H_
#define _SILC_CLI_CMD_SHOW_SNMP_H_

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"
#include "silc_cli_error.h"

/*
 * configuration
 */
static inline void is_cli_show_snmp_configured_state(silc_mgmtd_if_node* p_node)
{
	silc_cstr trans[] = { "false", "disable", "true", "enable"};

	silc_cli_simple_node_display(p_node, "SNMP Server Configured State:", trans, 2);
}

static inline void is_cli_show_snmp_configured_communities(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-12s", "%-12s", "%-8s" };
	silc_cstr names[] = { "Community", "Access", "State" };
	silc_cstr nodes[] = { NULL, "full-access", "state" };
	silc_cstr trans_access[] = { "false", "readonly", "true", "readwrite" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr* trans[] = { NULL, trans_access, trans_state };
	int trans_num[] = { 0, 2, 2 };

	silc_cli_l2tree_display(p_node,
			"SNMP v1 or v2c Community Configuration:", nodes, names, formats, trans, trans_num, 3);
}

static inline void is_cli_show_snmp_configured_users(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-12s", "%-12s", "%-12s", "%-12s", "%-8s" };
	silc_cstr names[] = { "User", "Access", "Password", "Auth/Priv", "State" };
	silc_cstr nodes[] = { NULL, "full-access", "password", "auth", "state" };
	silc_cstr trans_access[] = { "false", "readonly", "true", "readwrite" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr trans_auth[] = { "sha", "sha/aes", "md5", "md5/des"};
	silc_cstr* trans[] = { NULL, trans_access, NULL, trans_auth, trans_state };
	int trans_num[] = { 0, 2, 0, 2, 2 };

	silc_cli_l2tree_display(p_node,
			"SNMP v3 User Configuration:", nodes, names, formats, trans, trans_num, 5);
}

static inline void is_cli_show_snmp_configured_trap_hosts(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-4s", "%-12s", "%-8s", "%-12s", "%-12s" };
	silc_cstr names[] = { "HostIP", "Ver", "Community", "State", "Password", "Auth/Priv" };
	silc_cstr nodes[] = { NULL, "version", "community", "state", "password", "auth" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr trans_auth[] = { "sha", "sha/aes", "md5", "md5/des"};
	silc_cstr* trans[] = { NULL, NULL, NULL, trans_state, NULL, trans_auth};
	int trans_num[] = { 0, 0, 0, 2, 0, 2 };

	silc_cli_l2tree_display(p_node,
			"SNMP Trap Host Configuration:", nodes, names, formats, trans, trans_num, 6);
}

static inline void is_cli_show_snmp_configured_trap_ctl(silc_mgmtd_if_node* p_node)
{
	if(silc_cli_get_product_info()->show_snmp_configure_trap_ctrl_func)
	{
		silc_cli_get_product_info()->show_snmp_configure_trap_ctrl_func(p_node);
	}
}

static inline void is_cli_show_snmp_configured_threshold_fan(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-12s: %-8s";
	silc_cstr titles[] = { "Type", "Threshold" };
	silc_cstr names[] = { "Min", "Max" };
	silc_cstr nodes[] = { "min", "max" };

	silc_cli_l1tree_list_display(p_node,
			"SNMP Fan Warning Trap Threshold Configuration:", titles,
			nodes, names, format, NULL, NULL, 2, NULL);
}

static inline void is_cli_show_snmp_configured_threshold_sensor(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-12s: %-8s";
	silc_cstr titles[] = { "Sensor", "Max-Threshold" };
	silc_cstr names[] = { "BCM", "Switch", "Module", "Port" };
	silc_cstr nodes[] = { "bcm-max", "switch-max", "module-max", "port-max" };

	silc_cli_l1tree_list_display(p_node,
			"SNMP Sensor Warning Trap Threshold Configuration:", titles,
			nodes, names, format, NULL, NULL, 4, NULL);
}

static inline void is_cli_show_snmp_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node, *p_sub_node;
	int snmpv3_only = 0;
	int threshold_enabled = 1;

	if(silc_cli_get_product_info()->snmp_v3_only_func)
	{
		snmpv3_only = silc_cli_get_product_info()->snmp_v3_only_func();
	}
	if(silc_cli_get_product_info()->snmp_threshold_enabled_func)
	{
		threshold_enabled = silc_cli_get_product_info()->snmp_threshold_enabled_func();
	}

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "agent") == 0)
		{
			silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
			{
				if(strcmp(p_sub_node->name, "state") == 0)
					is_cli_show_snmp_configured_state(p_sub_node);
				else if(strcmp(p_sub_node->name, "communities") == 0 && !snmpv3_only)
					is_cli_show_snmp_configured_communities(p_sub_node);
				else if(strcmp(p_sub_node->name, "users") == 0)
					is_cli_show_snmp_configured_users(p_sub_node);
				else if(strcmp(p_sub_node->name, "trap-hosts") == 0)
					is_cli_show_snmp_configured_trap_hosts(p_sub_node);
			}
		}
		else if(strcmp(p_node->name, "trap") == 0)
			is_cli_show_snmp_configured_trap_ctl(p_node);
		else if(strcmp(p_node->name, "threshold") == 0  && threshold_enabled)
		{
			silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
			{
				if(strcmp(p_sub_node->name, "fan") == 0)
					is_cli_show_snmp_configured_threshold_fan(p_sub_node);
				else if(strcmp(p_sub_node->name, "sensor") == 0)
					is_cli_show_snmp_configured_threshold_sensor(p_sub_node);
			}

		}
	}
}

/*
 * status
 */
static inline void is_cli_show_snmp_engine_id(silc_mgmtd_if_node* p_node)
{
	silc_cli_simple_node_display(p_node, "SNMP Server Engine ID:", NULL, 0);
}

static inline void is_cli_show_snmp_status_engine_id_cb(silc_mgmtd_if_rsp* p_rsp)
{
	is_cli_show_snmp_engine_id(p_rsp->p_node_root);
}

static inline void is_cli_show_snmp_status_state(silc_mgmtd_if_node* p_node)
{
	silc_cstr trans[] = { "false", "disable", "true", "enable"};
	silc_cli_simple_node_display(p_node, "SNMP Server State:", trans, 2);
}

static inline void is_cli_show_snmp_status_communities(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-10s" };
	silc_cstr names[] = { "Community", "Access" };
	silc_cstr nodes[] = { NULL, "full-access" };
	silc_cstr trans_access[] = { "false", "readonly", "true", "readwrite" };
	silc_cstr* trans[] = { NULL, trans_access};
	int trans_num[] = { 0, 2 };

	silc_cli_l2tree_display(p_node,
			"SNMP v1 or v2c Communities:", nodes, names, formats, trans, trans_num, 2);
}

static inline void is_cli_show_snmp_status_users(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-10s" };
	silc_cstr names[] = { "User", "Access" };
	silc_cstr nodes[] = { NULL, "full-access" };
	silc_cstr trans_access[] = { "false", "readonly", "true", "readwrite" };
	silc_cstr* trans[] = { NULL, trans_access };
	int trans_num[] = { 0, 2 };

	silc_cli_l2tree_display(p_node,
			"SNMP v3 Users:", nodes, names, formats, trans, trans_num, 2);
}

static inline void is_cli_show_snmp_status_trap_hosts(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-6s", "%-16s", "%-12s", "%-20s" };
	silc_cstr names[] = { "HostIP", "Ver", "Community", "Auth/Priv", "Password" };
	silc_cstr nodes[] = { NULL, "version", "community", "auth", "passwd" };
	silc_cstr trans_auth[] = { "SHA", "sha/aes", "MD5", "md5/des"};
	silc_cstr* trans[] = { NULL, NULL, NULL, trans_auth, NULL};
	int trans_num[] = { 0, 0, 0, 2, 0 };

	silc_cli_l2tree_display(p_node,
			"SNMP Trap Hosts:", nodes, names, formats, trans, trans_num, 5);
}

static inline void is_cli_show_snmp_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;
	int snmpv3_only = 0;
	int show_engine_id = 1;

	if(silc_cli_get_product_info()->snmp_show_engine_id_func)
	{
		show_engine_id = silc_cli_get_product_info()->snmp_show_engine_id_func();
	}

	if(silc_cli_get_product_info()->snmp_v3_only_func)
	{
		snmpv3_only = silc_cli_get_product_info()->snmp_v3_only_func();
	}

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "state") == 0)
			is_cli_show_snmp_status_state(p_node);
		else if(strcmp(p_node->name, "engine-id") == 0 && show_engine_id)
			is_cli_show_snmp_engine_id(p_node);
		else if(strcmp(p_node->name, "communities") == 0 && !snmpv3_only)
			is_cli_show_snmp_status_communities(p_node);
		else if(strcmp(p_node->name, "users") == 0)
			is_cli_show_snmp_status_users(p_node);
		else if(strcmp(p_node->name, "trap-hosts") == 0)
			is_cli_show_snmp_status_trap_hosts(p_node);
	}
}

#endif
