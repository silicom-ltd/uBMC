#!/bin/sh

skip_part=0
if [ "$1" == "-u" ]; then
  skip_part=1
fi
. /usr/sbin/silc_util.sh

export PRODUCT=$(cat /etc/product.txt)
export PRODUCT_SUB=$(cat /etc/product_sub.txt 2>/dev/null)
echo "0 0 0 0" >/proc/sys/kernel/printk

echo "=============================================="
echo "Installing ${PRODUCT} image starting..."

IMG_FILE=$(/usr/sbin/${PRODUCT}/prod_find_image.sh)
test -z "${IMG_FILE}" && IMG_FILE=/install.img
test -f "${IMG_FILE}" || error_quit "Can't find image file to install"

echo "Found image $(basename ${IMG_FILE}) to install";


echo -n "Checking system time "
date "+%F %T"
IMG_DATE_SEC=$(date -d "$(stat ${IMG_FILE} |grep Modify|awk '{print $2 " " $3 " " $4}')" +%s)
SYS_DATE_SEC=$(date +%s)
if [ "${SYS_DATE_SEC}" -lt "${IMG_DATE_SEC}" ]; then
  echo "System date not initialized, setting to image time for installation"
  date -s "$(date -d "@$((${IMG_DATE_SEC} + 60))" "+%F %T")"
fi

if [ ${skip_part} == "1" ]; then
  echo "Not Initializing Flash"
else
  echo "Initializing Flash"
  /usr/sbin/is_init_flash.sh
fi


echo "Installing image on bank 0"
is_upgrade.sh -t fw-boot -v 1 || error_quit "Failed to update boot flag"
is_upgrade.sh -t fw-upgrade -v 0 || error_quit "Failed to update boot flag"
is_upgrade.sh -S || error_quit "Failed to read system info"

is_upgrade.sh -f -V ${IMG_FILE} || error_quit "Failed to extract image ${IMG_FILE}"
is_upgrade.sh -u || error_quit "Failed to install image ${IMG_FILE} to bank 0"
echo "Bank 0 installation done"
is_upgrade.sh -S

echo "Installing image on bank 1"
is_upgrade.sh -t fw-boot -v 0 || error_quit "Failed to update boot flag"
is_upgrade.sh -t fw-upgrade -v 0 || error_quit "Failed to update boot flag"
is_upgrade.sh -S || error_quit "Failed to read system info"
is_upgrade.sh -f -V ${IMG_FILE} || error_quit "Failed to extract image ${IMG_FILE}"
is_upgrade.sh -u "Failed to install image ${IMG_FILE} to bank 1"
echo "Bank 1 installation done"

is_upgrade.sh -t fw-boot -v 0 || error_quit "Failed to update boot flag"
is_upgrade.sh -t fw-upgrade -v 0 || error_quit "Failed to update boot flag"
is_upgrade.sh -S || error_quit "Failed to read system info"
is_upgrade.sh -s || error_quit "Failed to read system info"
export IMG_FILE
/usr/sbin/${PRODUCT}/post_install.sh

echo "Installation of ${PRODUCT} image finished, you can now reboot"
