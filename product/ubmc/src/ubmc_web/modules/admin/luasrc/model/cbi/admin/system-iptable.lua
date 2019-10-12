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

local section_cfg =
{
	[1] = {name="iptables-rule", tag="Iptable List", note="", multiple=true, addremove=true},
}

--[[
initial mdForm instance
]]--
mdForm = MdForm("ipsec", "/config/system/mgmt", PRIVILEGE_SUPER, section_cfg, translate("Iptables"))
mdForm:prepare_all()

--[[
list the conn-name for ipsec conn
]]--
iptableConfig = mdForm:get_section_obj("iptables-rule")
iptableConfig.template = "cbi/tsection"
iptableConfig.sectionhead = translate("Name")

iptableConfig:option(Value, "priority", translate("Priority"))

iptable = iptableConfig:option(ListValue, "table", translate("Table"))
iptable:value('filter', 'filter')

iptableVersion = iptableConfig:option(ListValue, "version", translate("Version"))
iptableVersion:value('ipv4', 'ipv4')
iptableVersion:value('ipv6', 'ipv6')

iptableConfig:option(Value, "rule", translate("Rule"), translate("Only support Insert/Append command rule"))

return mdForm
