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

module("luci.controller.admin.status", package.seeall)

local dpt = require "luci.dispatcher"

function index()
	local dpt = require "luci.dispatcher"

	local product = require "luci.product"

if product.verifyFeature('MENU', 'UPPERCASE') then
	entry({"admin", "status"}, alias("admin", "status", "index"), _("STATUS"), 20).index = true
else
	entry({"admin", "status"}, alias("admin", "status", "index"), _("Status"), 20).index = true
end
	entry({"admin", "status", "index"}, cbi("admin/status-system", {autoapply=true}), _("System"), 1)

if product.verifyFeature('STATUS', 'RESOURCE') then
	entry({"admin", "status", "resource"}, call("action_resource"), _("Resource"), 2)
end

if product.verifyFeature('STATUS', 'MODULE') then
	local mgmtdclient = require "mgmtdclient"
	local md_path = "/status/switch/module"

	ret, all_module = mgmtdclient.query_child(md_path)
	if ret ~= "OK" then
		msg = "Get switch module list error: " .. ret
	else
		if type(all_module) == 'table' then
			for i, v in pairs(all_module) do
				entry({"admin", "status", "module-" .. v}, cbi("admin/status-module", {autoapply=true}), _("Module " .. v), 10 * i)
			end
		end
	end
end
	entry({"admin", "status", "snmp"}, cbi("admin/status-snmp", {autoapply=true}), _("SNMP"), 80)
	entry({"admin", "status", "session"}, cbi("admin/status-session", {autoapply=true}), _("Session"), 90)
	entry({"admin", "status", "syslog"}, call("action_syslog"), _("System Log"), 100)
end

function check_privil()
	if dpt.context.user_privil < dpt.PRIVILEGE_SUPER then
		require("luci.i18n")
		require("luci.template")
		luci.template.render("admin/forbidden")
		return false
	end
	return true
end

function _generate_graphic(target, mode, path)
	local cmdline = "rrdtool graph"
	local currsec = os.date("%s")

	cmdline = cmdline .. " " .. path
	cmdline = cmdline .. " -l0  -w 1000 -h 260"
	cmdline = cmdline .. " -e " .. currsec
	cmdline = cmdline .. " -s " .. (currsec-1200)

	if target and target == 'cpu' then
		if mode and mode == 'all' then
			file = "/tmp/sys-rrd/cpu_all"
			cmdline = cmdline .. " -t 'CPU All'"
			cmdline = cmdline .. " DEF:line0=" .. file .. ":cpu:AVERAGE LINE3:line0#FF0000:'CPU All'"
		else
			file = "/tmp/sys-rrd/cpu_per_core"
			cmdline = cmdline .. " -t 'CPU Per Core'"
			cmdline = cmdline .. " DEF:line0=" .. file .. ":cpu0:AVERAGE LINE3:line0#FF0000:'CPU 0'"
			cmdline = cmdline .. " DEF:line1=" .. file .. ":cpu1:AVERAGE LINE3:line1#00FF00:'CPU 1'"
			cmdline = cmdline .. " DEF:line2=" .. file .. ":cpu2:AVERAGE LINE3:line2#00FFFF:'CPU 2'"
			cmdline = cmdline .. " DEF:line3=" .. file .. ":cpu3:AVERAGE LINE3:line3#FFFF00:'CPU 3'"
		end
	else
		file = "/tmp/sys-rrd/mem_usage"
		cmdline = cmdline .. " -t 'Memory'"
		cmdline = cmdline .. " DEF:line0=" .. file .. ":total:AVERAGE LINE3:line0#FF0000:'Total'"
		cmdline = cmdline .. " DEF:line1=" .. file .. ":used:AVERAGE LINE3:line1#00FF00:'Used'"
	end

	luci.util.exec(cmdline)
end

function _show_graphic(png, path)
	local l12 = require "luci.ltn12"

	luci.http.prepare_content("image/png")
	l12.pump.all(l12.source.file(png), luci.http.write)
	luci.util.exec("rm -rf " .. path)
end

function action_resource()
	local vars  = luci.http.formvalue()
	local dpt = require "luci.dispatcher"
	
	if vars.img then
		local suffix = "self"
		if vars.mode then
			suffix = vars.mode
		end

		--local path = "/tmp/rrd/" .. dpt.context.authsession .. "-" .. suffix .. ".png"--
		local path = "/tmp/sys-rrd/resource-" .. vars.target .. "-" .. suffix .. ".png"

		local png = io.open(path, "r")
		if png == nil then
			_generate_graphic(vars.target, vars.mode, path)
			png = io.open(path, "r")
			_show_graphic(png, path)
		else
			_show_graphic(png, path)
			--[[png:close()]]--
		end

		return
	end

	if vars.cnf then
		local l12 = require "luci.ltn12"
		local path = "/tmp/fs_config_usage.png"

		if vars.target and vars.target == 'log' then
			path = "/tmp/fs_log_usage.png"
			luci.util.exec("/etc/gen_log_usage_chart.sh")
		else
			luci.util.exec("/etc/gen_fs_usage_chart.sh /config/ /tmp/fs_config_usage.png")
		end

		local png = io.open(path, "r")
		if png then
			luci.http.prepare_content("image/png")
			l12.pump.all(l12.source.file(png), luci.http.write)
		end
		return
	end


	luci.template.render("admin/resource", {})
end

function action_syslog()

	local total = 120
	local luci_http = require "luci.http"
	local luci_util = require "luci.util"

	local reset = luci_http.formvalue("reset")
	if reset then
		if not check_privil() then
			return false
		end
		local mgmtdclient = require "mgmtdclient"
		mgmtdclient.set_login(dpt.context.authuser, dpt.context.user_privil, dpt.context.auth_proto)
		mgmtdclient.action('/action/system/reset-log', "")
	end

	local matching = luci_http.formvalue("keyword")
	local target = luci_http.formvalue("page")
	if matching then
		matching = luci_util.trim(matching)
	else
		matching = ""
	end
	if target then
		target = target + 0
	else
		target = 1
	end

	if matching == "" then
		luci.template.render("admin/syslog", log_message("/var/log/messages*", target, total, matching))
	else
		luci.template.render("admin/syslog", log_message_search("/var/log", "messages*", target, total, matching))
	end
end

function log_message_search(folder, filename, target, size, word)
	if string.find(word, "^[%w/_:#@~%[%]%.%+%-=%s]+$") then
		local words = word:gsub(" ", "_")
		words = words:gsub("/", "_")
		words = words:gsub("%[", "_")
		words = words:gsub("%]", "_")
		local filter = word:gsub("%.", "\\%.")
		filter = filter:gsub("%[", "\\%[")
		filter = filter:gsub("%]", "\\%]")
		luci.util.exec("echo " .. filter .." > /tmp/emp_log")
		luci.util.exec("cd " .. folder .. ";files=`ls " .. filename .. "`;for file in $files ; do grep -i '" .. filter .. "' $file > /tmp/'" .. words .. "'_$file;done")
		return log_message("/tmp/" .. words .. "_" .. filename, target, size, word)
	else
		word = word:gsub('"', '&quot;')
		luci.util.exec("echo '' > /tmp/empty_log")
		return log_message("/tmp/empty_log", target, size, word)
	end
end

function log_message(filename, target, size, word)

	local luci_util = require 'luci.util'

	local record = luci_util.exec("wc -l " .. filename)
	--luci_util.exec("echo wc -l " .. filename .. " > /tmp/console")
	local lines = luci_util.split(record)
	local count = #lines

	if count == 2 then
		elements = luci_util.split(lines[1], " ")
		local pages = math.floor(elements[1]/size) + 1
		local line = ""
		content = luci_util.exec("tail -n +" .. (pages-target)*size .. " " .. luci.util.trim(elements[2]))
		--luci_util.exec("echo 'tail -n +" .. (pages-target)*size .. " " .. luci.util.trim(elements[2]) .. " | head -n " .. size .. "' >> /tmp/console")
		record = luci_util.split(content)
		local cnt = 1
		while cnt <= #record and cnt <= size do
			line = line .. record[cnt] .. "\n"
			cnt = cnt + 1
		end
		if word ~= "" then
			luci_util.exec("rm -rf " .. elements[2])
		end
		return {syslog=line, count=pages, current=target, keyword=word}
	else
		elements = luci_util.split(luci.util.trim(lines[count-1]), " ")
		local total = elements[1] + 0
		local pages = math.floor(total/size) + 1
		local start = (pages - target)*size
		local i = count-2
		local line = ""
		local back = ""
		while i >= 1 do
			elements = luci_util.split(luci_util.trim(lines[i]), " ")
			if start-elements[1] > 0 then
				start = start-elements[1]
			else
				content = luci_util.exec("tail -n +" .. start .. " " .. elements[2])
				--luci_util.exec("echo 'tail -n +" .. start .. " " .. elements[2] .. " | head -n " .. size .. "' >> /tmp/console")
				record = luci_util.split(content)
				local cnt = 1
				while cnt <= #record and cnt <= size do
					line = line .. record[cnt] .. "\n"
					cnt = cnt + 1
				end
				if word == "" then
					if target ~= 1 and #record < size then
						elements = luci_util.split(luci_util.trim(lines[i-1]), " ")
						line = line .. luci_util.exec("head -n " .. (size-#record) .. " " .. elements[2])
						--luci_util.exec("echo head -n " .. (size-#record) .. " " .. elements[2] .. " >> /tmp/console")
					end
				else
					while #record < size do
						if i> 1 then
							elements = luci_util.split(luci_util.trim(lines[i-1]), " ")
							line = line .. luci_util.exec("head -n " .. (size-#record) .. " " .. elements[2])
							record = luci_util.split(line)
						else
							break
						end
						i = i-1
					end
					luci_util.exec("rm -rf " ..  filename)
				end
				break
			end
			i = i-1
		end
		return {syslog=line, count=pages, current=target, keyword=word}
	end
end
