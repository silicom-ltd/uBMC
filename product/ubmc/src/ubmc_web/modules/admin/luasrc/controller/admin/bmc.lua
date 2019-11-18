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

module("luci.controller.admin.bmc", package.seeall)

local dpt = require "luci.dispatcher"
local mgmtdclient = require "mgmtdclient"

function index()
	local dpt = require "luci.dispatcher"
	entry({"admin", "bmc"}, alias("admin", "bmc", "index"), _("BMC"), 45).index = true
	entry({"admin", "bmc", "index"}, cbi("admin/bmc-host", {autoapply=true}), _("Console Redirection"), 1)
	entry({"admin", "bmc", "shell"}, call("action_console"), _("Console Shell"), 2)
	entry({"admin", "bmc", "console"}, call("host_serial"), _("Console Log"), 2)
	entry({"admin", "bmc", "power"}, call("action_power"), _("Power Control"), 3)
if string.find(dpt.vendor, 'ATT') == nil then
	entry({"admin", "bmc", "bios"}, call("action_bios"), _("BIOS"), 4)
end
	entry({"admin", "bmc", "usb"}, call("usb_config"), _("USB CDROM"), 5)
	entry({"admin", "bmc", "eventlog"}, call("action_eventlog"), _("System Event Log"), 100)
end

function check_privil()
	if dpt.context.user_privil < dpt.PRIVILEGE_SUPER then
		luci.template.render("admin/forbidden")
		return false
	end
	mgmtdclient.set_login(dpt.context.authuser, dpt.context.user_privil, dpt.context.auth_proto)
	return true
end

function set_file_handler(f)
	-- Install upload handler
	local file
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not nixio.fs.access(f) and not file and chunk and #chunk > 0 then
				file = io.open(f, "w")
			end
			if file and chunk then
				file:write(chunk)
			end
			if file and eof then
				file:close()
			end
		end
	)
end

function action_console()
    luci.template.render("admin/console", {})
end

function action_bios()
	if not check_privil() then
		return false
	end

	local tmpfile = "/tmp/bmc_web_upload_bios.img"
	set_file_handler(tmpfile)

	local err, msg
	local _step = tonumber(luci.http.formvalue("step") or 1)
	local _all = tonumber(luci.http.formvalue("all") or 0)
	local has_image = nixio.fs.access(tmpfile)
	local _verify_result = {}

	if has_image then
		if nixio.fs.stat(tmpfile).size == 16777216 then
			if _step == 3 then
				luci.http.prepare_content("application/json")
				ret, _status = mgmtdclient.query_child("/status/bmc/bios/upgrade")
				local array = {}
				if ret == 'OK' then
					if _status == '' or string.find(_status, "OK") ~= nil then
						ret = mgmtdclient.action('/action/bmc/bios/upgrade', {["image"]=tmpfile})
						array[1] = {status=1}
					else
						array[1] = {status=2}
					end
				else
					array[1] = {status=0}
				end
				luci.http.write_json(array)
				return
			elseif _step == 4 then
				luci.http.prepare_content("application/json")
				ret, _status = mgmtdclient.query_child("/status/bmc/bios/upgrade")
				local array = {}
				if ret == 'OK' then
					if string.find(_status, "OK") ~= nil then
						nixio.fs.unlink(tmpfile)
					end
					if string.find(_status, "Error") ~= nil then
						nixio.fs.unlink(tmpfile)
					end
					array[1] = {status=_status}
				else
					nixio.fs.unlink(tmpfile)
					array[1] = {}
				end
				luci.http.write_json(array)
				return
			else 
				_step = 2
			end
		else 
			_verify_result['err_code'] = 1
			nixio.fs.unlink(tmpfile)
			_step = 1
		end
	else
		if _step == 2 then
			_verify_result['err_code'] = 1
		end
		_step = 1
	end

	luci.template.render("admin/bios", {
		all=_all,
		step=_step,
		verify_result=_verify_result,
	})
end

function action_eventlog()
	if not check_privil() then
		return false
	end

	local luci_http = require "luci.http"
	local luci_util = require 'luci.util'

	local clear = luci_http.formvalue("clear")
	if clear then
		local mgmtdclient = require "mgmtdclient"
		--mgmtdclient.set_login(dpt.context.authuser, dpt.context.user_privil, dpt.context.auth_proto)
		local ret = mgmtdclient.action('/action/bmc/sel/clear', "")
	end

	local record = luci_util.exec("ubmc_ipmi_sel_prt")
	local target = luci_http.formvalue("page")
	local lines = luci_util.split(record)
	local list = {}

	local count = #lines
	local size = 30
	local total = math.floor(count/size)+1

	if target then
		target = target + 0
		if target > total then
			target = total
		end
	else
		target = 1
	end

	for num, line in pairs(lines) do
		list[num] = luci_util.split(line, '|')
	end
	table.sort(list, function(a, b) return a[1] > b[1] end)

	local event = {}
	local loop = 1
	for key, val in pairs(list) do
		if key >= (target-1)*size then
			if loop > size then
				break
			end
			event[loop] = val
			loop = loop + 1
		end
	end

	luci.template.render("admin/eventlog", {
		events=event,
		current=target,	
		count=math.floor(count/size)+1
	})
end

function usb_config()
	if not check_privil() then
		return false
	end

	local iso_dir = "/var/images"
	local tmpfile = "/var/images/usb_image_"..os.time().."_"..math.random(1,1000)..".iso"
	set_file_handler(tmpfile)

	local mode = "list"
	local err, msg
--[[
	if luci.http.formvalue("upload") then
		local iso_name = #(luci.http.formvalue("upload_file_name")) > 0 and luci.http.formvalue("upload_file_name") or "usb_image_"..os.date("%Y%m%d%H%M%S")
		nixio.fs.move(tmpfile, iso_dir.."/"..iso_name..".iso")
		msg = "New configuration "..iso_name.." uploaded!"
	elseif luci.http.formvalue("save") then
		local iso_file = luci.http.formvalue("file_name")
		local ret = mgmtdclient.action("/action/bmc/usb-cdrom/attach-local", {["image"]=iso_file})
		if ret == "OK" then
			msg = "Attach local image "..iso_file.." Successfully."
		else
			err = ret
		end
	elseif luci.http.formvalue("attach") then
		local remote_path = luci.http.formvalue("remote_path") 
		local remote_user = luci.http.formvalue("remote_user") 
		local remote_address = luci.http.formvalue("remote_address") 
		local remote_password = luci.http.formvalue("remote_password") 
		ret = mgmtdclient.action("/action/bmc/usb-cdrom/attach-remote", {["address"]=remote_address, ["path"]=remote_path, ["user"]=remote_user, ["password"]=remote_password})
		if ret == "OK" then
			msg = "Attach remote image successfully."
		else
			err = "Remote ISO file cannot be found, please select a valid one!"
		end
	elseif luci.http.formvalue("detach") then
		ret = mgmtdclient.action('/action/bmc/usb-cdrom/detach', "")
		if ret == "OK" then
			msg = "Detach image successfully."
		else
			err = ret
		end
	end
]]--

	if luci.http.formvalue("detach") then
		ret = mgmtdclient.action('/action/bmc/usb-cdrom/detach', "")
		if ret == "OK" then
			msg = "Detach image successfully."
		else
			err = ret
		end
	elseif luci.http.formvalue("attach") then
		local range = luci.http.formvalue("attach_range")
		if range == 'local' then
			local iso_file = luci.http.formvalue("file_name")
			local ret = mgmtdclient.action("/action/bmc/usb-cdrom/attach-local", {["image"]=iso_file})
			if ret == "OK" then
				msg = "Attach local image "..iso_file.." Successfully."
			else
				err = ret
			end
		elseif range == 'remote' then
			local remote_path = luci.http.formvalue("remote_path") 
			local remote_user = luci.http.formvalue("remote_user") 
			local remote_address = luci.http.formvalue("remote_address") 
			local remote_password = luci.http.formvalue("remote_password") 
			ret = mgmtdclient.action("/action/bmc/usb-cdrom/attach-remote", {["address"]=remote_address, ["path"]=remote_path, ["user"]=remote_user, ["password"]=remote_password})
			if ret == "OK" then
				msg = "Attach remote image successfully."
			else
				if string.find(ret, "Internal") ~= nil then
					err = "Remote ISO file cannot be found, please select a valid one!"
				else
					err = ret 
				end
			end
		end
	end

	ret, usb_state = mgmtdclient.query_child("/status/bmc/usb-cdrom/enabled")
	ret, usb_image = mgmtdclient.query_child("/status/bmc/usb-cdrom/image")

	local iso_list = {}
	local function ends_with(str, ending)
		return ending == "" or str:sub(-#ending) == ending
	end
	for f in io.popen("ls -t "..iso_dir.."/"):lines() do
		if ends_with(f, ".iso") then
			file = {}
			file.name = f
			file.mtime = os.date("%Y%m%d %H:%M:%S", nixio.fs.stat(iso_dir.."/"..f).mtime)
			table.insert(iso_list, file)
		end
	end
	luci.template.render('admin/usb_config', {err=err, msg=msg, state=usb_state, image=usb_image, iso_list=iso_list})
end

function action_power()

	if not check_privil() then
		return false
	end

	local act
	if luci.http.formvalue("act") then
		act = luci.http.formvalue("act")
	end

	local message
	if act ~= "" then
		message = mgmtdclient.action('/action/bmc/power/control', act)
	end

	ret, power_state = mgmtdclient.query_child("/status/bmc/power/state")
	ret, power_action = mgmtdclient.query_child("/status/bmc/power/action")
	luci.template.render("admin/bmc_power", {state=power_state, action=power_action, msg=message})
end

function host_serial()

	local total = 120
	local luci_http = require "luci.http"
	local luci_util = require "luci.util"

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
		luci.template.render("admin/bmc_console", log_message("/var/log/host_serial*", target, total, matching))
	else
		luci.template.render("admin/bmc_console", log_message_search("/var/log", "host_serial*", target, total, matching))
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
	local lines = luci_util.split(record)
	local count = #lines

	if count == 2 then
		elements = luci_util.split(lines[1], " ")
		local pages = math.floor(elements[1]/size) + 1
		local line = luci_util.exec("tail -n +" .. (pages-target)*size .. " " .. luci.util.trim(elements[2]) .. " | head -n " .. size)
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
				line = luci_util.exec("tail -n +" .. start .. " " .. elements[2] .. " | head -n " .. size)
				record = luci_util.split(line)
				if word == "" then
					if target ~= 1 and #record < size then
						elements = luci_util.split(luci_util.trim(lines[i-1]), " ")
						line = line .. luci_util.exec("head -n " .. (size-#record) .. " " .. elements[2])
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
