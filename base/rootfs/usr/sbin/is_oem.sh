#!/bin/sh

#set -x

. /usr/sbin/silc_util.sh

cfg_dir=/etc/prod_defconfig/vendor_config

product=$(get_product_name)

#change version prefix to vendor device name
devname=$(/usr/sbin/is_upgrade.sh -N)
if [ "" != "${devname}" ]; then
	for verfile in $(ls /etc/prod_version/); do
		verfile=/etc/prod_version/${verfile}
		oldver=$(cat ${verfile}|sed "s/##//g"|head -n 1)
		#some old versions have SILICOM prefix, remove it
		newver=${oldver/SILICOM_/}
		#replace prefix with vendor device name
		newver=${newver/${product}/${devname}}
		echo "##${newver}##" > ${verfile}
	done
fi

#remove other vendor config
vendor=$(get_vendor_name)
if [ "" != "${vendor}" ] && [ -e ${cfg_dir}/${vendor} ]; then
	cd ${cfg_dir}
	ls ./|grep -v ${vendor}|xargs rm -rf
else
	#default process for SILICOM
	#remove other vendors mibs
	rm -rf ${cfg_dir}/*
	exit 0
fi

#copy EULA
if [ -e ${cfg_dir}/${vendor}/${vendor}.EULA ]; then
	cp ${cfg_dir}/${vendor}/${vendor}.EULA /etc/EULA.txt
fi

#copy MIBs
if [ -e ${cfg_dir}/${vendor}/mibs ]; then
	rm -rf /etc/mibs/*
	cp -ar ${cfg_dir}/${vendor}/mibs/* /etc/mibs/
fi

#copy SSL cert/key
if [ -e ${cfg_dir}/${vendor}/ssl ]; then
	cp ${cfg_dir}/${vendor}/ssl/server.crt /etc/nginx/nginx_ssl.crt
	cp ${cfg_dir}/${vendor}/ssl/server.key /etc/nginx/nginx_ssl.key
fi

#copy luci config
if [ -e /opt/silc-web/host/etc/config/${vendor}.luci ]; then
	mv /opt/silc-web/host/etc/config/${vendor}.luci /opt/silc-web/host/etc/config/luci
fi

