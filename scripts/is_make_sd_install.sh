#!/bin/bash

error_quit()
{
    test -z "$1" || echo -e "\nError: $1"
    echo "Usage:"
    echo "    $? <SD card dev>"
    echo "    $? <SD card dev>  [IMGDIR]   [IMGFILE] [SDENV]"
    echo "example:"
    echo "./$? sdg"
    echo "./$? sdg output/images ./PRODUCT_XXX_XXX.img ./sdcard_env"
    exit 1
}

SDCARD_DEV=$1
SDCARD=/dev/${SDCARD_DEV}
IMGDIR=$2
INSTALL_IMG=$3
SDCARD_ENV=$4

#input checking, 
test -z "$1" && error_quit "Please specify the sd card device"


#checking device, and make sure it is USB
test -b ${SDCARD} || error_quit "Device ${SDCARD} is not a block device"
test -f /sys/class/block/${SDCARD_DEV}/partition && error_quit "Device ${SDCARD} must be the root device, not a partition"
test "$(cat /sys/class/block/${SDCARD_DEV}/size)" == "0" && error_quit "Device ${SDCARD} size is 0, is the card inserted properly?"

test -z "${IMGDIR}" && IMGDIR=./
test -d "${IMGDIR}" || echo_quit "Can't find sdcard_env"

test -z "${INSTALL_IMG}" && INSTALL_IMG=$(ls ./*.img|grep -v u-boot|head -n1)
test -f "${INSTALL_IMG}" || echo_quit "Can't find image file to install in current path"

test -z "${SDCARD_ENV}" && SDCARD_ENV=./sdcard_env
test -f "${SDCARD_ENV}" || echo_quit "Can't find sdcard_env"

IMGDIR_UBOOT=${IMGDIR}

echo "Building SD card for install"

. ${SDCARD_ENV}

SDTMP=$(mktemp -d) || error_quit "Failed to create temp mount dir"

sys_dev_path=$(readlink -e /sys/block/$(basename ${SDCARD}))
test -z "$(echo ${sys_dev_path} |grep usb)" && error_quit "Device ${SDCARD} doesnt seems to be an usb device"

#make sure our dev is not mouned
if [ ! -z "$(mount |grep ${SDCARD})" ]; then
  sudo umount ${SDCARD}*
  test -z "$(mount |grep ${SDCARD})" || error_quit "Failed to umount ${SDCARD}* devices";
fi

if [ ! -d ${SDTMP} ]; then
  if [ ! -e ${SDTMP} ]; then
    mkdir -p ${SDTMP} || error_quit "Failed to create directory ${SDTMP}, maybe exist but not a dir"
  fi
fi

test -z "${INSTALL_CP_SRC_NAMES}" && error_quit "invalid sdcard_env, INSTALL_CP_SRC_NAMES is empty"

for SRC_NAME in ${INSTALL_SRC_NAMES}; do
  SRC=$(eval echo \$${SRC_NAME})
  test -z "${SRC}" && error_quit "${SRC_NAME} is not defined or empty in ${SDCARD_ENV}"
  test -f "${SRC}" || error_quit "${SRC_NAME} ${SRC} is not a regular file"
done

is_sd_partition ${SDCARD}
SD_PART=${SDCARD}$(is_sd_get_img_part)
echo "--> Copying images to ${SD_PART}"

sudo mount ${SD_PART} ${SDTMP} -o dmask=0000,fmask=0000 || error_quit "Failed to mount ${SD_PART}"
for CP_SRC_NAME in ${INSTALL_CP_SRC_NAMES}; do
  CP_SRC="$(eval echo \$${CP_SRC_NAME})"
  test -z "${CP_SRC}" && error_quit "${CP_SRC_NAME} is not defined or empty in ${SDCARD_ENV}"
  test -f "${CP_SRC}" || error_quit "${CP_SRC_NAME} ${CP_SRC} is not a regular file"
  echo "----> Copying ${CP_SRC} to ${SD_PART}"
  cp ${CP_SRC} ${SDTMP}/ || error_quit "Failed to copy ${CP_SRC_NAME} ${CP_SRC} into ${SD_PART} mounted on ${SDTMP}"
done

  echo "----> Copying ${INSTALL_IMG} to ${SD_PART}"
cp ${INSTALL_IMG} ${SDTMP}/

sudo sync || error_quit "Failed to sync disk"
sleep 1
sudo umount ${SD_PART} || error_quit "Failed to umount ${SD_PART}"
rmdir ${SDTMP}

echo "--> Ejecting ${SDCARD}"
sudo eject ${SDCARD} || error_quit "Failed to eject ${SDCARD}"
echo "--> You can now remove the SD card, and put it into the product for installation"
echo "--> Done"



