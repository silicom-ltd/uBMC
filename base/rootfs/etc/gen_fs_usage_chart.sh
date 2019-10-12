#!/bin/sh

ITEMNUM="2"

if [ -z $1 ]; then
	echo "Please specify a mounted dir"
	exit 1
fi

TARGETDIR=$1
FORMAT="png"
OUTPUT="/tmp/fs_usage."${FORMAT}
if [ ! -z $2 ]; then
	OUTPUT=$2
fi

DATA1=$(df -k ${TARGETDIR}|grep -v File|awk {'print $3'})
DATA2=$(df -k ${TARGETDIR}|grep -v File|awk {'print $4'})

TOTAL="0"
for i in `seq 1 ${ITEMNUM}`; do TOTAL=$((${TOTAL}+$((DATA${i})))); done

COLOR1="red"
COLOR2="green"

LABEL1="used "${DATA1}"KB"
LABEL2="free "${DATA2}"KB"

. /etc/gnuplot_gen_pie_chart.sh

