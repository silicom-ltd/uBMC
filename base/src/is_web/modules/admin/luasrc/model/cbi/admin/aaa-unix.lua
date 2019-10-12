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
	--[1] = {name="static", tag="Global", note="", multiple=false, addremove=false},
	[1] = {name="user", tag="User", note="", multiple=true, addremove=true},
}

f = MdForm("unix", "/config/unix", PRIVILEGE_SUPER, section_cfg, translate("Local"))

f:prepare_all()

--s = f:get_section_obj("static")
--s:option(Value, "uid-base", translate("UID Base"))

t = f:get_section_obj("user")
t.template = "cbi/tblsection"
t.sectionhead = translate("Username")
t.keydesc = translate("Username: 1-31 characters, [a-z][0-9][_-]")
t.keydatatype = "usrname"
t.keymaxlen = "31"

t:option(Value, "full-name", translate("Full Name")).maxlength = 31

p = t:option(ListValue, "privilege", translate("Privilege"))
p:value("1", translate("Readonly"))
p:value("2", translate("Normal"))
p:value("3", translate("Admin"))

return f
