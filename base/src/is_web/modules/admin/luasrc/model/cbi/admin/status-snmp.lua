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
	[1] = {name=ROOTNODE, tag="SNMP", note="", multiple=false, addremove=false}
}

--[[
initial the mdForm instance
]]--
mdForm = MdForm("snmp", "/status/snmp", PRIVILEGE_MONITOR, section_cfg, translate("SNMP Status"))
mdForm.submit = false
mdForm.reset = false

mdForm:prepare_all()

--[[
display the status information
]]--
statusInfo = mdForm:get_section_obj(ROOTNODE)

state = statusInfo:option(DummyFlag, "state", translate("Enabled"))
state:mapvalue("true", "Yes")
state:mapvalue("false", "No")

--statusInfo:option(DummyValue, "communities", translate("Communities"))
--statusInfo:option(DummyValue, "users", translate("Users"))
--statusInfo:option(DummyValue, "trap-hosts", translate("Trap Hosts"))
local dpt = require "luci.dispatcher"

statusInfo:option(DummyValue, "engine-id", translate("Engine ID"))

return mdForm
