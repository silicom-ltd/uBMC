/* this file is created by silc_mgmtd_inst_memdb_init_gen.py */ 

#include "silc_mgmtd_error.h"
#include "silc_mgmtd_config.h"
#include "is_mgmtd_func_def.h"
#include <dlfcn.h>

/* define memdb node info */ 
static silc_mgmtd_node_info s_silc_mgmtd_node_list[] = {
{"/config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/status", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/stats", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/notify", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/config/system", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "2", is_mgmtd_system_upgrade, 10},
{"/config/system/mgmt", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/session-exp-time", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "600", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "on", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/mac", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_MACADDR, "00:E0:ED:15:50:FF", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/ip-origin", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "static", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/ip-address", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "192.168.1.254", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/ip-mask", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "255.255.255.0", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/ip-default-gateway", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "192.168.1.1", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface/dhcp-sendname", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/key-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/key-list/key-id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/key-list/key-id/key-content", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING_LINES, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/key-list/key-id/key-path", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/cert-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/cert-list/cert-id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/cert-list/cert-id/cert-content", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING_LINES, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/cert-list/cert-id/cert-path", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/netdev-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/netdev-list/netdev-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/netdev-list/netdev-name/type", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "link/ether", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-list/vrf-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-list/vrf-name/table", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "10", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-list/vrf-name/state", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "on", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-process-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-process-list/vrf-process-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-process-list/vrf-process-name/process", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/vrf-process-list/vrf-process-name/vrf", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/desc", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/state", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "off", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/dev", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/master", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/mtu", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1500", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/vlan", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "0", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/dhcp-sendname", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv4-enabled", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv4-origin", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "static", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv4-address", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv4-prefix", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "24", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv6-enabled", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv6-origin", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "static", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv6-address", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV6ADDR, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/interface-list/interface-name/ipv6-prefix", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "64", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/address-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/address-list/address-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/address-list/address-name/address", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPADDR, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/address-list/address-name/prefix", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "24", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/address-list/address-name/interface", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/default-gateway", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPADDR, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name/dest", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name/via", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name/table", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name/dev", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/route-list/route-name/metric", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/type", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "tunnel", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/keyexchange", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "ikev2", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/keyingtries", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/ike", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/esp", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/compress", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/replay-window", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "0", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/dpdaction", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/dpddelay", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/dpdtimeout", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/ikelifetime", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/lifetime", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-ip", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-auth", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "psk", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-auth2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-id", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-id2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-src", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-sub", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-cert", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/local-cert2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-ip", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-auth", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "psk", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-auth2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-id", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-id2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-src", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-sub", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-cert", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-conn/conn-name/peer-cert2", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca/ca-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca/ca-name/cacert", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca/ca-name/ocspuri", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca/ca-name/crluri", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-ca/ca-name/certuribase", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/host", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/peer", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/type", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "PSK", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/key-string", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/key-file", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-secret/secret-name/passphrase", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/ipsec-enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/dns", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/dns/dns-ip", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPADDR, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule/rule-name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule/rule-name/version", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "ipv4", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule/rule-name/table", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "filter", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule/rule-name/priority", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "0", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/iptables-rule/rule-name/rule", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/permit-enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/permit-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/permit-list/permit-ip", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "192.168.1.1", is_mgmtd_system_mgmt_config, 10},
{"/config/system/mgmt/permit-list/permit-ip/mask", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPV4ADDR, "255.255.255.255", is_mgmtd_system_mgmt_config, 10},
{"/config/system/service", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/com", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/speed", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "115200", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/data", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "8", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/parity", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "odd", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/stopbits", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/hwflowctrl", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/swflowctrl", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_service_config, 10},
{"/config/system/service/com/termtype", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "vt100", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "22", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/host-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/host-key-pub", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/dsa-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/dsa-key-pub", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/ecdsa-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/ecdsa-key-pub", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/ed25519-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/ed25519-key-pub", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/rsa-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/ssh/rsa-key-pub", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_HEX, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/web", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/web/session-path", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "/tmp/silc-web-sessions", is_mgmtd_system_service_config, 10},
{"/config/system/service/web/session-timeout", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "900", is_mgmtd_system_service_config, 10},
{"/config/system/service/http", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/http/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_service_config, 10},
{"/config/system/service/http/port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "80", is_mgmtd_system_service_config, 10},
{"/config/system/service/https", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/https/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_system_service_config, 10},
{"/config/system/service/https/port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "443", is_mgmtd_system_service_config, 10},
{"/config/system/service/https/certificate", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING_LINES, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/https/private-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING_LINES, "", is_mgmtd_system_service_config, 10},
{"/config/system/service/telnet", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_service_config, 10},
{"/config/system/service/telnet/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_service_config, 10},
{"/config/system/service/telnet/port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "21", is_mgmtd_system_service_config, 10},
{"/config/system/misc", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/hostname", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "localhost", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/login-banner", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING_LINES, "Welcome to Silicom IS Platform", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/timezone", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "Etc/UTC", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/ntp-enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/ntp-server-v2", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/ntp-server-v2/host", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "pool.ntp.org", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/ntp-server-v2/host/key", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "0", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/datetime/ntp-server-v2/host/version", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "4", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/level", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "5", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/max-size", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "5", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/remote-enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/remote-server-v2", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/remote-server-v2/host", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_misc_config, 10},
{"/config/system/misc/log/remote-server-v2/host/port", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "514", is_mgmtd_system_misc_config, 10},
{"/status", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/status/system", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/session", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index/user", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index/type", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index/login-time", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index/login-ip", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/session/index/login-port", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_system_status, 5},
{"/status/system/cpu", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/cpu/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/cpu/name/user", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/cpu/name/sys", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/cpu/name/idle", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/memory", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/memory/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/memory/name/total", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/memory/name/used", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/memory/name/free", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/storage", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/storage/path", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/storage/path/total", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/storage/path/used", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/storage/path/free", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/interface-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/interface-list/interface", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "", is_mgmtd_system_status, 5},
{"/status/system/mgmt/interface-list/interface/info", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/dhcp", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/dhcp/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/dhcp/leases", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/iptables", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/iptables/iptables-s", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/iptables/iptables-l", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/iptables/ip6tables-s", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/iptables/ip6tables-l", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/ipsec", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_status, 5},
{"/status/system/mgmt/ipsec/connection", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/time", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/uptime", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/running-config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/config-file-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/status/system/config-saved", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_system_status, 5},
{"/status/system/upgrade-percent", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_system_status, 5},
{"/status/system/event", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_status, 5},
{"/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/action/system", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/init-config", MEMDB_NODE_TYPE_BOOT, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/check-upgrade", MEMDB_NODE_TYPE_BOOT, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/switch-software", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/generate-ssh-key", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/set-date", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_DATE, "None", is_mgmtd_system_action, 5},
{"/action/system/set-time", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_TIME, "None", is_mgmtd_system_action, 5},
{"/action/system/set-datetime", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/save-config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/save-config-as", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/load-default-config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/load-config-from", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/get-config-list", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/remove-config-file", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/reset-log", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/create-log-dump", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/remove-log-dump", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/halt", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/reboot", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/reboot-force", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/reset", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/check-is-admin", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/reset-sys-evt", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/set-notify-path", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/type", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/username", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/ip", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/privilege", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_UINT32, "None", is_mgmtd_system_action, 5},
{"/action/system/set-connect-login/protocol", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_system_action, 5},
{"/config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/config/radius", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_radius_upgrade, 5},
{"/config/radius/static", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_radius_static_config, 10},
{"/config/radius/static/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_radius_static_config, 10},
{"/config/radius/static/fallback", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_radius_static_config, 10},
{"/config/radius/static/privilege", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_radius_static_config, 10},
{"/config/radius/static/retry", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_radius_static_config, 10},
{"/config/radius/static/mapped-user", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "is_default_radius", is_mgmtd_radius_static_config, 10},
{"/config/radius/server", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_radius_server_config, 10},
{"/config/radius/server/server-id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "", is_mgmtd_radius_server_config, 10},
{"/config/radius/server/server-id/server-ip", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_radius_server_config, 10},
{"/config/radius/server/server-id/server-port", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1812", is_mgmtd_radius_server_config, 10},
{"/config/radius/server/server-id/secret", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_radius_server_config, 10},
{"/config/radius/server/server-id/timeout", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "3", is_mgmtd_radius_server_config, 10},
{"/config/tacacs", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_tacacs_upgrade, 5},
{"/config/tacacs/static", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/enabled", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/fallback", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/service", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "silc-system", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/timeout", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "5", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/mapped-user", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "is_default_tacacs", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/static/login", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "login", is_mgmtd_tacacs_static_config, 10},
{"/config/tacacs/server", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_tacacs_server_config, 10},
{"/config/tacacs/server/server-id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "", is_mgmtd_tacacs_server_config, 10},
{"/config/tacacs/server/server-id/server-ip", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_tacacs_server_config, 10},
{"/config/tacacs/server/server-id/server-port", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "49", is_mgmtd_tacacs_server_config, 10},
{"/config/tacacs/server/server-id/secret", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_tacacs_server_config, 10},
{"/config/unix", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_unix_upgrade, 5},
{"/config/unix/static", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_unix_static_config, 10},
{"/config/unix/static/uid-base", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "2000", is_mgmtd_unix_static_config, 10},
{"/config/unix/user", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id/full-name", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id/passwd", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id/shadow", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id/privilege", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_unix_user_config, 10},
{"/config/unix/user/id/uid", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "0", is_mgmtd_unix_user_config, 10},
{"/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/action/aaa", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/add-default-admin", MEMDB_NODE_TYPE_BOOT, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/reset-admin-passwd", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/change-passwd", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/change-passwd/user", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/change-passwd/curr-passwd", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_aaa_action, 10},
{"/action/aaa/change-passwd/new-passwd", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_READONLY, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_aaa_action, 10},
{"/config", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/config/snmp", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_UINT32, "1", is_mgmtd_snmp_upgrade, 10},
{"/config/snmp/agent", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/listen-port", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "161", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/communities", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/communities/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "public", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/communities/name/state", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/communities/name/full-access", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "customer", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users/name/state", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users/name/auth", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "sha", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users/name/full-access", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "false", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/users/name/password", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_IPADDR, "127.0.0.1", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/state", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_BOOL, "true", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/port", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "162", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/version", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "v2c", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/community", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "public", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/auth", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_snmp_config, 10},
{"/config/snmp/agent/trap-hosts/ip/password", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_STRING, "", is_mgmtd_snmp_config, 10},
{"/config/snmp/threshold", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor/i2c-max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "90", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor/bcm-max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "90", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor/switch-max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "90", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor/module-max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "90", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/sensor/port-max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "90", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/fan", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/fan/min", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "2000", is_mgmtd_threshold_config, 5},
{"/config/snmp/threshold/fan/max", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_UINT32, "40000", is_mgmtd_threshold_config, 5},
{"/status", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/status/snmp", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/state", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/communities", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/communities/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/communities/name/full-access", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/users", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/users/name", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/users/name/full-access", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_BOOL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts/ip", MEMDB_NODE_TYPE_TEMPLATE, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts/ip/version", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts/ip/community", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts/ip/auth", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/trap-hosts/ip/passwd", MEMDB_NODE_TYPE_TEMPLATE_SUB, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/status/snmp/engine-id", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_STRING, "None", is_mgmtd_snmp_status, 10},
{"/action", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},
{"/action/snmp", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_action, 10},
{"/action/snmp/apply", MEMDB_NODE_TYPE_BOOT, SILC_MGMTD_IF_LEVEL_ADMIN, SILC_MGMTD_VAR_NULL, "None", is_mgmtd_snmp_action, 10},
{"/notify", MEMDB_NODE_TYPE_STATIC, SILC_MGMTD_IF_LEVEL_NORMAL, SILC_MGMTD_VAR_NULL, "None", NULL, 0},

};

/* define memdb node info */ 
static silc_mgmtd_cberr_info s_silc_mgmtd_cberr_list[] = {
{"/", IS_MGMTD_ERR_BASE_NOT_SUPPORT, "Operation is not supported"},
{"/", IS_MGMTD_ERR_BASE_FORBIDDEN, "Operation is forbidden"},
{"/", IS_MGMTD_ERR_BASE_INTERNAL_NODE_ERR, "Internal error"},
{"/", IS_MGMTD_ERR_BASE_NODE_NOT_EXIST, "Internal error, node doesn't exist"},
{"/", IS_MGMTD_ERR_BASE_MISS_PARAM, "Parameters missing"},
{"/", IS_MGMTD_ERR_BASE_INVALID_PARAM, "Invalid parameter"},
{"/", IS_MGMTD_ERR_BASE_EXEC_FAILED, "Internal system error"},
{"/", IS_MGMTD_ERR_BASE_GET_INFO_FAILED, "Internal system error, failed to retrieve information"},
{"/", IS_MGMTD_ERR_BASE_CREATE_NOTIFY, "Not enough memory"},
{"/", IS_MGMTD_ERR_BASE_SEND_NOTIFY, "Internal system error, failed to send notification"},
{"/", IS_MGMTD_ERR_BASE_ADD_RSP_NODE, "Not enough memory"},
{"/", IS_MGMTD_ERR_BASE_CSTR_SPLIT_FAILED, "Not enough memory"},
{"/", IS_MGMTD_ERR_BASE_MALLOC_FAILED, "Not enough memory"},
{"/", IS_MGMTD_ERR_BASE_NOT_ENOUGH_BUF, "Not enough memory"},
{"/", IS_MGMTD_ERR_SYSTEM_CERT_KEY_IN_USE, "Certificate/key file is in use"},
{"/", IS_MGMTD_ERR_SYSTEM_CERT_KEY_MISMATCH, "Certificate and key are mismatched"},
{"/", IS_MGMTD_ERR_SYSTEM_DISK_FULL, "Disk space is not enough"},
{"/", IS_MGMTD_ERR_SYSTEM_IP_MASK_MISMATCH, "IP and subnet mask are mismatched"},
{"/", IS_MGMTD_ERR_SYSTEM_TERMINAL_TYPE_INVALID, "Invalid terminal type, support xterm/vt100/vt102/vt200/vt220/ansi/linux"},
{"/", IS_MGMTD_ERR_SYSTEM_COM_SPEED_INVALID, "Invalid COM speed, support 9600/38400/115200"},
{"/", IS_MGMTD_ERR_SYSTEM_SET_COM_FAILED, "Set COM configuration failed"},
{"/", IS_MGMTD_ERR_AAA_USER_NAME_LEN, "Invalid user name length, must be between 2 - 30"},
{"/", IS_MGMTD_ERR_AAA_USER_NAME_CHAR, "Invalid character for user name, only [a~z][0~9][_][-] are allowed"},
{"/", IS_MGMTD_ERR_AAA_USER_FORBIDDEN, "User name is reserved by system"},
{"/", IS_MGMTD_ERR_AAA_USER_NO_PRIVIL, "The current user has not enough privilege for this operation"},
{"/", IS_MGMTD_ERR_AAA_PASSWD_LEN_6_40, "Invalid password length, must be 6-40"},
{"/", IS_MGMTD_ERR_AAA_PASSWD_LEN_7_40, "Invalid password length, must be 7-40"},
{"/", IS_MGMTD_ERR_AAA_PASSWD_TOO_WEAK, "Password is too weak"},
{"/", IS_MGMTD_ERR_AAA_PRIVIL_INVALID, "Invalid user privilege"},
{"/", IS_MGMTD_ERR_AAA_PRIVIL_FORBIDDEN, "The operation is not allowed for the current user's privilege"},
{"/", IS_MGMTD_ERR_AAA_MAPPED_USER_NOTALLOWED, "The mapped user can't be an admin"},
{"/", IS_MGMTD_ERR_AAA_MAPPED_USER_INUSE, "This user is a Radius/TACACS+ mapped user"},
{"/", IS_MGMTD_ERR_AAA_RADIUS_TAC_CONFLICT, "RADIUS and TACACS can't be both enabled, please disable the other one."},
{"/", IS_MGMTD_ERR_AAA_AUTH_FAIL, "Password error or user not found"},
{"/", IS_MGMTD_ERR_AAA_SERVER_DUPLICATED, "Server with the same ip and port already exists"},
{"/config/snmp", IS_MGMTD_ERR_SNMP_PASSWD_INVALID, "SNMP password is too short, need at least 8 characters"},
{"/config/snmp", IS_MGMTD_ERR_SNMP_AUTH_INVALID, "SNMP auth should be md5 or sha"},
{"/config/snmp", IS_MGMTD_ERR_SNMP_COMMUNITY_NAME_INVALID, "SNMP community name should only contain a~z/A~Z/0~9/_."},
{"/config/snmp", IS_MGMTD_ERR_SNMP_USER_NAME_INVALID, "SNMP user name should only contain a~z/A~Z/0~9/_."},
};

/* define memdb node info */ 
static silc_mgmtd_ref_info s_silc_mgmtd_ref_list[] = {
};

#define PRODUCT_LIB		"/usr/lib/libprod_mgmtd_inst.so"

static silc_mgmtd_product_info* s_silc_mgmtd_product_info = NULL;

silc_mgmtd_product_info* silc_mgmtd_memdb_get_product_info()
{
	return s_silc_mgmtd_product_info;
}

static silc_mgmtd_product_info* load_product_info()
{
	struct stat fs;
	void *dl_handle;
	char *filename = PRODUCT_LIB;

	if ((stat(filename, &fs)) != 0 || !(fs.st_mode & S_IFREG))
	{
		SILC_ERR("%s: File does not exist.", filename);
		return NULL;
	}

	if ((dl_handle = dlopen(filename, RTLD_NOW)) == NULL)
	{
		SILC_ERR("%s: dlopen %s", filename, dlerror());
		return NULL;
	}

	s_silc_mgmtd_product_info = (silc_mgmtd_product_info*)dlsym(dl_handle, "mgmtd_product_info");
	if (s_silc_mgmtd_product_info == NULL)
	{
		SILC_ERR("%s: dlsym %s", filename, dlerror());
		dlclose(dl_handle);
		return NULL;
	}

	silc_mgmtd_if_product_info_set(s_silc_mgmtd_product_info->product_name,
			s_silc_mgmtd_product_info->product_id,
			s_silc_mgmtd_product_info->vendor_list,
			s_silc_mgmtd_product_info->vendor_cnt);

	return s_silc_mgmtd_product_info;
}

/* define memdb init */ 
int is_mgmtd_memdb_init() 
{
    int loop;
    silc_mgmtd_node_info* p_info;
    silc_mgmtd_cberr_info* p_err;
    silc_mgmtd_ref_info* p_ref;
    silc_mgmtd_product_info* product_info;

    silc_mgmtd_memdb_init();

    product_info = load_product_info();
    if(product_info)
    {
    	if(product_info->admin_name)
    		silc_mgmtd_set_default_admin(product_info->admin_name);
    	if(product_info->admin_shadow)
    		silc_mgmtd_set_default_admin_shadow(product_info->admin_shadow);
    }

    for(loop=0; loop<sizeof(s_silc_mgmtd_node_list)/sizeof(silc_mgmtd_node_info); loop++)
    {
        p_info = &s_silc_mgmtd_node_list[loop];
        if(silc_mgmtd_memdb_add_node(p_info->path, p_info->level, p_info->type,
                p_info->val_type, p_info->val_str, p_info->cb, p_info->cb_timeout) < 0)
            goto ERROR_OUT;
    }
    if(product_info)
    {
    	silc_mgmtd_node_info* node_list;
    	int node_cnt;
    	if(product_info->get_node_func && 0 == product_info->get_node_func(&node_list, &node_cnt))
    	{
			for(loop=0; loop<node_cnt; loop++)
			{
				p_info = &node_list[loop];
				if(silc_mgmtd_memdb_add_node(p_info->path, p_info->level, p_info->type,
						p_info->val_type, p_info->val_str, p_info->cb, p_info->cb_timeout) < 0)
					goto ERROR_OUT;
			}
    	}
    }

    for(loop=0; loop<sizeof(s_silc_mgmtd_cberr_list)/sizeof(silc_mgmtd_cberr_info); loop++)
    {
        p_err = &s_silc_mgmtd_cberr_list[loop];
        if(silc_mgmtd_memdb_add_cb_err(p_err->path, p_err->num, p_err->info) < 0)
            goto ERROR_OUT;
    }
    if(product_info)
    {
    	silc_mgmtd_cberr_info* cberr_list;
    	int cberr_cnt;
    	if(product_info->get_cberr_func && 0 == product_info->get_cberr_func(&cberr_list, &cberr_cnt))
    	{
			for(loop=0; loop<cberr_cnt; loop++)
			{
				p_err = &cberr_list[loop];
				if(silc_mgmtd_memdb_add_cb_err(p_err->path, p_err->num, p_err->info) < 0)
					goto ERROR_OUT;
			}
    	}
    }

    for(loop=0; loop<sizeof(s_silc_mgmtd_ref_list)/sizeof(silc_mgmtd_ref_info); loop++)
    {
        p_ref = &s_silc_mgmtd_ref_list[loop];
        if(silc_mgmtd_memdb_add_node_ref(p_ref->path, p_ref->ref_path, p_ref->ref_cb) < 0)
            goto ERROR_OUT;
    }

    // silc_mgmtd_memdb_show_all();
    return 0;
ERROR_OUT:
    silc_mgmtd_memdb_deinit();
    return -1;
}

