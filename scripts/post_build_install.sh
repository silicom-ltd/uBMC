#!/bin/bash
set +e

echo "=============Target post install ==============="
PRODUCT=$(cat ${TARGET_DIR}/etc/product.txt)
PRODUCT_SUB=$(cat ${TARGET_DIR}/etc/product_sub.txt 2>/dev/null)
#rm -rf ${TARGET_DIR}/etc/product
#We copy files needed for install image into target
#We don't want to store them in two different rootfs skeleton
#So we store them only in rootfs
mkdir -p ${TARGET_DIR}/etc/product
cp -a product/rootfs/etc/product/${PRODUCT} ${TARGET_DIR}/etc/product/
for prod_subdir in $(ls ${TARGET_DIR}/etc/product/${PRODUCT}/); do
	subdname=$(basename ${prod_subdir})
	ln_src=/etc/product/${PRODUCT}/${subdname}
	ln_dst=/etc/prod_${subdname}
	echo "Creating link ${ln_dst} -> ${ln_src}"
	rm -rf ${TARGET_DIR}/${ln_dst}
	ln -s ${ln_src} ${TARGET_DIR}/${ln_dst}
done
cp -a ${PWD}/../base/rootfs/usr/sbin/is_copy_file_and_sync ${TARGET_DIR}/usr/sbin/
cp -a ${PWD}/../base/rootfs/usr/sbin/is_upgrade.sh ${TARGET_DIR}/usr/sbin/
cp -a ${PWD}/../base/rootfs/usr/sbin/silc_util.sh ${TARGET_DIR}/usr/sbin/
cp -a ${PWD}/../base/rootfs/etc/silicom-is-public.key ${TARGET_DIR}/etc/
if [ "${PRODUCT_SUB}" == "UBMC_M" ];then
	sed -i "s/SILC_PRODUCT_TTY/ttyMV0/g" ${TARGET_DIR}/etc/inittab
else
	sed -i "s/SILC_PRODUCT_TTY/ttyS0/g" ${TARGET_DIR}/etc/inittab
fi

