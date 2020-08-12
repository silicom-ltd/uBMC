/* this file is created by silc_mgmtd_inst_func_source_gen.py */ 

#include "is_mgmtd_func_def.h"
#include "silc_mgmtd_interface.h"

int is_file_exist(silc_cstr path);
int is_write_file(silc_cstr path, silc_cstr buf, int len);

#ifndef SILC_MGMTD_LOCAL_DEBUG
#define IS_MGMTD_MGMT_IF_NAME	"eth0"
#else
#define IS_MGMTD_MGMT_IF_NAME	"eth0:0"
#endif

#define IS_MGMTD_SYSTEM_MGMT_PATH	"/config/system/mgmt"

typedef enum
{
	IS_IP_ORIGIN_STATIC = 0,
	IS_IP_ORIGIN_DHCP,
	IS_IP_ORIGIN_MAX
} is_ip_origin;

typedef struct is_ip_config_s
{
	int		enabled;
	int		origin;
	char 	addr[SILC_MGMTD_IF_NAME_MAX_LEN];
	int		prefixlen;
} is_ip_config;

typedef struct is_inf_config_s
{
	silc_cstr			name;
	silc_cstr			dev;
	silc_cstr			master;
	int				enabled;
	int				mtu;
	int 			vlan;
	int				dhcp_sendname;
	is_ip_config	ipv4;
	is_ip_config	ipv6;
} is_inf_config;

int is_trans_ip_origin(char* str)
{
	if(strcasecmp(str, "static") == 0)
		return IS_IP_ORIGIN_STATIC;
	if(strcasecmp(str, "dhcp") == 0)
		return IS_IP_ORIGIN_DHCP;
	return IS_IP_ORIGIN_MAX;
}

void is_gen_dhcp_conf(char* dhcp_conf, int is_sendname)
{
	char content[256] = {0};

	if(is_sendname)
	{
		char hostname[64];
		gethostname(hostname, 64);
		sprintf(content, "send host-name \"%s\";\n", hostname);
	}
	strcat(content,
			"request subnet-mask, broadcast-address, time-offset, routers, "
			"domain-name, domain-name-servers, host-name, "
			"netbios-name-servers, netbios-scope;\n"
			"timeout 60;\n"
			"retry 20;\n");

	is_write_file(dhcp_conf, content, strlen(content));
}

#define IS_DHCLIENT_SCRIPT	"/usr/sbin/is-dhclient-script"

int is_config_interface(silc_mgmtd_if_req_type req_type, is_inf_config* config)
{
	char cmd[512];
	char dhcp_conf[64];

	if(SILC_MGMTD_IF_REQ_DELETE == req_type)
	{
		// the if attached to a dev can be deleted
		if(config->dev[0])
		{
			sprintf(cmd, "ip link del %s", config->name);
			system(cmd);
		}
		// the if can't be deleted if it is a dev
		return 0;
	}
	else if(SILC_MGMTD_IF_REQ_ADD == req_type)
	{
		if(config->dev[0] && config->vlan > 0)
		{
			sprintf(cmd, "ip link add link %s name %s type vlan id %d", config->dev, config->name, config->vlan);
			system(cmd);
		}
	}

	if(!config->enabled)
	{
		sprintf(cmd, "ifconfig %s down", config->name);
		system(cmd);
		return 0;
	}
	// enable allmulti
	sprintf(cmd, "ifconfig %s allmulti up", config->name);
	system(cmd);

	if(config->master[0])
		sprintf(cmd, "ip link set %s master %s", config->name, config->master);
	else
		sprintf(cmd, "ip link set %s nomaster", config->name);
	system(cmd);

	//kill all dhclient tasks for this interface
	sprintf(cmd, "ps -ef|grep dhclient|grep %s|awk '{print $1}'|xargs kill", config->name);
	system(cmd);
	if(config->ipv4.origin == IS_IP_ORIGIN_DHCP || config->ipv6.origin == IS_IP_ORIGIN_DHCP)
	{
		sprintf(dhcp_conf, "/tmp/dhclient_%s.conf", config->name);
		is_gen_dhcp_conf(dhcp_conf, config->dhcp_sendname);
	}

	if(config->ipv4.enabled)
	{
		if(config->ipv4.origin == IS_IP_ORIGIN_DHCP)
		{
			sprintf(cmd, "dhclient -4 -nw -cf %s -sf %s %s", dhcp_conf, IS_DHCLIENT_SCRIPT, config->name);
		}
		else if(strlen(config->ipv4.addr) > 0)
		{
			int mask = 0xffffffff;
			int left = 32 - config->ipv4.prefixlen;
			int i = 0;
			for(i=0; i<left; i++)
			{
				mask &= ~(1<<i);
			}
			sprintf(cmd, "ifconfig %s %s netmask 0x%x", config->name, config->ipv4.addr, mask);
		}
	}
	else
	{
		sprintf(cmd, "for addr in $(ip addr show dev %s |grep -w inet|awk '{print $2}'); do ip addr del ${addr} dev %s; done", config->name, config->name);
	}
	system(cmd);

	if(config->ipv6.enabled)
	{
		if(config->ipv6.origin == IS_IP_ORIGIN_DHCP)
		{
			sprintf(cmd, "dhclient -6 -nw -cf %s -sf %s %s", dhcp_conf, IS_DHCLIENT_SCRIPT, config->name);
		}
		else if(strlen(config->ipv6.addr) > 0)
		{
			char output[256];
			int output_len= 256;
			sprintf(cmd, "ifconfig %s|grep %s|awk '{print $3}'", config->name, config->ipv6.addr);
			if(silc_mgmtd_if_exec_system_cmd(cmd, output, &output_len, 1000, silc_false) != 0)
			{
				SILC_ERR("Fail to exec cmd: %s", cmd);
			}
			else if(strncmp(output, config->ipv6.addr, strlen(config->ipv6.addr)) == 0)
			{
				// remove the ipv6 addr if present, else changing prefixlen will fail
				sprintf(cmd, "ifconfig %s del %s", config->name, output);
				system(cmd);
			}
			sprintf(cmd, "ifconfig %s %s/%d", config->name, config->ipv6.addr, config->ipv6.prefixlen);
		}
	}
	else
	{
		sprintf(cmd, "for addr in $(ip addr show dev %s |grep -w inet6|awk '{print $2}'); do ip addr del ${addr} dev %s; done", config->name, config->name);
	}
	system(cmd);

	return 0;
}

void silc_mgmtd_update_ts(char* module, void* conn_entry);

void is_mgmtd_system_mgmt_update_ts(void* conn_entry)
{
	silc_mgmtd_update_ts("sys_mgmt", conn_entry);
}

int is_mgmtd_system_mgmt_disable_if(silc_mgmtd_node* p_node_if)
{
	int str_len = 64;
	char cmd[200] = "";
	char dftgw_str[str_len];
	silc_cstr dftgw = NULL;
	silc_mgmtd_node *p_dftgw = NULL;

	sprintf(cmd, "killall -q dhclient; ifconfig %s down", IS_MGMTD_MGMT_IF_NAME);
	silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 10000, silc_false);
	return 0;

	p_dftgw = silc_mgmtd_memdb_find_sub_node(p_node_if, "ip-default-gateway");
	if(!p_dftgw)
		return 0;
	dftgw = silc_mgmtd_var_to_str(&p_dftgw->value, dftgw_str, str_len);
	if(!dftgw)
		return 0;
	sprintf(cmd, "route delete default gw %s %s", dftgw, IS_MGMTD_MGMT_IF_NAME);
	silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 10000, silc_false);
	return 0;
}

int is_mgmtd_system_mgmt_modify_if(silc_mgmtd_node* p_node, void* conn_entry)
{
	int str_len = 64;
	char cmd[200] = "";
	char addr[str_len], mask[str_len], dftgw[str_len], origin[str_len], sendname[str_len];
	silc_mgmtd_node *p_addr = NULL, *p_mask = NULL, *p_dftgw = NULL, *p_state = NULL, *p_origin = NULL, *p_sendname = NULL;
	int origin_change = 0, ip_change = 0, gw_change = 0;

	if(strcmp(p_node->name, "state") == 0 ||
			strcmp(p_node->name, "ip-address") == 0 || strcmp(p_node->name, "ip-mask") == 0 ||
			strcmp(p_node->name, "ip-default-gateway") == 0 || strcmp(p_node->name, "ip-origin") == 0)
		p_node = p_node->p_parent;

	if(strcmp(p_node->name, "interface") != 0)
		return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;

	p_state = silc_mgmtd_memdb_find_sub_node(p_node, "state");
	p_addr = silc_mgmtd_memdb_find_sub_node(p_node, "ip-address");
	p_mask = silc_mgmtd_memdb_find_sub_node(p_node, "ip-mask");
	p_dftgw = silc_mgmtd_memdb_find_sub_node(p_node, "ip-default-gateway");
	p_origin = silc_mgmtd_memdb_find_sub_node(p_node, "ip-origin");
	p_sendname = silc_mgmtd_memdb_find_sub_node(p_node, "dhcp-sendname");
	if(!p_state || !p_addr || !p_mask || !p_dftgw || !p_origin || !p_sendname)
		return IS_MGMTD_ERR_BASE_MISS_PARAM;

	if(p_state->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		if (!p_state->value.val.bool_val)
		{
			USER_OP_LOG(conn_entry, "Management interface is disabled");
			is_mgmtd_system_mgmt_disable_if(p_state->p_parent);
			return 0;
		}
		else
		{
			USER_OP_LOG(conn_entry, "Management interface is enabled");
			sprintf(cmd, "ip link show %s|grep -w UP", IS_MGMTD_MGMT_IF_NAME);
			if(silc_mgmtd_if_system(cmd) != 0)
			{
				// if the inf is re-enabled, apply all config
				origin_change = 1;
				ip_change = 1;
				gw_change = 1;
			}
		}
	}

	silc_mgmtd_var_to_str(&p_origin->value, origin, str_len);
	silc_mgmtd_var_to_str(&p_sendname->value, sendname, str_len);
	if(p_origin->tmp_value.type != SILC_MGMTD_VAR_NULL ||
			p_sendname->tmp_value.type != SILC_MGMTD_VAR_NULL)	// modified
	{
		origin_change = 1;
		silc_mgmtd_var_clear(&p_origin->tmp_value);
		silc_mgmtd_var_clear(&p_sendname->tmp_value);
		USER_OP_LOG(conn_entry, "Management interface is configured: ip origination %s dhcp-sendname %s", origin, sendname);
	}

	silc_mgmtd_var_to_str(&p_addr->value, addr, str_len);
	silc_mgmtd_var_to_str(&p_mask->value, mask, str_len);
	if(p_addr->tmp_value.type != SILC_MGMTD_VAR_NULL ||
			p_mask->tmp_value.type != SILC_MGMTD_VAR_NULL)	// modified
	{
		ip_change = 1;
		silc_mgmtd_var_clear(&p_addr->tmp_value);
		silc_mgmtd_var_clear(&p_mask->tmp_value);
		USER_OP_LOG(conn_entry, "Management interface is configured: ip %s mask %s", addr, mask);
	}

	silc_mgmtd_var_to_str(&p_dftgw->value, dftgw, str_len);
	if(p_dftgw->tmp_value.type != SILC_MGMTD_VAR_NULL)	// modified
	{
		gw_change = 1;
		silc_mgmtd_var_clear(&p_dftgw->tmp_value);
		USER_OP_LOG(conn_entry, "Management interface is configured: default gateway %s", dftgw);
	}

	if(origin_change)
	{
		system("killall -q dhclient");
		if(strcasecmp(origin, "dhcp") == 0)
		{
			char dhcp_conf[str_len];
			sprintf(dhcp_conf, "/tmp/dhclient_%s.conf", IS_MGMTD_MGMT_IF_NAME);
			is_gen_dhcp_conf(dhcp_conf, p_sendname->value.val.bool_val);
			sprintf(cmd, "dhclient -4 -nw -cf %s -sf %s %s", dhcp_conf, IS_DHCLIENT_SCRIPT, IS_MGMTD_MGMT_IF_NAME);
			system(cmd);
			return 0;
		}
		else
			ip_change = 1;
	}

	if(ip_change)
	{
		sprintf(cmd, "ifconfig %s %s netmask %s", IS_MGMTD_MGMT_IF_NAME, addr, mask);
		system(cmd);
	}

	if(gw_change)
	{
		sprintf(cmd, "if [ \"$(route|grep default|grep %s)\" ]; then route del default dev %s; fi;",
				IS_MGMTD_MGMT_IF_NAME, IS_MGMTD_MGMT_IF_NAME);
		system(cmd);
	}

	if(ip_change || gw_change)
	{
		// changing ip may delete default gw, so add it back
		sprintf(cmd, "if [ -z \"$(route|grep default|grep %s)\" ]; then route add default gw %s dev %s; fi",
				IS_MGMTD_MGMT_IF_NAME, dftgw, IS_MGMTD_MGMT_IF_NAME);
		system(cmd);
	}

	return 0;
}

#define IS_RESOLV_CONF		"/tmp/resolv.conf"

int is_mgmtd_system_mgmt_config_dns(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node *p_db_node;
	silc_mgmtd_node *p_db_sub_node;
	FILE* fp;
	char svr[64];

	fp = fopen(IS_RESOLV_CONF".bak", "w");

	p_db_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/dns");
	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
		if(p_db_sub_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		silc_mgmtd_var_to_str(&p_db_sub_node->value, svr, 64);
		if(silc_mgmtd_var_equal(&p_db_sub_node->value, &p_node->value))
		{
			if (type == SILC_MGMTD_IF_REQ_DELETE)
			{
				USER_OP_LOG(conn_entry, "DNS server %s is deleted", svr);
				continue;
			}
			else
				USER_OP_LOG(conn_entry, "DNS server %s is added", svr);
		}
		fprintf(fp, "nameserver %s\n", svr);
	}

	fflush(fp);
	fclose(fp);
	rename(IS_RESOLV_CONF".bak", IS_RESOLV_CONF);

	return 0;
}

int is_mgmtd_system_mgmt_config_permit(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node *p_db_node;
	silc_mgmtd_node *p_db_sub_node;
	silc_mgmtd_node *p_sub_node;
	char ip[64], mask[64], cmd[256];

	p_db_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/permit-enabled");
	if(!p_db_node->value.val.bool_val)
	{
		if(p_db_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		{
			USER_OP_LOG(conn_entry, "IP permit is disabled");
			system("iptables -F");
		}
		return 0;
	}
	if(p_db_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		USER_OP_LOG(conn_entry, "IP permit is enabled");
	}
	system("iptables -F");
	system("iptables -A INPUT -s localhost -j ACCEPT");

	p_db_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/permit-list");
	silc_list_for_each_entry(p_db_sub_node, &p_db_node->sub_node_list, node)
	{
		if(p_db_sub_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		silc_mgmtd_var_to_str(&p_db_sub_node->value, ip, 64);
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_db_sub_node, "mask");
		silc_mgmtd_var_to_str(&p_sub_node->value, mask, 64);
		if(silc_mgmtd_var_equal(&p_db_sub_node->value, &p_node->value))
		{
			if(type == SILC_MGMTD_IF_REQ_DELETE)
			{
				USER_OP_LOG(conn_entry, "IP Permit subnet %s/%s is deleted", ip, mask);
				continue;
			}
			else
				USER_OP_LOG(conn_entry, "IP Permit subnet %s/%s is added", ip, mask);
		}
		sprintf(cmd, "iptables -A INPUT -s %s/%s -j ACCEPT", ip, mask);
		system(cmd);
	}
	system("iptables -A INPUT -j DROP");

	return 0;
}

int is_mgmtd_system_mgmt_check_perimit(silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node;
	uint32_t ip, mask;

	ip = p_node->value.val.ipaddr_val.addr.v4;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "mask");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		mask = p_sub_node->tmp_value.val.ipaddr_val.addr.v4;
	}
	else
	{
		mask = p_sub_node->value.val.ipaddr_val.addr.v4;
	}
	if((ip & ~mask) != 0)
	{
		SILC_ERR("IP Permit subnet 0x%x and mask 0x%x mismatch", ip, mask);
		return IS_MGMTD_ERR_SYSTEM_IP_MASK_MISMATCH;
	}
	return 0;
}

int ipsec_secret_use_keyfile(silc_cstr secret_type)
{
	return (strcmp("RSA", secret_type) == 0 ||
			strcmp("ECDSA", secret_type) == 0 ||
			strcmp("BLISS", secret_type) == 0);
}

int is_mgmtd_system_mgmt_key_inuse(silc_mgmtd_node* p_req_node)
{
	silc_mgmtd_node *p_list, *p_node, *p_type, *p_key;
	silc_cstr node_id = p_req_node->value.val.string_val;
	silc_cstr secret_type;

	p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-secret");
	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		p_type = silc_mgmtd_memdb_find_sub_node(p_node, "type");
		secret_type = p_type->value.val.string_val;
		if(ipsec_secret_use_keyfile(secret_type))
		{
			p_key = silc_mgmtd_memdb_find_sub_node(p_node, "key-file");
			if(strcmp(p_key->value.val.string_val, node_id) == 0)
				return 1;
		}
	}
	return 0;
}

int is_mgmtd_system_mgmt_check_key(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	if(SILC_MGMTD_IF_REQ_CHECK_ADD == type)
	{
		silc_mgmtd_node *p_content;
		p_content = silc_mgmtd_memdb_find_sub_node(p_node, "key-content");
		if(!p_content->tmp_value.val.string_val || strlen(p_content->tmp_value.val.string_val) == 0)
		{
			SILC_ERR("Key content is null");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
	{
#if 0
		silc_mgmtd_node *p_ref;
		p_ref = silc_mgmtd_memdb_find_sub_node(p_node, "key-ref");
		if(p_ref->value.val.uint32_val > 0)
#else
		if(is_mgmtd_system_mgmt_key_inuse(p_node))
#endif
		{
			SILC_ERR("Key %s is still in use", p_node->value.val.string_val);
			return IS_MGMTD_ERR_SYSTEM_CERT_KEY_IN_USE;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_MODIFY == type)
	{
		SILC_ERR("Key is not allowed to modify");
		return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
	}
	return 0;
}

int is_mgmtd_system_mgmt_cert_inuse(silc_mgmtd_node* p_req_node)
{
	silc_mgmtd_node *p_list, *p_node, *p_cert;
	silc_cstr node_id = p_req_node->value.val.string_val;

	p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-conn");
	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		p_cert = silc_mgmtd_memdb_find_sub_node(p_node, "local-cert");
		if(strcmp(p_cert->value.val.string_val, node_id) == 0)
			return 1;
		p_cert = silc_mgmtd_memdb_find_sub_node(p_node, "peer-cert");
		if(strcmp(p_cert->value.val.string_val, node_id) == 0)
			return 1;
	}
	return 0;
}

int is_mgmtd_system_mgmt_check_cert(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	if(SILC_MGMTD_IF_REQ_CHECK_ADD == type)
	{
		silc_mgmtd_node *p_content;
		p_content = silc_mgmtd_memdb_find_sub_node(p_node, "cert-content");
		if(!p_content->tmp_value.val.string_val || strlen(p_content->tmp_value.val.string_val) == 0)
		{
			SILC_ERR("Cert content is null");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
	{
#if 0
		silc_mgmtd_node *p_ref;
		p_ref = silc_mgmtd_memdb_find_sub_node(p_node, "cert-ref");
		if(p_ref->value.val.uint32_val > 0)
#else
		if(is_mgmtd_system_mgmt_cert_inuse(p_node))
#endif
		{
			SILC_ERR("Certificate %s is still in use", p_node->value.val.string_val);
			return IS_MGMTD_ERR_SYSTEM_CERT_KEY_IN_USE;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_MODIFY == type)
	{
		SILC_ERR("Certificate is not allowed to modify");
		return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
	}
	return 0;
}

#define IS_KEY_STORE_PATH	"/data/keys"
int is_mgmtd_system_mgmt_config_key(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_req_node, void* conn_entry)
{
	silc_cstr node_id = p_req_node->value.val.string_val;
	silc_mgmtd_node *p_sub_node;
	char filepath[128], cmd[256];
	silc_cstr content;

	if(conn_entry)
		SILC_LOG("Configure key: %s, action: %s", node_id, silc_mgmtd_if_get_req_type_str(req_type));

	sprintf(filepath, "%s/%s", IS_KEY_STORE_PATH, node_id);
	if(req_type == SILC_MGMTD_IF_REQ_DELETE)
	{
		sprintf(cmd, "rm -rf %s", filepath);
		system(cmd);
		return 0;
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_req_node, "key-content");
	content = p_sub_node->value.val.string_val;
	is_write_file(filepath, content, strlen(content));

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_req_node, "key-path");
	silc_mgmtd_var_set_by_str(&p_sub_node->value, filepath);
	return 0;
}

silc_cstr is_mgmtd_system_mgmt_get_key_path(silc_cstr node_id)
{
	silc_mgmtd_node *p_node, *p_sub_node;
	char node_path[64];

	if(!node_id[0])
		return NULL;
	sprintf(node_path, "%s/key-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, node_id);
	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_node)
	{
		SILC_ERR("Fail to find node: %s", node_path);
		return NULL;
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "key-path");
	return p_sub_node->value.val.string_val;
}

#if 0
silc_mgmtd_node* is_mgmtd_system_mgmt_get_key_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node, *p_sub_node;
	char node_path[64];

	sprintf(node_path, "%s/key-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, node_id);
	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_node)
	{
		SILC_ERR("Fail to find node: %s", node_path);
		return NULL;
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "key-ref");

	return p_sub_node;
}

int is_mgmtd_system_mgmt_inc_key_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node = is_mgmtd_system_mgmt_get_key_ref(node_id);
	if(!p_node)
		return -1;

	p_node->value.val.uint32_val++;
	return 0;
}

int is_mgmtd_system_mgmt_dec_key_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node = is_mgmtd_system_mgmt_get_key_ref(node_id);
	if(!p_node)
		return -1;

	if(p_node->value.val.uint32_val > 0)
		p_node->value.val.uint32_val--;
	return 0;
}
#endif
#define IS_CERT_STORE_PATH	"/data/certs"
int is_mgmtd_system_mgmt_config_cert(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_req_node, void* conn_entry)
{
	silc_cstr node_id = p_req_node->value.val.string_val;
	silc_mgmtd_node *p_sub_node;
	char filepath[64], cmd[256];
	silc_cstr content;

	if(conn_entry)
		SILC_LOG("Configure cert: %s, action: %s", node_id, silc_mgmtd_if_get_req_type_str(req_type));

	sprintf(filepath, "%s/%s", IS_CERT_STORE_PATH, node_id);
	if(req_type == SILC_MGMTD_IF_REQ_DELETE)
	{
		sprintf(cmd, "rm -rf %s", filepath);
		system(cmd);
		return 0;
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_req_node, "cert-content");
	content = p_sub_node->value.val.string_val;
	is_write_file(filepath, content, strlen(content));

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_req_node, "cert-path");
	silc_mgmtd_var_set_by_str(&p_sub_node->value, filepath);
	return 0;
}

silc_cstr is_mgmtd_system_mgmt_get_cert_path(silc_cstr node_id)
{
	silc_mgmtd_node *p_node, *p_sub_node;
	char node_path[256];

	if(!node_id[0])
		return NULL;
	if(strlen(node_id) > 200)
		return NULL;
	sprintf(node_path, "%s/cert-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, node_id);
	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_node)
	{
		SILC_ERR("Fail to find node: %s", node_path);
		return NULL;
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "cert-path");
	return p_sub_node->value.val.string_val;
}

#if 0
silc_mgmtd_node* is_mgmtd_system_mgmt_get_cert_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node, *p_sub_node;
	char node_path[64];

	sprintf(node_path, "%s/cert-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, node_id);
	p_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_node)
	{
		SILC_ERR("Fail to find node: %s", node_path);
		return NULL;
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "cert-ref");

	return p_sub_node;
}

int is_mgmtd_system_mgmt_inc_cert_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node = is_mgmtd_system_mgmt_get_key_ref(node_id);
	if(!p_node)
		return -1;

	p_node->value.val.uint32_val++;
	return 0;
}

int is_mgmtd_system_mgmt_dec_cert_ref(silc_cstr node_id)
{
	silc_mgmtd_node *p_node = is_mgmtd_system_mgmt_get_key_ref(node_id);
	if(!p_node)
		return -1;

	if(p_node->value.val.uint32_val > 0)
		p_node->value.val.uint32_val--;
	return 0;
}
#endif

int is_mgmtd_system_mgmt_check_ipsec_conn(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node;
	silc_cstr val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "local-cert");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		val = p_sub_node->tmp_value.val.string_val;
		if(val[0] && !is_mgmtd_system_mgmt_get_cert_path(val))
		{
			SILC_ERR("local-cert %s is invalid cert", val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "local-cert2");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		val = p_sub_node->tmp_value.val.string_val;
		if(val[0] && !is_mgmtd_system_mgmt_get_cert_path(val))
		{
			SILC_ERR("local-cert2 %s is invalid cert", val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "peer-cert");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		val = p_sub_node->tmp_value.val.string_val;
		if(val[0] && !is_mgmtd_system_mgmt_get_cert_path(val))
		{
			SILC_ERR("peer-cert %s is invalid", val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "peer-cert2");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		val = p_sub_node->tmp_value.val.string_val;
		if(val[0] && !is_mgmtd_system_mgmt_get_cert_path(val))
		{
			SILC_ERR("peer-cert2 %s is invalid", val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	return 0;
}

int is_mgmtd_system_mgmt_check_ipsec_secret(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node;
	silc_cstr val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "key-file");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		val = p_sub_node->tmp_value.val.string_val;
		if(val[0] && !is_mgmtd_system_mgmt_get_key_path(val))
		{
			SILC_ERR("key-file %s is invalid", val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	return 0;
}

int is_mgmtd_system_mgmt_check_interface(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_dev, *p_vlan, *p_master;

	p_dev = silc_mgmtd_memdb_find_sub_node(p_node, "dev");
	p_vlan = silc_mgmtd_memdb_find_sub_node(p_node, "vlan");
	p_master = silc_mgmtd_memdb_find_sub_node(p_node, "master");
	if(SILC_MGMTD_IF_REQ_CHECK_ADD == type)
	{
		silc_mgmtd_node *p_sub_node;
		char node_path[128];
		// it should be a vlan inf on a valid netdev
		if(p_dev->tmp_value.type == SILC_MGMTD_VAR_NULL ||
				p_vlan->tmp_value.type == SILC_MGMTD_VAR_NULL)
		{
			SILC_ERR("Missing dev or vlan when adding interface");
			return IS_MGMTD_ERR_BASE_MISS_PARAM;
		}
		if(!p_dev->tmp_value.val.string_val[0])
		{
			SILC_ERR("Missing dev when adding interface");
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}
		sprintf(node_path, "%s/netdev-list/%s/type", IS_MGMTD_SYSTEM_MGMT_PATH, p_dev->tmp_value.val.string_val);
		p_sub_node = silc_mgmtd_memdb_find_node(node_path);
		if(!p_sub_node)
		{
			SILC_ERR("Invalid dev %s when adding interface", p_dev->tmp_value.val.string_val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		if(p_vlan->tmp_value.val.uint32_val < 2 || p_vlan->tmp_value.val.uint32_val > 4094)
		{
			SILC_ERR("Invalid vlan %u when adding interface", p_vlan->tmp_value.val.uint32_val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
	{
		// netdev can't be deleted
		if(!p_dev->value.val.string_val[0])
		{
			SILC_ERR("Interface device can't be deleted");
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}
	}
	else if(SILC_MGMTD_IF_REQ_CHECK_MODIFY == type)
	{
		// dev and vlan can't be modified when running
		if(p_dev->tmp_value.type != SILC_MGMTD_VAR_NULL &&
				!silc_mgmtd_var_equal(&p_dev->tmp_value, &p_dev->value))
		{
			SILC_ERR("Interface device can't be modified");
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}
		if(p_vlan->tmp_value.type != SILC_MGMTD_VAR_NULL &&
				!silc_mgmtd_var_equal(&p_vlan->tmp_value, &p_vlan->value))
		{
			SILC_ERR("Interface vlan can't be modified");
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}
	}

	if(p_master->tmp_value.type != SILC_MGMTD_VAR_NULL &&
			p_master->tmp_value.val.string_val[0])
	{
		silc_cstr vrf = p_master->tmp_value.val.string_val;
		char node_path[256];
		sprintf(node_path, "%s/vrf-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, vrf);
		if(NULL == silc_mgmtd_memdb_find_node(node_path))
		{
			SILC_ERR("Invalid interface master %s", vrf);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}
	return 0;
}

int is_mgmtd_system_mgmt_check_netdev(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	// netdev is not allowed to be modified when running
	return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
}

int is_mgmtd_system_mgmt_config_interface(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	is_inf_config config;
	silc_mgmtd_node* p_sub_node;

	if(conn_entry)
		SILC_LOG("Configure interface %s, OP:%s", p_node->value.val.string_val, silc_mgmtd_if_get_req_type_str(req_type));

	config.name = p_node->value.val.string_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "dev");
	config.dev = p_sub_node->value.val.string_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "master");
	config.master = p_sub_node->value.val.string_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "state");
	config.enabled = p_sub_node->value.val.bool_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "mtu");
	config.mtu = p_sub_node->value.val.uint32_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "vlan");
	config.vlan = p_sub_node->value.val.uint32_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "dhcp-sendname");
	config.dhcp_sendname = p_sub_node->value.val.bool_val;

	//ipv4
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv4-enabled");
	config.ipv4.enabled = p_sub_node->value.val.bool_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv4-origin");
	config.ipv4.origin = is_trans_ip_origin(p_sub_node->value.val.string_val);

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv4-address");
	silc_mgmtd_var_to_str(&p_sub_node->value, config.ipv4.addr, SILC_MGMTD_IF_NAME_MAX_LEN);

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv4-prefix");
	config.ipv4.prefixlen = p_sub_node->value.val.uint32_val;

	//ipv6
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv6-enabled");
	config.ipv6.enabled = p_sub_node->value.val.bool_val;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv6-origin");
	config.ipv6.origin = is_trans_ip_origin(p_sub_node->value.val.string_val);

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv6-address");
	silc_mgmtd_var_to_str(&p_sub_node->value, config.ipv6.addr, SILC_MGMTD_IF_NAME_MAX_LEN);

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "ipv6-prefix");
	config.ipv6.prefixlen = p_sub_node->value.val.uint32_val;

	return is_config_interface(req_type, &config);
}

void is_mgmtd_system_mgmt_restart_interface()
{
	silc_mgmtd_node *p_list, *p_node;
	p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/interface-list");
	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		if (p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		is_mgmtd_system_mgmt_config_interface(SILC_MGMTD_IF_REQ_MODIFY, p_node, NULL);
	}
}

int is_mgmtd_system_mgmt_config_address(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node* p_sub_node;
	char cmd[512];
	silc_cstr name = p_node->value.val.string_val;
	silc_cstr op = (SILC_MGMTD_IF_REQ_ADD == req_type) ? "add" : "del";
	silc_cstr inf;
	uint32_t prefix;
	char addr[SILC_MGMTD_IF_NAME_MAX_LEN];

	if(conn_entry)
		SILC_LOG("Configure address %s, OP:%s", name, silc_mgmtd_if_get_req_type_str(req_type));

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "address");
	silc_mgmtd_var_to_str(&p_sub_node->value, addr, SILC_MGMTD_IF_NAME_MAX_LEN);
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "prefix");
	prefix = p_sub_node->value.val.uint32_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "interface");
	inf = p_sub_node->value.val.string_val;

	sprintf(cmd, "ip addr %s %s/%d dev %s", op, addr, prefix, inf);
	system(cmd);
	return 0;
}

int is_mgmtd_system_mgmt_check_address(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node* p_sub_node;
	silc_cstr inf;
	char node_path[256];

	if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
		return 0;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "interface");
	inf = p_sub_node->tmp_value.val.string_val;
	if(!inf[0])
	{
		SILC_ERR("Interface name is empty");
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}

	sprintf(node_path, "%s/interface-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, inf);
	p_sub_node = silc_mgmtd_memdb_find_node(node_path);
	if(!p_sub_node)
	{
		SILC_ERR("Interface name %s is invalid", inf);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}

	return 0;
}

int is_mgmtd_system_mgmt_config_vrf(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node* p_sub_node;
	char cmd[512];
	silc_cstr vrf = p_node->value.val.string_val;
	uint32_t table;
	silc_bool state;

	if(conn_entry)
		SILC_LOG("Configure VRF %s, OP:%s", vrf, silc_mgmtd_if_get_req_type_str(req_type));

	if(SILC_MGMTD_IF_REQ_DELETE == req_type)
	{
		sprintf(cmd, "ip link del %s", vrf);
		system(cmd);
		return 0;
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "table");
	table = p_sub_node->value.val.uint32_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "state");
	state = p_sub_node->value.val.bool_val;

	sprintf(cmd, "ip link add %s type vrf table %d; ip link set dev %s %s;", vrf, table, vrf, state?"up":"down");
	system(cmd);
	return 0;
}

int is_mgmtd_system_mgmt_config_vrf_process(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node *p_sub_node;
	silc_cstr proc, vrf, vrf_proc=p_node->value.val.string_val;
	char cmd[256] = {0};

	if(conn_entry)
		SILC_LOG("Configure VRF Process %s, OP:%s", vrf_proc, silc_mgmtd_if_get_req_type_str(req_type));

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "process");
	proc = p_sub_node->value.val.string_val;
	// proc should be only one except ssh & web
	if(strcmp("ssh", proc) != 0 &&
			strcmp("web", proc) != 0)
	{
		if(req_type == SILC_MGMTD_IF_REQ_ADD)
		{
			p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "vrf");
			vrf = p_sub_node->value.val.string_val;
			sprintf(cmd, "echo '%s' > /tmp/%s.vrf", vrf, proc);
		}
		else
			sprintf(cmd, "rm -rf /tmp/%s.vrf", proc);
	}

	if(cmd[0])
		system(cmd);

	return 0;
}

int is_mgmtd_get_vrf_process_config(silc_cstr proc, silc_cstr* vrf_list, int len)
{
	silc_mgmtd_node *p_list, *p_node, *p_sub_node;
	int num = 0;

	p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/vrf-process-list");
	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "process");
		if(strcmp(p_sub_node->value.val.string_val, proc))
			continue;
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "vrf");
		if(vrf_list)
			vrf_list[num] = p_sub_node->value.val.string_val;
		num++;
		if(num >= len)
			break;
	}
	return num;
}

#define MAX_VRF_NUM	4
int is_mgmtd_run_vrf_process_cmd(silc_cstr proc, silc_cstr cmd)
{
	silc_cstr vrf_list[MAX_VRF_NUM] = {NULL};
	int num, i;
	char vrf_cmd[256];

	num = is_mgmtd_get_vrf_process_config(proc, vrf_list, MAX_VRF_NUM);
	if(num == 0)
	{
		silc_mgmtd_if_system_cmd_later(cmd, 1);
		return 0;
	}
	for(i=0; i<num; i++)
	{
		SILC_INFO("Run '%s' with VRF=%s", cmd, vrf_list[i]);
		sprintf(vrf_cmd, "ip vrf exec %s %s", vrf_list[i], cmd);
		// if calling system(), restart syslog will affect mgmtd client msg seq
		silc_mgmtd_if_system_cmd_later(vrf_cmd, 1);
	}

	return 0;
}

int is_mgmtd_system_mgmt_check_vrf(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_cstr vrf = p_node->value.val.string_val;
	char cmd[128], out[512];
	int len = 512, ret;

	if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
	{
		silc_mgmtd_node *p_list, *p_child, *p_sub_node;

		p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/vrf-process-list");
		silc_list_for_each_entry(p_child, &p_list->sub_node_list, node)
		{
			if(p_child->type != MEMDB_NODE_TYPE_DYNAMIC)
				continue;
			p_sub_node = silc_mgmtd_memdb_find_sub_node(p_child, "vrf");
			if(strcmp(p_sub_node->value.val.string_val, vrf) == 0)
			{
				SILC_ERR("VRF %s still has process %s", vrf, p_child->value.val.string_val);
				return IS_MGMTD_ERR_BASE_FORBIDDEN;
			}
		}
		p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/interface-list");
		silc_list_for_each_entry(p_child, &p_list->sub_node_list, node)
		{
			if(p_child->type != MEMDB_NODE_TYPE_DYNAMIC)
				continue;
			p_sub_node = silc_mgmtd_memdb_find_sub_node(p_child, "master");
			if(strcmp(p_sub_node->value.val.string_val, vrf) == 0)
			{
				SILC_ERR("VRF %s still has interface %s", vrf, p_child->value.val.string_val);
				return IS_MGMTD_ERR_BASE_FORBIDDEN;
			}
		}
		return 0;
	}

	sprintf(cmd, "ip link show | grep -w '%s:'", vrf);
	ret = silc_mgmtd_if_exec_system_cmd(cmd, out, &len, 3000, silc_false);
	if(ret == 0 && strstr(out, vrf) != NULL)
	{
		SILC_ERR("ip link %s already exists", vrf);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}
	return 0;
}

int is_mgmtd_system_mgmt_check_vrf_process(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node;
	static silc_cstr names[] = {
		"ipsec",
		"netconf",
		"ssh",
		"web",
		"dying-gasp",
		"ntp",
		"syslog",
		"phonehome",
	};
	silc_cstr proc="", vrf="";
	int i, num = sizeof(names)/sizeof(silc_cstr);
	int found = 0;

	if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
		return 0;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "process");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		proc = p_sub_node->tmp_value.val.string_val;
		for(i=0; i<num; i++)
		{
			if(strcmp(proc, names[i]) == 0)
			{
				found = 1;
				break;
			}
		}
	}
	if(!found)
	{
		SILC_ERR("VRF process %s is invalid", proc);
		return IS_MGMTD_ERR_BASE_INVALID_PARAM;
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "vrf");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		char node_path[256];
		vrf = p_sub_node->tmp_value.val.string_val;
		if(!vrf[0])
		{
			SILC_ERR("VRF name is empty");
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		sprintf(node_path, "%s/vrf-list/%s", IS_MGMTD_SYSTEM_MGMT_PATH, vrf);
		if(NULL == silc_mgmtd_memdb_find_node(node_path))
		{
			SILC_ERR("VRF name %s is invalid", vrf);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	return 0;
}

int is_mgmtd_system_mgmt_config_route(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node* p_sub_node;
	char cmd[512];
	silc_cstr route = p_node->value.val.string_val;
	silc_cstr op, dest, via, dev, metric, table;

	if(conn_entry)
		SILC_LOG("Configure Route %s, OP:%s", route, silc_mgmtd_if_get_req_type_str(req_type));

	if(SILC_MGMTD_IF_REQ_DELETE == req_type)
		op = "del";
	else
		op = "add";

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "dest");
	dest = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "via");
	via = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "dev");
	dev = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "metric");
	metric = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "table");
	table = p_sub_node->value.val.string_val;

	sprintf(cmd, "ip route %s %s", op, dest);
	if(via[0])
		sprintf(cmd+strlen(cmd), " via %s", via);
	if(dev[0])
		sprintf(cmd+strlen(cmd), " dev %s", dev);
	if(metric[0])
		sprintf(cmd+strlen(cmd), " metric %s", metric);
	if(table[0])
		sprintf(cmd+strlen(cmd), " table %s", table);

	SILC_INFO("Configure route cmd: %s", cmd);
	system(cmd);
	return 0;
}

#define IS_IPSEC_CONF		"/etc/ipsec.conf"
int is_mgmtd_system_mgmt_config_ipsec_conf(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_req_node, void* conn_entry)
{
	silc_cstr node_name = p_req_node->name;
	silc_cstr node_id = p_req_node->value.val.string_val;
	int is_conn = (strcmp(node_name, "conn-name")==0);
	silc_mgmtd_node *p_node_list, *p_node;
	FILE* fp;
	silc_cstr node_names[] = {
			"type", "keyexchange", "keyingtries", "ike", "esp", "compress", "replay-window", "dpdaction", "dpddelay", "dpdtimeout", "ikelifetime", "lifetime",
			"local-ip", "local-id", "local-id2", "local-src", "local-sub", "local-auth", "local-auth2", "local-cert", "local-cert2",
			"peer-ip", "peer-id", "peer-id2", "peer-src", "peer-sub", "peer-auth", "peer-auth2", "peer-cert", "peer-cert2",
	};
	silc_cstr opt_names[] = {
			"type", "keyexchange", "keyingtries", "ike", "esp", "compress", "replay_window", "dpdaction", "dpddelay", "dpdtimeout", "ikelifetime", "lifetime",
			"left", "leftid", "leftid2", "leftsourceip", "leftsubnet", "leftauth", "leftauth2", "leftcert", "leftcert2",
			"right", "rightid", "rightid2", "rightsourceip", "rightsubnet", "rightauth", "rightauth2", "rightcert", "rightcert2",
	};
	silc_cstr val, cert, cert_path, ocspuri, crluri, certuribase;
	int i, map_num=sizeof(node_names)/sizeof(silc_cstr);

	if(conn_entry)
		SILC_LOG("Configure ipsec conf %s : %s, action: %s", node_name, node_id, silc_mgmtd_if_get_req_type_str(req_type));

	fp = fopen(IS_IPSEC_CONF".bak", "w");
	fprintf(fp, "config setup\n");
	fprintf(fp, "\tuniqueids=no\n\n");

	//write connection
	p_node_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-conn");
	silc_list_for_each_entry(p_node, &p_node_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		if(SILC_MGMTD_IF_REQ_DELETE == req_type && is_conn &&
				strcmp(node_id, p_node->value.val.string_val) == 0)
			continue;
		if(!silc_mgmtd_memdb_find_sub_node(p_node, "local-ip")->value.val.string_val[0]
			||!silc_mgmtd_memdb_find_sub_node(p_node, "peer-ip")->value.val.string_val[0])
		{
			SILC_INFO("ipsec connection %s configuration is not complete, ignore it", node_id);
			continue;
		}

		fprintf(fp, "conn %s\n", p_node->value.val.string_val);
		for(i=0; i<map_num; i++)
		{
			val = silc_mgmtd_memdb_find_sub_node(p_node, node_names[i])->value.val.string_val;
			if(!val || !val[0])
				continue;
			if(strcmp(node_names[i], "local-cert") == 0 ||
					strcmp(node_names[i], "local-cert2") == 0 ||
					strcmp(node_names[i], "peer-cert") == 0 ||
					strcmp(node_names[i], "peer-cert2") == 0)
			{
				cert = silc_mgmtd_memdb_find_sub_node(p_node, node_names[i])->value.val.string_val;
				cert_path = is_mgmtd_system_mgmt_get_cert_path(cert);
				if(!cert_path)
				{
					SILC_ERR("Config %s %s is invalid", node_names[i], cert);
					continue;
				}
				val = cert_path;
			}
			fprintf(fp, "\t%s=%s\n", opt_names[i], val);
		}
		fprintf(fp, "\tcloseaction=restart\n\tauto=start\n\n");
	}

	//write ca
	p_node_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-ca");
	silc_list_for_each_entry(p_node, &p_node_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		if(SILC_MGMTD_IF_REQ_DELETE == req_type && !is_conn &&
				strcmp(node_id, p_node->value.val.string_val) == 0)
			continue;
		fprintf(fp, "ca %s\n", p_node->value.val.string_val);
		cert = silc_mgmtd_memdb_find_sub_node(p_node, "cacert")->value.val.string_val;
		cert_path = is_mgmtd_system_mgmt_get_cert_path(cert);
		if(!cert_path)
		{
			SILC_ERR("cacert %s is invalid", cert);
			continue;
		}
		fprintf(fp, "\tcacert=%s\n", cert_path);
		ocspuri = silc_mgmtd_memdb_find_sub_node(p_node, "ocspuri")->value.val.string_val;
		crluri = silc_mgmtd_memdb_find_sub_node(p_node, "crluri")->value.val.string_val;
		certuribase = silc_mgmtd_memdb_find_sub_node(p_node, "certuribase")->value.val.string_val;
		if(ocspuri[0])
			fprintf(fp, "\tocspuri=%s\n", ocspuri);
		else if(crluri[0])
			fprintf(fp, "\tcrluri=%s\n", crluri);
		else if(certuribase[0])
			fprintf(fp, "\tcerturibasei=%s\n", certuribase);
		else
		{
			fprintf(fp, "\tauto=ignore\n\n");
			continue;
		}
		fprintf(fp, "\tauto=add\n\n");
	}

	fflush(fp);
	fclose(fp);
	rename(IS_IPSEC_CONF".bak", IS_IPSEC_CONF);

	if(conn_entry &&
			silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-enabled")->value.val.bool_val)
		is_mgmtd_run_vrf_process_cmd("ipsec", "ipsec restart");

	return 0;
}

#define IS_IPSEC_SECRETS		"/etc/ipsec.secrets"
int is_mgmtd_system_mgmt_config_ipsec_secret(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_req_node, void* conn_entry)
{
	silc_cstr node_id = p_req_node->value.val.string_val;
	silc_mgmtd_node *p_node_list, *p_node;
	FILE* fp;
	silc_cstr host, peer, type, key, key_file, pass;

	if(conn_entry)
		SILC_LOG("Configure ipsec secret: %s, action: %s", node_id, silc_mgmtd_if_get_req_type_str(req_type));

	fp = fopen(IS_IPSEC_SECRETS".bak", "w");

	p_node_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-secret");
	silc_list_for_each_entry(p_node, &p_node_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		if(SILC_MGMTD_IF_REQ_DELETE == req_type && strcmp(node_id, p_node->value.val.string_val) == 0)
			continue;

		host = silc_mgmtd_memdb_find_sub_node(p_node, "host")->value.val.string_val;
		peer = silc_mgmtd_memdb_find_sub_node(p_node, "peer")->value.val.string_val;
		type = silc_mgmtd_memdb_find_sub_node(p_node, "type")->value.val.string_val;
		if(ipsec_secret_use_keyfile(type))
		{
			key_file = silc_mgmtd_memdb_find_sub_node(p_node, "key-file")->value.val.string_val;
			key = is_mgmtd_system_mgmt_get_key_path(key_file);
			if(!key)
			{
				SILC_ERR("key-file is invalid");
				return IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR;
			}
		}
		else
			key = silc_mgmtd_memdb_find_sub_node(p_node, "key-string")->value.val.string_val;
		pass = silc_mgmtd_memdb_find_sub_node(p_node, "passphrase")->value.val.string_val;

		fprintf(fp, "%s %s : %s %s %s\n", host, peer, type, key, pass);
	}

	fflush(fp);
	fclose(fp);
	rename(IS_IPSEC_SECRETS".bak", IS_IPSEC_SECRETS);

	if(conn_entry &&
			silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-enabled")->value.val.bool_val)
		is_mgmtd_run_vrf_process_cmd("ipsec", "ipsec restart");

	return 0;
}

int is_mgmtd_system_mgmt_check_iptables_rule(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_node)
{
	silc_mgmtd_node *p_sub_node;
	static silc_cstr table_list[] = {
			"filter",
	};
	static silc_cstr arg_blacklist[] = {
		"-C", "--check",
		"-D", "--delete",
		"-R", "--replace",
		"-L", "--list",
		"-S", "---list-rules",
		"-F", "--flush",
		"-Z", "--zero",
		"-N", "--new",
		"-X", "--delete-chain",
		"-P", "--policy",
		"-E", "--rename-chain",
		"-4", "--ipv4",
		"-6", "--ipv6",
		"-t", "--table",
		"-V", "--version",
	};
	int i, j, num;

	if(SILC_MGMTD_IF_REQ_CHECK_DELETE == type)
		return 0;

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "table");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		silc_cstr table = p_sub_node->tmp_value.val.string_val;
		num = sizeof(table_list)/sizeof(silc_cstr);
		for(i=0; i<num; i++)
		{
			if(strcmp(table, table_list[i]) == 0)
				break;
		}
		if(i == num)
		{
			SILC_ERR("Unsupported iptables table name %s", table);
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "version");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		silc_cstr version = p_sub_node->tmp_value.val.string_val;
		if(strcmp(version, "ipv4") != 0 && strcmp(version, "ipv6") != 0)
		{
			SILC_ERR("Invalid iptables version %s", version);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "rule");
	if(p_sub_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
	{
		silc_cstr rule = p_sub_node->tmp_value.val.string_val;
		silc_cstr_array* arg_list;
		silc_cstr arg;

		if(silc_mgmtd_if_multi_cmd(rule))
		{
			SILC_ERR("Invalid iptables rule '%s'", rule);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}

		arg_list = silc_cstr_split(rule, " ");
		if(!arg_list)
		{
			SILC_ERR("silc_cstr_split error");
			return IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED;
		}
		if(arg_list->length < 4)
		{
			SILC_ERR("Invalid iptables rule '%s'", rule);
			silc_cstr_array_destroy(arg_list);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
		arg = silc_cstr_array_get_quick(arg_list, 0);
		if(strcmp(arg, "-I") && strcmp(arg, "-A"))
		{
			SILC_ERR("Unsupported iptables rule command '%s'", rule);
			silc_cstr_array_destroy(arg_list);
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
		}

		num = sizeof(arg_blacklist)/sizeof(silc_cstr);
		for(i=0; i<num; i++)
		{
			for(j=0; j<arg_list->length; j++)
			{
				arg = silc_cstr_array_get_quick(arg_list, j);
				if(strcmp(arg_blacklist[i], arg) == 0)
				{
					SILC_ERR("Unsupported iptables rule opt '%s'", rule);
					silc_cstr_array_destroy(arg_list);
					return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
				}
			}
		}
		silc_cstr_array_destroy(arg_list);
	}

	return 0;
}

int is_mgmtd_get_reserved_iptables_rule_num(silc_cstr bin);

int is_mgmtd_get_inserted_iptables_rule_pos(silc_cstr name, silc_cstr version, silc_cstr table, silc_cstr chain, uint32_t priority)
{
	silc_mgmtd_node *p_list, *p_node, *p_sub_node;
	silc_cstr_array* arg_list;
	int pos;

	pos = is_mgmtd_get_reserved_iptables_rule_num(version);
	pos++;

	p_list = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/iptables-rule");
	silc_list_for_each_entry(p_node, &p_list->sub_node_list, node)
	{
		if(p_node->type != MEMDB_NODE_TYPE_DYNAMIC)
			continue;
		if(0 == strcmp(p_node->value.val.string_val, name))
		{
			// when init, just compare the previous rules because the latter rules are not inserted yet
			// when dynamic insert, it will be the last record
			break;
		}
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "version");
		if(strcmp(p_sub_node->value.val.string_val, version))
			continue;
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "table");
		if(strcmp(p_sub_node->value.val.string_val, table))
			continue;
		p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "rule");
		arg_list = silc_cstr_split(p_sub_node->value.val.string_val, " ");
		if(!arg_list)
		{
			SILC_ERR("[%s] silc_cstr_split error", __func__);
			continue;
		}
		if(strcmp(chain, silc_cstr_array_get_quick(arg_list, 1)) == 0)
		{
			p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "priority");
			if(priority < p_sub_node->value.val.uint32_val)
				pos++;
		}
		silc_cstr_array_destroy(arg_list);
	}
	return pos;
}

int is_mgmtd_system_mgmt_config_iptables_rule(silc_mgmtd_if_req_type req_type, silc_mgmtd_node* p_node, void* conn_entry)
{
	silc_mgmtd_node* p_sub_node;
	silc_cstr name = p_node->value.val.string_val;
	silc_cstr bin = "iptables";
	silc_cstr version;
	silc_cstr table;
	silc_cstr rule;
	silc_cstr chain;
	uint32_t priority;
	char cmd[512], output[256]={0};
	silc_cstr_array* arg_list;
	silc_cstr arg;
	int i, len=256;

	if(conn_entry)
		SILC_LOG("Configure iptables rule %s, OP:%s", name, silc_mgmtd_if_get_req_type_str(req_type));

	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "version");
	version = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "table");
	table = p_sub_node->value.val.string_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "priority");
	priority = p_sub_node->value.val.uint32_val;
	p_sub_node = silc_mgmtd_memdb_find_sub_node(p_node, "rule");
	rule = p_sub_node->value.val.string_val;

	if(strcmp(version, "ipv6") == 0)
		bin = "ip6tables";

	arg_list = silc_cstr_split(rule, " ");
	if(!arg_list)
	{
		SILC_ERR("[%s] silc_cstr_split error", __func__);
		return -1;
	}
	if(arg_list->length < 4)
	{
		SILC_ERR("Invalid iptables rule '%s'", rule);
		silc_cstr_array_destroy(arg_list);
		return -1;
	}

	// command & chain
	chain = silc_cstr_array_get_quick(arg_list, 1);
	if(SILC_MGMTD_IF_REQ_ADD == req_type)
	{
		int pos = is_mgmtd_get_inserted_iptables_rule_pos(p_node->value.val.string_val, version, table, chain, priority);
		sprintf(cmd, "%s -t %s -I %s %d", bin, table, chain, pos);
	}
	else
	{
		sprintf(cmd, "%s -t %s -D %s", bin, table, chain);
	}
	i = 2;
	// if rulenum present, ignore it
	arg = silc_cstr_array_get_quick(arg_list, 2);
	if(isdigit(arg[0]))
	{
		i = 3;
	}

	for(; i<arg_list->length; i++)
	{
		arg = silc_cstr_array_get_quick(arg_list, i);
		sprintf(cmd+strlen(cmd), " %s", arg);
	}
	silc_cstr_array_destroy(arg_list);

	SILC_INFO("iptables command: %s", cmd);
	if (silc_mgmtd_if_exec_system_cmd(cmd, output, &len, 1000, silc_false) != 0)
	{
		SILC_ERR("Fail to exec cmd: %s, out: %s", cmd, output);
		return 0;
	}

	return 0;
}

int is_mgmtd_match_apply_node(silc_mgmtd_node* p_req_node, char* node_name, silc_mgmtd_node **p_node)
{
	*p_node = NULL;
	if(strcmp(p_req_node->name, node_name) == 0 || strcmp(p_req_node->p_parent->name, node_name) == 0)
	{
		if(strcmp(p_req_node->name, node_name) == 0)
			*p_node = p_req_node;
		else
			*p_node = p_req_node->p_parent;
		return 1;
	}
	return 0;
}

int is_mgmtd_system_mgmt_check_req(silc_mgmtd_if_req_type type, silc_mgmtd_node* p_req_node)
{
	silc_mgmtd_node *p_node;

	if(type == SILC_MGMTD_IF_REQ_CHECK_ADD && p_req_node->value.type == SILC_MGMTD_VAR_STRING)
	{
		if(!silc_mgmtd_if_check_name(p_req_node->value.val.string_val))
		{
			SILC_ERR("New node %s name %s is invalid", p_req_node->name, p_req_node->value.val.string_val);
			return IS_MGMTD_ERR_BASE_INVALID_PARAM;
		}
	}

	if(is_mgmtd_match_apply_node(p_req_node, "permit-ip", &p_node))
		return is_mgmtd_system_mgmt_check_perimit(p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "netdev-name", &p_node))
		return is_mgmtd_system_mgmt_check_netdev(type, p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "interface-name", &p_node))
		return is_mgmtd_system_mgmt_check_interface(type, p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "key-id", &p_node))
		return is_mgmtd_system_mgmt_check_key(type, p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "cert-id", &p_node))
		return is_mgmtd_system_mgmt_check_cert(type, p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "conn-name", &p_node))
		return is_mgmtd_system_mgmt_check_ipsec_conn(type, p_node);
	else if(is_mgmtd_match_apply_node(p_req_node, "secret-name", &p_node))
		return is_mgmtd_system_mgmt_check_ipsec_secret(type, p_node);

	if(type == SILC_MGMTD_IF_REQ_CHECK_MODIFY)
	{
		if(is_mgmtd_match_apply_node(p_req_node, "vrf-name", &p_node) ||
				is_mgmtd_match_apply_node(p_req_node, "vrf-process-name", &p_node) ||
				is_mgmtd_match_apply_node(p_req_node, "address-name", &p_node) ||
				is_mgmtd_match_apply_node(p_req_node, "route-name", &p_node) ||
				is_mgmtd_match_apply_node(p_req_node, "rule-name", &p_node))
			return IS_MGMTD_ERR_BASE_NOT_SUPPORT;
	}
	else
	{
		if(is_mgmtd_match_apply_node(p_req_node, "vrf-name", &p_node))
			return is_mgmtd_system_mgmt_check_vrf(type, p_node);
		else if(is_mgmtd_match_apply_node(p_req_node, "vrf-process-name", &p_node))
			return is_mgmtd_system_mgmt_check_vrf_process(type, p_node);
		else if(is_mgmtd_match_apply_node(p_req_node, "address-name", &p_node))
			return is_mgmtd_system_mgmt_check_address(type, p_node);
		else if(is_mgmtd_match_apply_node(p_req_node, "rule-name", &p_node))
			return is_mgmtd_system_mgmt_check_iptables_rule(type, p_node);
	}

	return 0;
}

static char is_default_gw[64] = {0};

int is_mgmtd_system_mgmt_modify_dft_gw(silc_mgmtd_node *p_node, void* conn_entry)
{
	char ip[SILC_MGMTD_IF_NAME_MAX_LEN];
	char cmd[256];

	silc_mgmtd_var_to_str(&p_node->value, ip, SILC_MGMTD_IF_NAME_MAX_LEN);
	if(is_default_gw[0])
	{
		//delete the original def-gw if present
		sprintf(cmd, "if [ '$(route|grep default|grep %s)' != '' ]; then route del default gw %s; fi",
				is_default_gw, is_default_gw);
		system(cmd);
	}
	if(strcmp(ip, "0.0.0.0") != 0 && strcmp(ip, "::") != 0)
	{
		sprintf(cmd, "route add default gw %s", ip);
		system(cmd);
	}
	strcpy(is_default_gw, ip);

	if(conn_entry)
		SILC_LOG("Default gateway is modified to %s", ip);
	return 0;
}

int is_mgmtd_system_mgmt_modify_ipsec_enable(silc_mgmtd_node *p_node, void* conn_entry)
{
	if(p_node->value.val.bool_val)
		is_mgmtd_run_vrf_process_cmd("ipsec", "ipsec start");
	else
		is_mgmtd_run_vrf_process_cmd("ipsec", "ipsec stop");

	if(conn_entry)
		SILC_LOG("IPSEC is %s", p_node->value.val.bool_val ? "enabled" : "disabled");
	return 0;
}

int is_mgmtd_system_mgmt_modify(silc_mgmtd_node *p_req_node, void* conn_entry)
{
	silc_mgmtd_node *p_node;
	if(strcmp(p_req_node->name, "mgmt") == 0)
	{
		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/default-gateway");
		if(p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		{
			is_mgmtd_system_mgmt_modify_dft_gw(p_node, conn_entry);
			silc_mgmtd_var_clear(&p_node->tmp_value);
		}

		p_node = silc_mgmtd_memdb_find_node(IS_MGMTD_SYSTEM_MGMT_PATH"/ipsec-enabled");
		if(p_node->tmp_value.type != SILC_MGMTD_VAR_NULL)
		{
			is_mgmtd_system_mgmt_modify_ipsec_enable(p_node, conn_entry);
			silc_mgmtd_var_clear(&p_node->tmp_value);
		}
	}
	else if(strcmp(p_req_node->name, "default-gateway") == 0)
		return is_mgmtd_system_mgmt_modify_dft_gw(p_req_node, conn_entry);
	else if(strcmp(p_req_node->name, "ipsec-enabled") == 0)
		return is_mgmtd_system_mgmt_modify_ipsec_enable(p_req_node, conn_entry);
	else if(is_mgmtd_match_apply_node(p_req_node, "interface-name", &p_node))
		return is_mgmtd_system_mgmt_config_interface(SILC_MGMTD_IF_REQ_MODIFY, p_node, conn_entry);
	else if(is_mgmtd_match_apply_node(p_req_node, "conn-name", &p_node))
		return is_mgmtd_system_mgmt_config_ipsec_conf(SILC_MGMTD_IF_REQ_MODIFY, p_node, conn_entry);
	else if(is_mgmtd_match_apply_node(p_req_node, "ca-name", &p_node))
		return is_mgmtd_system_mgmt_config_ipsec_conf(SILC_MGMTD_IF_REQ_MODIFY, p_node, conn_entry);
	else if(is_mgmtd_match_apply_node(p_req_node, "secret-name", &p_node))
		return is_mgmtd_system_mgmt_config_ipsec_secret(SILC_MGMTD_IF_REQ_MODIFY, p_node, conn_entry);
	else if(is_mgmtd_match_apply_node(p_req_node, "key-id", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "cert-id", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "vrf-name", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "vrf-process-name", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "address-name", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "route-name", &p_node) ||
			is_mgmtd_match_apply_node(p_req_node, "rule-name", &p_node))
		return IS_MGMTD_ERR_BASE_NOT_SUPPORT;

	return 0;
}

int is_mgmtd_system_mgmt_config(silc_mgmtd_if_req_type type, void* p_db_node, void* conn_entry)
{
	silc_mgmtd_node* p_node = (silc_mgmtd_node*)p_db_node;

	if(type == SILC_MGMTD_IF_REQ_MODIFY)
	{
		is_mgmtd_system_mgmt_update_ts(conn_entry);
		if(strcmp(p_node->name, "interface") == 0 ||
				strcmp(p_node->name, "state") == 0 ||
				strcmp(p_node->name, "ip-origin") == 0 ||
				strcmp(p_node->name, "ip-address") == 0 ||
				strcmp(p_node->name, "ip-mask") == 0 ||
				strcmp(p_node->name, "ip-default-gateway") == 0)
		{
			if(!silc_mgmtd_memdb_get_product_info()->multi_eth_support)
			{//single interface
				return is_mgmtd_system_mgmt_modify_if(p_node, conn_entry);
			}
		}
		else if (strcmp(p_node->name, "mgmt") == 0
				|| strcmp(p_node->name, "permit-enabled") == 0
				|| strcmp(p_node->name, "permit-ip") == 0
				|| strcmp(p_node->name, "mask") == 0)
		{
			if(silc_mgmtd_memdb_get_product_info()->permit_ip_support)
				return is_mgmtd_system_mgmt_config_permit(type, p_node, conn_entry);
		}
		if(silc_mgmtd_memdb_get_product_info()->multi_eth_support)
		{//multiple eth interfaces in system
			return is_mgmtd_system_mgmt_modify(p_node, conn_entry);
		}
	}
	else if (type == SILC_MGMTD_IF_REQ_ADD
		|| type == SILC_MGMTD_IF_REQ_DELETE)
	{
		is_mgmtd_system_mgmt_update_ts(conn_entry);
		if (strcmp(p_node->name, "dns-ip") == 0)
			return is_mgmtd_system_mgmt_config_dns(type, p_node, conn_entry);
		else if (strcmp(p_node->name, "permit-ip") == 0)
		{
			if(silc_mgmtd_memdb_get_product_info()->permit_ip_support)
				return is_mgmtd_system_mgmt_config_permit(type, p_node, conn_entry);
		}
		else if(silc_mgmtd_memdb_get_product_info()->multi_eth_support)
		{//multiple eth interface support
			if (strcmp(p_node->name, "key-id") == 0)
				return is_mgmtd_system_mgmt_config_key(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "cert-id") == 0)
				return is_mgmtd_system_mgmt_config_cert(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "interface-name") == 0)
				return is_mgmtd_system_mgmt_config_interface(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "address-name") == 0)
				return is_mgmtd_system_mgmt_config_address(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "vrf-name") == 0)
				return is_mgmtd_system_mgmt_config_vrf(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "vrf-process-name") == 0)
				return is_mgmtd_system_mgmt_config_vrf_process(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "route-name") == 0)
				return is_mgmtd_system_mgmt_config_route(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "conn-name") == 0)
				return is_mgmtd_system_mgmt_config_ipsec_conf(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "ca-name") == 0)
				return is_mgmtd_system_mgmt_config_ipsec_conf(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "secret-name") == 0)
				return is_mgmtd_system_mgmt_config_ipsec_secret(type, p_node, conn_entry);
			else if (strcmp(p_node->name, "rule-name") == 0)
				return is_mgmtd_system_mgmt_config_iptables_rule(type, p_node, conn_entry);
		}
	}
	else  if (type == SILC_MGMTD_IF_REQ_CHECK_ADD
			|| type == SILC_MGMTD_IF_REQ_CHECK_MODIFY
			|| type == SILC_MGMTD_IF_REQ_CHECK_DELETE)
	{
		return is_mgmtd_system_mgmt_check_req(type, p_node);
	}

    return 0;
}

