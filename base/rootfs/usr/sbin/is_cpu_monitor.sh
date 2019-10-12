#!/bin/sh

cpunum=$(cat /proc/cpuinfo|grep processor|wc -l)

is_cpu_usage.sh &

for i in `seq 0 $((cpunum-1))`;
do
	is_cpu_usage.sh $i &
done


