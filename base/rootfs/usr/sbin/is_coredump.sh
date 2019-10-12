#!/bin/sh
dd of=/tmp/$1.core

DUMPNAME=is_core_$(date "+%Y%m%d%H%M%S")
DUMPDIR=/tmp/$DUMPNAME
is_sys_collect.sh $DUMPDIR

mv /tmp/$1.core $DUMPDIR/core

DUMPINFO=$1
POS=0
for i in ${DUMPINFO//-/ }; do
	if [ $POS == "1" ]; then
		echo "Program: "$i >> $DUMPDIR/core.info
	elif [ $POS == "2" ]; then
		echo "PID: "$i >> $DUMPDIR/core.info
	elif [ $POS == "3" ]; then
		echo "Time: "$(date -d @$i) >> $DUMPDIR/core.info
	elif [ $POS == "4" ]; then
		echo "Signal: "$i >> $DUMPDIR/core.info
	fi
	POS=$(($POS+1))
done

USAGE=$(df -k /var/log/|grep -v Filesystem|awk '{print $5}')
USAGE=${USAGE%"%"}
if [ $USAGE -gt 90 ]; then
	for i in $(ls -tr /var/log/dump/is_*); do
		rm -rf $i
		sync
		USAGE=$(df -k /var/log/|grep -v Filesystem|awk '{print $5}')
		USAGE=${USAGE%"%"}
		if [ $USAGE -le 90 ]; then
			break
		fi
	done
fi

cd /tmp/
tar -cf ./$DUMPNAME.tar ./$DUMPNAME
gzip ./$DUMPNAME.tar
mv ./$DUMPNAME.tar.gz /var/log/dump/
rm -rf ./$DUMPNAME


