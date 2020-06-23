#!/bin/bash

########
#Internal script for create ftp release tar file
#
INSTALL_IMG=$1

INSTALL_DIR=./buildroot/output/target/install_image
mkdir -p ${INSTALL_DIR}
rm -rf ${INSTALL_DIR}/*.img

cp ${INSTALL_IMG} ${INSTALL_DIR}
make

RELEASE=$(basename ${INSTALL_IMG} .img)-FTP

__TARTMP=$(mktemp -d) || error_quit "Failed to create temp dir"
TARTMP=${__TARTMP}/${RELEASE}
mkdir -p ${TARTMP} || error_quit "Failed to create temp target dir"

INSTALL_SRC_DIR=./buildroot/output/images
INSTALL_SRC_NAMES="flash-image.bin rootfs.cpio.uboot dt.dtb Image"

for f in ${INSTALL_SRC_NAMES}; do
  CP_SRC=${INSTALL_SRC_DIR}/$f
  test -f "${CP_SRC}" || error_quit "${CP_SRC} is not a regular file"
  echo "----> Copying ${CP_SRC} to ${TARTMP}"
  cp -v ${CP_SRC} ${TARTMP}/ || error_quit "Failed to copy ${CP_SRC} into ${TARTMP}"
done

echo "---->Creating tar"
( cd ${__TARTMP}; tar -zcf ${RELEASE}.tar.gz ${RELEASE} || error_quit "Failed to create tar ${RELEASE}.tar.gz"; )
cp -v ${__TARTMP}/*.gz ./
rm -rf ${__TARTMP}

echo "--> Done. Created ${RELEASE}.tar.gz"

