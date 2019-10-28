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
	[1] = {name=ROOTNODE, tag="IPsec State", note="State", multiple=false, addremove=false},
	[2] = {name="ipsec-conn", tag="IPsec Connections", note="", multiple=true, addremove=true},
	[3] = {name="ipsec-ca", tag="IPsec CAs", note="", multiple=true, addremove=true},
	[4] = {name="ipsec-secret", tag="IPsec Secrets", note="", multiple=true, addremove=true},
}

--[[
initial mdForm instance
]]--
mdForm = MdForm("ipsec", "/config/system/mgmt", PRIVILEGE_SUPER, section_cfg, translate("IPsec"))
mdForm:prepare_all()

--[[
get the basic config
]]--
baseConfig = mdForm:get_section_obj(ROOTNODE)
baseConfig:option(TfFlag, "ipsec-enabled", translate("Enabled"))

--[[
list the conn-name for ipsec conn
]]--
ipsecConfig = mdForm:get_section_obj("ipsec-conn")
ipsecConfig.template = "cbi/tsection"
ipsecConfig.sectionhead = translate("Connection")
ipsecType = ipsecConfig:option(ListValue, "type", translate("Type"))
ipsecType:value('tunnel', 'Tunnel')
ipsecType:value('transport', 'Transport')
ipsecType:value('transport_proxy', 'Transport Proxy')
ipsecType:value('passthrough', 'Passthrough')
ipsecType:value('drop', 'Drop')

keyexchange = ipsecConfig:option(ListValue, "keyexchange", translate("Key Exchange"))
keyexchange:value('ike', 'ike')
keyexchange:value('ikev1', 'ikev1')
keyexchange:value('ikev2', 'ikev2')
keyexchange.default = 'ikev2'

ipsecConfig:option(Value, "keyingtries", translate("Keying Tries"))

ipsecConfig:option(Value, "ike", translate("IKE"))
ipsecConfig:option(Value, "esp", translate("ESP"))

compress = ipsecConfig:option(ListValue, "compress", translate("Compress"))
compress:value('no', 'no')
compress:value('yes', 'yes')

ipsecConfig:option(Value, "replay-window", translate("Replay-window"))

dpdaction = ipsecConfig:option(ListValue, "dpdaction", translate("DPD Action"))
dpdaction:value('none', 'none')
dpdaction:value('clear', 'clear')
dpdaction:value('hold', 'hold')
dpdaction:value('restart', 'restart')

ipsecConfig:option(Value, "dpddelay", translate("DPD Delay"))
ipsecConfig:option(Value, "dpdtimeout", translate("DPD Timeout"))
ipsecConfig:option(Value, "ikelifetime", translate("IKE Lifetime"))
ipsecConfig:option(Value, "lifetime", translate("Lifetime"))

ipsecConfig:option(Value, "local-ip", translate("Local IP"))
ipsecConfig:option(Value, "local-id", translate("Local ID"))
ipsecConfig:option(Value, "local-id2", translate("Local ID2"))
ipsecConfig:option(Value, "local-src", translate("Local Source IP"))
ipsecConfig:option(Value, "local-sub", translate("Local Subnet"))

ipsecAuth = ipsecConfig:option(ListValue, "local-auth", translate("Local Auth"))
ipsecAuth:value('pubkey', 'pubkey')
ipsecAuth:value('psk', 'psk')
ipsecAuth:value('eap', 'eap')
ipsecAuth:value('eap-aka', 'eap-aka')
ipsecAuth:value('eap-gtc', 'eap-gtc')
ipsecAuth:value('eap-md5', 'eap-md5')
ipsecAuth:value('eap-mschapv2', 'eap-mschapv2')
ipsecAuth:value('eap-peap', 'eap-peap')
ipsecAuth:value('xauth', 'xauth')
ipsecAuth:value('xauth-eap', 'xauth-eap')
ipsecAuth:value('xauth-generic', 'xauth-generic')

ipsecAuth2 = ipsecConfig:option(ListValue, "local-auth2", translate("Local Auth2"))
ipsecAuth2:value('', '--')
ipsecAuth2:value('pubkey', 'pubkey')
ipsecAuth2:value('psk', 'psk')
ipsecAuth2:value('eap', 'eap')
ipsecAuth2:value('eap-aka', 'eap-aka')
ipsecAuth2:value('eap-gtc', 'eap-gtc')
ipsecAuth2:value('eap-md5', 'eap-md5')
ipsecAuth2:value('eap-mschapv2', 'eap-mschapv2')
ipsecAuth2:value('eap-peap', 'eap-peap')
ipsecAuth2:value('xauth', 'xauth')
ipsecAuth2:value('xauth-eap', 'xauth-eap')
ipsecAuth2:value('xauth-generic', 'xauth-generic')

local ret, cert_list = mgmtdclient.query_child("/config/system/mgmt/cert-list")
localCert = ipsecConfig:option(ListValue, "local-cert", translate("Local Cert"))
localCert2 = ipsecConfig:option(ListValue, "local-cert2", translate("Local Cert2"))
localCert:value('', '--')
localCert2:value('', '--')
if ret == 'OK' then
	if cert_list ~= nil then
		if type(cert_list) == 'table' then
			for v in pairs(cert_list) do
				localCert:value(v, v)
				localCert2:value(v, v)
			end
		end
	end
end

ipsecConfig:option(Value, "peer-ip", translate("Peer IP"))
ipsecConfig:option(Value, "peer-id", translate("Peer ID"))
ipsecConfig:option(Value, "peer-id2", translate("Peer ID2"))
ipsecConfig:option(Value, "peer-src", translate("Peer Source IP"))
ipsecConfig:option(Value, "peer-sub", translate("Peer Subnet"))

ipsecPeerAuth = ipsecConfig:option(ListValue, "peer-auth", translate("Peer Auth"))
ipsecPeerAuth:value('pubkey', 'pubkey')
ipsecPeerAuth:value('psk', 'psk')
ipsecPeerAuth:value('eap', 'eap')
ipsecPeerAuth:value('eap-aka', 'eap-aka')
ipsecPeerAuth:value('eap-gtc', 'eap-gtc')
ipsecPeerAuth:value('eap-md5', 'eap-md5')
ipsecPeerAuth:value('eap-mschapv2', 'eap-mschapv2')
ipsecPeerAuth:value('eap-peap', 'eap-peap')
ipsecPeerAuth:value('xauth', 'xauth')
ipsecPeerAuth:value('xauth-eap', 'xauth-eap')
ipsecPeerAuth:value('xauth-generic', 'xauth-generic')

ipsecPeerAuth2 = ipsecConfig:option(ListValue, "peer-auth2", translate("Peer Auth2"))
ipsecPeerAuth2:value('', '--')
ipsecPeerAuth2:value('pubkey', 'pubkey')
ipsecPeerAuth2:value('psk', 'psk')
ipsecPeerAuth2:value('eap', 'eap')
ipsecPeerAuth2:value('eap-aka', 'eap-aka')
ipsecPeerAuth2:value('eap-gtc', 'eap-gtc')
ipsecPeerAuth2:value('eap-md5', 'eap-md5')
ipsecPeerAuth2:value('eap-mschapv2', 'eap-mschapv2')
ipsecPeerAuth2:value('eap-peap', 'eap-peap')
ipsecPeerAuth2:value('xauth', 'xauth')
ipsecPeerAuth2:value('xauth-eap', 'xauth-eap')
ipsecPeerAuth2:value('xauth-generic', 'xauth-generic')

peerCert = ipsecConfig:option(ListValue, "peer-cert", translate("Peer Cert"))
peerCert2 = ipsecConfig:option(ListValue, "peer-cert2", translate("Peer Cert2"))
peerCert:value('', '--')
peerCert2:value('', '--')
if ret == 'OK' then
	if cert_list ~= nil then
		if type(cert_list) == 'table' then
			for v in pairs(cert_list) do
				peerCert:value(v, v)
				peerCert2:value(v, v)
			end
		end
	end
end

--[[
List the ca name for ipsec ca
]]--
ipsecCa = mdForm:get_section_obj("ipsec-ca")
ipsecCa.sectionhead = translate("name")
cacert = ipsecCa:option(ListValue, "cacert", translate("CA Cert"))
cacert:value('', '--')
if ret == 'OK' then
	if cert_list ~= nil then
		if type(cert_list) == 'table' then
			for v in pairs(cert_list) do
				cacert:value(v, v)
			end
		end
	end
end
--cacert.readonly = true
ipsecCa:option(Value, "ocspuri", translate("OCSP URI"))
ipsecCa:option(Value, "crluri", translate("CRL URI"))
ipsecCa:option(Value, "certuribase", translate("Base URI"))


--[[
List the secret name for ipsec secret
]]--
ipsecSecret = mdForm:get_section_obj("ipsec-secret")
ipsecSecret.sectionhead = translate("name")

secretType = ipsecSecret:option(ListValue, "type", translate("Type"))
secretType:value('', '--')
secretType:value('RSA', 'RSA')
secretType:value('ECDSA', 'ECDSA')
secretType:value('BLISS', 'BLISS')
secretType:value('P12', 'P12')
secretType:value('PSK', 'PSK')
secretType:value('EAP', 'EAP')
secretType:value('NTLM', 'NTLM')
secretType:value('XAUTH', 'XAUTH')
secretType:value('PIN', 'PIN')
secretType.datatype = 'minlength(1)'

ipsecSecret:option(Value, "host", translate("Host/User"))
ipsecSecret:option(Value, "peer", translate("Peer"))
ipsecSecret:option(Value, "key-string", translate("Key/Password"))
certKey = ipsecSecret:option(ListValue, "key-file", translate("Key File"))
certKey:value('', '--')
local ret, key_list = mgmtdclient.query_child("/config/system/mgmt/key-list")
if ret == 'OK' then
	if key_list ~= nil then
		if type(key_list) == 'table' then
			for v in pairs(key_list) do
				certKey:value(v, v)
			end
		end
	end
end

pass = ipsecSecret:option(Value, "passphrase", translate("Passphrase"))
--pass.password = true
pass:depends({["key-file"]=""})

return mdForm
