#ifndef _SILC_CLI_CMD_SHOW_SYSTEM_H_
#define _SILC_CLI_CMD_SHOW_SYSTEM_H_

#include "silc_mgmtd_interface.h"
#include "silc_cli_terminal.h"
#include "silc_cli_types.h"
#include "silc_cli_common.h"

#if 0
/*
 * format idx,username,type,time,ip,port;idx,username,type,time,ip,port;...
 */
static inline int is_cli_show_session_runtime()
{
	silc_cstr formats[] = { "%-5s", "%-20s", "%-8s", "%-22s", "%-18s", "%-s" };
	silc_cstr names[] = { "ID", "Username", "Type", "LoginTime", "LoginIP", "LoginPort" };

    return silc_cli_show_complex_string_query("/status/system/session/sessions-list",
    		"Connected Sessions:", names, formats, 6);
}

static inline int is_cli_show_session_configured()
{
	return silc_cli_show_simple_string_query("/config/system/session/exp-time-sec", "Session Expired Time is %s seconds.");
}
#endif
/*
 * configuration
 */
static inline void is_cli_show_mgmt_if_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr format = "%-20s: %-20s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "MacAddr", "Port", "IPOrigin", "IPAddress", "IPMask", "Default-Gateway", "DHCPSendName" };
	silc_cstr nodes[] = { "mac", "state", "ip-origin", "ip-address", "ip-mask", "ip-default-gateway", "dhcp-sendname" };
	silc_cstr trans_state[] = { "false", "down", "true", "up"};
	silc_cstr trans_enable[] = { "false", "off", "true", "on"};
	silc_cstr trans_null_ip[] = { "0.0.0.0", "none", "", "none" };
	silc_cstr* trans[] = { NULL, trans_state, trans_enable, trans_null_ip, trans_null_ip, trans_null_ip, trans_enable };
	int trans_num[] = { 0, 2, 2, 2, 2, 2, 2 };

	silc_cli_l1tree_list_display(p_node,
			"Interface Configurations:", titles,
			nodes, names, format, trans, trans_num, sizeof(nodes)/sizeof(silc_cstr), NULL);
}

static inline void is_cli_show_mgmt_if_list_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-16s", "%-8s", "%-16s", "%-6s", "%-6s", "%-6s", "%-8s",
							"%-6s", "%-8s", "%-16s", "%-8s",
							"%-6s", "%-8s", "%-40s", "%-8s", };
	silc_cstr names[] = { "Inf", "Desc", "Dev", "Master", "State", "MTU", "VLAN", "SndName",
						"[IPV4", "Origin", "Addr", "Subnet]",
						"[IPV6", "Origin", "Addr", "Subnet]", };
	silc_cstr nodes[] = { NULL, "desc", "dev", "master", "state", "mtu", "vlan", "dhcp-sendname",
						"ipv4-enabled", "ipv4-origin", "ipv4-address", "ipv4-prefix",
						"ipv6-enabled", "ipv6-origin", "ipv6-address", "ipv6-prefix", };
	silc_cstr trans_state[] = { "false", "down", "true", "up" };
	silc_cstr trans_enable[] = { "false", "off", "true", "on"};
	silc_cstr trans_null_ipv4[] = { "0.0.0.0", "none", "", "none" };
	silc_cstr trans_null_ipv6[] = { "::/0", "none", "", "none" };
	silc_cstr* trans[] = { NULL, NULL, NULL, NULL, trans_state, NULL, NULL, trans_enable,
						trans_enable, NULL, trans_null_ipv4, NULL,
						trans_enable, NULL, trans_null_ipv6, NULL, };
	int trans_num[] = { 0, 0, 0, 0, 2, 0, 0, 2,
						2, 0, 2, 0,
						2, 0, 2, 0, };

	silc_cli_l2tree_display(p_node,
			"Interface List:", nodes, names, formats, trans, trans_num, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_addr_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-40s", "%-8s", "%-16s" };
	silc_cstr names[] = { "Name", "Addr", "Subnet", "Inf" };
	silc_cstr nodes[] = { NULL, "address", "prefix", "interface" };

	silc_cli_l2tree_display(p_node,
			"Secondary Address List:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_def_gw_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr trans_null_ip[] = { "0.0.0.0", "none", "::", "none", "", "none" };
	silc_cli_simple_node_display(p_node, "Default Gateway:", trans_null_ip, 3);
}

static inline void is_cli_show_mgmt_vrf_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-8s", "%-8s" };
	silc_cstr names[] = { "Name", "Table", "State" };
	silc_cstr nodes[] = { NULL, "table", "state" };
	silc_cstr trans_state[] = { "false", "down", "true", "up" };
	silc_cstr* trans[] = { NULL, NULL, trans_state };
	int trans_num[] = { 0, 0, 2 };

	silc_cli_l2tree_display(p_node,
			"VRF List:", nodes, names, formats, trans, trans_num, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_vrf_process_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-24s", "%-16s", "%-16s" };
	silc_cstr names[] = { "Name", "Process", "VRF" };
	silc_cstr nodes[] = { NULL, "process", "vrf" };

	silc_cli_l2tree_display(p_node,
			"VRF Process List:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_route_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-24s", "%-24s", "%-24s", "%-12s", "%-12s", "%-8s" };
	silc_cstr names[] = { "Name", "Destination", "Via", "Dev", "Metric", "Table" };
	silc_cstr nodes[] = { NULL, "dest", "via", "dev", "metric", "table" };

	silc_cli_l2tree_display(p_node,
			"Route List:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_key_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-32s" };
	silc_cstr names[] = { "Key-ID" };
	silc_cstr nodes[] = { NULL };

	silc_cli_l2tree_display(p_node,
			"Key List:", nodes, names, formats, NULL, NULL, 1);
}

static inline void is_cli_show_mgmt_cert_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-32s" };
	silc_cstr names[] = { "Cert-ID" };
	silc_cstr nodes[] = { NULL };

	silc_cli_l2tree_display(p_node,
			"Certificate List:", nodes, names, formats, NULL, NULL, 1);
}

static inline void is_cli_show_mgmt_ipsec_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr trans_enable[] = { "false", "off", "true", "on"};
	silc_cli_simple_node_display(p_node, "IPSEC state:", trans_enable, 2);
}

static inline void is_cli_show_mgmt_ipsec_conn_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr names[] = { "Name", "Type", "Exchg", "Tries", "IKE", "ESP", "Compress", "Replay-window", "DPD Action", "DPD Delay", "DPD Timeout", "IKE Lifetime", "Lifetime",
						"Local IP", "Local Auth", "Local Auth2", "Local ID", "Local ID2", "Local Src", "Local Sub", "Local Cert", "Local Cert2",
						"Peer IP", "Peer Auth", "Peer Auth2", "Peer ID", "Peer ID2", "Peer Src", "Peer Sub", "Peer Cert", "Peer Cert2" };
	silc_cstr nodes[] = { NULL, "type", "keyexchange", "keyingtries", "ike", "esp", "compress", "replay-window", "dpdaction", "dpddelay", "dpdtimeout", "ikelifetime", "lifetime",
						"local-ip", "local-auth", "local-auth2", "local-id", "local-id2", "local-src", "local-sub", "local-cert", "local-cert2",
						"peer-ip", "peer-auth", "peer-auth2", "peer-id", "peer-id2", "peer-src", "peer-sub", "peer-cert", "peer-cert2" };
#if 0
	silc_cstr formats[] = { "%-8s", "%-8s", "%-6s", "%-10s", "%-24s", "%-24s",
							"%-16s", "%-8s", "%-8s", "%-8s", "%-8s", "%-16s", "%-16s", "%-8s", "%-8s",
							"%-16s", "%-8s", "%-8s", "%-8s", "%-8s", "%-16s", "%-16s", "%-8s", "%-8s" };
	silc_cli_l2tree_display(p_node,
			"IPSEC Connection Configurations:", nodes, names, formats, NULL, NULL, 24);
#else
	silc_mgmtd_if_node *p_sub_node, *p_ss_node;
	int i, node_num=sizeof(nodes)/sizeof(silc_cstr);

	silc_cli_print("IPSEC Connection:\n");
	if(silc_list_empty(&p_node->sub_node_list))
	{
		silc_cli_print("None\n\n");
		return;
	}
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		silc_cli_print("%-16s : %s\n", names[0], p_sub_node->name);
		for(i=1; i<node_num; i++)
		{
			p_ss_node = silc_mgmtd_if_req_find_node(p_sub_node, nodes[i]);
			if(p_ss_node && p_ss_node->val_str && p_ss_node->val_str[0])
			{
				silc_cli_print("%-16s : %s\n", names[i], p_ss_node->val_str);
			}
		}
		silc_cli_print("\n");
	}
#endif
}

static inline void is_cli_show_mgmt_ipsec_ca_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-16s", "%-32s", "%-32s", "%-32s" };
	silc_cstr names[] = { "Name", "CaCert", "OCSPUri", "CRLUri", "BaseUri" };
	silc_cstr nodes[] = { NULL, "cacert", "ocspuri", "crluri", "certuribase" };

	silc_cli_l2tree_display(p_node,
			"IPSEC CA:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_ipsec_secret_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-16s", "%-16s", "%-16s", "%-32s", "%-32s", "%-32s" };
	silc_cstr names[] = { "Name", "Type", "Host", "Peer", "Key-String", "Key-File", "Passphase" };
	silc_cstr nodes[] = { NULL, "type", "host", "peer", "key-string", "key-file", "passphrase" };

	silc_cli_l2tree_display(p_node,
			"IPSEC Secret:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_iptables_rule_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-16s", "%-16s", "%-16s", "%-16s", "%-s" };
	silc_cstr names[] = { "Name", "Version", "Table", "Priority", "Rule" };
	silc_cstr nodes[] = { NULL, "version", "table", "priority", "rule" };

	silc_cli_l2tree_display(p_node,
			"Iptables Rule:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_dns_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-20s" };
	silc_cstr names[] = { "IP" };
	silc_cstr nodes[] = { NULL };

	return silc_cli_l2tree_display(p_node,
			"DNS Server:", nodes, names, formats, NULL, NULL, 1);
}

static inline void is_cli_show_mgmt_permit_state_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr trans[] = { "false", "disable", "true", "enable"};

	silc_cli_simple_node_display(p_node, "Permitted IP Filter:", trans, 2);
}

static inline void is_cli_show_mgmt_permit_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-20s", "%-20s" };
	silc_cstr names[] = { "IP", "Mask" };
	silc_cstr nodes[] = { NULL, "mask" };

	return silc_cli_l2tree_display(p_node,
			"Permitted IP List:", nodes, names, formats, NULL, NULL, sizeof(nodes)/sizeof(silc_cstr));
}

static inline void is_cli_show_mgmt_if_status(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%s" };
	silc_cstr names[] = { "======== Interface Info ========" };
	silc_cstr nodes[] = { "info" };

	silc_cli_l2tree_display(p_node,
			"", nodes, names, formats, NULL, NULL, 1);
}

static inline void is_cli_show_mgmt_dhcp_status(silc_mgmtd_if_node* p_node)
{
	silc_mgmtd_if_node *p_sub_node;

	silc_cli_print("======== DHCP Status ========\n");
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, "state") == 0)
		{
			if(strcmp(p_sub_node->val_str, "true") == 0)
				silc_cli_print("DHCP is running\n\n");
			else
				silc_cli_print("DHCP is not started\n\n");
		}
		else if(strcmp(p_sub_node->name, "leases") == 0)
		{
			silc_cli_print("DHCP leases:\n%s\n", p_sub_node->val_str);
		}
	}
}

static inline void is_cli_show_mgmt_ipsec_status(silc_mgmtd_if_node* p_node)
{
	silc_mgmtd_if_node *p_sub_node;

	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, "connection") == 0)
		{
			silc_cli_print("======== IPSEC Connection Status ========\n%s", p_sub_node->val_str);
		}
	}
}

static inline void is_cli_show_mgmt_iptables_status(silc_mgmtd_if_node* p_node)
{
	silc_mgmtd_if_node *p_sub_node;

	silc_cli_print("======== Iptables Status ========\n");
	silc_list_for_each_entry(p_sub_node, &p_node->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, "iptables-s") == 0)
		{
			silc_cli_print("## iptables -S ##\n%s\n", p_sub_node->val_str);
		}
		else if(strcmp(p_sub_node->name, "iptables-l") == 0)
		{
			silc_cli_print("## iptables -L --line-numbers ##\n%s\n", p_sub_node->val_str);
		}
		else if(strcmp(p_sub_node->name, "ip6tables-s") == 0)
		{
			silc_cli_print("## ip6tables -S ##\n%s\n", p_sub_node->val_str);
		}
		else if(strcmp(p_sub_node->name, "ip6tables-l") == 0)
		{
			silc_cli_print("## ip6tables -L --line-numbers ##\n%s\n", p_sub_node->val_str);
		}
	}
}

static inline void is_cli_show_mgmt_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "interface-list") == 0)
			is_cli_show_mgmt_if_status(p_node);
		else if(strcmp(p_node->name, "dhcp") == 0)
			is_cli_show_mgmt_dhcp_status(p_node);
		else if(silc_cli_get_product_info()->multi_eth_support)
		{
			if(strcmp(p_node->name, "iptables") == 0)
				is_cli_show_mgmt_iptables_status(p_node);
			else if(strcmp(p_node->name, "ipsec") == 0)
				is_cli_show_mgmt_ipsec_status(p_node);
		}
	}
}

static inline void is_cli_show_mgmt_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;
	int permit_ip_support = 1;
	int dns_support = 1;


	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(!silc_cli_get_product_info()->multi_eth_support)
		{
			if(strcmp(p_node->name, "interface") == 0)
				is_cli_show_mgmt_if_configured(p_node);
		}
		else
		{
		if(strcmp(p_node->name, "interface-list") == 0)
			is_cli_show_mgmt_if_list_configured(p_node);
		else if(strcmp(p_node->name, "address-list") == 0)
			is_cli_show_mgmt_addr_configured(p_node);
		else if(strcmp(p_node->name, "default-gateway") == 0)
			is_cli_show_mgmt_def_gw_configured(p_node);
		else if(strcmp(p_node->name, "vrf-list") == 0)
			is_cli_show_mgmt_vrf_configured(p_node);
		else if(strcmp(p_node->name, "vrf-process-list") == 0)
			is_cli_show_mgmt_vrf_process_configured(p_node);
		else if(strcmp(p_node->name, "route-list") == 0)
			is_cli_show_mgmt_route_configured(p_node);
		else if(strcmp(p_node->name, "key-list") == 0)
			is_cli_show_mgmt_key_configured(p_node);
		else if(strcmp(p_node->name, "cert-list") == 0)
			is_cli_show_mgmt_cert_configured(p_node);
		else if(strcmp(p_node->name, "ipsec-enabled") == 0)
			is_cli_show_mgmt_ipsec_configured(p_node);
		else if(strcmp(p_node->name, "ipsec-conn") == 0)
			is_cli_show_mgmt_ipsec_conn_configured(p_node);
		else if(strcmp(p_node->name, "ipsec-ca") == 0)
			is_cli_show_mgmt_ipsec_ca_configured(p_node);
		else if(strcmp(p_node->name, "ipsec-secret") == 0)
			is_cli_show_mgmt_ipsec_secret_configured(p_node);
		else if(strcmp(p_node->name, "iptables-rule") == 0)
			is_cli_show_mgmt_iptables_rule_configured(p_node);
		}

		if(silc_cli_get_product_info()->dns_support_func)
		{
			dns_support = silc_cli_get_product_info()->dns_support_func();
		}
		if(dns_support)
		{
			if(strcmp(p_node->name, "dns") == 0)
				is_cli_show_mgmt_dns_configured(p_node);
		}

		if(silc_cli_get_product_info()->permit_ip_support_func)
		{
			permit_ip_support = silc_cli_get_product_info()->permit_ip_support_func();
		}
		if(permit_ip_support)
		{
			if(strcmp(p_node->name, "permit-enabled") == 0)
				is_cli_show_mgmt_permit_state_configured(p_node);
			else if(strcmp(p_node->name, "permit-list") == 0)
				is_cli_show_mgmt_permit_configured(p_node);
		}
	}
}

static inline void is_cli_show_service_configured(silc_mgmtd_if_node* p_node, const silc_cstr info)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "State", "Port" };
	silc_cstr nodes[] = { "enabled", "port" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr* trans[] = { trans_state, NULL };
	int trans_num[] = { 2, 0 };

	silc_cli_l1tree_list_display(p_node, info, titles,
			nodes, names, format, trans, trans_num, 2, NULL);
}

static inline void is_cli_show_ssh_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	is_cli_show_service_configured(p_rsp->p_node_root, "SSH Configuration:");
}

static inline void is_cli_show_telnet_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	is_cli_show_service_configured(p_rsp->p_node_root, "Telnet Configuration:");
}

static inline void is_cli_show_web_sess_configured(silc_mgmtd_if_node* p_node, const silc_cstr info)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Expired Time"};
	silc_cstr nodes[] = { "session-timeout"};
	silc_cstr* trans[] = { NULL };
	int trans_num[] = { 0 };

	silc_cli_l1tree_list_display(p_node, info, titles,
			nodes, names, format, trans, trans_num, 1, NULL);
}

static inline void is_cli_show_web_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_mgmtd_if_node *p_node;
	int show_http = 1;

	if(silc_cli_get_product_info()->show_http_func)
	{
		show_http = silc_cli_get_product_info()->show_http_func();
	}

	silc_list_for_each_entry(p_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_node->name, "web") == 0)
			is_cli_show_web_sess_configured(p_node, "Session Configuration:");
		if(show_http)
		{
			if(strcmp(p_node->name, "http") == 0)
				is_cli_show_service_configured(p_node, "HTTP Configuration:");
			else if(strcmp(p_node->name, "https") == 0)
				is_cli_show_service_configured(p_node, "HTTPS Configuration:");
		}
	}
}

static inline void is_cli_show_ntp_server_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-32s" };
	silc_cstr names[] = { "Host" };
	silc_cstr nodes[] = { NULL };

	return silc_cli_l2tree_display(p_node,
			"NTP Server:", nodes, names, formats, NULL, NULL, 1);
}

static inline void is_cli_show_ntp_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "NTP" };
	silc_cstr nodes[] = { "ntp-enabled" };
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr* trans[] = { trans_state };
	int trans_num[] = { 2 };
	silc_mgmtd_if_node* p_sub_node;

	silc_cli_l1tree_list_display(p_rsp->p_node_root, "NTP Configuration:", titles,
			nodes, names, format, trans, trans_num, 1, NULL);

	silc_list_for_each_entry(p_sub_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, "ntp-server-v2") == 0)
		{
			is_cli_show_ntp_server_configured(p_sub_node);
			break;
		}
	}
}

static inline void is_cli_show_log_server_configured(silc_mgmtd_if_node* p_node)
{
	silc_cstr formats[] = { "%-32s", "%-8s" };
	silc_cstr names[] = { "Host", "Port" };
	silc_cstr nodes[] = { NULL, "port" };

	silc_cli_l2tree_display(p_node,
			"Log Remote Server:", nodes, names, formats, NULL, NULL, 2);
}

static inline void is_cli_show_log_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value"};
	silc_cstr names[] = { "Level", "Max Size", "Remote Log"};
	silc_cstr nodes[] = { "level", "max-size", "remote-enabled"};
	silc_cstr trans_level[] = { "0", "emerg", "1", "alert", "2", "crit", "3", "err",
								"4", "warn", "5", "notice", "6", "info", "7", "debug"};
	silc_cstr trans_state[] = { "false", "disable", "true", "enable"};
	silc_cstr* trans[] = { trans_level, NULL, trans_state };
	int trans_num[] = { 8, 0, 2 };
	silc_mgmtd_if_node* p_sub_node;

	silc_cli_l1tree_list_display(p_rsp->p_node_root, "Log Configuration:", titles,
			nodes, names, format, trans, trans_num, 3, NULL);

	silc_list_for_each_entry(p_sub_node, &p_rsp->p_node_root->sub_node_list, node)
	{
		if(strcmp(p_sub_node->name, "remote-server-v2") == 0)
		{
			is_cli_show_log_server_configured(p_sub_node);
			break;
		}
	}
}

static inline void is_cli_show_com_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr format = "%-20s: %-8s";
	silc_cstr titles[] = { "Attribute", "Value" };
	silc_cstr names[] = { "Speed", "Terminal Type" };
	silc_cstr nodes[] = { "speed", "termtype" };

	silc_cli_l1tree_list_display(p_rsp->p_node_root, "Com Console Configuration:", titles,
			nodes, names, format, NULL, NULL, 2, NULL);
}

static inline void is_cli_show_dev_name_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cli_simple_node_display(p_rsp->p_node_root, "Device Name:", NULL, 0);
}

static inline void is_cli_show_dev_login_banner_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	// this may be multi-lines, need to do unescape
	if(p_rsp->p_node_root->val_str)
		silc_mgmtd_if_str_unescape(p_rsp->p_node_root->val_str, p_rsp->p_node_root->val_str);
	silc_cli_simple_node_display(p_rsp->p_node_root, "Device Login Banner:", NULL, 0);
}

static inline void is_cli_show_session_configured_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cli_simple_node_display(p_rsp->p_node_root, "Session Expired Time(s):", NULL, 0);
}

static inline void is_cli_show_session_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr formats[] = { "%-5s", "%-20s", "%-8s", "%-22s", "%-18s", "%-s" };
	silc_cstr names[] = { "ID", "Username", "Type", "LoginTime", "LoginIP", "LoginPort" };
	silc_cstr nodes[] = { NULL, "user", "type", "login-time", "login-ip", "login-port" };

	silc_cli_l2tree_display(p_rsp->p_node_root,
			"Connected Sessions:", nodes, names, formats, NULL, NULL, 6);
}

static inline void is_cli_show_cpu_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr formats[] = { "%-8s", "%-10s", "%-10s", "%-10s" };
	silc_cstr names[] = { "ID", "User", "Sys", "Idle" };
	silc_cstr nodes[] = { NULL, "user", "sys", "idle" };

	silc_cli_l2tree_display(p_rsp->p_node_root,
			"CPU Usage(%):", nodes, names, formats, NULL, NULL, 4);
}

static inline void is_cli_show_memory_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr formats[] = { "%-10s", "%-10s", "%-10s", "%-10s" };
	silc_cstr names[] = { "Title", "Total", "Used", "Free" };
	silc_cstr nodes[] = { NULL, "total", "used", "free" };

	silc_cli_l2tree_display(p_rsp->p_node_root,
			"Memory Usage(Byte):", nodes, names, formats, NULL, NULL, 4);
}

static inline void is_cli_show_storage_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cstr formats[] = { "%-20s", "%-10s", "%-10s", "%-10s" };
	silc_cstr names[] = { "Path", "Total", "Used", "Free" };
	silc_cstr nodes[] = { NULL, "total", "used", "free" };
	silc_cstr trans_path[] = { "*var*images", "/var/images", "*var*log", "/var/log", "*config", "/config"};
	silc_cstr* trans[] = { trans_path, NULL, NULL, NULL };
	int trans_num[] = { 3, 0, 0, 0 };

	silc_cli_l2tree_display(p_rsp->p_node_root,
			"Storage Usage(KByte):", nodes, names, formats, trans, trans_num, 4);
}

static inline void is_cli_show_clock_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cli_simple_node_display(p_rsp->p_node_root, "Current Clock:", NULL, 0);
}

static inline void is_cli_show_uptime_status_cb(silc_mgmtd_if_rsp* p_rsp)
{
	silc_cli_simple_node_display(p_rsp->p_node_root, "System Uptime:", NULL, 0);
}

#endif
