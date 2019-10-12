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
	[1] = {name=ROOTNODE, tag="System", note="", multiple=false, addremove=false},
	[2] = {name="datetime", tag="DateTime", note="", multiple=false, addremove=false},
	[3] = {name="datetime/ntp-server-v2", tag="NTP Server", note="", multiple=true, addremove=true},
	[4] = {name="log", tag="Log", note="", multiple=false, addremove=false},
	[5] = {name="log/remote-server-v2", tag="Log Server", note="", multiple=true, addremove=true},
}

--[[
initial mdForm 
]]--
mdForm = MdForm("sys", "/config/system/misc", PRIVILEGE_SUPER, section_cfg, translate("General Configuration"))
--mdForm:prepare_all()
mdForm:set_acl()
mdForm:get_md_data()

if type(mdForm.curr_data["datetime"]["ntp-server-v2"]) ~= "table" then
	mdForm.curr_data["datetime"]["ntp-server-v2"] = {}
	if luci.http.formvalue("cbi.commit") then
		mdForm.orig_data["datetime"]["ntp-server-v2"] = {}
	end
end
mdForm.curr_data["datetime/ntp-server-v2"] = mdForm.curr_data["datetime"]["ntp-server-v2"]
if luci.http.formvalue("cbi.commit") then
	mdForm.orig_data["datetime/ntp-server-v2"] = mdForm.orig_data["datetime"]["ntp-server-v2"]
end

if type(mdForm.curr_data["log"]["remote-server-v2"]) ~= "table" then
	mdForm.curr_data["log"]["remote-server-v2"] = {}
	if luci.http.formvalue("cbi.commit") then
		mdForm.orig_data["log"]["remote-server-v2"] = {}
	end
end
mdForm.curr_data["log/remote-server-v2"] = mdForm.curr_data["log"]["remote-server-v2"]
if luci.http.formvalue("cbi.commit") then
	mdForm.orig_data["log/remote-server-v2"] = mdForm.orig_data["log"]["remote-server-v2"]
end

-- login-banner may be multi-lines, need to do unescape
s = mdForm.curr_data["login-banner"]     
s = string.gsub(s, "\\n", "\n")                                                                   
s = string.gsub(s, "\\r", "\r")                                                           
s = string.gsub(s, "\\\\", "\\")                 
mdForm.curr_data["login-banner"] = s

mdForm:cfg_section()

--[[
basic information config
]]--
baseInfo = mdForm:get_section_obj(ROOTNODE)
local dpt = require "luci.dispatcher"
local product = require "luci.product"

hostname = baseInfo:option(Value, "hostname", translate("Device Name"))
hostname.datatype = "host"

if product.verifyFeature('SYSTEM_MISC', 'LOGIN_BANNER') then
		loginBanner = baseInfo:option(TextValue, "login-banner", translate("CLI Login Banner"))
		loginBanner.size = 90                           
		loginBanner.height = '288px'
end

--[[
datetime config
]]--
dt = mdForm:get_section_obj("datetime")

tz = dt:option(ListValue, "timezone", translate("Timezone"))
tz:value("Etc/UTC", "UTC")
tz_dir = {"America", "Asia", "Europe", "Africa", "Australia", "Indian", "Pacific", "Antarctica", "Atlantic", "Arctic"}
for i, dir in ipairs(tz_dir) do
        filelist = luci.util.exec("ls /usr/share/zoneinfo/"..dir):gmatch("[^%s]+")
        for file in filelist do
                tz:value(dir.."/"..file, dir.."/"..file:gsub("_", "% "))
        end
end

if mdForm.readonly == false then
	newdate = dt:option(Value, "newdate", translate("New Date"), translate("YYYY-MM-DD"))
	newdate.datatype = "date"
	newdate:depends({["ntp-enabled"]=""})

	setdate = dt:option(Button, "setdate", translate(" "))
	setdate.inputstyle = "reload"
	setdate.inputtitle = translate("Set Date")
	setdate:depends({["ntp-enabled"]=""})
	function setdate.write(self, section)
		local date = self.map:formvalue("cbid.datetime.1.newdate")
		if date and date ~= "" then
			local mgmtdclient = require "mgmtdclient"
			mgmtdclient.action("/action/system/set-date", date)
			--luci.http.redirect(luci.dispatcher.build_url("/admin/system/index/"))
		end
	end

	newtime = dt:option(Value, "newtime", translate("New Time"), translate("HH:MM:SS"))
	newtime.datatype = "time"
	newtime:depends({["ntp-enabled"]=""})

	settime = dt:option(Button, 'settime', translate(' '))
	settime.inputstyle = "reload"
	settime.inputtitle = translate("Set Time")
	settime:depends({["ntp-enabled"]=""})
	function settime.write(self, section)
		local time = self.map:formvalue("cbid.datetime.1.newtime")
		if time and time ~= "" then
			local mgmtdclient = require "mgmtdclient"
			mgmtdclient.action("/action/system/set-time", time)
			--luci.http.redirect(luci.dispatcher.build_url("/admin/system/index/"))
		end
	end
end

dt:option(TfFlag, "ntp-enabled", translate("NTP Enabled"))
--[[
svr = dt:option(Value, "ntp-server", translate("NTP Server"), translate("IP or hostname, 1-31 bytes"))
svr.datatype = "host"
svr:depends({["ntp-enabled"]="true"})
]]--
ntpserver = mdForm:get_section_obj("datetime/ntp-server-v2")
ntpserver.template = "cbi/tblsection"
ntpserver.sectionhead = translate("Host")
ntpserver.keydesc = translate("Host: Host Name or IPv4 Address")
ntpserver.keydatatype = "host"

--[[
log config
]]--
log = mdForm:get_section_obj("log")

loglevel = log:option(ListValue, "level", translate("Log Level"))
loglevel:value("7", translate("DEBUG"))
loglevel:value("6", translate("INFO"))
loglevel:value("5", translate("NOTICE"))
loglevel:value("4", translate("WARNING"))
loglevel:value("3", translate("ERROR"))
loglevel:value("2", translate("CRITICAL"))
loglevel:value("1", translate("ALERT"))
loglevel:value("0", translate("EMERGENCY"))

log:option(Value, "max-size", translate("Max Log File Size"), translate("1-10 MB")).datatype = "uinteger"

log:option(TfFlag, "remote-enabled", translate("Remote Log Enabled"))
--[[
svr = log:option(Value, "remote-server", translate("Remote Log Server IP"))
svr.datatype = "ip4addr"
svr:depends({["remote-enabled"]="true"})
]]--
logserver = mdForm:get_section_obj("log/remote-server-v2")
logserver.template = "cbi/tblsection"
logserver.sectionhead = translate("Host")
logserver.keydesc = translate("Host: Host Name or IPv4 Address")
logserver.keydatatype = "host"

port = logserver:option(Value, 'port', translate('Port'))
port.datatype = 'port'
port.default = '514'

return mdForm
