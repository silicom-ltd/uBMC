#!/bin/sh

ITEMNUM="3"

TARGETDIR="/var/log"
FORMAT="png"
OUTPUT="/tmp/fs_log_usage."${FORMAT}

DATA1=$(du -shk ${TARGETDIR}/dump|awk {'print $1'})
DATA2=$(du -shk ${TARGETDIR}|awk {'print $1'})
DATA3=$(df -k ${TARGETDIR}|grep -v File|awk {'print $4'})

DATA2=$((${DATA2}-${DATA1}))

TOTAL="0"
for i in `seq 1 ${ITEMNUM}`; do
	TOTAL=$((${TOTAL}+$((DATA${i}))));
done

COLOR1="red"
COLOR2="yellow"
COLOR3="green"

LABEL1="dump "${DATA1}"KB"
LABEL2="log  "${DATA2}"KB"
LABEL3="free "${DATA3}"KB"

. /etc/gnuplot_gen_pie_chart.sh

