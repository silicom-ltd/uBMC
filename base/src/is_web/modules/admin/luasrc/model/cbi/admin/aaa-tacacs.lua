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

function get_mapped_user()
	local mgmtdclient = require "mgmtdclient"
	local md_path = "/config/unix/user"

	ret, all_user = mgmtdclient.query_child(md_path)
	if ret ~= "OK" then
		return ret
	end
	if not all_user or all_user == "null" then
		return "UNIX user list is null"
	end

	return "OK", all_user
end

local section_cfg = 
{
	[1] = {name="static", tag="Global", note="", multiple=false, addremove=false},
	[2] = {name="server", tag="Server", note="", multiple=true, addremove=true},
}

f = MdForm("tacacs", "/config/tacacs", PRIVILEGE_SUPER, section_cfg, translate("TACACS+"))

f:prepare_all()

s = f:get_section_obj("static")

s:option(TfFlag, "enabled", translate("Enable"))
s:option(Value, "service", translate("Service Tag"), translate("tag except slip/ppp/arap/shell/tty-daemon/connection/system/firewall")):depends({enabled="true"})

--[[
mu = s:option(ListValue, "mapped-user", translate("Mapped Local User"))
ret, mapped_user = get_mapped_user()
if ret == "OK" then
	for k,v in pairs(mapped_user) do
		mu:value(v, translate(v))
	end
else
	f.errmessage = translate("Get mapped user list error: " .. ret)
end
mu:depends({enabled="true"})
]]--

to = s:option(Value, "timeout", translate("Timeout"), translate("sec"))
to.datatype = "uinteger"
to:depends({enabled="true"})

s:option(TfFlag, "fallback", translate("Local Login"), translate("Allow local users login")):depends({enabled="true"})

--[[
login = s:option(ListValue, "login", translate("Login"))
login:value("pap", translate("pap"))
login:value("chap", translate("chap"))
login:value("login", translate("login"))
login:depends({enabled="true"})
]]--

t = f:get_section_obj("server")
t.template = "cbi/tblsection"
t.sectionhead = translate("ID")
t.keydesc = translate("Server ID: Number only")
t.keydatatype = "uinteger"

t:option(Value, "server-ip", translate("Host")).datatype = "host"

port = t:option(Value, "server-port", translate("Port"))
port.datatype = "port"
port.default = 49

t:option(Value, "secret", translate("Secret")).password = true

return f
