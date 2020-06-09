#!/bin/bash

########
#Internal script for create sd release tar file
#
set -e

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
  BR2_ROOT=$(dirname $BR2_ROOT);
done
if [ $BR2_ROOT == "/" ]; then
  echo "This script can't find buildroot home"
  BR2_ROOT=$(pwd)
  echo "Using current path as BR2_ROOT $(BR2_ROOT)"
fi
BASE_ROOT=${BR2_ROOT}/..
. ${BASE_ROOT}/base/rootfs/usr/sbin/silc_util.sh
SILCBASE=${BASE_ROOT}/scripts
BR2_OUT=${BR2_ROOT}/output
#set -x

#input checking, 
test -z "$1" && error_quit "Please specify the install boot image dir"
test -z "$2" && error_quit "Please specify the install boot image uboot dir, can be the same as output/images"
test -z "$3" && error_quit "Please specify the system image to install"

PRODUCT=$(cat ${BR2_OUT}/target/etc/product.txt)
PRODUCT_SUB=$(cat ${BR2_OUT}/target/etc/product_sub.txt)
echo "Building SD tar for ${PRODUCT}:${PRODUCT_SUB}"
IMGDIR=$1
IMGDIR_UBOOT=$2
INSTALL_IMG=$3
RELEASE=$(basename ${INSTALL_IMG} .img)-MAKESD
if [ "${PRODUCT_SUB}" == "UBMC_M" ]; then
	SDCARD_ENV=${BR2_ROOT}/product/scripts/upgrade/mvebu_sd_env
else
	SDCARD_ENV=${BR2_ROOT}/product/scripts/upgrade/sdcard_env
fi
. ${SDCARD_ENV}

test -z "${INSTALL_CP_SRC_NAMES}" && error_quit "invalid sdcard_env, INSTALL_CP_SRC_NAMES is empty"

__TARTMP=$(mktemp -d) || error_quit "Failed to create temp dir"
TARTMP=${__TARTMP}/${RELEASE}
mkdir -p ${TARTMP} || error_quit "Failed to create temp target dir"


#checking device, and make sure it is USB
test -f "${INSTALL_IMG}" || error_quit "${INSTALL_IMG} is not a file"

for SRC_NAME in ${INSTALL_SRC_NAMES}; do
  SRC=$(eval echo \$${SRC_NAME})
  test -z "${SRC}" && error_quit "${SRC_NAME} is not defined or empty in ${SDCARD_ENV}"
  test -f "${SRC}" || error_quit "${SRC_NAME} ${SRC} is not a regular file"
done

cp -v ${SDCARD_ENV} ${TARTMP}/
cp -v ${SILCBASE}/is_/make_sd_install.sh ${TARTMP}/

for CP_SRC_NAME in ${INSTALL_CP_SRC_NAMES}; do
  CP_SRC="$(eval echo \$${CP_SRC_NAME})"
  test -z "${CP_SRC}" && error_quit "${CP_SRC_NAME} is not defined or empty in ${SDCARD_ENV}"
  test -f "${CP_SRC}" || error_quit "${CP_SRC_NAME} ${CP_SRC} is not a regular file"
  echo "----> Copying ${CP_SRC} to ${SD_PART}"
  cp -v ${CP_SRC} ${TARTMP}/ || error_quit "Failed to copy ${CP_SRC_NAME} ${CP_SRC} into ${SD_PART} mounted on ${TARTMP}"
done

echo "----> Copying ${INSTALL_IMG} to ${SD_PART}"
cp -v ${INSTALL_IMG} ${TARTMP}/

echo "---->Creating tar"
( cd ${__TARTMP}; tar -zcf ${RELEASE}.tar.gz ${RELEASE} || error_quit "Failed to create tar ${RELEASE}.tar.gz"; )
cp -v ${__TARTMP}/*.gz ./
rm -rf ${__TARTMP}

echo "--> Done. Created ${RELEASE}.tar.gz"



