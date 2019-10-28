--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--
local dpt = require "luci.dispatcher"
local product = require "luci.product"
local section_cfg = {}
local loop = 1

if product.verifyFeature('SYSTEM_MGMT', 'INTERFACE') then
	section_cfg[loop] = {name="interface", tag="Interface", note="", multiple=false, addremove=false}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'INTERFACE_LIST') then
	section_cfg[loop] = {name="interface-list", tag="Interfaces", note="", multiple=true, addremove=false}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'CLI_SESSION') then
	section_cfg[loop] = {name=ROOTNODE, tag="CLI Session", note="", multiple=false, addremove=false}
	loop = loop + 1
else
	section_cfg[loop] = {name=ROOTNODE, tag="", note="", multiple=false, addremove=false}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'PERMIT_LIST') then
	section_cfg[loop] = {name="permit-list", tag="Permitted IP List", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'VRF_LIST') then
	section_cfg[loop] = {name="vrf-list", tag="VRF List", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'VRF_PROCESS_LIST') then
	section_cfg[loop] = {name="vrf-process-list", tag="VRF Process List", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'ROUTE_LIST') then
	section_cfg[loop] = {name="route-list", tag="Route List", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'ADDRESS_LIST') then
	section_cfg[loop] = {name="address-list", tag="Address List", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_MGMT', 'DNS') then
	section_cfg[loop] = {name="dns", tag="DNS Server", note="", multiple=true, addremove=true}
	loop = loop + 1
end


--[[
initial mdForm
]]--
mdForm = MdForm("mgmt", "/config/system/mgmt", PRIVILEGE_SUPER, section_cfg, translate("Management Interface Configuration"))
mdForm:prepare_all()


--[[
interface config
]]--
if product.verifyFeature('SYSTEM_MGMT', 'INTERFACE') then
	interfaceConfig = mdForm:get_section_obj("interface")
	interfaceConfig:option(TfFlag, "state", translate("Enabled"))

	interfaceConfig:option(DummyValue, "mac", translate("MAC address"))

	ip = interfaceConfig:option(Value, "ip-address", translate("IP Address"))
	ip.datatype = 'ip4addr'
	ip:depends({state="true"})

	mask = interfaceConfig:option(Value, "ip-mask", translate("Mask"))
	mask.datatype = 'ip4addr'
	mask:depends({state="true"})

	gw = interfaceConfig:option(Value, "ip-default-gateway", translate("Gateway"))
	gw.datatype = 'ip4addr'
	gw:depends({state="true"})
end

--[[
permit state and session-timeout
]]--
rootConfig = mdForm:get_section_obj(ROOTNODE)

if product.verifyFeature('SYSTEM_MGMT', 'CLI_SESSION') then
rootConfig:option(Value, "session-exp-time", translate("CLI Session Timeout"), translate("sec, 0 means disable")).datatype = 'uinteger'
end

if product.verifyFeature('SYSTEM_MGMT', 'PERMIT_LIST') then
rootConfig:option(TfFlag, "permit-enabled", translate("Permitted IP"))

--[[
permit list
]]--
	permitList = mdForm:get_section_obj("permit-list")
	permitList.template = "cbi/tblsection"
	permitList.sectionhead = translate("IP")
	permitList.keydesc = translate("IP: IPv4 Address")
	permitList.keydatatype = "ip4addr"

	mask = permitList:option(Value, "mask", translate("Mask"))
	mask.datatype = "ip4addr"
	mask.default = "255.255.255.255"
end


if product.verifyFeature('SYSTEM_MGMT', 'INTERFACE_LIST') then
	interfaceList = mdForm:get_section_obj("interface-list")
	interfaceList.template = "cbi/tsection"
	interfaceList.sectionhead = translate("Interface")

	interfaceList:option(Value, 'desc', translate("Interface Description"))
	interfaceList:option(TfFlag, 'state', translate("Current State"))

local vrf_ret, vrf_list = mgmtdclient.query_child("/config/system/mgmt/vrf-list")
masterList = interfaceList:option(ListValue, 'master', translate("Master"))
masterList:value('', '--')
if vrf_ret == 'OK' then
	if vrf_list ~= nil then
		if type(vrf_list) == 'table' then
			for v in pairs(vrf_list) do
				masterList:value(v, v)
			end
		end
	end
end
	interfaceList:option(Value, 'mtu', translate("Interface MTU"))
	interfaceList:option(Value, 'vlan', translate("VLAN"))

	interfaceList:option(TfFlag, 'dhcp-sendname', translate("DHCP Send Hostname"))
	interfaceList:option(TfFlag, 'ipv4-enabled', translate("IPv4 Enabled"))

	originIpv4 = interfaceList:option(ListValue, 'ipv4-origin', translate("IPv4 Origin"))
	originIpv4:value('static', 'Static IP')
	originIpv4:value('dhcp', 'DHCP')

	addressIpv4 = interfaceList:option(Value, 'ipv4-address', translate("IPv4 Address"))
	addressIpv4:depends({["ipv4-origin"]  = "static"})

	prefixIpv4 = interfaceList:option(Value, 'ipv4-prefix', translate("IPv4 Prefix"))
	prefixIpv4:depends({["ipv4-origin"]  = "static"})

		--gatewayIpv4 = interfaceList:option(Value, 'ipv4-default-gateway', translate("IPv4 Gateway"))
		--gatewayIpv4:depends({["ipv4-origin"]  = "static"})

		--dhcpnameIpv4 = interfaceList:option(Value, 'ipv4-dhcpname', translate("IPv4 DHCP"))
		--dhcpnameIpv4:depends({["ipv4-origin"]  = "dhcp"})  

	interfaceList:option(TfFlag, 'ipv6-enabled', translate("IPv6 Enabled"))
	originIpv6 = interfaceList:option(ListValue, 'ipv6-origin', translate("IPv6 Origin"))
	originIpv6:value('static', 'Static IP')
	originIpv6:value('dhcp', 'DHCP')

	addressIpv6 = interfaceList:option(Value, 'ipv6-address', translate("IPv6 Address"))
		--addressIpv6.datatype = 'ipv6addr'
	addressIpv6:depends({["ipv6-origin"]  = "static"})

	prefixIpv6 = interfaceList:option(Value, 'ipv6-prefix', translate("IPv6 Prefix"))
	prefixIpv6:depends({["ipv6-origin"]  = "static"})
end

--gatewayIpv6 = interfaceList:option(Value, 'ipv6-default-gateway', translate("IPv6 Gateway"))
--gatewayIpv6.datatype = 'ipv6addr'
--gatewayIpv6:depends({["ipv6-origin"]  = "static"})

--dhcpnameIpv6 = interfaceList:option(Value, 'ipv6-dhcpname', translate("IPv6 DHCP"))
--dhcpnameIpv6:depends({["ipv6-origin"]  = "dhcp"})

--[[
permit state and session-timeout
rootConfig = mdForm:get_section_obj(ROOTNODE)

rootConfig:option(Value, "session-exp-time", translate("CLI Session Timeout"), translate("sec, 0 means disable")).datatype = 'uinteger'
]]--

if product.verifyFeature('SYSTEM_MGMT', 'VRF_LIST') then
	vrf = mdForm:get_section_obj("vrf-list")
	vrf.template = "cbi/tblsection"
	vrf.sectionhead = translate("VRF Name")

	vrf:option(Value, 'table', translate("Table")).datatype = "uinteger"
	vrf:option(TfFlag, 'state', translate("State"))
end

if product.verifyFeature('SYSTEM_MGMT', 'VRF_PROCESS_LIST') then
	vrfprocess = mdForm:get_section_obj("vrf-process-list")
	vrfprocess.template = "cbi/tblsection"
	vrfprocess.sectionhead = translate("VRF Process Name")

	process = vrfprocess:option(ListValue, 'process', translate("Process"))
	process:value('ipsec', 'ipsec')
	process:value('netconf', 'netconf')
	process:value('ssh', 'ssh')
	process:value('web', 'web')
	process:value('dying-gasp', 'dying-gasp')
	process:value('ntp', 'ntp')
	process:value('syslog', 'syslog')

local vrf_ret, vrf_list = mgmtdclient.query_child("/config/system/mgmt/vrf-list")
vrfList = vrfprocess:option(ListValue, 'vrf', translate("VRF"))
vrfList:value('', '--')
if vrf_ret == 'OK' then
	if vrf_list ~= nil then
		if type(vrf_list) == 'table' then
			for v in pairs(vrf_list) do
				vrfList:value(v, v)
			end
		end
	end
end
end

--vrfprocess:option(Value, 'binding-address', translate("Binding Address"))

if product.verifyFeature('SYSTEM_MGMT', 'ROUTE_LIST') then
	route = mdForm:get_section_obj("route-list")
	route.template = "cbi/tblsection"
	route.sectionhead = translate("Route Name")

	route:option(Value, 'dest', translate("Destination"))
	route:option(Value, 'via', translate("Via Address"))
	route:option(Value, 'table', translate("Table"))
	route:option(Value, 'dev', translate("Device"))
end

if product.verifyFeature('SYSTEM_MGMT', 'ADDRESS_LIST') then
	address = mdForm:get_section_obj("address-list")
	address.template = "cbi/tblsection"
	address.sectionhead = translate("Address Name")

	address:option(Value, 'address', translate("IP address")).datatype = 'ip4addr'
	address:option(Value, 'prefix', translate("Prefix length"))
	address:option(Value, 'interface', translate("Interface name"))

	local address_list = {}
	local base = 0
	for seg, seg_v in pairs(mdForm.curr_data["address-list"]) do
		address_list[seg] = seg_v
		base = base + 1
	end

	addressList = mdForm:section(Table, address_list, translate(""))
end

--[[
DNS server
]]--
if product.verifyFeature('SYSTEM_MGMT', 'DNS') then
	dns = mdForm:get_section_obj("dns")
	dns.template = "cbi/tblsection"
	dns.sectionhead = translate("IP")
	dns.keydesc = translate("IP: IPv4 Address")
	dns.keydatatype = "ip4addr"
end


mdForm.template = "admin/system_mgmt_mdform"

return mdForm
