#!/bin/sh

set -x

if [ -z "$1" ]; then
  echo "Please specify the install.img or -r option to revert"
  exit 1
fi

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
	BR2_ROOT=""
fi
BASE_ROOT=${BR2_ROOT}/../base

rebuild()
{
	make uboot-reconfigure
	make linux-reconfigure
	make
}

#${BASE_ROOT}/scripts/is_build_clean.sh
echo "Cleaning up output/build"
find output/build -name ".stamp_target_installed" |xargs rm -rf
rm -rf output/build/packages-file-list.txt
rm -rf ${BR2_ROOT}/output/build/busybox* 
rm -rf output/build/linux-tools
rm -rf output/target
rm -rf output/images

if [ "$1" == "-r" ]; then
  echo "Revert to normal make"
  cd ${BR2_ROOT}
  cp ./product/configs/is.config .config
  rebuild
  exit 0
fi

if [ ! -f $1 ]; then
  echo "$1 is not a regular file"
  exit 1
fi
echo "Using $1 as the the image copying to output/target/install.img"
#the first big file will be corrupted for unknown reason, so we need to copy it twice.
mkdir -p output/target
cp $1 output/target/aaa-unused.img
cp $1 output/target/install.img

cd ${BR2_ROOT}
cp ./product/configs/is_install.config .config
rebuild
