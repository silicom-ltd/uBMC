#!/bin/sh

#set -x
BR2_ROOT=$(pwd)

echo "Clearning up ${BR2_ROOT}"
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

RELEASE_CLEAN=0
if [ "$1" == "-r" ]; then
	RELEASE_CLEAN=1
elif [ ! -z "$1" ]; then
	echo -e "Usage:\n\t$(basename $0)      # cleanup output build for recompiling Silicom software"
	echo -e "\t$(basename $0) -r   # cleanup output build for release, this also removes uboot/kernel, and check configuration"
	exit 1
fi

cd ${BR2_ROOT}

chk_config()
{
	PROD_NAME=$1
	PROD_CFG=$2
	if [ ! -z "$(cat ${BR2_ROOT}/.config |grep "BR2_PACKAGE_SILC_PRODUCT_${PROD_NAME}=y"|grep -v \#)" ]; then
		if [ ! -z "$(diff -u ${BR2_ROOT}/.config ${BR2_ROOT}/product/config/${PROD_CFG})" ]; then
			echo "Master configuration for ubmc is not the same as product/config/${PROD_CFG}"
			echo "***************************************************************************"
			diff -u ${BR2_ROOT}/.config ${BR2_ROOT}/product/config/${PROD_CFG}
			echo "***************************************************************************"
			echo -e "\tBe careful if you are buildign a release"
		else
			echo -e "\t.config is the same as master config product/config/${PROD_CFG}"
		fi

	fi
}

echo "Cleaning up output/build"
find output/build -name ".stamp_target_installed" |xargs rm -rf
rm -rf output/build/packages-file-list.txt
if [ -e output/build/uboot-custom ]; then
	( cd output/build/uboot-custom && make distclean ;)
	rm -rf output/build/uboot-custom/.stamp_configured
fi
rm -rf output/build/linux-tools
rm -rf output/target
rm -rf output/images
rm -rf output/build/is*
rm -rf output/build/silc*
rm -rf output/build/netconf*
rm -rf output/build/dying*
rm -rf output/build/switch*
rm -rf output/build/ubmc*
rm -rf output/staging/usr/include/silc_*
rm -rf output/staging/usr/include/is_*
rm -rf output/staging/usr/include/switch_*
rm -rf output/staging/usr/include/ubmc_*


if [ "${RELEASE_CLEAN}" == "1" ]; then
	echo "Removing kernel and uboot also for release"
	rm -rf output/build/uboot-custom
	rm -rf output/build/linux-custom
	echo "Check configuration change"
	chk_config UBMC ubmc.config
	chk_config IS is.config
	
fi




