
#include "silc_common.h"
#include "silc_mgmtd_interface.h"
#include "silc_mgmtd_memdb.h"
#include "silc_mgmtd_config.h"
#include "silc_mgmtd_error.h"

static silc_cstr is_mgmtd_config_get_snmp_host_cmd(void *p_cmd)
{
	static char cmd_str[200];
	silc_mgmtd_config_cmd *p_cur = (silc_mgmtd_config_cmd*)p_cmd;
	silc_cstr ver_str = silc_cstr_array_get_quick(p_cur->cmd_params[1], 1);

	if(strcmp(ver_str, "v3") == 0)
		strcpy(cmd_str, "snmp host ? ? user ? password ? ?");
	else
		strcpy(cmd_str, "snmp host ? ? community ?");
	return cmd_str;
}
#if 0
static silc_cstr is_mgmtd_config_get_tz_val_param(silc_cstr val_str, int param_idx)
{
	static char param[2][100];
	silc_cstr_array *p_arr = silc_cstr_split(val_str, "/");
	if(!p_arr || p_arr->length <= param_idx)
	{
		SILC_ERR("[%s] invalid string %s\n", __func__, val_str);
		return "error";
	}
	strcpy(param[param_idx], silc_cstr_array_get_quick(p_arr, param_idx));
	silc_cstr_array_destroy(p_arr);
	return param[param_idx];
}
#endif
static silc_cstr is_mgmtd_config_password_encrypt(silc_cstr val_str, int param_idx)
{
	static char output[256];
	int output_len = 256;
	char cmd[320];

	if (!val_str || strlen(val_str) == 0)
		return "''";

	sprintf(cmd, "echo '%s'|openssl enc -A -base64", val_str);
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &output_len, 1000, silc_false) != 0)
		return "*";
	silc_mgmtd_if_base64_escape(output);

	return output;
}

static silc_cstr is_mgmtd_config_password_decrypt(silc_cstr val_str)
{
	static char output[256];
	int output_len = 256;
	char cmd[320];
	char enc_str[256];

	strcpy(enc_str, val_str);
	silc_mgmtd_if_base64_unescape(enc_str);

	sprintf(cmd, "echo '%s'| openssl enc -base64 -d", enc_str);
	if(silc_mgmtd_if_exec_system_cmd(cmd, output, &output_len, 1000, silc_false) != 0)
		return "*";
	if(output[strlen(output)-1] == '\n')
		output[strlen(output)-1] = 0;
	return output;
}

#if 0
static inline silc_cstr is_mgmtd_config_trans_line_multi2one(silc_cstr val_str, int param_idx)
{
	static char output[65536];

	if (!val_str || strlen(val_str) == 0)
		return "''";
	is_trans_line_multi2one(val_str, output);
	return output;
}

static inline silc_cstr is_mgmtd_config_trans_line_one2multi(silc_cstr val_str)
{
	static char output[65536];

	if (!val_str || strlen(val_str) == 0)
		return "''";
	is_trans_line_one2multi(val_str, output);
	return output;
}

static inline silc_cstr is_mgmtd_config_get_node_encrypt(silc_cstr path)
{
	static char param[65536];
	silc_mgmtd_node* p_node = silc_mgmtd_memdb_find_node(path);
	if (!p_node || !p_node->value.val.string_val || strlen(p_node->value.val.string_val) == 0)
		return "''";

	snprintf(param, 65535, "'%s'", p_node->value.val.string_val);
	is_merge_base64_lines(param);

	return param;
}

static inline silc_cstr is_mgmtd_config_get_cert_encrypt(silc_cstr val_str, int param_idx)
{
	return is_mgmtd_config_get_node_encrypt("/config/system/service/https/certificate");
}

static inline silc_cstr is_mgmtd_config_get_key_encrypt(silc_cstr val_str, int param_idx)
{
	return is_mgmtd_config_get_node_encrypt("/config/system/service/https/private-key");
}
#endif
static silc_mgmtd_config_cmd_map s_is_mgmtd_common_config_2_cmds[] = {
		/* SNMP */
		{"snmp ?", {{"/config/snmp/agent/state", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp community ?", {{"/config/snmp/agent/communities/*", NULL, {4}}}},
		{"snmp community ? ?", {{"/config/snmp/agent/communities/*/state", NULL, {4}},
							    {"/config/snmp/agent/communities/*/state", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp community ? ?", {{"/config/snmp/agent/communities/*/full-access", NULL, {4}},
							    {"/config/snmp/agent/communities/*/full-access", "false,read-only,true,full-access"}}},
		{"snmp user ? password ? ?", {{"/config/snmp/agent/users/*", NULL, {4}},
									  {"/config/snmp/agent/users/*/password"},
									  {"/config/snmp/agent/users/*/auth"}}},
		{"snmp user ? ?", {{"/config/snmp/agent/users/*/state", NULL, {4}},
						   {"/config/snmp/agent/users/*/state", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp user ? ?", {{"/config/snmp/agent/users/*/full-access", NULL, {4}},
						   {"/config/snmp/agent/users/*/full-access", "false,read-only,true,full-access"}}},
		{"", {{"/config/snmp/agent/trap-hosts/*", NULL, {4}},
			  {"/config/snmp/agent/trap-hosts/*/version"},
			  {"/config/snmp/agent/trap-hosts/*/community"},
			  {"/config/snmp/agent/trap-hosts/*/password"},
			  {"/config/snmp/agent/trap-hosts/*/auth"}}, 0, is_mgmtd_config_get_snmp_host_cmd},
		{"snmp host ? ? community ?", {{"/config/snmp/agent/trap-hosts/*", NULL, {4}},
			  {"/config/snmp/agent/trap-hosts/*/version"},
			  {"/config/snmp/agent/trap-hosts/*/community"}}},
		{"snmp host ? ? user ? password ? ?", {{"/config/snmp/agent/trap-hosts/*", NULL, {4}},
			  {"/config/snmp/agent/trap-hosts/*/version"},
			  {"/config/snmp/agent/trap-hosts/*/community"},
			  {"/config/snmp/agent/trap-hosts/*/password"},
			  {"/config/snmp/agent/trap-hosts/*/auth"}}},
		{"snmp trap ?", {{"/config/snmp/trap/system", "false,system"TAG_CMD_TRANS_FALSE2NO",true,system"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/application", "false,application"TAG_CMD_TRANS_FALSE2NO",true,application"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/terminal", "false,terminal"TAG_CMD_TRANS_FALSE2NO",true,terminal"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/power", "false,power"TAG_CMD_TRANS_FALSE2NO",true,power"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/sensor", "false,sensor"TAG_CMD_TRANS_FALSE2NO",true,sensor"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/fan", "false,fan"TAG_CMD_TRANS_FALSE2NO",true,fan"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/switch", "false,switch"TAG_CMD_TRANS_FALSE2NO",true,switch"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/threshold", "false,threshold"TAG_CMD_TRANS_FALSE2NO",true,threshold"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/app-fail", "false,app-fail"TAG_CMD_TRANS_FALSE2NO",true,app-fail"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/bypass", "false,bypass"TAG_CMD_TRANS_FALSE2NO",true,bypass"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/mon-link", "false,mon-link"TAG_CMD_TRANS_FALSE2NO",true,mon-link"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/net-link", "false,net-link"TAG_CMD_TRANS_FALSE2NO",true,net-link"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"snmp trap ?", {{"/config/snmp/trap/error", "false,error"TAG_CMD_TRANS_FALSE2NO",true,error"}}, FLAG_CMD_TRANS_FALSE2NO},
//		{"snmp threshold sensor i2c-max ?", {{"/config/snmp/threshold/sensor/i2c-max"}}},
		{"snmp threshold sensor bcm-max ?", {{"/config/snmp/threshold/sensor/bcm-max"}}},
		{"snmp threshold sensor switch-max ?", {{"/config/snmp/threshold/sensor/switch-max"}}},
		{"snmp threshold sensor module-max ?", {{"/config/snmp/threshold/sensor/module-max"}}},
		{"snmp threshold sensor port-max ?", {{"/config/snmp/threshold/sensor/port-max"}}},
		{"snmp threshold fan min ?", {{"/config/snmp/threshold/fan/min"}}},
		{"snmp threshold fan max ?", {{"/config/snmp/threshold/fan/max"}}},
		/* AAA */
		{"radius ?", {{"/config/radius/static/enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"radius ?", {{"/config/radius/static/fallback", "false,local-login"TAG_CMD_TRANS_FALSE2NO",true,local-login"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"radius privilege ?", {{"/config/radius/static/privilege", "1,readonly,2,normal,3,admin"}}},
		{"radius mapped-user ?", {{"/config/radius/static/mapped-user"}}},
		{"radius retry ?", {{"/config/radius/static/retry"}}},
		{"radius server ? ip ? port ? secret ? timeout ?", {{"/config/radius/server/*", NULL, {3}},
															{"/config/radius/server/*/server-ip"},
															{"/config/radius/server/*/server-port"},
															{"/config/radius/server/*/secret"},
															{"/config/radius/server/*/timeout"}}},
		{"tacacs ?", {{"/config/tacacs/static/enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"tacacs ?", {{"/config/tacacs/static/fallback", "false,local-login"TAG_CMD_TRANS_FALSE2NO",true,local-login"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"tacacs mapped-user ?", {{"/config/tacacs/static/mapped-user"}}},
		{"tacacs timeout ?", {{"/config/tacacs/static/timeout"}}},
		{"tacacs service ?", {{"/config/tacacs/static/service"}}},
//		{"tacacs login-type ?", {{"/config/tacacs/static/login"}}},
		{"tacacs server ? ip ? port ? secret ?", {{"/config/tacacs/server/*", NULL, {3}},
											  	  {"/config/tacacs/server/*/server-ip"},
												  {"/config/tacacs/server/*/server-port"},
												  {"/config/tacacs/server/*/secret"}}},

		{"user name ? full-name ? encrypt-password ? privilege ?", {{"/config/unix/user/*", NULL, {3}},
													   {"/config/unix/user/*/full-name"},
													   {"/config/unix/user/*/shadow", NULL, {0}, is_mgmtd_config_password_encrypt, is_mgmtd_config_password_decrypt},
													   {"/config/unix/user/*/privilege", "1,readonly,2,normal,3,admin"}}},
		/* SYSTEM */
		{"session expired-time ?", {{"/config/system/mgmt/session-exp-time"}}},
		{"management eth-if ?", {{"/config/system/mgmt/interface/state", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management eth-if ip-origin ?", {{"/config/system/mgmt/interface/ip-origin"}}},
		{"management eth-if ip ?", {{"/config/system/mgmt/interface/ip-address"}}},
		{"management eth-if ip-mask ?", {{"/config/system/mgmt/interface/ip-mask"}}},
		{"management eth-if ip ? mask ?", {{"/config/system/mgmt/interface/ip-address"},
										   {"/config/system/mgmt/interface/ip-mask,255.255.255.0"}}},
		{"management eth-if default-gateway ?", {{"/config/system/mgmt/interface/ip-default-gateway"}}},
		{"management eth-if ?", {{"/config/system/mgmt/interface/dhcp-sendname", "false,dhcp-sendname"TAG_CMD_TRANS_FALSE2NO",true,dhcp-sendname"}}, FLAG_CMD_TRANS_FALSE2NO},

		{"management dns server ?", {{"/config/system/mgmt/dns/*", NULL, {4}}}},
		{"management permitted ?", {{"/config/system/mgmt/permit-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management permitted ip ? mask ?", {{"/config/system/mgmt/permit-list/*", NULL, {4}},
											  {"/config/system/mgmt/permit-list/*/mask"}}},

		{"management vrf ? table ?",	{{"/config/system/mgmt/vrf-list/*", NULL, {4}},
										{"/config/system/mgmt/vrf-list/*/table"}}},

		{"management vrf-process ? process ? vrf ?",	{{"/config/system/mgmt/vrf-process-list/*", NULL, {4}},
																		{"/config/system/mgmt/vrf-process-list/*/process"},
																		{"/config/system/mgmt/vrf-process-list/*/vrf"}}},

		{"management interface ? device ? vlan ?",  {{"/config/system/mgmt/interface-list/*", NULL, {4}},
													{"/config/system/mgmt/interface-list/*/dev"},
													{"/config/system/mgmt/interface-list/*/vlan"}}},
		{"management interface ? ?",       {{"/config/system/mgmt/interface-list/*/state", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/state", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management interface ? ?",       {{"/config/system/mgmt/interface-list/*/dhcp-sendname", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/dhcp-sendname", "false,dhcp-sendname"TAG_CMD_TRANS_FALSE2NO",true,dhcp-sendname"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management interface ? desc ?",  {{"/config/system/mgmt/interface-list/*/desc", NULL, {4}},
									  		{"/config/system/mgmt/interface-list/*/desc"}}},
		{"management interface ? master ?",{{"/config/system/mgmt/interface-list/*/master", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/master"}}},
		{"management interface ? mtu ?",   {{"/config/system/mgmt/interface-list/*/mtu", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/mtu"}}},
		{"management interface ? vlan ?",  {{"/config/system/mgmt/interface-list/*/vlan", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/vlan"}}},
		{"management interface ? ipv4 ?",  {{"/config/system/mgmt/interface-list/*/ipv4-enabled", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv4-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management interface ? ipv4 ?",  {{"/config/system/mgmt/interface-list/*/ipv4-origin", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv4-origin", "static,static,dhcp,dhcp"}}},
		{"management interface ? ipv4 ip ?", {{"/config/system/mgmt/interface-list/*/ipv4-address", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv4-address"}}},
		{"management interface ? ipv4 prefixlen ?", {{"/config/system/mgmt/interface-list/*/ipv4-prefix", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv4-prefix"}}},
		{"management interface ? ipv6 ?",  {{"/config/system/mgmt/interface-list/*/ipv6-enabled", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv6-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"management interface ? ipv6 ?",  {{"/config/system/mgmt/interface-list/*/ipv6-origin", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv6-origin", "static,static,dhcp,dhcp"}}},
		{"management interface ? ipv6 ip ?", {{"/config/system/mgmt/interface-list/*/ipv6-address", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv6-address"}}},
		{"management interface ? ipv6 prefixlen ?", {{"/config/system/mgmt/interface-list/*/ipv6-prefix", NULL, {4}},
											{"/config/system/mgmt/interface-list/*/ipv6-prefix"}}},

		{"management secondary-address ? ip ? prefixlen ? interface ?",	{{"/config/system/mgmt/address-list/*", NULL, {4}},
																{"/config/system/mgmt/address-list/*/address"},
																{"/config/system/mgmt/address-list/*/prefix"},
																{"/config/system/mgmt/address-list/*/interface"}}},

		{"management route ? dest ? via ? dev ? metric ? table ?",	{{"/config/system/mgmt/route-list/*", NULL, {4}},
														{"/config/system/mgmt/route-list/*/dest"},
														{"/config/system/mgmt/route-list/*/via"},
														{"/config/system/mgmt/route-list/*/dev"},
														{"/config/system/mgmt/route-list/*/metric"},
														{"/config/system/mgmt/route-list/*/table"}}},

		{"management default-gw ?", {{"/config/system/mgmt/default-gateway"}}},

		{"management ipsec key ? raw ?",	{{"/config/system/mgmt/key-list/*", NULL, {4}},
											{"/config/system/mgmt/key-list/*/key-content"}}},

		{"management ipsec cert ? raw ?",	{{"/config/system/mgmt/cert-list/*", NULL, {4}},
											{"/config/system/mgmt/cert-list/*/cert-content"}}},

		{"management ipsec connection ?",	{{"/config/system/mgmt/ipsec-conn/*", NULL, {4}}}},
		{"management ipsec connection ? tries ?",	{{"/config/system/mgmt/ipsec-conn/*/keyingtries", NULL, {4}},
									  				{"/config/system/mgmt/ipsec-conn/*/keyingtries"}}},
		{"management ipsec connection ? ike ?",		{{"/config/system/mgmt/ipsec-conn/*/ike", NULL, {4}},
													{"/config/system/mgmt/ipsec-conn/*/ike"}}},
		{"management ipsec connection ? esp ?",		{{"/config/system/mgmt/ipsec-conn/*/esp", NULL, {4}},
													{"/config/system/mgmt/ipsec-conn/*/esp"}}},
		{"management ipsec connection ? compress ?",	{{"/config/system/mgmt/ipsec-conn/*/compress", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/compress"}}},
		{"management ipsec connection ? replay-window ?",	{{"/config/system/mgmt/ipsec-conn/*/replay-window", NULL, {4}},
															{"/config/system/mgmt/ipsec-conn/*/replay-window"}}},
		{"management ipsec connection ? dpdaction ?",	{{"/config/system/mgmt/ipsec-conn/*/dpdaction", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/dpdaction"}}},
		{"management ipsec connection ? dpddelay ?",	{{"/config/system/mgmt/ipsec-conn/*/dpddelay", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/dpddelay"}}},
		{"management ipsec connection ? dpdtimeout ?",	{{"/config/system/mgmt/ipsec-conn/*/dpdtimeout", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/dpdtimeout"}}},
		{"management ipsec connection ? ikelifetime ?",	{{"/config/system/mgmt/ipsec-conn/*/ikelifetime", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/ikelifetime"}}},
		{"management ipsec connection ? lifetime ?",	{{"/config/system/mgmt/ipsec-conn/*/lifetime", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/lifetime"}}},
		{"management ipsec connection ? local-ip ?",	{{"/config/system/mgmt/ipsec-conn/*/local-ip", NULL, {4}},
									  					{"/config/system/mgmt/ipsec-conn/*/local-ip"}}},
		{"management ipsec connection ? local-src ?",	{{"/config/system/mgmt/ipsec-conn/*/local-src", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-src"}}},
		{"management ipsec connection ? local-sub ?",	{{"/config/system/mgmt/ipsec-conn/*/local-sub", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-sub"}}},
		{"management ipsec connection ? local-auth ?",	{{"/config/system/mgmt/ipsec-conn/*/local-auth", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-auth"}}},
		{"management ipsec connection ? local-auth2 ?",	{{"/config/system/mgmt/ipsec-conn/*/local-auth2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-auth2"}}},
		{"management ipsec connection ? local-id ?",	{{"/config/system/mgmt/ipsec-conn/*/local-id", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-id"}}},
		{"management ipsec connection ? local-id2 ?",	{{"/config/system/mgmt/ipsec-conn/*/local-id2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-id2"}}},
		{"management ipsec connection ? local-cert ?",	{{"/config/system/mgmt/ipsec-conn/*/local-cert", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-cert"}}},
		{"management ipsec connection ? local-cert2 ?",	{{"/config/system/mgmt/ipsec-conn/*/local-cert2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/local-cert2"}}},
		{"management ipsec connection ? peer-ip ?",		{{"/config/system/mgmt/ipsec-conn/*/peer-ip", NULL, {4}},
									  					{"/config/system/mgmt/ipsec-conn/*/peer-ip"}}},
		{"management ipsec connection ? peer-src ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-src", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-src"}}},
		{"management ipsec connection ? peer-sub ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-sub", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-sub"}}},
		{"management ipsec connection ? peer-auth ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-auth", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-auth"}}},
		{"management ipsec connection ? peer-auth2 ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-auth2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-auth2"}}},
		{"management ipsec connection ? peer-id ?",		{{"/config/system/mgmt/ipsec-conn/*/peer-id", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-id"}}},
		{"management ipsec connection ? peer-id2 ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-id2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-id2"}}},
		{"management ipsec connection ? peer-cert ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-cert", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-cert"}}},
		{"management ipsec connection ? peer-cert2 ?",	{{"/config/system/mgmt/ipsec-conn/*/peer-cert2", NULL, {4}},
														{"/config/system/mgmt/ipsec-conn/*/peer-cert2"}}},

		{"management ipsec ca ? cacert ? ocspuri ?",	{{"/config/system/mgmt/ipsec-ca/*", NULL, {4}},
														{"/config/system/mgmt/ipsec-ca/*/cacert"},
														{"/config/system/mgmt/ipsec-ca/*/ocspuri"}}},

		{"management ipsec secret ? type ? host ? peer ? key-string ? key-file ? pass ?",	{{"/config/system/mgmt/ipsec-secret/*", NULL, {4}},
														{"/config/system/mgmt/ipsec-secret/*/type"},
														{"/config/system/mgmt/ipsec-secret/*/host"},
														{"/config/system/mgmt/ipsec-secret/*/peer"},
														{"/config/system/mgmt/ipsec-secret/*/key-string"},
														{"/config/system/mgmt/ipsec-secret/*/key-file"},
														{"/config/system/mgmt/ipsec-secret/*/passphrase"}}},

		{"management ipsec ?", {{"/config/system/mgmt/ipsec-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},

		{"management iptables ? ? priority ? rule ? ?",	{{"/config/system/mgmt/iptables-rule/*", NULL, {4}},
														{"/config/system/mgmt/iptables-rule/*/table"},
														{"/config/system/mgmt/iptables-rule/*/priority"},
														{"/config/system/mgmt/iptables-rule/*/rule"},
														{"/config/system/mgmt/iptables-rule/*/version"}}},

		{"com speed ?", {{"/config/system/service/com/speed"}}},
		{"com terminal-type ?", {{"/config/system/service/com/termtype"}}},
		{"ssh ?", {{"/config/system/service/ssh/enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"ssh port ?", {{"/config/system/service/ssh/port"}}},
		{"web session-expired-time ?", {{"/config/system/service/web/session-timeout"}}},
		{"web http ?", {{"/config/system/service/http/enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"web http port ?", {{"/config/system/service/http/port"}}},
		{"web https ?", {{"/config/system/service/https/enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"web https port ?", {{"/config/system/service/https/port"}}},
		{"web https ssl cert raw ?", {{"/config/system/service/https/certificate"}}},
		{"web https ssl key raw ?", {{"/config/system/service/https/private-key"}}},

		{"name ?", {{"/config/system/misc/hostname"}}},
		{"login-banner ?", {{"/config/system/misc/login-banner"}}},
#if 0
		{"clock timezone ? area ?", {{"/config/system/misc/datetime/timezone", NULL, {0}, is_mgmtd_config_get_tz_val_param},
									 {"/config/system/misc/datetime/timezone", NULL, {0}, is_mgmtd_config_get_tz_val_param}}},
#else
		{"clock timezone ?", {{"/config/system/misc/datetime/timezone"}}},
#endif
		{"ntp ?", {{"/config/system/misc/datetime/ntp-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"ntp server ?", {{"/config/system/misc/datetime/ntp-server-v2/*", NULL, {5}}}},
		{"log level ?", {{"/config/system/misc/log/level", "0,emerg,1,alert,2,crit,3,err,4,warn,5,notice,6,info,7,debug"}}},
		{"log max-size ?", {{"/config/system/misc/log/max-size"}}},
		{"log remote ?", {{"/config/system/misc/log/remote-enabled", "false,enable"TAG_CMD_TRANS_FALSE2NO",true,enable"}}, FLAG_CMD_TRANS_FALSE2NO},
		{"log remote server ? port ?", {{"/config/system/misc/log/remote-server-v2/*", NULL, {5}},
										{"/config/system/misc/log/remote-server-v2/*/port"}}},
};

static silc_mgmtd_config_cmd_map** s_is_mgmtd_config_2_cmds = NULL;
static int s_is_mgmtd_config_2_cmds_cnt = 0;

int is_mgmtd_config_cmd_init()
{
	static int init_done = 0;
    silc_mgmtd_product_info* product_info = NULL;
    silc_mgmtd_config_cmd_map* config_list = NULL;
    int common_config_cnt = sizeof(s_is_mgmtd_common_config_2_cmds)/sizeof(silc_mgmtd_config_cmd_map);
	int config_cnt = 0, loop;

    if(init_done)
    	return 0;
    init_done = 1;

    product_info = silc_mgmtd_memdb_get_product_info();
    if(product_info && product_info->get_config_cmd_func &&
    		0 != product_info->get_config_cmd_func(&config_list, &config_cnt))
    {
		SILC_ERR("[%s] get_config_cmd_func error!", __func__);
		return -1;
    }

    s_is_mgmtd_config_2_cmds_cnt = common_config_cnt + config_cnt;
    s_is_mgmtd_config_2_cmds = malloc(sizeof(silc_mgmtd_config_cmd_map*) * s_is_mgmtd_config_2_cmds_cnt);
    if(!s_is_mgmtd_config_2_cmds)
    {
		SILC_ERR("[%s] malloc error!", __func__);
		init_done = 0;
		return -1;
    }

    for(loop=0; loop<common_config_cnt; loop++)
    {
    	s_is_mgmtd_config_2_cmds[loop] = &s_is_mgmtd_common_config_2_cmds[loop];
    }
    for(loop=0; loop<config_cnt; loop++)
    {
    	s_is_mgmtd_config_2_cmds[common_config_cnt+loop] = &config_list[loop];
    }

    return 0;
}

static silc_bool is_mgmtd_config_path_compare(silc_cstr def_path, silc_cstr cur_path)
{
	int loop;
	silc_cstr split_dft;
	char path_str[255];

	strcpy(path_str, def_path);
	split_dft = strstr(path_str, ",");
	if(split_dft)
		split_dft = 0;

	if(strstr(path_str, "/*") != NULL) //
	{
		silc_cstr def_item, cur_item;
		silc_bool ret = silc_false;
		silc_cstr_array *p_def_arr = silc_cstr_split(path_str, "/");
		silc_cstr_array *p_cur_arr = silc_cstr_split(cur_path, "/");
		if(!p_def_arr || !p_cur_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			goto OUT;
		}

		if(p_def_arr->length != p_cur_arr->length)
			goto OUT;

		for(loop=0; loop<p_def_arr->length; loop++)
		{
			def_item = silc_cstr_array_get_quick(p_def_arr, loop);
			cur_item = silc_cstr_array_get_quick(p_cur_arr, loop);
			if(strcmp(def_item, "*") != 0)
			{
				if(strcmp(def_item, cur_item) != 0)
					goto OUT;
			}
		}
		ret = silc_true;
OUT:
		if(p_def_arr)
			silc_cstr_array_destroy(p_def_arr);
		if(p_cur_arr)
			silc_cstr_array_destroy(p_cur_arr);
		return ret;
	}
	else
		return (strcmp(path_str, cur_path) == 0);
}

static int is_mgmtd_config_insert_to_cmd(silc_list* p_cmd_list, silc_cstr_array *p_param_arr)
{
	int loop, cmd_cnt;
	silc_mgmtd_config_cmd *p_cmd;
	silc_cstr path_str = silc_cstr_array_get_quick(p_param_arr, 0);

	if(0 != is_mgmtd_config_cmd_init())
		return -1;
	cmd_cnt = s_is_mgmtd_config_2_cmds_cnt;

	silc_list_for_each_entry(p_cmd, p_cmd_list, node)
	{
		if(strncmp(p_cmd->cmd_key_str, path_str, strlen(p_cmd->cmd_key_str)) == 0 &&
				path_str[strlen(p_cmd->cmd_key_str)] == '/')	// else will match the key with the same prefix
		{
			for(loop=1; loop<SILC_MGMTD_CONFIG_CMD_PARAM_MAX; loop++)
			{
				if(p_cmd->p_map->config_params[loop].path_str == NULL)
					break;
				if(p_cmd->cmd_params[loop])
					continue;
				if(is_mgmtd_config_path_compare(p_cmd->p_map->config_params[loop].path_str, path_str))
				{
					p_cmd->cmd_params[loop] = p_param_arr;
					return 0;
				}
			}
		}
	}

	for(loop=0;loop<cmd_cnt; loop++)
	{
		if(is_mgmtd_config_path_compare(s_is_mgmtd_config_2_cmds[loop]->config_params[0].path_str, path_str))
		{
			p_cmd = malloc(sizeof(silc_mgmtd_config_cmd) + strlen(path_str) + 1);
			if(!p_cmd)
			{
				SILC_ERR("[%s] malloc error!", __func__);
				silc_cstr_array_destroy(p_param_arr);
				return -1;
			}
			memset(p_cmd, 0, sizeof(silc_mgmtd_config_cmd));
			p_cmd->cmd_key_str = (silc_cstr)(p_cmd+1);
			strcpy(p_cmd->cmd_key_str, path_str);
			p_cmd->p_map = s_is_mgmtd_config_2_cmds[loop];
			p_cmd->cmd_params[0] = p_param_arr;
			for(loop=1; loop<SILC_MGMTD_CONFIG_CMD_PARAM_MAX; loop++)
			{
				silc_cstr dft_val;
				if(p_cmd->p_map->config_params[loop].path_str == NULL)
					break;
				dft_val = strstr(p_cmd->p_map->config_params[loop].path_str, ",");
				if(dft_val)
					p_cmd->cmd_params_dft[loop] = dft_val+1;
				if(is_mgmtd_config_path_compare(p_cmd->p_map->config_params[loop].path_str, path_str))
				{
					p_cmd->cmd_params[loop] = p_param_arr;
				}
			}
			silc_list_add_tail(&p_cmd->node, p_cmd_list);
			return 0;
		}
	}

	// if not found, destroy p_param_arr
	silc_cstr_array_destroy(p_param_arr);
	return 0;
}

static void is_mgmtd_config_free_cmd_list(silc_list *p_cmd_list)
{
	int loop;
	silc_mgmtd_config_cmd *p_cmd, *p_tmp_cmd;

	silc_list_for_each_entry_safe(p_cmd, p_tmp_cmd, p_cmd_list, node)
	{
		for(loop=1; loop<SILC_MGMTD_CONFIG_CMD_PARAM_MAX; loop++)
		{
			if(p_cmd->cmd_params[loop] &&
					p_cmd->cmd_params[loop] != p_cmd->cmd_params[0])
				silc_cstr_array_destroy(p_cmd->cmd_params[loop]);
		}
		if(p_cmd->cmd_params[0])
			silc_cstr_array_destroy(p_cmd->cmd_params[0]);
		silc_list_del(&p_cmd->node);
		free(p_cmd);
	}
}

static silc_cstr is_mgmtd_config_get_param(silc_mgmtd_config_cmd *p_cmd, int param_idx)
{
	static char param_buf[SILC_MGMTD_CONFIG_CMD_PARAM_MAX][SILC_MGMTD_CONFIG_CMD_LEN_MAX];
	static int param_buf_idx = 0;
	int loop, cur_idx;
	silc_cstr_array *p_arr = NULL;
	silc_cstr path_str, val_str, param_str = NULL;
	silc_cstr mix_param_str = param_buf[param_buf_idx];

	param_buf_idx++;
	param_buf_idx = param_buf_idx % SILC_MGMTD_CONFIG_CMD_PARAM_MAX;

	if(p_cmd->cmd_params[param_idx] == NULL)
	{
		if(p_cmd->cmd_params_dft[param_idx])
			return p_cmd->cmd_params_dft[param_idx];
		if(p_cmd->p_map->config_params[param_idx].val_param_map)
			return p_cmd->p_map->config_params[param_idx].val_param_map;
		else
			return "*";
	}

	path_str = silc_cstr_array_get_quick(p_cmd->cmd_params[param_idx], 0);
	val_str = silc_cstr_array_get_quick(p_cmd->cmd_params[param_idx], 1);

	cur_idx = p_cmd->p_map->config_params[param_idx].param_path_idx[0];
	if(p_cmd->p_map->config_params[param_idx].val_param_map)
	{
		p_arr = silc_cstr_split(p_cmd->p_map->config_params[param_idx].val_param_map, ",");
		if(!p_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			return NULL;
		}
		for(loop=0; loop<p_arr->length; loop++)
		{
			silc_cstr item = silc_cstr_array_get_quick(p_arr, loop);
			if(strcmp(item, val_str) == 0)
			{
				strcpy(mix_param_str, silc_cstr_array_get_quick(p_arr, loop+1));
				param_str = mix_param_str;
				break;
			}
			loop++;
		}
	}
	else if(cur_idx != 0)
	{
		silc_cstr item;
		p_arr = silc_cstr_split(p_cmd->cmd_key_str, "/");
		if(!p_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			return NULL;
		}
		if(cur_idx >= p_arr->length)
		{
			SILC_ERR("index:%d more than have %d", cur_idx, p_arr->length);
			silc_cstr_array_destroy(p_arr);
			return NULL;
		}
		item = silc_cstr_array_get_quick(p_arr, cur_idx);
		strcpy(mix_param_str, item);
		for(loop=1; loop<SILC_MGMTD_CONFIG_PATH_PARAM_MIX; loop++)
		{
			if(p_cmd->p_map->config_params[param_idx].param_path_idx[loop])
			{
				item = silc_cstr_array_get_quick(p_arr, p_cmd->p_map->config_params[param_idx].param_path_idx[loop]);
				sprintf(mix_param_str+strlen(mix_param_str), ".%s", item);
			}
		}
		param_str = mix_param_str;
	}
	if(p_arr)
		silc_cstr_array_destroy(p_arr);

	SILC_TRACE("[%s]    parameter: '%s', path: '%s', val: '%s', param: '%s'\n",
			__func__, p_cmd->p_map->command_str, path_str, val_str, param_str);

	if(param_str)
		return param_str;
	if(p_cmd->p_map->config_params[param_idx].val_param_func)
		return p_cmd->p_map->config_params[param_idx].val_param_func(val_str, param_idx);
	if (!val_str || strlen(val_str) == 0)
		return "''";
	else if(strstr(val_str, " ") != NULL)
	{
		if(strlen(val_str) >= (SILC_MGMTD_CONFIG_CMD_LEN_MAX-4))
		{
			SILC_ERR("val_str len:%d >= %d\n", strlen(val_str), (SILC_MGMTD_CONFIG_CMD_LEN_MAX-4));
			return NULL;
		}
		sprintf(mix_param_str, "'%s'", val_str);
		return mix_param_str;
	}
	else
		return val_str;
}

static int is_mgmtd_config_transfer_cmds(silc_list *p_cmd_list, silc_cstr cmd_buff, int buff_len)
{
	int loop;
	silc_cstr_array *p_cmd_arr;
	silc_mgmtd_config_cmd *p_cmd;
	silc_cstr param;
	char tmp_str[64];

	silc_list_for_each_entry(p_cmd, p_cmd_list, node)
	{
		silc_cstr item;
		int param_idx = 0;

		if(p_cmd->p_map->get_cmd_func)
			p_cmd->p_map->command_str = p_cmd->p_map->get_cmd_func(p_cmd);
		SILC_TRACE("[%s] command:%s\n", __func__, p_cmd->p_map->command_str);
		p_cmd_arr = silc_cstr_split(p_cmd->p_map->command_str, " ");
		if(!p_cmd_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			return -1;
		}
		for(loop=0; loop<p_cmd_arr->length; loop++)
		{
			item = silc_cstr_array_get_quick(p_cmd_arr, loop);
			if(strcmp(item, "?") == 0)
			{
				param = is_mgmtd_config_get_param(p_cmd, param_idx);
				if (!param)
				{
					SILC_ERR("is_mgmtd_config_get_param return NULL!\n");
					silc_cstr_array_destroy(p_cmd_arr);
					return -1;
				}
				silc_cstr_array_set_no_copy(p_cmd_arr, loop, param);
				param_idx++;
			}
		}
		//check whether need to transfer the false value to a 'no' cmd
		if(p_cmd->p_map->flag & FLAG_CMD_TRANS_FALSE2NO)
		{
			silc_cstr tag;
			item = silc_cstr_array_get_quick(p_cmd_arr, p_cmd_arr->length-1);
			tag = strstr(item, TAG_CMD_TRANS_FALSE2NO);
			if(tag)
			{
				strncpy(tmp_str, item, tag-item);
				tmp_str[tag-item] = 0;
				silc_cstr_array_set_no_copy(p_cmd_arr, p_cmd_arr->length-1, tmp_str);
				strcat(cmd_buff, "no ");
			}
		}
		for(loop=0; loop<p_cmd_arr->length; loop++)
		{
			item = silc_cstr_array_get_quick(p_cmd_arr, loop);
			strcat(cmd_buff, item);
			if(loop != p_cmd_arr->length-1)
				strcat(cmd_buff, " ");
		}
		strcat(cmd_buff, "\n");
		silc_cstr_array_destroy(p_cmd_arr);
	}
	if(strstr(cmd_buff, "snmp"))
		strcat(cmd_buff, "snmp apply\n");
	return 0;
}

static int is_mgmtd_config_transfer(silc_cstr diff_config, silc_cstr cmd_buff, int buff_len)
{
	int loop;
	silc_list cmd_list;
	silc_cstr config_line;
	silc_cstr_array *p_param_arr;
	silc_cstr_array *p_config_arr = silc_cstr_split(diff_config, "\n");
	if(!p_config_arr)
	{
		SILC_ERR("[%s] silc_cstr_split error!", __func__);
		return -1;
	}

	silc_list_init(&cmd_list);

	for(loop=0; loop<p_config_arr->length; loop++)
	{
		config_line = silc_cstr_array_get_quick(p_config_arr, loop);
		p_param_arr = silc_cstr_split(config_line, ",");
		if(!p_param_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			return -1;
		}
		if(p_param_arr->length > 2)
		{
			//the value str has ','
			silc_cstr val = strchr(config_line, ',')+1;
			silc_cstr_array_set_no_copy(p_param_arr, 1, val);
		}
		if(is_mgmtd_config_insert_to_cmd(&cmd_list, p_param_arr) != 0)
		{
			silc_cstr_array_destroy(p_config_arr);
			is_mgmtd_config_free_cmd_list(&cmd_list);
			return -1;
		}
	}

	is_mgmtd_config_transfer_cmds(&cmd_list, cmd_buff, buff_len);

	silc_cstr_array_destroy(p_config_arr);
	is_mgmtd_config_free_cmd_list(&cmd_list);
	return 0;
}


static int silc_mgmtd_memdb_get_running_config_from_file(silc_cstr buff, int len, silc_cstr filename)
{
//	int loop;
	char cmd[512];
	int diff_len = 1024*256;
	silc_cstr diff_buf;

	diff_buf = malloc(diff_len);
	if(!diff_buf)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		return -1;
	}

#ifndef SILC_MGMTD_LOCAL_DEBUG
	sprintf(cmd,
#if 0
			"/usr/bin/sort %s > /dev/shm/conf.templet;"
			"/usr/bin/sort %s > /dev/shm/conf.running;"
#else
			"cp -f %s /dev/shm/conf.templet;"
			"cp -f %s /dev/shm/conf.running;"
			"/usr/bin/diff /dev/shm/conf.templet /dev/shm/conf.running"
			"|grep +/config|sed \"s/+\\/config/\\/config/g\";rm -f /dev/shm/conf.*", silc_mgmtd_cfg_get_running_config_filename(), filename);
#endif
#else
	sprintf(cmd,
			"sort %s > /dev/shm/conf.templet;"
			"sort %s > /dev/shm/conf.running;"
			"diff /dev/shm/conf.templet /dev/shm/conf.running"
			"|grep \"> /config\"|sed \"s/> //g\";rm -f /dev/shm/conf.*", silc_mgmtd_cfg_get_running_config_filename(), filename);
#endif
//	printf("run cmd: %s", cmd);
	if(silc_mgmtd_if_exec_system_cmd(cmd, diff_buf, &diff_len, 5000, 0) != 0)
	{
		goto ERROR;
	}
	buff[0] = 0;
	if(is_mgmtd_config_transfer(diff_buf, buff, len) != 0)
		goto ERROR;

	free(diff_buf);
	return 0;
ERROR:
	free(diff_buf);
	return -1;
}

int silc_mgmtd_memdb_get_running_config(silc_cstr buff, int len)
{
	silc_cstr conf_running = "/dev/shm/conf.running.tmp";
	if(silc_mgmtd_cfg_save_running_config_to_file(conf_running) != 0)
		return -1;

	return silc_mgmtd_memdb_get_running_config_from_file(buff, len, conf_running);
}

/*
 *
 */
#ifndef SILC_MGMTD_LOCAL_DEBUG
#define SILC_MGMTD_CFG_INNER_PATH	"/config/inner"
#define SILC_MGMTD_CFG_EXTEND_PATH	"/config/extend"
#else
#define SILC_MGMTD_CFG_INNER_PATH	"./inner"
#define SILC_MGMTD_CFG_EXTEND_PATH	"./extend"
#endif

static int silc_mgmtd_memdb_save_file(silc_cstr buff, const silc_cstr filename)
{
	FILE* fd = 0;
	char cmd[255];
	char* devname = silc_mgmtd_get_devname();

	fd = fopen(filename, "w");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	// write configurations
	sprintf(cmd, "#Device type: %s\n", devname);
	if(fputs(cmd, fd) < 0 ||
			fputs(buff, fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	if(fclose(fd) < 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		goto ERROR_OUT;
	}

	sprintf(cmd, "md5sum %s|awk '{print $1}'|openssl enc -base64|awk '{print \"#Fingerprint \" $1 }' >> %s", filename, filename);
	system(cmd);
	return 0;

ERROR_OUT:
	if(fd)
		fclose(fd);
	return -1;
}

void silc_mgmtd_memdb_remove_ext_conf_file(silc_cstr filename)
{
	char cmd[255];

	sprintf(cmd, "rm -f %s/%s", SILC_MGMTD_CFG_EXTEND_PATH, filename);
	system(cmd);
}

int silc_mgmtd_memdb_save_ext_conf_file(silc_cstr filename, silc_cstr output_filename)
{
	int len = 4096;
	silc_cstr running_buff;
	char name[128];

	running_buff = malloc(len);
	if(!running_buff)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		return -1;
	}

	sprintf(name, "%s/%s", SILC_MGMTD_CFG_INNER_PATH, filename);

	if(silc_mgmtd_memdb_get_running_config_from_file(running_buff, len, name) != 0)
		goto ERROR;

	sprintf(name, "%s/%s", SILC_MGMTD_CFG_EXTEND_PATH, output_filename);
	if(silc_mgmtd_memdb_save_file(running_buff, name) != 0)
		goto ERROR;

	free(running_buff);
	return 0;

ERROR:
	if(running_buff)
		free(running_buff);
	return -1;
}

static int silc_mgmtd_memdb_check_file(const silc_cstr filename)
{
	char cmd[255];
	int len = 255, check_len = 255;
	char buff[len];
	char check_buff[len];
	char* devname = silc_mgmtd_get_devname();

	sprintf(cmd, "cat %s|grep \"#Device type: %s\"", filename, devname);

	if(silc_mgmtd_if_exec_system_cmd(cmd, NULL, NULL, 5000, silc_false) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_OP_INVALID_AUTHORITY, filename);
		return -1;
	}

	sprintf(cmd, "cat %s|grep \"#Fingerprint \"|awk '{print $2}'", filename);
	if(silc_mgmtd_if_exec_system_cmd(cmd, buff, &len, 5000, silc_false) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_OP_INVALID_AUTHORITY, filename);
		return -1;
	}

	sprintf(cmd, "cat %s|grep -v \"#Fingerprint \"|md5sum|awk '{print $1}'|openssl enc -base64", filename);
	if(silc_mgmtd_if_exec_system_cmd(cmd, check_buff, &check_len, 5000, silc_false) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_OP_INVALID_AUTHORITY, filename);
		return -1;
	}

//	printf("[%s]Fignerprint:%s, checked:%s\n", __func__, buff, check_buff);

	if(strcmp(buff, check_buff) != 0)
	{
		silc_mgmtd_err_set(MGMTD_ERR_OP_INVALID_AUTHORITY, filename);
		return -1;
	}
	return 0;
}

static silc_bool is_mgmtd_running_cmd_compare(silc_cstr def_cmd, silc_cstr cur_cmd)
{
	int loop;

	if(strstr(def_cmd, "?") != NULL) //
	{
		silc_cstr def_item, cur_item;
		silc_bool ret = silc_false;
		silc_cstr_array *p_def_arr = silc_cstr_split(def_cmd, " ");
		silc_cstr_array *p_cur_arr = silc_cstr_split(cur_cmd, " ");
		if(!p_def_arr || !p_cur_arr)
		{
			SILC_ERR("[%s] silc_cstr_split error!", __func__);
			goto OUT;
		}

		if(p_def_arr->length != p_cur_arr->length)
			goto OUT;

		for(loop=0; loop<p_def_arr->length; loop++)
		{
			def_item = silc_cstr_array_get_quick(p_def_arr, loop);
			cur_item = silc_cstr_array_get_quick(p_cur_arr, loop);
			if(strcmp(def_item, "?") != 0)
			{
				if(strcmp(def_item, cur_item) != 0)
					goto OUT;
			}
		}
		ret = silc_true;
OUT:
		if(p_def_arr)
			silc_cstr_array_destroy(p_def_arr);
		if(p_cur_arr)
			silc_cstr_array_destroy(p_cur_arr);
		return ret;
	}
	else
		return (strcmp(def_cmd, cur_cmd) == 0);
}

static void silc_mgmtd_single_quotes_space_convert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);
	silc_bool quote_start = silc_false;

	for(i=0; i<len; i++)
	{
		if(i+1 >= len)
		{
			if(quote_start && cstr[i] == ' ')
				cstr[i] = 28;
			return;
		}

		if(quote_start)
		{
			if(cstr[i] == '\'' && cstr[i+1] == ' ')
				quote_start = silc_false;
			else if(cstr[i] == ' ')
				cstr[i] = 28;
		}
		else
		{
			if(cstr[i] == ' ' && cstr[i+1] == '\'')
				quote_start = silc_true;
		}
	}
}

static void silc_mgmtd_single_quotes_space_revert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);

	for(i=0; i<len; i++)
	{
		if(cstr[i] == 28)
			cstr[i] = ' ';
	}
}

static int silc_mgmtd_running_cmd_write_dbconf(silc_cstr cur_cmd, silc_mgmtd_config_cmd_map* p_cmd_map, silc_cstr out_buff)
{
	int loop, loop_v, loop_p, loop_c, index = 0;
	silc_cstr def_item, cur_item;
	silc_cstr_array *p_def_arr = silc_cstr_split(p_cmd_map->command_str, " ");
	silc_cstr_array *p_cur_arr = silc_cstr_split(cur_cmd, " ");
	silc_cstr_array *p_val_arr = NULL, *p_path_arr = NULL;
	char path_prefix[255] = "";
	char buff[1024];
	silc_cstr path_val, paths[SILC_MGMTD_CONFIG_PATH_PARAM_MIX] = {0};
	silc_cstr node_name, tmp_char;
//	int fdno = fileno(fd);

//	printf("command: %s\n", cur_cmd);
	if(!p_def_arr || !p_cur_arr)
	{
		SILC_ERR("[%s] silc_cstr_split error!", __func__);
		goto OUT;
	}

	// get path prefix first
	if(strstr(p_cmd_map->config_params[0].path_str, "/*"))
	{
		for(loop=0; loop<SILC_MGMTD_CONFIG_CMD_PARAM_MAX; loop++)
		{
			if(p_cmd_map->config_params[loop].param_path_idx[0] != 0)
				break;
		}
		for(loop_c=0; loop_c<p_def_arr->length; loop_c++)
		{
			def_item = silc_cstr_array_get_quick(p_def_arr, loop_c);
			if(strcmp(def_item, "?") == 0)
			{
				if(index == loop)
				{
					silc_mgmtd_config_param *p_conf = &p_cmd_map->config_params[index];
					cur_item = silc_cstr_array_get_quick(p_cur_arr, loop_c);
					p_path_arr = silc_cstr_split(p_conf->path_str, "/");
					if(!p_path_arr)
					{
						SILC_ERR("[%s] silc_cstr_split error!", __func__);
						goto OUT;
					}
					if(p_conf->param_path_idx[1])
					{
						p_val_arr = silc_cstr_split(cur_item, ".");
						if(!p_val_arr)
						{
							SILC_ERR("[%s] silc_cstr_split error!", __func__);
							goto OUT;
						}
						for(loop_v=0; loop_v<p_val_arr->length; loop_v++)
						{
							paths[loop_v] = silc_cstr_array_get_quick(p_val_arr, loop_v);
						}
					}
					else
						paths[0] = cur_item;
					for(loop_p=0; loop_p<p_path_arr->length; loop_p++)
					{
						silc_bool done = silc_false;
						for(loop_v=0; loop_v<SILC_MGMTD_CONFIG_PATH_PARAM_MIX; loop_v++)
						{
							if(p_conf->param_path_idx[loop_v] &&
									p_conf->param_path_idx[loop_v] == loop_p)
							{
								sprintf(path_prefix+strlen(path_prefix), "/%s", paths[loop_v]);
								sprintf(out_buff+strlen(out_buff), "%s,\n", path_prefix);
								done = silc_true;
							}
						}
						if(!done)
							sprintf(path_prefix+strlen(path_prefix), "/%s", silc_cstr_array_get_quick(p_path_arr, loop_p));
					}
					silc_cstr_array_destroy(p_path_arr);
					p_path_arr = NULL;
					if(p_val_arr)
					{
						silc_cstr_array_destroy(p_val_arr);
						p_val_arr = NULL;
					}
				}
				index++;
			}
		}
	}

//	printf("[%s]cmd path prefix:%s\n", __func__, path_prefix);
	index = 0;
	for(loop_c=0; loop_c<p_def_arr->length; loop_c++)
	{
		def_item = silc_cstr_array_get_quick(p_def_arr, loop_c);
		if(strcmp(def_item, "?") == 0)
		{
			silc_mgmtd_config_param *p_conf = &p_cmd_map->config_params[index];
			cur_item = silc_cstr_array_get_quick(p_cur_arr, loop_c);
			if(p_conf->param_path_idx[0] == 0)
			{
				path_val = NULL;
				p_val_arr = NULL;
				if(p_conf->val_param_map)
				{
					silc_cstr path_map;
//					printf("val map:%s, val:%s\n", p_conf->val_param_map, cur_item);
					p_val_arr = silc_cstr_split(p_conf->val_param_map, ",");
					if(!p_val_arr)
					{
						SILC_ERR("[%s] silc_cstr_split error!", __func__);
						goto OUT;
					}
					for(loop_v=0; loop_v<p_val_arr->length/2; loop_v++)
					{
						path_map = silc_cstr_array_get_quick(p_val_arr, loop_v*2+1);
//						printf("find map cur:%s, val:%s\n", cur_item, path_val);
						if(strcmp(cur_item, path_map) == 0)
						{
							path_val = silc_cstr_array_get_quick(p_val_arr, loop_v*2);
//							printf("find map val:%s\n", path_val);
							break;
						}
					}
				}
				else if(p_conf->set_val_param)
					path_val = p_conf->set_val_param(cur_item);
				else
				{
					if(cur_item[0] == '\'')
					{
						path_val = cur_item + 1;
						path_val[strlen(path_val)-1] = 0;
						silc_mgmtd_single_quotes_space_revert(path_val);
					}
					else
						path_val = cur_item;
				}
				if(!path_val)
				{
					// value not found, so the cmd is not for this map
					break;
				}
				node_name = silc_mgmtd_if_path_get_last_name(p_conf->path_str);
				if(strstr(p_conf->path_str, "*") &&
						strcmp(p_conf->path_str, p_cmd_map->config_params[0].path_str) == 0)
					sprintf(buff, "%s", path_prefix);
				else
				{
					if(strlen(path_prefix) == 0)
					{
						sprintf(buff, "%s", p_conf->path_str);
					}
					else
						sprintf(buff, "%s/%s", path_prefix, node_name);
				}
				tmp_char = strstr(buff, ",");
				if(tmp_char) // fix default value in path
					*tmp_char = 0;
				sprintf(out_buff+strlen(out_buff), "%s,%s\n", buff, path_val);
//				printf("write to:%s,%s\n", buff, path_val);
				if(p_val_arr)
				{
					silc_cstr_array_destroy(p_val_arr);
					p_val_arr = NULL;
				}
			}
			index++;
		}
	}

OUT:
	if(p_def_arr)
		silc_cstr_array_destroy(p_def_arr);
	if(p_cur_arr)
		silc_cstr_array_destroy(p_cur_arr);
	if(p_val_arr)
		silc_cstr_array_destroy(p_val_arr);
	if(p_path_arr)
		silc_cstr_array_destroy(p_path_arr);
	return 0;
}

static int silc_mgmtd_memdb_transfer_running_to_db_file(silc_cstr running_cmd, silc_cstr out_buff)
{
	int loop, cmd_cnt;
	silc_mgmtd_config_cmd_map *p_cmd;

	if(0 != is_mgmtd_config_cmd_init())
		return -1;
	cmd_cnt = s_is_mgmtd_config_2_cmds_cnt;

	for(loop=0; loop<cmd_cnt; loop++)
	{
		p_cmd = s_is_mgmtd_config_2_cmds[loop];
		if(is_mgmtd_running_cmd_compare(p_cmd->command_str, running_cmd))
		{
			silc_mgmtd_running_cmd_write_dbconf(running_cmd, p_cmd, out_buff);
		}
	}
	return 0;
}


int silc_mgmtd_memdb_restore_ext_conf_file(silc_cstr filename, silc_cstr output_filename)
{
	FILE *fd = NULL, *out_fd = NULL;
	int out_len = 1024*256;
	silc_cstr line, out_buff;
	int len = 1024;
	char buff[len];
	char name[128];

	sprintf(name, "%s/%s", SILC_MGMTD_CFG_EXTEND_PATH, filename);

	if(silc_mgmtd_memdb_check_file(name) != 0)
		return -1;

	fd = fopen(name, "r");
	if(!fd)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		return -1;
	}
	out_buff = malloc(out_len);
	if(!out_buff)
	{
		silc_mgmtd_err_set(MGMTD_ERR_CLIB, filename);
		fclose(fd);
		return -1;
	}
	*out_buff = 0;

	line = fgets(buff, len, fd);
	while(line)
	{
		if (strncmp(line, "#", 1) != 0)
		{
			if(line[strlen(line)-1] == '\n')
				line[strlen(line)-1] = 0;
			silc_mgmtd_single_quotes_space_convert(line);
			if(strncmp(line, "no ", 3) == 0)
			{
				//transfer 'no' cmd to a map cmd
				strcpy(line, line+3);
				strcat(line, TAG_CMD_TRANS_FALSE2NO);
			}
			silc_mgmtd_memdb_transfer_running_to_db_file(line, out_buff);
		}
		line = fgets(buff, len, fd);
	}

	sprintf(name, "%s/%s", SILC_MGMTD_CFG_INNER_PATH, output_filename);
	out_fd = fopen(name, "w");
	fputs(out_buff, out_fd);

	free(out_buff);
	fclose(out_fd);
	fclose(fd);
	return 0;
}
