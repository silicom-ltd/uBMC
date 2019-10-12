#!/bin/sh

WEBDIR=/opt/silc-web

BIND_ADDR="127.0.0.1:58080"

if [ "$1" != "" ]; then
        BIND_ADDR=$1
fi

#rm -rf /tmp/luci-*
#rm -rf /tmp/silc-web-*

${WEBDIR}/build/hostenv.sh ${WEBDIR}/host /usr/lib/lua /usr/lib/lua "${WEBDIR}/host/usr/sbin/uhttpd -p ${BIND_ADDR} -h ${WEBDIR}/host/www -f -D"

