#!/bin/sh

module=0
segment=0
port=0
name=image.png

if [ ! -z "$1" ]; then
        port=$1
fi

if [ ! -z "$2" ]; then
        segment=$2
fi

if [ ! -z "$3" ]; then
        module=$3
fi

curr_sec=$(date -u +%s)
                                                                       
rrdtool graph $name \
-l0  -w 1000 -h 400 \
-s $(($curr_sec - 1000)) -e $curr_sec \
-t "Segment "$(($module + 1))"."$(($segment + 1))" Net Ports Traffic" \
DEF:rxpkt=/tmp/rrd/$module"_"$segment"_"$port.rrd:RXPacket:AVERAGE LINE3:rxpkt#FF0000:"RX Packet" \
DEF:txpkt=/tmp/rrd/$module"_"$segment"_"$port.rrd:TXPacket:AVERAGE LINE3:txpkt#00FF00:"TX Packet" 
