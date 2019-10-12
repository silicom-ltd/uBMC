#!/bin/bash
BR2_ROOT=$(pwd)
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

BASE_ROOT=$(dirname ${BR2_ROOT})
export BASE_ROOT
export BR2_ROOT

PATCH_CFG_PATH1=${BASE_ROOT}/patch_config
PATCH_CFG_PATH2=${BR2_ROOT}/product/patch_config

print_help()
{
	echo "Please specifiy the name of the package, available options are:"
	for mmm in $(ls ${PATCH_CFG_PATH1}/ ${PATCH_CFG_PATH2}/); do
		echo -e "\t $(basename $mmm)"
	done
	exit 1
}
if [ -z $1 ]; then
	print_help
fi

for CFG_PATH in ${PATCH_CFG_PATH1} ${PATCH_CFG_PATH2}; do
	if [ -d ${CFG_PATH}/$1 ]; then
		PATCH_BASE=${CFG_PATH}/$1
		break
	fi
done

if [ -z "${PATCH_BASE}" ]; then
	echo "Package $1 doesn't exist"
	print_help
fi

if [ ! -f ${PATCH_BASE}/config ]; then
	echo "${PATCH_BASE}/config file not found"
	print_help
	exit 1
fi
. ${PATCH_BASE}/config
. ${BASE_ROOT}/scripts/__patch_gen.sh

