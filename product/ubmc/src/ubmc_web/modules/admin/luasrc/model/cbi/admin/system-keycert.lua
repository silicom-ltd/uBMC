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
	[1] = {name="key-list", tag="Private Keys", note="", multiple=true, addremove=true},
	[2] = {name="cert-list", tag="Certificates", note="", multiple=true, addremove=true},
}

--[[
initial mdForm instance
]]--
mdForm = MdForm("keycert", "/config/system/mgmt", PRIVILEGE_SUPER, section_cfg, translate("Keys & Certs"))
mdForm:prepare_all()

--[[
list the key
]]--
keysConfig = mdForm:get_section_obj("key-list")
keysConfig.sectionhead = translate("Key-ID")
keysConfig.keymaxlen = "31"
keysConfig:option(FileUpload, "key-content", translate(""))
keysConfig:option(Value, "key-path", translate(""))

--[[
list the cert 
]]--
certsConfig = mdForm:get_section_obj("cert-list")
certsConfig.sectionhead = translate("Cert-ID")
certsConfig.certmaxlen = "31"
certsConfig:option(FileUpload, "cert-content", translate(""))
certsConfig:option(Value, "cert-path", translate(""))

mdForm.template = "admin/keycert_mdform"
return mdForm
