#!/bin/sh
DUMPNAME=is_log_$(date "+%Y%m%d%H%M%S")
if [ "$1" != "" ]; then
	DUMPNAME=$1
fi
DUMPDIR=/tmp/$DUMPNAME
is_sys_collect.sh $DUMPDIR

cd /tmp/
tar -cf ./$DUMPNAME.tar ./$DUMPNAME
gzip ./$DUMPNAME.tar
mv ./$DUMPNAME.tar.gz /var/log/dump/
rm -rf ./$DUMPNAME

