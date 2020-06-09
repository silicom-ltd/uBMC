#!/bin/bash
#set -x
BR2_ROOT=$(pwd)
IMAGE_PATH=$(pwd)

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
	if [ -d "${BR2_ROOT}/buildroot" ]; then
		BR2_ROOT=${BR2_ROOT}/buildroot
		break
	fi
  BR2_ROOT=$(dirname $BR2_ROOT);
done
BASE_ROOT=${BR2_ROOT}/..
if [ $BR2_ROOT == "/" ]; then
  echo "This script can't find buildroot home"
  BR2_ROOT=$(pwd)
  echo "Using current path as BR2_ROOT $(BR2_ROOT)"
fi

. ${BASE_ROOT}/base/rootfs/usr/sbin/silc_util.sh

BR2_OUT=${BR2_ROOT}/output
#set -x
PRODUCT=$(cat ${BR2_OUT}/target/etc/product.txt)
PRODUCT_SUB=$(cat ${BR2_OUT}/target/etc/product_sub.txt)
echo "Building SD card for ${PRODUCT}"
SDCARD=$1
IMGDIR=$2
INSTALL_IMG=$3

SDTMP=$(mktemp -d) || error_quit "Failed to create temp mount dir"

#input checking, 
test -z "$1" && error_quit "Please specify the sd card device"
test -z "$2" && error_quit "Please specify the image dir"
test -z "$3" && error_quit "Please specify the image to install"
if [ ${PRODUCT_SUB} == "UBMC_M" ]; then
${BASE_ROOT}/scripts/is_make_sd_install.sh $1 $2 $3 ${BR2_ROOT}/product/scripts/upgrade/mvebu_sd_env
else
${BASE_ROOT}/scripts/is_make_sd_install.sh $1 $2 $3 ${BR2_ROOT}/product/scripts/upgrade/sdcard_env
fi



