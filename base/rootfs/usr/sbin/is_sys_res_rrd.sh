#!/bin/sh

#set -x

. /usr/sbin/silc_util.sh

rrdtool="/usr/bin/rrdtool"

rrd_db_cpu_all="/tmp/sys-rrd/cpu_all"
rrd_db_cpu_percore="/tmp/sys-rrd/cpu_per_core"
rrd_db_mem="/tmp/sys-rrd/mem_usage"

rra_args="RRA:AVERAGE:0:1s:1200s RRA:AVERAGE:0:10s:12000s RRA:AVERAGE:0:1m:1200m RRA:AVERAGE:0:5m:100h RRA:AVERAGE:0:1h:1200h"

cpu_num=$(cat /proc/cpuinfo|grep processor|wc -l)

create_rrd()
{
	mkdir -p /tmp/sys-rrd
	rm -rf /tmp/sys-rrd/*

	${rrdtool} create ${rrd_db_cpu_all} --start now --step 1 DS:cpu:GAUGE:1:0:100 ${rra_args}

	if [ "${cpu_num}" > "1" ]; then
		ds_args=""
		for i in `seq 0 $((cpu_num-1))`; do
			ds_args=${ds_args}"DS:cpu${i}:GAUGE:1:0:100 "
		done
		${rrdtool} create ${rrd_db_cpu_percore} --start now --step 1 ${ds_args} ${rra_args}
	fi

	${rrdtool} create ${rrd_db_mem} --start now --step 1 DS:total:GAUGE:1:0:68719476736 DS:used:GAUGE:1:0:68719476736 ${rra_args}

	sleep 1
}

cpu_idle_list=""
get_cpu()
{
	idx=$1
	cpu_idle=$(echo ${cpu_idle_list}|awk -v i=${idx} '{printf($i)}')
	echo $(lua -e "print(100-${cpu_idle})")
}

last_exec_ts=$(lua -e "print(os.time())")
create_rrd

while :;
do
	# "sar -P ALL 1 1" will run for 1s
	#sleep 1

	ts=$(lua -e "print(os.time())")
	if [ "${ts}" -le "${last_exec_ts}" ] || [ "${ts}" -ge "$((last_exec_ts+3600))" ]; then
		echo "sys-rrd curr time ${ts} and save time ${last_exec_ts} diff too much, reset sys-rrd"
		last_exec_ts=${ts}
		create_rrd
		continue
	fi
	last_exec_ts=${ts}

	cpu_idle_list=$(sar -P ALL 1 1|tail -n $((cpu_num+1))|awk '{printf($8" ")}')
	rrdtool update ${rrd_db_cpu_all} ${ts}:$(get_cpu 1)

	if [ "${cpu_num}" > "1" ]; then
		ds_args=""
		for i in `seq 2 $((cpu_num+1))`; do
			ds_args=${ds_args}":$(get_cpu ${i})"
		done
		rrdtool update ${rrd_db_cpu_percore} ${ts}${ds_args}
	fi

	mem_total=$(lua -e "print($(free|grep Mem|awk '{print $2}')*1024)")
	mem_used=$(lua -e "print($(free|grep Mem|awk '{print $3}')*1024)")
	rrdtool update ${rrd_db_mem} ${ts}:${mem_total}:${mem_used}

done

rm -rf /tmp/sys-rrd/*
sync

