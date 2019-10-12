#!/bin/sh
#set -x
#echo "Mount flash ubifs!"
. /usr/sbin/silc_util.sh

MTDDEV_PATH=/dev/mtd

config_mount_dev=NAND_Parameters
config_mount_ubi=0
config_mount_path=/config
config_mount_size=20MiB
log_mount_dev=NAND_Log_files
log_mount_ubi=1
log_mount_path=/var/log
log_mount_size=240MiB

format_ubi_vol()
{
	mtd_devname=$1
	vol_size=$2
	ubi_id=$3
	path=$4
	echo "Format ubi on ${mtd_devname} path $path"
	umount $path > /dev/null
	ubidetach /dev/ubi_ctrl -p ${mtd_devname} > /dev/null
	ubiformat ${mtd_devname} -y
	ubiattach /dev/ubi_ctrl -p ${mtd_devname} -d $ubi_id
	major=$(cat /proc/devices|grep ubi${ubi_id}|awk '{print $1}')
	mknod /dev/ubi${ubi_id} c $major 0
	ubimkvol /dev/ubi${ubi_id} -N vol0 -s $vol_size
}

for mp in config log; do 
	mountdev=$(attrib_get $mp mount_dev)
	mountpath=$(attrib_get $mp mount_path)
	mountubi=$(attrib_get $mp mount_ubi)
	mountsize=$(attrib_get $mp mount_size)
	format_ubi_vol /dev/mtd/$mountdev $mountsize $mountubi $mountpath
	if [ ! $? -eq 0 ]; then
		echo "Failed to format $mountdev"
	fi
done


