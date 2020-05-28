#! /bin/sh

#set -x
set -e

. /usr/sbin/silc_util.sh

TARGET_DEV=/dev/mmcblk0
if [ ! -b ${TARGET_DEV} ]; then
	TARGET_DEV=/dev/mmcblk1
fi
TARGET_NAME=${TARGET_DEV##*/}
test -z "$1" || TARGET_DEV=$1

test -z ${TARGET_DEV} && error_quit "must specify disk to device to operate on"
test -b ${TARGET_DEV} || error_quit "Specified device must be a block device"

#zero out 
dd if=/dev/zero of=$TARGET_DEV bs=1024 count=1024
PRODUCT_SUB=$(get_product_sub_name)
#UBMC_ESP boot from /dev/mmcblkxboo0,so let's init the partition
if [ "${PRODUCT_SUB}" == "UBMC_ESP" ]; then
	#echo 0 > /sys/block/$TARGET_NAME"boot0"/force_ro
	#dd if=/dev/zero of=$TARGET_DEV"boot0" bs=1024 count=2048 conv=fdatasync
	#echo 1 > /sys/block/$TARGET_NAME"boot0"/force_ro
	#flash_erase  /dev/mtd0 0 0 || error_quit "Failed to erase spi flash"
	echo "do flash erase"
fi
SIZE=`fdisk -l $TARGET_DEV | grep Disk | awk '{print $5}'`

echo DISK SIZE - $SIZE bytes

PART_UNIT=MiB
PART_START=1
FIRMWARE_PART_SIZE=128
SYSTEM_PART_SIZE=384
CONFIG_PART_SIZE=128
VAR_PART_SIZE=1024
KEY_PART_SIZE=256
IMG_PART_SIZE=256
DEFAULT_PART_FS=ext4
DEFAULT_PART_MKFS=ext4
DEFAULT_PART_LABEL=e2label
DEFAULT_PART_MKFS_OPT=F
DEFAULT_PART_LABEL_OPT=L
#mmc partition device has a special charater before part num
PARTDEV_CHAR=p

FIRMWARE_PART_START=${PART_START}
FIRMWARE_PART_END=$((${FIRMWARE_PART_START}+${FIRMWARE_PART_SIZE}))
FIRMWARE_PART_TYPE=primary
FIRMWARE_PART_NUM=1
#ubmc TI use fat as partition format
#ubmc Marvell use ext4 as partition format
if [ "${PRODUCT_SUB}" != "UBMC_ESP" ]; then
FIRMWARE_PART_FS=fat16
FIRMWARE_PART_MKFS=fat
FIRMWARE_PART_MKFS_OPT=none
FIRMWARE_PART_LABEL=fatlabel
FIRMWARE_PART_LABEL_OPT=n
fi

SYSTEM1_PART_START=${FIRMWARE_PART_END}
SYSTEM1_PART_END=$((${SYSTEM1_PART_START}+${SYSTEM_PART_SIZE}))
SYSTEM1_PART_TYPE=primary
SYSTEM1_PART_NUM=2

SYSTEM2_PART_START=${SYSTEM1_PART_END}
SYSTEM2_PART_END=$((${SYSTEM2_PART_START}+${SYSTEM_PART_SIZE}))
SYSTEM2_PART_TYPE=primary
SYSTEM2_PART_NUM=3

#extended partition, use all left space
EXT_PART_START=${SYSTEM2_PART_END}
EXT_PART_END=-1
EXT_PART_TYPE=extended
EXT_PART_NUM=4
EXT_PART_FS=none
EXT_PART_MKFS=none
EXT_PART_LABEL=none
EXT_PART_LABEL_OPT=none

#every partition below is a logical, logical partition are linked list partition table
# that uses 1 sector in front of every logical partition, thus we add 1MiB to each
# part start
CONFIG_PART_START=$((${EXT_PART_START}+1))
CONFIG_PART_END=$((${CONFIG_PART_START}+${CONFIG_PART_SIZE}))
CONFIG_PART_TYPE=logical
CONFIG_PART_NUM=5

#Key store partion, not used for now, but we might need it
KEY_PART_START=$((${CONFIG_PART_END}+1))
KEY_PART_END=$((${KEY_PART_START}+${KEY_PART_SIZE}))
KEY_PART_TYPE=logical
KEY_PART_NUM=6

VAR_PART_START=$((${KEY_PART_END}+1))
VAR_PART_END=$((${VAR_PART_START}+${VAR_PART_SIZE}))
VAR_PART_TYPE=logical
VAR_PART_NUM=7

#last partition, use all left space
#image partition is for store upgrade images for ubmc and host
#these images maybe exported to the host via usb
IMG_PART_START=$((${VAR_PART_END}+1))
IMG_PART_END=-1
IMG_PART_TYPE=logical
IMG_PART_NUM=8

parse_part_attr()
{
	#parse part attr
	#if attr is empty assign default
	#if attr is none, return ""
	#if attr has other value, return directly
	part_name=$1
	attrn=$2
	retv=$(attrib_get ${part_name} ${attrn})
	test "" == "${retv}" && retv=$(attrib_get DEFAULT ${attrn})
	#attr have either default or specified value, if it is none, we empty it, so we don't pass it to parted
	test "none" == "${retv}" && retv=""
	echo ${retv}
}

wait_part_dev()
{
	wait_dev=$1
	for tmo in $(seq 1 10); do
		if [ -b ${wait_dev} ]; then
			real_dev=${wait_dev}
			test -L ${wait_dev} && real_dev=$(readlink ${wait_dev})
			real_dev=$(basename ${real_dev})
			BLK_SIZE=$(cat /sys/class/block/${real_dev}/size)
			if [ ! -z ${BLK_SIZE} ]; then
				if [ "${BLK_SIZE}" -eq 0 ]; then
					echo "Block size is not ready, size is 0"
					blockdev --rereadpt ${TARGET_DEV}
					sleep 1
					continue
				fi
				break
			fi
		fi
		echo "Waiting for ${wait_dev}"
		blockdev --rereadpt ${TARGET_DEV}
		sleep 1
	done
	test -b ${wait_dev} || return -1
	return 0
}

create_part()
{
	target_dev=${TARGET_DEV}
	part_name=$1
	part_format=$2
	#normal attributes
	part_start=$(attrib_get ${part_name} PART_START)${PART_UNIT}
	part_end=$(attrib_get ${part_name} PART_END)${PART_UNIT}
	part_type=$(attrib_get ${part_name} PART_TYPE)
	part_dev=${target_dev}${PARTDEV_CHAR}$(attrib_get ${part_name} PART_NUM)

	#parse_part_attr type of attributes
	part_fs=$(parse_part_attr ${part_name} PART_FS)
	part_mkfs=$(parse_part_attr ${part_name} PART_MKFS)
	part_mkfs_opt=$(parse_part_attr ${part_name} PART_MKFS_OPT)
    test -z ${part_mkfs_opt} || part_mkfs_opt=-${part_mkfs_opt}
	part_label=$(parse_part_attr ${part_name} PART_LABEL)
	part_label_opt=$(parse_part_attr ${part_name} PART_LABEL_OPT)
    test -z ${part_label_opt} || part_label_opt=-${part_label_opt}
	echo "====>Creating Partition ${part_type} ${part_name} ${part_fs} ${part_start} ${part_end}"
	parted -s ${target_dev} "mkpart ${part_type} ${part_fs} ${part_start} ${part_end}" || \
		error_quit "failed to create partition ${part_name} ${part_fs} ${part_start} ${part_end}"
	test -z "${part_mkfs}" && return
	wait_part_dev ${part_dev} || error_quit "${part_dev} not ready"
	echo "Creating ${part_fs} on ${part_dev}, label ${part_name}"
	mkfs.${part_mkfs} ${part_label_opt} ${part_name} ${part_mkfs_opt} ${part_dev} || \
		error_quit "Failed to create ${part_fs} on ${part_dev}"

	if [ ${part_name} == "FIRMWARE" ]; then
		parted -s ${target_dev} set 1 boot on
		parted -s ${target_dev} set 1 lba on
	fi

#	test -z "${part_label}" && return
#	${part_label} ${part_dev} ${part_name}
}

copy_image()
{
	src_dir=$1
	part_name=$2
	echo "Installing ${part_name}"
	dev_path=/dev/disk/by-label/${part_name}
	mnt_target=/mnt/target
	mkdir -p ${mnt_target}
	if [ ! -b ${dev_path} ]; then
		echo "Try rereading partition table"
		blockdev --rereadpt ${TARGET_DEV}
		test -b ${dev_path} || error_quit "${dev_path} is not a block device"
	fi
	
	mount ${dev_path} ${mnt_target} || error_quit "Failed ot mount ${dev_path} on ${mnt_target}"

	cp -a ${src_dir}/* ${mnt_target}/ || error_quit "Failed to copy image files from ${src_dir} to ${mnt_target}"
	sync
	umount ${src_dir} ||error_quit "Failed to umount ${src_dir}"
}

check_label()
{
	target_dev=${TARGET_DEV}
	part_name=$1
	part_dev=${target_dev}${PARTDEV_CHAR}$(attrib_get ${part_name} PART_NUM)
	dev_path=/dev/disk/by-label/${part_name}
	wait_part_dev ${dev_path} || error_quit "${dev_path} not ready"
	
	if [ "$(basename $(readlink ${dev_path}))" != "$(basename ${part_dev})" ]; then
		ls -l ${dev_path}
		error_quit "${dev_path} points to a difference device from ${part_dev}"
	fi
}

#must be in the following order
parted -s $TARGET_DEV mklabel msdos

for part in FIRMWARE SYSTEM1 SYSTEM2 EXT CONFIG KEY VAR IMG; do 
	create_part $part
done


ls -l /dev/disk/by-label
#copy_image /install/boot FIRMWARE
#copy_image /install/root SYSTEM1
#copy_image /install/root SYSTEM2

echo "Using VAR partition to hold temp files for installation"
mkdir -p /var/log/
mount /dev/disk/by-label/VAR /var/log/


for part in FIRMWARE SYSTEM1 SYSTEM2 CONFIG KEY VAR IMG; do
	check_label $part
done
