#!/bin/sh

. /usr/sbin/silc_util.sh

if [ "${PRODUCT_SUB}" == "UBMC_EVAL" ]; then
	return
fi
# When we install IMG for armada-37xx ,we should use usb storage
if [ "${PRODUCT_SUB}" == "UBMC_ESP" ]; then
	if [ -b /dev/sda1 ] && [ -b /dev/mmcblk0 ]; then
		DUAL_DEV=1
		SRC_DEV=/dev/sda1
	else
		echo "Can not find usb storage ,please check it!" >&2
	fi
else
	if [ -b /dev/mmcblk1 ] && [ -b /dev/mmcblk0 ]; then
		echo "Dual mmc found, install from mmc1 to mmc0" >&2
		DUAL_DEV=1
		SRC_DEV=/dev/mmcblk1p1
	else
		DUAL_DEV=0
		if [ -b /dev/mmcblk1 ]; then
			echo "Single mmc found, install from mmc1 to mmc1" >&2
			SRC_DEV=/dev/mmcblk1p1
		else
			echo "Single mmc found, install from mmc0 to mmc0" >&2
			SRC_DEV=/dev/mmcblk0p1
		fi
	fi
fi

mkdir -p /install_image
IMG_EXIST=$(find /install_image -name "*.img" |grep -v u-boot|head -n1)
if [ ! -z ${IMG_EXIST} ]; then
	echo ${IMG_EXIST}
	return
fi


if [ ${DUAL_DEV} == "1" ]; then
	test -z "$(mount|grep /install_image)" || umount /install_image
	mount ${SRC_DEV} /install_image || error_quit "Failed to mount ${SRC_DEV}"
else
	mkdir -p /tmp/install_image;
	test -z "$(mount|grep /tmp/install_image)" || umount /tmp/install_image
	mount ${SRC_DEV} /tmp/install_image || error_quit "Failed to mount ${SRC_DEV}"
	SRC_IMG="$(find /tmp/install_image -name "*.img" |grep -v u-boot|head -n1)"
	test -z "${SRC_IMG}" && error_quit "Can't find image to install"
	cp --preserve=timestamps ${SRC_IMG} /install_image 
	umount /tmp/install_image
fi
echo $(find /install_image -name "*.img" |grep -v u-boot|head -n1)

