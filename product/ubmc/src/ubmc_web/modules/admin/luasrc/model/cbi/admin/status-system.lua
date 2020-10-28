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
	[1] = {name="info", tag="Global", note="", multiple=false, addremove=false},
--[[
	[2] = {name="health/fan", tag="Host Fan Status", note="", multiple=true, addremove=false},
	[3] = {name="health/voltage", tag="Host Voltage Status", note="", multiple=false, addremove=false},
	[4] = {name="health/temperature", tag="Host Temperature Status", note="", multiple=false, addremove=false},
	[2] = {name="console", tag="BMC Console Status", note="", multiple=false, addremove=false},
	[3] = {name="power", tag="BMC Power Status", note="", multiple=false, addremove=false},
	[4] = {name="usb-cdrom", tag="USB and CDROM", note="", multiple=false, addremove=false},
]]--
}

local dpt = require "luci.dispatcher"

--[[
initial the mdForm instance
]]--
mdForm = MdForm("system", "/status/bmc", PRIVILEGE_MONITOR, section_cfg, translate("System Information"))
mdForm.submit = false
mdForm.reset = false

mdForm:prepare_all()
mdForm.readonly = true

--[[
display the base information
]]--
baseInfo = mdForm:get_section_obj("info")

dev_type = baseInfo:option(DummyValue, "device_name", translate("Device Type"))
if type(mdForm.curr_data["info"]) == 'table' then
	if mdForm.curr_data["info"]["device_sn"] and mdForm.curr_data["info"]["device_sn"] ~= "" then
		baseInfo:option(DummyValue, "device_sn", translate("Device Serial Number"))
	end
	if mdForm.curr_data["info"]["device_tn"] and mdForm.curr_data["info"]["device_tn"] ~= "" then
		baseInfo:option(DummyValue, "device_tn", translate("Device Tracking Number"))
	end
end

baseInfo:option(DummyValue, "hw_version", translate("Hardware Version"))
baseInfo:option(DummyValue, "sw_version", translate("Software Version"))
baseInfo:option(DummyValue, "uboot_version", translate("UBoot Version"))

ret, systime = mgmtdclient.query_child("/status/system/time")
time = baseInfo:option(DummyValue, "sys-time", translate("System Time"))
time.default = systime

ret, uptime = mgmtdclient.query_child("/status/system/uptime")
uptime_begin,_,_ = uptime:find(",")

ut = baseInfo:option(DummyValue, "sys-uptime", translate("System Uptime"))
ut.default = uptime:sub(0, uptime_begin-1)

_,uptime_end,_ = uptime:find("load average")
la = baseInfo:option(DummyValue, "load-average", translate("Load Average"))
la.default = uptime:sub(uptime_end+3)

ret, is_saved = mgmtdclient.query_child("/status/system/config-saved")
cfgsaved = baseInfo:option(DummyValue, "sys-config-saved", translate("Config Change Saved"))
cfgsaved.default = is_saved == "true" and "Yes" or "No"


if mdForm.readonly == false then
	clear_evt = baseInfo:option(Button, 'clear-evt', translate("Clear System Events"))
	clear_evt.inputstyle = "reload"
	clear_evt.inputtitle = translate("Clear")
	function clear_evt.write(self, section)
		mgmtdclient.action("/action/system/reset-sys-evt", {})
		luci.http.redirect(luci.dispatcher.build_url("/admin/status/index/"))
	end
end

local evt_tbl = {}
local evt_idx = 1
if nixio.fs.access("/var/log/message-sys-event") then
	for l in io.lines("/var/log/message-sys-event") do
		local evt = {}
		evt["event"] = l
		evt_tbl[evt_idx] = evt
		evt_idx = evt_idx + 1
	end
end

sysEvt = mdForm:section(Table, evt_tbl, translate("System Events"))
sysEvt.template = "cbi/tblsection"
sysEvt.anonymous = true

sysEvt:option(DummyValue, "event", translate("Event"))

local ntp_state = {}
local ntp_state_info = {}
ret, ns_content = mgmtdclient.query_child("/status/system/ntp-state")
ntp_state_info['state'] = ns_content
ntp_state['ntp'] = ntp_state_info
ntpState = mdForm:section(Table, ntp_state, translate("NTP State"))
ntpState.template = "cbi/tblsection"
ntpState.anonymous = true

ntpState:option(TextValue, "state", translate("State")).height = "100px"

local ntp_associations = {}
local ntp_associations_info = {}
ret, na_content = mgmtdclient.query_child("/status/system/ntp-associations")
ntp_associations_info['associations'] = na_content
ntp_associations['ntp'] = ntp_associations_info
ntpAssociations = mdForm:section(Table, ntp_associations, translate("NTP Associations"))
ntpAssociations.template = "cbi/tblsection"
ntpAssociations.anonymous = true

ntpAssociations:option(TextValue, "associations", translate("Associations")).height = "130px"

	ret, mgmt_list = mgmtdclient.query_sub("/status/system/mgmt")

	---ret, inf_list = mgmtdclient.query_sub("/status/system/mgmt/interface-list") 
	infList = mdForm:section(Table, mgmt_list['interface-list'], translate("System Interfaces"))
	infList.template = "cbi/tblsection"                                                  
	infList:option(TextValue, "info", translate("Info")).height = "200px"   

	local dhcp_list = {}
	local dhcp_info = {}
	--ret, dhcp_state = mgmtdclient.query_child("/status/system/mgmt/dhcp/state")
	dhcp_info['state'] = mgmt_list['dhcp']['state'] == "true" and "DHCP is running" or "DHCP is not running"
	--ret, dhcp_leases = mgmtdclient.query_child("/status/system/mgmt/dhcp/leases")
	dhcp_info['leases'] = mgmt_list['dhcp']['leases'] 
	dhcp_list['dhcp'] = dhcp_info
	dhcpInfo = mdForm:section(Table, dhcp_list, translate("System DHCP"))
	dhcpInfo.template = "cbi/tsection"                                                  
	dhcpState = dhcpInfo:option(DummyValue, "state", translate("State"))
	dhcpInfo:option(TextValue, 'leases', translate("Leases")).height = "540px"

	local ipt = {}
	local ipt_list = {}
	--ret, iptables_s = mgmtdclient.query_sub("/status/system/mgmt/iptables/iptables-s")
	ipt['iptables-s'] = mgmt_list['iptables']['iptables-s']
	--ret, iptables_l = mgmtdclient.query_sub("/status/system/mgmt/iptables/iptables-l")
	ipt['iptables-l'] = mgmt_list['iptables']['iptables-l']
	--ret, ip6tables_s = mgmtdclient.query_sub("/status/system/mgmt/iptables/ip6tables-s")
	ipt['ip6tables-s'] = mgmt_list['iptables']['ip6tables-s']
	--ret, ip6tables_l = mgmtdclient.query_sub("/status/system/mgmt/iptables/ip6tables-l")
	ipt['ip6tables-l'] = mgmt_list['iptables']['ip6tables-l']
	ipt_list['list'] = ipt
	iptables = mdForm:section(Table, ipt_list, translate("System Iptables"))
	iptables.template = "cbi/tsection"                                                  
	iptables:option(TextValue, "iptables-s", translate("iptables -S ")).height = "380px"
	iptables:option(TextValue, "iptables-l", translate("iptables -L --line-numbers ")).height = "380px"
	iptables:option(TextValue, "ip6tables-s", translate("ip6tables -S ")).height = "380px"
	iptables:option(TextValue, "ip6tables-l", translate("ip6tables -L --line-numbers ")).height = "380px"

	local ips_conn = {}                                                        
	local ips_list = {}                                                        
	--ret, conn = mgmtdclient.query_child("/status/system/mgmt/ipsec/connection")                  
	ips_conn['info'] = mgmt_list['ipsec']['connection'] 
	ips_list['ipsec'] = ips_conn                                             
	ipsList = mdForm:section(Table, ips_list, translate("System IPSEC"))
	ipsList:option(TextValue, "info", translate("Connection")).height = "200px"   

--[[
consoleInfo = mdForm:get_section_obj("console")
cableState = consoleInfo:option(DummyValue, "cable-connected", translate("Console Cable"))            
cableState:mapvalue('true', '<span>Connected</span>')                                                 
cableState:mapvalue('false', '<span>Disconnected</span>')

bmcInfo = mdForm:get_section_obj("power")
bmcInfo:option(DummyValue, "state", translate("Power State"))
bmcInfo:option(DummyValue, "action", translate("Power Action"))

ubsInfo = mdForm:get_section_obj("usb-cdrom")
ubsInfo:option(DummyValue, "enabled", translate("Mounted"))
ubsInfo:option(DummyValue, "image", translate("Image"))
]]--

local fan_data = type(mdForm.curr_data["health"])=="table" and mdForm.curr_data["health"]["fan"] or {}
if type(fan_data) ~= 'table' then
	fan_data = {}
end

fanInfo = mdForm:section(Table, fan_data, translate("Host Fan Status"))
fanInfo.template = 'cbi/tblsection'
fanInfo.anonymous = false
fanInfo.sectionhead = translate("ID")

fanInfo:option(DummyValue, 'name', translate("Name"))

fault = fanInfo:option(DummyValue, 'fault', translate("Fault"))
fault:mapvalue("true", "<span style='color: red'>Yes</span>")
fault:mapvalue("false", "<span style='color: green'>No</span>")

warn = fanInfo:option(DummyValue, 'warning', translate("Warning"))
warn:mapvalue("true", "<span style='color: red'>Yes</span>")
warn:mapvalue("false", "<span style='color: green'>No</span>")

st = fanInfo:option(DummyValue, 'status', translate("Status"))
st:mapvalue("0", "<span style='color: green'>Green</span>")
st:mapvalue("1", "<span style='color: green'>Green</span>")
st:mapvalue("2", "<span style='color: yellow'>Yellow</span>")
st:mapvalue("4", "<span style='color: orange'>Orange</span>")
st:mapvalue("8", "<span style='color: red'>Red</span>")

fanInfo:option(DummyValue, 'speed', translate("Speed (RPM)"))
--fanInfo:option(DummyValue, 'run_time', translate("Run Time (H)"))


local vol_data = type(mdForm.curr_data["health"])=="table" and mdForm.curr_data["health"]["voltage"] or {}
if type(vol_data) ~= 'table' then
	vol_data = {}
end

volInfo = mdForm:section(Table, vol_data, translate("Host Voltage Status"))
volInfo.template = 'cbi/tblsection'
volInfo.anonymous = false
volInfo.sectionhead = translate("ID")

volInfo:option(DummyValue, 'name', translate("Name"))
--[[
volFault = volInfo:option(DummyValue, 'fault', translate("Fault"))
volFault:mapvalue("true", "<span style='color: red'>Yes</span>")
volFault:mapvalue("false", "<span style='color: green'>No</span>")

volWarn = volInfo:option(DummyValue, 'warning', translate("Warning"))
volWarn:mapvalue("true", "<span style='color: red'>Yes</span>")
volWarn:mapvalue("false", "<span style='color: green'>No</span>")

volSt = volInfo:option(DummyValue, 'status', translate("Status"))
volSt:mapvalue("0", "<span style='color: black'>Unknown</span>")
volSt:mapvalue("1", "<span style='color: green'>Green</span>")
volSt:mapvalue("2", "<span style='color: yellow'>Yellow</span>")
volSt:mapvalue("4", "<span style='color: orange'>Orange</span>")
volSt:mapvalue("8", "<span style='color: red'>Red</span>")
]]--
volInfo:option(DummyValue, 'voltage', translate("Voltage(V)"))
--volInfo:option(DummyValue, 'run_time', translate("Run Time (H)"))

local tem_data = type(mdForm.curr_data["health"])=="table" and mdForm.curr_data["health"]["temperature"] or {}
if type(tem_data) ~= 'table' then
	tem_data = {}
end

temInfo = mdForm:section(Table, tem_data, translate("Host Temperature Status"))
temInfo.template = 'cbi/tblsection'
temInfo.anonymous = false
temInfo.sectionhead = translate("ID")

temInfo:option(DummyValue, 'name', translate("Name"))
temInfo:option(DummyValue, 'temp', translate("Temperature(°C)"))
temInfo:option(DummyValue, 'peak', translate("Peak(°C)"))

local pow_data = type(mdForm.curr_data["health"])=="table" and mdForm.curr_data["health"]["power_supply"] or {}
if type(pow_data) ~= 'table' then
	pow_data = {}
end

powInfo = mdForm:section(Table, pow_data, translate("Power Supply Status"))
powInfo.template = 'cbi/tblsection'
powInfo.anonymous = false
powInfo.sectionhead = translate("ID")

powInfo:option(DummyValue, 'name', translate("Name"))
powInfo:option(DummyValue, 'vout_val', translate("Voltage out(V)"))
power_supply_status = powInfo:option(DummyValue, 'status', translate("Status"))
power_supply_status:mapvalue("true", "Present")
power_supply_status:mapvalue("false", "Not Present")
power_supply_fault = powInfo:option(DummyValue, 'fault', translate("Fault"))
power_supply_fault:mapvalue("true", "<span style='color: red'>Yes</span>")
power_supply_fault:mapvalue("false", "<span style='color: green'>No</span>")
power_supply_warning = powInfo:option(DummyValue, 'warning', translate("Warning"))
power_supply_warning:mapvalue("true", "<span style='color: red'>Yes</span>")
power_supply_warning:mapvalue("false", "<span style='color: green'>No</span>")

return mdForm
