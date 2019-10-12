#!/bin/sh
#set -x

. /usr/sbin/silc_util.sh

filename=$1

devname=$(cat /etc/product/devname)
if [ "" == "${devname}" ]; then
	devname=$(get_product_name)
fi
cat $filename|grep "#Device type: ${devname}" > /dev/null
if [ "$?" != "0" ]; then
	echo "failed"
	exit 1
fi

fingerorg=$(cat $filename|grep "#Fingerprint "|awk '{print $2}')
fingercur=$(cat $filename|grep -v "#Fingerprint "|md5sum|awk '{print $1}'|openssl enc -base64)

if [ "$fingerorg" == "$fingercur" ]; then
	echo "OK"
	exit 0
else
	echo "failed"
	exit 1
fi

