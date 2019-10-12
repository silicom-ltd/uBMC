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

module("luci.controller.admin.snmp", package.seeall)

function index()
	local dpt = require "luci.dispatcher"

	local product = require "luci.product"

	entry({"admin", "snmp"}, alias("admin", "snmp", "index"), _("SNMP"), 60).index = true

if product.verifyFeature('SNMP', 'TRAP') then
	entry({"admin", "snmp", "index"}, cbi("admin/snmp-trap", {autoapply=true}), _("Trap Filter"), 1)
	entry({"admin", "snmp", "agent"}, cbi("admin/snmp-agent", {autoapply=true}), _("Agent"), 10)
else
	entry({"admin", "snmp", "index"}, cbi("admin/snmp-agent", {autoapply=true}), _("Agent"), 1)
end
if product.verifyFeature('SNMP', 'THRESHOLD') then
	entry({"admin", "snmp", "threshold"}, cbi("admin/snmp-threshold", {autoapply=true}), _("Threshold"), 20)
end
	entry({"admin", "snmp", "download"}, call("action_download"), _("Mib File"), 30)
end

function action_download()

	local cfg_dir = "/etc/mibs"
	local cfg_list = {}
	local err, msg

	if luci.http.formvalue("download") then
		local cfg_file = luci.http.formvalue('config_file')
		if cfg_file then
			local f = io.open(cfg_dir.."/"..cfg_file, "r")
			if f then
				local cfg_file_content = f:read("*a")
				f:close()

				luci.http.header('Content-Disposition', 'attachment; filename="%s"' % {cfg_file})
				luci.http.prepare_content("application/octet-stream")
				luci.http.write(cfg_file_content)
				return true
			end
		end
	end
	

	for f in io.popen("ls -t "..cfg_dir.."/"):lines() do
		file = {}
		file.name = f
		file.mtime = os.date("%Y%m%d %H:%M:%S", nixio.fs.stat(cfg_dir.."/"..f).mtime)
		table.insert(cfg_list, file)
	end

	luci.template.render("admin/snmp_download", {err=err, msg=msg, cfg_list=cfg_list})
	return true
end
