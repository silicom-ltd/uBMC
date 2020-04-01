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

module("luci.controller.admin.aaa", package.seeall)

local dpt = require "luci.dispatcher"

function index()
	local product = require "luci.product"

	if product.verifyFeature('MENU', 'UPPERCASE') then
		entry({"admin", "aaa"}, alias("admin", "aaa", "index"), _("USER"), 50).index = true
	else
		entry({"admin", "aaa"}, alias("admin", "aaa", "index"), _("User"), 50).index = true
	end
	entry({"admin", "aaa", "index"}, cbi("admin/aaa-unix", {autoapply=true}), _("LOCAL"), 1)
	entry({"admin", "aaa", "radius"}, cbi("admin/aaa-radius", {autoapply=true}), _("RADIUS"), 10)
	entry({"admin", "aaa", "tacacs"}, cbi("admin/aaa-tacacs", {autoapply=true}), _("TACACS+"), 20)
	entry({"admin", "aaa", "change_passwd"}, call("action_change_passwd"), _("Change Password"), 90)
	--entry({"admin", "aaa", "change_passwd"}, cbi("admin/aaa-passwd", {autoapply=true}), _("Change Password"), 90)
end

function action_change_passwd()
	local mgmtdclient = require "mgmtdclient"
	local dpt = require "luci.dispatcher"
	local md_path = "/config/unix/user"
	local curr_user = dpt.context.authuser
	local curr_user_privil = dpt.context.user_privil
	local all_user = nil

	local user = luci.http.formvalue("target_user")
	local old_passwd = luci.http.formvalue("old_password")
	local new_passwd = luci.http.formvalue("new_password")
	local confirm = luci.http.formvalue("confirm")

	if new_passwd and confirm then
		if user == "$##$" then
			msg = "Please choose a local user!"
		elseif new_passwd == confirm then
			if curr_user_privil < dpt.PRIVILEGE_SUPER then
				user = curr_user
			end
			mgmtdclient.set_login(curr_user, curr_user_privil, dpt.context.auth_proto)
			ret = mgmtdclient.action("/action/aaa/change-passwd", {["user"]=user, ["curr-passwd"]=old_passwd, ["new-passwd"]=new_passwd})
			if ret == "OK" then
				msg = "Password for " .. user .. " has been changed!"
			else
				msg = "Failed to change password for " .. user .. " error: " .. ret
			end
		else
			msg = "Error! Password and confirmation password mismatch!"
		end
	end

	if curr_user_privil == dpt.PRIVILEGE_SUPER then
		ret, all_user = mgmtdclient.query_child(md_path)
		if ret ~= "OK" then
			msg = "Get user list error: " .. ret
		end
		if all_user == "null" then
			all_user = {}
		end
	end

	require("luci.i18n")
	require("luci.template")
	luci.template.render("admin/change_passwd", {err_msg=msg, curr_user=curr_user, all_user=all_user})
	return false
end
