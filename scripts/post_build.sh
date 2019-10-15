#!/bin/bash

set -e
PRODUCT=$(cat ${TARGET_DIR}/etc/product.txt) || ( echo "/etc/product.txt not found in target dir, try make silc_product-rebuild"; exit 1;)

#SVNREV=$( svn info |grep Revision|awk '{print $2}')
SVNREV=$(git rev-parse --short HEAD)
sed -i "s/SVN/$SVNREV/" ${TARGET_DIR}/etc/product/${PRODUCT}/version/software_version.txt
sed -i "s/SVN/$SVNREV/" ${TARGET_DIR}/etc/product/${PRODUCT}/version/firmware_version.txt
sed -i "s/SVN/$SVNREV/" ${TARGET_DIR}/etc/product/${PRODUCT}/version/uboot_version.txt
sed -i "s/SVN/$SVNREV/" ${TARGET_DIR}/etc/product/${PRODUCT}/version/dtb_version.txt
sed -i 's/\/root:\/bin\/sh/\/root:\/sbin\/nologin/' ${TARGET_DIR}/etc/passwd

echo "Run product post build scripts $(pwd)/product/scripts/post_build_prod.sh"
. product/scripts/post_build_prod.sh

echo $(ls ${TARGET_DIR}/etc/product/${PRODUCT}/)

for prod_subdir in $(ls ${TARGET_DIR}/etc/product/${PRODUCT}/); do
	subdname=$(basename ${prod_subdir})
	ln_src=/etc/product/${PRODUCT}/${subdname}
	ln_dst=/etc/prod_${subdname}
	echo "Creating link ${ln_dst} -> ${ln_src}"
	rm -rf ${TARGET_DIR}/${ln_dst}
	ln -s ${ln_src} ${TARGET_DIR}/${ln_dst}
done
for conf_dir_name in $(ls ${TARGET_DIR}/etc/product/); do
	conf_dname=$(basename ${conf_dir_name})
	if [ "${PRODUCT}" != "${conf_dname}" ]; then
		echo "Removing not needed ${TARGET_DIR}/etc/product/${conf_dname}"
		rm -rf ${TARGET_DIR}/etc/product/${conf_dname}
	fi
done
