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

module("luci.controller.admin.system", package.seeall)

require("luci.i18n")
require("luci.template")
local dpt = require "luci.dispatcher"
local mgmtdclient = require "mgmtdclient"

function index()
	local dpt = require "luci.dispatcher"

	local product = require "luci.product"

if product.verifyFeature('MENU', 'UPPERCASE') then
	entry({"admin", "system"}, alias("admin", "system", "index"), _("SYSTEM"), 40).index = true
else
	entry({"admin", "system"}, alias("admin", "system", "index"), _("System"), 40).index = true
end

	entry({"admin", "system", "index"}, cbi("admin/system-misc", {autoapply=true}), _("General"), 10)
	entry({"admin", "system", "service"}, cbi("admin/system-service", {autoapply=true}), _("Service"), 30)
	entry({"admin", "system", "mgmt"}, cbi("admin/system-mgmt", {autoapply=true}), _("Management Interface"), 50)

if product.verifyFeature('SYSTEM', 'IPSEC') then
	entry({"admin", "system", "ipsec"}, cbi("admin/system-ipsec", {autoapply=true}), _("IPsec"), 51)
end
if product.verifyFeature('SYSTEM', 'IPTABLE') then
	entry({"admin", "system", "iptable"}, cbi("admin/system-iptable", {autoapply=true}), _("Iptables"), 52)
end
if product.verifyFeature('SYSTEM', 'KEYCERT') then
	entry({"admin", "system", "keycert"}, cbi("admin/system-keycert"), _("Keys/Certs"), 53)
end
	entry({"admin", "system", "configs"}, call("action_configs"), _("Configurations"), 70)

if product.verifyFeature('SYSTEM', 'PHONEHOME') then
	entry({"admin", "system", "phonehome"}, call("action_phonehome"), _("Phone Home"), 75)
end
	entry({"admin", "system", "dump"}, call("action_dump"), _("System Dump"), 80)
	entry({"admin", "system", "upgrade"}, call("action_upgrade"), _("Upgrade/Switch"), 90)
if product.verifyFeature('MENU', 'UPPERCASE') then
	entry({"admin", "system", "reboot"}, call("action_reboot"), _("Reboot/Shutdown/Reset"), 100)
else
	if product.verifyFeature('SYSTEM', 'HALT') then
		entry({"admin", "system", "reboot"}, call("action_reboot"), _("Reboot/Halt/Reset"), 100)
	else
		entry({"admin", "system", "reboot"}, call("action_reboot"), _("Reboot/Reset"), 100)
	end
end
	entry({"admin", "system", "syscheck"}, call("action_syscheck"), _(""), 0)
	entry({"admin", "system", "switch"}, call("action_switch"), _(""), 0)
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

function action_keycert()
	if not check_privil() then
		return false
	end

	local tmpfile = "/tmp/ipsec_"..os.time().."_"..math.random(1,1000)..".key.cert"
	set_file_handler(tmpfile)

	local err, msg		

	function get_file_content(file)
		local f = io.open(file, "r")
		local file_content = f:read("*a")
		f:close()
		return file_content
	end

	local ret, key_list = mgmtdclient.query_sub("/config/system/mgmt/key-list")
	local ret, cert_list = mgmtdclient.query_sub("/config/system/mgmt/cert-list")

	if luci.http.formvalue("upload") then
		filename = luci.http.formvalue("upload_file")
		mode = luci.http.formvalue("mode")
		tbl = {}
		if mode == "0" then
			if key_list ~= nil then
				if type(key_list) == 'table' then
					for n in pairs(key_list) do
						if filename == n then
							err = 'The upload file has existed.'
							break
						end
					end
				end
			end

			if err == nil then
				ret = mgmtdclient.add("/config/system/mgmt/key-list/"..filename, {["key-content"]=get_file_content(tmpfile)})
				if ret == "OK" then
					table.insert(key_list, {["key-ID"]=filename})
				end
			end
		else
			ret = mgmtdclient.add("/config/system/mgmt/cert-list/"..filename, {["cert-content"]=get_file_content(tmpfile)})
		end
		if ret ~= "OK" then
			err = string.format("add error: %s!!!!!", ret)
			--	err = "The uploaded configuration is invalid. Configuration file must be downloaded from device and can not be modified manually !"
		end
		nixio.fs.unlink(tmpfile)
	end
--[[
	if ret == "OK" and #list_str > 0 then
		-- filelist = list_str:gmatch("[^%s]+")
		for file in list_str do
			table.insert(key_list, file)
		end
	end
]]--

	luci.template.render("admin/keycert", {mode=mode, err=err, msg=msg, key_list=key_list, cert_list=cert_list})

	return true
end

function action_configs()
	if not check_privil() then
		return false
	end

	local cfg_dir = "/config/extend"
	local tmpfile = "/tmp/is_sys_"..os.time().."_"..math.random(1,1000)..".conf"
	set_file_handler(tmpfile)

	local mode = "list"
	local err, msg
	local cfg_file = luci.http.formvalue("config_file")

	function get_cfg_content(cfg_file)
		local f = io.open(cfg_dir.."/"..cfg_file, "r")
		local cfg_file_content = f:read("*a")
		f:close()
		return cfg_file_content
	end

	if luci.http.formvalue("view") or luci.http.formvalue("restore") or luci.http.formvalue("delete") or luci.http.formvalue("download") then
		if not cfg_file then
			err = "Please select a configuration!"
		elseif not nixio.fs.access(cfg_dir.."/"..cfg_file) then
			err = "Configuration file "..cfg_file.." is missing!"
		elseif luci.http.formvalue("view") then
			mode = "view"
		elseif luci.http.formvalue("restore") then
			local ret = mgmtdclient.action("/action/system/load-config-from", cfg_file)
			if ret == "OK" then
				mgmtdclient.action("/action/system/reboot", "")
				msg = "Configuration "..cfg_file.." has been restored! The device is now rebooting......"
			else
				err = ret
			end
		elseif luci.http.formvalue("delete") then
			local ret = mgmtdclient.action("/action/system/remove-config-file", cfg_file)
			if ret == "OK" then
				msg = "Configuration "..cfg_file.." deleted!"
			else
				err = ret
			end
		elseif luci.http.formvalue("download") then
			local cfg_file_content = get_cfg_content(cfg_file)
			luci.http.header('Content-Disposition', 'attachment; filename="%s"' % {cfg_file})
			luci.http.prepare_content("application/octet-stream")
			luci.http.write(cfg_file_content)
			return
		end
	elseif luci.http.formvalue("save") then
		local cfg_name = #(luci.http.formvalue("save_file_name")) > 0 and luci.http.formvalue("save_file_name") or "is_config_"..os.date("%Y%m%d%H%M%S")
		if string.find(cfg_name, "^[%w_%-%.]+$") then
			local ret = mgmtdclient.action("/action/system/save-config-as", cfg_name)
			if ret == "OK" then
				msg = "Configuration saved as "..cfg_name.."."
			else
				err = ret
			end
		else
			err = "Invalid file name, only support [a~z][A~Z][0~9][._-]."
		end
	elseif luci.http.formvalue("upload") then
		local cfg_name = #(luci.http.formvalue("upload_file_name")) > 0 and luci.http.formvalue("upload_file_name") or "is_config_"..os.date("%Y%m%d%H%M%S")
		if luci.http.formvalue("upload_file_name") and nixio.fs.access(tmpfile) then
			if string.find(cfg_name, "^[%w_%-%.]+$") then
				local ret = sync_execute("is_check_conf.sh "..tmpfile)
				if ret.output:sub(1,2) == "OK" then
					nixio.fs.move(tmpfile, cfg_dir.."/"..cfg_name)
					msg = "New configuration "..cfg_name.." uploaded!"
				else
					nixio.fs.unlink(tmpfile)
					err = "The uploaded configuration is invalid. Configuration file must be downloaded from device and can not be modified manually !"
				end
			else
				err = "Invalid file name, only support [a~z][A~Z][0~9][._-]."
			end
		else
			err = "Please upload a configuration file!"
		end
	elseif luci.http.formvalue("reset") then
		local ret = mgmtdclient.action("/action/system/load-default-config", "")
		if ret == "OK" then
			mgmtdclient.action("/action/system/reboot", "")
			msg = "Configuration has been reset to default! The device is rebooting......"
		else
			err = ret
		end
	end

	if mode == "list" then
		local cfg_list = {}
--[[
		local ret, list_str = mgmtdclient.query("/status/system/config-file-list")
		if ret == "OK" and #list_str > 0 then
			filelist = list_str:gmatch("[^%s]+")
			for file in filelist do
				table.insert(cfg_list, file)
			end
		end
]]--
		for f in io.popen("ls -t "..cfg_dir.."/"):lines() do
			file = {}
			file.name = f
			file.mtime = os.date("%Y%m%d %H:%M:%S", nixio.fs.stat(cfg_dir.."/"..f).mtime)
			table.insert(cfg_list, file)
		end
		luci.template.render("admin/configs", {mode=mode, err=err, msg=msg, cfg_list=cfg_list})
	elseif mode == "view" then
		local cfg_file_content = get_cfg_content(cfg_file)
		luci.template.render("admin/configs", {mode=mode, cfg_file=cfg_file, cfg_file_content=cfg_file_content})
	end

	return true
end

function action_dump()
	if not check_privil() then
		return false
	end

	local dump_dir = "/var/log/dump"
	local err, msg
	if luci.http.formvalue("delete") then
		local dump_files = luci.http.formvaluetable("cbid.dumpfile.check")
		if not next(dump_files) then
			err = "Please select a dump file!"
		else
			for k,v in pairs(dump_files) do
				if nixio.fs.access(dump_dir.."/"..v) then
					nixio.fs.unlink(dump_dir.."/"..v)
				end
			end
			msg = "Delete dump file successfully!"
		end
	elseif luci.http.formvalue("download") then
		local dump_file = luci.http.formvalue("dump_file")
		local f = io.open(dump_dir.."/"..dump_file, "r")
		local dump_file_content = f:read("*a")
		f:close()
		luci.http.header('Content-Disposition', 'attachment; filename="%s"' % {dump_file})
		luci.http.prepare_content("application/octet-stream")
		luci.http.write(dump_file_content)
		os.execute("sleep 2")
		return
	elseif luci.http.formvalue("create") then
		--local filename = "is_log_"..os.date("%Y%m%d%H%M%S")
		--local dump_file = dump_dir.."/"..filename
		--os.execute("is_logdump.sh "..filename);
		local ret = mgmtdclient.action("/action/system/create-log-dump", "")
		if ret == "OK" then
			msg = "Create log dump file successfully!"
		else
			err = ret
		end
	end

	local dump_list = {}
	if nixio.fs.access(dump_dir) then
		for f in io.popen("ls -t "..dump_dir):lines() do
			file = {}
			file.name = f
			file.size = nixio.fs.stat(dump_dir.."/"..f).size
			file.mtime = os.date("%Y%m%d %H:%M:%S", nixio.fs.stat(dump_dir.."/"..f).mtime)
			table.insert(dump_list, file)
		end
	end
	luci.template.render("admin/dump", {err=err, msg=msg, dump_list=dump_list})
	return true
end

function action_reboot()
	if not check_privil() then
		return false
	end

	local act
	local dpt = require "luci.dispatcher"
	--[[if luci.http.formvalue("reboot") then
		act = "reboot"
	elseif luci.http.formvalue("halt") then
		act = "halt"
	end]]--
	if luci.http.formvalue("act") then
		act = luci.http.formvalue("act")
	end

	luci.template.render("admin/reboot", {action=act})

	if act == "reboot" then
		mgmtdclient.action('/action/system/reboot', "")
	elseif act == "halt" then
		mgmtdclient.action('/action/system/halt', "")
	elseif act == "shut down" then
		mgmtdclient.action('/action/system/halt', "")
	elseif act == "reset" then
		mgmtdclient.action('/action/system/reset', "")
	end
end

function action_switch()
	if not check_privil() then
		return false
	end

	local method = luci.http.getenv("REQUEST_METHOD")
	if method ~= "POST" then
		return false
	end

	mgmtdclient.action('/action/system/switch-software', "")
	luci.template.render("admin/reboot", {action="reboot"})
	mgmtdclient.action('/action/system/reboot', "")
end

function action_upgrade()
	if not check_privil() then
		return false
	end

	local upgrade_tool = "/usr/sbin/is_upgrade.sh"
	local upgrade_status = "/tmp/is_upgrade.status"
	local upgrade_log = "/tmp/is_upgrade.log"
	local upgrade_pid = "/tmp/is_upgrade.pid"
	local tmpfile = "/var/log/tmp/is_upgrade_pkg.img"

	local function get_current_info()
		return sync_execute(upgrade_tool .. " -S")
	end

	local function get_upgrade_flag()
		ret = sync_execute(upgrade_tool .. " -s")
		_,_,flag = ret.output:find("software upgrade flag:(%d+)")
		return tonumber(flag)
	end

	local function get_upgrade_status()
		local status, progress
		if not nixio.fs.access(upgrade_status) then
			status = "Error Unknown"
			progress = "0"
		else
			local f = io.open(upgrade_status, "r")
			local content = f:read("*a")
			f:close()
			if content ~= '' then
				_,_,status,progress = content:find("(%C+)\n(%d+)")
				if status == nil then
					os.execute("echo '"..content.."' >> /tmp/syslog;sync;")
					status = "Error Unknown"
					progress = "0"
				end
			end
		end
		return status, progress
	end

	local function verify_image()
		return sync_execute(upgrade_tool .. " -V " .. tmpfile)
	end

	local function upgrade_image()
		--return sync_execute(upgrade_tool .. " -p -u " .. tmpfile)
		-- ensure the upgrade_status is ready
		os.execute("echo -e 'Not starting\n0' > " .. upgrade_status .. ";sync;")
		return os.execute(upgrade_tool .. " -p -u > " .. upgrade_log .. " & echo $! > " .. upgrade_pid)
	end

	local function upgrade_image_m()
		--return sync_execute(upgrade_tool .. " -p -u " .. tmpfile)
		-- ensure the upgrade_status is ready
		os.execute("echo -e 'Not starting\n0' > " .. upgrade_status .. ";sync;")
		return os.execute(upgrade_tool .. " -p -M > " .. upgrade_log .. " & echo $! > " .. upgrade_pid)
	end

	local function storage_size()
		local size = 0
		if nixio.fs.access("/proc/mtd") then
			for l in io.lines("/proc/mtd") do
				local d, s, e, n = l:match('^([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+"([^%s]+)"')
				if n == "linux" then
					size = tonumber(s, 16)
					break
				end
			end
		elseif nixio.fs.access("/proc/partitions") then
			for l in io.lines("/proc/partitions") do
				local x, y, b, n = l:match('^%s*(%d+)%s+(%d+)%s+([^%s]+)%s+([^%s]+)')
				if b and n and not n:match('[0-9]') then
					size = tonumber(b) * 1024
					break
				end
			end
		end
		return size
	end

	set_file_handler(tmpfile)

	-- Determine state
	local step         = tonumber(luci.http.formvalue("step") or 1)
	local has_image    = nixio.fs.access(tmpfile)
	local has_upgrade  = nixio.fs.access(upgrade_status)
	local has_platform = nixio.fs.access(upgrade_tool)
	local has_upload   = luci.http.formvalue("image")

	if has_image then
		if has_upgrade then
			if step == 1 then
				local sta, prog = get_upgrade_status()
				if sta:sub(1, 5) == "Error" then
					-- error, retry
					nixio.fs.unlink(tmpfile)
					nixio.fs.unlink(upgrade_status)
				else
					-- show upgrade progress
					step = 3
				end
			end
		else
			-- only upload not upgrade yet
			if step == 1 then
				if luci.http.formvalue("cancel") then
					nixio.fs.unlink(tmpfile)
				else
					step = 2
				end
			end
		end
	else
		step = 1
	end

	--
	-- This is step 1-3, which does the user interaction and
	-- image upload.
	--

	-- Step 1: file upload, error on unsupported image format
	if step == 1 then
		local upg_flag = 1
		if has_platform then
			if 0 ~= get_upgrade_flag() then
				upg_flag = 2
			end
		else
			upg_flag = 0
		end
		luci.template.render("admin/upgrade", {
			step=1,
			current_info=get_current_info(),
			verify_result={},
			upg_flag=upg_flag,
		} )

	-- Step 2: present uploaded file, show checksum, confirmation
	elseif step == 2 then
		local verify_ret = verify_image()
		if verify_ret.err_code == 0 then
			luci.template.render("admin/upgrade", {
				step=2,
				verify_result=verify_ret,
			} )
		else
			nixio.fs.unlink(tmpfile)
			luci.template.render("admin/upgrade", {
				step=1,
				current_info=get_current_info(),
				verify_result=verify_ret,
				upg_flag=1,
			} )
		end

	-- Step 3: load iframe which calls the actual flash procedure
	elseif step == 3 then
		--local upgrade_ret = has_image and upgrade_image() or {}
		if not has_upgrade and luci.http.formvalue("proceed") then
			if luci.http.formvalue("manufacture") then
				local currentInfo = get_current_info()
				local output = currentInfo.output
				if output:find("MANUFACTURE") then 
					upgrade_image_m()
				else
					nixio.fs.unlink(tmpfile)
					luci.template.render("admin/upgrade", {
						step=1,
						current_info=get_current_info(),
						verify_result=verify_ret,
						upg_flag=1,
					} )
				end
			else
				upgrade_image()
			end
		end
		luci.template.render("admin/upgrade", {
			step=3,
			--upgrade_result=upgrade_ret,
		} )

	elseif step == 4 then
		-- refresh upgrade progress
		luci.http.prepare_content("application/json")
		local array = {}
		local sta, prog = get_upgrade_status()
		array[1] = {status=sta, progress=prog}
		luci.http.write_json(array)

	end
end

function action_syscheck()
	luci.http.prepare_content("application/json")
	ret, is_saved = mgmtdclient.query_child("/status/system/config-saved")

	local array = {}
	array["config-saved"] = is_saved

	luci.http.write_json(array)
end

function sync_execute(command)
	-- returns err_code, output.
	local f = io.popen(command..' 2>&1 ; echo $?')
	local output = f:read("*a")
	f:close()
	local begin, finish, code = output:find("(%d+)\n$")
	output, code = output:sub(1, begin-1), tonumber(code)
	return {err_code=code, output=output}
end

function action_phonehome()
	if not check_privil() then
		return false
	end

	if luci.http.formvalue("action") then
		act = luci.http.formvalue("action")
		if act == 'Start' then
			mgmtdclient.action("/action/bmc/phonehome/enabled", 1)
		else
			mgmtdclient.action("/action/bmc/phonehome/enabled", 0)
		end
	end

	ret, sta = mgmtdclient.query_child("/status/bmc/phonehome/state")
	luci.template.render("admin/phonehome", {status=sta})
	return true
end
