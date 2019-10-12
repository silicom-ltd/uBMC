#!/bin/bash

#set -x
BR2_ROOT=$(pwd)
BR2_OUTPUT=${BR2_OUT}
#search for buildroot in the path, and use it as BR2_ROOT if found
#if not found BR2_ROOT will be empty
while [ $BR2_ROOT != / ]; do
	if [ "buildroot" == $(basename $BR2_ROOT) ]; then
		break
	fi
	if [ -d "${BR2_ROOT}/buildroot" ]; then
		BR2_ROOT=${BR2_ROOT}/buildroot
		break
	fi
	BR2_ROOT=$(dirname $BR2_ROOT);
done

if [ $BR2_ROOT == "/" ]; then
	BR2_ROOT=""
fi

SDCARD=$1
UBOOTBIN=${BR2_OUT}/images/u-boot.pbl
UIMAGE=${BR2_OUT}/images/uImage
DTB=${BR2_OUT}/images/p2041rdb.dtb
ROOTFS=${BR2_OUT}/images/rootfs.cpio.uboot
SDTMP=${BR2_OUT}/sdcard

if [ -z "$1" ]; then
  echo "Please specify the sd card device"
  exit 1
fi

if [ -z "$(echo ${SDCARD}|grep "^/")" ]; then
  echo "Device ${SDCARD} must a absolute path"
  exit 1
fi
if [ ! -b ${SDCARD} ]; then
  echo "Device ${SDCARD} is not a block device"
  exit 1
fi
sys_dev_path=$(readlink -e /sys/block/$(basename ${SDCARD}))

if [ -z "$(echo ${sys_dev_path} |grep usb)" ]; then
  echo "Device ${SDCARD} doesn't seems to be an usb device"
  exit 1
fi

if [ ! -z "$(mount |grep ${SDCARD})" ]; then
  sudo umount ${SDCARD}*
  if [ ! -z "$(mount |grep ${SDCARD})" ]; then
    echo "Can't umount ${SDCARD}* devices";
    exit 1
  fi
fi

if [ ! -d ${SDTMP} ]; then
  if [ ! -e ${SDTMP} ]; then
    mkdir -p ${SDTMP}
  else
    echo "${SDTMP} exists but is not a directory"
    exit 1
  fi
fi

function chk_file ()
{
  if [ ! -f $1 ]; then
    echo "$1 is not found"
    exit 1
  fi
}

function build_sd_uboot ()
{
  rm -rf ${BR2_OUT}/build/uboot-custom
  cp ${BR2_ROOT}/product/config/config_sd_uboot ${BR2_ROOT}/.config
  make uboot
}

if [ -z "$( strings output/images/u-boot.bin |grep bootcmd|grep fatload)" ]; then
  echo "--> uboot is not sd card u-boot, rebuilding with config_sd_uboot"
  build_sd_uboot
fi

chk_file ${UBOOTBIN}
chk_file ${UIMAGE}
chk_file ${DTB}
chk_file ${ROOTFS}

echo "--> Zero out first 1MB"
sudo dd if=/dev/zero of=${SDCARD} bs=1M count=1
echo "--> Writing uboot"
sudo dd if=${UBOOTBIN} of=${SDCARD} bs=512 seek=8
echo "--> repartitioning ${SDCARD}"
sudo blockdev --rereadpt ${SDCARD}
#sfdisk ${SDCARD} << EOF
#1,16,c
#17,,L
#EOF
sudo sfdisk ${SDCARD} << EOF
1,,L
EOF

echo "--> Creating vfat on ${SDCARD}1"
sudo mkfs.vfat ${SDCARD}1

echo "--> Copying images to ${SDCARD}1"
sudo mount ${SDCARD}1 ${SDTMP}
sudo cp ${UIMAGE} ${SDTMP}
sudo cp ${DTB} ${SDTMP}
sudo cp ${ROOTFS} ${SDTMP}
sudo sync
sleep 1
sudo umount ${SDCARD}1

echo "--> Done"

exit 0
eject ${SDCARD}
#mount ${SDCARD}2 ${SDTMP}
#cp ${ROOTFS} ${SDTMP}
#umount ${SDTMP}

