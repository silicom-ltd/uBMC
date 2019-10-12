#!/bin/sh

WEBDIR=/opt/silc-web

$WEBDIR/build/hostenv.sh $WEBDIR/host /usr/lib/lua /usr/lib/lua "/usr/bin/lua $WEBDIR/host/luci/shellcmd.lua $1"

