#!/bin/sh
set -x
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
rm -rf /dev/ubi*
log_mount_size=240MiB

major=$(cat /sys/class/misc/ubi_ctrl/dev|sed "s/:/ /g"|awk '{print $1}')
minor=$(cat /sys/class/misc/ubi_ctrl/dev|sed "s/:/ /g"|awk '{print $2}') 

#echo "ubi ctrl major $major minor $minor"
rm -f /dev/ubi_ctrl
mknod /dev/ubi_ctrl c $major $minor

create_char_dev()
{
	dev_type=$1
	dev_name=$2
	if [ ! -e /sys/class/${dev_type}/${dev_name}/dev ]; then
		return 1
	fi
	mknod /dev/$dev_name c $(cat /sys/class/${dev_type}/${dev_name}/dev |sed "s/:/ /")
}

create_mtd_links()
{
	rm -rf /dev/mtd*
	rm -rf ${MTDDEV_PATH}
	mkdir -p ${MTDDEV_PATH}
	for mtddev in $(ls /sys/class/mtd/|grep -v ro); do 
		create_char_dev mtd ${mtddev}
		mtd_link_name=$(cat /sys/class/mtd/${mtddev}/name|sed "s/ /_/g")
		ln -s /dev/${mtddev} ${MTDDEV_PATH}/${mtd_link_name}
	done
}


create_ubi_vol()
{
	mtd_devname=$1
	vol_size=$2
	ubi_id=$3
	path=$4
	echo "Format and create ubi on ${mtd_devname} size $vol_size"
	umount $path > /dev/null
	ubidetach /dev/ubi_ctrl -p ${mtd_devname} > /dev/null
	ubiformat ${mtd_devname} -y
	ubiattach /dev/ubi_ctrl -p ${mtd_devname} -d $ubi_id
	major=$(cat /proc/devices|grep ubi${ubi_id}|awk '{print $1}')
	mknod /dev/ubi${ubi_id} c $major 0
	ubimkvol /dev/ubi${ubi_id} -N vol0 -s $vol_size
#	mount_ubi_vol 1 $path
}

create_char_dev misc ubi_ctrl
create_mtd_links

for mp in config log; do 
	mountdev=$(attrib_get $mp mount_dev)
	mountpath=$(attrib_get $mp mount_path)
	mountubi=$(attrib_get $mp mount_ubi)
	mountsize=$(attrib_get $mp mount_size)
	create_ubi_vol /dev/mtd/$mountdev $mountsize $mountubi $mountpath
	if [ ! $? -eq 0 ]; then
		echo "Failed to mount $mountdev"
	fi
done

#mount_ubi_vol 1 $config_path || create_ubi_vol 1 20MiB $config_path || exit 1
#mount_ubi_vol 2 $log_path || create_ubi_vol 2 240MiB $log_path || exit 1

