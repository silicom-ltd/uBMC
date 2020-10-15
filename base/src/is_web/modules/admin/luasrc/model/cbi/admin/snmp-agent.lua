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

if product.verifyFeature('SNMP_AGENT', 'STATE') then
	section_cfg[loop] = {name=ROOTNODE, tag="State", note="State", multiple=false, addremove=false}
	loop = loop + 1
end

if product.verifyFeature('SNMP_AGENT', 'COMMUNITIES') then
	section_cfg[loop] = {name="communities", tag="SNMP V1/V2C Communities", note="", multiple=true, addremove=true}
	loop = loop + 1
end

if product.verifyFeature('SNMP_AGENT', 'USERS') then
	section_cfg[loop] = {name="users", tag="SNMP V3 Users", note="", multiple=true, addremove=true}
	loop = loop + 1
end

	section_cfg[loop] = {name="trap-hosts", tag="Trap Hosts", note="", multiple=true, addremove=true}

--[[
initial mdForm instance
]]--
mdForm = MdForm("agent", "/config/snmp/agent", PRIVILEGE_SUPER, section_cfg, translate("Agent Configuration"))
mdForm:prepare_all()


--[[
get the basic config
]]--
if product.verifyFeature('SNMP_AGENT', 'STATE') then
	baseConfig = mdForm:get_section_obj(ROOTNODE)
	baseConfig:option(TfFlag, "state", translate("Enabled"))
end

--[[
list the communities for snmp agent
]]--
if product.verifyFeature('SNMP_AGENT', 'COMMUNITIES') then
		communitiesConfig = mdForm:get_section_obj("communities")
		communitiesConfig.sectionhead = translate("Community Name")
		communitiesConfig.keydesc = "Community Name: 1-31 characters, [a-z][A-Z][0-9][_.]"
		communitiesConfig.keymaxlen = "31"
		communitiesConfig:option(TfFlag, "state", translate("Enabled")).default = "true"

		fc = communitiesConfig:option(TfFlag, "full-access", translate("Full Access"))
		fc:depends({state="true"})
end

--[[
list the users for snmp agent
]]--
if product.verifyFeature('SNMP_AGENT', 'USERS') then
		usersConfig = mdForm:get_section_obj("users")
		usersConfig.sectionhead = translate("Username")
		usersConfig.keydesc = translate("Username: 1-31 characters, [a-z][A-Z][0-9][_.]")
		usersConfig.keymaxlen = "31"
		usersConfig:option(TfFlag, "state", translate("Enabled")).default = "true"

		auth = usersConfig:option(ListValue, "auth", translate("Authentication Protocol"))
		auth:value('sha', 'sha')
		auth:value('md5', 'md5')
		auth:depends({state="true"})

		fc = usersConfig:option(TfFlag, "full-access", translate("Full Access"))
		fc:depends({state="true"})

		pw = usersConfig:option(Value, "password-tmp", translate("Password"))
		pw.password = true
--		pw.datatype = "minlength(8)"
		pw:depends({state="true"})
end


--[[
list the hosts for snmp agent
]]--
hostsConfig = mdForm:get_section_obj("trap-hosts")
hostsConfig.description = "** Note for SNMP V3: INFORMS are used rather than simple TRAPS"
hostsConfig.sectionhead = translate("Host")
hostsConfig.keydesc = translate("Host: IPV4 Address")
hostsConfig.keydatatype = "ip4addr"
hostsConfig:option(TfFlag, "state", translate("Enabled")).default = "true"

version = hostsConfig:option(ListValue, "version", translate("Version"))
version:value('v1', 'SNMP V1')
version:value('v2c', 'SNMP V2C')
version:value('v3', 'SNMP V3')
--version:depends({state="true"})

communityConfig = hostsConfig:option(Value, "community", translate("Community/SNMPv3 User"))
--[[
for i, v in pairs(mdForm.curr_data['communities']) do
	--if type(v) == 'table' then
		--for ii, iv in pairs(v) do
	communityConfig:value(i, i)
		--end
	--end
end
]]--
communityConfig.datatype = "minlength(1)"
communityConfig:depends({state="true"})

auth = hostsConfig:option(ListValue, "auth", translate("Auth"))
auth:value('', '')
auth:value('sha', 'sha')
auth:value('md5', 'md5')
--auth:depends({state="true"})

pw = hostsConfig:option(Value, "password-tmp", translate("Password"))
pw.password = true
--pw:depends({state="true"})

function mdForm.post_commit()
	local mgmtdclient = require "mgmtdclient"
	mgmtdclient.action("/action/snmp/apply")
end

mdForm.template = "admin/agent_mdform"
return mdForm
