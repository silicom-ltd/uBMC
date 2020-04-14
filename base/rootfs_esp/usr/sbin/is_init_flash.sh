#!/bin/sh
#set -x
. /usr/sbin/silc_util.sh
PRODUCT=$(cat /etc/product.txt)
rngd -o /dev/random -r /dev/urandom
/usr/sbin/${PRODUCT}/init_flash.sh


