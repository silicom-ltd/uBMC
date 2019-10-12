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

section_cfg = 
{
	[1] = {name=ROOTNODE, tag="BMC Console Status", note="Status", multiple=false, addremove=false},
	[2] = {name="console", tag="BMC Console Redirection", note="", multiple=false, addremove=false},
}

--[[
initial mdForm
]]--
mdForm = MdForm("bmc", "/config/bmc", PRIVILEGE_SUPER, section_cfg, translate("BMC Console Redirection"))
mdForm:prepare_all()

hostStatus = mdForm:get_section_obj(ROOTNODE)

ret, status = mgmtdclient.query_child("/status/bmc/console/cable-connected")
consoleStatus = hostStatus:option(DummyValue, "", translate("Console Cable"))
if status == 'true' then
	consoleStatus.default = '<span>Connected</span>'
else
	consoleStatus.default = '<span>Disconnected</span>'
end


hostConfig = mdForm:get_section_obj("console")
comSpeed = hostConfig:option(ListValue, 'com-speed', translate("Console Speed"))
comSpeed:value('9600', '9600')
comSpeed:value('38400', '38400')
comSpeed:value('115200', '115200')

comData = hostConfig:option(ListValue, "com-data", translate("Console Data"))
comData:value('5', '5')
comData:value('6', '6')
comData:value('7', '7')
comData:value('8', '8')

comParity = hostConfig:option(ListValue, "com-parity", translate('Console Parity'))
comParity:value('none', 'none')
comParity:value('even', 'even')
comParity:value('odd', 'odd')
comParity:value('mark', 'mark')
comParity:value('space', 'space')

comStopbits = hostConfig:option(ListValue, "com-stopbits", translate("Console Stopbits"))
comStopbits:value('1', '1')
comStopbits:value('2', '2')

hostConfig:option(TfFlag, "com-hwflowctrl", translate("Enable hardware flow control"))
hostConfig:option(TfFlag, "com-swflowctrl", translate("Enable software flow control"))

hostConfig:option(TfFlag, "log-to-file", translate("Log to file"))
rotate_num = hostConfig:option(Value, "log-rotate-num", translate("Log rotate number"), translate("1-50"))
rotate_num:depends({["log-to-file"]="true"})
rotate_num.datatype = 'range(1, 50)'
rotate_size = hostConfig:option(Value, "log-rotate-size", translate("Log rotate size"), translate("1-10"))
rotate_size:depends({["log-to-file"]="true"})
rotate_size.datatype = 'range(1, 10)'

--[[
local host_entire = {}
local host_log = {}
for seg, seg_v in pairs(mdForm.curr_data["host"]) do
	if string.find(seg, "log") ~= nil then
		host_log[seg] = seg_v
	end
end
host_entire["host"] = host_log

hostLog = mdForm:section(Table, host_entire, translate("Host Log"))
hostLog.template = "cbi/tsection"
--]]

return mdForm
