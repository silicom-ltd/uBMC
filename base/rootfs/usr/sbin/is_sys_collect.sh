#!/bin/sh
DUMPDIR=$1
mkdir -p $DUMPDIR/var/log
mkdir -p $DUMPDIR/tmp

cp -ar /var/log/message* $DUMPDIR/var/log/
cp -ar /var/log/kernel.log* $DUMPDIR/var/log/

cp -ar /tmp/init_log $DUMPDIR/tmp/
if [ -e "/tmp/bcm.log" ]; then
	cp -ar /tmp/bcm.log* $DUMPDIR/tmp/
fi
if [ -e "/tmp/nginx_error.log" ]; then
	cp -ar /tmp/nginx_error.log $DUMPDIR/tmp/
fi

cp -ar /etc $DUMPDIR/
#rm -rf $DUMPDIR/etc/product

cp -ar /config $DUMPDIR/

