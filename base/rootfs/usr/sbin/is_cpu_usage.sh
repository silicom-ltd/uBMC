#!/bin/sh

INTERVAL=1
COUNT=3600

while :;
do
	if [ "$1" == "" ]; then
		sar -u $INTERVAL $COUNT > /tmp/cpu_all.log
	else
		sar -P $1 -u $INTERVAL $COUNT > /tmp/cpu_$1.log
	fi
	sleep 1
done

