local nixio = require "nixio", require "nixio.util"
local fs = require "nixio.fs"
require "luci.sys"

function showsess()
	local f = io.popen("uci show luci.sauth")
	local output = f:read("*a")
	f:close()
	_,_,sessiontime = output:find("sessiontime=(%S+)")
	_,_,sessionpath = output:find("sessionpath=(%S+)")

	if not fs.access(sessionpath) then
		return
	end

	local id
	for id in fs.dir(sessionpath) do
		local blob = fs.readfile(sessionpath .. "/" .. id)
		local func = loadstring(blob)
		setfenv(func, {})
		local sess = func()
		if sess.atime and sess.atime + sessiontime < luci.sys.uptime() then
			fs.unlink(sessionpath .. "/" .. id)
		else
			print(id .." ".. sess.user .." ".. sess.logname .." ".. os.date("%Y-%m-%d %X", sess.ctime) .." ".. sess.from  .." ")
		end
	end
end

local args={...}

if args[1] == "showsess" then
	showsess()
else
	print("Invalid command!")
end

