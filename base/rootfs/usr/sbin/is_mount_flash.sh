#!/bin/sh
#set -x
#echo "Mount flash ubifs!"

. /usr/sbin/silc_util.sh
PRODUCT=$(get_product_name)

/etc/prod_init/mount_flash


