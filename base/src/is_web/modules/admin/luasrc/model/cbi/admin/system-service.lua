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

section_cfg[loop] = {name='com', tag='COM', note='', multiple=false, addremove=false}
loop = loop + 1

section_cfg[loop] = {name='ssh', tag='SSH', note='', multiple=false, addremove=false}
loop = loop + 1

section_cfg[loop] = {name='web', tag='Web', note='', multiple=false, addremove=false}
loop = loop + 1

if product.verifyFeature('SYSTEM_SERVICE', 'HTTP') then
	section_cfg[loop] = {name='http', tag='HTTP', note='', multiple=false, addremove=false}
	loop = loop + 1
end

if product.verifyFeature('SYSTEM_SERVICE', 'HTTPS') then
	section_cfg[loop] = {name='https', tag='HTTPS', note='', multiple=false, addremove=false}
	loop = loop + 1
end


mdForm = MdForm('service', '/config/system/service', PRIVILEGE_SUPER, section_cfg, translate("Service Configuration"), translate("Note: Change web/http/https configurations will cause the web access to be temporarily unavailable, please refresh the page."))
mdForm:prepare_all()

--[[
com config
]]--
comConfig = mdForm:get_section_obj('com')

speed = comConfig:option(ListValue, 'speed', translate('Speed'))
speed:value('9600', '9600')
speed:value('38400', '38400')
speed:value('115200', '115200')

termtype = comConfig:option(ListValue, 'termtype', translate('Terminal Type'))
termtype:value('vt100', 'vt100')
termtype:value('vt102', 'vt102')
termtype:value('vt200', 'vt200')
termtype:value('vt220', 'vt220')
termtype:value('xterm', 'xterm')
termtype:value('ansi', 'ansi')
termtype:value('linux', 'linux')

--[[
ssh config
]]--
sshConfig = mdForm:get_section_obj('ssh')
--sshConfig:option(TfFlag, 'enabled', translate('Enabled'))

sshConfig:option(Value, 'port', translate('Port')).datatype = 'port'

if mdForm.readonly == false then
	genkey = sshConfig:option(Button, 'genkey', translate(' '))
	genkey.inputstyle = 'reload'
	genkey.inputtitle = translate('Re-generate SSH Keys')
	function genkey.write(self, section)
		local mgmtdclient = require 'mgmtdclient'
		mgmtdclient.action('/action/system/generate-ssh-key', "")
		luci.http.redirect(luci.dispatcher.build_url('/admin/system/service/'))
	end
end

--[[
web config
]]--
webConfig = mdForm:get_section_obj('web')
timeout = webConfig:option(Value, 'session-timeout', translate('Session Timeout'), translate('minimal 60 sec'))
timeout.datatype = 'uinteger'

--[[
http config
]]--
if product.verifyFeature('SYSTEM_SERVICE', 'HTTP') then
		httpConfig = mdForm:get_section_obj('http')
		httpConfig:option(TfFlag, 'enabled', translate('Enabled'))
		port = httpConfig:option(Value, 'port', translate('Port'))
		port.datatype = 'port'
		port:depends({enabled="true"})
end

--[[
hhtps config
]]--
if product.verifyFeature('SYSTEM_SERVICE', 'HTTPS') then
httpsConfig = mdForm:get_section_obj('https')
httpsConfig:option(TfFlag, 'enabled', translate('Enabled'))
port = httpsConfig:option(Value, 'port', translate('Port'))
port.datatype = 'port'
port:depends({enabled="true"})
--[[
crt = httpsConfig:option(TextValue, 'certificate', translate('SSL Certificate'))
--crt.size = "75"
crt.rows = '30'
crt.readonly = true
crt.default = mdForm.curr_data["https"]["certificate"]
]]--
if not mdForm.readonly then
	httpsConfig:option(FileUpload, 'certificate', translate('SSL Certificate File'))
end
--[[
key = httpsConfig:option(TextValue, 'private-key', translate('SSL Private Key'))
--key.size = "75"
key.rows = '30'
key.readonly = true
key.default = mdForm.curr_data["https"]["private-key"]
]]--
if not mdForm.readonly then
	httpsConfig:option(FileUpload, 'private-key', translate('SSL Private Key File'))
end
end

--[[
telnet config
]]--
--[[
telnetConfig = mdForm:get_section_obj('telnet')
telnetConfig:option(TfFlag, 'enabled', translate('Enabled'))
telnetConfig:option(Value, 'port', translate('Port')).datatype = 'port'
]]--

return mdForm
