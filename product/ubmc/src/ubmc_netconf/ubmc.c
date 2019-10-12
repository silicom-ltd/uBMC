/*
 * This is automatically generated callbacks file
 * It contains 3 parts: Configuration callbacks, RPC callbacks and state data callbacks.
 * Do NOT alter function signatures or any structures unless you know exactly what you are doing.
 */

#include <stdlib.h>
#include <sys/inotify.h>
#include <libxml/tree.h>
#include <libnetconf_xml.h>

#include <pthread.h>
#include <silc_mgmtd_interface.h>

/* transAPI version which must be compatible with libnetconf */
int transapi_version = 6;

/* Signal to libnetconf that configuration data were modified by any callback.
 * 0 - data not modified
 * 1 - data have been modified
 */
int config_modified = 0;

/*
 * Determines the callbacks order.
 * Set this variable before compilation and DO NOT modify it in runtime.
 * TRANSAPI_CLBCKS_LEAF_TO_ROOT (default)
 * TRANSAPI_CLBCKS_ROOT_TO_LEAF
 */
const TRANSAPI_CLBCKS_ORDER_TYPE callbacks_order = TRANSAPI_CLBCKS_ORDER_DEFAULT;

/* Do not modify or set! This variable is set by libnetconf to announce edit-config's error-option
Feel free to use it to distinguish module behavior for different error-option values.
 * Possible values:
 * NC_EDIT_ERROPT_STOP - Following callback after failure are not executed, all successful callbacks executed till
                         failure point must be applied to the device.
 * NC_EDIT_ERROPT_CONT - Failed callbacks are skipped, but all callbacks needed to apply configuration changes are executed
 * NC_EDIT_ERROPT_ROLLBACK - After failure, following callbacks are not executed, but previous successful callbacks are
                         executed again with previous configuration data to roll it back.
 */
NC_EDIT_ERROPT_TYPE erropt = NC_EDIT_ERROPT_NOTSET;

typedef struct md_req_node_s {
	char node_path[128];
	char* val;
} md_req_node;

int md_cli_init()
{
	if(0 != silc_mgmtd_if_client_init("127.0.0.1", 2626, -1, 3))
		return EXIT_FAILURE;
	silc_mgmtd_if_client_set_login_info_ex(SILC_MGMTD_IF_CLIENT_NETCONF, "root", NULL, NULL, "3", NULL);
	return EXIT_SUCCESS;
}

static silc_mgmtd_msg* md_cli_req(silc_mgmtd_if_req_type req_type, char* path,
		char* rootval, md_req_node* node_list, int node_cnt)
{
	silc_mgmtd_if_req* req = NULL;
	silc_mgmtd_msg* msg = NULL;
	const char* val;
	int i;

	if (rootval)
		val = rootval;
	else
		val = silc_mgmtd_if_path_get_last_name(path);
	req = silc_mgmtd_if_req_create(path, req_type, val);
	if (!req)
	{
		nc_verb_error("Fail to creat md cli req!");
		return NULL;
	}
	for(i=0; i<node_cnt; i++)
	{
		silc_mgmtd_if_req_add_node(req, node_list[i].node_path, node_list[i].val);
	}
	msg = silc_mgmtd_if_client_send_req_sync(req, 10);
	if (!msg)
	{
		nc_verb_error("Fail to send md cli req!");
		return NULL;
	}
	return msg;
}

static silc_mgmtd_if_rsp* md_cli_rsp(silc_mgmtd_msg* msg)
{
	silc_mgmtd_if_rsp* rsp = NULL;
	silc_list_for_each_entry(rsp, &msg->if_recv_items, node)
	{
		break;
	}
	return rsp;
}

#define MD_CLI_RSP_OK(rsp)	(strcmp(rsp->return_str, "OK") == 0)

silc_mgmtd_msg* md_cli_query(char* path)
{
	silc_mgmtd_msg* msg = NULL;
	msg = md_cli_req(SILC_MGMTD_IF_REQ_QUERY_SUB, path, "", NULL, 0);
	if(!msg)
		return NULL;
	return msg;
}

int md_cli_set(silc_mgmtd_if_req_type req_type, char* path,
		char* rootval, md_req_node* node_list, int node_cnt, char* err)
{
	silc_mgmtd_msg* msg = NULL;
	silc_mgmtd_if_rsp* rsp = NULL;
	int ret = EXIT_SUCCESS;

	msg = md_cli_req(req_type, path, rootval, node_list, node_cnt);
	if(!msg)
		return EXIT_FAILURE;
	rsp = md_cli_rsp(msg);
	if(!MD_CLI_RSP_OK(rsp))
	{
		if(err)
			strcpy(err, rsp->return_str);
		ret = EXIT_FAILURE;
	}
	silc_mgmtd_if_client_free_msg(msg);
	return ret;
}

int md_cli_action_simple(char* path, char* err)
{
	return md_cli_set(SILC_MGMTD_IF_REQ_ACTION, path, "", NULL, 0, err);
}

int md_cli_action_save(char* err)
{
	return md_cli_action_simple("/action/system/save-config", err);
}

static const char* get_xml_node_content(const xmlNodePtr node)
{
	if (node == NULL || node->children == NULL || node->children->type != XML_TEXT_NODE) {
		return NULL;
	}

	return (const char*) (node->children->content);
}

static int fail(struct nc_err** error, char* msg, int ret)
{
	if (error != NULL) {
		*error = nc_err_new(NC_ERR_OP_FAILED);
		if (msg != NULL) {
			nc_err_set(*error, NC_ERR_PARAM_MSG, msg);
		}
	}

	if (msg != NULL) {
		nc_verb_error(msg);
//		free(msg);
	}

	return ret;
}

static int rpc_fail(char* msg)
{
	struct nc_err* error;
	error = nc_err_new(NC_ERR_OP_FAILED);
	nc_err_set(error, NC_ERR_PARAM_MSG, msg);
	nc_verb_error(msg);

	return nc_reply_error(error);
}

#define SILC_UBMC_NS	"urn:silicom:params:xml:ns:yang:ubmc"

#define MAX_PATH_LEVEL		10
#define MAX_SUB_NODE_NUM	40

typedef struct xml_md_val_map_s {
	char* xml_val;
	char* md_val;
} xml_md_val_map;

typedef struct xml_md_node_map_s {
	char* xml_path[MAX_PATH_LEVEL];
	char* md_path[MAX_PATH_LEVEL];
	xml_md_val_map*	val_map;
} xml_md_node_map;

typedef struct xml_md_list_map_s {
	char* xml_parent[MAX_PATH_LEVEL];
	char* xml_node;
	char* md_path[MAX_PATH_LEVEL];
	int sub_node_cnt;
	xml_md_node_map sub_node_map[MAX_SUB_NODE_NUM];
} xml_md_list_map;

xml_md_val_map g_log_level_map[] = {
		{"EMERGENCY", "0"},
		{"ALERT", "1"},
		{"CRITICAL", "2"},
		{"ERROR", "3"},
		{"WARNING", "4"},
		{"NOTICE", "5"},
		{"INFORMATIONAL", "6"},
		{"DEBUG", "7"},
		{NULL, NULL},
};

xml_md_node_map g_sys_misc_map[] = {
		{{"hostname"},						{"hostname"},					NULL},
		{{"clock", "timezone-name"},		{"datetime", "timezone"},		NULL},
		{{"ntp", "enabled"},				{"datetime", "ntp-enabled"},	NULL},
		{{"syslog", "severity"},			{"log", "level"},				g_log_level_map},
		{{"syslog", "max-file-size"},		{"log", "max-size"},			NULL},
		{{"syslog", "remote", "enabled"},	{"log", "remote-enabled"},		NULL},
};

xml_md_list_map g_sys_misc_ntpserver_map = {
		.xml_parent = {"ntp"},
		.xml_node = "server",
		.md_path = {"datetime", "ntp-server-v2"},
		.sub_node_cnt = 1,
		.sub_node_map = {
				{{"address"},	{"*"},		NULL},
		}
};

xml_md_list_map g_sys_misc_logserver_map = {
		.xml_parent = {"syslog", "remote"},
		.xml_node = "destination",
		.md_path = {"log", "remote-server-v2"},
		.sub_node_cnt = 2,
		.sub_node_map = {
				{{"address"},	{"*"},		NULL},
				{{"port"},		{"port"},	NULL},
		}
};

xml_md_node_map g_sys_mgmt_map[] = {
		{{"session", "idle-timeout"},		{"session-exp-time"},			NULL},
		{{"ipsec", "enabled"},				{"ipsec-enabled"},				NULL},
};

xml_md_list_map g_sys_mgmt_vrf_map = {
		.xml_parent = {NULL},
		.xml_node = "vrf",
		.md_path = {"vrf-list"},
		.sub_node_cnt = 2,
		.sub_node_map = {
				{{"name"},		{"*"},		NULL},
				{{"table"},		{"table"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_vrf_process_map = {
		.xml_parent = {NULL},
		.xml_node = "vrf-process",
		.md_path = {"vrf-process-list"},
		.sub_node_cnt = 3,
		.sub_node_map = {
				{{"name"},		{"*"},			NULL},
				{{"process"},	{"process"},	NULL},
				{{"vrf"},		{"vrf"},		NULL},
		}
};

xml_md_list_map g_sys_mgmt_interface_map = {
		.xml_parent = {NULL},
		.xml_node = "interface",
		.md_path = {"interface-list"},
		.sub_node_cnt = 15,
		.sub_node_map = {
				{{"name"},				{"*"},				NULL},
				{{"description"},		{"desc"},			NULL},
				{{"enabled"},			{"state"},			NULL},
				{{"master"},			{"master"},			NULL},
				{{"mtu"},				{"mtu"},			NULL},
				{{"vlan"},				{"vlan"},			NULL},
				{{"dhcp-sendname"},		{"dhcp-sendname"},	NULL},
				{{"ipv4", "enabled"},								{"ipv4-enabled"},	NULL},
				{{"ipv4", "address", "origin"},						{"ipv4-origin"},	NULL},
				{{"ipv4", "address", "static", "ip"},				{"ipv4-address"},	NULL},
				{{"ipv4", "address", "static", "ip-prefix-length"},	{"ipv4-prefix"},	NULL},
				{{"ipv6", "enabled"},								{"ipv6-enabled"},	NULL},
				{{"ipv6", "address", "origin"},						{"ipv6-origin"},	NULL},
				{{"ipv6", "address", "static", "ip"},				{"ipv6-address"},	NULL},
				{{"ipv6", "address", "static", "ip-prefix-length"},	{"ipv6-prefix"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_interface_address_map = {
		.xml_parent = {NULL},
		.xml_node = "interface-address",
		.md_path = {"address-list"},
		.sub_node_cnt = 4,
		.sub_node_map = {
				{{"name"},				{"*"},			NULL},
				{{"address"},			{"address"},	NULL},
				{{"ip-prefix-length"},	{"prefix"},		NULL},
				{{"interface"},			{"interface"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_ip_route_map = {
		.xml_parent = {NULL},
		.xml_node = "ip-route",
		.md_path = {"route-list"},
		.sub_node_cnt = 4,
		.sub_node_map = {
				{{"name"},		{"*"},		NULL},
				{{"dest"},		{"dest"},	NULL},
				{{"via"},		{"via"},	NULL},
				{{"table"},		{"table"},	NULL},
				{{"dev"},		{"dev"},	NULL},
				{{"metric"},	{"metric"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_ipsec_key_map = {
		.xml_parent = {"ipsec"},
		.xml_node = "private-key",
		.md_path = {"key-list"},
		.sub_node_cnt = 2,
		.sub_node_map = {
				{{"name"},		{"*"},				NULL},
				{{"content"},	{"key-content"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_ipsec_cert_map = {
		.xml_parent = {"ipsec"},
		.xml_node = "certificate",
		.md_path = {"cert-list"},
		.sub_node_cnt = 2,
		.sub_node_map = {
				{{"name"},		{"*"},				NULL},
				{{"content"},	{"cert-content"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_ipsec_conn_map = {
		.xml_parent = {"ipsec"},
		.xml_node = "connection",
		.md_path = {"ipsec-conn"},
		.sub_node_cnt = 31,
		.sub_node_map = {
				{{"name"},			{"*"},				NULL},
				{{"mode"},			{"type"},			NULL},
				{{"protocol"},		{"keyexchange"},	NULL},
				{{"tries"},			{"keyingtries"},	NULL},
				{{"ike"},			{"ike"},			NULL},
				{{"esp"},			{"esp"},			NULL},
				{{"compress"},		{"compress"},		NULL},
				{{"replay-window"},	{"replay-window"},	NULL},
				{{"dpdaction"},		{"dpdaction"},		NULL},
				{{"dpddelay"},		{"dpddelay"},		NULL},
				{{"dpdtimeout"},	{"dpdtimeout"},		NULL},
				{{"ikelifetime"},	{"ikelifetime"},	NULL},
				{{"lifetime"},		{"lifetime"},		NULL},

				{{"local-ip"},		{"local-ip"},		NULL},
				{{"local-src-ip"},	{"local-src"},		NULL},
				{{"local-subnet"},	{"local-sub"},		NULL},
				{{"local-auth"},	{"local-auth"},		NULL},
				{{"local-auth2"},	{"local-auth2"},	NULL},
				{{"local-id"},		{"local-id"},		NULL},
				{{"local-id2"},		{"local-id2"},		NULL},
				{{"local-cert"},	{"local-cert"},		NULL},
				{{"local-cert2"},	{"local-cert2"},	NULL},

				{{"peer-ip"},		{"peer-ip"},		NULL},
				{{"peer-src-ip"},	{"peer-src"},		NULL},
				{{"peer-subnet"},	{"peer-sub"},		NULL},
				{{"peer-auth"},		{"peer-auth"},		NULL},
				{{"peer-auth2"},	{"peer-auth2"},		NULL},
				{{"peer-id"},		{"peer-id"},		NULL},
				{{"peer-id2"},		{"peer-id2"},		NULL},
				{{"peer-cert"},		{"peer-cert"},		NULL},
				{{"peer-cert2"},	{"peer-cert2"},		NULL},
		}
};

xml_md_list_map g_sys_mgmt_ipsec_secret_map = {
		.xml_parent = {"ipsec"},
		.xml_node = "secret",
		.md_path = {"ipsec-secret"},
		.sub_node_cnt = 7,
		.sub_node_map = {
				{{"name"},			{"*"},			NULL},
				{{"type"},			{"type"},		NULL},
				{{"host"},			{"host"},		NULL},
				{{"peer"},			{"peer"},		NULL},
				{{"key"},			{"key-string"},	NULL},
				{{"key-ref"},		{"key-file"},	NULL},
				{{"passphrase"},	{"passphrase"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_ipsec_ca_map = {
		.xml_parent = {"ipsec"},
		.xml_node = "ca",
		.md_path = {"ipsec-ca"},
		.sub_node_cnt = 5,
		.sub_node_map = {
				{{"name"},				{"*"},				NULL},
				{{"cacert"},			{"cacert"},			NULL},
				{{"ocspuri"},			{"ocspuri"},		NULL},
				{{"crluri"},			{"crluri"},			NULL},
				{{"certuribase"},		{"certuribase"},	NULL},
		}
};

xml_md_list_map g_sys_mgmt_dnsserver_map = {
		.xml_parent = {"dns-resolver"},
		.xml_node = "server",
		.md_path = {"dns"},
		.sub_node_cnt = 1,
		.sub_node_map = {
				{{"address"},	{"*"},		NULL},
		}
};

xml_md_list_map g_sys_mgmt_iptables_rule_map = {
		.xml_parent = {NULL},
		.xml_node = "iptables-rule",
		.md_path = {"iptables-rule"},
		.sub_node_cnt = 5,
		.sub_node_map = {
				{{"name"},		{"*"},			NULL},
				{{"version"},	{"version"},	NULL},
				{{"table"},		{"table"},		NULL},
				{{"priority"},	{"priority"},	NULL},
				{{"rule"},		{"rule"},		NULL},
		}
};

xml_md_node_map g_sys_service_map[] = {
		{{"console", "speed"},				{"com", "speed"},				NULL},
		{{"console", "terminal"},			{"com", "termtype"},			NULL},
		{{"ssh", "port"},					{"ssh", "port"},				NULL},
		{{"web", "idle-timeout"},			{"web", "session-timeout"},		NULL},
		{{"web", "http", "enabled"},		{"http", "enabled"},			NULL},
		{{"web", "http", "port"},			{"http", "port"},				NULL},
		{{"web", "https", "enabled"},		{"https", "enabled"},			NULL},
		{{"web", "https", "port"},			{"https", "port"},				NULL},
};

xml_md_node_map g_aaa_tacacs_map[] = {
		{{"enabled"},			{"static", "enabled"},	NULL},
		{{"timeout"},			{"static", "timeout"},	NULL},
		{{"service-tag"},		{"static", "service"},	NULL},
};

xml_md_list_map g_aaa_tacacs_server_map = {
		.xml_parent = {NULL},
		.xml_node = "server",
		.md_path = {"server"},
		.sub_node_cnt = 4,
		.sub_node_map = {
				{{"id"},			{"*"},				NULL},
				{{"address"},		{"server-ip"},		NULL},
				{{"port"},			{"server-port"},	NULL},
				{{"secret-key"},	{"secret"},			NULL},
		}
};

xml_md_node_map g_snmp_map[] = {
		{{"enabled"},			{"agent", "state"},	NULL},
};

xml_md_list_map g_snmp_user_map = {
		.xml_parent = {NULL},
		.xml_node = "user",
		.md_path = {"agent", "users"},
		.sub_node_cnt = 5,
		.sub_node_map = {
				{{"name"},			{"*"},				NULL},
				{{"enabled"},		{"state"},			NULL},
				{{"hash"},			{"auth"},			NULL},
				{{"pass"},			{"password"},		NULL},
				{{"write-enabled"},	{"full-access"},	NULL},
		}
};

xml_md_list_map g_snmp_traphost_map = {
		.xml_parent = {NULL},
		.xml_node = "trap-host",
		.md_path = {"agent", "trap-hosts"},
		.sub_node_cnt = 6,
		.sub_node_map = {
				{{"address"},		{"*"},			NULL},
				{{"enabled"},		{"state"},		NULL},
				{{"version"},		{"version"},	NULL},
				{{"access-id"},		{"community"},	NULL},
				{{"hash"},			{"auth"},		NULL},
				{{"pass"},			{"password"},	NULL},
		}
};

xml_md_node_map g_bmc_map[] = {
		{{"console", "speed"},			{"console", "com-speed"},		NULL},
		{{"console", "data"},			{"console", "com-data"},		NULL},
		{{"console", "parity"},			{"console", "com-parity"},		NULL},
		{{"console", "stopbits"},		{"console", "com-stopbits"},	NULL},
		{{"console", "hw-flowctrl"},	{"console", "com-hwflowctrl"},	NULL},
		{{"console", "sw-flowctrl"},	{"console", "com-swflowctrl"},	NULL},
		{{"log", "to-file"},			{"console", "log-to-file"},		NULL},
		{{"log", "rotate-num"},			{"console", "log-rotate-num"},	NULL},
		{{"log", "rotate-size"},		{"console", "log-rotate-size"},	NULL},
};

xml_md_node_map g_status_sys_map[] = {
		{{"clock", "current-datetime"},		{"time"},	NULL},
		{{"clock", "uptime"},				{"uptime"},	NULL},
};

xml_md_node_map g_status_switch_map[] = {
		{{"platform", "device-name"},		{"info", "device_name"},	NULL},
		{{"platform", "device-id"},			{"info", "device_sn"},		NULL},
		{{"platform", "hardware-version"},	{"info", "hw_version"},		NULL},
		{{"platform", "software-version"},	{"info", "sw_version"},		NULL},
		{{"platform", "uboot-version"},		{"info", "uboot_version"},	NULL},
};

static xmlNodePtr find_xml_node(xmlNodePtr root, char** xml_path, int need_create)
{
	xmlNodePtr xml_cur, xml_sub;
	char* xml_node;
	int i;

	xml_cur = root;
	for(i=0; xml_path[i] && i<MAX_PATH_LEVEL; i++)
	{
		xml_node = xml_path[i];
		xml_sub = xml_cur->children;
		while(xml_sub)
		{
			if(xmlStrEqual(BAD_CAST xml_node, BAD_CAST xml_sub->name))
				break;
			xml_sub = xml_sub->next;
		}
		if(!xml_sub)
		{
			if(need_create)
			{
				xml_sub = xmlNewChild(xml_cur, xml_cur->ns, BAD_CAST xml_node, NULL);
				if(!xml_sub)
					return NULL;
			}
			else
				return NULL;
		}
		xml_cur = xml_sub;
	}
	return xml_cur;
}

static silc_mgmtd_if_node* find_md_node(silc_mgmtd_if_node* root, char** md_path)
{
	silc_mgmtd_if_node *md_cur, *md_sub;
	char* md_node;
	int i;

	md_cur = root;
	for(i=0; md_path[i] && i<MAX_PATH_LEVEL; i++)
	{
		md_node = md_path[i];
		md_sub = silc_mgmtd_if_req_find_node(md_cur, md_node);
		if(!md_sub)
		{
			return NULL;
		}
		md_cur = md_sub;
	}
	return md_cur;
}

static char* find_md_val_map(char* xml_val, xml_md_val_map* val_map)
{
	int i;
	for(i=0;val_map[i].xml_val;i++)
	{
		if(strcmp(xml_val, val_map[i].xml_val) == 0)
			return val_map[i].md_val;
	}
	return xml_val;
}

static char* find_xml_val_map(char* md_val, xml_md_val_map* val_map)
{
	int i;
	for(i=0;val_map[i].xml_val;i++)
	{
		if(strcmp(md_val, val_map[i].md_val) == 0)
			return val_map[i].xml_val;
	}
	return md_val;
}

static void get_md_node_path(char** nodes, char* node_path)
{
	int i;
	node_path[0] = 0;
	for(i=0; nodes[i] && i<MAX_PATH_LEVEL; i++)
	{
		if(node_path[0])
			strcat(node_path, "/");
		strcat(node_path, nodes[i]);
	}
}

static int trans_md2xml(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root, xml_md_node_map* node_map, int map_cnt)
{
	xml_md_node_map* map;
	xmlNodePtr xml_node;
	silc_mgmtd_if_node *md_node;
	char *md_val, *xml_val;
	int i;

	for(i=0; i<map_cnt; i++)
	{
		map = &node_map[i];
		if(0 == i && 0 == strcmp(map->md_path[0], "*"))
		{
			// '*' means the key value for list
			md_node = md_root;
		}
		else
			md_node = find_md_node(md_root, map->md_path);
		if(!md_node)
		{
			nc_verb_error("Fail to find map %d mgmtd node %s!", i, map->md_path[0]);
			return EXIT_FAILURE;
		}
		xml_node = find_xml_node(xml_root, map->xml_path, 1);
		if(!xml_node)
		{
			nc_verb_error("Fail to find map %d xml node %s!", i, map->xml_path[0]);
			return EXIT_FAILURE;
		}
		if(map->val_map)
			xml_val = find_xml_val_map(md_node->val_str, map->val_map);
		else
			xml_val = md_node->val_str;
		xmlNodeSetContent(xml_node, BAD_CAST xml_val);
	}
	return EXIT_SUCCESS;
}

static int trans_md2xml_list(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root, xml_md_list_map* list_map)
{
	silc_mgmtd_if_node *p_list, *p_node;
	xmlNodePtr parent, child;

	p_list = find_md_node(md_root, list_map->md_path);
	if(!p_list)
	{
		nc_verb_error("Fail to find mgmtd list node %s!", list_map->md_path[0]);
		return EXIT_FAILURE;
	}
	if(silc_list_empty(&p_list->sub_node_list))
	{
		return EXIT_SUCCESS;
	}

	parent = find_xml_node(xml_root, list_map->xml_parent, 1);
	if(!parent)
	{
		nc_verb_error("Fail to find xml parent node %s!");
		return EXIT_FAILURE;
	}

	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		child = xmlNewChild(parent, parent->ns, BAD_CAST list_map->xml_node, NULL);
		if(!child)
		{
			nc_verb_error("Fail to create xml child node!");
			return EXIT_FAILURE;
		}
		if(EXIT_SUCCESS != trans_md2xml(p_node, child, list_map->sub_node_map, list_map->sub_node_cnt))
		{
			nc_verb_error("Fail to trans xml %s!", p_node->name);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int trans_xml2md(xmlNodePtr xml_root, md_req_node* md_nodes, int* node_cnt, xml_md_node_map* node_map, int map_cnt)
{
	xml_md_node_map* map;
	xmlNodePtr xml_node;
	char *md_val, *xml_val;
	int i, cnt=0;

	for(i=0; i<map_cnt; i++)
	{
		map = &node_map[i];
		xml_node = find_xml_node(xml_root, map->xml_path, 0);
		if(!xml_node)
		{
			// '*' means the key value for list
			if(0 == i && 0 == strcmp(map->md_path[0], "*"))
			{
				nc_verb_error("Fail to find xml key node!");
				return EXIT_FAILURE;
			}
			continue;
		}
		xml_val = get_xml_node_content(xml_node);
		if(!xml_val)
		{
			nc_verb_warning("xml node %s value is null, ignore it.", xml_node->name);
			continue;
		}
		if(map->val_map)
			md_val = find_md_val_map(xml_val, map->val_map);
		else
			md_val = xml_val;
		get_md_node_path(map->md_path, md_nodes[cnt].node_path);
		md_nodes[cnt].val = md_val;
		cnt++;
	}
	*node_cnt = cnt;
	return EXIT_SUCCESS;
}

static int trans_md2xml_sys_misc(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_sys_misc_map, sizeof(g_sys_misc_map)/sizeof(xml_md_node_map));
	trans_md2xml_list(md_root, xml_root, &g_sys_misc_ntpserver_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_misc_logserver_map);
	return EXIT_SUCCESS;
}

static int trans_md2xml_sys_mgmt(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_sys_mgmt_map, sizeof(g_sys_mgmt_map)/sizeof(xml_md_node_map));
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_vrf_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_vrf_process_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_interface_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_interface_address_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ip_route_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ipsec_key_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ipsec_cert_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ipsec_conn_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ipsec_secret_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_ipsec_ca_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_dnsserver_map);
	trans_md2xml_list(md_root, xml_root, &g_sys_mgmt_iptables_rule_map);
	return EXIT_SUCCESS;
}

static int trans_md2xml_sys_service(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_sys_service_map, sizeof(g_sys_service_map)/sizeof(xml_md_node_map));
	return EXIT_SUCCESS;
}

static int trans_md2xml_aaa_tacacs(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_aaa_tacacs_map, sizeof(g_aaa_tacacs_map)/sizeof(xml_md_node_map));
	trans_md2xml_list(md_root, xml_root, &g_aaa_tacacs_server_map);
	return EXIT_SUCCESS;
}

static int trans_md2xml_snmp(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_snmp_map, sizeof(g_snmp_map)/sizeof(xml_md_node_map));
	trans_md2xml_list(md_root, xml_root, &g_snmp_user_map);
	trans_md2xml_list(md_root, xml_root, &g_snmp_traphost_map);
	return EXIT_SUCCESS;
}

static int trans_md2xml_bmc(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_bmc_map, sizeof(g_bmc_map)/sizeof(xml_md_node_map));
	return EXIT_SUCCESS;
}

static int trans_md2xml_status_sys(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_status_sys_map, sizeof(g_status_sys_map)/sizeof(xml_md_node_map));
	return EXIT_SUCCESS;
}

static int trans_md2xml_status_switch(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	trans_md2xml(md_root, xml_root, g_status_switch_map, sizeof(g_status_switch_map)/sizeof(xml_md_node_map));
	return EXIT_SUCCESS;
}

typedef int (*trans_md2xml_cb)(silc_mgmtd_if_node* md_root, xmlNodePtr xml_root);

typedef struct md_section_map_s {
	char* name;
	char* md_path;
	char* xml_node;
	char* ts_file;
	trans_md2xml_cb trans_func;
} md_section_map;

typedef struct md_module_map_s {
	char* md_path;
	int sec_num;
	md_section_map* sec_list[MAX_SUB_NODE_NUM];
} md_module_map;

#define MD_PATH_CFG_SYS			"/config/system"
#define MD_PATH_CFG_TACACS		"/config/tacacs"
#define MD_PATH_CFG_SNMP		"/config/snmp"
#define MD_PATH_CFG_BMC			"/config/bmc"

#define MD_PATH_STA_SYS			"/status/system"
#define MD_PATH_STA_SWITCH		"/status/bmc"

#define TS_FILE_SYS_MISC	"/tmp/.stamp_sys_misc"
#define TS_FILE_SYS_MGMT	"/tmp/.stamp_sys_mgmt"
#define TS_FILE_SYS_SERVICE	"/tmp/.stamp_sys_service"
#define TS_FILE_AAA_TACACS	"/tmp/.stamp_aaa_tacacs"
#define TS_FILE_SNMP		"/tmp/.stamp_snmp"
#define TS_FILE_BMC			"/tmp/.stamp_bmc"

md_section_map g_md_sec_sys_misc = {
		.name = "misc",
		.md_path = MD_PATH_CFG_SYS"/misc",
		.xml_node = "general",
		.ts_file = TS_FILE_SYS_MISC,
		.trans_func = trans_md2xml_sys_misc
};

md_section_map g_md_sec_sys_mgmt = {
		.name = "mgmt",
		.md_path = MD_PATH_CFG_SYS"/mgmt",
		.xml_node = "management",
		.ts_file = TS_FILE_SYS_MGMT,
		.trans_func = trans_md2xml_sys_mgmt
};

md_section_map g_md_sec_sys_service = {
		.name = "service",
		.md_path = MD_PATH_CFG_SYS"/service",
		.xml_node = "service",
		.ts_file = TS_FILE_SYS_SERVICE,
		.trans_func = trans_md2xml_sys_service
};

md_section_map g_md_sec_aaa_tacacs = {
		.name = NULL,
		.md_path = MD_PATH_CFG_TACACS,
		.xml_node = "tacacs",
		.ts_file = TS_FILE_AAA_TACACS,
		.trans_func = trans_md2xml_aaa_tacacs
};

md_section_map g_md_sec_snmp = {
		.name = NULL,
		.md_path = MD_PATH_CFG_SNMP,
		.xml_node = "snmp",
		.ts_file = TS_FILE_SNMP,
		.trans_func = trans_md2xml_snmp
};

md_section_map g_md_sec_bmc = {
		.name = NULL,
		.md_path = MD_PATH_CFG_BMC,
		.xml_node = "bmc",
		.ts_file = TS_FILE_BMC,
		.trans_func = trans_md2xml_bmc
};

md_module_map g_md_module_map[] = {
		// system
		{
				.md_path = MD_PATH_CFG_SYS,
				.sec_num = 3,
				.sec_list = {
						&g_md_sec_sys_misc,
						&g_md_sec_sys_mgmt,
						&g_md_sec_sys_service,
				},
		},
		// aaa
		{
				.md_path = MD_PATH_CFG_TACACS,
				.sec_num = 1,
				.sec_list = {
						&g_md_sec_aaa_tacacs,
				},
		},
		// snmp
		{
				.md_path = MD_PATH_CFG_SNMP,
				.sec_num = 1,
				.sec_list = {
						&g_md_sec_snmp,
				},
		},
		// bmc
		{
				.md_path = MD_PATH_CFG_BMC,
				.sec_num = 1,
				.sec_list = {
						&g_md_sec_bmc,
				},
		},
};

md_section_map g_md_sec_status_sys = {
		.name = NULL,
		.md_path = MD_PATH_STA_SYS,
		.xml_node = NULL,
		.ts_file = "",
		.trans_func = trans_md2xml_status_sys
};

md_section_map g_md_sec_status_switch = {
		.name = NULL,
		.md_path = MD_PATH_STA_SWITCH,
		.xml_node = NULL,
		.ts_file = "",
		.trans_func = trans_md2xml_status_switch
};

md_module_map g_md_status_map[] = {
		// system
		{
				.md_path = MD_PATH_STA_SYS,
				.sec_num = 1,
				.sec_list = {
						&g_md_sec_status_sys,
				},
		},
		// switch
		{
				.md_path = MD_PATH_STA_SWITCH,
				.sec_num = 1,
				.sec_list = {
						&g_md_sec_status_switch,
				},
		},
};

static int trans_md2xml_section(md_section_map* sec, silc_mgmtd_if_node* md_root, xmlNodePtr xml_root)
{
	silc_mgmtd_msg* p_msg = NULL;
	silc_mgmtd_if_rsp* p_rsp = NULL;
	int ret = EXIT_SUCCESS;

	if(md_root)
		return sec->trans_func(md_root, xml_root);

	p_msg = md_cli_query(sec->md_path);
	if (!p_msg)
	{
		return EXIT_FAILURE;
	}
	p_rsp = md_cli_rsp(p_msg);
	if(!MD_CLI_RSP_OK(p_rsp))
	{
		nc_verb_error("fail to query %s, error: %s", sec->md_path, p_rsp->return_str);
		ret = EXIT_FAILURE;
		goto END;
	}
	if(EXIT_SUCCESS != sec->trans_func(p_rsp->p_node_root, xml_root))
	{
		nc_verb_error("fail to trans md2xml %s -> %s", sec->md_path, sec->xml_node);
		ret = EXIT_FAILURE;
		goto END;
	}

END:
	silc_mgmtd_if_client_free_msg(p_msg);
	return ret;
};

static int trans_md2xml_module(md_module_map* mod, xmlNodePtr xml_root)
{
	silc_mgmtd_msg* p_msg = NULL;
	silc_mgmtd_if_rsp* p_rsp = NULL;
	silc_mgmtd_if_node *p_node;
	xmlNodePtr xml_node;
	md_section_map* sec;
	int i, ret=EXIT_SUCCESS;

	p_msg = md_cli_query(mod->md_path);
	if (!p_msg)
	{
		return EXIT_FAILURE;
	}
	p_rsp = md_cli_rsp(p_msg);
	if(!MD_CLI_RSP_OK(p_rsp))
	{
		nc_verb_error("fail to query %s, error: %s", mod->md_path, p_rsp->return_str);
		silc_mgmtd_if_client_free_msg(p_msg);
		return EXIT_FAILURE;
	}
#if 0
	if(mod->xml_node && strlen(mod->xml_node))
		xml_root = xmlNewChild(xml_root, xml_root->ns, BAD_CAST mod->xml_node, NULL);
#endif
	for(i=0; i<mod->sec_num; i++)
	{
		sec = mod->sec_list[i];
		if(sec->name)
			p_node = silc_mgmtd_if_req_find_node(p_rsp->p_node_root, sec->name);
		else
			p_node = p_rsp->p_node_root;
		if(!p_node)
		{
			nc_verb_error("fail to find %s", sec->md_path);
			ret = EXIT_FAILURE;
			break;
		}
		if(sec->xml_node)
			xml_node = xmlNewChild(xml_root, xml_root->ns, BAD_CAST sec->xml_node, NULL);
		else
			xml_node = xml_root;
		if(!xml_node)
		{
			nc_verb_error("fail to create xml node %s", sec->xml_node);
			ret = EXIT_FAILURE;
			break;
		}
		if(EXIT_SUCCESS != trans_md2xml_section(sec, p_node, xml_node))
		{
			nc_verb_error("fail to trans xml %s->%s", sec->md_path, sec->xml_node);
			ret = EXIT_FAILURE;
			break;
		}
	}
	silc_mgmtd_if_client_free_msg(p_msg);
	return ret;
}

static xmlNodePtr create_xml_root(xmlDocPtr *doc, char* node_name)
{
	xmlNodePtr root;
	xmlNsPtr ns;

	*doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST node_name);
	xmlDocSetRootElement(*doc, root);
	ns = xmlNewNs(root, BAD_CAST SILC_UBMC_NS, NULL);
	xmlSetNs(root, ns);

	return root;
}

static xmlNodePtr create_xml_sys_cfg_root(xmlDocPtr *doc)
{
	return create_xml_root(doc, "system-config");
}

static xmlNodePtr create_xml_sys_state_root(xmlDocPtr *doc)
{
	return create_xml_root(doc, "system-state");
}

/**
 * @brief Initialize plugin after loaded and before any other functions are called.

 * This function should not apply any configuration data to the controlled device. If no
 * running is returned (it stays *NULL), complete startup configuration is consequently
 * applied via module callbacks. When a running configuration is returned, libnetconf
 * then applies (via module's callbacks) only the startup configuration data that
 * differ from the returned running configuration data.

 * Please note, that copying startup data to the running is performed only after the
 * libnetconf's system-wide close - see nc_close() function documentation for more
 * information.

 * @param[out] running	Current configuration of managed device.

 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int transapi_init(xmlDocPtr *running) {
	xmlNodePtr root;
	md_module_map* mod;
	int i, mod_cnt = sizeof(g_md_module_map)/sizeof(md_module_map);

	if(md_cli_init() != EXIT_SUCCESS)
		return EXIT_FAILURE;

	root = create_xml_sys_cfg_root(running);

	for(i=0; i<mod_cnt; i++)
	{
		mod = &g_md_module_map[i];
		if(mod->sec_num == 0)
			continue;
		if(EXIT_SUCCESS != trans_md2xml_module(mod, root))
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Free all resources allocated on plugin runtime and prepare plugin for removal.
 */
void transapi_close(void) {
	silc_mgmtd_if_client_shutdown();
	return;
}

/**
 * @brief Retrieve state data from device and return them as XML document
 *
 * @param model	Device data model. libxml2 xmlDocPtr.
 * @param running	Running datastore content. libxml2 xmlDocPtr.
 * @param[out] err  Double pointer to error structure. Fill error when some occurs.
 * @return State data as libxml2 xmlDocPtr or NULL in case of error.
 */
xmlDocPtr get_state_data(xmlDocPtr model, xmlDocPtr running, struct nc_err **err) {
	xmlDocPtr state_doc;
	xmlNodePtr root;
	md_module_map* mod;
	int i, mod_cnt = sizeof(g_md_status_map)/sizeof(md_module_map);

	root = create_xml_sys_state_root(&state_doc);
	for(i=0; i<mod_cnt; i++)
	{
		mod = &g_md_status_map[i];
		if(mod->sec_num == 0)
			continue;
		if(EXIT_SUCCESS != trans_md2xml_module(mod, root))
			return NULL;
	}

	return state_doc;
}
/*
 * Mapping prefixes with namespaces.
 * Do NOT modify this structure!
 */
struct ns_pair namespace_mapping[] = {{"ubmc", "urn:silicom:params:xml:ns:yang:ubmc"}, {NULL, NULL}};

/*
 * CONFIGURATION callbacks
 * Here follows set of callback functions run every time some change in associated part of running datastore occurs.
 * You can safely modify the bodies of all function as well as add new functions for better lucidity of code.
 */
static int g_md_save_on_write = 1;

int md_save_on_write(char* err)
{
	if(!g_md_save_on_write)
		return EXIT_SUCCESS;
	return md_cli_action_save(err);
}

int callback_ubmc_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		char* parent_path, xml_md_node_map* map, struct nc_err **error)
{
	char *xml_val, *md_val;
	char full_path[256], sub_path[128], err[128];

	if(op & XMLDIFF_REM)
		return EXIT_SUCCESS;
	xml_val = get_xml_node_content(xml_node);
	if(!xml_val)
		return fail(error, "fail to get xml node value!", EXIT_FAILURE);

	if(map->val_map)
		md_val = find_md_val_map(xml_val, map->val_map);
	else
		md_val = xml_val;
	get_md_node_path(map->md_path, sub_path);
	sprintf(full_path, "%s/%s", parent_path, sub_path);
	if(EXIT_SUCCESS != md_cli_set(SILC_MGMTD_IF_REQ_MODIFY, full_path, md_val,
			NULL, 0, err))
		return fail(error, err, EXIT_FAILURE);

	if(EXIT_SUCCESS != md_save_on_write(err))
		return fail(error, err, EXIT_FAILURE);

	return EXIT_SUCCESS;
}

int callback_ubmc_dyn_node_modify(XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node,
		char* parent_path, xml_md_list_map* map, struct nc_err **error)
{
	xmlNodePtr xml_node;
	md_req_node nodes[MAX_SUB_NODE_NUM];
	int node_cnt;
	char full_path[256], sub_path[128], err[128];
	char* key;
	silc_mgmtd_if_req_type req_type;

	xml_node = (op & XMLDIFF_REM ? old_node : new_node);
	if(EXIT_SUCCESS != trans_xml2md(xml_node, nodes, &node_cnt, map->sub_node_map, map->sub_node_cnt))
		return fail(error, "fail to get xml node value!", EXIT_FAILURE);
	key = nodes[0].val;
	node_cnt--;

	req_type = (op & XMLDIFF_ADD ? SILC_MGMTD_IF_REQ_ADD : (op & XMLDIFF_REM ? SILC_MGMTD_IF_REQ_DELETE : SILC_MGMTD_IF_REQ_MODIFY));
	get_md_node_path(map->md_path, sub_path);
	sprintf(full_path, "%s/%s/%s", parent_path, sub_path, key);
	if(EXIT_SUCCESS != md_cli_set(req_type, full_path, key,
			&nodes[1], node_cnt, err))
		return fail(error, err, EXIT_FAILURE);

	if(EXIT_SUCCESS != md_save_on_write(err))
		return fail(error, err, EXIT_FAILURE);

	return EXIT_SUCCESS;
}

int callback_ubmc_sys_misc_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_sys_misc_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_sys_misc.md_path, map, error);
}

int callback_ubmc_sys_mgmt_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_sys_mgmt_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_sys_mgmt.md_path, map, error);
}

int callback_ubmc_sys_service_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_sys_service_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_sys_service.md_path, map, error);
}

int callback_ubmc_aaa_tacacs_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_aaa_tacacs_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_aaa_tacacs.md_path, map, error);
}

int callback_ubmc_snmp_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_snmp_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_snmp.md_path, map, error);
}

int callback_ubmc_bmc_static_node_modify(XMLDIFF_OP op, xmlNodePtr xml_node,
		int map_idx, struct nc_err **error)
{
	xml_md_node_map* map = &g_bmc_map[map_idx];
	return callback_ubmc_static_node_modify(op, xml_node,
			g_md_sec_bmc.md_path, map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:hostname changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */

int callback_ubmc_system_config_ubmc_general_ubmc_hostname(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:clock/ubmc:timezone-name changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_clock_ubmc_timezone_name(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 1, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:ntp/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_ntp_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 2, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:ntp/ubmc:server changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_ntp_ubmc_server(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_misc.md_path, &g_sys_misc_ntpserver_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:severity changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_severity(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 3, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:max-file-size changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_max_file_size(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 4, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:remote/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_remote_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 5, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:remote/ubmc:destination changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_remote_ubmc_destination(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_misc.md_path, &g_sys_misc_logserver_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:session/ubmc:idle-timeout changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_session_ubmc_idle_timeout(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_mgmt_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:vrf changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_vrf(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_vrf_map, error);;
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:vrf-process changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_vrf_process(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_vrf_process_map, error);;
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:interface changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_interface(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_interface_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:interface-address changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_interface_address(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_interface_address_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ip-route changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ip_route(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ip_route_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_misc_static_node_modify(op, new_node, 1, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:private-key changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_private_key(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ipsec_key_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:certificate changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_certificate(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ipsec_cert_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:connection changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_connection(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ipsec_conn_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:secret changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_secret(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ipsec_secret_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:ca changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_ca(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_ipsec_ca_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:dns-resolver/ubmc:server changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_dns_resolver_ubmc_server(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_dnsserver_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:management/ubmc:iptables-rule changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_management_ubmc_iptables_rule(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_sys_mgmt.md_path, &g_sys_mgmt_iptables_rule_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:console/ubmc:speed changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_console_ubmc_speed(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:console/ubmc:terminal changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_console_ubmc_terminal(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 1, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:ssh/ubmc:port changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_ssh_ubmc_port(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 2, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:web/ubmc:idle-timeout changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_idle_timeout(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 3, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:web/ubmc:http/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_http_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 4, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:web/ubmc:http/ubmc:port changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_http_ubmc_port(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 5, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:web/ubmc:https/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_https_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 6, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:service/ubmc:web/ubmc:https/ubmc:port changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_https_ubmc_port(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_sys_service_static_node_modify(op, new_node, 7, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:tacacs/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_tacacs_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_aaa_tacacs_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:tacacs/ubmc:timeout changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_tacacs_ubmc_timeout(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_aaa_tacacs_static_node_modify(op, new_node, 1, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:tacacs/ubmc:service-tag changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_tacacs_ubmc_service_tag(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_aaa_tacacs_static_node_modify(op, new_node, 2, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:tacacs/ubmc:server changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_tacacs_ubmc_server(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_aaa_tacacs.md_path, &g_aaa_tacacs_server_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:snmp/ubmc:enabled changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_snmp_ubmc_enabled(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_snmp_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:snmp/ubmc:user changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_snmp_ubmc_user(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_snmp.md_path, &g_snmp_user_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:snmp/ubmc:trap-host changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_snmp_ubmc_trap_host(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_dyn_node_modify(op, old_node, new_node,
			g_md_sec_snmp.md_path, &g_snmp_traphost_map, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:speed changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_speed(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 0, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:data changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_data(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 1, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:parity changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_parity(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 2, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:stopbits changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_stopbits(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 3, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:hw-flowctrl changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_hw_flowctrl(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 4, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:sw-flowctrl changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_sw_flowctrl(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 5, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:to-file changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_to_file(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 6, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:rotate-num changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_rotate_num(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 7, error);
}

/**
 * @brief This callback will be run when node in path /ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:rotate-size changes
 *
 * @param[in] data	Double pointer to void. Its passed to every callback. You can share data using it.
 * @param[in] op	Observed change in path. XMLDIFF_OP type.
 * @param[in] old_node	Old configuration node. If op == XMLDIFF_ADD, it is NULL.
 * @param[in] new_node	New configuration node. if op == XMLDIFF_REM, it is NULL.
 * @param[out] error	If callback fails, it can return libnetconf error structure with a failure description.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
/* !DO NOT ALTER FUNCTION SIGNATURE! */
int callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_rotate_size(void **data, XMLDIFF_OP op, xmlNodePtr old_node, xmlNodePtr new_node, struct nc_err **error) {
	return callback_ubmc_bmc_static_node_modify(op, new_node, 8, error);
}

/*
 * Structure transapi_config_callbacks provide mapping between callback and path in configuration datastore.
 * It is used by libnetconf library to decide which callbacks will be run.
 * DO NOT alter this structure
 */
struct transapi_data_callbacks clbks =  {
	.callbacks_count = 46,
	.data = NULL,
	.callbacks = {
		{.path = "/ubmc:system-config/ubmc:general/ubmc:hostname", .func = callback_ubmc_system_config_ubmc_general_ubmc_hostname},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:clock/ubmc:timezone-name", .func = callback_ubmc_system_config_ubmc_general_ubmc_clock_ubmc_timezone_name},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:ntp/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_general_ubmc_ntp_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:ntp/ubmc:server", .func = callback_ubmc_system_config_ubmc_general_ubmc_ntp_ubmc_server},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:severity", .func = callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_severity},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:max-file-size", .func = callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_max_file_size},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:remote/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_remote_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:general/ubmc:syslog/ubmc:remote/ubmc:destination", .func = callback_ubmc_system_config_ubmc_general_ubmc_syslog_ubmc_remote_ubmc_destination},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:session/ubmc:idle-timeout", .func = callback_ubmc_system_config_ubmc_management_ubmc_session_ubmc_idle_timeout},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:vrf", .func = callback_ubmc_system_config_ubmc_management_ubmc_vrf},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:vrf-process", .func = callback_ubmc_system_config_ubmc_management_ubmc_vrf_process},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:interface", .func = callback_ubmc_system_config_ubmc_management_ubmc_interface},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:interface-address", .func = callback_ubmc_system_config_ubmc_management_ubmc_interface_address},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ip-route", .func = callback_ubmc_system_config_ubmc_management_ubmc_ip_route},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:private-key", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_private_key},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:certificate", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_certificate},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:connection", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_connection},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:secret", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_secret},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:ipsec/ubmc:ca", .func = callback_ubmc_system_config_ubmc_management_ubmc_ipsec_ubmc_ca},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:dns-resolver/ubmc:server", .func = callback_ubmc_system_config_ubmc_management_ubmc_dns_resolver_ubmc_server},
		{.path = "/ubmc:system-config/ubmc:management/ubmc:iptables-rule", .func = callback_ubmc_system_config_ubmc_management_ubmc_iptables_rule},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:console/ubmc:speed", .func = callback_ubmc_system_config_ubmc_service_ubmc_console_ubmc_speed},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:console/ubmc:terminal", .func = callback_ubmc_system_config_ubmc_service_ubmc_console_ubmc_terminal},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:ssh/ubmc:port", .func = callback_ubmc_system_config_ubmc_service_ubmc_ssh_ubmc_port},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:web/ubmc:idle-timeout", .func = callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_idle_timeout},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:web/ubmc:http/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_http_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:web/ubmc:http/ubmc:port", .func = callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_http_ubmc_port},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:web/ubmc:https/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_https_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:service/ubmc:web/ubmc:https/ubmc:port", .func = callback_ubmc_system_config_ubmc_service_ubmc_web_ubmc_https_ubmc_port},
		{.path = "/ubmc:system-config/ubmc:tacacs/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_tacacs_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:tacacs/ubmc:timeout", .func = callback_ubmc_system_config_ubmc_tacacs_ubmc_timeout},
		{.path = "/ubmc:system-config/ubmc:tacacs/ubmc:service-tag", .func = callback_ubmc_system_config_ubmc_tacacs_ubmc_service_tag},
		{.path = "/ubmc:system-config/ubmc:tacacs/ubmc:server", .func = callback_ubmc_system_config_ubmc_tacacs_ubmc_server},
		{.path = "/ubmc:system-config/ubmc:snmp/ubmc:enabled", .func = callback_ubmc_system_config_ubmc_snmp_ubmc_enabled},
		{.path = "/ubmc:system-config/ubmc:snmp/ubmc:user", .func = callback_ubmc_system_config_ubmc_snmp_ubmc_user},
		{.path = "/ubmc:system-config/ubmc:snmp/ubmc:trap-host", .func = callback_ubmc_system_config_ubmc_snmp_ubmc_trap_host},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:speed", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_speed},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:data", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_data},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:parity", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_parity},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:stopbits", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_stopbits},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:hw-flowctrl", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_hw_flowctrl},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:console/ubmc:sw-flowctrl", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_console_ubmc_sw_flowctrl},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:to-file", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_to_file},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:rotate-num", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_rotate_num},
		{.path = "/ubmc:system-config/ubmc:bmc/ubmc:log/ubmc:rotate-size", .func = callback_ubmc_system_config_ubmc_bmc_ubmc_log_ubmc_rotate_size}
	}
};


/**
 * @brief Get a node from the RPC input. The first found node is returned, so if traversing lists,
 * call repeatedly with result->next as the node argument.
 *
 * @param name	Name of the node to be retrieved.
 * @param node	List of nodes that will be searched.
 * @return Pointer to the matching node or NULL
 */
xmlNodePtr get_rpc_node(const char *name, const xmlNodePtr node) {
	xmlNodePtr ret = NULL;

	for (ret = node; ret != NULL; ret = ret->next) {
		if (xmlStrEqual(BAD_CAST name, ret->name)) {
			break;
		}
	}

	return ret;
}

/*
 * RPC callbacks
 * Here follows set of callback functions run every time RPC specific for this device arrives.
 * You can safely modify the bodies of all function as well as add new functions for better lucidity of code.
 * Every function takes an libxml2 list of inputs as an argument.
 * If input was not set in RPC message argument is set to NULL. To retrieve each argument, preferably use get_rpc_node().
 */

nc_reply *rpc_set_current_datetime(xmlNodePtr input) {
	const char* path = "/action/system/set-datetime";
	const char* new_time = NULL;
	char err[256];
	xmlNodePtr current_datetime = get_rpc_node("current-datetime", input);

	if (current_datetime == NULL) {
		return rpc_fail("RPC set-current-datetime without the datetime node.");
	}
	new_time = get_xml_node_content(current_datetime);
	if (new_time == NULL) {
		return rpc_fail("RPC set-current-datetime without the datetime value.");
	}
	if(EXIT_SUCCESS != md_cli_set(SILC_MGMTD_IF_REQ_ACTION, path, new_time, NULL, 0, err))
		return rpc_fail(err);

	return nc_reply_ok();
}
nc_reply *rpc_system_restart(xmlNodePtr input) {
	const char* path = "/action/system/reboot";
	char err[256];
	if(EXIT_SUCCESS != md_cli_action_simple(path, err))
		return rpc_fail(err);
	return nc_reply_ok();
}
nc_reply *rpc_system_shutdown(xmlNodePtr input) {
	const char* path = "/action/system/halt";
	char err[256];
	if(EXIT_SUCCESS != md_cli_action_simple(path, err))
		return rpc_fail(err);
	return nc_reply_ok();
}
nc_reply *rpc_config_save(xmlNodePtr input) {
	const char* path = "/action/system/save-config";
	char err[256];
	if(EXIT_SUCCESS != md_cli_action_simple(path, err))
		return rpc_fail(err);
	return nc_reply_ok();
}
nc_reply *rpc_snmp_apply(xmlNodePtr input) {
	const char* path = "/action/snmp/apply";
	char err[256];
	if(EXIT_SUCCESS != md_cli_action_simple(path, err))
		return rpc_fail(err);
	return nc_reply_ok();
}
/*
 * Structure transapi_rpc_callbacks provides mapping between callbacks and RPC messages.
 * It is used by libnetconf library to decide which callbacks will be run when RPC arrives.
 * DO NOT alter this structure
 */
struct transapi_rpc_callbacks rpc_clbks = {
	.callbacks_count = 5,
	.callbacks = {
		{.name="set-current-datetime", .func=rpc_set_current_datetime},
		{.name="system-restart", .func=rpc_system_restart},
		{.name="system-shutdown", .func=rpc_system_shutdown},
		{.name="config-save", .func=rpc_config_save},
		{.name="snmp-apply", .func=rpc_snmp_apply}
	}
};

int callback_ubmc_md_sec_change(xmlDocPtr *edit_config, md_section_map* sec)
{
	xmlNodePtr root, config = NULL;
	xmlNsPtr ns;

	nc_verb_verbose("file %s is changed", sec->ts_file);

	root = create_xml_sys_cfg_root(edit_config);
	ns = root->ns;
	xmlNewNs(root, BAD_CAST "urn:ietf:params:xml:ns:netconf:base:1.0", BAD_CAST "ncop");

	config = xmlNewNode(ns, BAD_CAST sec->xml_node);

	if (EXIT_SUCCESS != trans_md2xml_section(sec, NULL, config)) {
		xmlFreeDoc(*edit_config);
		*edit_config = NULL;
		return EXIT_FAILURE;
	}

	xmlSetProp(config, BAD_CAST "ncop:operation", BAD_CAST "replace");
	xmlAddChild(root, config);

	return EXIT_SUCCESS;
}

int callback_ubmc_sys_misc_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_sys_misc;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

int callback_ubmc_sys_mgmt_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_sys_mgmt;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

int callback_ubmc_sys_service_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_sys_service;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

int callback_ubmc_aaa_tacacs_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_aaa_tacacs;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

int callback_ubmc_snmp_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_snmp;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

int callback_ubmc_bmc_change(const char *filepath, xmlDocPtr *edit_config, int *exec)
{
	md_section_map* sec = &g_md_sec_bmc;

	*exec = 0;
	return callback_ubmc_md_sec_change(edit_config, sec);
}

/*
 * Structure transapi_file_callbacks provides mapping between specific files
 * (e.g. configuration file in /etc/) and the callback function executed when
 * the file is modified.
 * The structure is empty by default. Add items, as in example, as you need.
 *
 * Example:
 * int example_callback(const char *filepath, xmlDocPtr *edit_config, int *exec) {
 *     // do the job with changed file content
 *     // if needed, set edit_config parameter to the edit-config data to be applied
 *     // if needed, set exec to 1 to perform consequent transapi callbacks
 *     return 0;
 * }
 *
 * struct transapi_file_callbacks file_clbks = {
 *     .callbacks_count = 1,
 *     .callbacks = {
 *         {.path = "/etc/my_cfg_file", .func = example_callback}
 *     }
 * }
 */
struct transapi_file_callbacks file_clbks = {
	.callbacks_count = 6,
	.callbacks = {
			{.path = TS_FILE_SYS_MISC,		.func = callback_ubmc_sys_misc_change},
			{.path = TS_FILE_SYS_MGMT,		.func = callback_ubmc_sys_mgmt_change},
			{.path = TS_FILE_SYS_SERVICE,	.func = callback_ubmc_sys_service_change},
			{.path = TS_FILE_AAA_TACACS,	.func = callback_ubmc_aaa_tacacs_change},
			{.path = TS_FILE_SNMP,			.func = callback_ubmc_snmp_change},
			{.path = TS_FILE_BMC,			.func = callback_ubmc_bmc_change},
	}
};

