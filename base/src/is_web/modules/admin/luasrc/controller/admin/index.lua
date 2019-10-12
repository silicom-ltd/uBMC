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

module("luci.controller.admin.index", package.seeall)

function index()
	require("luci.config")

	local root = node()
	if not root.lock then
		root.target = alias("admin")
		root.index = true
	end

	--entry({"about"}, template("about"))
	local product = require "luci.product"

	local page   = entry({"admin"}, alias("admin", "index"), _("Essentials"), 10)
	page.target  = alias("admin", "status")
	page.title   = _("Administration")
	page.order   = 10
	page.sysauth = luci.config.sauth.defaultuser
	page.sysauth_authenticator = "pam_htmlauth"
	page.index = true

	--entry({"admin", "index"}, alias("admin", "index", "index"), _("Overview"), 10).index = true
	--entry({"admin", "index", "index"}, form("admin/index"), _("General"), 1).ignoreindex = true

	if product.verifyFeature('MENU', 'HELP') then
		entry({"admin", "help"}, call("action_help"), _("HELP"))
	end

	if product.verifyFeature('MENU', 'LOGOUT') then
		entry({"admin", "logout"}, call("action_logout"), _("Logout"))
	else
		entry({"admin", "logout"}, call("action_logout"), _(""))
	end

	if product.verifyFeature('MENU', 'SAVE') then
		if product.verifyFeature('MENU', 'UPPERCASE') then
			entry({"admin", "save"}, call("action_save"), _("SAVE"))
		else
			entry({"admin", "save"}, call("action_save"), _("Save"))
		end
	end

	if product.verifyFeature('MENU', 'LICENSE') then
		entry({"admin", "license"}, call("action_license"), _(""))
	end
end

function action_logout()
	local dsp = require "luci.dispatcher"
	local sauth = require "luci.sauth"
	if dsp.context.authsession then
		sauth.kill(dsp.context.authsession)
		dsp.context.urltoken.stok = nil
	end

	luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
	luci.http.redirect(luci.dispatcher.build_url())
end

function action_help()
	local mgmtdclient = require "mgmtdclient"
	ret, software = mgmtdclient.query_child("/status/switch/info/sw_version")
	ret, serial = mgmtdclient.query_child("/status/switch/info/device_tn")
	luci.template.render("admin/help", {sw=software, sr=serial})
end

function action_license()
	local dsp = require "luci.dispatcher"
	if luci.http.formvalue("noaccept") then
		local sauth = require "luci.sauth"
		if dsp.context.authsession then
			sauth.kill(dsp.context.authsession)
			dsp.context.urltoken.stok = nil
		end

		luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
		luci.http.redirect(luci.dispatcher.build_url())
		return
	end

	if luci.http.formvalue("accept") then
		sync_execute("touch /config/EULA.accept")
		local page = dsp.build_url("/admin/status")
		luci.http.redirect(page)
		return
	end

	local accept = io.open("/config/EULA.accept", "r")
	if accept ~= nil then
		local page = dsp.build_url("/admin/status")
		luci.http.redirect(page)
		return
	end

	local f=io.open("/etc/EULA.txt", "r")
	local content = f:read("*a")
	luci.template.render("admin/license", {cont=content})
end

function action_rollback()
	luci.template.render("admin/rollback")
end

function action_save()
	require("luci.i18n")
	require("luci.template")

	local mgmtdclient = require "mgmtdclient"
	local dpt = require "luci.dispatcher"
	if dpt.context.user_privil < dpt.PRIVILEGE_SUPER then
		luci.template.render("admin/forbidden")
		return false
	end
	local curr_user = dpt.context.authuser

	local save = luci.http.formvalue("save")
	local commit_ret
	local saved
	if save then
		mgmtdclient.set_login(curr_user, dpt.context.user_privil, dpt.context.auth_proto)
		commit_ret = mgmtdclient.action("/action/system/save-config", "")
	end
	query_ret, saved = mgmtdclient.query("/status/system/config-saved", "")

	luci.template.render("admin/save", {result=commit_ret, saved=saved})
	return false
end

function sync_execute(command)
	-- returns err_code, output.
	local f = io.popen(command..' 2>&1 ; echo $?')
	local output = f:read("*a")
	os.execute("echo '"..output.."' >> /tmp/syslog;sync;")
	f:close()
end
