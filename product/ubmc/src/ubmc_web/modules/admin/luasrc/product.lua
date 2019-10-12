local PRODUCT = {}

local config = require "luci.config"

PRODUCT.MENU = {
	UPPERCASE = 0,
	HELP = 0,
	LOGOUT = 1,
	SAVE = 1,
	LICENSE = 0,
}

PRODUCT.SNMP = {
    TRAP = 0,
    THRESHOLD = 0,
}

PRODUCT.STATUS = {
	RESOURCE = 0,
	MODULE = 0,
}

PRODUCT.SYSTEM = {
	IPSEC = 1,
	IPTABLE = 1,
	KEYCERT = 1,
	PHONEHOME = 1,
	HALT = 0,
}

PRODUCT.SYSTEM_MGMT = {
	INTERFACE = 0,
	INTERFACE_LIST = 1,
	CLI_ESSION = 1,
	VRF_LIST = 1,
	VRF_PROCESS_LIST = 1,
	ROUTE_LIST = 1,
	ADDRESS_LIST = 1,
	PERMIT_LIST = 1,
	DNS = 1
}

PRODUCT.SNMP_AGENT = {
	STATE = 0,
	COMMUNITIES = 0,
	USERS = 0,
}

PRODUCT.SYSTEM_SERVICE = {
	HTTP = 1,
	HTTPS = 1,
}

PRODUCT.SYSTEM_MISC = {
	LOGIN_BANNER = 1
}

PRODUCT.NAVIGATION = {
	ALARM = 0
}

local function verifyFeature(name, subname)
	if config[name] ~= nil and config[name][subname] ~= nil then
		return config[name][subname] == '1'
	end
	return PRODUCT[name][subname] == 1
end

PRODUCT.verifyFeature = verifyFeature

return PRODUCT
