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
	--[1] = {name=ROOTNODE, tag="Global", note="", multiple=false, addremove=false},
	--[1] = {name="session", tag="Session", note="", multiple=true, addremove=false},
}

--[[
initial mdForm instance
]]--
mdForm = MdForm("session", "/status/system/session", PRIVILEGE_MONITOR, section_cfg, translate("Session"))
mdForm.submit = false
mdForm.reset = false

mdForm:prepare_all()

--[[
display the base status
]]--
--[[
baseStatus = mdForm:get_section_obj(ROOTNODE)
--baseStatus.template = "cbi/tblsection"

baseStatus:option(DummyValue, "time", translate("Time"))

cfgsaved = baseStatus:option(DummyValue, "config-saved", translate("Config Change Saved"))
cfgsaved:mapvalue("true", "Yes")
cfgsaved:mapvalue("false", "No")
]]--
--[[
display the session status
]]--
--sessionStatus = mdForm:get_section_obj("session")
sessionStatus = mdForm:section(DynTable, mdForm.curr_data)
sessionStatus.template = "cbi/tblsection"
sessionStatus.anonymous = false
sessionStatus.sectionhead = translate("ID")

sessionStatus:option(DummyValue, "user", translate("User"))
sessionStatus:option(DummyValue, "type", translate("Login Type"))
sessionStatus:option(DummyValue, "login-time", translate("Login Time"))
sessionStatus:option(DummyValue, "login-ip", translate("Login IP"))
sessionStatus:option(DummyValue, "login-port", translate("Login Port"))

return mdForm
